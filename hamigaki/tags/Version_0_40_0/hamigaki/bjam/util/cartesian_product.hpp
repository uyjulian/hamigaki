// cartesian_product.hpp: calculate cartesian product

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_CARTESIAN_PRODUCT_HPP
#define HAMIGAKI_BJAM_UTIL_CARTESIAN_PRODUCT_HPP

#include <hamigaki/bjam/util/list.hpp>
#include <boost/functional.hpp>
#include <algorithm>
#include <functional>
#include <iterator>

namespace hamigaki { namespace bjam {

template<
    class InputIterator, class ForwardIterator,
    class OutputIterator, class BinaryOperation
>
inline OutputIterator cartesian_product(
    InputIterator first1, InputIterator last1,
    ForwardIterator first2, ForwardIterator last2,
    OutputIterator result, BinaryOperation binary_op)
{
    while (first1 != last1)
    {
        result =
            std::transform(
                first2, last2, result,
                boost::bind1st(binary_op, *(first1++))
            );
    }
    return result;
}

inline void append_production(
    string_list& out, const string_list& lhs, const string_list& rhs)
{
    bjam::cartesian_product(
        lhs.begin(), lhs.end(),
        rhs.begin(), rhs.end(),
        std::back_inserter(out),
        (std::plus<std::string>())
    );
}

inline void append_production(
    string_list& out, const std::string& lhs, const string_list& rhs)
{
    std::transform(
        rhs.begin(), rhs.end(),
        std::back_inserter(out),
        boost::bind1st(std::plus<std::string>(), lhs)
    );
}

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_CARTESIAN_PRODUCT_HPP
