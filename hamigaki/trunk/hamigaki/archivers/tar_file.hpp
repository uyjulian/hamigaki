// tar_file.hpp: POSIX tar file device

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_ARCHIVERS_TAR_FILE_HPP
#define HAMIGAKI_ARCHIVERS_TAR_FILE_HPP

#include <hamigaki/archivers/detail/tar_file_sink_impl.hpp>
#include <hamigaki/archivers/detail/tar_file_source_impl.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <boost/shared_ptr.hpp>

namespace hamigaki { namespace archivers {

template<class Source, class Path=boost::filesystem::path>
class basic_tar_file_source
{
private:
    typedef detail::basic_tar_file_source_impl<Source,Path> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef Path path_type;
    typedef tar::basic_header<Path> header_type;

    explicit basic_tar_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    header_type header() const
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

class tar_file_source
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef boost::filesystem::path path_type;
    typedef tar::header header_type;

    explicit tar_file_source(const std::string& filename)
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
    basic_tar_file_source<iostreams::file_source> impl_;
};


template<class Sink, class Path=boost::filesystem::path>
class basic_tar_file_sink
{
private:
    typedef detail::basic_tar_file_sink_impl<Sink,Path> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef Path path_type;
    typedef tar::basic_header<Path> header_type;

    explicit basic_tar_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void create_entry(const header_type& head)
    {
        pimpl_->create_entry(head);
    }

    void rewind_entry()
    {
        throw std::runtime_error("unsupported operation");
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

class tar_file_sink
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef boost::filesystem::path path_type;
    typedef tar::header header_type;

    explicit tar_file_sink(const std::string& filename)
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
    basic_tar_file_sink<iostreams::file_sink> impl_;
};

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
class wtar_file_source
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef boost::filesystem::wpath path_type;
    typedef tar::wheader header_type;

    explicit wtar_file_source(const std::string& filename)
        : impl_(iostreams::file_source(filename, BOOST_IOS::binary))
    {
    }

    bool next_entry()
    {
        return impl_.next_entry();
    }

    tar::wheader header() const
    {
        return impl_.header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_tar_file_source<iostreams::file_source,path_type> impl_;
};

class wtar_file_sink
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef boost::filesystem::wpath path_type;
    typedef tar::wheader header_type;

    explicit wtar_file_sink(const std::string& filename)
        : impl_(iostreams::file_sink(filename, BOOST_IOS::binary))
    {
    }

    void create_entry(const tar::wheader& head)
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
    basic_tar_file_sink<iostreams::file_sink,path_type> impl_;
};
#endif // !defined(BOOST_FILESYSTEM_NARROW_ONLY)

} } // End namespaces archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_TAR_FILE_HPP
