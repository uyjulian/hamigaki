// order.cpp: bjam order module

// Copyright Takeshi Mouri 2007-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/modules/order.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/topological_sort.hpp>
#include <algorithm>
#include <iterator>
#include <vector>

namespace hamigaki { namespace bjam {

namespace modules
{

namespace order
{

namespace
{

int name_to_index(const string_list& objects, const std::string& name)
{
    typedef string_list::const_iterator iter_type;
    iter_type it = std::find(objects.begin(), objects.end(), name);
    if (it != objects.end())
        return static_cast<int>(std::distance(objects.begin(), it));
    else
        return -1;
}

} // namespace

HAMIGAKI_BJAM_DECL string_list add_pair(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg1 = args[0];

    const std::string& first = arg1[0];
    const std::string& second = arg1[1];

    f.current_module().variables.append_values(first, string_list(second));

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list order(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& objects = args[0];

    module& m = f.current_module();
    variable_table& vt = m.variables;

    typedef boost::adjacency_list<> graph_type;
    graph_type g;
    for (std::size_t i = 0, size = objects.size(); i < size; ++i)
    {
        const string_list& values = vt.get_values(objects[i]);

        for (std::size_t j = 0; j < values.size(); ++j)
        {
            int index = name_to_index(objects, values[j]);
            if (index != -1)
                boost::add_edge(static_cast<int>(i), index, g);
        }
    }

    typedef boost::graph_traits<graph_type>::vertex_descriptor vertex_type;
    typedef std::vector<vertex_type> buffer_type;
    buffer_type tmp;
    boost::topological_sort(g, std::back_inserter(tmp));

    boost::property_map<
        graph_type,
        boost::vertex_index_t
    >::type id = boost::get(boost::vertex_index, g);

    string_list result;
    for (buffer_type::reverse_iterator i = tmp.rbegin(); i != tmp.rend(); ++i)
        result += objects[id[*i]];

    return result;
}

} // namespace order

HAMIGAKI_BJAM_DECL void set_order_rules(context& ctx)
{
    module& m = ctx.get_module(std::string("class@order"));

    {
        native_rule rule;
        rule.parameters.push_back(boost::assign::list_of("first")("second"));
        rule.native = &order::add_pair;
        rule.version = 1;
        m.native_rules["add-pair"] = rule;
    }

    {
        native_rule rule;
        rule.parameters.push_back(boost::assign::list_of("objects")("*"));
        rule.native = &order::order;
        rule.version = 1;
        m.native_rules["order"] = rule;
    }
}

} // namespace modules

} } // End namespaces bjam, hamigaki.
