//  archive.cpp: multi-format archiver

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

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
#include <boost/none.hpp>
#include <clocale>
#include <exception>
#include <iostream>

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

    entry() : type(fs_ex::status_unknown)
    {
    }
};

template<class Header>
struct header_traits;

template<>
struct header_traits<io_ex::lha::header>
{
    static io_ex::lha::header to_header(const entry& e)
    {
        io_ex::lha::header head;

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
            io_ex::lha::windows_timestamp ts;
            ts.last_write_time = e.last_write_time->to_windows_file_time();
            ts.last_access_time = e.last_access_time->to_windows_file_time();
            ts.creation_time = e.creation_time->to_windows_file_time();
            head.timestamp = ts;
        }

        if (e.permission)
            head.permission = e.permission.get();

        if (e.uid && e.gid)
        {
            io_ex::lha::unix_owner o;
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
struct header_traits<io_ex::tar::header>
{
    static io_ex::tar::header to_header(const entry& e)
    {
        io_ex::tar::header head;

        head.path = e.path;

        if (e.permission)
            head.mode = e.permission.get();
        if (e.uid)
            head.uid = e.uid.get();
        if (e.gid)
            head.gid = e.gid.get();
        if (e.file_size)
            head.size = e.file_size.get();

        if (e.last_write_time)
        {
            const fs_ex::timestamp& ts = e.last_write_time.get();
            head.modified_time =
                io_ex::tar::timestamp(ts.seconds, ts.nanoseconds);
        }

        if (e.last_access_time)
        {
            const fs_ex::timestamp& ts = e.last_access_time.get();
            head.access_time =
                io_ex::tar::timestamp(ts.seconds, ts.nanoseconds);
        }

        if (e.last_change_time)
        {
            const fs_ex::timestamp& ts = e.last_change_time.get();
            head.change_time =
                io_ex::tar::timestamp(ts.seconds, ts.nanoseconds);
        }

        if (!e.hard_link_path.empty())
        {
            head.type = io_ex::tar::type::link;
            head.link_name = e.hard_link_path;
        }
        else if (e.type == fs_ex::symlink_file)
        {
            head.type = io_ex::tar::type::symbolic_link;
            head.link_name = e.link_path;
        }
        else if (e.type == fs_ex::directory_file)
            head.type = io_ex::tar::type::directory;
        else
            head.type = io_ex::tar::type::regular;

        head.user_name = e.user_name;
        head.group_name = e.group_name;
        head.comment = e.comment;

        return head;
    }
};

template<>
struct header_traits<io_ex::zip::header>
{
    static io_ex::zip::header to_header(const entry& e)
    {
        io_ex::zip::header head;

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
        if (e.permission)
            head.permission = e.permission.get();

        head.comment = e.comment;

        if (e.last_write_time)
            head.modified_time = e.last_write_time->to_time_t();
        if (e.last_access_time)
            head.access_time = e.last_access_time->to_time_t();
        if (e.creation_time)
            head.creation_time = e.creation_time->to_time_t();

        if (e.uid)
            head.uid = e.uid.get();
        if (e.gid)
            head.gid = e.gid.get();

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
        fs::path::default_name_check(fs::no_check);

        std::auto_ptr<archiver_base> arc_ptr;
        const std::string filename(argv[1]);
        if (algo::ends_with(filename, ".lzh"))
        {
            arc_ptr.reset(new archiver<
                io_ex::lzh_file_sink>(io_ex::lzh_file_sink(filename)));
        }
        else if (algo::ends_with(filename, ".tar"))
        {
            arc_ptr.reset(new archiver<
                io_ex::tar_file_sink>(io_ex::tar_file_sink(filename)));
        }
        else if (algo::ends_with(filename, ".zip"))
        {
            arc_ptr.reset(new archiver<
                io_ex::zip_file_sink>(io_ex::zip_file_sink(filename)));
        }
        else if (
            algo::ends_with(filename, ".tar.bz2") ||
            algo::ends_with(filename, ".tbz2") ||
            algo::ends_with(filename, ".tb2") ||
            algo::ends_with(filename, ".tbz") )
        {
            arc_ptr.reset(new archiver<
                io_ex::tbz2_file_sink>(io_ex::tbz2_file_sink(filename)));
        }
        else if (
            algo::ends_with(filename, ".tar.gz") ||
            algo::ends_with(filename, ".tgz") )
        {
            arc_ptr.reset(new archiver<
                io_ex::tgz_file_sink>(io_ex::tgz_file_sink(filename)));
        }
        else if (algo::ends_with(filename, ".gz"))
        {
            if (argc != 3)
            {
                throw std::runtime_error(
                    "gzip cannot contain two files or more");
            }

            fs::path ph(argv[2], fs::native);
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

            fs::path ph(argv[2], fs::native);
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
            e.path = fs::path(argv[i], fs::native);

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
                e.attributes = s.attributes();
            if (s.has_permissions())
                e.permission = s.permissions();

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
                            e.path.native_file_string(),
                            std::ios_base::binary),
                        boost::ref(*arc_ptr)
                    );
                }
                catch (const io_ex::give_up_compression&)
                {
                    arc_ptr->rewind_entry();

                    io::copy(
                        io_ex::file_source(
                            e.path.native_file_string(),
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
