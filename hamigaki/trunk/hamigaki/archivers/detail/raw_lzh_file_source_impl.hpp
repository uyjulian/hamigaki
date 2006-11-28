//  raw_lzh_file_source_impl.hpp: raw LZH file source implementation

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/lha/lzh_headers.hpp>
#include <hamigaki/checksum/sum8.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/relative_restrict.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/crc.hpp>
#include <boost/scoped_array.hpp>
#include <algorithm>
#include <stdexcept>

namespace hamigaki { namespace archivers { namespace detail {

class crc16_and_lha_checksum
{
public:
    explicit crc16_and_lha_checksum(boost::crc_16_type& crc) : crc_(crc)
    {
    }

    void process_byte(unsigned char byte)
    {
        crc_.process_byte(byte);
        cs_.process_byte(byte);
    }

    void process_bytes(void const* buffer, std::size_t byte_count)
    {
        crc_.process_bytes(buffer, byte_count);
        cs_.process_bytes(buffer, byte_count);
    }

    unsigned char checksum()
    {
        return cs_.checksum();
    }

private:
    boost::crc_16_type& crc_;
    hamigaki::checksum::sum8 cs_;
};

template<class Source>
class basic_raw_lzh_file_source_impl
{
private:
    typedef iostreams::relative_restriction<Source> restricted_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    explicit basic_raw_lzh_file_source_impl(const Source& src)
        : src_(src), pos_(0)
    {
        header_.compressed_size = 0;
    }

    bool next_entry()
    {
        if (boost::int64_t rest = header_.compressed_size - pos_)
            boost::iostreams::seek(src_, rest, BOOST_IOS::cur);
        pos_ = 0;
        header_ = lha::header();

        char buf[hamigaki::struct_size<lha::lv0_header>::type::value];
        if (!this->read_basic_header(src_, buf, sizeof(buf)))
            return false;

        boost::crc_16_type crc;

        boost::optional<restricted_type> hsrc;
        boost::uint16_t next_size = 0;
        char lv = buf[sizeof(buf)-1];
        if (lv == '\0')
        {
            hamigaki::checksum::sum8 cs;
            cs.process_bytes(buf+2, sizeof(buf)-2);

            lha::lv0_header lv0;
            hamigaki::binary_read(buf, lv0);

            const std::streamsize rest_size =
                static_cast<std::streamsize>(lv0.header_size - (sizeof(buf)-2));
            if (rest_size <= 0)
                throw std::runtime_error("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            header_.level = 0;
            header_.method = lv0.method;
            header_.compressed_size = lv0.compressed_size;
            header_.file_size = lv0.file_size;
            header_.update_time = lv0.update_date_time.to_time_t();
            header_.attributes = lv0.attributes;
            boost::tie(header_.path, header_.link_path) = read_path(*hsrc, cs);
            header_.crc16_checksum = read_optional_crc16(*hsrc, cs);
            if (header_.crc16_checksum)
                skip_unknown_header(*hsrc, cs);

            if (cs.checksum() != lv0.header_checksum)
                throw std::runtime_error("LZH header checksum missmatch");
        }
        else if (lv == '\x01')
        {
            crc.process_bytes(buf, 2);

            crc16_and_lha_checksum cs(crc);
            cs.process_bytes(buf+2, sizeof(buf)-2);

            lha::lv1_header lv1;
            hamigaki::binary_read(buf, lv1);

            const std::streamsize rest_size =
                static_cast<std::streamsize>(lv1.header_size - sizeof(buf));
            if (rest_size <= 0)
                throw std::runtime_error("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            header_.level = 1;
            header_.method = lv1.method;
            header_.compressed_size = lv1.skip_size;
            header_.file_size = lv1.file_size;
            header_.update_time = lv1.update_date_time.to_time_t();

            if (lv1.method == "-lhd-")
                header_.attributes = msdos::attributes::directory;
            else
                header_.attributes = msdos::attributes::archive;

            boost::tie(header_.path, header_.link_path) = read_path(*hsrc, cs);
            header_.crc16_checksum = read_little16(*hsrc, cs);
            header_.os = get(*hsrc, cs);

            skip_unknown_header(*hsrc, cs);
            next_size = read_little16(src_, cs);
            if (cs.checksum() != lv1.header_checksum)
                throw std::runtime_error("LZH header checksum missmatch");

            hsrc = restricted_type(src_, 0, -1);
        }
        else if (lv == '\x02')
        {
            crc.process_bytes(buf, sizeof(buf));

            lha::lv2_header lv2;
            hamigaki::binary_read(buf, lv2);

            const std::streamsize rest_size =
                static_cast<std::streamsize>(lv2.header_size - sizeof(buf));
            if (rest_size <= 0)
                throw std::runtime_error("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            header_.level = 2;
            header_.method = lv2.method;
            header_.compressed_size = lv2.compressed_size;
            header_.file_size = lv2.file_size;
            header_.update_time = static_cast<std::time_t>(lv2.update_time);

            if (lv2.method == "-lhd-")
                header_.attributes = msdos::attributes::directory;
            else
                header_.attributes = msdos::attributes::archive;

            header_.crc16_checksum = read_little16(*hsrc, crc);
            header_.os = get(*hsrc, crc);
            next_size = read_little16(*hsrc, crc);
        }
        else
            throw std::runtime_error("unsupported LZH header");

        read_extended_header(*hsrc, crc, next_size);

        if (lv == '\x01')
            header_.compressed_size -= iostreams::tell_offset(*hsrc);

        return true;
    }

    lha::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if ((pos_ >= header_.compressed_size) || (n <= 0))
            return -1;

        boost::int64_t rest = header_.compressed_size - pos_;
        std::streamsize amt =
            static_cast<std::streamsize>(
                (std::min)(static_cast<boost::int64_t>(n), rest));

        iostreams::blocking_read(src_, s, amt);
        pos_ += amt;
        return amt;
    }

private:
    Source src_;
    lha::header header_;
    boost::int64_t pos_;

    template<class OtherSource, class Checksum>
    static char get(OtherSource& src, Checksum& cs)
    {
        char c;
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);
        if (boost::iostreams::read(nb, &c, 1) != 1)
            throw boost::iostreams::detail::bad_read();
        cs.process_byte(c);
        return c;
    }

    template<class OtherSource, class Checksum>
    static boost::uint16_t read_little16(OtherSource& src, Checksum& cs)
    {
        char buf[2];
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);
        if (boost::iostreams::read(nb, buf, 2) != 2)
            throw boost::iostreams::detail::bad_read();
        cs.process_bytes(buf, sizeof(buf));
        return hamigaki::decode_uint<little,2>(buf);
    }

    static boost::filesystem::path parse_path_old(const std::string& s)
    {
        boost::filesystem::path ph;

        std::string::size_type pos = 0;
        std::string::size_type start = 0;
        while (pos < s.size())
        {
            unsigned char uc = static_cast<unsigned char>(s[pos]);
            if (((uc >  0x80) && (uc < 0xA0)) ||
                ((uc >= 0xE0) && (uc < 0xFD)) )
            {
                if (++pos == s.size())
                    break;
                ++pos;
            }
            else if (s[pos] == '\\')
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

    template<class OtherSource, class Checksum>
    static std::pair<boost::filesystem::path,boost::filesystem::path>
    read_path(OtherSource& src, Checksum& cs)
    {
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);

        char c;
        if (boost::iostreams::read(nb, &c, 1) != 1)
            throw boost::iostreams::detail::bad_read();
        cs.process_byte(c);

        std::streamsize count = static_cast<unsigned char>(c);
        if (count == 0)
            return std::pair<boost::filesystem::path,boost::filesystem::path>();
        boost::scoped_array<char> buffer(new char[count]);

        if (boost::iostreams::read(nb, buffer.get(), count) != count)
            throw boost::iostreams::detail::bad_read();
        cs.process_bytes(buffer.get(), count);

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

    template<class OtherSource, class Checksum>
    boost::optional<boost::uint16_t>
    read_optional_crc16(OtherSource& src, Checksum& cs)
    {
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);

        char buf[2];
        std::streamsize n = boost::iostreams::read(nb, buf, sizeof(buf));
        if (n != -1)
            cs.process_bytes(buf, n);

        if (n == 2)
            return hamigaki::decode_uint<little,2>(buf);
        else
            return boost::optional<boost::uint16_t>();
    }

    template<class OtherSource, class Checksum>
    void skip_unknown_header(OtherSource& src, Checksum& cs)
    {
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);

        char buf[256];
        std::streamsize n = boost::iostreams::read(nb, buf, sizeof(buf));
        if (n != -1)
            cs.process_bytes(buf, n);
    }

    static bool read_basic_header(
        Source& src, char* buffer, std::streamsize size)
    {
        boost::iostreams::non_blocking_adapter<Source> nb(src);

        std::streamsize amt = boost::iostreams::read(nb, buffer, size);

        if (amt < size)
        {
            if ((amt <= 0) || (buffer[0] != '\0'))
                throw std::runtime_error("LZH end-mark not found");
            return false;
        }

        if (buffer[0] == '\0')
            return false;

        return true;
    }

    static boost::uint16_t parse_common(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw std::runtime_error("bad LZH common extended header");

        boost::uint16_t header_crc = hamigaki::decode_uint<little,2>(s);

        s[0] = '\0';
        s[1] = '\0';

        return header_crc;
    }

    static boost::uint16_t parse_attributes(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw std::runtime_error("bad LZH attributes extended header");

        return hamigaki::decode_uint<little,2>(s);
    }

    static lha::windows::timestamp
    parse_windows_timestamp(char* s, boost::uint32_t n)
    {
        if (n < hamigaki::struct_size<lha::windows::timestamp>::type::value)
            throw std::runtime_error("bad LZH timestamp extended header");

        lha::windows::timestamp ts;
        hamigaki::binary_read(s, ts);
        return ts;
    }

    static std::pair<boost::int64_t,boost::int64_t>
    parse_file_size(char* s, boost::uint32_t n)
    {
        if (n < 16)
            throw std::runtime_error("bad LZH file size extended header");

        boost::int64_t comp = hamigaki::decode_int<little,8>(s);
        boost::int64_t org = hamigaki::decode_int<little,8>(s);
        return std::make_pair(comp, org);
    }

    static boost::uint32_t parse_code_page(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw std::runtime_error("bad LZH code page extended header");

        return hamigaki::decode_uint<little,4>(s);
    }

    static boost::uint16_t parse_unix_permission(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw std::runtime_error("bad LZH permission extended header");

        return hamigaki::decode_uint<little,2>(s);
    }

    static lha::unix::gid_uid
    parse_unix_owner(char* s, boost::uint32_t n)
    {
        if (n < hamigaki::struct_size<lha::unix::gid_uid>::type::value)
            throw std::runtime_error("bad LZH owner extended header");

        lha::unix::gid_uid owner;
        hamigaki::binary_read(s, owner);
        return owner;
    }

    static std::time_t parse_unix_timestamp(char* s, boost::uint32_t n)
    {
        if (n < 4)
            throw std::runtime_error("bad LZH timestamp extended header");

        return static_cast<std::time_t>(hamigaki::decode_int<little,4>(s));
    }

    static boost::filesystem::path parse_path(const std::string& s)
    {
        boost::filesystem::path ph;

        std::string::size_type pos = 0;
        std::string::size_type delim;

        while (delim = s.find('\xFF', pos), delim != std::string::npos)
        {
            ph /= s.substr(pos, delim-pos);
            pos = delim + 1;
        }

        if (pos != s.size())
            ph /= s.substr(pos);

        return ph;
    }

    template<class OtherSource>
    void read_extended_header(
        OtherSource& src, boost::crc_16_type& crc, boost::uint16_t next_size)
    {
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);

        std::string filename;
        std::string dirname;
        boost::optional<boost::uint16_t> header_crc;
        while (next_size)
        {
            if (next_size < 3)
                throw std::runtime_error("bad LZH extended header");

            boost::scoped_array<char> buf(new char[next_size]);
            const std::streamsize ssize =
                static_cast<std::streamsize>(next_size);
            if (boost::iostreams::read(nb, buf.get(), ssize) != ssize)
                throw std::runtime_error("bad LZH extended header");

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
            else if (buf[0] == '\x46')
                header_.code_page = parse_code_page(data, size);
            else if (buf[0] == '\x50')
                header_.permission = parse_unix_permission(data, size);
            else if (buf[0] == '\x51')
                header_.owner = parse_unix_owner(data, size);
            else if (buf[0] == '\x52')
                header_.group_name.assign(data, size);
            else if (buf[0] == '\x53')
                header_.user_name.assign(data, size);
            else if (buf[0] == '\x54')
                header_.update_time = parse_unix_timestamp(data, size);

            crc.process_bytes(buf.get(), next_size);
            next_size = hamigaki::decode_uint<little,2>(data+size);
        }

        if (header_crc)
        {
            if (crc.checksum() != header_crc.get())
                throw std::runtime_error("LZH header CRC missmatch");
        }

        if (!header_.path.empty() && !dirname.empty())
            filename = header_.path.leaf();

        if (!dirname.empty() && (dirname[dirname.size()-1] != '\xFF'))
            dirname.push_back('\xFF');

        std::string dir_file = dirname + filename;
        if (!dir_file.empty())
        {
            std::string::size_type delim = dir_file.find('|');
            if (delim != std::string::npos)
            {
                header_.path = parse_path(dir_file.substr(0, delim));
                header_.link_path = parse_path(dir_file.substr(delim+1));
            }
            else
                header_.path = parse_path(dir_file);
        }
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SOURCE_IMPL_HPP
