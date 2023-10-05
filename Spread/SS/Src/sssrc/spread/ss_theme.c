//--------------------------------------------------------------------
//
//  File: ss_theme.c
//
//  Description: XP Theme wrapper implementation
//
//  Copyright (c) 2008 by FarPoint Technologies, Inc.
//
//  All rights reserved.  No part of this source code may be
//  copied, modified or reproduced in any form without retaining
//  the above copyright notice.  This source code, or source code
//  derived from it, may not be redistributed without express
//  written permission of FarPoint Technologies, Inc.
//
//--------------------------------------------------------------------

#include <windows.h>
#include <tchar.h>
#include "uxtheme.h"
#include "ss_theme.h"

#define UXTHEME _T("uxtheme.dll")

static HINSTANCE h = NULL;

BOOL SS_CloseThemeData(HTHEME hTheme)
{
   BOOL ret = FALSE;
   if( h == NULL )
      h = LoadLibrary(UXTHEME);
   if( h )
   {
      FARPROC p = GetProcAddress(h, "CloseThemeData");
      if( p )
         ret = (BOOL)p(hTheme);
   }
   return ret;
}
BOOL SS_DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect)
{
   BOOL ret = FALSE;
   if( h == NULL )
      h = LoadLibrary(UXTHEME);
   if( h )
   {
      FARPROC p = GetProcAddress(h, "DrawThemeBackground");
      if( p )
         ret = (BOOL)p(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
   }
   return ret;
}
BOOL SS_DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect)
{
   BOOL ret = FALSE;
   if( h == NULL )
      h = LoadLibrary(UXTHEME);
   if( h )
   {
      FARPROC p = GetProcAddress(h, "DrawThemeText");
      if( p )
         ret = (BOOL)p(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
   }
   return ret;
}
HTHEME SS_OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
   HTHEME ret = NULL;
   if( h == NULL )
      h = LoadLibrary(UXTHEME);
   if( h )
   {
      FARPROC p = GetProcAddress(h, "OpenThemeData");
      if( p )
         ret = (HTHEME)p(hwnd, pszClassList);
   }
   return ret;
}

BOOL SS_IsThemeActive()
{
   BOOL ret = FALSE;
   if( h == NULL )
      h = LoadLibrary(UXTHEME);
   if( h )
   {
      FARPROC p = GetProcAddress(h, "IsThemeActive");
      if( p )
         ret = (BOOL)p();
	  p = GetProcAddress(h, "GetThemeAppProperties");
	  if (p)
		  ret &= (((DWORD)p() & STAP_ALLOW_CONTROLS) != 0);
   }
   if( !ret )
      SS_UnloadUxTheme();
   return ret;
}
BOOL SS_GetThemePosition(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint)
{
   BOOL ret = FALSE;
   if( h == NULL )
      h = LoadLibrary(UXTHEME);
   if( h )
   {
      FARPROC p = GetProcAddress(h, "GetThemePosition");
      if( p )
         ret = (BOOL)p(hTheme, iPartId, iStateId, iPropId, pPoint);
   }
   return ret;
}
BOOL SS_GetCurrentThemeName(LPWSTR pszThemeFileName, int cchMaxNameChars, LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars)
{
   BOOL ret = FALSE;
   if( h == NULL )
      h = LoadLibrary(UXTHEME);
   if( h )
   {
      FARPROC p = GetProcAddress(h, "GetCurrentThemeName");
      if( p )
         ret = (BOOL)p(pszThemeFileName, cchMaxNameChars, pszColorBuff, cchMaxColorChars, pszSizeBuff, cchMaxSizeChars);
   }
   return ret;
}
void SS_UnloadUxTheme()
{
   if( h )
   {
      FreeLibrary(h);
      h = NULL;
   }
}