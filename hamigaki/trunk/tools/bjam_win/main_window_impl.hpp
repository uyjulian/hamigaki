// main_window.hpp: main window implementation for bjam_win

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAIN_WINDOW_IMPL_HPP
#define MAIN_WINDOW_IMPL_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include <windows.h>

class main_window
{
public:
    static const ::UINT proc_end_msg = WM_APP;

    explicit main_window(::HWND handle);

    void open(const std::string& filename);
    void run();
    void clean();
    void stop();
    void wait();
    bool running() const;

    void update_size();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // MAIN_WINDOW_IMPL_HPP
