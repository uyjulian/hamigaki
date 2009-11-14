// pipe_device.cpp: pipe device

// Copyright Takeshi Mouri 2007-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#define HAMIGAKI_PROCESS_SOURCE
#define NOMINMAX
#include <boost/config.hpp>
#include <hamigaki/process/pipe_device.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/noncopyable.hpp>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace hamigaki { namespace process {

#if defined(BOOST_WINDOWS)
class pipe_source::impl : private boost::noncopyable
{
public:
    impl(::HANDLE h, bool close_on_exit)
        : handle_(h), close_on_exit_(close_on_exit)
    {
    }

    ~impl()
    {
        if (close_on_exit_ && (handle_ != INVALID_HANDLE_VALUE))
            ::CloseHandle(handle_);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        ::DWORD amt = 0;
        ::DWORD buf_size = static_cast<DWORD>(n);
        if (::ReadFile(handle_, s, buf_size, &amt, 0) == FALSE)
        {
            ::DWORD code = ::GetLastError();
            if (code == ERROR_BROKEN_PIPE)
                return -1;
            throw BOOST_IOSTREAMS_FAILURE("bad read");
        }

        return
            amt == 0
            ? static_cast<std::streamsize>(-1)
            : static_cast<std::streamsize>(amt);
    }

private:
    ::HANDLE handle_;
    bool close_on_exit_;
};

class pipe_sink::impl : private boost::noncopyable
{
public:
    impl(::HANDLE h, bool close_on_exit)
        : handle_(h), close_on_exit_(close_on_exit)
    {
    }

    ~impl()
    {
        if (close_on_exit_ && (handle_ != INVALID_HANDLE_VALUE))
            ::CloseHandle(handle_);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        ::DWORD amt = 0;
        ::DWORD buf_size = static_cast<DWORD>(n);
        if (::WriteFile(handle_, s, buf_size, &amt, 0) == FALSE)
            throw BOOST_IOSTREAMS_FAILURE("bad write");

        return amt;
    }

private:
    ::HANDLE handle_;
    bool close_on_exit_;
};
#else // not defined(BOOST_WINDOWS)
class pipe_source::impl : private boost::noncopyable
{
public:
    impl(int h, bool close_on_exit)
        : handle_(h), close_on_exit_(close_on_exit)
    {
    }

    ~impl()
    {
        if (close_on_exit_ && (handle_ != -1))
            ::close(handle_);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize amt = ::read(handle_, s, static_cast<std::size_t>(n));
        if (amt == -1)
            throw BOOST_IOSTREAMS_FAILURE("bad read");

        return
            amt == 0
            ? static_cast<std::streamsize>(-1)
            : static_cast<std::streamsize>(amt);
    }

private:
    int handle_;
    bool close_on_exit_;
};

class pipe_sink::impl : private boost::noncopyable
{
public:
    impl(int h, bool close_on_exit)
        : handle_(h), close_on_exit_(close_on_exit)
    {
    }

    ~impl()
    {
        if (close_on_exit_ && (handle_ != -1))
            ::close(handle_);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt = ::write(handle_, s, static_cast<std::size_t>(n));
        if (amt == -1)
            throw BOOST_IOSTREAMS_FAILURE("bad write");

        return amt;
    }

private:
    int handle_;
    bool close_on_exit_;
};
#endif // not defined(BOOST_WINDOWS)

pipe_source::pipe_source()
{
}

pipe_source::pipe_source(handle_type h, bool close_on_exit)
{
    try
    {
        pimpl_.reset(new impl(h, close_on_exit));
    }
    catch (...)
    {
        if (close_on_exit)
        {
#if defined(BOOST_WINDOWS)
            ::CloseHandle(h);
#else
            ::close(h);
#endif
        }
        throw;
    }
}

bool pipe_source::is_open() const
{
    return pimpl_.get() != 0;
}

std::streamsize pipe_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void pipe_source::close()
{
    pimpl_.reset();
}


pipe_sink::pipe_sink()
{
}

pipe_sink::pipe_sink(handle_type h, bool close_on_exit)
{
    try
    {
        pimpl_.reset(new impl(h, close_on_exit));
    }
    catch (...)
    {
        if (close_on_exit)
        {
#if defined(BOOST_WINDOWS)
            ::CloseHandle(h);
#else
            ::close(h);
#endif
        }
        throw;
    }
}

bool pipe_sink::is_open() const
{
    return pimpl_.get() != 0;
}

std::streamsize pipe_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

void pipe_sink::close()
{
    pimpl_.reset();
}

} } // End namespaces process, hamigaki.
