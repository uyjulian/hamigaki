// order.hpp: bjam order module

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_MODULES_ORDER_HPP
#define HAMIGAKI_BJAM_MODULES_ORDER_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/util/list.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

class context;

namespace modules
{

namespace order
{

HAMIGAKI_BJAM_DECL string_list add_pair(context& ctx);
HAMIGAKI_BJAM_DECL string_list order(context& ctx);

} // namespace order

HAMIGAKI_BJAM_DECL void set_order_rules(context& ctx);

} // namespace modules

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_MODULES_ORDER_HPP
