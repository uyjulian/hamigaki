//  bjam_grammar.hpp: bjam grammar

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_BJAM_GRAMMAR_HPP
#define IMPL_BJAM_GRAMMAR_HPP

#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/utility/chset_operators.hpp>
#include <vector>
#include <string>

inline void remove_branch_path(std::string& s)
{
    std::string::size_type pos = s.rfind('/');
    if (pos != std::string::npos)
        s.erase(0, pos+1);
}

class push_back_target_actor
{
public:
    explicit push_back_target_actor(std::vector<std::string>& vs) : vs_(vs)
    {
    }

    template<class Iterator>
    void operator()(Iterator first, Iterator last) const
    {
        std::string s(first, last);
        ::remove_branch_path(s);

        if (s.find('$') != std::string::npos)
            return;

        vs_.push_back(s);
    }

private:
    std::vector<std::string>& vs_;
};

struct testing_params
{
    std::string source;
    std::string target_name;

    testing_params(){}

    testing_params(const std::string& src, const std::string& name)
        : source(src), target_name(name)
    {
    }
};

class push_back_test_actor
{
public:
    explicit push_back_test_actor(std::vector<std::string>& vs) : vs_(vs)
    {
    }

    void operator()(const testing_params& val) const
    {
        std::string s = val.target_name;

        if (s.empty())
        {
            s = val.source;

            ::remove_branch_path(s);

            std::string::size_type pos = s.rfind('.');
            if (pos != std::string::npos)
                s.erase(pos);
        }

        if (s.find('$') != std::string::npos)
            return;

        vs_.push_back(s);
    }

private:
    std::vector<std::string>& vs_;
};

class push_back_bpl_test_actor
{
public:
    explicit push_back_bpl_test_actor(std::vector<std::string>& vs) : vs_(vs)
    {
    }

    template<class Iterator>
    void operator()(Iterator first, Iterator last) const
    {
        std::string s(first, last);

        if (s.find('$') != std::string::npos)
            return;

        vs_.push_back(s);
    }

private:
    std::vector<std::string>& vs_;
};

struct bjam_skip_grammar : boost::spirit::grammar<bjam_skip_grammar>
{
    template<class ScannerT>
    struct definition
    {
        typedef boost::spirit::rule<ScannerT> rule_t;

        rule_t skip;

        definition(const bjam_skip_grammar&)
        {
            using namespace boost::spirit;

            skip
                =   space_p
                |   '#' >> *(anychar_p - '\n') >> '\n'
                ;
        }

        const rule_t& start() const { return skip; }
    };
};

struct bjam_grammar : boost::spirit::grammar<bjam_grammar>
{
    explicit bjam_grammar(std::vector<std::string>& vs) : storage(vs)
    {
    }

    std::vector<std::string>& storage;

    template<class ScannerT>
    struct definition
    {
        typedef boost::spirit::rule<ScannerT> rule_t;

        typedef boost::spirit::rule<
            typename boost::spirit::lexeme_scanner<ScannerT>::type
        > lexeme_rule_t;

        lexeme_rule_t literal_char, quote_char, pattern_char;

        boost::spirit::symbols<> keyword;

        rule_t statements, statement, literal, varexp, rulename;
        rule_t actions, action_modifiers, action_binds;
        rule_t cond0, cond, block, if_, for_, while_, rule_;
        rule_t invoke, rule_expansion;
        rule_t pattern, switch_;
        rule_t exe, lib, test_rule, run_rule, bpl_test;
#if BOOST_VERSION >= 103400
        rule_t jamfile;
        bjam_skip_grammar skip;
#endif

        struct test_closure
            : boost::spirit::closure<
                test_closure,
                testing_params, std::string, std::string
            >
        {
            member1 val;
            member2 source;
            member3 target_name;
        };

        boost::spirit::rule<ScannerT, typename test_closure::context_t> test;
        boost::spirit::rule<ScannerT, typename test_closure::context_t> run;

        definition(const bjam_grammar& self)
        {
            using namespace boost::spirit;
            using namespace phoenix;

            literal_char
                =   anychar_p - space_p - '"' - '\\'
                ;

            quote_char
                =   anychar_p - '"' - '\\'
                ;

            pattern_char
                =   anychar_p - space_p - '\\' - '[' - ']'
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
                ,   "break"
                ,   "case"
                ,   "continue"
                ,   "else"
                ,   "existing"
                ,   "for"
                ,   "if"
                ,   "ignore"
                ,   "in"
                ,   "include"
                ,   "local"
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

            literal
                =   lexeme_d
                    [
                       +(   literal_char
                        |   '"'
                            >> *( quote_char | '\\' >> anychar_p )
                            >> '"'
                        ) - keyword
                    ]
                ;

            varexp
                =   rule_expansion
                |   literal
                ;

            action_modifiers
                =  *(   str_p("existing")
                    |   str_p("ignore")
                    |   str_p("piecemeal")
                    |   str_p("quietly")
                    |   str_p("together")
                    |   str_p("updated")
                    )
                ;

            action_binds
                =   str_p("bind")
                    >> +literal
                ;

            rulename
                =   literal - "exe" - "lib" - test_rule - run_rule
                ;

            actions
                =   str_p("actions")
                    >> action_modifiers
                    >> rulename
                    >> !action_binds
                    >> '{'
                    >> lexeme_d[ *(~chset<>('}')) ]
                    >> '}'
                ;

            cond0
                =   '(' >> cond >> ')'
                |   '!' >> cond
                |   varexp
                    >> !(
                            (
                                ch_p('=')
                            |   "!="
                            |   '<'
                            |   "<="
                            |   '>'
                            |   ">="
                            |   "in"
                            )
                            >> varexp
                        )
                ;

            cond
                =   cond0
                    >> *(
                            ( str_p("&&") | "||" )
                            >> cond0
                        )
                ;

            block
                =   '{' >> statements >> '}'
                ;

            if_
                =   "if" >> cond
                    >> block
                    >> !( "else" >> ( if_ | block ) )
                ;

            for_
                =   "for"
                    >> !str_p("local") >> literal
                    >> "in" >> *varexp
                    >> block
                ;

            while_
                =   "while" >> cond
                    >> block
                ;

            rule_
                =   "rule" >> literal
                    >>  (   '(' >> *( varexp | ':' ) >> ')'
                        |   *( varexp | ':' )
                        )
                    >> block
                ;

            pattern
                =   lexeme_d
                    [
                       +(   pattern_char
                        |   '[' >> !ch_p('^') >> +pattern_char >> ']'
                        |   '\\' >> anychar_p
                        )
                    ]
                ;

            switch_
                =   "switch"
                    >> varexp
                    >> '{'
                    >> *( "case" >> pattern >> ':' >> statements )
                    >> '}'
                ;

            invoke
                =   !str_p("local")
                    >> rulename
                    >> !( "on" >> +varexp )
                    >> !( ch_p('=') | "+=" | "?=" )
                    >> *( varexp | ':' )
                ;

            rule_expansion
                =   '[' >> statement >> ']'
                ;

            exe
                =   "exe"
                    >> literal[push_back_target_actor(self.storage)]
                    >> ':'
                    >> *( varexp | ':' )
                ;

            lib
                =   "lib"
                    >> literal[push_back_target_actor(self.storage)]
                    >> ':'
                    >> *( varexp | ':' )
                ;

            test_rule
                =   str_p("compile-fail")
                |   "compile"
                |   "link-fail"
                |   "link"
                ;

            test
                =   test_rule
                    // sources
                    >>  literal
                        [
                            test.source = construct_<std::string>(arg1, arg2)
                        ]
                    >> *( varexp )
                    >> !( ':' >> *( varexp ) )  // requirements
                    // target-name
                    >> !(   ':'
                            >>  varexp
                                [
                                    test.target_name =
                                        construct_<std::string>(arg1, arg2)
                                ]
                        )
                    >>  eps_p
                        [
                            test.val =
                                construct_<testing_params>(
                                    test.source, test.target_name)
                        ]
                ;

            run_rule
                =   str_p("run-fail")
                |   "run"
                ;

            run
                =   run_rule
                    // sources
                    >>  literal
                        [
                            run.source = construct_<std::string>(arg1, arg2)
                        ]
                    >> *( varexp )
                    >> !( ':' >> *( varexp ) )  // args
                    >> !( ':' >> *( varexp ) )  // input-files
                    >> !( ':' >> *( varexp ) )  // requirements
                    // target-name
                    >> !(   ':'
                            >>  varexp
                                [
                                    run.target_name =
                                        construct_<std::string>(arg1, arg2)
                                ]
                        )
                    >> !( ':' >> *( varexp ) )  // default-build
                    >>  eps_p
                        [
                            run.val =
                                construct_<testing_params>(
                                    run.source, run.target_name)
                        ]
                ;

            bpl_test
                =   "bpl-test"
                    >> literal[push_back_bpl_test_actor(self.storage)]
                    >> *( varexp | ':' )
                ;

            statement
                =   exe
                |   lib
                |   test[push_back_test_actor(self.storage)]
                |   run[push_back_test_actor(self.storage)]
                |   bpl_test
                |   invoke
                |   "break"
                |   "continue"
                |   "return" >> *( varexp | ':' )
                |   "include" >> varexp
                ;

            statements
                =  *(   actions
                    |   block
                    |   if_
                    |   for_
                    |   while_
                    |   switch_
                    |   rule_
                    |   statement >> ';'
                    )
                ;

#if BOOST_VERSION >= 103400
            jamfile
                =   statements
                    >> lexeme_d[!skip]
                ;
#endif
        }

        const rule_t& start() const
        {
#if BOOST_VERSION < 103400
            return statements;
#else
            return jamfile;
#endif
        }
    };
};

#endif // IMPL_BJAM_GRAMMAR_HPP
