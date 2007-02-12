//  iso_image_file_source_impl.hpp: ISO image file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/iso9660_file_source_impl.hpp>
#include <hamigaki/archivers/iso/tf_flags.hpp>

namespace hamigaki { namespace archivers { namespace detail {

class sl_components_parser
{
private:
    static const boost::uint8_t continue_       = 0x01;
    static const boost::uint8_t current         = 0x02;
    static const boost::uint8_t parent          = 0x04;
    static const boost::uint8_t root            = 0x08;

public:
    sl_components_parser() : ends_(false), bad_(false)
    {
    }

    void parse(const char* s, std::size_t size)
    {
        if (ends_ || bad_)
            return;

        if (size < 1)
        {
            bad_ = true;
            return;
        }

        boost::uint8_t flags = static_cast<boost::uint8_t>(*s);

        std::size_t pos = 1;
        while (pos < size)
            pos += parse_components(s+pos, size-pos);

        ends_ = (flags & continue_) == 0;
    }

    bool valid() const
    {
        return !bad_ && !ph_.empty() && leaf_.empty();
    }

    boost::filesystem::path path() const
    {
        return ph_;
    }

private:
    bool ends_;
    bool bad_;
    std::string leaf_;
    boost::filesystem::path ph_;

    std::size_t parse_components(const char* s, std::size_t size)
    {
        using boost::filesystem::no_check;

        if (size < 2)
        {
            bad_ = true;
            return size;
        }

        boost::uint8_t flags = static_cast<boost::uint8_t>(s[0]);
        std::size_t amt = static_cast<boost::uint8_t>(s[1]);

        if (2u + amt > size)
        {
            bad_ = true;
            return size;
        }

        if ((flags & current) != 0)
        {
            const boost::uint8_t mask = continue_ | parent | root;

            if (((flags & mask) != 0) || !leaf_.empty())
            {
                bad_ = true;
                return size;
            }
            ph_ /= ".";
        }
        else if ((flags & parent) != 0)
        {
            const boost::uint8_t mask = continue_ | current | root;

            if (((flags & mask) != 0) || !leaf_.empty())
            {
                bad_ = true;
                return size;
            }
            ph_ /= "..";
        }
        else if ((flags & root) != 0)
        {
            const boost::uint8_t mask = continue_ | current | parent;

            if (((flags & mask) != 0) || !leaf_.empty() || !ph_.empty())
            {
                bad_ = true;
                return size;
            }
            ph_ = "/";
        }
        else
        {
            leaf_.append(s+2, amt);

            if ((flags & continue_) == 0)
            {
                ph_ /= boost::filesystem::path(leaf_, no_check);
                leaf_.clear();
            }
        }

        return 2+amt;
    }
};

template<class Source>
class basic_iso_image_file_source_impl : private boost::noncopyable
{
private:
    enum rrip_type
    {
        rrip_ind, rrip_none, rrip_1991a, ieee_p1282
    };

public:
    explicit basic_iso_image_file_source_impl(const Source& src)
        : impl_(src), rrip_(rrip_ind)
    {
    }

    bool next_entry()
    {
        if (rrip_ == rrip_ind)
            select_volume_descriptor(0);

        while (impl_.next_entry())
        {
            const iso::header& head = impl_.header();

            if (head.is_associated())
                continue;

            if (impl_.is_latest())
            {
                if (rrip_ != rrip_none)
                    this->parse_rock_ridge(head);
                else
                    this->fix_header(head);

                return true;
            }
        }
        return false;
    }

    iso::header header() const
    {
        return header_;
    }

    const std::vector<iso::volume_descriptor>& volume_descriptors() const
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
        BOOST_ASSERT(rrip_ != rrip_ind);

        return rrip_ != rrip_none;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

private:
    basic_iso9660_file_source_impl<Source> impl_;
    iso::header header_;
    rrip_type rrip_;

    void rock_ridge_check()
    {
        rrip_ = rrip_none;

        const iso::header& head = impl_.directory_header();
        const std::string& su = head.system_use;
        if (su.empty())
            return;

        const std::size_t head_size =
            hamigaki::struct_size<iso::system_use_entry_header>::value;

        const std::size_t er_size =
            head_size +
            hamigaki::struct_size<iso::er_system_use_entry_data>::value;

        std::size_t pos = 0;
        while (pos + head_size < su.size())
        {
            iso::system_use_entry_header head;
            hamigaki::binary_read(su.c_str()+pos, head);
            if (std::memcmp(head.signature, "ER", 2) == 0)
            {
                if ((head.entry_size >= er_size) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    iso::er_system_use_entry_data er;
                    hamigaki::binary_read(su.c_str()+pos+head_size, er);

                    if (er.id_size == 10)
                    {
                        const char* s = su.c_str()+pos+er_size;
                        if (std::memcmp(s, "RRIP_1991A", 10) == 0)
                        {
                            rrip_ = rrip_1991a;
                            return;
                        }
                        else if (std::memcmp(s, "IEEE_P1282", 10) == 0)
                        {
                            rrip_ = ieee_p1282;
                            return;
                        }
                    }
                }
            }
            pos += head.entry_size;
        }
    }

    void fix_header(const iso::header& head)
    {
        using namespace boost::filesystem;

        header_ = head;

        std::string leaf = head.path.leaf();
        std::string filename(leaf, 0, leaf.find(';'));
        if (!filename.empty() && (filename[filename.size()-1] == '.'))
            filename.resize(filename.size()-1);

        header_.path = head.path.branch_path()/path(filename,no_check);
    }

    void parse_tf_system_use_entry(const char* s, std::size_t size)
    {
        using iso::tf_flags;

        boost::uint8_t flags =
            static_cast<boost::uint8_t>(*(s++));

        std::size_t count = 0;
        if ((flags & tf_flags::creation) != 0)
            ++count;
        if ((flags & tf_flags::modify) != 0)
            ++count;
        if ((flags & tf_flags::access) != 0)
            ++count;
        if ((flags & tf_flags::attributes) != 0)
            ++count;
        if ((flags & tf_flags::backup) != 0)
            ++count;
        if ((flags & tf_flags::expiration) != 0)
            ++count;
        if ((flags & tf_flags::effective) != 0)
            ++count;

        if ((flags & tf_flags::long_form) != 0)
        {
            typedef iso::date_time time_type;

            const std::size_t dt_size = hamigaki::struct_size<time_type>::value;

            if (1+count*dt_size > size)
                return;

            if ((flags & tf_flags::creation) != 0)
            {
                header_.creation_time = hamigaki::binary_read<time_type>(s);
                s += dt_size;
            }

            if ((flags & tf_flags::modify) != 0)
            {
                header_.last_write_time = hamigaki::binary_read<time_type>(s);
                s += dt_size;
            }

            if ((flags & tf_flags::access) != 0)
            {
                header_.last_access_time = hamigaki::binary_read<time_type>(s);
                s += dt_size;
            }

            if ((flags & tf_flags::attributes) != 0)
            {
                header_.last_change_time = hamigaki::binary_read<time_type>(s);
                s += dt_size;
            }

            if ((flags & tf_flags::backup) != 0)
            {
                header_.last_backup_time = hamigaki::binary_read<time_type>(s);
                s += dt_size;
            }

            if ((flags & tf_flags::expiration) != 0)
            {
                header_.expiration_time = hamigaki::binary_read<time_type>(s);
                s += dt_size;
            }

            if ((flags & tf_flags::effective) != 0)
            {
                header_.effective_time = hamigaki::binary_read<time_type>(s);
                s += dt_size;
            }
        }
        else
        {
            typedef iso::binary_date_time time_type;
            const std::size_t dt_size = hamigaki::struct_size<time_type>::value;

            if (1+count*dt_size > size)
                return;

            if ((flags & tf_flags::creation) != 0)
            {
                header_.creation_time =
                    hamigaki::binary_read<time_type>(s).to_date_time();
                s += dt_size;
            }

            if ((flags & tf_flags::modify) != 0)
            {
                header_.last_write_time =
                    hamigaki::binary_read<time_type>(s).to_date_time();
                s += dt_size;
            }

            if ((flags & tf_flags::access) != 0)
            {
                header_.last_access_time =
                    hamigaki::binary_read<time_type>(s).to_date_time();
                s += dt_size;
            }

            if ((flags & tf_flags::attributes) != 0)
            {
                header_.last_change_time =
                    hamigaki::binary_read<time_type>(s).to_date_time();
                s += dt_size;
            }

            if ((flags & tf_flags::backup) != 0)
            {
                header_.last_backup_time =
                    hamigaki::binary_read<time_type>(s).to_date_time();
                s += dt_size;
            }

            if ((flags & tf_flags::expiration) != 0)
            {
                header_.expiration_time =
                    hamigaki::binary_read<time_type>(s).to_date_time();
                s += dt_size;
            }

            if ((flags & tf_flags::effective) != 0)
            {
                header_.effective_time =
                    hamigaki::binary_read<time_type>(s).to_date_time();
                s += dt_size;
            }
        }
    }

    void parse_rock_ridge(const iso::header& head)
    {
        using namespace boost::filesystem;

        header_ = head;

        const std::string& su = head.system_use;

        const std::size_t head_size =
            hamigaki::struct_size<iso::system_use_entry_header>::value;

        std::size_t pos = 0;
        std::string filename;
        bool filename_ends = false;
        sl_components_parser sl_parser;
        while (pos + head_size < su.size())
        {
            const char* s = su.c_str()+pos;

            iso::system_use_entry_header head;
            hamigaki::binary_read(s, head);
            s += head_size;

            if (std::memcmp(head.signature, "PX", 2) == 0)
            {
                if (rrip_ == rrip_1991a)
                {
                    typedef iso::old_px_system_use_entry_data data_type;
                    const std::size_t data_size =
                        hamigaki::struct_size<data_type>::value;

                    if ((head.entry_size >= head_size+data_size) &&
                        (pos + head.entry_size <= su.size()) )
                    {
                        data_type data;
                        hamigaki::binary_read(s, data);

                        iso::posix::file_attributes attr;
                        attr.permissions = data.file_mode;
                        attr.links = data.links;
                        attr.uid = data.uid;
                        attr.gid = data.gid;
                        attr.serial_no = 0;
                        header_.attributes = attr;
                    }
                }
                else
                {
                    typedef iso::px_system_use_entry_data data_type;
                    const std::size_t data_size =
                        hamigaki::struct_size<data_type>::value;

                    if ((head.entry_size >= head_size+data_size) &&
                        (pos + head.entry_size <= su.size()) )
                    {
                        data_type data;
                        hamigaki::binary_read(s, data);

                        iso::posix::file_attributes attr;
                        attr.permissions = data.file_mode;
                        attr.links = data.links;
                        attr.uid = data.uid;
                        attr.gid = data.gid;
                        attr.serial_no = data.serial_no;
                        header_.attributes = attr;
                    }
                }
            }
            if (std::memcmp(head.signature, "PN", 2) == 0)
            {
                typedef iso::pn_system_use_entry_data data_type;
                const std::size_t data_size =
                    hamigaki::struct_size<data_type>::value;

                if ((head.entry_size >= head_size+data_size) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    data_type data;
                    hamigaki::binary_read(s, data);

                    header_.device_number =
                        filesystem::device_number(
                            data.device_number_high,
                            data.device_number_low
                        );
                }
            }
            else if (std::memcmp(head.signature, "SL", 2) == 0)
                sl_parser.parse(s, head.entry_size - head_size);
            else if (std::memcmp(head.signature, "NM", 2) == 0)
            {
                if (!filename_ends &&
                    (head.entry_size >= head_size+1) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    boost::uint8_t flags =
                        static_cast<boost::uint8_t>(*(s++));

                    filename_ends = (flags & 0x01) == 0;

                    filename.append(s, head.entry_size-(head_size+1));
                }
            }
            else if (std::memcmp(head.signature, "TF", 2) == 0)
            {
                if ((head.entry_size >= head_size+1) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    parse_tf_system_use_entry(s, head.entry_size - head_size);
                }
            }
            pos += head.entry_size;
        }

        if (filename_ends)
            header_.path = head.path.branch_path()/path(filename,no_check);

        if (sl_parser.valid())
            header_.link_path = sl_parser.path();
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_IMAGE_FILE_SOURCE_IMPL_HPP
