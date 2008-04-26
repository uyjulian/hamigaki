// archive.cpp: multi-format archiver

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/lzh_file.hpp>
#include <hamigaki/archivers/tbz2_file.hpp>
#include <hamigaki/archivers/tgz_file.hpp>
#include <hamigaki/archivers/zip_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/none.hpp>
#include <clocale>
#include <exception>
#include <iostream>

namespace ar = hamigaki::archivers;
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
    boost::optional<boost::uint16_t> permissions;
    boost::optional<boost::intmax_t> uid;
    boost::optional<boost::intmax_t> gid;
    std::string user_name;
    std::string group_name;
    std::string comment;

    entry() : type(fs_ex::status_unknown)
    {
    }
};

template<class Header>
struct header_traits;

template<>
struct header_traits<ar::lha::header>
{
    static ar::lha::header to_header(const entry& e)
    {
        ar::lha::header head;

        if (e.compressed_size)
            head.compressed_size = e.compressed_size.get();
        if (e.file_size)
            head.file_size = e.file_size.get();
        if (e.last_write_time)
            head.update_time = e.last_write_time->to_time_t();
        if (e.attributes)
            head.attributes = e.attributes.get();

        head.path = e.path;
        head.link_path = e.link_path;

        if (e.last_write_time && e.last_access_time && e.creation_time)
        {
            ar::lha::windows::timestamp ts;
            ts.last_write_time = e.last_write_time->to_windows_file_time();
            ts.last_access_time = e.last_access_time->to_windows_file_time();
            ts.creation_time = e.creation_time->to_windows_file_time();
            head.timestamp = ts;
        }

        if (e.permissions)
            head.permissions = e.permissions.get();

        if (e.uid && e.gid)
        {
            ar::lha::posix::gid_uid o;
            o.gid = e.gid.get();
            o.uid = e.uid.get();
            head.owner = o;
        }

        head.group_name = e.group_name;
        head.user_name = e.user_name;
        head.comment = e.comment;

        return head;
    }
};

template<>
struct header_traits<ar::tar::header>
{
    static ar::tar::header to_header(const entry& e)
    {
        ar::tar::header head;

        head.path = e.path;

        if (e.permissions)
            head.permissions = e.permissions.get();
        else if (e.type == fs_ex::directory_file)
            head.permissions = 0755;

        if (e.uid)
            head.uid = e.uid.get();
        if (e.gid)
            head.gid = e.gid.get();
        if (e.file_size)
            head.file_size = e.file_size.get();

        if (e.last_write_time)
            head.modified_time = e.last_write_time.get();

        if (e.last_access_time)
            head.access_time = e.last_access_time.get();

        if (e.last_change_time)
            head.change_time = e.last_change_time.get();

        if (!e.hard_link_path.empty())
        {
            head.type_flag = ar::tar::type_flag::link;
            head.link_path = e.hard_link_path;
        }
        else if (e.type == fs_ex::symlink_file)
        {
            head.type_flag = ar::tar::type_flag::symlink;
            head.link_path = e.link_path;
        }
        else if (e.type == fs_ex::directory_file)
            head.type_flag = ar::tar::type_flag::directory;
        else
            head.type_flag = ar::tar::type_flag::regular;

        head.user_name = e.user_name;
        head.group_name = e.group_name;
        head.comment = e.comment;

        return head;
    }
};

template<>
struct header_traits<ar::zip::header>
{
    static ar::zip::header to_header(const entry& e)
    {
        ar::zip::header head;

        head.path = e.path;
        head.link_path = e.link_path;

        if (e.last_write_time)
            head.update_time = e.last_write_time->to_time_t();

        if (e.compressed_size)
            head.compressed_size = e.compressed_size.get();
        if (e.file_size)
            head.file_size = e.file_size.get();
        if (e.attributes)
            head.attributes = e.attributes.get();
        if (e.permissions)
            head.permissions = e.permissions.get();

        head.comment = e.comment;

        if (e.last_write_time)
            head.modified_time = e.last_write_time->to_time_t();
        if (e.last_access_time)
            head.access_time = e.last_access_time->to_time_t();
        if (e.creation_time)
            head.creation_time = e.creation_time->to_time_t();

        if (e.uid)
            head.uid = static_cast<boost::uint16_t>(e.uid.get());
        if (e.gid)
            head.gid = static_cast<boost::uint16_t>(e.gid.get());

        return head;
    }
};

class archiver_base
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    virtual ~archiver_base(){}

    void create_entry(const entry& e)
    {
        return do_create_entry(e);
    }

    void rewind_entry()
    {
        return do_rewind_entry();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return do_write(s, n);
    }

    void close()
    {
        return do_close();
    }

    void close_archive()
    {
        return do_close_archive();
    }

private:
    virtual void do_create_entry(const entry& e) = 0;
    virtual void do_rewind_entry() = 0;
    virtual std::streamsize do_write(const char* s, std::streamsize n) = 0;
    virtual void do_close() = 0;
    virtual void do_close_archive() = 0;
};

template<class Sink>
class archiver : public archiver_base
{
public:
    explicit archiver(const Sink& sink) : sink_(sink) {}

private:
    Sink sink_;

    void do_create_entry(const entry& e) // virtual
    {
        typedef typename Sink::header_type header_type;
        return sink_.create_entry(header_traits<header_type>::to_header(e));
    }

    void do_rewind_entry() // virtual
    {
        sink_.rewind_entry();
    }

    std::streamsize do_write(const char* s, std::streamsize n) // virtual
    {
        return sink_.write(s, n);
    }

    void do_close() // virtual
    {
        sink_.close();
    }

    void do_close_archive() // virtual
    {
        sink_.close_archive();
    }
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
        {
            std::cerr << "Usage: archive (archive) (filename) ..." << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        std::auto_ptr<archiver_base> arc_ptr;
        const std::string filename(argv[1]);
        if (algo::ends_with(filename, ".lzh"))
        {
            arc_ptr.reset(new archiver<
                ar::lzh_file_sink>(ar::lzh_file_sink(filename)));
        }
        else if (algo::ends_with(filename, ".tar"))
        {
            arc_ptr.reset(new archiver<
                ar::tar_file_sink>(ar::tar_file_sink(filename)));
        }
        else if (algo::ends_with(filename, ".zip"))
        {
            arc_ptr.reset(new archiver<
                ar::zip_file_sink>(ar::zip_file_sink(filename)));
        }
        else if (
            algo::ends_with(filename, ".tar.bz2") ||
            algo::ends_with(filename, ".tbz2") ||
            algo::ends_with(filename, ".tb2") ||
            algo::ends_with(filename, ".tbz") )
        {
            arc_ptr.reset(new archiver<
                ar::tbz2_file_sink>(ar::tbz2_file_sink(filename)));
        }
        else if (
            algo::ends_with(filename, ".tar.gz") ||
            algo::ends_with(filename, ".tgz") )
        {
            arc_ptr.reset(new archiver<
                ar::tgz_file_sink>(ar::tgz_file_sink(filename)));
        }
        else if (algo::ends_with(filename, ".gz"))
        {
            if (argc != 3)
            {
                throw std::runtime_error(
                    "gzip cannot contain two files or more");
            }

            fs::path ph(argv[2]);
            if (fs::is_directory(ph))
                throw std::runtime_error("gzip cannot compress a directory");

            io::gzip_params params;
            params.file_name = ph.leaf();
            params.mtime = fs::last_write_time(ph);

            io::copy(
                io_ex::file_source(argv[2]),
                io::compose(
                    io::gzip_compressor(params), io_ex::file_sink(filename)
                )
            );

            return 0;
        }
        else if (algo::ends_with(filename, ".bz2"))
        {
            if (argc != 3)
            {
                throw std::runtime_error(
                    "bzip2 cannot contain two files or more");
            }

            fs::path ph(argv[2]);
            if (fs::is_directory(ph))
                throw std::runtime_error("bzip2 cannot compress a directory");

            io::copy(
                io_ex::file_source(argv[2]),
                io::compose(
                    io::bzip2_compressor(), io_ex::file_sink(filename)
                )
            );

            return 0;
        }
        else
            throw std::runtime_error("unsupported format");

        for (int i = 2; i < argc; ++i)
        {
            entry e;
            e.path = fs::path(argv[i]);

            const fs_ex::file_status& s = fs_ex::symlink_status(e.path);

            if (is_symlink(s))
            {
                e.type = fs_ex::symlink_file;
                e.link_path = fs_ex::symlink_target(e.path);
            }
            else if (is_directory(s))
                e.type = fs_ex::directory_file;
            else
            {
                e.type = fs_ex::regular_file;
                e.file_size = fs::file_size(e.path);
            }

            if (s.has_attributes())
                e.attributes = static_cast<boost::uint16_t>(s.attributes());
            if (s.has_permissions())
                e.permissions = s.permissions();

            e.last_write_time = s.last_write_time();
            e.last_access_time = s.last_access_time();
            if (s.has_last_change_time())
                e.last_change_time = s.last_change_time();
            if (s.has_creation_time())
                e.creation_time = s.creation_time();

            if (s.has_uid())
                e.uid = s.uid();
            if (s.has_gid())
                e.gid = s.gid();

            arc_ptr->create_entry(e);

            if (!is_directory(s))
            {
                try
                {
                    io::copy(
                        io_ex::file_source(
                            e.path.file_string(),
                            std::ios_base::binary),
                        boost::ref(*arc_ptr)
                    );
                }
                catch (const ar::give_up_compression&)
                {
                    arc_ptr->rewind_entry();

                    io::copy(
                        io_ex::file_source(
                            e.path.file_string(),
                            std::ios_base::binary),
                        boost::ref(*arc_ptr)
                    );
                }
            }
        }
        arc_ptr->close_archive();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
