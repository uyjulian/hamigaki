//  child.cpp: child process

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/process for library home page.

#define HAMIGAKI_PROCESS_SOURCE
#define NOMINMAX
#include <boost/config.hpp>
#include <hamigaki/process/child.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <cstring>
#include <stdexcept>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#else
#endif

namespace hamigaki { namespace process {

#if defined(BOOST_WINDOWS)
namespace
{

void make_command_line(
    std::vector<char>& cmd, const std::vector<std::string>& args)
{
    for (std::size_t i = 0; i < args.size(); ++i)
    {
        std::string arg = args[i];
        boost::algorithm::replace_all(arg, "\"", "\\\"");
        if (arg.find(' ') != std::string::npos)
        {
            arg.insert(arg.begin(), '"');
            arg.push_back('"');
        }
        cmd.insert(cmd.end(), arg.begin(), arg.end());
        cmd.push_back(' ');
    }

    if (cmd.empty())
        cmd.push_back('\0');
    else
        cmd.back() = '\0';

    cmd.push_back('\0');
}

class file_descriptor : private boost::noncopyable
{
public:
    file_descriptor() : handle_(INVALID_HANDLE_VALUE)
    {
    }

    ~file_descriptor()
    {
        if (handle_ != INVALID_HANDLE_VALUE)
            ::CloseHandle(handle_);
    }

    void reset(::HANDLE h)
    {
        if ((handle_ != INVALID_HANDLE_VALUE) && (handle_ != h))
            ::CloseHandle(handle_);
        handle_ = h;
    }

private:
    ::HANDLE handle_;
};

} // namespace

class child::impl : private boost::noncopyable
{
public:
    impl(const std::string& path,
        const std::vector<std::string>& args, const ipc_map& ipc) : ipc_(ipc)
    {
        std::vector<char> cmd;
        hamigaki::process::make_command_line(cmd, args);

        ::STARTUPINFOA start_info;
        std::memset(&start_info, 0, sizeof(start_info));
        start_info.cb = sizeof(start_info);
        start_info.dwFlags = STARTF_USESTDHANDLES;
        start_info.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
        start_info.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
        start_info.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);

        file_descriptor peer_stdin;
        file_descriptor peer_stdout;
        file_descriptor peer_stderr;

        stream_behavior::type t = ipc_.stdin_behavior().get_type();
        if (t == stream_behavior::capture)
        {
            ::HANDLE read_end;
            ::HANDLE write_end;
            if (::CreatePipe(&read_end, &write_end, 0, 0) == FALSE)
                throw std::runtime_error("CreatePipe() failed");

            peer_stdin.reset(read_end);
            stdin_ = pipe_sink(write_end, true);

            ::SetHandleInformation(
                read_end, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info.hStdInput = read_end;
        }
        else if (t == stream_behavior::close)
            start_info.hStdInput = INVALID_HANDLE_VALUE;
        else if (t == stream_behavior::silence)
        {
            ::HANDLE h = ::CreateFile(
                "NUL", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
            peer_stdin.reset(h);

            ::SetHandleInformation(
                h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info.hStdInput = h;
        }

        t = ipc_.stdout_behavior().get_type();
        if (t == stream_behavior::capture)
        {
            ::HANDLE read_end;
            ::HANDLE write_end;
            if (::CreatePipe(&read_end, &write_end, 0, 0) == FALSE)
                throw std::runtime_error("CreatePipe() failed");

            peer_stdout.reset(write_end);
            stdout_ = pipe_source(read_end, true);

            ::SetHandleInformation(
                write_end, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info.hStdOutput = write_end;
        }
        else if (t == stream_behavior::close)
            start_info.hStdOutput = INVALID_HANDLE_VALUE;
        else if (t == stream_behavior::silence)
        {
            ::HANDLE h = ::CreateFile(
                "NUL", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
            peer_stdout.reset(h);

            ::SetHandleInformation(
                h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info.hStdOutput = h;
        }

        t = ipc_.stderr_behavior().get_type();
        if (t == stream_behavior::capture)
        {
            ::HANDLE read_end;
            ::HANDLE write_end;
            if (::CreatePipe(&read_end, &write_end, 0, 0) == FALSE)
                throw std::runtime_error("CreatePipe() failed");

            peer_stderr.reset(write_end);
            stderr_ = pipe_source(read_end, true);

            ::SetHandleInformation(
                write_end, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info.hStdError = write_end;
        }
        else if (t == stream_behavior::close)
            start_info.hStdError = INVALID_HANDLE_VALUE;
        else if (t == stream_behavior::silence)
        {
            ::HANDLE h = ::CreateFile(
                "NUL", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
            peer_stderr.reset(h);

            ::SetHandleInformation(
                h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info.hStdError = h;
        }

        ::PROCESS_INFORMATION proc_info;
        if (::CreateProcessA(
            path.c_str(), &cmd[0], 0, 0,
            TRUE, 0, 0, 0, &start_info, &proc_info) == FALSE)
        {
            throw std::runtime_error("CreateProcessA() failed");
        }

        ::CloseHandle(proc_info.hThread);
        handle_ = proc_info.hProcess;
    }

    ~impl()
    {
        ::DWORD code;
        ::GetExitCodeProcess(handle_, &code);
        if (code == STILL_ACTIVE)
        {
            ::TerminateProcess(handle_, 0);
            ::WaitForSingleObject(handle_, INFINITE);
        }

        ::CloseHandle(handle_);
    }

    status wait()
    {
        ::WaitForSingleObject(handle_, INFINITE);

        ::DWORD code;
        if (::GetExitCodeProcess(handle_, &code) == FALSE)
            throw std::runtime_error("GetExitCodeProcess() failed");

        BOOST_ASSERT(code != STILL_ACTIVE);

        return status(static_cast<unsigned>(code));
    }

    pipe_sink& stdin_sink()
    {
        return stdin_;
    }

    pipe_source& stdout_source()
    {
        return stdout_;
    }

    pipe_source& stderr_source()
    {
        return stderr_;
    }

private:
    ::HANDLE handle_;
    ipc_map ipc_;
    pipe_sink stdin_;
    pipe_source stdout_;
    pipe_source stderr_;
};
#else // not defined(BOOST_WINDOWS)
#endif // not defined(BOOST_WINDOWS)

child::child(
    const std::string& path, const std::vector<std::string>& args,
    const ipc_map& ipc
)
    : pimpl_(new impl(path, args, ipc))
{
}

child::child(const std::string& path, const ipc_map& ipc)
{
    std::vector<std::string> args;
    args.push_back(path);

    pimpl_.reset(new impl(path, args, ipc));
}

status child::wait()
{
    return pimpl_->wait();
}

pipe_sink& child::stdin_sink()
{
    return pimpl_->stdin_sink();
}

pipe_source& child::stdout_source()
{
    return pimpl_->stdout_source();
}

pipe_source& child::stderr_source()
{
    return pimpl_->stderr_source();
}

} } // End namespaces process, hamigaki.
