//  iso_image_file.hpp: ISO image file device

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_IMAGE_FILE_HPP
#define HAMIGAKI_ARCHIVERS_ISO_IMAGE_FILE_HPP

#include <hamigaki/archivers/detail/iso_image_file_sink_impl.hpp>
#include <hamigaki/archivers/detail/iso_image_file_source_impl.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

namespace hamigaki { namespace archivers {

template<class Source>
class basic_iso_image_file_source
{
private:
    typedef detail::basic_iso_image_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef iso9660::header header_type;

    explicit basic_iso_image_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    iso9660::header header() const
    {
        return pimpl_->header();
    }

    const std::vector<iso9660::volume_descriptor>& volume_descriptors() const
    {
        return pimpl_->volume_descriptors();
    }

    void select_volume_descriptor(std::size_t index)
    {
        pimpl_->select_volume_descriptor(index);
    }

    bool is_rock_ridge() const
    {
        return pimpl_->is_rock_ridge();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class iso_image_file_source
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef iso9660::header header_type;

    explicit iso_image_file_source(const std::string& filename)
        : impl_(iostreams::file_source(filename, BOOST_IOS::binary))
    {
    }

    bool next_entry()
    {
        return impl_.next_entry();
    }

    iso9660::header header() const
    {
        return impl_.header();
    }

    const std::vector<iso9660::volume_descriptor>& volume_descriptors() const
    {
        return impl_.volume_descriptors();
    }

    void select_volume_descriptor(std::size_t index)
    {
        impl_.select_volume_descriptor(index);
    }

    bool is_rock_ridge() const
    {
        return impl_.is_rock_ridge();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_iso_image_file_source<iostreams::file_source> impl_;
};


template<class Sink>
class basic_iso_image_file_sink
{
private:
    typedef detail::basic_iso_image_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef iso9660::header header_type;

    explicit basic_iso_image_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void create_entry(const iso9660::header& head)
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

class iso_image_file_sink
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    typedef iso9660::header header_type;

    explicit iso_image_file_sink(const std::string& filename)
        : impl_(iostreams::file_sink(filename, BOOST_IOS::binary))
    {
    }

    void create_entry(const iso9660::header& head)
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
    basic_iso_image_file_sink<iostreams::file_sink> impl_;
};

} } // End namespaces archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO_IMAGE_FILE_HPP
