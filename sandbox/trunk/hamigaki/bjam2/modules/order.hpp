// order.hpp: bjam order module

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_MODULES_ORDER_HPP
#define HAMIGAKI_BJAM2_MODULES_ORDER_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <hamigaki/bjam2/util/list.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

class context;

namespace modules
{

namespace order
{

HAMIGAKI_BJAM2_DECL string_list add_pair(context& ctx);
HAMIGAKI_BJAM2_DECL string_list order(context& ctx);

} // namespace order

HAMIGAKI_BJAM2_DECL void set_order_rules(context& ctx);

} // namespace modules

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_MODULES_ORDER_HPP
