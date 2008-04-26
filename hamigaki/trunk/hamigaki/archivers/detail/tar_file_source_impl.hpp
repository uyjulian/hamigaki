// tar_file_source_impl.hpp: POSIX tar file source implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SOURCE_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SOURCE_IMPL_HPP

#include <hamigaki/archivers/detail/tar_ex_header.hpp>
#include <hamigaki/archivers/detail/ustar_file_source_impl.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_array.hpp>

namespace hamigaki { namespace archivers { namespace tar_detail {

template<class String>
inline String from_tar_string(const std::string& s)
{
    return s;
}

template<class String>
inline String from_pax_string(const std::string& s)
{
    return charset::to_code_page(charset::from_utf8(s), 0, "_");
}

template<class Path>
inline tar::basic_header<Path> to_wide(const tar::header& head)
{
    return head;
}

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
template<>
inline std::wstring from_tar_string<std::wstring>(const std::string& s)
{
    return charset::from_code_page(s, 0);
}

template<>
inline std::wstring from_pax_string<std::wstring>(const std::string& s)
{
    return charset::from_utf8(s);
}

template<>
inline tar::wheader to_wide<boost::filesystem::wpath>(const tar::header& head)
{
    tar::wheader tmp;

    tmp.path = tar_detail::from_tar_string<std::wstring>(head.path.string());
    tmp.permissions = head.permissions;
    tmp.uid = head.uid;
    tmp.gid = head.gid;
    tmp.file_size = head.file_size;
    tmp.modified_time = head.modified_time;
    tmp.access_time = head.access_time;
    tmp.change_time = head.change_time;
    tmp.type_flag = head.type_flag;
    tmp.link_path =
        tar_detail::from_tar_string<std::wstring>(head.link_path.string());
    tmp.format = head.format;
    tmp.user_name = charset::from_code_page(head.user_name, 0);
    tmp.group_name = charset::from_code_page(head.group_name, 0);
    tmp.dev_major = head.dev_major;
    tmp.dev_minor = head.dev_minor;
    tmp.comment = charset::from_code_page(head.comment, 0);
    tmp.charset = charset::from_code_page(head.charset, 0);

    return tmp;
}
#endif

inline boost::uint32_t decode_nsec(const char* beg, const char* end)
{
    std::string buf(beg, end);
    std::size_t size = buf.size();
    if (size < 9)
        buf.append(9-size, '0');
    else
        buf.resize(9);
    return from_dec<boost::uint32_t>(buf);
}

inline filesystem::timestamp to_timestamp(const std::string& s)
{
    if (s.empty() || (s == "-"))
        return filesystem::timestamp();

    bool minus = s[0] == '-';
    std::string::size_type pos = s.find('.');
    if (pos != std::string::npos)
    {
        filesystem::timestamp tmp;
        const char* beg = s.c_str();
        tmp.seconds = from_dec<std::time_t>(beg, beg+pos);
        tmp.nanoseconds = decode_nsec(beg+pos+1, beg+s.size());

        if (minus)
        {
            --(tmp.seconds);
            tmp.nanoseconds = 1000000000ul - tmp.nanoseconds;
        }
        return tmp;
    }
    else
        return filesystem::timestamp::from_time_t(from_dec<std::time_t>(s));
}

inline filesystem::timestamp to_timestamp(const char* beg, const char* end)
{
    return tar_detail::to_timestamp(std::string(beg, end));
}

} } } // End namespaces tar_detail, archivers, hamigaki.

namespace hamigaki { namespace archivers { namespace detail {

template<class Source, class Path>
class basic_tar_file_source_impl
{
private:
    typedef detail::basic_ustar_file_source_impl<Source> ustar_type;
    typedef detail::tar_ex_header<Path> tar_ex_header;

public:
    typedef Path path_type;
    typedef typename Path::string_type string_type;
    typedef tar::basic_header<Path> header_type;

    explicit basic_tar_file_source_impl(const Source& src)
        : ustar_(src), is_pax_(false)
    {
    }

    bool next_entry()
    {
        if (!ustar_.next_entry())
            return false;

        header_ = tar_detail::to_wide<Path>(ustar_.header());

        if (header_.type_flag == tar::type_flag::global)
        {
            read_extended_header(global_);

            if (!ustar_.next_entry())
                throw boost::iostreams::detail::bad_read();

            header_ = tar_detail::to_wide<Path>(ustar_.header());
            is_pax_ = true;
        }

        tar_ex_header ext = global_;
        bool is_pax = is_pax_;

        if (header_.type_flag == tar::type_flag::extended)
        {
            read_extended_header(ext);

            if (!ustar_.next_entry())
                throw boost::iostreams::detail::bad_read();

            header_ = tar_detail::to_wide<Path>(ustar_.header());
            is_pax = true;
        }

        while (header_.is_long())
        {
            if (header_.type_flag == tar::type_flag::long_link)
                ext.link_path = read_long_link();
            else
                ext.path = read_long_link();

            if (!ustar_.next_entry())
                throw boost::iostreams::detail::bad_read();

            header_ = tar_detail::to_wide<Path>(ustar_.header());
        }

        if (is_pax)
            header_.format = tar::pax;

        if (!ext.path.empty())
            header_.path = ext.path;

        if (ext.uid)
            header_.uid = ext.uid.get();

        if (ext.gid)
            header_.gid = ext.gid.get();

        if (ext.file_size)
            header_.file_size = ext.file_size.get();

        if (ext.modified_time)
            header_.modified_time = ext.modified_time;

        if (ext.access_time)
            header_.access_time = ext.access_time;

        if (ext.change_time)
            header_.change_time = ext.change_time;

        if (!ext.link_path.empty())
            header_.link_path = ext.link_path;

        if (!ext.user_name.empty())
            header_.user_name = ext.user_name;

        if (!ext.group_name.empty())
            header_.group_name = ext.group_name;

        if (!ext.comment.empty())
            header_.comment = ext.comment;

        if (!ext.charset.empty())
            header_.charset = ext.charset;

        return true;
    }

    header_type header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return ustar_.read(s, n);
    }

private:
    ustar_type ustar_;
    header_type header_;
    tar_ex_header global_;
    bool is_pax_;

    Path read_long_link()
    {
        std::string buf;

        boost::iostreams::copy(
            boost::ref(ustar_),
            boost::iostreams::back_inserter(buf));

        if (!buf.empty() && (*(buf.rbegin()) == '\0'))
            buf.resize(buf.size()-1);

        return tar_detail::from_tar_string<string_type>(buf);
    }

    void read_extended_header(tar_ex_header& ext)
    {
        boost::iostreams::stream<
            boost::reference_wrapper<ustar_type>
        > is(boost::ref(ustar_));

        std::string size_str;
        while (std::getline(is, size_str, ' '))
        {
            std::size_t size = hamigaki::from_dec<std::size_t>(size_str);
            if (size < size_str.size() + 1)
                throw BOOST_IOSTREAMS_FAILURE("bad tar extended header");
            size -= (size_str.size()+1);

            std::string key;
            if (!std::getline(is, key, '='))
                throw BOOST_IOSTREAMS_FAILURE("bad tar extended header");

            if (size <= key.size() + 1)
                throw BOOST_IOSTREAMS_FAILURE("bad tar extended header");
            size -= (key.size() + 1);

            boost::scoped_array<char> buf(new char[size]);
            buf[size-1] = '\0';
            is.read(&buf[0], size);
            if (buf[size-1] != '\n')
                throw BOOST_IOSTREAMS_FAILURE("bad tar extended header");
            buf[size-1] = '\0';

            const char* beg = buf.get();
            const char* end = beg + (size - 1);

            if (key == "path")
                ext.path = tar_detail::from_pax_string<string_type>(beg);
            else if (key == "uid")
                ext.uid = hamigaki::from_dec<boost::intmax_t>(beg, end);
            else if (key == "gid")
                ext.gid = hamigaki::from_dec<boost::intmax_t>(beg, end);
            else if (key == "size")
                ext.file_size = hamigaki::from_dec<boost::uintmax_t>(beg, end);
            else if (key == "mtime")
                ext.modified_time = tar_detail::to_timestamp(beg, end);
            else if (key == "atime")
                ext.access_time = tar_detail::to_timestamp(beg, end);
            else if (key == "ctime")
                ext.change_time = tar_detail::to_timestamp(beg, end);
            else if (key == "linkpath")
                ext.link_path = tar_detail::from_pax_string<string_type>(beg);
            else if (key == "uname")
                ext.user_name = tar_detail::from_pax_string<string_type>(beg);
            else if (key == "gname")
                ext.group_name = tar_detail::from_pax_string<string_type>(beg);
            else if (key == "comment")
                ext.comment = tar_detail::from_pax_string<string_type>(beg);
            else if (key == "charset")
                ext.charset = tar_detail::from_pax_string<string_type>(beg);
        }
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SOURCE_IMPL_HPP
