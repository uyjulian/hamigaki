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

class push_back_boostbook_actor
{
public:
    explicit push_back_boostbook_actor(std::vector<std::string>& vs) : vs_(vs)
    {
    }

    template<class Iterator>
    void operator()(Iterator, Iterator) const
    {
        vs_.push_back("html");
        vs_.push_back("onehtml");
        vs_.push_back("man");
        vs_.push_back("docbook");
        vs_.push_back("fo");
        vs_.push_back("pdf");
        vs_.push_back("ps");
        vs_.push_back("tests");
    }

private:
    std::vector<std::string>& vs_;
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

        boost::spirit::symbols<> keyword;

        rule_t literal_char, quote_char, pattern_char;
        rule_t comment, space;
        rule_t statements, statement0, statement, literal, varexp, rulename;
        rule_t actions, action_modifiers, action_binds;
        rule_t cond0, cond, block, if_, for_, while_, rule_, module;
        rule_t invoke, rule_expansion;
        rule_t pattern, switch_;
        rule_t exe, lib, install, boostbook, test_rule, run_rule, bpl_test;
        rule_t jamfile;

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
                =  +(   literal_char
                    |   '"'
                        >> *( quote_char | '\\' >> anychar_p )
                        >> '"'
                    ) - keyword
                ;

            varexp
                =   rule_expansion
                |   literal
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

            rulename
                =   literal - "exe" - "lib" - "install" - "boostbook"
                    - test_rule - run_rule - bpl_test
                ;

            actions
                =   str_p("actions")
                    >> action_modifiers
                    >> space
                    >> rulename
                    >> !action_binds
                    >> space
                    >> '{'
                    >> space_p
                    >> *(~chset<>('}'))
                    >> '}'
                ;

            cond0
                =   '(' >> space >> cond >> space >> ')'
                |   '!' >> space >> cond
                |   varexp
                    >>
                   !(   space >> '='  >> space >> varexp
                    |   space >> "!=" >> space >> varexp
                    |   space >> '<'  >> space >> varexp
                    |   space >> "<=" >> space >> varexp
                    |   space >> '>'  >> space >> varexp
                    |   space >> ">=" >> space >> varexp
                    |   space >> "in" >> *(space >> varexp)
                    )
                ;

            cond
                =   cond0
                    >> *(   space >> "&&" >> space >> cond0
                        |   space >> "||" >> space >> cond0
                        )
                ;

            block
                =   '{' >> !(space >> statements) >> space >> '}'
                ;

            if_
                =   "if" >> space >> cond >> space
                    >> block
                    >> !( space >> "else" >> space >> ( if_ | block ) )
                ;

            for_
                =   "for"
                    >> !( space >> str_p("local") )
                    >> space
                    >> literal
                    >> space
                    >> "in" >> *( space >> varexp )
                    >> space
                    >> block
                ;

            while_
                =   "while" >> space >> cond >> space
                    >> block
                ;

            rule_
                =   "rule" >> space >> literal >> space
                    >>  (   '(' >> *( space >> (varexp | ':') ) >> space >> ')'
                        |   *( space >> (varexp | ':') )
                        )
                    >> space
                    >> block
                ;

            module
                =   "module" >> space >> literal >> space
                    >> block
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
                            >> !(space >> statements)
                        )
                    >> space
                    >> '}'
                ;

            invoke
                =   !(str_p("local") >> space)
                    >> rulename
                    >> !( space >> "on" >> +(space >> varexp) )
                    >> !( space >> (ch_p('=') | "+=" | "?=") )
                    >> *( space >> (varexp | ':') )
                ;

            rule_expansion
                =   '[' >> space >> statement0 >> space >> ']'
                ;

            exe
                =   "exe"
                    >> space
                    >> literal[push_back_target_actor(self.storage)]
                    >> space
                    >> ':'
                    >> *( space >> (varexp | ':') )
                ;

            lib
                =   "lib"
                    >> space
                    >> literal[push_back_target_actor(self.storage)]
                    >> space
                    >> ':'
                    >> *( space >> (varexp | ':') )
                ;

            install
                =   "install"
                    >> space
                    >> literal[push_back_target_actor(self.storage)]
                    >> space
                    >> ':'
                    >> *( space >> (varexp | ':') )
                ;

            boostbook
                =   "boostbook"
                    >> *( space >> (varexp | ':') )
                ;

            test_rule
                =   str_p("compile-fail")
                |   "compile"
                |   "link-fail"
                |   "link"
                ;

            test
                =   test_rule
                    >> space
                    // sources
                    >>  literal
                        [
                            test.source = construct_<std::string>(arg1, arg2)
                        ]
                    >> *( space >> varexp )
                    >> !( space >> ':' >> *(space >> varexp) )  // requirements
                    // target-name
                    >> !(   space
                            >> ':'
                            >> space
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
                    >> space
                    // sources
                    >>  literal
                        [
                            run.source = construct_<std::string>(arg1, arg2)
                        ]
                    >> *( space >> varexp )
                    >> !( space >> ':' >> *(space >> varexp) )  // args
                    >> !( space >> ':' >> *(space >> varexp) )  // input-files
                    >> !( space >> ':' >> *(space >> varexp) )  // requirements
                    // target-name
                    >> !(   space
                            >> ':'
                            >> space
                            >>  varexp
                                [
                                    run.target_name =
                                        construct_<std::string>(arg1, arg2)
                                ]
                        )
                    >> !( space >> ':' >> *(space >> varexp) )  // default-build
                    >>  eps_p
                        [
                            run.val =
                                construct_<testing_params>(
                                    run.source, run.target_name)
                        ]
                ;

            bpl_test
                =   "bpl-test"
                    >> space
                    >> literal[push_back_bpl_test_actor(self.storage)]
                    >> *( space >> (varexp | ':') )
                ;

            statement0
                =   exe
                |   lib
                |   install
                |   boostbook[push_back_boostbook_actor(self.storage)]
                |   test[push_back_test_actor(self.storage)]
                |   run[push_back_test_actor(self.storage)]
                |   bpl_test
                |   invoke
                |   "break"
                |   "continue"
                |   "return" >> *( space >> (varexp | ':') )
                |   "include" >> space >> varexp
                ;

            statement
                =   (   actions
                    |   block
                    |   if_
                    |   for_
                    |   while_
                    |   switch_
                    |   rule_
                    |   module
                    |   statement0 >> space >> ';'
                    )
                ;

            statements
                =   statement >> *(space >> statement)
                ;

            jamfile
                =   *(space_p | comment)
                    >> !statements
                    >> !space
                ;
        }

        const rule_t& start() const
        {
            return jamfile;
        }
    };
};

#endif // IMPL_BJAM_GRAMMAR_HPP
