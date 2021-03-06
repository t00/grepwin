// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2010, 2012-2015 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "resource.h"
#include "maxpath.h"
#include "BookmarksDlg.h"
#include "NameDlg.h"
#include <string>

#include <boost/regex.hpp>


CBookmarksDlg::CBookmarksDlg(HWND hParent)
    : m_hParent(hParent)
    , m_bUseRegex(false)
    , m_bCaseSensitive(false)
    , m_bDotMatchesNewline(false)
    , m_bBackup(false)
    , m_bUtf8(false)
    , m_bIncludeSystem(false)
    , m_bIncludeFolder(false)
    , m_bIncludeHidden(false)
    , m_bIncludeBinary(false)
    , m_bFileMatchRegex(false)
{
}

CBookmarksDlg::~CBookmarksDlg(void)
{
}

LRESULT CBookmarksDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_GREPWIN);
            CLanguage::Instance().TranslateWindow(*this);
            // initialize the controls
            InitBookmarks();

            m_resizer.Init(hwndDlg);
            m_resizer.AddControl(hwndDlg, IDC_BOOKMARKS, RESIZER_TOPLEFTBOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);
            ExtendFrameIntoClientArea(IDC_BOOKMARKS, IDC_BOOKMARKS, IDC_BOOKMARKS, IDC_BOOKMARKS);
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDOK));
            m_aerocontrols.SubclassControl(GetDlgItem(*this, IDCANCEL));
            if (m_Dwm.IsDwmCompositionEnabled())
                m_resizer.ShowSizeGrip(false);
        }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam), HIWORD(wParam));
    case WM_SIZE:
        {
            m_resizer.DoResize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO * mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = m_resizer.GetDlgRect()->right;
            mmi->ptMinTrackSize.y = m_resizer.GetDlgRect()->bottom;
            return 0;
        }
        break;
    case WM_CONTEXTMENU:
        {
            long x = GET_X_LPARAM(lParam);
            long y = GET_Y_LPARAM(lParam);
            HWND hListControl = GetDlgItem(*this, IDC_BOOKMARKS);
            if (HWND(wParam) == hListControl)
            {
                int nCount = ListView_GetItemCount(hListControl);
                if (nCount == 0)
                    break;
                int iItem = ListView_GetSelectionMark(hListControl);
                if (iItem < 0)
                    break;

                POINT pt = {x,y};
                if ((x==-1)&&(y==-1))
                {
                    RECT rc;
                    ListView_GetItemRect(hListControl, iItem, &rc, LVIR_LABEL);
                    pt.x = (rc.right-rc.left)/2;
                    pt.y = (rc.bottom-rc.top)/2;
                    ClientToScreen(hListControl, &pt);
                }
                HMENU hMenu = LoadMenu(hResource, MAKEINTRESOURCE(IDC_BKPOPUP));
                HMENU hPopup = GetSubMenu(hMenu, 0);
                CLanguage::Instance().TranslateMenu(hPopup);
                TrackPopupMenu(hPopup, TPM_LEFTALIGN|TPM_RIGHTBUTTON, x, y, 0, *this, NULL);
            }
        }
        break;
    case WM_NOTIFY:
        {
            if (wParam == IDC_BOOKMARKS)
            {
                LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
                if (lpnmitem->hdr.code == NM_DBLCLK)
                {
                    DoCommand(IDOK, 0);
                }
            }
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

LRESULT CBookmarksDlg::DoCommand(int id, int /*msg*/)
{
    switch (id)
    {
    case IDOK:
        {
            m_bookmarks.Save();
            int iItem = ListView_GetNextItem(GetDlgItem(*this, IDC_BOOKMARKS), -1, LVNI_SELECTED);
            if (iItem >= 0)
            {
                std::unique_ptr<TCHAR[]> buf(new TCHAR[MAX_PATH_NEW]);
                LVITEM lv = {0};
                lv.iItem = iItem;
                lv.mask = LVIF_TEXT;
                lv.pszText = buf.get();
                lv.cchTextMax = MAX_PATH_NEW;
                ListView_GetItem(GetDlgItem(*this, IDC_BOOKMARKS), &lv);
                m_searchString = m_bookmarks.GetValue(buf.get(), _T("searchString"), _T(""));
                m_replaceString = m_bookmarks.GetValue(buf.get(), _T("replaceString"), _T(""));
                m_sExcludeDirs = m_bookmarks.GetValue(buf.get(), _T("excludedirs"), _T(""));
                m_sFileMatch = m_bookmarks.GetValue(buf.get(), _T("filematch"), _T(""));
                RemoveQuotes(m_searchString);
                RemoveQuotes(m_replaceString);
                RemoveQuotes(m_sExcludeDirs);
                RemoveQuotes(m_sFileMatch);
                m_bUseRegex             = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("useregex"),           _T("false")), _T("true")) == 0;
                m_bCaseSensitive        = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("casesensitive"),      _T("false")), _T("true")) == 0;
                m_bDotMatchesNewline    = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("dotmatchesnewline"),  _T("false")), _T("true")) == 0;
                m_bBackup               = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("backup"),             _T("false")), _T("true")) == 0;
                m_bUtf8                 = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("utf8"),               _T("false")), _T("true")) == 0;
                m_bIncludeSystem        = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("includesystem"),      _T("false")), _T("true")) == 0;
                m_bIncludeFolder        = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("includefolder"),      _T("false")), _T("true")) == 0;
                m_bIncludeHidden        = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("includehidden"),      _T("false")), _T("true")) == 0;
                m_bIncludeBinary        = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("includebinary"),      _T("false")), _T("true")) == 0;
                m_bFileMatchRegex       = _tcscmp(m_bookmarks.GetValue(buf.get(), _T("filematchregex"),     _T("false")), _T("true")) == 0;
            }
        }
        // fall through
    case IDCANCEL:
        EndDialog(*this, id);
        break;
    case ID_REMOVEBOOKMARK:
        {
            int iItem = ListView_GetNextItem(GetDlgItem(*this, IDC_BOOKMARKS), -1, LVNI_SELECTED);
            if (iItem >= 0)
            {
                std::unique_ptr<TCHAR[]> buf(new TCHAR[MAX_PATH_NEW]);
                LVITEM lv = {0};
                lv.iItem = iItem;
                lv.mask = LVIF_TEXT;
                lv.pszText = buf.get();
                lv.cchTextMax = MAX_PATH_NEW;
                ListView_GetItem(GetDlgItem(*this, IDC_BOOKMARKS), &lv);
                m_bookmarks.RemoveBookmark(buf.get());
                ListView_DeleteItem(GetDlgItem(*this, IDC_BOOKMARKS), iItem);
            }
        }
        break;
    case ID_RENAMEBOOKMARK:
        {
            int iItem = ListView_GetNextItem(GetDlgItem(*this, IDC_BOOKMARKS), -1, LVNI_SELECTED);
            if (iItem >= 0)
            {
                std::unique_ptr<TCHAR[]> buf(new TCHAR[MAX_PATH_NEW]);
                LVITEM lv = {0};
                lv.iItem = iItem;
                lv.mask = LVIF_TEXT;
                lv.pszText = buf.get();
                lv.cchTextMax = MAX_PATH_NEW;
                ListView_GetItem(GetDlgItem(*this, IDC_BOOKMARKS), &lv);
                CNameDlg nameDlg(*this);
                nameDlg.SetName(buf.get());
                if (nameDlg.DoModal(hResource, IDD_NAME, *this) == IDOK)
                {
                    if (nameDlg.GetName().compare(buf.get()))
                    {
                        Bookmark bk = m_bookmarks.GetBookmark(buf.get());
                        RemoveQuotes(bk.Search);
                        RemoveQuotes(bk.Replace);
                        bk.Name = nameDlg.GetName();
                        m_bookmarks.AddBookmark(bk);
                        m_bookmarks.RemoveBookmark(buf.get());
                        m_bookmarks.Save();
                        InitBookmarks();
                    }
                }
            }
        }
        break;
    }
    return 1;
}

void CBookmarksDlg::InitBookmarks()
{
    HWND hListControl = GetDlgItem(*this, IDC_BOOKMARKS);
    DWORD exStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT;
    ListView_DeleteAllItems(hListControl);

    int c = Header_GetItemCount(ListView_GetHeader(hListControl))-1;
    while (c>=0)
        ListView_DeleteColumn(hListControl, c--);

    ListView_SetExtendedListViewStyle(hListControl, exStyle);

    std::wstring sName          = TranslatedString(hResource, IDS_NAME);
    std::wstring sSearchString  = TranslatedString(hResource, IDS_SEARCHSTRING);
    std::wstring sReplaceString = TranslatedString(hResource, IDS_REPLACESTRING);

    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_TEXT;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = -1;
    lvc.pszText = const_cast<LPWSTR>((LPCWSTR)sName.c_str());
    ListView_InsertColumn(hListControl, 0, &lvc);
    lvc.pszText = const_cast<LPWSTR>((LPCWSTR)sSearchString.c_str());
    ListView_InsertColumn(hListControl, 1, &lvc);
    lvc.pszText = const_cast<LPWSTR>((LPCWSTR)sReplaceString.c_str());
    ListView_InsertColumn(hListControl, 2, &lvc);


    m_bookmarks.Load();
    CSimpleIni::TNamesDepend sections;
    m_bookmarks.GetAllSections(sections);
    for (CSimpleIni::TNamesDepend::iterator it = sections.begin(); it != sections.end(); ++it)
    {
        std::wstring searchString = m_bookmarks.GetValue(*it, _T("searchString"), _T(""));
        std::wstring replaceString = m_bookmarks.GetValue(*it, _T("replaceString"), _T(""));
        RemoveQuotes(searchString);
        RemoveQuotes(replaceString);

        LVITEM lv = {0};
        lv.mask = LVIF_TEXT;
        TCHAR * pBuf = new TCHAR[_tcslen(*it)+1];
        _tcscpy_s(pBuf, _tcslen(*it)+1, *it);
        lv.pszText = pBuf;
        lv.iItem = ListView_GetItemCount(hListControl);
        int ret = ListView_InsertItem(hListControl, &lv);
        delete [] pBuf;
        if (ret >= 0)
        {
            lv.iItem = ret;
            lv.iSubItem = 1;
            pBuf = new TCHAR[searchString.size()+1];
            lv.pszText = pBuf;
            _tcscpy_s(lv.pszText, searchString.size()+1, searchString.c_str());
            ListView_SetItem(hListControl, &lv);
            delete [] pBuf;
            lv.iSubItem = 2;
            pBuf = new TCHAR[replaceString.size()+1];
            lv.pszText = pBuf;
            _tcscpy_s(lv.pszText, replaceString.size()+1, replaceString.c_str());
            ListView_SetItem(hListControl, &lv);
            delete [] pBuf;
        }
    }

    ListView_SetColumnWidth(hListControl, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hListControl, 1, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hListControl, 2, LVSCW_AUTOSIZE_USEHEADER);
}

void CBookmarksDlg::RemoveQuotes(std::wstring& str)
{
    if (!str.empty())
    {
        if (str[0] == '"')
            str = str.substr(1);
        if (!str.empty())
        {
            if (str[str.size()-1] == '"')
                str = str.substr(0, str.size()-1);
        }
    }
}