//  rock_ridge_file_source_impl.hpp: IEEE P1281 file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/iso9660_id.hpp>
#include <hamigaki/archivers/detail/sl_components_parser.hpp>
#include <hamigaki/archivers/iso/headers.hpp>
#include <hamigaki/archivers/iso/tf_flags.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>
#include <stack>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class basic_rock_ridge_file_source_impl : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;

    enum rrip_type
    {
        rrip_ind, rrip_none, rrip_1991a, ieee_p1282
    };

    struct directory_record
    {
        iso9660_id file_id;
        boost::uint32_t data_pos;
        boost::uint32_t data_size;
        iso::binary_date_time recorded_time;
        boost::uint8_t flags;
        std::string system_use;

        directory_record()
            : data_pos(0), data_size(0), flags(0)
        {
        }

        explicit directory_record(const iso9660_id& id)
            : file_id(id), data_pos(0), data_size(0), flags(0)
        {
        }

        bool operator<(const directory_record& rhs) const
        {
            if (int cmp = file_id.compare(rhs.file_id))
                return cmp < 0;

            bool lhs_assoc = (flags & iso::file_flags::associated) != 0;
            bool rhs_assoc = (rhs.flags & iso::file_flags::associated) != 0;

            if (lhs_assoc && !rhs_assoc)
                return true;
            else if (!lhs_assoc && rhs_assoc)
                return false;

            if (data_pos < rhs.data_pos)
                return true;
            else
                return false;
        }

        bool is_directory() const
        {
            return (flags & iso::file_flags::directory) != 0;
        }
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
    explicit basic_rock_ridge_file_source_impl(const Source& src)
        : src_(src), pos_(0)
    {
        header_.file_size = 0;

        read_volume_descriptor();

        if (volume_desc_.logical_block_size == 2048)
            lbn_shift_ = 11;
        else if (volume_desc_.logical_block_size == 1024)
            lbn_shift_ = 10;
        else if (volume_desc_.logical_block_size == 512)
            lbn_shift_ = 9;
        else
        {
            throw BOOST_IOSTREAMS_FAILURE(
                "invalid ISO 9660 logical block size");
        }
        lbn_mask_ = volume_desc_.logical_block_size - 1;

        select_directory(volume_desc_.root_record.data_pos);
        dir_pos_ = 1;

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
        if (!header_.is_regular())
            return -1;

        if (pos_ == 0)
            seek_logical_block(dir_records_[dir_pos_].data_pos);

        std::streamsize total = 0;
        while (total < n)
        {
            boost::uint64_t rest = header_.file_size - pos_;
            if (rest == 0)
                break;

            std::size_t offset = pos_ & lbn_mask_;
            if (offset == 0)
                fill_buffer();

            rest = hamigaki::auto_min(
                rest, volume_desc_.logical_block_size-offset);
            std::streamsize amt = hamigaki::auto_min(n-total, rest);

            std::memcpy(s+total, &block_[offset], amt);
            total += amt;
            pos_ += amt;
        }

        return total != 0 ? total : -1;
    }

    bool is_latest() const
    {
        const iso9660_id& cur = dir_records_[dir_pos_].file_id;

        for (std::size_t i = dir_pos_ + 1; i < dir_records_.size(); ++i)
        {
            const iso9660_id& next = dir_records_[i].file_id;
            if (cur.filename_compare(next) != 0)
                return true;

            if (cur.version_compare(next) != 0)
                return false;
        }

        return true;
    }

private:
    Source src_;
    iso::volume_descriptor volume_desc_;
    iso::volume_descriptor desc_;
    std::vector<directory_record> dir_records_;
    iso::header header_;
    internal_data internal_;
    boost::uint32_t lbn_shift_;
    boost::uint32_t lbn_mask_;
    boost::uint64_t pos_;
    boost::filesystem::path dir_path_;
    std::stack<boost::uint32_t> stack_;
    std::size_t dir_pos_;
    rrip_type rrip_;
    char block_[logical_sector_size];

    void seek_logical_block(boost::uint32_t num)
    {
        boost::uint64_t off = static_cast<boost::uint64_t>(num) << lbn_shift_;

        boost::iostreams::seek(
            src_,
            static_cast<boost::iostreams::stream_offset>(off),
            BOOST_IOS::beg);
    }

    void fill_buffer()
    {
        iostreams::blocking_read(src_, block_, volume_desc_.logical_block_size);
    }

    void read_volume_descriptor()
    {
        boost::iostreams::seek(src_, logical_sector_size*16, BOOST_IOS::beg);

        char block[logical_sector_size];
        while (true)
        {
            iostreams::blocking_read(src_, block, sizeof(block));
            if (block[0] == '\xFF')
                break;

            if (block[0] == '\x01')
            {
                hamigaki::binary_read(block, volume_desc_);
                return;
            }
        }

        throw BOOST_IOSTREAMS_FAILURE(
            "ISO 9660 volume descriptor not found");
    }

    void read_continuation_area(directory_record& rec)
    {
        std::string& su = rec.system_use;
        if (su.empty())
            return;

        const std::size_t head_size =
            hamigaki::struct_size<iso::system_use_entry_header>::value;

        const std::size_t ce_size =
            head_size +
            hamigaki::struct_size<iso::ce_system_use_entry_data>::value;

        std::size_t pos = 0;
        iso::ce_system_use_entry_data ce;
        ce.next_size = 0;
        while (pos + head_size < su.size())
        {
            iso::system_use_entry_header head;
            hamigaki::binary_read(su.c_str()+pos, head);
            if (std::memcmp(head.signature, "CE", 2) == 0)
            {
                if ((head.entry_size == ce_size) &&
                    (pos + head.entry_size <= su.size()) )
                {
                    hamigaki::binary_read(su.c_str()+pos+head_size, ce);
                }
            }
            pos += head.entry_size;
        }

        // TODO: support multiple "CE" System Use Entries
        if (ce.next_size != 0)
        {
            boost::uint64_t off =
                static_cast<boost::uint64_t>(ce.next_pos) << lbn_shift_;
            off += ce.next_offset;

            boost::iostreams::seek(
                src_,
                static_cast<boost::iostreams::stream_offset>(off),
                BOOST_IOS::beg);

            boost::scoped_array<char> buffer(new char[ce.next_size]);
            iostreams::blocking_read(src_, buffer.get(), ce.next_size);
            su.append(buffer.get(), ce.next_size);
        }
    }

    void select_directory(boost::uint32_t data_pos)
    {
        std::vector<directory_record> records;

        const std::size_t bin_size =
            struct_size<iso::directory_record>::value;

        seek_logical_block(data_pos);
        fill_buffer();

        iso::directory_record raw;
        hamigaki::binary_read(block_, raw);
        if (raw.record_size < bin_size + 1)
            throw BOOST_IOSTREAMS_FAILURE("invalid ISO 9660 directory records");

        directory_record self(iso9660_id('\x00'));
        self.data_pos = raw.data_pos;
        self.data_size = raw.data_size;
        self.recorded_time = raw.recorded_time;
        self.flags = raw.flags;
        if (std::size_t su_len = raw.record_size - (bin_size + 1))
            self.system_use.assign(&block_[bin_size + 1], su_len);
        records.push_back(self);

        boost::uint32_t pos = raw.record_size;
        while (pos < self.data_size)
        {
            boost::uint32_t offset = pos & lbn_mask_;
            if (offset == 0)
                fill_buffer();

            if (block_[offset] != 0)
            {
                boost::uint32_t next = offset + bin_size;
                if (next > volume_desc_.logical_block_size)
                    break;

                hamigaki::binary_read(&block_[offset], raw);

                boost::uint32_t id_size = raw.file_id_size;
                if ((id_size & 1) == 0)
                    ++id_size;

                if (bin_size + id_size > raw.record_size)
                    break;

                next += id_size;
                if (next > volume_desc_.logical_block_size)
                    break;

                std::string id(&block_[offset+bin_size], raw.file_id_size);
                directory_record rec((iso9660_id(id)));
                rec.data_pos = raw.data_pos;
                rec.data_size = raw.data_size;
                rec.recorded_time = raw.recorded_time;
                rec.flags = raw.flags;
                if (std::size_t su_len = raw.record_size - (bin_size + id_size))
                {
                    rec.system_use.assign(
                        &block_[offset+bin_size + id_size], su_len);
                }

                const directory_record& prev = records.back();
                if (!(prev < rec))
                {
                    throw BOOST_IOSTREAMS_FAILURE(
                        "invalid ISO 9660 order of directory records");
                }

                records.push_back(rec);

                pos += raw.record_size;
            }
            else
                pos += (volume_desc_.logical_block_size - offset);
        }

        if (pos != self.data_size)
            throw BOOST_IOSTREAMS_FAILURE("invalid ISO 9660 directory records");

        for (std::size_t i = 0; i < records.size(); ++i)
            read_continuation_area(records[i]);

        dir_records_.swap(records);
        dir_pos_ = 2;
    }

    void rock_ridge_check()
    {
        rrip_ = rrip_none;

        const std::string& su = dir_records_[0].system_use;
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
        if ((dir_pos_ != 1) &&
            !internal_.is_relocated && header_.is_directory())
        {
            dir_path_ = header_.path;
            stack_.push(dir_pos_);
            if (internal_.child_pos)
                select_directory(internal_.child_pos);
            else
                select_directory(dir_records_[dir_pos_].data_pos);
        }
        else
            ++dir_pos_;

        while (dir_pos_ == dir_records_.size())
        {
            if (stack_.empty())
                return false;

            if (rrip_ != rrip_none)
            {
                iso::header h;
                internal_data internal;
                h.system_use = dir_records_[1].system_use;
                parse_rock_ridge(h, internal);

                if (internal.parent_pos != 0)
                    select_directory(internal.parent_pos);
                else
                    select_directory(dir_records_[1].data_pos);
            }
            else
                select_directory(dir_records_[1].data_pos);

            dir_path_ = dir_path_.branch_path();
            dir_pos_ = stack_.top() + 1;
            stack_.pop();
        }

        const directory_record& rec = dir_records_[dir_pos_];

        iso::header h;
        h.path = dir_path_ / rec.file_id.to_path();
        h.file_size = rec.data_size;
        h.recorded_time = rec.recorded_time;
        h.flags = rec.flags;
        h.system_use = rec.system_use;
        header_ = h;

        if (rrip_ != rrip_none)
        {
            internal_ = internal_data();
            parse_rock_ridge(header_, internal_);
            if (internal_.child_pos != 0)
                header_.flags |= iso::file_flags::directory;
        }
        else
            header_ = h;

        seek_logical_block(rec.data_pos);

        return true;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_FILE_SOURCE_IMPL_HPP
