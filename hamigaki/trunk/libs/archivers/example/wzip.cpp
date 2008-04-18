// zip.cpp: a simple ZIP compressing program (Unicode version)

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <boost/config.hpp>

#include <hamigaki/archivers/zip_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <hamigaki/iostreams/device/file_descriptor.hpp>
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
            std::cerr << "Usage: wzip (archive) (filename) ..." << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        std::vector<std::wstring> args;
        get_cmd_line_args(argc, argv, args);

        // file_descriptor_sink supports 64bit offset
        ar::basic_zip_file_sink<io_ex::file_descriptor_sink,fs::wpath>
            zip((io_ex::file_descriptor_sink(
                std::string(argv[1]), BOOST_IOS::binary)));

        for (int i = 2; i < argc; ++i)
        {
            ar::zip::wheader head;
            head.path = fs::wpath(args[i]);

            const fs_ex::file_status& s = fs_ex::symlink_status(head.path);

            if (is_symlink(s))
                head.link_path = fs_ex::symlink_target(head.path);
            else if (is_directory(s))
                head.attributes = ar::msdos::attributes::directory;
            else
                head.file_size = s.file_size();
            head.update_time = s.last_write_time().to_time_t();

            if (s.has_attributes())
                head.attributes = s.attributes();

            if (s.has_creation_time())
            {
                head.modified_time = s.last_write_time().to_time_t();
                head.access_time = s.last_access_time().to_time_t();
                head.creation_time = s.creation_time().to_time_t();
            }

            if (s.has_permissions())
                head.permissions = s.permissions();

            if (s.has_uid() && s.has_gid())
            {
                head.gid = static_cast<boost::uint16_t>(s.gid());
                head.uid = static_cast<boost::uint16_t>(s.uid());
            }

            zip.create_entry(head);

            if (!fs::is_directory(head.path))
            {
                fs::ifstream is(head.path, std::ios_base::binary);
                if (!is)
                    throw std::runtime_error("cannot open file");

                try
                {
                    io::copy(is, zip);
                }
                catch (const ar::give_up_compression&)
                {
                    is.clear();
                    is.seekg(0, std::ios_base::beg);
                    zip.rewind_entry();
                    io::copy(is, zip);
                }
            }
        }
        zip.close_archive();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
