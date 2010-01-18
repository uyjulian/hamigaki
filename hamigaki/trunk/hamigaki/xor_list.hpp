// xor_list.hpp: XOR linked list container class

// Copyright Takeshi Mouri 2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/xor_list for library home page.

#ifndef HAMIGAKI_XOR_LIST_HPP
#define HAMIGAKI_XOR_LIST_HPP

#include <algorithm>
#include <climits>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>

namespace hamigaki {

template<class T, class Allocator=std::allocator<T> >
class xor_list;

namespace xl_detail
{

// uintptr_t

template<int Rank>
struct uint_fast_helper;

#if defined(ULLONG_MAX)
template<> struct uint_fast_helper<1> { typedef unsigned long long type; };
#endif
template<> struct uint_fast_helper<2> { typedef unsigned long type; };
template<> struct uint_fast_helper<3> { typedef unsigned type; };

template<int Bits>
struct uint_fast_t
{
    typedef typename uint_fast_helper<
#if defined(ULLONG_MAX)
        (Bits <= std::numeric_limits<unsigned long long>::digits) +
#else
        1 +
#endif
        (Bits <= std::numeric_limits<unsigned long>::digits) +
        (Bits <= std::numeric_limits<unsigned>::digits)
    >::type type;
};

typedef uint_fast_t<sizeof(void*)*CHAR_BIT>::type uintptr_t;


// is_integral

template<class T>
struct remove_cv
{
    typedef T type;
};

template<class T>
struct remove_cv<const T>
{
    typedef T type;
};

template<class T>
struct remove_cv<volatile T>
{
    typedef T type;
};

template<class T>
struct remove_cv<const volatile T>
{
    typedef T type;
};

template<bool val>
struct boolean_constant
{
    typedef boolean_constant<val> type;
};

typedef boolean_constant<true> true_type;
typedef boolean_constant<false> false_type;

template<class T>
struct is_integral_impl
: boolean_constant<
        std::numeric_limits<T>::is_specialized &&
        std::numeric_limits<T>::is_integer
> {};

template<class T>
struct is_integral : is_integral_impl<typename remove_cv<T>::type> {};


// node

struct xl_node_base
{
    uintptr_t link_;

    void link(xl_node_base* prev, xl_node_base* next)
    {
        link_ =
            reinterpret_cast<uintptr_t>(prev) ^
            reinterpret_cast<uintptr_t>(next) ;
    }

    void replace(xl_node_base* old_ptr, xl_node_base* new_ptr)
    {
        link_ ^=
            reinterpret_cast<uintptr_t>(old_ptr) ^
            reinterpret_cast<uintptr_t>(new_ptr) ;
    }

    xl_node_base* other(xl_node_base* p) const
    {
        return reinterpret_cast<xl_node_base*>(
            reinterpret_cast<uintptr_t>(p) ^ link_
        );
    }
};

template<class T>
struct xl_node : xl_node_base
{
    T value;
};


// iterators

class xl_iterator_base
{
public:
    xl_iterator_base()
    {
    }

    xl_iterator_base(xl_node_base* prev, xl_node_base* curr)
        : prev_ptr_(prev), curr_ptr_(curr)
    {
    }

    bool equal(const xl_iterator_base& lhs) const
    {
        return (prev_ptr_ == lhs.prev_ptr_) && (curr_ptr_ == lhs.curr_ptr_);
    }

    void increment()
    {
        xl_node_base* next_ptr = curr_ptr_->other(prev_ptr_);
        prev_ptr_ = curr_ptr_;
        curr_ptr_ = next_ptr;
    }

    void decrement()
    {
        xl_node_base* next_ptr = prev_ptr_->other(curr_ptr_);
        curr_ptr_ = prev_ptr_;
        prev_ptr_ = next_ptr;
    }

    xl_node_base* curr() const
    {
        return curr_ptr_;
    }

    xl_node_base* prev() const
    {
        return prev_ptr_;
    }

    xl_node_base* next() const
    {
        return curr_ptr_->other(prev_ptr_);
    }

private:
    xl_node_base* prev_ptr_;
    xl_node_base* curr_ptr_;
};

template<class T, class Distance, class Pointer, class Reference>
class const_xl_iterator;

template<class T, class Distance, class Pointer, class Reference>
class xl_iterator : private xl_iterator_base
{
    friend class const_xl_iterator<T,Distance,Pointer,Reference>;

    template<class U, class Allocator>
    friend class xor_list;

private:
    typedef xl_node<T> node_type;

public:
    typedef Distance difference_type;
    typedef T value_type;
    typedef Pointer pointer;
    typedef Reference reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    xl_iterator()
    {
    }

    xl_iterator(xl_node_base* prev, xl_node_base* curr)
        : xl_iterator_base(prev, curr)
    {
    }

    bool operator==(const xl_iterator& rhs) const
    {
        return this->equal(rhs);
    }

    bool operator!=(const xl_iterator& rhs) const
    {
        return !(*this == rhs);
    }

    T& operator*() const
    {
        return dereference();
    }

    T* operator->() const
    {
        return &dereference();
    }

    xl_iterator& operator++()
    {
        increment();
        return *this;
    }

    xl_iterator operator++(int)
    {
        xl_iterator tmp(*this);
        increment();
        return tmp;
    }

    xl_iterator& operator--()
    {
        decrement();
        return *this;
    }

    xl_iterator operator--(int)
    {
        xl_iterator tmp(*this);
        decrement();
        return tmp;
    }

private:
    T& dereference() const
    {
        return static_cast<node_type*>(curr())->value;
    }
};

template<class T, class Distance, class Pointer, class Reference>
class const_xl_iterator : private xl_iterator_base
{
    template<class U, class Allocator>
    friend class xor_list;

private:
    typedef xl_node<T> node_type;

public:
    typedef Distance difference_type;
    typedef T value_type;
    typedef Pointer pointer;
    typedef Reference reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    const_xl_iterator()
    {
    }

    const_xl_iterator(const xl_iterator<T,Distance,Pointer,Reference>& x)
        : xl_iterator_base(x)
    {
    }

    const_xl_iterator(xl_node_base* prev, xl_node_base* curr)
        : xl_iterator_base(prev, curr)
    {
    }

    bool operator==(const const_xl_iterator& rhs) const
    {
        return this->equal(rhs);
    }

    bool operator!=(const const_xl_iterator& rhs) const
    {
        return !(*this == rhs);
    }

    const T& operator*() const
    {
        return dereference();
    }

    const T* operator->() const
    {
        return &dereference();
    }

    const_xl_iterator& operator++()
    {
        increment();
        return *this;
    }

    const_xl_iterator operator++(int)
    {
        const_xl_iterator tmp(*this);
        increment();
        return tmp;
    }

    const_xl_iterator& operator--()
    {
        decrement();
        return *this;
    }

    const_xl_iterator operator--(int)
    {
        const_xl_iterator tmp(*this);
        decrement();
        return tmp;
    }

private:
    const T& dereference() const
    {
        return static_cast<node_type*>(curr())->value;
    }
};


// functors

template<class T>
struct default_less
{
    bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs < rhs;
    }
};

template<class T>
struct default_equal_to
{
    bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs == rhs;
    }
};

// utilities

template<class T>
inline T next(T i)
{
    return ++i;
}


// base list

class xor_list_base
{
private:
    typedef xl_node_base node_base;

public:
    xor_list_base()
    {
        head_.link(0, &tail_);
        tail_.link(&head_, 0);
    }

    xor_list_base(node_base* head_ptr, node_base* tail_ptr)
    {
        if (head_ptr == tail_ptr)
        {
            if (head_ptr)
            {
                head_.link(0, head_ptr);
                head_ptr->link(&head_, &tail_);
                tail_.link(tail_ptr, 0);
            }
            else
            {
                head_.link(0, &tail_);
                tail_.link(&head_, 0);
            }
        }
        else
        {
            head_.link(0, head_ptr);
            head_ptr->replace(0, &head_);
            tail_ptr->replace(0, &tail_);
            tail_.link(tail_ptr, 0);
        }
    }

    std::pair<xl_node_base*,xl_node_base*> release()
    {
        if (empty())
        {
            return std::pair<xl_node_base*,xl_node_base*>(
                static_cast<xl_node_base*>(0),
                static_cast<xl_node_base*>(0)
            );
        }

        xl_node_base* first = first_node();
        xl_node_base* last = last_node();

        head_.replace(first, &tail_);
        first->replace(&head_, 0);
        last->replace(&tail_, 0);
        tail_.replace(last, &head_);

        return std::pair<xl_node_base*,xl_node_base*>(first, last);
    }

    xl_node_base* first_node() const
    {
        return head_.other(0);
    }

    xl_node_base* last_node() const
    {
        return tail_.other(0);
    }

    xl_iterator_base begin()
    {
        return xl_iterator_base(&head_, first_node());
    }

    xl_iterator_base end()
    {
        return xl_iterator_base(last_node(), &tail_);
    }

    xl_iterator_base fix_iterator(xl_iterator_base i) const
    {
        xl_node_base* prev = i.prev();
        xl_node_base* curr = i.curr();

        if (!prev)
            prev = const_cast<xl_node_base*>(&head_);

        if (!curr)
            curr = const_cast<xl_node_base*>(&tail_);

        return xl_iterator_base(prev, curr);
    }

    bool empty() const
    {
        return head_.other(0) == &tail_;
    }

    xl_iterator_base insert(xl_iterator_base position, xl_node_base* node)
    {
        xl_node_base* prev = position.prev();
        xl_node_base* curr = position.curr();

        prev->replace(curr, node);
        node->link(prev, curr);
        curr->replace(prev, node);

        return xl_iterator_base(prev, node);
    }

    xl_iterator_base erase(xl_iterator_base position)
    {
        xl_node_base* prev = position.prev();
        xl_node_base* curr = position.curr();
        xl_node_base* next = position.next();

        prev->replace(curr, next);
        next->replace(curr, prev);

        return xl_iterator_base(prev, next);
    }

    void splice(xl_iterator_base position, xor_list_base& x)
    {
        if (x.empty())
            return;

        xl_node_base* prev = position.prev();
        xl_node_base* curr = position.curr();

        xl_node_base* first = x.first_node();
        xl_node_base* last = x.last_node();

        prev->replace(curr, first);
        first->replace(&x.head_, prev);
        last->replace(&x.tail_, curr);
        curr->replace(prev, last);

        x.head_.link(0, &x.tail_);
        x.tail_.link(&x.head_, 0);
    }

    void splice(
        xl_iterator_base position,
        xor_list_base& x, xl_iterator_base first, xl_iterator_base last)
    {
        if (first.curr() == last.curr())
            return;

        xl_node_base* prev = position.prev();
        xl_node_base* curr = position.curr();

        xl_node_base* first_prev = first.prev();
        xl_node_base* first_curr = first.curr();
        xl_node_base* last_prev = last.prev();
        xl_node_base* last_curr = last.curr();

        prev->replace(curr, first_curr);
        first_curr->replace(first_prev, prev);
        last_prev->replace(last_curr, curr);
        curr->replace(prev, last_prev);

        first_prev->replace(first_curr, last_curr);
        last_curr->replace(last_prev, first_prev);
    }

    template<class T, class Compare>
    void merge(xor_list_base& x, Compare comp)
    {
        typedef xl_node<T> node_type;

        if (x.empty())
            return;

        if (empty())
        {
            std::pair<xl_node_base*,xl_node_base*> ht = x.release();
            head_.link(0, ht.first);
            ht.first->replace(0, &head_);
            tail_.link(0, ht.second);
            ht.second->replace(0, &tail_);
            return;
        }

        xl_iterator_base li(&head_, first_node());
        xl_iterator_base ri(&x.head_, x.first_node());

        while ((li.curr() != &tail_) && (ri.curr() != &x.tail_))
        {
            if (comp(
                static_cast<node_type*>(ri.curr())->value,
                static_cast<node_type*>(li.curr())->value) )
            {
                xl_node_base* node = ri.curr();
                ri = x.erase(ri);
                li = this->insert(li, node);
                li.increment();
            }
            else
                li.increment();
        }
        this->splice(xl_iterator_base(last_node(), &tail_), x);
    }

    void swap(xor_list_base& x)
    {
        std::pair<xl_node_base*,xl_node_base*> lhs = release();
        std::pair<xl_node_base*,xl_node_base*> rhs = x.release();

        if (rhs.first)
        {
            head_.link(0, rhs.first);
            rhs.first->replace(0, &head_);
            tail_.link(0, rhs.second);
            rhs.second->replace(0, &tail_);
        }
        else
        {
            head_.link(0, &tail_);
            tail_.link(&head_, 0);
        }

        if (lhs.first)
        {
            x.head_.link(0, lhs.first);
            lhs.first->replace(0, &x.head_);
            x.tail_.link(0, lhs.second);
            lhs.second->replace(0, &x.tail_);
        }
        else
        {
            x.head_.link(0, &x.tail_);
            x.tail_.link(&x.head_, 0);
        }
    }

private:
    xl_node_base head_;
    xl_node_base tail_;

    // noncopyable
    xor_list_base(const xor_list_base&);
    xor_list_base& operator=(const xor_list_base&);
};

} // namespace xl_detail

template<class T, class Allocator>
class xor_list
    // for empty base optimization
    : private Allocator::template rebind<
        xl_detail::xl_node<T>
    >::other
{
private:
    typedef xl_detail::xl_node_base node_base;
    typedef xl_detail::xl_node<T> node_type;
    typedef typename Allocator::template rebind<node_type>::other node_alloc;
    typedef typename node_alloc::pointer node_ptr_t;

public:
    // types:
    typedef typename Allocator::reference reference;
    typedef typename Allocator::const_reference const_reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::size_t size_type;
    typedef T value_type;
    typedef Allocator allocator_type;
    typedef typename Allocator::pointer pointer;
    typedef typename Allocator::const_pointer const_pointer;

    typedef xl_detail::xl_iterator<
        T,difference_type,pointer,reference
    > iterator;

    typedef xl_detail::const_xl_iterator<
        T,difference_type,const_pointer,const_reference
    > const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // construct/copy/destroy:
    explicit xor_list(const Allocator& a=Allocator())
        : node_alloc(a), head_ptr_(0), tail_ptr_(0)
    {
    }

    explicit xor_list(
        size_type n, const T& value = T(), const Allocator& a = Allocator()
    )
        : node_alloc(a), head_ptr_(0), tail_ptr_(0)
    {
        this->insert(begin(), n, value);
    }

    template<class InputIterator>
    xor_list(
        InputIterator first, InputIterator last,
        const Allocator& a = Allocator()
    )
        : node_alloc(a), head_ptr_(0), tail_ptr_(0)
    {
        this->insert(begin(), first, last);
    }

    xor_list(const xor_list<T,Allocator>& x)
        : node_alloc(x.get_allocator()), head_ptr_(0), tail_ptr_(0)
    {
        this->insert(begin(), x.begin(), x.end());
    }

    ~xor_list()
    {
        clear();
    }

    xor_list<T,Allocator>& operator=(const xor_list<T,Allocator>& x)
    {
        if (&x != this)
        {
            iterator next = this->insert_helper<iterator>(
                begin(),
                iterator(0, x.head_ptr_),
                iterator(x.tail_ptr_, 0)
            );
            this->erase(next, end());
        }
        return *this;
    }

    template<class InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        iterator next =
            this->insert_helper<InputIterator>(begin(), first, last);
        this->erase(next, end());
    }

    void assign(size_type n, const T& t)
    {
        iterator next = this->insert_n(begin(), n, t);
        this->erase(next, end());
    }

    allocator_type get_allocator() const
    {
        return *this;
    }

    // iterators:
    iterator begin()
    {
        return iterator(0, head_ptr_);
    }

    const_iterator begin() const
    {
        return const_iterator(0, head_ptr_);
    }

    iterator end()
    {
        return iterator(tail_ptr_, 0);
    }

    const_iterator end() const
    {
        return const_iterator(tail_ptr_, 0);
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    // capacity:
    bool empty() const
    {
        return head_ptr_ == 0;
    }

    size_type size() const
    {
        return std::distance(begin(), end());
    }

    size_type max_size() const
    {
        return (std::numeric_limits<size_type>::max)();
    }

    void resize(size_type sz, T c = T())
    {
        size_type old_sz = size();
        if (old_sz < sz)
            this->insert(end(), sz-old_sz, c);
        else if (sz < old_sz)
        {
            iterator i = begin();
            std::advance(i, sz);
            this->erase(i, end());
        }
    }

    // element access:
    reference front()
    {
        return static_cast<node_type*>(head_ptr_)->value;
    }

    const_reference front() const
    {
        return static_cast<node_type*>(head_ptr_)->value;
    }

    reference back()
    {
        return static_cast<node_type*>(tail_ptr_)->value;
    }

    const_reference back() const
    {
        return static_cast<node_type*>(tail_ptr_)->value;
    }

    // modifiers:
    void push_front(const T& x)
    {
        node_type* new_ptr = this->alloc_node(x);
        if (head_ptr_)
        {
            new_ptr->link(0, head_ptr_);
            head_ptr_->replace(0, new_ptr);
        }
        else
        {
            new_ptr->link(0, 0);
            tail_ptr_ = new_ptr;
        }
        head_ptr_ = new_ptr;
    }

    void pop_front()
    {
        node_base* next = head_ptr_->other(0);
        if (next)
            next->replace(head_ptr_, 0);
        else
            tail_ptr_ = 0;
        this->free_node(static_cast<node_type*>(head_ptr_));
        head_ptr_ = next;
    }

    void push_back(const T& x)
    {
        node_type* new_ptr = this->alloc_node(x);
        if (tail_ptr_)
        {
            new_ptr->link(tail_ptr_, 0);
            tail_ptr_->replace(0, new_ptr);
        }
        else
        {
            new_ptr->link(0, 0);
            head_ptr_ = new_ptr;
        }
        tail_ptr_ = new_ptr;
    }

    void pop_back()
    {
        node_base* prev = tail_ptr_->other(0);
        if (prev)
            prev->replace(tail_ptr_, 0);
        else
            head_ptr_ = 0;
        this->free_node(static_cast<node_type*>(tail_ptr_));
        tail_ptr_ = prev;
    }


    iterator insert(iterator position, const T& x)
    {
        node_type* new_ptr = this->alloc_node(x);
        node_base* prev = position.prev();
        node_base* curr = position.curr();
        new_ptr->link(prev, curr);

        if (curr)
            curr->replace(prev, new_ptr);
        else
            tail_ptr_ = new_ptr;

        if (prev)
            prev->replace(curr, new_ptr);
        else
            head_ptr_ = new_ptr;

        return iterator(prev, new_ptr);
    }

    void insert(iterator position, size_type n, const T& x)
    {
        this->insert_n(position, n, x);
    }

    template<class InputIterator>
    void insert(
        iterator position,
        InputIterator first, InputIterator last)
    {
        this->insert_helper<InputIterator>(position, first, last);
    }


    iterator erase(iterator position)
    {
        node_base* node = position.curr();
        position = this->release_node(position);
        this->free_node(static_cast<node_type*>(node));
        return position;
    }

    iterator erase(iterator position, iterator last)
    {
        node_base* last_node = last.curr();
        while (position.curr() != last_node)
            position = this->erase(position);
        return position;
    }

    void swap(xor_list<T,Allocator>& x)
    {
        if (get_allocator() == x.get_allocator())
        {
            std::swap(head_ptr_, x.head_ptr_);
            std::swap(tail_ptr_, x.tail_ptr_);
        }
        else
        {
            iterator l_mid = this->insert_helper(begin(), x.begin(), x.end());
            iterator r_mid;
            try
            {
                r_mid = x.insert_helper(x.begin(), l_mid, end());
            }
            catch (...)
            {
                this->erase(begin(), l_mid);
                throw;
            }
            this->erase(l_mid, end());
            x.erase(r_mid, x.end());
        }
    }

    void clear()
    {
        node_base* prev_ptr = 0;
        node_base* curr_ptr = head_ptr_;
        while (curr_ptr)
        {
            node_base* next_ptr = curr_ptr->other(prev_ptr);
            this->free_node(static_cast<node_type*>(curr_ptr));
            prev_ptr = curr_ptr;
            curr_ptr = next_ptr;
        }
        head_ptr_ = 0;
        tail_ptr_ = 0;
    }

    // list operations:
    void splice(iterator position, xor_list<T,Allocator>& x)
    {
        if (x.empty())
            return;

        if (get_allocator() == x.get_allocator())
        {
            xl_detail::xor_list_base dst(head_ptr_, tail_ptr_);
            xl_detail::xor_list_base src(x.head_ptr_, x.tail_ptr_);

            dst.splice(dst.fix_iterator(position), src);

            this->reset(dst);
            x.head_ptr_ = 0;
            x.tail_ptr_ = 0;
        }
        else
        {
            this->insert(position, x.begin(), x.end());
            x.clear();
        }
    }

    void splice(iterator position, xor_list<T,Allocator>& x, iterator i)
    {
        if ((position == i) || (position == xl_detail::next(i)))
            return;

        if (get_allocator() == x.get_allocator())
        {
            node_base* node = i.curr();
            if (position.prev() == node)
                position = iterator(i.prev(), position.curr());
            x.release_node(i);
            this->insert_node(position, node);
        }
        else
        {
            this->insert(position, *i);
            x.erase(i);
        }
    }

    void splice(
        iterator position, xor_list<T,Allocator>& x,
        iterator first, iterator last)
    {
        if (first == last)
            return;

        if (&x == this)
        {
            xl_detail::xor_list_base dst(head_ptr_, tail_ptr_);
            xl_detail::xor_list_base src;

            xl_detail::xl_iterator_base pos = dst.fix_iterator(position);
            node_base* prev = pos.prev();
            node_base* curr = pos.curr();

            if (position == last)
                prev = first.prev();

            src.splice(
                src.end(),
                dst,
                dst.fix_iterator(first),
                dst.fix_iterator(last)
            );

            dst.splice(
                dst.fix_iterator(xl_detail::xl_iterator_base(prev, curr)),
                src
            );

            this->reset(dst);
        }
        else if (get_allocator() == x.get_allocator())
        {
            xl_detail::xor_list_base dst(head_ptr_, tail_ptr_);
            xl_detail::xor_list_base src(x.head_ptr_, x.tail_ptr_);

            dst.splice(
                dst.fix_iterator(position),
                src,
                src.fix_iterator(first),
                src.fix_iterator(last)
            );

            this->reset(dst);
            x.reset(src);
        }
        else
        {
            this->insert(position, first, last);
            x.erase(first, last);
        }
    }


    void remove(const T& value)
    {
        iterator i = begin();
        while (i.curr())
        {
            if (*i == value)
                i = this->erase(i);
            else
                ++i;
        }
    }

    template<class Predicate>
    void remove_if(Predicate pred)
    {
        iterator i = begin();
        while (i.curr())
        {
            if (pred(*i))
                i = this->erase(i);
            else
                ++i;
        }
    }


    void unique()
    {
        this->unique(xl_detail::default_equal_to<T>());
    }

    template<class BinaryPredicate>
    void unique(BinaryPredicate binary_pred)
    {
        // (size() == 0) or (size() == 1)
        if (head_ptr_ == tail_ptr_)
            return;

        iterator prev = begin();
        iterator curr = prev;
        ++curr;
        while (curr.curr())
        {
            if (binary_pred(*curr, *prev))
            {
                curr = this->erase(curr);
                prev = curr;
                --prev;
            }
            else
            {
                prev = curr;
                ++curr;
            }
        }
    }


    void merge(xor_list<T,Allocator>& x)
    {
        this->merge(x, xl_detail::default_less<T>());
    }

    template<class Compare>
    void merge(xor_list<T,Allocator>& x, Compare comp)
    {
        if (x.empty())
            return;

        if (empty())
        {
            this->splice(end(), x);
            return;
        }

        if (get_allocator() == x.get_allocator())
        {
            iterator position = begin();
            while (position.curr() && !x.empty())
            {
                if (comp(x.front(), *position))
                {
                    iterator i = x.begin();
                    node_base* node = i.curr();
                    x.release_node(i);
                    position = this->insert_node(position, node);
                }
                ++position;
            }
            this->splice(end(), x);
        }
        else
        {
            iterator mid = this->insert_helper(end(), x.begin(), x.end());
            x.clear();
            iterator position = begin();
            while ((position != mid) && mid.curr())
            {
                if (comp(*mid, *position))
                {
                    node_base* node = mid.curr();
                    mid = this->release_node(mid);
                    position = this->insert_node(position, node);
                }
                ++position;
            }
        }
    }


    void sort()
    {
        this->sort(xl_detail::default_less<T>());
    }

    template<class Compare>
    void sort(Compare comp)
    {
        // (size() == 0) or (size() == 1)
        if (head_ptr_ == tail_ptr_)
            return;

        xl_detail::xor_list_base list(head_ptr_, tail_ptr_);

        xl_detail::xor_list_base stack[sizeof(size_type)*CHAR_BIT];
        unsigned sz = 1;
        try
        {
            while (!list.empty())
            {
                node_base* node = list.first_node();
                list.erase(list.begin());
                node->link(0, 0);
                xl_detail::xor_list_base tmp(node, node);
                unsigned i = 0;
                try
                {
                    for ( ; i < sz; ++i)
                    {
                        if (stack[i].empty())
                            break;

                        stack[i].merge<T>(tmp, comp);
                        tmp.swap(stack[i]);
                    }
                }
                catch (...)
                {
                    list.splice(list.end(), tmp);
                    throw;
                }
                stack[i].swap(tmp);
                if (i == sz)
                    ++sz;
            }

            for (unsigned i = 0; i < sz; ++i)
                list.merge<T>(stack[i], comp);
        }
        catch (...)
        {
            for (unsigned i = 0; i < sz; ++i)
                list.splice(list.end(), stack[i]);
            this->reset(list);
            throw;
        }

        this->reset(list);
    }


    void reverse()
    {
        std::swap(head_ptr_, tail_ptr_);
    }

private:
    node_base* head_ptr_;
    node_base* tail_ptr_;

    node_type* alloc_node(const T& x)
    {
        node_type* new_ptr = &*node_alloc::allocate(1);
        try
        {
            new(static_cast<void*>(&(new_ptr->value))) T(x);
        }
        catch (...)
        {
            node_alloc::deallocate(node_ptr_t(new_ptr), 1);
            throw;
        }
        return new_ptr;
    }

    void free_node(node_type* ptr)
    {
        ptr->value.~T();
        node_alloc::deallocate(node_ptr_t(ptr), 1);
    }

    void reset(xl_detail::xor_list_base& x)
    {
        std::pair<node_base*,node_base*> ht = x.release();
        head_ptr_ = ht.first;
        tail_ptr_ = ht.second;
    }

    iterator insert_n(iterator position, size_type n, const T& x)
    {
        if (n == 0)
            return position;

        position = this->insert(position, x);
        iterator i = position;
        ++position;
        --n;
        try
        {
            while (n != 0)
            {
                position = this->insert(position, x);
                ++position;
                --n;
            }
        }
        catch (...)
        {
            this->erase(i, position);
            throw;
        }
        return position;
    }

    iterator insert_impl(iterator position, T n, T x, xl_detail::true_type)
    {
        return insert_n(
            position,
            static_cast<size_type>(n), static_cast<value_type>(x)
        );
    }

    template<class InputIterator>
    iterator insert_impl(
        iterator position,
        InputIterator first, InputIterator last,
        xl_detail::false_type)
    {
        if (first == last)
            return position;

        position = this->insert(position, *first);
        iterator i = position;
        ++position;
        ++first;
        try
        {
            while (first != last)
            {
                position = this->insert(position, *first);
                ++position;
                ++first;
            }
        }
        catch (...)
        {
            this->erase(i, position);
            throw;
        }
        return position;
    }

    template<class InputIterator>
    iterator insert_helper(
        iterator position,
        InputIterator first, InputIterator last)
    {
        return insert_impl(
            position, first, last,
            typename xl_detail::is_integral<InputIterator>::type());
    }

    iterator release_node(iterator position)
    {
        node_base* prev = position.prev();
        node_base* curr = position.curr();
        node_base* next = curr->other(prev);

        if (next)
            next->replace(curr, prev);
        else
            tail_ptr_ = prev;

        if (prev)
            prev->replace(curr, next);
        else
            head_ptr_ = next;

        return iterator(prev, next);
    }

    iterator insert_node(iterator position, node_base* node)
    {
        node_base* prev = position.prev();
        node_base* curr = position.curr();

        if (curr)
            curr->replace(prev, node);
        else
            tail_ptr_ = node;

        if (prev)
            prev->replace(curr, node);
        else
            head_ptr_ = node;

        node->link(prev, curr);

        return iterator(prev, node);
    }
};

template<class T, class Allocator>
inline bool
operator==(const xor_list<T,Allocator>& x, const xor_list<T,Allocator>& y)
{
    return (x.size() == y.size()) && std::equal(x.begin(), x.end(), y.begin());
}

template<class T, class Allocator>
inline bool
operator< (const xor_list<T,Allocator>& x, const xor_list<T,Allocator>& y)
{
    return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}

template<class T, class Allocator>
inline bool
operator!=(const xor_list<T,Allocator>& x, const xor_list<T,Allocator>& y)
{
    return !(x == y);
}

template<class T, class Allocator>
inline bool
operator> (const xor_list<T,Allocator>& x, const xor_list<T,Allocator>& y)
{
    return y < x;
}

template<class T, class Allocator>
inline bool
operator>=(const xor_list<T,Allocator>& x, const xor_list<T,Allocator>& y)
{
    return !(x < y);
}

template<class T, class Allocator>
inline bool
operator<=(const xor_list<T,Allocator>& x, const xor_list<T,Allocator>& y)
{
    return !(x > y);
}

// specialized algorithms:
template<class T, class Allocator>
inline void swap(xor_list<T,Allocator>& x, xor_list<T,Allocator>& y)
{
    x.swap(y);
}

} // End namespace hamigaki.

#endif // HAMIGAKI_XOR_LIST_HPP
