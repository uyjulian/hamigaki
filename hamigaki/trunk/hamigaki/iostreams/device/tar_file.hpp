//  tar_file.hpp: POSIX tar file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_TAR_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_TAR_FILE_HPP

#include <hamigaki/iostreams/device/ustar_file.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_array.hpp>

namespace hamigaki { namespace iostreams {

namespace tar
{

struct extended_header
{
    boost::filesystem::path path;
    boost::optional<boost::intmax_t> uid;
    boost::optional<boost::intmax_t> gid;
    boost::optional<boost::uintmax_t> size;
    boost::optional<timestamp> modified_time;
    boost::optional<timestamp> access_time;
    boost::optional<timestamp> change_time;
    boost::filesystem::path link_path;
    std::string user_name;
    std::string group_name;
    std::string comment;
};

namespace detail
{

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
        return static_cast<unsigned char>(c) & 0x80;
    }
};

inline bool is_non_ascii(const std::string& s)
{
    return std::find_if(s.begin(), s.end(), is_non_ascii_func()) != s.end();
}

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

inline timestamp to_timestamp(const std::string& s)
{
    if (s.empty() || (s == "-"))
        return timestamp();

    bool minus = s[0] == '-';
    std::string::size_type pos = s.find('.');
    if (pos != std::string::npos)
    {
        timestamp tmp;
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
        return timestamp(from_dec<std::time_t>(s));
}

inline timestamp to_timestamp(const char* beg, const char* end)
{
    return detail::to_timestamp(std::string(beg, end));
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

inline std::string from_timestamp(const timestamp& x)
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

} // namespace detail

} // namespace tar

template<class Source>
class basic_tar_file_source_impl
{
private:
    typedef basic_ustar_file_source_impl<Source> ustar_type;

public:
    explicit basic_tar_file_source_impl(const Source& src) : ustar_(src)
    {
    }

    bool next_entry()
    {
        if (!ustar_.next_entry())
            return false;

        header_ = ustar_.header();

        if (header_.type == tar::type::global)
        {
            read_extended_header(global_);

            if (!ustar_.next_entry())
                throw boost::iostreams::detail::bad_read();

            header_ = ustar_.header();
        }

        tar::extended_header ext = global_;

        if (header_.type == tar::type::extended)
        {
            read_extended_header(ext);

            if (!ustar_.next_entry())
                throw boost::iostreams::detail::bad_read();

            header_ = ustar_.header();
        }

        while (header_.is_long())
        {
            if (header_.type == tar::type::long_link)
                ext.link_path = read_long_link();
            else
                ext.path = read_long_link();

            if (!ustar_.next_entry())
                throw boost::iostreams::detail::bad_read();

            header_ = ustar_.header();
        }

        if (!ext.path.empty())
            header_.path = ext.path;

        if (ext.uid)
            header_.uid = ext.uid.get();

        if (ext.gid)
            header_.gid = ext.gid.get();

        if (ext.size)
            header_.size = ext.size.get();

        if (ext.modified_time)
            header_.modified_time = ext.modified_time;

        if (ext.access_time)
            header_.access_time = ext.access_time;

        if (ext.change_time)
            header_.change_time = ext.change_time;

        if (!ext.link_path.empty())
            header_.link_name = ext.link_path;

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
    tar::extended_header global_;

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

    void read_extended_header(tar::extended_header& ext)
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
                ext.size = hamigaki::from_dec<boost::uintmax_t>(beg, end);
            else if (key == "mtime")
                ext.modified_time = tar::detail::to_timestamp(beg, end);
            else if (key == "atime")
                ext.access_time = tar::detail::to_timestamp(beg, end);
            else if (key == "ctime")
                ext.change_time = tar::detail::to_timestamp(beg, end);
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

template<class Source>
class basic_tar_file_source
{
private:
    typedef basic_tar_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_tar_file_source(const Source& src)
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

class tar_file_source : public basic_tar_file_source<file_source>
{
    typedef basic_tar_file_source<file_source> base_type;

public:
    explicit tar_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};


template<class Sink>
class basic_tar_file_sink_impl
{
private:
    typedef basic_ustar_file_sink_impl<Sink> ustar_type;

public:
    explicit basic_tar_file_sink_impl(const Sink& sink) : ustar_(sink)
    {
    }

    void create_entry(const tar::header& head)
    {
        using namespace boost::filesystem;

        tar::header local = head;

        const std::string& leaf = head.path.leaf();
        const path& branch = head.path.branch_path();
        const std::string prefix = branch.string();
        std::string long_link = head.link_name.string();
        if (head.format == tar::pax)
        {
            std::string ex;
            if ((leaf.size() > tar::raw_header::name_size) ||
                (prefix.size() > tar::raw_header::prefix_size))
            {
                std::string long_name = head.path.string();
                ex += tar::detail::make_ex_header_recoed("path", long_name);

                long_name.resize(tar::raw_header::name_size);
                local.path = path(long_name, no_check);
            }

            if ((head.uid < 0) || (head.uid > tar::raw_header::max_uid))
            {
                ex += tar::detail::
                    make_ex_header_recoed("uid", to_dec<char>(head.uid));

                local.uid = 0;
            }

            if ((head.gid < 0) || (head.gid > tar::raw_header::max_gid))
            {
                ex += tar::detail::
                    make_ex_header_recoed("gid", to_dec<char>(head.gid));

                local.gid = 0;
            }

            if (head.size > tar::raw_header::max_size)
            {
                ex += tar::detail::
                    make_ex_header_recoed("size", to_dec<char>(head.size));

                local.size = 0;
            }

            if (head.modified_time && (head.modified_time->nanoseconds != 0))
            {
                ex += tar::detail::make_ex_header_recoed(
                    "mtime",
                    tar::detail::from_timestamp(head.modified_time.get()));
            }

            if (head.access_time)
            {
                ex += tar::detail::make_ex_header_recoed(
                    "atime",
                    tar::detail::from_timestamp(head.access_time.get()));
            }

            if (head.change_time)
            {
                ex += tar::detail::make_ex_header_recoed(
                    "ctime",
                    tar::detail::from_timestamp(head.change_time.get()));
            }

            if (long_link.size() > tar::raw_header::name_size)
            {
                ex += tar::detail::make_ex_header_recoed("linkpath", long_link);

                long_link.resize(tar::raw_header::name_size);
                local.link_name = path(long_link, no_check);
            }

            if (tar::detail::is_non_ascii(head.user_name))
            {
                ex += tar::detail::
                    make_ex_header_recoed("uname", head.user_name);
            }

            if (tar::detail::is_non_ascii(head.group_name))
            {
                ex += tar::detail::
                    make_ex_header_recoed("gname", head.group_name);
            }

            if (!head.comment.empty())
            {
                ex += tar::detail::
                    make_ex_header_recoed("comment", head.comment);
            }

            if (!ex.empty())
                write_extended_header(head.path, ex);
        }
        else if (head.format == tar::gnu)
        {
            if ((leaf.size() > tar::raw_header::name_size) ||
                (prefix.size() > tar::raw_header::prefix_size))
            {
                std::string long_name = head.path.string();
                if (head.type == tar::type::directory)
                    long_name += '/';

                tar::header tmp;
                tmp.mode = 0;
                tmp.size = long_name.size()+1;
                tmp.type = tar::type::long_name;
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
                tmp.mode = 0;
                tmp.size = long_link.size()+1;
                tmp.type = tar::type::long_link;
                tmp.format = tar::gnu;
                tmp.group_name = "root";

                ustar_.create_entry(tmp);
                ustar_.write(long_link.c_str(), long_link.size()+1);
                ustar_.close();

                long_link.resize(tar::raw_header::name_size);
                local.link_name = path(long_link, no_check);
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

    void write_end_mark()
    {
        ustar_.write_end_mark();
    }

private:
    ustar_type ustar_;

    void write_extended_header(
        const boost::filesystem::path& ph, const std::string& ex)
    {
        tar::header tmp;
        tmp.path = ph;
        tmp.mode = 0;
        tmp.size = ex.size();
        tmp.type = tar::type::extended;
        tmp.format = tar::pax;

        ustar_.create_entry(tmp);
        ustar_.write(ex.c_str(), ex.size());
        ustar_.close();
    }
};

template<class Sink>
class basic_tar_file_sink
{
private:
    typedef basic_tar_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_tar_file_sink(const Sink& sink)
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

class tar_file_sink : public basic_tar_file_sink<file_sink>
{
private:
    typedef basic_tar_file_sink<file_sink> base_type;

public:
    explicit tar_file_sink(const std::string& filename)
        : base_type(file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_TAR_FILE_HPP
