// main_window.cpp: main window implementation for action_game_editor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "char_class_io.hpp"
#include "char_select_window.hpp"
#include "game_project_io.hpp"
#include "map_edit_window.hpp"
#include "msg_utilities.hpp"
#include "stage_map_load.hpp"
#include "stage_map_save.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <map>
#include <set>

#include <hamigaki/system/windows_error.hpp>
#include "popup_menus.h"

using hamigaki::system::windows_error;

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;

namespace
{

void load_char_classes(
    const fs::path& dir,
    std::set<game_character_class>& char_table,
    std::map<hamigaki::uuid,fs::path>& filename_table)
{
    fs::directory_iterator it(dir);
    fs::directory_iterator end;

    std::set<game_character_class> tmp;
    std::map<hamigaki::uuid,fs::path> tmp2;
    std::locale loc("");
    for ( ; it != end; ++it)
    {
        const fs::path& ph = it->path();
        const std::string& leaf = ph.leaf();
        if (algo::iends_with(leaf, ".agc-yh", loc))
        {
            const game_character_class& c =
                load_character_class(ph.file_string().c_str());
            tmp.insert(c);
            tmp2[c.id] = ph;
        }
    }

    char_table.swap(tmp);
    filename_table.swap(tmp2);
}

void save_char_classes(
    const fs::path& dir,
    std::set<game_character_class>& char_table,
    const std::map<hamigaki::uuid,fs::path>& filename_table)
{
    typedef std::set<game_character_class>::iterator char_iter;
    typedef std::map<hamigaki::uuid,fs::path>::const_iterator name_iter;

    for (char_iter i = char_table.begin(), end = char_table.end(); i!=end; ++i)
    {
        game_character_class& c = *i;

        if (c.modified)
        {
            name_iter pos = filename_table.find(c.id);
            BOOST_ASSERT(pos != filename_table.end());
            const fs::path& ph = pos->second;
            save_character_class(ph.file_string().c_str(), c);
            c.modified = false;
        }
    }
}

void load_maps(const fs::path& dir, std::map<std::string,stage_map>& table)
{
    fs::directory_iterator it(dir);
    fs::directory_iterator end;

    std::map<std::string,stage_map> tmp;
    std::locale loc("");
    for ( ; it != end; ++it)
    {
        const fs::path& ph = it->path();
        const std::string& leaf = ph.leaf();
        if (algo::iends_with(leaf, ".agm-yh", loc))
        {
            stage_map& m = tmp[leaf];
            load_map_from_binary(ph.file_string().c_str(), m);
            m.modified = false;
        }
    }

    table.swap(tmp);
}

void save_maps(const fs::path& dir, std::map<std::string,stage_map>& table)
{
    typedef std::map<std::string,stage_map>::iterator iter_type;

    for (iter_type i = table.begin(), end = table.end(); i != end; ++i)
    {
        stage_map& m = i->second;
        if (m.modified)
        {
            const fs::path& ph = dir / i->first;
            save_map_to_binary(ph.file_string().c_str(), m);
            m.modified = false;
        }
    }
}

void setup_map_list(::HWND hwnd, const fs::path& dir)
{
    send_msg(hwnd, LB_RESETCONTENT);

    fs::directory_iterator it(dir);
    fs::directory_iterator end;

    std::locale loc("");
    for ( ; it != end; ++it)
    {
        const fs::path& ph = it->path();
        const std::string& leaf = ph.leaf();
        if (algo::iends_with(leaf, ".agm-yh", loc))
            send_msg(hwnd, LB_ADDSTRING, 0, leaf);
    }
}

::HINSTANCE get_parent_module(::HWND hwnd)
{
    return reinterpret_cast< ::HINSTANCE>(
        GetWindowLongPtr(hwnd, GWLP_HINSTANCE)
    );
}

class menu : private boost::noncopyable
{
public:
    menu(::HINSTANCE hInstance, const char* name)
        : handle_(::LoadMenuA(hInstance, name))
    {
        if (handle_ == 0)
            throw windows_error(::GetLastError(), "failed LoadMenuA()");
    }

    ~menu()
    {
        ::DestroyMenu(handle_);
    }

    void popup(int index, int x, int y, ::HWND hwnd)
    {
        ::TrackPopupMenuEx(
            ::GetSubMenu(handle_, index), TPM_RIGHTBUTTON, x, y, hwnd, 0);
    }

private:
    ::HMENU handle_;
};

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle), modified_(false)
        , menu_(get_parent_module(handle_), MAKEINTRESOURCE(HAMIGAKI_IDR_POPUP))
        , char_sel_window_(0), map_window_(0), map_sel_window_(0)
    {
    }

    ~impl()
    {
    }

    void update_size()
    {
        if (map_window_ == 0)
            return;

        ::RECT r;
        ::GetWindowRect(char_sel_window_, &r);
        int char_sel_width = r.right - r.left;
        int char_sel_height = r.bottom - r.top;

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        ::MoveWindow(
            map_window_, char_sel_width + 2, 0,
            cr.right - (char_sel_width + 2), cr.bottom, TRUE);

        ::MoveWindow(
            map_sel_window_, 0, char_sel_height + 2,
            char_sel_width, cr.bottom - (char_sel_height + 2), TRUE);
    }

    void update_selected_char()
    {
        const hamigaki::uuid& c = get_selected_char(char_sel_window_);
        map_edit_window_select_char(map_window_, c);
    }

    void new_project(const std::string& filename, const game_project& proj)
    {
        project_ = proj;

        const fs::path& dir = fs::path(filename).branch_path();

        if (map_window_ == 0)
            create_child_windows();

        load_char_classes(dir, char_table_, filename_table_);
        load_maps(dir, map_table_);
        setup_map_list(map_sel_window_, dir);
        map_edit_window_set(map_window_, 0);

        project_file_ = filename;
        modified_ = true;
    }

    void close_project()
    {
        ::DestroyWindow(map_sel_window_);
        map_sel_window_ = 0;

        ::DestroyWindow(map_window_);
        map_window_ = 0;

        ::DestroyWindow(char_sel_window_);
        char_sel_window_ = 0;

        map_table_.clear();
        filename_table_.clear();
        char_table_.clear();
        project_file_.clear();
        project_ = game_project();

        modified_ = false;
    }

    void load_project(const std::string& filename)
    {
        this->new_project(filename, load_game_project(filename.c_str()));
        modified_ = false;
    }

    void save_project()
    {
        const fs::path& dir = fs::path(project_file_).branch_path();

        save_game_project(project_file_.c_str(), project_);

        save_char_classes(dir, char_table_, filename_table_);
        save_maps(dir, map_table_);

        modified_ = false;
    }

    bool new_stage(const std::string& filename, int width, int height)
    {
        fs::path ph = fs::path(project_file_).branch_path() / filename;
        if (exists(ph))
            return false;

        if (map_window_ == 0)
            create_child_windows();

        if (map_edit_window_select_modified(map_window_))
            modified_ = true;

        int index = send_msg(map_sel_window_, LB_ADDSTRING, 0, filename);

        send_msg(map_sel_window_, LB_SETCURSEL, index);

        stage_map& m = map_table_[filename];
        m.width = width * 32;
        m.height = height * 32;
        m.modified = true;

        map_edit_window_set(map_window_, &m);

        return true;
    }

    int stage_count() const
    {
        return map_table_.size();
    }

    std::string stage_name() const
    {
        int index = send_msg(map_sel_window_, LB_GETCURSEL);
        if (index == -1)
            map_edit_window_set(map_window_, 0);

        int size = send_msg(map_sel_window_, LB_GETTEXTLEN, index);

        boost::scoped_array<char> buf(new char[size+1]);
        send_msg_with_ptr(map_sel_window_, LB_GETTEXT, index, buf.get());

        return std::string(buf.get(), size);
    }

    void delete_stage()
    {
        int index = send_msg(map_sel_window_, LB_GETCURSEL);
        if (index != -1)
        {
            map_edit_window_set(map_window_, 0);
            map_table_.erase(stage_name());
            send_msg(map_sel_window_, LB_DELETESTRING, index);
        }
    }

    void change_stage()
    {
        if (map_edit_window_select_modified(map_window_))
            modified_ = true;

        map_edit_window_set(map_window_, &map_table_[stage_name()]);
    }

    bool modified()
    {
        return modified_ || map_edit_window_select_modified(map_window_);
    }

    void track_popup_menu(::HWND hwnd, int x, int y)
    {
        if ((x == -1) && (y == -1))
        {
            ::RECT r;
            ::GetWindowRect(hwnd, &r);
            x = r.left;
            y = r.top;
        }

        if (hwnd == map_sel_window_)
            menu_.popup(0, x, y, handle_);
    }

private:
    ::HWND handle_;
    game_project project_;
    std::string project_file_;
    std::set<game_character_class> char_table_;
    std::map<hamigaki::uuid,fs::path> filename_table_;
    std::map<std::string,stage_map> map_table_;
    ::HWND char_sel_window_;
    ::HWND map_window_;
    ::HWND map_sel_window_;
    bool modified_;
    menu menu_;

    void create_child_windows()
    {
        ::HINSTANCE hInstance = get_parent_module(handle_);

        char_sel_window_ =
            create_char_select_window(handle_, char_select_id, hInstance);

        ::RECT r;
        ::GetWindowRect(char_sel_window_, &r);
        int char_sel_width = r.right - r.left;
        int char_sel_height = r.bottom - r.top;

        map_window_ =
            create_map_edit_window(
                handle_, map_edit_id, char_sel_width + 2, hInstance);

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        map_sel_window_ = ::CreateWindowExA(
            WS_EX_CLIENTEDGE, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
            0, char_sel_height + 2,
            char_sel_width, cr.bottom - (char_sel_height + 2),
            handle_,
            reinterpret_cast< ::HMENU>(static_cast< ::LONG_PTR>(map_select_id)),
            hInstance, 0
        );
    }
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

void main_window::new_project(
    const std::string& filename, const game_project& proj)
{
    pimpl_->new_project(filename, proj);
}

void main_window::close_project()
{
    pimpl_->close_project();
}

void main_window::load_project(const std::string& filename)
{
    pimpl_->load_project(filename);
}

void main_window::save_project()
{
    pimpl_->save_project();
}

bool main_window::new_stage(const std::string& filename, int width, int height)
{
    return pimpl_->new_stage(filename, width, height);
}

int main_window::stage_count() const
{
    return pimpl_->stage_count();
}

std::string main_window::stage_name() const
{
    return pimpl_->stage_name();
}

void main_window::delete_stage()
{
    pimpl_->delete_stage();
}

void main_window::change_stage()
{
    pimpl_->change_stage();
}

bool main_window::modified()
{
    return pimpl_->modified();
}

void main_window::track_popup_menu(::HWND hwnd, int x, int y)
{
    pimpl_->track_popup_menu(hwnd, x, y);
}
