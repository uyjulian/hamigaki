// iso_file.hpp: ISO image file device

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_FILE_HPP
#define HAMIGAKI_ARCHIVERS_ISO_FILE_HPP

#include <hamigaki/archivers/detail/iso_file_sink_impl.hpp>
#include <hamigaki/archivers/detail/iso_file_source_impl.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

namespace hamigaki { namespace archivers {

template<class Source, class Path=boost::filesystem::path>
class basic_iso_file_source
{
private:
    typedef detail::basic_iso_file_source_impl<Source,Path> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef Path path_type;
    typedef iso::basic_header<Path> header_type;
    typedef iso::basic_volume_desc<Path> volume_desc;

    explicit basic_iso_file_source(const Source& src)
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

    const std::vector<volume_desc>& volume_descs() const
    {
        return pimpl_->volume_descs();
    }

    void select_volume_desc(std::size_t index, bool use_rrip=true)
    {
        pimpl_->select_volume_desc(index, use_rrip);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class iso_file_source
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef boost::filesystem::path path_type;
    typedef iso::header header_type;
    typedef iso::volume_desc volume_desc;

    explicit iso_file_source(const std::string& filename)
        : impl_(iostreams::file_source(filename, BOOST_IOS::binary))
    {
    }

    bool next_entry()
    {
        return impl_.next_entry();
    }

    iso::header header() const
    {
        return impl_.header();
    }

    const std::vector<iso::volume_desc>& volume_descs() const
    {
        return impl_.volume_descs();
    }

    void select_volume_desc(std::size_t index, bool use_rrip=true)
    {
        impl_.select_volume_desc(index, use_rrip);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_iso_file_source<iostreams::file_source> impl_;
};


template<class Sink, class Path=boost::filesystem::path>
class basic_iso_file_sink
{
private:
    typedef detail::basic_iso_file_sink_impl<Sink,Path> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef Path path_type;
    typedef iso::basic_header<Path> header_type;
    typedef iso::basic_volume_desc<Path> volume_desc;

    explicit basic_iso_file_sink(
            const Sink& sink, const iso::volume_info& info=iso::volume_info(),
            boost::uint32_t max_extent_size = 0)
        : pimpl_(new impl_type(sink, info, max_extent_size))
    {
    }

    void add_volume_desc(const volume_desc& desc)
    {
        pimpl_->add_volume_desc(desc);
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

class iso_file_sink
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef boost::filesystem::path path_type;
    typedef iso::header header_type;
    typedef iso::volume_desc volume_desc;

    explicit iso_file_sink(
        const std::string& filename,
        const iso::volume_info& info=iso::volume_info(),
        boost::uint32_t max_extent_size = 0)
        : impl_(
            iostreams::file_sink(filename, BOOST_IOS::binary),
            info, max_extent_size
        )
    {
    }

    void add_volume_desc(const iso::volume_desc& desc)
    {
        impl_.add_volume_desc(desc);
    }

    void create_entry(const iso::header& head)
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
    basic_iso_file_sink<iostreams::file_sink> impl_;
};

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
class wiso_file_source
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef boost::filesystem::wpath path_type;
    typedef iso::wheader header_type;
    typedef iso::wvolume_desc volume_desc;

    explicit wiso_file_source(const std::string& filename)
        : impl_(iostreams::file_source(filename, BOOST_IOS::binary))
    {
    }

    bool next_entry()
    {
        return impl_.next_entry();
    }

    iso::wheader header() const
    {
        return impl_.header();
    }

    const std::vector<iso::wvolume_desc>& volume_descs() const
    {
        return impl_.volume_descs();
    }

    void select_volume_desc(std::size_t index, bool use_rrip=true)
    {
        impl_.select_volume_desc(index, use_rrip);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_iso_file_source<iostreams::file_source,path_type> impl_;
};

class wiso_file_sink
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef boost::filesystem::wpath path_type;
    typedef iso::wheader header_type;
    typedef iso::wvolume_desc volume_desc;

    explicit wiso_file_sink(
        const std::string& filename,
        const iso::volume_info& info=iso::volume_info(),
        boost::uint32_t max_extent_size = 0)
        : impl_(
            iostreams::file_sink(filename, BOOST_IOS::binary),
            info, max_extent_size
        )
    {
    }

    void add_volume_desc(const iso::wvolume_desc& desc)
    {
        impl_.add_volume_desc(desc);
    }

    void create_entry(const iso::wheader& head)
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
    basic_iso_file_sink<iostreams::file_sink,path_type> impl_;
};
#endif // !defined(BOOST_FILESYSTEM_NARROW_ONLY)

} } // End namespaces archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO_FILE_HPP
