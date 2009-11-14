// rock_ridge_directory_writer.hpp: IEEE P1282 Rock Ridge directory writer

// Copyright Takeshi Mouri 2007-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_WRITER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_WRITER_HPP

#include <hamigaki/archivers/detail/iso_directory_writer.hpp>
#include <hamigaki/archivers/iso/ce_system_use_entry_data.hpp>
#include <hamigaki/archivers/iso/px_system_use_entry_data.hpp>
#include <hamigaki/archivers/iso/rrip_type.hpp>
#include <hamigaki/archivers/iso/system_use_entry_header.hpp>
#include <hamigaki/integer/rounding.hpp>

namespace hamigaki { namespace archivers { namespace detail {

class rock_ridge_directory_writer : private boost::noncopyable
{
private:
    typedef rock_ridge_directory_writer self;

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
    typedef std::map<path,iso_directory_record*> link_table;

public:
    rock_ridge_directory_writer(
        boost::uint32_t lbn_shift, const iso_directory_record& root,
        unsigned iso_level, iso::rrip_type rrip
    )
        : lbn_shift_(lbn_shift), lbn_mask_((1ul << lbn_shift) - 1), root_(root)
        , iso_level_(iso_level), rrip_(rrip)
    {
    }

    void add(const path& ph, const std::vector<iso_directory_record>& recs)
    {
        unsigned depth = self::iso_directory_depth(ph);
        bool need_move = (depth == 7) || ((depth > 7) && ((depth-7) % 6 == 0));
        bool is_moved = (depth == 8) || ((depth > 8) && ((depth-8) % 6 == 0));

        if (iso_level_ == 4u)
        {
            need_move = false;
            is_moved = false;
        }

        std::vector<bool> flags(recs.size());
        directory_entries entries;

        path cur_path = cvt_table_[ph];

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
            self::remove_nm_system_use_entry(cur_dir.system_use);
            if (is_moved)
                cur_dir.system_use.resize(cur_dir.system_use.size()-4);

            par_dir = *rec.entries.begin();
            par_dir.file_id.assign(1u, '\x01');
            self::remove_nm_system_use_entry(par_dir.system_use);
            if (is_moved)
                par_dir.system_use.append("PL\x0C\x01\0\0\0\0\0\0\0\0", 12);
        }
        entries.insert(cur_dir);
        entries.insert(par_dir);

        bool found_dir = false;
        const std::size_t size = recs.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            const iso_directory_record& rec = recs[i];
            if (has_valid_iso_id(iso_level_, rec))
            {
                iso_directory_record new_rec(rec);
                self::set_nm_system_use_entry(new_rec);
                if (rec.is_directory())
                {
                    found_dir = true;
                    if (need_move)
                        self::fix_moved_directory_record(new_rec);
                    cvt_table_[ph/rec.file_id] = cur_path/rec.file_id;
                }
                entries.insert(new_rec);
                flags[i] = true;
            }
        }

        for (std::size_t i = 0; i < size; ++i)
        {
            const iso_directory_record& rec = recs[i];
            if (!flags[i])
            {
                const std::string& new_id =
                    detail::make_iso_alt_id(iso_level_, entries, rec);

                iso_directory_record new_rec(rec);
                self::set_nm_system_use_entry(new_rec);
                new_rec.file_id = new_id;

                if (rec.is_directory())
                {
                    found_dir = true;
                    if (need_move)
                        self::fix_moved_directory_record(new_rec);
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
        ptr->full_path = cur_path;

        iso_directory_record* cur =
            const_cast<iso_directory_record*>(&*ptr->entries.begin());

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
            this->add_moved(ph, cur, recs);
    }

    template<class Sink>
    iso_path_table_info write(Sink& sink)
    {
        std::string& su = const_cast<iso_directory_record&>(
            *path_table_.at(0).at(0).entries.begin()).system_use;
        su.insert(0, "SP\x07\x01\xBE\xEF\x00", 7);
        if (rrip_ == iso::rrip_1991a)
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

        if (!rr_moved_.empty())
        {
            typedef directory_entries::iterator iter_type;

            directory_entries& roots = path_table_.at(0).at(0).entries;
            self::increment_link_count(
                const_cast<iso_directory_record&>(*roots.begin()).system_use);
            self::increment_link_count(
                const_cast<iso_directory_record&>(
                    *boost::next(roots.begin())).system_use);

            path_table_records& table1 = path_table_.at(1);
            for (std::size_t i = 0; i < table1.size(); ++i)
            {
                iso_directory_record& parent =
                    const_cast<iso_directory_record&>(
                        *boost::next(table1[i].entries.begin()));

                self::increment_link_count(parent.system_use);
            }

            boost::uint16_t rr_moved_index = find_rr_moved();
            directory_entries& moves = table1.at(rr_moved_index).entries;

            boost::uint32_t moved_links = 0u;
            for (iter_type i = moves.begin(), end = moves.end(); i != end; ++i)
            {
                if (i->is_directory())
                    ++moved_links;
            }

            self::set_link_count(
                const_cast<iso_directory_record&>(*moves.begin()).system_use,
                moved_links
            );

            path_table_records& table2 = path_table_.at(2);
            for (iter_type i = moves.begin(), end = moves.end(); i != end; ++i)
            {
                if (i->is_directory())
                {
                    const std::string& id = i->file_id;
                    if ((id.size() == 1u) && ((id[0]=='\0') || (id[0]=='\x01')))
                        continue;

                    boost::uint16_t index =
                        this->find_path(2u, rr_moved_index, i->file_id);

                    iso_directory_record& parent =
                        const_cast<iso_directory_record&>(
                            *boost::next(table2.at(index).entries.begin()));

                    self::set_link_count(parent.system_use, moved_links);
                }
            }

            iso_directory_record x;
            x.data_pos = 0;
            x.data_size = 0;
            x.flags = iso::file_flags::directory;
            x.file_id = rr_moved_.leaf();

            iso_directory_record& rr_moved =
                const_cast<iso_directory_record&>(*roots.find(x));
            self::set_link_count(rr_moved.system_use, moved_links);
        }

        boost::uint32_t pos = this->tell(sink);
        this->set_directory_sizes(pos);

        for (std::size_t level = 0; level < path_table_.size(); ++level)
        {
            path_table_records& table = path_table_[level];
            for (std::size_t i = 0; i < table.size(); ++i)
            {
                this->write_directory_records(
                    sink, table[i].full_path, table[i].entries);
            }
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
    unsigned iso_level_;
    path_cvt_table cvt_table_;
    boost::ptr_vector<path_table_records> path_table_;
    char block_[logical_sector_size];
    link_table cl_table_;
    link_table pl_table_;
    path rr_moved_;
    iso::rrip_type rrip_;

    static unsigned iso_directory_depth(const boost::filesystem::path& ph)
    {
        return static_cast<unsigned>(std::distance(ph.begin(), ph.end()));
    }

    static void set_nm_system_use_entry(iso_directory_record& rec)
    {
        const std::size_t size = rec.file_id.size();
        std::size_t pos = 0;
        while (pos < size)
        {
            std::size_t amt =
                std::min<std::size_t>(size-pos, 0xFFu-sys_entry_head_size);

            iso::system_use_entry_header head;
            head.signature[0] = 'N';
            head.signature[1] = 'M';
            head.entry_size =
                static_cast<boost::uint8_t>(sys_entry_head_size+1u+amt);
            head.version = 1u;

            char buf[sys_entry_head_size];
            hamigaki::binary_write(buf, head);
            rec.system_use.append(buf, sizeof(buf));

            if (pos + amt < size)
                rec.system_use.append(1, '\x01');
            else
                rec.system_use.append(1, '\x00');

            rec.system_use.append(rec.file_id, pos, amt);

            pos += amt;
        }
    }

    static void remove_nm_system_use_entry(std::string& su)
    {
        std::string result;
        const std::size_t size = su.size();
        std::size_t pos = 0;
        while (pos+sys_entry_head_size <= size)
        {
            iso::system_use_entry_header head;
            hamigaki::binary_read(su.c_str()+pos, head);
            if (std::memcmp(head.signature, "NM", 2) != 0)
                result.append(su, pos, head.entry_size);
            pos += head.entry_size;
        }
        su.swap(result);
    }

    static void fix_moved_directory_record(iso_directory_record& rec)
    {
        rec.data_size = 0;
        rec.flags &= ~iso::file_flags::directory;
        rec.system_use.append("CL\x0C\x01\0\0\0\0\0\0\0\0", 12);
    }

    template<class Data>
    static void append_system_use_entry(
        std::string& s, char c1, char c2, const Data& data)
    {
        static const std::size_t head_size =
            hamigaki::struct_size<iso::system_use_entry_header>::value;

        static const std::size_t data_size = hamigaki::struct_size<Data>::value;

        char buf[head_size+data_size];

        iso::system_use_entry_header head;
        head.signature[0] = c1;
        head.signature[1] = c2;
        head.entry_size = sizeof(buf);
        head.version = 1u;

        hamigaki::binary_write(buf, head);
        hamigaki::binary_write(buf+head_size, data);

        s.append(buf, sizeof(buf));
    }

    static void increment_link_count(std::string& su)
    {
        const std::size_t size = su.size();
        std::size_t pos = 0;
        while (pos+sys_entry_head_size <= size)
        {
            iso::system_use_entry_header head;
            hamigaki::binary_read(su.c_str()+pos, head);
            if (std::memcmp(head.signature, "PX", 2) == 0)
            {
                std::size_t off =
                    pos + sys_entry_head_size +
                    hamigaki::binary_offset<
                        iso::px_system_use_entry_data,
                        boost::uint32_t,
                        &iso::px_system_use_entry_data::links
                    >::value;

                const char* p = su.c_str() + off;
                boost::uint32_t links = hamigaki::decode_uint<little,4>(p) + 1u;

                char buf[8];
                hamigaki::encode_uint<little,4>(buf, links);
                hamigaki::encode_uint<big,4>(buf+4, links);
                su.replace(off, 8u, buf, 8u);
            }
            pos += head.entry_size;
        }
    }

    static void set_link_count(std::string& su, boost::uint32_t links)
    {
        const std::size_t size = su.size();
        std::size_t pos = 0;
        while (pos+sys_entry_head_size <= size)
        {
            iso::system_use_entry_header head;
            hamigaki::binary_read(su.c_str()+pos, head);
            if (std::memcmp(head.signature, "PX", 2) == 0)
            {
                std::size_t off =
                    pos + sys_entry_head_size +
                    hamigaki::binary_offset<
                        iso::px_system_use_entry_data,
                        boost::uint32_t,
                        &iso::px_system_use_entry_data::links
                    >::value;

                char buf[8];
                hamigaki::encode_uint<little,4>(buf, links);
                hamigaki::encode_uint<big,4>(buf+4, links);
                su.replace(off, 8u, buf, 8u);
            }
            pos += head.entry_size;
        }
    }

    template<class Sink>
    boost::uint32_t tell(Sink& sink)
    {
        iostreams::stream_offset offset = iostreams::tell_offset(sink);
        BOOST_ASSERT((offset & lbn_mask_) == 0);
        return static_cast<boost::uint32_t>(
            static_cast<boost::uint64_t>(offset) >> lbn_shift_);
    }

    boost::uint32_t round_to_block_size_impl(boost::uint32_t n)
    {
        if ((n & lbn_mask_) != 0u)
            return (n | lbn_mask_) + 1u;
        else
            return n;
    }

    std::size_t round_to_block_size(std::size_t n)
    {
        return round_to_block_size_impl(static_cast<boost::uint32_t>(n));
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

        return static_cast<boost::uint16_t>(i - table.begin());
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

        boost::uint32_t su_size = static_cast<boost::uint32_t>(su.size());
        if (su_size <= limit)
            return result_type(round_to_even(su_size), 0u);

        BOOST_ASSERT(limit >= ce_size);
        limit -= ce_size;

        boost::uint32_t pos = 0;
        while (pos+3 < su_size)
        {
            boost::uint32_t n = static_cast<unsigned char>(su[pos+2]);
            if (pos + n > limit)
            {
                return result_type(
                    round_to_even(pos+static_cast<boost::uint32_t>(ce_size)),
                    su_size - pos
                );
            }

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
            if (!rec.is_directory() && (rec.version != 0) && (iso_level_ != 4u))
            {
                id += ';';
                id += hamigaki::to_dec<char>(rec.version);
            }
            std::size_t id_size = id.size();
            std::size_t size = round_to_even(bin_size + id_size);

            boost::uint32_t sys_size;
            boost::uint32_t rest_size;
            boost::tie(sys_size, rest_size) =
                calc_system_use_size(
                    rec.system_use, static_cast<boost::uint32_t>(0xFFu-size)
                );
            size += sys_size;
            cont_size += rest_size;

            std::size_t offset = pos & lbn_mask_;
            if (offset + size > block_size)
                pos = round_to_block_size(pos);

            pos += size;
        }

        pos = round_to_block_size(pos);
        cont_size = round_to_block_size(cont_size);

        return std::make_pair(
            static_cast<boost::uint32_t>(pos),
            static_cast<boost::uint32_t>(cont_size)
        );
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

                pos += ((dir_size+cont_size) >> lbn_shift_);
            }
            prev_count = static_cast<boost::uint16_t>(table.size());
            base += prev_count;
        }
    }

    template<class Sink>
    void write_directory_records(
        Sink& sink, const path& ph, const directory_entries& entries)
    {
        const boost::uint32_t block_size = 1ul << lbn_shift_;
        const std::size_t bin_size = struct_size<iso::directory_record>::value;
        unsigned depth = self::iso_directory_depth(ph);

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
            if (!rec.is_directory() && (rec.version != 0) && (iso_level_ != 4u))
            {
                id += ';';
                id += hamigaki::to_dec<char>(rec.version);
            }
            std::size_t id_size = id.size();
            std::size_t size = round_to_even(bin_size + id_size);

            std::string su = rec.system_use;
            if (rec.is_directory() && (id.size() == 1) && (id[0] == '\x01'))
            {
                const path& full_path = ph;
                link_table::iterator pl = pl_table_.find(full_path);
                if (pl != pl_table_.end())
                {
                    boost::uint32_t link_pos = pl->second->data_pos;
                    su.resize(su.size()-8u);
                    char buf[8];
                    hamigaki::encode_uint<little,4>(buf, link_pos);
                    hamigaki::encode_uint<big,4>(buf+4, link_pos);
                    su.append(buf, sizeof(buf));
                }
            }
            else if (!rec.is_directory() && (depth == 7))
            {
                const path& full_path = ph/rec.file_id;
                link_table::iterator cl = cl_table_.find(full_path);
                if (cl != cl_table_.end())
                {
                    boost::uint32_t link_pos = cl->second->data_pos;
                    su.resize(su.size()-8u);
                    char buf[8];
                    hamigaki::encode_uint<little,4>(buf, link_pos);
                    hamigaki::encode_uint<big,4>(buf+4, link_pos);
                    su.append(buf, sizeof(buf));
                }
            }
            boost::uint32_t sys_size;
            boost::uint32_t rest_size;
            boost::tie(sys_size, rest_size) =
                calc_system_use_size(
                    su, static_cast<boost::uint32_t>(0xFFu-size)
                );
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

            std::size_t id_len = round_to_odd(id_size);
            std::memcpy(out, id.c_str(), id_len);
            out += id_len;

            if (rest_size != 0)
            {
                std::size_t su_size = su.size();
                std::size_t copy_size = su_size - rest_size;
                std::memcpy(out, su.c_str(), copy_size);
                out += copy_size;

                iso::system_use_entry_header head;
                head.signature[0] = 'C';
                head.signature[1] = 'E';
                head.entry_size = ce_size;
                head.version = 1u;

                hamigaki::binary_write(out, head);
                out += sys_entry_head_size;

                iso::ce_system_use_entry_data data;
                data.next_pos = static_cast<boost::uint32_t>(
                    cont_base + (cont_off & lbn_mask_)
                );
                data.next_offset = static_cast<boost::uint32_t>(cont_off);
                data.next_size = static_cast<boost::uint32_t>(rest_size);

                hamigaki::binary_write(out, data);
                out += ce_data_size;

                if (((copy_size + ce_size) & 1u) != 0u)
                    *(out++) = '\0';

                cont_area.append(su, copy_size, rest_size);
            }
            else
                std::memcpy(out, su.c_str(), sys_size);

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
                std::size_t rest =
                    static_cast<std::size_t>(1ul << lbn_shift_) - off;
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
                raw.dir_id_size =
                    static_cast<boost::uint8_t>(rec.dir_id.size());
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

        return static_cast<boost::uint32_t>(buffer.size());
    }

    boost::uint16_t create_rr_moved()
    {
        directory_entries& entries = path_table_.at(0).at(0).entries;

        iso_directory_record rec;
        rec.flags = iso::file_flags::directory;
        rec.file_id = "rr_moved";
        if (rrip_ == iso::rrip_1991a)
        {
            iso::old_px_system_use_entry_data px;
            px.file_mode = 040755;
            px.links = 1u;
            px.uid = 0;
            px.gid = 0;

            self::append_system_use_entry(rec.system_use, 'P','X', px);
        }
        else
        {
            iso::px_system_use_entry_data px;
            px.file_mode = 040755;
            px.links = 1u;
            px.uid = 0;
            px.gid = 0;
            px.serial_no = 0xFFFFFFFFul;

            self::append_system_use_entry(rec.system_use, 'P','X', px);
        }
        self::set_nm_system_use_entry(rec);
        rec.file_id = detail::make_iso_alt_id(iso_level_, entries, rec);
        entries.insert(rec);

        rr_moved_ = path(rec.file_id);

        std::auto_ptr<iso_path_table_record> ptr(new iso_path_table_record);
        ptr->dir_id = rec.file_id;
        ptr->data_pos = 0;
        ptr->parent_index = 0;
        ptr->full_path = rr_moved_;

        iso_directory_record cur_dir = rec;
        cur_dir.file_id.assign(1u, '\x00');
        self::remove_nm_system_use_entry(cur_dir.system_use);
        ptr->entries.insert(cur_dir);

        iso_directory_record par_dir = *entries.begin();
        par_dir.file_id.assign(1u, '\x01');
            self::remove_nm_system_use_entry(par_dir.system_use);
        ptr->entries.insert(par_dir);

        path_table_records& table = path_table_.at(1);
        path_table_records::iterator it =
            std::lower_bound(table.begin(), table.end(), *ptr);

        boost::uint16_t rr_moved_index =
            static_cast<boost::uint16_t>(it - table.begin());
        table.insert(it, ptr.release());

        path_table_records& child_table = path_table_.at(2);
        for (std::size_t i = 0; i < child_table.size(); ++i)
        {
            if (child_table[i].parent_index >= 2)
                ++(child_table[i].parent_index);
        }
        return rr_moved_index;
    }

    boost::uint16_t find_rr_moved() const
    {
        iso_path_table_record x;
        x.dir_id = rr_moved_.string();
        x.data_pos = 0;
        x.parent_index = 0;
        x.full_path = rr_moved_;

        const path_table_records& table = path_table_.at(1);
        path_table_records::const_iterator it =
            std::lower_bound(table.begin(), table.end(), x);

        return static_cast<boost::uint16_t>(it - table.begin());
    }

    void add_moved(
        const path& ph, iso_directory_record* parent,
        const std::vector<iso_directory_record>& recs)
    {
        path_table_records& table = path_table_.at(1);

        boost::uint16_t rr_moved_index;
        if (rr_moved_.empty())
            rr_moved_index = create_rr_moved();
        else
            rr_moved_index = find_rr_moved();

        directory_entries& entries = table.at(rr_moved_index).entries;

        const std::size_t size = recs.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            const iso_directory_record& rec = recs[i];
            if (rec.is_directory())
            {
                const std::string& new_id =
                    detail::make_iso_alt_id(iso_level_, entries, rec);

                iso_directory_record new_rec(rec);
                new_rec.file_id = new_id;
                new_rec.system_use.append("RE\x04\x01", 4);

                const path& old_path = ph/rec.file_id;
                const path& moved_path = rr_moved_/new_id;
                const path cvt_path = cvt_table_[old_path];
                pl_table_[moved_path] = parent;
                cvt_table_[old_path] = moved_path;

                directory_entries::iterator it = entries.insert(new_rec).first;
                cl_table_[cvt_path] = const_cast<iso_directory_record*>(&*it);
            }
        }
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ROCK_RIDGE_DIRECTORY_WRITER_HPP
