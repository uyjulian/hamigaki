// file.hpp: a thin wrapper for POSIX file

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_POSIX_FILE_HPP
#define HAMIGAKI_DETAIL_POSIX_FILE_HPP

#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace hamigaki { namespace detail { namespace posix {

class file : boost::noncopyable
{
public:
    file(const char* path, int oflag)
    {
        if ((oflag & O_CREAT) != 0)
            throw std::runtime_error("bad open flags");

        fd_ = ::open(path, oflag);
        if (fd_ == -1)
        {
            std::string msg("cannot open ");
            msg += path;
            throw std::runtime_error(msg);
        }
    }

    file(const char* path, int oflag, ::mode_t mode)
    {
        if ((oflag & O_CREAT) == 0)
            throw std::runtime_error("bad open flags");

        fd_ = ::open(path, oflag, mode);
        if (fd_ == -1)
        {
            std::string msg("cannot open ");
            msg += path;
            throw std::runtime_error(msg);
        }
    }

    ~file()
    {
        try
        {
            if (fd_ != -1)
                close();
        }
        catch (...)
        {
        }
    }

    int get()
    {
        return fd_;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt = ::write(fd_, s, n);
        if (amt < 0)
            throw std::runtime_error("write error");
        return amt;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize amt = ::read(fd_, s, n);
        if (amt < 0)
            throw std::runtime_error("read error");
        return amt;
    }

    void close()
    {
        if (fd_ == -1)
            throw std::runtime_error("already closed");

        ::close(fd_);
        fd_ = -1;
    }

private:
    int fd_;
};

} } } // End namespaces posix, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_POSIX_FILE_HPP
