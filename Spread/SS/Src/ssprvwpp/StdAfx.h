// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__FDB27245_3DAE_11D2_B03D_0020AF131A57__INCLUDED_)
#define AFX_STDAFX_H__FDB27245_3DAE_11D2_B03D_0020AF131A57__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define STRICT


#define _WIN32_WINNT 0x0400
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#ifdef SPREAD_JPN
#include <atlhost.h>
#endif

// copied from pvctl.cpp:
const IID IID_DPv =
		{ 0x664e2201, 0x24db, 0x11d2, { 0x9a, 0x82, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };

// dispatch IDs for the Spred Print Preview control
#include "dispids.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__FDB27245_3DAE_11D2_B03D_0020AF131A57__INCLUDED)
