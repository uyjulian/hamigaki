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
    #include <hamigaki/detail/windows/dynamic_link_library.hpp>
    #include <windows.h>

    #if !defined(EXTENDED_STARTUPINFO_PRESENT)

        #define EXTENDED_STARTUPINFO_PRESENT        0x00080000
        #define PROC_THREAD_ATTRIBUTE_HANDLE_LIST   0x00020002

        struct _PROC_THREAD_ATTRIBUTE_LIST;

        typedef struct _STARTUPINFOEXA
        {
            STARTUPINFOA StartupInfo;
            _PROC_THREAD_ATTRIBUTE_LIST* lpAttributeList;
        } STARTUPINFOEXA;
    #endif
#else
    #include <sys/wait.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

namespace hamigaki { namespace process {

#if defined(BOOST_WINDOWS)
namespace
{

using detail::windows::dynamic_link_library;

class attr_list : private boost::noncopyable
{
private:
    typedef ::BOOL (__stdcall *InitializeProcThreadAttributeListPtr)(
        ::_PROC_THREAD_ATTRIBUTE_LIST* lpAttributeList,
        ::DWORD dwAttributeCount,
        ::DWORD dwFlags,
        ::SIZE_T* lpSize
    );

    typedef void (__stdcall *DeleteProcThreadAttributeListPtr)(
        ::_PROC_THREAD_ATTRIBUTE_LIST* lpAttributeList
    );

    typedef ::BOOL (__stdcall *UpdateProcThreadAttributePtr)(
        ::_PROC_THREAD_ATTRIBUTE_LIST* lpAttributeList,
        ::DWORD dwFlags,
        ::DWORD_PTR Attribute,
        void* lpValue,
        ::SIZE_T cbSize,
        void* lpPreviousValue,
        ::SIZE_T* lpReturnSize
    );

public:
    explicit attr_list(unsigned long size) : dll_("kernel32.dll"), ptr_(0)
    {
        init_func_ =
            reinterpret_cast<InitializeProcThreadAttributeListPtr>(
                dll_.get_proc_address(
                    "InitializeProcThreadAttributeList", std::nothrow));
        if (init_func_ == 0)
            return;

        del_func_ =
            reinterpret_cast<DeleteProcThreadAttributeListPtr>(
                dll_.get_proc_address(
                    "DeleteProcThreadAttributeList", std::nothrow));
        if (del_func_ == 0)
            return;

        update_func_ =
            reinterpret_cast<UpdateProcThreadAttributePtr>(
                dll_.get_proc_address(
                    "UpdateProcThreadAttribute", std::nothrow));
        if (update_func_ == 0)
            return;

        ::SIZE_T buf_size = 0;
        (*init_func_)(0, size, 0, &buf_size);

        BOOST_ASSERT(::GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        buffer_.reset(new char[buf_size]);
        ptr_ = reinterpret_cast< ::_PROC_THREAD_ATTRIBUTE_LIST*>(buffer_.get());

        if ((*init_func_)(ptr_, size, 0, &buf_size) == FALSE)
        {
            throw std::runtime_error(
                "InitializeProcThreadAttributeList() failed");
        }
    }

    ~attr_list()
    {
        if (del_func_ != 0)
            (*del_func_)(ptr_);
    }

    ::_PROC_THREAD_ATTRIBUTE_LIST* get() const
    {
        return ptr_;
    }

    void set_handl_list(::HANDLE* handles, std::size_t size)
    {
        if ((*update_func_)(
            ptr_, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
            handles, size*sizeof(::HANDLE), 0, 0) == FALSE)
        {
            throw std::runtime_error(
                "UpdateProcThreadAttribute() failed");
        }
    }

private:
    dynamic_link_library dll_;
    InitializeProcThreadAttributeListPtr init_func_;
    DeleteProcThreadAttributeListPtr del_func_;
    UpdateProcThreadAttributePtr update_func_;
    boost::scoped_array<char> buffer_;
    ::_PROC_THREAD_ATTRIBUTE_LIST* ptr_;
};

class startup_info
{
public:
    explicit startup_info(::_PROC_THREAD_ATTRIBUTE_LIST* attr)
    {
        if (attr)
        {
            buffer_.reset(new char[sizeof(::STARTUPINFOEXA)]);
            std::memset(buffer_.get(), 0, sizeof(::STARTUPINFOEXA));
            ::STARTUPINFOEXA* ex_ptr =
                reinterpret_cast< ::STARTUPINFOEXA*>(buffer_.get());
            ex_ptr->lpAttributeList = attr;
            ptr_ = &ex_ptr->StartupInfo;
            ptr_->cb = sizeof(::STARTUPINFOA);
        }
        else
        {
            buffer_.reset(new char[sizeof(::STARTUPINFOA)]);
            std::memset(buffer_.get(), 0, sizeof(::STARTUPINFOA));
            ptr_ = reinterpret_cast< ::STARTUPINFOA*>(buffer_.get());
            ptr_->cb = sizeof(::STARTUPINFOA);
        }
    }

    ::STARTUPINFOA* get() const
    {
        return ptr_;
    }

    ::STARTUPINFOA* operator->() const
    {
        return ptr_;
    }

private:
    boost::scoped_array<char> buffer_;
    ::STARTUPINFOA* ptr_;
};

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

bool is_console(::HANDLE h)
{
    ::DWORD mode;
    // Is this OK?
    return ::GetConsoleMode(h, &mode) != FALSE;
}

// Note:
// The process attributes (LPPROC_THREAD_ATTRIBUTE_LIST)
// never contain any console input buffer and any console screen buffer.
bool need_inheritance(::HANDLE h)
{
    if (h == INVALID_HANDLE_VALUE)
        return false;
    else
        return !is_console(h);
}

} // namespace

class child::impl : private boost::noncopyable
{
public:
    impl(const std::string& path,
        const std::vector<std::string>& args, const context& ctx) : ctx_(ctx)
    {
        std::vector<char> cmd;
        hamigaki::process::make_command_line(cmd, args);

        // This variable must be defined before "attr"
        ::HANDLE handles[3];
        std::size_t handle_count = 0;

        attr_list attr(3u);

        startup_info start_info(attr.get());
        start_info->dwFlags = STARTF_USESTDHANDLES;
        start_info->hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
        start_info->hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
        start_info->hStdError = ::GetStdHandle(STD_ERROR_HANDLE);

        file_descriptor peer_stdin;
        file_descriptor peer_stdout;
        file_descriptor peer_stderr;

        stream_behavior::type t = ctx_.stdin_behavior().get_type();
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
            start_info->hStdInput = read_end;
        }
        else if (t == stream_behavior::close)
            start_info->hStdInput = INVALID_HANDLE_VALUE;
        else if (t == stream_behavior::silence)
        {
            ::HANDLE h = ::CreateFile(
                "NUL", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
            peer_stdin.reset(h);

            ::SetHandleInformation(
                h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info->hStdInput = h;
        }

        t = ctx_.stdout_behavior().get_type();
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
            start_info->hStdOutput = write_end;
        }
        else if (t == stream_behavior::close)
            start_info->hStdOutput = INVALID_HANDLE_VALUE;
        else if (t == stream_behavior::silence)
        {
            ::HANDLE h = ::CreateFile(
                "NUL", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
            peer_stdout.reset(h);

            ::SetHandleInformation(
                h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info->hStdOutput = h;
        }

        t = ctx_.stderr_behavior().get_type();
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
            start_info->hStdError = write_end;
        }
        else if (t == stream_behavior::close)
            start_info->hStdError = INVALID_HANDLE_VALUE;
        else if (t == stream_behavior::silence)
        {
            ::HANDLE h = ::CreateFile(
                "NUL", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
            peer_stderr.reset(h);

            ::SetHandleInformation(
                h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            start_info->hStdError = h;
        }

        if (need_inheritance(start_info->hStdInput))
            handles[handle_count++] = start_info->hStdInput;
        if (need_inheritance(start_info->hStdOutput))
            handles[handle_count++] = start_info->hStdOutput;
        if (need_inheritance(start_info->hStdError))
            handles[handle_count++] = start_info->hStdError;

        if (attr.get() != 0)
        {
            if (handle_count != 0)
                attr.set_handl_list(handles, handle_count);
        }

        ::DWORD flags = CREATE_NO_WINDOW;
        if (attr.get())
            flags |= EXTENDED_STARTUPINFO_PRESENT;

        const std::string dir_buf = ctx_.work_directory();
        const char* work_dir =
            dir_buf.empty() ? static_cast<const char*>(0) : dir_buf.c_str();

        ::PROCESS_INFORMATION proc_info;
        if (::CreateProcessA(
            path.c_str(), &cmd[0], 0, 0, TRUE, flags,
            0, work_dir, start_info.get(), &proc_info) == FALSE)
        {
            throw std::runtime_error("CreateProcessA() failed");
        }

        ::CloseHandle(proc_info.hThread);
        handle_ = proc_info.hProcess;
    }

    ~impl()
    {
        terminate();

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

    void terminate()
    {
        ::DWORD code;
        ::GetExitCodeProcess(handle_, &code);
        if (code == STILL_ACTIVE)
        {
            ::TerminateProcess(handle_, 0);
            ::WaitForSingleObject(handle_, INFINITE);
        }
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
    context ctx_;
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
        const std::vector<std::string>& args, const context& ctx) : ctx_(ctx)
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

        stream_behavior::type t = ctx_.stdin_behavior().get_type();
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

        t = ctx_.stdout_behavior().get_type();
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

        t = ctx_.stderr_behavior().get_type();
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

        const std::string dir_buf = ctx_.work_directory();
        const char* work_dir =
            dir_buf.empty() ? static_cast<const char*>(0) : dir_buf.c_str();

        int open_max = static_cast<int>(::sysconf(_SC_OPEN_MAX));
        if (open_max == -1)
            open_max = 256;

        handle_ = ::fork();
        if (handle_ == 0)
        {
            for (int i = 0; i < open_max; ++i)
            {
                if ((i != peer_stdin .get()) &&
                    (i != peer_stdout.get()) &&
                    (i != peer_stderr.get()) )
                {
                    ::close(i);
                }
            }

            int fds[3];
            fds[0] = ::fcntl(peer_stdin .get(), F_DUPFD, 3);
            fds[1] = ::fcntl(peer_stdout.get(), F_DUPFD, 4);
            fds[2] = ::fcntl(peer_stderr.get(), F_DUPFD, 5);

            peer_stdin .reset(-1);
            peer_stdout.reset(-1);
            peer_stderr.reset(-1);

            for (int i = 0; i < 3; ++i)
            {
                ::dup2(fds[i], i);
                ::close(fds[i]);
            }

#if !defined(NDEBUG)
            int next_fd = ::dup(0);
            ::close(next_fd);

            // Note: assert() isn't async-signal-safe
            if (next_fd != 3)
                ::_exit(next_fd);
#endif

            if ((work_dir != 0) && (::chdir(work_dir) == -1))
                ::_exit(127);

            if (::execve(ph, a, e) == -1)
                ::_exit(127);
        }
        else if (handle_ == static_cast< ::pid_t>(-1))
            throw std::runtime_error("fork() failed");
    }

    ~impl()
    {
        terminate();
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

    void terminate()
    {
        if (handle_ != static_cast< ::pid_t>(-1))
        {
            ::kill(handle_, SIGKILL);
            wait();
        }
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
    context ctx_;
    pipe_sink stdin_;
    pipe_source stdout_;
    pipe_source stderr_;
};
#endif // not defined(BOOST_WINDOWS)

child::child(
    const std::string& path, const std::vector<std::string>& args,
    const context& ctx
)
    : pimpl_(new impl(path, args, ctx))
{
}

child::child(const std::string& path, const context& ctx)
{
    std::vector<std::string> args;
    args.push_back(path);

    pimpl_.reset(new impl(path, args, ctx));
}

status child::wait()
{
    return pimpl_->wait();
}

void child::terminate()
{
    return pimpl_->terminate();
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
