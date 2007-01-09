//  iso9660_file_source_impl.hpp: ISO 9660 file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/iso9660_id.hpp>
#include <hamigaki/archivers/detail/joliet_id.hpp>
#include <hamigaki/archivers/iso9660/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/noncopyable.hpp>
#include <cstring>
#include <memory>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

class iso9660_file_reader_base
{
public:
    virtual ~iso9660_file_reader_base(){};

    bool next_entry(iso9660::header& head)
    {
        return do_next_entry(head);
    }

    bool is_latest() const
    {
        return do_is_latest();
    }

private:
    virtual bool do_next_entry(iso9660::header& head) = 0;
    virtual bool do_is_latest() const = 0;
};

template<class Source, class FileId>
class iso9660_file_reader : public iso9660_file_reader_base
{
private:
    static const std::size_t logical_sector_size = 2048;

    struct path_table_record
    {
        FileId dir_id;
        boost::uint32_t data_pos;
        boost::uint16_t parent_index;
    };

    struct path_table_record_id_less
    {
        bool operator()(
            const path_table_record& lhs, const path_table_record& rhs) const
        {
            return lhs.dir_id.compare(rhs.dir_id) < 0;
        }
    };

    struct path_table_record_parent_less
    {
        bool operator()(
            const path_table_record& lhs, const path_table_record& rhs) const
        {
            return lhs.parent_index < rhs.parent_index;
        }
    };

    struct directory_record
    {
        FileId file_id;
        boost::uint32_t data_pos;
        boost::uint32_t data_size;
        iso9660::binary_date_time recorded_time;
        boost::uint8_t flags;

        directory_record()
            : data_pos(0), data_size(0), flags(0)
        {
        }

        explicit directory_record(const FileId& id)
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

public:
    iso9660_file_reader(Source& src, const iso9660::volume_descriptor& desc)
        : src_(src), volume_desc_(desc)
    {
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

        read_path_table();
        select_directory(1);
        dir_pos_ = 1;
    }

private:
    Source& src_;
    iso9660::volume_descriptor volume_desc_;
    std::vector<path_table_record> path_table_;
    std::vector<directory_record> dir_records_;
    boost::uint32_t lbn_shift_;
    boost::uint32_t lbn_mask_;
    boost::filesystem::path dir_path_;
    boost::uint16_t dir_index_;
    std::size_t dir_pos_;
    char block_[logical_sector_size];

    bool do_next_entry(iso9660::header& head) // virtual
    {
        if ((dir_pos_ != 1) && dir_records_[dir_pos_].is_directory())
        {
            select_directory(get_directory_index());
        }
        else
            ++dir_pos_;

        while (dir_pos_ == dir_records_.size())
        {
            boost::uint16_t old_dir_index = dir_index_;
            if (old_dir_index == 1)
                return false;

            select_directory(path_table_[old_dir_index].parent_index);
            select_file(path_table_[old_dir_index].dir_id);
            ++dir_pos_;
        }

        const directory_record& rec = dir_records_[dir_pos_];

        iso9660::header h;
        h.path = dir_path_ / rec.file_id.to_path();
        h.file_size = rec.data_size;
        h.recorded_time = rec.recorded_time;
        h.flags = rec.flags;

        seek_logical_block(rec.data_pos);
        head = h;
        return true;
    }

    bool do_is_latest() const // virtual
    {
        const FileId& cur = dir_records_[dir_pos_].file_id;

        for (std::size_t i = dir_pos_ + 1; i < dir_records_.size(); ++i)
        {
            const FileId& next = dir_records_[i].file_id;
            if (cur.filename_compare(next) != 0)
                return true;

            if (cur.version_compare(next) != 0)
                return false;
        }

        return true;
    }

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

    boost::filesystem::path get_full_path(boost::uint16_t num) const
    {
        if (num == 1)
            return boost::filesystem::path();
        else
        {
            const path_table_record& rec = path_table_.at(num);
            return get_full_path(rec.parent_index) / rec.dir_id.to_path();
        }
    }

    boost::uint16_t get_directory_index() const
    {
        typedef std::vector<path_table_record> table_type;
        typedef typename table_type::const_iterator iter_type;

        path_table_record x;
        x.dir_id = dir_records_[dir_pos_].file_id;
        x.parent_index = dir_index_;

        std::pair<iter_type,iter_type> rng =
            std::equal_range(
                path_table_.begin(),
                path_table_.end(),
                x,
                path_table_record_parent_less());

        if (rng.first == rng.second)
        {
            throw BOOST_IOSTREAMS_FAILURE(
                "ISO 9660 path table record not found");
        }

        iter_type iter =
            std::lower_bound(
                rng.first,
                rng.second,
                x,
                path_table_record_id_less());

        if ((iter == rng.second) ||
            (iter->dir_id.compare(x.dir_id) != 0) )
        {
            throw BOOST_IOSTREAMS_FAILURE(
                "ISO 9660 path table record not found");
        }

        return iter - path_table_.begin();
    }

    void read_path_table()
    {
#if defined(BOOST_LITTLE_ENDIAN)
        seek_logical_block(volume_desc_.l_path_table_pos);
#else
        seek_logical_block(volume_desc_.m_path_table_pos);
#endif

        boost::scoped_array<char>
            records(new char[volume_desc_.path_table_size]);
        iostreams::blocking_read(
            src_, records.get(), volume_desc_.path_table_size);

        std::size_t i = 0;
        std::vector<path_table_record> table(1);
        while (i < volume_desc_.path_table_size)
        {
            const std::size_t bin_size =
                struct_size<iso9660::path_table_record>::type::value;

            std::size_t next = i + bin_size;
            if (next > volume_desc_.path_table_size)
                break;

            iso9660::path_table_record raw;
            hamigaki::binary_read(&records[i], raw);

            next += raw.dir_id_size;
            if ((raw.dir_id_size & 1) != 0)
                ++next;
            if (next > volume_desc_.path_table_size)
                break;

            std::string id(&records[i+bin_size], raw.dir_id_size);
            path_table_record rec;
            rec.dir_id = FileId(id);
            rec.data_pos = raw.data_pos;
            rec.parent_index = raw.parent_dir_number;

            if (table.size() != 1)
            {
                const path_table_record& prev = table.back();
                if (prev.dir_id.compare(rec.dir_id) >= 0)
                {
                    throw BOOST_IOSTREAMS_FAILURE(
                        "invalid ISO 9660 order of path table records");
                }
            }

            table.push_back(rec);
            i = next;
        }

        if (i != volume_desc_.path_table_size)
            throw BOOST_IOSTREAMS_FAILURE("invalid ISO 9660 path table");

        path_table_.swap(table);
    }

    void select_directory(boost::uint16_t num)
    {
        std::vector<directory_record> records;

        const std::size_t bin_size =
            struct_size<iso9660::directory_record>::type::value;

        seek_logical_block(path_table_.at(num).data_pos);
        fill_buffer();

        iso9660::directory_record raw;
        hamigaki::binary_read(block_, raw);

        directory_record self(FileId('\x00'));
        self.data_pos = raw.data_pos;
        self.data_size = raw.data_size;
        self.recorded_time = raw.recorded_time;
        self.flags = raw.flags;
        records.push_back(self);

        hamigaki::binary_read(&block_[bin_size+1], raw);

        directory_record parent(FileId('\x01'));
        parent.data_pos = raw.data_pos;
        parent.data_size = raw.data_size;
        parent.recorded_time = raw.recorded_time;
        parent.flags = raw.flags;
        records.push_back(parent);

        boost::uint32_t pos = (bin_size+1)*2;
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

                next += raw.file_id_size;
                if ((raw.file_id_size & 1) == 0)
                    ++next;

                if (next > volume_desc_.logical_block_size)
                    break;

                std::string id(&block_[offset+bin_size], raw.file_id_size);
                directory_record rec((FileId(id)));
                rec.data_pos = raw.data_pos;
                rec.data_size = raw.data_size;
                rec.recorded_time = raw.recorded_time;
                rec.flags = raw.flags;

                const directory_record& prev = records.back();
                if (!(prev < rec))
                {
                    throw BOOST_IOSTREAMS_FAILURE(
                        "invalid ISO 9660 order of directory records");
                }

                records.push_back(rec);

                pos = next;
            }
            else
                pos += (volume_desc_.logical_block_size - offset);
        }

        if (pos != self.data_size)
            throw BOOST_IOSTREAMS_FAILURE("invalid ISO 9660 directory records");

        dir_path_ = get_full_path(num);
        dir_records_.swap(records);
        dir_index_ = num;
        dir_pos_ = 2;
    }

    void select_file(const FileId& file_id)
    {
        typedef std::vector<directory_record> records_type;
        typedef typename records_type::const_iterator iter_type;
        const std::vector<directory_record>& records = dir_records_;

        const directory_record x(file_id);
        iter_type iter = std::lower_bound(records.begin(), records.end(), x);
        if ((iter == records.end()) ||
            (iter->file_id.compare(file_id) != 0) )
        {
            throw BOOST_IOSTREAMS_FAILURE("ISO 9660 file ID not found");
        }

        dir_pos_ = iter - records.begin();
    }
};

template<class Source>
class basic_iso9660_file_source_impl : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    explicit basic_iso9660_file_source_impl(const Source& src)
        : src_(src), pos_(0)
    {
        iso9660::volume_descriptor volume_desc;

        boost::iostreams::seek(src_, logical_sector_size*16, BOOST_IOS::beg);
        iostreams::binary_read(src_, volume_desc);

        bool is_joliet = false;
        char block[logical_sector_size];
        while (true)
        {
            iostreams::blocking_read(src_, block, sizeof(block));
            if (block[0] == '\xFF')
                break;

            if (block[0] == '\x02')
            {
                iso9660::volume_descriptor desc;
                hamigaki::binary_read(block, desc);

                if ((std::memcmp(desc.escape_sequences, "%/@", 4) == 0) ||
                    (std::memcmp(desc.escape_sequences, "%/C", 4) == 0) ||
                    (std::memcmp(desc.escape_sequences, "%/E", 4) == 0) )
                {
                    is_joliet = true;
                    volume_desc = desc;
                    break;
                }
            }
        }

        if (is_joliet)
        {
            typedef iso9660_file_reader<Source,joliet_id> parser_type;
            parser_.reset(new parser_type(src_, volume_desc));
        }
        else
        {
            typedef iso9660_file_reader<Source,iso9660_id> parser_type;
            parser_.reset(new parser_type(src_, volume_desc));
        }
    }

    bool next_entry()
    {
        pos_ = 0;

        iso9660::header head;
        if (!parser_->next_entry(head))
            return false;

        header_ = head;
        return true;
    }

    iso9660::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if (!header_.is_regular())
            return -1;

        boost::uint64_t rest = header_.file_size - pos_;
        if (rest == 0)
            return -1;

        std::streamsize amt = auto_min(n, rest);
        iostreams::blocking_read(src_, s, amt);
        pos_ += amt;
        return amt;
    }

    bool is_latest() const
    {
        return parser_->is_latest();
    }

private:
    Source src_;
    iso9660::header header_;
    boost::uint64_t pos_;
    std::auto_ptr<iso9660_file_reader_base> parser_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SOURCE_IMPL_HPP
