//  variables.hpp: the table of the variables

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_VARIABLES_HPP
#define IMPL_VARIABLES_HPP

#include <boost/assert.hpp>
#include <map>
#include <vector>
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

private:
    variables* global_;
    table_type local_;
};

#endif // IMPL_VARIABLES_HPP
