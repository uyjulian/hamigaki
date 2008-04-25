// ustar_file_source_impl.hpp: POSIX.1-1988 tar file source implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_USTAR_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_USTAR_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/tar_checksum.hpp>
#include <hamigaki/archivers/tar/headers.hpp>
#include <hamigaki/integer/auto_min.hpp>
#include <hamigaki/iostreams/blocking.hpp>
#include <hamigaki/binary/binary_io.hpp>
#include <hamigaki/dec_format.hpp>
#include <hamigaki/oct_format.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <cstring>

namespace hamigaki { namespace archivers { namespace tar_detail {

template<std::size_t Size>
inline bool is_valid(const char (&s)[Size])
{
    return std::memchr(&s[0], '\0', Size) != 0;
}

template<std::size_t Size>
inline const char* find_end(const char (&s)[Size])
{
    return std::find(&s[0], &s[0] + Size, '\0');
}

template<std::size_t Size>
inline std::string read_string(const char (&s)[Size])
{
    return std::string(&s[0], find_end(s));
}

template<std::size_t Size>
inline std::string read_c_string(const char (&s)[Size])
{
    if (std::memchr(&s[0], '\0', Size) == 0)
        throw BOOST_IOSTREAMS_FAILURE("invalid tar header");
    return std::string(&s[0]);
}

template<class T, std::size_t Size>
inline T read_oct_impl(const char (&s)[Size], boost::mpl::bool_<false>)
{
    static const char table[] = " ";
    const char* begin = &s[0];
    const char* end = begin + Size;
    while ((begin != end) && (*begin == ' '))
        ++begin;
    const char* delim = std::find_first_of(begin, end, &table[0], &table[0]+2);
    if (delim == end)
        throw BOOST_IOSTREAMS_FAILURE("invalid tar header");
    if (delim == begin)
        return 0;
    return from_oct<T,char>(begin, delim);
}

template<class T, std::size_t Size>
inline T read_negative_oct(const char (&s)[Size])
{
    return static_cast<T>(
        hamigaki::decode_uint<big,sizeof(T)>(&s[Size-sizeof(T)]));
}

template<class T, std::size_t Size>
inline T read_oct_impl(const char (&s)[Size], boost::mpl::bool_<true>)
{
    if ((static_cast<unsigned char>(s[0]) & 0x80) != 0)
        return read_negative_oct<T>(s);
    else
        return read_oct_impl<T>(s, boost::mpl::bool_<false>());
}

template<class T, std::size_t Size>
inline T read_oct(const char (&s)[Size])
{
    return read_oct_impl<T,Size>(
        s, boost::mpl::bool_<std::numeric_limits<T>::is_signed>());
}

inline tar::header read_tar_header(const char* block)
{
    tar::raw_header raw;
    hamigaki::binary_read(block, raw);

    if (std::memcmp(raw.magic, "ustar", 5) != 0)
        throw BOOST_IOSTREAMS_FAILURE("unknown tar header format");

    if (!tar_detail::is_valid(raw.uname) || !tar_detail::is_valid(raw.gname))
        throw BOOST_IOSTREAMS_FAILURE("invalid tar header");

    const boost::filesystem::path name(tar_detail::read_string(raw.name));
    const boost::filesystem::path prefix(tar_detail::read_string(raw.prefix));

    tar::header head;

    if (raw.magic[5] == ' ')
    {
        head.format = tar::gnu;
        head.path = name;
    }
    else
    {
        head.format = tar::ustar;
        head.path = prefix / name;
    }

    head.permissions = tar_detail::read_oct<boost::uint16_t>(raw.mode);
    head.uid = tar_detail::read_oct<boost::int32_t>(raw.uid);
    head.gid = tar_detail::read_oct<boost::int32_t>(raw.gid);
    head.file_size = tar_detail::read_oct<boost::uint64_t>(raw.size);
    head.modified_time =
        filesystem::timestamp::from_time_t(
            tar_detail::read_oct<std::time_t>(raw.mtime));

    detail::uint17_t chksum =
        tar_detail::read_oct<detail::uint17_t>(raw.chksum);
    if (detail::tar_checksum(block) != chksum)
        throw BOOST_IOSTREAMS_FAILURE("invalid tar checksum");

    head.type_flag = raw.typeflag ? raw.typeflag : '0';
    head.link_path = tar_detail::read_string(raw.linkname);

    head.user_name = tar_detail::read_c_string(raw.uname);
    head.group_name = tar_detail::read_c_string(raw.gname);
    if ((head.format != tar::gnu) || (head.is_device()))
    {
        head.dev_major = tar_detail::read_oct<boost::uint16_t>(raw.devmajor);
        head.dev_minor = tar_detail::read_oct<boost::uint16_t>(raw.devminor);
    }
    else
    {
        head.dev_major = 0;
        head.dev_minor = 0;
    }

    return head;
}

} } } // End namespaces tar_detail, archivers, hamigaki.

namespace hamigaki { namespace archivers { namespace detail {

template<class Source>
class basic_ustar_file_source_impl : private boost::noncopyable
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    explicit basic_ustar_file_source_impl(const Source& src)
        : src_(src), pos_(0)
    {
        header_.type_flag = tar::type_flag::directory;
        header_.file_size = 0;
    }

    bool next_entry()
    {
        if (header_.is_regular() && (pos_ < header_.file_size))
        {
            pos_ = tar::raw_header::round_up_block_size(pos_);
            while (pos_ < header_.file_size)
            {
                iostreams::blocking_read(src_, block_, sizeof(block_));
                pos_ += tar::raw_header::block_size;
            }
        }
        pos_ = 0;

        iostreams::blocking_read(src_, block_, sizeof(block_));
        if (block_[0] == '\0')
        {
            iostreams::blocking_read(src_, block_, sizeof(block_));
            return false;
        }
        header_ = tar_detail::read_tar_header(block_);

        return true;
    }

    tar::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if (!header_.is_regular())
            return -1;

        std::streamsize total = 0;
        while (total < n)
        {
            boost::uint64_t rest = header_.file_size - pos_;
            if (rest == 0)
                break;

            std::size_t offset = pos_ % sizeof(block_);
            if (offset == 0)
                iostreams::blocking_read(src_, block_, sizeof(block_));

            rest = hamigaki::auto_min(rest, 512-offset);
            std::streamsize amt = hamigaki::auto_min(n-total, rest);

            std::memcpy(s+total, &block_[offset], amt);
            total += amt;
            pos_ += amt;
        }

        return total != 0 ? total : -1;
    }

private:
    Source src_;
    tar::header header_;
    boost::uint64_t pos_;
    char block_[tar::raw_header::block_size];
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_USTAR_FILE_SOURCE_IMPL_HPP
