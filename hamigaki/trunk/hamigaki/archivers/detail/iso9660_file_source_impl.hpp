//  iso9660_file_source_impl.hpp: ISO 9660 file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SOURCE_IMPL_HPP

#include <boost/config.hpp>

#include <hamigaki/archivers/detail/iso9660_id.hpp>
#include <hamigaki/archivers/detail/joliet_id.hpp>
#include <hamigaki/archivers/iso9660/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>
#include <vector>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    #define HAMIGAKI_ARCHIVERS_WINDOWS

    extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(
        unsigned, unsigned long, const wchar_t*, int,
        char*, int, const char*, int*);
#endif

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class basic_iso9660_file_source_impl : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;

    struct path_table_record
    {
        std::string dir_id;
        boost::uint32_t data_pos;
        boost::uint16_t parent_index;
    };

    class path_table_record_id_less
    {
    public:
        explicit path_table_record_id_less(bool joliet) : is_joliet_(joliet)
        {
        }

        bool operator()(
            const path_table_record& lhs, const path_table_record& rhs) const
        {
            if (is_joliet_)
                return detail::joliet_id_compare(lhs.dir_id, rhs.dir_id) < 0;
            else
                return detail::iso9660_id_compare(lhs.dir_id, rhs.dir_id) < 0;
        }

    private:
        bool is_joliet_;
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
        std::string file_id;
        boost::uint32_t data_pos;
        boost::uint32_t data_size;
        iso9660::binary_date_time recorded_time;
        boost::uint8_t flags;

        directory_record()
            : data_pos(0), data_size(0), flags(0)
        {
        }

        explicit directory_record(const std::string& id)
            : file_id(id), data_pos(0), data_size(0), flags(0)
        {
        }

        int iso9660_compare(const directory_record& rhs) const
        {
            int cmp = detail::iso9660_id_compare(file_id, rhs.file_id);

            if (cmp != 0)
                return cmp;

            bool lhs_assoc = (flags & iso9660::file_flags::associated) != 0;
            bool rhs_assoc = (rhs.flags & iso9660::file_flags::associated) != 0;

            if (lhs_assoc && !rhs_assoc)
                return -1;
            else if (!lhs_assoc && rhs_assoc)
                return 1;

            if (data_pos < rhs.data_pos)
                return -1;
            else if (data_pos > rhs.data_pos)
                return 1;
            else
                return 0;
        }

        int joliet_compare(const directory_record& rhs) const
        {
            int cmp = detail::joliet_id_compare(file_id, rhs.file_id);

            if (cmp != 0)
                return cmp;

            bool lhs_assoc = (flags & iso9660::file_flags::associated) != 0;
            bool rhs_assoc = (rhs.flags & iso9660::file_flags::associated) != 0;

            if (lhs_assoc && !rhs_assoc)
                return -1;
            else if (!lhs_assoc && rhs_assoc)
                return 1;

            if (data_pos < rhs.data_pos)
                return -1;
            else if (data_pos > rhs.data_pos)
                return 1;
            else
                return 0;
        }
    };

    class directory_record_less
    {
    public:
        explicit directory_record_less(bool joliet) : is_joliet_(joliet)
        {
        }

        bool operator()(
            const directory_record& lhs, const directory_record& rhs) const
        {
            if (is_joliet_)
                return lhs.joliet_compare(rhs) < 0;
            else
                return lhs.iso9660_compare(rhs) < 0;
        }

    private:
        bool is_joliet_;
    };

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    explicit basic_iso9660_file_source_impl(const Source& src)
        : src_(src), pos_(0), is_joliet_(false)
    {
        header_.file_size = 0;

        boost::iostreams::seek(src_, logical_sector_size*16, BOOST_IOS::beg);
        iostreams::binary_read(src_, volume_desc_);

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

        while (true)
        {
            iostreams::blocking_read(src_, block_, sizeof(block_));
            if (block_[0] == '\xFF')
                break;

            if (block_[0] == '\x02')
            {
                iso9660::volume_descriptor desc;
                hamigaki::binary_read(block_, desc);

                if ((std::memcmp(desc.escape_sequences, "%/@", 4) == 0) ||
                    (std::memcmp(desc.escape_sequences, "%/C", 4) == 0) ||
                    (std::memcmp(desc.escape_sequences, "%/E", 4) == 0) )
                {
                    is_joliet_ = true;
                    volume_desc_ = desc;
                }
            }
        }

        read_path_table();
        select_directory(1);
        dir_pos_ = 1;
    }

    bool next_entry()
    {
        pos_ = 0;

        if (header_.is_directory())
            select_directory(get_directory_index());
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

        iso9660::header head;
        head.path = dir_path_ / id_to_path(rec.file_id);
        head.file_size = rec.data_size;
        head.recorded_time = rec.recorded_time;
        head.flags = rec.flags;

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
        if (is_joliet_)
            return true;
        else
        {
            iso9660_id_accessor cur(dir_records_[dir_pos_].file_id);

            for (std::size_t i = dir_pos_ + 1; i < dir_records_.size(); ++i)
            {
                iso9660_id_accessor next(dir_records_[i].file_id);

                if (detail::iso9660_name_compare(cur, next) != 0)
                    return true;

                if (detail::iso9660_extension_compare(cur, next) != 0)
                    return true;

                if (detail::iso9660_version_compare(cur, next) != 0)
                    return false;
            }
        }

        return true;
    }

private:
    Source src_;
    iso9660::volume_descriptor volume_desc_;
    std::vector<path_table_record> path_table_;
    std::vector<directory_record> dir_records_;
    iso9660::header header_;
    boost::uint32_t lbn_shift_;
    boost::uint32_t lbn_mask_;
    boost::uint64_t pos_;
    boost::filesystem::path dir_path_;
    boost::uint16_t dir_index_;
    std::size_t dir_pos_;
    bool is_joliet_;
    char block_[logical_sector_size];

    int id_compare(const std::string& lhs, const std::string& rhs) const
    {
        if (is_joliet_)
            return detail::joliet_id_compare(lhs, rhs);
        else
            return detail::iso9660_id_compare(lhs, rhs);
    }

    int dir_compare(
        const directory_record& lhs, const directory_record& rhs) const
    {
        if (is_joliet_)
            return lhs.joliet_compare(rhs);
        else
            return lhs.iso9660_compare(rhs);
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

    boost::filesystem::path id_to_path(const std::string& s) const
    {
        using namespace boost::filesystem;

        if (is_joliet_)
        {
            std::size_t src_size = s.size()/2;
            boost::scoped_array<wchar_t> src(new wchar_t[src_size + 1]);
            for (std::size_t i = 0; i < s.size(); i += 2)
            {
                unsigned char uc1 = static_cast<unsigned char>(s[i]);
                unsigned char uc2 = static_cast<unsigned char>(s[i+1]);
                boost::uint16_t n =
                    (static_cast<boost::uint16_t>(uc1) << 8) |
                    (static_cast<boost::uint16_t>(uc2)     ) ;

                src[i/2] = static_cast<wchar_t>(n);
            }
            src[src_size] = 0;

#if defined(HAMIGAKI_ARCHIVERS_WINDOWS)
            std::size_t size = ::WideCharToMultiByte(
                0, 0, src.get(), src_size, 0, 0, 0, 0);
            boost::scoped_array<char> buf(new char[size+1]);
            ::WideCharToMultiByte(
                0, 0, src.get(), src_size, buf.get(), size+1, 0, 0);
            return path(std::string(buf.get(), size), no_check);
#else
            std::size_t size = std::wcstombs(0, src.get(), 0);
            boost::scoped_array<char> buf(new char[size+1]);
            std::wcstombs(buf.get(), src.get(), size+1);
            return path(std::string(buf.get(), size), no_check);
#endif
        }
        else
            return path(s, no_check);
    }

    boost::filesystem::path get_full_path(boost::uint16_t num) const
    {
        if (num == 1)
            return boost::filesystem::path();
        else
        {
            using namespace boost::filesystem;
            const path_table_record& rec = path_table_.at(num);
            return get_full_path(rec.parent_index) / id_to_path(rec.dir_id);
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
                path_table_record_id_less(is_joliet_));

        if ((iter == rng.second) ||
            (this->id_compare(iter->dir_id, x.dir_id) != 0) )
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

            path_table_record rec;
            rec.dir_id.assign(&records[i+bin_size], raw.dir_id_size);
            rec.data_pos = raw.data_pos;
            rec.parent_index = raw.parent_dir_number;

            if (table.size() != 1)
            {
                const path_table_record& prev = table.back();
                if (this->id_compare(prev.dir_id, rec.dir_id) >= 0)
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

        directory_record self;
        self.file_id = '\x00';
        self.data_pos = raw.data_pos;
        self.data_size = raw.data_size;
        self.recorded_time = raw.recorded_time;
        self.flags = raw.flags;
        records.push_back(self);

        hamigaki::binary_read(&block_[bin_size+1], raw);

        directory_record parent;
        parent.file_id = '\x01';
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

                directory_record rec;
                rec.file_id.assign(&block_[offset+bin_size], raw.file_id_size);
                rec.data_pos = raw.data_pos;
                rec.data_size = raw.data_size;
                rec.recorded_time = raw.recorded_time;
                rec.flags = raw.flags;

                const directory_record& prev = records.back();
                if (this->dir_compare(prev, rec) >= 0)
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

    void select_file(const std::string& file_id)
    {
        typedef std::vector<directory_record> records_type;
        typedef typename records_type::const_iterator iter_type;
        const std::vector<directory_record>& records = dir_records_;

        const directory_record x(file_id);
        iter_type iter =
            std::lower_bound(
                records.begin(), records.end(),
                x, directory_record_less(is_joliet_)
            );

        if ((iter == records.end()) ||
            (this->id_compare(iter->file_id, file_id) != 0) )
        {
            throw BOOST_IOSTREAMS_FAILURE("ISO 9660 file ID not found");
        }

        dir_pos_ = iter - records.begin();
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO9660_FILE_SOURCE_IMPL_HPP
