// keyword_parser.hpp: bjam keyword parser

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_KEYWORD_PARSER_HPP
#define HAMIGAKI_BJAM_UTIL_KEYWORD_PARSER_HPP

#include <boost/spirit/core.hpp>

namespace hamigaki { namespace bjam {

template<class IteratorT=const char*>
class keyword : public boost::spirit::parser<keyword<IteratorT> >
{
public:
    typedef keyword<IteratorT> self_t;

    keyword(IteratorT first, IteratorT last) : name_(first, last)
    {
    }

    keyword(IteratorT first) : name_(first)
    {
    }

    template<class ScannerT>
    typename boost::spirit::parser_result<self_t,ScannerT>::type
    parse(const ScannerT& scan) const
    {
        using namespace boost::spirit;

        return
            lexeme_d
            [
                name_ >> (eps_p(space_p|end_p))
            ].parse(scan);
    }

private:
    boost::spirit::chseq<IteratorT> name_;
};

template<class CharT>
inline keyword<const CharT*> keyword_p(const CharT* str)
{ 
    return keyword<const CharT*>(str); 
}

template<class IteratorT>
inline keyword<IteratorT> keyword_p(IteratorT first, IteratorT last)
{ 
    return keyword<IteratorT>(first, last); 
}

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_KEYWORD_PARSER_HPP
