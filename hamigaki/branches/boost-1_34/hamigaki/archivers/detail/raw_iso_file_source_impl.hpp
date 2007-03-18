//  raw_iso_file_source_impl.hpp: raw ISO file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_ISO_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_ISO_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/iso_data_reader.hpp>
#include <hamigaki/archivers/detail/iso_file_reader.hpp>
#include <hamigaki/archivers/detail/iso9660_directory_parser.hpp>
#include <hamigaki/archivers/detail/joliet_directory_parser.hpp>
#include <hamigaki/archivers/detail/rock_ridge_check.hpp>
#include <hamigaki/archivers/detail/rock_ridge_directory_parser.hpp>
#include <boost/lambda/lambda.hpp>
#include <memory>

namespace hamigaki { namespace archivers { namespace detail {

template<std::size_t Size>
inline std::string make_iso_string(const char (&data)[Size], char fill)
{
    typedef std::reverse_iterator<const char*> iter_type;

    iter_type rbeg(data+Size);
    iter_type rend(data);

    using boost::lambda::_1;
    iter_type i = std::find_if(rbeg, rend, _1 != fill);
    if (i != rend)
        return std::string(data, i.base());
    else
        return std::string(data, Size);
}

template<std::size_t Size>
inline boost::filesystem::path make_iso_file_id(const char (&data)[Size])
{
    using namespace boost::filesystem;
    return path(detail::make_iso_string(data, ' '), no_check);
}

template<std::size_t Size>
inline std::string make_joliet_string(const char (&data)[Size])
{
    std::size_t src_size = Size/2;
    boost::scoped_array<wchar_t> src(new wchar_t[src_size + 1]);
    detail::ucs2be_to_wide(src.get(), data, Size);
    src[src_size] = 0;

    return detail::wide_to_narrow(src.get());
}

template<std::size_t Size>
inline boost::filesystem::path make_joliet_file_id(const char (&data)[Size])
{
    using namespace boost::filesystem;
    return path(detail::make_joliet_string(data), no_check);
}

template<class Source>
class basic_raw_iso_file_source_impl : private boost::noncopyable
{
private:
    typedef basic_raw_iso_file_source_impl<Source> self;

    static const std::size_t logical_sector_size = 2048;

public:
    explicit basic_raw_iso_file_source_impl(const Source& src) : src_(src)
    {
        read_volume_descriptor();
        rock_ridge_check();
    }

    bool next_entry()
    {
        if (!reader_.get())
            select_volume_desc(0);

        return reader_->next_entry();
    }

    iso::header header() const
    {
        return reader_->header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return reader_->read(s, n);
    }

    const std::vector<iso::volume_desc>& volume_descs() const
    {
        return volume_descs_;
    }

    void select_volume_desc(std::size_t index, bool use_rrip=true)
    {
        const iso::volume_desc& desc = volume_descs_.at(index);
        std::auto_ptr<iso_directory_parser> parser;
        if (desc.is_joliet())
            parser.reset(new joliet_directory_parser);
        else
            parser.reset(new iso9660_directory_parser);

        if (use_rrip && desc.is_rock_ridge())
        {
            std::auto_ptr<iso_directory_parser> tmp(
                new rock_ridge_directory_parser(parser, desc.rrip)
            );
            parser = tmp;
        }

        reader_.reset(
            new iso_file_reader<Source>(src_, parser, volume_info_, desc)
        );
    }

    boost::uint64_t total_size() const
    {
        return reader_->total_size();
    }

private:
    Source src_;
    iso::volume_info volume_info_;
    std::vector<iso::volume_desc> volume_descs_;
    std::auto_ptr<iso_file_reader<Source> > reader_;

    static iso::volume_desc make_volume_desc(const iso::volume_descriptor& desc)
    {
        iso::volume_desc tmp;
        tmp.rrip = iso::rrip_none;
        tmp.type = desc.type;
        tmp.version = desc.version;
        tmp.flags = desc.flags;
        tmp.system_id = detail::make_iso_string(desc.system_id, ' ');
        tmp.volume_id = detail::make_iso_string(desc.volume_id, ' ');
        tmp.escape_sequences =
            detail::make_iso_string(desc.escape_sequences, '\0');
        tmp.path_table_size = desc.path_table_size;
        tmp.l_path_table_pos = desc.l_path_table_pos;
        tmp.l_path_table_pos2 = desc.l_path_table_pos2;
        tmp.m_path_table_pos = desc.m_path_table_pos;
        tmp.m_path_table_pos2 = desc.m_path_table_pos2;
        tmp.root_record = desc.root_record;
        tmp.volume_set_id = detail::make_iso_string(desc.volume_set_id, ' ');
        tmp.publisher_id = detail::make_iso_string(desc.publisher_id, ' ');
        tmp.data_preparer_id =
            detail::make_iso_string(desc.data_preparer_id, ' ');
        tmp.application_id = detail::make_iso_string(desc.application_id, ' ');
        tmp.copyright_file_id =
            detail::make_iso_file_id(desc.copyright_file_id);
        tmp.abstract_file_id = detail::make_iso_file_id(desc.abstract_file_id);
        tmp.bibliographic_file_id =
            detail::make_iso_file_id(desc.bibliographic_file_id);
        tmp.file_structure_version = desc.file_structure_version;
        std::memcpy(tmp.application_use, desc.application_use, 512);
        return tmp;
    }

    static iso::volume_desc
    make_joliet_volume_desc(const iso::volume_descriptor& desc)
    {
        iso::volume_desc tmp;
        tmp.rrip = iso::rrip_none;
        tmp.type = desc.type;
        tmp.version = desc.version;
        tmp.flags = desc.flags;
        tmp.system_id = detail::make_joliet_string(desc.system_id);
        tmp.volume_id = detail::make_joliet_string(desc.volume_id);
        tmp.escape_sequences =
            detail::make_iso_string(desc.escape_sequences, '\0');
        tmp.path_table_size = desc.path_table_size;
        tmp.l_path_table_pos = desc.l_path_table_pos;
        tmp.l_path_table_pos2 = desc.l_path_table_pos2;
        tmp.m_path_table_pos = desc.m_path_table_pos;
        tmp.m_path_table_pos2 = desc.m_path_table_pos2;
        tmp.root_record = desc.root_record;
        tmp.volume_set_id = detail::make_joliet_string(desc.volume_set_id);
        tmp.publisher_id = detail::make_joliet_string(desc.publisher_id);
        tmp.data_preparer_id =
            detail::make_joliet_string(desc.data_preparer_id);
        tmp.application_id = detail::make_joliet_string(desc.application_id);
        tmp.copyright_file_id =
            detail::make_joliet_file_id(desc.copyright_file_id);
        tmp.abstract_file_id =
            detail::make_joliet_file_id(desc.abstract_file_id);
        tmp.bibliographic_file_id =
            detail::make_joliet_file_id(desc.bibliographic_file_id);
        tmp.file_structure_version = desc.file_structure_version;
        std::memcpy(tmp.application_use, desc.application_use, 512);
        return tmp;
    }

    static iso::volume_info make_volume_info(const iso::volume_descriptor& desc)
    {
        iso::volume_info info;
        std::memcpy(info.std_id, desc.std_id, 5);
        info.volume_space_size = desc.volume_space_size;
        info.volume_set_size = desc.volume_set_size;
        info.volume_seq_number = desc.volume_seq_number;
        info.logical_block_size = desc.logical_block_size;
        info.creation_time = desc.creation_time;
        info.modification_time = desc.modification_time;
        info.expiration_time = desc.expiration_time;
        info.effective_time = desc.effective_time;
        return info;
    }

    void read_volume_descriptor()
    {
        boost::iostreams::seek(src_, logical_sector_size*16, BOOST_IOS::beg);

        char block[logical_sector_size];
        iso::volume_descriptor desc;
        while (true)
        {
            iostreams::blocking_read(src_, block, sizeof(block));
            if (block[0] == '\xFF')
                break;

            if ((block[0] == '\x01') || (block[0] == '\x02'))
            {
                hamigaki::binary_read(block, desc);

                if (volume_descs_.empty())
                    volume_info_ = self::make_volume_info(desc);

                if (desc.is_joliet())
                {
                    volume_descs_.push_back(
                        self::make_joliet_volume_desc(desc));
                }
                else
                    volume_descs_.push_back(self::make_volume_desc(desc));
            }
        }

        if (volume_descs_.empty())
        {
            throw BOOST_IOSTREAMS_FAILURE(
                "ISO 9660 volume descriptor not found");
        }
    }

    void rock_ridge_check()
    {
        typedef iso_data_reader<Source> reader_type;

        const boost::uint32_t lbn_shift =
            calc_lbn_shift(volume_info_.logical_block_size);

        for (std::size_t i = 0; i < volume_descs_.size(); ++i)
        {
            iso::volume_desc& desc = volume_descs_[i];

            reader_type reader(src_, lbn_shift);
            reader.select_directory(desc.root_record.data_pos);
            const iso_directory_record& root = reader.entries().at(0);
            desc.rrip = detail::rock_ridge_check(root.system_use);
        }
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_ISO_FILE_SOURCE_IMPL_HPP
