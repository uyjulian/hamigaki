// variables.hpp: the table of the variables

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_VARIABLES_HPP
#define IMPL_VARIABLES_HPP

#include <boost/assert.hpp>
#include <map>
#include <vector>
#include <stdexcept>
#include <string>

class variables
{
public:
    typedef std::string key_type;
    typedef std::vector<std::string> mapped_type;
    typedef std::map<key_type,mapped_type> table_type;

    explicit variables(variables* g=0) : global_(g)
    {
    }

    void add_local(const std::string& k)
    {
        local_[k];
    }

    void add(const std::string& k)
    {
        if (global_)
            global_->add(k);
        else
            this->add_local(k);
    }

    void assign(const std::string& k, const mapped_type& m)
    {
        table_type::iterator it = local_.find(k);
        if (it != local_.end())
            it->second = m;
        else
        {
            BOOST_ASSERT(global_);
            global_->assign(k, m);
        }
    }

    void append(const std::string& k, const mapped_type& m)
    {
        table_type::iterator it = local_.find(k);
        if (it != local_.end())
        {
            mapped_type& vec = it->second;
            vec.insert(vec.end(), m.begin(), m.end());
        }
        else
        {
            BOOST_ASSERT(global_);
            global_->append(k, m);
        }
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

    void poke(const std::string& k, const mapped_type& m)
    {
        if (global_)
            global_->poke(k, m);
        else
            local_[k] = m;
    }

    const mapped_type* peek(const std::string& k) const
    {
        if (global_)
            return global_->peek(k);
        else
        {
            table_type::const_iterator it = local_.find(k);
            if (it != local_.end())
                return &(it->second);
            else
                return 0;
        }
    }

private:
    variables* global_;
    table_type local_;
};

inline void set_rule_arguments(
    variables& vars,
    const std::vector<std::string>& params,
    const std::vector<std::string>& args)
{
    typedef std::vector<std::string>::const_iterator iter_type;
    iter_type p = params.begin();
    iter_type a = args.begin();

    while (p != params.end())
    {
        std::string name;
        if ((*p != "?") && (*p != "*") && (*p != "+"))
            name = *(p++);

        std::string opt;
        if (p != params.end())
        {
            if ((*p == "?") || (*p == "*") || (*p == "+"))
                opt = *(p++);
        }

        std::vector<std::string> values;
        if (opt == "?")
        {
            if (a != args.end())
                values.push_back(*(a++));
        }
        else if (opt == "*")
        {
            values.assign(a, args.end());
            a = args.end();
        }
        else
        {
            if (a == args.end())
                throw std::runtime_error("missing a rule argument");

            if (opt == "+")
            {
                values.assign(a, args.end());
                a = args.end();
            }
            else
                values.push_back(*(a++));
        }

        if (!name.empty())
        {
            vars.add_local(name);
            vars.assign(name, values);
        }
    }
}

#endif // IMPL_VARIABLES_HPP
