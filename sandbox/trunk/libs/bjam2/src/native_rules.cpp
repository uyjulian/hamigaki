// native_rules.cpp: bjam native rules

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM2_SOURCE
#include <hamigaki/bjam2/bjam_context.hpp>
#include <hamigaki/bjam2/native_rules.hpp>

namespace hamigaki { namespace bjam2 {

HAMIGAKI_BJAM2_DECL void set_native_rules(context& ctx)
{
    modules::set_order_rules(ctx);
    modules::set_path_rules(ctx);
    modules::set_property_set_rules(ctx);
    modules::set_regex_rules(ctx);
    modules::set_sequence_rules(ctx);
    modules::set_set_rules(ctx);
}

} } // End namespaces bjam2, hamigaki.
