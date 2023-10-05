//--------------------------------------------------------------------
//
//  File: ss_theme.h
//
//  Description: XP Theme wrapper definitions
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

#ifndef SS_THEME_H
#define SS_THEME_H

#ifdef __cplusplus
extern "C" {
#endif

BOOL SS_CloseThemeData(HTHEME hTheme);
BOOL SS_DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
BOOL SS_DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect);
HTHEME SS_OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
BOOL SS_IsThemeActive();
BOOL SS_GetThemePosition(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint);
BOOL SS_GetCurrentThemeName(LPWSTR pszThemeFileName, int cchMaxNameChars, LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars);
void SS_UnloadUxTheme();

#ifdef __cplusplus
}
#endif

#endif // !SS_THEME_H