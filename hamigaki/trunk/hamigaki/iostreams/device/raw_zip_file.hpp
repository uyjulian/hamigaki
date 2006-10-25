//  zip_file.hpp: Phil Katz Zip file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_RAW_ZIP_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_RAW_ZIP_FILE_HPP

#include <hamigaki/iostreams/detail/error.hpp>
#include <hamigaki/iostreams/detail/msdos_attributes.hpp>
#include <hamigaki/iostreams/detail/msdos_date_time.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/scoped_array.hpp>
#include <boost/optional.hpp>
#include <stdexcept>
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

    std::string path_string() const
    {
        if (is_directory())
            return path.native_directory_string();
        else
            return path.native_file_string();
    }
};

class header_path_match
{
public:
    explicit header_path_match(const boost::filesystem::path& ph) : path_(ph)
    {
    }

    bool operator()(const header& head) const
    {
        return head.path == path_;
    }

private:
    boost::filesystem::path path_;
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

    throw std::runtime_error("cannot find ZIP footer");
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

template<class Sink>
inline void write_local_ex_timestamp(Sink& sink, const header& head)
{
    extra_field_header ex_head;
    ex_head.id = extra_field_id::extended_timestamp;
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
inline void write_central_ex_timestamp(Sink& sink, const header& head)
{
    extra_field_header ex_head;
    ex_head.id = extra_field_id::extended_timestamp;
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
inline void write_local_extra_field(Sink& sink, const header& head)
{
    write_local_ex_timestamp(sink, head);

    if (head.uid && head.gid)
    {
        extra_field_header ex_head;
        ex_head.id = extra_field_id::info_zip_unix2;
        ex_head.size = 4;

        iostreams::binary_write(sink, ex_head);

        iostreams::write_uint16<little>(sink, head.uid.get());
        iostreams::write_uint16<little>(sink, head.gid.get());
    }
}

template<class Sink>
inline void write_central_extra_field(Sink& sink, const header& head)
{
    write_central_ex_timestamp(sink, head);

    if (head.uid && head.gid)
    {
        extra_field_header ex_head;
        ex_head.id = extra_field_id::info_zip_unix2;
        ex_head.size = 0;

        iostreams::binary_write(sink, ex_head);
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

        select_entry(next_index_);
        return true;
    }

    void select_entry(const boost::filesystem::path& ph)
    {
        typedef std::vector<zip::detail::internal_header> headers_type;
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
                throw std::runtime_error("bad ZIP signature");

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

    void select_entry(std::size_t index)
    {
        using namespace boost::filesystem;

        zip::detail::internal_header head = headers_[index];
        next_index_ = ++index;

        boost::iostreams::seek(src_, head.offset, BOOST_IOS::beg);
        pos_ = 0;

        boost::uint32_t signature = iostreams::read_uint32<little>(src_);
        if (signature != zip::local_file_header::signature)
            throw std::runtime_error("bad ZIP signature");

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

    void select_entry(const boost::filesystem::path& ph)
    {
        pimpl_->select_entry(ph);
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


template<class Sink>
class basic_raw_zip_file_sink_impl
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
            header_.method = 0;
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
        header_.method = 0;

        boost::iostreams::seek(sink_, header_.offset, BOOST_IOS::beg);
        write_local_file_header(header_);

        size_ = 0;
        overflow_ = false;
        headers_.back() = header_;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if (size_ + n > header_.file_size)
        {
            overflow_ = true;
            throw give_up_compression();
        }

        blocking_write(sink_, s, n);
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

        headers_.back() = header_;
    }

    void write_end_mark()
    {
        boost::uint32_t start_offset =
            static_cast<boost::uint32_t>(iostreams::tell_offset(sink_));

        for (std::size_t i = 0; i < headers_.size(); ++i)
        {
            const zip::detail::internal_header& head = headers_[i];

            std::string filename = head.path.string();
            if (head.is_directory())
                filename += '/';

            std::string extra_field;
            boost::iostreams::
                back_insert_device<std::string> ex_sink(extra_field);
            zip::detail::write_central_extra_field(ex_sink, head);

            zip::file_header file_head;
            file_head.made_by = 10; // TODO
            file_head.needed_to_extract = 10; // TODO
            file_head.flags = 0;
            file_head.method = head.method;
            file_head.update_date_time = msdos_date_time(head.update_time);
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
    zip::detail::internal_header header_;
    boost::uint32_t size_;
    bool overflow_;
    std::vector<zip::detail::internal_header> headers_;

    void write_local_file_header(const zip::header& head)
    {
        std::string filename = head.path.string();
        if (head.is_directory())
            filename += '/';

        std::string extra_field;
        boost::iostreams::back_insert_device<std::string> ex_sink(extra_field);
        zip::detail::write_local_extra_field(ex_sink, head);

        zip::local_file_header local;
        local.needed_to_extract = 10; // TODO
        local.flags = 0;
        local.method = head.method;
        local.update_date_time = msdos_date_time(head.update_time);
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

template<class Sink>
class basic_raw_zip_file_sink
{
private:
    typedef basic_raw_zip_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_raw_zip_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void create_entry(const zip::header& head)
    {
        pimpl_->create_entry(head);
    }

    void rewind_entry()
    {
        pimpl_->rewind_entry();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    void close()
    {
        pimpl_->close();
    }

    void write_end_mark()
    {
        pimpl_->write_end_mark();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class raw_zip_file_sink : public basic_raw_zip_file_sink<file_sink>
{
private:
    typedef basic_raw_zip_file_sink<file_sink> base_type;

public:
    explicit raw_zip_file_sink(const std::string& filename)
        : base_type(file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_RAW_ZIP_FILE_HPP
