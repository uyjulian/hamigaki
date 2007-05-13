// multiplexer.hpp: stream multiplexer

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_MULTIPLEXER_HPP
#define HAMIGAKI_IOSTREAMS_MULTIPLEXER_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <vector>

namespace hamigaki { namespace iostreams {

namespace detail
{

template<typename CharT>
class muxer_component_base
{
public:
    typedef CharT char_type;

    virtual ~muxer_component_base() {}
    virtual std::streamsize read(CharT* s, std::streamsize n) = 0;
    virtual void close() = 0;
};

template<class Source>
class muxer_component
    : public muxer_component_base<
        typename boost::iostreams::char_type_of<Source>::type
    >
{
public:
    typedef typename boost::iostreams::char_type_of<Source>::type char_type;

    explicit muxer_component(const Source& src) : src_(src)
    {
    }

    std::streamsize read(char_type* s, std::streamsize n) // virtual
    {
        return boost::iostreams::read(src_, s, n);
    }

    void close() // virtual
    {
        boost::iostreams::close(src_, BOOST_IOS::in);
    }

    const Source* component() const
    {
        return &src_;
    }

private:
    Source src_;
};

template<typename CharT>
class multiplexer_impl
{
private:
    struct closer
    {
        void operator()(detail::muxer_component_base<CharT>& src) const
        {
            src.close();
        }
    };

public:
    multiplexer_impl() : pos_(0)
    {
    }

    template<typename T>
    const T* component(int n) const
    {
        typedef detail::muxer_component<T> wrapper;

        if (const wrapper* ptr = dynamic_cast<const wrapper*>(&sources_[n]))
            return ptr->component();
        else
            return 0;
    }

    template<typename Source>
    void push(const Source& src)
    {
        sources_.push_back(new detail::muxer_component<Source>(src));
    }

    std::streamsize read(CharT* s, std::streamsize n)
    {
        if (sources_.empty())
            return -1;

        std::streamsize total = 0;
        while (total < n)
        {
            std::streamsize amt = sources_[pos_].read(s + total, 1);
            if (amt < 0)
                break;
            pos_ = (pos_ + 1) % sources_.size();
            ++total;
        }

        return (total != 0) ? total : -1;
    }

    void close()
    {
        std::for_each(sources_.begin(), sources_.end(), closer());
    }

private:
    boost::ptr_vector<detail::muxer_component_base<CharT> > sources_;
    std::size_t pos_;
};

} // namespace detail

template<typename CharT>
class basic_multiplexer
{
    typedef detail::multiplexer_impl<CharT> impl_type;

public:
    typedef CharT char_type;

    struct category :
        boost::iostreams::source_tag,
        boost::iostreams::closable_tag {};

    basic_multiplexer() : pimpl_(new impl_type)
    {
    }

    template<typename T>
    const T* component(int n) const
    {
        return pimpl_->component<T>(n);
    }

    template<typename Source>
    void push(const Source& src)
    {
        pimpl_->push(src);
    }

    std::streamsize read(CharT* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

    void close()
    {
        pimpl_->close();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_MULTIPLEXER_HPP
