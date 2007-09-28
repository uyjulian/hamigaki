// routine_state.hpp: the state of the moving routine

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ROUTINE_STATE_HPP
#define ROUTINE_STATE_HPP

#include "routine_base.hpp"

class routine_state
{
public:
    routine_state(
        routine_type::self& self,
        rect& r, velocity& v, sprite_form& form, input_command& cmd,
        acceleration& a
    )
        : self_(self), r_(r), v_(v), form_(form), cmd_(cmd), a_(a)
    {
    }

    void yield()
    {
        boost::tie(r_,v_,form_,cmd_) = self_.yield(a_,form_);
    }

    void wait(std::size_t frames)
    {
        for (std::size_t i = 0; i < frames; ++i)
            yield();
    }

    void accelerate(float vx)
    {
        if ((form_.options & sprite_options::back) != 0)
            a_.ax = -vx;
        else
            a_.ax = vx;
    }

    void go_forward(float dx)
    {
        a_.ax = -v_.vx;
        a_.ay = -v_.vy;

        if ((form_.options & sprite_options::back) != 0)
            a_.ax -= dx;
        else
            a_.ax += dx;

        yield();

        a_.ax = 0.0f;
        a_.ay = 0.0f;
    }

    void go_backward(float dx)
    {
        this->go_forward(-dx);
    }

    void turn(float vx = 0.0f)
    {
        if ((form_.options & sprite_options::back) != 0)
        {
            a_.ax = vx;
            form_.options &= ~sprite_options::back;
        }
        else
        {
            a_.ax = -vx;
            form_.options |= sprite_options::back;
        }
    }

    void stop()
    {
        a_.ax = -v_.vx;
    }

private:
    routine_type::self& self_;
    rect& r_;
    velocity& v_;
    sprite_form& form_;
    input_command& cmd_;
    acceleration& a_;
};

#endif // ROUTINE_STATE_HPP
