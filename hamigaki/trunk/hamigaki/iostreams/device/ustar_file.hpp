//  ustar_file.hpp: POSIX.1-1988 tar file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_USTAR_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_USTAR_FILE_HPP

#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/blocking.hpp>
#include <hamigaki/binary_io.hpp>
#include <hamigaki/oct_format.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <numeric>
#include <string>

namespace hamigaki { namespace iostreams { namespace tar {

struct mode
{
    static const boost::uint16_t set_uid        = 04000;
    static const boost::uint16_t set_gid        = 02000;
    static const boost::uint16_t sticky         = 01000;
    static const boost::uint16_t user_read      = 00400;
    static const boost::uint16_t user_write     = 00200;
    static const boost::uint16_t user_exec      = 00100;
    static const boost::uint16_t group_read     = 00040;
    static const boost::uint16_t group_write    = 00020;
    static const boost::uint16_t group_exec     = 00010;
    static const boost::uint16_t other_read     = 00004;
    static const boost::uint16_t other_write    = 00002;
    static const boost::uint16_t other_exec     = 00001;

    static const boost::uint16_t mask           = 07777;
};

struct type
{
    // POSIX.1-1988
    static const char regular       = '0';
    static const char link          = '1';
    static const char symbolic_link = '2';
    static const char char_device   = '3';
    static const char block_device  = '4';
    static const char directory     = '5';
    static const char fifo          = '6';
    static const char reserved      = '7';

    // GNU extension
    static const char long_link     = 'K';
    static const char long_name     = 'L';

    // POSIX.1-2001
    static const char global        = 'g';
    static const char extended      = 'x';
};

enum file_format
{
    ustar, pax, gnu
};

struct header
{
    boost::filesystem::path path;
    boost::uint16_t mode;
    boost::intmax_t uid;
    boost::intmax_t gid;
    boost::uintmax_t size;
    std::time_t modified_time;
    char type;
    boost::filesystem::path link_name;
    file_format format;
    std::string user_name;
    std::string group_name;
    boost::uint16_t dev_major;
    boost::uint16_t dev_minor;

    header()
        : mode(0644), uid(0), gid(0), size(0), modified_time(0)
        , type(tar::type::regular), format(gnu), dev_major(0), dev_minor(0)
    {
    }

    bool is_regular() const
    {
        return (type <= type::regular) || (type >= type::reserved);
    }

    bool is_device() const
    {
        return (type == type::char_device) || (type == type::block_device);
    }

    bool is_long() const
    {
        return (type == type::long_link) || (type == type::long_name);
    }

    std::string path_string() const
    {
        if (type == type::directory)
            return path.native_directory_string();
        else
            return path.native_file_string();
    }
};

struct raw_header
{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
};

} } } // End namespaces tar, iostreams, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<iostreams::tar::raw_header>
{
private:
    typedef iostreams::tar::raw_header self;

public:
    typedef boost::mpl::list<
        member<self, char[100], &self::name>,
        member<self, char[8], &self::mode>,
        member<self, char[8], &self::uid>,
        member<self, char[8], &self::gid>,
        member<self, char[12], &self::size>,
        member<self, char[12], &self::mtime>,
        member<self, char[8], &self::chksum>,
        member<self, char, &self::typeflag>,
        member<self, char[100], &self::linkname>,
        member<self, char[6], &self::magic>,
        member<self, char[2], &self::version>,
        member<self, char[32], &self::uname>,
        member<self, char[32], &self::gname>,
        member<self, char[8], &self::devmajor>,
        member<self, char[8], &self::devminor>,
        member<self, char[155], &self::prefix>
    > members;
};

} // namespace hamigaki

namespace hamigaki { namespace iostreams {

namespace tar
{

namespace detail
{

typedef boost::uint_t<17>::least uint17_t;

inline uint17_t cheksum(const void* block)
{
    const unsigned char* s = static_cast<const unsigned char*>(block);

    static const std::size_t chksum_offset =
        binary_offset<raw_header,char[8],&raw_header::chksum>::type::value;

    static const std::size_t typeflag_offset =
        binary_offset<raw_header,char,&raw_header::typeflag>::type::value;

    uint17_t chksum = 0;
    chksum = std::accumulate(s, s+chksum_offset, chksum);
    chksum += static_cast<uint17_t>(static_cast<unsigned char>(' ') * 8u);
    chksum = std::accumulate(s+typeflag_offset, s+512, chksum);
    return chksum;
}

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
inline T read_oct(const char (&s)[Size])
{
    static const char table[] = " ";
    const char* begin = &s[0];
    const char* end = begin + Size;
    const char* delim = std::find_first_of(begin, end, &table[0], &table[0]+2);
    if ((delim == begin) || (delim == end))
        throw BOOST_IOSTREAMS_FAILURE("invalid tar header");
    return from_oct<T,char>(begin, delim);
}

inline std::time_t read_negative_time_t(const char (&s)[12])
{
    boost::uint32_t tmp = 0;
    for (std::size_t i = 0; i < 12; ++i)
    {
        tmp <<= 8;
        tmp |= static_cast<unsigned char>(s[i]);
    }
    return static_cast<std::time_t>(static_cast<boost::int32_t>(tmp));
}

inline std::time_t read_time_t(const char (&s)[12])
{
    if (s[0] == '\xFF')
        return read_negative_time_t(s);
    else
        return static_cast<std::time_t>(read_oct<boost::uint32_t>(s));
}

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
inline void write_oct(char (&buf)[Size], T x)
{
    const std::string& s = to_oct<char,Size-1>(x);
    s.copy(buf, s.size());
}

inline void write_negative_time_t(char (&buf)[12], std::time_t t)
{
    boost::uint64_t tmp =
        static_cast<boost::uint64_t>(static_cast<boost::int64_t>(t));

    std::memset(buf, '\xFF', 4);
    for (std::size_t i = 12-1; i >= 4; --i)
    {
        buf[i] = static_cast<char>(static_cast<unsigned char>(tmp));
        tmp >>= 8;
    }
}

inline void write_time_t(char (&buf)[12], std::time_t t)
{
    if (t < 0)
        return write_negative_time_t(buf, t);
    else
        return write_oct(buf, static_cast<boost::uint32_t>(t));
}

} // namespace detail

inline header read_header(const char* block)
{
    using namespace boost::filesystem;

    raw_header raw;
    hamigaki::binary_read(block, raw);

    if (std::memcmp(raw.magic, "ustar", 5) != 0)
        throw BOOST_IOSTREAMS_FAILURE("unknown tar header format");

    if (!detail::is_valid(raw.uname) || !detail::is_valid(raw.gname))
        throw BOOST_IOSTREAMS_FAILURE("invalid tar header");

    const path leaf(detail::read_string(raw.name), no_check);
    const path branch(detail::read_string(raw.prefix), no_check);

    header head;
    head.path = branch / leaf;
    head.mode = detail::read_oct<boost::uint16_t>(raw.mode);
    head.uid = detail::read_oct<boost::uint32_t>(raw.uid);
    head.gid = detail::read_oct<boost::uint32_t>(raw.gid);
    head.size = detail::read_oct<boost::uint64_t>(raw.size);
    head.modified_time = detail::read_time_t(raw.mtime);

    detail::uint17_t chksum = detail::read_oct<detail::uint17_t>(raw.chksum);
    if (detail::cheksum(block) != chksum)
        throw BOOST_IOSTREAMS_FAILURE("invalid tar checksum");

    head.type = raw.typeflag ? raw.typeflag : '0';
    head.link_name = detail::read_string(raw.linkname);

    if (raw.magic[5] == ' ')
        head.format = gnu;
    else
        head.format = ustar;

    head.user_name = detail::read_c_string(raw.uname);
    head.group_name = detail::read_c_string(raw.gname);
    if ((head.format != gnu) || (head.is_device()))
    {
        head.dev_major = detail::read_oct<boost::uint16_t>(raw.devmajor);
        head.dev_minor = detail::read_oct<boost::uint16_t>(raw.devminor);
    }
    else
    {
        head.dev_major = 0;
        head.dev_minor = 0;
    }

    return head;
}

inline void write_header(char* block, const header& head)
{
    using namespace boost::filesystem;

    const std::string& leaf = head.path.leaf();
    std::string branch = head.path.branch_path().string();
    std::string linkname = head.link_name.string();

    raw_header raw;
    std::memset(&raw, 0, sizeof(raw));

    if (head.is_long())
        detail::write_string(raw.name, "././@LongLink");
    else
        detail::write_string(raw.name, leaf);

    detail::write_oct(raw.mode, head.mode);
    detail::write_oct(raw.uid, head.uid);
    detail::write_oct(raw.gid, head.gid);
    detail::write_oct(raw.size, head.size);
    detail::write_time_t(raw.mtime, head.modified_time);
    std::memset(raw.chksum, ' ', sizeof(raw.chksum));
    raw.typeflag = head.type;
    detail::write_string(raw.linkname, linkname);

    std::strcpy(raw.magic, "ustar");
    if (head.format == gnu)
    {
        raw.magic[5] = ' ';
        raw.version[0] = ' ';
        raw.version[1] = '\0';
    }
    else
        std::memcpy(raw.version, "00", 2);

    detail::write_c_string(raw.uname, head.user_name);
    detail::write_c_string(raw.gname, head.group_name);

    if ((head.format != gnu) || (head.dev_major != 0))
        detail::write_oct(raw.devmajor, head.dev_major);
    if ((head.format != gnu) || (head.dev_minor != 0))
        detail::write_oct(raw.devminor, head.dev_minor);

    detail::write_string(raw.prefix, branch);

    std::memset(block, 0, 512);
    hamigaki::binary_write(block, raw);

    const std::string& chksum = to_oct<char>(detail::cheksum(block));
    chksum.copy(raw.chksum, chksum.size());
    if (chksum.size() < sizeof(raw.chksum)-1)
        raw.chksum[chksum.size()] = '\0';

    hamigaki::binary_write(block, raw);
}

} // namespace tar

template<class Source>
class basic_ustar_file_source_impl
{
public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_ustar_file_source_impl(const Source& src)
        : src_(src), pos_(0)
    {
        header_.type = tar::type::directory;
        header_.size = 0;
    }

    bool next_entry()
    {
        if (header_.is_regular() && (pos_ < header_.size))
        {
            pos_ = ((pos_ + 511) / 512) * 512;
            while (pos_ < header_.size)
            {
                blocking_read(src_, block_, 512);
                pos_ += 512;
            }
        }
        pos_ = 0;

        blocking_read(src_, block_, 512);
        if (block_[0] == '\0')
        {
            blocking_read(src_, block_, 512);
            return false;
        }
        header_ = tar::read_header(block_);

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
            boost::uint64_t rest = header_.size - pos_;
            if (rest == 0)
                break;

            std::size_t offset = pos_ % 512;
            if (offset == 0)
                blocking_read(src_, block_, 512);

            rest = (std::min)(rest, static_cast<boost::uint64_t>(512-offset));

            std::streamsize amt = (std::min)(
                n-total, static_cast<std::streamsize>(rest));

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
    char block_[512];
};

template<class Source>
class basic_ustar_file_source
{
private:
    typedef basic_ustar_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_ustar_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    tar::header header() const
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

class ustar_file_source : public basic_ustar_file_source<file_source>
{
    typedef basic_ustar_file_source<file_source> base_type;

public:
    explicit ustar_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};


template<class Sink>
class basic_ustar_file_sink_impl
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

        write_header(block_, head);
        blocking_write(sink_, block_, 512);

        pos_ = 0;
        size_ = head.is_regular() ? head.size : 0;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if (pos_ + n > size_)
            throw BOOST_IOSTREAMS_FAILURE("out of tar entry size");

        std::streamsize total = 0;
        while (total < n)
        {
            std::size_t offset = pos_ % 512;

            std::streamsize amt = (std::min)(
                n-total, static_cast<std::streamsize>(512-offset));
            std::memcpy(&block_[offset], s+total, amt);

            total += amt;
            pos_ += amt;

            if (pos_ % 512 == 0)
                blocking_write(sink_, block_, 512);
        }

        return total;
    }

    void close()
    {
        if (pos_ != size_)
            throw BOOST_IOSTREAMS_FAILURE("tar entry size mismatch");

        std::size_t offset = pos_ % 512;
        if (offset != 0)
        {
            std::memset(&block_[offset], 0, 512-offset);
            blocking_write(sink_, block_, 512);
        }
    }

    void write_end_mark()
    {
        std::memset(block_, 0, sizeof(block_));
        blocking_write(sink_, block_, 512);
        blocking_write(sink_, block_, 512);
    }

private:
    Sink sink_;
    boost::uint64_t pos_;
    boost::uint64_t size_;
    char block_[512];
};

template<class Sink>
class basic_ustar_file_sink
{
private:
    typedef basic_ustar_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_ustar_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void create_entry(const tar::header& head)
    {
        pimpl_->create_entry(head);
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

class ustar_file_sink : public basic_ustar_file_sink<file_sink>
{
private:
    typedef basic_ustar_file_sink<file_sink> base_type;

public:
    explicit ustar_file_sink(const std::string& filename)
        : base_type(file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_USTAR_FILE_HPP
