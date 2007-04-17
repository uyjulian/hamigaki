//  bjam_parser.cpp: bjam parser

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#include "bjam_parser.hpp"
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/utility/chset_operators.hpp>
#include <fstream>
#include <iterator>

using namespace boost::spirit;

namespace
{

struct bjam_grammar : grammar<bjam_grammar>
{
    explicit bjam_grammar(std::vector<std::string>& vs) : storage(vs)
    {
    }

    std::vector<std::string>& storage;

    template<class ScannerT>
    struct definition
    {
        typedef rule<ScannerT> rule_t;
        typedef rule<typename lexeme_scanner<ScannerT>::type> lexeme_rule_t;

        lexeme_rule_t literal_char, escape_char, pattern_char;

        symbols<> keyword;

        rule_t statements, literal, varexp, vardef, rulename;
        rule_t actions, action_modifiers, action_binds;
        rule_t cond0, cond, block, if_, for_, while_;
        rule_t invoke0, invoke, rule_expansion;
        rule_t pattern, switch_;
        rule_t exe, lib;

        definition(const bjam_grammar& self)
        {
            literal_char
                =   anychar_p - space_p - '"' - '\\'
                ;

            escape_char
                =   '\\' >> anychar_p
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
                            (+literal_char - keyword)
                        |   ('"' >> *( escape_char | literal_char ) >> '"')
                    ]
                ;

            varexp
                =   rule_expansion
                |   literal
                ;

            vardef
                =   !str_p("local")
                    >> literal
                    >> !( "on" >> +varexp )
                    >>  ( ch_p('=') | "+=" | "?=" )
                    >> *varexp
                    >> ';'
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
                =   literal - "exe" - "lib"
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
                    >> !( "else" >> block )
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

            pattern
                =   lexeme_d
                    [
                       +(   pattern_char
                        |   '[' >> !ch_p('^') >> +pattern_char >> ']'
                        |   escape_char
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

            invoke0
                =   rulename >> *( varexp | ':' )
                ;

            invoke
                =   invoke0 >> ';'
                ;

            rule_expansion
                =   '[' >> invoke0 >> ']'
                ;

            exe
                =   "exe"
                    >> literal[push_back_a(self.storage)]
                    >> ':'
                    >> *( varexp | ':' )
                    >> ';'
                ;

            lib
                =   "lib"
                    >> literal[push_back_a(self.storage)]
                    >> ':'
                    >> *( varexp | ':' )
                    >> ';'
                ;

            statements
                =  *(   actions
                    |   block
                    |   if_
                    |   for_
                    |   while_
                    |   switch_
                    |   invoke
                    |   vardef
                    |   exe
                    |   lib
                    )
                ;
        }

        const rule_t& start() const
        {
            return statements;
        }
    };
};

struct bjam_skip_grammar : grammar<bjam_skip_grammar>
{
    template<class ScannerT>
    struct definition
    {
        typedef rule<ScannerT> rule_t;

        rule_t skip;

        definition(const bjam_skip_grammar&)
        {
            skip
                =   space_p
                |   '#' >> *(anychar_p - '\n') >> '\n'
                ;
        }

        const rule_t& start() const { return skip; }
    };
};

} // namespace

bool parse_jamfile(
    const std::string& filename, std::vector<std::string>& targets)
{
    std::ifstream is(filename.c_str(), std::ios_base::binary);

    std::string src(
        std::istreambuf_iterator<char>(is),
        (std::istreambuf_iterator<char>())
    );

    bjam_grammar g(targets);
    bjam_skip_grammar skip;

    return parse(src.c_str(), g, skip).full;
}
