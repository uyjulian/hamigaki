// bjam_target.hpp: the target information for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef IMPL_BJAM_TARGET_HPP
#define IMPL_BJAM_TARGET_HPP

#include <boost/operators.hpp>
#include <string>
#include <vector>

struct bjam_target : boost::totally_ordered<bjam_target>
{
    std::string type;
    std::string name;
    std::vector<std::string> requirements;
    std::vector<std::string> usage_requirements;

    bool operator<(const bjam_target& rhs) const
    {
        return name < rhs.name;
    }

    bool operator==(const bjam_target& rhs) const
    {
        return name == rhs.name;
    }

    bjam_target& set_typed_name(const std::string& s)
    {
        type = s;
        name = s;
        return *this;
    }
};

#endif // IMPL_BJAM_TARGET_HPP
