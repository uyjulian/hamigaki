// raw_iso_file_sink_impl.hpp: raw ISO file sink implementation

// Copyright Takeshi Mouri 2007-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_ISO_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_ISO_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/iso_directory_record.hpp>
#include <hamigaki/archivers/detail/iso_directory_writer.hpp>
#include <hamigaki/archivers/detail/iso_logical_block_number.hpp>
#include <hamigaki/archivers/detail/iso_string.hpp>
#include <hamigaki/archivers/detail/joliet_directory_writer.hpp>
#include <hamigaki/archivers/detail/rock_ridge_directory_writer.hpp>
#include <hamigaki/archivers/detail/sl_components_composer.hpp>
#include <hamigaki/archivers/iso/headers.hpp>
#include <hamigaki/archivers/iso/tf_flags.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/assert.hpp>
#include <boost/next_prior.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
template<std::size_t Size>
inline void copy_iso9660_str(
    char (&buf)[Size], const std::wstring& s, char fill=' ')
{
    std::memset(buf, fill, sizeof(buf));
    charset::to_code_page(s,0,"_").copy(buf, sizeof(buf));
}

template<std::size_t Size>
inline void copy_joliet_str(
    char (&buf)[Size], const std::wstring& s)
{
    std::memset(buf, 0, sizeof(buf));
    charset::to_utf16be(s).copy(buf, sizeof(buf));
}
#endif // !defined(BOOST_FILESYSTEM_NARROW_ONLY)

template<std::size_t Size>
inline void copy_iso9660_str(
    char (&buf)[Size], const std::string& s, char fill=' ')
{
    std::memset(buf, fill, sizeof(buf));
    s.copy(buf, sizeof(buf));
}

template<std::size_t Size, class Path>
inline void copy_iso9660_path(char (&buf)[Size], const Path& ph)
{
    detail::copy_iso9660_str(buf, ph.string());
}

template<std::size_t Size>
inline void copy_joliet_str(
    char (&buf)[Size], const std::string& s)
{
    std::memset(buf, 0, sizeof(buf));
    charset::to_utf16be(charset::from_code_page(s,0)).copy(buf, sizeof(buf));
}

template<std::size_t Size, class Path>
inline void copy_joliet_path(char (&buf)[Size], const Path& ph)
{
    detail::copy_joliet_str(buf, ph.string());
}

template<class Path>
inline boost::filesystem::path to_iso9660_path(const Path& ph)
{
    return detail::to_iso9660_string(ph.string());
}

template<class Sink, class Path>
class basic_raw_iso_file_sink_impl : private boost::noncopyable
{
private:
    typedef basic_raw_iso_file_sink_impl<Sink,Path> self;

    static const std::size_t logical_sector_size = 2048;

public:
    typedef Path path_type;
    typedef iso::basic_header<Path> header_type;
    typedef iso::basic_volume_desc<Path> volume_desc;
    typedef std::vector<header_type> directory_entries;

    explicit basic_raw_iso_file_sink_impl(
            const Sink& sink, const iso::volume_info& info=iso::volume_info() )
        : sink_(sink), volume_info_(info)
        , lbn_shift_(calc_lbn_shift(info.logical_block_size))
        , freeze_volume_descs_(false)
    {
        std::memset(block_, 0, sizeof(block_));
        for (int i = 0; i < 16; ++i)
            iostreams::blocking_write(sink_, block_);

        dirs_[Path()];
    }

    void add_volume_desc(const volume_desc& desc)
    {
        BOOST_ASSERT(!freeze_volume_descs_);
        volume_descs_.push_back(desc);
    }

    void create_entry(const header_type& head)
    {
        if (!freeze_volume_descs_)
            write_volume_descs_set_terminator();

        if (head.file_size > 0xFFFFFFFFull)
            throw BOOST_IOSTREAMS_FAILURE("ISO 9660 file size too large");

        header_type h = head;
        h.path = head.path.leaf();

        if (h.is_directory())
            h.data_pos = 0u;
        else
            h.data_pos = tell();

        if (!h.is_directory() && (h.version == 0))
            h.version = 1u;

        Path parent(head.path.branch_path());
        dirs_[parent].push_back(h);
        if (h.is_directory())
            dirs_[head.path];

        pos_ = 0;
        size_ = static_cast<boost::uint32_t>(h.file_size);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        boost::uint32_t rest = size_ - pos_;
        std::streamsize amt = hamigaki::auto_min(n, rest);
        amt = boost::iostreams::write(sink_, s, amt);
        pos_ += static_cast<boost::uint32_t>(amt);
        return amt;
    }

    void close()
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("ISO 9660 file size mismatch");

        boost::uint32_t block_size = volume_info_.logical_block_size;
        boost::uint32_t offset = size_ & (block_size-1);
        if (offset != 0)
        {
            boost::uint32_t pad_size = block_size - offset;
            boost::iostreams::write(sink_, block_, pad_size);
        }
    }

    void close_archive()
    {
        if (!freeze_volume_descs_)
            write_volume_descs_set_terminator();

        const std::size_t size = volume_descs_.size();

        header_type root;
        root.flags = iso::file_flags::directory;
        iso::posix::file_attributes attr;
        attr.permissions = 040755;
        attr.links = static_cast<boost::uint32_t>(
            2u + this->count_directory(Path())
        );
        attr.uid = 0u;
        attr.gid = 0u;
        attr.serial_no = 0;
        root.attributes = attr;

        count_directories();

        for (std::size_t i = 0; i < size; ++i)
        {
            volume_desc& desc = volume_descs_[i];
            if (desc.is_joliet())
                this->write_joliet_directory_descs(desc, root);
            else if (desc.is_rock_ridge())
                this->write_rock_ridge_directory_descs(desc, root);
            else
                this->write_directory_descs(desc, root);
        }

        volume_info_.volume_space_size = tell();

        boost::iostreams::seek(sink_, logical_sector_size*16, BOOST_IOS::beg);

        for (std::size_t i = 0; i < size; ++i)
        {
            volume_desc& desc = volume_descs_[i];
            if (desc.is_joliet())
                this->write_joliet_volume_desc(desc);
            else
                this->write_volume_desc(desc);
        }

        boost::iostreams::seek(sink_, 0, BOOST_IOS::end);
        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    iso::volume_info volume_info_;
    boost::uint32_t lbn_shift_;
    bool freeze_volume_descs_;
    std::vector<volume_desc> volume_descs_;
    std::map<Path,directory_entries> dirs_;

    char block_[logical_sector_size];
    boost::uint32_t pos_;
    boost::uint32_t size_;

    static iso_directory_record make_dir_record(const header_type& head)
    {
        iso_directory_record rec;
        rec.data_pos = head.data_pos;
        rec.data_size = static_cast<boost::uint32_t>(head.file_size);
        rec.recorded_time = head.recorded_time;
        rec.flags = head.flags;
        rec.file_id = detail::to_iso9660_string(head.path.leaf());
        rec.version = head.version;
        rec.system_use = detail::to_iso9660_string(head.system_use);
        return rec;
    }

    static iso_directory_record make_joliet_dir_record(const header_type& head)
    {
        iso_directory_record rec;
        rec.data_pos = head.data_pos;
        rec.data_size = static_cast<boost::uint32_t>(head.file_size);
        rec.recorded_time = head.recorded_time;
        rec.flags = head.flags;
        rec.file_id = detail::to_joliet_string(head.path.leaf());
        rec.version = head.version;
        rec.system_use = detail::to_joliet_string(head.system_use);
        return rec;
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

    static iso_directory_record& make_rrip_dir_record(
        iso_directory_record& rec, const header_type& head, iso::rrip_type rrip)
    {
        if (head.attributes)
        {
            const iso::posix::file_attributes& attr = *head.attributes;
            if (rrip == iso::rrip_1991a)
            {
                iso::old_px_system_use_entry_data data;
                data.file_mode = attr.permissions;
                data.links = attr.links;
                data.uid = attr.uid;
                data.gid = attr.gid;

                self::append_system_use_entry(rec.system_use, 'P', 'X', data);
            }
            else
            {
                iso::px_system_use_entry_data data;
                data.file_mode = attr.permissions;
                data.links = attr.links;
                data.uid = attr.uid;
                data.gid = attr.gid;
                data.serial_no = attr.serial_no;

                self::append_system_use_entry(rec.system_use, 'P', 'X', data);
            }
        }

        using iso::tf_flags;
        typedef iso::binary_date_time time_type;
        std::string tf_buf;
        boost::uint8_t flags = 0;

        if (!head.creation_time.empty())
        {
            flags |= tf_flags::creation;
            hamigaki::binary_write(
                tf_buf, time_type::from_date_time(head.creation_time));
        }

        if (!head.last_write_time.empty())
        {
            flags |= tf_flags::modify;
            hamigaki::binary_write(
                tf_buf, time_type::from_date_time(head.last_write_time));
        }

        if (!head.last_access_time.empty())
        {
            flags |= tf_flags::access;
            hamigaki::binary_write(
                tf_buf, time_type::from_date_time(head.last_access_time));
        }

        if (!head.last_change_time.empty())
        {
            flags |= tf_flags::attributes;
            hamigaki::binary_write(
                tf_buf, time_type::from_date_time(head.last_change_time));
        }

        if (!head.last_backup_time.empty())
        {
            flags |= tf_flags::backup;
            hamigaki::binary_write(
                tf_buf, time_type::from_date_time(head.last_backup_time));
        }

        if (!head.expiration_time.empty())
        {
            flags |= tf_flags::expiration;
            hamigaki::binary_write(
                tf_buf, time_type::from_date_time(head.expiration_time));
        }

        if (!head.effective_time.empty())
        {
            flags |= tf_flags::effective;
            hamigaki::binary_write(
                tf_buf, time_type::from_date_time(head.effective_time));
        }

        if (flags != 0)
        {
            static const std::size_t head_size =
                hamigaki::struct_size<iso::system_use_entry_header>::value;

            char buf[head_size+1];

            iso::system_use_entry_header head;
            head.signature[0] = 'T';
            head.signature[1] = 'F';
            head.entry_size =
                static_cast<boost::uint8_t>(sizeof(buf) + tf_buf.size());
            head.version = 1u;

            hamigaki::binary_write(buf, head);
            buf[head_size] = static_cast<char>(flags);

            rec.system_use.append(buf, sizeof(buf));
            rec.system_use.append(tf_buf);
        }

        if (head.device_number)
        {
            const filesystem::device_number& dev = *head.device_number;
            iso::pn_system_use_entry_data data;
            data.device_number_high = static_cast<boost::uint32_t>(dev.major);
            data.device_number_low = static_cast<boost::uint32_t>(dev.minor);

            self::append_system_use_entry(rec.system_use, 'P', 'N', data);
        }

        if (!head.link_path.empty())
        {
            typedef typename Path::const_iterator iter_type;

            sl_components_composer composer;
            iter_type end = head.link_path.end();
            for (iter_type i = head.link_path.begin(); i != end; ++i)
                composer.compose(detail::to_iso9660_string(*i));
            rec.system_use.append(composer.entry_string());
        }

        return rec;
    }

    static std::size_t count_directory(const directory_entries& entries)
    {
        std::size_t n = 0;
        for (std::size_t i = 0, size = entries.size(); i < size; ++i)
        {
            if (entries[i].is_directory())
                ++n;
        }
        return n;
    }

    std::size_t count_directory(const Path& ph) const
    {
        typedef typename std::map<
            Path, directory_entries
        >::const_iterator dirs_iter;

        dirs_iter it = dirs_.find(ph);
        if (it != dirs_.end())
            return self::count_directory(it->second);
        else
            return 0u;
    }

    void count_directories(const Path& ph, directory_entries& entries)
    {
        for (std::size_t i = 0, size = entries.size(); i < size; ++i)
        {
            header_type& head = entries[i];
            if (head.attributes)
            {
                if (head.is_directory())
                {
                    head.attributes->links = static_cast<boost::uint32_t>(
                        2u + this->count_directory(ph/head.path)
                    );
                }
                else
                    head.attributes->links = 1u;
            }
        }
    }

    void count_directories()
    {
        typedef typename std::map<
            Path, directory_entries
        >::iterator dirs_iter;

        for (dirs_iter i = dirs_.begin(), end = dirs_.end(); i != end; ++i)
            count_directories(i->first, i->second);
    }

    void make_dir_records(
        std::vector<iso_directory_record>& dst,
        const std::vector<header_type>& src, iso::rrip_type rrip)
    {
        std::vector<iso_directory_record> result;
        std::size_t size = src.size();
        result.reserve(size);

        for (std::size_t i = 0; i < size; ++i)
        {
            const header_type& head = src[i];
            iso_directory_record rec = self::make_dir_record(head);
            if (rrip != iso::rrip_none)
                self::make_rrip_dir_record(rec, head, rrip);
            result.push_back(rec);
        }

        dst.swap(result);
    }

    void make_joliet_dir_records(
        std::vector<iso_directory_record>& dst,
        const std::vector<header_type>& src, iso::rrip_type rrip)
    {
        std::vector<iso_directory_record> result;
        std::size_t size = src.size();
        result.reserve(size);

        for (std::size_t i = 0; i < size; ++i)
        {
            const header_type& head = src[i];
            iso_directory_record rec = self::make_joliet_dir_record(head);
            if (rrip != iso::rrip_none)
                self::make_rrip_dir_record(rec, head, rrip);
            result.push_back(rec);
        }

        dst.swap(result);
    }

    boost::uint32_t tell()
    {
        iostreams::stream_offset offset = iostreams::tell_offset(sink_);
        BOOST_ASSERT((offset & (volume_info_.logical_block_size - 1u)) == 0);
        return static_cast<boost::uint32_t>(
            static_cast<boost::uint64_t>(offset) >> lbn_shift_
        );
    }

    bool has_primary_volume_desc() const
    {
        for (std::size_t i = 0, size = volume_descs_.size(); i < size; ++i)
        {
            if (volume_descs_[i].type == 1u)
                return true;
        }
        return false;
    }

    void write_directory_descs(volume_desc& desc, const header_type& root)
    {
        typedef typename std::map<
            Path, directory_entries
        >::const_iterator dirs_iter;

        iso_directory_writer writer(
            lbn_shift_, self::make_dir_record(root), desc.level);
        for (dirs_iter i = dirs_.begin(), end = dirs_.end(); i != end; ++i)
        {
            std::vector<iso_directory_record> tmp;
            self::make_dir_records(tmp,i->second, iso::rrip_none);
            writer.add(to_iso9660_path(i->first), tmp);
        }

        const iso_path_table_info& info = writer.write(sink_);
        desc.root_record = info.root_record;
        desc.path_table_size = info.path_table_size;
        desc.l_path_table_pos = info.l_path_table_pos;
        desc.m_path_table_pos = info.m_path_table_pos;
    }

    void write_rock_ridge_directory_descs(
        volume_desc& desc, const header_type& root)
    {
        typedef typename std::map<
            Path, directory_entries
        >::const_iterator dirs_iter;

        iso_directory_record root_rec = self::make_dir_record(root);
        if (desc.rrip != iso::rrip_none)
            self::make_rrip_dir_record(root_rec, root, desc.rrip);
        rock_ridge_directory_writer writer(
            lbn_shift_, root_rec, desc.level, desc.rrip);
        for (dirs_iter i = dirs_.begin(), end = dirs_.end(); i != end; ++i)
        {
            std::vector<iso_directory_record> tmp;
            self::make_dir_records(tmp,i->second, desc.rrip);
            writer.add(to_iso9660_path(i->first), tmp);
        }

        const iso_path_table_info& info = writer.write(sink_);
        desc.root_record = info.root_record;
        desc.path_table_size = info.path_table_size;
        desc.l_path_table_pos = info.l_path_table_pos;
        desc.m_path_table_pos = info.m_path_table_pos;
    }

    void write_joliet_directory_descs(
        volume_desc& desc, const header_type& root)
    {
        typedef typename std::map<
            Path, directory_entries
        >::const_iterator dirs_iter;

        iso_directory_record root_rec = self::make_joliet_dir_record(root);
        if (desc.rrip != iso::rrip_none)
            self::make_rrip_dir_record(root_rec, root, desc.rrip);
        joliet_directory_writer<Path> writer(lbn_shift_, root_rec);
        for (dirs_iter i = dirs_.begin(), end = dirs_.end(); i != end; ++i)
        {
            std::vector<iso_directory_record> tmp;
            self::make_joliet_dir_records(tmp,i->second, desc.rrip);
            writer.add(i->first, tmp);
        }

        const iso_path_table_info& info = writer.write(sink_);
        desc.root_record = info.root_record;
        desc.path_table_size = info.path_table_size;
        desc.l_path_table_pos = info.l_path_table_pos;
        desc.m_path_table_pos = info.m_path_table_pos;
    }

    void set_volume_info(iso::volume_descriptor& raw)
    {
        std::memcpy(raw.std_id, volume_info_.std_id, 5);
        raw.volume_space_size = volume_info_.volume_space_size;
        raw.volume_set_size = volume_info_.volume_set_size;
        raw.volume_seq_number = volume_info_.volume_seq_number;
        raw.logical_block_size = volume_info_.logical_block_size;
        raw.creation_time = volume_info_.creation_time;
        raw.modification_time = volume_info_.modification_time;
        raw.expiration_time = volume_info_.expiration_time;
        raw.effective_time = volume_info_.effective_time;
    }

    void write_volume_desc(const volume_desc& desc)
    {
        iso::volume_descriptor raw;
        this->set_volume_info(raw);

        raw.type = desc.type;
        raw.version = desc.version;
        raw.flags = desc.flags;
        detail::copy_iso9660_str(raw.system_id, desc.system_id);
        detail::copy_iso9660_str(raw.volume_id, desc.volume_id);
        detail::copy_iso9660_str(
            raw.escape_sequences, desc.escape_sequences, '\0');
        raw.path_table_size = desc.path_table_size;
        raw.l_path_table_pos = desc.l_path_table_pos;
        raw.l_path_table_pos2 = desc.l_path_table_pos2;
        raw.m_path_table_pos = desc.m_path_table_pos;
        raw.m_path_table_pos2 = desc.m_path_table_pos2;
        raw.root_record = desc.root_record;
        raw.root_file_id  = '\0';
        detail::copy_iso9660_str(raw.volume_set_id, desc.volume_set_id);
        detail::copy_iso9660_str(raw.publisher_id, desc.publisher_id);
        detail::copy_iso9660_str(raw.data_preparer_id, desc.data_preparer_id);
        detail::copy_iso9660_str(raw.application_id, desc.application_id);
        detail::copy_iso9660_path(
            raw.copyright_file_id, desc.copyright_file_id);
        detail::copy_iso9660_path(raw.abstract_file_id, desc.abstract_file_id);
        detail::copy_iso9660_path(
            raw.bibliographic_file_id, desc.bibliographic_file_id);
        raw.file_structure_version = desc.file_structure_version;
        std::memcpy(raw.application_use, desc.application_use, 512);

        iostreams::binary_write(sink_, raw);
    }

    void write_joliet_volume_desc(const volume_desc& desc)
    {
        iso::volume_descriptor raw;
        this->set_volume_info(raw);

        raw.type = desc.type;
        raw.version = desc.version;
        raw.flags = desc.flags;
        detail::copy_joliet_str(raw.system_id, desc.system_id);
        detail::copy_joliet_str(raw.volume_id, desc.volume_id);
        detail::copy_iso9660_str(
            raw.escape_sequences, desc.escape_sequences, '\0');
        raw.path_table_size = desc.path_table_size;
        raw.l_path_table_pos = desc.l_path_table_pos;
        raw.l_path_table_pos2 = desc.l_path_table_pos2;
        raw.m_path_table_pos = desc.m_path_table_pos;
        raw.m_path_table_pos2 = desc.m_path_table_pos2;
        raw.root_record = desc.root_record;
        raw.root_file_id  = '\0';
        detail::copy_joliet_str(raw.volume_set_id, desc.volume_set_id);
        detail::copy_joliet_str(raw.publisher_id, desc.publisher_id);
        detail::copy_joliet_str(raw.data_preparer_id, desc.data_preparer_id);
        detail::copy_joliet_str(raw.application_id, desc.application_id);
        detail::copy_joliet_path(
            raw.copyright_file_id, desc.copyright_file_id);
        detail::copy_joliet_path(raw.abstract_file_id, desc.abstract_file_id);
        detail::copy_joliet_path(
            raw.bibliographic_file_id, desc.bibliographic_file_id);
        raw.file_structure_version = desc.file_structure_version;
        std::memcpy(raw.application_use, desc.application_use, 512);

        iostreams::binary_write(sink_, raw);
    }

    void write_volume_descs_set_terminator()
    {
        if (!has_primary_volume_desc())
            volume_descs_.push_back(volume_desc());

        for (std::size_t i = 0; i < volume_descs_.size(); ++i)
            iostreams::blocking_write(sink_, block_);

        iso::volume_desc_set_terminator term;
        term.type = 255u;
        std::memcpy(term.std_id, volume_info_.std_id, 5);
        term.version = 1u;
        iostreams::binary_write(sink_, term);

        freeze_volume_descs_ = true;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_ISO_FILE_SINK_IMPL_HPP
