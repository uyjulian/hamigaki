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
#include "combo_box.hpp"
#include "list_box.hpp"
#include "window.hpp"
#include "main_window_impl.hpp"
#include "rebar.hpp"
#include "menus.h"

namespace proc = hamigaki::process;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

namespace
{

const char app_title[] = "bjam for Windows";

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
    try
    {
        list_box::reset_content(hwnd);

        io::stream<proc::pipe_source> is(src);
        std::string line;
        while (std::getline(is, line))
        {
            if (!line.empty() && (line[line.size()-1] == '\r'))
                line.resize(line.size()-1);

            list_box::add_string(hwnd, line);
        }

    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwnd, e.what(), app_title, MB_OK);
    }

    ::PostMessage(::GetParent(hwnd), main_window::proc_end_msg, 0, 0);
}

::HWND create_toolset_combo_box(::HWND parent)
{
    ::HINSTANCE hInstance =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::HWND hwnd = ::CreateWindowExA(
        WS_EX_CLIENTEDGE, "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CBS_DROPDOWNLIST,
        0, 0, 0, 100,
        parent, 0, hInstance, 0
    );

    combo_box::add_string(hwnd, "msvc-8.0");
    combo_box::add_string(hwnd, "msvc-7.1");
    combo_box::add_string(hwnd, "gcc-3.4.2");

    combo_box::select_by_index(hwnd, 0);

    return hwnd;
}

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle) : handle_(handle)
    {
        ::HINSTANCE hInstance =
            reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

        rebar_ = ::CreateWindowExA(
            WS_EX_TOOLWINDOW, REBARCLASSNAME, "",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
            RBS_VARHEIGHT | CCS_NODIVIDER,
            0, 0, 0, 0,
            handle_, 0, hInstance, 0
        );

        ::REBARINFO bar_info;
        std::memset(&bar_info, 0, sizeof(bar_info));
        bar_info.cbSize = sizeof(bar_info);
        rebar::bar_info(rebar_, bar_info);

        toolset_ = create_toolset_combo_box(rebar_);

        ::RECT rect;
        ::GetWindowRect(toolset_, &rect);

        ::REBARBANDINFOA band_info;
        std::memset(&band_info, 0, sizeof(band_info));
        band_info.cbSize = sizeof(band_info);
        band_info.fMask =
            RBBIM_TEXT | RBBIM_STYLE | RBBIM_CHILD |
            RBBIM_CHILDSIZE | RBBIM_SIZE;
        band_info.fStyle = RBBS_CHILDEDGE;
        band_info.lpText = "Toolset:";
        band_info.hwndChild = toolset_;
        band_info.cxMinChild = 0;
        band_info.cyMinChild = rect.bottom - rect.top;
        band_info.cx = 200;

        rebar::add_band(rebar_, band_info);

        rect = calc_log_list_size();

        log_list_ = ::CreateWindowExA(
            WS_EX_CLIENTEDGE, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_CLIPSIBLINGS |
            LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
            rect.left, rect.top,
            rect.right-rect.left, rect.bottom-rect.top,
            handle_, 0, hInstance, 0
        );
        subclass_list_box(log_list_);
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
        if (running())
            stop();

        jamfile_ = fs::path(filename, fs::native);

        std::string title;
        title += filename;
        title += " - ";
        title += app_title;
        window::set_text(handle_, title);

        enable_menu_item(ID_BUILD_RUN);
    }

    void run()
    {
        std::string bjam = search_path("bjam.exe");

        std::vector<std::string> args;
        args.push_back("bjam");
        args.push_back("--v2");
        args.push_back("toolset=" + window::get_text(toolset_));

        proc::context ctx;
        ctx.stdin_behavior(proc::silence_stream());
        ctx.stdout_behavior(proc::capture_stream());
        ctx.stderr_behavior(proc::redirect_stream_to_stdout());

        ctx.work_directory(jamfile_.branch_path().native_directory_string());

        bjam_proc_.reset(new proc::child(bjam, args, ctx));

        proc::pipe_source src = bjam_proc_->stdout_source();

        thread_.reset(new boost::thread(
            boost::bind(&bjam_thread, log_list_, src)));

        disable_menu_item(ID_BUILD_RUN);
        enable_menu_item(ID_BUILD_STOP);
    }

    void stop()
    {
        bjam_proc_->terminate();
        wait();
    }

    void wait()
    {
        thread_->join();
        thread_.reset();
        bjam_proc_.reset();

        enable_menu_item(ID_BUILD_RUN);
        disable_menu_item(ID_BUILD_STOP);
    }

    bool running() const
    {
        return thread_.get() != 0;
    }

    void update_size()
    {
        ::RECT rect = calc_log_list_size();

        ::MoveWindow(log_list_, rect.left, rect.top,
            rect.right-rect.left, rect.bottom-rect.top, TRUE);
    }

private:
    ::HWND handle_;
    ::HWND rebar_;
    ::HWND toolset_;
    ::HWND log_list_;
    fs::path jamfile_;
    boost::scoped_ptr<proc::child> bjam_proc_;
    boost::scoped_ptr<boost::thread> thread_;

    ::RECT calc_log_list_size()
    {
        ::RECT rect;
        ::GetClientRect(handle_, &rect);

        ::RECT bar;
        ::GetWindowRect(rebar_, &bar);

        rect.top += (bar.bottom - bar.top);

        return rect;
    }

    void enable_menu_item(::UINT id)
    {
        ::EnableMenuItem(::GetMenu(handle_), id, MF_BYCOMMAND|MF_ENABLED);
    }

    void disable_menu_item(::UINT id)
    {
        ::EnableMenuItem(::GetMenu(handle_), id, MF_BYCOMMAND|MF_GRAYED);
    }
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

void main_window::wait()
{
    pimpl_->wait();
}

bool main_window::running() const
{
    return pimpl_->running();
}

void main_window::update_size()
{
    pimpl_->update_size();
}
