//  iso_image_file_sink_impl.hpp: ISO image file sink implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/iso9660_file_sink_impl.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink>
class basic_iso_image_file_sink_impl : private boost::noncopyable
{
public:
    explicit basic_iso_image_file_sink_impl(const Sink& sink)
        : impl_(sink)
    {
    }

    void create_entry(const iso::header& head)
    {
        using namespace boost::filesystem;

        iso::header h = head;
        if (h.is_regular())
            h.path = h.path.branch_path() / (h.path.leaf() + ";1");

        impl_.create_entry(h);
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
    basic_iso9660_file_sink_impl<Sink> impl_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SINK_IMPL_HPP
