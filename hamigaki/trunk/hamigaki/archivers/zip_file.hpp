//  zip_file.hpp: Phil Katz Zip file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_FILE_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_FILE_HPP

#include <hamigaki/archivers/detail/raw_zip_file_sink_impl.hpp>
#include <hamigaki/archivers/detail/raw_zip_file_source_impl.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <boost/functional/hash/hash.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/crc.hpp>
#include <boost/none.hpp>
#include <boost/ref.hpp>

namespace hamigaki { namespace archivers {

namespace detail
{

inline boost::iostreams::zlib_params make_zlib_params()
{
    boost::iostreams::zlib_params params;
    params.noheader = true;
    return params;
}

class zip_encryption_keys
{
public:
    explicit zip_encryption_keys(const std::string& s)
        : key0_(boost::detail::reflector<32>::reflect(0x12345678))
        , key1_(0x23456789)
        , key2_(boost::detail::reflector<32>::reflect(0x34567890))
    {
        for (std::size_t i = 0; i < s.size(); ++i)
            update_keys(s[i]);
    }

    char decrypt(char c)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        uc = static_cast<unsigned char>(uc ^ decrypt_byte());
        c = static_cast<char>(uc);
        update_keys(c);
        return c;
    }

    char encrypt(char c)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        uc = static_cast<unsigned char>(uc ^ decrypt_byte());
        update_keys(c);
        return static_cast<char>(uc);
    }

private:
    typedef boost::crc_optimal<32,0x04C11DB7,0,0,true,true> crc_type;

    crc_type key0_;
    boost::uint32_t key1_;
    crc_type key2_;

    void update_keys(char c)
    {
        key0_.process_byte(static_cast<unsigned char>(c));
        key1_ += (key0_.checksum() & 0xFF);
        key1_ = key1_ * 134775813 + 1;
        key2_.process_byte(static_cast<unsigned char>(key1_ >> 24));
    }

    unsigned char decrypt_byte()
    {
        boost::uint32_t temp = (key2_.checksum() | 2) & 0xFFFF;
        return static_cast<unsigned char>((temp * (temp ^ 1)) >> 8);
    }
};

template<typename T>
inline T binary_hash(const T& x)
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

template<class Source>
class zip_decrypter
{
private:
    typedef detail::basic_raw_zip_file_source_impl<Source> raw_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    explicit zip_decrypter(const Source& src)
        : raw_(src)
    {
    }

    void password(const std::string& pswd)
    {
        password_ = pswd;
    }

    bool next_entry()
    {
        if (!raw_.next_entry())
            return false;

        prepare_reading();
        return true;
    }

    void select_entry(const boost::filesystem::path& ph)
    {
        raw_.select_entry(ph);
        prepare_reading();
    }

    zip::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if (header_.encrypted && !keys_)
            init_keys();

        std::streamsize amt = raw_.read(s, n);
        if (header_.encrypted)
        {
            for (std::streamsize i = 0; i < amt; ++i)
                s[i] = keys_->decrypt(s[i]);
        }
        return amt;
    }

private:
    raw_type raw_;
    zip::header header_;
    std::string password_;
    char enc_header_[zip::consts::encryption_header_size];
    boost::optional<zip_encryption_keys> keys_;

    void prepare_reading()
    {
        keys_ = boost::none;

        header_ = raw_.header();
        if (header_.encrypted)
        {
            if (header_.compressed_size < 12)
                throw std::runtime_error("bad ZIP encryption");
            header_.compressed_size -= 12;

            raw_.read(enc_header_, sizeof(enc_header_));
        }
    }

    void init_keys()
    {
        zip_encryption_keys keys(password_);

        char buf[sizeof(enc_header_)];
        for (std::size_t i = 0; i < sizeof(buf); ++i)
            buf[i] = keys.decrypt(enc_header_[i]);

        boost::uint16_t cs =
            hamigaki::decode_uint<little,2>(buf + (sizeof(buf)-2));
        if (!header_.match_encryption_checksum(cs))
            throw password_incorrect();

        keys_ = keys;
    }
};

template<class Sink>
class zip_encrypter
{
private:
    typedef detail::basic_raw_zip_file_sink_impl<Sink> raw_zip_type;

public:
    typedef char char_type;

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

    void create_entry(const zip::header& head)
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
        if (keys_)
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

    // void close();

    void close(boost::uint32_t crc32_checksum, boost::uint32_t file_size)
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
    zip::header header_;
    std::string password_;
    boost::optional<zip_encryption_keys> keys_;

    void prepare_writing()
    {
        if (header_.encrypted)
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

} // namespace detail

template<class Source>
class basic_zip_file_source_impl
{
private:
    typedef detail::zip_decrypter<Source> raw_type;

public:
    explicit basic_zip_file_source_impl(const Source& src)
        : raw_(src)
        , zlib_(detail::make_zlib_params())
    {
    }

    void password(const std::string& pswd)
    {
        raw_.password(pswd);
    }

    bool next_entry()
    {
        if (!raw_.next_entry())
            return false;

        prepare_reading();
        return true;
    }

    void select_entry(const boost::filesystem::path& ph)
    {
        raw_.select_entry(ph);
        prepare_reading();
    }

    zip::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize amt = read_impl(s, n);
        if (amt != -1)
            crc32_.process_bytes(s, amt);
        else if (crc32_.checksum() != header_.crc32_checksum)
            throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");
        return amt;
    }

private:
    raw_type raw_;
    zip::header header_;
    boost::crc_32_type crc32_;
    boost::iostreams::zlib_decompressor zlib_;
    boost::iostreams::bzip2_decompressor bzip2_;

    std::streamsize read_impl(char* s, std::streamsize n)
    {
        if (header_.method == zip::method::store)
            return raw_.read(s, n);
        else if (header_.method == zip::method::deflate)
            return boost::iostreams::read(zlib_, boost::ref(raw_), s, n);
        else if (header_.method == zip::method::bzip2)
            return boost::iostreams::read(bzip2_, boost::ref(raw_), s, n);

        return -1;
    }

    void prepare_reading()
    {
        header_ = raw_.header();

        if ((header_.method != zip::method::store) &&
            (header_.method != zip::method::deflate) &&
            (header_.method != zip::method::bzip2) )
        {
            throw std::runtime_error("unsupported ZIP format");
        }

        crc32_.reset();
        boost::iostreams::close(zlib_, boost::ref(raw_), BOOST_IOS::in);
        boost::iostreams::close(bzip2_, boost::ref(raw_), BOOST_IOS::in);

        if ((header_.permission & 0170000) == 0120000)
        {
            using namespace boost::filesystem;

            boost::scoped_array<char> buf(new char[header_.file_size+1]);
            read(buf.get(), header_.file_size);
            buf[header_.file_size] = '\0';

            header_.link_path = path(buf.get(), no_check);
            header_.compressed_size = 0;
            header_.file_size = 0;
        }
    }
};

template<class Source>
class basic_zip_file_source
{
private:
    typedef basic_zip_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef zip::header header_type;

    explicit basic_zip_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    void password(const std::string& pswd)
    {
        pimpl_->password(pswd);
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    void select_entry(const boost::filesystem::path& ph)
    {
        pimpl_->select_entry(ph);
    }

    zip::header header() const
    {
        return pimpl_->header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class zip_file_source
    : public basic_zip_file_source<iostreams::file_source>
{
    typedef basic_zip_file_source<iostreams::file_source> base_type;

public:
    explicit zip_file_source(const std::string& filename)
        : base_type(iostreams::file_source(filename, BOOST_IOS::binary))
    {
    }
};


template<class Sink>
class basic_zip_file_sink_impl
{
private:
    typedef detail::zip_encrypter<Sink> raw_zip_type;

public:
    explicit basic_zip_file_sink_impl(const Sink& sink)
        : raw_(sink), method_(zip::method::store), size_(0)
        , zlib_(detail::make_zlib_params())
    {
    }

    void password(const std::string& pswd)
    {
        raw_.password(pswd);
    }

    void create_entry(const zip::header& head)
    {
        zip::header header = head;

        const std::string link_path = header.link_path.string();

        if (!link_path.empty())
        {
            header.method = zip::method::store;
            header.file_size = link_path.size();
        }
        else if (header.file_size < 6)
            header.method = zip::method::store;

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
        else if (method_ == zip::method::bzip2)
            boost::iostreams::close(bzip2_, boost::ref(raw_), BOOST_IOS::out);

        raw_.close(crc32_.checksum(), size_);
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
    boost::uint32_t size_;
    boost::iostreams::zlib_compressor zlib_;
    boost::iostreams::bzip2_compressor bzip2_;

    std::streamsize write_impl(const char* s, std::streamsize n)
    {
        if (method_ == zip::method::store)
            return raw_.write(s, n);
        else if (method_ == zip::method::deflate)
            return boost::iostreams::write(zlib_, boost::ref(raw_), s, n);
        else if (method_ == zip::method::bzip2)
            return boost::iostreams::write(bzip2_, boost::ref(raw_), s, n);

        return n;
    }
};

template<class Sink>
class basic_zip_file_sink
{
private:
    typedef basic_zip_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef zip::header header_type;

    explicit basic_zip_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void password(const std::string& pswd)
    {
        pimpl_->password(pswd);
    }

    void create_entry(const zip::header& head)
    {
        pimpl_->create_entry(head);
    }

    void rewind_entry()
    {
        pimpl_->rewind_entry();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    void close()
    {
        pimpl_->close();
    }

    void close_archive()
    {
        pimpl_->close_archive();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class zip_file_sink
    : public basic_zip_file_sink<iostreams::file_sink>
{
private:
    typedef basic_zip_file_sink<iostreams::file_sink> base_type;

public:
    explicit zip_file_sink(const std::string& filename)
        : base_type(iostreams::file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ZIP_FILE_HPP
