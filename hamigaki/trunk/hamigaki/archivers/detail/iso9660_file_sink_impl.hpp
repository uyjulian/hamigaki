//  iso9660_file_sink_impl.hpp: ISO 9660 file sink implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/iso9660_id.hpp>
#include <hamigaki/archivers/iso9660/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/assert.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <stdexcept>

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink>
class basic_iso9660_file_sink_impl : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;

    struct directory_record
    {
        iso9660_id file_id;
        boost::uint32_t data_pos;
        boost::uint32_t data_size;
        iso9660::binary_date_time recorded_time;
        boost::uint8_t flags;

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

            bool lhs_assoc = (flags & iso9660::file_flags::associated) != 0;
            bool rhs_assoc = (rhs.flags & iso9660::file_flags::associated) != 0;

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
            return (flags & iso9660::file_flags::directory) != 0;
        }
    };

    typedef std::vector<directory_record> directory_records;

    struct path_table_record
    {
        iso9660_id dir_id;
        boost::uint32_t data_pos;
        boost::uint16_t parent_index;
        boost::shared_ptr<directory_records> records;

        bool operator<(const path_table_record& rhs) const
        {
            if (parent_index != rhs.parent_index)
                return parent_index < rhs.parent_index;
            else
                return dir_id < rhs.dir_id;
        }
    };

    typedef std::vector<path_table_record> path_table_records;

public:
    explicit basic_iso9660_file_sink_impl(const Sink& sink)
        : sink_(sink), pos_(0), size_(0)
        , logical_block_size_(2048), lbn_shift_(11), lbn_mask_(0x7FF)
    {
        directory_record x;
        x.file_id = iso9660_id('\x00');
        x.flags = iso9660::file_flags::directory;
        add_directory(0, 0, x);

        std::memset(block_, 0, sizeof(block_));
        for (std::size_t i = 0; i < 16; ++i)
            iostreams::blocking_write(sink_, block_);

        // for Primary Volume Descriptor
        iostreams::blocking_write(sink_, block_);

        iso9660::volume_desc_set_terminator term;
        term.type = 255u;
        std::memcpy(term.std_id, "CD001", 5);
        term.version = 1u;
        iostreams::binary_write(sink_, term);
    }

    void create_entry(const iso9660::header& head)
    {
        pos_ = 0;
        size_ = head.file_size;

        boost::filesystem::path::iterator cur = head.path.begin();
        boost::filesystem::path::iterator end = head.path.end();
        if (cur == end)
            throw std::runtime_error("bad path");

        std::size_t level = 0;
        boost::uint16_t parent = 0;
        for (--end; cur != end; ++cur)
            parent = find_path(++level, parent, *cur);

        directory_record rec((iso9660_id(*cur)));
        rec.data_pos = tell();
        rec.data_size = head.file_size;
        if (head.recorded_time)
        {
            rec.recorded_time =
                iso9660::binary_date_time::from_timestamp(*head.recorded_time);
        }
        rec.flags = head.flags;

        add_record(level, parent, rec);

        if (rec.is_directory())
            add_directory(level+1, parent, rec);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        boost::uint64_t rest = size_ - pos_;
        std::streamsize amt = hamigaki::auto_min(n, rest);
        amt = boost::iostreams::write(sink_, s, amt);
        pos_ += amt;
        return amt;
    }

    void close()
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("ISO 9660 file size mismatch");

        boost::uint32_t offset = size_ & lbn_mask_;
        boost::uint32_t pad_size = (lbn_mask_+1) - offset;
        boost::iostreams::write(sink_, block_, pad_size);
    }

    void close_archive()
    {
        boost::uint16_t prev_count = 0;
        boost::uint16_t base = 1;
        boost::uint32_t pos = tell();
        for (std::size_t level = 0; level < path_table_.size(); ++level)
        {
            path_table_records& table = path_table_[level];
            for (std::size_t i = 0; i < table.size(); ++i)
            {
                directory_records& records = *(table[i].records);
                std::sort(records.begin(), records.end());

                boost::uint32_t dir_size = calc_directory_size(records);
                records[0].data_pos = pos;
                records[0].data_size = dir_size;
                if (level != 0)
                {
                    path_table_record& parent_table =
                        path_table_[level-1][table[i].parent_index];
                    directory_records& parent_records = *(parent_table.records);
                    records[1].data_size = parent_records[0].data_size;

                    directory_record x(table[i].dir_id);

                    typename directory_records::iterator it =
                        std::lower_bound(
                            parent_records.begin(),
                            parent_records.end(),
                            x
                        );
                    BOOST_ASSERT(it != parent_records.end());
                    BOOST_ASSERT(!(*it < x));
                    it->data_pos = pos;
                    it->data_size = dir_size;
                }
                else
                {
                    records[1].data_pos = pos;
                    records[1].data_size = dir_size;
                }

                table[i].parent_index += (base - prev_count);
                table[i].data_pos = pos;

                pos += (dir_size >> lbn_shift_);
            }
            prev_count = table.size();
            base += prev_count;
        }

        for (std::size_t level = 0; level < path_table_.size(); ++level)
        {
            path_table_records& table = path_table_[level];
            for (std::size_t i = 0; i < table.size(); ++i)
                write_directory_records(*(table[i].records));
        }

        const directory_record& root = (*(path_table_[0][0].records))[0];

        iso9660::volume_descriptor desc;
        desc.type = 1u;
        std::memcpy(desc.std_id, "CD001", 5);
        desc.version = 1u;
        desc.flags = 0u;
        std::memset(desc.system_id, ' ', 32);
        std::memset(desc.volume_id, ' ', 32);
        std::memset(desc.escape_sequences, 0, 32);
        desc.volume_set_size = 1;
        desc.volume_seq_number = 1;
        desc.logical_block_size = logical_block_size_;
        desc.root_record.record_size =
            struct_size<iso9660::directory_record>::value + 1;
        desc.root_record.ext_record_size = 0;
        desc.root_record.data_pos = root.data_pos;
        desc.root_record.data_size = root.data_size;
        desc.root_record.recorded_time = root.recorded_time;
        desc.root_record.flags = root.flags;
        desc.root_record.unit_size = 0;
        desc.root_record.interleave_gap_size = 0;
        desc.root_record.volume_seq_number = 1;
        desc.root_record.file_id_size = 1;
        desc.root_file_id  = '\0';
        std::memset(desc.volume_set_id, ' ', 128);
        std::memset(desc.publisher_id, ' ', 128);
        std::memset(desc.data_preparer_id, ' ', 128);
        std::memset(desc.application_id, ' ', 128);
        std::memset(desc.copyright_file_id, ' ', 37);
        std::memset(desc.abstract_file_id, ' ', 37);
        std::memset(desc.bibliographic_file_id, ' ', 37);
        desc.file_structure_version = 1u;
        std::memset(desc.application_use, 0, 512);

        desc.l_path_table_pos = tell();
        desc.l_path_table_pos2 = 0;
        desc.path_table_size = write_path_table<little>();

        desc.m_path_table_pos = tell();
        desc.m_path_table_pos2 = 0;
        write_path_table<big>();

        desc.volume_space_size = tell();

        boost::iostreams::seek(sink_, logical_sector_size*16, BOOST_IOS::beg);
        iostreams::binary_write(sink_, desc);

        boost::iostreams::seek(sink_, 0, BOOST_IOS::end);
        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    boost::uint64_t pos_;
    boost::uint64_t size_;
    boost::uint32_t logical_block_size_;
    boost::uint32_t lbn_shift_;
    boost::uint32_t lbn_mask_;
    char block_[logical_sector_size];
    boost::ptr_vector<path_table_records> path_table_;

    boost::uint32_t tell()
    {
        iostreams::stream_offset offset = iostreams::tell_offset(sink_);
        BOOST_ASSERT((offset & lbn_mask_) == 0);
        return static_cast<boost::uint64_t>(offset) >> lbn_shift_;
    }

    boost::uint16_t find_path(
        std::size_t level, boost::uint16_t parent, const std::string& s) const
    {
        const path_table_records& table = path_table_.at(level);

        path_table_record x;
        x.dir_id = iso9660_id(s);
        x.parent_index = parent;

        typename path_table_records::const_iterator i =
            std::lower_bound(table.begin(), table.end(), x);
        if ((i == table.end()) || (x < *i))
            throw std::runtime_error("directory not found");

        return i - table.begin();
    }

    void add_directory(
        std::size_t level, boost::uint16_t parent, const directory_record& rec)
    {
        if (level >= path_table_.size())
            path_table_.push_back(new path_table_records);

        path_table_records& table = path_table_[level];

        path_table_record x;
        x.dir_id = rec.file_id;
        x.parent_index = parent;
        x.records.reset(new directory_records);

        directory_records& records = *(x.records);
        directory_record self = rec;
        self.file_id = iso9660_id('\x00');
        records.push_back(self);

        if (level == 0)
        {
            directory_record up = rec;
            up.file_id = iso9660_id('\x01');
            records.push_back(up);
        }
        else
        {
            directory_record up =
                path_table_[level-1][parent].records->front();
            up.file_id = iso9660_id('\x01');
            records.push_back(up);
        }

        if (table.empty())
            table.push_back(x);
        else
        {
            typename path_table_records::iterator it =
                std::lower_bound(table.begin(), table.end(), x);
            boost::uint16_t index =
                static_cast<boost::uint16_t>(it - table.begin());
            table.insert(it, x);

            if (level+1 < path_table_.size())
            {
                path_table_records& table = path_table_[level+1];
                for (std::size_t i = 0; i < table.size(); ++i)
                {
                    if (table[i].parent_index >= index)
                        ++(table[i].parent_index);
                }
            }
        }
    }

    void add_record(
        std::size_t level, boost::uint16_t parent, const directory_record& rec)
    {
        path_table_.at(level)[parent].records->push_back(rec);
    }

    boost::uint32_t calc_directory_size(const directory_records& records)
    {
        const std::size_t bin_size =
            struct_size<iso9660::directory_record>::value;

        std::size_t pos = 0;
        for (std::size_t i = 0; i < records.size(); ++i)
        {
            const directory_record& rec = records[i];
            const std::string& id = rec.file_id.to_string();
            std::size_t id_size = id.size();
            std::size_t size = bin_size + id_size;
            if ((id_size & 1) == 0)
                ++size;

            std::size_t offset = pos & lbn_mask_;
            if (offset + size > logical_block_size_)
            {
                pos |= lbn_mask_;
                ++pos;
                offset = 0;
            }

            pos += size;
        }

        if ((pos & lbn_mask_) != 0)
        {
            pos |= lbn_mask_;
            ++pos;
        }

        return pos;
    }

    void write_directory_records(const directory_records& records)
    {
        const std::size_t bin_size =
            struct_size<iso9660::directory_record>::value;

        std::memset(block_, 0, sizeof(block_));

        std::size_t pos = 0;
        for (std::size_t i = 0; i < records.size(); ++i)
        {
            const directory_record& rec = records[i];
            const std::string& id = rec.file_id.to_string();
            std::size_t id_size = id.size();
            std::size_t size = bin_size + id_size;
            if ((id_size & 1) == 0)
                ++size;

            std::size_t offset = pos & lbn_mask_;
            if (offset + size > logical_block_size_)
            {
                iostreams::blocking_write(sink_, block_, logical_block_size_);
                std::memset(block_, 0, sizeof(block_));
                pos |= lbn_mask_;
                ++pos;
                offset = 0;
            }

            iso9660::directory_record raw;
            raw.record_size = size;
            raw.ext_record_size = 0;
            raw.data_pos = rec.data_pos;
            raw.data_size = rec.data_size;
            raw.recorded_time = rec.recorded_time;
            raw.flags = rec.flags;
            raw.unit_size = 0;
            raw.interleave_gap_size = 0;
            raw.volume_seq_number = 1;
            raw.file_id_size = id_size;

            hamigaki::binary_write(block_+offset, raw);
            std::memcpy(block_+offset+bin_size, id.c_str(), id_size);
            pos += size;
        }

        if ((pos & lbn_mask_) != 0)
        {
            iostreams::blocking_write(sink_, block_, logical_block_size_);
            std::memset(block_, 0, sizeof(block_));
        }
    }

    template<endianness E>
    boost::uint32_t write_path_table()
    {
        std::vector<char> buffer;
        boost::iostreams::back_insert_device<std::vector<char> > sink(buffer);

        for (std::size_t level = 0; level < path_table_.size(); ++level)
        {
            const path_table_records& table = path_table_[level];
            for (std::size_t i = 0; i < table.size(); ++i)
            {
                const path_table_record& rec = table[i];
                const std::string& id = rec.dir_id.to_string();
                iso9660::path_table_record raw;
                raw.dir_id_size = id.size();
                raw.ext_record_size = 0;
                raw.data_pos = rec.data_pos;
                raw.parent_dir_number = rec.parent_index;
                iostreams::binary_write<E>(sink, raw);

                std::size_t size = raw.dir_id_size;
                if ((size & 1) != 0)
                    ++size;
                iostreams::blocking_write(sink, id.c_str(), size);
            }
        }

        if (!buffer.empty())
            iostreams::blocking_write(sink_, &buffer[0], buffer.size());

        boost::uint32_t offset = buffer.size() & lbn_mask_;
        boost::uint32_t pad_size = logical_block_size_ - offset;
        if (pad_size != 0)
            iostreams::blocking_write(sink_, block_, pad_size);

        return buffer.size();
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SINK_IMPL_HPP
