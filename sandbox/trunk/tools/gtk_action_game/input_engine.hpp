// input_engine.hpp: input engine for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef INPUT_ENGINE_HPP
#define INPUT_ENGINE_HPP

#include <boost/shared_ptr.hpp>
#include <string>

struct input_command
{
    float x;
    float y;
    bool jump;
    bool dash;
    bool punch;
    bool reset;

    input_command()
        : x(0.0f), y(0.0f)
        , jump(false), dash(false), punch(false), reset(false)
    {
    }
};

class input_engine
{
public:
    explicit input_engine(void* widget);
    ~input_engine();

    input_command operator()();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // INPUT_ENGINE_HPP
