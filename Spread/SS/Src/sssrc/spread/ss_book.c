/*********************************************************
* SS_BOOK.C
*
* Copyright (C) 1991-2003 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "spread.h"
#include "ss_alloc.h"
#include "ss_book.h"
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_main.h"
#include "ss_win.h"


#ifdef SS_V70
#include "ss_tab.h"

#define CX_TABSPLITBOX 5

#define Tab_IsRectEmpty(lpRect) ((lpRect)->right == 0 && (lpRect)->bottom == 0)

short DLLENTRY SSGetSheetCount(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
short nSheetCount;

nSheetCount = SS_GetSheetCount(lpBook);

SS_BookUnlock(hWnd);
return (nSheetCount);
}


short SS_GetSheetCount(LPSS_BOOK lpBook)
{
return (lpBook->nSheetCnt);
}


BOOL DLLENTRY SSSetSheetCount(HWND hWnd, short nSheetCnt)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet;

fRet = SS_SetSheetCount(lpBook, nSheetCnt);

if (nSheetCnt == 0)
	SS_SetSheetCount(lpBook, 1);

SS_BookUnlock(hWnd);
return (fRet);
}


short DLLENTRY SSGetActiveSheet(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
short nActiveSheet;

nActiveSheet = SS_GetActiveSheet(lpBook);
nActiveSheet = nActiveSheet == -1 ? -1 : nActiveSheet + 1;

SS_BookUnlock(hWnd);
return (nActiveSheet);
}


short SS_GetActiveSheet(LPSS_BOOK lpBook)
{
return (lpBook->nActiveSheet);
}


BOOL DLLENTRY SSSetActiveSheet(HWND hWnd, short nSheet)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet;

nSheet = nSheet == -1 ? -1 : nSheet - 1;
fRet = SS_SetActiveSheet(lpBook, nSheet);

SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetActiveSheet(LPSS_BOOK lpBook, short nSheet)
{
if (nSheet >= 0 && nSheet < lpBook->nSheetCnt)
	{
	RECT Rect;

	SS_GetClientRect(lpBook, &Rect);
	if (nSheet != lpBook->nActiveSheet)
		{
		BOOL bCancel = FALSE;
		short nSheetOld = lpBook->nActiveSheet;

		if (nSheetOld != -1)
			SS_FireSheetChanging(lpBook, nSheetOld, nSheet, &bCancel);

		if (!bCancel)
			{
			LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
			if (lpSS)
				{
				SS_CellEditModeOff(lpSS, 0);
				SS_BookUnlockActiveSheet(lpBook);
				}

			SS_SetActiveSheetIndex(lpBook, nSheet);
			SS_FireSheetChanged(lpBook, nSheetOld, nSheet);
			SS_InvalidateRect(lpBook, &Rect, FALSE);
			SS_EmptyTabStripRect(lpBook);
			SSTab_DisplayActiveTab(lpBook);
			SS_InvalidateTabStrip(lpBook);
			}
		}
	else
		{
		SS_InvalidateRect(lpBook, &Rect, FALSE);
		SS_EmptyTabStripRect(lpBook);
		SS_InvalidateTabStrip(lpBook);
		}
	}

return (TRUE);
}


short DLLENTRY SSGetTabStripLeftSheet(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
short nTabLeftSheet;

nTabLeftSheet = SS_GetTabStripLeftSheet(lpBook) + 1;

SS_BookUnlock(hWnd);
return (nTabLeftSheet);
}


short SS_GetTabStripLeftSheet(LPSS_BOOK lpBook)
{
return (lpBook->nTabStripLeftSheet);
}


BOOL DLLENTRY SSSetTabStripLeftSheet(HWND hWnd, short nSheet)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet;

fRet = SS_SetTabStripLeftSheet(lpBook, (short)(nSheet - 1));

SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetTabStripLeftSheet(LPSS_BOOK lpBook, short nSheet)
{
BOOL fRet = TRUE;

if (nSheet >= 0 && nSheet < lpBook->nSheetCnt)
	lpBook->nTabStripLeftSheet = nSheet;

SS_InvalidateTabStrip(lpBook);
return (fRet);
}


BOOL SS_GetTabStripRect(LPSS_BOOK lpBook, LPRECT lpRect)
{
if (Tab_IsRectEmpty(&lpBook->TabSplitBoxRect))
	{
	if (SS_IsTabStripVisible(lpBook))
		{
		RECT RectClient;
		RECT RectClientWindow;

		SS_GetClientRect(lpBook, &RectClient);
		GetClientRect(lpBook->hWnd, &RectClientWindow);

#ifdef kmtest
		if (TRUE)
		{

			SetRect(&lpBook->TabStripRect,
					  0,
					  RectClientWindow.top,
					  RectClientWindow.right,
					  RectClientWindow.top +  lpBook->dHScrollHeight + 1);

		    SetRect(&lpBook->TabSplitBoxRect,
				  lpBook->TabStripRect.right,
				  RectClientWindow.top,
				  lpBook->TabStripRect.right + CX_TABSPLITBOX,
				  RectClientWindow.top + lpBook->dHScrollHeight + 1);
		}
		else
		{
#endif		
		if (lpBook->fHScrollVisible)
			SetRect(&lpBook->TabStripRect,
					  0,
					  RectClientWindow.bottom - lpBook->dHScrollHeight + 1,
					  (int)((double)(RectClient.right - CX_TABSPLITBOX) * lpBook->dfTabStripRatio),
					  RectClientWindow.bottom);
		else
			SetRect(&lpBook->TabStripRect,
					  0,
					  RectClientWindow.bottom - lpBook->dHScrollHeight + 1,
					  RectClientWindow.right,
					  RectClientWindow.bottom);

		SetRect(&lpBook->TabSplitBoxRect,
				  lpBook->TabStripRect.right,
				  RectClientWindow.bottom - lpBook->dHScrollHeight + 1,
				  lpBook->TabStripRect.right + CX_TABSPLITBOX,
				  RectClientWindow.bottom);
#ifdef kmtest
		}
#endif
		}

	SS_AutoSize(lpBook, FALSE);
	}

if (lpRect)
	*lpRect = lpBook->TabStripRect;

return (TRUE);
}


BOOL SS_IsPointInTabStripRect(LPSS_BOOK lpBook, int x, int y)
{
BOOL fRet = FALSE;

if (SS_IsTabStripVisible(lpBook))
	{
	RECT Rect;
	POINT pt = {x, y};

	SS_GetTabStripRect(lpBook, &Rect);
	fRet = PtInRect(&Rect, pt);
	}

return (fRet);
}


BOOL SS_GetTabSplitBoxRect(LPSS_BOOK lpBook, LPRECT lpRect)
{
if (Tab_IsRectEmpty(&lpBook->TabSplitBoxRect))
	SS_GetTabStripRect(lpBook, NULL);

if (lpRect)
	*lpRect = lpBook->TabSplitBoxRect;
return (TRUE);
}


BOOL SS_IsPointInTabSplitBoxRect(LPSS_BOOK lpBook, int x, int y)
{
BOOL fRet = FALSE;

if (SS_IsTabStripVisible(lpBook))
	{
	RECT Rect;
	POINT pt = {x, y};

	SS_GetTabSplitBoxRect(lpBook, &Rect);
	fRet = PtInRect(&Rect, pt);
	}

return (fRet);
}


void SS_EmptyTabStripRect(LPSS_BOOK lpBook)
{
SetRectEmpty(&lpBook->TabStripRect);
SetRectEmpty(&lpBook->TabSplitBoxRect);
}


BOOL SS_IsTabStripVisible(LPSS_BOOK lpBook)
{
BOOL fRet = FALSE;

if ((lpBook->wTabStripPolicy == SS_TABSTRIPPOLICY_ASNEEDED && lpBook->nSheetCnt > 1) ||
     lpBook->wTabStripPolicy == SS_TABSTRIPPOLICY_ALWAYS)
	fRet = TRUE;

return (fRet);
}


void SS_InvalidateTabStrip(LPSS_BOOK lpBook)
{
RECT Rect;

SS_GetTabStripRect(lpBook, &Rect);

if (SS_IsTabStripVisible(lpBook))
	{
	RECT RectTemp;

	SS_GetTabSplitBoxRect(lpBook, &RectTemp);
	Rect.right = RectTemp.right;
	SS_InvalidateRect(lpBook, &Rect, FALSE);
	}
}


HFONT DLLENTRY SSGetTabStripFont(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
HFONT hFont;

hFont = SS_GetTabStripFont(lpBook);

SS_BookUnlock(hWnd);
return (hFont);
}


HFONT SS_GetTabStripFont(LPSS_BOOK lpBook)
{
SS_FONTID FontId;
HFONT hFont = NULL;

if (lpBook->TabStripFontId)
	FontId = lpBook->TabStripFontId;
else
	FontId = lpBook->DefaultFontId;

if (FontId)
	{
	SS_FONT SSFont;

   if (SS_GetFont(lpBook, &SSFont, FontId))
      hFont = SSFont.hFont;
	}

return (hFont);
}


BOOL DLLENTRY SSSetTabStripFont(HWND hWnd, HFONT hFont)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet;

fRet = SS_SetTabStripFont(lpBook, hFont);

SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetTabStripFont(LPSS_BOOK lpBook, HFONT hFont)
{
lpBook->TabStripFontId = SS_InitFont(lpBook, hFont, TRUE, TRUE, NULL);
SS_InvalidateTabStrip(lpBook);
return (TRUE);
}


// nLen includes the null at the end of the string
short DLLENTRY SSGetSheetName(HWND hWnd, short nSheet, LPTSTR lpszName, short nLen)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
TBGLOBALHANDLE hSheetName;
LPTSTR lpszSheetName;
short nRet = 0;

hSheetName = SS_GetSheetName(lpBook, (short)(nSheet - 1));
if (hSheetName)
	{
	lpszSheetName = (LPTSTR)tbGlobalLock(hSheetName);
	nRet = lstrlen(lpszSheetName);
	if (lpszName)
		lstrcpyn(lpszName, lpszSheetName, nLen == -1 ? nRet + 1 : nLen);
	tbGlobalLock(hSheetName);
	}
else
  {
  if (lpszName)
    lstrcpyn(lpszName, _T(""), nLen == -1 ? nRet + 1 : nLen);
  }

SS_BookUnlock(hWnd);
return (nRet);
}


TBGLOBALHANDLE SS_GetSheetName(LPSS_BOOK lpBook, short nSheet)
{
TBGLOBALHANDLE hSheetName = 0;
LPSPREADSHEET  lpSS = SS_BookLockSheetIndex(lpBook, nSheet);

if (lpSS)
	{
	hSheetName = lpSS->hSheetName;
	SS_BookUnlockSheetIndex(lpBook, nSheet);
	}

return (hSheetName);
}


BOOL DLLENTRY SSSetSheetName(HWND hWnd, short nSheet, LPCTSTR lpszSheetName)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet;

fRet = SS_SetSheetName(lpBook, (short)(nSheet - 1), lpszSheetName);

SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetSheetName(LPSS_BOOK lpBook, short nSheet, LPCTSTR lpszSheetName)
{
LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, nSheet);
BOOL fRet = TRUE;

if (!lpSS)
	return (FALSE);

if (lpSS->hSheetName)
	{
	tbGlobalFree(lpSS->hSheetName);
	lpSS->hSheetName = 0;
	}

if (lpszSheetName && *lpszSheetName)
	{
	LPTSTR lpszNameTemp;
   short  nNameLen = min(lstrlen(lpszSheetName), SS_MAXSHEETNAME);

	lpSS->hSheetName = tbGlobalAlloc(GHND, (nNameLen + 1) * sizeof(TCHAR));

	if (lpSS->hSheetName)
		{
		lpszNameTemp = (LPTSTR)tbGlobalLock(lpSS->hSheetName);
		lstrcpyn(lpszNameTemp, lpszSheetName, nNameLen + 1);
		tbGlobalUnlock(lpSS->hSheetName);
		}
	else
		fRet = FALSE;
	}

SS_InvalidateTabStrip(lpBook);
SS_BookUnlockSheetIndex(lpBook, nSheet);
return (fRet);
}


WORD DLLENTRY SSGetTabStripPolicy(HWND hWnd)
{
WORD wPolicy;
LPSS_BOOK lpBook = SS_BookLock(hWnd);

wPolicy = SS_GetTabStripPolicy(lpBook);
SS_BookUnlock(hWnd);
return (wPolicy);
}


WORD SS_GetTabStripPolicy(LPSS_BOOK lpBook)
{
return (lpBook->wTabStripPolicy);
}


WORD DLLENTRY SSSetTabStripPolicy(HWND hWnd, WORD wTabStripPolicy)
{
WORD wPolicyOld;
LPSS_BOOK lpBook = SS_BookLock(hWnd);

wPolicyOld = SS_SetTabStripPolicy(lpBook, wTabStripPolicy);
SS_BookUnlock(hWnd);
return (wPolicyOld);
}


WORD SS_SetTabStripPolicy(LPSS_BOOK lpBook, WORD wTabStripPolicy)
{
WORD wPolicyOld = lpBook->wTabStripPolicy;

if (wTabStripPolicy >= 0 && wTabStripPolicy <= 2)
	{
	SS_InvalidateTabStrip(lpBook);
	lpBook->wTabStripPolicy = wTabStripPolicy;
	SS_EmptyTabStripRect(lpBook);
	SS_InvalidateTabStrip(lpBook);
	}

return (wPolicyOld);
}

#ifdef SS_V80
WORD DLLENTRY SSGetTabStripButtonPolicy(HWND hWnd)
{
WORD wPolicy;
LPSS_BOOK lpBook = SS_BookLock(hWnd);

wPolicy = SS_GetTabStripButtonPolicy(lpBook);
SS_BookUnlock(hWnd);
return (wPolicy);
}


WORD SS_GetTabStripButtonPolicy(LPSS_BOOK lpBook)
{
return (lpBook->wTabStripButtonPolicy);
}


WORD DLLENTRY SSSetTabStripButtonPolicy(HWND hWnd, WORD wTabStripButtonPolicy)
{
WORD wPolicyOld;
LPSS_BOOK lpBook = SS_BookLock(hWnd);

wPolicyOld = SS_SetTabStripButtonPolicy(lpBook, wTabStripButtonPolicy);
SS_BookUnlock(hWnd);
return (wPolicyOld);
}


WORD SS_SetTabStripButtonPolicy(LPSS_BOOK lpBook, WORD wTabStripButtonPolicy)
{
WORD wPolicyOld = lpBook->wTabStripButtonPolicy;

if (wTabStripButtonPolicy >= 0 && wTabStripButtonPolicy <= 2)
	{
	SS_InvalidateTabStrip(lpBook);
	lpBook->wTabStripButtonPolicy = wTabStripButtonPolicy;
	SS_EmptyTabStripRect(lpBook);
	SS_InvalidateTabStrip(lpBook);
	}

return (wPolicyOld);
}

#endif

double DLLENTRY SSGetTabStripRatio(HWND hWnd)
{
double dfRatio;
LPSS_BOOK lpBook = SS_BookLock(hWnd);

dfRatio = SS_GetTabStripRatio(lpBook);
SS_BookUnlock(hWnd);
return (dfRatio);
}


double SS_GetTabStripRatio(LPSS_BOOK lpBook)
{
return (lpBook->dfTabStripRatio);
}


double DLLENTRY SSSetTabStripRatio(HWND hWnd, double dfTabStripRatio)
{
double dfRatioOld;
LPSS_BOOK lpBook = SS_BookLock(hWnd);

dfRatioOld = SS_SetTabStripRatio(lpBook, dfTabStripRatio);
SS_BookUnlock(hWnd);
return (dfRatioOld);
}


double SS_SetTabStripRatio(LPSS_BOOK lpBook, double dfTabStripRatio)
{
double dfRatioOld = lpBook->dfTabStripRatio;

if (dfTabStripRatio >= 0.0 && dfTabStripRatio <= 1.0)
	{
	SS_InvalidateTabStrip(lpBook);
	lpBook->dfTabStripRatio = dfTabStripRatio;
	SS_EmptyTabStripRect(lpBook);
	SS_InvalidateTabStrip(lpBook);
	}

return (dfRatioOld);
}


BOOL DLLENTRY SSSetSheetVisible(HWND hWnd, short nSheet, BOOL fVisible)
{
LPSS_BOOK     lpBook = SS_BookLock(hWnd);
BOOL          fRet;

fRet = SS_SetSheetVisible(lpBook, (short)(nSheet - 1), fVisible);

SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetSheetVisible(LPSS_BOOK lpBook, short nSheet, BOOL fVisible)
{
LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, nSheet);
BOOL          fRet = FALSE;

if (lpSS)
	{
	if (lpSS->fVisible != fVisible)
		{
		fRet = lpSS->fVisible;
		lpSS->fVisible = fVisible;

		// RFW - 6/29/04 - 14551
		if (!fVisible && nSheet == lpBook->nTabStripLeftSheet)
			{
			lpBook->nTabStripLeftSheet = SSTab_GetNextVisibleTab(lpBook, nSheet);
			if (lpBook->nTabStripLeftSheet == -1)
				lpBook->nTabStripLeftSheet = SSTab_GetPrevVisibleTab(lpBook, nSheet);
			}

		// RFW - 1/26/07 - 19758
		else if (fVisible && lpBook->nTabStripLeftSheet == -1)
			lpBook->nTabStripLeftSheet = nSheet;

		// RFW - 1/26/07 - 19758
		else if (fVisible && lpBook->nTabStripLeftSheet > nSheet)
			lpBook->nTabStripLeftSheet = nSheet;

		SS_InvalidateTabStrip(lpBook);

		// RFW - 5/10/04 - 14212
		if (!fVisible && nSheet == lpBook->nActiveSheet)
			{
			short nActiveSheet = SSTab_GetNextVisibleTab(lpBook, nSheet);

			if (nActiveSheet == -1)
				SSTab_GetPrevVisibleTab(lpBook, nSheet);
			else
				SS_SetActiveSheet(lpBook, nActiveSheet);
			}
		}

	SS_BookUnlockSheetIndex(lpBook, nSheet);
	}

return (fRet);
}


BOOL DLLENTRY SSGetSheetVisible(HWND hWnd, short nSheet)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      fVisible = FALSE;

fVisible = SS_GetSheetVisible(lpBook, (short)(nSheet - 1));

SS_BookUnlock(hWnd);
return (fVisible);
}


BOOL SS_GetSheetVisible(LPSS_BOOK lpBook, short nSheet)
{
LPSPREADSHEET lpSS = SS_BookLockSheetIndex(lpBook, nSheet);
BOOL          fVisible = FALSE;

if (lpSS)
	fVisible = lpSS->fVisible;

SS_BookUnlockSheetIndex(lpBook, nSheet);
return (fVisible);
}


void SS_FireSheetChanging(LPSS_BOOK lpBook, short nOldSheet, short nNewSheet, LPBOOL lpbCancel)
{
LRESULT lRet = SSx_SendMsgActiveSheet(lpBook, SSM_SHEETCHANGING, GetWindowID(lpBook->hWnd), MAKELONG(nOldSheet == -1 ? -1 : nOldSheet + 1, nNewSheet == -1 ? -1 : nNewSheet + 1));
*lpbCancel = (lRet ? TRUE : FALSE);
}

void SS_FireSheetChanged(LPSS_BOOK lpBook, short nOldSheet, short nNewSheet)
{
if (lpBook->hWnd) SSx_SendMsgActiveSheet(lpBook, SSM_SHEETCHANGED, GetWindowID(lpBook->hWnd), MAKELONG(nOldSheet == -1 ? -1 : nOldSheet + 1, nNewSheet == -1 ? -1 : nNewSheet + 1));
}


void SS_FireTabScrolling(LPSS_BOOK lpBook, short nOldLeftSheet, short nNewLeftSheet)
{
SSx_SendMsgActiveSheet(lpBook, SSM_TABSCROLLING, GetWindowID(lpBook->hWnd), MAKELONG(nOldLeftSheet == -1 ? -1 : nOldLeftSheet + 1, nNewLeftSheet == -1 ? -1 : nNewLeftSheet + 1));
}


void SS_FireTabScrolled(LPSS_BOOK lpBook, short nOldLeftSheet, short nNewLeftSheet)
{
SSx_SendMsgActiveSheet(lpBook, SSM_TABSCROLLED, GetWindowID(lpBook->hWnd), MAKELONG(nOldLeftSheet == -1 ? -1 : nOldLeftSheet + 1, nNewLeftSheet == -1 ? -1 : nNewLeftSheet + 1));
}


void SS_TabResize(LPSPREADSHEET lpSS, int iPos)
{
RECT RectClient;
int  xMax;
int  iNewPos;

SS_GetClientRect(lpSS->lpBook, &RectClient);

xMax = RectClient.right - 1 - CX_TABSPLITBOX;

iNewPos = iPos - lpSS->lpBook->iTabResizeOffset;
iNewPos = min(iNewPos, xMax);
iNewPos = max(iNewPos, 0);

lpSS->FreezeHighlight = TRUE;
SS_SetTabStripRatio(lpSS->lpBook, (double)iNewPos / (double)xMax);
SS_UpdateWindow(lpSS->lpBook);
lpSS->FreezeHighlight = FALSE;
}


void SS_SetActiveSheetIndex(LPSS_BOOK lpBook, short nActiveSheet)
{
LPTBGLOBALHANDLE lpSheets;
if (!lpBook->hSheets)
    return;
lpSheets = (LPTBGLOBALHANDLE)tbGlobalLock(lpBook->hSheets);

lpBook->nActiveSheet = nActiveSheet;
if (nActiveSheet < lpBook->nSheetCnt)
	lpBook->hActiveSheet = lpSheets[nActiveSheet];
else
	lpBook->hActiveSheet = 0;
	
tbGlobalUnlock(lpBook->hSheets);
}

#endif // SS_V70