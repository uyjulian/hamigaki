//  extract.cpp: multi-format extractor

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.


// Security warning:
// This program never check the validity of paths in the archive.
// See http://www.forest.impress.co.jp/article/2004/07/30/arcsecurity.html .
// (The above link is Japanese site)

#include <boost/config.hpp>

#include <hamigaki/filesystem/operations.hpp>
#include <hamigaki/iostreams/device/lzh_file.hpp>
#include <hamigaki/iostreams/device/tbz2_file.hpp>
#include <hamigaki/iostreams/device/tgz_file.hpp>
#include <hamigaki/iostreams/device/zip_file.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace fs_ex = hamigaki::filesystem;
namespace io_ex = hamigaki::iostreams;
namespace algo = boost::algorithm;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

struct entry
{
    fs_ex::file_type type;
    boost::filesystem::path path;
    boost::filesystem::path link_path;
    boost::filesystem::path hard_link_path;
    boost::optional<boost::uintmax_t> compressed_size;
    boost::optional<boost::uintmax_t> file_size;
    boost::optional<fs_ex::timestamp> last_write_time;
    boost::optional<fs_ex::timestamp> last_access_time;
    boost::optional<fs_ex::timestamp> last_change_time;
    boost::optional<fs_ex::timestamp> creation_time;
    boost::optional<boost::uint16_t> attributes;
    boost::optional<boost::uint16_t> permission;
    boost::optional<boost::intmax_t> uid;
    boost::optional<boost::intmax_t> gid;
    std::string user_name;
    std::string group_name;
    std::string comment;

    std::string path_string() const
    {
        if (type == fs_ex::directory_file)
            return path.native_directory_string();
        else
            return path.native_file_string();
    }
};

template<class Header>
struct header_traits;

template<>
struct header_traits<io_ex::lha::header>
{
    static entry to_entry(const io_ex::lha::header& head)
    {
        entry e;

        if (head.is_symbolic_link())
            e.type = fs_ex::symlink_file;
        else if (head.is_directory())
            e.type = fs_ex::directory_file;
        else
            e.type = fs_ex::regular_file;

        e.path = head.path;
        e.link_path = head.link_path;
        e.compressed_size = head.compressed_size;
        e.file_size = head.file_size;

        if (head.timestamp)
        {
            const io_ex::lha::windows_timestamp& ts = head.timestamp.get();
            e.last_write_time =
                fs_ex::timestamp::from_windows_file_time(ts.last_write_time);
            e.last_access_time =
                fs_ex::timestamp::from_windows_file_time(ts.last_access_time);
            e.creation_time =
                fs_ex::timestamp::from_windows_file_time(ts.creation_time);
        }
        else
            e.last_write_time = fs_ex::timestamp::from_time_t(head.update_time);

        e.attributes = head.attributes;

        if (head.permission)
            e.permission = head.permission.get();

        if (head.owner)
        {
            e.uid = head.owner->uid;
            e.gid = head.owner->gid;
        }

        e.user_name = head.user_name;
        e.group_name = head.group_name;
        e.comment = head.comment;

        return e;
    }
};

template<>
struct header_traits<io_ex::tar::header>
{
    static entry to_entry(const io_ex::tar::header& head)
    {
        entry e;

        if (head.type == io_ex::tar::type::symbolic_link)
            e.type = fs_ex::symlink_file;
        else if (head.type == io_ex::tar::type::directory)
            e.type = fs_ex::directory_file;
        else
            e.type = fs_ex::regular_file;

        e.path = head.path;
        if (head.type == io_ex::tar::type::link)
            e.hard_link_path = head.link_name;
        else
            e.link_path = head.link_name;
        e.compressed_size = head.size;
        e.file_size = head.size;

        if (head.modified_time)
        {
            e.last_write_time =
                fs_ex::timestamp(
                    head.modified_time->seconds,
                    head.modified_time->nanoseconds);
        }
        if (head.access_time)
        {
            e.last_access_time =
                fs_ex::timestamp(
                    head.access_time->seconds,
                    head.access_time->nanoseconds);
        }
        if (head.change_time)
        {
            e.last_change_time =
                fs_ex::timestamp(
                    head.change_time->seconds,
                    head.change_time->nanoseconds);
        }

        e.permission = head.mode;
        e.uid = head.uid;
        e.gid = head.gid;

        e.user_name = head.user_name;
        e.group_name = head.group_name;
        e.comment = head.comment;

        return e;
    }
};

template<>
struct header_traits<io_ex::zip::header>
{
    static entry to_entry(const io_ex::zip::header& head)
    {
        entry e;

        if (head.is_symbolic_link())
            e.type = fs_ex::symlink_file;
        else if (head.is_directory())
            e.type = fs_ex::directory_file;
        else
            e.type = fs_ex::regular_file;

        e.path = head.path;
        e.link_path = head.link_path;
        e.compressed_size = head.compressed_size;
        e.file_size = head.file_size;

        if (head.modified_time)
        {
            e.last_write_time =
                fs_ex::timestamp::from_time_t(*head.modified_time);
        }
        else
            e.last_write_time = fs_ex::timestamp::from_time_t(head.update_time);

        if (head.access_time)
        {
            e.last_access_time =
                fs_ex::timestamp::from_time_t(*head.access_time);
        }
        if (head.creation_time)
        {
            e.creation_time =
                fs_ex::timestamp::from_time_t(*head.creation_time);
        }

        e.attributes = head.attributes;
        e.permission = head.permission;

        if (head.uid)
            e.uid = head.uid.get();

        if (head.gid)
            e.gid = head.gid.get();

        e.comment = head.comment;

        return e;
    }
};

class extractor_base
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::device_tag
    {};

    virtual ~extractor_base(){}

    bool next_entry()
    {
        return do_next_entry();
    }

    entry current_entry() const
    {
        return do_current_entry();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return do_read(s, n);
    }

private:
    virtual bool do_next_entry() = 0;
    virtual entry do_current_entry() const = 0;
    virtual std::streamsize do_read(char* s, std::streamsize n) = 0;
};

template<class Source>
class extractor : public extractor_base
{
public:
    explicit extractor(const Source& src) : src_(src) {}

private:
    Source src_;

    bool do_next_entry() // virtual
    {
        return src_.next_entry();
    }

    entry do_current_entry() const // virtual
    {
        typedef typename Source::header_type header_type;
        return header_traits<header_type>::to_entry(src_.header());
    }

    std::streamsize do_read(char* s, std::streamsize n) // virtual
    {
        return src_.read(s, n);
    }
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: extract (filename)" << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");
        fs::path::default_name_check(fs::no_check);

        std::auto_ptr<extractor_base> ext_ptr;
        const std::string filename(argv[1]);
        if (!fs::exists(filename))
            throw std::runtime_error("file not found");

        if (algo::ends_with(filename, ".lzh"))
        {
            ext_ptr.reset(new extractor<
                io_ex::lzh_file_source>(io_ex::lzh_file_source(filename)));
        }
        else if (algo::ends_with(filename, ".tar"))
        {
            ext_ptr.reset(new extractor<
                io_ex::tar_file_source>(io_ex::tar_file_source(filename)));
        }
        else if (algo::ends_with(filename, ".zip"))
        {
            ext_ptr.reset(new extractor<
                io_ex::zip_file_source>(io_ex::zip_file_source(filename)));
        }
        else if (
            algo::ends_with(filename, ".tar.bz2") ||
            algo::ends_with(filename, ".tbz2") ||
            algo::ends_with(filename, ".tb2") ||
            algo::ends_with(filename, ".tbz") )
        {
            ext_ptr.reset(new extractor<
                io_ex::tbz2_file_source>(io_ex::tbz2_file_source(filename)));
        }
        else if (
            algo::ends_with(filename, ".tar.gz") ||
            algo::ends_with(filename, ".tgz") )
        {
            ext_ptr.reset(new extractor<
                io_ex::tgz_file_source>(io_ex::tgz_file_source(filename)));
        }
        else if (algo::ends_with(filename, ".gz"))
        {
            const std::string& leaf = fs::path(filename, fs::native).leaf();
            if (leaf.size() < 4)
                throw std::runtime_error("bad filename");

            std::string new_name = leaf.substr(0, leaf.size()-3);

            io::gzip_decompressor gzip;
            io_ex::file_source src(filename);
            char buf[4096];
            std::streamsize amt = io::read(gzip, src, buf, sizeof(buf));

            const std::string& org_name = gzip.file_name();
            if (!org_name.empty())
                new_name = org_name;

            std::cout << new_name << '\n';

            io_ex::file_sink sink(new_name);
            if (amt != -1)
                io::write(sink, buf, amt);

            io::copy(
                io::compose(boost::ref(gzip), boost::ref(src)),
                boost::ref(sink)
            );

            if (gzip.mtime())
                fs::last_write_time(new_name, gzip.mtime());

            return 0;
        }
        else if (algo::ends_with(filename, ".bz2"))
        {
            const std::string& leaf = fs::path(filename, fs::native).leaf();
            if (leaf.size() < 5)
                throw std::runtime_error("bad filename");

            const std::string new_name = leaf.substr(0, leaf.size()-4);
            std::cout << new_name << '\n';

            io::copy(
                io::compose(
                    io::bzip2_decompressor(), io_ex::file_source(filename)
                ),
                io_ex::file_sink(new_name)
            );

            return 0;
        }
        else
            throw std::runtime_error("unsupported format");

        while (ext_ptr->next_entry())
        {
            const entry& e = ext_ptr->current_entry();

            std::cout << e.path_string() << '\n';

            if (!e.hard_link_path.empty())
                fs_ex::create_hard_link(e.hard_link_path, e.path);
            else if (e.type == fs_ex::symlink_file)
                fs_ex::create_symlink(e.link_path, e.path);
            else if (e.type == fs_ex::directory_file)
                fs::create_directories(e.path);
            else
            {
                fs::create_directories(e.path.branch_path());

                io::copy(
                    boost::ref(*ext_ptr),
                    io_ex::file_sink(
                        e.path.native_file_string(), std::ios_base::binary),
                    1024*8
                );
            }

            // Note:
            // The POSIX chown() clears S_ISUID/S_ISGID bits.
            // So, we must call owner() before calling file_mode().
            int ec = 0;
            fs_ex::change_owner(e.path, e.uid, e.gid, ec);

            fs_ex::file_attributes attr = 0;
            fs_ex::file_permissions perm =
                (e.type == fs_ex::directory_file) ? 0755 : 0644;

            if (e.attributes)
            {
                boost::uint16_t flags = e.attributes.get();
                if ((flags & io_ex::msdos_attributes::read_only) != 0)
                    attr |= fs_ex::read_only;
                if ((flags & io_ex::msdos_attributes::hidden) != 0)
                    attr |= fs_ex::hidden;
                if ((flags & io_ex::msdos_attributes::archive) != 0)
                    attr |= fs_ex::archive;
            }

            if (e.permission)
            {
                boost::uint16_t flags = e.permission.get();
                if ((flags & 04000) != 0)
                    attr |= fs_ex::set_uid;
                if ((flags & 02000) != 0)
                    attr |= fs_ex::set_gid;
                if ((flags & 01000) != 0)
                    attr |= fs_ex::sticky;
                perm = flags & 0777;
            }

            fs_ex::file_mode(e.path, attr, perm);

            if (e.last_write_time)
                fs_ex::last_write_time(e.path, e.last_write_time.get());
            if (e.last_access_time)
                fs_ex::last_access_time(e.path, e.last_access_time.get());
            if (e.creation_time)
                fs_ex::creation_time(e.path, e.creation_time.get());
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
