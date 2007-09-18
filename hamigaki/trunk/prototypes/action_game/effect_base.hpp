// effect_base.hpp: base definitions for the effects

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef EFFECT_BASE_HPP
#define EFFECT_BASE_HPP

#include <hamigaki/coroutine/shared_coroutine.hpp>

typedef hamigaki::coroutines::shared_coroutine<
    unsigned long()
> effect_type;

#endif // EFFECT_BASE_HPP
