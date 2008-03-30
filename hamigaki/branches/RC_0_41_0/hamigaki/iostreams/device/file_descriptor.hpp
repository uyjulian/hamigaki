// file_descriptor.hpp: low-level file device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_FILE_DESCRIPTOR_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_FILE_DESCRIPTOR_HPP

#include <hamigaki/iostreams/detail/config.hpp>
#include <hamigaki/iostreams/detail/auto_link.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251)
#endif

namespace hamigaki { namespace iostreams {

namespace detail
{

class HAMIGAKI_IOSTREAMS_DECL file_descriptor_impl;

} // namespace detail

class HAMIGAKI_IOSTREAMS_DECL file_descriptor_source
{
private:
    typedef detail::file_descriptor_impl impl_type;

public:
    typedef char char_type;

    struct category
        : public boost::iostreams::device_tag
        , public boost::iostreams::input_seekable
        , public boost::iostreams::closable_tag
    {};

    file_descriptor_source()
    {
    }

    explicit file_descriptor_source(
        const std::string& filename, BOOST_IOS::openmode mode=BOOST_IOS::in)
    {
        this->open(filename, mode);
    }

    void open(
        const std::string& filename, BOOST_IOS::openmode mode=BOOST_IOS::in);

    bool is_open() const
    {
        return pimpl_.get() != 0;
    }

    std::streamsize read(char* s, std::streamsize n);

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way);

    void close()
    {
        pimpl_.reset();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class HAMIGAKI_IOSTREAMS_DECL file_descriptor_sink
{
private:
    typedef detail::file_descriptor_impl impl_type;

public:
    typedef char char_type;

    struct category
        : public boost::iostreams::device_tag
        , public boost::iostreams::output_seekable
        , public boost::iostreams::closable_tag
    {};

    file_descriptor_sink()
    {
    }

    explicit file_descriptor_sink(
        const std::string& filename, BOOST_IOS::openmode mode=BOOST_IOS::out)
    {
        this->open(filename, mode);
    }

    void open(
        const std::string& filename, BOOST_IOS::openmode mode=BOOST_IOS::out);

    bool is_open() const
    {
        return pimpl_.get() != 0;
    }

    std::streamsize write(const char* s, std::streamsize n);

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way);

    void close()
    {
        pimpl_.reset();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class HAMIGAKI_IOSTREAMS_DECL file_descriptor
{
private:
    typedef detail::file_descriptor_impl impl_type;

public:
    typedef char char_type;

    struct category
        : public boost::iostreams::device_tag
        , public boost::iostreams::seekable
        , public boost::iostreams::closable_tag
    {};

    file_descriptor()
    {
    }

    explicit file_descriptor(
        const std::string& filename,
        BOOST_IOS::openmode mode=BOOST_IOS::in|BOOST_IOS::out)
    {
        this->open(filename, mode);
    }

    void open(
        const std::string& filename,
        BOOST_IOS::openmode mode=BOOST_IOS::in|BOOST_IOS::out);

    bool is_open() const
    {
        return pimpl_.get() != 0;
    }

    std::streamsize read(char* s, std::streamsize n);
    std::streamsize write(const char* s, std::streamsize n);

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way);

    void close()
    {
        pimpl_.reset();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::file_descriptor_source, 0)

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_IOSTREAMS_DEVICE_FILE_DESCRIPTOR_HPP
