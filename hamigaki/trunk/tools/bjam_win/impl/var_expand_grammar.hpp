//  var_expand_grammar.hpp: bjam variable expansion grammar

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_VAR_EXPAND_GRAMMAR_HPP
#define IMPL_VAR_EXPAND_GRAMMAR_HPP

#include "./variables.hpp"
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/spirit/phoenix/binders.hpp>

struct vars_closure
    : boost::spirit::closure<
        vars_closure,
        variables::mapped_type
    >
{
    member1 val;
};

class vars_expand_actor
{
public:
    typedef std::vector<std::string> result_type;

    explicit vars_expand_actor(const variables& t)
        : table_(t)
    {
    }

    std::vector<std::string>
    operator()(const std::vector<std::string>& vs) const
    {
        std::vector<std::string> result;
        for (std::size_t i = 0; i < vs.size(); ++i)
        {
            const std::vector<std::string>* p = table_.find(vs[i]);
            if (p)
                result.insert(result.end(), p->begin(), p->end());
        }
        return result;
    }

private:
    const variables& table_;
};

struct make_str_vec_actor
{
    typedef std::vector<std::string> result_type;

    std::vector<std::string>
    operator()(const char* first, const char* last) const
    {
        std::vector<std::string> tmp;
        tmp.push_back(std::string(first, last));
        return tmp;
    }
};

struct append_str_vec_actor
{
    typedef std::vector<std::string> result_type;

    std::vector<std::string>
    operator()(
        const std::vector<std::string>& lhs,
        const std::vector<std::string>& rhs) const
    {
        std::vector<std::string> result;
        for (std::size_t i = 0; i < lhs.size(); ++i)
        {
            for (std::size_t j = 0; j < rhs.size(); ++j)
                result.push_back(lhs[i] + rhs[j]);
        }
        return result;
    }
};

struct var_expand_grammar
    : boost::spirit::grammar<var_expand_grammar,vars_closure::context_t>
{
    explicit var_expand_grammar(const variables& t)
        : table(t)
    {
    }

    const variables& table;

    template<class ScannerT>
    struct definition
    {
        typedef boost::spirit::rule<ScannerT> rule_t;

        typedef boost::spirit::rule<
            ScannerT, typename vars_closure::context_t> vars_rule_t;

        vars_rule_t top, expr, variable;

        definition(const var_expand_grammar& self)
        {
            using namespace boost::spirit;
            using namespace phoenix;

            variable
                =   "$("
                    >> expr
                        [
                            variable.val =
                                bind(vars_expand_actor(self.table))(arg1)
                        ]
                    >> ')'
                ;

            expr
                =   eps_p[expr.val = bind(make_str_vec_actor())(arg1, arg2)]
                    >>
                   *(   variable
                        [
                            expr.val =
                                bind(append_str_vec_actor())(expr.val, arg1)
                        ]
                    |   (+(anychar_p - '$' - ')'))
                        [
                            expr.val =
                                bind(append_str_vec_actor())(
                                    expr.val, 
                                    bind(make_str_vec_actor())(arg1, arg2)
                                )
                        ]
                    )
                ;

            top
                =   expr[ self.val = arg1 ]
                ;
        }

        const vars_rule_t& start() const
        {
            return top;
        }
    };
};

#endif // IMPL_VAR_EXPAND_GRAMMAR_HPP
