// tar_file_source_impl.hpp: POSIX tar file source implementation

// Copyright Takeshi Mouri 2006, 2007.
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

namespace hamigaki { namespace archivers { namespace detail {

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
    return detail::to_timestamp(std::string(beg, end));
}

template<class Source>
class basic_tar_file_source_impl
{
private:
    typedef detail::basic_ustar_file_source_impl<Source> ustar_type;

public:
    explicit basic_tar_file_source_impl(const Source& src)
        : ustar_(src), is_pax_(false)
    {
    }

    bool next_entry()
    {
        if (!ustar_.next_entry())
            return false;

        header_ = ustar_.header();

        if (header_.type_flag == tar::type_flag::global)
        {
            read_extended_header(global_);

            if (!ustar_.next_entry())
                throw boost::iostreams::detail::bad_read();

            header_ = ustar_.header();
            is_pax_ = true;
        }

        tar_ex_header ext = global_;
        bool is_pax = is_pax_;

        if (header_.type_flag == tar::type_flag::extended)
        {
            read_extended_header(ext);

            if (!ustar_.next_entry())
                throw boost::iostreams::detail::bad_read();

            header_ = ustar_.header();
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

            header_ = ustar_.header();
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

        return true;
    }

    tar::header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return ustar_.read(s, n);
    }

private:
    ustar_type ustar_;
    tar::header header_;
    tar_ex_header global_;
    bool is_pax_;

    boost::filesystem::path read_long_link()
    {
        using namespace boost::filesystem;

        std::string buf;

        boost::iostreams::copy(
            boost::ref(ustar_),
            boost::iostreams::back_inserter(buf));

        if (!buf.empty() && (*(buf.rbegin()) == '\0'))
            buf.resize(buf.size()-1);

        return path(buf, no_check);
    }

    void read_extended_header(tar_ex_header& ext)
    {
        using namespace boost::filesystem;

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
                ext.path = path(beg, no_check);
            else if (key == "uid")
                ext.uid = hamigaki::from_dec<boost::intmax_t>(beg, end);
            else if (key == "gid")
                ext.gid = hamigaki::from_dec<boost::intmax_t>(beg, end);
            else if (key == "size")
                ext.file_size = hamigaki::from_dec<boost::uintmax_t>(beg, end);
            else if (key == "mtime")
                ext.modified_time = detail::to_timestamp(beg, end);
            else if (key == "atime")
                ext.access_time = detail::to_timestamp(beg, end);
            else if (key == "ctime")
                ext.change_time = detail::to_timestamp(beg, end);
            else if (key == "linkpath")
                ext.link_path = path(beg, no_check);
            else if (key == "uname")
                ext.user_name = beg;
            else if (key == "gname")
                ext.group_name = beg;
            else if (key == "comment")
                ext.comment = beg;
        }
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_TAR_FILE_SOURCE_IMPL_HPP
