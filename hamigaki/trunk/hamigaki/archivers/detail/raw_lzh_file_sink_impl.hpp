// raw_lzh_file_sink_impl.hpp: raw LZH file sink implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/path.hpp>
#include <hamigaki/archivers/lha/headers.hpp>
#include <hamigaki/archivers/error.hpp>
#include <hamigaki/checksum/sum8.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <hamigaki/static_widen.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/crc.hpp>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    #include <hamigaki/archivers/detail/code_page.hpp>
    #include <hamigaki/charset/utf16.hpp>
#endif

#if defined(BOOST_HAS_UNISTD_H)
    #define HAMIGAKI_ARCHIVERS_LHA_OS_TYPE 'U'
#else
    #define HAMIGAKI_ARCHIVERS_LHA_OS_TYPE 'M'
#endif

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink, class Path>
class basic_raw_lzh_file_sink_impl
{
public:
    typedef char char_type;
    typedef Path path_type;
    typedef typename Path::string_type string_type;
    typedef lha::basic_header<Path> header_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_raw_lzh_file_sink_impl(const Sink& sink)
        : sink_(sink), overflow_(false), header_pos_(0), pos_(0)
    {
    }

    void create_entry(const header_type& head)
    {
        if (overflow_)
            throw std::runtime_error("need to rewind the current entry");

        header_ = head;

        header_pos_ = iostreams::tell(sink_);
        write_header();
    }

    void rewind_entry()
    {
        overflow_ = false;

        iostreams::seek(sink_, header_pos_);

        header_.method = "-lh0-";
        header_.compressed_size = header_.file_size;
        write_header();

        pos_ = 0;
    }

    void close()
    {
        if (header_.is_directory())
            return;

        if (pos_ != header_.compressed_size)
            throw BOOST_IOSTREAMS_FAILURE("LZH entry size mismatch");

        if (!header_.crc16_checksum)
            throw BOOST_IOSTREAMS_FAILURE("LZH CRC is not set");

        pos_ = 0;
    }

    void close(
        boost::uint16_t crc16_checksum, boost::int64_t file_size)
    {
        if (header_.is_directory())
            return;

        std::streampos next = iostreams::tell(sink_);

        header_.crc16_checksum = crc16_checksum;
        header_.compressed_size = pos_;
        header_.file_size = file_size;
        pos_ = 0;

        iostreams::seek(sink_, header_pos_);
        write_header();

        iostreams::seek(sink_, next);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if ((header_.file_size != -1) && (pos_ + n > header_.file_size))
        {
            overflow_ = true;
            throw give_up_compression();
        }

        iostreams::blocking_write(sink_, s, n);
        pos_ += n;
        return n;
    }

    void close_archive()
    {
        iostreams::blocking_put(sink_, '\0');
        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    bool overflow_;
    header_type header_;
    std::streampos header_pos_;
    boost::int64_t pos_;


    static Path make_linked_path_impl(const Path& path, const Path& link_path)
    {
        if (link_path.empty())
            return path;

        typename Path::string_type s;
        s = path.string();
        s += hamigaki::static_widen<typename Path::value_type,'|'>::value;
        s += link_path.string();
        return Path(s);
    }

    static Path make_linked_path(const Path& path, const Path& link_path)
    {
        if (link_path.empty())
            return detail::remove_root_name(path);

        return make_linked_path_impl(
            detail::remove_root_name(path),
            detail::remove_root_name(link_path)
        );
    }

    static std::string to_narrow(const std::string& s, unsigned, bool&)
    {
        return s;
    }

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    static std::string to_narrow(
        const std::wstring& ws, unsigned code_page, bool& used_def_char)
    {
        return charset::to_code_page(ws, code_page, "_", &used_def_char);
    }
#endif

    static std::string convert_path_old(
        const boost::filesystem::path& ph, bool directory)
    {
        std::ostringstream os;

        boost::filesystem::path::const_iterator beg = ph.begin();
        if (ph.has_root_name())
            ++beg;
        if (ph.has_root_directory())
        {
            ++beg;
            os << "\\";
        }

        std::copy(beg, ph.end(), std::ostream_iterator<std::string>(os, "\\"));

        std::string s(os.str());
        if (!directory && !s.empty())
            s.resize(s.size()-1);
        return s;
    }

    static std::string convert_path(
        const boost::filesystem::path& ph, unsigned, bool&)
    {
        if (ph.empty())
            return std::string();

        std::ostringstream os;

        boost::filesystem::path::const_iterator beg = ph.begin();
        if (ph.has_root_name())
            ++beg;
        if (ph.has_root_directory())
        {
            ++beg;
            os << "\xFF";
        }

        std::copy(beg, ph.end(),
            std::ostream_iterator<std::string>(os, "\xFF"));
        return os.str();
    }

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    static std::string convert_path_old(
        const boost::filesystem::wpath& ph, bool directory)
    {
        std::ostringstream os;

        boost::filesystem::wpath::const_iterator beg = ph.begin();
        if (ph.has_root_name())
            ++beg;
        if (ph.has_root_directory())
        {
            ++beg;
            os << "\\";
        }

        std::transform(beg, ph.end(),
            std::ostream_iterator<std::string>(os, "\\"),
            detail::to_code_page(932, "_")
        );

        std::string s(os.str());
        if (!directory && !s.empty())
            s.resize(s.size()-1);
        return s;
    }

    static std::string convert_path(
        const boost::filesystem::wpath& ph,
        unsigned code_page, bool& used_def_char)
    {
        if (ph.empty())
            return std::string();

        std::ostringstream os;

        boost::filesystem::wpath::const_iterator beg = ph.begin();
        if (ph.has_root_name())
            ++beg;
        if (ph.has_root_directory())
        {
            ++beg;
            os << "\xFF";
        }

        std::transform(beg, ph.end(),
            std::ostream_iterator<std::string>(os, "\xFF"),
            detail::to_code_page(code_page, "_", &used_def_char)
        );
        return os.str();
    }

    static std::wstring convert_path_wide(const boost::filesystem::wpath& ph)
    {
        if (ph.empty())
            return std::wstring();

        std::wostringstream os;

        boost::filesystem::wpath::const_iterator beg = ph.begin();
        if (ph.has_root_name())
            ++beg;
        if (ph.has_root_directory())
        {
            ++beg;
            os << L"\uFFFF";
        }

        std::copy(beg, ph.end(),
            std::ostream_iterator<std::wstring,wchar_t>(os, L"\xFFFF"));
        return os.str();
    }
#endif

    template<unsigned char Type, class OtherSink>
    static void write_empty_extended_header(OtherSink& sink)
    {
        static const char buf[3] = { '\x03', '\x00', static_cast<char>(Type) };
        sink.write(buf, 3);
    }

    template<unsigned char Type, class T, class OtherSink>
    static void write_extended_header(OtherSink& sink, const T& x)
    {
        static const char buf[3] =
        {
            static_cast<unsigned char>((binary_size<T>::value+3)),
            static_cast<unsigned char>((binary_size<T>::value+3) >> 8),
            static_cast<char>(Type)
        };
        sink.write(buf, 3);
        iostreams::binary_write<little>(sink, x);
    }

    template<class OtherSink>
    static void write_extended_header(
        OtherSink& sink, unsigned char type, const std::string& s)
    {
        iostreams::write_uint16<little>(sink, s.size()+3);
        boost::iostreams::put(sink, static_cast<char>(type));
        if (!s.empty())
            sink.write(s.c_str(), s.size());
    }

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    template<class OtherSink>
    static void write_extended_header(
        OtherSink& sink, unsigned char type, const std::wstring& ws)
    {
        write_extended_header(sink, type, hamigaki::charset::to_utf16le(ws));
    }
#endif

    template<class OtherSink>
    static void write_wide_filename_header(OtherSink&, const std::string&)
    {
    }

    template<class OtherSink>
    static void write_wide_dirname_header(
        OtherSink&, const boost::filesystem::path&)
    {
    }

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    template<class OtherSink>
    static void write_wide_filename_header(
        OtherSink& sink, const std::wstring& filename)
    {
        write_extended_header(sink, 0x44, filename);
    }

    template<class OtherSink>
    static void write_wide_dirname_header(
        OtherSink& sink, const boost::filesystem::wpath& ph)
    {
        write_extended_header(sink, 0x45, convert_path_wide(ph));
    }
#endif

    void write_lv0_header()
    {
        lha::lv0_header lv0;
        lv0.header_size = 0;
        lv0.header_checksum = 0;
        lv0.method = header_.method;

        if (header_.compressed_size != -1)
        {
            if (header_.compressed_size > 0xFFFFFFFF)
                throw BOOST_IOSTREAMS_FAILURE("too big compressed size");

            lv0.compressed_size =
                static_cast<boost::uint32_t>(
                    header_.compressed_size & 0xFFFFFFFF);
        }
        else
            lv0.compressed_size = 0;

        if (header_.file_size != -1)
        {
            if (header_.file_size > 0xFFFFFFFF)
                throw BOOST_IOSTREAMS_FAILURE("too big compressed size");

            lv0.file_size =
                static_cast<boost::uint32_t>(
                    header_.file_size & 0xFFFFFFFF);
        }
        else
            lv0.file_size = 0;

        lv0.update_date_time = msdos::date_time(header_.update_time);
        lv0.attributes = header_.attributes & msdos::attributes::mask;

        std::string buffer;
        boost::iostreams::back_insert_device<std::string> tmp(buffer);
        iostreams::binary_write(tmp, lv0);

        std::string name;
        if (header_.link_path.empty())
            name = convert_path_old(header_.path, header_.is_directory());
        else
        {
            name = convert_path_old(header_.path, false);
            name += '|';
            name += convert_path_old(header_.link_path, false);
        }

        if (name.size() > 0xFF)
            throw BOOST_IOSTREAMS_FAILURE("too long path");

        iostreams::write_uint8<little>(
            tmp, static_cast<boost::uint8_t>(name.size()));
        tmp.write(name.c_str(), name.size());

        if (header_.crc16_checksum)
            iostreams::write_uint16<little>(tmp, header_.crc16_checksum.get());
        else
            tmp.write("\0", 2);

        if (buffer.size()-2 > 0xFF)
            throw BOOST_IOSTREAMS_FAILURE("too big LZH header");

        buffer[0] =
            static_cast<char>(static_cast<unsigned char>(buffer.size()-2));

        checksum::sum8 sum;
        sum.process_bytes(buffer.c_str()+2, buffer.size()-2);
        buffer[1] = sum.checksum();

        boost::iostreams::write(sink_, buffer.c_str(), buffer.size());
    }

    void write_lv1_header()
    {
        lha::lv1_header lv1;
        lv1.header_size = 0;
        lv1.header_checksum = 0;
        lv1.method = header_.method;

        if (header_.compressed_size > 0xFFFFFFFF)
            throw BOOST_IOSTREAMS_FAILURE("too big compressed size");
        lv1.skip_size = 0;

        if (header_.file_size != -1)
        {
            if (header_.file_size > 0xFFFFFFFF)
                throw BOOST_IOSTREAMS_FAILURE("too big compressed size");

            lv1.file_size =
                static_cast<boost::uint32_t>(
                    header_.file_size & 0xFFFFFFFF);
        }
        else
            lv1.file_size = 0;

        lv1.update_date_time = msdos::date_time(header_.update_time);
        lv1.reserved = msdos::attributes::archive;
        lv1.level = 1;

        std::string buffer;
        boost::iostreams::back_insert_device<std::string> tmp(buffer);
        iostreams::binary_write(tmp, lv1);

        Path ph = make_linked_path(header_.path, header_.link_path);

        std::string filename;
        std::string dirname;
        bool dummy = false;

        unsigned code_page = 0;
        if (header_.code_page)
            code_page = header_.code_page.get();

        if (header_.link_path.empty() && header_.is_directory())
            dirname = convert_path(ph, code_page, dummy);
        else
        {
            filename = to_narrow(ph.leaf(), code_page, dummy);
            dirname = convert_path(ph.branch_path(), code_page, dummy);
        }

        std::size_t max_fname_size = 0xFF-binary_size<lha::lv1_header>::value-4;
        bool use_fname_head = filename.size() > max_fname_size;

        if (!filename.empty() && !use_fname_head)
        {
            iostreams::write_uint8<little>(
                tmp, static_cast<boost::uint8_t>(filename.size()));
            tmp.write(filename.c_str(), filename.size());
        }
        else
            iostreams::write_uint8<little>(tmp, 0);

        if (header_.crc16_checksum)
            iostreams::write_uint16<little>(tmp, header_.crc16_checksum.get());
        else
            tmp.write("\0", 2);

        if (header_.os)
            boost::iostreams::put(tmp, header_.os.get());
        else
            boost::iostreams::put(tmp, HAMIGAKI_ARCHIVERS_LHA_OS_TYPE);

        std::size_t basic_size = buffer.size();

        if (basic_size > 0xFF)
            throw BOOST_IOSTREAMS_FAILURE("too big LZH header");

        buffer[0] =
            static_cast<char>(static_cast<unsigned char>(basic_size));

        if (use_fname_head)
            write_extended_header(tmp, 0x01, filename);
        if (!dirname.empty())
            write_extended_header(tmp, 0x02, dirname);

        if (header_.attributes != msdos::attributes::archive)
            write_extended_header<0x40>(tmp, header_.attributes);

        if (header_.permissions)
            write_extended_header<0x50>(tmp, header_.permissions.get());

        if (header_.owner)
            write_extended_header<0x51>(tmp, header_.owner.get());

        if (!header_.group_name.empty())
            write_extended_header(tmp, 0x52, header_.group_name);

        if (!header_.user_name.empty())
            write_extended_header(tmp, 0x53, header_.user_name);

        write_extended_header<0x54>(
            tmp, static_cast<boost::int32_t>(header_.update_time));

        tmp.write("\x06\x00\x00", 3);
        std::size_t crc_off = buffer.size();
        // TODO: timezone
        tmp.write("\x00\x00\x40", 3);

        // end of extended headers
        iostreams::write_uint16<little>(tmp, 0);

        boost::int64_t size = buffer.size()-basic_size-2;
        if (header_.compressed_size != -1)
            size += header_.compressed_size;

        if (size > 0xFFFFFFFF)
            throw BOOST_IOSTREAMS_FAILURE("too big compressed size");

        char size_buf[4];
        hamigaki::encode_uint<little,4>(
            size_buf, static_cast<boost::uint32_t>(size & 0xFFFF));
        buffer[ 7] = size_buf[0];
        buffer[ 8] = size_buf[1];
        buffer[ 9] = size_buf[2];
        buffer[10] = size_buf[3];

        checksum::sum8 sum;
        sum.process_bytes(buffer.c_str()+2, basic_size);
        buffer[1] = sum.checksum();

        boost::crc_16_type crc;
        crc.process_bytes(buffer.c_str(), buffer.size());
        char crc_buf[2];
        hamigaki::encode_uint<little,2>(crc_buf, crc.checksum());
        buffer[crc_off] = crc_buf[0];
        buffer[crc_off+1] = crc_buf[1];

        boost::iostreams::write(sink_, buffer.c_str(), buffer.size());
    }

    void write_lv2_header()
    {
        lha::lv2_header lv2;
        lv2.header_size = 0;
        lv2.method = header_.method;

        if (header_.compressed_size != -1)
        {
            lv2.compressed_size =
                static_cast<boost::uint32_t>(
                    header_.compressed_size & 0xFFFFFFFF);
        }
        else
            lv2.compressed_size = 0;

        if (header_.file_size != -1)
        {
            lv2.file_size =
                static_cast<boost::uint32_t>(
                    header_.file_size & 0xFFFFFFFF);
        }
        else
            lv2.file_size = 0;

        lv2.update_time = header_.update_time;
        lv2.reserved = static_cast<boost::uint8_t>(msdos::attributes::archive);
        lv2.level = 2;

        std::string buffer;
        boost::iostreams::back_insert_device<std::string> tmp(buffer);
        iostreams::binary_write(tmp, lv2);
        if (header_.crc16_checksum)
            iostreams::write_uint16<little>(tmp, header_.crc16_checksum.get());
        else
            tmp.write("\0", 2);

        if (header_.os)
            boost::iostreams::put(tmp, header_.os.get());
        else
            boost::iostreams::put(tmp, HAMIGAKI_ARCHIVERS_LHA_OS_TYPE);

        unsigned code_page = 0;
        if (header_.code_page)
        {
            code_page = header_.code_page.get();
            write_extended_header<0x46>(tmp, code_page);
        }

        bool need_size_hdr = false;
        if ((header_.compressed_size != -1) &&
            ((header_.compressed_size >> 32) != 0))
        {
            need_size_hdr = true;
        }
        if ((header_.file_size != -1) &&
            ((header_.file_size >> 32) != 0))
        {
            need_size_hdr = true;
        }

        if (need_size_hdr)
        {
            write_extended_header<0x42>(tmp,
                lha::windows::file_size(
                    header_.compressed_size, header_.file_size
                )
            );
        }

        Path ph = make_linked_path(header_.path, header_.link_path);

        std::string filename;
        std::string dirname;
        bool need_wfname = false;
        bool need_wdname = false;

        if (header_.link_path.empty() && header_.is_directory())
            dirname = convert_path(ph, code_page, need_wdname);
        else
        {
            filename = to_narrow(ph.leaf(), code_page, need_wfname);
            dirname = convert_path(ph.branch_path(), code_page, need_wdname);
        }

        write_extended_header(tmp, 0x01, filename);
        if (!dirname.empty())
            write_extended_header(tmp, 0x02, dirname);

        if (!header_.comment.empty())
            write_extended_header(tmp, 0x3F, header_.comment);

        if (header_.attributes != msdos::attributes::archive)
            write_extended_header<0x40>(tmp, header_.attributes);

        if (header_.timestamp)
            write_extended_header<0x41>(tmp, header_.timestamp.get());

        if (need_wfname)
            write_wide_filename_header(tmp, ph.leaf());

        if (need_wdname)
            write_wide_dirname_header(tmp, ph.branch_path());

        if (header_.permissions)
            write_extended_header<0x50>(tmp, header_.permissions.get());

        if (header_.owner)
            write_extended_header<0x51>(tmp, header_.owner.get());

        if (!header_.group_name.empty())
            write_extended_header(tmp, 0x52, header_.group_name);

        if (!header_.user_name.empty())
            write_extended_header(tmp, 0x53, header_.user_name);

        tmp.write("\x06\x00\x00", 3);
        std::size_t crc_off = buffer.size();
        // TODO: timezone
        tmp.write("\x00\x00\x07", 3);

        // end of extended headers
        iostreams::write_uint16<little>(tmp, 0);

        // LZH header must not start by '\0'.
        // So the low byte of the header size must not be 0.
        if ((buffer.size() & 0xFF) == 0)
            buffer.push_back('\0');

        char size_buf[2];
        hamigaki::encode_uint<little,2>(size_buf, buffer.size());
        buffer[0] = size_buf[0];
        buffer[1] = size_buf[1];

        boost::crc_16_type crc;
        crc.process_bytes(buffer.c_str(), buffer.size());
        char crc_buf[2];
        hamigaki::encode_uint<little,2>(crc_buf, crc.checksum());
        buffer[crc_off] = crc_buf[0];
        buffer[crc_off+1] = crc_buf[1];

        boost::iostreams::write(sink_, buffer.c_str(), buffer.size());
    }

    void write_header()
    {
        if (header_.level == 0)
            write_lv0_header();
        else if (header_.level == 1)
            write_lv1_header();
        else if (header_.level == 2)
            write_lv2_header();
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported LZH header level");
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_RAW_LZH_FILE_SINK_IMPL_HPP
