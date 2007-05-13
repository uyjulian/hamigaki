// stream_behavior.hpp: stream behavior

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_STREAM_BEHAVIOR_HPP
#define HAMIGAKI_PROCESS_STREAM_BEHAVIOR_HPP

namespace hamigaki { namespace process {

class stream_behavior
{
public:
    enum type { capture, close, inherit, redirect_to_stdout, silence };

    stream_behavior() : type_(inherit)
    {
    }

    explicit stream_behavior(type t) : type_(t)
    {
    }

    type get_type() const
    {
        return type_;
    }

private:
    type type_;
};

inline stream_behavior capture_stream()
{
    return stream_behavior(stream_behavior::capture);
}

inline stream_behavior close_stream()
{
    return stream_behavior(stream_behavior::close);
}

inline stream_behavior inherit_stream()
{
    return stream_behavior(stream_behavior::inherit);
}

inline stream_behavior redirect_stream_to_stdout()
{
    return stream_behavior(stream_behavior::redirect_to_stdout);
}

inline stream_behavior silence_stream()
{
    return stream_behavior(stream_behavior::silence);
}

} } // End namespaces process, hamigaki.

#endif // HAMIGAKI_PROCESS_STREAM_BEHAVIOR_HPP
