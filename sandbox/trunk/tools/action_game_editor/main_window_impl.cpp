// main_window.cpp: main window implementation for action_game_editor

// Copyright Takeshi Mouri 2007, 2008.
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
#include "transfer_dialog.hpp"
#include "transfer_info.hpp"
#include <hamigaki/iterator/first_iterator.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <map>
#include <set>

#if !defined(NDEBUG)
    #include "bjam_dll_path.hpp"
    #include <hamigaki/detail/environment.hpp>
    #include <boost/algorithm/string/case_conv.hpp>
#endif

#include <hamigaki/system/windows_error.hpp>
#include "popup_menus.h"

using hamigaki::system::windows_error;

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;

namespace
{

const ::GUID warp_routines[] =
{
    {0x31AAA83D,0x5A2D,0x45E0,{0x91,0x0D,0x74,0xAF,0xE0,0xE1,0x73,0xF4}}
};

void load_char_classes(
    const fs::path& dir, std::set<game_character_class>& char_table)
{
    fs::directory_iterator it(dir);
    fs::directory_iterator end;

    std::set<game_character_class> tmp;
    std::locale loc("");
    for ( ; it != end; ++it)
    {
        const fs::path& ph = it->path();
        const std::string& leaf = ph.leaf();
        if (algo::iends_with(leaf, ".agc-yh", loc))
        {
            const game_character_class& c =
                load_character_class(ph.file_string().c_str());
            tmp.insert(c).first->name = leaf.substr(0, leaf.size()-7);
        }
    }

    char_table.swap(tmp);
}

void save_char_classes(
    const fs::path& dir, std::set<game_character_class>& char_table)
{
    typedef std::set<game_character_class>::iterator char_iter;
    typedef std::map<hamigaki::uuid,fs::path>::const_iterator name_iter;

    for (char_iter i = char_table.begin(), end = char_table.end(); i!=end; ++i)
    {
        game_character_class& c = *i;

        if (c.modified)
        {
            const fs::path& ph = dir / fs::path(c.name + ".agc-yh");
            save_character_class(ph.file_string().c_str(), c);
            c.modified = false;
        }
    }
}

void load_maps(
    const fs::path& dir, std::map<std::string,stage_map>& table,
    std::map<std::string,transfer_info_table>& transfers)
{
    fs::directory_iterator it(dir);
    fs::directory_iterator end;

    std::map<std::string,stage_map> tmp;
    std::map<std::string,transfer_info_table> tmp2;
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

            const fs::path& ph2 = fs::change_extension(ph, ".agt-yh");
            if (fs::exists(ph2))
            {
                std::string agt = ph2.leaf();
                load_transfer_infos(agt.c_str(), tmp2[agt]);
            }
        }
    }

    table.swap(tmp);
    transfers.swap(tmp2);
}

void save_maps(
    const fs::path& dir, std::map<std::string,stage_map>& table,
    const std::map<std::string,transfer_info_table>& transfers)
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

            typedef std::map<
                std::string,transfer_info_table
            >::const_iterator iter2_type;

            const fs::path& ph2 = fs::change_extension(ph, ".agt-yh");
            std::string agt = ph2.leaf();
            iter2_type it = transfers.find(agt);
            if ((it != transfers.end()) && (!it->second.empty()))
                save_transfer_infos(agt.c_str(), it->second);
            else
                fs::remove(ph2);
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

std::string get_exe_filename()
{
    char buf[MAX_PATH];
    ::GetModuleFileNameA(::GetModuleHandle(0), buf, sizeof(buf));
    return std::string(buf);
}

fs::path get_game_exe_path()
{
    const std::string& exe = get_exe_filename();

    fs::path ph(exe);
    ph.remove_leaf();
    ph /= "action_game.exe";
    if (fs::exists(ph))
        return ph;

#if !defined(NDEBUG)
    ph = algo::replace_all_copy(exe, "action_game_editor", "action_game");
    if (fs::exists(ph))
        return ph;
#endif

    return fs::path();
}

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle), modified_(false)
        , menu_(get_parent_module(handle_), MAKEINTRESOURCE(HAMIGAKI_IDR_POPUP))
        , char_sel_window_(0), map_window_(0), map_sel_window_(0)
        , runner_(get_game_exe_path())
    {
#if !defined(NDEBUG)
        if (!runner_.empty())
            dll_paths_ = get_bjam_dll_paths(runner_.file_string());
#endif
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

    void edit_additional_data(int x, int y)
    {
        const std::pair<int,int> pos(x,y);

        std::string agt_name = stage_name();
        agt_name.replace(agt_name.size()-7, 7, ".agt-yh");

        game_character_class dummy;
        dummy.id = get_selected_char(char_sel_window_);

        if (dummy.id.is_null())
        {
            transfer_table_[agt_name].erase(pos);
            return;
        }

        typedef std::set<game_character_class>::iterator iter_type;
        iter_type it = char_table_.find(dummy);
        if (it == char_table_.end())
            return;

        game_character_class& c = *it;
        if (c.on_touch_player == hamigaki::uuid(warp_routines[0]))
        {
            transfer_info_params params;

            params.map_table = &map_table_;
            params.chars = &char_table_;
            params.bg_color = project_.bg_color;
            params.info.map_file = stage_name();
            params.info.x = x;
            params.info.y = y;

            if (get_transfer_info(handle_, params))
                transfer_table_[agt_name][pos] = params.info;
        }
        else
            transfer_table_[agt_name].erase(pos);
    }

    void new_project(const std::string& filename, const game_project& proj)
    {
        fs::path dir = fs::path(filename).branch_path();
        if (dir.empty())
            dir = fs::current_path();
        else
            ::SetCurrentDirectoryA(dir.directory_string().c_str());

        if (map_window_ == 0)
            create_child_windows();

        project_info(proj);

        load_char_classes(dir, char_table_);
        load_maps(dir, map_table_, transfer_table_);

        setup_map_list(map_sel_window_, dir);
        map_edit_window_set(map_window_, 0);
        map_edit_window_set_char_list(map_window_, &char_table_);
        setup_char_list(char_sel_window_, &char_table_);

        project_file_ = filename;
        project_.dir = dir.directory_string();
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

        transfer_table_.clear();
        map_table_.clear();
        char_table_.clear();
        project_file_.clear();
        project_info(game_project());

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

        save_char_classes(dir, char_table_);
        save_maps(dir, map_table_, transfer_table_);

        char_select_window_modified(char_sel_window_, false);
        modified_ = false;
    }

    game_project project_info() const
    {
        return project_;
    }

    void project_info(const game_project& info)
    {
        project_ = info;
        modified_ = true;

        std::string s;
        if (!project_.title.empty())
        {
            s += project_.title;
            s += " - ";
        }
        s += "Action Game Editor";
        ::SetWindowTextA(handle_, s.c_str());

        map_edit_window_set_bg_color(map_window_, project_.bg_color);
        char_select_window_set_bg_color(char_sel_window_, project_.bg_color);
    }

    bool new_stage(const std::string& filename, int width, int height)
    {
        fs::path ph = fs::path(project_file_).branch_path() / filename;
        if (exists(ph))
            return false;

        if (map_window_ == 0)
            create_child_windows();

        if (map_edit_window_modified(map_window_))
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

    void get_stage_names(std::vector<std::string>& names) const
    {
        names.assign(
            hamigaki::make_first_iterator(map_table_.begin()),
            hamigaki::make_first_iterator(map_table_.end())
        );
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
        if (map_edit_window_modified(map_window_))
            modified_ = true;

        map_edit_window_set(map_window_, &map_table_[stage_name()]);
    }

    bool modified()
    {
        return
            modified_ ||
            map_edit_window_modified(map_window_) ||
            char_select_window_modified(char_sel_window_) ;
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

    bool has_test_runner() const
    {
        return !runner_.empty();
    }

    void test_play()
    {
#if !defined(NDEBUG)
        if (!dll_paths_.empty())
        {
            boost::scoped_array<char> args(new char[project_file_.size()+1]);
            project_file_.copy(&args[0], project_file_.size());
            args[project_file_.size()] = '\0';

            typedef hamigaki::detail::environment_type table_type;
            typedef table_type::iterator iter_type;
            table_type env_table;
            hamigaki::detail::get_environment_variables(env_table);

            std::string old_path = env_table["PATH"];
            if (old_path.empty())
                env_table["PATH"] = dll_paths_;
            else
                env_table["PATH"] = dll_paths_ + ";" + old_path;

            std::size_t env_size = 1;
            for (iter_type i = env_table.begin(); i != env_table.end(); ++i)
            {
                env_size += i->first.size();
                ++env_size;
                env_size += i->second.size();
                ++env_size;
            }

            boost::scoped_array<char> env(new char[env_size]);
            std::size_t env_pos = 0;
            for (iter_type i = env_table.begin(); i != env_table.end(); ++i)
            {
                const std::string& name = i->first;
                const std::string& value = i->second;

                env_pos += name.copy(&env[env_pos], name.size());
                env[env_pos++] = '=';

                env_pos += value.copy(&env[env_pos], value.size());
                env[env_pos++] = '\0';
            }
            env[env_pos] = '\0';

            ::STARTUPINFOA startup = {};
            startup.cb = sizeof(startup);

            ::PROCESS_INFORMATION info;
            if (::CreateProcessA(
                runner_.file_string().c_str(),
                &args[0],
                0, 0, FALSE, 0, env.get(), 0, &startup, &info) != FALSE)
            {
                ::CloseHandle(info.hProcess);
                ::CloseHandle(info.hThread);
            }
            return;
        }
#endif

        ::ShellExecuteA(handle_, 0,
            runner_.file_string().c_str(),
            project_file_.c_str(), 0, SW_SHOWNORMAL);
    }

private:
    ::HWND handle_;
    game_project project_;
    std::string project_file_;
    std::set<game_character_class> char_table_;
    std::map<std::string,stage_map> map_table_;
    std::map<std::string,transfer_info_table> transfer_table_;
    ::HWND char_sel_window_;
    ::HWND map_window_;
    ::HWND map_sel_window_;
    bool modified_;
    menu menu_;
    fs::path runner_;
#if !defined(NDEBUG)
    std::string dll_paths_;
#endif

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

void main_window::edit_additional_data(int x, int y)
{
    pimpl_->edit_additional_data(x, y);
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

game_project main_window::project_info() const
{
    return pimpl_->project_info();
}

void main_window::project_info(const game_project& info)
{
    pimpl_->project_info(info);
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

void main_window::get_stage_names(std::vector<std::string>& names) const
{
    pimpl_->get_stage_names(names);
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

bool main_window::has_test_runner() const
{
    return pimpl_->has_test_runner();
}

void main_window::test_play()
{
    pimpl_->test_play();
}
