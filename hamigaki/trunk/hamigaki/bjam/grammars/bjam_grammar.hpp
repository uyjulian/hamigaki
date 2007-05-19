// bjam_grammar.hpp: bjam grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP

#include <hamigaki/bjam/util/argument_parser.hpp>
#include <hamigaki/bjam/util/keyword_parser.hpp>
#include <hamigaki/bjam/util/string_parser.hpp>

namespace hamigaki { namespace bjam {

struct bjam_grammar : boost::spirit::grammar<bjam_grammar>
{
    template<class ScannerT>
    struct definition
    {
        typedef boost::spirit::rule<ScannerT> rule_t;

        rule_t run;
        rule_t block, rules, assign_list, arglist, rule;
        rule_t assign;
        rule_t expr, and_expr, eq_expr, rel_expr, not_expr, prim_expr;
        rule_t cases, case_;
        rule_t lol, list, non_punct, arg, func;
        rule_t eflags, eflag, bindlist;

        definition(const bjam_grammar& self)
        {
            using namespace boost::spirit;

            run
                =   !rules >> end_p
                ;

            block
                =   !rules
                ;

            rules
                =   rule
                    >> !rules
                |   keyword_p("local")
                    >> !list
                    >> !assign_list
                    >> keyword_p(";")
                    >> block
                ;

            assign_list
                =   keyword_p("=") >> !list
                ;

            arglist
                =   keyword_p("(") >> !lol >> keyword_p(")")
                ;

            rule
                =   keyword_p("{") >> block >> keyword_p("}")
                |   keyword_p("include") >> !list >> keyword_p(";")
                |   arg_p >> !lol >> keyword_p(";")
                |   arg >> assign >> !list >> keyword_p(";")
                |   arg >> keyword_p("on") >> !list
                    >> assign >> !list >> keyword_p(";")
                |   keyword_p("return") >> !list >> keyword_p(";")
                |   keyword_p("for") >> !keyword_p("local") >> arg_p
                    >> keyword_p("in") >> !list
                    >> keyword_p("{") >> block >> keyword_p("}")
                |   keyword_p("switch") >> !list
                    >> keyword_p("{") >> cases >> keyword_p("}")
                |   keyword_p("module") >> !list
                    >> keyword_p("{") >> block >> keyword_p("}") 
                |   keyword_p("class") >> !lol
                    >> keyword_p("{") >> block >> keyword_p("}") 
                |   keyword_p("while") >> expr
                    >> keyword_p("{") >> block >> keyword_p("}") 
                |   keyword_p("if") >> expr
                    >> keyword_p("{") >> block >> keyword_p("}")
                    >> !(keyword_p("else") >> rule)
                |   !keyword_p("local") >> keyword_p("rule") >> arg_p
                    >> !arglist >> rule
                |   keyword_p("on") >> arg >> rule
                |   keyword_p("actions") >> eflags >> arg_p >> !bindlist
                    >> eps_p(keyword_p("{"))
                    >> lexeme_d
                    [
                        '{' >> string_p
                    ]
                    >> keyword_p("}")
                ;

            assign
                =   keyword_p("=")
                |   keyword_p("+=")
                |   keyword_p("?=")
                |   keyword_p("default") >> keyword_p("=")
                ;

            expr
                =   and_expr
                    >> *(   keyword_p("|")  >> and_expr
                        |   keyword_p("||") >> and_expr
                        )
                ;

            and_expr
                =   eq_expr
                    >> *(   keyword_p("&")   >> eq_expr
                        |   keyword_p("&&")  >> eq_expr
                        )
                ;

            eq_expr
                =   rel_expr
                    >> *(   keyword_p("=")   >> rel_expr
                        |   keyword_p("!=")  >> rel_expr
                        )
                ;

            rel_expr
                =   not_expr
                    >> *(   keyword_p("<")  >> not_expr
                        |   keyword_p("<=") >> not_expr
                        |   keyword_p(">")  >> not_expr
                        |   keyword_p(">=") >> not_expr
                        )
                ;

            not_expr
                =   !keyword_p("!") >> prim_expr
                ;

            prim_expr
                =   arg >> !(keyword_p("in")  >> !list)
                |   keyword_p("(") >> expr >> keyword_p(")")
                ;

            cases
                =   *case_
                ;

            case_
                =   keyword_p("case") >> arg_p >> keyword_p(":") >> block
                ;

            lol
                =   list || (keyword_p(":") >> !lol)
                ;

            list
                = +non_punct
                ;

            non_punct
                =   non_punct_p
                |   keyword_p("[") >> func >> keyword_p("]")
                ;

            arg
                =   arg_p
                |   keyword_p("[") >> func >> keyword_p("]")
                ;

            func
                =   arg >> !lol
                |   keyword_p("on") >> arg >> arg >> !lol
                |   keyword_p("on") >> arg >> keyword_p("return") >> list
                ;

            eflags
                = *eflag;

            eflag
                =   keyword_p("updated")
                |   keyword_p("together")
                |   keyword_p("ignore")
                |   keyword_p("quietly")
                |   keyword_p("piecemeal")
                |   keyword_p("existing")
                ;

            bindlist
                =   keyword_p("bind") >> list
                ;

            BOOST_SPIRIT_DEBUG_RULE(run);
            BOOST_SPIRIT_DEBUG_RULE(block);
            BOOST_SPIRIT_DEBUG_RULE(rules);
            BOOST_SPIRIT_DEBUG_RULE(assign_list);
            BOOST_SPIRIT_DEBUG_RULE(arglist);
            BOOST_SPIRIT_DEBUG_RULE(rule);
            BOOST_SPIRIT_DEBUG_RULE(assign);
            BOOST_SPIRIT_DEBUG_RULE(expr);
            BOOST_SPIRIT_DEBUG_RULE(and_expr);
            BOOST_SPIRIT_DEBUG_RULE(eq_expr);
            BOOST_SPIRIT_DEBUG_RULE(rel_expr);
            BOOST_SPIRIT_DEBUG_RULE(not_expr);
            BOOST_SPIRIT_DEBUG_RULE(prim_expr);
            BOOST_SPIRIT_DEBUG_RULE(cases);
            BOOST_SPIRIT_DEBUG_RULE(case_);
            BOOST_SPIRIT_DEBUG_RULE(lol);
            BOOST_SPIRIT_DEBUG_RULE(list);
            BOOST_SPIRIT_DEBUG_RULE(non_punct);
            BOOST_SPIRIT_DEBUG_RULE(arg);
            BOOST_SPIRIT_DEBUG_RULE(func);
            BOOST_SPIRIT_DEBUG_RULE(eflags);
            BOOST_SPIRIT_DEBUG_RULE(eflag);
            BOOST_SPIRIT_DEBUG_RULE(bindlist);
        }

        const rule_t& start() const { return run; }
    };
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP
