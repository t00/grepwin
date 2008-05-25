// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008 - Stefan Kueng

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
#include "StdAfx.h"
#include "Resource.h"
#include "RegexTestDlg.h"
#include <string>

#include <regex>

using namespace std;


CRegexTestDlg::CRegexTestDlg(HWND hParent)
{
	m_hParent = hParent;
}

CRegexTestDlg::~CRegexTestDlg(void)
{
}

LRESULT CRegexTestDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			InitDialog(hwndDlg, IDI_GREPWIN);
			// initialize the controls
			SetDlgItemText(hwndDlg, IDC_SEARCHTEXT, m_searchText.c_str());
			SetDlgItemText(hwndDlg, IDC_REPLACETEXT, m_replaceText.c_str());

			SetFocus(GetDlgItem(hwndDlg, IDC_SEARCHTEXT));

			m_resizer.Init(hwndDlg);
			m_resizer.AddControl(hwndDlg, IDC_TEXTCONTENT, RESIZER_TOPLEFTRIGHT);
			m_resizer.AddControl(hwndDlg, IDC_SEARCHTEXT, RESIZER_TOPLEFTRIGHT);
			m_resizer.AddControl(hwndDlg, IDC_REPLACETEXT, RESIZER_TOPLEFTRIGHT);
			m_resizer.AddControl(hwndDlg, IDC_REGEXMATCH, RESIZER_TOPLEFTRIGHT);
			m_resizer.AddControl(hwndDlg, IDC_REGEXREPLACED, RESIZER_TOPLEFTBOTTOMRIGHT);
			m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
			m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);
		}
		return FALSE;
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
	case WM_TIMER:
		{
			if (wParam == ID_REGEXTIMER)
			{
				KillTimer(*this, ID_REGEXTIMER);
				DoRegex();
			}
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
}

LRESULT CRegexTestDlg::DoCommand(int id, int msg)
{
	switch (id)
	{
	case IDOK:
		{
			TCHAR buf[MAX_PATH*4] = {0};
			GetDlgItemText(*this, IDC_SEARCHTEXT, buf, MAX_PATH*4);
			m_searchText = buf;
			GetDlgItemText(*this, IDC_REPLACETEXT, buf, MAX_PATH*4);
			m_replaceText = buf;
		}
		// fall through
	case IDCANCEL:
		EndDialog(*this, id);
		break;
	case IDC_TEXTCONTENT:
		{
			if (msg == EN_CHANGE)
			{
				TCHAR * buf = new TCHAR[10*1024*1024];
				GetDlgItemText(*this, IDC_TEXTCONTENT, buf, 10*1024*1024);
				m_textContent = wstring(buf);
				delete [] buf;

				SetTimer(*this, ID_REGEXTIMER, 300, NULL);
			}
		}
		break;
	case IDC_REPLACETEXT:
	case IDC_SEARCHTEXT:
		{
			if (msg == EN_CHANGE)
			{
				TCHAR buf[MAX_PATH*4] = {0};
				GetDlgItemText(*this, IDC_SEARCHTEXT, buf, MAX_PATH*4);
				m_searchText = buf;
				GetDlgItemText(*this, IDC_REPLACETEXT, buf, MAX_PATH*4);
				m_replaceText = buf;

				SetTimer(*this, ID_REGEXTIMER, 300, NULL);
			}
		}
		break;
	}
	return 1;
}

void CRegexTestDlg::SetStrings(const wstring& search, const wstring& replace)
{
	m_replaceText = replace;
	m_searchText = search;
}

void CRegexTestDlg::DoRegex()
{
	if (m_textContent.empty())
	{
		SetDlgItemText(*this, IDC_REGEXMATCH, _T("no text to test with available"));
		SetDlgItemText(*this, IDC_REGEXREPLACED, _T("no text to test with available"));
	}
	else if (m_searchText.empty())
	{
		SetDlgItemText(*this, IDC_REGEXMATCH, _T("search string is empty"));
		SetDlgItemText(*this, IDC_REGEXREPLACED, _T("search string is empty"));
	}
	else if (m_replaceText.empty())
	{
		SetDlgItemText(*this, IDC_REGEXREPLACED, _T("no text to replace with"));
	}

	if (!m_textContent.empty())
	{
		wstring searchresult;
		wstring replaceresult;
		if (!m_searchText.empty())
		{
			try
			{
				const tr1::wregex expression(m_searchText);
				const tr1::wsregex_iterator endregex;
				for (tr1::wsregex_iterator it(m_textContent.begin(), m_textContent.end(), expression); it != endregex; ++it)
				{
					// (*it)[0] is the matched string
					wstring matchedString = (*it)[0];
					if (!searchresult.empty())
						searchresult = searchresult + _T("\r\n----------------------------\r\n");
					searchresult = searchresult + matchedString;
					if (!m_searchText.empty())
					{
						wstring replaced = tr1::regex_replace(matchedString, expression, m_replaceText);
						if (!replaceresult.empty())
							replaceresult = replaceresult + _T("\r\n----------------------------\r\n");
						replaceresult = replaceresult + replaced;
					}
				}
			}
			catch (const exception&)
			{

			}
			if (searchresult.empty())
				SetDlgItemText(*this, IDC_REGEXMATCH, _T("no match"));
			else
				SetDlgItemText(*this, IDC_REGEXMATCH, searchresult.c_str());
		}
		if (!searchresult.empty())
			SetDlgItemText(*this, IDC_REGEXMATCH, searchresult.c_str());
		if (!replaceresult.empty())
			SetDlgItemText(*this, IDC_REGEXREPLACED, replaceresult.c_str());
	}
}