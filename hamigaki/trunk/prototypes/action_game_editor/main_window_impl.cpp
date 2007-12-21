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
    const std::set<game_character_class>& char_table,
    const std::map<hamigaki::uuid,fs::path>& filename_table)
{
    typedef std::set<game_character_class>::const_iterator char_iter;
    typedef std::map<hamigaki::uuid,fs::path>::const_iterator name_iter;

    for (char_iter i = char_table.begin(), end = char_table.end(); i!=end; ++i)
    {
        const game_character_class& c = *i;
        name_iter pos = filename_table.find(c.id);
        BOOST_ASSERT(pos != filename_table.end());
        const fs::path& ph = pos->second;
        save_character_class(ph.file_string().c_str(), c);
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
    ::SendMessageA(hwnd, LB_RESETCONTENT, 0, 0);

    fs::directory_iterator it(dir);
    fs::directory_iterator end;

    std::locale loc("");
    for ( ; it != end; ++it)
    {
        const fs::path& ph = it->path();
        const std::string& leaf = ph.leaf();
        if (algo::iends_with(leaf, ".agm-yh", loc))
        {
            ::SendMessageA(hwnd, LB_ADDSTRING, 0,
                reinterpret_cast< ::LPARAM>(leaf.c_str()));
        }
    }
}

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
        , modified_(false)
    {
        scoped_window select_window(
            create_char_select_window(
                handle_, char_select_id,
                hInstance_, select_class_.get()
            )
        );

        ::RECT r;
        ::GetWindowRect(select_window.get(), &r);
        int char_sel_width = r.right - r.left;
        int char_sel_height = r.bottom - r.top;

        scoped_window map_window(
            create_map_edit_window(
                handle_, map_edit_id,
                char_sel_width + 2,
                hInstance_, map_class_.get()
            )
        );

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        map_sel_window_ = ::CreateWindowExA(
            WS_EX_CLIENTEDGE, "LISTBOX", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
            0, char_sel_height + 2,
            char_sel_width, cr.bottom - (char_sel_height + 2),
            handle_,
            reinterpret_cast< ::HMENU>(static_cast< ::LONG_PTR>(map_select_id)),
            hInstance_, 0
        );

        char_sel_window_ = select_window.release();
        map_window_ = map_window.release();
    }

    ~impl()
    {
    }

    void update_size()
    {
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

        load_char_classes(dir, char_table_, filename_table_);
        load_maps(dir, map_table_);
        setup_map_list(map_sel_window_, dir);
        map_edit_window_set(map_window_, 0);

        project_file_ = filename;
        modified_ = true;
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

    void new_stage(int width, int height)
    {
        // TODO
    }

    void change_stage()
    {
        if (map_edit_window_select_modified(map_window_))
            modified_ = true;

        int index = ::SendMessageA(map_sel_window_, LB_GETCURSEL, 0, 0);
        if (index == -1)
            map_edit_window_set(map_window_, 0);

        int size = ::SendMessageA(map_sel_window_, LB_GETTEXTLEN, index, 0);

        boost::scoped_array<char> buf(new char[size+1]);
        ::SendMessageA(
            map_sel_window_, LB_GETTEXT, index,
            reinterpret_cast< ::LPARAM>(buf.get()));

        std::string filename(buf.get(), size);

        map_edit_window_set(map_window_, &map_table_[filename]);
    }

    bool modified()
    {
        return modified_ || map_edit_window_select_modified(map_window_);
    }

private:
    ::HWND handle_;
    ::HINSTANCE hInstance_;
    game_project project_;
    std::string project_file_;
    std::set<game_character_class> char_table_;
    std::map<hamigaki::uuid,fs::path> filename_table_;
    std::map<std::string,stage_map> map_table_;
    scoped_window_class select_class_;
    scoped_window_class map_class_;
    ::HWND char_sel_window_;
    ::HWND map_window_;
    ::HWND map_sel_window_;
    bool modified_;
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

void main_window::load_project(const std::string& filename)
{
    pimpl_->load_project(filename);
}

void main_window::save_project()
{
    pimpl_->save_project();
}

void main_window::new_stage(int width, int height)
{
    pimpl_->new_stage(width, height);
}

void main_window::change_stage()
{
    pimpl_->change_stage();
}

bool main_window::modified()
{
    return pimpl_->modified();
}
