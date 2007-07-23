// list.hpp: list of string

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_LIST_HPP
#define HAMIGAKI_BJAM_UTIL_LIST_HPP

#include <hamigaki/iterator/optional_iterator.hpp>
#include <boost/assert.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/version.hpp>
#include <string>
#include <vector>

#if defined(BOOST_SPIRIT_DEBUG)
    #include <ostream>
#endif

namespace hamigaki { namespace bjam {

class string_list : boost::totally_ordered<string_list>
{
private:
    typedef std::vector<std::string> impl_type;

    struct safe_bool_helper
    {
        void non_null() {};
    };

    typedef void (safe_bool_helper::*safe_bool)();

public:
    typedef impl_type::reference reference;
    typedef impl_type::const_reference const_reference;
    typedef optional_iterator<impl_type::iterator> iterator;
    typedef optional_iterator<impl_type::const_iterator> const_iterator;
    typedef impl_type::size_type size_type;
    typedef impl_type::difference_type difference_type;
    typedef impl_type::value_type value_type;
    typedef impl_type::allocator_type allocator_type;
    typedef impl_type::pointer pointer;
    typedef impl_type::const_pointer const_pointer;
    typedef optional_iterator<impl_type::reverse_iterator> reverse_iterator;
    typedef optional_iterator<
        impl_type::const_reverse_iterator
    > const_reverse_iterator;

    string_list()
    {
    }

    explicit string_list(const std::string& s)
        : pimpl_(new impl_type(1u, s))
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
        else if (pimpl_.unique())
            pimpl_->push_back(s);
        else
        {
            boost::shared_ptr<impl_type> tmp(new impl_type(*pimpl_));
            tmp->push_back(s);
            pimpl_.swap(tmp);
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
        else if (pimpl_.unique())
            pimpl_->insert(position.base(), first, last);
        else
        {
            boost::shared_ptr<impl_type> tmp(new impl_type(*pimpl_));
            std::ptrdiff_t dist = position.base() - pimpl_->begin();
            typename impl_type::iterator pos = tmp->begin() + dist;
            tmp->insert(pos, first, last);
            pimpl_.swap(tmp);
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

    // additional member functions

    int compare(const string_list& rhs) const
    {
        const_iterator lb = this->begin();
        const_iterator le = this->end();

        const_iterator rb = rhs.begin();
        const_iterator re = rhs.end();

        while (true)
        {
            if (lb != le)
            {
                if (rb != re)
                {
                    if (int cmp = (lb++)->compare(*(rb++)))
                        return cmp;
                }
                else
                {
                    if (!(lb++)->empty())
                        return 1;
                }
            }
            else
            {
                if (rb != re)
                {
                    if (!(rb++)->empty())
                        return -1;
                }
                else
                    break;
            }
        }
        return 0;
    }

    operator safe_bool() const
    {
        if (this->compare(string_list()) != 0)
            return &safe_bool_helper::non_null;
        else
            return static_cast<safe_bool>(0);
    }

    bool operator!() const
    {
        return !this->operator safe_bool();
    }

    bool operator<(const string_list& rhs) const
    {
        return this->compare(rhs) < 0;
    }

    bool operator==(const string_list& rhs) const
    {
        return this->compare(rhs) == 0;
    }

    string_list& operator+=(const std::string& rhs)
    {
        this->push_back(rhs);
        return *this;
    }

    string_list& operator+=(const string_list& rhs)
    {
        if (impl_type* rp = rhs.pimpl_.get())
        {
            if (pimpl_.get() == 0)
                pimpl_ = rhs.pimpl_;
            else if (pimpl_.unique())
                pimpl_->insert(pimpl_->end(), rp->begin(), rp->end());
            else
            {
                boost::shared_ptr<impl_type> tmp(new impl_type(*pimpl_));
                tmp->insert(tmp->end(), rp->begin(), rp->end());
                pimpl_.swap(tmp);
            }
        }
        return *this;
    }

    boost::optional<std::string> try_front() const
    {
        if (impl_type* p = pimpl_.get())
            return p->front();
        else
            return boost::optional<std::string>();
    }

    void sort()
    {
        if (impl_type* p = pimpl_.get())
        {
            if (pimpl_.unique())
                std::sort(p->begin(), p->end());
            else
            {
                boost::shared_ptr<impl_type> tmp(new impl_type(*p));
                std::sort(tmp->begin(), tmp->end());
                pimpl_.swap(tmp);
            }
        }
    }

    void unique()
    {
        if (impl_type* p = pimpl_.get())
        {
            if (pimpl_.unique())
                p->erase(std::unique(p->begin(), p->end()), p->end());
            else
            {
                boost::shared_ptr<impl_type> tmp(new impl_type);
                std::unique_copy(
                    tmp->begin(), tmp->end(),
                    std::back_inserter(*tmp)
                );
                pimpl_.swap(tmp);
            }
        }
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

#if defined(BOOST_SPIRIT_DEBUG)
inline std::ostream& operator<<(std::ostream& os, const string_list& x)
{
    typedef string_list::const_iterator iter_type;

    iter_type beg = x.begin();
    iter_type end = x.end();

    for (iter_type i = beg; i != end; ++i)
    {
        if (i != beg)
            os << ' ';

        os << *i;
    }
    return os;
}
#endif

} } // End namespaces bjam, hamigaki.

namespace phoenix {

struct logical_not_op;

template<typename TagT, typename T>
struct unary_operator;

template<>
struct unary_operator<logical_not_op,hamigaki::bjam::string_list>
{
    typedef bool result_type;

    static result_type eval(const hamigaki::bjam::string_list& v)
    {
        return !v;
    }
};

template<>
struct unary_operator<logical_not_op,const hamigaki::bjam::string_list>
{
    typedef bool result_type;

    static result_type eval(const hamigaki::bjam::string_list& v)
    {
        return !v;
    }
};

} // End namespace phoenix.

#endif // HAMIGAKI_BJAM_UTIL_LIST_HPP
