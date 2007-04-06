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
#include <boost/scoped_array.hpp>
#include <cstring>
#include <stdexcept>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#else
    #include <sys/wait.h>
    #include <fcntl.h>
    #include <unistd.h>
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
namespace
{

class file_descriptor : private boost::noncopyable
{
public:
    file_descriptor() : handle_(-1)
    {
    }

    ~file_descriptor()
    {
        if (handle_ != -1)
            ::close(handle_);
    }

    int get() const
    {
        return handle_;
    }

    void reset(int h)
    {
        if ((handle_ != -1) && (handle_ != h))
            ::close(handle_);
        handle_ = h;
    }

private:
    int handle_;
};

} // namespace

class child::impl : private boost::noncopyable
{
public:
    impl(const std::string& path,
        const std::vector<std::string>& args, const ipc_map& ipc) : ipc_(ipc)
    {
        boost::scoped_array<char*> argv(new char*[args.size()+1]);

        std::size_t argv_size = 0;
        for (std::size_t i = 0; i < args.size(); ++i)
            argv_size += (args[i].size() + 1);

        boost::scoped_array<char> argv_buf(new char[argv_size]);
        char* p = argv_buf.get();
        for (std::size_t i = 0; i < args.size(); ++i)
        {
            const std::string& arg = args[i];
            arg.copy(p, arg.size());
            p[arg.size()] = '\0';
            argv[i] = p;
            p += static_cast<std::ptrdiff_t>(arg.size() + 1);
        }
        argv[args.size()] = 0;

        file_descriptor peer_stdin;
        file_descriptor peer_stdout;
        file_descriptor peer_stderr;

        stream_behavior::type t = ipc_.stdin_behavior().get_type();
        if (t == stream_behavior::capture)
        {
            int fds[2];
            if (::pipe(fds) == -1)
                throw std::runtime_error("pipe() failed");

            peer_stdin.reset(fds[0]);
            stdin_ = pipe_sink(fds[1], true);
        }
        else if (t == stream_behavior::silence)
        {
            int fd = ::open("/dev/null", O_RDONLY);
            if (fd == -1)
                throw std::runtime_error("open() failed");
            peer_stdin.reset(fd);
        }

        t = ipc_.stdout_behavior().get_type();
        if (t == stream_behavior::capture)
        {
            int fds[2];
            if (::pipe(fds) == -1)
                throw std::runtime_error("pipe() failed");

            peer_stdout.reset(fds[1]);
            stdout_ = pipe_source(fds[0], true);
        }
        else if (t == stream_behavior::silence)
        {
            int fd = ::open("/dev/null", O_WRONLY);
            if (fd == -1)
                throw std::runtime_error("open() failed");
            peer_stdout.reset(fd);
        }

        t = ipc_.stderr_behavior().get_type();
        if (t == stream_behavior::capture)
        {
            int fds[2];
            if (::pipe(fds) == -1)
                throw std::runtime_error("pipe() failed");

            peer_stderr.reset(fds[1]);
            stderr_ = pipe_source(fds[0], true);
        }
        else if (t == stream_behavior::silence)
        {
            int fd = ::open("/dev/null", O_WRONLY);
            if (fd == -1)
                throw std::runtime_error("open() failed");
            peer_stderr.reset(fd);
        }

        const char* ph = path.c_str();
        char* const* a = (char* const*)argv.get();
        char* const* e = (char* const*)environ; // TODO

        int max_fd = static_cast<int>(::sysconf(_SC_OPEN_MAX));
        if (max_fd == -1)
            max_fd = 255;

        handle_ = ::fork();
        if (handle_ == 0)
        {
            ::dup2(peer_stdin.get(), 0);
            ::dup2(peer_stdout.get(), 1);
            ::dup2(peer_stderr.get(), 2);

            for (int i = max_fd; i > 2; --i)
                ::close(i);

            if (::execve(ph, a, e) == -1)
                ::_exit(127);
        }
        else if (handle_ == static_cast< ::pid_t>(-1))
            throw std::runtime_error("fork() failed");
    }

    ~impl()
    {
        if (handle_ != static_cast< ::pid_t>(-1))
        {
            ::kill(handle_, SIGKILL);
            ::waitpid(handle_, 0, 0);
        }
    }

    status wait()
    {
        BOOST_ASSERT(handle_ != static_cast< ::pid_t>(-1));

        int st;
        if (::waitpid(handle_, &st, 0) == -1)
            throw std::runtime_error("waitpid() failed");

        handle_ = static_cast< ::pid_t>(-1);

        if (WIFEXITED(st))
            return status(static_cast<unsigned>(WEXITSTATUS(st)));
        else if (WIFSIGNALED(st))
        {
#if defined(WCOREDUMP)
            return status(
                status::signaled,
                static_cast<unsigned>(WTERMSIG(st)),
                WCOREDUMP(st) != 0
            );
#else
            return status(
                status::signaled,
                static_cast<unsigned>(WTERMSIG(st))
            );
#endif
        }
        else if (WIFSTOPPED(st))
            return status(status::stopped, static_cast<unsigned>(WSTOPSIG(st)));
#if defined(WIFCONTINUED)
        else if (WIFCONTINUED(st))
            return status(status::continued, 0u);
#endif
        BOOST_ASSERT(!"unknown exit status");
        return status();
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
    ::pid_t handle_;
    ipc_map ipc_;
    pipe_sink stdin_;
    pipe_source stdout_;
    pipe_source stderr_;
};
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
