// linux_joystick.cpp: Joystick class for Linux

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying joystick_file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "linux_joystick.hpp"
#include <boost/dynamic_bitset.hpp>
#include <stdexcept>
#include <vector>
#include <linux/joystick.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

namespace
{

class joystick_file
{
public:
    explicit joystick_file(const char* path)
        : fd_(::open(path, O_RDONLY|O_NONBLOCK))
    {
        if (fd_ == -1)
        {
            std::string msg = "cannot open \"";
            msg += path;
            msg += '"';
            throw std::runtime_error(msg);
        }
    }

    ~joystick_file()
    {
        ::close(fd_);
    }

    boost::uint8_t axes() const
    {
        boost::uint8_t value;
        if (::ioctl(fd_, JSIOCGAXES, &value) != -1)
            return value;
        else
            return 0;
    }

    boost::uint8_t buttons() const
    {
        boost::uint8_t value;
        if (::ioctl(fd_, JSIOCGBUTTONS, &value) != -1)
            return value;
        else
            return 0;
    }

    std::string name() const
    {
        char buf[64];
        int len = ::ioctl(fd_, JSIOCGNAME(sizeof(buf)), buf);
        if (len > 0)
        {
            if (buf[len-1] == '\0')
                --len;
            return std::string(buf, len);
        }
        else
            return std::string();
    }

    bool read(js_event& ev)
    {
        return ::read(fd_, &ev, sizeof(ev)) == static_cast<ssize_t>(sizeof(ev));
    }

private:
    int fd_;

    joystick_file(const joystick_file&);
    joystick_file& operator=(const joystick_file&);
};

} // namespace

namespace hamigaki {

class linux_joystick::impl
{
public:
    explicit impl(const char* path)
        : file_(path), axes_(file_.axes()), buttons_(file_.buttons())
    {
    }

    std::size_t axes() const
    {
        return axes_.size();
    }

    boost::int16_t axis_value(std::size_t i) const
    {
        if (i < axes_.size())
            return axes_[i];
        else
            return 0;
    }

    std::size_t buttons() const
    {
        return buttons_.size();
    }

    bool button_value(std::size_t i) const
    {
        if (i < buttons_.size())
            return buttons_[i];
        else
            return false;
    }

    std::string name() const
    {
        return file_.name();
    }

    void update()
    {
        js_event ev;
        while (file_.read(ev))
        {
            unsigned type = ev.type & ~JS_EVENT_INIT;
            if (type == JS_EVENT_BUTTON)
            {
                if (ev.number < buttons_.size())
                    buttons_[ev.number] = (ev.value != 0);
            }
            else if (type == JS_EVENT_AXIS)
            {
                if (ev.number < axes_.size())
                    axes_[ev.number] = ev.value;
            }
        }
    }

private:
    joystick_file file_;
    std::vector<boost::int16_t> axes_;
    boost::dynamic_bitset<> buttons_;
};

linux_joystick::linux_joystick(const char* path) : pimpl_(new impl(path))
{
}

std::size_t linux_joystick::axes() const
{
    return pimpl_->axes();
}

boost::int16_t linux_joystick::axis_value(std::size_t i) const
{
    return pimpl_->axis_value(i);
}

std::size_t linux_joystick::buttons() const
{
    return pimpl_->buttons();
}

bool linux_joystick::button_value(std::size_t i) const
{
    return pimpl_->button_value(i);
}

std::string linux_joystick::name() const
{
    return pimpl_->name();
}

void linux_joystick::update()
{
    pimpl_->update();
}

} // End namespace hamigaki.
