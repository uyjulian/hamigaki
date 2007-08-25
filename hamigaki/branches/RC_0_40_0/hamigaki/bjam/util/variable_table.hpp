// variable_table.hpp: bjam variable table

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_VARIABLE_TABLE_HPP
#define HAMIGAKI_BJAM_UTIL_VARIABLE_TABLE_HPP

#include <hamigaki/bjam/util/assign_modes.hpp>
#include <hamigaki/bjam/util/list.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <stdexcept>

namespace hamigaki { namespace bjam {

class variable_table
{
public:
    typedef std::map<std::string,string_list> table_type;
    typedef table_type::const_iterator iterator;

    const string_list& get_values(const std::string& name) const
    {
        typedef table_type::const_iterator iter_type;

        iter_type pos = table_.find(name);
        if (pos != table_.end())
            return pos->second;
        else
            return empty_;
    }

    void set_values(const std::string& name, const string_list& values)
    {
        table_[name] = values;
    }

    void append_values(const std::string& name, const string_list& values)
    {
        table_[name] += values;
    }

    void set_default_values(const std::string& name, const string_list& values)
    {
        string_list& v = table_[name];
        if (v.empty())
            v = values;
    }

    void clear()
    {
        table_.clear();
    }

    void swap_values(const std::string& name, string_list& values)
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

    void push_local_variables(variable_table& local)
    {
        typedef table_type::iterator iter_type;

        table_type& t = local.table_;
        for (iter_type i = t.begin(), end = t.end(); i != end; ++i)
            this->swap_values(i->first, i->second);
    }

    void pop_local_variables(variable_table& local)
    {
        this->push_local_variables(local);
    }

private:
    table_type table_;
    string_list empty_;
};

inline void set_variables(
    variable_table& table, assign_mode::values mode,
    const string_list& names, const string_list& values)
{
    typedef string_list::const_iterator iter_type;

    if (mode == assign_mode::set)
    {
        for (iter_type i = names.begin(), end = names.end(); i != end; ++i)
            table.set_values(*i, values);
    }
    else if (mode == assign_mode::append)
    {
        for (iter_type i = names.begin(), end = names.end(); i != end; ++i)
            table.append_values(*i, values);
    }
    else
    {
        for (iter_type i = names.begin(), end = names.end(); i != end; ++i)
            table.set_default_values(*i, values);
    }
}

class scoped_swap_values : boost::noncopyable
{
public:
    scoped_swap_values(
        variable_table& table, const std::string& name, bool is_local
    )
        : table_(table), name_(name), is_local_(is_local)
    {
        if (is_local_)
            table_.swap_values(name_, old_values_);
    }

    ~scoped_swap_values()
    {
        if (is_local_)
            table_.swap_values(name_, old_values_);
    }

private:
    variable_table& table_;
    std::string name_;
    bool is_local_;
    string_list old_values_;
};

class scoped_push_local_variables : boost::noncopyable
{
public:
    scoped_push_local_variables(variable_table& table, variable_table& local)
        : table_(table), local_(local)
    {
        table_.push_local_variables(local_);
    }

    ~scoped_push_local_variables()
    {
        table_.pop_local_variables(local_);
    }

private:
    variable_table& table_;
    variable_table& local_;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_VARIABLE_TABLE_HPP
