// variable_table.hpp: bjam variable table

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_VARIABLE_TABLE_HPP
#define HAMIGAKI_BJAM_UTIL_VARIABLE_TABLE_HPP

#include <hamigaki/bjam/util/list.hpp>
#include <map>

namespace hamigaki { namespace bjam {

class variable_table
{
public:
    typedef std::map<std::string,list_type> table_type;
    typedef table_type::const_iterator iterator;

    const list_type& get_values(const std::string& name) const
    {
        typedef table_type::const_iterator iter_type;

        iter_type pos = table_.find(name);
        if (pos != table_.end())
            return pos->second;
        else
            return empty_;
    }

    void set_values(const std::string& name, const list_type& values)
    {
        table_[name] = values;
    }

    void append_values(const std::string& name, const list_type& values)
    {
        list_type& v = table_[name];
        v.insert(v.end(), values.begin(), values.end());
    }

    void set_default_values(const std::string& name, const list_type& values)
    {
        list_type& v = table_[name];
        if (v.empty())
            v = values;
    }

    void swap_values(const std::string& name, list_type& values)
    {
        table_[name].swap(values);
    }

    std::pair<iterator,iterator> entries() const
    {
        return std::make_pair(table_.begin(), table_.end());
    }

    void swap(variable_table& rhs)
    {
        table_.swap(rhs.table_);
    }

private:
    table_type table_;
    list_type empty_;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_VARIABLE_TABLE_HPP
