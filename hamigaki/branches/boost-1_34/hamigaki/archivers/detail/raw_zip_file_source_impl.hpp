//  raw_zip_file_source_impl.hpp: raw ZIP file source implementation

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/zip_internal_header.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <boost/iostreams/detail/adapter/direct_adapter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <stdexcept>
#include <vector>

namespace hamigaki { namespace archivers { namespace detail {

inline const char* find_footer_signature(const char* start, const char* last)
{
    const char* cur = last - 4;
    while (cur >= start)
    {
        if (cur[0] == 'P')
        {
            if ((cur[1] == 'K') && (cur[2] == '\x05') && (cur[3] == '\x06'))
                return cur;
        }
        --cur;
    }
    return 0;
}

template<class Source>
inline void seek_end_of_central_dir(Source& src)
{
    const iostreams::stream_offset file_size =
        iostreams::to_offset(boost::iostreams::seek(src, 0, BOOST_IOS::end));

    std::streamsize entry_size = static_cast<std::streamsize>(
        struct_size<zip::end_of_central_directory>::value);
    iostreams::stream_offset pos = file_size - entry_size - 4;

    boost::iostreams::seek(src, pos, BOOST_IOS::beg);
    boost::uint32_t signature = iostreams::read_uint32<little>(src);
    if (signature == zip::end_of_central_directory::signature)
        return;

    char buf[1024];
    std::streamsize size = static_cast<std::streamsize>(sizeof(buf));
    do
    {
        pos -= size;
        if (pos < 0)
        {
            size += pos;
            pos = 0;
        }
        boost::iostreams::seek(src, pos, BOOST_IOS::beg);
        iostreams::blocking_read(src, buf, size);

        if (const char* ptr = find_footer_signature(buf, buf + size))
        {
            iostreams::stream_offset offset = (ptr+4) - (buf + size);
            boost::iostreams::seek(src, offset, BOOST_IOS::cur);
            return;
        }

    } while (pos != 0);

    throw std::runtime_error("cannot find ZIP footer");
}

template<class Source>
inline void read_local_extra_field(Source& src, zip::header& head)
{
    zip::extra_field_header ex_head;
    while (iostreams::binary_read(src, ex_head, std::nothrow))
    {
        boost::scoped_array<char> data(new char[ex_head.size]);
        if (ex_head.size)
            iostreams::blocking_read(src, data.get(), ex_head.size);
        if (ex_head.id == zip::extra_field_id::zip64)
        {
            boost::uint16_t pos = 0;

            if ((head.file_size == 0xFFFFFFFFull) ||
                (head.compressed_size == 0xFFFFFFFFull) )
            {
                if (pos + 8 > ex_head.size)
                    throw std::runtime_error("bad ZIP64 extended header");
                head.file_size = decode_uint<little,8>(&data[pos]);
                pos += 8;

                if (pos + 8 > ex_head.size)
                    throw std::runtime_error("bad ZIP64 extended header");
                head.compressed_size = decode_uint<little,8>(&data[pos]);
                pos += 8;
            }
        }
        else if (ex_head.id == zip::extra_field_id::extended_timestamp)
        {
            if (ex_head.size == 0)
                throw std::runtime_error("bad ZIP extended timestamp");

            unsigned char flags = static_cast<unsigned char>(data[0]);
            boost::uint16_t pos = 1;
            if ((flags & 0x01) != 0)
            {
                if (pos + 4 > ex_head.size)
                    throw std::runtime_error("bad ZIP extended timestamp");
                head.modified_time = static_cast<std::time_t>(
                    hamigaki::decode_int<little,4>(&data[pos]));
                pos += 4;
            }
            if ((flags & 0x02) != 0)
            {
                if (pos + 4 > ex_head.size)
                    throw std::runtime_error("bad ZIP extended timestamp");
                head.access_time = static_cast<std::time_t>(
                    hamigaki::decode_int<little,4>(&data[pos]));
                pos += 4;
            }
            if ((flags & 0x04) != 0)
            {
                if (pos + 4 > ex_head.size)
                    throw std::runtime_error("bad ZIP extended timestamp");
                head.creation_time = static_cast<std::time_t>(
                    hamigaki::decode_int<little,4>(&data[pos]));
            }
        }
        else if (ex_head.id == zip::extra_field_id::info_zip_unix2)
        {
            head.uid = hamigaki::decode_int<little,2>(&data[0]);
            head.gid = hamigaki::decode_int<little,2>(&data[2]);
        }
    }
}

template<class Source>
inline void read_central_extra_field(Source& src, zip_internal_header& head)
{
    zip::extra_field_header ex_head;
    while (iostreams::binary_read(src, ex_head, std::nothrow))
    {
        boost::scoped_array<char> data(new char[ex_head.size]);
        if (ex_head.size)
            iostreams::blocking_read(src, data.get(), ex_head.size);
        if (ex_head.id == zip::extra_field_id::zip64)
        {
            boost::uint16_t pos = 0;

            if (head.file_size == 0xFFFFFFFFull)
            {
                if (pos + 8 > ex_head.size)
                    throw std::runtime_error("bad ZIP64 extended header");
                head.file_size = decode_uint<little,8>(&data[pos]);
                pos += 8;
            }
            if (head.compressed_size == 0xFFFFFFFFull)
            {
                if (pos + 8 > ex_head.size)
                    throw std::runtime_error("bad ZIP64 extended header");
                head.compressed_size = decode_uint<little,8>(&data[pos]);
                pos += 8;
            }
            if (head.offset == 0xFFFFFFFFull)
            {
                if (pos + 8 > ex_head.size)
                    throw std::runtime_error("bad ZIP64 extended header");
                head.offset = decode_uint<little,8>(&data[pos]);
                pos += 8;
            }
        }
        else if (ex_head.id == zip::extra_field_id::extended_timestamp)
        {
            if (ex_head.size == 0)
                throw std::runtime_error("bad ZIP extended timestamp");

            unsigned char flags = static_cast<unsigned char>(data[0]);
            if ((flags & 0x01) != 0)
            {
                if (5 > ex_head.size)
                    throw std::runtime_error("bad ZIP extended timestamp");
                head.modified_time = static_cast<std::time_t>(
                    hamigaki::decode_int<little,4>(&data[1]));
            }
        }
    }
}

template<class Source>
class basic_raw_zip_file_source_impl : private boost::noncopyable
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    explicit basic_raw_zip_file_source_impl(const Source& src)
        : src_(src), pos_(0), next_index_(0)
    {
        read_central_dir();
    }

    bool next_entry()
    {
        using namespace boost::filesystem;

        if (next_index_ >= headers_.size())
            return false;

        select_entry(next_index_);
        return true;
    }

    void select_entry(const boost::filesystem::path& ph)
    {
        typedef std::vector<zip_internal_header> headers_type;
        typedef headers_type::const_iterator iter_type;

        iter_type pos = std::find_if(
            headers_.begin(), headers_.end(), zip::header_path_match(ph));
        if (pos == headers_.end())
            throw std::runtime_error("no such path");

        select_entry(pos - headers_.begin());
    }

    zip::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if ((pos_ >= header_.compressed_size) || (n <= 0))
            return -1;

        boost::uint64_t rest = header_.compressed_size - pos_;
        std::streamsize amt = hamigaki::auto_min(n, rest);
        iostreams::blocking_read(src_, s, amt);
        pos_ += amt;
        return amt;
    }

private:
    Source src_;
    zip::header header_;
    boost::uint64_t pos_;
    std::size_t next_index_;
    std::vector<zip_internal_header> headers_;

    void read_central_dir()
    {
        using namespace boost::filesystem;
        std::vector<zip_internal_header> tmp;

        detail::seek_end_of_central_dir(src_);
        iostreams::stream_offset footer_offset = iostreams::tell_offset(src_);

        zip::end_of_central_directory footer;
        iostreams::binary_read(src_, footer);

        if (footer.offset == 0xFFFFFFFF)
        {
            std::streamsize entry_size = static_cast<std::streamsize>(
                struct_size<zip::zip64_end_cent_dir_locator>::value);

            iostreams::stream_offset off = footer_offset - (entry_size + 8);
            boost::iostreams::seek(src_, off, BOOST_IOS::beg);

            boost::uint32_t signature = iostreams::read_uint32<little>(src_);
            if (signature == zip::zip64_end_cent_dir_locator::signature)
            {
                zip::zip64_end_cent_dir_locator loc;
                iostreams::binary_read(src_, loc);

                boost::iostreams::seek(
                    src_,
                    static_cast<boost::iostreams::stream_offset>(loc.offset),
                    BOOST_IOS::beg);
                signature = iostreams::read_uint32<little>(src_);
                if (signature != zip::zip64_end_cent_dir::signature)
                    throw std::runtime_error("bad ZIP signature");

                zip::zip64_end_cent_dir zip64;
                iostreams::binary_read(src_, zip64);
                boost::iostreams::seek(
                    src_,
                    static_cast<boost::iostreams::stream_offset>(zip64.offset),
                    BOOST_IOS::beg);
            }
            else
                boost::iostreams::seek(src_, footer.offset, BOOST_IOS::beg);
        }
        else
            boost::iostreams::seek(src_, footer.offset, BOOST_IOS::beg);

        tmp.reserve(footer.entries);

        for (boost::uint16_t i = 0; i < footer.entries; ++i)
        {
            boost::uint32_t signature = iostreams::read_uint32<little>(src_);
            if (signature != zip::file_header::signature)
                throw std::runtime_error("bad ZIP signature");

            zip::file_header file_head;
            iostreams::binary_read(src_, file_head);

            zip_internal_header head;
            head.version =
                static_cast<boost::uint8_t>(file_head.needed_to_extract);
            if ((file_head.flags & zip::flags::encrypted) != 0)
                head.encrypted = true;
            head.method = file_head.method;
            head.update_time = file_head.update_date_time.to_time_t();
            head.compressed_size = file_head.compressed_size;
            head.crc32_checksum = file_head.crc32_checksum;
            head.file_size = file_head.file_size;
            head.attributes =
                static_cast<boost::uint16_t>(file_head.external_attributes);
            head.permissions =
                static_cast<boost::uint16_t>(file_head.external_attributes>>16);
            head.offset = file_head.offset;

            if (file_head.file_name_length != 0)
            {
                boost::scoped_array<char>
                    filename(new char[file_head.file_name_length+1]);

                iostreams::blocking_read(
                    src_, &filename[0], file_head.file_name_length);
                filename[file_head.file_name_length] = '\0';
                head.path = path(&filename[0], no_check);
                if (filename[file_head.file_name_length-1] == '/')
                    head.attributes |= msdos::attributes::directory;
            }

            if (file_head.extra_field_length)
            {
                boost::scoped_array<char> extra(
                    new char[file_head.extra_field_length]);
                iostreams::blocking_read(
                    src_, &extra[0], file_head.extra_field_length);

                using boost::iostreams::array_source;
                typedef boost::iostreams::detail::
                    direct_adapter<array_source> source_type;

                source_type src(array_source(
                    extra.get(),
                    static_cast<std::size_t>(file_head.extra_field_length)));

                detail::read_central_extra_field(src, head);
            }

            if (file_head.comment_length)
            {
                boost::scoped_array<char> comment(
                    new char[file_head.comment_length]);
                iostreams::blocking_read(
                    src_, &comment[0], file_head.comment_length);
                head.comment = std::string(
                    &comment[0], &comment[0]+file_head.comment_length);
            }

            tmp.push_back(head);
        }

        boost::iostreams::seek(src_, 0, BOOST_IOS::beg);
        tmp.swap(headers_);
    }

    void select_entry(std::size_t index)
    {
        using namespace boost::filesystem;

        zip_internal_header head = headers_[index];
        next_index_ = ++index;

        boost::iostreams::seek(
            src_,
            static_cast<boost::iostreams::stream_offset>(head.offset),
            BOOST_IOS::beg);
        pos_ = 0;

        boost::uint32_t signature = iostreams::read_uint32<little>(src_);
        if (signature != zip::local_file_header::signature)
            throw std::runtime_error("bad ZIP signature");

        zip::local_file_header local;
        iostreams::binary_read(src_, local);

        head.version = static_cast<boost::uint8_t>(local.needed_to_extract);
        if ((local.flags & zip::flags::encrypted) != 0)
            head.encrypted = true;
        head.method = local.method;
        head.update_time = local.update_date_time.to_time_t();
        if ((local.flags & zip::flags::has_data_dec) != 0)
            head.encryption_checksum = local.update_date_time.time;
        else
        {
            head.encryption_checksum =
                static_cast<boost::uint16_t>(local.crc32_checksum >> 16);
            head.crc32_checksum = local.crc32_checksum;
            head.compressed_size = local.compressed_size;
            head.file_size = local.file_size;
        }

        if (local.file_name_length != 0)
        {
            boost::scoped_array<char>
                filename(new char[local.file_name_length+1]);

            iostreams::blocking_read(
                src_, &filename[0], local.file_name_length);
            filename[local.file_name_length] = '\0';
            head.path = path(&filename[0], no_check);
            if (filename[local.file_name_length-1] == '/')
                head.attributes |= msdos::attributes::directory;
        }

        if (local.extra_field_length)
        {
            boost::scoped_array<char> extra(new char[local.extra_field_length]);
            iostreams::blocking_read(src_, &extra[0], local.extra_field_length);

            using boost::iostreams::array_source;
            typedef boost::iostreams::detail::
                direct_adapter<array_source> source_type;

            source_type src(array_source(
                extra.get(),
                static_cast<std::size_t>(local.extra_field_length)));

            detail::read_local_extra_field(src, head);
        }

        header_ = head;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_ZIP_FILE_SOURCE_IMPL_HPP
