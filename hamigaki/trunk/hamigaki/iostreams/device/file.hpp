// file.hpp: file device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_FILE_HPP

#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>

namespace hamigaki { namespace iostreams {

namespace detail
{

class file_impl
{
public:
    file_impl(const std::string& filename, BOOST_IOS::openmode mode)
    {
        char m[4];
        if (mode & BOOST_IOS::in)
        {
            if (mode & BOOST_IOS::trunc)
                std::strcpy(m, "w+");
            else if (mode & BOOST_IOS::out)
                std::strcpy(m, "r+");
            else
                std::strcpy(m, "r");
        }
        else
        {
            if (mode & BOOST_IOS::app)
                std::strcpy(m, "a");
            else
                std::strcpy(m, "w");
        }
        if (mode & BOOST_IOS::binary)
            std::strcat(m, "b");

        fp_ = std::fopen(filename.c_str(), m);
        if (!fp_)
            throw BOOST_IOSTREAMS_FAILURE("bad open");
    }

    ~file_impl()
    {
        std::fclose(fp_);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::size_t amt = std::fread(s, 1, n, fp_);
        if (amt == 0)
        {
            using namespace std;
            if (ferror(fp_))
                throw BOOST_IOSTREAMS_FAILURE("bad read");
            else
                return -1;
        }
        return amt;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::size_t amt = std::fwrite(s, 1, n, fp_);
        if (amt != static_cast<std::size_t>(n))
            throw BOOST_IOSTREAMS_FAILURE("bad write");
        return amt;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        typedef boost::iostreams::stream_offset off_t;
        if ((off > static_cast<off_t>((std::numeric_limits<long>::max)())) ||
            (off < static_cast<off_t>((std::numeric_limits<long>::min)())))
        {
            throw BOOST_IOSTREAMS_FAILURE("bad seek offset");
        }

        if (std::fseek(fp_, static_cast<long>(off),
            way == BOOST_IOS::beg ? SEEK_SET :
            way == BOOST_IOS::cur ? SEEK_CUR : SEEK_END) != 0)
        {
            throw BOOST_IOSTREAMS_FAILURE("bad seek");
        }
        return boost::iostreams::offset_to_position(std::ftell(fp_));
    }

private:
    std::FILE* fp_;
};

} // namespace detail

class file_source
{
private:
    typedef detail::file_impl impl_type;

public:
    typedef char char_type;

    struct category
        : public boost::iostreams::input_seekable
        , public boost::iostreams::device_tag
        , public boost::iostreams::closable_tag
    {};

    file_source()
    {
    }

    explicit file_source(
        const std::string& filename, BOOST_IOS::openmode mode=BOOST_IOS::in)
    {
        this->open(filename, mode);
    }

    void open(
        const std::string& filename, BOOST_IOS::openmode mode=BOOST_IOS::in)
    {
        pimpl_.reset(new impl_type(filename, mode|BOOST_IOS::in));
    }

    bool is_open() const
    {
        return pimpl_.get() != 0;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return pimpl_->seek(off, way);
    }

    void close()
    {
        pimpl_.reset();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class file_sink
{
private:
    typedef detail::file_impl impl_type;

public:
    typedef char char_type;

    struct category
        : public boost::iostreams::output_seekable
        , public boost::iostreams::device_tag
        , public boost::iostreams::closable_tag
    {};

    file_sink()
    {
    }

    explicit file_sink(
        const std::string& filename, BOOST_IOS::openmode mode=BOOST_IOS::out)
    {
        this->open(filename, mode);
    }

    void open(
        const std::string& filename, BOOST_IOS::openmode mode=BOOST_IOS::out)
    {
        pimpl_.reset(new impl_type(filename, mode|BOOST_IOS::out));
    }

    bool is_open() const
    {
        return pimpl_.get() != 0;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return pimpl_->seek(off, way);
    }

    void close()
    {
        pimpl_.reset();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class file
{
private:
    typedef detail::file_impl impl_type;

public:
    typedef char char_type;

    struct category
        : public boost::iostreams::seekable
        , public boost::iostreams::device_tag
        , public boost::iostreams::closable_tag
    {};

    file()
    {
    }

    explicit file(
        const std::string& filename,
        BOOST_IOS::openmode mode=BOOST_IOS::in|BOOST_IOS::out)
    {
        this->open(filename, mode);
    }

    void open(
        const std::string& filename,
        BOOST_IOS::openmode mode=BOOST_IOS::in|BOOST_IOS::out)
    {
        pimpl_.reset(
            new impl_type(filename, mode|BOOST_IOS::in|BOOST_IOS::out));
    }

    bool is_open() const
    {
        return pimpl_.get() != 0;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return pimpl_->seek(off, way);
    }

    void close()
    {
        pimpl_.reset();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::file_source, 0)

#endif // HAMIGAKI_IOSTREAMS_DEVICE_FILE_HPP
