//  main_window.cpp: main window implementation for bjam_win

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#include <hamigaki/process/child.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include "main_window_impl.hpp"
#include <commctrl.h>
#include "menus.h"

namespace proc = hamigaki::process;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

namespace
{

std::string search_path(const std::string& name)
{
    char buf[MAX_PATH];
    char* filename;
    ::DWORD n = ::SearchPathA(0, name.c_str(), 0, sizeof(buf), buf, &filename);
    if (n == 0)
    {
        std::string msg;
        msg += "cannot found ";
        msg += name;
        throw std::runtime_error(msg);
    }
    return buf;
}

void bjam_thread(::HWND hwnd, proc::pipe_source src)
{
    ::SendMessage(hwnd, LB_RESETCONTENT, 0, 0);

    io::stream<proc::pipe_source> is(src);
    std::string line;
    while (std::getline(is, line))
    {
        if (!line.empty() && (line[line.size()-1] == '\r'))
            line.resize(line.size()-1);

        ::SendMessage(hwnd, LB_ADDSTRING, 0,
            reinterpret_cast< ::LPARAM>(line.c_str())
        );
    }
}

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle) : handle_(handle)
    {
        ::HINSTANCE hInstance =
            reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

        ::RECT rect;
        ::GetClientRect(handle_, &rect);

        log_list_ = ::CreateWindowEx(
            WS_EX_CLIENTEDGE, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
            LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
            0, 0,
            rect.right-rect.left, rect.bottom-rect.top,
            handle_, 0, hInstance, 0
        );
    }

    ~impl()
    {
        try
        {
            if (running())
                stop();
        }
        catch (...)
        {
        }
    }

    void open(const std::string& filename)
    {
        jamfile_ = fs::path(filename, fs::native);
    }

    void run()
    {
        std::string bjam = search_path("bjam.exe");

        std::vector<std::string> args;
        args.push_back("bjam");
        args.push_back("--v2");

        proc::context ctx;
        ctx.stdin_behavior(proc::silence_stream());
        ctx.stdout_behavior(proc::capture_stream());
        ctx.stderr_behavior(proc::silence_stream());

        // FIXME
        ::SetCurrentDirectory(
            jamfile_.branch_path().native_directory_string().c_str());

        bjam_proc_.reset(new proc::child(bjam, args, ctx));

        proc::pipe_source src = bjam_proc_->stdout_source();

        thread_.reset(new boost::thread(
            boost::bind(&bjam_thread, log_list_, src)));
    }

    void stop()
    {
        bjam_proc_->terminate();
        thread_->join();
        thread_.reset();
        bjam_proc_.reset();
    }

    bool running() const
    {
        return thread_.get() != 0;
    }

    void update_size()
    {
        ::RECT rect;
        ::GetClientRect(handle_, &rect);

        ::MoveWindow(log_list_, 0, 0,
            rect.right-rect.left, rect.bottom-rect.top, TRUE);
    }

private:
    ::HWND handle_;
    ::HWND log_list_;
    fs::path jamfile_;
    boost::scoped_ptr<proc::child> bjam_proc_;
    boost::scoped_ptr<boost::thread> thread_;
};

main_window::main_window(::HWND handle) : pimpl_(new impl(handle))
{
}

void main_window::open(const std::string& filename)
{
    pimpl_->open(filename);
}

void main_window::run()
{
    pimpl_->run();
}

void main_window::stop()
{
    pimpl_->stop();
}

bool main_window::running() const
{
    return pimpl_->running();
}

void main_window::update_size()
{
    pimpl_->update_size();
}
