// raw_zip_file_sink_impl.hpp: raw ZIP file sink implementation

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/zip_internal_header.hpp>
#include <hamigaki/archivers/error.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/crc.hpp>
#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <vector>

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    #include <hamigaki/charset/code_page.hpp>
#endif

namespace hamigaki { namespace archivers { namespace detail {

inline std::string make_zip_path_string(const boost::filesystem::path& ph)
{
    return ph.string();
}

inline std::string make_zip_comment(const std::string& s)
{
    return s;
}

template<class Path>
struct default_zip_flags
{
    static const boost::uint16_t value = 0;
};

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
inline std::string make_zip_path_string(const boost::filesystem::wpath& ph)
{
    return charset::to_code_page(ph.string(), 65001);
}

inline std::string make_zip_comment(const std::wstring& s)
{
    return charset::to_code_page(s, 65001);
}

template<>
struct default_zip_flags<boost::filesystem::wpath>
{
    static const boost::uint16_t value = zip::flags::utf8_encoded;
};
#endif

template<class Sink, class Path>
inline void write_local_zip64(Sink& sink, const zip::basic_header<Path>& head)
{
    if ((head.file_size >= 0xFFFFFFFFull) ||
        (head.compressed_size >= 0xFFFFFFFFull) )
    {
        zip::extra_field_header ex_head;
        ex_head.id = zip::extra_field_id::zip64;
        ex_head.size = 16;

        iostreams::binary_write(sink, ex_head);
        iostreams::write_uint64<little>(sink, head.file_size);
        iostreams::write_uint64<little>(sink, head.compressed_size);
    }
}

template<class Sink, class Path>
inline void
write_central_zip64(Sink& sink, const zip_internal_header<Path>& head)
{
    zip::extra_field_header ex_head;
    ex_head.id = zip::extra_field_id::zip64;
    ex_head.size = 0;

    if (head.file_size >= 0xFFFFFFFFull)
        ex_head.size += 8;
    if (head.compressed_size >= 0xFFFFFFFFull)
        ex_head.size += 8;
    if (head.offset >= 0xFFFFFFFFull)
        ex_head.size += 8;

    if (ex_head.size != 0)
    {
        iostreams::binary_write(sink, ex_head);
        if (head.file_size >= 0xFFFFFFFFull)
            iostreams::write_uint64<little>(sink, head.file_size);
        if (head.compressed_size >= 0xFFFFFFFFull)
            iostreams::write_uint64<little>(sink, head.compressed_size);
        if (head.offset >= 0xFFFFFFFFull)
            iostreams::write_uint64<little>(sink, head.offset);
    }
}

template<class Sink, class Path>
inline void
write_local_ex_timestamp(Sink& sink, const zip::basic_header<Path>& head)
{
    zip::extra_field_header ex_head;
    ex_head.id = zip::extra_field_id::extended_timestamp;
    ex_head.size = 1;
    boost::uint8_t flags = 0;
    if (head.modified_time)
    {
        flags |= 0x01;
        ex_head.size += 4;
    }
    if (head.access_time)
    {
        flags |= 0x02;
        ex_head.size += 4;
    }
    if (head.creation_time)
    {
        flags |= 0x04;
        ex_head.size += 4;
    }

    if (flags != 0)
    {
        iostreams::binary_write(sink, ex_head);
        iostreams::write_uint8<little>(sink, flags);
        if (flags & 0x01)
        {
            iostreams::write_int32<little>(
                sink, static_cast<boost::uint32_t>(head.modified_time.get()));
        }
        if (flags & 0x02)
        {
            iostreams::write_int32<little>(
                sink, static_cast<boost::uint32_t>(head.access_time.get()));
        }
        if (flags & 0x04)
        {
            iostreams::write_int32<little>(
                sink, static_cast<boost::uint32_t>(head.creation_time.get()));
        }
    }
}

template<class Sink, class Path>
inline void
write_central_ex_timestamp(Sink& sink, const zip::basic_header<Path>& head)
{
    zip::extra_field_header ex_head;
    ex_head.id = zip::extra_field_id::extended_timestamp;
    ex_head.size = 1;
    boost::uint8_t flags = 0;
    if (head.modified_time)
    {
        flags |= 0x01;
        ex_head.size += 4;
    }
    if (head.access_time)
        flags |= 0x02;
    if (head.creation_time)
        flags |= 0x04;

    if (flags != 0)
    {
        iostreams::binary_write(sink, ex_head);
        iostreams::write_uint8<little>(sink, flags);
        if (flags & 0x01)
        {
            iostreams::write_int32<little>(
                sink, static_cast<boost::uint32_t>(head.modified_time.get()));
        }
    }
}

template<class Sink, class Path>
inline void
write_local_extra_field(Sink& sink, const zip::basic_header<Path>& head)
{
    write_local_zip64(sink, head);
    write_local_ex_timestamp(sink, head);

    if (head.uid && head.gid)
    {
        zip::extra_field_header ex_head;
        ex_head.id = zip::extra_field_id::info_zip_unix2;
        ex_head.size = 4;

        iostreams::binary_write(sink, ex_head);

        iostreams::write_uint16<little>(sink, head.uid.get());
        iostreams::write_uint16<little>(sink, head.gid.get());
    }
}

template<class Sink, class Path>
inline void write_central_extra_field(
    Sink& sink, const zip_internal_header<Path>& head)
{
    write_central_zip64(sink, head);
    write_central_ex_timestamp(sink, head);

    if (head.uid && head.gid)
    {
        zip::extra_field_header ex_head;
        ex_head.id = zip::extra_field_id::info_zip_unix2;
        ex_head.size = 0;

        iostreams::binary_write(sink, ex_head);
    }
}

template<class Sink, class Path>
class basic_raw_zip_file_sink_impl : private boost::noncopyable
{
public:
    typedef char char_type;
    typedef Path path_type;
    typedef zip::basic_header<Path> header_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_raw_zip_file_sink_impl(const Sink& sink)
        : sink_(sink), size_(0), overflow_(false), zip64_(false)
    {
    }

    void create_entry(const header_type& head)
    {
        if (overflow_)
            throw std::runtime_error("need to rewind the current entry");

        if (size_ != header_.compressed_size)
            throw std::runtime_error("ZIP entry size mismatch");

        static_cast<header_type&>(header_) = head;
        if (header_.is_directory())
        {
            header_.method = zip::method::store;
            header_.crc32_checksum = 0;
            header_.compressed_size = 0;
            header_.file_size = 0;
        }

        header_.offset =
            static_cast<boost::uint64_t>(iostreams::tell_offset(sink_));
        if (header_.offset >= 0xFFFFFFFFull)
            zip64_ = true;

        write_local_file_header(header_);

        size_ = 0;
        headers_.push_back(header_);
    }

    void rewind_entry()
    {
        header_.method = zip::method::store;

        boost::iostreams::seek(
            sink_,
            static_cast<boost::iostreams::stream_offset>(header_.offset),
            BOOST_IOS::beg);
        write_local_file_header(header_);

        size_ = 0;
        overflow_ = false;
        headers_.back() = header_;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        // Note: If header_.file_size >= 2^32, use ZIP64.
        boost::uint32_t max_size =
            static_cast<boost::uint32_t>(header_.file_size);
        if (header_.encrypted)
            max_size += zip::consts::encryption_header_size;

        if (size_ + n > max_size)
        {
            overflow_ = true;
            throw give_up_compression();
        }

        iostreams::blocking_write(sink_, s, n);
        size_ += static_cast<boost::uint32_t>(n);
        return n;
    }

    void close()
    {
        if (size_ != header_.compressed_size)
            throw BOOST_IOSTREAMS_FAILURE("ZIP entry size mismatch");

        if (header_.encrypted)
            write_data_descriptor();

        headers_.back() = header_;
    }

    void close(
        boost::uint32_t crc32_checksum, boost::uint64_t file_size)
    {
        header_.crc32_checksum = crc32_checksum;
        header_.compressed_size = size_;
        header_.file_size = file_size;

        if (header_.compressed_size != 0)
        {
            boost::iostreams::seek(
                sink_,
                static_cast<boost::iostreams::stream_offset>(header_.offset),
                BOOST_IOS::beg);
            write_local_file_header(header_);
            boost::iostreams::seek(sink_, 0, BOOST_IOS::end);
        }

        if (header_.encrypted)
            write_data_descriptor();

        headers_.back() = header_;
    }

    void close_archive()
    {
        boost::uint64_t start_offset =
            static_cast<boost::uint64_t>(iostreams::tell_offset(sink_));

        for (std::size_t i = 0; i < headers_.size(); ++i)
        {
            const zip_internal_header<Path>& head = headers_[i];

            std::string filename = detail::make_zip_path_string(head.path);
            if (head.is_directory())
                filename += '/';

            std::string extra_field;
            boost::iostreams::
                back_insert_device<std::string> ex_sink(extra_field);
            detail::write_central_extra_field(ex_sink, head);

            zip::file_header file_head;
            file_head.made_by = 20;
            file_head.needed_to_extract = head.version;

            file_head.flags = default_zip_flags<Path>::value;
            if (head.encrypted)
            {
                file_head.flags |=
                    zip::flags::encrypted | zip::flags::has_data_dec;
            }

            const std::string& comment = detail::make_zip_comment(head.comment);

            file_head.method = head.method;
            file_head.update_date_time = msdos::date_time(head.update_time);
            file_head.compressed_size =
                static_cast<boost::uint32_t>(head.compressed_size);
            file_head.crc32_checksum = head.crc32_checksum;
            file_head.file_size = static_cast<boost::uint32_t>(head.file_size);
            file_head.file_name_length =
                static_cast<boost::uint16_t>(filename.size());
            file_head.extra_field_length =
                static_cast<boost::uint16_t>(extra_field.size());
            file_head.comment_length =
                static_cast<boost::uint16_t>(comment.size());
            file_head.disk_number_start = 0; // TODO
            file_head.internal_attributes = 0; // TODO
            file_head.external_attributes =
                head.attributes |
                (static_cast<boost::uint32_t>(head.permissions) << 16);

            if (head.offset < 0xFFFFFFFFull)
                file_head.offset = static_cast<boost::uint32_t>(head.offset);
            else
                file_head.offset = 0xFFFFFFFFu;

            iostreams::write_uint32<little>(sink_, zip::file_header::signature);
            iostreams::binary_write(sink_, file_head);

            if (!filename.empty())
                iostreams::blocking_write(sink_, filename);

            if (!extra_field.empty())
                iostreams::blocking_write(sink_, extra_field);

            if (!comment.empty())
                iostreams::blocking_write(sink_, comment);
        }

        boost::uint64_t end_offset =
            static_cast<boost::uint64_t>(iostreams::tell_offset(sink_));
        boost::uint64_t cent_dir_size = end_offset - start_offset;

        if ((cent_dir_size >= 0xFFFFFFFFull) ||
            (start_offset >= 0xFFFFFFFFull) ||
            (headers_.size() >= 0xFFFFu))
        {
            zip64_ = true;
        }

        if (zip64_)
        {
            boost::uint64_t offset =
                static_cast<boost::uint64_t>(iostreams::tell_offset(sink_));

            zip::zip64_end_cent_dir zip64;
            zip64.self_size = struct_size<zip::zip64_end_cent_dir>::value - 8;
            zip64.made_by = 45;
            zip64.needed_to_extract = 45;
            zip64.disk_number = 0; // TODO
            zip64.start_disk_number = 0; // TODO
            zip64.entries = headers_.size();
            zip64.total_entries = zip64.entries; // TODO
            zip64.size = cent_dir_size;
            zip64.offset = start_offset;

            iostreams::write_uint32<little>(
                sink_, zip::zip64_end_cent_dir::signature);
            iostreams::binary_write(sink_, zip64);


            zip::zip64_end_cent_dir_locator loc;
            loc.disk_number = 0; // TODO
            loc.offset = offset;
            loc.total_disk_number = 1; // TODO

            iostreams::write_uint32<little>(
                sink_, zip::zip64_end_cent_dir_locator::signature);
            iostreams::binary_write(sink_, loc);
        }

        zip::end_of_central_directory footer;
        footer.disk_number = 0; // TODO
        footer.start_disk_number = 0; // TODO
        if (headers_.size() < 0xFFFFu)
            footer.entries = static_cast<boost::uint16_t>(headers_.size());
        else
            footer.entries = 0xFFFFu;
        footer.total_entries = footer.entries; // TODO

        if (cent_dir_size < 0xFFFFFFFFull)
            footer.size = static_cast<boost::uint32_t>(cent_dir_size);
        else
            footer.size = 0xFFFFFFFFu;

        if (start_offset < 0xFFFFFFFFull)
            footer.offset = static_cast<boost::uint32_t>(start_offset);
        else
            footer.offset = 0xFFFFFFFFu;

        footer.comment_length = 0; //TODO

        iostreams::write_uint32<little>(
            sink_, zip::end_of_central_directory::signature);
        iostreams::binary_write(sink_, footer);

        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    zip_internal_header<Path> header_;
    boost::uint32_t size_;
    bool overflow_;
    bool zip64_;
    std::vector<zip_internal_header<Path> > headers_;

    void write_local_file_header(const header_type& head)
    {
        std::string filename = detail::make_zip_path_string(head.path);
        if (head.is_directory())
            filename += '/';

        std::string extra_field;
        boost::iostreams::back_insert_device<std::string> ex_sink(extra_field);
        detail::write_local_extra_field(ex_sink, head);

        zip::local_file_header local;
        local.needed_to_extract = head.version;

        local.flags = default_zip_flags<Path>::value;
        if (head.encrypted)
            local.flags |= zip::flags::encrypted | zip::flags::has_data_dec;

        local.method = head.method;
        local.update_date_time = msdos::date_time(head.update_time);
        local.crc32_checksum = head.crc32_checksum;

        bool zip64 = false;
        if ((head.compressed_size >= 0xFFFFFFFFull) ||
            (head.file_size >= 0xFFFFFFFFull) )
        {
            zip64 = true;
            zip64_ = true;
        }

        if (zip64)
            local.compressed_size = 0xFFFFFFFFu;
        else
        {
            local.compressed_size =
                static_cast<boost::uint32_t>(head.compressed_size);
        }

        if (zip64)
            local.file_size = 0xFFFFFFFFu;
        else
            local.file_size = static_cast<boost::uint32_t>(head.file_size);

        local.file_name_length = static_cast<boost::uint16_t>(filename.size());
        local.extra_field_length =
            static_cast<boost::uint16_t>(extra_field.size());

        iostreams::write_uint32<little>(
            sink_, zip::local_file_header::signature);
        iostreams::binary_write(sink_, local);
        if (!filename.empty())
            iostreams::blocking_write(sink_, filename);
        if (!extra_field.empty())
            iostreams::blocking_write(sink_, extra_field);
    }

    void write_data_descriptor()
    {
        // Note: 7-Zip doesn't support Zip64 formatted data descriptor
        if ((header_.compressed_size >= 0xFFFFFFFFull) ||
            (header_.file_size >= 0xFFFFFFFFull) )
        {
            return;
        }

        zip::data_descriptor desc;
        desc.crc32_checksum = header_.crc32_checksum;
        desc.compressed_size =
            static_cast<boost::uint32_t>(header_.compressed_size);
        desc.file_size = static_cast<boost::uint32_t>(header_.file_size);

        iostreams::write_uint32<little>(sink_, zip::data_descriptor::signature);
        iostreams::binary_write(sink_, desc);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SINK_IMPL_HPP
