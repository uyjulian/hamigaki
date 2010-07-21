// raw_cpio_file_sink_impl.hpp: raw cpio file sink implementation

// Copyright Takeshi Mouri 2006-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_CPIO_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_CPIO_FILE_SINK_IMPL_HPP

#include <boost/config.hpp>

#include <hamigaki/archivers/cpio/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <hamigaki/hex_format.hpp>
#include <hamigaki/oct_format.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/noncopyable.hpp>
#include <cstring>

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink>
inline iostreams::stream_offset try_seek_sink_impl(
    Sink& sink, iostreams::stream_offset off, BOOST_IOS::seekdir way,
    boost::mpl::bool_<true>)
{
    return iostreams::to_offset(boost::iostreams::seek(sink, off, way));
}

template<class Sink>
inline iostreams::stream_offset try_seek_sink_impl(
    Sink& sink, iostreams::stream_offset off, BOOST_IOS::seekdir way,
    boost::mpl::bool_<false>)
{
    return -1;
}

template<class Sink>
inline iostreams::stream_offset try_seek_sink(
    Sink& sink, iostreams::stream_offset off, BOOST_IOS::seekdir way)
{
    typedef boost::is_convertible<
        typename boost::iostreams::mode_of<Sink>::type,
        boost::iostreams::output_seekable
    > can_seek;

    return detail::try_seek_sink_impl(sink, off, way, can_seek());
}

template<std::size_t Size, class T>
inline void cpio_write_oct_impl(
    char (&buf)[Size], T x, boost::mpl::bool_<false>)
{
#if BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3003))
    std::string s = to_oct<char,Size>(x);
#else
    const std::string& s = to_oct<char,Size>(x);
#endif
    std::memset(buf, '0', sizeof(buf));
    s.copy(buf+(Size-s.size()), s.size());
}

template<std::size_t Size, class T>
inline void cpio_write_oct_impl(char (&buf)[Size], T x, boost::mpl::bool_<true>)
{
    if (x < 0)
        throw BOOST_IOSTREAMS_FAILURE("invalid cpio header");

    return cpio_write_oct_impl(buf, x, boost::mpl::bool_<false>());
}

template<std::size_t Size, class T>
inline void cpio_write_oct(char (&buf)[Size], T x)
{
    cpio_write_oct_impl(buf, x,
        boost::mpl::bool_<std::numeric_limits<T>::is_signed>());
}

template<std::size_t Size, class T>
inline void cpio_write_hex_impl(
    char (&buf)[Size], T x, boost::mpl::bool_<false>)
{
#if BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3003))
    std::string s = to_hex<char>(x, false);
#else
    const std::string& s = to_hex<char>(x, false);
#endif
    if (s.size() > Size)
        throw BOOST_IOSTREAMS_FAILURE("invalid cpio header");
    std::memset(buf, '0', sizeof(buf));
    s.copy(buf+(Size-s.size()), s.size());
}

template<std::size_t Size, class T>
inline void cpio_write_hex_impl(char (&buf)[Size], T x, boost::mpl::bool_<true>)
{
    if (x < 0)
        throw BOOST_IOSTREAMS_FAILURE("invalid cpio header");

    return cpio_write_hex_impl(buf, x, boost::mpl::bool_<false>());
}

template<std::size_t Size, class T>
inline void cpio_write_hex(char (&buf)[Size], T x)
{
    cpio_write_hex_impl(buf, x,
        boost::mpl::bool_<std::numeric_limits<T>::is_signed>());
}

inline boost::uint16_t to_cpio_dev_num(const filesystem::device_number& dev)
{
    return
        (static_cast<boost::uint16_t>(dev.major) << 8) |
        (static_cast<boost::uint16_t>(dev.minor)     ) ;
}

inline void write_cpio_header(cpio::raw_header& raw, const cpio::header& head)
{
    std::memset(&raw, 0, sizeof(raw));

    std::memcpy(raw.magic, "070707", 6);
    cpio_write_oct(raw.dev, detail::to_cpio_dev_num(head.parent_device));
    cpio_write_oct(raw.ino, head.file_id);
    cpio_write_oct(raw.mode, head.permissions);
    cpio_write_oct(raw.uid, head.uid);
    cpio_write_oct(raw.gid, head.gid);
    cpio_write_oct(raw.nlink, head.links);
    cpio_write_oct(raw.rdev, detail::to_cpio_dev_num(head.device));
    cpio_write_oct(raw.mtime, static_cast<boost::int32_t>(head.modified_time));
    cpio_write_oct(raw.namesize, head.path.string().size()+1);
    cpio_write_oct(raw.filesize, head.file_size);
}

inline void write_cpio_header(
    cpio::binary_header& bin, const cpio::header& head)
{
    std::memset(&bin, 0, sizeof(bin));

    bin.magic = 070707;
    bin.dev = detail::to_cpio_dev_num(head.parent_device);
    bin.ino = static_cast<boost::uint16_t>(head.file_id);
    bin.mode = head.permissions;
    bin.uid = static_cast<boost::uint16_t>(head.uid);
    bin.gid = static_cast<boost::uint16_t>(head.gid);
    bin.nlink = static_cast<boost::uint16_t>(head.links);
    bin.rdev = detail::to_cpio_dev_num(head.device);

    boost::uint32_t t =
        static_cast<boost::uint32_t>(
            static_cast<boost::int32_t>(head.modified_time)
        );
    bin.mtime[0] = static_cast<boost::uint16_t>(t >> 16);
    bin.mtime[1] = static_cast<boost::uint16_t>(t & 0xFFFF);

    bin.namesize = static_cast<boost::uint16_t>(head.path.string().size()+1);

    bin.filesize[0] = static_cast<boost::uint16_t>(head.file_size >> 16);
    bin.filesize[1] = static_cast<boost::uint16_t>(head.file_size & 0xFFFF);
}

inline void write_cpio_header(cpio::svr4_header& raw, const cpio::header& head)
{
    std::memset(&raw, 0, sizeof(raw));

    if (head.format == cpio::svr4)
        std::memcpy(raw.magic, "070701", 6);
    else
        std::memcpy(raw.magic, "070702", 6);

    cpio_write_hex(raw.ino, head.file_id);
    cpio_write_hex(raw.mode, head.permissions);
    cpio_write_hex(raw.uid, head.uid);
    cpio_write_hex(raw.gid, head.gid);
    cpio_write_hex(raw.nlink, head.links);
    cpio_write_hex(raw.mtime, static_cast<boost::int32_t>(head.modified_time));
    cpio_write_hex(raw.filesize, head.file_size);

    cpio_write_hex(raw.dev_major,
        static_cast<boost::uint16_t>(head.parent_device.major));
    cpio_write_hex(raw.dev_minor,
        static_cast<boost::uint16_t>(head.parent_device.minor));

    cpio_write_hex(raw.rdev_major,
        static_cast<boost::uint16_t>(head.device.major));
    cpio_write_hex(raw.rdev_minor,
        static_cast<boost::uint16_t>(head.device.minor));

    cpio_write_hex(raw.namesize,
        static_cast<boost::uint32_t>(head.path.string().size()+1));

    if ((head.format == cpio::svr4_chksum) && head.checksum)
        cpio_write_hex(raw.checksum, *head.checksum);
    else
        std::memset(raw.checksum, '0', 8);
}

template<class Sink>
class basic_raw_cpio_file_sink_impl : private boost::noncopyable
{
public:
    explicit basic_raw_cpio_file_sink_impl(const Sink& sink)
        : sink_(sink), pos_(0), size_(0), format_(cpio::posix)
        , chksum_offset_(-1)
    {
    }

    void create_entry(const cpio::header& head)
    {
        using namespace boost::filesystem;

        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("cpio entry size mismatch");

        format_ = head.format;

        cpio::header local = head;
        std::string link_path = local.link_path.string();
        if (local.is_symlink())
            local.file_size = static_cast<boost::uint32_t>(link_path.size());

        write_header(local);

        pos_ = 0;
        size_ = local.file_size;

        if (local.is_symlink())
            this->write(link_path.c_str(), link_path.size());
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if (pos_ + n > size_)
            throw BOOST_IOSTREAMS_FAILURE("out of cpio entry size");

        iostreams::blocking_write(sink_, s, n);
        pos_ += static_cast<boost::uint32_t>(n);

        return n;
    }

    void close()
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("cpio entry size mismatch");
    }

    void close(boost::uint16_t checksum)
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("cpio entry size mismatch");

        if (format_ == cpio::svr4_chksum)
        {
            iostreams::stream_offset next =
                detail::try_seek_sink(sink_, 0, BOOST_IOS::cur);

            char buf[8];
            cpio_write_hex(buf, checksum);
            iostreams::blocking_write(sink_, buf);

            detail::try_seek_sink(sink_, next, BOOST_IOS::beg);
        }
    }

    void close_archive()
    {
        cpio::header head;
        head.format = format_;
        head.permissions = 0;
        head.path = "TRAILER!!!";
        create_entry(head);

        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    boost::uint32_t pos_;
    boost::uint32_t size_;
    cpio::file_format format_;
    iostreams::stream_offset chksum_offset_;

    void write_header(const cpio::header& head)
    {
        using namespace boost::filesystem;

        if (head.format == cpio::posix)
        {
            cpio::raw_header raw;
            detail::write_cpio_header(raw, head);
            iostreams::binary_write(sink_, raw);
        }
        else if (head.format == cpio::svr4)
        {
            cpio::svr4_header raw;
            detail::write_cpio_header(raw, head);
            iostreams::binary_write(sink_, raw);
        }
        else if (head.format == cpio::svr4_chksum)
        {
            chksum_offset_ = detail::try_seek_sink(sink_, 0, BOOST_IOS::cur);
            if (chksum_offset_ != -1)
            {
                std::size_t offset =
                    binary_offset<
                        cpio::svr4_header,
                        char[8],&cpio::svr4_header::checksum
                    >::value;
                chksum_offset_ += offset;
            }
            else if (!head.checksum)
                throw BOOST_IOSTREAMS_FAILURE("cpio checksum needed");

            cpio::svr4_header raw;
            detail::write_cpio_header(raw, head);
            iostreams::binary_write(sink_, raw);
        }
        else if (head.format == cpio::binary)
        {
            cpio::binary_header bin;
            detail::write_cpio_header(bin, head);
            iostreams::binary_write(sink_, bin);
        }
        else
            throw BOOST_IOSTREAMS_FAILURE("unknown cpio header format");

        std::string filename = head.path.string();
        iostreams::blocking_write(sink_, filename.c_str(), filename.size()+1);
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_CPIO_FILE_SINK_IMPL_HPP
