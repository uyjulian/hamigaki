// background_copy.hpp: copy operations by the other thread

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_BACKGROUND_COPY_HPP
#define HAMIGAKI_IOSTREAMS_BACKGROUND_COPY_HPP

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
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/constants.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp> 
#include <boost/iostreams/write.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

namespace hamigaki { namespace iostreams {

namespace detail
{

class bg_copy_base
{
public:
    virtual ~bg_copy_base(){}

    void run()
    {
        do_run();
    }

    bool done()
    {
        return do_done();
    }

    std::streamsize total()
    {
        return do_total();
    }

    void stop()
    {
        return do_stop();
    }

private:
    virtual void do_run() = 0;
    virtual bool do_done() = 0;
    virtual std::streamsize do_total() = 0;
    virtual void do_stop() = 0;
};

template<class Source, class Sink, class ExceptionStorage>
class bg_copy_impl : public bg_copy_base
{
    typedef typename boost::iostreams::char_type_of<Source>::type char_type;
    typedef boost::iostreams::detail::buffer<char_type> buffer_type;

public:
    bg_copy_impl(
            const Source& src, const Sink& sink,
            std::streamsize buffer_size, ExceptionStorage& storage)
        : src_(src), sink_(sink), buffer_(buffer_size), total_(0)
        , done_(false), interrupted_(false), except_ptr_(&storage)
    {
        buffer_.set(0, 0);
    }

private:
    boost::mutex mutex_;
    Source src_;
    Sink sink_;
    buffer_type buffer_;
    volatile std::streamsize total_;
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
                    {
                        boost::iostreams::close(src_, BOOST_IOS::in);
                        boost::iostreams::close(sink_, BOOST_IOS::out);
                        break;
                    }

                    buf.set(0, amt);
                }

                while (buf.ptr() != buf.eptr())
                {
                    std::streamsize amt =
                        boost::iostreams::write(
                            sink_, buf.ptr(), buf.eptr() - buf.ptr());

                    buf.ptr() += amt;

                    boost::mutex::scoped_lock locking(mutex_);
                    total_ += amt;
                }
            }
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

    std::streamsize do_total() // virtual
    {
        boost::mutex::scoped_lock locking(mutex_);
        return total_;
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
};

} // namespace detail

template<class ExceptionStorage=hamigaki::thread::exception_storage>
class basic_background_copy
    : boost::noncopyable
#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    , ExceptionStorage // for Empty Base Optimization
#endif
{
public:
    template<typename Source, typename Sink>
    basic_background_copy(const Source& src, const Sink& sink,
        std::streamsize buffer_size = 
            boost::iostreams::default_device_buffer_size)
    {
        typedef detail::bg_copy_impl<
            Source,Sink,ExceptionStorage> impl_type;

        pimpl_.reset(new impl_type(
            src, sink, buffer_size, except()));

        thread_ptr_.reset(
            new boost::thread(
                boost::bind(&detail::bg_copy_base::run, pimpl_.get())
            )
        );
    }

    ~basic_background_copy()
    {
        try
        {
            stop();
        }
        catch (...)
        {
        }
    }

    bool done()
    {
        bool result = pimpl_->done();
        if (result)
        {
            thread_ptr_->join();
            thread_ptr_.reset();
            except().rethrow();
        }
        return result;
    }

    void wait()
    {
        thread_ptr_->join();
        thread_ptr_.reset();
        except().rethrow();
    }

    void stop()
    {
        if (thread_ptr_.get())
        {
            pimpl_->stop();
            thread_ptr_->join();
            thread_ptr_.reset();
        }
        except().rethrow();
    }

    std::streamsize total()
    {
        return pimpl_->total();
    }

    const ExceptionStorage& exception() const
    {
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
        return except_;
#else
        return static_cast<const ExceptionStorage&>(*this);
#endif
    }

private:
    std::auto_ptr<detail::bg_copy_base> pimpl_;
    std::auto_ptr<boost::thread> thread_ptr_;
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    ExceptionStorage except_;
#endif

    ExceptionStorage& except()
    {
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
        return except_;
#else
        return static_cast<ExceptionStorage&>(*this);
#endif
    }
};

typedef basic_background_copy<> background_copy;

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_BACKGROUND_COPY_HPP
