// context.hpp: process context

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_CONTEXT_HPP
#define HAMIGAKI_PROCESS_CONTEXT_HPP

#include <hamigaki/process/stream_behavior.hpp>
#include <string>

namespace hamigaki { namespace process {

class context
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

    std::string work_directory() const
    {
        return work_directory_;
    }

    void work_directory(const std::string& dir)
    {
        work_directory_ = dir;
    }

private:
    stream_behavior stdin_behavior_;
    stream_behavior stdout_behavior_;
    stream_behavior stderr_behavior_;
    std::string work_directory_;
};

} } // End namespaces process, hamigaki.

#endif // HAMIGAKI_PROCESS_CONTEXT_HPP
