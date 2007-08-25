// functional.hpp: function object adaptors

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/

#ifndef HAMIGAKI_FUNCTIONAL_HPP
#define HAMIGAKI_FUNCTIONAL_HPP

#include <functional>

namespace hamigaki {

template<class S, class T>
class mem_var_equal_to_t : public std::binary_function<S,S,bool>
{
public:
    explicit mem_var_equal_to_t(T S::* p) : ptr_(p)
    {
    }

    bool operator()(const S& x, const S& y) const
    {
        return x.*ptr_ == y.*ptr_;
    }

private:
    T S::* ptr_;
};

template<class S, class T>
inline mem_var_equal_to_t<S,T> mem_var_equal_to(T S::* p)
{
    return mem_var_equal_to_t<S,T>(p);
}

template<class S, class T>
class mem_var_not_equal_to_t : public std::binary_function<S,S,bool>
{
public:
    explicit mem_var_not_equal_to_t(T S::* p) : ptr_(p)
    {
    }

    bool operator()(const S& x, const S& y) const
    {
        return x.*ptr_ != y.*ptr_;
    }

private:
    T S::* ptr_;
};

template<class S, class T>
inline mem_var_not_equal_to_t<S,T> mem_var_not_equal_to(T S::* p)
{
    return mem_var_not_equal_to_t<S,T>(p);
}

template<class S, class T>
class mem_var_greator_t : public std::binary_function<S,S,bool>
{
public:
    explicit mem_var_greator_t(T S::* p) : ptr_(p)
    {
    }

    bool operator()(const S& x, const S& y) const
    {
        return x.*ptr_ > y.*ptr_;
    }

private:
    T S::* ptr_;
};

template<class S, class T>
inline mem_var_greator_t<S,T> mem_var_greator(T S::* p)
{
    return mem_var_greator_t<S,T>(p);
}

template<class S, class T>
class mem_var_less_t : public std::binary_function<S,S,bool>
{
public:
    explicit mem_var_less_t(T S::* p) : ptr_(p)
    {
    }

    bool operator()(const S& x, const S& y) const
    {
        return x.*ptr_ < y.*ptr_;
    }

private:
    T S::* ptr_;
};

template<class S, class T>
inline mem_var_less_t<S,T> mem_var_less(T S::* p)
{
    return mem_var_less_t<S,T>(p);
}

template<class S, class T>
class mem_var_greator_equal_t : public std::binary_function<S,S,bool>
{
public:
    explicit mem_var_greator_equal_t(T S::* p) : ptr_(p)
    {
    }

    bool operator()(const S& x, const S& y) const
    {
        return x.*ptr_ >= y.*ptr_;
    }

private:
    T S::* ptr_;
};

template<class S, class T>
inline mem_var_greator_equal_t<S,T> mem_var_greator_equal(T S::* p)
{
    return mem_var_greator_equal_t<S,T>(p);
}

template<class S, class T>
class mem_var_less_equal_t : public std::binary_function<S,S,bool>
{
public:
    explicit mem_var_less_equal_t(T S::* p) : ptr_(p)
    {
    }

    bool operator()(const S& x, const S& y) const
    {
        return x.*ptr_ <= y.*ptr_;
    }

private:
    T S::* ptr_;
};

template<class S, class T>
inline mem_var_less_equal_t<S,T> mem_var_less_equal(T S::* p)
{
    return mem_var_less_equal_t<S,T>(p);
}

} // End namespace hamigaki.

#endif // HAMIGAKI_FUNCTIONAL_HPP
