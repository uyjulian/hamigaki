// bjam_grammar_gen.hpp: bjam grammar generator

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_GEN_HPP
#define HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_GEN_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <hamigaki/bjam2/util/list.hpp>
#include <boost/spirit/tree/common.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

class context;

template<class IteratorT>
struct HAMIGAKI_BJAM2_DECL bjam_grammar_gen
{
    typedef IteratorT iterator_type;

    static boost::spirit::tree_parse_info<IteratorT>
    parse_bjam_grammar(
        const iterator_type& first, const iterator_type& last
    );
};

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_GEN_HPP
