//  tar_file.hpp: POSIX tar file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_TAR_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_TAR_FILE_HPP

#include <hamigaki/iostreams/device/ustar_file.hpp>
#include <hamigaki/dec_format.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/optional.hpp>
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
    boost::filesystem::path link_path;
    std::string user_name;
    std::string group_name;
};

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

        if (!ext.link_path.empty())
            header_.link_name = ext.link_path;

        if (ext.uid)
            header_.uid = ext.uid.get();

        if (ext.gid)
            header_.gid = ext.gid.get();

        if (ext.size)
            header_.size = ext.size.get();

        if (!ext.path.empty())
            header_.path = ext.path;

        if (!ext.user_name.empty())
            header_.user_name = ext.user_name;

        if (!ext.group_name.empty())
            header_.group_name = ext.group_name;

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

        return path(buf, portable_posix_name);
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
                ext.path = path(beg, portable_posix_name);
            else if (key == "uid")
                ext.uid = hamigaki::from_dec<boost::intmax_t>(beg, end);
            else if (key == "gid")
                ext.gid = hamigaki::from_dec<boost::intmax_t>(beg, end);
            else if (key == "size")
                ext.size = hamigaki::from_dec<boost::uintmax_t>(beg, end);
            else if (key == "linkpath")
                ext.link_path = path(beg, portable_posix_name);
            else if (key == "uname")
                ext.user_name = beg;
            else if (key == "gname")
                ext.group_name = beg;
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

        if (head.format == tar::gnu)
        {
            std::string leaf = head.path.leaf();
            const path& branch = head.path.branch_path();
            std::string prefix = branch.string();
            if ((leaf.size() > 100) || (prefix.size() > 155))
            {
                const std::string long_name = head.path.string();

                tar::header tmp;
                tmp.mode = 0;
                tmp.size = long_name.size()+1;
                tmp.type = tar::type::long_name;
                tmp.format = tar::gnu;

                ustar_.create_entry(tmp);
                ustar_.write(long_name.c_str(), long_name.size()+1);
                ustar_.close();

                if (leaf.size() > 100)
                    leaf.resize(100);
                if (prefix.size() > 155)
                    prefix.resize(155);

                local.path = path(prefix, no_check);
                local.path /= path(leaf, no_check);
            }

            std::string long_link = head.link_name.string();
            if (long_link.size() > 100)
            {
                tar::header tmp;
                tmp.mode = 0;
                tmp.size = long_link.size()+1;
                tmp.type = tar::type::long_link;
                tmp.format = tar::gnu;

                ustar_.create_entry(tmp);
                ustar_.write(long_link.c_str(), long_link.size()+1);
                ustar_.close();

                long_link.resize(100);
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
