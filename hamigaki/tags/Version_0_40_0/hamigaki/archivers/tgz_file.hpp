// tgz_file.hpp: tar.gz file device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_TGZ_FILE_HPP
#define HAMIGAKI_ARCHIVERS_TGZ_FILE_HPP

#include <hamigaki/archivers/detail/gzip.hpp>
#include <hamigaki/archivers/tar_file.hpp>

namespace hamigaki { namespace archivers {

template<class Source>
class basic_tgz_file_source
{
private:
    typedef boost::iostreams::composite<
        boost::iostreams::gzip_decompressor,
        Source
    > source_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef tar::header header_type;

    explicit basic_tgz_file_source(const Source& src)
        : impl_(source_type(boost::iostreams::gzip_decompressor(), src))
    {
    }

    bool next_entry()
    {
        return impl_.next_entry();
    }

    tar::header header() const
    {
        return impl_.header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_tar_file_source<source_type> impl_;
};

class tgz_file_source
{
private:
    typedef basic_tgz_file_source<iostreams::file_source> base_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef tar::header header_type;

    explicit tgz_file_source(const std::string& filename)
        : impl_(iostreams::file_source(filename, BOOST_IOS::binary))
    {
    }

    bool next_entry()
    {
        return impl_.next_entry();
    }

    tar::header header() const
    {
        return impl_.header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_tgz_file_source<iostreams::file_source> impl_;
};


template<class Sink>
class basic_tgz_file_sink
{
private:
    typedef boost::iostreams::composite<
        boost::iostreams::gzip_compressor,
        Sink
    > sink_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef tar::header header_type;

    explicit basic_tgz_file_sink(const Sink& sink)
        : impl_(sink_type(boost::iostreams::gzip_compressor(), sink))
    {
    }

    void create_entry(const tar::header& head)
    {
        impl_.create_entry(head);
    }

    void rewind_entry()
    {
        throw std::runtime_error("unsupported operation");
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return impl_.write(s, n);
    }

    void close()
    {
        impl_.close();
    }

    void close_archive()
    {
        impl_.close_archive();
    }

private:
    basic_tar_file_sink<sink_type> impl_;
};

class tgz_file_sink
{
private:
    typedef basic_tgz_file_sink<iostreams::file_sink> base_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef tar::header header_type;

    explicit tgz_file_sink(const std::string& filename)
        : impl_(iostreams::file_sink(filename, BOOST_IOS::binary))
    {
    }

    void create_entry(const tar::header& head)
    {
        impl_.create_entry(head);
    }

    void rewind_entry()
    {
        throw std::runtime_error("unsupported operation");
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return impl_.write(s, n);
    }

    void close()
    {
        impl_.close();
    }

    void close_archive()
    {
        impl_.close_archive();
    }

private:
    basic_tgz_file_sink<iostreams::file_sink> impl_;
};

} } // End namespaces archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_TGZ_FILE_HPP
