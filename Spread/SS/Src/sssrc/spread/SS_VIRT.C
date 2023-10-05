/*********************************************************
* SS_VIRT.C       
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "spread.h"
#include "ss_alloc.h"
#include "ss_cell.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_scbar.h"
#include "ss_scrol.h"
#include "ss_virt.h"
#include "ss_win.h"
#include "..\..\vbsrc\vbsscnvt.h"


#ifndef SS_NOVIRTUAL

extern HANDLE hDynamicInst;

BOOL SS_VQueryData(LPSPREADSHEET lpSS, SS_COORD BottomVisCell)
{
SS_VQUERYDATA VQueryData;
SS_COORD      RowsPerScreen;
SS_COORD      VSize;
SS_COORD      RowNew;
SS_COORD      VNewTop;
SS_COORD      VPhysOverlap;
SS_COORD      VOverlap;
SS_COORD      RowsNeeded;
SS_COORD      VQueryDataRowOffset = 0;
BOOL          fRedraw;
BOOL          fAutoSize = FALSE;
BOOL          fRet = TRUE;

fRedraw = lpSS->lpBook->Redraw;      
lpSS->lpBook->Redraw = FALSE;

lpSS->fProcessingVQueryData = TRUE;

_fmemset(&VQueryData, '\0', sizeof(SS_VQUERYDATA));

RowsPerScreen = BottomVisCell - lpSS->Row.UL + 1;

VSize = max(lpSS->Virtual.VSize, RowsPerScreen);
VPhysOverlap = max(lpSS->Virtual.VOverlap, RowsPerScreen);

/************************************************
* Check to see if requested row is above buffer
************************************************/

if (lpSS->Row.UL < lpSS->Virtual.VTop)
   {
   VOverlap = 0;
   RowNew = lpSS->Row.UL;
   RowsNeeded = VSize;

   if (lpSS->Virtual.VPhysSize)
      {
      // Set up overlap portion of buffer

      if (lpSS->Virtual.VTop - lpSS->Row.UL <= VSize)
         {
         RowsNeeded = min(VSize, lpSS->Virtual.VTop - lpSS->Row.HeaderCnt);
         VOverlap = min(VPhysOverlap, lpSS->Virtual.VPhysSize);
         RowNew = max(lpSS->Row.HeaderCnt, lpSS->Virtual.VTop - RowsNeeded); // RFW - 2/11/00
         }

      // Clear any un-needed rows

      if (VOverlap <= lpSS->Virtual.VPhysSize - 1)
         SS_VClear(lpSS, lpSS->Virtual.VTop + VOverlap,
                   lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1);
      }

   if (fRet && RowsNeeded)
      {
      VQueryData.RowsNeeded = RowsNeeded;

      if (lpSS->lpBook->wScrollDirection == SS_VSCROLL_HOME)
         {
         VQueryData.wDirection = SS_VTOP;
         VQueryData.Row = lpSS->Row.HeaderCnt;
         }
      else
         {
         VQueryData.wDirection = SS_VUP;
         VQueryData.Row = RowNew;
         }

      VQueryDataRowOffset = VQueryData.Row;
      RowNew = VQueryData.Row;         // 6/5/93
      lpSS->Virtual.VTop = RowNew;

		SS_AdjustCellCoordsOut(lpSS, NULL, &VQueryData.Row);
      SS_SendMsg(lpSS->lpBook, lpSS, SSM_VQUERYDATA, GetWindowID(lpSS->lpBook->hWnd),
                 (LPARAM)(LPSS_VQUERYDATA)&VQueryData);
		SS_AdjustCellCoords(lpSS, NULL, &VQueryData.Row);

      VQueryDataRowOffset = VQueryData.Row - VQueryDataRowOffset;

      VQueryData.RowsLoaded = min(VQueryData.RowsLoaded, RowsNeeded);
      lpSS->Virtual.VPhysSize = VQueryData.RowsLoaded + VOverlap;

      if (VOverlap != 0 && VQueryData.RowsLoaded < RowsNeeded)
         {
         SS_MoveRange(lpSS, SS_ALLCOLS, lpSS->Virtual.VTop,
                      SS_ALLCOLS, lpSS->Virtual.VTop +
                      VQueryData.RowsLoaded - 1, SS_ALLCOLS,
                      lpSS->Virtual.VTop + (RowsNeeded -
                      VQueryData.RowsLoaded));

         lpSS->Virtual.VTop += RowsNeeded - VQueryData.RowsLoaded;
         }

      if (lpSS->fVirtualMode &&
          (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER))
         SS_SetVScrollBar(lpSS);
      }
   }

/************************************************
* Check to see if requested row is below buffer
************************************************/

else if (BottomVisCell >= lpSS->Virtual.VTop +
         lpSS->Virtual.VPhysSize)
   {
   VOverlap = 0;
   RowNew = lpSS->Row.UL;
   VNewTop = lpSS->Row.UL;

   if (lpSS->Virtual.VPhysSize)
      {
      if (lpSS->Row.UL - lpSS->Virtual.VTop >
          lpSS->Virtual.VPhysSize)
         {
         if (lpSS->Virtual.VTop <= lpSS->Virtual.VTop +
             lpSS->Virtual.VPhysSize - 1)
            SS_VClear(lpSS, lpSS->Virtual.VTop, lpSS->Virtual.VTop +
                      lpSS->Virtual.VPhysSize - 1);
         }
      else
         {
         VOverlap = min(VPhysOverlap, lpSS->Virtual.VPhysSize);
         RowNew = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize;
         VNewTop = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize -
                   VOverlap;

         if (lpSS->Virtual.VTop <= VNewTop - 1)
            fRet = SS_VClear(lpSS, lpSS->Virtual.VTop, VNewTop - 1);
         }
      }

   if (fRet)
      {
      VQueryData.Row = RowNew;

      if (lpSS->lpBook->wScrollDirection == SS_VSCROLL_END)
         {
         VQueryData.wDirection = SS_VBOTTOM;
// RFW - 10/16/2002         if (VOverlap == 0)
            {
            VQueryData.Row = max(lpSS->Row.HeaderCnt, SS_GetRowCnt(lpSS) - VSize); // RFW - 2/11/00
            VNewTop = VQueryData.Row;
            }
         }
      else
         VQueryData.wDirection = SS_VDOWN;

      VQueryData.RowsNeeded = min(VSize, SS_GetRowCnt(lpSS) - VQueryData.Row);

      if (VQueryData.RowsNeeded)
         {
         VQueryDataRowOffset = VQueryData.Row;
         lpSS->Virtual.VTop = VNewTop;

			SS_AdjustCellCoordsOut(lpSS, NULL, &VQueryData.Row);
         SS_SendMsg(lpSS->lpBook, lpSS, SSM_VQUERYDATA, GetWindowID(lpSS->lpBook->hWnd),
                    (LPARAM)(LPSS_VQUERYDATA)&VQueryData);
			SS_AdjustCellCoords(lpSS, NULL, &VQueryData.Row);

         VQueryDataRowOffset = VQueryData.Row - VQueryDataRowOffset;

         lpSS->Virtual.VPhysSize = VQueryData.RowsLoaded + VOverlap;

         if (VQueryData.fAtBottom && lpSS->Virtual.VTop +
             lpSS->Virtual.VPhysSize < SS_GetRowCnt(lpSS) &&
             lpSS->Virtual.VMax == -1)
            {
            SS_SetMaxRows(lpSS, lpSS->Virtual.VTop +
                          lpSS->Virtual.VPhysSize - lpSS->Row.HeaderCnt);
            fAutoSize = TRUE;
            }

         if (lpSS->fVirtualMode &&
             (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER))
            SS_SetVScrollBar(lpSS);
         }
      }
   }

if (VQueryData.fAtTop && lpSS->Virtual.VTop > lpSS->Row.HeaderCnt)
   {
   SS_MoveRange(lpSS, SS_ALLCOLS, lpSS->Virtual.VTop, SS_ALLCOLS,
                lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1,
                SS_ALLCOLS, lpSS->Row.HeaderCnt);

   lpSS->Row.UL = max(lpSS->Row.HeaderCnt, lpSS->Row.UL -
                         (lpSS->Virtual.VTop - lpSS->Row.HeaderCnt));
   lpSS->Row.CurAt = max(lpSS->Row.HeaderCnt, lpSS->Row.CurAt -
                         (lpSS->Virtual.VTop - lpSS->Row.HeaderCnt));

   lpSS->Virtual.VTop = lpSS->Row.HeaderCnt;
   lpSS->Row.DataCnt = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize;
   SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
   }

if (VQueryDataRowOffset)
   {
   VNewTop = max(lpSS->Row.HeaderCnt, lpSS->Virtual.VTop +
                 VQueryDataRowOffset);

   if (VNewTop != lpSS->Virtual.VTop)
      {
      SS_MoveRange(lpSS, SS_ALLCOLS, lpSS->Virtual.VTop, SS_ALLCOLS,
                   lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1,
                   SS_ALLCOLS, VNewTop);

      lpSS->Row.UL = max(lpSS->Row.HeaderCnt, lpSS->Row.UL -
                            (lpSS->Virtual.VTop - VNewTop));
      lpSS->Row.CurAt = max(lpSS->Row.HeaderCnt, lpSS->Row.CurAt -
                            (lpSS->Virtual.VTop - VNewTop));

      lpSS->Virtual.VTop = VNewTop;
      lpSS->Row.DataCnt = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize;
      SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
      }
   }

//#ifdef SPREAD_JPN
//- JPNFIX0019 - (Masanori Iwasa)
if (lpSS->Row.AllocCnt < lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize)
   lpSS->Row.AllocCnt = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize;
//#else
//lpSS->Row.AllocCnt = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize;
//#endif
lpSS->lpBook->Redraw = fRedraw;
lpSS->fProcessingVQueryData = FALSE;

if (fAutoSize)
   SS_AutoSize(lpSS->lpBook, FALSE);

// BJO 23Apr97 GRB5681 - Before fix
//return (VQueryData.fAtTop || VQueryData.fAtBottom ||
//        0 == VQueryData.RowsLoaded);
// BJO 23Apr97 GRB5681 - Begin fix
return (VQueryData.fAtBottom || 0 == VQueryData.RowsLoaded);
// BJO 23Apr97 GRB5681 - End fix
}


void SS_VRefreshBuffer(LPSPREADSHEET lpSS, BOOL fTop)
{
SS_VQUERYDATA VQueryData;
SS_COORD      BottomVisCell;
SS_COORD      RowsPerScreen;
SS_COORD      RowOld;
SS_COORD      Size;
BOOL          fRedraw;

fRedraw = lpSS->lpBook->Redraw;
lpSS->lpBook->Redraw = FALSE;

_fmemset(&VQueryData, '\0', sizeof(SS_VQUERYDATA));

#ifdef SS_BOUNDCONTROL
{
LPSS_ROW lpRow;

lpSS->hlstrVRefreshBookmark = 0;

if (lpRow = SS_LockRowItem(lpSS, lpSS->Virtual.VTop))
	{
   if (lpRow->hlstrBookMark)
      lpSS->hlstrVRefreshBookmark = ssVBCreateHlstr(lpRow->hlstrBookMark, ssVBGetHlstrLen(lpRow->hlstrBookMark));
   SS_UnlockRowItem(lpSS, lpSS->Virtual.VTop);
   }
}
#endif

SS_VClear(lpSS, lpSS->Virtual.VTop, lpSS->Virtual.VTop +
          lpSS->Virtual.VPhysSize - 1);

if (fTop)
   VQueryData.Row = lpSS->Row.HeaderCnt;
else
   VQueryData.Row = max(lpSS->Virtual.VTop, lpSS->Row.HeaderCnt);

VQueryData.wDirection = SS_VREFRESH;

BottomVisCell = SS_GetBottomCell(lpSS, lpSS->Row.UL);

RowsPerScreen = BottomVisCell - lpSS->Row.UL + 1;
Size = max(lpSS->Virtual.VSize, RowsPerScreen);

// RFW - 4/27/00 - KEM62
if (BottomVisCell > VQueryData.Row + Size - 1)
   VQueryData.Row = BottomVisCell - Size + 1;

// RFW - 9/7/94

if (VQueryData.Row + Size > SS_GetRowCnt(lpSS))
   Size = SS_GetRowCnt(lpSS) - VQueryData.Row;

if (fTop)
   VQueryData.RowsNeeded = max(lpSS->Virtual.VSize, RowsPerScreen);
else
   VQueryData.RowsNeeded = max(max(lpSS->Virtual.VSize, RowsPerScreen),
                               lpSS->Virtual.VPhysSize);

if (VQueryData.RowsNeeded && Size > 0)
   {
   VQueryData.RowsNeeded = min(VQueryData.RowsNeeded, Size);

   RowOld = VQueryData.Row;
	SS_AdjustCellCoordsOut(lpSS, NULL, &VQueryData.Row);
   SS_SendMsg(lpSS->lpBook, lpSS, SSM_VQUERYDATA, GetWindowID(lpSS->lpBook->hWnd),
              (LPARAM)(LPSS_VQUERYDATA)&VQueryData);
	SS_AdjustCellCoords(lpSS, NULL, &VQueryData.Row);

   lpSS->Virtual.VTop = VQueryData.Row;
   lpSS->Virtual.VPhysSize = VQueryData.RowsLoaded;

   if (VQueryData.fAtBottom && lpSS->Virtual.VTop +
       lpSS->Virtual.VPhysSize < SS_GetRowCnt(lpSS) &&
       lpSS->Virtual.VMax == -1)
      SS_SetMaxRows(lpSS, lpSS->Virtual.VTop +
                    lpSS->Virtual.VPhysSize - lpSS->Row.HeaderCnt);

   if (lpSS->fVirtualMode &&
       (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER))
      SS_SetVScrollBar(lpSS);
   }

#ifdef SS_BOUNDCONTROL
if (lpSS->hlstrVRefreshBookmark)
	{
//   void DLLENTRY vbSS_SetBookmark(LPVOID lpDestBookMark, LPVOID srcBookMark);   

//   vbSS_SetBookmark((LPVOID)&lpSS->hlstrVRefreshBookmark, NULL);   
	lpSS->hlstrVRefreshBookmark = 0;
	}
#endif

if (VQueryData.fAtTop && lpSS->Virtual.VTop > lpSS->Row.HeaderCnt)
   {
   SS_MoveRange(lpSS, SS_ALLCOLS, lpSS->Virtual.VTop, SS_ALLCOLS,
                lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1,
                SS_ALLCOLS, lpSS->Row.HeaderCnt);

   lpSS->Row.UL = max(lpSS->Row.HeaderCnt, lpSS->Row.UL -
                         (lpSS->Virtual.VTop - lpSS->Row.HeaderCnt));
   lpSS->Row.CurAt = max(lpSS->Row.HeaderCnt, lpSS->Row.CurAt -
                         (lpSS->Virtual.VTop - lpSS->Row.HeaderCnt));

   lpSS->Virtual.VTop = lpSS->Row.HeaderCnt;
   SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
   }

if (lpSS->Virtual.VTop > lpSS->Row.UL || lpSS->Virtual.VTop +
    lpSS->Virtual.VPhysSize - 1 < BottomVisCell)
   {
   SS_ShowCell(lpSS, lpSS->Col.UL, lpSS->Virtual.VTop, SS_SHOW_TOPLEFT);

   if (RowOld != VQueryData.Row)
      lpSS->Row.CurAt = lpSS->Row.UL;
   }

lpSS->lpBook->Redraw = fRedraw;
}


BOOL SS_VClear(LPSPREADSHEET lpSS, SS_COORD Row1, SS_COORD Row2)
{
SS_VCLEARDATA VClearData;

VClearData.Row = Row1;
VClearData.RowsBeingCleared = Row2 - Row1 + 1;

if (VClearData.RowsBeingCleared > 0)
	{
	SS_AdjustCellCoordsOut(lpSS, NULL, &VClearData.Row);
   if (!SS_SendMsg(lpSS->lpBook, lpSS, SSM_VCLEARDATA, GetWindowID(lpSS->lpBook->hWnd),
                   (LPARAM)(LPSS_VCLEARDATA)&VClearData))
      return (SS_ClearRange(lpSS, SS_ALLCOLS, Row1, SS_ALLCOLS, Row2, FALSE));
	}

return (TRUE);
}


BOOL DLLENTRY SSVRefreshBuffer(HWND hWnd)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_VRefreshBuffer(lpSS, FALSE);
SS_SheetUnlock(hWnd);

return (TRUE);
}


#ifndef SS_NOSCBAR

BOOL DLLENTRY SSVScrollSetSpecial(HWND hWnd, BOOL fUseSpecialVScroll,
                                  WORD wOptions)
{
LPSS_BOOK lpBook;
BOOL      fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_VScrollSetSpecial(lpBook, fUseSpecialVScroll, wOptions);
SS_BookUnlock(hWnd);
return fRet;
}

BOOL SS_VScrollSetSpecial(LPSS_BOOK lpBook, BOOL fUseSpecialVScroll,
                          WORD wOptions)
{
long lStyle;
BOOL fRet;

fRet = lpBook->fUseSpecialVScroll;
lpBook->fUseSpecialVScroll = fUseSpecialVScroll;
lpBook->wSpecialVScrollOptions = wOptions;

if (!lpBook->hWnd)  // handle OCX control without a HWND
   return fRet;

if (fUseSpecialVScroll)
   {
   lStyle = SBS_VERT | SBRS_HOMEEND | SBRS_UPDN | SBRS_PAGEUPDN;

   if (wOptions & SS_VSCROLL_NOHOMEEND)
      lStyle &= ~SBRS_HOMEEND;

   if (wOptions & SS_VSCROLL_NOUPDN)
      lStyle &= ~SBRS_UPDN;

   if (wOptions & SS_VSCROLL_NOPAGEUPDN)
      lStyle &= ~SBRS_PAGEUPDN;

   if (fRet)
      {
      SetWindowLong(lpBook->hWndVScroll, GWL_STYLE,
                    (GetWindowLong(lpBook->hWndVScroll, GWL_STYLE) &
                    ~(SBRS_HOMEEND | SBRS_UPDN | SBRS_PAGEUPDN)) | lStyle);	  
      InvalidateRect(lpBook->hWndVScroll, NULL, TRUE);
      UpdateWindow(lpBook->hWndVScroll);
      }
   else
      {
	   BOOL fIsHorizontal = FALSE;
      if (lpBook->hWndVScroll)
         DestroyWindow(lpBook->hWndVScroll);

      lpBook->hWndVScroll = CreateWindow(
                              SSClassNames[dClassNameIndex].TBScrlBar, NULL,
                              WS_CHILD | WS_VISIBLE | lStyle,
                              0, 0, 0, 0, lpBook->hWnd, (HMENU)SBS_VERT, hDynamicInst, (LPVOID)(LPBOOL)&fIsHorizontal);

      SS_AutoSize(lpBook, FALSE);
      }
#ifdef SS_V80
   SendMessage(lpBook->hWndVScroll, SBR_SETSTYLE, (WPARAM)lpBook->wScrollBarStyle == 0 ? lpBook->wAppearanceStyle : lpBook->wScrollBarStyle-1, 0L);
   if (lpBook->hWndVScroll)
		ScrlBarEnhancedScrollBarColors(lpBook->hWndVScroll, lpBook->scrollTrackColor, lpBook->scrollArrowColor, lpBook->scrollUpperNormalStartColor, lpBook->scrollUpperNormalEndColor,
										   lpBook->scrollLowerNormalStartColor, lpBook->scrollLowerNormalEndColor, lpBook->scrollUpperPushedStartColor, lpBook->scrollUpperPushedEndColor,
										   lpBook->scrollLowerPushedStartColor, lpBook->scrollLowerPushedEndColor, lpBook->scrollUpperHoverStartColor, lpBook->scrollUpperHoverEndColor,
										   lpBook->scrollLowerHoverStartColor, lpBook->scrollLowerHoverEndColor, lpBook->scrollHoverBorderColor, lpBook->scrollBorderColor);
#endif
   }

#ifdef SS_V80
else if (lpBook->wAppearanceStyle != 2 && lpBook->wScrollBarStyle != 3)
#else
else
#endif
   {
   if (fRet)
      {
      if (lpBook->hWndVScroll)
         DestroyWindow(lpBook->hWndVScroll);

      lpBook->hWndVScroll = CreateWindow(_T("ScrollBar"), NULL, WS_CHILD |
                                           WS_VISIBLE | SBS_VERT, 0, 0, 0, 0,
                                           lpBook->hWnd, (HMENU)SBS_VERT, hDynamicInst, NULL);

#ifdef WIN32
{
SCROLLINFO si;
WORD       wLoWordVersion = LOWORD(GetVersion());

if (HIBYTE(wLoWordVersion) == 95 ||
    (LOBYTE(wLoWordVersion) == 4 && HIBYTE(wLoWordVersion) == 0))
   {
   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_PAGE;
   si.nPage = 1;

   SendMessage(lpBook->hWndHScroll, SBM_SETSCROLLINFO, 0, (LPARAM)(LPVOID)&si);

   if(!lpBook->fUseSpecialVScroll )
      SendMessage(lpBook->hWndVScroll, SBM_SETSCROLLINFO, 0, (LPARAM)(LPVOID)&si);
   }
}

      SS_AutoSize(lpBook, FALSE);
      }
   }
#endif
#ifdef SS_V80
else
{
      lStyle = SBRS_UPDN | SBRS_THUMB;	  
      SetWindowLong(lpBook->hWndVScroll, GWL_STYLE,
                    (GetWindowLong(lpBook->hWndVScroll, GWL_STYLE) &
                    ~(SBRS_HOMEEND | SBRS_UPDN | SBRS_PAGEUPDN)) | lStyle);
      InvalidateRect(lpBook->hWndVScroll, NULL, TRUE);
      UpdateWindow(lpBook->hWndVScroll);
	  SendMessage(lpBook->hWndVScroll, SBM_SETPOS, 1, FALSE);

}
#endif

return fRet;
}


BOOL DLLENTRY SSVScrollGetSpecial(HWND hWnd, LPWORD lpwOptions)
{
LPSS_BOOK lpBook;
BOOL      fRet;

lpBook = SS_BookLock(hWnd);
fRet = SS_VScrollGetSpecial(lpBook, lpwOptions);
SS_BookUnlock(hWnd);
return fRet;
}


BOOL SS_VScrollGetSpecial(LPSS_BOOK lpBook, LPWORD lpwOptions)
{
if (lpwOptions)
   *lpwOptions = lpBook->wSpecialVScrollOptions;
return lpBook->fUseSpecialVScroll;
}

#endif


#ifndef SS_GRID

LONG DLLENTRY SSVSetStyle(HWND hWnd, LONG lStyle)
{
LPSPREADSHEET lpSS;
long          lRet;

lpSS = SS_SheetLock(hWnd);
lRet = SS_VSetStyle(lpSS, lStyle);
SS_SheetUnlock(hWnd);

return (lRet);
}


LONG SS_VSetStyle(LPSPREADSHEET lpSS, LONG lStyle)
{
long          lRet;

lRet = lpSS->Virtual.lVStyle;
lpSS->Virtual.lVStyle = lStyle;
return (lRet);
}


LONG DLLENTRY SSVGetStyle(HWND hWnd)
{
LPSPREADSHEET lpSS;
long          lRet;

lpSS = SS_SheetLock(hWnd);
lRet = SS_VGetStyle(lpSS);
SS_SheetUnlock(hWnd);

return (lRet);
}


LONG SS_VGetStyle(LPSPREADSHEET lpSS)
{
return lpSS->Virtual.lVStyle;
}


void DLLENTRY SSVSetBufferSize(HWND hWnd, SS_COORD BufferSize,
                               SS_COORD Overlap)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_VSetBufferSize(lpSS, BufferSize, Overlap);
SS_SheetUnlock(hWnd);
}


void SS_VSetBufferSize(LPSPREADSHEET lpSS, SS_COORD BufferSize,
                       SS_COORD Overlap)
{
lpSS->Virtual.VSize = BufferSize;
lpSS->Virtual.VOverlap = Overlap;
}


void DLLENTRY SSVGetBufferSize(HWND hWnd, LPSS_COORD lpBufferSize,
                               LPSS_COORD lpOverlap)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_VGetBufferSize(lpSS, lpBufferSize, lpOverlap);
SS_SheetUnlock(hWnd);
}


void SS_VGetBufferSize(LPSPREADSHEET lpSS, LPSS_COORD lpBufferSize,
                       LPSS_COORD lpOverlap)
{
if (lpBufferSize)
   *lpBufferSize = lpSS->Virtual.VSize;
if (lpOverlap)
   *lpOverlap = lpSS->Virtual.VOverlap;
}


SS_COORD DLLENTRY SSVGetPhysBufferTop(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD      Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_VGetPhysBufferTop(lpSS);
SS_AdjustCellCoordsOut(lpSS, NULL, &Ret);
SS_SheetUnlock(hWnd);

return (Ret);
}


SS_COORD SS_VGetPhysBufferTop(LPSPREADSHEET lpSS)
{
return lpSS->Virtual.VTop;
}


SS_COORD DLLENTRY SSVGetPhysBufferSize(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD      Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_VGetPhysBufferSize(lpSS);
SS_SheetUnlock(hWnd);
return (Ret);
}


SS_COORD SS_VGetPhysBufferSize(LPSPREADSHEET lpSS)
{
return lpSS->Virtual.VPhysSize;
}

#endif


SS_COORD DLLENTRY SSVSetMax(HWND hWnd, SS_COORD Max)
{
LPSPREADSHEET lpSS;
SS_COORD      Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_VSetMax(lpSS, Max);
SS_SheetUnlock(hWnd);
return Ret;
}


SS_COORD SS_VSetMax(LPSPREADSHEET lpSS, SS_COORD Max)
{
SS_COORD      Ret;

Ret = lpSS->Virtual.VMax;
lpSS->Virtual.VMax = Max;

if (lpSS->fVirtualMode)
   {
   if (Max == -1)
      SS_SetMaxRows(lpSS, SS_VMAXCOORD);
   else
      {
      SS_SetMaxRows(lpSS, Max);

      if (Max < lpSS->Virtual.VTop)
         {
         lpSS->Virtual.VTop = lpSS->Row.HeaderCnt;
         lpSS->Virtual.VPhysSize = 0;
         }

      else if (Max < lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - lpSS->Row.HeaderCnt)
         lpSS->Virtual.VPhysSize = Max - lpSS->Virtual.VTop + lpSS->Row.HeaderCnt;
      }
   }

return (Ret);
}


SS_COORD DLLENTRY SSVGetMax(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD      Ret;

lpSS = SS_SheetLock(hWnd);
Ret = SS_VGetMax(lpSS);
SS_SheetUnlock(hWnd);

return (Ret);
}


SS_COORD SS_VGetMax(LPSPREADSHEET lpSS)
{
return lpSS->Virtual.VMax;
}

#endif
