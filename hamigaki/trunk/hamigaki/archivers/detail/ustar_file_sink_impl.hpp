// ustar_file_sink_impl.hpp: POSIX.1-1988 tar file sink implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_USTAR_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_USTAR_FILE_SINK_IMPL_HPP

#include <boost/config.hpp>

#include <hamigaki/archivers/detail/path.hpp>
#include <hamigaki/archivers/detail/tar_checksum.hpp>
#include <hamigaki/archivers/tar/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/blocking.hpp>
#include <hamigaki/binary/binary_io.hpp>
#include <hamigaki/dec_format.hpp>
#include <hamigaki/oct_format.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <cstring>

#if defined(BOOST_WINDOWS)
    extern "C" __declspec(dllimport)
    unsigned long __stdcall GetCurrentProcessId();
#elif defined(BOOST_HAS_UNISTD_H)
    #include <unistd.h>
#endif

namespace hamigaki { namespace archivers { namespace tar_detail {

#if defined(BOOST_WINDOWS)
inline unsigned long get_pid()
{
    return ::GetCurrentProcessId();
}
#elif defined(BOOST_HAS_UNISTD_H)
inline pid_t get_pid()
{
    return ::getpid();
}
#else
inline int get_pid()
{
    return 0;
}
#endif

template<std::size_t Size>
inline void write_string(char (&buf)[Size], const std::string& s)
{
    if (s.size() > Size)
        throw BOOST_IOSTREAMS_FAILURE("invalid tar header");

    if (!s.empty())
        s.copy(buf, s.size());
}

template<std::size_t Size>
inline void write_c_string(char (&buf)[Size], const std::string& s)
{
    if (s.size() >= Size)
        throw BOOST_IOSTREAMS_FAILURE("invalid tar header");

    if (!s.empty())
        s.copy(buf, s.size());
}

template<std::size_t Size, class T>
inline void write_oct_impl(char (&buf)[Size], T x, boost::mpl::bool_<false>)
{
#if BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3003))
    std::string s = to_oct<char,Size-1>(x);
#else
    const std::string& s = to_oct<char,Size-1>(x);
#endif
    s.copy(buf, s.size());
}

template<std::size_t Size, class T>
inline void write_negative_oct(char (&buf)[Size], T x)
{
    if (std::size_t rest = Size-sizeof(T))
        std::memset(buf, '\xFF', rest);
    hamigaki::encode_int<big,sizeof(T)>(&buf[Size-sizeof(T)], x);
}

template<std::size_t Size, class T>
inline void write_oct_impl(char (&buf)[Size], T x, boost::mpl::bool_<true>)
{
    if (x < 0)
        return write_negative_oct(buf, x);
    else
        return write_oct_impl(buf, x, boost::mpl::bool_<false>());
}

template<std::size_t Size, class T>
inline void write_oct(char (&buf)[Size], T x)
{
    write_oct_impl(buf, x,
        boost::mpl::bool_<std::numeric_limits<T>::is_signed>());
}

inline std::string make_ex_header_name(const boost::filesystem::path& ph)
{
    std::string buf;
    if (detail::has_parent_path(ph))
        buf += ph.branch_path().string();
    else
        buf += '.';
    buf += "/PaxHeaders.";
    buf += to_dec<char>(get_pid());
    buf += '/';
    buf += ph.leaf();

    if (buf.size() > tar::raw_header::name_size)
        buf.resize(tar::raw_header::name_size);

    return buf;
}

inline void split_path(
    const boost::filesystem::path& ph, std::string& prefix, std::string& name)
{
    name = ph.string();
    prefix.clear();
    if (name.size() > tar::raw_header::name_size)
    {
        boost::filesystem::path::iterator beg = ph.begin();
        boost::filesystem::path::iterator end = ph.end();
        prefix = *(beg++);
        for ( ; beg != end; ++beg)
        {
            const std::string& s = *beg;
            if (prefix.size() + 1 + s.size() > tar::raw_header::prefix_size)
                break;
            prefix += '/';
            prefix += s;
        }

        name.clear();
        for ( ; beg != end; ++beg)
        {
            if (!name.empty())
                name += '/';
            name += *beg;
        }
    }
}

inline void write_tar_header(char* block, const tar::header& head)
{
    using namespace boost::filesystem;

    std::string linkname = head.link_path.string();

    tar::raw_header raw;
    std::memset(&raw, 0, sizeof(raw));

    std::string prefix;
    if (head.is_long())
        tar_detail::write_string(raw.name, "././@LongLink");
    else if (head.type_flag == tar::type_flag::extended)
    {
        tar_detail::write_string(
            raw.name, tar_detail::make_ex_header_name(head.path));
    }
    else
    {
        std::string name;
        split_path(head.path, prefix, name);
        tar_detail::write_string(raw.name, name);
    }

    tar_detail::write_oct(raw.mode, head.permissions);
    tar_detail::write_oct(raw.uid, head.uid);
    tar_detail::write_oct(raw.gid, head.gid);
    tar_detail::write_oct(raw.size, head.file_size);
    if (head.modified_time)
        tar_detail::write_oct(raw.mtime, head.modified_time->seconds);
    else
        tar_detail::write_oct(raw.mtime, static_cast<std::time_t>(0));
    std::memset(raw.chksum, ' ', sizeof(raw.chksum));
    raw.typeflag = head.type_flag;

    if (head.type_flag != tar::type_flag::extended)
        tar_detail::write_string(raw.linkname, linkname);

    std::strcpy(raw.magic, "ustar");
    if (head.format == tar::gnu)
    {
        raw.magic[5] = ' ';
        raw.version[0] = ' ';
        raw.version[1] = '\0';
    }
    else
        std::memcpy(raw.version, "00", 2);

    tar_detail::write_c_string(raw.uname, head.user_name);
    tar_detail::write_c_string(raw.gname, head.group_name);

    if ((head.format != tar::gnu) || (head.dev_major != 0))
        tar_detail::write_oct(raw.devmajor, head.dev_major);
    if ((head.format != tar::gnu) || (head.dev_minor != 0))
        tar_detail::write_oct(raw.devminor, head.dev_minor);

    tar_detail::write_string(raw.prefix, prefix);

    std::memset(block, 0, tar::raw_header::block_size);
    hamigaki::binary_write(block, raw);

    const std::string& chksum = to_oct<char>(detail::tar_checksum(block));
    chksum.copy(raw.chksum, chksum.size());
    if (chksum.size() < sizeof(raw.chksum)-1)
        raw.chksum[chksum.size()] = '\0';

    hamigaki::binary_write(block, raw);
}

} } } // End namespaces tar_detail, archivers, hamigaki.

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink>
class basic_ustar_file_sink_impl : private boost::noncopyable
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_ustar_file_sink_impl(const Sink& sink)
        : sink_(sink), pos_(0), size_(0)
    {
    }

    void create_entry(const tar::header& head)
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("tar entry size mismatch");

        tar_detail::write_tar_header(block_, head);
        iostreams::blocking_write(sink_, block_, sizeof(block_));

        pos_ = 0;
        size_ = head.is_regular() ? head.file_size : 0;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if (pos_ + n > size_)
            throw BOOST_IOSTREAMS_FAILURE("out of tar entry size");

        std::streamsize total = 0;
        while (total < n)
        {
            std::size_t offset = pos_ % sizeof(block_);

            std::streamsize amt =
                hamigaki::auto_min(n-total, sizeof(block_)-offset);
            std::memcpy(&block_[offset], s+total, amt);

            total += amt;
            pos_ += amt;

            if (pos_ % sizeof(block_) == 0)
                iostreams::blocking_write(sink_, block_, sizeof(block_));
        }

        return total;
    }

    void close()
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("tar entry size mismatch");

        std::size_t offset = pos_ % sizeof(block_);
        if (offset != 0)
        {
            std::memset(&block_[offset], 0, sizeof(block_)-offset);
            iostreams::blocking_write(sink_, block_, sizeof(block_));
        }
    }

    void close_archive()
    {
        std::memset(block_, 0, sizeof(block_));
        iostreams::blocking_write(sink_, block_, sizeof(block_));
        iostreams::blocking_write(sink_, block_, sizeof(block_));
        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    boost::uint64_t pos_;
    boost::uint64_t size_;
    char block_[tar::raw_header::block_size];
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_USTAR_FILE_SINK_IMPL_HPP
