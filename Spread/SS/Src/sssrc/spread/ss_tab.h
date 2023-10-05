//--------------------------------------------------------------------
//
//  File: sstab.h
//
//  Description: Tab object
//
//  Copyright (c) 1999 by FarPoint Technologies, Inc.
//
//  All rights reserved.  No part of this source code may be
//  copied, modified or reproduced in any form without retaining
//  the above copyright notice.  This source code, or source code
//  derived from it, may not be redistributed without express
//  written permission of FarPoint Technologies, Inc.
//
//--------------------------------------------------------------------

#if !defined(SSTAB_H)
#define SSTAB_H

#define HSS_TAB TBGLOBALHANDLE

// Constructors and destructors
HSS_TAB SSTab_Create(void);
long SSTab_AddRef(HSS_TAB hThis);
long SSTab_Release(HSS_TAB hThis);
void SSTab_Attach(HSS_TAB hThis, HWND hWndSS);
void SSTab_Detach(HSS_TAB hThis);

// Methods
BOOL SSTab_Draw(HSS_TAB hThis, HDC hdc, RECT *prc, BOOL bBottom);
BOOL SSTab_HitTest(HSS_TAB hThis, int x, int y, RECT *prc, short *pnSheet, int *pnBtn, RECT *prcHit);

// Message handlers
LRESULT SSTab_OnLButtonDown(HSS_TAB hThis, WPARAM wParam, LPARAM lParam, RECT *prc);
LRESULT SSTab_OnLButtonUp(HSS_TAB hThis, WPARAM wParam, LPARAM lParam, RECT *prc);
LRESULT SSTab_OnMouseMove(HSS_TAB hThis, WPARAM wParam, LPARAM lParam, RECT *prc);

#ifdef SS_V80
void SS_DrawSplitBox(HDC hdc, RECT *prc, LPSS_BOOK lpBook);
#else
void SS_DrawSplitBox(HDC hdc, RECT *prc);
#endif
void SSTab_DisplayActiveTab(LPSS_BOOK lpBook);
short SSTab_GetSheetDisplayName(LPSS_BOOK lpBook, short nSheet, LPTSTR lpszName, int nLen);
short SSTab_GetFirstVisibleTab(LPSS_BOOK lpBook);
short SSTab_GetLastVisibleTab(LPSS_BOOK lpBook);
short SSTab_GetNextVisibleTab(LPSS_BOOK lpBook, short nSheet);
short SSTab_GetPrevVisibleTab(LPSS_BOOK lpBook, short nSheet);

#define SS_MAXSHEETNAME 31

#endif  // !defined(SSTAB_H)
