//  var_expand_grammar_test.cpp: a test driver for var_expand_grammar

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#include "../impl/var_expand_grammar.hpp"
#include <exception>
#include <iostream>
#include <iterator>

using namespace boost::spirit;
using namespace phoenix;

void expand_test(const variables& table, const std::string& str)
{
    std::cout << "Input: " << str << std::endl;

    var_expand_grammar g(table);
    variables::mapped_type val;
    parse_info<const char*> info = parse(str.c_str(), g[var(val) = arg1]);

    if (info.full)
    {
        std::copy(
            val.begin(), val.end(),
            std::ostream_iterator<std::string>(std::cout, "\n")
        );
    }
    else
    {
        std::cerr << "Error: stopping at" << std::endl;
        std::cerr << info.stop << std::endl;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        variables table;
        {
            std::vector<std::string> vs;
            vs.push_back("a");
            vs.push_back("b");
            vs.push_back("c");
            table.add("X");
            table.assign("X", vs);
        }
        {
            std::vector<std::string> vs;
            vs.push_back("1");
            vs.push_back("2");
            table.add("Y");
            table.assign("Y", vs);
        }
        {
            std::vector<std::string> vs;
            vs.push_back("X");
            vs.push_back("Y");
            table.add("Z");
            table.assign("Z", vs);
        }
        {
            std::vector<std::string> vs;
            vs.push_back("abc.txt");
            vs.push_back("C:\\Temp\\log.txt");
            vs.push_back("C:\\Windows\\System32\\kernel32.dll");
            table.add("P");
            table.assign("P", vs);
        }
        {
            std::vector<std::string> vs;
            vs.push_back("<grist>hoge");
            vs.push_back("<non-grist");
            vs.push_back("abc");
            table.add("G");
            table.assign("G", vs);
        }

        ::expand_test(table, "$($(Z)[2-4]:UL)");
        ::expand_test(table, "$(P:P)");
        ::expand_test(table, "$(G:G)");
        ::expand_test(table, "$(G:G=new)");

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
