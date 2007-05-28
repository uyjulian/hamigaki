// list.hpp: list of string

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_LIST_HPP
#define HAMIGAKI_BJAM_UTIL_LIST_HPP

#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

namespace hamigaki { namespace bjam {

class string_list
{
private:
    typedef std::vector<std::string> impl_type;

public:
    typedef impl_type::reference reference;
    typedef impl_type::const_reference const_reference;
    typedef impl_type::iterator iterator;
    typedef impl_type::const_iterator const_iterator;
    typedef impl_type::size_type size_type;
    typedef impl_type::difference_type difference_type;
    typedef impl_type::value_type value_type;
    typedef impl_type::allocator_type allocator_type;
    typedef impl_type::pointer pointer;
    typedef impl_type::const_pointer const_pointer;
    typedef impl_type::reverse_iterator reverse_iterator;
    typedef impl_type::const_reverse_iterator const_reverse_iterator;

    string_list()
    {
    }

    template<class InputIterator>
    string_list(InputIterator first, InputIterator last)
        : pimpl_(first == last
            ? static_cast<impl_type*>(0)
            : new impl_type(first, last)
        )
    {
    }

    iterator begin()
    {
        if (impl_type* p = pimpl_.get())
            return p->begin();
        else
            return iterator();
    }

    const_iterator begin() const
    {
        if (const impl_type* p = pimpl_.get())
            return p->begin();
        else
            return const_iterator();
    }

    iterator end()
    {
        if (impl_type* p = pimpl_.get())
            return p->end();
        else
            return iterator();
    }

    const_iterator end() const
    {
        if (const impl_type* p = pimpl_.get())
            return p->end();
        else
            return const_iterator();
    }

    reverse_iterator rbegin()
    {
        if (impl_type* p = pimpl_.get())
            return p->rbegin();
        else
            return reverse_iterator();
    }

    const_reverse_iterator rbegin() const
    {
        if (const impl_type* p = pimpl_.get())
            return p->rbegin();
        else
            return const_reverse_iterator();
    }

    reverse_iterator rend()
    {
        if (impl_type* p = pimpl_.get())
            return p->rend();
        else
            return reverse_iterator();
    }

    const_reverse_iterator rend() const
    {
        if (const impl_type* p = pimpl_.get())
            return p->rend();
        else
            return const_reverse_iterator();
    }

    size_type size() const
    {
        if (const impl_type* p = pimpl_.get())
            return p->size();
        else
            return 0;
    }

    bool empty() const
    {
        return pimpl_.get() == 0;
    }

    const std::string& operator[](size_type n) const
    {
        return (*pimpl_)[n];
    }

    void push_back(const std::string& s)
    {
        if (pimpl_.get() == 0)
            pimpl_.reset(new impl_type(1u, s));
        else
        {
            unique();
            pimpl_->push_back(s);
        }
    }

    template<class InputIterator>
    void insert(iterator position, InputIterator first, InputIterator last)
    {
        if (pimpl_.get() == 0)
        {
            BOOST_ASSERT(position == iterator());
            pimpl_.reset(new impl_type(first, last));
        }
        else
        {
            unique();
            pimpl_->insert(position, first, last);
        }
    }

    void swap(string_list& rhs)
    {
        pimpl_.swap(rhs.pimpl_);
    }

    void clear()
    {
        pimpl_.reset();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;

    void unique()
    {
        if (!pimpl_.unique())
        {
            boost::shared_ptr<impl_type> tmp(new impl_type(*pimpl_));
            pimpl_.swap(tmp);
        }
    }
};

typedef string_list list_type;

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_LIST_HPP
