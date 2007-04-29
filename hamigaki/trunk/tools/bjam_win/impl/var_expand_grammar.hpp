//  var_expand_grammar.hpp: bjam variable expansion grammar

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_VAR_EXPAND_GRAMMAR_HPP
#define IMPL_VAR_EXPAND_GRAMMAR_HPP

#include "./variables.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/spirit/phoenix/binders.hpp>
#include <boost/optional.hpp>
#include <algorithm>

struct vars_closure
    : boost::spirit::closure<
        vars_closure,
        variables::mapped_type
    >
{
    member1 val;
};

struct range_closure
    : boost::spirit::closure<
        range_closure,
        std::pair<unsigned,unsigned>,
        unsigned,
        unsigned
    >
{
    member1 val;
    member2 lower;
    member3 upper;
};

struct expand_closure
    : boost::spirit::closure<
        expand_closure,
        variables::mapped_type,
        boost::optional<std::string>
    >
{
    member1 val;
    member2 str;
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

struct slice_actor
{
    typedef std::vector<std::string> result_type;

    std::vector<std::string>
    operator()(
        const std::vector<std::string>& vs,
        const std::pair<unsigned,unsigned>& r) const
    {
        unsigned n = r.first;
        unsigned m = r.second;

        if (m == 0)
            m = static_cast<unsigned>(vs.size());

        std::vector<std::string> result;
        for (unsigned i = n; i <= m; ++i)
        {
            if (i <= static_cast<unsigned>(vs.size()))
                result.push_back(vs[i-1]);
            else
                result.push_back(std::string());
        }
        return result;
    }
};

struct parent_dir_actor
{
    typedef void result_type;

    void operator()(std::vector<std::string>& vs) const
    {
        for (std::size_t i = 0; i < vs.size(); ++i)
        {
            const std::string& s = vs[i];

            std::string::size_type sl = s.rfind('/');
            std::string::size_type bs = s.rfind('\\');

            const std::string::size_type npos = std::string::npos;
            if (sl == npos)
                sl = 0;
            if (bs == npos)
                bs = 0;

            vs[i] = s.substr(0, (std::max)(sl, bs));
        }
    }
};

struct upper_actor
{
    typedef void result_type;

    void operator()(std::vector<std::string>& vs) const
    {
        for (std::size_t i = 0; i < vs.size(); ++i)
            boost::algorithm::to_upper(vs[i]);
    }
};

struct lower_actor
{
    typedef void result_type;

    void operator()(std::vector<std::string>& vs) const
    {
        for (std::size_t i = 0; i < vs.size(); ++i)
            boost::algorithm::to_lower(vs[i]);
    }
};

struct grist_actor
{
    typedef void result_type;

    static std::string::size_type find_grist_end(const std::string& s)
    {
        if (s.empty() || (s[0] != '<'))
            return 0;
        else
        {
            std::string::size_type pos = s.find('>');
            if (pos == std::string::npos)
                return 0;
            else
                return pos+1;
        }
    }

    void operator()(
        std::vector<std::string>& vs,
        const boost::optional<std::string>& val) const
    {
        if (val)
        {
            std::string grist;
            grist += '<';
            grist += *val;
            grist += '>';

            for (std::size_t i = 0; i < vs.size(); ++i)
            {
                std::string& s = vs[i];
                s.replace(0, grist_actor::find_grist_end(s), grist);
            }
        }
        else
        {
            for (std::size_t i = 0; i < vs.size(); ++i)
            {
                const std::string& s = vs[i];
                vs[i] = s.substr(0, grist_actor::find_grist_end(s));
            }
        }
    }
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

        typedef boost::spirit::rule<
            ScannerT, typename range_closure::context_t> range_rule_t;

        typedef boost::spirit::rule<
            ScannerT, typename expand_closure::context_t> expand_rule_t;

        vars_rule_t top, expr;
        range_rule_t index_range;
        expand_rule_t variable;

        definition(const var_expand_grammar& self)
        {
            using namespace boost::spirit;
            using namespace phoenix;

            index_range
                =   '['
                    >> uint_p
                    [
                        index_range.lower = index_range.upper = arg1
                    ]
                    >> !(
                        ch_p('-')[index_range.upper = 0u]
                        >> !uint_p[index_range.upper = arg1]
                        )
                    >> ch_p(']')
                        [
                            index_range.val =
                                construct_<std::pair<unsigned,unsigned> >(
                                    index_range.lower, index_range.upper
                                )
                        ]
                ;

            variable
                =   "$("
                    >> expr
                        [
                            variable.val =
                                bind(vars_expand_actor(self.table))(arg1)
                        ]
                    >> !index_range
                        [
                            variable.val =
                                bind(slice_actor())(variable.val, arg1)
                        ]
                    >> *(
                            ':'
                            >> +(   ch_p('P')
                                    [
                                        bind(parent_dir_actor())(variable.val)
                                    ]
                                |   ch_p('U')
                                    [
                                        bind(upper_actor())(variable.val)
                                    ]
                                |   ch_p('L')
                                    [
                                        bind(lower_actor())(variable.val)
                                    ]
                                |   ch_p('G')
                                    >> !(   '='
                                            >> (*(anychar_p - ')' - ':'))
                                            [
                                                variable.str =
                                                    construct_<std::string>(
                                                        arg1, arg2
                                                    )
                                            ]
                                        )
                                    >> eps_p
                                    [
                                        bind(grist_actor())
                                            (variable.val, variable.str)
                                    ]
                                |   alpha_p
                                    >> !( '=' >> *(anychar_p - ')' - ':') )
                                )
                        )
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
                    |   (+(anychar_p - '$' - ')' - '[' - ':'))
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
