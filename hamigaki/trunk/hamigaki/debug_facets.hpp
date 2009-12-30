// debug_facets.hpp: some facets for debugging

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/

#ifndef HAMIGAKI_DEBUG_FACETS_HPP
#define HAMIGAKI_DEBUG_FACETS_HPP

#include <boost/config.hpp>
#include <iostream>
#include <locale>

namespace hamigaki {

class debug_num_get : public std::num_get<char>
{
private:
    typedef std::num_get<char> base_type;

public:
    typedef char char_type;
    typedef base_type::iter_type iter_type;

    explicit debug_num_get(size_t refs = 0) : base_type(refs)
    {
    }

protected:
    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, bool& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, long& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

#if defined(BOOST_HAS_LONG_LONG)
    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, long long& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }
#endif

    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, unsigned short& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, unsigned int& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, unsigned long& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

#if defined(BOOST_HAS_LONG_LONG)
    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, unsigned long long& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }
#endif

    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, float& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, double& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, long double& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

    iter_type do_get(iter_type in, iter_type end, std::ios_base& f,
        std::ios_base::iostate& err, void*& v) const
    {
        print_warning();
        return base_type::do_get(in, end, f, err, v);
    }

private:
    static void print_warning()
    {
        std::cerr << "(debug_num_get)" << std::endl;
    }
};

class debug_num_put : public std::num_put<char>
{
private:
    typedef std::num_put<char> base_type;

public:
    typedef char char_type;
    typedef base_type::iter_type iter_type;

    explicit debug_num_put(std::size_t ref=0) : base_type(ref)
    {
    }

protected:
    iter_type do_put(
        iter_type s, std::ios_base& f, char_type fill, bool v) const
    {
        return put_message(base_type::do_put(s, f, fill, v));
    }

    iter_type do_put(
        iter_type s, std::ios_base& f, char_type fill, long v) const
    {
        return put_message(base_type::do_put(s, f, fill, v));
    }

    iter_type do_put(
        iter_type s, std::ios_base& f, char_type fill, unsigned long v) const
    {
        return put_message(base_type::do_put(s, f, fill, v));
    }

#if defined(BOOST_HAS_LONG_LONG)
    iter_type do_put(
        iter_type s, std::ios_base& f, char_type fill, long long v) const
    {
        return put_message(base_type::do_put(s, f, fill, v));
    }

    iter_type do_put(
        iter_type s, std::ios_base& f,
        char_type fill, unsigned long long v) const
    {
        return put_message(base_type::do_put(s, f, fill, v));
    }
#endif

    iter_type do_put(
        iter_type s, std::ios_base& f, char_type fill, double v) const
    {
        return put_message(base_type::do_put(s, f, fill, v));
    }

    iter_type do_put(
        iter_type s, std::ios_base& f, char_type fill, long double v) const
    {
        return put_message(base_type::do_put(s, f, fill, v));
    }

    iter_type do_put(
        iter_type s, std::ios_base& f, char_type fill, const void* v) const
    {
        return put_message(base_type::do_put(s, f, fill, v));
    }

private:
    static iter_type put_message(iter_type s)
    {
        const char msg[] = "(debug_num_put)";
        return std::copy(msg, msg+sizeof(msg)-1, s);
    }
};

class debug_numpunct : public std::numpunct<char>
{
private:
    typedef std::numpunct<char> base_type;

public:
    typedef char char_type;
    typedef base_type::string_type string_type;

    explicit debug_numpunct(std::size_t ref=0) : base_type(ref)
    {
    }

protected:
    char_type do_decimal_point() const // virtual
    {
        return ':';
    }

    char_type do_thousands_sep() const // virtual
    {
        return ';';
    }

    std::string do_grouping() const // virtual
    {
        return "\1";
    }

    string_type truename() const // virtual
    {
        return "true(debug_numpunct)";
    }

    string_type falsename() const // virtual
    {
        return "false(debug_numpunct)";
    }
};

} // End namespace hamigaki.

#endif // HAMIGAKI_DEBUG_FACETS_HPP
