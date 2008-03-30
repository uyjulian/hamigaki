// eval_in_module.hpp: the directive to evaluate in the specified module

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_EVAL_IN_MODULE_HPP
#define HAMIGAKI_BJAM_UTIL_EVAL_IN_MODULE_HPP

#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/spirit/core.hpp>

namespace hamigaki { namespace bjam {

template<class FunctorT, class ParserT>
struct eval_in_module_parser
    : public boost::spirit::unary<
        ParserT,
        boost::spirit::parser<
            eval_in_module_parser<FunctorT, ParserT>
        >
    >
{
    typedef eval_in_module_parser<FunctorT, ParserT> self_t;

    typedef boost::spirit::unary<
        ParserT,
        boost::spirit::parser<
            eval_in_module_parser<FunctorT, ParserT>
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

    eval_in_module_parser(
        bjam::context& ctx, const FunctorT& func, const parser_t& p
    )
        : base_t(p), context(ctx), functor(func)
    {
    }

    template <typename ScannerT>
    typename boost::spirit::parser_result<self_t,ScannerT>::type
    parse(const ScannerT& scan) const
    {
        scoped_change_module guard(context, (functor()));
        return this->subject().parse(scan);
    }

    bjam::context& context;
    FunctorT functor;
};

template<class FunctorT>
struct eval_in_module_parser_gen
{
    eval_in_module_parser_gen(bjam::context& ctx, const FunctorT& func)
        : context(ctx), functor(func)
    {
    }

    template<class ParserT>
    eval_in_module_parser<
        FunctorT,
        typename boost::spirit::as_parser<ParserT>::type
    >
    operator[](const ParserT& p) const
    {
        typedef ::boost::spirit::as_parser<ParserT> as_parser_t;
        typedef typename as_parser_t::type parser_t;

        return
            eval_in_module_parser<FunctorT,parser_t>
                (context, functor, as_parser_t::convert(p));
    }

    bjam::context& context;
    FunctorT functor;
};

template<class FunctorT>
eval_in_module_parser_gen<FunctorT>
eval_in_module_d(bjam::context& ctx, const FunctorT& func)
{
    return eval_in_module_parser_gen<FunctorT>(ctx, func);
}

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_EVAL_IN_MODULE_HPP
