//  tar_file.hpp: POSIX tar file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_ARCHIVERS_TAR_FILE_HPP
#define HAMIGAKI_ARCHIVERS_TAR_FILE_HPP

#include <hamigaki/archivers/detail/tar_file_sink_impl.hpp>
#include <hamigaki/archivers/detail/tar_file_source_impl.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <boost/shared_ptr.hpp>

namespace hamigaki { namespace archivers {

template<class Source>
class basic_tar_file_source
{
private:
    typedef detail::basic_tar_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef tar::header header_type;

    explicit basic_tar_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    tar::header header() const
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
    : public basic_tar_file_source<iostreams::file_source>
{
    typedef basic_tar_file_source<iostreams::file_source> base_type;

public:
    explicit tar_file_source(const std::string& filename)
        : base_type(iostreams::file_source(filename, BOOST_IOS::binary))
    {
    }
};


template<class Sink>
class basic_tar_file_sink
{
private:
    typedef detail::basic_tar_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef tar::header header_type;

    explicit basic_tar_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void create_entry(const tar::header& head)
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
    : public basic_tar_file_sink<iostreams::file_sink>
{
private:
    typedef basic_tar_file_sink<iostreams::file_sink> base_type;

public:
    explicit tar_file_sink(const std::string& filename)
        : base_type(iostreams::file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_TAR_FILE_HPP
