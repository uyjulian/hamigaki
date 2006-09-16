//  lzh_file.hpp: LZH file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

// Note:
// The current implementation is fake.
// The real compressed files are not supported yet.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP

#include <hamigaki/iostreams/binary_io.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/mpl/list.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>

namespace hamigaki { namespace iostreams { namespace lha {

struct lv2_header
{
    boost::uint16_t header_size;
    char signature[5];
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    boost::int32_t update_time;
    boost::uint8_t reserved;
    boost::uint8_t level;
    boost::uint16_t crc;
    char os_type;
};

} } } // End namespaces lha, iostreams, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<iostreams::lha::lv2_header>
{
private:
    typedef iostreams::lha::lv2_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::header_size, little>,
        member<self, char[5], &self::signature>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, boost::int32_t, &self::update_time, little>,
        member<self, boost::uint8_t, &self::reserved>,
        member<self, boost::uint8_t, &self::level>,
        member<self, boost::uint16_t, &self::crc, little>,
        member<self, char, &self::os_type>
    > members;
};

} // End namespace hamigaki.

namespace hamigaki { namespace iostreams {

template<class Source>
class lzh_file_source
{
private:
    typedef boost::iostreams::restriction<Source> restricted_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit lzh_file_source(const Source& src) : src_(src), next_offset_(0)
    {
        // TODO: skip data before the archives

        if (!next_entry())
            throw BOOST_IOSTREAMS_FAILURE("bad LZH file");
    }

    bool next_entry()
    {
        boost::iostreams::seek(src_, next_offset_, std::ios_base::beg);

        lha::lv2_header head;
        boost::crc_16_type crc;
        if (!read_basic_header(src_, head, crc))
            return false;

        // currently support only level2 header
        if (head.level != 2)
            throw BOOST_IOSTREAMS_FAILURE("unsupported LZH header");

        is_directory_ = (std::memcmp(head.signature, "-lhd-", 5) == 0);
        checksum_ = head.crc;

        // currently support only lh0 format
        if (!is_directory_ && (std::memcmp(head.signature, "-lh0-", 5) != 0))
            throw BOOST_IOSTREAMS_FAILURE("unsupported LZH format");

        const std::size_t base_size =
            hamigaki::struct_size<lha::lv2_header>::type::value;

        read_extended_header(
            boost::iostreams::restrict(
                src_, 0, head.header_size - base_size),
            crc
        );

        next_offset_ =
            boost::iostreams::position_to_offset(
                boost::iostreams::seek(src_, 0, std::ios_base::cur)
            ) + head.compressed_size;

        if (is_directory_)
            image_ = boost::none;
        else
            image_ = boost::iostreams::restrict(src_, 0, head.compressed_size);

        crc_.reset();

        return true;
    }

    boost::filesystem::path path() const
    {
        return path_;
    }

    bool is_directory() const
    {
        return is_directory_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize result = image_->read(s, n);
        if (result > 0)
            crc_.process_bytes(s, result);
        if (result == -1)
        {
            if (crc_.checksum() != checksum_)
                throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");
        }
        return result;
    }

private:
    Source src_;
    boost::filesystem::path path_;
    bool is_directory_;
    boost::optional<restricted_type> image_;
    boost::iostreams::stream_offset next_offset_;
    boost::crc_16_type crc_;
    boost::uint16_t checksum_;

    static bool read_basic_header(
        Source& src, lha::lv2_header& head, boost::crc_16_type& crc)
    {
        boost::iostreams::non_blocking_adapter<Source> nb(src);

        char buf[hamigaki::struct_size<lha::lv2_header>::type::value];
        std::streamsize amt = boost::iostreams::read(nb, buf, sizeof(buf));

        if (amt < static_cast<std::streamsize>(sizeof(buf)))
        {
            if ((amt <= 0) || (buf[0] != '\0'))
                throw BOOST_IOSTREAMS_FAILURE("LZH end-mark not found");
            return false;
        }

        if (buf[0] == '\0')
            return false;

        hamigaki::binary_read(buf, head);
        crc.process_bytes(buf, sizeof(buf));

        return true;
    }

    template<class Source2>
    static boost::uint16_t read_little16(Source2& src, boost::crc_16_type& crc)
    {
        char buf[2];
        boost::iostreams::non_blocking_adapter<Source2> nb(src);
        if (boost::iostreams::read(nb, buf, 2) != 2)
            throw BOOST_IOSTREAMS_FAILURE("read_little16 error");
        crc.process_bytes(buf, sizeof(buf));
        return hamigaki::decode_uint<hamigaki::little, 2>(buf);
    }

    static boost::uint16_t parse_common(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw BOOST_IOSTREAMS_FAILURE("bad LZH common extended header");

        boost::uint16_t header_crc =
            hamigaki::decode_uint<hamigaki::little,2>(s);

        s[0] = '\0';
        s[1] = '\0';

        return header_crc;
    }

    static boost::filesystem::path
    parse_directory(const char* s, boost::uint32_t n)
    {
        if (s[n-1] != '\xFF')
            throw BOOST_IOSTREAMS_FAILURE("bad LZH directory extended header");

        const char* cur = s;
        const char* end = s + n;
        boost::filesystem::path ph;

        while (cur != end)
        {
            const char* delim =
                static_cast<const char*>(std::memchr(cur, '\xFF', end-cur));

            ph /= std::string(cur, delim-cur);
            cur = ++delim;
        }

        return ph;
    }

    template<class Source2>
    void read_extended_header(Source2& src, boost::crc_16_type& crc)
    {
        boost::iostreams::non_blocking_adapter<Source2> nb(src);

        std::string leaf;
        boost::filesystem::path branch;
        boost::optional<boost::uint16_t> header_crc;
        while (boost::uint16_t size = read_little16(src, crc))
        {
            if (size < 3)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH extended header");
            size -= 2;

            boost::scoped_array<char> buf(new char[size]);
            const std::streamsize ssize = static_cast<std::streamsize>(size);
            if (boost::iostreams::read(nb, buf.get(), ssize) != ssize)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH extended header");

            if (buf[0] == '\0')
                header_crc = parse_common(buf.get()+1, size-1);
            else if (buf[0] == '\1')
                leaf.assign(buf.get()+1, size-1);
            else if (buf[0] == '\2')
                branch = parse_directory(buf.get()+1, size-1);

            crc.process_bytes(buf.get(), size);
        }

        if (!header_crc)
        {
            throw BOOST_IOSTREAMS_FAILURE(
                "LZH common extended header not found");
        }

        if (crc.checksum() != header_crc.get())
            throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");

        path_ = branch / leaf;
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP
