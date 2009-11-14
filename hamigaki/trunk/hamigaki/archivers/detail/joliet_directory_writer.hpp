// joliet_directory_writer.hpp: Joliet directory extent writer

// Copyright Takeshi Mouri 2007-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_DIRECTORY_WRITER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_DIRECTORY_WRITER_HPP

#include <hamigaki/archivers/detail/iso_path_table.hpp>
#include <hamigaki/archivers/detail/iso_string.hpp>
#include <hamigaki/archivers/iso/headers.hpp>
#include <hamigaki/charset/utf16.hpp>
#include <hamigaki/dec_format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/noncopyable.hpp>
#include <functional>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

template<class String>
inline std::string make_joliet_dir_id(const String& s)
{
    if (s.empty())
        return std::string(1, '\0');
    else
        return detail::to_joliet_string(s);
}

template<class Path>
class joliet_directory_writer : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;

    typedef boost::ptr_vector<iso_path_table_record> path_table_records;
    typedef std::set<iso_directory_record> directory_entries;

public:
    typedef Path path_type;
    typedef typename Path::string_type string_type;

    joliet_directory_writer(
        boost::uint32_t lbn_shift, const iso_directory_record& root
    )
        : lbn_shift_(lbn_shift), lbn_mask_((1ul << lbn_shift) - 1), root_(root)
    {
    }

    void add(const Path& ph, const std::vector<iso_directory_record>& recs)
    {
        directory_entries entries;

        std::size_t level = 0;
        boost::uint16_t parent_index = 0;

        iso_directory_record cur_dir;
        iso_directory_record par_dir;
        if (ph.empty())
        {
            cur_dir = root_;
            cur_dir.file_id.assign(1u, '\x00');

            par_dir = root_;
            par_dir.file_id.assign(1u, '\x01');
        }
        else
        {
            std::size_t parent_level;
            boost::tie(parent_level, parent_index)
                = find_directory(ph.branch_path());
            level = parent_level + 1;

            const iso_path_table_record& rec =
                path_table_.at(parent_level).at(parent_index);

            cur_dir.data_pos = 0;
            cur_dir.data_size = 0;
            cur_dir.flags = iso::file_flags::directory;
            cur_dir.file_id = detail::to_joliet_string(ph.leaf());

            cur_dir = *rec.entries.find(cur_dir);
            cur_dir.file_id.assign(1u, '\x00');

            par_dir = *rec.entries.begin();
            par_dir.file_id.assign(1u, '\x01');
        }
        entries.insert(cur_dir);
        entries.insert(par_dir);

        const std::size_t size = recs.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            iso_directory_record rec(recs[i]);
            rec.file_id = rec.file_id;
            entries.insert(rec);
        }

        if (level >= path_table_.size())
            path_table_.push_back(new path_table_records);
        path_table_records& table = path_table_.at(level);

        std::auto_ptr<iso_path_table_record> ptr(new iso_path_table_record);
        ptr->dir_id = detail::make_joliet_dir_id(ph.leaf());
        ptr->data_pos = 0;
        ptr->parent_index = parent_index;
        ptr->entries.swap(entries);

        if (table.empty())
            table.push_back(ptr.release());
        else
        {
            path_table_records::iterator it =
                std::lower_bound(table.begin(), table.end(), *ptr);
            boost::uint16_t index =
                static_cast<boost::uint16_t>(it - table.begin());
            table.insert(it, ptr.release());

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

    template<class Sink>
    iso_path_table_info write(Sink& sink)
    {
        boost::uint32_t pos = this->tell(sink);
        this->set_directory_sizes(pos);

        for (std::size_t level = 0; level < path_table_.size(); ++level)
        {
            path_table_records& table = path_table_[level];
            for (std::size_t i = 0; i < table.size(); ++i)
                this->write_directory_records(sink, table[i].entries);
        }

        const iso_directory_record& root = *path_table_[0][0].entries.begin();

        iso_path_table_info info;
        info.root_record.record_size =
            struct_size<iso::directory_record>::value + 1u;
        info.root_record.ext_record_size = 0;
        info.root_record.data_pos = root.data_pos;
        info.root_record.data_size = root.data_size;
        info.root_record.recorded_time = root.recorded_time;
        info.root_record.flags = root.flags;
        info.root_record.unit_size = 0u;
        info.root_record.interleave_gap_size = 0u;
        info.root_record.volume_seq_number = 1u;
        info.root_record.file_id_size = 1u;

        info.l_path_table_pos = this->tell(sink);
        info.path_table_size = this->write_path_table<little>(sink);

        info.m_path_table_pos = this->tell(sink);
        this->write_path_table<big>(sink);

        return info;
    }

private:
    boost::uint32_t lbn_shift_;
    boost::uint32_t lbn_mask_;
    iso_directory_record root_;
    boost::ptr_vector<path_table_records> path_table_;
    char block_[logical_sector_size];

    template<class Sink>
    boost::uint32_t tell(Sink& sink)
    {
        iostreams::stream_offset offset = iostreams::tell_offset(sink);
        BOOST_ASSERT((offset & lbn_mask_) == 0);
        return static_cast<boost::uint32_t>(
            static_cast<boost::uint64_t>(offset) >> lbn_shift_);
    }

    boost::uint16_t find_path(
        std::size_t level, boost::uint16_t parent, const string_type& s) const
    {
        const path_table_records& table = path_table_.at(level);

        iso_path_table_record x;
        x.dir_id = detail::make_joliet_dir_id(s);
        x.parent_index = parent;

        path_table_records::const_iterator i =
            std::lower_bound(table.begin(), table.end(), x);
        if ((i == table.end()) || (x < *i))
            throw std::runtime_error("directory not found");

        return static_cast<boost::uint16_t>(i - table.begin());
    }

    std::pair<std::size_t,boost::uint16_t> find_directory(const Path& ph) const
    {
        typedef typename Path::const_iterator iter_type;

        std::size_t level = 0;
        boost::uint16_t parent = 0;
        for (iter_type cur = ph.begin(), end = ph.end(); cur != end; ++cur)
            parent = this->find_path(++level, parent, *cur);

        return std::make_pair(level, parent);
    }

    boost::uint32_t calc_directory_size(const directory_entries& entries)
    {
        const boost::uint32_t block_size = 1ul << lbn_shift_;
        const std::size_t bin_size = struct_size<iso::directory_record>::value;

        std::size_t pos = 0;
        typedef directory_entries::const_iterator iter_type;
        for (iter_type i = entries.begin(), end = entries.end(); i != end; ++i)
        {
            const iso_directory_record& rec = *i;
            std::string id = rec.file_id;
            if (!rec.is_directory())
            {
                id.append("\0;", 2);
                id += charset::to_utf16be(
                    hamigaki::to_dec<wchar_t>(rec.version));
            }
            std::size_t id_size = id.size();
            std::size_t size = bin_size + id_size;
            if ((id_size & 1) == 0)
                ++size;

            std::size_t sys_size = rec.system_use.size();
            if ((sys_size & 1) != 0)
                ++sys_size;
            size += sys_size;

            std::size_t offset = pos & lbn_mask_;
            if (offset + size > block_size)
            {
                pos |= lbn_mask_;
                ++pos;
            }

            pos += size;
        }

        if ((pos & lbn_mask_) != 0)
        {
            pos |= lbn_mask_;
            ++pos;
        }

        return static_cast<boost::uint32_t>(pos);
    }

    void set_directory_sizes(boost::uint32_t pos)
    {
        boost::uint16_t prev_count = 0;
        boost::uint16_t base = 1;
        for (std::size_t level = 0; level < path_table_.size(); ++level)
        {
            path_table_records& table = path_table_[level];
            for (std::size_t i = 0; i < table.size(); ++i)
            {
                directory_entries& entries = table[i].entries;

                boost::uint32_t dir_size = calc_directory_size(entries);

                typedef directory_entries::iterator iter_type;
                iter_type cur = entries.begin();
                iso_directory_record& cur_ref =
                    const_cast<iso_directory_record&>(*cur);
                cur_ref.data_pos = pos;
                cur_ref.data_size = dir_size;

                if (level != 0)
                {
                    iso_path_table_record& parent_table =
                        path_table_[level-1][table[i].parent_index];
                    directory_entries& parent_entries = parent_table.entries;

                    iter_type dst = boost::next(entries.begin());
                    iter_type src = parent_entries.begin();
                    iso_directory_record& dst_ref =
                        const_cast<iso_directory_record&>(*dst);
                    dst_ref.data_pos = src->data_pos;
                    dst_ref.data_size = src->data_size;

                    iso_directory_record x;
                    x.data_pos = 0;
                    x.data_size = 0;
                    x.flags = iso::file_flags::directory;
                    x.file_id = table[i].dir_id;
                    iter_type it = parent_entries.find(x);

                    BOOST_ASSERT(it != parent_entries.end());

                    iso_directory_record& it_ref =
                        const_cast<iso_directory_record&>(*it);
                    it_ref.data_pos = pos;
                    it_ref.data_size = dir_size;
                }
                else
                {
                    iter_type dst = boost::next(entries.begin());
                    iso_directory_record& dst_ref =
                        const_cast<iso_directory_record&>(*dst);
                    dst_ref.data_pos = pos;
                    dst_ref.data_size = dir_size;
                }

                table[i].parent_index += (base - prev_count);
                table[i].data_pos = pos;

                pos += (dir_size >> lbn_shift_);
            }
            prev_count = static_cast<boost::uint16_t>(table.size());
            base += prev_count;
        }
    }

    template<class Sink>
    void write_directory_records(Sink& sink, const directory_entries& entries)
    {
        const boost::uint32_t block_size = 1ul << lbn_shift_;
        const std::size_t bin_size = struct_size<iso::directory_record>::value;

        std::memset(block_, 0, sizeof(block_));

        std::size_t pos = 0;
        typedef directory_entries::const_iterator iter_type;
        for (iter_type i = entries.begin(), end = entries.end(); i != end; ++i)
        {
            const iso_directory_record& rec = *i;
            std::string id = rec.file_id;
            if (!rec.is_directory())
            {
                id.append("\0;", 2);
                id += charset::to_utf16be(
                    hamigaki::to_dec<wchar_t>(rec.version));
            }
            std::size_t id_size = id.size();
            std::size_t size = bin_size + id_size;
            if ((id_size & 1) == 0)
                ++size;

            std::size_t sys_size = rec.system_use.size();
            if ((sys_size & 1) != 0)
                ++sys_size;
            size += sys_size;

            std::size_t offset = pos & lbn_mask_;
            if (offset + size > block_size)
            {
                iostreams::blocking_write(sink, block_, block_size);
                std::memset(block_, 0, sizeof(block_));
                pos |= lbn_mask_;
                ++pos;
                offset = 0;
            }

            iso::directory_record raw;
            raw.record_size = static_cast<boost::uint8_t>(size);
            raw.ext_record_size = 0;
            raw.data_pos = rec.data_pos;
            raw.data_size = rec.data_size;
            raw.recorded_time = rec.recorded_time;
            raw.flags = rec.flags;
            raw.unit_size = 0;
            raw.interleave_gap_size = 0;
            raw.volume_seq_number = 1;
            raw.file_id_size = static_cast<boost::uint8_t>(id_size);

            char* out = block_+offset;
            hamigaki::binary_write(out, raw);
            out += bin_size;

            std::size_t id_len = (id_size & 1) == 0 ? id_size+1 : id_size;
            std::memcpy(out, id.c_str(), id_len);
            out += id_len;

            std::memcpy(out, rec.system_use.c_str(), sys_size);
            pos += size;
        }

        if ((pos & lbn_mask_) != 0)
        {
            iostreams::blocking_write(sink, block_, block_size);
            std::memset(block_, 0, sizeof(block_));
        }
    }

    template<endianness E>
    void make_path_table(std::vector<char>& buffer)
    {
        boost::iostreams::back_insert_device<std::vector<char> > sink(buffer);

        for (std::size_t level = 0; level < path_table_.size(); ++level)
        {
            const path_table_records& table = path_table_[level];
            for (std::size_t i = 0; i < table.size(); ++i)
            {
                const iso_path_table_record& rec = table[i];
                iso::path_table_record raw;
                raw.dir_id_size =
                    static_cast<boost::uint8_t>(rec.dir_id.size());
                raw.ext_record_size = 0;
                raw.data_pos = rec.data_pos;
                raw.parent_dir_number = rec.parent_index;
                iostreams::binary_write<E>(sink, raw);

                std::size_t size = raw.dir_id_size;
                if ((size & 1) != 0)
                    ++size;
                iostreams::blocking_write(sink, rec.dir_id.c_str(), size);
            }
        }
    }

    template<endianness E, class Sink>
    boost::uint32_t write_path_table(Sink& sink)
    {
        std::vector<char> buffer;
        this->make_path_table<E>(buffer);

        if (!buffer.empty())
            iostreams::blocking_write(sink, &buffer[0], buffer.size());

        boost::uint32_t offset = buffer.size() & lbn_mask_;
        boost::uint32_t pad_size = (1ul << lbn_shift_) - offset;
        if (pad_size != 0)
            iostreams::blocking_write(sink, block_, pad_size);

        return static_cast<boost::uint32_t>(buffer.size());
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_JOLIET_DIRECTORY_WRITER_HPP
