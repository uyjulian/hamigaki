// launch_shell.cpp: launch a process on command shell

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#define HAMIGAKI_PROCESS_SOURCE
#define NOMINMAX
#include <boost/config.hpp>
#include <hamigaki/process/shell.hpp>
#include <stdexcept>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#endif

namespace hamigaki { namespace process {

#if defined(BOOST_WINDOWS)
namespace
{

std::string system_directory()
{
    char buf[MAX_PATH];
    ::DWORD res = ::GetSystemDirectoryA(buf, sizeof(buf));
    if (res == 0)
        throw std::runtime_error("GetSystemDirectoryA() failed");
    return std::string(buf, static_cast<std::size_t>(res));
}

} // namespace
#endif

HAMIGAKI_PROCESS_DECL
child launch_shell(const std::string& cmd, const context& ctx)
{
#if defined(BOOST_WINDOWS)
    std::string sh = system_directory();
    if (sh[sh.size()-1] != '\\')
        sh += '\\';
    sh += "cmd.exe";

    std::string cmd_line = "cmd /c ";
    cmd_line += cmd;

    return process::child(sh, cmd_line, ctx);
#else // not defined(BOOST_WINDOWS)
    std::string sh("/bin/sh");

    std::vector<std::string> args;
    args.push_back("sh");
    args.push_back("-c");
    args.push_back(cmd);

    return process::child(sh, args, ctx);
#endif // not defined(BOOST_WINDOWS)
}

} } // End namespaces process, hamigaki.
