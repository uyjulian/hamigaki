// main_window.cpp: main window implementation for bjam_win

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <hamigaki/process/child.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include "bjam_parser.hpp"
#include "combo_box.hpp"
#include "list_box.hpp"
#include "window.hpp"
#include "main_window_impl.hpp"
#include "rebar.hpp"
#include "controls.h"
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

            ::UINT_PTR index = list_box::add_string(hwnd, line);
            list_box::anchor_index(hwnd, index);
        }

    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwnd, e.what(), app_title, MB_OK);
    }

    ::PostMessage(::GetParent(hwnd), main_window::proc_end_msg, 0, 0);
}

::HWND create_combo_box(::HWND parent, int height=100)
{
    ::HINSTANCE hInstance =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::HWND hwnd = window::create_child(
        WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_VSCROLL | CBS_DROPDOWNLIST,
        0, 0, 0, height, parent, 0, hInstance
    );

    return hwnd;
}

::HWND create_toolset_combo_box(::HWND parent)
{
    ::HWND hwnd = create_combo_box(parent);

    combo_box::add_string(hwnd, "msvc-8.0");
    combo_box::add_string(hwnd, "msvc-7.1");
    combo_box::add_string(hwnd, "gcc-3.4.2");

    combo_box::select_by_index(hwnd, 0);

    return hwnd;
}

::HWND create_variant_combo_box(::HWND parent)
{
    ::HWND hwnd = create_combo_box(parent);

    combo_box::add_string(hwnd, "debug");
    combo_box::add_string(hwnd, "release");
    combo_box::add_string(hwnd, "profile");

    combo_box::select_by_index(hwnd, 0);

    return hwnd;
}

::HWND create_threading_combo_box(::HWND parent)
{
    ::HWND hwnd = create_combo_box(parent);

    combo_box::add_string(hwnd, "single");
    combo_box::add_string(hwnd, "multi");

    combo_box::select_by_index(hwnd, 0);

    return hwnd;
}

::HWND create_link_combo_box(::HWND parent)
{
    ::HWND hwnd = create_combo_box(parent);

    combo_box::add_string(hwnd, "shared");
    combo_box::add_string(hwnd, "static");

    combo_box::select_by_index(hwnd, 0);

    return hwnd;
}

void set_bjam_targets(::HWND hwnd, const std::string& filename)
{
    combo_box::reset_content(hwnd);
    combo_box::add_string(hwnd, "(all)");

    std::vector<bjam_target> targets;
    if (parse_jamfile(filename, targets))
    {
        for (std::size_t i = 0, size = targets.size(); i < size; ++i)
            combo_box::add_string(hwnd, targets[i].name.c_str());
    }

    combo_box::select_by_index(hwnd, 0);
}

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle) : handle_(handle), total_band_width_(0)
    {
        ::HINSTANCE hInstance =
            reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

        rebar_ = window::create_child(
            WS_EX_TOOLWINDOW, REBARCLASSNAME, "",
            WS_CLIPCHILDREN | RBS_VARHEIGHT | RBS_BANDBORDERS | CCS_NODIVIDER,
            0, 0, 0, 0, handle_, IDC_REBAR, hInstance
        );

        ::REBARINFO bar_info;
        std::memset(&bar_info, 0, sizeof(bar_info));
        bar_info.cbSize = sizeof(bar_info);
        rebar::bar_info(rebar_, bar_info);

        toolset_ = create_toolset_combo_box(rebar_);
        add_band(toolset_, "Toolset:", 170);

        variant_ = create_variant_combo_box(rebar_);
        add_band(variant_, "Variant:", 140);

        threading_ = create_threading_combo_box(rebar_);
        add_band(threading_, "Threading:", 150);

        link_ = create_link_combo_box(rebar_);
        add_band(link_, "Link:", 120);

        runtime_link_ = create_link_combo_box(rebar_);
        add_band(runtime_link_, "Runtime link:", 170);

        target_ = create_combo_box(rebar_, 200);
        add_band(target_, "Target:", 180);

        ::RECT rect = calc_log_list_rect();

        log_list_ = window::create_child(
            WS_EX_CLIENTEDGE, "LISTBOX", "",
            WS_HSCROLL | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
            rect.left, rect.top,
            rect.right-rect.left, rect.bottom-rect.top,
            handle_, 0, hInstance
        );
        list_box::enable_horizontal_scroll_bar(log_list_);
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

        set_bjam_targets(target_, filename);

        std::string title;
        title += filename;
        title += " - ";
        title += app_title;
        window::set_text(handle_, title);

        list_box::reset_content(log_list_);
        enable_menu_item(ID_BUILD_RUN);
        enable_menu_item(ID_BUILD_CLEAN);
    }

    void run()
    {
        excute(false);
    }

    void clean()
    {
        excute(true);
    }

    void stop()
    {
        bjam_proc_->terminate();
        wait();
    }

    void wait()
    {
        if (thread_.get())
        {
            thread_->join();
            thread_.reset();
            bjam_proc_.reset();

            enable_menu_item(ID_BUILD_RUN);
            enable_menu_item(ID_BUILD_CLEAN);
            disable_menu_item(ID_BUILD_STOP);
        }
    }

    bool running() const
    {
        return thread_.get() != 0;
    }

    void update_size()
    {
        ::SIZE client_size = window::client_area_size(handle_);

        ::SendMessage(
            rebar_, WM_SIZE, SIZE_RESTORED,
            MAKELPARAM(client_size.cx, client_size.cy)
        );

        ::RECT rect = calc_log_list_rect();

        ::MoveWindow(log_list_, rect.left, rect.top,
            rect.right-rect.left, rect.bottom-rect.top, TRUE);
    }

private:
    ::HWND handle_;
    ::HWND rebar_;
    ::HWND toolset_;
    ::HWND variant_;
    ::HWND threading_;
    ::HWND link_;
    ::HWND runtime_link_;
    ::HWND target_;
    ::HWND log_list_;
    fs::path jamfile_;
    boost::scoped_ptr<proc::child> bjam_proc_;
    boost::scoped_ptr<boost::thread> thread_;
    ::UINT total_band_width_;

    void add_band(::HWND hwnd, const char* text, ::UINT width)
    {
        ::REBARBANDINFOA band_info;
        std::memset(&band_info, 0, sizeof(band_info));
        band_info.cbSize = sizeof(band_info);
        band_info.fMask =
            RBBIM_TEXT | RBBIM_STYLE | RBBIM_CHILD |
            RBBIM_CHILDSIZE | RBBIM_SIZE;
        band_info.fStyle = RBBS_CHILDEDGE;
        band_info.lpText = const_cast<char*>(text);
        band_info.hwndChild = hwnd;
        band_info.cxMinChild = 0;
        band_info.cyMinChild = window::height(hwnd);
        band_info.cx = width;

        ::UINT client_width =
            static_cast< ::UINT>(window::client_area_width(handle_));

        total_band_width_ += width;
        if (total_band_width_ > client_width)
        {
            total_band_width_ = width;
            band_info.fStyle |= RBBS_BREAK;
        }

        rebar::add_band(rebar_, band_info);
    }

    ::RECT calc_log_list_rect()
    {
        ::SIZE client_size = window::client_area_size(handle_);
        ::LONG rebar_height = window::height(rebar_);

        ::RECT rect;
        rect.left = 0;
        rect.top = rebar_height;
        rect.right = client_size.cx;
        rect.bottom = client_size.cy;
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

    void excute(bool clean)
    {
        std::string bjam = search_path("bjam.exe");

        std::vector<std::string> args;
        args.push_back("bjam");
        args.push_back("--v2");
        args.push_back("toolset=" + window::get_text(toolset_));
        args.push_back("variant=" + window::get_text(variant_));
        args.push_back("threading=" + window::get_text(threading_));
        args.push_back("link=" + window::get_text(link_));
        args.push_back("runtime-link=" + window::get_text(runtime_link_));

        if (clean)
            args.push_back("--clean");

        if (combo_box::selected_index(target_) != 0)
            args.push_back(window::get_text(target_));

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
        disable_menu_item(ID_BUILD_CLEAN);
        enable_menu_item(ID_BUILD_STOP);

        ::SetFocus(log_list_);
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

void main_window::clean()
{
    pimpl_->clean();
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
