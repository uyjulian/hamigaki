//  rock_ridge_directory_writer.hpp: IEEE P1282 Rock Ridge directory writer

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_WRITER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_WRITER_HPP

#include <hamigaki/archivers/detail/iso_directory_writer.hpp>
#include <hamigaki/archivers/iso/ce_system_use_entry_data.hpp>
#include <hamigaki/integer/rounding.hpp>

namespace hamigaki { namespace archivers { namespace detail {

inline unsigned iso_directory_depth(const boost::filesystem::path& ph)
{
    return static_cast<unsigned>(std::distance(ph.begin(), ph.end()));
}

class rock_ridge_directory_writer : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;
    static const std::size_t cl_size = 12u;
    static const std::size_t pl_size = 12u;

    static const std::size_t sys_entry_head_size =
        hamigaki::struct_size<iso::system_use_entry_header>::value;

    static const std::size_t ce_data_size =
        hamigaki::struct_size<iso::ce_system_use_entry_data>::value;

    static const std::size_t ce_size = sys_entry_head_size + ce_data_size;

    typedef boost::filesystem::path path;
    typedef std::map<path,path> path_cvt_table;
    typedef boost::ptr_vector<iso_path_table_record> path_table_records;
    typedef std::set<iso_directory_record> directory_entries;

public:
    rock_ridge_directory_writer(boost::uint32_t lbn_shift, iso::rrip_type rrip)
        : lbn_shift_(lbn_shift), lbn_mask_((1ul << lbn_shift) - 1), rrip_(rrip)
    {
    }

    void add(const path& ph, const std::vector<iso_directory_record>& recs)
    {
        unsigned depth = detail::iso_directory_depth(ph);
        bool need_move = (depth == 7) || ((depth > 7) && ((depth-7) % 6 == 0));
        bool is_moved = (depth == 8) || ((depth > 8) && ((depth-8) % 6 == 0));

        std::vector<bool> flags(recs.size());
        directory_entries entries;

        path cur_path = cvt_table_[ph];

        std::size_t level = 0;
        boost::uint16_t parent_index = 0;

        iso_directory_record cur_dir;
        iso_directory_record par_dir;
        if (ph.empty())
        {
            cur_dir.data_pos = 0;
            cur_dir.data_size = 0;
            cur_dir.flags = iso::file_flags::directory;
            cur_dir.file_id.assign(1u, '\x00');

            par_dir.data_pos = 0;
            par_dir.data_size = 0;
            par_dir.flags = iso::file_flags::directory;
            par_dir.file_id.assign(1u, '\x01');
        }
        else
        {
            std::size_t parent_level;
            boost::tie(parent_level, parent_index)
                = find_directory(cur_path.branch_path());
            level = parent_level + 1;

            const iso_path_table_record& rec =
                path_table_.at(parent_level).at(parent_index);

            cur_dir.data_pos = 0;
            cur_dir.data_size = 0;
            cur_dir.flags = iso::file_flags::directory;
            cur_dir.file_id = cur_path.leaf();

            cur_dir = *rec.entries.find(cur_dir);
            cur_dir.file_id.assign(1u, '\x00');

            par_dir = *rec.entries.begin();
            par_dir.file_id.assign(1u, '\x01');
            if (is_moved)
                par_dir.system_use.assign(pl_size, '\0');
        }
        entries.insert(cur_dir);
        entries.insert(par_dir);

        bool found_dir = false;
        const std::size_t size = recs.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            const iso_directory_record& rec = recs[i];
            if (has_valid_iso_id(rec))
            {
                if (rec.is_directory())
                {
                    found_dir = true;
                    if (need_move)
                    {
                        iso_directory_record new_rec(rec);
                        new_rec.system_use.append(cl_size, '\0');
                        entries.insert(new_rec);
                    }
                    else
                        entries.insert(rec);
                    cvt_table_[ph/rec.file_id] = cur_path/rec.file_id;
                }
                else
                    entries.insert(rec);
                flags[i] = true;
            }
        }

        for (std::size_t i = 0; i < size; ++i)
        {
            const iso_directory_record& rec = recs[i];
            if (!flags[i])
            {
                const std::string& new_id =
                    detail::make_iso_alt_id(entries, rec);

                iso_directory_record new_rec(rec);
                new_rec.file_id = new_id;

                if (rec.is_directory())
                {
                    found_dir = true;
                    if (need_move)
                        new_rec.system_use.append(cl_size, '\0');
                    cvt_table_[ph/rec.file_id] = cur_path/new_id;
                }

                entries.insert(new_rec);
            }
        }

        if (level >= path_table_.size())
            path_table_.push_back(new path_table_records);
        path_table_records& table = path_table_.at(level);

        std::auto_ptr<iso_path_table_record> ptr(new iso_path_table_record);
        ptr->dir_id = detail::make_iso_dir_id(cur_path.leaf());
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

        if (need_move && found_dir)
            this->add_moved(ph, recs);
    }

    template<class Sink>
    iso_path_table_info write(Sink& sink)
    {
        std::string& su = path_table_.at(0).at(0).entries.begin()->system_use;
        su.insert(0, "SP\x07\x01\xBE\xEF\x00", 7);
        if (rrip_ == rrip_1991a)
        {
            su.append(
                "ER\xED\x01\x0A\x54\x87\x01"
                "RRIP_1991A"
                "THE ROCK RIDGE INTERCHANGE PROTOCOL PROVIDES SUPPORT "
                "FOR POSIX FILE SYSTEM SEMANTICS"
                "PLEASE CONTACT DISC PUBLISHER FOR SPECIFICATION SOURCE.  "
                "SEE PUBLISHER IDENTIFIER IN PRIMARY VOLUME DESCRIPTOR "
                "FOR CONTACT INFORMATION.", 237u);
        }
        else
        {
            su.append(
                "ER\xB9\x01\x0A\x49\x5E\x01"
                "IEEE_P1282"
                "THE IEEE P1282 PROTOCOL PROVIDES SUPPORT "
                "FOR POSIX FILE SYSTEM SEMANTICS."
                "PLEASE CONTACT THE IEEE STANDARDS DEPARTMENT, "
                "PISCATAWAY, NJ, USA FOR THE P1282 SPECIFICATION.", 185u);
        }

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
    path_cvt_table cvt_table_;
    boost::ptr_vector<path_table_records> path_table_;
    char block_[logical_sector_size];
    path_cvt_table re_table_;
    path rr_moved_;
    boost::uint16_t rr_moved_index_;
    iso::rrip_type rrip_;

    template<class Sink>
    boost::uint32_t tell(Sink& sink)
    {
        iostreams::stream_offset offset = iostreams::tell_offset(sink);
        BOOST_ASSERT((offset & lbn_mask_) == 0);
        return static_cast<boost::uint64_t>(offset) >> lbn_shift_;
    }

    boost::uint32_t round_to_block_size(boost::uint32_t n)
    {
        if ((n & lbn_mask_) != 0u)
            return (n | lbn_mask_) + 1u;
        else
            return n;
    }


    boost::uint16_t find_path(
        std::size_t level, boost::uint16_t parent, const std::string& s) const
    {
        const path_table_records& table = path_table_.at(level);

        iso_path_table_record x;
        x.dir_id = detail::make_iso_dir_id(s);
        x.parent_index = parent;

        path_table_records::const_iterator i =
            std::lower_bound(table.begin(), table.end(), x);
        if ((i == table.end()) || (x < *i))
            throw std::runtime_error("directory not found");

        return i - table.begin();
    }

    std::pair<std::size_t,boost::uint16_t> find_directory(const path& ph) const
    {
        std::size_t level = 0;
        boost::uint16_t parent = 0;
        for (path::iterator cur = ph.begin(), end = ph.end(); cur != end; ++cur)
            parent = this->find_path(++level, parent, *cur);

        return std::make_pair(level, parent);
    }

    std::pair<boost::uint32_t,boost::uint32_t>
    calc_system_use_size(const std::string& su, boost::uint32_t limit)
    {
        typedef std::pair<boost::uint32_t,boost::uint32_t> result_type;

        if ((limit & 1u) != 0)
            --limit;

        boost::uint32_t su_size = su.size();
        if (su_size <= limit)
            return result_type(round_to_even(su_size), 0u);

        BOOST_ASSERT(limit >= ce_size);
        limit -= ce_size;

        boost::uint32_t total = 0;
        boost::uint32_t = pos;
        while (pos+3 < su_size)
        {
            boost::uint32_t n = static_cast<unsigned char>(su[pos]);
            if (total + n > limit)
                return result_type(round_to_even(total+ce_size), su_size - pos);

            total += n;
            pos += n;
        }
        throw std::runtime_error("bad system use entry");
        BOOST_UNREACHABLE_RETURN(result_type(0u,0u))
    }

    std::pair<boost::uint32_t,boost::uint32_t>
    calc_directory_size(const directory_entries& entries)
    {
        const boost::uint32_t block_size = 1ul << lbn_shift_;
        const std::size_t bin_size = struct_size<iso::directory_record>::value;

        std::size_t pos = 0;
        std::size_t cont_size = 0;
        typedef directory_entries::const_iterator iter_type;
        for (iter_type i = entries.begin(), end = entries.end(); i != end; ++i)
        {
            const iso_directory_record& rec = *i;
            std::string id = rec.file_id;
            if (!rec.is_directory())
            {
                id += ';';
                id += hamigaki::to_dec<char>(rec.version);
            }
            std::size_t id_size = id.size();
            std::size_t size = round_to_even(bin_size + id_size);

            boost::uint32_t sys_size;
            boost::uint32_t rest_size;
            boost::tie(sys_size, rest_size) =
                calc_system_use_size(rec.system_use, 0xFFu-size);
            size += sys_size;
            cont_size += rest_size;

            std::size_t offset = pos & lbn_mask_;
            if (offset + size > block_size)
                pos = round_to_block_size(pos);

            pos += size;
        }

        pos = round_to_block_size(pos);
        cont_size = round_to_block_size(cont_size);

        return std::pair<boost::uint32_t,boost::uint32_t>(pos, cont_size);
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

                boost::uint32_t dir_size;
                boost::uint32_t cont_size;
                boost::tie(dir_size, cont_size) = calc_directory_size(entries);

                typedef directory_entries::iterator iter_type;
                iter_type cur = entries.begin();
                cur->data_pos = pos;
                cur->data_size = dir_size;

                if (level != 0)
                {
                    iso_path_table_record& parent_table =
                        path_table_[level-1][table[i].parent_index];
                    directory_entries& parent_entries = parent_table.entries;

                    iter_type dst = boost::next(entries.begin());
                    iter_type src = parent_entries.begin();
                    dst->data_pos = src->data_pos;
                    dst->data_size = src->data_size;

                    iso_directory_record x;
                    x.data_pos = 0;
                    x.data_size = 0;
                    x.flags = iso::file_flags::directory;
                    x.file_id = table[i].dir_id;
                    iter_type it = parent_entries.find(x);

                    BOOST_ASSERT(it != parent_entries.end());

                    it->data_pos = pos;
                    it->data_size = dir_size;
                }
                else
                {
                    iter_type dst = boost::next(entries.begin());
                    dst->data_pos = pos;
                    dst->data_size = dir_size;
                }

                table[i].parent_index += (base - prev_count);
                table[i].data_pos = pos;

                pos += ((dir_size+cont_size) >> lbn_shift_);
            }
            prev_count = table.size();
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
        std::size_t cont_base =
            this->tell(sink) + (entries.begin()->data_size >> lbn_shift_);
        std::size_t cont_off =  0;
        std::string cont_area;
        typedef directory_entries::const_iterator iter_type;
        for (iter_type i = entries.begin(), end = entries.end(); i != end; ++i)
        {
            const iso_directory_record& rec = *i;
            std::string id = rec.file_id;
            if (!rec.is_directory())
            {
                id += ';';
                id += hamigaki::to_dec<char>(rec.version);
            }
            std::size_t id_size = id.size();
            std::size_t size = round_to_even(bin_size + id_size);

            boost::uint32_t sys_size;
            boost::uint32_t rest_size;
            boost::tie(sys_size, rest_size) =
                calc_system_use_size(rec.system_use, 0xFFu-size);
            size += sys_size;

            std::size_t offset = pos & lbn_mask_;
            if (offset + size > block_size)
            {
                iostreams::blocking_write(sink, block_, block_size);
                std::memset(block_, 0, sizeof(block_));
                pos = round_to_block_size(pos);
                offset = 0;
            }

            iso::directory_record raw;
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

            char* out = block_+offset;
            hamigaki::binary_write(out, raw);
            out += bin_size;

            std::size_t id_len = round_to_odd(id_size);
            std::memcpy(out, id.c_str(), id_len);
            out += id_len;

            if (rest_size != 0)
            {
                std::size_t su_size = rec.system_use.size();
                std::size_t copy_size = su_size - rest_size - ce_size;
                std::memcpy(out, rec.system_use.c_str(), copy_size);
                out += copy_size;

                iso::system_use_entry_header head;
                head.signature[0] = 'C';
                head.signature[1] = 'E';
                head.entry_size = ce_size;
                head.version = 1u;

                hamigaki::binary_write(out, head);
                out += sys_entry_head_size;

                iso::ce_system_use_entry_data data;
                data.next_pos = cont_base + (cont_off & lbn_mask_);
                data.next_offset = cont_off;
                data.next_size = cont_size;

                hamigaki::binary_write(out, data);
                out += ce_data_size;

                if (((copy_size + ce_size) & 1u) != 0u)
                    *(out++) = '\0';

                cont_area.append(rec.system_use, copy_size, rest_size);
            }
            else
                std::memcpy(out, rec.system_use.c_str(), sys_size);

            pos += size;
            cont_off += rest_size;
        }

        if ((pos & lbn_mask_) != 0)
        {
            iostreams::blocking_write(sink, block_, block_size);
            std::memset(block_, 0, sizeof(block_));
        }

        if (!cont_area.empty())
        {
            std::size_t size = cont_area.size();
            iostreams::blocking_write(sink, cont_area.data(), size);

            if (std::size_t off = size & lbn_mask_)
            {
                std::size_t rest = (1ul << lbn_shift_) - off;
                iostreams::blocking_write(sink, block_, rest);
            }
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
                raw.dir_id_size = rec.dir_id.size();
                raw.ext_record_size = 0;
                raw.data_pos = rec.data_pos;
                raw.parent_dir_number = rec.parent_index;
                iostreams::binary_write<E>(sink, raw);

                std::size_t size = round_to_even(raw.dir_id_size);
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

        return buffer.size();
    }

    void create_rr_moved()
    {
        directory_entries& entries = path_table_.at(0).at(0).entries;

        iso_directory_record rec;
        rec.flags = iso::file_flags::directory;
        rec.file_id = "rr_moved";
        rec.file_id = detail::make_iso_alt_id(entries, rec);
        entries.insert(rec);

        rr_moved_ = path(rec.file_id);

        std::auto_ptr<iso_path_table_record> ptr(new iso_path_table_record);
        ptr->dir_id = rec.file_id;
        ptr->data_pos = 0;
        ptr->parent_index = 0;

        iso_directory_record cur_dir = rec;
        cur_dir.file_id.assign(1u, '\x00');
        ptr->entries.insert(cur_dir);

        iso_directory_record par_dir = *entries.begin();
        par_dir.file_id.assign(1u, '\x01');
        ptr->entries.insert(par_dir);

        path_table_records& table = path_table_.at(1);
        path_table_records::iterator it =
            std::lower_bound(table.begin(), table.end(), *ptr);

        rr_moved_index_ =
            static_cast<boost::uint16_t>(it - table.begin());
        table.insert(it, ptr.release());

        path_table_records& child_table = path_table_.at(2);
        for (std::size_t i = 0; i < child_table.size(); ++i)
        {
            if (child_table[i].parent_index >= index)
                ++(child_table[i].parent_index);
        }
    }

    void add_moved(
        const path& ph, const std::vector<iso_directory_record>& recs)
    {
        if (re_table_.empty())
            create_rr_moved();

        directory_entries& entries =
            path_table_.at(1).at(rr_moved_index_).entries;

        std::size_t level = 0;
        boost::uint16_t parent_index = 0;

        const std::size_t size = recs.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            if (rec.is_directory())
            {
                const iso_directory_record& rec = recs[i];
                const std::string& new_id =
                    detail::make_iso_alt_id(entries, rec);

                iso_directory_record new_rec(rec);
                new_rec.file_id = new_id;
                new_rec.system_use.append("RE\x04\x01", 4);

                const path& old_path = ph/rec.file_id;
                const path& moved_path = rr_moved_/new_id;
                re_table_[moved_path] = cvt_table_[old_path];
                cvt_table_[old_path] = moved_path;
                entries.insert(new_rec);
            }
        }
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_WRITER_HPP
