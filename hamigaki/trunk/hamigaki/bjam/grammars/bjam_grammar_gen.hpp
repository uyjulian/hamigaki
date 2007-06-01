// bjam_grammar_gen.hpp: bjam grammar generator

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_GEN_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_GEN_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/spirit/core.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

template<class IteratorT>
struct HAMIGAKI_BJAM_DECL bjam_grammar_gen
{
    typedef IteratorT iterator_type;

    static boost::spirit::parse_info<IteratorT>
    parse_bjam_grammar(
        const iterator_type& first, const iterator_type& last, context& ctx);
};

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_GEN_HPP
