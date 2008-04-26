// tar_file_sink_impl.hpp: POSIX tar file sink implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/ustar_file_sink_impl.hpp>
#include <hamigaki/charset/code_page.hpp>
#include <hamigaki/charset/utf8.hpp>

namespace hamigaki { namespace archivers { namespace tar_detail {

inline std::string to_tar_string(const std::string& s)
{
    return s;
}

inline std::string to_pax_string(const std::string& s)
{
    return charset::to_utf8(charset::from_code_page(s, 0));
}

inline tar::header to_narrow(const tar::header& head)
{
    return head;
}

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
inline std::string to_tar_string(const std::wstring& ws)
{
    return charset::to_code_page(ws, 0, "_");
}

inline std::string to_pax_string(const std::wstring& ws)
{
    return charset::to_utf8(ws);
}

inline tar::header to_narrow(const tar::wheader& head)
{
    tar::header tmp;

    tmp.path = tar_detail::to_tar_string(head.path.string());
    tmp.permissions = head.permissions;
    tmp.uid = head.uid;
    tmp.gid = head.gid;
    tmp.file_size = head.file_size;
    tmp.modified_time = head.modified_time;
    tmp.access_time = head.access_time;
    tmp.change_time = head.change_time;
    tmp.type_flag = head.type_flag;
    tmp.link_path = tar_detail::to_tar_string(head.link_path.string());
    tmp.format = head.format;
    tmp.user_name = charset::to_code_page(head.user_name, 0);
    tmp.group_name = charset::to_code_page(head.group_name, 0);
    tmp.dev_major = head.dev_major;
    tmp.dev_minor = head.dev_minor;
    tmp.comment = charset::to_code_page(head.comment, 0);
    tmp.charset = charset::to_code_page(head.charset, 0);

    return tmp;
}
#endif

inline std::string
make_ex_header_recoed(const std::string& key, const std::string value)
{
    // "size key=value\n"
    std::size_t size0 = 1 + key.size() + 1 + value.size() + 1;
    std::size_t size1 = to_dec<char>(size0).size() + size0;
    std::size_t size = to_dec<char>(size1).size() + size0;

    std::string buf = to_dec<char>(size);
    buf += ' ';
    buf += key;
    buf += '=';
    buf += value;
    buf += '\n';

    return buf;
}

struct is_non_ascii_func
{
    bool operator()(char c) const
    {
        return (static_cast<unsigned char>(c) & 0x80) != 0;
    }

    bool operator()(wchar_t wc) const
    {
        return static_cast<boost::uint32_t>(wc) > 0x7F;
    }
};

template<class String>
inline bool is_non_ascii(const String& s)
{
    return std::find_if(s.begin(), s.end(), is_non_ascii_func()) != s.end();
}

inline std::string encode_nsec(boost::uint32_t x)
{
    std::string buf = to_dec<char>(x);
    if (buf.size() < 9)
        buf.insert(buf.begin(), 9-buf.size(), '0');
    while (*buf.rbegin() == '0')
        buf.resize(buf.size()-1);
    return buf;
}

inline std::string from_timestamp(const filesystem::timestamp& x)
{
    if (x.nanoseconds != 0)
    {
        std::time_t sec = x.seconds;
        boost::uint32_t nsec = x.nanoseconds;

        std::string buf;

        if (sec < 0)
        {
            ++sec;
            nsec = 1000000000ul - nsec;
            if (sec == 0)
                buf += '-';
        }

        buf += to_dec<char>(sec);
        buf += '.';
        buf += encode_nsec(nsec);
        return buf;
    }
    else
        return to_dec<char>(x.seconds);
}

} } } // End namespaces tar_detail, archivers, hamigaki.

namespace hamigaki { namespace archivers { namespace detail {

template<class Sink, class Path>
class basic_tar_file_sink_impl
{
private:
    typedef detail::basic_ustar_file_sink_impl<Sink> ustar_type;

public:
    typedef Path path_type;
    typedef tar::basic_header<Path> header_type;

    explicit basic_tar_file_sink_impl(const Sink& sink) : ustar_(sink)
    {
    }

    void create_entry(const header_type& head)
    {
        tar::header local = tar_detail::to_narrow(head);

        std::string name = tar_detail::to_tar_string(head.path.string());
        std::string prefix;
        if ((head.format != tar::gnu) &&
            (name.size() > tar::raw_header::name_size) )
        {
            name = tar_detail::to_tar_string(head.path.leaf());
            prefix =
                tar_detail::to_tar_string(head.path.branch_path().string());
        }

        std::string long_link =
            tar_detail::to_tar_string(head.link_path.string());
        if (head.format == tar::pax)
        {
            std::string ex;
            if ((name.size() > tar::raw_header::name_size) ||
                (prefix.size() > tar::raw_header::prefix_size) ||
                tar_detail::is_non_ascii(head.path.string()) )
            {
                std::string path =
                    tar_detail::to_pax_string(head.path.string());
                ex += tar_detail::make_ex_header_recoed("path", path);

                std::string long_name =
                    tar_detail::to_tar_string(head.path.string());
                long_name.resize(tar::raw_header::name_size);
                local.path = long_name;
            }

            if ((head.uid < 0) || (head.uid > tar::raw_header::max_uid))
            {
                ex += tar_detail::
                    make_ex_header_recoed("uid", to_dec<char>(head.uid));

                local.uid = 0;
            }

            if ((head.gid < 0) || (head.gid > tar::raw_header::max_gid))
            {
                ex += tar_detail::
                    make_ex_header_recoed("gid", to_dec<char>(head.gid));

                local.gid = 0;
            }

            if (head.file_size > tar::raw_header::max_size)
            {
                ex += tar_detail::
                    make_ex_header_recoed("size", to_dec<char>(head.file_size));

                local.file_size = 0;
            }

            if (head.modified_time && (head.modified_time->nanoseconds != 0))
            {
                ex += tar_detail::make_ex_header_recoed(
                    "mtime",
                    tar_detail::from_timestamp(head.modified_time.get()));
            }

            if (head.access_time)
            {
                ex += tar_detail::make_ex_header_recoed(
                    "atime",
                    tar_detail::from_timestamp(head.access_time.get()));
            }

            if (head.change_time)
            {
                ex += tar_detail::make_ex_header_recoed(
                    "ctime",
                    tar_detail::from_timestamp(head.change_time.get()));
            }

            if ((long_link.size() > tar::raw_header::name_size) ||
                tar_detail::is_non_ascii(head.link_path.string()) )
            {
                std::string linkpath =
                    tar_detail::to_pax_string(head.link_path.string());
                ex += tar_detail::make_ex_header_recoed("linkpath", linkpath);

                long_link.resize(tar::raw_header::name_size);
                local.link_path = long_link;
            }

            if (tar_detail::is_non_ascii(head.user_name))
            {
                ex +=
                    tar_detail::make_ex_header_recoed(
                        "uname",
                        tar_detail::to_pax_string(head.user_name)
                    );
            }

            if (tar_detail::is_non_ascii(head.group_name))
            {
                ex +=
                    tar_detail::make_ex_header_recoed(
                        "gname",
                        tar_detail::to_pax_string(head.group_name)
                    );
            }

            if (!head.comment.empty())
            {
                ex +=
                    tar_detail::make_ex_header_recoed(
                        "comment",
                        tar_detail::to_pax_string(head.comment)
                    );
            }

            if (!head.charset.empty())
            {
                ex +=
                    tar_detail::make_ex_header_recoed(
                        "charset",
                        tar_detail::to_pax_string(head.charset)
                    );
            }

            if (!ex.empty())
                write_extended_header(local.path, ex);
        }
        else if (head.format == tar::gnu)
        {
            if (name.size() > tar::raw_header::name_size)
            {
                std::string long_name =
                    tar_detail::to_tar_string(head.path.string());
                if (head.type_flag == tar::type_flag::directory)
                    long_name += '/';

                tar::header tmp;
                tmp.permissions = 0;
                tmp.file_size = long_name.size()+1;
                tmp.type_flag = tar::type_flag::long_name;
                tmp.format = tar::gnu;
                tmp.group_name = "root";

                ustar_.create_entry(tmp);
                ustar_.write(long_name.c_str(), long_name.size()+1);
                ustar_.close();

                long_name.resize(tar::raw_header::name_size);
                local.path = long_name;
            }

            if (long_link.size() > tar::raw_header::name_size)
            {
                tar::header tmp;
                tmp.permissions = 0;
                tmp.file_size = long_link.size()+1;
                tmp.type_flag = tar::type_flag::long_link;
                tmp.format = tar::gnu;
                tmp.group_name = "root";

                ustar_.create_entry(tmp);
                ustar_.write(long_link.c_str(), long_link.size()+1);
                ustar_.close();

                long_link.resize(tar::raw_header::name_size);
                local.link_path = long_link;
            }
        }

        ustar_.create_entry(local);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return ustar_.write(s, n);
    }

    void close()
    {
        ustar_.close();
    }

    void close_archive()
    {
        ustar_.close_archive();
    }

private:
    ustar_type ustar_;

    void write_extended_header(
        const boost::filesystem::path& ph, const std::string& ex)
    {
        tar::header tmp;
        tmp.path = ph;
        tmp.permissions = 0;
        tmp.file_size = ex.size();
        tmp.type_flag = tar::type_flag::extended;
        tmp.format = tar::pax;

        ustar_.create_entry(tmp);
        ustar_.write(ex.c_str(), ex.size());
        ustar_.close();
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SINK_IMPL_HPP
