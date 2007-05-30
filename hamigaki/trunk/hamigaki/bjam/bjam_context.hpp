// bjam_context.hpp: the context information for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_BJAM_CONTEXT_HPP
#define HAMIGAKI_BJAM_BJAM_CONTEXT_HPP

#include <hamigaki/bjam/util/frame.hpp>
#include <boost/noncopyable.hpp>

namespace hamigaki { namespace bjam {

class context : private boost::noncopyable
{
public:
    typedef rule_table::rule_def_ptr rule_def_ptr;
    typedef variable_table::iterator variable_iterator;

    context()
    {
        frames_.push_back(frame(root_module_));
    }

    frame& current_frame()
    {
        return frames_.back();
    }

private:
    module root_module_;
    std::map<std::string,module> modules_;
    std::vector<frame> frames_;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_BJAM_CONTEXT_HPP
