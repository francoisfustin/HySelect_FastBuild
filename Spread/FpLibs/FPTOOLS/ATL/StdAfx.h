// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__4DFEC5A5_029C_11D1_8D8E_0000C004958C__INCLUDED_)
#define AFX_STDAFX_H__4DFEC5A5_029C_11D1_8D8E_0000C004958C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define STRICT

#if VC8
#include <afxtempl.h>
#else
#define _WIN32_WINNT 0x0400
#define _ATL_APARTMENT_THREADED
#endif


#include <windows.h>
#include <winnls.h>
#include <ole2.h>

#include <stddef.h>
#include <tchar.h>
#include <malloc.h>
#ifndef _ATL_NO_DEBUG_CRT
// Warning: if you define the above symbol, you will have
// to provide your own definition of the _ASSERTE(x) macro
// in order to compile ATL
	#include <crtdbg.h>
#endif

#include <olectl.h>
#include <winreg.h>

#ifndef FP_EXCLUDE_ATL
#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4DFEC5A5_029C_11D1_8D8E_0000C004958C__INCLUDED)
