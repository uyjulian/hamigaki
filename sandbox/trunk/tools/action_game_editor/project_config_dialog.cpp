// project_config_dialog.cpp: the dialog to input data for setting a project

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "project_config_dialog.hpp"
#include "folder_select_dialog.hpp"
#include "msg_utilities.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <exception>
#include "proj_cfg_dialog.h"

namespace fs = boost::filesystem;

namespace
{

class solid_brush : private boost::noncopyable
{
public:
    solid_brush() : handle_(0)
    {
    }

    explicit solid_brush(::COLORREF c)
        : handle_(::CreateSolidBrush(c))
    {
    }

    ~solid_brush()
    {
        if (handle_ != 0)
            ::DeleteObject(reinterpret_cast< ::HGDIOBJ>(handle_));
    }

    ::HBRUSH get() const
    {
        return handle_;
    }

    void swap(solid_brush& rhs)
    {
        std::swap(handle_, rhs.handle_);
    }

private:
    ::HBRUSH handle_;
};

struct dialog_data
{
    game_project info;
    const std::vector<std::string>* map_names;
    std::string old_name;
    solid_brush brush;
};

inline ::DWORD to_color_ref(::DWORD xrgb)
{
    ::DWORD r = (xrgb >> 16) & 0xFF;
    ::DWORD g = (xrgb >>  8) & 0xFF;
    ::DWORD b = (xrgb      ) & 0xFF;

    return r | (g << 8) | (b << 16);
}

inline ::DWORD to_d3d_color(::DWORD bgr)
{
    ::DWORD r = (bgr      ) & 0xFF;
    ::DWORD g = (bgr >>  8) & 0xFF;
    ::DWORD b = (bgr >> 16) & 0xFF;

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

inline void set_dialog_item_text(::HWND hwnd, int id, const std::string& s)
{
    ::SetDlgItemTextA(hwnd, id, s.c_str());
}

std::string get_dialog_item_text(::HWND hwnd, int id)
{
    char buf[256];
    ::GetDlgItemTextA(hwnd, id, buf, sizeof(buf));
    return std::string(buf);
}

inline void set_dialog_item_int(::HWND hwnd, int id, int value)
{
    ::SetDlgItemInt(hwnd, id, static_cast<unsigned>(value), FALSE);
}

inline bool get_dialog_item_int(::HWND hwnd, int id, int& value)
{
    ::BOOL res;
    value = static_cast<int>(::GetDlgItemInt(hwnd, id, &res, FALSE));
    return res != FALSE;
}

::INT_PTR CALLBACK proj_cfg_dialog_proc(
    ::HWND hwndDlg, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        dialog_data* data;
        if (uMsg == WM_INITDIALOG)
        {
            data = reinterpret_cast<dialog_data*>(lParam);
            ::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
        }
        else
        {
            data = reinterpret_cast<dialog_data*>(
                ::GetWindowLongPtr(hwndDlg, DWLP_USER)
            );
        }

        if (uMsg == WM_INITDIALOG)
        {
            game_project& info = data->info;

            std::string name = fs::path(info.dir).leaf();
            set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_FOLDER, name);

            if (!info.title.empty())
            {
                set_dialog_item_text(
                    hwndDlg, HAMIGAKI_IDC_TITLE, info.title);
            }
            else
                set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_TITLE, name);

            set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_DIR, info.dir);
            set_dialog_item_int(
                hwndDlg, HAMIGAKI_IDC_SCREEN_W, info.screen_width);
            set_dialog_item_int(
                hwndDlg, HAMIGAKI_IDC_SCREEN_H, info.screen_height);
            set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_GRAVITY, info.gravity);
            set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_MIN_VY, info.min_vy);

            ::HWND map_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_START_MAP);
            if (data->map_names)
            {
                const std::vector<std::string>& v = *(data->map_names);
                for (std::size_t i = 0; i < v.size(); ++i)
                    send_msg(map_hwnd, CB_ADDSTRING, 0, v[i]);

                if (!info.start_map.empty())
                {
                    int index = send_msg(
                        map_hwnd, CB_FINDSTRINGEXACT, 0, info.start_map);
                    if (index != CB_ERR)
                        send_msg(map_hwnd, CB_SETCURSEL, index);
                }

                ::EnableWindow(
                    ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_FOLDER), FALSE);
                ::EnableWindow(
                    ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_DIR), FALSE);
                ::EnableWindow(
                    ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_DIR_SEL), FALSE);
            }
            else
                ::EnableWindow(map_hwnd, FALSE);

            data->old_name = name;
            return 1;
        }
        else if (uMsg == WM_DRAWITEM)
        {
            ::DRAWITEMSTRUCT& item =
                *reinterpret_cast< ::DRAWITEMSTRUCT*>(lParam);
            ::FillRect(item.hDC, &item.rcItem, data->brush.get());
        }
        else if (uMsg == WM_COMMAND)
        {
            ::WORD id = LOWORD(wParam);
            ::WORD code = HIWORD(wParam);
            if (id == IDOK)
            {
                game_project& info = data->info;

                info.title =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_TITLE);
                info.dir =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_DIR);
                info.gravity =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_GRAVITY);
                info.min_vy =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_MIN_VY);
                info.start_map =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_START_MAP);

                boost::lexical_cast<float>(info.gravity);
                boost::lexical_cast<float>(info.min_vy);

                if (!info.dir.empty() &&
                    get_dialog_item_int(
                        hwndDlg, HAMIGAKI_IDC_SCREEN_W, info.screen_width) &&
                    get_dialog_item_int(
                        hwndDlg, HAMIGAKI_IDC_SCREEN_H, info.screen_height) )
                {
                    ::EndDialog(hwndDlg, IDOK);
                }
                return 1;
            }
            else if (id == IDCANCEL)
            {
                ::EndDialog(hwndDlg, IDCANCEL);
                return 1;
            }
            else if (id == HAMIGAKI_IDC_FOLDER)
            {
                if (code == EN_CHANGE)
                {
                    const std::string& name =
                        get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_FOLDER);

                    const std::string& title =
                        get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_TITLE);

                    if (title == data->old_name)
                        set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_TITLE, name);

                    fs::path ph =
                        get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_DIR);
                    if (ph.leaf() == data->old_name)
                        ph.remove_leaf();

                    ph /= name;
                    set_dialog_item_text(
                        hwndDlg, HAMIGAKI_IDC_DIR, ph.directory_string());

                    data->old_name = name;
                }
            }
            else if (id == HAMIGAKI_IDC_DIR_SEL)
            {
                const fs::path org_ph =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_DIR);

                std::string dir;
                for (fs::path ph = org_ph; ph.has_leaf(); ph.remove_leaf())
                {
                    if (fs::exists(ph))
                    {
                        dir = ph.directory_string();
                        break;
                    }
                }

                if (select_folder(hwndDlg, dir))
                {
                    const std::string& leaf =
                        get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_FOLDER);

                    const fs::path& ph = fs::path(dir) / leaf;
                    set_dialog_item_text(
                        hwndDlg, HAMIGAKI_IDC_DIR, ph.directory_string());
                }
                return 1;
            }
            else if (id == HAMIGAKI_IDC_BG)
            {
                // FIXME
                ::DWORD custom_colors[16];
                std::fill_n(custom_colors, 16, 0xFFFFFF);

                ::CHOOSECOLORA info = { sizeof(::CHOOSECOLORA) };
                info.hwndOwner = hwndDlg;
                info.rgbResult = to_color_ref(data->info.bg_color);
                info.lpCustColors = custom_colors;
                info.Flags = CC_FULLOPEN|CC_RGBINIT;

                if (::ChooseColorA(&info) != FALSE)
                {
                    data->info.bg_color = to_d3d_color(info.rgbResult);

                    {
                        solid_brush tmp(to_color_ref(data->info.bg_color));
                        data->brush.swap(tmp);
                    }

                    ::InvalidateRect(
                        reinterpret_cast< ::HWND>(lParam), 0, FALSE);
                }
                return 1;
            }
        }
        else
            return 0;
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwndDlg, e.what(), "Action Game Editor", MB_OK);
    }
    return 0;
}

bool get_project_info_impl(
    ::HWND hwnd, game_project& info, const std::vector<std::string>* map_names)
{
    // FIXME
    ::HINSTANCE module =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    dialog_data data;
    data.map_names = map_names;
    data.info = info;

    {
        solid_brush tmp(to_color_ref(info.bg_color));
        data.brush.swap(tmp);
    }

    ::INT_PTR res = ::DialogBoxParamA(
        module, MAKEINTRESOURCE(HAMIGAKI_IDD_PROJ_CFG),
        hwnd, &proj_cfg_dialog_proc, reinterpret_cast< ::LPARAM>(&data)
    );

    if (res == IDOK)
    {
        info = data.info;
        return true;
    }
    else
        return false;
}

} // namespace

bool get_project_info(
    ::HWND hwnd, game_project& info, const std::vector<std::string>& map_names)
{
    return get_project_info_impl(hwnd, info, &map_names);
}

bool get_project_info(::HWND hwnd, game_project& info)
{
    return get_project_info_impl(hwnd, info, 0);
}
