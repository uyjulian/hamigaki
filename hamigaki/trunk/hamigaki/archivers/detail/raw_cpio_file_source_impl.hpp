// raw_cpio_file_source_impl.hpp: raw cpio file source implementation

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_CPIO_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_CPIO_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/cpio/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/skip.hpp>
#include <hamigaki/hex_format.hpp>
#include <hamigaki/oct_format.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>

namespace hamigaki { namespace archivers { namespace detail {

template<class T, std::size_t Size>
inline T cpio_read_oct(const char (&s)[Size])
{
    return from_oct<T,char>(s, s + Size);
}

template<class T, std::size_t Size>
inline T cpio_read_hex(const char (&s)[Size])
{
    return from_hex<char>(s);
}

inline filesystem::device_number cpio_make_device(boost::uint16_t n)
{
    return filesystem::device_number((n >> 8) & 0xFFu, n & 0xFFu);
}

inline boost::uint16_t read_cpio_header(
    cpio::header& head, const cpio::raw_header& raw)
{
    head.format = cpio::posix;
    head.parent_device =
        cpio_make_device(cpio_read_oct<boost::uint16_t>(raw.dev));
    head.file_id = cpio_read_oct<boost::uint32_t>(raw.ino);
    head.permissions = cpio_read_oct<boost::uint16_t>(raw.mode);
    head.uid = cpio_read_oct<boost::uint32_t>(raw.uid);
    head.gid = cpio_read_oct<boost::uint32_t>(raw.gid);
    head.links = cpio_read_oct<boost::uint32_t>(raw.nlink);
    head.device = cpio_make_device(cpio_read_oct<boost::uint16_t>(raw.rdev));
    head.modified_time =
        static_cast<std::time_t>(cpio_read_oct<boost::int32_t>(raw.mtime));

    head.file_size = cpio_read_oct<boost::uint32_t>(raw.filesize);

    return cpio_read_oct<boost::uint16_t>(raw.namesize);
}

inline boost::uint16_t read_cpio_header(
    cpio::header& head, const cpio::binary_header& bin)
{
    head.format = cpio::binary;
    head.parent_device = cpio_make_device(bin.dev);
    head.file_id = bin.ino;
    head.permissions = bin.mode;
    head.uid = bin.uid;
    head.gid = bin.gid;
    head.links = bin.nlink;
    head.device = cpio_make_device(bin.rdev);

    boost::int32_t t =
        static_cast<boost::int32_t>(
            (static_cast<boost::uint32_t>(bin.mtime[0]) << 16) |
            (static_cast<boost::uint32_t>(bin.mtime[1])      )
        );
    head.modified_time = static_cast<std::time_t>(t);

    head.file_size =
        (static_cast<boost::uint32_t>(bin.filesize[0]) << 16) |
        (static_cast<boost::uint32_t>(bin.filesize[1])      ) ;

    return bin.namesize;
}

inline boost::uint16_t read_cpio_header(
    cpio::header& head, const cpio::svr4_header& raw)
{
    if (std::memcmp(raw.magic, "070701", 6) == 0)
        head.format = cpio::svr4;
    else
        head.format = cpio::svr4_chksum;

    head.file_id = cpio_read_hex<boost::uint32_t>(raw.ino);
    head.permissions =
        static_cast<boost::uint16_t>(cpio_read_hex<boost::uint32_t>(raw.mode));
    head.uid = cpio_read_hex<boost::uint32_t>(raw.uid);
    head.gid = cpio_read_hex<boost::uint32_t>(raw.gid);
    head.links = cpio_read_hex<boost::uint32_t>(raw.nlink);
    head.modified_time =
        static_cast<std::time_t>(cpio_read_hex<boost::int32_t>(raw.mtime));
    head.file_size = cpio_read_hex<boost::uint32_t>(raw.filesize);

    head.parent_device =
        filesystem::device_number(
            cpio_read_hex<boost::uint32_t>(raw.dev_major),
            cpio_read_hex<boost::uint32_t>(raw.dev_minor)
        );

    head.device =
        filesystem::device_number(
            cpio_read_hex<boost::uint32_t>(raw.rdev_major),
            cpio_read_hex<boost::uint32_t>(raw.rdev_minor)
        );

    if (head.format == cpio::svr4_chksum)
    {
        head.checksum = static_cast<boost::uint16_t>(
            cpio_read_hex<boost::uint32_t>(raw.checksum));
    }

    return static_cast<boost::uint16_t>(
        cpio_read_hex<boost::uint32_t>(raw.namesize)
    );
}

template<class Source>
class basic_raw_cpio_file_source_impl : private boost::noncopyable
{
public:
    explicit basic_raw_cpio_file_source_impl(const Source& src)
        : src_(src), pos_(0)
    {
        header_.file_size = 0;
    }

    bool next_entry()
    {
        if (boost::uint32_t rest = header_.file_size - pos_)
            iostreams::skip(src_, rest);
        pos_ = 0;

        header_ = read_header();
        if (header_.path == "TRAILER!!!")
            return false;
        else if (header_.is_symlink())
        {
            boost::scoped_array<char> buf(new char[header_.file_size+1]);
            this->read(buf.get(), header_.file_size);
            buf[header_.file_size] = '\0';
            header_.link_path = buf.get();

            pos_ = 0;
            header_.file_size = 0;
        }
        return true;
    }

    cpio::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if ((pos_ >= header_.file_size) || (n <= 0))
            return -1;

        boost::uint32_t rest = header_.file_size - pos_;
        std::streamsize amt = auto_min(n, rest);

        iostreams::blocking_read(src_, s, amt);
        pos_ += static_cast<boost::uint32_t>(amt);
        return amt;
    }

private:
    Source src_;
    cpio::header header_;
    boost::uint32_t pos_;

    cpio::header read_header()
    {
        cpio::header head;
        std::size_t name_size = 0;

        char magic[6];
        iostreams::blocking_read(src_, magic);

        if (std::memcmp(magic, "070707", sizeof(magic)) == 0)
        {
            char buf[hamigaki::struct_size<cpio::raw_header>::value];
            std::memcpy(buf, magic, sizeof(magic));
            iostreams::blocking_read(
                src_, buf+sizeof(magic), sizeof(buf)-sizeof(magic));

            cpio::raw_header raw;
            hamigaki::binary_read(buf, raw);
            name_size = read_cpio_header(head, raw);
        }
        else if (
            (std::memcmp(magic, "070701", sizeof(magic)) == 0) ||
            (std::memcmp(magic, "070702", sizeof(magic)) == 0) )
        {
            char buf[hamigaki::struct_size<cpio::svr4_header>::value];
            std::memcpy(buf, magic, sizeof(magic));
            iostreams::blocking_read(
                src_, buf+sizeof(magic), sizeof(buf)-sizeof(magic));

            cpio::svr4_header raw;
            hamigaki::binary_read(buf, raw);
            name_size = read_cpio_header(head, raw);
        }
        else if (hamigaki::decode_int<native,2>(magic) == 070707)
        {
            char buf[hamigaki::struct_size<cpio::binary_header>::value];
            std::memcpy(buf, magic, sizeof(magic));
            iostreams::blocking_read(
                src_, buf+sizeof(magic), sizeof(buf)-sizeof(magic));

            cpio::binary_header bin;
            hamigaki::binary_read(buf, bin);
            name_size = read_cpio_header(head, bin);
        }
        else
            throw BOOST_IOSTREAMS_FAILURE("unknown cpio header format");

        boost::scoped_array<char> buf(new char[name_size]);
        iostreams::blocking_read(src_, buf.get(), name_size);
        head.path = buf.get();

        return head;
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_CPIO_FILE_SOURCE_IMPL_HPP
