// shortcut.cpp: create shortcut

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/noncopyable.hpp>
#include <iostream>
#include <stdexcept>

#include <windows.h>
#include <objbase.h>

namespace fs_ex = hamigaki::filesystem;
namespace fs = boost::filesystem;

class com_library : boost::noncopyable
{
public:
    com_library()
    {
        ::HRESULT res = ::CoInitialize(0);
        if (FAILED(res))
            throw std::runtime_error("failed CoInitialize()");
    }

    ~com_library()
    {
        ::CoUninitialize();
    }
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: shortcut (target) (path)" << std::endl;
            return 1;
        }

        com_library using_com;

        fs::path target(argv[1]);
        fs::path ph(argv[2]);
        if (extension(ph) != ".lnk")
            ph = ph.branch_path() / (ph.leaf() + ".lnk");
        fs_ex::create_shell_link(target, ph);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
