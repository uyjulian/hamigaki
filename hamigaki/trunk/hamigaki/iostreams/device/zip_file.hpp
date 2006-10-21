//  zip_file.hpp: Phil Katz Zip file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP

#include <hamigaki/iostreams/detail/msdos_date_time.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/crc.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_array.hpp>

namespace hamigaki { namespace iostreams { namespace zip {

struct flags
{
    static const boost::uint16_t has_data_dec   = 0x0008;
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

struct central_directory_footer
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

struct header
{
    boost::filesystem::path path;
    bool directory;
    boost::uint16_t method;
    std::time_t update_time;
    boost::uint32_t crc32_checksum;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;

    header()
        : directory(false), method(0), update_time(0), crc32_checksum(0)
        , compressed_size(0), file_size(0)
    {
    }

    bool is_directory() const
    {
        return directory;
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
struct struct_traits<iostreams::zip::central_directory_footer>
{
private:
    typedef iostreams::zip::central_directory_footer self;

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

} // namespace hamigaki

namespace hamigaki { namespace iostreams {

namespace zip
{

namespace detail
{

inline boost::iostreams::zlib_params make_zlib_params()
{
    boost::iostreams::zlib_params params;
    params.noheader = true;
    return params;
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
        : src_(src), pos_(0)
    {
    }

    bool next_entry()
    {
        using namespace boost::filesystem;

        if (boost::uint32_t rest = header_.compressed_size - pos_)
            boost::iostreams::seek(src_, rest, BOOST_IOS::cur);
        pos_ = 0;

        boost::uint32_t signature = iostreams::read_uint32<little>(src_);
        if (signature != zip::local_file_header::signature)
            return false;

        zip::local_file_header local;
        iostreams::binary_read(src_, local);

        zip::header head;
        head.method = local.method;
        head.update_time = local.update_date_time.to_time_t();
        head.compressed_size = local.compressed_size;
        head.crc32_checksum = local.crc32_checksum;
        head.file_size = local.file_size;

        if (local.file_name_length != 0)
        {
            boost::scoped_array<char>
                filename(new char[local.file_name_length+1]);

            blocking_read(src_, &filename[0], local.file_name_length);
            filename[local.file_name_length] = '\0';
            head.path = path(&filename[0], no_check);
            head.directory = (filename[local.file_name_length-1] == '/');
        }

        // TODO
        if (local.extra_field_length)
        {
            boost::scoped_array<char> extra(new char[local.extra_field_length]);
            blocking_read(src_, &extra[0], local.extra_field_length);
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


template<class Source>
class basic_zip_file_source_impl
{
private:
    typedef basic_raw_zip_file_source_impl<Source> raw_type;

public:
    explicit basic_zip_file_source_impl(const Source& src)
        : raw_(src), method_(0)
        , zlib_(zip::detail::make_zlib_params())
    {
    }

    bool next_entry()
    {
        if (!raw_.next_entry())
            return false;

        const zip::header& head = raw_.header();
        method_ = head.method;

        if ((method_ != 0) && (method_ != 8))
            throw BOOST_IOSTREAMS_FAILURE("unsupported ZIP format");

        crc32_.reset();
        boost::iostreams::close(zlib_, boost::ref(raw_), BOOST_IOS::in);

        return true;
    }

    zip::header header() const
    {
        return raw_.header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize amt = read_impl(s, n);
        if (amt != -1)
            crc32_.process_bytes(s, amt);
        else if (crc32_.checksum() != raw_.header().crc32_checksum)
            throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");
        return amt;
    }

private:
    raw_type raw_;
    boost::uint16_t method_;
    boost::crc_32_type crc32_;
    boost::iostreams::zlib_decompressor zlib_;

    std::streamsize read_impl(char* s, std::streamsize n)
    {
        if (method_ == 0)
            return raw_.read(s, n);
        else if (method_ == 8)
            return boost::iostreams::read(zlib_, boost::ref(raw_), s, n);

        return -1;
    }
};

template<class Source>
class basic_zip_file_source
{
private:
    typedef basic_zip_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_zip_file_source(const Source& src)
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

class zip_file_source : public basic_zip_file_source<file_source>
{
    typedef basic_zip_file_source<file_source> base_type;

public:
    explicit zip_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_ZIP_FILE_HPP
