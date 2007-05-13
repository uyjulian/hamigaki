// demultiplexer.hpp: stream demultiplexer

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEMULTIPLEXER_HPP
#define HAMIGAKI_IOSTREAMS_DEMULTIPLEXER_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <vector>

namespace hamigaki { namespace iostreams {

namespace detail
{

template<typename CharT>
class demuxer_component_base
{
public:
    typedef CharT char_type;

    virtual ~demuxer_component_base() {}
    virtual std::streamsize write(const CharT* s, std::streamsize n) = 0;
    virtual void close() = 0;
};

template<class Sink>
class demuxer_component
    : public demuxer_component_base<
        typename boost::iostreams::char_type_of<Sink>::type
    >
{
public:
    typedef typename boost::iostreams::char_type_of<Sink>::type char_type;

    explicit demuxer_component(const Sink& sink) : sink_(sink)
    {
    }

    std::streamsize write(const char_type* s, std::streamsize n) // virtual
    {
        return boost::iostreams::write(sink_, s, n);
    }

    void close() // virtual
    {
        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

    const Sink* component() const
    {
        return &sink_;
    }

private:
    Sink sink_;
};

template<typename CharT>
class demultiplexer_impl
{
private:
    struct closer
    {
        void operator()(detail::demuxer_component_base<CharT>& sink) const
        {
            sink.close();
        }
    };

public:
    demultiplexer_impl() : pos_(0)
    {
    }

    template<typename T>
    const T* component(int n) const
    {
        typedef detail::demuxer_component<T> wrapper;

        if (const wrapper* ptr = dynamic_cast<const wrapper*>(&sinks_[n]))
            return ptr->component();
        else
            return 0;
    }

    template<typename Sink>
    void push(const Sink& sink)
    {
        sinks_.push_back(new detail::demuxer_component<Sink>(sink));
    }

    std::streamsize write(const CharT* s, std::streamsize n)
    {
        if (sinks_.empty())
            return std::max<std::streamsize>(n, 0);

        std::streamsize total = 0;
        while (total < n)
        {
            std::streamsize amt = sinks_[pos_].write(s + total, 1);
            if (amt < 0)
                break;
            pos_ = (pos_ + 1) % sinks_.size();
            ++total;
        }

        return total;
    }

    void close()
    {
        std::for_each(sinks_.begin(), sinks_.end(), closer());
    }

private:
    boost::ptr_vector<detail::demuxer_component_base<CharT> > sinks_;
    std::size_t pos_;
};

} // namespace detail

template<typename CharT>
class basic_demultiplexer
{
    typedef detail::demultiplexer_impl<CharT> impl_type;

public:
    typedef CharT char_type;

    struct category :
        boost::iostreams::sink_tag,
        boost::iostreams::closable_tag {};

    basic_demultiplexer() : pimpl_(new impl_type)
    {
    }

    template<typename T>
    const T* component(int n) const
    {
        return pimpl_->component<T>(n);
    }

    template<typename Sink>
    void push(const Sink& sink)
    {
        pimpl_->push(sink);
    }

    std::streamsize write(const CharT* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    void close()
    {
        pimpl_->close();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEMULTIPLEXER_HPP
