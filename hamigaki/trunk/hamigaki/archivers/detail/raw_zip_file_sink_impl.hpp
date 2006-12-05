//  raw_zip_file_sink_impl.hpp: raw ZIP file sink implementation

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/zip_internal_header.hpp>
#include <hamigaki/archivers/error.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink>
inline void write_local_ex_timestamp(Sink& sink, const zip::header& head)
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
            iostreams::write_int32<little>(sink, head.modified_time.get());
        if (flags & 0x02)
            iostreams::write_int32<little>(sink, head.access_time.get());
        if (flags & 0x04)
            iostreams::write_int32<little>(sink, head.creation_time.get());
    }
}

template<class Sink>
inline void write_central_ex_timestamp(Sink& sink, const zip::header& head)
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
            iostreams::write_int32<little>(sink, head.modified_time.get());
    }
}

template<class Sink>
inline void write_local_extra_field(Sink& sink, const zip::header& head)
{
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

template<class Sink>
inline void write_central_extra_field(Sink& sink, const zip::header& head)
{
    write_central_ex_timestamp(sink, head);

    if (head.uid && head.gid)
    {
        zip::extra_field_header ex_head;
        ex_head.id = zip::extra_field_id::info_zip_unix2;
        ex_head.size = 0;

        iostreams::binary_write(sink, ex_head);
    }
}

template<class Sink>
class basic_raw_zip_file_sink_impl : private boost::noncopyable
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_raw_zip_file_sink_impl(const Sink& sink)
        : sink_(sink), size_(0), overflow_(false)
    {
    }

    void create_entry(const zip::header& head)
    {
        if (overflow_)
            throw std::runtime_error("need to rewind the current entry");

        if (size_ != header_.compressed_size)
            throw std::runtime_error("ZIP entry size mismatch");

        static_cast<zip::header&>(header_) = head;
        if (header_.is_directory())
        {
            header_.method = zip::method::store;
            header_.crc32_checksum = 0;
            header_.compressed_size = 0;
            header_.file_size = 0;
        }

        header_.offset =
            static_cast<boost::uint32_t>(iostreams::tell_offset(sink_));

        write_local_file_header(header_);

        size_ = 0;
        headers_.push_back(header_);
    }

    void rewind_entry()
    {
        header_.method = zip::method::store;

        boost::iostreams::seek(sink_, header_.offset, BOOST_IOS::beg);
        write_local_file_header(header_);

        size_ = 0;
        overflow_ = false;
        headers_.back() = header_;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        boost::uint32_t max_size = header_.file_size;
        if (header_.encrypted)
            max_size += zip::consts::encryption_header_size;

        if (size_ + n > max_size)
        {
            overflow_ = true;
            throw give_up_compression();
        }

        iostreams::blocking_write(sink_, s, n);
        size_ += n;
        return n;
    }

    void close()
    {
        if (size_ != header_.compressed_size)
            throw BOOST_IOSTREAMS_FAILURE("ZIP entry size mismatch");

        headers_.back() = header_;
    }

    void close(
        boost::uint32_t crc32_checksum, boost::uint32_t file_size)
    {
        header_.crc32_checksum = crc32_checksum;
        header_.compressed_size = size_;
        header_.file_size = file_size;

        if (header_.compressed_size != 0)
        {
            boost::iostreams::seek(sink_, header_.offset, BOOST_IOS::beg);
            write_local_file_header(header_);
            boost::iostreams::seek(sink_, 0, BOOST_IOS::end);
        }
        if (header_.encrypted)
        {
            iostreams::write_uint32<little>(
                sink_, zip::data_descriptor::signature);

            zip::data_descriptor desc;
            desc.crc32_checksum = header_.crc32_checksum;
            desc.compressed_size = header_.compressed_size;
            desc.file_size = header_.file_size;
            iostreams::binary_write(sink_, desc);
        }

        headers_.back() = header_;
    }

    void close_archive()
    {
        boost::uint32_t start_offset =
            static_cast<boost::uint32_t>(iostreams::tell_offset(sink_));

        for (std::size_t i = 0; i < headers_.size(); ++i)
        {
            const zip_internal_header& head = headers_[i];

            std::string filename = head.path.string();
            if (head.is_directory())
                filename += '/';

            std::string extra_field;
            boost::iostreams::
                back_insert_device<std::string> ex_sink(extra_field);
            detail::write_central_extra_field(ex_sink, head);

            zip::file_header file_head;
            file_head.made_by = 20;
            file_head.needed_to_extract = head.version;
            file_head.flags = head.encrypted
                ? zip::flags::encrypted | zip::flags::has_data_dec
                : 0;
            file_head.method = head.method;
            file_head.update_date_time = msdos::date_time(head.update_time);
            file_head.compressed_size = head.compressed_size;
            file_head.crc32_checksum = head.crc32_checksum;
            file_head.file_size = head.file_size;
            file_head.file_name_length = filename.size();
            file_head.extra_field_length = extra_field.size();
            file_head.comment_length = head.comment.size();
            file_head.disk_number_start = 0; // TODO
            file_head.internal_attributes = 0; // TODO
            file_head.external_attributes =
                head.attributes |
                (static_cast<boost::uint32_t>(head.permission) << 16);
            file_head.offset = head.offset;

            iostreams::write_uint32<little>(sink_, zip::file_header::signature);
            iostreams::binary_write(sink_, file_head);

            if (!filename.empty())
                iostreams::blocking_write(sink_, filename);

            if (!extra_field.empty())
                iostreams::blocking_write(sink_, extra_field);

            if (!head.comment.empty())
                iostreams::blocking_write(sink_, head.comment);
        }

        boost::uint32_t end_offset =
            static_cast<boost::uint32_t>(iostreams::tell_offset(sink_));

        zip::end_of_central_directory footer;
        footer.disk_number = 0; // TODO
        footer.start_disk_number = 0; // TODO
        footer.entries = headers_.size();
        footer.total_entries = footer.entries; // TODO
        footer.size = end_offset - start_offset;
        footer.offset = start_offset;
        footer.comment_length = 0; //TODO

        iostreams::write_uint32<little>(
            sink_, zip::end_of_central_directory::signature);
        iostreams::binary_write(sink_, footer);

        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    zip_internal_header header_;
    boost::uint32_t size_;
    bool overflow_;
    std::vector<zip_internal_header> headers_;

    void write_local_file_header(const zip::header& head)
    {
        std::string filename = head.path.string();
        if (head.is_directory())
            filename += '/';

        std::string extra_field;
        boost::iostreams::back_insert_device<std::string> ex_sink(extra_field);
        detail::write_local_extra_field(ex_sink, head);

        zip::local_file_header local;
        local.needed_to_extract = head.version;
        local.flags = head.encrypted
            ? zip::flags::encrypted | zip::flags::has_data_dec
            : 0;
        local.method = head.method;
        local.update_date_time = msdos::date_time(head.update_time);
        local.crc32_checksum = head.crc32_checksum;
        local.compressed_size = head.compressed_size;
        local.file_size = head.file_size;
        local.file_name_length = filename.size();
        local.extra_field_length = extra_field.size();

        iostreams::write_uint32<little>(
            sink_, zip::local_file_header::signature);
        iostreams::binary_write(sink_, local);
        if (!filename.empty())
            iostreams::blocking_write(sink_, filename);
        if (!extra_field.empty())
            iostreams::blocking_write(sink_, extra_field);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SINK_IMPL_HPP
