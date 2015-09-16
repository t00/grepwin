// grepWin - regex search and replace for Windows

// Copyright (C) 2012-2013 - Stefan Kueng

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
#include "Settings.h"
#include "BrowseFolder.h"
#include "DirFileEnum.h"
#include <Commdlg.h>


CSettingsDlg::CSettingsDlg(HWND hParent)
    : m_hParent(hParent)
    , m_regEditorCmd(_T("Software\\grepWin\\editorcmd"))
{
}

CSettingsDlg::~CSettingsDlg(void)
{
}

LRESULT CSettingsDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_GREPWIN);

            CLanguage::Instance().TranslateWindow(*this);

            SetDlgItemText(hwndDlg, IDC_EDITORCMD, std::wstring(m_regEditorCmd).c_str());

            wchar_t modulepath[MAX_PATH] = {0};
            GetModuleFileName(NULL, modulepath, MAX_PATH);
            std::wstring path = modulepath;
            path = path.substr(0, path.find_last_of('\\'));
            CDirFileEnum fileEnumerator(path.c_str());
            bool bRecurse = false;
            bool bIsDirectory = false;
            std::wstring sPath;
            CRegStdString regLang(L"Software\\grepWin\\languagefile");
            int index = 1;
            int langIndex = 0;
            SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)L"English");
            while (fileEnumerator.NextFile(sPath, &bIsDirectory, bRecurse))
            {
                std::wstring ext = sPath.substr(sPath.find_last_of('.'));
                if (ext.compare(L".lang"))
                    continue;
                m_langpaths.push_back(sPath);
                if (sPath.compare(regLang)==0)
                    langIndex = index;
                sPath = sPath.substr(sPath.find_last_of('\\')+1);
                sPath = sPath.substr(0, sPath.find_last_of('.'));
                SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)sPath.c_str());
                ++index;
            }
            SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_SETCURSEL, langIndex, 0);

            m_resizer.Init(hwndDlg);
            m_resizer.AddControl(hwndDlg, IDC_EDITORGROUP, RESIZER_TOPLEFTBOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_EDITORCMD, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC1, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC2, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC3, RESIZER_TOPLEFT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC4, RESIZER_TOPLEFT);
            m_resizer.AddControl(hwndDlg, IDC_LANGUAGE, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_DWM, RESIZER_BOTTOMLEFT);
            m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);
            ExtendFrameIntoClientArea(0, 0, 0, IDC_DWM);
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
    default:
        return FALSE;
    }
    return FALSE;
}

LRESULT CSettingsDlg::DoCommand(int id, int /*msg*/)
{
    switch (id)
    {
    case IDOK:
        {
            auto buf = GetDlgItemText(IDC_EDITORCMD);
            m_regEditorCmd = buf.get();
            int langIndex = (int)SendDlgItemMessage(*this, IDC_LANGUAGE, CB_GETCURSEL, 0, 0);
            CRegStdString regLang(L"Software\\grepWin\\languagefile");
            if (langIndex==0)
            {
                regLang.removeValue();
                CLanguage::Instance().LoadFile(L"");
            }
            else
            {
                regLang = m_langpaths[langIndex-1];
                CLanguage::Instance().LoadFile(m_langpaths[langIndex-1]);
            }
            CLanguage::Instance().TranslateWindow(::GetParent(*this));
        }
        // fall through
    case IDCANCEL:
        EndDialog(*this, id);
        break;
    case IDC_SEARCHPATHBROWSE:
        {
            OPENFILENAME ofn = {0};     // common dialog box structure
            TCHAR szFile[MAX_PATH] = {0};  // buffer for file name
            // Initialize OPENFILENAME
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = *this;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = _countof(szFile);
            ofn.lpstrTitle = TranslatedString(hResource, IDS_SELECTEDITOR).c_str();
            ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_DONTADDTORECENT;
            ofn.lpstrFilter = _T("Programs\0*.exe;*.com\0All files\0*.*\0\0");
            ofn.nFilterIndex = 1;
            // Display the Open dialog box.
            if (GetOpenFileName(&ofn)==TRUE)
            {
                SetDlgItemText(*this, IDC_EDITORCMD, szFile);
            }
        }
        break;
    }
    return 1;
}
