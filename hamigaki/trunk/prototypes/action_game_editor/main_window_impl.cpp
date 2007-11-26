// main_window.cpp: main window implementation for action_game_editor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "char_select_window.hpp"
#include "map_edit_window.hpp"
#include <boost/noncopyable.hpp>

namespace
{

class scoped_window_class : private boost::noncopyable
{
public:
    scoped_window_class(::HINSTANCE hInstance, ::ATOM id)
        : hInstance_(hInstance), id_(id)
    {
    }

    ~scoped_window_class()
    {
        ::UnregisterClassA(MAKEINTATOM(id_), hInstance_);
    }

    ::ATOM get() const
    {
        return id_;
    }

private:
    ::HINSTANCE hInstance_;
    ::ATOM id_;
};

class scoped_window : private boost::noncopyable
{
public:
    explicit scoped_window(::HWND handle) : handle_(handle)
    {
    }

    ~scoped_window()
    {
        if (handle_ != 0)
            ::DestroyWindow(handle_);
    }

    ::HWND get() const
    {
        return handle_;
    }

    ::HWND release()
    {
        ::HWND tmp = handle_;
        handle_ = 0;
        return tmp;
    }

private:
    ::HWND handle_;
};

::HINSTANCE get_parent_module(::HWND hwnd)
{
    return reinterpret_cast< ::HINSTANCE>(
        GetWindowLongPtr(hwnd, GWLP_HINSTANCE)
    );
}

int get_window_width(::HWND hwnd)
{
    ::RECT r;
    ::GetWindowRect(hwnd, &r);
    return r.right - r.left;
}

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle)
        , hInstance_(get_parent_module(handle_))
        , select_class_(
            hInstance_,
            register_char_select_window_class(hInstance_)
        )
        , map_class_(
            hInstance_,
            register_map_edit_window_class(hInstance_)
        )
    {
        scoped_window select_window(
            create_char_select_window(
                handle_, char_select_id,
                hInstance_, select_class_.get()
            )
        );

        scoped_window map_window(
            create_map_edit_window(
                handle_, map_edit_id,
                get_window_width(select_window.get()) + 2,
                hInstance_, map_class_.get()
            )
        );

        select_window_ = select_window.release();
        map_window_ = map_window.release();
    }

    ~impl()
    {
    }

    void update_size()
    {
        int left = get_window_width(select_window_) + 2;

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        ::MoveWindow(
            map_window_, left, 0,
            cr.right - cr.left -left, cr.bottom - cr.top, TRUE);
    }

    void update_selected_char()
    {
        char c = get_selected_char(select_window_);
        map_edit_window_select_char(map_window_, c);
    }

    void load_stage(const std::string& filename)
    {
        map_edit_window_load(map_window_, filename);
    }

private:
    ::HWND handle_;
    ::HINSTANCE hInstance_;
    scoped_window_class select_class_;
    scoped_window_class map_class_;
    ::HWND select_window_;
    ::HWND map_window_;
};

main_window::main_window(::HWND handle) : pimpl_(new impl(handle))
{
}

main_window::~main_window()
{
}

void main_window::update_size()
{
    pimpl_->update_size();
}

void main_window::update_selected_char()
{
    pimpl_->update_selected_char();
}

void main_window::load_stage(const std::string& filename)
{
    pimpl_->load_stage(filename);
}
