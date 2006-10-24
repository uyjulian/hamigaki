//  zip_file.hpp: Phil Katz Zip file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_RAW_ZIP_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_RAW_ZIP_FILE_HPP

#include <hamigaki/iostreams/detail/msdos_attributes.hpp>
#include <hamigaki/iostreams/detail/msdos_date_time.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/scoped_array.hpp>
#include <boost/optional.hpp>
#include <vector>

namespace hamigaki { namespace iostreams { namespace zip {

struct flags
{
    static const boost::uint16_t has_data_dec   = 0x0008;
};

struct internal_attributes
{
    static const boost::uint16_t ascii  = 0x0001;
};

struct extra_field_id
{
    static const boost::uint16_t extended_timestamp = 0x5455;
    static const boost::uint16_t info_zip_unix2     = 0x7855;
};

struct local_file_header
{
    static const boost::uint32_t signature = 0x04034B50;

    boost::uint16_t needed_to_extract;
    boost::uint16_t flags;
    boost::uint16_t method;
    msdos_date_time update_date_time;
    boost::uint32_t crc32_checksum;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    boost::uint16_t file_name_length;
    boost::uint16_t extra_field_length;
};

struct data_descriptor
{
    static const boost::uint32_t signature = 0x08074B50;

    boost::uint32_t crc32_checksum;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
};

struct archive_extra_data
{
    static const boost::uint32_t signature = 0x08064B50;

    boost::uint32_t extra_field_length;
};

struct file_header
{
    static const boost::uint32_t signature = 0x02014B50;

    boost::uint16_t made_by;
    boost::uint16_t needed_to_extract;
    boost::uint16_t flags;
    boost::uint16_t method;
    msdos_date_time update_date_time;
    boost::uint32_t crc32_checksum;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    boost::uint16_t file_name_length;
    boost::uint16_t extra_field_length;
    boost::uint16_t comment_length;
    boost::uint16_t disk_number_start;
    boost::uint16_t internal_attributes;
    boost::uint32_t external_attributes;
    boost::uint32_t offset;
};

struct digital_signature
{
    static const boost::uint32_t signature = 0x05054B50;

    boost::uint16_t size;
};

struct end_of_central_directory
{
    static const boost::uint32_t signature = 0x06054B50;

    boost::uint16_t disk_number;
    boost::uint16_t start_disk_number;
    boost::uint16_t entries;
    boost::uint16_t total_entries;
    boost::uint32_t size;
    boost::uint32_t offset;
    boost::uint16_t comment_length;
};

struct extra_field_header
{
    boost::uint16_t id;
    boost::uint16_t size;
};

struct header
{
    boost::filesystem::path path;
    boost::uint16_t method;
    std::time_t update_time;
    boost::uint32_t crc32_checksum;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    boost::uint16_t attributes;
    boost::uint16_t permission;
    std::string comment;

    boost::optional<std::time_t> modified_time;
    boost::optional<std::time_t> access_time;
    boost::optional<std::time_t> creation_time;

    boost::optional<boost::uint16_t> uid;
    boost::optional<boost::uint16_t> gid;

    header()
        : method(0), update_time(0), crc32_checksum(0)
        , compressed_size(0), file_size(0)
        , attributes(msdos_attributes::archive), permission(0644)
    {
    }

    bool is_directory() const
    {
        return (attributes & msdos_attributes::directory) != 0;
    }
};

} } } // End namespaces zip, iostreams, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<iostreams::zip::local_file_header>
{
private:
    typedef iostreams::zip::local_file_header self;
    typedef iostreams::msdos_date_time date_time_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::needed_to_extract, little>,
        member<self, boost::uint16_t, &self::flags, little>,
        member<self, boost::uint16_t, &self::method, little>,
        member<self, date_time_type, &self::update_date_time>,
        member<self, boost::uint32_t, &self::crc32_checksum, little>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, boost::uint16_t, &self::file_name_length, little>,
        member<self, boost::uint16_t, &self::extra_field_length, little>
    > members;
};

template<>
struct struct_traits<iostreams::zip::data_descriptor>
{
private:
    typedef iostreams::zip::data_descriptor self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::crc32_checksum, little>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>
    > members;
};

template<>
struct struct_traits<iostreams::zip::archive_extra_data>
{
private:
    typedef iostreams::zip::archive_extra_data self;

public:
    typedef boost::mpl::single_view<
        member<self, boost::uint32_t, &self::extra_field_length, little>
    > members;
};

template<>
struct struct_traits<iostreams::zip::file_header>
{
private:
    typedef iostreams::zip::file_header self;
    typedef iostreams::msdos_date_time date_time_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::made_by, little>,
        member<self, boost::uint16_t, &self::needed_to_extract, little>,
        member<self, boost::uint16_t, &self::flags, little>,
        member<self, boost::uint16_t, &self::method, little>,
        member<self, date_time_type, &self::update_date_time>,
        member<self, boost::uint32_t, &self::crc32_checksum, little>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, boost::uint16_t, &self::file_name_length, little>,
        member<self, boost::uint16_t, &self::extra_field_length, little>,
        member<self, boost::uint16_t, &self::comment_length, little>,
        member<self, boost::uint16_t, &self::disk_number_start, little>,
        member<self, boost::uint16_t, &self::internal_attributes, little>,
        member<self, boost::uint32_t, &self::external_attributes, little>,
        member<self, boost::uint32_t, &self::offset, little>
    > members;
};

template<>
struct struct_traits<iostreams::zip::digital_signature>
{
private:
    typedef iostreams::zip::digital_signature self;

public:
    typedef boost::mpl::single_view<
        member<self, boost::uint16_t, &self::size, little>
    > members;
};

template<>
struct struct_traits<iostreams::zip::end_of_central_directory>
{
private:
    typedef iostreams::zip::end_of_central_directory self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::disk_number, little>,
        member<self, boost::uint16_t, &self::start_disk_number, little>,
        member<self, boost::uint16_t, &self::entries, little>,
        member<self, boost::uint16_t, &self::total_entries, little>,
        member<self, boost::uint32_t, &self::size, little>,
        member<self, boost::uint32_t, &self::offset, little>,
        member<self, boost::uint16_t, &self::comment_length, little>
    > members;
};

template<>
struct struct_traits<iostreams::zip::extra_field_header>
{
private:
    typedef iostreams::zip::extra_field_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::id, little>,
        member<self, boost::uint16_t, &self::size, little>
    > members;
};

} // namespace hamigaki

namespace hamigaki { namespace iostreams {

namespace zip
{

namespace detail
{

struct internal_header : public zip::header
{
    stream_offset offset;

    internal_header() : offset(-1)
    {
    }
};

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
    const stream_offset file_size =
        to_offset(boost::iostreams::seek(src, 0, BOOST_IOS::end));

    std::streamsize entry_size = static_cast<std::streamsize>(
        binary_size<end_of_central_directory>::type::value);
    stream_offset pos = file_size - entry_size - 4;

    boost::iostreams::seek(src, pos, BOOST_IOS::beg);
    boost::uint32_t signature = iostreams::read_uint32<little>(src);
    if (signature == end_of_central_directory::signature)
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
            stream_offset offset = (ptr+4) - (buf + size);
            boost::iostreams::seek(src, offset, BOOST_IOS::cur);
            return;
        }

    } while (pos != 0);

    throw BOOST_IOSTREAMS_FAILURE("cannot find ZIP footer");
}

template<class Source>
inline void read_local_extra_field(Source& src, header& head)
{
    extra_field_header ex_head;
    while (iostreams::binary_read(src, ex_head, std::nothrow))
    {
        boost::scoped_array<char> data(new char[ex_head.size]);
        if (ex_head.size)
            iostreams::blocking_read(src, data.get(), ex_head.size);
        if (ex_head.id == extra_field_id::extended_timestamp)
        {
            if (ex_head.size == 0)
                throw BOOST_IOSTREAMS_FAILURE("bad ZIP extended timestamp");

            unsigned char flags = static_cast<unsigned char>(data[0]);
            boost::uint16_t pos = 1;
            if ((flags & 0x01) != 0)
            {
                if (pos + 4 > ex_head.size)
                    throw BOOST_IOSTREAMS_FAILURE("bad ZIP extended timestamp");
                head.modified_time = static_cast<std::time_t>(
                    hamigaki::decode_int<little,4>(&data[pos]));
                pos += 4;
            }
            if ((flags & 0x02) != 0)
            {
                if (pos + 4 > ex_head.size)
                    throw BOOST_IOSTREAMS_FAILURE("bad ZIP extended timestamp");
                head.access_time = static_cast<std::time_t>(
                    hamigaki::decode_int<little,4>(&data[pos]));
                pos += 4;
            }
            if ((flags & 0x04) != 0)
            {
                if (pos + 4 > ex_head.size)
                    throw BOOST_IOSTREAMS_FAILURE("bad ZIP extended timestamp");
                head.creation_time = static_cast<std::time_t>(
                    hamigaki::decode_int<little,4>(&data[pos]));
            }
        }
        else if (ex_head.id == extra_field_id::info_zip_unix2)
        {
            head.uid = hamigaki::decode_int<little,2>(&data[0]);
            head.gid = hamigaki::decode_int<little,2>(&data[2]);
        }
    }
}

template<class Source>
inline void read_central_extra_field(Source& src, header& head)
{
    extra_field_header ex_head;
    while (iostreams::binary_read(src, ex_head, std::nothrow))
    {
        boost::scoped_array<char> data(new char[ex_head.size]);
        if (ex_head.size)
            iostreams::blocking_read(src, data.get(), ex_head.size);
        if (ex_head.id == extra_field_id::extended_timestamp)
        {
            if (ex_head.size == 0)
                throw BOOST_IOSTREAMS_FAILURE("bad ZIP extended timestamp");

            unsigned char flags = static_cast<unsigned char>(data[0]);
            if ((flags & 0x01) != 0)
            {
                if (5 > ex_head.size)
                    throw BOOST_IOSTREAMS_FAILURE("bad ZIP extended timestamp");
                head.modified_time = static_cast<std::time_t>(
                    hamigaki::decode_int<little,4>(&data[1]));
            }
        }
    }
}

} // namespace detail

} // namespace zip

template<class Source>
class basic_raw_zip_file_source_impl
{
public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

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

        zip::detail::internal_header head = headers_[next_index_++];
        boost::iostreams::seek(src_, head.offset, BOOST_IOS::beg);
        pos_ = 0;

        boost::uint32_t signature = iostreams::read_uint32<little>(src_);
        if (signature != zip::local_file_header::signature)
            throw BOOST_IOSTREAMS_FAILURE("bad ZIP signature");

        zip::local_file_header local;
        iostreams::binary_read(src_, local);

        head.method = local.method;
        head.update_time = local.update_date_time.to_time_t();
        if ((local.flags & zip::flags::has_data_dec) == 0)
        {
            head.crc32_checksum = local.crc32_checksum;
            head.compressed_size = local.compressed_size;
            head.file_size = local.file_size;
        }

        if (local.file_name_length != 0)
        {
            boost::scoped_array<char>
                filename(new char[local.file_name_length+1]);

            blocking_read(src_, &filename[0], local.file_name_length);
            filename[local.file_name_length] = '\0';
            head.path = path(&filename[0], no_check);
            if (filename[local.file_name_length-1] == '/')
                head.attributes |= msdos_attributes::directory;
        }

        if (local.extra_field_length)
        {
            boost::scoped_array<char> extra(new char[local.extra_field_length]);
            blocking_read(src_, &extra[0], local.extra_field_length);

            using boost::iostreams::array_source;
            typedef boost::iostreams::detail::
                direct_adapter<array_source> source_type;

            source_type src(array_source(
                extra.get(),
                static_cast<std::size_t>(local.extra_field_length)));

            zip::detail::read_local_extra_field(src, head);
        }

        header_ = head;

        return true;
    }

    zip::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if ((pos_ >= header_.compressed_size) || (n <= 0))
            return -1;

        boost::uint32_t rest = header_.compressed_size - pos_;
        std::streamsize amt =
            static_cast<std::streamsize>(
                (std::min)(static_cast<boost::uint32_t>(n), rest));

        blocking_read(src_, s, amt);
        pos_ += amt;
        return amt;
    }

private:
    Source src_;
    zip::header header_;
    boost::uint32_t pos_;
    std::size_t next_index_;
    std::vector<zip::detail::internal_header> headers_;

    void read_central_dir()
    {
        using namespace boost::filesystem;
        std::vector<zip::detail::internal_header> tmp;

        zip::detail::seek_end_of_central_dir(src_);

        zip::end_of_central_directory footer;
        iostreams::binary_read(src_, footer);

        tmp.reserve(footer.entries);

        boost::iostreams::seek(src_, footer.offset, BOOST_IOS::beg);
        for (boost::uint16_t i = 0; i < footer.entries; ++i)
        {
            boost::uint32_t signature = iostreams::read_uint32<little>(src_);
            if (signature != zip::file_header::signature)
                throw BOOST_IOSTREAMS_FAILURE("bad ZIP signature");

            zip::file_header file_head;
            iostreams::binary_read(src_, file_head);

            zip::detail::internal_header head;
            head.method = file_head.method;
            head.update_time = file_head.update_date_time.to_time_t();
            head.compressed_size = file_head.compressed_size;
            head.crc32_checksum = file_head.crc32_checksum;
            head.file_size = file_head.file_size;
            head.attributes =
                static_cast<boost::uint16_t>(file_head.external_attributes);
            head.permission =
                static_cast<boost::uint16_t>(file_head.external_attributes>>16);
            head.offset = file_head.offset;

            if (file_head.file_name_length != 0)
            {
                boost::scoped_array<char>
                    filename(new char[file_head.file_name_length+1]);

                blocking_read(src_, &filename[0], file_head.file_name_length);
                filename[file_head.file_name_length] = '\0';
                head.path = path(&filename[0], no_check);
                if (filename[file_head.file_name_length-1] == '/')
                    head.attributes |= msdos_attributes::directory;
            }

            if (file_head.extra_field_length)
            {
                boost::scoped_array<char> extra(
                    new char[file_head.extra_field_length]);
                blocking_read(src_, &extra[0], file_head.extra_field_length);

                using boost::iostreams::array_source;
                typedef boost::iostreams::detail::
                    direct_adapter<array_source> source_type;

                source_type src(array_source(
                    extra.get(),
                    static_cast<std::size_t>(file_head.extra_field_length)));

                zip::detail::read_central_extra_field(src, head);
            }

            if (file_head.comment_length)
            {
                boost::scoped_array<char> comment(
                    new char[file_head.comment_length]);
                blocking_read(src_, &comment[0], file_head.comment_length);
                head.comment = std::string(
                    &comment[0], &comment[0]+file_head.comment_length);
            }

            tmp.push_back(head);
        }

        boost::iostreams::seek(src_, 0, BOOST_IOS::beg);
        tmp.swap(headers_);
    }
};

template<class Source>
class basic_raw_zip_file_source
{
private:
    typedef basic_raw_zip_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_raw_zip_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    zip::header header() const
    {
        return pimpl_->header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class raw_zip_file_source : public basic_raw_zip_file_source<file_source>
{
    typedef basic_raw_zip_file_source<file_source> base_type;

public:
    explicit raw_zip_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_RAW_ZIP_FILE_HPP
