//  cpio_file_sink_impl.hpp: POSIX cpio file sink implementation

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SINK_IMPL_HPP

#include <boost/config.hpp>

#include <hamigaki/archivers/cpio/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/oct_format.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/noncopyable.hpp>
#include <cstring>

namespace hamigaki { namespace archivers { namespace detail {

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

inline void write_cpio_header(cpio::raw_header& raw, const cpio::header& head)
{
    std::memset(&raw, 0, sizeof(raw));

    std::memcpy(raw.magic, "070707", 6);
    cpio_write_oct(raw.dev, head.parent_device_id);
    cpio_write_oct(raw.ino, head.file_id);
    cpio_write_oct(raw.mode, head.permissions);
    cpio_write_oct(raw.uid, head.uid);
    cpio_write_oct(raw.gid, head.gid);
    cpio_write_oct(raw.nlink, head.links);
    cpio_write_oct(raw.rdev, head.device_id);
    cpio_write_oct(raw.mtime, static_cast<boost::int32_t>(head.modified_time));
    cpio_write_oct(raw.namesize, head.path.string().size()+1);
    cpio_write_oct(raw.filesize, head.file_size);
}

inline void write_cpio_header(
    cpio::binary_header& bin, const cpio::header& head)
{
    std::memset(&bin, 0, sizeof(bin));

    bin.magic = 070707;
    bin.dev = head.parent_device_id;
    bin.ino = head.file_id;
    bin.mode = head.permissions;
    bin.uid = head.uid;
    bin.gid = head.gid;
    bin.nlink = head.links;
    bin.rdev = head.device_id;

    boost::uint32_t t =
        static_cast<boost::uint32_t>(
            static_cast<boost::int32_t>(head.modified_time)
        );
    bin.mtime[0] = static_cast<boost::uint16_t>(t >> 16);
    bin.mtime[1] = static_cast<boost::uint16_t>(t & 0xFFFF);

    bin.namesize = head.path.string().size()+1;

    bin.filesize[0] = static_cast<boost::uint16_t>(head.file_size >> 16);
    bin.filesize[1] = static_cast<boost::uint16_t>(head.file_size & 0xFFFF);
}

template<class Sink>
class basic_cpio_file_sink_impl : private boost::noncopyable
{
public:
    explicit basic_cpio_file_sink_impl(const Sink& sink)
        : sink_(sink), pos_(0), size_(0), format_(cpio::posix)
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
            local.file_size = link_path.size();

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
        pos_ += n;

        return n;
    }

    void close()
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("cpio entry size mismatch");
    }

    void close_archive()
    {
        using namespace boost::filesystem;

        cpio::header head;
        head.format = format_;
        head.permissions = 0;
        head.path = path("TRAILER!!!", no_check);
        create_entry(head);

        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    boost::uint32_t pos_;
    boost::uint32_t size_;
    cpio::file_format format_;

    void write_header(const cpio::header& head)
    {
        using namespace boost::filesystem;

        if (head.format == cpio::posix)
        {
            cpio::raw_header raw;
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

#endif // HAMIGAKI_ARCHIVERS_DETAIL_CPIO_FILE_SINK_IMPL_HPP
