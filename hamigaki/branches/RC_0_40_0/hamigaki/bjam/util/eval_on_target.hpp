// eval_on_target.hpp: the directive to evaluate on the specified target

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_EVAL_ON_TARGET_HPP
#define HAMIGAKI_BJAM_UTIL_EVAL_ON_TARGET_HPP

#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/spirit/core.hpp>

namespace hamigaki { namespace bjam {

template<class FunctorT, class ParserT>
struct eval_on_target_parser
    : public boost::spirit::unary<
        ParserT,
        boost::spirit::parser<
            eval_on_target_parser<FunctorT, ParserT>
        >
    >
{
    typedef eval_on_target_parser<FunctorT, ParserT> self_t;

    typedef boost::spirit::unary<
        ParserT,
        boost::spirit::parser<
            eval_on_target_parser<FunctorT, ParserT>
        >
    > base_t;

    typedef ParserT parser_t;

    template<class ScannerT>
    struct result
    {
        typedef typename boost::spirit::parser_result<
            parser_t,ScannerT
        >::type type;
    };

    eval_on_target_parser(
        bjam::context& ctx, const FunctorT& func, const parser_t& p
    )
        : base_t(p), context(ctx), functor(func)
    {
    }

    template <typename ScannerT>
    typename boost::spirit::parser_result<self_t,ScannerT>::type
    parse(const ScannerT& scan) const
    {
        scoped_on_target guard(context, (functor()));
        return this->subject().parse(scan);
    }

    bjam::context& context;
    FunctorT functor;
};

template<class FunctorT>
struct eval_on_target_parser_gen
{
    eval_on_target_parser_gen(bjam::context& ctx, const FunctorT& func)
        : context(ctx), functor(func)
    {
    }

    template<class ParserT>
    eval_on_target_parser<
        FunctorT,
        typename boost::spirit::as_parser<ParserT>::type
    >
    operator[](const ParserT& p) const
    {
        typedef ::boost::spirit::as_parser<ParserT> as_parser_t;
        typedef typename as_parser_t::type parser_t;

        return
            eval_on_target_parser<FunctorT,parser_t>
                (context, functor, as_parser_t::convert(p));
    }

    bjam::context& context;
    FunctorT functor;
};

template<class FunctorT>
eval_on_target_parser_gen<FunctorT>
eval_on_target_d(bjam::context& ctx, const FunctorT& func)
{
    return eval_on_target_parser_gen<FunctorT>(ctx, func);
}

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_EVAL_ON_TARGET_HPP
