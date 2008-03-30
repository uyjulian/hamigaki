// bjam_check.cpp: bjam grammar checker

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

//#define BOOST_SPIRIT_DEBUG
#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/none.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace bjam = hamigaki::bjam;

std::string get_first_line(const char* s)
{
    const char* p = std::strchr(s, '\n');
    if (p)
    {
        if ((p != s) && (p[-1] == '\r'))
            --p;
        return std::string(s, p);
    }
    else
        return s;
}

bjam::parse_info<const char*>
parse_bjam(const std::string& s, bjam::context& ctx)
{
    typedef bjam::bjam_grammar_gen<const char*> grammar_type;

    const char* first = s.c_str();
    const char* last = first + s.size();

    return grammar_type::parse_bjam_grammar(first, last, ctx);
}

void parse(bjam::context& ctx, const char* filename)
{
    std::string str;
    {
        std::ifstream is(filename, std::ios_base::binary);

        str.assign(
            std::istreambuf_iterator<char>(is),
            (std::istreambuf_iterator<char>())
        );
    }

    bjam::parse_info<const char*> info = parse_bjam(str, ctx);

    if (!info.full)
    {
        throw std::runtime_error(
            "syntax error at \"" + get_first_line(info.stop) + '"');
    }
}

std::string html_escape(const std::string& name)
{
    std::string tmp;
    for (std::size_t i = 0, size = name.size(); i < size; ++i)
    {
        const char c = name[i];
        if (c == '<')
            tmp += "&lt;";
        else if (c == '>')
            tmp += "&gt;";
        else
            tmp += c;
    }
    return tmp;
}

std::string make_module_output_name(const std::string& name)
{
    std::string tmp;
    tmp.reserve(name.size()+5);
    for (std::size_t i = 0, size = name.size(); i < size; ++i)
    {
        const char c = name[i];
        if ((c == '/') || (c == '<') || (c == '>') || (c == ':'))
            tmp += '_';
        else
            tmp += c;
    }
    tmp += ".html";
    return tmp;
}

std::string markup_link(const std::string& s)
{
    std::string::size_type obj_pos = s.find("object(");

    if (obj_pos != std::string::npos)
    {
        std::string obj(s, obj_pos);
        std::string tmp;
        tmp += html_escape(s.substr(0u, obj_pos));
        tmp += "<a href=\"";
        tmp += make_module_output_name(obj);
        tmp += "\">";
        tmp += html_escape(obj);
        tmp += "</a>";
        return tmp;
    }
    else
        return html_escape(s);
}

void dump_module(const std::string& name, const bjam::module& m)
{
    const std::string& filename = make_module_output_name(name);
    std::ofstream os(filename.c_str());
    if (!os)
        throw std::runtime_error("cannot open file: " + filename);

    os << "<html>\n";
    os << "<head>\n";
    os << "<title>" << html_escape(name) << "</title>";
    os << "</head>\n";

    typedef bjam::variable_table::iterator iter_type;
    const std::pair<iter_type,iter_type>& vars = m.variables.entries();

    os << "<table border=\"1\">\n";
    os << "<tr>\n";
    os << "<th>name</th>\n";
    os << "<th>values</th>\n";
    os << "</tr>\n";
    for (iter_type i = vars.first; i != vars.second; ++i)
    {
        const bjam::string_list& values = i->second;
        if (values.empty())
            continue;

        os << "<tr>\n";

        if (values.size() < 2)
            os << "<td>";
        else
            os << "<td rowspan=\"" << values.size() << "\">";
        os << markup_link(i->first) << "</td>\n";

        if (values.empty())
            os << "<td></td>\n";
        else
            os << "<td>" << markup_link(values[0]) << "</td>\n";

        os << "</tr>\n";

        for (std::size_t j = 1; j < values.size(); ++j)
        {
            os << "<tr>\n";
            os << "<td>" << markup_link(values[j]) << "</td>\n";
            os << "</tr>\n";
        }
    }
    os << "</table>\n";

    os << "</html>\n";
}

void dump(bjam::context& ctx)
{
    dump_module("root", ctx.get_module(boost::none));

    typedef bjam::context::module_iterator module_iterator;
    std::pair<module_iterator,module_iterator> modules = ctx.module_entries();
    for (module_iterator i = modules.first; i != modules.second; ++i)
        dump_module(i->first, i->second);
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: bjam_dump (filename) ..." << std::endl;
            return 1;
        }

        bjam::context ctx;
        parse(ctx, argv[1]);
        dump(ctx);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
