// jam2bbk.cpp: make BoostBook section from bjam module

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/tools/jam2bbk/ for library home page.
#include <boost/algorithm/string/predicate.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace algo = boost::algorithm;

typedef std::vector<std::string> string_list;
typedef std::vector<string_list> list_of_list;

struct rule_definition
{
    std::string name;
    list_of_list parameters;

    void swap(rule_definition& rhs)
    {
        name.swap(rhs.name);
        parameters.swap(rhs.parameters);
    }
};

struct class_definition
{
    std::string name;
    std::string base;
    std::vector<rule_definition> rules;

    void swap(class_definition& rhs)
    {
        name.swap(rhs.name);
        base.swap(rhs.base);
        rules.swap(rhs.rules);
    }
};

template<class C>
void push_back_move(C& c, typename C::value_type& x)
{
    c.resize(c.size()+1);
    c.back().swap(x);
}

std::istream& skip_to(std::istream& is, char c)
{
    return is.ignore((std::numeric_limits<std::streamsize>::max)(), c);
}

std::istream& assert_token(std::istream& is, const char* s)
{
    std::string tmp;
    is >> tmp;
    if (tmp != s)
    {
        std::string msg;
        msg = "unexpected token \"";
        msg += tmp;
        msg += "\" is not \"";
        msg += s;
        msg += '"';
        throw std::runtime_error(msg);
    }
    return is;
}

std::ostream& print_rule_params(std::ostream& os, const list_of_list& params)
{
    os << "( ";
    for (std::size_t i = 0; i < params.size(); ++i)
    {
        if (i != 0)
            os << ": ";

        std::copy(
            params[i].begin(),
            params[i].end(),
            std::ostream_iterator<std::string>(os, " ")
        );
    }
    return os << ")";
}

class jam2xml
{
public:
    void parse(std::istream& is)
    {
        class_definition class_def;
        while (true)
        {
            typedef std::char_traits<char> tr;
            tr::int_type ic = is.peek();
            if (tr::eq_int_type(ic, tr::eof()))
            {
                is.ignore();
                break;
            }

            char c = tr::to_char_type(ic);
            if (c == '\n')
            {
                is.ignore();
                continue;
            }
            bool has_indent = (c == ' ');

            std::string s;
            if (!(is >> s))
                break;

            if (s == "class")
            {
                if (!class_def.name.empty())
                    ::push_back_move(classes_, class_def);

                this->parse_class(is, class_def);
            }
            else if (s == "rule")
            {
                if (!has_indent && !class_def.name.empty())
                    ::push_back_move(classes_, class_def);

                if (class_def.name.empty())
                    this->parse_rule(is, rules_);
                else
                    this->parse_rule(is, class_def.rules);
            }

            ::skip_to(is, '\n');
        }
    }

    void print(std::ostream& os, const std::string& module) const
    {
        os <<
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<!DOCTYPE section PUBLIC \"-//Boost//DTD BoostBook XML V1.0//EN\"\n"
        "  \"http://www.boost.org/tools/boostbook/dtd/boostbook.dtd\">\n";

        os << "<section id=\"bbv2.reference." << module << "\">\n";
        os << "  <title>" << module << " module</title>\n";

        this->print_synopsis(os, module);
        this->print_classes(os, module);
        this->print_rules(os, module);

        os << "</section>\n";
    }

private:
    std::vector<class_definition> classes_;
    std::vector<rule_definition> rules_;

    void parse_class(std::istream& is, class_definition& class_def)
    {
        class_definition def;
        is >> def.name;

        typedef std::char_traits<char> tr;
        if (!tr::eq_int_type(is.peek(), tr::to_int_type('\n')))
        {
            ::assert_token(is, ":");
            is >> def.base;
        }

        class_def.swap(def);
    }

    void parse_rule(std::istream& is, std::vector<rule_definition>& rules)
    {
        rule_definition rule;
        is >> rule.name;

        std::vector<std::string> param;
        std::string s;
        is >> s;
        if (s == "(")
        {
            while (is >> s)
            {
                if (s[0] == '#')
                    ::skip_to(is, '\n');
                else if (s == ")")
                    break;
                else if (s == ":")
                    ::push_back_move(rule.parameters, param);
                else
                    param.push_back(s);
            }

            if (!param.empty())
                ::push_back_move(rule.parameters, param);
        }

        ::push_back_move(rules, rule);
    }

    void print_synopsis(std::ostream& os, const std::string& module) const
    {
        os << "  <synopsis>";

        for (std::size_t i = 0; i < classes_.size(); ++i)
        {
            const class_definition& def = classes_[i];

            if (i != 0)
                os << '\n';

            os << "<emphasis role=\"bold\">class</emphasis> ";

            os
                << "<link linkend=\"bbv2.reference."
                << module << '.' << def.name << "\">"
                << def.name
                << "</link>"
                << " { }";
        }

        if (!rules_.empty())
        {
            if (!classes_.empty())
                os << '\n';

            os
                << "<emphasis role=\"bold\">module</emphasis> "
                << module << " {\n";

            for (std::size_t i = 0; i < rules_.size(); ++i)
            {
                const rule_definition& rule = rules_[i];

                os << "  <emphasis role=\"bold\">rule</emphasis> ";

                os
                    << "<link linkend=\"bbv2.reference."
                    << module << '.' << rule.name << "\">"
                    << rule.name
                    << "</link>"
                    << " ";

                ::print_rule_params(os, rule.parameters);

                os << " { }\n";
            }

            os << "}\n";
        }

        os << "</synopsis>\n";
    }

    void print_classes(std::ostream& os, const std::string& module) const
    {
        for (std::size_t i = 0; i < classes_.size(); ++i)
        {
            const class_definition& def = classes_[i];

            os
                << "  <refentry id=\"bbv2.reference."
                << module << "." << def.name << "\">\n";

            os << "    <refmeta>\n";
            os
                << "      <refentrytitle>Class "
                << def.name << "</refentrytitle>\n";
            os << "      <manvolnum>7</manvolnum>\n";
            os << "    </refmeta>\n";

            os << "    <refnamediv>\n";
            os << "      <refname>" << def.name << "</refname>\n";
            os << "      <refpurpose><simpara>TODO</simpara></refpurpose>\n";
            os << "    </refnamediv>\n";

            os << "    <refsynopsisdiv>\n";

            os << "      <programlisting>";
            os << "<emphasis role=\"bold\">class</emphasis> " << def.name;
            if (!def.base.empty())
            {
                os
                    << " : <link linkend=\"bbv2.reference."
                    << module << '.' << def.base << "\">"
                    << def.base
                    << "</link>";
            }
            os << " {\n";

            const std::vector<rule_definition>& rules = def.rules;
            for (std::size_t j = 0; j < rules.size(); ++j)
            {
                const rule_definition& rule = rules[j];
                os
                    << "  <emphasis role=\"bold\">rule</emphasis> "
                    << "<link linkend=\"bbv2.reference."
                    << module << '.' << def.name << '.' << rule.name << "\">"
                    << rule.name
                    << "</link>"
                    << " ";

                ::print_rule_params(os, rule.parameters);

                os << " { }\n";
            }

            os << "}</programlisting>\n";

            os << "    </refsynopsisdiv>\n";

            os
                << "    <refsection>\n"
                << "      <title>Description</title>\n"
                << "      <orderedlist>\n";

            for (std::size_t j = 0; j < rules.size(); ++j)
            {
                const rule_definition& rule = rules[j];

                os << "        <listitem>\n";

                os
                    << "          <para><literallayout class=\"monospaced\">"
                    << "<emphasis role=\"bold\">rule</emphasis> "
                    << "<anchor id=\"bbv2.reference."
                    << module << '.' << def.name << '.' << rule.name << "\"/>"
                    << rule.name
                    << " ";

                ::print_rule_params(os, rule.parameters);

                os << "</literallayout></para>\n";

                os << "        </listitem>\n";
            }

            os << "      </orderedlist>\n";
            os << "    </refsection>\n";
            os << "  </refentry>\n";
        }
    }

    void print_rules(std::ostream& os, const std::string& module) const
    {
        for (std::size_t i = 0; i < rules_.size(); ++i)
        {
            const rule_definition& rule = rules_[i];

            os
                << "  <refentry id=\"bbv2.reference."
                << module << "." << rule.name << "\">\n";

            os << "    <refmeta>\n";
            os
                << "      <refentrytitle>Rule "
                << rule.name << "</refentrytitle>\n";
            os << "      <manvolnum>7</manvolnum>\n";
            os << "    </refmeta>\n";

            os << "    <refnamediv>\n";
            os
                << "      <refname>"
                << module << '.' << rule.name
                << "</refname>\n";
            os << "      <refpurpose><simpara>TODO</simpara></refpurpose>\n";
            os << "    </refnamediv>\n";

            os << "    <refsynopsisdiv>\n";

            os
                << "      <programlisting>"
                << "<emphasis role=\"bold\">rule</emphasis> "
                << rule.name << ' ';

            ::print_rule_params(os, rule.parameters);

            os << "</programlisting>\n";

            os << "    </refsynopsisdiv>\n";

            os
                << "    <refsection>\n"
                << "      <title>Description</title>\n"
                << "      <para>TODO</para>\n"
                << "    </refsection>\n";

            os << "  </refentry>\n";
        }
    }
};

std::string make_module_name(const std::string& filename)
{
#if defined(BOOST_WINDOWS)
    const char delim = '\\';
#else
    const char delim = '/';
#endif
    std::string::size_type slash = filename.rfind(delim);
    std::string::size_type start = (slash == std::string::npos) ? 0 : slash+1;
    std::string::size_type finish = filename.rfind('.');
    return filename.substr(start, finish-start);
}

int main(int argc, char* argv[])
{
    try
    {
        if ((argc != 2) && (argc != 3))
        {
            std::cerr << "Usage: jam2bbk (jam) [xml]" << std::endl;
            return 1;
        }

        jam2xml parser;

        {
            std::ifstream is(argv[1]);
            parser.parse(is);
        }

        const std::string& module = make_module_name(argv[1]);
        if (argc == 3)
        {
            std::ofstream os(argv[2]);
            parser.print(os, module);
        }
        else
            parser.print(std::cout, module);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
