// xor_list_test.hpp: tests for XOR linked list container class

// Copyright Takeshi Mouri 2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/xor_list for library home page.

#include <hamigaki/xor_list.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/is_unsigned.hpp>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/utility.hpp>
#include "fake_allocator.hpp"
#include "own_allocator.hpp"

namespace ut = boost::unit_test;

struct dummy
{
    int value;

    dummy() : value(0)
    {
    }

    explicit dummy(int n) : value(n)
    {
    }

    bool operator==(const dummy& rhs) const
    {
        return value == rhs.value;
    }

    bool operator!=(const dummy& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(const dummy& rhs) const
    {
        return value < rhs.value;
    }

    bool operator>(const dummy& rhs) const
    {
        return rhs < *this;
    }
};

template<class CharT, class Traits>
std::basic_ostream<CharT,Traits>&
operator<<(std::basic_ostream<CharT,Traits>& os, const dummy& x)
{
    return os << x.value;
}

template<class T>
struct max_value
{
    static const boost::uintmax_t value =
        static_cast<boost::uintmax_t>(boost::integer_traits<T>::const_max);
};


void type_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    BOOST_MPL_ASSERT((boost::is_same<X::value_type, T>));
    BOOST_MPL_ASSERT((boost::is_same<X::reference, T&>));
    BOOST_MPL_ASSERT((boost::is_same<X::const_reference, const T&>));

    BOOST_MPL_ASSERT((boost::is_signed<X::difference_type>));
    BOOST_MPL_ASSERT(
        (boost::is_same<X::difference_type, X::iterator::difference_type>));
    BOOST_MPL_ASSERT((
        boost::is_same<
            X::difference_type,
            X::const_iterator::difference_type
        >
    ));

    BOOST_MPL_ASSERT((boost::is_unsigned<X::size_type>));
    BOOST_STATIC_ASSERT(
        max_value<X::difference_type>::value <= max_value<X::size_type>::value
    );

    BOOST_MPL_ASSERT((boost::is_same<X::pointer, T*>));
    BOOST_MPL_ASSERT((boost::is_same<X::const_pointer, const T*>));
}

void construct_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    BOOST_CHECK(u.size() == 0);
    BOOST_CHECK(X().size() == 0);
}

void construct_n_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u(2, T(1));
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == T(1));
    BOOST_CHECK(u.back() == T(1));
}

void construct_range_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(1), T(2), T(3) }};
    X u(data.begin(), data.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(), u.end(), data.begin(), data.end());
}

void construct_fake_range_test()
{
    typedef int T;
    typedef hamigaki::xor_list<T> X;
    short n = 2;
    short x = 1;
    X u(n, x);
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == 1);
    BOOST_CHECK(u.back() == 1);
}

void copy_construct_test()
{
    typedef int T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(1), T(2), T(3) }};
    X a(data.begin(), data.end());
    X b(a);
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
}

void copy_assign_test()
{
    typedef int T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(1), T(2), T(3) }};
    X a(data.begin(), data.end());
    X b;
    b = a;
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(), a.end(), b.begin(), b.end());
}

void assign_range_test()
{
    typedef int T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(1), T(2), T(3) }};
    X u;
    u.assign(data.begin(), data.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(),u.end(),data.begin(),data.end());
}

void assign_fake_range_test()
{
    typedef int T;
    typedef hamigaki::xor_list<T> X;

    X u;
    short n = 2;
    short x = 1;
    u.assign(n, x);
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == 1);
    BOOST_CHECK(u.back() == 1);
}

void assign_n_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    u.assign(2, T(1));
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == T(1));
    BOOST_CHECK(u.back() == T(1));
}

void get_allocator_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X::allocator_type a;
    X u(a);
    const X& t = u;
    BOOST_CHECK(t.get_allocator() == a);
}

void begin_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    u.push_back(T(1));
    u.push_back(T(2));
    BOOST_CHECK(*u.begin() == T(1));

    const X& t = u;
    BOOST_CHECK(*t.begin() == T(1));
}

void end_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    u.push_back(T(1));
    u.push_back(T(2));
    BOOST_CHECK(*boost::prior(u.end()) == T(2));

    const X& t = u;
    BOOST_CHECK(*boost::prior(t.end()) == T(2));
}

void rbegin_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    u.push_back(T(1));
    u.push_back(T(2));
    BOOST_CHECK(*u.rbegin() == T(2));

    const X& t = u;
    BOOST_CHECK(*t.rbegin() == T(2));
}

void rend_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    u.push_back(T(1));
    u.push_back(T(2));
    BOOST_CHECK(*boost::prior(u.rend()) == T(1));

    const X& t = u;
    BOOST_CHECK(*boost::prior(t.rend()) == T(1));
}

void empty_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    BOOST_CHECK(static_cast<const X&>(u).empty());

    u.push_back(T(1));
    BOOST_CHECK(!static_cast<const X&>(u).empty());
}

void size_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    BOOST_CHECK(u.size() == 0);

    u.push_back(T(1));
    BOOST_CHECK(u.size() == 1);

    u.push_back(T(2));
    BOOST_CHECK(u.size() == 2);
}

void max_size_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    const X t;
    BOOST_CHECK(t.max_size() != 0);
}

void resize_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    u.resize(0);
    BOOST_CHECK(u.size() == 0);

    u.resize(1);
    BOOST_REQUIRE(u.size() == 1);
    BOOST_REQUIRE(u.front() == T());

    u.resize(2, T(2));
    BOOST_REQUIRE(u.size() == 2);
    BOOST_REQUIRE(u.front() == T());
    BOOST_REQUIRE(u.back() == T(2));

    u.resize(1);
    BOOST_REQUIRE(u.size() == 1);
    BOOST_REQUIRE(u.front() == T());
}

void front_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    u.push_back(T(1));
    u.push_back(T(2));
    BOOST_CHECK(u.front() == T(1));

    const X& t = u;
    BOOST_CHECK(t.front() == T(1));
}

void back_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X u;
    u.push_back(T(1));
    u.push_back(T(2));
    BOOST_CHECK(u.back() == T(2));

    const X& t = u;
    BOOST_CHECK(t.back() == T(2));
}

void push_front_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    T x(1);
    u.push_front(x);
    BOOST_REQUIRE(u.size() == 1);
    BOOST_CHECK(u.front() == x);
    BOOST_CHECK(u.back() == x);

    T x2(2);
    u.push_front(x2);
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == x2);
    BOOST_CHECK(u.back() == x);
}

void pop_front_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    T x(1);
    T x2(2);
    u.push_front(x);
    u.push_front(x2);

    u.pop_front();
    BOOST_REQUIRE(u.size() == 1);
    BOOST_CHECK(u.front() == x);
    BOOST_CHECK(u.back() == x);

    u.pop_front();
    BOOST_REQUIRE(u.size() == 0);
}

void push_back_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    T x(1);
    u.push_back(x);
    BOOST_REQUIRE(u.size() == 1);
    BOOST_CHECK(u.front() == x);
    BOOST_CHECK(u.back() == x);

    T x2(2);
    u.push_back(x2);
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == x);
    BOOST_CHECK(u.back() == x2);
}

void pop_back_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    T x(1);
    T x2(2);
    u.push_back(x);
    u.push_back(x2);

    u.pop_back();
    BOOST_REQUIRE(u.size() == 1);
    BOOST_CHECK(u.front() == x);
    BOOST_CHECK(u.back() == x);

    u.pop_back();
    BOOST_REQUIRE(u.size() == 0);
}

void insert_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    T x(1);
    X::iterator i = u.insert(u.end(), x);
    BOOST_REQUIRE(u.size() == 1);
    BOOST_CHECK(u.front() == x);
    BOOST_CHECK(u.back() == x);
    BOOST_CHECK(i == u.begin());
    BOOST_CHECK(*i == x);

    T x2(2);
    i = u.insert(u.begin(), x2);
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == x2);
    BOOST_CHECK(u.back() == x);
    BOOST_CHECK(i == u.begin());
    BOOST_CHECK(*i == x2);

    T x3(3);
    ++i;
    i = u.insert(i, x3);
    BOOST_REQUIRE(u.size() == 3);
    BOOST_CHECK(u.front() == x2);
    BOOST_CHECK(u.back() == x);
    BOOST_CHECK(*i == x3);
}

void insert_n_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    u.insert(u.end(), 2, T(1));
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == T(1));
    BOOST_CHECK(u.back() == T(1));

    X::iterator i = u.begin();
    ++i;
    u.insert(i, 1, T(2));
    BOOST_REQUIRE(u.size() == 3);
    i = u.begin();
    BOOST_CHECK(*i == T(1));
    ++i;
    BOOST_CHECK(*i == T(2));
    ++i;
    BOOST_CHECK(*i == T(1));
}

void insert_range_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    T data[] = { T(1), T(2), T(3) };
    u.insert(u.end(), data, data+3);
    BOOST_REQUIRE(u.size() == 3);
    BOOST_CHECK(u.front() == data[0]);
    BOOST_CHECK(u.back() == data[2]);

    X::iterator i = u.begin();
    ++i;
    BOOST_CHECK(*i == data[1]);

    T data2[] = { T(4), T(5) };
    u.insert(i, data2, data2+2);
    BOOST_REQUIRE(u.size() == 5);

    i = u.begin();
    BOOST_CHECK(*i == data[0]);
    ++i;
    BOOST_CHECK(*i == data2[0]);
    ++i;
    BOOST_CHECK(*i == data2[1]);
    ++i;
    BOOST_CHECK(*i == data[1]);
    ++i;
    BOOST_CHECK(*i == data[2]);
}

void insert_fake_range_test()
{
    typedef int T;
    typedef hamigaki::xor_list<T> X;
    X u;
    const short n = 2;
    const short x = 1;
    u.insert(u.end(), n, x);
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == 1);
    BOOST_CHECK(u.back() == 1);
}

void erase_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    T x(1);
    T x2(2);
    T x3(3);
    u.push_back(x);
    u.push_back(x2);
    u.push_back(x3);

    X::iterator i = u.begin();
    ++i;
    i = u.erase(i);
    BOOST_REQUIRE(u.size() == 2);
    BOOST_CHECK(u.front() == x);
    BOOST_CHECK(u.back() == x3);
    BOOST_CHECK(i != u.end());
    BOOST_CHECK(*i == x3);

    i = u.erase(i);
    BOOST_REQUIRE(u.size() == 1);
    BOOST_CHECK(u.front() == x);
    BOOST_CHECK(u.back() == x);
    BOOST_CHECK(i == u.end());

    i = u.erase(u.begin());
    BOOST_REQUIRE(u.size() == 0);
    BOOST_CHECK(i == u.end());
}

void erase_range_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X u;
    T data[] = { T(1), T(2), T(3), T(4), T(5) };
    u.insert(u.end(), data, data+5);

    X::iterator i = u.begin();
    ++i;
    X::iterator j = i;
    ++j;
    ++j;
    i = u.erase(i, j);
    BOOST_REQUIRE(u.size() == 3);
    BOOST_CHECK(u.front() == data[0]);
    BOOST_CHECK(u.back() == data[4]);
    BOOST_CHECK(i != u.end());
    BOOST_CHECK(*i == data[3]);

    i = u.erase(u.begin(), u.end());
    BOOST_REQUIRE(u.size() == 0);
    BOOST_CHECK(i == u.end());
}

void swap_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X a;
    T x(1);
    T x2(2);
    a.push_back(x);
    a.push_back(x2);

    X b;
    T x3(3);
    b.push_back(x3);

    a.swap(b);

    BOOST_REQUIRE(a.size() == 1);
    BOOST_CHECK(a.front() == x3);
    BOOST_CHECK(a.back() == x3);

    BOOST_REQUIRE(b.size() == 2);
    BOOST_CHECK(b.front() == x);
    BOOST_CHECK(b.back() == x2);
}

void clear_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;

    X a;
    a.clear();
    BOOST_CHECK(a.size() == 0);

    a.push_back(T(1));
    a.clear();
    BOOST_CHECK(a.size() == 0);
}

void splice_all_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(1), T(2), T(3) }};
    X a(data.begin(), data.end());

    boost::array<T,2> data2 = {{ T(4), T(5) }};
    X b(data2.begin(), data2.end());

    a.splice(a.begin(), b);
    boost::array<T,5> data3 = {{ T(4), T(5), T(1), T(2), T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_CHECK(b.empty());
}

void splice_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(1), T(2), T(3) }};
    X a(data.begin(), data.end());

    X::iterator i = a.begin();
    ++i;
    a.splice(a.begin(), a, i);
    data[0] = T(2);
    data[1] = T(1);
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data.begin(),data.end());

    boost::array<T,2> data2 = {{ T(4), T(5) }};
    X b(data2.begin(), data2.end());

    i = a.begin();
    ++i;
    X::iterator j = b.begin();
    a.splice(i, b, j);
    boost::array<T,4> data3 = {{ T(2), T(4), T(1), T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_REQUIRE(b.size() == 1);
    BOOST_CHECK(b.front() == T(5));
}

void splice_range_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(1), T(2), T(3) }};
    X a(data.begin(), data.end());

    X::iterator i = a.begin();
    ++i;
    a.splice(a.begin(), a, i, a.end());
    data[0] = T(2);
    data[1] = T(3);
    data[2] = T(1);
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data.begin(),data.end());

    i = a.begin();
    ++i;
    ++i;
    a.splice(a.begin(), a, i, a.end());
    data[0] = T(1);
    data[1] = T(2);
    data[2] = T(3);
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data.begin(),data.end());

    boost::array<T,3> data2 = {{ T(4), T(5), T(6) }};
    X b(data2.begin(), data2.end());

    i = a.begin();
    ++i;
    X::iterator j = b.begin();
    ++j;
    a.splice(i, b, j, b.end());
    boost::array<T,5> data3 = {{ T(1), T(5), T(6), T(2), T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_REQUIRE(b.size() == 1);
    BOOST_CHECK(b.front() == T(4));
}

void remove_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,4> data = {{ T(1), T(2), T(3), T(1) }};
    X u(data.begin(), data.end());

    u.remove(T(2));
    boost::array<T,3> data2 = {{ T(1), T(3), T(1) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(),u.end(),data2.begin(),data2.end());

    u.remove(T(1));
    boost::array<T,1> data3 = {{ T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(),u.end(),data3.begin(),data3.end());
}

void remove_if_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,4> data = {{ T(1), T(2), T(3), T(1) }};
    X u(data.begin(), data.end());

    u.remove_if(std::bind2nd(std::less<T>(), T(3)));
    boost::array<T,1> data2 = {{ T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(),u.end(),data2.begin(),data2.end());
}

void unique_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,5> data = {{ T(1), T(1), T(2), T(3), T(3) }};
    X u(data.begin(), data.end());

    u.unique();
    boost::array<T,3> data2 = {{ T(1), T(2), T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(),u.end(),data2.begin(),data2.end());
}

void unique_pred_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,5> data = {{ T(1), T(1), T(2), T(3), T(3) }};
    X u(data.begin(), data.end());

    u.unique(std::equal_to<T>());
    boost::array<T,3> data2 = {{ T(1), T(2), T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(),u.end(),data2.begin(),data2.end());
}

void merge_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    X a;
    X b;

    a.merge(b);
    BOOST_CHECK(a.empty());
    BOOST_CHECK(b.empty());

    boost::array<T,3> data = {{ T(1), T(3), T(5) }};
    b.insert(b.end(), data.begin(), data.end());
    a.merge(b);
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data.begin(),data.end());
    BOOST_CHECK(b.empty());

    a.merge(b);
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data.begin(),data.end());
    BOOST_CHECK(b.empty());

    boost::array<T,2> data2 = {{ T(2), T(4) }};
    b.insert(b.end(), data2.begin(), data2.end());
    a.merge(b);
    boost::array<T,5> data3 = {{ T(1), T(2), T(3), T(4), T(5) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_CHECK(b.empty());
}

void merge_pred_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(5), T(3), T(1) }};
    boost::array<T,2> data2 = {{ T(4), T(2) }};
    X a(data.begin(), data.end());
    X b(data2.begin(), data2.end());

    a.merge(b, std::greater<T>());
    boost::array<T,5> data3 = {{ T(5), T(4), T(3), T(2), T(1) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_CHECK(b.empty());
}

void sort_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    T data[] = { T(3), T(1), T(4), T(2) };
    X u(data, data+4);

    u.sort();
    std::sort(data, data+4);
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(), u.end(), data, data+4);
}

void sort_pred_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    T data[] = { T(3), T(1), T(4), T(2) };
    X u(data, data+4);

    u.sort(std::greater<T>());
    std::sort(data, data+4, std::greater<T>());
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(), u.end(), data, data+4);
}

void reverse_test()
{
    typedef dummy T;
    typedef hamigaki::xor_list<T> X;
    boost::array<T,3> data = {{ T(1), T(2), T(3) }};
    X u(data.begin(), data.end());

    u.reverse();
    boost::array<T,3> data2 = {{ T(3), T(2), T(1) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(u.begin(),u.end(),data2.begin(),data2.end());
}

void fake_pointer_test()
{
    typedef dummy T;
    typedef fake_allocator<dummy> A;
    typedef hamigaki::xor_list<T,A> X;

    X a;
    a.push_back(T(1));

    X b;
    b.push_back(T(2));

    a.swap(b);
    a.merge(b);
    a.reverse();
    a.sort();
    b.splice(b.begin(), a);
}

void not_equal_swap_test()
{
    typedef dummy T;
    typedef own_allocator<dummy> A;
    typedef hamigaki::xor_list<T,A> X;

    boost::array<dummy,1> data = {{ T(1) }};
    X a(data.begin(), data.end());

    boost::array<dummy,2> data2 = {{ T(2), T(3) }};
    X b(data2.begin(), data2.end());

    BOOST_CHECK(a.get_allocator() != b.get_allocator());
    a.swap(b);

    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data2.begin(),data2.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(b.begin(),b.end(),data.begin(),data.end());
}

void not_equal_splice_all_test()
{
    typedef dummy T;
    typedef own_allocator<dummy> A;
    typedef hamigaki::xor_list<T,A> X;

    boost::array<dummy,1> data = {{ T(1) }};
    X a(data.begin(), data.end());

    boost::array<dummy,2> data2 = {{ T(2), T(3) }};
    X b(data2.begin(), data2.end());

    BOOST_CHECK(a.get_allocator() != b.get_allocator());
    a.splice(a.end(), b);

    boost::array<dummy,3> data3 = {{ T(1), T(2), T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_CHECK(b.empty());
}

void not_equal_splice_test()
{
    typedef dummy T;
    typedef own_allocator<dummy> A;
    typedef hamigaki::xor_list<T,A> X;

    boost::array<dummy,1> data = {{ T(1) }};
    X a(data.begin(), data.end());

    boost::array<dummy,2> data2 = {{ T(2), T(3) }};
    X b(data2.begin(), data2.end());

    BOOST_CHECK(a.get_allocator() != b.get_allocator());
    a.splice(a.end(), b, b.begin());

    boost::array<dummy,2> data3 = {{ T(1), T(2) }};
    boost::array<dummy,1> data4 = {{ T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(b.begin(),b.end(),data4.begin(),data4.end());
}

void not_equal_splice_range_test()
{
    typedef dummy T;
    typedef own_allocator<dummy> A;
    typedef hamigaki::xor_list<T,A> X;

    boost::array<dummy,1> data = {{ T(1) }};
    X a(data.begin(), data.end());

    boost::array<dummy,3> data2 = {{ T(2), T(3), T(4) }};
    X b(data2.begin(), data2.end());

    BOOST_CHECK(a.get_allocator() != b.get_allocator());
    a.splice(a.end(), b, boost::next(b.begin()), b.end());

    boost::array<dummy,3> data3 = {{ T(1), T(3), T(4) }};
    boost::array<dummy,1> data4 = {{ T(2) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(b.begin(),b.end(),data4.begin(),data4.end());
}

void not_equal_merge_test()
{
    typedef dummy T;
    typedef own_allocator<dummy> A;
    typedef hamigaki::xor_list<T,A> X;

    boost::array<dummy,1> data = {{ T(2) }};
    X a(data.begin(), data.end());

    boost::array<dummy,2> data2 = {{ T(1), T(3) }};
    X b(data2.begin(), data2.end());

    BOOST_CHECK(a.get_allocator() != b.get_allocator());
    a.merge(b);

    boost::array<dummy,3> data3 = {{ T(1), T(2), T(3) }};
    BOOST_CHECK_EQUAL_COLLECTIONS(a.begin(),a.end(),data3.begin(),data3.end());
    BOOST_CHECK(b.empty());
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("xor list test");
    test->add(BOOST_TEST_CASE(&type_test));
    test->add(BOOST_TEST_CASE(&construct_test));
    test->add(BOOST_TEST_CASE(&construct_n_test));
    test->add(BOOST_TEST_CASE(&construct_range_test));
    test->add(BOOST_TEST_CASE(&construct_fake_range_test));
    test->add(BOOST_TEST_CASE(&copy_construct_test));
    test->add(BOOST_TEST_CASE(&copy_assign_test));
    test->add(BOOST_TEST_CASE(&assign_range_test));
    test->add(BOOST_TEST_CASE(&assign_fake_range_test));
    test->add(BOOST_TEST_CASE(&assign_n_test));
    test->add(BOOST_TEST_CASE(&get_allocator_test));
    test->add(BOOST_TEST_CASE(&begin_test));
    test->add(BOOST_TEST_CASE(&end_test));
    test->add(BOOST_TEST_CASE(&rbegin_test));
    test->add(BOOST_TEST_CASE(&rend_test));
    test->add(BOOST_TEST_CASE(&empty_test));
    test->add(BOOST_TEST_CASE(&size_test));
    test->add(BOOST_TEST_CASE(&max_size_test));
    test->add(BOOST_TEST_CASE(&resize_test));
    test->add(BOOST_TEST_CASE(&front_test));
    test->add(BOOST_TEST_CASE(&back_test));
    test->add(BOOST_TEST_CASE(&push_front_test));
    test->add(BOOST_TEST_CASE(&pop_front_test));
    test->add(BOOST_TEST_CASE(&push_back_test));
    test->add(BOOST_TEST_CASE(&pop_back_test));
    test->add(BOOST_TEST_CASE(&insert_test));
    test->add(BOOST_TEST_CASE(&insert_n_test));
    test->add(BOOST_TEST_CASE(&insert_range_test));
    test->add(BOOST_TEST_CASE(&insert_fake_range_test));
    test->add(BOOST_TEST_CASE(&erase_test));
    test->add(BOOST_TEST_CASE(&erase_range_test));
    test->add(BOOST_TEST_CASE(&swap_test));
    test->add(BOOST_TEST_CASE(&clear_test));
    test->add(BOOST_TEST_CASE(&splice_all_test));
    test->add(BOOST_TEST_CASE(&splice_test));
    test->add(BOOST_TEST_CASE(&splice_range_test));
    test->add(BOOST_TEST_CASE(&remove_test));
    test->add(BOOST_TEST_CASE(&remove_if_test));
    test->add(BOOST_TEST_CASE(&unique_test));
    test->add(BOOST_TEST_CASE(&unique_pred_test));
    test->add(BOOST_TEST_CASE(&merge_test));
    test->add(BOOST_TEST_CASE(&merge_pred_test));
    test->add(BOOST_TEST_CASE(&sort_test));
    test->add(BOOST_TEST_CASE(&sort_pred_test));
    test->add(BOOST_TEST_CASE(&reverse_test));
    test->add(BOOST_TEST_CASE(&fake_pointer_test));
    test->add(BOOST_TEST_CASE(&not_equal_swap_test));
    test->add(BOOST_TEST_CASE(&not_equal_splice_all_test));
    test->add(BOOST_TEST_CASE(&not_equal_splice_test));
    test->add(BOOST_TEST_CASE(&not_equal_splice_range_test));
    return test;
}
