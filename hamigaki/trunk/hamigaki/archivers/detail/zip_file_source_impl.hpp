// zip_file_source_impl.hpp: ZIP file source implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ZIP_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ZIP_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/raw_zip_file_source_impl.hpp>
#include <hamigaki/archivers/detail/zip_encryption_keys.hpp>
#include <hamigaki/archivers/detail/zlib_params.hpp>
#include <boost/none.hpp>
#include <boost/ref.hpp>

#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
    #include <hamigaki/archivers/detail/bzip2.hpp>
#endif

namespace hamigaki { namespace archivers { namespace detail {

template<class Source, class Path>
class zip_decrypter
{
private:
    typedef basic_raw_zip_file_source_impl<Source,Path> raw_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef Path path_type;
    typedef zip::basic_header<Path> header_type;

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

    void select_entry(const Path& ph)
    {
        raw_.select_entry(ph);
        prepare_reading();
    }

    header_type header() const
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
    header_type header_;
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

template<class Source, class Path=boost::filesystem::path>
class basic_zip_file_source_impl
{
private:
    typedef zip_decrypter<Source,Path> raw_type;

public:
    typedef Path path_type;
    typedef zip::basic_header<Path> header_type;

    explicit basic_zip_file_source_impl(const Source& src)
        : raw_(src)
        , zlib_(make_zlib_params())
    {
        header_.method = zip::method::store;
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

    void select_entry(const Path& ph)
    {
        raw_.select_entry(ph);
        prepare_reading();
    }

    header_type header() const
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
    header_type header_;
    boost::crc_32_type crc32_;
    boost::iostreams::zlib_decompressor zlib_;
#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
    boost::iostreams::bzip2_decompressor bzip2_;
#endif

    std::streamsize read_impl(char* s, std::streamsize n)
    {
        if (header_.method == zip::method::store)
            return raw_.read(s, n);
        else if (header_.method == zip::method::deflate)
            return boost::iostreams::read(zlib_, boost::ref(raw_), s, n);
#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
        else if (header_.method == zip::method::bzip2)
            return boost::iostreams::read(bzip2_, boost::ref(raw_), s, n);
#endif

        return -1;
    }

    void prepare_reading()
    {
        boost::uint16_t old_method = header_.method;

        header_ = raw_.header();

        if ((header_.method != zip::method::deflate) &&
#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
            (header_.method != zip::method::bzip2) &&
#endif
            (header_.method != zip::method::store))
        {
            throw std::runtime_error("unsupported ZIP format");
        }

        crc32_.reset();
        if (old_method == zip::method::deflate)
            boost::iostreams::close(zlib_, boost::ref(raw_), BOOST_IOS::in);
#if !defined(HAMIGAKI_ARCHIVERS_NO_BZIP2)
        if (old_method == zip::method::bzip2)
            boost::iostreams::close(bzip2_, boost::ref(raw_), BOOST_IOS::in);
#endif

        if (filesystem::file_permissions::is_symlink(header_.permissions))
        {
            boost::scoped_array<char> buf(new char[header_.file_size+1]);
            read(buf.get(), header_.file_size);
            buf[header_.file_size] = '\0';

            header_.link_path =
                zip_source_traits<Path>::make_path(
                    buf.get(), header_.utf8_encoded
                );

            header_.compressed_size = 0;
            header_.file_size = 0;
        }
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ZIP_FILE_SOURCE_IMPL_HPP
