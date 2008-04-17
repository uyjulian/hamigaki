// lzh_header_parser.hpp: LZH header parser

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_LZH_HEADER_PARSER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_LZH_HEADER_PARSER_HPP

#include <hamigaki/archivers/lha/headers.hpp>
#include <hamigaki/checksum/sum8.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/relative_restrict.hpp>
#include <boost/iostreams/detail/adapter/direct_adapter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/crc.hpp>
#include <boost/scoped_array.hpp>
#include <algorithm>
#include <stdexcept>

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    #include <hamigaki/charset/code_page.hpp>
    #include <hamigaki/charset/utf16.hpp>
#endif

namespace hamigaki { namespace archivers { namespace lzh_detail {

inline boost::uint16_t parse_common(char* s, boost::uint32_t n)
{
    if (n < 2)
        throw std::runtime_error("bad LZH common extended header");

    boost::uint16_t header_crc = hamigaki::decode_uint<little,2>(s);

    s[0] = '\0';
    s[1] = '\0';

    return header_crc;
}

inline boost::uint16_t parse_attributes(char* s, boost::uint32_t n)
{
    if (n < 2)
        throw std::runtime_error("bad LZH attributes extended header");

    return hamigaki::decode_uint<little,2>(s);
}

inline lha::windows::timestamp
parse_windows_timestamp(char* s, boost::uint32_t n)
{
    if (n < hamigaki::struct_size<lha::windows::timestamp>::value)
        throw std::runtime_error("bad LZH timestamp extended header");

    lha::windows::timestamp ts;
    hamigaki::binary_read(s, ts);
    return ts;
}

inline std::pair<boost::int64_t,boost::int64_t>
parse_file_size(char* s, boost::uint32_t n)
{
    if (n < 16)
        throw std::runtime_error("bad LZH file size extended header");

    boost::int64_t comp = hamigaki::decode_int<little,8>(s);
    boost::int64_t org = hamigaki::decode_int<little,8>(s);
    return std::make_pair(comp, org);
}

inline boost::uint32_t parse_code_page(char* s, boost::uint32_t n)
{
    if (n < 2)
        throw std::runtime_error("bad LZH code page extended header");

    return hamigaki::decode_uint<little,4>(s);
}

inline boost::uint16_t parse_unix_permissions(char* s, boost::uint32_t n)
{
    if (n < 2)
        throw std::runtime_error("bad LZH permissions extended header");

    return hamigaki::decode_uint<little,2>(s);
}

inline lha::posix::gid_uid
parse_unix_owner(char* s, boost::uint32_t n)
{
    if (n < hamigaki::struct_size<lha::posix::gid_uid>::value)
        throw std::runtime_error("bad LZH owner extended header");

    lha::posix::gid_uid owner;
    hamigaki::binary_read(s, owner);
    return owner;
}

inline std::time_t parse_unix_timestamp(char* s, boost::uint32_t n)
{
    if (n < 4)
        throw std::runtime_error("bad LZH timestamp extended header");

    return static_cast<std::time_t>(hamigaki::decode_int<little,4>(s));
}

inline bool is_sjis_lead_byte(const char c)
{
    unsigned char uc = static_cast<unsigned char>(c);
    return ((uc > 0x80) && (uc < 0xA0)) || ((uc >= 0xE0) && (uc < 0xFD));
}

inline boost::filesystem::path parse_path_old(const std::string& s)
{
    boost::filesystem::path ph;

    std::string::size_type pos = 0;
    std::string::size_type start = 0;

    if (s[0] == '\\')
    {
        ph = "/";
        start = ++pos;
    }

    while (pos < s.size())
    {
        char c = s[pos];
        if (is_sjis_lead_byte(c))
        {
            if (++pos == s.size())
                break;
            ++pos;
        }
        else if (c == '\\')
        {
            ph /= s.substr(start, pos-start);
            start = ++pos;
        }
        else
            ++pos;
    }

    if (start != s.size())
        ph /= s.substr(start);

    return ph;
}

inline boost::filesystem::path parse_path(const std::string& s)
{
    boost::filesystem::path ph;

    std::string::size_type pos = 0;
    std::string::size_type delim;

    if (s[0] == '\xFF')
    {
        ph = "/";
        ++pos;
    }

    while (delim = s.find('\xFF', pos), delim != std::string::npos)
    {
        ph /= s.substr(pos, delim-pos);
        pos = delim + 1;
    }

    if (pos != s.size())
        ph /= s.substr(pos);

    return ph;
}

template<class Source>
inline std::pair<boost::filesystem::path,boost::filesystem::path>
read_path(Source& src)
{
    char c = iostreams::blocking_get(src);

    std::streamsize count = static_cast<unsigned char>(c);
    if (count == 0)
        return std::pair<boost::filesystem::path,boost::filesystem::path>();

    boost::scoped_array<char> buffer(new char[count]);
    iostreams::blocking_read(src, buffer.get(), count);

    const char* s = buffer.get();
    if (const char* delim =
        static_cast<const char*>(std::memchr(s, '|', count)))
    {
        return std::make_pair(
            parse_path_old(std::string(s, delim-s)),
            parse_path_old(std::string(delim+1, s+count-(delim+1)))
        );
    }
    else
    {
        return std::make_pair(
            parse_path_old(std::string(s, count)),
            boost::filesystem::path()
        );
    }
}

inline void set_path(
    lha::header& head,
    const std::pair<boost::filesystem::path,boost::filesystem::path>& paths)
{
    head.path = paths.first;
    head.link_path = paths.second;
}

inline void set_path(
    lha::header& head,
    std::string& filename, std::string& dirname,
    std::string&, std::string&)
{
    if (filename.empty())
        filename = head.path.leaf();

    if (!dirname.empty() && (dirname[dirname.size()-1] != '\xFF'))
        dirname.push_back('\xFF');

    std::string dir_file = dirname + filename;
    std::string::size_type delim = dir_file.find('|');
    if (delim != std::string::npos)
    {
        head.path = parse_path(dir_file.substr(0, delim));
        head.link_path = parse_path(dir_file.substr(delim+1));
    }
    else
        head.path = parse_path(dir_file);
}

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
inline boost::filesystem::wpath parse_path(const std::wstring& s)
{
    boost::filesystem::wpath ph;

    std::wstring::size_type pos = 0;
    std::wstring::size_type delim;

    if (s[0] == L'\xFFFF')
    {
        ph = L"/";
        ++pos;
    }

    while (delim = s.find(L'\xFFFF', pos), delim != std::wstring::npos)
    {
        ph /= s.substr(pos, delim-pos);
        pos = delim + 1;
    }

    if (pos != s.size())
        ph /= s.substr(pos);

    return ph;
}

inline boost::filesystem::wpath to_wide(
    const boost::filesystem::path& ph, unsigned code_page)
{
    boost::filesystem::wpath tmp;

    boost::filesystem::path::const_iterator beg = ph.begin();
    boost::filesystem::path::const_iterator end = ph.end();
    for ( ; beg != end; ++beg)
        tmp /= charset::from_code_page(*beg, code_page);

    return tmp;
}

inline void set_path(
    lha::wheader& head,
    const std::pair<boost::filesystem::path,boost::filesystem::path>& paths)
{
    unsigned code_page = 0;
    if (head.code_page)
        code_page = *head.code_page;

    head.path = lzh_detail::to_wide(paths.first, code_page);
    head.link_path = lzh_detail::to_wide(paths.second, code_page);
}

inline void set_path(
    lha::wheader& head,
    std::string& filename, std::string& dirname,
    std::string& wfilename, std::string& wdirname)
{
    unsigned code_page = 0;
    if (head.code_page)
        code_page = *head.code_page;

    std::wstring fname;
    if (wfilename.empty())
    {
        if (filename.empty())
            fname = head.path.leaf();
        else
            fname = charset::from_code_page(filename, code_page);
    }
    else
        fname = charset::from_utf16le(wfilename);

    boost::filesystem::wpath dir;
    if (wdirname.empty())
        dir = lzh_detail::to_wide(lzh_detail::parse_path(dirname), code_page);
    else
        dir = lzh_detail::parse_path(charset::from_utf16le(wdirname));

    std::wstring dir_file = (dir/fname).string();

    std::wstring::size_type delim = dir_file.find(L'|');
    if (delim != std::string::npos)
    {
        head.path = parse_path(dir_file.substr(0, delim));
        head.link_path = parse_path(dir_file.substr(delim+1));
    }
    else
        head.path = parse_path(dir_file);
}
#endif // !defined(BOOST_FILESYSTEM_NARROW_ONLY)

template<class Source>
inline boost::optional<boost::uint16_t> read_optional_crc16(Source& src)
{
    boost::iostreams::non_blocking_adapter<Source> nb(src);

    char buf[2];
    std::streamsize n = boost::iostreams::read(nb, buf, sizeof(buf));

    if (n == 2)
        return hamigaki::decode_uint<little,2>(buf);
    else
        return boost::optional<boost::uint16_t>();
}

template<class Source, class Path>
class lzh_header_parser
{
private:
    typedef iostreams::relative_restriction<Source> restricted_type;

public:
    typedef Path path_type;
    typedef lha::basic_header<Path> header_type;

    explicit lzh_header_parser(Source& src) : src_(src)
    {
    }

    bool parse()
    {
        typedef std::char_traits<char> traits;

        int nc = iostreams::blocking_get(src_, std::nothrow);
        if (traits::eq_int_type(nc, traits::eof()))
            throw std::runtime_error("LZH end-mark not found");
        char c = traits::to_char_type(nc);
        if (c == '\0')
            return false;

        buffer_[0] = c;
        iostreams::blocking_read(src_, &buffer_[1], sizeof(buffer_)-1);

        char lv = buffer_[sizeof(buffer_)-1];
        if (lv == '\x00')
            parse_lv0_header();
        else if (lv == '\x01')
            parse_lv1_header();
        else if (lv == '\x02')
            parse_lv2_header();
        else
            throw std::runtime_error("unsupported LZH header");

        return true;
    }

    header_type header() const
    {
        return header_;
    }

private:
    Source& src_;
    header_type header_;
    char buffer_[hamigaki::struct_size<lha::lv0_header>::value];
    boost::crc_16_type crc_;

    void parse_lv0_header()
    {
        hamigaki::checksum::sum8 cs;
        cs.process_bytes(buffer_+2, sizeof(buffer_)-2);

        lha::lv0_header lv0;
        hamigaki::binary_read(buffer_, lv0);

        if (lv0.header_size < sizeof(buffer_)-2)
            throw std::runtime_error("bad LZH header size");

        const std::size_t rest_size = lv0.header_size - (sizeof(buffer_)-2);
        boost::scoped_array<char> buffer(new char[rest_size]);
        iostreams::blocking_read(src_, buffer.get(), rest_size);

        using boost::iostreams::array_source;
        typedef boost::iostreams::detail::
            direct_adapter<array_source> source_type;

        source_type src(array_source(buffer.get(), rest_size));
        cs.process_bytes(buffer.get(), rest_size);
        if (cs.checksum() != lv0.header_checksum)
            throw std::runtime_error("LZH header checksum missmatch");

        header_.level = 0;
        header_.method = lv0.method;
        header_.compressed_size = lv0.compressed_size;
        header_.file_size = lv0.file_size;
        header_.update_time = lv0.update_date_time.to_time_t();
        header_.attributes = lv0.attributes;
        lzh_detail::set_path(header_, lzh_detail::read_path(src));
        header_.crc16_checksum = read_optional_crc16(src);
    }

    void parse_lv1_header()
    {
        hamigaki::checksum::sum8 cs;
        cs.process_bytes(buffer_+2, sizeof(buffer_)-2);

        lha::lv1_header lv1;
        hamigaki::binary_read(buffer_, lv1);
        crc_.process_bytes(buffer_, sizeof(buffer_));

        if (lv1.header_size < sizeof(buffer_)-2)
            throw std::runtime_error("bad LZH header size");

        const std::size_t rest_size = lv1.header_size - (sizeof(buffer_)-2);
        boost::scoped_array<char> buffer(new char[rest_size]);
        iostreams::blocking_read(src_, buffer.get(), rest_size);

        using boost::iostreams::array_source;
        typedef boost::iostreams::detail::
            direct_adapter<array_source> source_type;

        source_type src(array_source(buffer.get(), rest_size));
        cs.process_bytes(buffer.get(), rest_size);
        if (cs.checksum() != lv1.header_checksum)
            throw std::runtime_error("LZH header checksum missmatch");
        crc_.process_bytes(buffer.get(), rest_size);

        header_.level = 1;
        header_.method = lv1.method;
        header_.compressed_size = lv1.skip_size;
        header_.file_size = lv1.file_size;
        header_.update_time = lv1.update_date_time.to_time_t();

        lzh_detail::set_path(header_, lzh_detail::read_path(src));
        header_.crc16_checksum = iostreams::read_uint16<little>(src);
        header_.os = iostreams::blocking_get(src);

        boost::uint16_t next_size =
            hamigaki::decode_uint<little,2>(&buffer[rest_size-2]);

        restricted_type ex_src(src_, 0, lv1.skip_size);
        header_.compressed_size -= read_extended_header(ex_src, next_size);
    }

    void parse_lv2_header()
    {
        lha::lv2_header lv2;
        hamigaki::binary_read(buffer_, lv2);
        crc_.process_bytes(buffer_, sizeof(buffer_));

        lha::lv2_header_rest lv2rest;
        char buf[hamigaki::struct_size<lha::lv2_header_rest>::value];

        if (lv2.header_size < sizeof(buffer_)+sizeof(buf))
            throw std::runtime_error("bad LZH header size");

        iostreams::blocking_read(src_, buf, sizeof(buf));
        hamigaki::binary_read(buf, lv2rest);
        crc_.process_bytes(buf, sizeof(buf));

        header_.level = 2;
        header_.method = lv2.method;
        header_.compressed_size = lv2.compressed_size;
        header_.file_size = lv2.file_size;
        header_.update_time = static_cast<std::time_t>(lv2.update_time);
        header_.crc16_checksum = lv2rest.crc16_checksum;
        header_.os = lv2rest.os;
        boost::uint16_t next_size = lv2rest.next_size;

        const std::streamsize rest_size =
            static_cast<std::streamsize>(
                lv2.header_size - (sizeof(buffer_) + sizeof(buf))
            );

        restricted_type ex_src(src_, 0, rest_size);
        read_extended_header(ex_src, next_size);
    }

    boost::uint32_t skip_unknown_header(restricted_type& src)
    {
        boost::uint32_t total = 0;
        char buf[256];
        while (true)
        {
            std::streamsize n = boost::iostreams::read(src, buf, sizeof(buf));
            if (n == -1)
                break;
            crc_.process_bytes(buf, n);
            total += n;
        }
        return total;
    }

    boost::uint32_t read_extended_header(
        restricted_type& src, boost::uint16_t next_size)
    {
        boost::uint32_t total = 0;

        std::string filename;
        std::string dirname;
        std::string wfilename;
        std::string wdirname;
        boost::optional<boost::uint16_t> header_crc;
        while (next_size)
        {
            if (next_size < 3)
                throw std::runtime_error("bad LZH extended header");

            boost::scoped_array<char> buf(new char[next_size]);
            iostreams::blocking_read(src, buf.get(), next_size);
            total += next_size;

            char* data = buf.get()+1;
            boost::uint16_t size = next_size - 3;
            if (buf[0] == '\0')
                header_crc = parse_common(data, size);
            else if (buf[0] == '\x01')
                filename.assign(data, size);
            else if (buf[0] == '\x02')
                dirname.assign(data, size);
            else if (buf[0] == '\x3F')
                header_.comment.assign(data, size);
            else if (buf[0] == '\x40')
                header_.attributes = parse_attributes(data, size);
            else if (buf[0] == '\x41')
                header_.timestamp = parse_windows_timestamp(data, size);
            else if (buf[0] == '\x42')
            {
                boost::tie(header_.compressed_size, header_.file_size)
                    = parse_file_size(data, size);
            }
            else if (buf[0] == '\x44')
                wfilename.assign(data, size);
            else if (buf[0] == '\x45')
                wdirname.assign(data, size);
            else if (buf[0] == '\x46')
                header_.code_page = parse_code_page(data, size);
            else if (buf[0] == '\x50')
                header_.permissions = parse_unix_permissions(data, size);
            else if (buf[0] == '\x51')
                header_.owner = parse_unix_owner(data, size);
            else if (buf[0] == '\x52')
                header_.group_name.assign(data, size);
            else if (buf[0] == '\x53')
                header_.user_name.assign(data, size);
            else if (buf[0] == '\x54')
                header_.update_time = parse_unix_timestamp(data, size);

            crc_.process_bytes(buf.get(), next_size);
            next_size = hamigaki::decode_uint<little,2>(data+size);
        }

        if (header_.level == 2)
            total += skip_unknown_header(src);

        if (header_crc)
        {
            if (crc_.checksum() != header_crc.get())
                throw std::runtime_error("LZH header CRC missmatch");
        }

        lzh_detail::set_path(header_, filename, dirname, wfilename, wdirname);

        return total;
    }
};

} } } // End namespaces lzh_detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_LZH_HEADER_PARSER_HPP
