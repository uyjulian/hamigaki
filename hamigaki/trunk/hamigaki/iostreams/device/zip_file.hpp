//  zip_file.hpp: Phil Katz Zip file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP

#include <hamigaki/iostreams/device/raw_zip_file.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/crc.hpp>
#include <boost/ref.hpp>

namespace hamigaki { namespace iostreams {

namespace zip
{

namespace detail
{

inline boost::iostreams::zlib_params make_zlib_params()
{
    boost::iostreams::zlib_params params;
    params.noheader = true;
    return params;
}

} // namespace detail

} // namespace zip

template<class Source>
class basic_zip_file_source_impl
{
private:
    typedef basic_raw_zip_file_source_impl<Source> raw_type;

public:
    explicit basic_zip_file_source_impl(const Source& src)
        : raw_(src), method_(0)
        , zlib_(zip::detail::make_zlib_params())
    {
    }

    bool next_entry()
    {
        if (!raw_.next_entry())
            return false;

        const zip::header& head = raw_.header();
        method_ = head.method;

        if ((method_ != 0) && (method_ != 8))
            throw BOOST_IOSTREAMS_FAILURE("unsupported ZIP format");

        crc32_.reset();
        boost::iostreams::close(zlib_, boost::ref(raw_), BOOST_IOS::in);

        return true;
    }

    zip::header header() const
    {
        return raw_.header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize amt = read_impl(s, n);
        if (amt != -1)
            crc32_.process_bytes(s, amt);
        else if (crc32_.checksum() != raw_.header().crc32_checksum)
            throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");
        return amt;
    }

private:
    raw_type raw_;
    boost::uint16_t method_;
    boost::crc_32_type crc32_;
    boost::iostreams::zlib_decompressor zlib_;

    std::streamsize read_impl(char* s, std::streamsize n)
    {
        if (method_ == 0)
            return raw_.read(s, n);
        else if (method_ == 8)
            return boost::iostreams::read(zlib_, boost::ref(raw_), s, n);

        return -1;
    }
};

template<class Source>
class basic_zip_file_source
{
private:
    typedef basic_zip_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_zip_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
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

class zip_file_source : public basic_zip_file_source<file_source>
{
    typedef basic_zip_file_source<file_source> base_type;

public:
    explicit zip_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP
