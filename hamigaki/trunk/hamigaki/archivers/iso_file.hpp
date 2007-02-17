//  iso_file.hpp: ISO file

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_FILE_HPP
#define HAMIGAKI_ARCHIVERS_ISO_FILE_HPP

#include <hamigaki/archivers/detail/iso_file_source_impl.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>

namespace hamigaki { namespace archivers {

template<class Source>
class basic_iso_file_source
{
private:
    typedef detail::basic_iso_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    typedef iso::header header_type;

    explicit basic_iso_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    iso::header header() const
    {
        return pimpl_->header();
    }

    const std::vector<iso::volume_descriptor>& volume_descriptors() const
    {
        return pimpl_->volume_descriptors();
    }

    void select_volume_descriptor(std::size_t index)
    {
        pimpl_->select_volume_descriptor(index);
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

    typedef iso::header header_type;

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

    const std::vector<iso::volume_descriptor>& volume_descriptors() const
    {
        return impl_.volume_descriptors();
    }

    void select_volume_descriptor(std::size_t index)
    {
        impl_.select_volume_descriptor(index);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_iso_file_source<iostreams::file_source> impl_;
};

} } // End namespaces archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO_FILE_HPP
