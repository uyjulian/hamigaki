// zip_file_sink_impl.hpp: ZIP file sink implementation

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ZIP_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ZIP_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/raw_zip_file_sink_impl.hpp>
#include <hamigaki/archivers/detail/zip_encryption_keys.hpp>
#include <hamigaki/archivers/detail/zlib_params.hpp>
#include <boost/functional/hash/hash.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/none.hpp>
#include <boost/ref.hpp>

#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
    #include <hamigaki/archivers/detail/bzip2.hpp>
#endif

namespace hamigaki { namespace archivers { namespace detail {

template<typename T>
inline std::size_t binary_hash(const T& x)
{
    char buf[sizeof(T)];
    std::memcpy(buf, &x, sizeof(T));
    return boost::hash_range(&buf[0], &buf[0]+sizeof(buf));
}

inline boost::uint32_t make_random_seed()
{
    std::size_t val = detail::binary_hash(std::time(0));
    val ^= detail::binary_hash(std::clock());
    return static_cast<boost::uint32_t>(val);
}

template<class Sink, class Path>
class zip_encrypter
{
private:
    typedef basic_raw_zip_file_sink_impl<Sink,Path> raw_zip_type;

public:
    typedef char char_type;
    typedef zip::basic_header<Path> header_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit zip_encrypter(const Sink& sink)
        : raw_(sink), rand_gen_(make_random_seed())
    {
    }

    void password(const std::string& pswd)
    {
        password_ = pswd;
    }

    void create_entry(const header_type& head)
    {
        header_ = head;
        raw_.create_entry(header_);
        prepare_writing();
    }

    void rewind_entry()
    {
        raw_.rewind_entry();
        prepare_writing();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if (header_.encrypted && (header_.compressed_size == 0))
        {
            char buf[1024];
            std::streamsize total = 0;
            while (total < n)
            {
                std::streamsize amt = (std::min)(
                    n-total, static_cast<std::streamsize>(sizeof(buf)));

                for (std::streamsize i = 0; i < amt; ++i)
                    buf[i] = keys_->encrypt(s[total+i]);
                raw_.write(buf, amt);
                total += amt;
            }
            return total;
        }
        else
            return raw_.write(s, n);
    }

    void close()
    {
        raw_.close();
    }

    void close(boost::uint32_t crc32_checksum, boost::uint64_t file_size)
    {
        raw_.close(crc32_checksum, file_size);
    }

    void close_archive()
    {
        raw_.close_archive();
    }

private:
    raw_zip_type raw_;
    boost::mt19937 rand_gen_;
    header_type header_;
    std::string password_;
    boost::optional<zip_encryption_keys> keys_;

    void prepare_writing()
    {
        if (header_.encrypted && (header_.compressed_size == 0))
        {
            keys_ = zip_encryption_keys(password_);

            char enc_haeder[zip::consts::encryption_header_size];

            boost::variate_generator<
                boost::mt19937&,
                boost::uniform_int<unsigned char>
            > rand(rand_gen_, boost::uniform_int<unsigned char>(0, 0xFF));

            char* beg = &enc_haeder[0];
            char* end = beg + sizeof(enc_haeder);

            if (header_.version < 20)
            {
                end -= 2;
                hamigaki::encode_uint<little,2>(
                    end, msdos::date_time(header_.update_time).time);
            }
            else
            {
                --end;
                *end = static_cast<char>(static_cast<unsigned char>(
                    msdos::date_time(header_.update_time).time >> 8));
            }

            while (beg != end)
                *(beg++) = static_cast<char>(rand());

            for (std::size_t i = 0; i < sizeof(enc_haeder); ++i)
                enc_haeder[i] = keys_->encrypt(enc_haeder[i]);
            raw_.write(enc_haeder, sizeof(enc_haeder));

            if (header_.is_directory())
                close(0, 0);
        }
        else
            keys_ = boost::none;
    }
};

template<class Sink, class Path>
class basic_zip_file_sink_impl
{
private:
    typedef zip_encrypter<Sink,Path> raw_zip_type;

public:
    typedef zip::basic_header<Path> header_type;

    explicit basic_zip_file_sink_impl(const Sink& sink)
        : raw_(sink), method_(zip::method::store), size_(0), compressed_(false)
        , zlib_(make_zlib_params())
    {
    }

    void password(const std::string& pswd)
    {
        raw_.password(pswd);
    }

    void create_entry(const header_type& head)
    {
        header_type header = head;

        const std::string link_path =
            detail::make_zip_path_string(header.link_path);

        if (!link_path.empty())
        {
            header.method = zip::method::store;
            header.file_size = link_path.size();
        }
        else if (header.compressed_size != 0)
            compressed_ = true;
        else if (header.file_size < 6)
            header.method = zip::method::store;

        if ((header.method != zip::method::deflate) &&
#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
            (header.method != zip::method::bzip2) &&
#endif
            (header.method != zip::method::store))
        {
            throw std::runtime_error("unsupported ZIP format");
        }

        if (compressed_)
            method_ = zip::method::store;
        else
            method_ = header.method;
        raw_.create_entry(header);
        size_ = 0;

        if (!link_path.empty())
        {
            write(link_path.c_str(), link_path.size());
            close();
        }
    }

    void rewind_entry()
    {
        raw_.rewind_entry();
        method_ = zip::method::store;
        size_ = 0;
        crc32_.reset();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt = write_impl(s, n);
        if (amt != 0)
        {
            crc32_.process_bytes(s, amt);
            size_ += amt;
        }
        return amt;
    }

    void close()
    {
        if (method_ == zip::method::deflate)
            boost::iostreams::close(zlib_, boost::ref(raw_), BOOST_IOS::out);
#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
        else if (method_ == zip::method::bzip2)
            boost::iostreams::close(bzip2_, boost::ref(raw_), BOOST_IOS::out);
#endif

        if (compressed_)
            raw_.close();
        else
            raw_.close(crc32_.checksum(), size_);
        compressed_ = false;
        crc32_.reset();
    }

    void close_archive()
    {
        raw_.close_archive();
    }

private:
    raw_zip_type raw_;
    boost::uint16_t method_;
    boost::crc_32_type crc32_;
    boost::uint64_t size_;
    bool compressed_;
    boost::iostreams::zlib_compressor zlib_;
#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
    boost::iostreams::bzip2_compressor bzip2_;
#endif

    std::streamsize write_impl(const char* s, std::streamsize n)
    {
        if (method_ == zip::method::store)
            return raw_.write(s, n);
        else if (method_ == zip::method::deflate)
            return boost::iostreams::write(zlib_, boost::ref(raw_), s, n);
#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
        else if (method_ == zip::method::bzip2)
            return boost::iostreams::write(bzip2_, boost::ref(raw_), s, n);
#endif

        return n;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ZIP_FILE_SINK_IMPL_HPP
