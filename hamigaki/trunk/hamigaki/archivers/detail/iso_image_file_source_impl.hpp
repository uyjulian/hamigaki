//  iso_image_file_source_impl.hpp: ISO image file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/iso9660_file_source_impl.hpp>
#include <boost/logic/tribool.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class basic_iso_image_file_source_impl : private boost::noncopyable
{
public:
    explicit basic_iso_image_file_source_impl(const Source& src)
        : impl_(src), is_rock_ridge_(boost::logic::indeterminate)
    {
    }

    bool next_entry()
    {
        if (indeterminate(is_rock_ridge_))
            select_volume_descriptor(0);

        while (impl_.next_entry())
        {
            const iso9660::header& head = impl_.header();

            if (head.is_associated())
                continue;

            if (impl_.is_latest())
            {
                if (is_rock_ridge_)
                    this->parse_rock_ridge(head);
                else
                    this->fix_header(head);

                return true;
            }
        }
        return false;
    }

    iso9660::header header() const
    {
        return header_;
    }

    const std::vector<iso9660::volume_descriptor>& volume_descriptors() const
    {
        return impl_.volume_descriptors();
    }

    void select_volume_descriptor(std::size_t index)
    {
        impl_.select_volume_descriptor(index);
        rock_ridge_check();
    }

    bool is_rock_ridge() const
    {
        return is_rock_ridge_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_iso9660_file_source_impl<Source> impl_;
    iso9660::header header_;
    boost::logic::tribool is_rock_ridge_;

    void rock_ridge_check()
    {
        is_rock_ridge_ = false;

        const iso9660::header& head = impl_.directory_header();
        const std::string& su = head.system_use;
        if (su.empty())
            return;

        const std::size_t head_size =
            hamigaki::struct_size<iso9660::system_use_entry_header>::value;

        const std::size_t er_size =
            head_size +
            hamigaki::struct_size<iso9660::er_system_use_entry_data>::value;

        std::size_t pos = 0;
        while (pos + head_size < su.size())
        {
            iso9660::system_use_entry_header head;
            hamigaki::binary_read(su.c_str()+pos, head);
            if (std::memcmp(head.signature, "ER", 2) == 0)
            {
                if ((head.entry_size >= er_size) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    iso9660::er_system_use_entry_data er;
                    hamigaki::binary_read(su.c_str()+pos+head_size, er);

                    if (er.id_size == 10)
                    {
                        const char* s = su.c_str()+pos+er_size;
                        if ((std::memcmp(s, "RRIP_1991A", 10) == 0) ||
                            (std::memcmp(s, "IEEE_P1282", 10) == 0) )
                        {
                            is_rock_ridge_ = true;
                            return;
                        }
                    }
                }
            }
            pos += head.entry_size;
        }
    }

    void fix_header(const iso9660::header& head)
    {
        using namespace boost::filesystem;

        header_ = head;

        std::string leaf = head.path.leaf();
        std::string filename(leaf, 0, leaf.find(';'));
        if (!filename.empty() && (filename[filename.size()-1] == '.'))
            filename.resize(filename.size()-1);

        header_.path = head.path.branch_path()/path(filename,no_check);
    }

    void parse_rock_ridge(const iso9660::header& head)
    {
        using namespace boost::filesystem;

        header_ = head;

        const std::string& su = head.system_use;

        const std::size_t head_size =
            hamigaki::struct_size<iso9660::system_use_entry_header>::value;

        std::size_t pos = 0;
        std::string filename;
        bool filename_ends = false;
        while (pos + head_size < su.size())
        {
            iso9660::system_use_entry_header head;
            hamigaki::binary_read(su.c_str()+pos, head);

            if (!filename_ends && (std::memcmp(head.signature, "NM", 2) == 0))
            {
                if ((head.entry_size >= head_size+1) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    boost::uint8_t flags =
                        static_cast<boost::uint8_t>(su[pos+head_size]);

                    filename_ends = (flags & 0x01) == 0;

                    filename.append(
                        su, pos+head_size+1, head.entry_size-(head_size+1));
                }
            }
            pos += head.entry_size;
        }

        if (filename_ends)
            header_.path = head.path.branch_path()/path(filename,no_check);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP
