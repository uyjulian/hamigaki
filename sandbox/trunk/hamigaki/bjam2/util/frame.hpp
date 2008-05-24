// frame.hpp: bjam frame

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_FRAME_HPP
#define HAMIGAKI_BJAM2_UTIL_FRAME_HPP

#include <hamigaki/bjam2/util/module.hpp>
#include <boost/noncopyable.hpp>

namespace hamigaki { namespace bjam2 {

struct frame
{
public:
    explicit frame(module& m) : module_(&m), line_(1)
    {
    }

    frame(module& m, const boost::optional<std::string>& name)
        : module_(&m), module_name_(name)
    {
    }

    void change_module(module& m, const boost::optional<std::string>& name)
    {
        module_ = &m;
        module_name_ = name;
    }

    module& current_module()
    {
        return *module_;
    }

    const module& current_module() const
    {
        return *module_;
    }

    const boost::optional<std::string>& module_name() const
    {
        return module_name_;
    }

    const boost::optional<std::string>& rule_name() const
    {
        return rule_name_;
    }

    void rule_name(const boost::optional<std::string>& name)
    {
        rule_name_ = name;
    }

    list_of_list& arguments()
    {
        return arguments_;
    }

    const std::string& filename() const
    {
        return filename_;
    }

    void filename(const std::string& name)
    {
        filename_ = name;
    }

    void swap_filename(std::string& name)
    {
        filename_.swap(name);
    }

    int line() const
    {
        return line_;
    }

    void line(int n)
    {
        line_ = n;
    }

private:
    module* module_;
    boost::optional<std::string> module_name_;
    boost::optional<std::string> rule_name_;
    list_of_list arguments_;
    std::string filename_;
    int line_;
};

class scoped_change_filename : private boost::noncopyable
{
public:
    scoped_change_filename(frame& f, const std::string& name)
        : frame_(f), name_(name)
    {
        frame_.swap_filename(name_);
    }

    ~scoped_change_filename()
    {
        frame_.swap_filename(name_);
    }

private:
    frame& frame_;
    std::string name_;
};

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_FRAME_HPP
