//  bjam_grammar.hpp: bjam grammar

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_BJAM_GRAMMAR_HPP
#define IMPL_BJAM_GRAMMAR_HPP

// TODO
#define PHOENIX_LIMIT 5
#define BOOST_SPIRIT_GRAMMAR_STARTRULE_TYPE_LIMIT 5

#include "./rule_table.hpp"
#include "./var_expand_grammar.hpp"
#include <hamigaki/spirit/phoenix/stl/append.hpp>
#include <hamigaki/spirit/phoenix/stl/empty.hpp>
#include <hamigaki/spirit/phoenix/stl/push_back.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/spirit/dynamic/if.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/utility/chset_operators.hpp>
#include <boost/spirit/utility/grammar_def.hpp>
#include <vector>
#include <string>

inline void remove_branch_path(std::string& s)
{
    std::string::size_type pos = s.rfind('/');
    if (pos != std::string::npos)
        s.erase(0, pos+1);
}

struct has_non_empty_element_impl
{
    typedef bool result_type;

    bool operator()(const std::vector<std::string>& vs) const
    {
        using hamigaki::phoenix::empty;
        using namespace ::phoenix;
        return std::find_if(vs.begin(), vs.end(), !empty(arg1)) != vs.end();
    }
};

const ::phoenix::functor<has_non_empty_element_impl>
has_non_empty_element = has_non_empty_element_impl();

struct includes_impl
{
    typedef bool result_type;

    bool operator()(
        const std::vector<std::string>& big,
        const std::vector<std::string>& small) const
    {
        std::vector<std::string> b(big);
        std::vector<std::string> s(small);

        std::sort(b.begin(), b.end());
        std::sort(s.begin(), s.end());

        return std::includes(b.begin(), b.end(), s.begin(), s.end());
    }
};

const ::phoenix::functor<includes_impl> includes = includes_impl();

class vars_add_actor
{
public:
    typedef void result_type;

    explicit vars_add_actor(variables& t) : table_(t)
    {
    }

    void operator()(const std::vector<std::string>& vs) const
    {
        std::vector<std::string> result;
        for (std::size_t i = 0; i < vs.size(); ++i)
            table_.add(vs[i]);
    }

private:
    variables& table_;
};

struct vars_assign_impl
{
    typedef void result_type;

    void operator()(
        variables& table,
        const std::vector<std::string>& keys,
        const std::vector<std::string>& vals) const
    {
        for (std::size_t i = 0; i < keys.size(); ++i)
            table.assign(keys[i], vals);
    }
};

const ::phoenix::functor<vars_assign_impl> vars_assign = vars_assign_impl();

struct vars_append_impl
{
    typedef void result_type;

    void operator()(
        variables& table,
        const std::vector<std::string>& keys,
        const std::vector<std::string>& vals) const
    {
        for (std::size_t i = 0; i < keys.size(); ++i)
            table.append(keys[i], vals);
    }
};

const ::phoenix::functor<vars_append_impl> vars_append = vars_append_impl();

struct vars_default_impl
{
    typedef void result_type;

    void operator()(
        variables& table,
        const std::vector<std::string>& keys,
        const std::vector<std::string>& vals) const
    {
        for (std::size_t i = 0; i < keys.size(); ++i)
        {
            const std::string& key = keys[i];
            const std::vector<std::string>* p = table.find(key);
            if (!p || p->empty())
                table.assign(key, vals);
        }
    }
};

const ::phoenix::functor<vars_default_impl> vars_default = vars_default_impl();

struct vars_expand_impl
{
    typedef std::vector<std::string> result_type;

    std::vector<std::string>
    operator()(variables& table, const std::string& str) const
    {
        using namespace boost::spirit;
        using namespace phoenix;

        var_expand_grammar g(table);
        std::vector<std::string> val;
        parse_info<const char*> info = parse(str.c_str(), g[var(val) = arg1]);

        // TODO
        if (!info.full)
            throw std::runtime_error("prase error '" + str + "'");

        return val;
    }
};

const ::phoenix::functor<vars_expand_impl> vars_expand = vars_expand_impl();

struct add_rule_impl
{
    typedef void result_type;

    void operator()(
        rule_table& table,
        const std::string& name,
        const std::vector<std::vector<std::string> >& fields,
        const std::string& src) const
    {
        table.add(name, rule_data(fields, src));
    }
};

const ::phoenix::functor<add_rule_impl> add_rule = add_rule_impl();

struct bjam_grammar
    : boost::spirit::grammar<bjam_grammar,vars_closure::context_t>
{
    enum
    {
        jamfile, statements_, invoke, cond0, if_
    };

    bjam_grammar(std::vector<std::string>& vs, variables& vt, rule_table& rs)
        : storage(vs), vars(vt), rules(rs)
    {
    }

    std::vector<std::string>& storage;
    variables& vars;
    rule_table& rules;

    void enter_block(const std::string& src) const
    {
        using namespace boost::spirit;

        variables new_vars(&vars);
        rule_table new_rules(&rules);

        bjam_grammar g(storage, new_vars, new_rules);
        parse_info<const char*> info =
            boost::spirit::parse(
                src.c_str(),
                g.use_parser<bjam_grammar::statements_>()
            );

        // TODO
        if (!info.full)
            throw std::runtime_error("prase error");
    }

    void for_block(
        const std::string& name,
        const std::vector<std::string>& list,
        const std::string& src) const
    {
        using namespace boost::spirit;

        for (std::size_t i = 0; i < list.size(); ++i)
        {
            variables new_vars(&vars);
            rule_table new_rules(&rules);

            new_vars.add_local(name);
            std::vector<std::string> tmp;
            tmp.push_back(list[i]);
            new_vars.assign(name, tmp);

            bjam_grammar g(storage, new_vars, new_rules);
            parse_info<const char*> info =
                boost::spirit::parse(
                    src.c_str(),
                    g.use_parser<bjam_grammar::statements_>()
                );

            // TODO
            if (!info.full)
                throw std::runtime_error("prase error");
        }
    }

    void
    invoke_rule_normal(
        std::vector<std::string>& result,
        const std::string& name,
        const std::vector<std::vector<std::string> >& fields) const
    {
        namespace hp = hamigaki::phoenix;
        using namespace boost::spirit;
        using namespace phoenix;

        const rule_data* data = rules.find(name);
        // TODO
        if (!data)
            return;

        variables new_vars(&vars);
        rule_table new_rules(&rules);

        for (std::size_t i = 0; i < 9; ++i)
        {
            std::string name(1u, "123456789"[i]);
            new_vars.add_local(name);
            if (i < fields.size())
                new_vars.assign(name, fields[i]);
        }

        bjam_grammar g(storage, new_vars, new_rules);
        parse_info<const char*> info =
            boost::spirit::parse(
                data->source.c_str(),
                g.use_parser<bjam_grammar::statements_>()
                [
                    hp::append(var(result), arg1)
                ]
            );

        // TODO
        if (!info.full)
            throw std::runtime_error("prase error");
    }

    void invoke_rule_run(
        const std::vector<std::vector<std::string> >& fields) const
    {
        if (fields.empty() || fields[0].empty())
            return;

        const std::string& src = fields[0][0];

        std::string target;
        if ((fields.size() < 5) || fields[4].empty())
        {
            std::string s = src;
            ::remove_branch_path(s);

            std::string::size_type pos = s.rfind('.');
            if (pos != std::string::npos)
                s.erase(pos);

            target = s;
        }
        else
            target = fields[4][0];

        storage.push_back(target);
    }

    void invoke_rule_test(
        const std::vector<std::vector<std::string> >& fields) const
    {
        if (fields.empty() || fields[0].empty())
            return;

        const std::string& src = fields[0][0];

        std::string target;
        if ((fields.size() < 3) || fields[2].empty())
        {
            std::string s = src;
            ::remove_branch_path(s);

            std::string::size_type pos = s.rfind('.');
            if (pos != std::string::npos)
                s.erase(pos);

            target = s;
        }
        else
            target = fields[2][0];

        storage.push_back(target);
    }

    std::vector<std::string>
    invoke_rule(
        const std::vector<std::string>& rule_name,
        const std::vector<std::vector<std::string> >& fields) const
    {
        namespace hp = hamigaki::phoenix;
        using namespace boost::spirit;
        using namespace phoenix;

        std::vector<std::string> result;

        for (std::size_t i = 0; i < rule_name.size(); ++i)
        {
            const std::string& name = rule_name[i];
            if ((name == "exe") || (name == "lib") || (name == "bpl-test"))
            {
                if (!fields.empty() && !fields[0].empty())
                    storage.push_back(fields[0][0]);
            }
            else if (name == "boostbook")
            {
                storage.push_back("html");
                storage.push_back("onehtml");
                storage.push_back("man");
                storage.push_back("docbook");
                storage.push_back("fo");
                storage.push_back("pdf");
                storage.push_back("ps");
                storage.push_back("tests");
            }
            else if ((name == "run") || (name == "run-fail"))
                this->invoke_rule_run(fields);
            else if (
                (name == "compile") || (name == "compile-fail") ||
                (name == "link") || (name == "link-fail") )
            {
                this->invoke_rule_test(fields);
            }
            else
                this->invoke_rule_normal(result, name, fields);
        }

        return result;
    }

    struct invoke_closure
        : boost::spirit::closure<
              invoke_closure
            , variables::mapped_type
            , variables::mapped_type
            , std::vector<variables::mapped_type>
        >
    {
        member1 val;
        member2 rulename;
        member3 fields;
    };

    struct cond0_closure
        : boost::spirit::closure<
              cond0_closure
            , bool
            , variables::mapped_type
            , variables::mapped_type
        >
    {
        member1 val;
        member2 lhs;
        member3 rhs;
    };

    struct if_closure
        : boost::spirit::closure<
              if_closure
            , int
            , bool
        >
    {
        member2 cond;
    };

    template<class ScannerT>
    struct definition
        : boost::spirit::grammar_def<
              boost::spirit::rule<ScannerT>
            , boost::spirit::rule<ScannerT, typename vars_closure::context_t>
            , boost::spirit::rule<ScannerT, typename invoke_closure::context_t>
            , boost::spirit::rule<ScannerT, typename cond0_closure::context_t>
            , boost::spirit::rule<ScannerT, typename if_closure::context_t>
        >
    {
        typedef boost::spirit::rule<ScannerT> rule_t;

        boost::spirit::symbols<> keyword;

        rule_t comment, space;
        rule_t actions, action_modifiers, action_binds;
        rule_t while_, module, local;
        rule_t include, statement;
        rule_t pattern, switch_;
        rule_t jamfile;

        struct char_closure : boost::spirit::closure<char_closure,char>
        {
            member1 val;
        };

        boost::spirit::rule<
            ScannerT,
            typename char_closure::context_t
        > literal_char, quote_char, pattern_char;

        boost::spirit::rule<
            ScannerT, typename invoke_closure::context_t> invoke;

        boost::spirit::rule<
            ScannerT, typename cond0_closure::context_t> cond0;

        struct bool_closure : boost::spirit::closure<bool_closure,bool>
        {
            member1 val;
        };

        boost::spirit::rule<
            ScannerT, typename bool_closure::context_t> cond;

        boost::spirit::rule<
            ScannerT, typename if_closure::context_t> if_;

        struct str_closure : boost::spirit::closure<str_closure,std::string>
        {
            member1 val;
        };

        boost::spirit::rule<
            ScannerT, typename str_closure::context_t> block, literal;

        struct tmp_vars_closure
            : boost::spirit::closure<
                  tmp_vars_closure
                , int
                , std::vector<std::string>
            >
        {
            member2 tmp;
        };

        boost::spirit::rule<
            ScannerT, typename tmp_vars_closure::context_t> assign_;

        struct rule_closure
            : boost::spirit::closure<
                  rule_closure
                , int
                , std::string
                , std::vector<std::vector<std::string> >
            >
        {
            member2 rulename;
            member3 fields;
        };

        boost::spirit::rule<
            ScannerT, typename rule_closure::context_t> rule_;

        struct for_closure
            : boost::spirit::closure<
                  for_closure
                , int
                , std::string
                , variables::mapped_type
            >
        {
            member2 name;
            member3 list;
        };

        boost::spirit::rule<
            ScannerT, typename for_closure::context_t> for_;

        typedef boost::spirit::rule<
            ScannerT, typename vars_closure::context_t> vars_rule_t;

        vars_rule_t statements_, statements, list, varexp;
        vars_rule_t rule_expansion, return_;

        definition(const bjam_grammar& self)
        {
            namespace hp = hamigaki::phoenix;
            using namespace boost::spirit;
            using namespace phoenix;

            literal_char
                =   anychar_p[literal_char.val = arg1] - space_p - '"' - '\\'
                ;

            quote_char
                =   anychar_p[quote_char.val = arg1] - '"' - '\\'
                ;

            pattern_char
                =   anychar_p[pattern_char.val = arg1]
                    - space_p - '\\' - '[' - ']'
                ;

            keyword
                =   ";"
                ,   ":"
                ,   "{"
                ,   "}"
                ,   "("
                ,   ")"
                ,   "["
                ,   "]"
                ,   "!"
                ,   "="
                ,   "!="
                ,   "<"
                ,   "<="
                ,   ">"
                ,   ">="
                ,   "&&"
                ,   "||"
                ,   "actions"
                ,   "bind"
                ,   "case"
                ,   "else"
                ,   "existing"
                ,   "for"
                ,   "if"
                ,   "ignore"
                ,   "in"
                ,   "include"
                ,   "local"
                ,   "module"
                ,   "on"
                ,   "piecemeal"
                ,   "quietly"
                ,   "return"
                ,   "rule"
                ,   "switch"
                ,   "together"
                ,   "updated"
                ,   "while"
                ;

            comment
                =   '#' >> *(anychar_p - '\n') >> '\n'
                ;

            space
                =   space_p >> *(space_p | comment)
                ;

            literal
                =  +(   literal_char[literal.val += arg1]
                    |   '\\' >> anychar_p[literal.val += arg1]
                    |   '"'
                        >> *(   quote_char[literal.val += arg1]
                            | '\\' >> anychar_p[literal.val += arg1]
                            )
                        >> '"'
                    ) - keyword
                ;

            varexp
                =   rule_expansion
                    [
                        varexp.val = arg1
                    ]
                |   literal
                    [
                        varexp.val = vars_expand(boost::ref(self.vars), arg1)
                    ]
                ;

            action_modifiers
                =  *(
                        space
                        >>
                        (   str_p("existing")
                        |   str_p("ignore")
                        |   str_p("piecemeal")
                        |   str_p("quietly")
                        |   str_p("together")
                        |   str_p("updated")
                        )
                    )
                ;

            action_binds
                =   space
                    >> str_p("bind")
                    >> +(space >> literal)
                ;

            actions
                =   str_p("actions")
                    >> action_modifiers
                    >> space
                    >> varexp
                    >> !action_binds
                    >> space
                    >> '{'
                    >> space_p
                    >> *(~chset<>('}'))
                    >> '}'
                ;

            cond0
                =   '(' >> space >> cond[cond0.val = arg1] >> space >> ')'
                |   '!' >> space >> cond[cond0.val = !arg1]
                |   varexp
                    [
                        cond0.val = has_non_empty_element(cond0.lhs = arg1)
                    ]
                    >>
                   !(   space >> '='  >> space
                        >> varexp
                        [
                            cond0.val = (cond0.lhs == arg1)
                        ]
                    |   space >> "!=" >> space
                        >> varexp
                        [
                            cond0.val = (cond0.lhs != arg1)
                        ]
                    |   space >> '<'  >> space
                        >> varexp
                        [
                            cond0.val = (cond0.lhs < arg1)
                        ]
                    |   space >> "<=" >> space
                        >> varexp
                        [
                            cond0.val = (cond0.lhs <= arg1)
                        ]
                    |   space >> '>'  >> space
                        >> varexp
                        [
                            cond0.val = (cond0.lhs > arg1)
                        ]
                    |   space >> ">=" >> space
                        >> varexp
                        [
                            cond0.val = (cond0.lhs >= arg1)
                        ]
                    |   space >> "in"
                        >>  (
                               *(   space
                                    >> varexp[hp::append(cond0.rhs, arg1)]
                                )
                            )
                            [
                                cond0.val = includes(cond0.rhs, cond0.lhs)
                            ]
                    )
                ;

            cond
                =   cond0[cond.val = arg1]
                    >> *(   space
                            >> "&&"
                            >> space
                            >>  if_p(cond.val)
                                [
                                    cond0[cond.val = arg1]
                                ]
                                .else_p
                                [
                                    no_actions_d
                                    [
                                        self.use_parser<bjam_grammar::cond0>()
                                    ]
                                ]
                        |   space
                            >> "||"
                            >> space
                            >>  if_p(cond.val)
                                [
                                    no_actions_d
                                    [
                                        self.use_parser<bjam_grammar::cond0>()
                                    ]
                                ]
                                .else_p
                                [
                                    cond0[cond.val = arg1]
                                ]
                        )
                ;

            block
                =   '{'
                    >> space
                    >> !(
                            no_actions_d
                            [
                                self.use_parser<bjam_grammar::statements_>()
                                >> eps_p
                            ]
                            [
                                block.val = construct_<std::string>(arg1, arg2)
                            ]
                            >> space
                        )
                    >> '}'
                ;

            if_
                =   "if"
                    >> space
                    >> cond[if_.cond = arg1]
                    >> space
                    >>  if_p(if_.cond)
                        [
                            block
                            [
                                bind(&bjam_grammar::enter_block)
                                    (var(self), arg1)
                            ]
                        ].else_p
                        [
                            block
                        ]
                    >> !(
                            space
                            >> "else"
                            >> space
                            >>  if_p(if_.cond)
                                [
                                    no_actions_d
                                    [
                                        self.use_parser<bjam_grammar::if_>()
                                    ]
                                |   block
                                ].else_p
                                [
                                    if_
                                |   block
                                    [
                                        bind(&bjam_grammar::enter_block)
                                            (var(self), arg1)
                                    ]
                                ]
                        )
                ;

            for_
                =   "for"
                    >> !( space >> str_p("local") )
                    >> space
                    >> literal[for_.name = arg1]
                    >> space
                    >> "in"
                    >> space
                    >> !( list[for_.list = arg1] >> space )
                    >> block
                    [
                        bind(&bjam_grammar::for_block)
                            (var(self), for_.name, for_.list, arg1)
                    ]
                ;

            while_
                =   "while" >> space >> cond >> space
                    >> block
                ;

            rule_
                =   "rule"
                    >> space
                    >> literal[rule_.rulename = arg1]
                    >> space
                    >>  !(  '('
                            >> space
                            >> !(   list
                                    [
                                        hp::push_back(rule_.fields, arg1)
                                    ]
                                    >> space
                                )
                            >> *(   ':'
                                    >> space
                                    >> !(   list
                                            [
                                                hp::push_back(rule_.fields,arg1)
                                            ]
                                            >> space
                                        )
                                )
                            >> ')'
                            >> space
                        )
                    >> block
                    [
                        add_rule(
                            boost::ref(self.rules),
                            rule_.rulename,
                            rule_.fields,
                            arg1
                        )
                    ]
                ;

            module
                =   "module" >> space >> literal >> space
                    >> block
                ;

            local
                =   "local"
                    >> space
                    >>  (   rule_
                        |   +(varexp >> space)
                            >> !( '=' >> *(space >> varexp) >> space)
                            >> ';'
                        )
                ;

            pattern
                =  +(   pattern_char
                    |   '[' >> !ch_p('^') >> +pattern_char >> ']'
                    |   '\\' >> anychar_p
                    )
                ;

            switch_
                =   "switch"
                    >> space
                    >> varexp
                    >> space
                    >> '{'
                    >> *(
                            space
                            >> "case"
                            >> space
                            >> pattern
                            >> space
                            >> ':'
                            >> !(   space
                                    >> no_actions_d
                                    [
                                        self.use_parser<
                                            bjam_grammar::statements_>()
                                    ]
                                )
                        )
                    >> space
                    >> '}'
                ;

            list
                =   varexp[hp::append(list.val, arg1)]
                    >> *(space >> varexp[hp::append(list.val, arg1)])
                ;

            assign_
                =   varexp
                    [
                        bind(vars_add_actor(self.vars))(assign_.tmp = arg1)
                    ]
                    >> !( space >> "on" >> !(space >> list) )
                    >> space
                    >>  (   '='
                            >> !(   space
                                    >> list
                                    [
                                        vars_assign(
                                            boost::ref(self.vars),
                                            assign_.tmp, arg1
                                        )
                                    ]
                                )
                        |   "+="
                            >> !(   space
                                    >> list
                                    [
                                        vars_append(
                                            boost::ref(self.vars),
                                            assign_.tmp, arg1
                                        )
                                    ]
                                )
                        |   "?="
                            >> !(   space
                                    >> list
                                    [
                                        vars_default(
                                            boost::ref(self.vars),
                                            assign_.tmp, arg1
                                        )
                                    ]
                                )
                        )
                ;

            invoke
                =   varexp[invoke.rulename = arg1]
                    >>  (   space
                            >> list[hp::push_back(invoke.fields, arg1)]
                        |   eps_p
                            [
                                hp::push_back(
                                    invoke.fields,
                                    construct_<std::vector<std::string> >()
                                )
                            ]
                        )
                    >> *(   space
                            >> ':'
                            >>  (   space
                                    >> list[hp::push_back(invoke.fields, arg1)]
                                |   eps_p
                                    [
                                        hp::push_back(
                                            invoke.fields,
                                            construct_<
                                                std::vector<std::string> >()
                                        )
                                    ]
                                )
                        )
                    >> eps_p
                    [
                        invoke.val = bind(&bjam_grammar::invoke_rule)(
                            var(self), invoke.rulename, invoke.fields)
                    ]
                ;

            rule_expansion
                =   '['
                    >> space
                    >> invoke[rule_expansion.val = arg1]
                    >> space
                    >> ']'
                ;

            return_
                =   "return" >> !( space >> list[return_.val = arg1] )
                ;

            include
                =   "include" >> space >> varexp
                ;

            statement
                =   (   actions
                    |   block
                        [
                            bind(&bjam_grammar::enter_block)(var(self), arg1)
                        ]
                    |   if_
                    |   for_
                    |   while_
                    |   switch_
                    |   rule_
                    |   module
                    |   local
                    |   include >> space >> ';'
                    |   invoke >> space >> ';'
                    |   assign_ >> space >> ';'
                    )
                ;

            statements
                =   (   statement
                    |   return_[statements.val = arg1] >> space >> ';'
                    )
                    >> *(   space
                            >>  (   statement
                                |   return_[statements.val = arg1]
                                    >> space >> ';'
                                )
                        )
                ;

            statements_
                =   statements[self.val = arg1]
                ;

            jamfile
                =   *(space_p | comment)
                    >> !statements
                    >> !space
                ;

            this->start_parsers(
                jamfile, statements_, invoke, cond0, if_);
        }
    };
};

#endif // IMPL_BJAM_GRAMMAR_HPP
