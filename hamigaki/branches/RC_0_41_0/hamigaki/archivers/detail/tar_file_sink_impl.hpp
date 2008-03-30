// tar_file_sink_impl.hpp: POSIX tar file sink implementation

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SINK_IMPL_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SINK_IMPL_HPP

#include <hamigaki/archivers/detail/ustar_file_sink_impl.hpp>

namespace hamigaki { namespace archivers { namespace detail {

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
};

inline bool is_non_ascii(const std::string& s)
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

template<class Sink>
class basic_tar_file_sink_impl
{
private:
    typedef detail::basic_ustar_file_sink_impl<Sink> ustar_type;

public:
    explicit basic_tar_file_sink_impl(const Sink& sink) : ustar_(sink)
    {
    }

    void create_entry(const tar::header& head)
    {
        using namespace boost::filesystem;

        tar::header local = head;

        std::string name = head.path.string();
        std::string prefix;
        if ((head.format != tar::gnu) &&
            (name.size() > tar::raw_header::name_size) )
        {
            name = head.path.leaf();
            prefix = head.path.branch_path().string();
        }

        std::string long_link = head.link_path.string();
        if (head.format == tar::pax)
        {
            std::string ex;
            if ((name.size() > tar::raw_header::name_size) ||
                (prefix.size() > tar::raw_header::prefix_size))
            {
                std::string long_name = head.path.string();
                ex += detail::make_ex_header_recoed("path", long_name);

                long_name.resize(tar::raw_header::name_size);
                local.path = path(long_name, no_check);
            }

            if ((head.uid < 0) || (head.uid > tar::raw_header::max_uid))
            {
                ex += detail::
                    make_ex_header_recoed("uid", to_dec<char>(head.uid));

                local.uid = 0;
            }

            if ((head.gid < 0) || (head.gid > tar::raw_header::max_gid))
            {
                ex += detail::
                    make_ex_header_recoed("gid", to_dec<char>(head.gid));

                local.gid = 0;
            }

            if (head.file_size > tar::raw_header::max_size)
            {
                ex += detail::
                    make_ex_header_recoed("size", to_dec<char>(head.file_size));

                local.file_size = 0;
            }

            if (head.modified_time && (head.modified_time->nanoseconds != 0))
            {
                ex += detail::make_ex_header_recoed(
                    "mtime",
                    detail::from_timestamp(head.modified_time.get()));
            }

            if (head.access_time)
            {
                ex += detail::make_ex_header_recoed(
                    "atime",
                    detail::from_timestamp(head.access_time.get()));
            }

            if (head.change_time)
            {
                ex += detail::make_ex_header_recoed(
                    "ctime",
                    detail::from_timestamp(head.change_time.get()));
            }

            if (long_link.size() > tar::raw_header::name_size)
            {
                ex += detail::make_ex_header_recoed("linkpath", long_link);

                long_link.resize(tar::raw_header::name_size);
                local.link_path = path(long_link, no_check);
            }

            if (detail::is_non_ascii(head.user_name))
            {
                ex += detail::
                    make_ex_header_recoed("uname", head.user_name);
            }

            if (detail::is_non_ascii(head.group_name))
            {
                ex += detail::
                    make_ex_header_recoed("gname", head.group_name);
            }

            if (!head.comment.empty())
            {
                ex += detail::
                    make_ex_header_recoed("comment", head.comment);
            }

            if (!ex.empty())
                write_extended_header(head.path, ex);
        }
        else if (head.format == tar::gnu)
        {
            if (name.size() > tar::raw_header::name_size)
            {
                std::string long_name = head.path.string();
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
                local.path = path(long_name, no_check);
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
                local.link_path = path(long_link, no_check);
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
