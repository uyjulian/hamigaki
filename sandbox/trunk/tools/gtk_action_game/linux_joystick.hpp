// linux_joystick.hpp: Joystick class for Linux

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_LINUX_JOYSTICK_HPP
#define HAMIGAKI_LINUX_JOYSTICK_HPP

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

namespace hamigaki {

class linux_joystick
{
public:
    explicit linux_joystick(const char* path);
    std::size_t axes() const;
    boost::int16_t axis_value(std::size_t i) const;
    std::size_t buttons() const;
    bool button_value(std::size_t i) const;
    std::string name() const;
    void update();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_LINUX_JOYSTICK_HPP
