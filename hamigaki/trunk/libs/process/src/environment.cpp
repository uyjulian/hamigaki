// environment.cpp: environment variables

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#define HAMIGAKI_PROCESS_SOURCE
#define NOMINMAX
#include <boost/config.hpp>
#include <hamigaki/process/environment.hpp>
#include <boost/scoped_array.hpp>
#include <map>

#include <hamigaki/detail/environment.hpp>

namespace hamigaki { namespace process {

namespace
{

typedef hamigaki::detail::environment_type table_type;

std::size_t calc_environment_size(const table_type& t)
{
    typedef table_type::const_iterator iter_type;

    std::size_t size = 0;
    for (iter_type i = t.begin(), end = t.end(); i != end; ++i)
        size += (i->first.size() + i->second.size() + 2);
    return ++size;
}

class env_data
{
public:
    void reset(const table_type& t)
    {
        typedef table_type::const_iterator iter_type;

        std::size_t size = calc_environment_size(t);
        boost::scoped_array<char> tmp(new char[size]);

        char* p = tmp.get();
        for (iter_type i = t.begin(), end = t.end(); i != end; ++i)
        {
            const std::string& name = i->first;
            const std::string& value = i->second;
            p += name.copy(p, name.size());
            *(p++) = '=';
            p += value.copy(p, value.size());
            *(p++) = '\0';
        }
        *p = '\0';
        data_.swap(tmp);
    }

    char* data()
    {
        return data_.get();
    }

private:
    boost::scoped_array<char> data_;
};

} // namespace

class environment::impl
{
public:
    impl()
    {
        hamigaki::detail::get_environment_variables(table_);
    }

    void clear()
    {
        table_.clear();
    }

    void set(const std::string& name, const std::string& value)
    {
        table_[name] = value;
    }

    void unset(const std::string& name)
    {
        table_.erase(name);
    }

    const char* get(const std::string& name) const
    {
        table_type::const_iterator i = table_.find(name);
        if (i != table_.end())
            return i->second.c_str();
        else
            return 0;
    }

    std::size_t size() const
    {
        return table_.size();
    }

    char* data()
    {
        data_.reset(table_);
        return data_.data();
    }

private:
    table_type table_;
    env_data data_;
};

environment::environment() : pimpl_(new impl)
{
}

environment::~environment()
{
}

void environment::clear()
{
    pimpl_->clear();
}

void environment::set(const std::string& name, const std::string& value)
{
    pimpl_->set(name, value);
}

void environment::unset(const std::string& name)
{
    pimpl_->unset(name);
}

const char* environment::get(const std::string& name) const
{
    return pimpl_->get(name);
}

std::size_t environment::size() const
{
    return pimpl_->size();
}

char* environment::data()
{
    return pimpl_->data();
}

} } // End namespaces process, hamigaki.
