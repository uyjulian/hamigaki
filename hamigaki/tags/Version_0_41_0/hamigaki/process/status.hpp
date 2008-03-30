// status.hpp: process exit status

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_STATUS_HPP
#define HAMIGAKI_PROCESS_STATUS_HPP

namespace hamigaki { namespace process {

class status
{
public:
    enum type { exited, signaled, stopped, continued };

    explicit status(unsigned code=0)
        : type_(exited), code_(code), dumped_core_(false)
    {
    }

    status(type t, unsigned code, bool core=false)
        : type_(t), code_(code), dumped_core_(core)
    {
    }

    type get_type() const
    {
        return type_;
    }

    unsigned code() const
    {
        return code_;
    }

    bool dumped_core() const
    {
        return dumped_core_;
    }

private:
    type type_;
    unsigned code_;
    bool dumped_core_;
};

} } // End namespaces process, hamigaki.

#endif // HAMIGAKI_PROCESS_STATUS_HPP
