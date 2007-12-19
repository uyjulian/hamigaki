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
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
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

        ::MoveWindow(map_window_, left, 0, cr.right - left, cr.bottom, TRUE);
    }

    void update_selected_char()
    {
        const hamigaki::uuid& c = get_selected_char(select_window_);
        map_edit_window_select_char(map_window_, c);
    }

    void new_project(const std::string& filename, const game_project& proj)
    {
        project_ = proj;

        load_char_classes(
            fs::path(filename).branch_path(),
            char_table_,
            filename_table_
        );

        project_file_ = filename;
    }

    void load_project(const std::string& filename)
    {
        project_ = load_game_project(filename.c_str());

        load_char_classes(
            fs::path(filename).branch_path(),
            char_table_,
            filename_table_
        );

        project_file_ = filename;
    }

    void save_project()
    {
        save_game_project(project_file_.c_str(), project_);

        save_char_classes(
            fs::path(project_file_).branch_path(),
            char_table_, filename_table_
        );
    }

    void new_stage(int width, int height)
    {
        map_edit_window_new(map_window_, width, height);
        stage_file_.clear();
    }

    void load_stage(const std::string& filename)
    {
        map_edit_window_load(map_window_, filename);
        stage_file_ = filename;
    }

    void save_stage(const std::string& filename)
    {
        map_edit_window_save(map_window_, filename);
        stage_file_ = filename;
    }

    bool save_stage()
    {
        if (stage_file_.empty())
            return false;

        save_stage(stage_file_);
        return true;
    }

    bool modified()
    {
        return map_edit_window_select_modified(map_window_);
    }

private:
    ::HWND handle_;
    ::HINSTANCE hInstance_;
    game_project project_;
    std::string project_file_;
    std::set<game_character_class> char_table_;
    std::map<hamigaki::uuid,fs::path> filename_table_;
    scoped_window_class select_class_;
    scoped_window_class map_class_;
    ::HWND select_window_;
    ::HWND map_window_;
    std::string stage_file_;
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

void main_window::load_stage(const std::string& filename)
{
    pimpl_->load_stage(filename);
}

void main_window::save_stage(const std::string& filename)
{
    pimpl_->save_stage(filename);
}

bool main_window::save_stage()
{
    return pimpl_->save_stage();
}

bool main_window::modified()
{
    return pimpl_->modified();
}
