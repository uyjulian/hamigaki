// bjam_interpreter.hpp: bjam interpreter

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_BJAM_INTERPRETER_HPP
#define HAMIGAKI_BJAM2_BJAM_INTERPRETER_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <hamigaki/bjam2/util/list.hpp>
#include <hamigaki/bjam2/util/node_val_data.hpp>
#include <hamigaki/bjam2/bjam_context.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

HAMIGAKI_BJAM2_DECL
string_list evaluate_expression(context& ctx, const tree_node& tree);

HAMIGAKI_BJAM2_DECL
string_list evaluate_bjam(context& ctx, const tree_node& tree);

template<class IteratorT>
inline string_list evaluate_bjam(
    context& ctx, tree_parse_info<IteratorT>& info)
{
    if (!info.trees.empty())
    {
        return hamigaki::bjam2::evaluate_bjam(
            ctx,
            ctx.push_parse_tree(info.trees.front())
        );
    }
    else
        return string_list();
}

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_BJAM_INTERPRETER_HPP
