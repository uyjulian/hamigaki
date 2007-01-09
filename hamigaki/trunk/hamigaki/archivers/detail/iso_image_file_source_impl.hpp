//  iso_image_file_source_impl.hpp: ISO image file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/iso9660_file_source_impl.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class basic_iso_image_file_source_impl : private boost::noncopyable
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    explicit basic_iso_image_file_source_impl(const Source& src)
        : impl_(src)
    {
    }

    bool next_entry()
    {
        while (impl_.next_entry())
        {
            const iso9660::header& head = impl_.header();

            if (head.is_associated())
                continue;

            if (impl_.is_latest())
            {
                using namespace boost::filesystem;

                header_ = head;

                std::string leaf = head.path.leaf();
                std::string filename(leaf, 0, leaf.find(';'));
                if (!filename.empty() && (filename[filename.size()-1] == '.'))
                    filename.resize(filename.size()-1);

                header_.path = head.path.branch_path()/path(filename,no_check);

                return true;
            }
        }
        return false;
    }

    iso9660::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_iso9660_file_source_impl<Source> impl_;
    iso9660::header header_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP
