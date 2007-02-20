//  iso_file_source_impl.hpp: ISO file source implementation

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/iso9660_reader.hpp>
#include <hamigaki/archivers/detail/joliet_reader.hpp>
#include <hamigaki/archivers/detail/rock_ridge_check.hpp>
#include <hamigaki/archivers/detail/rock_ridge_reader.hpp>
#include <hamigaki/archivers/detail/ucs2.hpp>
#include <memory>

namespace hamigaki { namespace archivers { namespace detail {

class iso_file_reader_base
{
public:
    virtual ~iso_file_reader_base(){};

    bool next_entry()
    {
        return do_next_entry();
    }

    iso::header header() const
    {
        return do_header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return do_read(s, n);
    }

private:
    virtual bool do_next_entry() = 0;
    virtual iso::header do_header() const = 0;
    virtual std::streamsize do_read(char* s, std::streamsize n) = 0;
};

template<class Impl>
class iso_file_reader : public iso_file_reader_base
{
public:
    typedef typename Impl::source_type source_type;

    iso_file_reader(source_type& src,
            const iso::volume_info& info, const iso::volume_desc& desc)
        : impl_(src, info, desc)
    {
    }

private:
    Impl impl_;

    bool do_next_entry() // virtual
    {
        return impl_.next_entry();
    }

    iso::header do_header() const // virtual
    {
        return impl_.header();
    }

    std::streamsize do_read(char* s, std::streamsize n) // virtual
    {
        return impl_.read(s, n);
    }
};

template<class Source>
class basic_iso_file_source_impl : private boost::noncopyable
{
private:
    typedef basic_iso_file_source_impl<Source> self_type;

    static const std::size_t logical_sector_size = 2048;

public:
    explicit basic_iso_file_source_impl(const Source& src) : src_(src)
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
        if (use_rrip && desc.is_rock_ridge())
        {
            typedef iso_file_reader<rock_ridge_reader<Source> > reader_type;
            reader_.reset(new reader_type(src_, volume_info_, desc));
        }
        else if (desc.is_joliet())
        {
            typedef iso_file_reader<joliet_reader<Source> > reader_type;
            reader_.reset(new reader_type(src_, volume_info_, desc));
        }
        else
        {
            typedef iso_file_reader<iso9660_reader<Source> > reader_type;
            reader_.reset(new reader_type(src_, volume_info_, desc));
        }
    }

private:
    Source src_;
    iso::volume_info volume_info_;
    std::vector<iso::volume_desc> volume_descs_;
    std::auto_ptr<iso_file_reader_base> reader_;

    template<std::size_t Size>
    static std::string make_string(const char (&data)[Size], char fill)
    {
        const char* end = std::find(data, data+Size, fill);
        return std::string(data, end);
    }

    template<std::size_t Size>
    static boost::filesystem::path make_file_id(const char (&data)[Size])
    {
        using namespace boost::filesystem;
        return path(make_string(data, ' '), no_check);
    }

    static iso::volume_desc make_volume_desc(const iso::volume_descriptor& desc)
    {
        iso::volume_desc tmp;
        tmp.rrip = iso::rrip_none;
        tmp.type = desc.type;
        tmp.version = desc.version;
        tmp.flags = desc.flags;
        tmp.system_id = make_string(desc.system_id, ' ');
        tmp.volume_id = make_string(desc.volume_id, ' ');
        tmp.escape_sequences = make_string(desc.escape_sequences, '\0');
        tmp.path_table_size = desc.path_table_size;
        tmp.l_path_table_pos = desc.l_path_table_pos;
        tmp.l_path_table_pos2 = desc.l_path_table_pos2;
        tmp.m_path_table_pos = desc.m_path_table_pos;
        tmp.m_path_table_pos2 = desc.m_path_table_pos2;
        tmp.root_record = desc.root_record;
        tmp.volume_set_id = make_string(desc.volume_set_id, ' ');
        tmp.publisher_id = make_string(desc.publisher_id, ' ');
        tmp.data_preparer_id = make_string(desc.data_preparer_id, ' ');
        tmp.application_id = make_string(desc.application_id, ' ');
        tmp.copyright_file_id = make_file_id(desc.copyright_file_id);
        tmp.abstract_file_id = make_file_id(desc.abstract_file_id);
        tmp.bibliographic_file_id = make_file_id(desc.bibliographic_file_id);
        tmp.file_structure_version = desc.file_structure_version;
        std::memcpy(tmp.application_use, desc.application_use, 512);
        return tmp;
    }

    template<std::size_t Size>
    static std::string make_joliet_string(const char (&data)[Size])
    {
        std::size_t src_size = Size/2;
        boost::scoped_array<wchar_t> src(new wchar_t[src_size + 1]);
        detail::ucs2be_to_wide(src.get(), data, Size);
        src[src_size] = 0;

        return detail::wide_to_narrow(src.get());
    }

    template<std::size_t Size>
    static boost::filesystem::path make_joliet_file_id(const char (&data)[Size])
    {
        using namespace boost::filesystem;
        return path(make_joliet_string(data), no_check);
    }

    static iso::volume_desc
    make_joliet_volume_desc(const iso::volume_descriptor& desc)
    {
        iso::volume_desc tmp;
        tmp.rrip = iso::rrip_none;
        tmp.type = desc.type;
        tmp.version = desc.version;
        tmp.flags = desc.flags;
        tmp.system_id = make_joliet_string(desc.system_id);
        tmp.volume_id = make_joliet_string(desc.volume_id);
        tmp.escape_sequences = make_string(desc.escape_sequences, '\0');
        tmp.path_table_size = desc.path_table_size;
        tmp.l_path_table_pos = desc.l_path_table_pos;
        tmp.l_path_table_pos2 = desc.l_path_table_pos2;
        tmp.m_path_table_pos = desc.m_path_table_pos;
        tmp.m_path_table_pos2 = desc.m_path_table_pos2;
        tmp.root_record = desc.root_record;
        tmp.volume_set_id = make_joliet_string(desc.volume_set_id);
        tmp.publisher_id = make_joliet_string(desc.publisher_id);
        tmp.data_preparer_id = make_joliet_string(desc.data_preparer_id);
        tmp.application_id = make_joliet_string(desc.application_id);
        tmp.copyright_file_id = make_joliet_file_id(desc.copyright_file_id);
        tmp.abstract_file_id = make_joliet_file_id(desc.abstract_file_id);
        tmp.bibliographic_file_id =
            make_joliet_file_id(desc.bibliographic_file_id);
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
        info.file_structure_version = desc.file_structure_version;
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
                    volume_info_ = self_type::make_volume_info(desc);

                if (desc.is_joliet())
                {
                    volume_descs_.push_back(
                        self_type::make_joliet_volume_desc(desc));
                }
                else
                    volume_descs_.push_back(self_type::make_volume_desc(desc));
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

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_FILE_SOURCE_IMPL_HPP
