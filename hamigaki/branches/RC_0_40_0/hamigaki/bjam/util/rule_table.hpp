// rule_table.hpp: bjam rule table

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_RULE_TABLE_HPP
#define HAMIGAKI_BJAM_UTIL_RULE_TABLE_HPP

#include <hamigaki/bjam/util/rule_definition.hpp>
#include <hamigaki/bjam/bjam_exceptions.hpp>
#include <map>

namespace hamigaki { namespace bjam {

class rule_table
{
public:
    typedef std::map<std::string,rule_definition> table_type;
    typedef table_type::const_iterator iterator;

    rule_definition* get_rule_definition_ptr(const std::string& name)
    {
        table_type::iterator pos = table_.find(name);
        if (pos == table_.end())
            return 0;
        return &pos->second;
    }

    rule_definition& get_rule_definition(const std::string& name)
    {
        rule_definition* ptr = this->get_rule_definition_ptr(name);
        if (ptr == 0)
            throw rule_not_found(name);
        return *ptr;
    }

    const rule_definition*
    get_rule_definition_ptr(const std::string& name) const
    {
        table_type::const_iterator pos = table_.find(name);
        if (pos == table_.end())
            return 0;
        return &pos->second;
    }

    const rule_definition& get_rule_definition(const std::string& name) const
    {
        const rule_definition* ptr = this->get_rule_definition_ptr(name);
        if (ptr == 0)
            throw rule_not_found(name);
        return *ptr;
    }

    void set_rule_definition(
        const std::string& name, const rule_definition& def)
    {
        table_[name] = def;
    }

    void set_rule_body(
        const std::string& name, const rule_definition& def)
    {
        rule_definition& x = table_[name];
        x.parameters = def.parameters;
        x.body = def.body;
        x.module_name = def.module_name;
        x.exported = def.exported;
        x.filename = def.filename;
        x.line = def.line;
    }

    void set_native_rule(
        const std::string& name, const rule_definition& def)
    {
        rule_definition& x = table_[name];
        x.parameters = def.parameters;
        x.native = def.native;
        x.module_name = def.module_name;
        x.exported = def.exported;
    }

    void set_rule_actions(
        const std::string& name, const rule_definition& act)
    {
        rule_definition& x = table_[name];
        x.commands = act.commands;
        x.modifiers = act.modifiers;
        x.binds = act.binds;
    }

    void clear()
    {
        table_.clear();
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
