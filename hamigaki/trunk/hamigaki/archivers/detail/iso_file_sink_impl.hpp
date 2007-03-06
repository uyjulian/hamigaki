//  iso_file_sink_impl.hpp: ISO file sink implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/iso_directory_record.hpp>
#include <hamigaki/archivers/detail/iso_directory_writer.hpp>
#include <hamigaki/archivers/detail/iso_logical_block_number.hpp>
#include <hamigaki/archivers/detail/joliet_directory_writer.hpp>
#include <hamigaki/archivers/detail/rock_ridge_directory_writer.hpp>
#include <hamigaki/archivers/iso/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <boost/assert.hpp>
#include <boost/next_prior.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

template<std::size_t Size>
inline void copy_iso9660_str(
    char (&buf)[Size], const std::string& s, char fill=' ')
{
    std::memset(buf, fill, sizeof(buf));
    s.copy(buf, sizeof(buf));
}

template<std::size_t Size>
inline void copy_iso9660_path(
    char (&buf)[Size], const boost::filesystem::path& ph)
{
    detail::copy_iso9660_str(buf, ph.string());
}

template<std::size_t Size>
inline void copy_joliet_str(
    char (&buf)[Size], const std::string& s)
{
    std::memset(buf, 0, sizeof(buf));
    detail::narrow_to_ucs2be(s).copy(buf, sizeof(buf));
}

template<std::size_t Size>
inline void copy_joliet_path(
    char (&buf)[Size], const boost::filesystem::path& ph)
{
    detail::copy_joliet_str(buf, ph.string());
}

template<class Sink>
class basic_iso_file_sink_impl : private boost::noncopyable
{
private:
    static const std::size_t logical_sector_size = 2048;

    typedef boost::filesystem::path path;
    typedef std::vector<iso_directory_record> directory_entries;

public:
    explicit basic_iso_file_sink_impl(
            const Sink& sink, const iso::volume_info& info=iso::volume_info() )
        : sink_(sink), volume_info_(info)
        , lbn_shift_(calc_lbn_shift(info.logical_block_size))
        , freeze_volume_descs_(false)
    {
        std::memset(block_, 0, sizeof(block_));
        for (int i = 0; i < 16; ++i)
            iostreams::blocking_write(sink_, block_);
    }

    void add_volume_desc(const iso::volume_desc& desc)
    {
        BOOST_ASSERT(!freeze_volume_descs_);

        volume_descs_.push_back(desc);
    }

    void create_entry(const iso::header& head)
    {
        if (!freeze_volume_descs_)
        {
            if (!has_primary_volume_desc())
                volume_descs_.push_back(iso::volume_desc());

            for (std::size_t i = 0; i < volume_descs_.size(); ++i)
                iostreams::blocking_write(sink_, block_);

            iso::volume_desc_set_terminator term;
            term.type = 255u;
            std::memcpy(term.std_id, volume_info_.std_id, 5);
            term.version = 1u;
            iostreams::binary_write(sink_, term);

            freeze_volume_descs_ = true;
        }

        if (head.file_size > 0xFFFFFFFFull)
            throw BOOST_IOSTREAMS_FAILURE("ISO 9660 file size too large");

        iso_directory_record rec;
        if (head.is_directory())
            rec.data_pos = 0u;
        else
            rec.data_pos = tell();
        rec.data_size = head.file_size;
        rec.recorded_time = head.recorded_time;
        rec.flags = head.flags;
        rec.file_id = head.path.leaf();
        if (!rec.is_directory())
        {
            if (head.version)
                rec.version = head.version;
            else
                rec.version = 1u;
        }
        rec.system_use = head.system_use;

        path parent(head.path.branch_path());
        dirs_[parent].push_back(rec);

        pos_ = 0;
        size_ = head.file_size;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        boost::uint32_t rest = size_ - pos_;
        std::streamsize amt = hamigaki::auto_min(n, rest);
        amt = boost::iostreams::write(sink_, s, amt);
        pos_ += amt;
        return amt;
    }

    void close()
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("ISO 9660 file size mismatch");

        boost::uint32_t block_size = volume_info_.logical_block_size;
        boost::uint32_t offset = size_ & (block_size-1);
        boost::uint32_t pad_size = block_size - offset;
        boost::iostreams::write(sink_, block_, pad_size);
    }

    void close_archive()
    {
        const std::size_t size = volume_descs_.size();

        for (std::size_t i = 0; i < size; ++i)
        {
            iso::volume_desc& desc = volume_descs_[i];
            if (desc.is_joliet())
                this->write_joliet_directory_descs(desc);
            else if (desc.is_rock_ridge())
                this->write_rock_ridge_directory_descs(desc);
            else
                this->write_directory_descs(desc);
        }

        volume_info_.volume_space_size = tell();

        boost::iostreams::seek(sink_, logical_sector_size*16, BOOST_IOS::beg);

        for (std::size_t i = 0; i < size; ++i)
        {
            iso::volume_desc& desc = volume_descs_[i];
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
    std::vector<iso::volume_desc> volume_descs_;
    std::map<path,directory_entries> dirs_;

    char block_[logical_sector_size];
    boost::uint32_t pos_;
    boost::uint32_t size_;

    boost::uint32_t tell()
    {
        iostreams::stream_offset offset = iostreams::tell_offset(sink_);
        BOOST_ASSERT((offset & (volume_info_.logical_block_size - 1u)) == 0);
        return static_cast<boost::uint64_t>(offset) >> lbn_shift_;
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

    void write_directory_descs(iso::volume_desc& desc)
    {
        typedef std::map<path,directory_entries>::const_iterator dirs_iter;

        iso_directory_writer writer(lbn_shift_);
        for (dirs_iter i = dirs_.begin(), end = dirs_.end(); i != end; ++i)
            writer.add(i->first, i->second);

        const iso_path_table_info& info = writer.write(sink_);
        desc.root_record = info.root_record;
        desc.path_table_size = info.path_table_size;
        desc.l_path_table_pos = info.l_path_table_pos;
        desc.m_path_table_pos = info.m_path_table_pos;
    }

    void write_rock_ridge_directory_descs(iso::volume_desc& desc)
    {
        typedef std::map<path,directory_entries>::const_iterator dirs_iter;

        rock_ridge_directory_writer writer(lbn_shift_, desc.rrip);
        for (dirs_iter i = dirs_.begin(), end = dirs_.end(); i != end; ++i)
            writer.add(i->first, i->second);

        const iso_path_table_info& info = writer.write(sink_);
        desc.root_record = info.root_record;
        desc.path_table_size = info.path_table_size;
        desc.l_path_table_pos = info.l_path_table_pos;
        desc.m_path_table_pos = info.m_path_table_pos;
    }

    void write_joliet_directory_descs(iso::volume_desc& desc)
    {
        typedef std::map<path,directory_entries>::const_iterator dirs_iter;

        joliet_directory_writer writer(lbn_shift_);
        for (dirs_iter i = dirs_.begin(), end = dirs_.end(); i != end; ++i)
            writer.add(i->first, i->second);

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
        raw.file_structure_version = volume_info_.file_structure_version;
    }

    void write_volume_desc(const iso::volume_desc& desc)
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
        std::memcpy(raw.application_use, desc.application_use, 512);

        iostreams::binary_write(sink_, raw);
    }

    void write_joliet_volume_desc(const iso::volume_desc& desc)
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
        std::memcpy(raw.application_use, desc.application_use, 512);

        iostreams::binary_write(sink_, raw);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SINK_IMPL_HPP
