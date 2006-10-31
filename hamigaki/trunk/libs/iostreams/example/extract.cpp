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
    static entry to_entry(const io_ex::lha::header& head)
    {
        entry e;

        if (head.is_symbolic_link())
            e.type = symlink_file;
        else if (head.is_directory())
            e.type = directory_file;
        else
            e.type = regular_file;

        e.path = head.path;
        e.link_path = head.link_path;
        e.compressed_size = head.compressed_size;
        e.file_size = head.file_size;

        if (head.timestamp)
        {
            const io_ex::lha::windows_timestamp& ts = head.timestamp.get();
            e.last_write_time = timestamp::from_file_time(ts.last_write_time);
            e.last_access_time = timestamp::from_file_time(ts.last_access_time);
            e.creation_time = timestamp::from_file_time(ts.creation_time);
        }
        else
            e.last_write_time = timestamp::from_time_t(head.update_time);

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

        if (head.type == io_ex::tar::type::link)
            e.type = hard_link_file;
        else if (head.type == io_ex::tar::type::symbolic_link)
            e.type = symlink_file;
        else if (head.type == io_ex::tar::type::directory)
            e.type = directory_file;
        else
            e.type = regular_file;

        e.path = head.path;
        e.link_path = head.link_name;
        e.compressed_size = head.size;
        e.file_size = head.size;

        if (head.modified_time)
            e.last_write_time = timestamp(head.modified_time.get());
        if (head.access_time)
            e.last_access_time = timestamp(head.access_time.get());
        if (head.change_time)
            e.last_change_time = timestamp(head.change_time.get());

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
            e.type = symlink_file;
        else if (head.is_directory())
            e.type = directory_file;
        else
            e.type = regular_file;

        e.path = head.path;
        e.link_path = head.link_path;
        e.compressed_size = head.compressed_size;
        e.file_size = head.file_size;

        if (head.modified_time)
            e.last_write_time = timestamp::from_time_t(*head.modified_time);
        else
            e.last_write_time = timestamp::from_time_t(head.update_time);

        if (head.access_time)
            e.last_access_time = timestamp::from_time_t(*head.access_time);
        if (head.creation_time)
            e.creation_time = timestamp::from_time_t(*head.creation_time);

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

#if defined(BOOST_WINDOWS)
::FILETIME make_file_time(boost::uint64_t n)
{
    ::FILETIME ft;
    ft.dwLowDateTime = static_cast<boost::uint32_t>(n);
    ft.dwHighDateTime = static_cast<boost::uint32_t>(n >> 32);
    return ft;
}

void set_timestamp_impl(::HANDLE handle, const entry& e)
{
    ::FILETIME creation_time;
    if (e.creation_time)
        creation_time = make_file_time(e.creation_time->to_file_time());

    ::FILETIME last_write_time;
    if (e.last_write_time)
        last_write_time = make_file_time(e.last_write_time->to_file_time());

    ::FILETIME last_access_time;
    if (e.last_access_time)
        last_access_time = make_file_time(e.last_access_time->to_file_time());

    ::SetFileTime(
        handle,
        e.creation_time ? &creation_time : static_cast< ::FILETIME*>(0),
        e.last_access_time ? &last_access_time : static_cast< ::FILETIME*>(0),
        e.last_write_time ? &last_write_time : static_cast< ::FILETIME*>(0)
    );
}

void set_file_timestamp(const fs::path& ph, const entry& e)
{
    ::HANDLE handle = ::CreateFileA(
        ph.native_file_string().c_str(), GENERIC_WRITE, 0, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("CreateFile error");
    set_timestamp_impl(handle, e);
    ::CloseHandle(handle);
}

void set_directory_timestamp(const fs::path& ph, const entry& e)
{
    ::HANDLE handle = ::CreateFileA(
        ph.native_directory_string().c_str(), GENERIC_WRITE, 0, 0,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
    // Win9X does not support FILE_FLAG_BACKUP_SEMANTICS flag
    if (handle == INVALID_HANDLE_VALUE)
        return;
    set_timestamp_impl(handle, e);
    ::CloseHandle(handle);
}

void create_hard_link(const fs::path& old_fp, const fs::path& new_fp)
{
    throw std::runtime_error("hard link is not supported");
}

void create_symlink(const fs::path& old_fp, const fs::path& new_fp)
{
    throw std::runtime_error("symbolic link is not supported");
}
#elif defined(BOOST_HAS_UNISTD_H)
void create_hard_link(const fs::path& old_fp, const fs::path& new_fp)
{
    ::link(
        old_fp.native_file_string().c_str(),
        new_fp.native_file_string().c_str()
    );
}

void create_symlink(const fs::path& old_fp, const fs::path& new_fp)
{
    ::symlink(
        old_fp.native_file_string().c_str(),
        new_fp.native_file_string().c_str()
    );
}
#endif

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
        else
            throw std::runtime_error("unsupported format");

        while (ext_ptr->next_entry())
        {
            const entry& e = ext_ptr->current_entry();

            std::cout << e.path_string() << '\n';

            if (e.type == hard_link_file)
                ::create_hard_link(e.link_path, e.path);
            else if (e.type == symlink_file)
                ::create_symlink(e.link_path, e.path);
            else if (e.type == directory_file)
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

#if defined(BOOST_WINDOWS)
            if (e.attributes)
            {
                ::DWORD attr = e.attributes.get();
                ::SetFileAttributes(e.path_string().c_str(), attr);
            }
#elif defined(BOOST_HAS_UNISTD_H)
            if (e.permission)
                ::chmod(e.path_string().c_str(), e.permission.get());
            if (e.uid || e.gid)
            {
                ::chown(
                    e.path_string().c_str(),
                    e.uid ? e.uid.get() : -1,
                    e.gid ? e.gid.get() : -1
                );
            }
#endif

#if defined(BOOST_WINDOWS)
            if (e.last_write_time || e.last_access_time || e.creation_time)
            {
                if (e.type == directory_file)
                    set_directory_timestamp(e.path, e);
                else
                    set_file_timestamp(e.path, e);
            }
#else
            if (e.type != directory_file)
                fs::last_write_time(e.path, e.last_write_time->to_time_t());
#endif
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
