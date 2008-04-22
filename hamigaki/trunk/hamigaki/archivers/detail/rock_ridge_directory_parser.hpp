// rock_ridge_directory_parser.hpp: IEEE P1282 Rock Ridge directory parser

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_PARSER_HPP

#include <hamigaki/archivers/detail/iso_directory_parser.hpp>
#include <hamigaki/archivers/detail/sl_components_parser.hpp>
#include <hamigaki/archivers/iso/rrip_type.hpp>
#include <hamigaki/archivers/iso/tf_flags.hpp>
#include <hamigaki/binary/binary_io.hpp>
#include <memory>

namespace hamigaki { namespace archivers { namespace detail {

inline bool rock_ridge_fix_record(iso_directory_record& rec)
{
    using namespace boost::filesystem;

    const std::string& su = rec.system_use;

    const std::size_t head_size =
        hamigaki::struct_size<iso::system_use_entry_header>::value;

    std::size_t pos = 0;
    std::string filename;
    bool filename_ends = false;
    while (pos + head_size <= su.size())
    {
        const char* s = su.c_str()+pos;

        iso::system_use_entry_header head;
        hamigaki::binary_read(s, head);
        s += head_size;

        if (std::memcmp(head.signature, "NM", 2) == 0)
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
        else if (std::memcmp(head.signature, "CL", 2) == 0)
        {
            if ((head.entry_size >= head_size+8) &&
                (pos + head.entry_size <= su.size()) )
            {
#if defined(BOOST_LITTLE_ENDIAN)
                rec.data_pos = hamigaki::decode_uint<little,4>(s);
#else
                rec.data_pos = hamigaki::decode_uint<big,4>(s+4);
#endif
                rec.flags |= iso::file_flags::directory;
            }
        }
        else if (std::memcmp(head.signature, "PL", 2) == 0)
        {
            if ((head.entry_size >= head_size+8) &&
                (pos + head.entry_size <= su.size()) )
            {
#if defined(BOOST_LITTLE_ENDIAN)
                rec.data_pos = hamigaki::decode_uint<little,4>(s);
#else
                rec.data_pos = hamigaki::decode_uint<big,4>(s+4);
#endif
            }
        }
        else if (std::memcmp(head.signature, "RE", 2) == 0)
            return false;

        pos += head.entry_size;
    }

    if (filename_ends)
        rec.file_id = filename;

    return true;
}

template<class Path>
inline void parse_tf_system_use_entry(
    iso::basic_header<Path>& header, const char* s, std::size_t size)
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
            header.creation_time = hamigaki::binary_read<time_type>(s);
            s += dt_size;
        }

        if ((flags & tf_flags::modify) != 0)
        {
            header.last_write_time = hamigaki::binary_read<time_type>(s);
            s += dt_size;
        }

        if ((flags & tf_flags::access) != 0)
        {
            header.last_access_time = hamigaki::binary_read<time_type>(s);
            s += dt_size;
        }

        if ((flags & tf_flags::attributes) != 0)
        {
            header.last_change_time = hamigaki::binary_read<time_type>(s);
            s += dt_size;
        }

        if ((flags & tf_flags::backup) != 0)
        {
            header.last_backup_time = hamigaki::binary_read<time_type>(s);
            s += dt_size;
        }

        if ((flags & tf_flags::expiration) != 0)
        {
            header.expiration_time = hamigaki::binary_read<time_type>(s);
            s += dt_size;
        }

        if ((flags & tf_flags::effective) != 0)
        {
            header.effective_time = hamigaki::binary_read<time_type>(s);
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
            header.creation_time =
                hamigaki::binary_read<time_type>(s).to_date_time();
            s += dt_size;
        }

        if ((flags & tf_flags::modify) != 0)
        {
            header.last_write_time =
                hamigaki::binary_read<time_type>(s).to_date_time();
            s += dt_size;
        }

        if ((flags & tf_flags::access) != 0)
        {
            header.last_access_time =
                hamigaki::binary_read<time_type>(s).to_date_time();
            s += dt_size;
        }

        if ((flags & tf_flags::attributes) != 0)
        {
            header.last_change_time =
                hamigaki::binary_read<time_type>(s).to_date_time();
            s += dt_size;
        }

        if ((flags & tf_flags::backup) != 0)
        {
            header.last_backup_time =
                hamigaki::binary_read<time_type>(s).to_date_time();
            s += dt_size;
        }

        if ((flags & tf_flags::expiration) != 0)
        {
            header.expiration_time =
                hamigaki::binary_read<time_type>(s).to_date_time();
            s += dt_size;
        }

        if ((flags & tf_flags::effective) != 0)
        {
            header.effective_time =
                hamigaki::binary_read<time_type>(s).to_date_time();
            s += dt_size;
        }
    }
}

template<class Path>
inline void
parse_rock_ridge(iso::basic_header<Path>& header, iso::rrip_type rrip)
{
    typedef typename Path::string_type string_type;

    const std::string& su = header.system_use;

    const std::size_t head_size =
        hamigaki::struct_size<iso::system_use_entry_header>::value;

    std::size_t pos = 0;
    sl_components_parser sl_parser;
    while (pos + head_size <= su.size())
    {
        const char* s = su.c_str()+pos;

        iso::system_use_entry_header head;
        hamigaki::binary_read(s, head);
        s += head_size;

        if (std::memcmp(head.signature, "PX", 2) == 0)
        {
            if (rrip == iso::rrip_1991a)
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
                    header.attributes = attr;
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
                    header.attributes = attr;
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

                header.device_number =
                    filesystem::device_number(
                        data.device_number_high,
                        data.device_number_low
                    );
            }
        }
        else if (std::memcmp(head.signature, "SL", 2) == 0)
            sl_parser.parse(s, head.entry_size - head_size);
        else if (std::memcmp(head.signature, "TF", 2) == 0)
        {
            if ((head.entry_size >= head_size+1) &&
                (pos + head.entry_size <= su.size()) )
            {
                detail::parse_tf_system_use_entry(
                    header, s, head.entry_size - head_size);
            }
        }
        pos += head.entry_size;
    }

    if (sl_parser.valid())
    {
        header.link_path =
            detail::from_iso9660_string<string_type>(sl_parser.path().string());
    }
};

template<class Path>
class rock_ridge_directory_parser : public iso_directory_parser<Path>
{
public:
    typedef Path path_type;
    typedef iso::basic_header<Path> header_type;

    rock_ridge_directory_parser(
        std::auto_ptr<iso_directory_parser<Path> >& pimpl,
        iso::rrip_type rrip)
        : pimpl_(pimpl), rrip_(rrip)
    {
    }

private:
    std::auto_ptr<iso_directory_parser<Path> > pimpl_;
    iso::rrip_type rrip_;

    void do_fix_records(std::vector<iso_directory_record>& records) // virtual
    {
        pimpl_->fix_records(records);

        std::vector<iso_directory_record> tmp;
        for (std::size_t i = 0; i < records.size(); ++i)
        {
            iso_directory_record rec = records[i];
            if (detail::rock_ridge_fix_record(rec))
                tmp.push_back(rec);
        }
        records.swap(tmp);
    }

    header_type do_make_header(const iso_directory_record& rec) // virtual
    {
        header_type head = pimpl_->make_header(rec);
        detail::parse_rock_ridge(head, rrip_);
        return head;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_PARSER_HPP
