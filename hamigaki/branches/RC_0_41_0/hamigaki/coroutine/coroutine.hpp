// coroutine.hpp: a simple coroutine class

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

// Original Copyright
// ===========================================================================>
//  Copyright (c) 2006, Giovanni P. Deretta
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
// <===========================================================================

#ifndef HAMIGAKI_COROUTINE_COROUTINE_HPP
#define HAMIGAKI_COROUTINE_COROUTINE_HPP

// Note:
// The next line is a dummy for bjam "c-scanner".
// #include <hamigaki/coroutine/detail/coroutine_template.hpp>

#include <hamigaki/coroutine/detail/coroutine_utility.hpp>
#include <hamigaki/coroutine/detail/default_context.hpp>
#include <hamigaki/coroutine/exception.hpp>
#include <boost/mpl/if.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/blank.hpp>
#include <boost/function.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <new>

namespace hamigaki { namespace coroutines {

template<class Signature, class ContextImpl=detail::default_context_impl>
class coroutine;

template<class Signature, class ContextImpl=detail::default_context_impl>
class shared_coroutine;

} } // End namespaces coroutines, hamigaki.

#define BOOST_PP_ITERATION_PARAMS_1 \
    (3, \
    (0,BOOST_PP_DEC(BOOST_FUNCTION_MAX_ARGS), \
    <hamigaki/coroutine/detail/coroutine_template.hpp>) \
)

#include BOOST_PP_ITERATE()
#undef BOOST_PP_ITERATION_PARAMS_1

#endif // HAMIGAKI_COROUTINE_COROUTINE_HPP
