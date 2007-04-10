//  ipc_map.hpp: interprocess communication mapper

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_IPC_MAP_HPP
#define HAMIGAKI_PROCESS_IPC_MAP_HPP

#include <hamigaki/process/stream_behavior.hpp>

namespace hamigaki { namespace process {

class ipc_map
{
public:
    stream_behavior stdin_behavior() const
    {
        return stdin_behavior_;
    }

    void stdin_behavior(const stream_behavior& b)
    {
        stdin_behavior_ = b;
    }

    stream_behavior stdout_behavior() const
    {
        return stdout_behavior_;
    }

    void stdout_behavior(const stream_behavior& b)
    {
        stdout_behavior_ = b;
    }

    stream_behavior stderr_behavior() const
    {
        return stderr_behavior_;
    }

    void stderr_behavior(const stream_behavior& b)
    {
        stderr_behavior_ = b;
    }

private:
    stream_behavior stdin_behavior_;
    stream_behavior stdout_behavior_;
    stream_behavior stderr_behavior_;
};

} } // End namespaces process, hamigaki.

#endif // HAMIGAKI_PROCESS_IPC_MAP_HPP
