//  rock_ridge_reader.hpp: IEEE P1281 Rock Ridge reader

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_READER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_READER_HPP

#include <boost/config.hpp>
#include <hamigaki/archivers/detail/iso_data_reader.hpp>
#include <hamigaki/archivers/detail/iso_logical_block_number.hpp>
#include <hamigaki/archivers/detail/sl_components_parser.hpp>
#include <hamigaki/archivers/iso/headers.hpp>
#include <hamigaki/archivers/iso/tf_flags.hpp>
#include <stack>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class rock_ridge_reader : private boost::noncopyable
{
private:
    typedef typename iso_data_reader<Source>::directory_record directory_record;

    static const std::size_t logical_sector_size = 2048;

    enum rrip_type
    {
        rrip_ind, rrip_none, rrip_1991a, ieee_p1282
    };

    struct internal_data
    {
        boost::uint32_t child_pos;
        boost::uint32_t parent_pos;
        bool is_relocated;

        internal_data() : child_pos(0), parent_pos(0), is_relocated(false)
        {
        }
    };

public:
    typedef Source source_type;

    rock_ridge_reader(const Source& src, const iso::volume_descriptor& desc)
        : data_reader_(src, calc_lbn_shift(desc.logical_block_size))
    {
        data_reader_.select_directory(desc.root_record.data_pos);
        rock_ridge_check();
    }

    bool next_entry()
    {
        while (internal_next_entry())
        {
            if (!internal_.is_relocated)
                return true;
        }
        return false;
    }

    iso::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return data_reader_.read(s, n);
    }

private:
    iso_data_reader<Source> data_reader_;
    iso::header header_;
    internal_data internal_;
    boost::filesystem::path dir_path_;
    std::stack<boost::uint32_t> stack_;
    rrip_type rrip_;

    void rock_ridge_check()
    {
        rrip_ = rrip_none;

        const std::string& su = data_reader_.entries().at(0).system_use;
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

    void parse_tf_system_use_entry(
        iso::header& header, const char* s, std::size_t size)
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

    void parse_rock_ridge(iso::header& header, internal_data& internal)
    {
        using namespace boost::filesystem;

        const std::string& su = header.system_use;

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
            else if (std::memcmp(head.signature, "CL", 2) == 0)
            {
                if ((head.entry_size >= head_size+8) &&
                    (pos + head.entry_size <= su.size()) )
                {
#if defined(BOOST_LITTLE_ENDIAN)
                    internal.child_pos = hamigaki::decode_uint<little,4>(s);
#else
                    internal.child_pos = hamigaki::decode_uint<big,4>(s+4);
#endif
                }
            }
            else if (std::memcmp(head.signature, "PL", 2) == 0)
            {
                if ((head.entry_size >= head_size+8) &&
                    (pos + head.entry_size <= su.size()) )
                {
#if defined(BOOST_LITTLE_ENDIAN)
                    internal.parent_pos = hamigaki::decode_uint<little,4>(s);
#else
                    internal.parent_pos = hamigaki::decode_uint<big,4>(s+4);
#endif
                }
            }
            else if (std::memcmp(head.signature, "RE", 2) == 0)
                internal.is_relocated = true;
            else if (std::memcmp(head.signature, "TF", 2) == 0)
            {
                if ((head.entry_size >= head_size+1) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    parse_tf_system_use_entry(
                        header, s, head.entry_size - head_size);
                }
            }
            pos += head.entry_size;
        }

        if (filename_ends)
            header.path = header.path.branch_path()/path(filename,no_check);

        if (sl_parser.valid())
            header.link_path = sl_parser.path();
    }

    bool internal_next_entry()
    {
        using namespace boost::filesystem;

        if (header_.is_directory() && !internal_.is_relocated)
        {
            dir_path_ = header_.path;
            stack_.push(data_reader_.entry_index());
            if (internal_.child_pos)
                data_reader_.select_directory(internal_.child_pos);
            else
                data_reader_.select_directory(data_reader_.record().data_pos);
        }

        std::size_t next_index = data_reader_.entry_index();
        if (next_index)
            ++next_index;
        else
            next_index = 2;

        while (next_index == data_reader_.entries().size())
        {
            if (stack_.empty())
                return false;

            const directory_record& parent = data_reader_.entries().at(1);

            if (rrip_ != rrip_none)
            {

                iso::header h;
                internal_data internal;
                h.system_use = parent.system_use;
                parse_rock_ridge(h, internal);

                if (internal.parent_pos != 0)
                    data_reader_.select_directory(internal.parent_pos);
                else
                    data_reader_.select_directory(parent.data_pos);
            }
            else
                data_reader_.select_directory(parent.data_pos);

            dir_path_ = dir_path_.branch_path();
            next_index = stack_.top() + 1;
            stack_.pop();
        }

        data_reader_.select_entry(next_index);

        const directory_record& rec = data_reader_.record();

        iso::header h;
        if (rec.file_id.size() == 1)
        {
            if (rec.file_id[0] == '\0')
                h.path = dir_path_;
            else
                h.path = dir_path_ / "..";
        }
        else
            h.path = dir_path_ / path(rec.file_id, no_check);
        h.file_size = rec.data_size;
        h.recorded_time = rec.recorded_time;
        h.flags = rec.flags;
        h.system_use = rec.system_use;

        internal_data inter;
        if (rrip_ != rrip_none)
        {
            parse_rock_ridge(h, inter);
            if (inter.child_pos != 0)
                h.flags |= iso::file_flags::directory;
        }

        header_ = h;
        internal_ = inter;
        return true;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_READER_HPP
