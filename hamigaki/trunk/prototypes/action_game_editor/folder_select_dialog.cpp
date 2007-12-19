// folder_select_dialog.cpp: the dialog to select an folder

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "folder_select_dialog.hpp"
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <shlobj.h>

namespace
{

int CALLBACK browse_folder_callback(
    ::HWND hwnd, ::UINT uMsg, ::LPARAM lParam, ::LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
        ::SendMessageA(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    return 0;
}

} // namespace

bool select_folder(::HWND hwnd, std::string& path)
{
    ::BROWSEINFOA info = {};
    info.hwndOwner = hwnd;
    info.lpszTitle = "Select the location of the project:";
    info.ulFlags = BIF_RETURNONLYFSDIRS;

    if (!path.empty())
    {
        info.lpfn = &browse_folder_callback;
        info.lParam = reinterpret_cast< ::LPARAM>(path.c_str());
    }

    boost::shared_ptr< ::ITEMIDLIST> folder(
        ::SHBrowseForFolderA(&info), &CoTaskMemFree
    );
    if (folder.get() == 0)
        return false;

    char buf[MAX_PATH];
    if (!::SHGetPathFromIDListA(folder.get(), buf))
        throw std::runtime_error("failed SHGetPathFromIDListA()");

    path = buf;
    return true;
}
