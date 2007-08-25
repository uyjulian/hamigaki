// rule_table.hpp: the table of the rule_table

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_RULE_TABLE_HPP
#define IMPL_RULE_TABLE_HPP

#include <map>
#include <string>
#include <vector>

struct rule_data
{
    typedef std::vector<std::string> field_type;

    std::vector<field_type> fields;
    std::string source;

    rule_data()
    {
    }

    rule_data(const std::vector<field_type>& f, const std::string& src)
        : fields(f), source(src)
    {
    }
};

class rule_table
{
public:
    typedef std::string key_type;
    typedef rule_data mapped_type;
    typedef std::map<key_type,mapped_type> table_type;

    explicit rule_table(rule_table* g=0) : global_(g)
    {
    }

    void add_local(const std::string& k, const mapped_type& m)
    {
        local_[k] = m;
    }

    void add(const std::string& k, const mapped_type& m)
    {
        if (global_)
            global_->add(k, m);
        else
            this->add_local(k, m);
    }

    const mapped_type* find(const std::string& k) const
    {
        table_type::const_iterator it = local_.find(k);
        if (it != local_.end())
            return &(it->second);
        else if (global_)
            return global_->find(k);
        else
            return 0;
    }

private:
    rule_table* global_;
    table_type local_;
};

#endif // IMPL_RULE_TABLE_HPP
