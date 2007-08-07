#pragma once
#include "basedialog.h"
#include "SearchInfo.h"
#include <string>

using namespace std;

#define SEARCH_FOUND		(WM_APP+1)


/**
 * search dialog.
 */
class CSearchDlg : public CDialog
{
public:
	CSearchDlg(HWND hParent);
	~CSearchDlg(void);

	DWORD					SearchThread();

protected:
	LRESULT CALLBACK		DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT					DoCommand(int id);

	int						SearchFile(CSearchInfo& sinfo, bool bUseRegex, const wstring& searchString);

	bool					InitResultList();
	bool					AddFoundEntry(CSearchInfo * pInfo);
private:
	HWND					m_hParent;
	wstring					m_searchpath;
	wstring					m_searchString;
	bool					m_bUseRegex;
	bool					m_bAllSize;
	DWORD					m_lSize;
	int						m_sizeCmp;
	bool					m_bIncludeSystem;
	bool					m_bIncludeHidden;
	bool					m_bIncludeSubfolders;

	HANDLE					m_hSearchThread;
};
