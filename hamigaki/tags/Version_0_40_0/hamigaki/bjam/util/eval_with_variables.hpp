// eval_with_variables.hpp: the directive to evaluate with the local variables

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_EVAL_WITH_VARIABLES_HPP
#define HAMIGAKI_BJAM_UTIL_EVAL_WITH_VARIABLES_HPP

#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/spirit/core.hpp>

namespace hamigaki { namespace bjam {

template<class NamesFuncT, class ValuesFuncT, class ParserT>
struct eval_with_variables_parser
    : public boost::spirit::unary<
        ParserT,
        boost::spirit::parser<
            eval_with_variables_parser<NamesFuncT, ValuesFuncT, ParserT>
        >
    >
{
    typedef eval_with_variables_parser<NamesFuncT, ValuesFuncT, ParserT> self_t;

    typedef boost::spirit::unary<
        ParserT,
        boost::spirit::parser<
            eval_with_variables_parser<NamesFuncT, ValuesFuncT, ParserT>
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

    eval_with_variables_parser(
        bjam::context& ctx, const NamesFuncT& names, const ValuesFuncT& values,
        const parser_t& p
    )
        : base_t(p), context(ctx), names_(names), values_(values)
    {
    }

    template <typename ScannerT>
    typename boost::spirit::parser_result<self_t,ScannerT>::type
    parse(const ScannerT& scan) const
    {
        const string_list& names = names_();
        const string_list& values = values_();

        variable_table local;
        for (std::size_t i = 0; i < names.size(); ++i)
            local.set_values(names[i], values);

        module& m = context.current_frame().current_module();
        scoped_push_local_variables using_local(m.variables, local);

        return this->subject().parse(scan);
    }

    bjam::context& context;
    NamesFuncT names_;
    ValuesFuncT values_;
};

template<class NamesFuncT, class ValuesFuncT>
struct eval_with_variables_parser_gen
{
    eval_with_variables_parser_gen(
        bjam::context& ctx, const NamesFuncT& names, const ValuesFuncT& values
    )
        : context(ctx), names_(names), values_(values)
    {
    }

    template<class ParserT>
    eval_with_variables_parser<
        NamesFuncT,
        ValuesFuncT,
        typename boost::spirit::as_parser<ParserT>::type
    >
    operator[](const ParserT& p) const
    {
        typedef ::boost::spirit::as_parser<ParserT> as_parser_t;
        typedef typename as_parser_t::type parser_t;

        return
            eval_with_variables_parser<NamesFuncT,ValuesFuncT,parser_t>
                (context, names_, values_, as_parser_t::convert(p));
    }

    bjam::context& context;
    NamesFuncT names_;
    ValuesFuncT values_;
};

template<class NamesFuncT, class ValuesFuncT>
eval_with_variables_parser_gen<NamesFuncT,ValuesFuncT>
eval_with_variables_d(
    bjam::context& ctx, const NamesFuncT& names, const ValuesFuncT& values)
{
    return eval_with_variables_parser_gen<
        NamesFuncT,ValuesFuncT>(ctx, names, values);
}

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_EVAL_WITH_VARIABLES_HPP
