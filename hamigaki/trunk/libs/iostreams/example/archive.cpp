//  archive.cpp: multi-format archiver

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <boost/config.hpp>

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

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#elif defined(BOOST_HAS_UNISTD_H)
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace io_ex = hamigaki::iostreams;
namespace algo = boost::algorithm;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

struct timestamp
{
    boost::int64_t seconds;
    boost::uint32_t nanoseconds;

    timestamp() : seconds(0), nanoseconds(0)
    {
    }

    timestamp(std::time_t sec, boost::uint32_t nsec)
        : seconds(sec), nanoseconds(nsec)
    {
    }

    explicit timestamp(const io_ex::tar::timestamp& ts)
        : seconds(ts.seconds), nanoseconds(ts.nanoseconds)
    {
    }

    std::time_t to_time_t() const
    {
        // round up
        if (nanoseconds != 0)
            return static_cast<std::time_t>(seconds + 1);
        else
            return static_cast<std::time_t>(seconds);
    }

    boost::uint64_t to_file_time() const
    {
        // round up
        boost::int64_t sec = seconds;
        boost::uint32_t nsec = nanoseconds + 99;
        if (nsec >= 1000000000)
        {
            nsec -= 1000000000;
            ++sec;
        }

        return
            static_cast<boost::uint64_t>(sec + 11644473600LL) * 10000000ull +
            (nsec / 100);
    }

    static timestamp from_time_t(std::time_t t)
    {
        return timestamp(t, 0);
    }

    static timestamp from_file_time(boost::uint64_t ft)
    {
        boost::int64_t sec = static_cast<boost::int64_t>(ft / 10000000ull);

        boost::uint32_t nsec =
            static_cast<boost::uint32_t>(ft % 10000000ull) * 100;

        return timestamp(sec - 11644473600LL, nsec);
    }
};

enum file_type
{
    regular_file,
    hard_link_file,
    symlink_file,
    block_file,
    character_file,
    directory_file,
    fifo_file,
    socket_file,
    type_unknown
};

struct entry
{
    file_type type;
    boost::filesystem::path path;
    boost::filesystem::path link_path;
    boost::optional<boost::uintmax_t> compressed_size;
    boost::optional<boost::uintmax_t> file_size;
    boost::optional<timestamp> last_write_time;
    boost::optional<timestamp> last_access_time;
    boost::optional<timestamp> last_change_time;
    boost::optional<timestamp> creation_time;
    boost::optional<boost::uint16_t> attributes;
    boost::optional<boost::uint16_t> permission;
    boost::optional<boost::intmax_t> uid;
    boost::optional<boost::intmax_t> gid;
    std::string user_name;
    std::string group_name;
    std::string comment;

    std::string path_string() const
    {
        if (type == directory_file)
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
            ts.last_write_time = e.last_write_time->to_file_time();
            ts.last_access_time = e.last_access_time->to_file_time();
            ts.creation_time = e.creation_time->to_file_time();
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
            const timestamp& ts = e.last_write_time.get();
            head.modified_time =
                io_ex::tar::timestamp(ts.seconds, ts.nanoseconds);
        }

        if (e.last_access_time)
        {
            const timestamp& ts = e.last_access_time.get();
            head.access_time =
                io_ex::tar::timestamp(ts.seconds, ts.nanoseconds);
        }

        if (e.last_change_time)
        {
            const timestamp& ts = e.last_change_time.get();
            head.change_time =
                io_ex::tar::timestamp(ts.seconds, ts.nanoseconds);
        }

        if (e.type == hard_link_file)
            head.type = io_ex::tar::type::link;
        else if (e.type == symlink_file)
            head.type = io_ex::tar::type::symbolic_link;
        else if (e.type == directory_file)
            head.type = io_ex::tar::type::directory;
        else
            head.type = io_ex::tar::type::regular;

        head.link_name = e.link_path;
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

#if defined(BOOST_WINDOWS)
inline boost::uint64_t to_uint64(const ::FILETIME& ft)
{
    return
        (static_cast<boost::uint64_t>(ft.dwHighDateTime) << 32) |
        static_cast<boost::uint64_t>(ft.dwLowDateTime);
}

void get_timestamp_impl(
    ::HANDLE handle, io_ex::lha::windows_timestamp& ts)
{
    ::FILETIME creation_time, last_write_time, last_access_time;
    ::GetFileTime(handle, &creation_time,
        &last_access_time, &last_write_time);

    ts.creation_time = to_uint64(creation_time);
    ts.last_write_time = to_uint64(last_write_time);
    ts.last_access_time = to_uint64(last_access_time);
}

io_ex::lha::windows_timestamp get_file_timestamp(const fs::path& ph)
{
    ::HANDLE handle = ::CreateFileA(
        ph.native_file_string().c_str(), GENERIC_READ, 0, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("CreateFile error");
    io_ex::lha::windows_timestamp ts;
    get_timestamp_impl(handle, ts);
    ::CloseHandle(handle);
    return ts;
}

boost::optional<io_ex::lha::windows_timestamp>
get_directory_timestamp(const fs::path& ph)
{
    ::HANDLE handle = ::CreateFileA(
        ph.native_file_string().c_str(), GENERIC_READ, 0, 0,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
    // Win9X does not support FILE_FLAG_BACKUP_SEMANTICS flag
    if (handle == INVALID_HANDLE_VALUE)
        return boost::none;
    io_ex::lha::windows_timestamp ts;
    get_timestamp_impl(handle, ts);
    ::CloseHandle(handle);
    return ts;
}

fs::path read_link(const fs::path& ph)
{
    throw std::runtime_error("symbolic link is not supported");
}
#elif defined(BOOST_HAS_UNISTD_H)
fs::path read_link(const fs::path& ph)
{
    const std::string& s = ph.native_file_string();

    struct stat st;
    if (::lstat(s.c_str(), &st) != 0)
        throw std::runtime_error("lstat error");

    boost::scoped_array<char> buf(new char[st.st_size+1]);
    ::ssize_t amt = ::readlink(s.c_str(), &buf[0], st.st_size);
    if (amt != static_cast< ::ssize_t>(st.st_size))
        throw std::runtime_error("bad symbolic link");

    buf[st.st_size] = '\0';
    return fs::path(&buf[0], fs::native);
}
#endif

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
            if (fs::symbolic_link_exists(e.path))
            {
                e.type = symlink_file;
                e.link_path = read_link(e.path);
            }
            else if (fs::is_directory(e.path))
            {
                e.type = directory_file;
                e.attributes = static_cast<
                    boost::uint16_t>(io_ex::msdos_attributes::directory);
            }
            else
            {
                e.type = regular_file;
                e.file_size = fs::file_size(e.path);
            }

#if defined(BOOST_WINDOWS)
            e.attributes = ::GetFileAttributes(e.path_string().c_str());
            boost::optional<io_ex::lha::windows_timestamp> ts;
            if (fs::is_directory(e.path))
                ts = get_directory_timestamp(e.path);
            else
                ts = get_file_timestamp(e.path);
            if (ts)
            {
                e.last_write_time =
                    timestamp::from_file_time(ts->last_write_time);
                e.last_access_time =
                    timestamp::from_file_time(ts->last_access_time);
                e.creation_time =
                    timestamp::from_file_time(ts->creation_time);
            }
#elif defined(BOOST_HAS_UNISTD_H)
            struct stat st;
            if (::lstat(e.path_string().c_str(), &st) == 0)
            {
                e.permission = st.st_mode;

                e.gid = st.st_gid;
                e.uid = st.st_uid;

                e.last_write_time = timestamp::from_time_t(st.st_mtime);
                e.last_access_time = timestamp::from_time_t(st.st_atime);
                e.last_change_time = timestamp::from_time_t(st.st_ctime);
            }
#endif

            arc_ptr->create_entry(e);

            if (!fs::is_directory(e.path))
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
