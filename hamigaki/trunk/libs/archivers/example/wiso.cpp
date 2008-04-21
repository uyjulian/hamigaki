// wiso.cpp: a simple ISO image archiver program (Unicode version)

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/iso_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
    #if !defined(__GNUC__)
        #pragma comment(lib, "shell32.lib")
    #endif
#else
    #include <cstdlib>
#endif

namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;
namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

#if defined(BOOST_WINDOWS)
class scoped_local_memory : private boost::noncopyable
{
public:
    explicit scoped_local_memory(void* p) : ptr_(p)
    {
    }

    ~scoped_local_memory()
    {
        if (ptr_)
            ::LocalFree(ptr_);
    }

private:
    void* ptr_;
};

void get_cmd_line_args(int, char**, std::vector<std::wstring>& args)
{
    int argc = 0;
    wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    scoped_local_memory guard(argv);
    args.assign(argv, argv+argc);
}
#else
std::wstring narrow_to_wide(const char* s)
{
    std::size_t size = std::mbstowcs(0, s, 0);
    if (size == static_cast<std::size_t>(-1))
        throw std::runtime_error("failed mbstowcs()");

    boost::scoped_array<wchar_t> buf(new wchar_t[size+1]);
    size = std::mbstowcs(buf.get(), s, size+1);
    if (size == static_cast<std::size_t>(-1))
        throw std::runtime_error("failed mbstowcs()");

    return std::wstring(buf.get(), size);
}

void get_cmd_line_args(int argc, char** argv, std::vector<std::wstring>& args)
{
    std::vector<std::wstring> tmp;
    tmp.reserve(count);
    std::transform(argv, argv+argc, std::back_inserter(tmp), &narrow_to_wide);
    args.swap(tmp);
}
#endif

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
        {
            std::cerr << "Usage: wiso (archive) (filename) ..." << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        std::vector<std::wstring> args;
        get_cmd_line_args(argc, argv, args);

        ar::wiso_file_sink iso(argv[1]);

        {
            ar::iso::wvolume_desc desc;
            desc.rrip = ar::iso::rrip_1991a;
            iso.add_volume_desc(desc);
        }
        {
            ar::iso::wvolume_desc jol_desc;
            jol_desc.set_joliet();
            iso.add_volume_desc(jol_desc);
        }

        for (int i = 2; i < argc; ++i)
        {
            ar::iso::wheader head;
            head.path = fs::wpath(args[i]);

            const fs_ex::file_status& s = fs_ex::status(head.path);

            head.type(s.type());
            if (is_regular(s))
                head.file_size = s.file_size();

            ar::iso::posix::file_attributes attr;
            if (s.has_permissions())
                attr.permissions = s.permissions();
            else if (is_directory(s))
                attr.permissions = fs_ex::file_permissions::directory | 0755u;
            else
                attr.permissions = fs_ex::file_permissions::regular | 0644u;
            attr.links = 1u;
            if (s.has_uid())
                attr.uid = s.uid();
            if (s.has_gid())
                attr.gid = s.gid();
            attr.serial_no = static_cast<unsigned>(i);
            head.attributes = attr;

            typedef ar::iso::date_time time_time;
            head.last_write_time =
                time_time::from_timestamp(s.last_write_time());
            head.last_access_time =
                time_time::from_timestamp(s.last_access_time());
            if (s.has_last_change_time())
            {
                head.last_change_time =
                    time_time::from_timestamp(s.last_change_time());
            }

            iso.create_entry(head);

            if (is_regular(s))
            {
                fs::ifstream is(head.path, std::ios_base::binary);
                io::copy(is, iso);
            }
        }
        iso.close_archive();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
