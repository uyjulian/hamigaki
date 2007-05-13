// file_descriptor.cpp: low-level file device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#define HAMIGAKI_IOSTREAMS_SOURCE
#define NOMINMAX
#define _LARGEFILE64_SOURCE
#include <hamigaki/iostreams/device/file_descriptor.hpp>
#include <boost/noncopyable.hpp>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

#if defined(__USE_LARGEFILE64)
    #define HAMIGAKI_RTL(x) ::x##64
#else
    #define HAMIGAKI_RTL(x) ::x
#endif

namespace hamigaki { namespace iostreams {

namespace detail
{

#if defined(BOOST_WINDOWS)

#ifdef BOOST_MSVC
    #pragma warning(disable : 4275)
#endif

namespace
{

inline unsigned long make_access_flags(BOOST_IOS::openmode m)
{
    unsigned long flags = 0;
    if ((m & BOOST_IOS::in) != 0)
        flags |= GENERIC_READ;
    if ((m & BOOST_IOS::out) != 0)
        flags |= GENERIC_WRITE;
    return flags;
}

inline unsigned long make_create_flags(BOOST_IOS::openmode m)
{
    if ((m & BOOST_IOS::app) != 0)
        return OPEN_ALWAYS;
    else if (((m & BOOST_IOS::in) != 0) && ((m & BOOST_IOS::trunc) == 0))
        return OPEN_EXISTING;
    else
        return CREATE_ALWAYS;
}

} // namespace

class file_descriptor_impl : private boost::noncopyable
{
public:
    file_descriptor_impl(
        const std::string& filename, BOOST_IOS::openmode mode)
    {
        handle_ = ::CreateFileA(filename.c_str(),
            make_access_flags(mode), FILE_SHARE_READ|FILE_SHARE_WRITE, 0,
            make_create_flags(mode), FILE_ATTRIBUTE_NORMAL, 0);

        if (handle_ == INVALID_HANDLE_VALUE)
            throw BOOST_IOSTREAMS_FAILURE("bad open");

        try
        {
            if ((mode & BOOST_IOS::app) != 0)
                this->seek(0, BOOST_IOS::end);
        }
        catch (...)
        {
            ::CloseHandle(handle_);
            throw;
        }
    }

    ~file_descriptor_impl()
    {
        ::CloseHandle(handle_);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n > 0)
        {
            ::DWORD buf_size = n;
            ::DWORD amt;
            if (::ReadFile(handle_, s, buf_size, &amt, 0) == FALSE)
                throw BOOST_IOSTREAMS_FAILURE("bad read");
            if (amt == 0)
                break;
            total += amt;
            s += amt;
            n -= amt;
        }
        return total == 0 ? -1 : total;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n > 0)
        {
            ::DWORD buf_size = n;
            ::DWORD amt;
            if (::WriteFile(handle_, s, buf_size, &amt, 0) == FALSE)
                throw BOOST_IOSTREAMS_FAILURE("bad write");
            total += amt;
            s += amt;
            n -= amt;
        }
        return total;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        ::DWORD low = static_cast< ::DWORD>(off & 0xFFFFFFFF);
        ::LONG high = static_cast< ::DWORD>(off >> 32);
        low = ::SetFilePointer(handle_, low, &high,
            way == BOOST_IOS::beg ? FILE_BEGIN :
            way == BOOST_IOS::cur ? FILE_CURRENT : FILE_END);

        if ((low == INVALID_SET_FILE_POINTER) &&
            (::GetLastError() != NO_ERROR))
        {
            throw BOOST_IOSTREAMS_FAILURE("bad seek");
        }

        return boost::iostreams::offset_to_position(
            static_cast<boost::iostreams::stream_offset>(
                low |
                (static_cast<boost::uint64_t>(static_cast< ::DWORD>(high))<<32)
            )
        );
    }

private:
    ::HANDLE handle_;
};
#else // not defined(BOOST_WINDOWS)
namespace
{

inline int make_open_flags(BOOST_IOS::openmode m)
{
    if ((m & BOOST_IOS::in) == 0)
    {
        if ((m & BOOST_IOS::app) == 0)
            return O_WRONLY|O_CREAT|O_TRUNC;
        else
            return O_WRONLY|O_APPEND;
    }
    else
    {
        if ((m & BOOST_IOS::out) == 0)
            return O_RDONLY;
        else if ((m & BOOST_IOS::trunc) == 0)
            return O_RDWR;
        else
            return O_RDWR|O_CREAT|O_TRUNC;
    }
}

} // namespace

class file_descriptor_impl : private boost::noncopyable
{
public:
    file_descriptor_impl(const std::string& filename, BOOST_IOS::openmode mode)
    {
        fd_ = HAMIGAKI_RTL(open)(filename.c_str(), make_open_flags(mode), 0666);
        if (fd_ == -1)
            throw BOOST_IOSTREAMS_FAILURE("bad open");
    }

    ~file_descriptor_impl()
    {
        ::close(fd_);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n > 0)
        {
            int amt = ::read(fd_, s, n);
            if (amt == -1)
                throw BOOST_IOSTREAMS_FAILURE("bad read");
            else if (amt == 0)
                break;
            total += amt;
            s += amt;
            n -= amt;
        }
        return total == 0 ? -1 : total;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n > 0)
        {
            int amt = ::write(fd_, s, n);
            if (amt == -1)
                throw BOOST_IOSTREAMS_FAILURE("bad write");
            total += amt;
            s += amt;
            n -= amt;
        }
        return total;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        boost::iostreams::stream_offset res =
            HAMIGAKI_RTL(lseek)(fd_, off,
                way == BOOST_IOS::beg ? SEEK_SET :
                way == BOOST_IOS::cur ? SEEK_CUR : SEEK_END);
        if (res == -1)
            throw BOOST_IOSTREAMS_FAILURE("bad seek");
        return boost::iostreams::offset_to_position(res);
    }

private:
    int fd_;
};
#endif // not defined(BOOST_WINDOWS)

} // namespace detail

void file_descriptor_source::open(
    const std::string& filename, BOOST_IOS::openmode mode)
{
    pimpl_.reset(new impl_type(filename, mode|BOOST_IOS::in));
}

std::streamsize file_descriptor_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

std::streampos file_descriptor_source::seek(
    boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
{
    return pimpl_->seek(off, way);
}


void file_descriptor_sink::open(
    const std::string& filename, BOOST_IOS::openmode mode)
{
    pimpl_.reset(new impl_type(filename, mode|BOOST_IOS::out));
}

std::streamsize file_descriptor_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

std::streampos file_descriptor_sink::seek(
    boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
{
    return pimpl_->seek(off, way);
}


void file_descriptor::open(
    const std::string& filename, BOOST_IOS::openmode mode)
{
    pimpl_.reset(new impl_type(filename, mode|BOOST_IOS::in|BOOST_IOS::out));
}

std::streamsize file_descriptor::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

std::streamsize file_descriptor::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

std::streampos file_descriptor::seek(
    boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
{
    return pimpl_->seek(off, way);
}

} } // End namespaces iostreams, hamigaki.
