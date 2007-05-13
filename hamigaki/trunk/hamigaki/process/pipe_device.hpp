// pipe_device.hpp: pipe device

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_PIPE_DEVICE_HPP
#define HAMIGAKI_PROCESS_PIPE_DEVICE_HPP

#include <hamigaki/process/detail/config.hpp>
#include <hamigaki/process/detail/auto_link.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/shared_ptr.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251)
#endif

namespace hamigaki { namespace process {

#if defined(BOOST_WINDOWS)
typedef void* handle_type;
#else
typedef int handle_type;
#endif

class HAMIGAKI_PROCESS_DECL pipe_source
{
public:
    typedef char char_type;

    struct category
        : public boost::iostreams::input
        , public boost::iostreams::device_tag
        , public boost::iostreams::closable_tag
    {};

    pipe_source();
    explicit pipe_source(handle_type h, bool close_on_exit = false);
    bool is_open() const;
    std::streamsize read(char* s, std::streamsize n);
    void close();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

class HAMIGAKI_PROCESS_DECL pipe_sink
{
public:
    typedef char char_type;

    struct category
        : public boost::iostreams::output
        , public boost::iostreams::device_tag
        , public boost::iostreams::closable_tag
    {};

    pipe_sink();
    explicit pipe_sink(handle_type h, bool close_on_exit = false);
    bool is_open() const;
    std::streamsize write(const char* s, std::streamsize n);
    void close();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};


} } // End namespaces process, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_PROCESS_PIPE_DEVICE_HPP
