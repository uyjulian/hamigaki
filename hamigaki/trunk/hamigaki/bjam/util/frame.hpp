// frame.hpp: bjam frame

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_FRAME_HPP
#define HAMIGAKI_BJAM_UTIL_FRAME_HPP

#include <hamigaki/bjam/util/module.hpp>

namespace hamigaki { namespace bjam {

struct frame
{
public:
    explicit frame(module& m) : module_(&m)
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

private:
    module* module_;
    boost::optional<std::string> module_name_;
    boost::optional<std::string> rule_name_;
    list_of_list arguments_;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_FRAME_HPP
