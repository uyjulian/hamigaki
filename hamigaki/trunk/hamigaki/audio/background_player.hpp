// background_player.hpp: background audio player

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_BACKGROUND_PLAYER_HPP
#define HAMIGAKI_AUDIO_BACKGROUND_PLAYER_HPP

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/version.hpp>

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251)
#endif

#if BOOST_WORKAROUND(BOOST_VERSION, == 103800)
    #include <boost/date_time/date_defs.hpp> // kepp above thread.hpp
#endif
#include <boost/thread/thread.hpp>

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#include <hamigaki/thread/exception_storage.hpp>
#include <boost/iostreams/detail/buffer.hpp>
#include <boost/iostreams/constants.hpp>
#include <boost/iostreams/flush.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

namespace hamigaki { namespace audio {

namespace detail
{

template<typename T> 
struct seek_in_impl;

template<>
struct seek_in_impl<boost::iostreams::detail::random_access>
{
    template<typename T>
    static std::streampos
    seek(T& t, boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return boost::iostreams::seek(t, off, way, BOOST_IOS::in);
    }
};

template<>
struct seek_in_impl<boost::iostreams::any_tag>
{
    template<typename T>
    static std::streampos
    seek(T&, boost::iostreams::stream_offset, BOOST_IOS::seekdir)
    {
        return std::streampos(std::streamoff(-1));
    }
};

template<typename T>
inline std::streampos
seek_in(T& t, boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
{
    typedef typename boost::iostreams::detail::dispatch<
        T,
        boost::iostreams::detail::random_access,
        boost::iostreams::any_tag
    >::type tag;

    return seek_in_impl<tag>::seek(t, off, way);
}

class bg_player_base
{
public:
    virtual ~bg_player_base(){}

    void run()
    {
        do_run();
    }

    bool done()
    {
        return do_done();
    }

    std::streampos tell()
    {
        return do_tell();
    }

    void stop()
    {
        return do_stop();
    }

    void reset()
    {
        return do_reset();
    }

    std::streampos seek(
        boost::iostreams::stream_offset off,
        BOOST_IOS::seekdir way)
    {
        return do_seek(off, way);
    }

private:
    virtual void do_run() = 0;
    virtual bool do_done() = 0;
    virtual std::streampos do_tell() = 0;
    virtual void do_stop() = 0;
    virtual void do_reset() = 0;

    virtual std::streampos do_seek(
        boost::iostreams::stream_offset off,
        BOOST_IOS::seekdir way) = 0;
};

template<class Source, class Sink, class ExceptionStorage>
class bg_player_impl
    : public bg_player_base
{
    typedef typename boost::iostreams::char_type_of<Source>::type char_type;
    typedef boost::iostreams::detail::buffer<char_type> buffer_type;

public:
    bg_player_impl(
            const Source& src, const Sink& sink,
            std::streamsize buffer_size, ExceptionStorage& storage)
        : src_(src), sink_(sink), buffer_(buffer_size)
        , done_(false), interrupted_(false), except_ptr_(&storage)
    {
        buffer_.set(0, 0);
        position_ = boost::iostreams::position_to_offset(
            hamigaki::audio::detail::seek_in(src_, 0, BOOST_IOS::cur));
        seekable_ = (position_ != std::streampos(-1));
    }

private:
    boost::mutex mutex_;
    Source src_;
    Sink sink_;
    buffer_type buffer_;
    bool seekable_;
    volatile boost::iostreams::stream_offset position_;
    volatile bool done_;
    volatile bool interrupted_;
    ExceptionStorage* except_ptr_;

    void do_run() // virtual
    {
        try
        {
            buffer_type& buf = buffer_;

            while (!interrupted())
            {
                if (buf.ptr() == buf.eptr())
                {
                    std::streamsize amt =
                        boost::iostreams::read(src_, buf.data(), buf.size());

                    if (amt == -1)
                        break;

                    buf.set(0, amt);
                }

                while (buf.ptr() != buf.eptr())
                {
                    std::streamsize amt =
                        boost::iostreams::write(
                            sink_, buf.ptr(), buf.eptr() - buf.ptr());

                    buf.ptr() += amt;

                    if (seekable_)
                    {
                        boost::mutex::scoped_lock locking(mutex_);
                        position_ += amt;
                    }
                }
            }
            boost::iostreams::flush(sink_);
        }
        catch (...)
        {
            except_ptr_->store();
        }

        boost::mutex::scoped_lock locking(mutex_);
        done_ = true;
    }

    bool do_done() // virtual
    {
        boost::mutex::scoped_lock locking(mutex_);
        return done_;
    }

    std::streampos do_tell() // virtual
    {
        boost::mutex::scoped_lock locking(mutex_);
        return boost::iostreams::offset_to_position(position_);
    }

    void do_stop() // virtual
    {
        boost::mutex::scoped_lock locking(mutex_);
        interrupted_ = true;
    }

    bool interrupted()
    {
        boost::mutex::scoped_lock locking(mutex_);
        return interrupted_;
    }

    void do_reset() // virtual
    {
        done_ = false;
        interrupted_ = false;
    }

    std::streampos do_seek(
        boost::iostreams::stream_offset off,
        BOOST_IOS::seekdir way) // virtual
    {
        position_ = hamigaki::audio::detail::seek_in(src_, off, way);
        return boost::iostreams::offset_to_position(position_);
    }
};

} // namespace detail

template<class ExceptionStorage=hamigaki::thread::exception_storage>
class basic_background_player
    : boost::noncopyable
    , ExceptionStorage // for Empty Base Optimization
{
private:
    struct safe_bool_helper
    {
        void non_null() {};
    };

    typedef void (safe_bool_helper::*safe_bool)();

public:
    basic_background_player(){}

    template<typename Source, typename Sink>
    basic_background_player(const Source& src, const Sink& sink,
        std::streamsize buffer_size = 
            boost::iostreams::default_device_buffer_size)
    {
        this->open(src, sink, buffer_size);
    }

    ~basic_background_player()
    {
        if (thread_ptr_.get())
        {
            try
            {
                stop();
            }
            catch (...)
            {
            }
        }
    }

    template<typename Source, typename Sink>
    void open(const Source& src, const Sink& sink,
        std::streamsize buffer_size = 
            boost::iostreams::default_device_buffer_size)
    {
        BOOST_ASSERT(pimpl_.get() == 0);

        typedef detail::bg_player_impl<
            Source,Sink,ExceptionStorage> impl_type;

        pimpl_.reset(new impl_type(
            src, sink, buffer_size, static_cast<ExceptionStorage&>(*this)));
    }

    void close()
    {
        BOOST_ASSERT(pimpl_.get());

        if (thread_ptr_.get())
            stop();
        pimpl_.reset();
        ExceptionStorage::clear();
    }

    operator safe_bool() const
    {
        if (pimpl_.get() != 0)
            return &safe_bool_helper::non_null;
        else
            return static_cast<safe_bool>(0);
    }

    bool operator!() const
    {
        return pimpl_.get() == 0;
    }

    bool playing()
    {
        if (boost::thread* ptr = thread_ptr_.get())
        {
            BOOST_ASSERT(pimpl_.get());

            bool done = pimpl_->done();
            if (done)
            {
                ptr->join();
                thread_ptr_.reset();
                ExceptionStorage::rethrow();
            }
            return !done;
        }
        else
            return false;
    }

    void play()
    {
        BOOST_ASSERT(pimpl_.get());

        pimpl_->reset();
        thread_ptr_.reset(
            new boost::thread(
                boost::bind(&detail::bg_player_base::run, pimpl_.get())
            )
        );
    }

    void stop()
    {
        BOOST_ASSERT(pimpl_.get());

        if (thread_ptr_.get())
        {
            pimpl_->stop();
            thread_ptr_->join();
            thread_ptr_.reset();
        }
        ExceptionStorage::rethrow();
    }

    std::streampos tell()
    {
        BOOST_ASSERT(pimpl_.get());

        return pimpl_->tell();
    }

    std::streampos seek(
        boost::iostreams::stream_offset off,
        BOOST_IOS::seekdir way)
    { 
        BOOST_ASSERT(pimpl_.get());
        BOOST_ASSERT(thread_ptr_.get() == 0);

        return pimpl_->seek(off, way);
    }

    const ExceptionStorage& exception() const
    {
        return static_cast<const ExceptionStorage&>(*this);
    }

private:
    std::auto_ptr<detail::bg_player_base> pimpl_;
    std::auto_ptr<boost::thread> thread_ptr_;
};

typedef basic_background_player<> background_player;

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_BACKGROUND_PLAYER_HPP
