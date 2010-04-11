// mme_device.cpp: Windows Multimedia Extensions wave device

// Copyright Takeshi Mouri 2006-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/mme_device.hpp>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/noncopyable.hpp>
#include <cstring>
#include <stdexcept>

#include <hamigaki/iostreams/arbitrary_positional_facade.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include "detail/wave_format_ex.hpp"
#include <windows.h>
#include <mmsystem.h>

namespace hamigaki { namespace audio {

namespace
{

const std::size_t wave_buffer_count = 4;

class semaphore : boost::noncopyable
{
public:
    semaphore(long init_value, long max_value) :
        handle_(::CreateSemaphore(0, init_value, max_value, 0))
    {
    }

    ~semaphore()
    {
        ::CloseHandle(handle_);
    }

    long release(long n)
    {
        long prev;
        ::ReleaseSemaphore(handle_, n, &prev);
        return prev;
    }

    void wait()
    {
        ::WaitForSingleObject(handle_, INFINITE);
    }

private:
    ::HANDLE handle_;
};

class wave_buffer : boost::noncopyable
{
public:
    wave_buffer(::HWAVEOUT handle, std::size_t size) :
        handle_(handle), buffer_(size), pos_(0)
    {
        std::memset(&header_, 0, sizeof(header_));

        header_.lpData = &buffer_[0];
        header_.dwBufferLength  = size;
        header_.dwFlags = 0;

        ::MMRESULT res =
            ::waveOutPrepareHeader(handle_, &header_, sizeof(header_));
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("cannot prepare wave buffer");
    }

    ~wave_buffer()
    {
        ::waveOutUnprepareHeader(handle_, &header_, sizeof(header_));
    }

    void flush()
    {
        if (pos_ != 0)
        {
            header_.dwBufferLength = pos_;
            pos_ = 0;
            ::waveOutWrite(handle_, &header_, sizeof(header_));
        }
    }

    bool write(const char* data, std::size_t size, std::size_t& written)
    {
        written = (std::min)(buffer_.size() - pos_, size);
        std::memcpy(&buffer_[pos_], data, written);
        pos_ += written;
        if (pos_ == buffer_.size())
        {
            pos_ = 0;
            header_.dwBufferLength = buffer_.size();
            ::MMRESULT res = ::waveOutWrite(handle_, &header_, sizeof(header_));
            if (res != MMSYSERR_NOERROR)
                throw BOOST_IOSTREAMS_FAILURE("wave device write error");
            return true;
        }
        return false;
    }

private:
    ::HWAVEOUT handle_;
    std::vector<char> buffer_;
    ::WAVEHDR header_;
    std::size_t pos_;
};

class wave_in_buffer : boost::noncopyable
{
public:
    wave_in_buffer(::HWAVEIN handle, std::size_t size) :
        handle_(handle), buffer_(size), pos_(0)
    {
        std::memset(&header_, 0, sizeof(header_));

        header_.lpData = &buffer_[0];
        header_.dwBufferLength = size;
        header_.dwFlags = 0;

        ::MMRESULT res =
            ::waveInPrepareHeader(handle_, &header_, sizeof(header_));
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("cannot prepare wave buffer");
    }

    ~wave_in_buffer()
    {
        ::waveInUnprepareHeader(handle_, &header_, sizeof(header_));
    }

    void add()
    {
        pos_ = 0;
#if 0
        ::waveInUnprepareHeader(handle_, &header_, sizeof(header_));
        std::memset(&header_, 0, sizeof(header_));
        header_.lpData = &buffer_[0];
        header_.dwBufferLength = buffer_.size();
        header_.dwFlags = 0;
        ::waveInPrepareHeader(handle_, &header_, sizeof(header_));
#else
        // Is this OK?
        header_.dwBytesRecorded = 0;
        header_.dwFlags &= ~WHDR_DONE;
#endif

        ::MMRESULT res = ::waveInAddBuffer(handle_, &header_, sizeof(header_));
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("waveInAddBuffer failed");
    }

    bool read(char* data, std::size_t size, std::size_t& amt)
    {
        amt = (std::min)(buffer_.size() - pos_, size);
        std::memcpy(data, &buffer_[pos_], amt);
        pos_ += amt;
        if (pos_ == buffer_.size())
            return true;
        return false;
    }

private:
    ::HWAVEIN handle_;
    std::vector<char> buffer_;
    ::WAVEHDR header_;
    std::size_t pos_;
};

} // namespace

class mme_sink::impl
    : public hamigaki::iostreams::
        arbitrary_positional_facade<
#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
            mme_sink::
#endif
            impl, char, 4
        >
    , private boost::noncopyable
{
    friend class hamigaki::iostreams::core_access;

public:
    impl(unsigned id, const pcm_format& f, std::size_t buffer_size)
        : impl::arbitrary_positional_facade_(f.block_size())
        , sema_(wave_buffer_count, wave_buffer_count)
        , block_size_(f.block_size())
        , pos_(0), has_buffer_(false), format_(f)
    {
        if (buffer_size % block_size_ != 0)
            throw BOOST_IOSTREAMS_FAILURE("invalid buffer size");

        detail::wave_format_ex fmt(f);

        ::MMRESULT res = ::waveOutOpen(
            &handle_, id, &fmt,
            reinterpret_cast< ::DWORD_PTR>(&mme_sink::impl::Callback),
            reinterpret_cast< ::DWORD_PTR>(&sema_),
            CALLBACK_FUNCTION);
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("waveOutOpen failed");

        for (std::size_t i = 0; i < wave_buffer_count; ++i)
            buffers_.push_back(new wave_buffer(handle_, buffer_size));
    }

    ~impl()
    {
        ::waveOutReset(handle_);
        ::waveOutClose(handle_);
    }

    pcm_format format() const
    {
        return format_;
    }

    bool flush()
    {
        if (has_buffer_)
        {
            buffers_[pos_].flush();
            has_buffer_ = false;
            pos_ = (pos_ + 1) % buffers_.size();
        }

        for (std::size_t i = 0; i < buffers_.size(); ++i)
            sema_.wait();
        sema_.release(buffers_.size());

        return true;
    }

    void close()
    {
        flush();
        ::waveOutReset(handle_);
    }

private:
    ::HWAVEOUT handle_;
    semaphore sema_;
    boost::ptr_vector<wave_buffer> buffers_;
    std::size_t block_size_;
    std::size_t pos_;
    bool has_buffer_;
    pcm_format format_;

    static void CALLBACK Callback(::HWAVEOUT hwo, ::UINT uMsg,
        ::DWORD_PTR dwInstance, ::DWORD dwParam1, ::DWORD dwParam2)
    {
        if (uMsg != WOM_DONE)
            return;

        semaphore& sema = *reinterpret_cast<semaphore*>(dwInstance);
        sema.release(1);
    }

    std::streamsize write_blocks(const char* s, std::streamsize n)
    {
        std::streamsize result = n * block_size_;
        while (n > 0)
        {
            if (!has_buffer_)
            {
                sema_.wait();
                has_buffer_ = true;
            }

            std::size_t written;
            if (buffers_[pos_].write(s, n*block_size_, written))
            {
                has_buffer_ = false;
                pos_ = (pos_ + 1) % buffers_.size();
                n -= written/block_size_;
                s += written;
            }
            else
                break;
        }
        return result;
    }
};

class mme_source::impl
    : public hamigaki::iostreams::
        arbitrary_positional_facade<
#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
            mme_source::
#endif
            impl, char, 4
        >
    , private boost::noncopyable
{
    friend class hamigaki::iostreams::core_access;

public:
    impl(unsigned id, const pcm_format& f, std::size_t buffer_size)
        : impl::arbitrary_positional_facade_(f.block_size())
        , sema_(0, wave_buffer_count)
        , block_size_(f.block_size())
        , pos_(0), has_buffer_(false), format_(f)
    {
        if (buffer_size % block_size_ != 0)
            throw BOOST_IOSTREAMS_FAILURE("invalid buffer size");

        detail::wave_format_ex fmt(f);

        ::MMRESULT res = ::waveInOpen(
            &handle_, id, &fmt,
            reinterpret_cast< ::DWORD_PTR>(&mme_source::impl::Callback),
            reinterpret_cast< ::DWORD_PTR>(&sema_),
            CALLBACK_FUNCTION);
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("waveInOpen failed");

        for (std::size_t i = 0; i < wave_buffer_count; ++i)
        {
            buffers_.push_back(new wave_in_buffer(handle_, buffer_size));
            buffers_[i].add();
        }
    }

    ~impl()
    {
        close();
        ::waveInReset(handle_);
        ::waveInClose(handle_);
    }

    pcm_format format() const
    {
        return format_;
    }

    void close()
    {
        ::waveInStop(handle_);
        ::waveInReset(handle_);

        std::size_t count = buffers_.size();
        if (has_buffer_)
            --count;
        for (std::size_t i = 0; i < count; ++i)
            sema_.wait();
        has_buffer_ = false;

        for (std::size_t i = 0; i < wave_buffer_count; ++i)
            buffers_[i].add();
    }

private:
    ::HWAVEIN handle_;
    semaphore sema_;
    boost::ptr_vector<wave_in_buffer> buffers_;
    std::size_t block_size_;
    std::size_t pos_;
    bool has_buffer_;
    pcm_format format_;

    static void CALLBACK Callback(::HWAVEOUT hwo, ::UINT uMsg,
        ::DWORD_PTR dwInstance, ::DWORD dwParam1, ::DWORD dwParam2)
    {
        if (uMsg != WIM_DATA)
            return;

        semaphore& sema = *reinterpret_cast<semaphore*>(dwInstance);
        sema.release(1);
    }

    std::streamsize read_blocks(char* s, std::streamsize n)
    {
        ::waveInStart(handle_);
        std::streamsize total = 0;
        while (n > 0)
        {
            if (!has_buffer_)
            {
                sema_.wait();
                has_buffer_ = true;
            }

            std::size_t amt;
            if (buffers_[pos_].read(s, n*block_size_, amt))
            {
                has_buffer_ = false;
                buffers_[pos_].add();
                pos_ = (pos_ + 1) % buffers_.size();
                n -= amt/block_size_;
                s += amt;
                total += amt;
            }
            else
                break;
        }
        return (total != 0) ? total : -1;
    }
};

mme_sink::mme_sink(const pcm_format& f)
    : pimpl_(new impl(WAVE_MAPPER, f, f.optimal_buffer_size()))
{
}

mme_sink::mme_sink(const pcm_format& f, std::size_t buffer_size)
    : pimpl_(new impl(WAVE_MAPPER, f, buffer_size))
{
}

mme_sink::mme_sink(unsigned id, const pcm_format& f)
    : pimpl_(new impl(id, f, f.optimal_buffer_size()))
{
}

mme_sink::mme_sink(unsigned id, const pcm_format& f, std::size_t buffer_size)
    : pimpl_(new impl(id, f, buffer_size))
{
}

pcm_format mme_sink::format() const
{
    return pimpl_->format();
}

std::streamsize mme_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

bool mme_sink::flush()
{
    return pimpl_->flush();
}

void mme_sink::close()
{
    pimpl_->close();
}

mme_source::mme_source(const pcm_format& f)
    : pimpl_(new impl(WAVE_MAPPER, f, f.optimal_buffer_size()))
{
}

mme_source::mme_source(const pcm_format& f, std::size_t buffer_size)
    : pimpl_(new impl(WAVE_MAPPER, f, buffer_size))
{
}

mme_source::mme_source(unsigned id, const pcm_format& f)
    : pimpl_(new impl(id, f, f.optimal_buffer_size()))
{
}

mme_source::mme_source(
    unsigned id, const pcm_format& f, std::size_t buffer_size)
    : pimpl_(new impl(id, f, buffer_size))
{
}

pcm_format mme_source::format() const
{
    return pimpl_->format();
}

std::streamsize mme_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void mme_source::close()
{
    pimpl_->close();
}

} } // End namespaces audio, hamigaki.
