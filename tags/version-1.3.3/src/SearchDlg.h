// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2009 - Stefan Kueng

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
#pragma once
#include "basedialog.h"
#include "SearchInfo.h"
#include "DlgResizer.h"
#include "FileDropTarget.h"
#include "AutoComplete.h"
#include "Registry.h"
#include "hyperlink.h"
#include <string>
#include <vector>

using namespace std;

#define SEARCH_FOUND		(WM_APP+1)
#define SEARCH_START		(WM_APP+2)
#define SEARCH_PROGRESS		(WM_APP+3)
#define SEARCH_END			(WM_APP+4)

#define ID_ABOUTBOX			0x0010

/**
 * search dialog.
 */
class CSearchDlg : public CDialog
{
public:
	CSearchDlg(HWND hParent);
	~CSearchDlg(void);

	DWORD					SearchThread();
	void					SetSearchPath(const wstring& path) {m_searchpath = path;}

protected:
	LRESULT CALLBACK		DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT					DoCommand(int id, int msg);
	bool					PreTranslateMessage(MSG* pMsg);

	int						SearchFile(CSearchInfo& sinfo, bool bIncludeBinary, bool bUseRegex, bool bCaseSensitive, bool bDotMatchesNewline, const wstring& searchString);

	bool					InitResultList();
	bool					AddFoundEntry(CSearchInfo * pInfo, bool bOnlyListControl = false);
	void					ShowContextMenu(int x, int y);
	void					DoListNotify(LPNMITEMACTIVATE lpNMItemActivate);
	void					UpdateInfoLabel();
	void					UpdateSearchButton();
	bool					SaveSettings();
	void					SaveWndPosition();

private:
	static bool				NameCompareAsc(const CSearchInfo Entry1, const CSearchInfo Entry2);
	static bool				SizeCompareAsc(const CSearchInfo Entry1, const CSearchInfo Entry2);
	static bool				MatchesCompareAsc(const CSearchInfo Entry1, const CSearchInfo Entry2);
	static bool				PathCompareAsc(const CSearchInfo Entry1, const CSearchInfo Entry2);
	static bool				EncodingCompareAsc(const CSearchInfo Entry1, const CSearchInfo Entry2);

	static bool				NameCompareDesc(const CSearchInfo Entry1, const CSearchInfo Entry2);
	static bool				SizeCompareDesc(const CSearchInfo Entry1, const CSearchInfo Entry2);
	static bool				MatchesCompareDesc(const CSearchInfo Entry1, const CSearchInfo Entry2);
	static bool				PathCompareDesc(const CSearchInfo Entry1, const CSearchInfo Entry2);
	static bool				EncodingCompareDesc(const CSearchInfo Entry1, const CSearchInfo Entry2);

private:
	HWND					m_hParent;
	volatile LONG			m_dwThreadRunning;
	volatile LONG			m_Cancelled;
	wstring					m_searchpath;
	wstring					m_searchString;
	wstring					m_replaceString;
	vector<wstring>			m_patterns;
	wstring					m_patternregex;
	bool					m_bUseRegex;
	bool					m_bUseRegexForPaths;
	bool					m_bAllSize;
	DWORD					m_lSize;
	int						m_sizeCmp;
	bool					m_bIncludeSystem;
	bool					m_bIncludeHidden;
	bool					m_bIncludeSubfolders;
	bool					m_bIncludeBinary;
	bool					m_bCreateBackup;
	bool					m_bCaseSensitive;
	bool					m_bDotMatchesNewline;

	bool					m_bReplace;
	HANDLE					m_hSearchThread;

	vector<CSearchInfo>		m_items;
	int						m_totalitems;
	int						m_searchedItems;
	int						m_totalmatches;
	bool					m_bAscending;

	CDlgResizer				m_resizer;
	CHyperLink				m_link;

	CFileDropTarget *		m_pDropTarget;

	DWORD					m_startTime;
	static UINT				GREPWIN_STARTUPMSG;

	CAutoComplete			m_AutoCompleteFilePatterns;
	CAutoComplete			m_AutoCompleteSearchPatterns;
	CAutoComplete			m_AutoCompleteReplacePatterns;
	CRegStdWORD				m_regUseRegex;
	CRegStdWORD				m_regAllSize;
	CRegStdWORD				m_regSize;
	CRegStdWORD				m_regSizeCombo;
	CRegStdWORD				m_regIncludeSystem;
	CRegStdWORD				m_regIncludeHidden;
	CRegStdWORD				m_regIncludeSubfolders;
	CRegStdWORD				m_regIncludeBinary;
	CRegStdWORD				m_regCreateBackup;
	CRegStdWORD				m_regCaseSensitive;
	CRegStdWORD				m_regDotMatchesNewline;
	CRegStdString			m_regPattern;
	CRegStdString			m_regSearchPath;
};