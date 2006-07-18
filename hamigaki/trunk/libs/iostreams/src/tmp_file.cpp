//  tmp_file.cpp: temporary file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define HAMIGAKI_IOSTREAMS_SOURCE
#include <hamigaki/iostreams/device/tmp_file.hpp>
#include <boost/config.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/noncopyable.hpp>

#if defined(BOOST_WINDOWS)
    #include <cstdlib>
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

namespace hamigaki { namespace iostreams {

#if defined(BOOST_WINDOWS)
namespace
{

bool is_directory(const std::string& filename)
{
    ::DWORD attr = ::GetFileAttributesA(filename.c_str());
    return (attr != 0xFFFFFFFF) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

std::string get_temp_dir()
{
    ::DWORD size = ::GetTempPathA(0, 0);
    if (size == 0)
        return std::string(".");

    std::string buf;
    buf.resize(size);
    ::DWORD result = ::GetTempPathA(buf.size(), &buf[0]);
    if ((result == 0) || (result+1 != size))
        return std::string(".");

    buf.resize(result);
    return buf;
}

std::string make_tmp_filename(const std::string& tmpdir)
{
    char buf[MAX_PATH];
    ::DWORD n = std::rand() ^ ::GetTickCount();
    ::GetTempFileName(tmpdir.c_str(), "HAM", n, buf);
    return std::string(buf);
}

::HANDLE create_temp_file(const std::string& tmpdir)
{
    return ::CreateFile(make_tmp_filename(tmpdir).c_str(),
        GENERIC_READ|GENERIC_WRITE, 0, 0,
        CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);
}

} // namespace

class tmp_file::impl : boost::noncopyable
{
public:
    impl()
    {
        const std::string& tmpdir = get_temp_dir();
        if (!is_directory(tmpdir))
            throw BOOST_IOSTREAMS_FAILURE("temporary directory is unavailable");

        for (int i = 0; i < 10; ++i)
        {
            handle_ = create_temp_file(tmpdir);
            if (handle_ != INVALID_HANDLE_VALUE)
                break;
        }
        if (handle_ == INVALID_HANDLE_VALUE)
            throw BOOST_IOSTREAMS_FAILURE("cannot open temporary file");
    }

    ~impl()
    {
        close();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n != 0)
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
        while (n != 0)
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
        ::DWORD low = off & 0xFFFFFFFF;
        ::LONG high = off >> 32;
        low = ::SetFilePointer(handle_, low, &high,
            way == BOOST_IOS::beg ? FILE_BEGIN :
            way == BOOST_IOS::cur ? FILE_CURRENT : FILE_END);

        if ((low == INVALID_SET_FILE_POINTER) &&
            (::GetLastError() == NO_ERROR))
        {
            throw BOOST_IOSTREAMS_FAILURE("bad seek");
        }

        return boost::iostreams::offset_to_position(
            low | (static_cast<boost::iostreams::stream_offset>(high)<<32));
    }

    void close()
    {
        if (handle_ != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
        }
    }

private:
    ::HANDLE handle_;
};
#else // not defined(BOOST_WINDOWS)
class tmp_file::impl : boost::noncopyable
{
public:
    impl()
    {
        using namespace std;
        char buf[L_tmpnam];
        for (int i = 0; i < TMP_MAX; ++i)
        {
            fd_ = ::open(tmpnam(buf), O_RDWR|O_CREAT|O_EXCL, 0600);
            if (fd_ != -1)
                break;
        }
        if (fd_ == -1)
            throw BOOST_IOSTREAMS_FAILURE("cannot open temporary file");

        ::unlink(buf);
    }

    ~impl()
    {
        close();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n != 0)
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
        while (n != 0)
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
        ::off_t res = ::lseek(fd_, off,
            way == BOOST_IOS::beg ? SEEK_SET :
            way == BOOST_IOS::cur ? SEEK_CUR : SEEK_END);
        if (res == -1)
            throw BOOST_IOSTREAMS_FAILURE("bad seek");
        return boost::iostreams::offset_to_position(res);
    }

    void close()
    {
        if (fd_ != -1)
        {
            ::close(fd_);
            fd_ = -1;
        }
    }

private:
    int fd_;
};
#endif // not defined(BOOST_WINDOWS)

tmp_file::tmp_file()
    : pimpl_(new impl())
{
}

std::streamsize tmp_file::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

std::streamsize tmp_file::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

std::streampos tmp_file::seek(
    boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
{
    return pimpl_->seek(off, way);
}

void tmp_file::close()
{
    pimpl_->close();
}

} } // End namespaces iostreams, hamigaki.
