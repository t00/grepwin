// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2008, 2010 - Stefan Kueng

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
#include <string>
#include <vector>
#include "TextFile.h"

using namespace std;


class CSearchInfo
{
public:
    CSearchInfo(void);
    CSearchInfo(const wstring& path);
    ~CSearchInfo(void);

    wstring             filepath;
    DWORD               filesize;
    vector<DWORD>       matchlinesnumbers;
    vector<wstring>     matchlines;
    CTextFile::UnicodeType  encoding;
    FILETIME            modifiedtime;
    bool                readerror;
    bool                folder;
};