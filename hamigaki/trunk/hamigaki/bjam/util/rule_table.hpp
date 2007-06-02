// rule_table.hpp: bjam rule table

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_RULE_TABLE_HPP
#define HAMIGAKI_BJAM_UTIL_RULE_TABLE_HPP

#include <hamigaki/bjam/util/rule_definition.hpp>
#include <map>

namespace hamigaki { namespace bjam {

class rule_table
{
public:
    typedef std::map<std::string,rule_def_ptr> table_type;
    typedef table_type::const_iterator iterator;

    rule_def_ptr get_rule_definition(const std::string& name) const
    {
        typedef table_type::const_iterator iter_type;

        iter_type pos = table_.find(name);
        if (pos != table_.end())
            return pos->second;
        else
            return rule_def_ptr();
    }

    void set_rule_definition(const std::string& name, const rule_def_ptr& def)
    {
        table_[name] = def;
    }

    std::pair<iterator,iterator> entries() const
    {
        return std::make_pair(table_.begin(), table_.end());
    }

private:
    table_type table_;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_RULE_TABLE_HPP
