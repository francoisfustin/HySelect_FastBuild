/*********************************************************
* SS_SCROL.C
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* RAP01 - 8977                                    5.25.01
*********************************************************/

#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_main.h"
#include "ss_multi.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_span.h"
#include "ss_type.h"
#include "ss_win.h"
#ifdef SS_V70
#include "ss_book.h"
#endif // SS_V70
#ifdef SS_V80
#include "ss_scbar.h"
#endif

void                   SSx_InvalidateColArea(LPSPREADSHEET lpSS);
void                   SSx_InvalidateRowArea(LPSPREADSHEET lpSS);
BOOL                   SS_AllCellsHaveData(LPSPREADSHEET lpSS, SS_COORD Row);
void                   SS_UpdateRestrictCnt(LPSPREADSHEET lpSS,
                                            LPSS_COORD lpDataColCnt,
                                            LPSS_COORD lpDataRowCnt);
void                   SS_DrawCell(HDC hDC, HWND hWnd, LPSPREADSHEET lpSS,
                                   LPSS_COL lpCol, LPSS_ROW lpRow,
                                   LPRECT lpRect, SS_COORD CellCol,
                                   SS_COORD CellRow);

extern HANDLE hDynamicInst;


BOOL SS_ScrollHome(lpSS, fFlags)

LPSPREADSHEET lpSS;
WORD          fFlags;
{
BOOL          bRow;
BOOL          bMoveActiveCell;
SS_COORD      PosAtOld;
SS_COORD      CurAtTemp;
SS_COORD      ULTemp;
SS_COORD      BlockCellCurrentPos;
SS_COORD      MaxCnt;
LPSS_ROWCOL       lpRC;          
int           (*lpfnGetSize)(LPSPREADSHEET, SS_COORD);

if (!SS_CellEditModeOff(lpSS, 0))
   return (FALSE);

bRow = ((fFlags & SS_F_SCROLL_ROW) == SS_F_SCROLL_ROW);
bMoveActiveCell = ((fFlags & SS_F_MOVEACTIVECELL) == SS_F_MOVEACTIVECELL);

if (bRow) 
   {
   lpRC = &(lpSS->Col);
   MaxCnt = SS_GetColCnt(lpSS);
   lpfnGetSize = SS_GetColWidthInPixels;
   }
else // home of col
   {
   lpRC = &(lpSS->Row);
   MaxCnt = SS_GetRowCnt(lpSS);
   lpfnGetSize = SS_GetRowHeightInPixels;
   }

PosAtOld = lpRC->CurAt;
ULTemp = lpRC->UL;

BlockCellCurrentPos = lpRC->BlockCellCurrentPos;

if (bMoveActiveCell)
   {
   CurAtTemp = lpRC->HeaderCnt;
   while (CurAtTemp < MaxCnt - 1 && (lpfnGetSize(lpSS, CurAtTemp) == 0 ||
          (lpSS->lpBook->fEditModePermanent &&
          SS_GetLock(lpSS, bRow ? CurAtTemp : lpSS->Col.CurAt,
                     bRow ? lpSS->Row.CurAt : CurAtTemp, FALSE))))
      CurAtTemp++;

#ifdef SS_UTP
   if (bRow && CurAtTemp == lpRC->HeaderCnt + lpRC->Frozen - 1)
      {
      CurAtTemp++;

      while (CurAtTemp < MaxCnt - 1 && (lpfnGetSize(lpSS, CurAtTemp) == 0 ||
             (lpSS->lpBook->fEditModePermanent &&
             SS_GetLock(lpSS, bRow ? CurAtTemp : lpSS->Col.CurAt,
                        bRow ? lpSS->Row.CurAt : CurAtTemp, FALSE))))
         CurAtTemp++;
      }
#endif
   }

if (bMoveActiveCell && lpRC->UL == lpRC->HeaderCnt +
    lpRC->Frozen && lpRC->CurAt <= lpRC->LR)
   {
   if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
      {
      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bRow)
         lpSS->Row.CurAt = CurAtTemp;

      SSx_SelectBlock(lpSS,  bRow ? CurAtTemp : lpSS->Col.BlockCellCurrentPos,
                      bRow ? lpSS->Row.BlockCellCurrentPos : CurAtTemp);
      }
   else
      {
      SS_HighlightCell(lpSS, FALSE);
      lpRC->CurAt = CurAtTemp;

      if (SS_LeaveCell(lpSS, bRow ? PosAtOld : -1, -1, 
                       bRow ? -1 : PosAtOld, -1, 0))
         SS_HighlightCell(lpSS, TRUE);
      }
   }

else if (lpRC->UL > lpRC->HeaderCnt + lpRC->Frozen)
   {
   SS_HighlightCell(lpSS, FALSE);

   lpRC->UL = lpRC->HeaderCnt + lpRC->Frozen;
   while (lpSS->Col.UL < MaxCnt - 1 && lpfnGetSize(lpSS, lpRC->UL) == 0)
      lpRC->UL++;

   if (bMoveActiveCell)
      {
      if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
         {
         BlockCellCurrentPos = CurAtTemp;

         if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bRow)
            lpSS->Row.CurAt = BlockCellCurrentPos;
         }
      else
         {
         lpRC->CurAt = CurAtTemp;

         if (!SS_LeaveCell(lpSS, bRow ? PosAtOld : -1,
                           bRow ? ULTemp : -1,
                           bRow ? -1 : PosAtOld,
                           bRow ? -1 : ULTemp, 0))
            return (TRUE);
         }
      }

   if (bRow)
      SSx_InvalidateColArea(lpSS);
   else
      {
      lpSS->lpBook->wScrollDirection = SS_VSCROLL_HOME;
      SSx_InvalidateRowArea(lpSS);
      }
   
   if (lpSS->lpBook->wMode == SS_MODE_BLOCK && bMoveActiveCell)
      SSx_SelectBlock(lpSS,
                bRow ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
                bRow ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos);
   }

else
   SS_HighlightCell(lpSS, TRUE);

return (TRUE);
} 


void SSx_InvalidateColArea(LPSPREADSHEET lpSS)
{
if (!SS_IsActiveSheet(lpSS))
	return;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

#ifdef SS_UTP
if (lpSS->fUseScrollArrows)
   lpSS->lpBook->Redraw = FALSE;
#endif // SS_UTP

SS_InvalidateColRange(lpSS, lpSS->Col.HeaderCnt + lpSS->Col.Frozen, -1);
SS_SetHScrollBar(lpSS);
SS_LeftColChange(lpSS);

#ifdef SS_UTP
if (lpSS->fUseScrollArrows)
   {
   lpSS->lpBook->Redraw = TRUE;
   SS_SetHScrollBar(lpSS);
   SS_UpdateWindow(lpSS->lpBook);
   }
#endif // SS_UTP

}


void SSx_InvalidateRowArea(LPSPREADSHEET lpSS)
{
if (!SS_IsActiveSheet(lpSS))
	return;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

#ifdef SS_UTP
if (lpSS->fUseScrollArrows)
   lpSS->lpBook->Redraw = FALSE;
#endif // SS_UTP

SS_InvalidateRowRange(lpSS, lpSS->Row.HeaderCnt + lpSS->Row.Frozen, -1);
SS_SetVScrollBar(lpSS);
SS_TopRowChange(lpSS);

#ifdef SS_UTP
if (lpSS->fUseScrollArrows)
   {
   lpSS->lpBook->Redraw = TRUE;
   SS_SetVScrollBar(lpSS);
   SS_UpdateWindow(lpSS->lpBook);
   }
#endif // SS_UTP
}


BOOL SS_ScrollEnd(LPSPREADSHEET lpSS, WORD fFlags)
{
int           iSSSize;
int           iCellPos;
int           iRectSize;
RECT          Rect;
BOOL          bRow;
BOOL          bMoveActiveCell;
SS_COORD      CurAt;
SS_COORD      CurAtOld;
SS_COORD      ULOld;
SS_COORD      Cnt;
SS_COORD      BlockCellCurrentPos;
LPSS_ROWCOL   lpRC;
int           (*lpfnGetSize)(LPSPREADSHEET, SS_COORD);

if (!SS_CellEditModeOff(lpSS, 0))
   return (FALSE);

bRow = ((fFlags & SS_F_SCROLL_ROW) == SS_F_SCROLL_ROW);
bMoveActiveCell = ((fFlags & SS_F_MOVEACTIVECELL) == SS_F_MOVEACTIVECELL);

if (bRow) 
   {
   lpRC = &(lpSS->Col);
   lpfnGetSize = SS_GetColWidthInPixels;
   }
else // end of col
   {
   lpRC = &(lpSS->Row);
   lpfnGetSize = SS_GetRowHeightInPixels;
   }

CurAtOld = lpRC->CurAt;
ULOld = lpRC->UL;

BlockCellCurrentPos = lpRC->BlockCellCurrentPos;

if (fFlags & SS_F_SCROLLMAX)
   {
   if (bRow)
      Cnt = SS_GetColCnt(lpSS) - 1;
   else
      Cnt = SS_GetRowCnt(lpSS) - 1;
   }
else
	{
	// RFW - 6/10/04 - 14340
#ifdef SS_V40
	SS_COORD lColLastSpan;
	SS_COORD lRowLastSpan;
	BOOL     fSpan = FALSE;

	fSpan = SS_GetSpanLR(lpSS, &lColLastSpan, &lRowLastSpan);
#endif // SS_V40

   Cnt = lpRC->DataCnt - 1;

#ifdef SS_V40
	if (fSpan)
		{
	   if (bRow)
			Cnt = max(Cnt, lColLastSpan);
		else
			Cnt = max(Cnt, lRowLastSpan);
		}
#endif // SS_V40
	}

if (Cnt == lpRC->HeaderCnt - 1 && Cnt < lpRC->Max + lpRC->HeaderCnt - 1)
   {
   Cnt++;

   while (Cnt < lpRC->Max + lpRC->HeaderCnt - 1 && (lpfnGetSize(lpSS, Cnt) == 0 ||
          (lpSS->lpBook->fEditModePermanent &&
          SS_GetLock(lpSS, bRow ? Cnt : lpSS->Col.CurAt,
                     bRow ? lpSS->Row.CurAt : Cnt, FALSE))))
      Cnt++;
   }

// SPRD024 Wei Feng 1997/9/26
#ifdef	SPREAD_JPN1
while (Cnt > lpRC->HeaderCnt && lpfnGetSize(lpSS, Cnt) == 0)
#else
while (Cnt > lpRC->HeaderCnt && (lpfnGetSize(lpSS, Cnt) == 0 ||
       (bMoveActiveCell && lpSS->lpBook->fEditModePermanent &&
       SS_GetLock(lpSS, bRow ? Cnt : lpSS->Col.CurAt,
                  bRow ? lpSS->Row.CurAt : Cnt, FALSE))))
#endif
   Cnt--;

if (Cnt >= lpRC->UL && Cnt <= lpRC->LRAllVis)
   {
   if (bMoveActiveCell)
      {
      if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
         {
         if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bRow)
            lpSS->Row.CurAt = Cnt;

         SSx_SelectBlock(lpSS,
                         bRow ? Cnt : lpSS->Col.BlockCellCurrentPos,
                         bRow ? lpSS->Row.BlockCellCurrentPos : Cnt);
         }
      else
         {
         SS_HighlightCell(lpSS, FALSE);
         lpRC->CurAt = Cnt;

         if (SS_LeaveCell(lpSS, bRow ? CurAtOld : -1, -1,
                          bRow ? -1 : CurAtOld, -1, 0))
            SS_HighlightCell(lpSS, TRUE);
         }
      }
   }
else
   {
#ifdef SS_UTP
   {
   short dScrollArrowWidthOld = lpSS->dScrollArrowWidth;
   short dScrollArrowHeightOld = lpSS->dScrollArrowHeight;

   lpSS->dScrollArrowWidth = 0;
   lpSS->dScrollArrowHeight = 0;

   SS_GetClientRect(lpSS->lpBook, &Rect);

   lpSS->dScrollArrowWidth = dScrollArrowWidthOld;
   lpSS->dScrollArrowHeight = dScrollArrowHeightOld;
   }
#else
   SS_GetClientRect(lpSS->lpBook, &Rect);
#endif

   if (bRow)                    
      { 
      iCellPos = SS_GetCellPosX(lpSS, lpRC->UL, lpRC->UL);
      iRectSize = Rect.right - Rect.left;
      }
   else
      {
      iCellPos = SS_GetCellPosY(lpSS, lpRC->UL, lpRC->UL);
      iRectSize = Rect.bottom - Rect.top;
      }
   
   for (CurAt = Cnt + 1; CurAt > lpRC->HeaderCnt +
        lpRC->Frozen && iCellPos <= iRectSize; )
      {
      if (--CurAt < lpRC->HeaderCnt + lpRC->Frozen)
         break;

      iSSSize = lpfnGetSize(lpSS, CurAt);

      if (iCellPos + iSSSize > iRectSize)
         {
         CurAt++;
         break;
         }

      iCellPos += iSSSize;
      }

   SS_HighlightCell(lpSS, FALSE);

   lpRC->UL = max(lpRC->HeaderCnt + lpRC->Frozen, CurAt);

   if (bMoveActiveCell)
      {
      if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
         {
         BlockCellCurrentPos = Cnt;
         if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bRow)
            lpSS->Row.CurAt = Cnt;
         }
      else
         {
#if 0
         if (bRow)
            lpRC->CurAt = Cnt;
         else
            lpRC->CurAt = lpRC->UL;
#endif

         lpRC->CurAt = Cnt;

         if (!SS_LeaveCell(lpSS, bRow ? CurAtOld : -1,
                           bRow ? ULOld : -1,
                           bRow ? -1 : CurAtOld,
                           bRow ? -1 : ULOld, 0))
            return (TRUE);
         }
      }

   if (lpRC->UL != ULOld)
      {
      if (bRow)
         SSx_InvalidateColArea(lpSS);
      else
         {
         lpSS->lpBook->wScrollDirection = SS_VSCROLL_END;
         SSx_InvalidateRowArea(lpSS);
         }

      if (lpSS->lpBook->wMode == SS_MODE_BLOCK && bMoveActiveCell)
         SSx_SelectBlock(lpSS,
                   bRow ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
                   bRow ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos);
      }
   
   if (!bRow)
      {
      if (bMoveActiveCell && !(lpSS->lpBook->wMode == SS_MODE_BLOCK))
         SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);

      if (lpSS->Row.UL != ULOld)
         lpSS->lpBook->wScrollDirection = SS_VSCROLL_END;
      }

   SS_HighlightCell(lpSS, TRUE);
   }

return (TRUE);
}


void SS_GetScrollArea(LPSPREADSHEET lpSS, LPRECT lpRect, short ScrollArea)
{
SS_COORD i;

SS_GetClientRect(lpSS->lpBook, lpRect);

switch (ScrollArea)
   {
   case SS_SCROLLAREA_ALL:
      break;

   case SS_SCROLLAREA_COLS:
      for (i = 0; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen; i++)
         lpRect->left += SS_GetColWidthInPixels(lpSS, i);

      break;

   case SS_SCROLLAREA_ROWS:
      for (i = 0; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen; i++)
         lpRect->top += SS_GetRowHeightInPixels(lpSS, i);

      break;

   case SS_SCROLLAREA_CELLS:
      /*****************
      * Determine Left
      *****************/

      for (i = 0; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen; i++)
         lpRect->left += SS_GetColWidthInPixels(lpSS, i);

      /****************
      * Determine Top
      ****************/

      for (i = 0; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen; i++)
         lpRect->top += SS_GetRowHeightInPixels(lpSS, i);

      break;

   case SS_SCROLLAREA_NONHEADER:
      /*****************
      * Determine Left
      *****************/

      for (i = 0; i < lpSS->Col.HeaderCnt; i++)
         lpRect->left += SS_GetColWidthInPixels(lpSS, i);

      /****************
      * Determine Top
      ****************/

      for (i = 0; i < lpSS->Row.HeaderCnt; i++)
         lpRect->top += SS_GetRowHeightInPixels(lpSS, i);

      break;
   }
}


void SS_InvalidateScrollArea(LPSPREADSHEET lpSS, short ScrollArea)
{
RECT Rect;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (IsWindowVisible(lpSS->lpBook->hWnd) && SS_IsActiveSheet(lpSS))
   {
   SS_GetScrollArea(lpSS, &Rect, ScrollArea);
   SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);

   if (lpSS->lpBook->Redraw)
      SS_UpdateWindow(lpSS->lpBook);
   else
      lpSS->lpBook->fInvalidated = TRUE;
   }
}


void SS_InvalidateCell(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow)

{
RECT          Rect;
RECT          RectTemp;
RECT          RectClient;
HDC           hDC;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
WORD          wOverflow = 0;
BOOL          fDrawSelBlock = FALSE;
BOOL          fTurnEditModeOn = FALSE;
int           x;
int           y;
int           cx;
int           cy;
#ifdef SS_V40
SS_COORD      i;
SS_COORD      MaxCnt;
#endif // SS_V40
BOOL          fDone = FALSE;

#ifndef SS_NOOVERFLOW
RECT RectOverflowAll;
#endif

if (lpSS->lpBook->fNoInvalidate || !SS_IsActiveSheet(lpSS))
   return;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

#ifdef SS_V40
{
SS_COORD lSpanCols, lSpanRows;
if (SS_GetCellSpan(lpSS, CellCol, CellRow, NULL, NULL, &lSpanCols, &lSpanRows) == SS_SPAN_ANCHOR)
	SS_InvalidateCellRange(lpSS, CellCol, CellRow, CellCol + lSpanCols - 1,
                          CellRow + lSpanRows - 1);
}
#endif // SS_V40

if (IsWindowVisible(lpSS->lpBook->hWnd) && SS_IsCellVisible(lpSS, CellCol, CellRow,
    SS_VISIBLE_PARTIAL))
   {
   /*
   SS_GetClientRect(lpSS->lpBook, &Rect);
   */

   SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL,
           CellCol, CellRow, &x, &y, &cx, &cy);

   SetRect(&Rect, x + 1, y + 1, x + cx, y + cy);
   SS_GetClientRect(lpSS->lpBook, &RectClient);

#ifndef SS_NOOVERFLOW
   if (lpSS->lpBook->fAllowCellOverflow)
      wOverflow = SS_GetOverflowRects(lpSS, CellCol, CellRow,
                                      &RectClient, &Rect,
                                      NULL, NULL, NULL,
                                      &RectOverflowAll);
#endif

#ifdef SS_V40
   MaxCnt = SS_GetRowCnt(lpSS);
   for (fDone = FALSE, i = CellRow; i < MaxCnt && !fDone; i++)
      {
		if (lpRow = SS_LockRowItem(lpSS, i))
			{
			if ((i == CellRow && lpRow->bMerge) || lpRow->bMerge == SS_MERGE_RESTRICTED)
				SS_InvalidateRow(lpSS, i);
			else
				fDone = TRUE;

			SS_UnlockRowItem(lpSS, i);
			}
		else
			fDone = TRUE;
		}

   MaxCnt = SS_GetColCnt(lpSS);
   for (fDone = FALSE, i = CellCol; i < MaxCnt && !fDone; i++)
      {
		if (lpCol = SS_LockColItem(lpSS, i))
			{
			if ((i == CellCol && lpCol->bMerge) || lpCol->bMerge == SS_MERGE_RESTRICTED)
				SS_InvalidateCol(lpSS, i);
			else
				fDone = TRUE;

			SS_UnlockColItem(lpSS, i);
			}
		else
			fDone = TRUE;
		}
#endif // SS_V40

   if (wOverflow)
      {
#ifndef SS_NOOVERFLOW
      InflateRect(&RectOverflowAll, 1, 1);
      SS_InvalidateRect(lpSS->lpBook, &RectOverflowAll, TRUE);

      if (lpSS->lpBook->Redraw)
         SS_UpdateWindow(lpSS->lpBook);
      else
         lpSS->lpBook->fInvalidated = TRUE;
#endif
      }

#ifndef SS_NOLISTBOX
   else if (SS_USESELBAR(lpSS) &&
            SS_SelModeSendMessage(lpSS, SS_SELMODE_GETSEL, CellRow, 0L, 0L))
      {
      InflateRect(&Rect, 1, 1);
      SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
      lpSS->lpBook->fInvalidated = TRUE;
      }
#endif

   else if (lpSS->lpBook->Redraw)
      {
      if (CellCol >= lpSS->Col.CurAt - 1 && CellCol <= lpSS->Col.CurAt + 1 &&
          CellRow >= lpSS->Row.CurAt - 1 && CellRow <= lpSS->Row.CurAt + 1)
         SS_HighlightCell(lpSS, FALSE);

      if (lpSS->lpBook->EditModeOn && !lpSS->lpBook->fLeaveEditModeOn && !lpSS->lpBook->EditModeTurningOff &&
          lpSS->Row.CurAt == CellRow && lpSS->Col.CurAt == CellCol)
         {
         BOOL wMessageBeingSentOld = lpSS->lpBook->wMessageBeingSent;
         lpSS->lpBook->wMessageBeingSent = FALSE;
         lpSS->lpBook->fDontSendEditModeMsg = TRUE;      // RFW - 8/23/94
         SS_CellEditModeOff(lpSS, 0);
         lpSS->lpBook->fDontSendEditModeMsg = FALSE;     // RFW - 8/23/94
         lpSS->lpBook->wMessageBeingSent = wMessageBeingSentOld;
         fTurnEditModeOn = TRUE;
         }

		// RFW - 2/15/05 - 15735
      else if (((CellCol >= lpSS->Col.CurAt - 1 &&
                 CellCol <= lpSS->Col.CurAt + 1) || SS_USESINGLESELBAR(lpSS)) &&
               CellRow >= lpSS->Row.CurAt - 1 &&
               CellRow <= lpSS->Row.CurAt + 1 && !lpSS->lpBook->EditModeOn)
         SS_HighlightCell(lpSS, FALSE);

      hDC = SS_GetDC(lpSS->lpBook);

      lpCol = SS_LockColItem(lpSS, CellCol);
      lpRow = SS_LockRowItem(lpSS, CellRow);

      IntersectRect(&RectTemp, &Rect, &RectClient);
#ifdef SS_UTP
      IntersectClipRect(hDC, RectTemp.left, RectTemp.top, RectTemp.right,
                        RectTemp.bottom);
#else
      {
      HRGN hRgn = CreateRectRgnIndirect(&RectTemp);
      SelectClipRgn(hDC, hRgn);
      DeleteObject(hRgn);
      }
#endif

      if (GetFocus() == lpSS->lpBook->hWnd || lpSS->lpBook->fRetainSelBlock ||
          SS_IsBlockSelected(lpSS))
         {
         SS_DrawSelBlock(hDC, lpSS);
         fDrawSelBlock = TRUE;
         }

      SS_DrawCell(hDC, lpSS->lpBook->hWnd, lpSS, lpCol, lpRow, &Rect, CellCol, CellRow);

      /*
      SS_DrawText(hDC, hWnd, lpSS, lpCol, lpRow, &Rect, CellCol,
          CellRow, TRUE, NULL, NULL, NULL, NULL, FALSE);
      */

      if (fDrawSelBlock)
         SS_DrawSelBlock(hDC, lpSS);

#ifndef SS_UTP
      SelectClipRgn(hDC, 0);
#endif

      SS_UnlockColItem(lpSS, CellCol);
      SS_UnlockRowItem(lpSS, CellRow);

      SS_ReleaseDC(lpSS->lpBook, hDC);

      if (fTurnEditModeOn)
         SS_PostSetEditMode(lpSS, TRUE, 1L);


		/* RFW - 2/15/05 - 15735
      else if (CellCol >= lpSS->Col.CurAt - 1 &&
               CellCol <= lpSS->Col.CurAt + 1 &&
               CellRow >= lpSS->Row.CurAt - 1 &&
               CellRow <= lpSS->Row.CurAt + 1 && !lpSS->lpBook->EditModeOn)
		*/
      else if (((CellCol >= lpSS->Col.CurAt - 1 &&
                 CellCol <= lpSS->Col.CurAt + 1) || SS_USESINGLESELBAR(lpSS)) &&
               CellRow >= lpSS->Row.CurAt - 1 &&
               CellRow <= lpSS->Row.CurAt + 1 && !lpSS->lpBook->EditModeOn)
         SS_HighlightCell(lpSS, TRUE);
      }

   else
      {
      InflateRect(&Rect, 1, 1);
      SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
      lpSS->lpBook->fInvalidated = TRUE;
      }
   }
}


void SS_DrawCell(HDC hDC, HWND hWnd, LPSPREADSHEET lpSS, LPSS_COL lpCol,
                 LPSS_ROW lpRow, LPRECT lpRect, SS_COORD CellCol,
                 SS_COORD CellRow)
{
HDC       hDCMemory;
HBITMAP   hBitmapOld;
HBITMAP   hBitmapDC;
RECT      RectTemp;

/*********************************
* Create a memory device context
*********************************/

hBitmapDC = CreateCompatibleBitmap(hDC, lpRect->right - lpRect->left,
                                   lpRect->bottom - lpRect->top);

hDCMemory = CreateCompatibleDC(hDC);
hBitmapOld = SelectObject(hDCMemory, hBitmapDC);

SetRect(&RectTemp, 0, 0, lpRect->right - lpRect->left,
        lpRect->bottom - lpRect->top);

/************
* Draw Text
************/
SS_DrawText(hDCMemory, hWnd, lpSS, lpCol, lpRow, &RectTemp, CellCol, CellRow, TRUE,
            NULL, NULL, NULL, NULL, FALSE, SS_DRAWTEXTSEL_DEF, lpSS->Col.UL, lpSS->Row.UL,
            CellCol >= SS_GetColCnt(lpSS) - 1, CellRow >= SS_GetRowCnt(lpSS) - 1);

#ifdef SS_V40
SS_SpanFreePaintList(lpSS);
#endif // SS_V40

/*******************************************************
* Copy the memory device context bitmap to the display
*******************************************************/

BitBlt(hDC, lpRect->left, lpRect->top, lpRect->right - lpRect->left,
       lpRect->bottom - lpRect->top, hDCMemory, 0, 0, SRCCOPY);

/***********
* Clean up
***********/

SelectObject(hDCMemory, hBitmapOld);
DeleteDC(hDCMemory);
DeleteObject(hBitmapDC);
}


void SS_InvalidateRow(LPSPREADSHEET lpSS, SS_COORD Row)
{
RECT  Rect;
short y;
short cy;

if (lpSS->lpBook->fNoInvalidate || !SS_IsActiveSheet(lpSS))
   return;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without an HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (IsWindowVisible(lpSS->lpBook->hWnd) && SS_IsRowVisible(lpSS, Row,
                                                   SS_VISIBLE_PARTIAL))
   {
   SS_GetClientRect(lpSS->lpBook, &Rect);

   y = SS_GetCellPosY(lpSS, lpSS->Row.UL, Row);
   cy = SS_GetRowHeightInPixels(lpSS, Row);

   Rect.top = y + 1;
   Rect.bottom = y + cy;

   if (lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
       lpSS->wOpMode == SS_OPMODE_MULTISEL ||
       lpSS->wOpMode == SS_OPMODE_EXTSEL ||
       lpSS->wOpMode == SS_OPMODE_ROWMODE ||
       lpSS->fChangingOperationMode)
      {
      Rect.top = max(0, Rect.top - 1);
      Rect.bottom = Rect.bottom + 1;
      }

   InflateRect(&Rect, 0, 1);

   SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);

   if (lpSS->lpBook->Redraw)
      SS_UpdateWindow(lpSS->lpBook);
   else
      lpSS->lpBook->fInvalidated = TRUE;
   }

#ifdef SS_V40
SS_SpanInvalidateRange(lpSS, SS_ALLCOLS, Row, SS_ALLCOLS, Row, FALSE);
#endif // SS_V40
}


void SS_InvalidateRange(LPSPREADSHEET lpSS, SS_COORD Col1, SS_COORD Row1, SS_COORD Col2, SS_COORD Row2)
{
if (!lpSS->lpBook->hWnd)  // handle an OCX control without an HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (!SS_IsActiveSheet(lpSS))
	return;

if (Col1 != SS_ALLCOLS && Row1 != SS_ALLROWS)
   SS_InvalidateCellRange(lpSS, Col1, Row1, Col2, Row2);
else if (Col1 != SS_ALLCOLS && Row1 == SS_ALLROWS)
   SS_InvalidateColRange(lpSS, Col1, Col2);
else if (Col1 == SS_ALLCOLS && Row1 != SS_ALLROWS)
   SS_InvalidateRowRange(lpSS, Row1, Row2);
}


void SSx_InvalidateCellRange(LPSPREADSHEET lpSS, SS_COORD Col1, SS_COORD Row1, SS_COORD Col2, SS_COORD Row2)
{
SS_CELLTYPE CellType;
SS_COORD    i;
RECT        Rect;
short       x;
short       y;
short       cx;
short       cy;
BOOL        fIsButton = FALSE;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without an HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (!SS_IsActiveSheet(lpSS))
	return;

if (IsWindowVisible(lpSS->lpBook->hWnd) &&
   (Col1 < lpSS->Col.HeaderCnt + lpSS->Col.Frozen ||
   ((Col1 == -1 || Col1 <= lpSS->Col.LR) &&
   (Col2 == -1 || Col2 >= lpSS->Col.UL))) &&
   (Row1 < lpSS->Row.HeaderCnt + lpSS->Row.Frozen ||
   ((Row1 == -1 || Row1 <= lpSS->Row.LR) &&
   (Row2 == -1 || Row2 >= lpSS->Row.UL))))
   {
   SS_GetClientRect(lpSS->lpBook, &Rect);

   if (Row1 != -1 && (Row1 >= lpSS->Row.UL ||
       Row1 < lpSS->Row.HeaderCnt + lpSS->Row.Frozen))
      {
      y = SS_GetCellPosYInRect(lpSS, lpSS->Row.UL, Row1, &Rect);

      Rect.top = y + 1;

      if (Row1 == Row2)
			{
			cy = SS_GetRowHeightInPixels(lpSS, Row1);
			Rect.bottom = y + cy;
			}
      }

   if (Row2 != -1 && Row1 != Row2 && Row2 <= lpSS->Row.LR)
      {
      y = SS_GetCellPosYInRect(lpSS, lpSS->Row.UL, Row2 + 1, &Rect);
      Rect.bottom = y;
      }

   if (Col1 != -1 && (Col1 >= lpSS->Col.UL ||
       Col1 < lpSS->Col.HeaderCnt + lpSS->Col.Frozen))
      {
      x = SS_GetCellPosXInRect(lpSS, lpSS->Col.UL, Col1, &Rect);

      Rect.left = x + 1;

      if (Col1 == Col2)
			{
			cx = SS_GetColWidthInPixels(lpSS, Col1);
			Rect.right = x + cx;
			}
      }

   if (Col2 != -1 && Col1 != Col2 && Col2 <= lpSS->Col.LR)
      {
      x = SS_GetCellPosXInRect(lpSS, lpSS->Col.UL, Col2 + 1, &Rect);
      Rect.right = x;
      }

	// RFW - 7/31/06 - 19221

	// Check to see if any cells in the last row contain a button or combobox
	for (i = Col1; i <= Col2 && !fIsButton; i++)
		{
		SS_RetrieveCellType(lpSS, &CellType, NULL, i, Row2);

	   if (CellType.Type == SS_TYPE_BUTTON || CellType.Type == SS_TYPE_COMBOBOX)
			fIsButton = TRUE;
		}

	// Check to see if any cells in the last col contain a button or combobox
	for (i = Row1; i <= Row2 && !fIsButton; i++)
		{
		SS_RetrieveCellType(lpSS, &CellType, NULL, Col2, i);

	   if (CellType.Type == SS_TYPE_BUTTON || CellType.Type == SS_TYPE_COMBOBOX)
			fIsButton = TRUE;
		}

   if( SS_BACKCOLORSTYLE_OVERGRID == lpSS->lpBook->bBackColorStyle || fIsButton )
      {
      Rect.right++;
      Rect.bottom++;
      }
   else if( SS_BACKCOLORSTYLE_OVERHORZGRIDONLY == lpSS->lpBook->bBackColorStyle )
      {
      Rect.bottom++;
      }
   else if( SS_BACKCOLORSTYLE_OVERVERTGRIDONLY == lpSS->lpBook->bBackColorStyle )
      {
      Rect.right++;
      }

#ifdef SS_V80
	if (SS_SEL_HIGHLIGHTHEADERS(lpSS)
		 // RFW - 4/1/09 - 24942
		 && lpSS->lpBook->wAppearanceStyle == SS_APPEARANCESTYLE_ENHANCED
		 )
		{
		if (Col1 < lpSS->Col.HeaderCnt)
			Rect.top--;

		if (Row1 < lpSS->Row.HeaderCnt)
			Rect.left--;
		}
#endif // SS_V80

   SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);

   if (lpSS->lpBook->Redraw)
      SS_UpdateWindow(lpSS->lpBook);
   else
      lpSS->lpBook->fInvalidated = TRUE;
   }
}


void SS_InvalidateCellRange(LPSPREADSHEET lpSS, SS_COORD Col1, SS_COORD Row1, SS_COORD Col2, SS_COORD Row2)
{
BOOL fRedraw = lpSS->lpBook->Redraw;

if (!SS_IsActiveSheet(lpSS))
	return;

lpSS->lpBook->Redraw = FALSE;
SSx_InvalidateCellRange(lpSS, Col1, Row1, Col2, Row2);

#ifdef SS_V40
SS_SpanInvalidateRange(lpSS, Col1, Row1, Col2, Row2, TRUE);
// RFW - 6/10/04 - 14344
SS_MergeInvalidateRange(lpSS, Col1, Row1, Col2, Row2);
#endif // SS_V40

lpSS->lpBook->Redraw = fRedraw;
if (lpSS->lpBook->Redraw)
   SS_UpdateWindow(lpSS->lpBook);
}


void SS_InvalidateRowRange(LPSPREADSHEET lpSS, SS_COORD Row1, SS_COORD Row2)
{
RECT  Rect;
short y;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without an HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (!SS_IsActiveSheet(lpSS))
	return;

// BJO JAP6276 14Feb97 - Begin fix
if (Row1 > Row2 && Row2 != -1)
   return;
// BJO JAP6276 14Feb97 - End fix

if (IsWindowVisible(lpSS->lpBook->hWnd) &&
   (Row1 < lpSS->Row.HeaderCnt + lpSS->Row.Frozen ||
   ((Row1 == -1 || min(Row1, SS_GetRowCnt(lpSS) - 1) <= SS_GetBottomCell(lpSS, lpSS->Row.UL)) &&
   (Row2 == -1 || Row2 >= lpSS->Row.UL))))
   {
   SS_GetClientRect(lpSS->lpBook, &Rect);

   if (Row1 != -1 && Row1 >= lpSS->Row.UL)
      {
      y = SS_GetCellPosYInRect(lpSS, lpSS->Row.UL, Row1, &Rect);

// RFW - 8/16/01 - 9231       Rect.top = y;
      Rect.top = max(y - 1, 0);

      if (Row1 == Row2)
         Rect.bottom = y + SS_GetRowHeightInPixels(lpSS, Row1) + 1;
      }

   else if (Row1 != -1)
      Rect.top = SS_GetCellPosYInRect(lpSS, lpSS->Row.UL, min(Row1,
                      lpSS->Row.HeaderCnt + lpSS->Row.Frozen),
                      &Rect);

   if (Row2 != -1 && Row1 != Row2 && Row2 <= lpSS->Row.LR)
      {
      y = SS_GetCellPosYInRect(lpSS, lpSS->Row.UL, Row2 + 1, &Rect);
      Rect.bottom = y + 1;
      }

   if (lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
       lpSS->wOpMode == SS_OPMODE_MULTISEL ||
       lpSS->wOpMode == SS_OPMODE_EXTSEL ||
       lpSS->wOpMode == SS_OPMODE_ROWMODE ||
       lpSS->fChangingOperationMode)
      {
      Rect.top = max(0, Rect.top - 1);
      Rect.bottom = Rect.bottom + 1;
      }

   SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);

   if (lpSS->lpBook->Redraw)
      SS_UpdateWindow(lpSS->lpBook);
   else
      lpSS->lpBook->fInvalidated = TRUE;
   }

#ifdef SS_V40
SS_SpanInvalidateRange(lpSS, SS_ALLCOLS, Row1, SS_ALLCOLS, Row2, FALSE);
#endif // SS_V40
}


void SS_InvalidateCol(LPSPREADSHEET lpSS, SS_COORD Col)
{
RECT          Rect;
short         x;
short         cx;

if (lpSS->lpBook->fNoInvalidate || !SS_IsActiveSheet(lpSS))
   return;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without an HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (IsWindowVisible(lpSS->lpBook->hWnd) && SS_IsColVisible(lpSS, Col,
                         SS_VISIBLE_PARTIAL))
   {
   SS_GetClientRect(lpSS->lpBook, &Rect);

   x = SS_GetCellPosX(lpSS, lpSS->Col.UL, Col);
   cx = SS_GetColWidthInPixels(lpSS, Col);

   Rect.left = x + 1;
   Rect.right = x + cx;

   InflateRect(&Rect, 1, 0);

   SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);

   if (lpSS->lpBook->Redraw)
      SS_UpdateWindow(lpSS->lpBook);
   else
      lpSS->lpBook->fInvalidated = TRUE;
   }

#ifdef SS_V40
SS_SpanInvalidateRange(lpSS, Col, SS_ALLROWS, Col, SS_ALLROWS, FALSE);
#endif // SS_V40
}


void SS_InvalidateColRange(LPSPREADSHEET lpSS, SS_COORD Col1, SS_COORD Col2)
{
RECT          Rect;
short         x;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without an HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (!SS_IsActiveSheet(lpSS))
	return;

if (IsWindowVisible(lpSS->lpBook->hWnd) &&
   (Col1 < lpSS->Col.HeaderCnt + lpSS->Col.Frozen ||
   ((Col1 == -1 || min(Col1, SS_GetColCnt(lpSS) - 1) <= SS_GetRightCell(lpSS, lpSS->Col.UL)) &&
//   ((Col1 == -1 || Col1 <= lpSS->Col.LR) &&
   (Col2 == -1 || Col2 >= lpSS->Col.UL))))
   {
   SS_GetClientRect(lpSS->lpBook, &Rect);

   if (Col1 != -1 && Col1 >= lpSS->Col.UL)
      {
      x = SS_GetCellPosXInRect(lpSS, lpSS->Col.UL, Col1, &Rect);

// RFW - 8/16/01 - 9231       Rect.left = x;
      Rect.left = max(x - 1, 0);

		if (Col1 == Col2)
			Rect.right = x + SS_GetColWidthInPixels(lpSS, Col1) + 1;
      }

   else if (Col1 != -1)
		{
      x = SS_GetCellPosXInRect(lpSS, lpSS->Col.UL, min(Col1,
                       lpSS->Col.HeaderCnt + lpSS->Col.Frozen),
                       &Rect);
      Rect.left = max(x - 1, 0);
		}

   if (Col2 != -1 && Col1 != Col2 && Col2 <= lpSS->Col.LR)
      {
      x = SS_GetCellPosXInRect(lpSS, lpSS->Col.UL, Col2 + 1, &Rect);
      Rect.right = x + 1;
      }

   SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);

   if (lpSS->lpBook->Redraw)
      SS_UpdateWindow(lpSS->lpBook);
   else
      lpSS->lpBook->fInvalidated = TRUE;
   }

#ifdef SS_V40
SS_SpanInvalidateRange(lpSS, Col1, SS_ALLROWS, Col2, SS_ALLROWS, FALSE);
#endif // SS_V40
}


void SS_HScroll(LPSPREADSHEET lpSS, int ScrollInc)
{
RECT          RectTemp;
RECT          Rect;

if (!SS_IsActiveSheet(lpSS))
	return;

if (ScrollInc != 0)
   {
   SS_GetScrollArea(lpSS, &Rect, SS_SCROLLAREA_COLS);

   SS_Scroll(lpSS, -ScrollInc, 0, &Rect);
#ifdef SS_V30
   lpSS->xFocusRectOrig ^= ScrollInc % 2;
#endif

   if (ScrollInc < 0)
      if (SS_IsBlockSelected(lpSS))
         {
         SetRect(&RectTemp, Rect.left + -ScrollInc, Rect.top,
                 Rect.left + -ScrollInc + 1, Rect.bottom);
         SS_InvalidateRect(lpSS->lpBook, &RectTemp, TRUE);
         }

#ifdef SPREAD_JPN
// 97' 1/13 Modified by BOC Gao. BUG0629
// When EditModePermanent is True, 
// OldLeft parameter of TopLeftChange event is wrong
   if (lpSS->lpBook->fEditModePermanent )
#else
   if (FALSE)
#endif
// ------------------------------<<
      lpSS->lpBook->Redraw = FALSE;
/* RFW - 4/26/05 - 16156
   else
      SS_UpdateWindow(lpSS->lpBook);
*/

   SS_SetHScrollBar(lpSS);
   SS_LeftColChange(lpSS);

#ifdef SPREAD_JPN
// 97' 1/13 Modified by BOC Gao. BUG0629
// When EditModePermanent is True, 
// OldTop parameter of TopLeftChange event is wrong
   if (lpSS->lpBook->fEditModePermanent )
#else
   if (FALSE)
#endif
// --------------------------------<<
      {
      lpSS->lpBook->Redraw = TRUE;
      SS_SetHScrollBar(lpSS);
      SS_UpdateWindow(lpSS->lpBook);
      }
   }
}


void SS_HScrollCol(lpSS, Col, fRight, fUpdateHeader)

LPSPREADSHEET lpSS;
SS_COORD      Col;
BOOL          fRight;
BOOL          fUpdateHeader;
{
RECT          Rect;
short         dWidth;

if (!SS_IsActiveSheet(lpSS))
	return;

SS_GetScrollArea(lpSS, &Rect, SS_SCROLLAREA_COLS);

Rect.left = SS_GetCellPosX(lpSS, lpSS->Col.UL, Col);
dWidth = SS_GetColWidthInPixels(lpSS, Col);

if (!fRight)
   dWidth *= -1;

SS_Scroll(lpSS, dWidth, 0, &Rect);

if (fUpdateHeader)
   {
   Rect.bottom = SS_GetCellPosY(lpSS, lpSS->Row.HeaderCnt,
                   lpSS->Row.HeaderCnt);
   SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
   }

/* RFW - 4/26/05 - 16156
SS_UpdateWindow(lpSS->lpBook);
*/

SS_SetHScrollBar(lpSS);
SS_LeftColChange(lpSS);
}


void SS_VScroll(lpSS, ScrollInc)

LPSPREADSHEET lpSS;
int           ScrollInc;
{
RECT          RectTemp;
RECT          Rect;

if (!SS_IsActiveSheet(lpSS))
	return;

if (ScrollInc != 0)
   {
   SS_GetScrollArea(lpSS, &Rect, SS_SCROLLAREA_ROWS);
   SS_Scroll(lpSS, 0, -ScrollInc, &Rect);

   if (ScrollInc < 0)
      {
      if (SS_IsBlockSelected(lpSS))
         {
         SetRect(&RectTemp, Rect.left, Rect.top + -ScrollInc,
                 Rect.right, Rect.top + -ScrollInc + 1);
         SS_InvalidateRect(lpSS->lpBook, &RectTemp, TRUE);
         }
      }

#ifdef SPREAD_JPN
// 97' 1/13 Modified by BOC Gao. BUG0629
// When EditModePermanent is True, 
// OldTop parameter of TopLeftChange event is wrong
   if (lpSS->lpBook->fEditModePermanent )
#else
   if (FALSE)
#endif
      lpSS->lpBook->Redraw = FALSE;
/* RFW - 4/26/05 - 16156
   else
      SS_UpdateWindow(lpSS->lpBook);
*/

   SS_SetVScrollBar(lpSS);
   SS_TopRowChange(lpSS);

#ifdef SPREAD_JPN
// 97' 1/13 Modified by BOC Gao. BUG0629
// When EditModePermanent is True, 
// OldTop parameter of TopLeftChange event is wrong
   if (lpSS->lpBook->fEditModePermanent )
#else
   if (FALSE)
#endif
      {
      lpSS->lpBook->Redraw = TRUE;
      SS_SetVScrollBar(lpSS);
      SS_UpdateWindow(lpSS->lpBook);
      }
   }
}


void SS_VScrollRow(lpSS, Row, fDown, fUpdateHeader)

LPSPREADSHEET lpSS;
SS_COORD      Row;
BOOL          fDown;
BOOL          fUpdateHeader;
{
RECT          Rect;
short         dHeight;

if (!SS_IsActiveSheet(lpSS))
	return;

SS_GetScrollArea(lpSS, &Rect, SS_SCROLLAREA_ROWS);

Rect.top = SS_GetCellPosY(lpSS, lpSS->Row.UL, Row);
dHeight = SS_GetRowHeightInPixels(lpSS, Row);

if (!fDown)
   dHeight *= -1;

SS_Scroll(lpSS, 0, dHeight, &Rect);

if (fUpdateHeader)
   {
   Rect.right = SS_GetCellPosX(lpSS, lpSS->Col.HeaderCnt,
                   lpSS->Col.HeaderCnt);
   SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
   }

/* RFW - 4/26/05 - 16156
SS_UpdateWindow(lpSS->lpBook);
*/

SS_SetVScrollBar(lpSS);
SS_TopRowChange(lpSS);
}


void SS_ScrollLineUL(lpSS, fFlags)

LPSPREADSHEET lpSS;
WORD          fFlags;                   
{                     
short         i;
short         ScrollInc;
short         nSizeTemp;
BOOL          bLeft;
BOOL          bMoveActiveCell;
BOOL          bNoLeaveCell;
SS_COORD      BlockCellCurrentPos;
SS_COORD      ULTemp;
SS_COORD      ULOld;
SS_COORD      ULNew;
SS_COORD      PosAtOld;
SS_COORD      Pos1AtTemp;
SS_COORD      Pos2AtTemp;
LPSS_ROWCOL   lpRC1;        //either lpRow or lpCol structure
LPSS_ROWCOL   lpRC2;        //opposite of lpRC1
int           (*lpfnGetSize)(LPSPREADSHEET, SS_COORD);
            
if (!SS_CellEditModeOff(lpSS, 0))
   return;

bLeft = ((fFlags & SS_F_SCROLL_LEFT) == SS_F_SCROLL_LEFT);
bMoveActiveCell = ((fFlags & SS_F_MOVEACTIVECELL) == SS_F_MOVEACTIVECELL);
bNoLeaveCell = ((fFlags & SS_F_NOLEAVECELL) == SS_F_NOLEAVECELL);
            
if (bLeft) 
   {
   lpRC1 = &(lpSS->Col);
   lpRC2 = &(lpSS->Row);
   lpfnGetSize = SS_GetColWidthInPixels;
   }
else // scroll up
   {
   lpRC1 = &(lpSS->Row);
   lpRC2 = &(lpSS->Col); 
   lpfnGetSize = SS_GetRowHeightInPixels;
   }

if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
   {
   BlockCellCurrentPos = lpRC1->BlockCellCurrentPos;
   Pos2AtTemp = lpRC2->BlockCellCurrentPos;
   Pos1AtTemp = lpRC1->BlockCellCurrentPos;
   }
else
   {
   PosAtOld = lpRC1->CurAt;
   Pos2AtTemp = lpRC2->CurAt;
   Pos1AtTemp = lpRC1->CurAt;

   if (bMoveActiveCell)
      SS_ResetBlock(lpSS);
   }

if (bMoveActiveCell && !SS_IsCellVisible(lpSS,
    bLeft ? Pos1AtTemp : Pos2AtTemp,
    bLeft ? Pos2AtTemp : Pos1AtTemp, SS_VISIBLE_PARTIAL))
    {
    SS_HighlightCell(lpSS, FALSE);

   if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
      Pos1AtTemp = lpRC1->BlockCellCurrentPos;
   else
      Pos1AtTemp = lpRC1->CurAt;

   while (Pos1AtTemp > lpRC1->HeaderCnt)
      {
      Pos1AtTemp--;
      
      if (bLeft)
         i = (SS_GetColWidthInPixels(lpSS, Pos1AtTemp) > 0);
      else 
         i = (SS_GetRowHeightInPixels(lpSS, Pos1AtTemp) > 0 &&
             (!lpSS->lpBook->fEditModePermanent ||
             !SS_GetLock(lpSS, lpRC2->CurAt, Pos1AtTemp, FALSE)));

      if (i)
         {
         if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
            BlockCellCurrentPos = Pos1AtTemp;
         else
            lpRC1->CurAt = Pos1AtTemp;

         break;
         }
      }

   if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
      SS_ShowCell(lpSS,
                  bLeft ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
                  bLeft ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos,
                  SS_SHOW_NEAREST);

   else if (bNoLeaveCell || SS_LeaveCell(lpSS, bLeft ? PosAtOld : -1, -1,
            bLeft ? -1 : PosAtOld, -1, 0))
      SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
   }

else
   {
   if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
      Pos1AtTemp = lpRC1->BlockCellCurrentPos;
   else
      Pos1AtTemp = lpRC1->CurAt;

   ULTemp = lpRC1->UL;
   ULNew = lpRC1->UL;
   ULOld = ULTemp;
   ScrollInc = 0;

   if (bMoveActiveCell)
      {
      while (Pos1AtTemp > lpRC1->HeaderCnt)
         {
         Pos1AtTemp--;
         // BJO  27Feb98 SCS6298 - Before fix
         //if (Pos1AtTemp < lpRC1->UL && ULTemp >
         //    lpRC1->HeaderCnt + lpRC1->Frozen)
         // BJO  27Feb98 SCS6298 - Begin fix
         if (lpRC1->HeaderCnt + lpRC1->Frozen <= Pos1AtTemp &&
             Pos1AtTemp < lpRC1->UL &&
             ULTemp > lpRC1->HeaderCnt + lpRC1->Frozen)
         // BJO  27Feb98 SCS6298 - End fix
            {
            ULTemp--;
            if (nSizeTemp = lpfnGetSize(lpSS, ULTemp))
               {
               ScrollInc -= nSizeTemp;
               ULNew = ULTemp;
               }
            }

         if (lpfnGetSize(lpSS, Pos1AtTemp) > 0)
            {
            if (!lpSS->lpBook->fEditModePermanent ||
            !SS_GetLock(lpSS, bLeft ? Pos1AtTemp : lpRC2->CurAt,
                    bLeft ? lpRC2->CurAt : Pos1AtTemp, FALSE))
               {
               // RFW - 7/21/99 - Moved this line up here from
               // under the else condition.
               SS_HighlightCell(lpSS, FALSE);

               if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
                  BlockCellCurrentPos = Pos1AtTemp;
               else
                 lpRC1->CurAt = Pos1AtTemp;

               lpRC1->UL = ULNew;
               break;
               }
            else
               lpRC1->UL = ULNew;
            }
         }
      }

   else
      {
      while (ULTemp > lpRC1->HeaderCnt + lpRC1->Frozen)
         {
         ULTemp--;
         if (lpfnGetSize(lpSS, ULTemp) > 0)
            {
            SS_HighlightCell(lpSS, FALSE);
            lpRC1->UL = ULTemp;
            ScrollInc -= lpfnGetSize(lpSS, lpRC1->UL);
            break;
            }
         }
      }

   if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) || !bMoveActiveCell || bNoLeaveCell ||
       SS_LeaveCell(lpSS, 
       bLeft ? PosAtOld : -1,
       bLeft ? ULOld : -1,
       bLeft ? -1 : PosAtOld,
       bLeft ? -1 : ULOld, 0))
      {
      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bLeft && (lpSS->lpBook->wMode == SS_MODE_BLOCK))
         lpSS->Row.CurAt = BlockCellCurrentPos;

      if (ScrollInc)                    
         {
         if (bLeft)
            SS_HScroll(lpSS, ScrollInc);
         else
            SS_VScroll(lpSS, ScrollInc);
         }
      else
         SS_HighlightCell(lpSS, TRUE);

      if (!bLeft && !(lpSS->lpBook->wMode == SS_MODE_BLOCK) && bMoveActiveCell &&
          PosAtOld == lpSS->Row.CurAt)
         {
         if (!lpSS->lpBook->fSuspendExit)
            SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITPREV, TRUE);
         }
      else if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) && bMoveActiveCell)
         SSx_SelectBlock(lpSS,                                           
            bLeft ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
            bLeft ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos);
      }
   }
  
if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bLeft && (lpSS->lpBook->wMode == SS_MODE_BLOCK))
   lpSS->Row.CurAt = BlockCellCurrentPos;
   
if (!bLeft && lpSS->Row.UL != ULOld)
   lpSS->lpBook->wScrollDirection = SS_VSCROLL_UP;
}


void SS_ScrollLineLR(lpSS, fFlags)

LPSPREADSHEET lpSS;
WORD          fFlags;
{
short         ScrollInc;
BOOL          bRight;
BOOL          bMoveActiveCell;
BOOL          bNoLeaveCell;
BOOL          Found;
BOOL          fCancel = FALSE;
SS_COORD      BlockCellCurrentPos;
SS_COORD      PosAtOld;         
SS_COORD      ULTemp;           
SS_COORD      ULOld;                    
SS_COORD      PosLast;          
SS_COORD      Pos1AtTemp;       
SS_COORD      Pos2AtTemp;       
SS_COORD      MaxCnt;
LPSS_ROWCOL   lpRC1;        //either lpRow or lpCol structure
LPSS_ROWCOL   lpRC2;        //inverse of lpRC1
int           (*lpfnGetSize)(LPSPREADSHEET, SS_COORD);
BOOL          (*lpfnIsVisible)(LPSPREADSHEET, SS_COORD, short);

if (!SS_CellEditModeOff(lpSS, 0))
   return;

bRight = ((fFlags & SS_F_SCROLL_RIGHT) == SS_F_SCROLL_RIGHT);
bMoveActiveCell = ((fFlags & SS_F_MOVEACTIVECELL) == SS_F_MOVEACTIVECELL);
bNoLeaveCell = ((fFlags & SS_F_NOLEAVECELL) == SS_F_NOLEAVECELL);

if (bRight) 
   {
   MaxCnt = SS_GetColCnt(lpSS);
   lpRC1 = &(lpSS->Col);
   lpRC2 = &(lpSS->Row);
   lpfnGetSize = SS_GetColWidthInPixels;
   lpfnIsVisible = SS_IsColVisible;
   }
else // scroll down
   {
   MaxCnt = SS_GetRowCnt(lpSS);
   lpRC1 = &(lpSS->Row);
   lpRC2 = &(lpSS->Col); 
   lpfnGetSize = SS_GetRowHeightInPixels;
   lpfnIsVisible = SS_IsRowVisible;
   }

#ifdef  BUGS
// Bug-016
    if ( (lpRC1->CurAt + 1) == (MaxCnt-1) &&  (!lpfnGetSize(lpSS, lpRC1->CurAt + 1)) ) 
        bMoveActiveCell = FALSE;        
#endif

if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
   {
   BlockCellCurrentPos = lpRC1->BlockCellCurrentPos;

	if (SS_IsF8ExtSelMode(lpSS))
		{
		Pos1AtTemp = lpRC1->CurAt;
		Pos2AtTemp = lpRC2->CurAt;
		}
	else
		{
		Pos1AtTemp = lpRC1->BlockCellCurrentPos;
		Pos2AtTemp = lpRC2->BlockCellCurrentPos;
		}
   }
else
   {
   Pos1AtTemp = lpRC1->CurAt;
   Pos2AtTemp = lpRC2->CurAt;

/* RFW - 10/1/07 - 20858
	// RFW - 10/17/02
//   if (bMoveActiveCell)
   if (bMoveActiveCell && !SS_USESELBAR(lpSS))
*/
   if (bMoveActiveCell)
      SS_ResetBlock(lpSS);
   }

PosAtOld = lpRC1->CurAt;
ULTemp = lpRC1->UL;
ULOld = lpRC1->UL;
ScrollInc = 0;

if (bMoveActiveCell &&
    !SS_IsCellVisible(lpSS,
    bRight ? Pos1AtTemp : Pos2AtTemp,
    bRight ? Pos2AtTemp : Pos1AtTemp, SS_VISIBLE_PARTIAL))
   {
   SS_HighlightCell(lpSS, FALSE);

   while (Pos1AtTemp < MaxCnt - 1)
      {
      Pos1AtTemp++;
      if ((lpfnGetSize(lpSS, Pos1AtTemp) > 0) &&
          (!lpSS->lpBook->fEditModePermanent ||
           !SS_GetLock(lpSS, bRight ? Pos1AtTemp : Pos2AtTemp,
               bRight ? Pos2AtTemp : Pos1AtTemp, FALSE)))
         {
         if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
            BlockCellCurrentPos = Pos1AtTemp;
         else
            lpRC1->CurAt = Pos1AtTemp;

         break;
         }
      }

   if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
      SS_ShowCell(lpSS, 
          bRight ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
          bRight ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos,
          SS_SHOW_NEAREST);

   else if (bNoLeaveCell || SS_LeaveCell(lpSS, 
            bRight ? PosAtOld : -1, -1,
            bRight ? -1 : PosAtOld, -1, 0))
      SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
   }
else
	{
	if (bMoveActiveCell)
      {
      Found = FALSE;
      while (Pos1AtTemp < MaxCnt - 1)
         {
         if (Pos1AtTemp == lpRC1->HeaderCnt + lpRC1->Frozen - 1)
            Pos1AtTemp = max(lpRC1->UL, lpRC1->HeaderCnt + lpRC1->Frozen);
         else
            Pos1AtTemp++;

			// RFW - 8/4/05 - 16323
         if (lpfnGetSize(lpSS, Pos1AtTemp) > 0)
				{
				/* RFW - 12/27/04 - 15360
				while (!lpfnIsVisible(lpSS, Pos1AtTemp, SS_VISIBLE_ALL) &&
						 Pos1AtTemp > lpRC1->UL)
				*/
				// RFW - 3/9/05 - 15880
				lpSS->Row.LRAllVis = -1;
				while (Pos1AtTemp > lpRC1->UL && !lpfnIsVisible(lpSS, Pos1AtTemp, SS_VISIBLE_ALL))
					{
					SS_HighlightCell(lpSS, FALSE);
					ScrollInc += lpfnGetSize(lpSS, lpRC1->UL);
					lpRC1->UL++;
					}

				if (lpfnGetSize(lpSS, Pos1AtTemp) > 0 && (!lpSS->lpBook->fEditModePermanent ||
					 !SS_GetLock(lpSS, bRight ? Pos1AtTemp : Pos2AtTemp,
						  bRight ? Pos2AtTemp : Pos1AtTemp, FALSE)))
					{
					if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
						BlockCellCurrentPos = Pos1AtTemp;
					else
						{
						SS_HighlightCell(lpSS, FALSE);
						lpRC1->CurAt = Pos1AtTemp;
						}

					Found = TRUE;
					break;
					}
				}
         }

		// RFW - 10/28/05 - 17072
		if (-1 == lpSS->Row.LRAllVis)
			lpSS->Row.LRAllVis = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);

      if (!Found)
         {
         fCancel = TRUE;
         lpRC1->UL = ULTemp;
         if (!bRight)
            ScrollInc = 0;
         }
      }
	else
		{
		if (lpSS->lpBook->fScrollBarMaxAlign)
			{ 
			if (bRight)
				PosLast = SS_GetLastPageLeftCol(lpSS);
			else   
				PosLast = SS_GetLastPageTopRow(lpSS);
			}
		else
			PosLast = MaxCnt - 1;

		while (ULTemp < PosLast)
			{
			ULTemp++;
			if (lpfnGetSize(lpSS, ULTemp) > 0)
				{
				SS_HighlightCell(lpSS, FALSE);
				ScrollInc += lpfnGetSize(lpSS, lpRC1->UL);
				lpRC1->UL = ULTemp;

				SS_ResetCellSizeList(lpRC1);

				break;
				}
			}
		}

	if (!fCancel &&
		 ((lpSS->lpBook->wMode == SS_MODE_BLOCK) || !bMoveActiveCell || bNoLeaveCell ||
		  SS_LeaveCell(lpSS,
							bRight ? PosAtOld : -1,
							bRight ? ULOld : -1,
							bRight ? -1 : PosAtOld,
							bRight ? -1 : ULOld, 0)))
		{
		if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bRight && lpSS->lpBook->wMode == SS_MODE_BLOCK)
			lpSS->Row.CurAt = BlockCellCurrentPos;

		if (ScrollInc)
			{
			if (bRight)
				SS_HScroll(lpSS, ScrollInc);
			else
				SS_VScroll(lpSS, ScrollInc);

			// RFW - 12/1/08
#if SS_V80
         SS_InvalidateActiveHeaders(lpSS);
#endif // SS_V80
	      SS_UpdateWindow(lpSS->lpBook);
			SS_HighlightCell(lpSS, TRUE);
			}      
		else
			SS_HighlightCell(lpSS, TRUE);
      
		if (lpSS->lpBook->wMode == SS_MODE_BLOCK && bMoveActiveCell)
			SSx_SelectBlock(lpSS, 
				bRight ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
				bRight ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos);
		else if (!bRight && !(lpSS->lpBook->wMode == SS_MODE_BLOCK) && bMoveActiveCell &&
					PosAtOld == lpRC1->CurAt)
			{
			if (!lpSS->lpBook->fSuspendExit)
				SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITNEXT, TRUE);
			}
		}

	else if (fCancel && !bRight && !(lpSS->lpBook->wMode == SS_MODE_BLOCK) && bMoveActiveCell &&
				PosAtOld == lpRC1->CurAt)
		{
		// 96' 10/16 Modified by BOC Gao. No.600, 687 ------------------->>
		// When last row is locked, the cursor disappeared.
		#ifdef SPREAD_JPN
		SS_HighlightCell(lpSS, TRUE); // Need highlight again?
		SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITNEXT, TRUE);
		#else
		if (!lpSS->lpBook->fSuspendExit)
			SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITNEXT, TRUE);
		SS_HighlightCell(lpSS, TRUE);
		#endif
		}
	else
		SS_HighlightCell(lpSS, TRUE);
	}

if (!bRight && lpRC1->UL != ULOld)
   lpSS->lpBook->wScrollDirection = SS_VSCROLL_DOWN;
}


#if 0 // 17296

void SS_ScrollLineLR(lpSS, fFlags)

LPSPREADSHEET lpSS;
WORD          fFlags;
{
short         ScrollInc;
BOOL          bRight;
BOOL          bMoveActiveCell;
BOOL          bNoLeaveCell;
BOOL          Found;
BOOL          fCancel = FALSE;
SS_COORD      BlockCellCurrentPos;
SS_COORD      PosAtOld;         
SS_COORD      ULTemp;           
SS_COORD      ULOld;                    
SS_COORD      PosLast;          
SS_COORD      Pos1AtTemp;       
SS_COORD      Pos2AtTemp;       
SS_COORD      MaxCnt;
LPSS_ROWCOL   lpRC1;        //either lpRow or lpCol structure
LPSS_ROWCOL   lpRC2;        //inverse of lpRC1
int           (*lpfnGetSize)(LPSPREADSHEET, SS_COORD);
BOOL          (*lpfnIsVisible)(LPSPREADSHEET, SS_COORD, short);

if (!SS_CellEditModeOff(lpSS, 0))
   return;

bRight = ((fFlags & SS_F_SCROLL_RIGHT) == SS_F_SCROLL_RIGHT);
bMoveActiveCell = ((fFlags & SS_F_MOVEACTIVECELL) == SS_F_MOVEACTIVECELL);
bNoLeaveCell = ((fFlags & SS_F_NOLEAVECELL) == SS_F_NOLEAVECELL);

if (bRight) 
   {
   MaxCnt = SS_GetColCnt(lpSS);
   lpRC1 = &(lpSS->Col);
   lpRC2 = &(lpSS->Row);
   lpfnGetSize = SS_GetColWidthInPixels;
   lpfnIsVisible = SS_IsColVisible;
   }
else // scroll down
   {
   MaxCnt = SS_GetRowCnt(lpSS);
   lpRC1 = &(lpSS->Row);
   lpRC2 = &(lpSS->Col); 
   lpfnGetSize = SS_GetRowHeightInPixels;
   lpfnIsVisible = SS_IsRowVisible;
   }

#ifdef  BUGS
// Bug-016
    if ( (lpRC1->CurAt + 1) == (MaxCnt-1) &&  (!lpfnGetSize(lpSS, lpRC1->CurAt + 1)) ) 
        bMoveActiveCell = FALSE;        
#endif

if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
   {
   BlockCellCurrentPos = lpRC1->BlockCellCurrentPos;

	if (SS_IsF8ExtSelMode(lpSS))
		{
		Pos1AtTemp = lpRC1->CurAt;
		Pos2AtTemp = lpRC2->CurAt;
		}
	else
		{
		Pos1AtTemp = lpRC1->BlockCellCurrentPos;
		Pos2AtTemp = lpRC2->BlockCellCurrentPos;
		}
   }
else
   {
   Pos1AtTemp = lpRC1->CurAt;
   Pos2AtTemp = lpRC2->CurAt;

	// RFW - 10/17/02
//   if (bMoveActiveCell)
   if (bMoveActiveCell && !SS_USESELBAR(lpSS))
      SS_ResetBlock(lpSS);
   }


PosAtOld = lpRC1->CurAt;
ULTemp = lpRC1->UL;
ULOld = lpRC1->UL;
ScrollInc = 0;

if (bMoveActiveCell)
   {
   if (!SS_IsCellVisible(lpSS,
          bRight ? Pos1AtTemp : Pos2AtTemp,
          bRight ? Pos2AtTemp : Pos1AtTemp, SS_VISIBLE_PARTIAL))
      {
      SS_HighlightCell(lpSS, FALSE);

      while (Pos1AtTemp < MaxCnt - 1)
         {
         Pos1AtTemp++;
         if ((lpfnGetSize(lpSS, Pos1AtTemp) > 0) &&
             (!lpSS->lpBook->fEditModePermanent ||
              !SS_GetLock(lpSS, bRight ? Pos1AtTemp : Pos2AtTemp,
                  bRight ? Pos2AtTemp : Pos1AtTemp, FALSE)))
            {
            if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
               BlockCellCurrentPos = Pos1AtTemp;
            else
               lpRC1->CurAt = Pos1AtTemp;

            break;
            }
         }

      if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
         SS_ShowCell(lpSS, 
             bRight ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
             bRight ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos,
             SS_SHOW_NEAREST);

      else if (bNoLeaveCell || SS_LeaveCell(lpSS, 
               bRight ? PosAtOld : -1, -1,
               bRight ? -1 : PosAtOld, -1, 0))
         SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);

      if (bRight)
         return;
      }
   else
      {
      Found = FALSE;
      while (Pos1AtTemp < MaxCnt - 1)
         {
         if (Pos1AtTemp == lpRC1->HeaderCnt + lpRC1->Frozen - 1)
            Pos1AtTemp = max(lpRC1->UL, lpRC1->HeaderCnt + lpRC1->Frozen);
         else
            Pos1AtTemp++;

			// RFW - 8/4/05 - 16323
         if (lpfnGetSize(lpSS, Pos1AtTemp) > 0)
				{
				/* RFW - 12/27/04 - 15360
				while (!lpfnIsVisible(lpSS, Pos1AtTemp, SS_VISIBLE_ALL) &&
						 Pos1AtTemp > lpRC1->UL)
				*/
				// RFW - 3/9/05 - 15880
				lpSS->Row.LRAllVis = -1;
				while (Pos1AtTemp > lpRC1->UL && !lpfnIsVisible(lpSS, Pos1AtTemp, SS_VISIBLE_ALL))
					{
					SS_HighlightCell(lpSS, FALSE);
					ScrollInc += lpfnGetSize(lpSS, lpRC1->UL);
					lpRC1->UL++;
					}

				if (lpfnGetSize(lpSS, Pos1AtTemp) > 0 && (!lpSS->lpBook->fEditModePermanent ||
					 !SS_GetLock(lpSS, bRight ? Pos1AtTemp : Pos2AtTemp,
						  bRight ? Pos2AtTemp : Pos1AtTemp, FALSE)))
					{
					if ((lpSS->lpBook->wMode == SS_MODE_BLOCK))
						BlockCellCurrentPos = Pos1AtTemp;
					else
						{
						SS_HighlightCell(lpSS, FALSE);
						lpRC1->CurAt = Pos1AtTemp;
						}

					Found = TRUE;
					break;
					}
				}
         }

      if (!Found)
         {
         fCancel = TRUE;
         lpRC1->UL = ULTemp;
         if (!bRight)
            ScrollInc = 0;
         }
      }
   }
else
   {
   if (lpSS->lpBook->fScrollBarMaxAlign)
      { 
      if (bRight)
         PosLast = SS_GetLastPageLeftCol(lpSS);
      else   
         PosLast = SS_GetLastPageTopRow(lpSS);
      }
   else
      PosLast = MaxCnt - 1;

   while (ULTemp < PosLast)
      {
      ULTemp++;
      if (lpfnGetSize(lpSS, ULTemp) > 0)
         {
         SS_HighlightCell(lpSS, FALSE);
         ScrollInc += lpfnGetSize(lpSS, lpRC1->UL);
         lpRC1->UL = ULTemp;

         SS_ResetCellSizeList(lpRC1);

         break;
         }
      }
   }

if (!fCancel &&
    ((lpSS->lpBook->wMode == SS_MODE_BLOCK) || !bMoveActiveCell || bNoLeaveCell ||
     SS_LeaveCell(lpSS,
                  bRight ? PosAtOld : -1,
                  bRight ? ULOld : -1,
                  bRight ? -1 : PosAtOld,
                  bRight ? -1 : ULOld, 0)))
   {
   if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bRight && lpSS->lpBook->wMode == SS_MODE_BLOCK)
      lpSS->Row.CurAt = BlockCellCurrentPos;

   if (ScrollInc)
      {
      if (bRight)
         SS_HScroll(lpSS, ScrollInc);
      else
         SS_VScroll(lpSS, ScrollInc);
      }      
   else
      SS_HighlightCell(lpSS, TRUE);
      
   if (lpSS->lpBook->wMode == SS_MODE_BLOCK && bMoveActiveCell)
      SSx_SelectBlock(lpSS, 
         bRight ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
         bRight ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos);
   else if (!bRight && !(lpSS->lpBook->wMode == SS_MODE_BLOCK) && bMoveActiveCell &&
            PosAtOld == lpRC1->CurAt)
      {
      if (!lpSS->lpBook->fSuspendExit)
         SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITNEXT, TRUE);
      }
   }

else if (fCancel && !bRight && !(lpSS->lpBook->wMode == SS_MODE_BLOCK) && bMoveActiveCell &&
         PosAtOld == lpRC1->CurAt)
// 96' 10/16 Modified by BOC Gao. No.600, 687 ------------------->>
// When last row is locked, the cursor disappeared.
#ifdef SPREAD_JPN
{
   SS_HighlightCell(lpSS, TRUE); // Need highlight again?
   SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITNEXT, TRUE);
}
#else
   {
   if (!lpSS->lpBook->fSuspendExit)
      SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITNEXT, TRUE);
   SS_HighlightCell(lpSS, TRUE);
   }
#endif
// ------------------------------------------------------------<<
else
   SS_HighlightCell(lpSS, TRUE);

if (!bRight && lpRC1->UL != ULOld)
   lpSS->lpBook->wScrollDirection = SS_VSCROLL_DOWN;
}

#endif

void SS_ScrollPageUL(LPSPREADSHEET lpSS, WORD fFlags)
{
BOOL          bMoveActiveCell;
BOOL          bLeft;
RECT          Rect;
SS_COORD      PosAt;
SS_COORD      PosAtOld;
SS_COORD      PosAtPrev;
SS_COORD      ULOld;
SS_COORD      BlockCellCurrentPos;
int           iCellPos;
int           iCellSize;
int           iRectSize;
LPSS_ROWCOL   lpRC1;        //either lpRow or lpCol structure
LPSS_ROWCOL   lpRC2;        //opposite of lpRC1
int           (*lpfnGetSize)(LPSPREADSHEET, SS_COORD);
BOOL          (*lpfnIsSelected)(LPSPREADSHEET);

if (!SS_CellEditModeOff(lpSS, 0))
   return;

bMoveActiveCell = ((fFlags & SS_F_MOVEACTIVECELL) == SS_F_MOVEACTIVECELL);
bLeft = ((fFlags & SS_F_SCROLL_LEFT) == SS_F_SCROLL_LEFT);
            
if (bLeft) 
   {
   lpRC1 = &(lpSS->Col);
   lpRC2 = &(lpSS->Row);
   lpfnGetSize = SS_GetColWidthInPixels;
   lpfnIsSelected = SS_IsColsSelected;
   }
else // scroll up
   {
   lpRC1 = &(lpSS->Row);
   lpRC2 = &(lpSS->Col);
   lpfnGetSize = SS_GetRowHeightInPixels;
   lpfnIsSelected = SS_IsRowsSelected;
   }                     

PosAtOld = lpRC1->CurAt;
ULOld = lpRC1->UL;

BlockCellCurrentPos = lpRC1->BlockCellCurrentPos;

if (lpRC1->UL > lpRC1->HeaderCnt + lpRC1->Frozen)
   {
#ifdef SS_UTP
   short dScrollArrowSizeOld;
   BOOL  fScrollVisible;

   // review: should dScrollArrowWidth & dScrollArrowHeight be in SS_ROWCOL?
   if (bLeft)
      {
      dScrollArrowSizeOld = lpSS->dScrollArrowWidth;
      lpSS->dScrollArrowWidth = SS_SCROLLARROW_WIDTH;
      fScrollVisible = lpSS->fVScrollVisible;
      lpSS->fVScrollVisible = TRUE;
      }
   else
      {
      dScrollArrowSizeOld = lpSS->dScrollArrowHeight;
      lpSS->dScrollArrowHeight = SS_SCROLLARROW_HEIGHT;
      fScrollVisible = lpSS->fHScrollVisible;
      lpSS->fHScrollVisible = TRUE;
      }
   
#endif
   SS_GetClientRect(lpSS->lpBook, &Rect);
#ifdef SS_UTP
   if (bLeft)
      {
      lpSS->dScrollArrowWidth = dScrollArrowSizeOld;
      lpSS->fVScrollVisible = fScrollVisible;
      }
   else
      {
      lpSS->dScrollArrowHeight = dScrollArrowSizeOld;
      lpSS->fHScrollVisible = fScrollVisible;
      }
#endif

   if (bLeft)                    
      { 
      iCellPos = SS_GetCellPosX(lpSS, lpRC1->UL, lpRC1->UL);
      iRectSize = Rect.right - Rect.left;
      }
   else
      { 
      iCellPos = SS_GetCellPosY(lpSS, lpRC1->UL, lpRC1->UL);
      iRectSize = Rect.bottom - Rect.top;
      }

   PosAt = lpRC1->UL;
	PosAtPrev = PosAt;
	iCellSize = 0;

   for (; PosAt > lpRC1->HeaderCnt && iCellPos <= iRectSize; )
      {
		if (iCellSize)
			PosAtPrev = PosAt;

      if (--PosAt < lpRC1->HeaderCnt)
         break;

      iCellSize = lpfnGetSize(lpSS, PosAt);
      iCellPos += iCellSize;
      }

   if (iCellPos > iRectSize)
      {
      SS_HighlightCell(lpSS, FALSE);

		if (PosAt + 1 == lpRC1->UL)
			lpRC1->UL = max(lpRC1->HeaderCnt + lpRC1->Frozen, PosAt);
      else
#ifdef SS_UTP
			lpRC1->UL = max(lpRC1->HeaderCnt + lpRC1->Frozen,
                     min(lpRC1->UL - 1, PosAt + 2));
#else
			lpRC1->UL = max(lpRC1->HeaderCnt + lpRC1->Frozen, PosAtPrev);
#endif

      if (bMoveActiveCell)
			{
			if (lpRC1->UL == lpRC1->HeaderCnt + lpRC1->Frozen)
				PosAt = lpRC1->HeaderCnt;
			else
				PosAt = lpRC1->UL;

			if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
				{
				BlockCellCurrentPos = PosAt;

				if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bLeft)
				    lpSS->Row.CurAt = BlockCellCurrentPos;
				}
			else
				lpRC1->CurAt = PosAt;

#ifdef SS_UTP
			if (lpSS->fUseScrollArrows && lpfnIsSelected(lpSS) &&
				 ((bLeft ? lpSS->BlockCellUL.Col : lpSS->BlockCellUL.Row) >
					lpRC1->UL) && !(lpSS->lpBook->wMode == SS_MODE_BLOCK))
				{
				SS_ResetBlock(lpSS);
				if ((lpRC2->CurAt < lpRC2->UL &&
					  lpRC2->CurAt >= lpRC2->HeaderCnt + lpRC2->Frozen) ||
					  lpRC2->CurAt > lpRC2->LRAllVis)
					{
					lpRC2->CurAt = lpRC2->UL;
					SS_PostSetEditMode(lpSS, TRUE, 0L);
					}
				}
#endif
			}

      //*************
      //* Scroll All
      //*************

		if (bLeft)
     {
     if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) || !bMoveActiveCell ||
         SS_LeaveCell(lpSS, PosAtOld, ULOld, -1, -1, 0))
        SS_HScroll(lpSS, -iRectSize);
     }
     else
     {
     if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) || !bMoveActiveCell ||
         SS_LeaveCell(lpSS, -1, -1, PosAtOld, ULOld, 0))
        SS_VScroll(lpSS, -iRectSize);
     }

      if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) && bMoveActiveCell)
     SSx_SelectBlock(lpSS, bLeft ? BlockCellCurrentPos :
             lpRC2->BlockCellCurrentPos, bLeft ?
             lpRC2->BlockCellCurrentPos : BlockCellCurrentPos);
      }
   else                                            
      SS_ScrollHome(lpSS, (short)((bLeft ? SS_F_SCROLL_ROW : SS_F_SCROLL_COL) |
            (bMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));
   }

else
   SS_ScrollHome(lpSS, (short)((bLeft ? SS_F_SCROLL_ROW : SS_F_SCROLL_COL) |
                 (bMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));

if (!bLeft && lpRC1->UL != ULOld)
   lpSS->lpBook->wScrollDirection = SS_VSCROLL_PAGEUP;
}


void SS_ScrollPageLR(LPSPREADSHEET lpSS, WORD fFlags)
{   
BOOL          bMoveActiveCell;
BOOL          bRight;
RECT          Rect;
SS_COORD      PosAt;
SS_COORD      ULNew;
SS_COORD      PosAtOld;
SS_COORD      ULOld;
SS_COORD      LRPos;
SS_COORD      BlockCellCurrentPos;
SS_COORD      MaxCnt;
LPSS_ROWCOL   lpRC1;        //either lpRow or lpCol structure
LPSS_ROWCOL   lpRC2;        //opposite of lpRC1
BOOL          (*lpfnIsSelected)(LPSPREADSHEET);

if (!SS_CellEditModeOff(lpSS, 0))
   return;

bMoveActiveCell = ((fFlags & SS_F_MOVEACTIVECELL) == SS_F_MOVEACTIVECELL);
bRight = ((fFlags & SS_F_SCROLL_RIGHT) == SS_F_SCROLL_RIGHT);
            
if (bRight) 
   {
   lpRC1 = &(lpSS->Col);
   lpRC2 = &(lpSS->Row);
   lpfnIsSelected = SS_IsColsSelected;
   MaxCnt = SS_GetColCnt(lpSS);
   }
else // scroll down
   {
   lpRC1 = &(lpSS->Row);
   lpRC2 = &(lpSS->Col);
   lpfnIsSelected = SS_IsRowsSelected;
   MaxCnt = SS_GetRowCnt(lpSS);
   }                     

PosAtOld = lpRC1->CurAt;
ULOld = lpRC1->UL;

BlockCellCurrentPos = lpRC1->BlockCellCurrentPos;

// RFW - 4/22/05 - 16131
if (lpRC1->LRAllVis == -1)
	lpRC1->LRAllVis = SS_GetBottomVisCell(lpSS, lpRC1->UL);

LRPos = lpRC1->LRAllVis;

#ifdef SS_UTP
ULNew = LRPos;
if (ULNew == ULOld)
   ULNew++;
#else
ULNew = LRPos + 1;
#endif

if (LRPos < lpRC1->UL)
   LRPos = lpRC1->UL;

if (LRPos < MaxCnt - 1)
   {
#ifdef SS_UTP
   short dScrollArrowSizeOld;
   BOOL  fScrollVisible;

   // review: should dScrollArrowWidth & dScrollArrowHeight be in SS_ROWCOL?
   if (bRight)
      {
      dScrollArrowSizeOld = lpSS->dScrollArrowWidth;
      lpSS->dScrollArrowWidth = SS_SCROLLARROW_WIDTH;
      fScrollVisible = lpSS->fHScrollVisible;
      lpSS->fHScrollVisible = TRUE;
      }
   else
      {
      dScrollArrowSizeOld = lpSS->dScrollArrowHeight;
      lpSS->dScrollArrowHeight = SS_SCROLLARROW_HEIGHT;
      fScrollVisible = lpSS->fVScrollVisible;
      lpSS->fVScrollVisible = TRUE;
      }
#endif

   if (bRight)
      PosAt = SS_GetRightVisCell(lpSS, ULNew);
   else
      PosAt = SS_GetBottomVisCell(lpSS, ULNew);

#ifdef SS_UTP             
   if (bRight)
      {
      lpSS->dScrollArrowWidth = dScrollArrowSizeOld;
      lpSS->fHScrollVisible = fScrollVisible;
      }
   else
      {
      lpSS->dScrollArrowHeight = dScrollArrowSizeOld;
      lpSS->fVScrollVisible = fScrollVisible;
      }
#endif

// Added by BOC FMH 1996.07.04. --------------------->>
//       for PageDown at the last two pages with VirtualMode .
#ifdef SPREAD_JPN
   if (!lpSS->fVirtualMode && (PosAt == MaxCnt - 1))
#else
//----------------------<<
   if (PosAt == MaxCnt - 1)
#endif  // Added by BOC FMH
      SS_ScrollEnd(lpSS, (short)((bRight ? SS_F_SCROLL_ROW : SS_F_SCROLL_COL)
      | SS_F_SCROLLMAX | (bMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));

   else
      {
      SS_HighlightCell(lpSS, FALSE);
      lpRC1->UL = ULNew;

      if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
     BlockCellCurrentPos = lpRC1->UL;
      else if (bMoveActiveCell)
     {
     lpRC1->CurAt = lpRC1->UL;

#ifdef SS_UTP
     if (lpSS->fUseScrollArrows && lpfnIsSelected(lpSS) &&
         (bRight ? lpSS->BlockCellLR.Col != -1 :
          lpSS->BlockCellLR.Row != -1) &&
         ((bRight ? lpSS->BlockCellLR.Col : lpSS->BlockCellLR.Row) <
         lpRC1->UL) && !(lpSS->lpBook->wMode == SS_MODE_BLOCK))
        {
        SS_ResetBlock(lpSS);
        if ((lpRC2->CurAt < lpRC2->UL &&
         lpRC2->CurAt >= lpRC2->HeaderCnt + lpRC2->Frozen) ||
         lpRC2->CurAt > lpRC2->LRAllVis)
           {
           lpRC2->CurAt = lpRC2->UL;
           SS_PostSetEditMode(lpSS, TRUE, 0L);
           }
        }
#endif
     }

      //*************
      //* Scroll All
      //*************

      SS_GetClientRect(lpSS->lpBook, &Rect);

      if (bRight)
     {
     if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) || !bMoveActiveCell ||
         SS_LeaveCell(lpSS, PosAtOld, ULOld, -1, -1, 0))
        SS_HScroll(lpSS, Rect.right - Rect.left);
     }
      else
     {
     if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) || !bMoveActiveCell ||
         SS_LeaveCell(lpSS, -1, -1, PosAtOld, ULOld, 0))
        SS_VScroll(lpSS, Rect.bottom - Rect.top);
     }

      if ((lpSS->lpBook->wMode == SS_MODE_BLOCK) && bMoveActiveCell)
     SSx_SelectBlock(lpSS, 
         bRight ? BlockCellCurrentPos : lpSS->Col.BlockCellCurrentPos,
         bRight ? lpSS->Row.BlockCellCurrentPos : BlockCellCurrentPos);

      if (lpSS->wOpMode == SS_OPMODE_EXTSEL && !bRight && (lpSS->lpBook->wMode == SS_MODE_BLOCK))
     lpSS->Row.CurAt = BlockCellCurrentPos;
      }
   }

else
   SS_ScrollEnd(lpSS, (short)((bRight ? SS_F_SCROLL_ROW : SS_F_SCROLL_COL) | 
        SS_F_SCROLLMAX | (bMoveActiveCell ? SS_F_MOVEACTIVECELL : 0)));
}


void SS_HPos(LPSPREADSHEET lpSS, SS_COORD Pos)
{
short         ScrollInc;
RECT          Rect;

if (!SS_CellEditModeOff(lpSS, 0))
   return;

SS_HighlightCell(lpSS, FALSE);
SS_GetClientRect(lpSS->lpBook, &Rect);

/* RFW - 4/29/09
ScrollInc = SS_GetCellPosX(lpSS, lpSS->Col.HeaderCnt + lpSS->Col.Frozen, Pos) -
            SS_GetCellPosX(lpSS, lpSS->Col.HeaderCnt + lpSS->Col.Frozen, lpSS->Col.UL);
*/

if (Pos > lpSS->Col.UL)
   ScrollInc = SS_GetCellPosXExt(lpSS, lpSS->Col.UL, Pos, &Rect) -
               SS_GetCellPosXExt(lpSS, lpSS->Col.HeaderCnt + lpSS->Col.Frozen,
                                 lpSS->Col.HeaderCnt + lpSS->Col.Frozen, &Rect);
else
   ScrollInc = (SS_GetCellPosXExt(lpSS, Pos, lpSS->Col.UL, &Rect) -
                SS_GetCellPosXExt(lpSS, lpSS->Col.HeaderCnt + lpSS->Col.Frozen,
                                  lpSS->Col.HeaderCnt + lpSS->Col.Frozen, &Rect)) * -1;

lpSS->Col.UL = Pos;

SS_HScroll(lpSS, ScrollInc);
}


BOOL SS_ScrollLineUpTest(LPSPREADSHEET lpSS)
{
SS_COORD      RowAtOld;
SS_COORD      RowAtTemp;
SS_COORD      RowTopTemp;
SS_COORD      RowTopOld;
BOOL          fRet = FALSE;

RowAtOld = lpSS->Row.CurAt;
RowAtTemp = lpSS->Row.CurAt;

if (SS_IsRowVisible(lpSS, RowAtTemp, SS_VISIBLE_ALL))
   {
   RowAtTemp = lpSS->Row.CurAt;
   RowTopTemp = lpSS->Row.UL;
   RowTopOld = lpSS->Row.UL;

   while (RowAtTemp > lpSS->Row.HeaderCnt)
      {
      RowAtTemp--;
      if (RowAtTemp < lpSS->Row.UL && RowTopTemp >
      lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
     RowTopTemp--;

      if (SS_GetRowHeightInPixels(lpSS, RowAtTemp) > 0 &&
      (!lpSS->lpBook->fEditModePermanent ||
      !SS_GetLock(lpSS, lpSS->Col.CurAt, RowAtTemp, FALSE)))
     {
     lpSS->Row.CurAt = RowAtTemp;
     lpSS->Row.UL = RowTopTemp;
     break;
     }
      }

   if (RowAtOld == lpSS->Row.CurAt)
      fRet = TRUE;

   lpSS->Row.CurAt = RowAtOld;
   lpSS->Row.UL = RowTopOld;
   }

return (fRet);
}

BOOL SS_ScrollLineDownTest(LPSPREADSHEET lpSS)
{
SS_COORD      RowAtOld;
SS_COORD      RowAtTemp;
SS_COORD      RowTopTemp;
SS_COORD      RowTopOld;
BOOL          Found;
BOOL          fRet = FALSE;

RowAtTemp = lpSS->Row.CurAt;
RowAtOld = lpSS->Row.CurAt;
RowTopTemp = lpSS->Row.UL;
RowTopOld = lpSS->Row.UL;

if (SS_IsRowVisible(lpSS, RowAtTemp, SS_VISIBLE_ALL))
   {
   Found = FALSE;
   while (RowAtTemp < SS_GetRowCnt(lpSS) - 1)
      {
      if (RowAtTemp == lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1)
			RowAtTemp = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
      else
			RowAtTemp++;

		// RFW - 8/4/05 - 16323
      if (SS_GetRowHeightInPixels(lpSS, RowAtTemp) > 0)
			{
			while (!SS_IsRowVisible(lpSS, RowAtTemp, SS_VISIBLE_ALL) &&
					 RowAtTemp > lpSS->Row.UL)
				lpSS->Row.UL++;

			if ((!lpSS->lpBook->fEditModePermanent ||
				  !SS_GetLock(lpSS, lpSS->Col.CurAt, RowAtTemp, FALSE)))
				{
				lpSS->Row.CurAt = RowAtTemp;
				Found = TRUE;
				break;
				}
			}
      }

   if (!Found)
      lpSS->Row.UL = RowTopTemp;

   if (RowAtOld == lpSS->Row.CurAt)
      fRet = TRUE;

   lpSS->Row.CurAt = RowAtOld;
   lpSS->Row.UL = RowTopOld;
   }

return (fRet);
}


BOOL SS_LeaveCell(LPSPREADSHEET lpSS, SS_COORD ColOld, SS_COORD ColLeftOld, SS_COORD RowOld,
                  SS_COORD RowTopOld, BOOL fKillFocus)
{
HWND         hWnd = lpSS->lpBook->hWnd;
SS_LEAVECELL LeaveCell;
SS_LEAVEROW  LeaveRow;
BOOL         Cancel = FALSE;
BOOL         fRedraw;
BOOL         Ret;
BOOL         fHighlightOn;
SS_COORD     ColTemp;
SS_COORD     ColLeftTemp;
SS_COORD     RowTemp;
SS_COORD     RowTopTemp;
SS_COORD     ColAtSave;
SS_COORD     ColLeftSave;
SS_COORD     RowAtSave;
SS_COORD     RowTopSave;
#if SS_V80xxx
SS_CELLCOORD oldUL, oldLR, newUL, newLR;
#endif

if (!lpSS->lpBook->hWnd) // handle an OCX control without a HWND
   return (TRUE);

if (lpSS->wOpMode == SS_OPMODE_READONLY)
   return (TRUE);

if (ColOld == -1)
   LeaveCell.ColCurrent = lpSS->Col.CurAt;
else
   LeaveCell.ColCurrent = ColOld;

if (RowOld == -1)
   LeaveCell.RowCurrent = lpSS->Row.CurAt;
else
   LeaveCell.RowCurrent = RowOld;

//LeaveCell.ColCurrent = LeaveCell.ColCurrent - lpSS->Col.HeaderCnt + 1;
//LeaveCell.RowCurrent = LeaveCell.RowCurrent - lpSS->Row.HeaderCnt + 1;

if (fKillFocus)
   {
   LeaveCell.ColNew = -1;
   LeaveCell.RowNew = -1;
   }
else
   {
//   LeaveCell.ColNew = lpSS->Col.CurAt - lpSS->Col.HeaderCnt + 1;
//   LeaveCell.RowNew = lpSS->Row.CurAt - lpSS->Row.HeaderCnt + 1;
   LeaveCell.ColNew = lpSS->Col.CurAt;
   LeaveCell.RowNew = lpSS->Row.CurAt;
   }

if (LeaveCell.ColCurrent == LeaveCell.ColNew &&
    LeaveCell.RowCurrent == LeaveCell.RowNew)
   return (TRUE);

ColTemp = lpSS->Col.CurAt;
ColLeftTemp = lpSS->Col.UL;
RowTemp = lpSS->Row.CurAt;
RowTopTemp = lpSS->Row.UL;

if (LeaveCell.ColCurrent == LeaveCell.ColNew &&
    LeaveCell.RowCurrent == LeaveCell.RowNew)
   return (TRUE);

lpSS->lpBook->fWindowPainted = FALSE;

/* RFW - 18212 - 2/17/06
if (lpSS->lpBook->fEditModePermanent)
*/
if (lpSS->lpBook->fEditModePermanent && !fKillFocus)
   {
   if (SS_GetLock(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt, TRUE))
      {
      Ret = (BOOL)SS_SendMsgCoords(lpSS, SSM_CELLLOCKED,
                                   GetDlgCtrlID(lpSS->lpBook->hWnd),
                                   lpSS->Col.CurAt, lpSS->Row.CurAt);

      /*******************************************
      * Check to see if the cell is still locked
      *******************************************/
      
      if (SS_GetLock(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt, TRUE))
         {
#ifndef SS_UTP
         if (!Ret)
            SS_Beep(lpSS->lpBook);
#endif

         Cancel = TRUE;
         }
      }

   else if (lpSS->RestrictCols &&
       lpSS->Col.CurAt > lpSS->Col.DataCnt &&
       lpSS->RestrictRows &&
       lpSS->Row.CurAt > lpSS->Row.DataCnt)
      {
      if (!SS_SendMsgCoords(lpSS, SSM_COLROWRESTRICTED,
                            GetDlgCtrlID(lpSS->lpBook->hWnd),
                            lpSS->Col.CurAt, lpSS->Row.CurAt))
         SS_Beep(lpSS->lpBook);

      Cancel = TRUE;
      }

   else if (lpSS->RestrictCols && lpSS->Col.CurAt > lpSS->Col.DataCnt)
      {
		SS_COORD Col = lpSS->Col.CurAt;
		SS_AdjustCellCoordsOut(lpSS, &Col, NULL);
      if (!SS_SendMsg(lpSS->lpBook, lpSS, SSM_COLRESTRICTED, GetDlgCtrlID(lpSS->lpBook->hWnd), Col))
         SS_Beep(lpSS->lpBook);

      Cancel = TRUE;
      }

   else if (lpSS->RestrictRows && lpSS->Row.CurAt > lpSS->Row.DataCnt)
      {
		SS_COORD Row = lpSS->Row.CurAt;
		SS_AdjustCellCoordsOut(lpSS, NULL, &Row);
      if (!SS_SendMsg(lpSS->lpBook, lpSS, SSM_ROWRESTRICTED, GetDlgCtrlID(lpSS->lpBook->hWnd), Row))
         SS_Beep(lpSS->lpBook);

      Cancel = TRUE;
      }
   }

if (!Cancel)
   {
   lpSS->lpBook->wMessageBeingSent++;

   if (lpSS->wOpMode == SS_OPMODE_ROWMODE &&
       RowOld != -1 && RowOld != lpSS->Row.CurAt)
      {
      _fmemset(&LeaveRow, '\0', sizeof(SS_LEAVEROW));
      LeaveRow.RowCurrent = LeaveCell.RowCurrent;
      LeaveRow.RowNew = LeaveCell.RowNew;
      LeaveRow.fCancel = 0;

      if (lpSS->fRowModeEditing)
         {
         LeaveRow.fRowEdited = TRUE;
         LeaveRow.fRowChangeMade = lpSS->fRowModeChangeMade;
         LeaveRow.fAllCellsHaveData = SS_AllCellsHaveData(lpSS, RowOld);

         if (lpSS->DataRowCntOld < lpSS->Row.DataCnt)
            LeaveRow.fAddRow = TRUE;
         }

      if (lpSS->Row.CurAt >= lpSS->Row.DataCnt)
         LeaveRow.fRowNewBeyondLastDataRow = TRUE;

      RowAtSave = lpSS->Row.CurAt;
      RowTopSave = lpSS->Row.UL;

      if (RowOld != -1)
         lpSS->Row.CurAt = RowOld;

      if (RowTopOld != -1)
         lpSS->Row.UL = RowTopOld;

      fRedraw = lpSS->lpBook->Redraw;
      lpSS->lpBook->Redraw = FALSE;

      lpSS->fSetActiveCellCalled = FALSE;

		SS_AdjustCellCoordsOut(lpSS, NULL, &LeaveRow.RowCurrent);
		SS_AdjustCellCoordsOut(lpSS, NULL, &LeaveRow.RowNew);

      Cancel = (BOOL)SS_SendMsg(lpSS->lpBook, lpSS, SSM_LEAVEROW,
                                GetDlgCtrlID(lpSS->lpBook->hWnd),
                                (LPARAM)(LPSS_LEAVEROW)&LeaveRow);

      if (SS_IsDestroyed(hWnd))
         return (FALSE);

      lpSS->lpBook->Redraw = fRedraw;

      if (!lpSS->fSetActiveCellCalled)
         {
         lpSS->Row.CurAt = RowAtSave;
         lpSS->Row.UL = RowTopSave;
         }
      else
         lpSS->fSetActiveCellCalled = FALSE;

      if (LeaveRow.fCancel)
         Cancel = TRUE;

      if (!Cancel && lpSS->fRowModeEditing)
         {
         lpSS->fRowModeEditing = FALSE;

#ifndef SS_NOBUTTONDRAWMODE
         SS_InvalidateRowModeButtons(lpSS, ColOld, ColLeftOld,
                                     RowOld, RowTopOld);
#endif
         }
      }
   else
      {
      ColAtSave = lpSS->Col.CurAt;
      ColLeftSave = lpSS->Col.UL;
      RowAtSave = lpSS->Row.CurAt;
      RowTopSave = lpSS->Row.UL;

      if (ColOld != -1)
         lpSS->Col.CurAt = ColOld;

      if (ColLeftOld != -1)
         lpSS->Col.UL = ColLeftOld;

      if (RowOld != -1)
         lpSS->Row.CurAt = RowOld;

      if (RowTopOld != -1)
         lpSS->Row.UL = RowTopOld;

      fRedraw = lpSS->lpBook->Redraw;
      lpSS->lpBook->Redraw = FALSE;

      lpSS->fSetActiveCellCalled = FALSE;
      lpSS->fShowCellCalled = FALSE;
       
      // Fix for A-000378 (MsbBox in LeaveCell causes highlight problem)
      fHighlightOn = lpSS->HighlightOn;

		SS_AdjustCellCoordsOut(lpSS, &LeaveCell.ColCurrent, &LeaveCell.RowCurrent);
		SS_AdjustCellCoordsOut(lpSS, &LeaveCell.ColNew, &LeaveCell.RowNew);
      Cancel = (BOOL)SS_SendMsg(lpSS->lpBook, lpSS, SSM_LEAVECELL,
                                GetDlgCtrlID(lpSS->lpBook->hWnd),
                                (LPARAM)(LPSS_LEAVECELL)&LeaveCell);

      if (SS_IsDestroyed(hWnd))
       return (FALSE);
     
      // Fix for A-000378 (MsgBox in LeaveCell causes highlight problem)
      if (lpSS->HighlightOn && !fHighlightOn)
         SS_HighlightCell(lpSS, FALSE);
         
      lpSS->lpBook->Redraw = fRedraw;

      if (!lpSS->fSetActiveCellCalled)
         {
         lpSS->Col.CurAt = ColAtSave;
         lpSS->Row.CurAt = RowAtSave;
         if (!lpSS->fShowCellCalled)
            {
            lpSS->Col.UL = ColLeftSave;
            lpSS->Row.UL = RowTopSave;
            }
         }
      else
         {
         lpSS->fSetActiveCellCalled = FALSE;
         if (lpSS->Col.UL != ColLeftSave)
            SS_SetHScrollBar(lpSS);
         if (lpSS->Row.UL != RowTopSave)
            SS_SetVScrollBar(lpSS);
         }

#if defined(SS_V35) && !defined(SS_V40)
		// RFW - 4/16/02 - 9965
		if (SS_GetUserColAction(lpSS) == SS_USERCOLACTION_SORT && lpSS->Col.CurAt != ColOld)
			{
         int nIndicator = SS_GetMergedColUserSortIndicator(lpSS, lpSS->Col.CurAt);
         int nIndicatorOld = SS_GetMergedColUserSortIndicator(lpSS, ColOld);

			if (nIndicator == SS_COLUSERSORTINDICATOR_ASCENDING || nIndicator == SS_COLUSERSORTINDICATOR_ASCENDING ||
			    nIndicatorOld == SS_COLUSERSORTINDICATOR_ASCENDING || nIndicatorOld == SS_COLUSERSORTINDICATOR_ASCENDING)
				{
				BOOL fRedraw = lpSS->lpBook->Redraw;
				lpSS->lpBook->Redraw = FALSE;
				SS_InvalidateRow(lpSS, SS_GetColHeadersUserSortRow(lpSS));
				lpSS->lpBook->Redraw = fRedraw;
				}
			}
#endif // SS_V40

      lpSS->fShowCellCalled = FALSE;
      }

   lpSS->lpBook->wMessageBeingSent--;
   }

#ifndef SS_NOBUTTONDRAWMODE
if (!Cancel && lpSS->lpBook->wButtonDrawMode)
   if (lpSS->wOpMode != SS_OPMODE_ROWMODE)
      SS_InvalidateButtons(lpSS, ColOld, ColLeftOld, RowOld,
                           RowTopOld);
#endif

if (ColLeftTemp != lpSS->Col.UL || RowTopTemp != lpSS->Row.UL)
   {
   if (lpSS->lpBook->fEditModePermanent)
      SS_UpdateWindow(lpSS->lpBook);
   SS_HighlightCell(lpSS, TRUE);
   return (FALSE);
   }

if (ColTemp != lpSS->Col.CurAt || RowTemp != lpSS->Row.CurAt)
   {
   if (Cancel)
      {
      if (lpSS->lpBook->fEditModePermanent)
         SS_UpdateWindow(lpSS->lpBook);
      SS_HighlightCell(lpSS, TRUE);
      }

   return (!Cancel);
   }

if (Cancel)
   {
   if (ColOld != -1)
      lpSS->Col.CurAt = ColOld;

   if (ColLeftOld != -1)
      lpSS->Col.UL = ColLeftOld;

   if (RowOld != -1)
      lpSS->Row.CurAt = RowOld;

   if (RowTopOld != -1)
      lpSS->Row.UL = RowTopOld;

   SS_HighlightCell(lpSS, TRUE);
   }


return (!Cancel);
}


BOOL SS_AllCellsHaveData(lpSS, Row)

LPSPREADSHEET lpSS;
SS_COORD      Row;
{
SS_CELLTYPE   CellType;
LPSS_CELL     lpCell;
SS_COORD      i;
BOOL          fRet = TRUE;

for (i = lpSS->Col.HeaderCnt; i < SS_GetColCnt(lpSS) && fRet; i++)
   {
   SS_RetrieveCellType(lpSS, &CellType, NULL, i, Row);

   if (CellType.Type != SS_TYPE_STATICTEXT &&
       CellType.Type != SS_TYPE_OWNERDRAW &&
       CellType.Type != SS_TYPE_PICTURE &&
       CellType.Type != SS_TYPE_BUTTON &&
       CellType.Type != SS_TYPE_CHECKBOX)
      if (lpCell = SS_LockCellItem(lpSS, i, Row))
         {
         if (!lpCell->Data.bDataType)
            fRet = FALSE;

         SS_UnlockCellItem(lpSS, i, Row);
         }
      else
           fRet = FALSE;
   }

return (fRet);
}


BOOL SS_IsCellValid(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
BOOL fValid;

fValid = SS_GetColWidthInPixels(lpSS, Col) > 0 && SS_GetRowHeightInPixels(lpSS, Row) > 0 &&
         (!lpSS->lpBook->fEditModePermanent || !SS_GetLock(lpSS, Col, Row, FALSE));

#ifdef SS_V40
if (fValid)
	{
	// Only the Anchor cell of a span can be active
	if (SS_GetCellSpan(lpSS, Col, Row, NULL, NULL, NULL, NULL) == SS_SPAN_YES)
		fValid = FALSE;
	}
#endif // SS_V40

return (fValid);
}


BOOL SS_IsCellActivatable(LPSPREADSHEET lpSS, SS_COORD ColAt, SS_COORD RowAt)
{
BOOL Cancel = FALSE;
BOOL Ret;

if (ColAt < 0 || RowAt < 0)
	return (FALSE);

if (lpSS->lpBook->fEditModePermanent)
   {
   if (SS_GetLock(lpSS, ColAt, RowAt, FALSE))
      {
      Ret = (BOOL)SS_SendMsgCoords(lpSS, SSM_CELLLOCKED,
                                   GetDlgCtrlID(lpSS->lpBook->hWnd),
                                   ColAt, RowAt);

      /*******************************************
      * Check to see if the cell is still locked
      *******************************************/
      
      if (SS_GetLock(lpSS, ColAt, RowAt, FALSE))
         {
#ifndef SS_UTP
         if (!Ret)
            SS_Beep(lpSS->lpBook);
#endif

         Cancel = TRUE;
         }
      }

   else if (lpSS->RestrictCols && ColAt > lpSS->Col.DataCnt &&
       lpSS->RestrictRows && RowAt > lpSS->Row.DataCnt)
      {
      if (!SS_SendMsgCoords(lpSS, SSM_COLROWRESTRICTED,
                            GetDlgCtrlID(lpSS->lpBook->hWnd), ColAt, RowAt))
         SS_Beep(lpSS->lpBook);

      Cancel = TRUE;
      }

   else if (lpSS->RestrictCols && ColAt > lpSS->Col.DataCnt)
      {
      SS_AdjustCellCoordsOut(lpSS, &ColAt, NULL);
      if (!SS_SendMsg(lpSS->lpBook, lpSS, SSM_COLRESTRICTED, GetDlgCtrlID(lpSS->lpBook->hWnd), ColAt))
         SS_Beep(lpSS->lpBook);

      Cancel = TRUE;
      }

   else if (lpSS->RestrictRows && RowAt > lpSS->Row.DataCnt)
      {
      SS_AdjustCellCoordsOut(lpSS, NULL, &RowAt);
      if (!SS_SendMsg(lpSS->lpBook, lpSS, SSM_ROWRESTRICTED, GetDlgCtrlID(lpSS->lpBook->hWnd), RowAt))
         SS_Beep(lpSS->lpBook);

      Cancel = TRUE;
      }
   }

return (!Cancel);
}


void SS_VPos(LPSPREADSHEET lpSS, SS_COORD Pos)
{
short         ScrollInc;
RECT          Rect;

if (!SS_CellEditModeOff(lpSS, 0))
   return;

SS_HighlightCell(lpSS, FALSE);
SS_GetClientRect(lpSS->lpBook, &Rect);
/*
ScrollInc = SS_GetCellPosYExt(lpSS, lpSS->Row.HeaderCnt + lpSS->Row.Frozen, Pos,
                              &Rect) -
            SS_GetCellPosYExt(lpSS, lpSS->Row.HeaderCnt + lpSS->Row.Frozen,
                              lpSS->Row.UL, &Rect);
*/

if (Pos > lpSS->Row.UL)
   ScrollInc = SS_GetCellPosYExt(lpSS, lpSS->Row.UL, Pos, &Rect) -
               SS_GetCellPosYExt(lpSS, lpSS->Row.HeaderCnt + lpSS->Row.Frozen,
                                 lpSS->Row.HeaderCnt + lpSS->Row.Frozen, &Rect);
else
   ScrollInc = (SS_GetCellPosYExt(lpSS, Pos, lpSS->Row.UL, &Rect) -
                SS_GetCellPosYExt(lpSS, lpSS->Row.HeaderCnt + lpSS->Row.Frozen,
                                  lpSS->Row.HeaderCnt + lpSS->Row.Frozen, &Rect)) * -1;

lpSS->Row.UL = Pos;

SS_VScroll(lpSS, ScrollInc);
}


BOOL DLLENTRY SSShowCell(hWnd, CellCol, CellRow, Position)

HWND          hWnd;
SS_COORD      CellCol;
SS_COORD      CellRow;
short         Position;
{
LPSPREADSHEET lpSS;
BOOL          fRet;
BOOL          fEditModeOn = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &CellCol, &CellRow);

if (CellCol >= SS_GetColCnt(lpSS) ||
    CellRow >= SS_GetRowCnt(lpSS))
   {
   SS_SheetUnlock(hWnd);
   return (FALSE);
   }

if (lpSS->lpBook->EditModeOn)
   {
   BOOL wMessageBeingSentOld = lpSS->lpBook->wMessageBeingSent;
   lpSS->lpBook->wMessageBeingSent = FALSE;
   SS_CellEditModeOff(lpSS, 0);
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSentOld;
   SS_HighlightCell(lpSS, FALSE);
   fEditModeOn = TRUE;
   }

SS_SheetUnlock(hWnd);
fRet = SS_ShowCell(lpSS, CellCol, CellRow, Position);
lpSS->fShowCellCalled = TRUE;

if (fEditModeOn && SS_IsCellVisible(lpSS, lpSS->Col.CurAt,
    lpSS->Row.CurAt, SS_VISIBLE_ALL))
   SS_PostSetEditMode(lpSS, TRUE, 0L);
else
   SS_HighlightCell(lpSS, TRUE);

return (fRet);
}


BOOL DLLENTRY SSShowActiveCell(hWnd, Position)

HWND  hWnd;
short Position;
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
bRet = SS_ShowActiveCell(lpSS, Position);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_ShowActiveCell(lpSS, ShowWhere)

LPSPREADSHEET lpSS;
short         ShowWhere;
{
return SS_ShowCell(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt, ShowWhere);
}


BOOL SS_ShowCell(lpSS, CellCol, CellRow, ShowWhere)

LPSPREADSHEET lpSS;
SS_COORD      CellCol;
SS_COORD      CellRow;
short         ShowWhere;
{
RECT          Rect;
SS_COORD      RowAt;
SS_COORD      ColAt;
SS_COORD      RowTopOld;
SS_COORD      ColLeftOld;
SS_COORD      RowTopTemp;
SS_COORD      ColLeftTemp;
BOOL          ScrollCol = FALSE;
BOOL          ScrollRow = FALSE;
short         y;
short         x;
short         cy;
short         cx;

// BJO 18Jun97 GIL684 - Begin remove
//lpSS->Col.ULPrev = lpSS->Col.UL;
//lpSS->Row.ULPrev = lpSS->Row.UL;
// BJO 18Jun97 GIL684 - Begin remove

RowTopOld = lpSS->Row.UL;
ColLeftOld = lpSS->Col.UL;

// RFW - 6/1/04 - 14267
CellCol = min(CellCol, lpSS->Col.Max + lpSS->Col.HeaderCnt - 1);
CellRow = min(CellRow, lpSS->Row.Max + lpSS->Row.HeaderCnt - 1);

/* RFW - 9/30/05 - 17143
// RFW - 6/24/04 - 14338
if (CellCol < lpSS->Col.HeaderCnt || CellRow < lpSS->Row.HeaderCnt)
	return (FALSE);
*/
if (CellCol < lpSS->Col.HeaderCnt && CellRow < lpSS->Row.HeaderCnt)
	return (FALSE);

/**********
* Set Col
**********/

if (CellCol != -1)
   {
   switch (ShowWhere)
      {
      case SS_SHOW_TOPLEFT:
      case SS_SHOW_CENTERLEFT:
      case SS_SHOW_BOTTOMLEFT:
         if (CellCol != lpSS->Col.UL)
            {
            ColAt = max(lpSS->Col.HeaderCnt + lpSS->Col.Frozen, CellCol);

            if (ColAt != lpSS->Col.UL)
               {
               lpSS->Col.UL = ColAt;
               ScrollCol = TRUE;
               }
            }

         break;

      case SS_SHOW_TOPRIGHT:
      case SS_SHOW_CENTERRIGHT:
      case SS_SHOW_BOTTOMRIGHT:
         SS_GetClientRect(lpSS->lpBook, &Rect);

         x = SS_GetCellPosX(lpSS, CellCol, CellCol);
         cx = SS_GetColWidthInPixels(lpSS, CellCol);

         for (ColAt = CellCol; ColAt > lpSS->Col.HeaderCnt &&
              x + cx <= Rect.right - Rect.left; )
            {
            if (--ColAt < lpSS->Col.HeaderCnt)
               break;

            x += SS_GetColWidthInPixels(lpSS, ColAt);
            }

         if (x + cx > Rect.right - Rect.left)
            ColAt++;

         ColAt = max(ColAt, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);

         if (ColAt != lpSS->Col.UL)
            {
            lpSS->Col.UL = ColAt;
            ScrollCol = TRUE;
            }

         break;

      case SS_SHOW_TOPCENTER:
      case SS_SHOW_CENTER:
      case SS_SHOW_BOTTOMCENTER:
         SS_GetClientRect(lpSS->lpBook, &Rect);

         x = SS_GetCellPosX(lpSS, CellCol, CellCol);
         cx = SS_GetColWidthInPixels(lpSS, CellCol);

         Rect.left = x;

         for (ColAt = CellCol; ColAt > lpSS->Col.HeaderCnt &&
              x + cx / 2 < Rect.left + (Rect.right - Rect.left) / 2; )
            {
            if (--ColAt < lpSS->Col.HeaderCnt)
               break;

            x += SS_GetColWidthInPixels(lpSS, ColAt);
            }

         if (ColAt > lpSS->Col.HeaderCnt)
            ColAt++;

         ColAt = max(ColAt, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);

         if (ColAt != lpSS->Col.UL)
            {
            lpSS->Col.UL = ColAt;
            ScrollCol = TRUE;
            }

         break;

      case SS_SHOW_NEAREST:
         if (CellCol >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
             CellCol < lpSS->Col.UL)
            {
            lpSS->Col.UL = max(lpSS->Col.HeaderCnt +
                        lpSS->Col.Frozen, CellCol);
            ScrollCol = TRUE;
            }

         else if (CellCol >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
                  CellCol > SS_GetRightVisCell(lpSS, lpSS->Col.UL) &&
                  CellCol != lpSS->Col.UL)
            {
            SS_GetClientRect(lpSS->lpBook, &Rect);

            x = SS_GetCellPosX(lpSS, CellCol, CellCol) +
            SS_GetColWidthInPixels(lpSS, CellCol);

            for (ColAt = CellCol; ColAt > lpSS->Col.HeaderCnt; ColAt--)
               {
               x += SS_GetColWidthInPixels(lpSS, ColAt - 1);

               if (x > Rect.right - Rect.left)
                  break;
               }

            ColAt = max(lpSS->Col.HeaderCnt + lpSS->Col.Frozen, ColAt);

            if (ColAt != lpSS->Col.UL)
               {
               lpSS->Col.UL = ColAt;
               ScrollCol = TRUE;
               }
            }

         break;

      }
   }

/**********
* Set Row
**********/

if (CellRow != -1)
   {
   switch (ShowWhere)
      {
      case SS_SHOW_TOPLEFT:
      case SS_SHOW_TOPCENTER:
      case SS_SHOW_TOPRIGHT:
     if (CellRow != lpSS->Row.UL)
        {
        RowAt = max(lpSS->Row.HeaderCnt + lpSS->Row.Frozen, CellRow);

        if (RowAt != lpSS->Row.UL)
           {
           lpSS->Row.UL = RowAt;
           ScrollRow = TRUE;
           }
        }

     break;

      case SS_SHOW_BOTTOMLEFT:
      case SS_SHOW_BOTTOMCENTER:
      case SS_SHOW_BOTTOMRIGHT:
     SS_GetClientRect(lpSS->lpBook, &Rect);

     y = SS_GetCellPosY(lpSS, CellRow, CellRow);
     cy = SS_GetRowHeightInPixels(lpSS, CellRow);

     for (RowAt = CellRow; RowAt > lpSS->Row.HeaderCnt &&
          y + cy <= Rect.bottom - Rect.top; )
        {
        if (--RowAt < lpSS->Row.HeaderCnt)
           break;

        y += SS_GetRowHeightInPixels(lpSS, RowAt);
        }
     
     if (y + cy > Rect.bottom - Rect.top)
        RowAt++;

     RowAt = max(RowAt, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);

     if (lpSS->Row.UL != RowAt)
        {
        lpSS->Row.UL = RowAt;
        ScrollRow = TRUE;
        }

     break;

      case SS_SHOW_CENTERLEFT:
      case SS_SHOW_CENTER:
      case SS_SHOW_CENTERRIGHT:
     SS_GetClientRect(lpSS->lpBook, &Rect);

     y = SS_GetCellPosY(lpSS, CellRow, CellRow);
     cy = SS_GetRowHeightInPixels(lpSS, CellRow);

     Rect.top = y;

     for (RowAt = CellRow; RowAt > lpSS->Row.HeaderCnt &&
          y + cy / 2 < Rect.top + (Rect.bottom - Rect.top) / 2; )
        {
        if (--RowAt < lpSS->Row.HeaderCnt)
           break;

        y += SS_GetRowHeightInPixels(lpSS, RowAt);
        }

     if (RowAt > lpSS->Row.HeaderCnt)
        RowAt++;

     RowAt = max(RowAt, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);

     if (lpSS->Row.UL != RowAt)
        {
        lpSS->Row.UL = RowAt;
        ScrollRow = TRUE;
        }

     break;

      case SS_SHOW_NEAREST:
     if (CellRow >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
         CellRow < lpSS->Row.UL)
        {
        lpSS->Row.UL = max(lpSS->Row.HeaderCnt +
                      lpSS->Row.Frozen, CellRow);
        ScrollRow = TRUE;
        }

     else if (CellRow >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
              CellRow > SS_GetBottomVisCell(lpSS, lpSS->Row.UL) &&
              CellRow != lpSS->Row.UL)
        {
        SS_GetClientRect(lpSS->lpBook, &Rect);

        y = SS_GetCellPosY(lpSS, CellRow, CellRow) +
        SS_GetRowHeightInPixels(lpSS, CellRow);

        for (RowAt = CellRow; RowAt > lpSS->Row.HeaderCnt; RowAt--)
           {
           y += SS_GetRowHeightInPixels(lpSS, RowAt - 1);

           if (y > Rect.bottom - Rect.top)
              break;
           }

        lpSS->Row.UL = max(lpSS->Row.HeaderCnt + lpSS->Row.Frozen, RowAt);
        ScrollRow = TRUE;
        }

     break;

      }
   }

/************
* Show Cell
************/

if (ScrollCol || ScrollRow)
   {
   RowTopTemp = lpSS->Row.UL;
   ColLeftTemp = lpSS->Col.UL;
   lpSS->Row.UL = RowTopOld;
   lpSS->Col.UL = ColLeftOld;

   SS_HighlightCell(lpSS, FALSE);

	if (lpSS->lpBook->EditModeOn) // RFW - 5/13/05 - 16245
	   if (!SS_CellEditModeOff(lpSS, 0))
	      return (FALSE);

   // BJO 18Jun97 GIL684 - Begin add
   lpSS->Row.ULPrev = RowTopOld;
   lpSS->Col.ULPrev = ColLeftOld;
   // BJO 18Jun97 GIL684 - End add

   lpSS->Row.UL = RowTopTemp;
   lpSS->Col.UL = ColLeftTemp;
   }

SS_GetClientRect(lpSS->lpBook, &Rect);

x = 0;
y = 0;

if (ScrollCol)
   {
   if (lpSS->Col.UL < ColLeftOld)
      x = -(SS_GetCellPosXExt(lpSS, lpSS->Col.UL, ColLeftOld, &Rect) -
        SS_GetCellPosX(lpSS, ColLeftOld, ColLeftOld));
   else
      x = SS_GetCellPosXExt(lpSS, ColLeftOld, lpSS->Col.UL, &Rect) -
      SS_GetCellPosX(lpSS, ColLeftOld, ColLeftOld);
   }

if (ScrollRow)
   {
   if (lpSS->Row.UL < RowTopOld)
      y = -(SS_GetCellPosYExt(lpSS, lpSS->Row.UL, RowTopOld, &Rect) -
        SS_GetCellPosY(lpSS, RowTopOld, RowTopOld));
   else
      y = SS_GetCellPosYExt(lpSS, RowTopOld, lpSS->Row.UL, &Rect) -
      SS_GetCellPosY(lpSS, RowTopOld, RowTopOld);
   }

if (x == 0 && ScrollCol && lpSS->Col.hCellSizeList)
   {
   SS_ResetCellSizeList(&lpSS->Col);
   }

if (y == 0 && ScrollRow && lpSS->Row.hCellSizeList)
   {
   SS_ResetCellSizeList(&lpSS->Row);
   }

if (x && y)
   {
   SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
	/* RFW - 5/29/09 - 25762
   if (lpSS->lpBook->wMessageBeingSent)
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
   else
      {
      SS_GetScrollArea(lpSS, &Rect, SS_SCROLLAREA_ALL);
      SS_Scroll(lpSS, x, y, &Rect);
      SS_UpdateWindow(lpSS->lpBook);
      }
	*/

   SS_SetVScrollBar(lpSS);
   SS_SetHScrollBar(lpSS);
   SS_TopLeftChange(lpSS);
   }

else if (y)
   {
   if (abs(y) < Rect.bottom && !lpSS->lpBook->wMessageBeingSent && lpSS->lpBook->Redraw)
		{
		// RFW - 6/17/04 - 14603
	   SS_SetVScrollBar(lpSS);
		if (RowTopOld != lpSS->Row.UL)
	      SS_VScroll(lpSS, y);
		}
   else
      {
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ROWS);
      SS_SetVScrollBar(lpSS);
		if (RowTopOld != lpSS->Row.UL)
	      SS_TopRowChange(lpSS);
      }
   }

else if (x)
   {
   if (abs(x) < Rect.right && !lpSS->lpBook->wMessageBeingSent && lpSS->lpBook->Redraw)
		{
		// RFW - 6/17/04 - 14603
	   SS_SetHScrollBar(lpSS);
		if (ColLeftOld != lpSS->Col.UL)
			SS_HScroll(lpSS, x);
		}
   else
      {
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_COLS);
      SS_SetHScrollBar(lpSS);
		if (ColLeftOld != lpSS->Col.UL)
	      SS_LeftColChange(lpSS);
      }
   }

else
   SS_HighlightCell(lpSS, TRUE);

return (TRUE);
}


SS_COORD SS_CalcLeftAllVisCol(LPSPREADSHEET lpSS)
{
RECT rcClient;
SS_COORD nMaxCols = SS_GetColCnt(lpSS);
SS_COORD i;
int x;

SS_GetClientRect(lpSS->lpBook, &rcClient);
x = rcClient.left - 1;
for (i = 0; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen; i++)
   x += SS_GetColWidthInPixels(lpSS, i);
for (i = lpSS->Col.UL; i < nMaxCols; i++)
   {
   x += SS_GetColWidthInPixels(lpSS, i);
   if (x > rcClient.right)
      break;
   }
return max(lpSS->Col.UL, i - 1);
}


SS_COORD SS_CalcBottomAllVisRow(LPSPREADSHEET lpSS)
{
RECT rcClient;
SS_COORD nMaxRows = SS_GetRowCnt(lpSS);
SS_COORD i;
int x;

SS_GetClientRect(lpSS->lpBook, &rcClient);
x = rcClient.top - 1;
for (i = 0; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen; i++)
   x += SS_GetRowHeightInPixels(lpSS, i);
for (i = lpSS->Row.UL; i < nMaxRows; i++)
   {
   x += SS_GetRowHeightInPixels(lpSS, i);
   if (x > rcClient.bottom)
      break;
   }
return max(lpSS->Row.UL, i - 1);
}


void SS_SetHScrollBar(LPSPREADSHEET lpSS)
{
RECT Rect;

SS_GetClientRect(lpSS->lpBook, &Rect);
SSx_SetHScrollBar(lpSS, &Rect);

}


void SSx_SetHScrollBar(LPSPREADSHEET lpSS, LPRECT lpRect)
{
SS_COORD ScrollLast;
SS_COORD Cols;
SS_COORD i;
short    dPos;
short    x;
BOOL     SetScrollBar = TRUE;
BOOL     fNoScrollBar = FALSE;
BOOL     fDisableScrollBar = FALSE;
unsigned fHScrollVisibleSave = lpSS->lpBook->fHScrollVisible;
SS_COORD nMax;
#ifndef SS_NOCHILDSCROLL
HWND     hwndScroll = lpSS->lpBook->hWndHScroll;
int      nBar = SB_CTL;
#else
HWND     hwndScroll = lpSS->lpBook->hWnd;
int      nBar = SB_HORZ;
#endif

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   return;

// added this fix for !SPREAD_JPN -scl
// 97' 2/3 Modified by BOC Gao. for BUG000734
// When set Redraw to False, ScrollBars display wrong
// Here just allow it recalculating.
//#ifdef SPREAD_JPN
#if SS_V80
if (lpSS->lpBook->fNoSetScrollBars || lpSS->lpBook->fWmSizeMoveScrollBars || !lpSS->lpBook->Redraw)
#else
if (lpSS->lpBook->fNoSetScrollBars || !lpSS->lpBook->Redraw)
#endif
//#else
//if (lpSS->lpBook->fNoSetScrollBars || !lpSS->lpBook->Redraw)
//#endif
   return;

if (!SS_IsActiveSheet(lpSS))
	return;

lpSS->lpBook->fHScrollVisible = TRUE;

if (lpSS->lpBook->HorzScrollBar)
   {
   if (!lpSS->lpBook->fScrollBarShowMax)
      {
      ScrollLast = max(lpSS->Col.DataCnt - 1, SS_DEFAULT_HSCROLL +
               lpSS->Col.HeaderCnt - 1);

      ScrollLast = min(SS_GetColCnt(lpSS) - 1, ScrollLast);
      }

   else
   {
      ScrollLast = SS_GetColCnt(lpSS) - 1;
   }
#ifdef SS_V80
/* RFW - 1/13/10 - 25978
	  SendMessage(hwndScroll, SBR_SETPAGESIZE, SS_GetRightVisCell(lpSS, 0), lpSS->lpBook->Redraw);
*/
	  SendMessage(hwndScroll, SBR_SETPAGESIZE, max(1, SS_GetColCnt(lpSS) - SS_GetLastPageLeftCol(lpSS)), lpSS->lpBook->Redraw);
#endif
   nMax = ScrollLast;

   if (lpSS->lpBook->fScrollBarMaxAlign)
      {
      ScrollLast = min(SS_GetLastPageLeftCol(lpSS), ScrollLast);
      #ifdef WIN32
      if (!SS_AdjustThumbSize(lpSS))
      #endif
         nMax = ScrollLast;
      }

   if (lpSS->lpBook->fScrollBarExtMode)
      {
      Cols = SS_GetColCnt(lpSS);

      if (lpSS->lpBook->fAutoSize && lpSS->Col.AutoSizeVisibleCnt &&
          lpSS->Col.AutoSizeVisibleCnt < Cols - 1)
         {
         // BJO 02Apr98 JOK4981 - Before fix
         //SetScrollBar = TRUE;
         //fNoScrollBar = FALSE;
         // BJO 02Apr98 JOK4981 - Begin fix
         BOOL fAnyVisible = FALSE;
         for (i = lpSS->Col.AutoSizeVisibleCnt + 1; !fAnyVisible && i < Cols; i++)
            {
            fAnyVisible = SS_GetColWidthInPixels(lpSS, i) > 0;
            }
         if (fAnyVisible)
            {
            SetScrollBar = TRUE;
            fNoScrollBar = FALSE;
            }
         else
            {
            SetScrollBar = FALSE;
            fNoScrollBar = TRUE;
            }
         // BJO 02Apr98 JOK4981 - End fix
         }

      else
         {
			RECT RectClient;

         SetScrollBar = FALSE;
         fNoScrollBar = TRUE;

			GetClientRect(lpSS->lpBook->hWnd, &RectClient); // RFW - 8/22/06 - 19431

			if (!IsRectEmpty(&RectClient)) // RFW - 7/21/06 - 19098
				{
				for (i = 0, x = -1; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen && i < Cols; i++)
					x += SS_GetColWidthInPixels(lpSS, i);

				for (i = max(i, lpSS->Col.HeaderCnt); i < Cols; i++)
					{
					x += SS_GetColWidthInPixels(lpSS, i);
					// RFW - 7/14/99 - GIC9756            if (x > Rect.right + 1)
					if (x > lpRect->right)
						{
						SetScrollBar = TRUE;
						fNoScrollBar = FALSE;
						break;
						}
					}
				}

         /*
         Cols = min(SS_GetColCnt(lpSS) - 1, 50);

         x = SS_GetCellPosX(lpSS, lpSS->Col.HeaderCnt, Cols + 1);
         SS_GetClientRect(lpSS->lpBook, &Rect);
         if (x <= Rect.right + 1)
            {
            SetScrollBar = FALSE;
            fNoScrollBar = TRUE;
            }
         */
         }
      }

   if (SetScrollBar)
      {
		int nMinPos = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;

		// Set the scroll Min past any hidden cols
      while (nMinPos <= ScrollLast && SS_GetColWidthInPixels(lpSS, nMinPos) == 0)
         nMinPos++;

		// Set the scroll Max before any hidden cols
      while (ScrollLast > nMinPos && SS_GetColWidthInPixels(lpSS, ScrollLast) == 0)
			ScrollLast--;

		/* RFW - 5/15/07 - 20369
      if (nMinPos < ScrollLast)
		*/
      if (nMinPos <= ScrollLast)
         {
         if (nMax <= SS_SBMAX)
            dPos = (short)max(nMinPos, lpSS->Col.UL);
         else
            dPos = (short)((double)max(nMinPos, lpSS->Col.UL) * ((double)SS_SBMAX /
                   (double)ScrollLast));

         #ifndef SS_NOCHILDSCROLL
         ShowScrollBar(hwndScroll, nBar, TRUE);
         #endif
#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, nMinPos, (int)min(nMax, (long)SS_SBMAX));
#else
         SetScrollRange(hwndScroll, nBar, nMinPos,
                        (int)min(nMax, (long)SS_SBMAX), FALSE);
#endif

         #ifdef WIN32
         if (SS_AdjustThumbSize(lpSS))
            {
            SCROLLINFO si;
				/* RFW - 8/11/03 - 12342
            UINT nPage = SS_CalcLeftAllVisCol(lpSS) - lpSS->Col.UL + 1;
				*/
	         UINT nPage = max(1, SS_GetColCnt(lpSS) - SS_GetLastPageLeftCol(lpSS));
            if (nMax > SS_SBMAX)
              nPage = (UINT)((double)nPage * (double)SS_SBMAX / (double)nMax);
            si.cbSize = sizeof(si);
            si.fMask = SIF_PAGE;
            si.nPage = max(1, nPage);
            SetScrollInfo(hwndScroll, nBar, &si, FALSE);
            }
         #endif

#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETPOS, dPos, TRUE); 
#else
         SetScrollPos(hwndScroll, nBar, dPos, TRUE);
#endif
         }
      else
         {
#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, 0, 0);
#else
         SetScrollRange(hwndScroll, nBar, 0, 0, TRUE);
#endif
#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETPOS, 0, TRUE); 
#else
         SetScrollPos(hwndScroll, nBar, 0, TRUE);
#endif
         fDisableScrollBar = TRUE;
         }
      }
   
   }

else if (lpSS->lpBook->fHScrollVisible)
   {
   #ifndef SS_NOCHILDSCROLL
   lpSS->lpBook->fHScrollVisible = FALSE;
   ShowScrollBar(hwndScroll, nBar, FALSE);
   #endif
#ifdef SS_V80
/* RFW - 1/13/10 - 25978
	SendMessage(hwndScroll, SBR_SETPAGESIZE, SS_GetRightVisCell(lpSS, 0), lpSS->lpBook->Redraw);
*/
	SendMessage(hwndScroll, SBR_SETPAGESIZE, max(1, SS_GetColCnt(lpSS) - SS_GetLastPageLeftCol(lpSS)), lpSS->lpBook->Redraw);
	SendMessage(hwndScroll, SBM_SETRANGE, 0,0);
#else
   SetScrollRange(hwndScroll, nBar, 0, 0, TRUE);
#endif
   }

if (fNoScrollBar || fDisableScrollBar)
   {
   if (fNoScrollBar)
      {
      #ifndef SS_NOCHILDSCROLL
      lpSS->lpBook->fHScrollVisible = FALSE;
      ShowScrollBar(hwndScroll, nBar, FALSE);
      #endif
#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, 0,0);
#else
      SetScrollRange(hwndScroll, nBar, 0, 0, TRUE);
#endif
      }
   else
      {
      #ifndef SS_NOCHILDSCROLL
      ShowScrollBar(hwndScroll, nBar, TRUE);
      #endif
#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, 0,-1);
#else
      SetScrollRange(hwndScroll, nBar, 0, -1, TRUE);
#endif
      }

   if (lpSS->Col.UL > lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      {
      lpSS->Col.UL = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
      SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
      }
   }

#ifdef SS_V70
if (fHScrollVisibleSave != lpSS->lpBook->fHScrollVisible)
	{
	SS_EmptyTabStripRect(lpSS->lpBook);
	SS_InvalidateTabStrip(lpSS->lpBook);

   if (lpSS->lpBook->fVScrollVisible && lpSS->lpBook->fHScrollVisible)
		{
		RECT RectTemp;

		GetClientRect(lpSS->lpBook->hWnd, &RectTemp);
		RectTemp.left = RectTemp.right - lpSS->lpBook->dVScrollWidth - 1;
		RectTemp.top = RectTemp.bottom - lpSS->lpBook->dHScrollHeight - 1;
		InvalidateRect(lpSS->lpBook->hWnd, &RectTemp, FALSE);
		}
	}
#endif // SS_V70
}


void SS_SetVScrollBar(LPSPREADSHEET lpSS)
{
RECT Rect;
SS_GetClientRect(lpSS->lpBook, &Rect);
SSx_SetVScrollBar(lpSS, &Rect);
}


void SSx_SetVScrollBar(LPSPREADSHEET lpSS, LPRECT lpRect)
{
SS_COORD ScrollLast;
SS_COORD Rows;
SS_COORD RowTop;
SS_COORD i;
short    dPos;
short    y;
BOOL     SetScrollBar = TRUE;
BOOL     fNoScrollBar = FALSE;
BOOL     fShowBuffer = FALSE;
BOOL     fDisableScrollBar = FALSE;
SS_COORD nMax;
#ifndef SS_NOCHILDSCROLL
HWND     hwndScroll = lpSS->lpBook->hWndVScroll;
int      nBar = SB_CTL;
#else
HWND     hwndScroll = lpSS->lpBook->hWnd;
int      nBar = SB_VERT;
#endif

// added this fix for !SPREAD_JPN -scl
// 97' 2/3 Modified by BOC Gao. for BUG000734
// When set Redraw to False, ScrollBars display wrong
// Here just allow it recalculating.
//#ifdef SPREAD_JPN
#if SS_V80
if (lpSS->lpBook->fNoSetScrollBars || lpSS->lpBook->fWmSizeMoveScrollBars || !lpSS->lpBook->Redraw)
#else
if (lpSS->lpBook->fNoSetScrollBars || !lpSS->lpBook->Redraw)
#endif
//#else
//if (lpSS->lpBook->fNoSetScrollBars || !lpSS->lpBook->Redraw)
//#endif
// ----------------------------------------<<
   return;

if (!SS_IsActiveSheet(lpSS))
	return;

lpSS->lpBook->fVScrollVisible = TRUE;

if (lpSS->fVirtualMode && (lpSS->Virtual.lVStyle & SSV_SCROLLBARSHOWBUFFER))
   fShowBuffer = TRUE;

#ifdef SS_V80
/* RFW - 1/13/10 - 25978
SendMessage(hwndScroll, SBR_SETPAGESIZE, SS_GetBottomVisCell(lpSS, 0), lpSS->lpBook->Redraw);
*/
SendMessage(hwndScroll, SBR_SETPAGESIZE, max(1, SS_GetRowCnt(lpSS) - SS_GetLastPageTopRow(lpSS)), lpSS->lpBook->Redraw);
#endif

if (lpSS->lpBook->VertScrollBar)
   {
   if (fShowBuffer)
      ScrollLast = lpSS->Virtual.VPhysSize;

   else if (!lpSS->lpBook->fScrollBarShowMax)
      {
      ScrollLast = max(lpSS->Row.DataCnt - 1,
               SS_DEFAULT_VSCROLL + lpSS->Row.HeaderCnt - 1);
		// RFW - 2/21/05 - 15763
		ScrollLast = max(ScrollLast, SS_GetBottomCell(lpSS, lpSS->Row.UL));

      ScrollLast = min(SS_GetRowCnt(lpSS) - 1, ScrollLast);
      }

   else
      ScrollLast = SS_GetRowCnt(lpSS) - 1;

   nMax = ScrollLast;

   if (lpSS->lpBook->fScrollBarMaxAlign)
      {
      if (fShowBuffer)
         ScrollLast = min(SSx_GetLastPageTopRow(lpSS,
                          lpSS->Virtual.VTop +
                          lpSS->Virtual.VPhysSize) -
                          lpSS->Virtual.VTop, ScrollLast);
      else
         ScrollLast = min(SS_GetLastPageTopRow(lpSS), ScrollLast);
      #ifdef WIN32
      if (!SS_AdjustThumbSize(lpSS))
      #endif
         nMax = ScrollLast;
      }

   if (lpSS->lpBook->fScrollBarExtMode)
      {
      Rows = SS_GetRowCnt(lpSS);

      if (lpSS->lpBook->fAutoSize && lpSS->Row.AutoSizeVisibleCnt &&
          lpSS->Row.AutoSizeVisibleCnt < Rows - 1)
         {
         // BJO 01Apr98 JOK4981 - Before fix
         //SetScrollBar = TRUE;
         //fNoScrollBar = FALSE;
         // BJO 01Apr98 JOK4981 - Begin fix
         BOOL fAnyVisible = FALSE;
         for( i = lpSS->Row.AutoSizeVisibleCnt + 1; !fAnyVisible && i < Rows; i++)
            {
            fAnyVisible = SS_GetRowHeightInPixels(lpSS, i) > 0;
            }
         if (fAnyVisible)
            {
            SetScrollBar = TRUE;
            fNoScrollBar = FALSE;
            }
         else
            {
            SetScrollBar = FALSE;
            fNoScrollBar = TRUE;
            }
         // BJO 01Apr98 JOK4981 - Begin fix
         }

      else
         {
         SetScrollBar = FALSE;
         fNoScrollBar = TRUE;

         for (i = 0, y = -1; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
              i < Rows; i++)
            y += SS_GetRowHeightInPixels(lpSS, i);

         for (i = max(i, lpSS->Row.HeaderCnt); i < Rows; i++)
            {
            y += SS_GetRowHeightInPixels(lpSS, i);
// RFW - 7/14/99 - GIC9756            if (y > Rect.bottom + 1)
            if (y > lpRect->bottom)
               {
               SetScrollBar = TRUE;
               fNoScrollBar = FALSE;
               break;
               }
            }
         }
      }

   if (SetScrollBar)
      {
		int nMinPos = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;

		// Set the scroll Min past any hidden cols
      while (nMinPos <= ScrollLast && SS_GetRowHeightInPixels(lpSS, nMinPos) == 0)
         nMinPos++;

		// Set the scroll Max before any hidden cols
      while (ScrollLast > nMinPos && SS_GetRowHeightInPixels(lpSS, ScrollLast) == 0)
			ScrollLast--;

		/* RFW - 5/15/07 - 20369
      if (nMinPos < ScrollLast)
		*/
      if (nMinPos <= ScrollLast)
         {
         if (nMax <= SS_SBMAX)
            dPos = fShowBuffer ? (short)lpSS->Row.UL -
               (short)lpSS->Virtual.VTop + 1 : (short)lpSS->Row.UL;
         else
            {
            if (fShowBuffer)
               RowTop = lpSS->Row.UL - lpSS->Virtual.VTop + 1;
            else
               RowTop = lpSS->Row.UL;

            dPos = (short)((double)RowTop * ((double)SS_SBMAX /
                    (double)ScrollLast));
            }

         dPos = (short)max(nMinPos, dPos);

         #ifndef SS_NOCHILDSCROLL
         ShowScrollBar(hwndScroll, nBar, TRUE);
         #endif
#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, nMinPos, (short)min(nMax, (long)SS_SBMAX));
#else
         SetScrollRange(hwndScroll, nBar, nMinPos,
                        (short)min(nMax, (long)SS_SBMAX), FALSE);
#endif

         #ifdef WIN32
         if (SS_AdjustThumbSize(lpSS))
            {
            SCROLLINFO si;
				/* RFW - 8/11/03 - 12342
            UINT nPage = SS_CalcBottomAllVisRow(lpSS) - lpSS->Row.UL + 1;
				*/
	         UINT nPage = max(1, SS_GetRowCnt(lpSS) - SS_GetLastPageTopRow(lpSS));
            if (nMax > SS_SBMAX)
              nPage = (UINT)((double)nPage * (double)SS_SBMAX / (double)nMax);
            si.cbSize = sizeof(si);
            si.fMask = SIF_PAGE;
            si.nPage = max(1, nPage);
            SetScrollInfo(hwndScroll, nBar, &si, FALSE);
            }
         #endif

#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETPOS, dPos, TRUE); 
#else
         SetScrollPos(hwndScroll, nBar, dPos, TRUE);
#endif
         }
      else
         {
 #ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, 0,0);
#else
        SetScrollRange(hwndScroll, nBar, 0, 0, TRUE);
#endif
#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETPOS, 0, TRUE); 
#else
         SetScrollPos(hwndScroll, nBar, 0, TRUE);
#endif
         fDisableScrollBar = TRUE;
         }
      }
   }

else if (lpSS->lpBook->fVScrollVisible)
   {
   #ifndef SS_NOCHILDSCROLL
   lpSS->lpBook->fVScrollVisible = FALSE;
   ShowScrollBar(hwndScroll, nBar, FALSE);
   #endif
#ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, 0,0);
#else
   SetScrollRange(hwndScroll, nBar, 0, 0, TRUE);
#endif
   }

if (fNoScrollBar || fDisableScrollBar)
   {
   if (fNoScrollBar)
      {
      #ifndef SS_NOCHILDSCROLL
      lpSS->lpBook->fVScrollVisible = FALSE;
      ShowScrollBar(hwndScroll, nBar, FALSE);
      #endif
 #ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, 0,0);
#else
      SetScrollRange(hwndScroll, nBar, 0, 0, TRUE);
#endif
      }
   else
      {
      #ifndef SS_NOCHILDSCROLL
      ShowScrollBar(hwndScroll, nBar, TRUE);
      #endif
 #ifdef SS_V80
		 SendMessage(hwndScroll, SBM_SETRANGE, 0,-1);
#else
      SetScrollRange(hwndScroll, nBar, 0, -1, TRUE);
#endif
      }

   if (!fShowBuffer && lpSS->Row.UL > lpSS->Row.HeaderCnt +
       lpSS->Row.Frozen)
      {
      lpSS->Row.UL = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
      SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
      }
   }
}


BOOL SS_ActivateCell(lpSS, Col, Row, Show)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
BOOL          Show;
{
BOOL          fKillFocusReceived;
BOOL          fRet = FALSE;

if (lpSS->lpBook->EditModeOn)
   {
   WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;
   lpSS->lpBook->wMessageBeingSent = FALSE;
   if (!SS_CellEditModeOff(lpSS, 0))
      {
      lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;
      return (FALSE);
      }
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;
   }

if ((lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
    lpSS->wOpMode == SS_OPMODE_MULTISEL ||
    lpSS->wOpMode == SS_OPMODE_EXTSEL ||
    lpSS->wOpMode == SS_OPMODE_NORMAL) && GetFocus() != lpSS->lpBook->hWnd)
   {
   if (SS_USESINGLESELBAR(lpSS) && lpSS->lpBook->fKillFocusReceived)
      lpSS->fNoDrawFocusRect = TRUE;

   SS_HighlightCell(lpSS, FALSE);
   lpSS->fNoDrawFocusRect = FALSE;
   }

else
   SS_HighlightCell(lpSS, FALSE);

if (SS_IsCellActivatable(lpSS, Col, Row))
   {
   lpSS->Row.CurAt = Row;
   lpSS->Col.CurAt = Col;

   if (Show)
      {
      if (SS_USESINGLESELBAR(lpSS) && GetFocus() != lpSS->lpBook->hWnd)
         lpSS->FreezeHighlight = TRUE;

      SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
      }
   else
      SS_HighlightCell(lpSS, TRUE);

   fRet = TRUE;
   }
else
   SS_HighlightCell(lpSS, TRUE);

#if 0
if (lpSS->wOpMode == SS_OPMODE_SINGLESEL)
   {
   if (lpSS->lpBook->hWnd && GetFocus() != lpSS->lpBook->hWnd)
      {
      hDC = SS_GetDC(lpSS->lpBook);
      SSx_HighlightCell(lpSS, hDC, TRUE);

      if (lpSS->fKillFocusReceived)
      {
      lpSS->fProcessingKillFocus = TRUE;
      SSx_HighlightCell(lpSS, hDC, FALSE);
      lpSS->fProcessingKillFocus = FALSE;
      }

      SS_ReleaseDC(lpSS->lpBook, hDC);
      }
   }
#endif

if (SS_USESINGLESELBAR(lpSS))
   {
   if (lpSS->lpBook->hWnd && GetFocus() != lpSS->lpBook->hWnd)
      {
      lpSS->FreezeHighlight = FALSE;
      fKillFocusReceived = lpSS->lpBook->fKillFocusReceived;
      SS_HighlightCell(lpSS, TRUE);

      if (fKillFocusReceived)
         {
         lpSS->lpBook->fProcessingKillFocus = TRUE;
         SS_HighlightCell(lpSS, FALSE);
         lpSS->lpBook->fProcessingKillFocus = FALSE;
         }
      }
   else
      lpSS->lpBook->fKillFocusReceived = FALSE;
   }

return (fRet);
}


void SS_Scroll(lpSS, xScroll, yScroll, Rect)

LPSPREADSHEET   lpSS;
int             xScroll;
int             yScroll;
LPRECT          Rect;
{
SS_COLORTBLITEM BackColorTblItem;
HRGN            hRgn;
HDC             hDC;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

hDC = SS_GetDC(lpSS->lpBook);

if ((xScroll == 0 && abs(yScroll) >= Rect->bottom - Rect->top) ||
    (yScroll == 0 && abs(xScroll) >= Rect->right - Rect->left))
   {
   SS_GetColorItem(&BackColorTblItem, lpSS->Color.BackgroundId);
   SS_InvalidateRect(lpSS->lpBook, Rect, TRUE);
   }
else
   {
	HRGN hRgn2;

   if (hRgn = CreateRectRgn(0, 0, 0, 0))
      {
      if (GetUpdateRgn(lpSS->lpBook->hWnd, hRgn, FALSE) != NULLREGION)
         {
         if (hRgn2 = CreateRectRgn(Rect->left, Rect->top, Rect->right,
             Rect->bottom))
            {
            if (CombineRgn(hRgn, hRgn, hRgn2, RGN_AND) != NULLREGION)
               {
               RECT RectTemp;

               GetRgnBox(hRgn, &RectTemp);
               ValidateRect(lpSS->lpBook->hWnd, &RectTemp);

               OffsetRgn(hRgn, xScroll, yScroll);
               InvalidateRgn(lpSS->lpBook->hWnd, hRgn, TRUE);
               }

            DeleteObject(hRgn2);
				}
			}

		DeleteObject(hRgn);
	   }

   if (xScroll)
      lpSS->bHPixelOffset = (abs(xScroll) + lpSS->bHPixelOffset) % 2;

   if (yScroll)
      lpSS->bVPixelOffset = (abs(yScroll) + lpSS->bVPixelOffset) % 2;

	/* RFW - 12/1/08 - Slow scrolling with highlighted headers
	{
	RECT RectUpd;

   ScrollDC(hDC, xScroll, yScroll, Rect, Rect, NULL, &RectUpd);
   SS_InvalidateRect(lpSS->lpBook, &RectUpd, TRUE);
	}
	*/

   hRgn = CreateRectRgn(0, 0, 0, 0);
   ScrollDC(hDC, xScroll, yScroll, Rect, Rect, hRgn, NULL);

	if (hRgn)
		{
		InvalidateRgn(lpSS->lpBook->hWnd, hRgn, TRUE);
		DeleteObject(hRgn);
		}
   }

SS_ReleaseDC(lpSS->lpBook, hDC);
}


void SS_ScrollNextCell(LPSPREADSHEET lpSS)

{
SSx_ScrollNextCell(lpSS, FALSE, FALSE);
}


BOOL SSx_ScrollNextCell(LPSPREADSHEET lpSS, BOOL fIsNext, BOOL fNextRow)
{
SS_CELLTYPE   CellType;
SS_COORD      ColLeftOld;
SS_COORD      ColAtOld;
SS_COORD      ColAtTemp;
SS_COORD      RowAtOld;
SS_COORD      RowAtTemp;
SS_COORD      DataColCnt;
SS_COORD      DataRowCnt;
BOOL          Found = FALSE;
BOOL          fRet = TRUE;
BOOL          fRedraw;

ColLeftOld = lpSS->Col.UL;
ColAtOld = lpSS->Col.CurAt;
ColAtTemp = lpSS->Col.CurAt;
RowAtOld = lpSS->Row.CurAt;
RowAtTemp = lpSS->Row.CurAt;

if (!fIsNext)
   {
   if (!SS_CellEditModeOff(lpSS, 0))
      return (FALSE);
   }

DataColCnt = lpSS->Col.DataCnt;
DataRowCnt = lpSS->Row.DataCnt;

if (fIsNext)
   SS_UpdateRestrictCnt(lpSS, &DataColCnt, &DataRowCnt);

if (fNextRow)
   {
   if (RowAtTemp == lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1)
      RowAtTemp = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
   else
      RowAtTemp++;

   ColAtTemp = lpSS->Col.HeaderCnt - 1;
   }

while (!Found && RowAtTemp < SS_GetRowCnt(lpSS) &&
       (!lpSS->RestrictRows ||
       (lpSS->RestrictRows && RowAtTemp <= DataRowCnt)))
   {
   if (SS_GetRowHeightInPixels(lpSS, RowAtTemp) > 0)
      {
      while (!Found && ColAtTemp < SS_GetColCnt(lpSS) - 1 &&
             (!lpSS->RestrictCols ||
              (lpSS->RestrictCols &&
               ColAtTemp <= DataColCnt)))
         {
         if (ColAtTemp == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1 &&
             RowAtTemp == RowAtOld)
            ColAtTemp = max(lpSS->Col.UL, lpSS->Col.HeaderCnt +
                            lpSS->Col.Frozen);
         else
            ColAtTemp++;

         SS_RetrieveCellType(lpSS, &CellType, NULL, ColAtTemp,
                             RowAtTemp);

         if (SS_GetColWidthInPixels(lpSS, ColAtTemp) > 0 &&
             (!lpSS->lpBook->fEditModePermanent ||
              (CellType.Type != SS_TYPE_STATICTEXT &&
               CellType.Type != SS_TYPE_OWNERDRAW &&
               CellType.Type != SS_TYPE_PICTURE &&
               !SS_GetLock(lpSS, ColAtTemp, RowAtTemp, FALSE))))
            Found = TRUE;
         }
      }

   if (!Found)
      {
      if (RowAtTemp == lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1)
         RowAtTemp = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
      else
         RowAtTemp++;

      ColAtTemp = lpSS->Col.HeaderCnt - 1;
      }
   }

if (fIsNext)
   fRet = !Found;

else if (Found)
   {
   SS_HighlightCell(lpSS, FALSE);
   lpSS->Col.CurAt = ColAtTemp;
   lpSS->Row.CurAt = RowAtTemp;

   if (SS_LeaveCell(lpSS, ColAtOld, ColLeftOld, RowAtOld, -1, 0))
      {
      if (lpSS->Col.CurAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
          lpSS->Col.UL != lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
         {
         lpSS->Col.UL = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
         fRedraw = lpSS->lpBook->Redraw;
         lpSS->lpBook->Redraw = FALSE;

         SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);

         lpSS->lpBook->Redraw = fRedraw;
         SSx_InvalidateColArea(lpSS);
         }

      else
         SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
      }
   }
else
   {
   SS_HighlightCell(lpSS, TRUE);
   if (!lpSS->lpBook->fSuspendExit)
      SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITNEXT, TRUE);
   }

return (fRet);
}


void SS_ScrollPrevCell(lpSS)

LPSPREADSHEET lpSS;
{
SSx_ScrollPrevCell(lpSS, FALSE);
}


BOOL SSx_ScrollPrevCell(lpSS, fIsNext)

LPSPREADSHEET lpSS;
BOOL          fIsNext;
{
SS_CELLTYPE   CellType;
SS_COORD      ColAtOld;
SS_COORD      ColAtTemp;
SS_COORD      RowAtOld;
SS_COORD      RowAtTemp;
BOOL          Found = FALSE;
BOOL          fRet = TRUE;

ColAtOld = lpSS->Col.CurAt;
ColAtTemp = lpSS->Col.CurAt;
RowAtOld = lpSS->Row.CurAt;
RowAtTemp = lpSS->Row.CurAt;

// RFW - 7/1/04 - 14575
if (!fIsNext)
   {
   if (!SS_CellEditModeOff(lpSS, 0))
      return (FALSE);
   }

while (!Found && RowAtTemp >= lpSS->Row.HeaderCnt)
   {
   /*
   if (SS_GetRowHeightInPixels(lpSS, RowAtTemp) > 0 &&
       (!lpSS->lpBook->fEditModePermanent ||
       !SS_GetLock(lpSS, SS_ALLCOLS, RowAtTemp, FALSE)))
   */
   if (SS_GetRowHeightInPixels(lpSS, RowAtTemp) > 0)
      {
      while (!Found && ColAtTemp > lpSS->Col.HeaderCnt)
         {
         ColAtTemp--;

         SS_RetrieveCellType(lpSS, &CellType, NULL, ColAtTemp,
                             RowAtTemp);

         if (SS_GetColWidthInPixels(lpSS, ColAtTemp) > 0 &&
             (!lpSS->lpBook->fEditModePermanent ||
              (CellType.Type != SS_TYPE_STATICTEXT &&
               CellType.Type != SS_TYPE_OWNERDRAW &&
               CellType.Type != SS_TYPE_PICTURE &&
               !SS_GetLock(lpSS, ColAtTemp, RowAtTemp, FALSE))))
         Found = TRUE;
         }
      }

   if (!Found)
      {
      RowAtTemp--;
      ColAtTemp = SS_GetColCnt(lpSS);
      }
   }

if (fIsNext)
   fRet = !Found;

else if (Found)
   {
   if (!SS_CellEditModeOff(lpSS, 0))
      return (FALSE);

   SS_HighlightCell(lpSS, FALSE);
   lpSS->Col.CurAt = ColAtTemp;
   lpSS->Row.CurAt = RowAtTemp;

   if (SS_LeaveCell(lpSS, ColAtOld, -1, RowAtOld, -1, 0))
      SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
   }
else
   {
	// RFW - 7/1/04 - 14575
   SS_HighlightCell(lpSS, TRUE);

   if (!lpSS->lpBook->fSuspendExit)
      SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_EXITPREV, TRUE);
   }

return (fRet);
}


SS_COORD SS_GetLastPageTopRow(LPSPREADSHEET lpSS)
{
return (SSx_GetLastPageTopRow(lpSS, SS_GetRowCnt(lpSS)));
}


SS_COORD SSx_GetLastPageTopRow(LPSPREADSHEET lpSS, SS_COORD MaxRows)
{
RECT     Rect;
SS_COORD RowAt;
SS_COORD RowAtTemp;
short    dHeight;
short    y;

SS_GetClientRect(lpSS->lpBook, &Rect);

y = SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);

RowAt = MaxRows - 1;

for (RowAtTemp = RowAt;
     RowAtTemp >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
     RowAtTemp--)
   {
   dHeight = SS_GetRowHeightInPixels(lpSS, RowAtTemp);
   y += dHeight;

   if (y > Rect.bottom - Rect.top)
      break;

   if (dHeight > 0)
      RowAt = RowAtTemp;
   }

return (min(MaxRows - 1, RowAt));
}


SS_COORD SS_GetLastPageLeftCol(lpSS)

LPSPREADSHEET lpSS;
{
RECT          Rect;
SS_COORD      MaxCols;
SS_COORD      ColAt;
SS_COORD      ColAtTemp;
short         dWidth;
short         x;

SS_GetClientRect(lpSS->lpBook, &Rect);

x = SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);

MaxCols = SS_GetColCnt(lpSS);
ColAt = MaxCols - 1;

for (ColAtTemp = ColAt;
     ColAtTemp >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
     ColAtTemp--)
   {
   dWidth = SS_GetColWidthInPixels(lpSS, ColAtTemp);
   x += dWidth;

   if (x > Rect.right - Rect.left)
      break;

   if (dWidth > 0)
      ColAt = ColAtTemp;
   }

return (min(MaxCols - 1, ColAt));
}


#ifndef SS_NOBUTTONDRAWMODE

void SS_InvalidateRowModeButtons(lpSS, ColOld, ColLeftOld, RowOld, RowTopOld)

LPSPREADSHEET lpSS;
SS_COORD      ColOld;
SS_COORD      ColLeftOld;
SS_COORD      RowOld;
SS_COORD      RowTopOld;
{
if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (ColOld == -1)
   ColOld = lpSS->Col.CurAt;

if (ColLeftOld == -1)
   ColLeftOld = lpSS->Col.UL;

if (RowOld == -1)
   RowOld = lpSS->Row.CurAt;

if (RowTopOld == -1)
   RowTopOld = lpSS->Row.UL;

#ifndef SS_NOBUTTONDRAWMODE
if (RowOld != lpSS->Row.CurAt)
   {
   if (lpSS->Col.Frozen)
      SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, 1, RowOld,
                            lpSS->Col.Frozen, RowOld);

   SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, ColLeftOld,
             RowOld, lpSS->Col.LR, RowOld);
   }
#endif
}


void SS_InvalidateButtons(lpSS, ColOld, ColLeftOld, RowOld, RowTopOld)

LPSPREADSHEET lpSS;
SS_COORD      ColOld;
SS_COORD      ColLeftOld;
SS_COORD      RowOld;
SS_COORD      RowTopOld;
{
if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

if (!SS_IsActiveSheet(lpSS))
	return;

if (ColOld == -1)
   ColOld = lpSS->Col.CurAt;

if (ColLeftOld == -1)
   ColLeftOld = lpSS->Col.UL;

if (RowOld == -1)
   RowOld = lpSS->Row.CurAt;

if (RowTopOld == -1)
   RowTopOld = lpSS->Row.UL;

if (lpSS->lpBook->wButtonDrawMode & SS_BDM_CURRENTCELL)
   {
   SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, ColOld,
             RowOld, ColOld, RowOld);
   SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld,
             lpSS->Col.CurAt, lpSS->Row.CurAt,
             lpSS->Col.CurAt, lpSS->Row.CurAt);
   }

if (lpSS->lpBook->wButtonDrawMode & SS_BDM_CURRENTCOL &&
    ColOld != lpSS->Col.CurAt)
   {
   if (lpSS->Row.Frozen)
      {
      SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, ColOld, 1,
                            ColOld, lpSS->Row.Frozen);
      SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, lpSS->Col.CurAt, 1,
                            lpSS->Col.CurAt, lpSS->Row.Frozen);
      }

   SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, ColOld,
                         RowTopOld, ColOld, lpSS->Row.LR);
   SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld,
                         lpSS->Col.CurAt, lpSS->Row.UL,
                         lpSS->Col.CurAt, lpSS->Row.LR);
   }

if (lpSS->lpBook->wButtonDrawMode & SS_BDM_CURRENTROW &&
    RowOld != lpSS->Row.CurAt)
   {
   if (lpSS->Col.Frozen)
      {
      SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, 1, RowOld,
                            lpSS->Col.Frozen, RowOld);
      SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, 1, lpSS->Row.CurAt,
                            lpSS->Col.Frozen, lpSS->Row.CurAt);
      }

   SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld, ColLeftOld,
                         RowOld, lpSS->Col.LR, RowOld);
   SSx_InvalidateButtons(lpSS, ColLeftOld, RowTopOld,
                         lpSS->Col.UL, lpSS->Row.CurAt,
                         lpSS->Col.LR, lpSS->Row.CurAt);
   }
}


void SSx_InvalidateButtons(lpSS, ColLeft, RowTop, Col1, Row1, Col2, Row2)

LPSPREADSHEET lpSS;
SS_COORD      ColLeft;
SS_COORD      RowTop;
SS_COORD      Col1;
SS_COORD      Row1;
SS_COORD      Col2;
SS_COORD      Row2;
{
SS_CELLTYPE   CellType;
RECT          Rect;
int           x, y, cx, cy;
#ifdef SS_V40
SS_COORD      lSpanCols, lSpanRows;
#endif
SS_COORD      i, j;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

for (i = Col1; i <= Col2; i++)
   {
   for (j = Row1; j <= Row2; j++)
      {
      SS_RetrieveCellType(lpSS, &CellType, NULL, i, j);

      if ((CellType.Type == SS_TYPE_BUTTON &&
           (lpSS->wOpMode == SS_OPMODE_ROWMODE ||
            !(lpSS->lpBook->wButtonDrawMode & SS_BDM_ALWAYSBUTTON))) ||
          (CellType.Type == SS_TYPE_COMBOBOX &&
           (lpSS->wOpMode == SS_OPMODE_ROWMODE ||
            !(lpSS->lpBook->wButtonDrawMode & SS_BDM_ALWAYSCOMBO))))
         {
#ifdef SS_V40
			if (SS_SPAN_ANCHOR == SS_GetCellSpan(lpSS, i, j, NULL, NULL, &lSpanCols, &lSpanRows))
				SS_GetCellRangeCoord(lpSS, ColLeft, RowTop, i, j, i + lSpanCols - 1,
                                 j + lSpanRows - 1, NULL, &x, &y, &cx, &cy);
			else
#endif // SS_V40
				SS_GetCellCoord(lpSS, ColLeft, RowTop, i, j, &x, &y, &cx, &cy);

         SetRect(&Rect, x, y, x + cx + 1, y + cy + 1);
         SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
         }
      }
   }
}


#if 0 // RFW - 8/2/04 - 14842
void SSx_InvalidateButtons(lpSS, ColLeft, RowTop, Col1, Row1, Col2, Row2)

LPSPREADSHEET lpSS;
SS_COORD      ColLeft;
SS_COORD      RowTop;
SS_COORD      Col1;
SS_COORD      Row1;
SS_COORD      Col2;
SS_COORD      Row2;
{
SS_CELLTYPE   CellType;
RECT          Rect;
short         x;
short         y;
short         cx;
short         cy;
SS_COORD      i;
SS_COORD      j;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
   lpSS->lpBook->fInvalidated = TRUE;
   return;
   }

x = SS_GetCellPosX(lpSS, ColLeft, Col1);

for (i = Col1; i <= Col2; i++)
   {
   cx = SS_GetColWidthInPixels(lpSS, i);
   y = SS_GetCellPosY(lpSS, RowTop, Row1);

   for (j = Row1; j <= Row2; j++)
      {
      cy = SS_GetRowHeightInPixels(lpSS, j);

      SS_RetrieveCellType(lpSS, &CellType, NULL, i, j);

      if ((CellType.Type == SS_TYPE_BUTTON &&
           (lpSS->wOpMode == SS_OPMODE_ROWMODE ||
            !(lpSS->lpBook->wButtonDrawMode & SS_BDM_ALWAYSBUTTON))) ||
          (CellType.Type == SS_TYPE_COMBOBOX &&
           (lpSS->wOpMode == SS_OPMODE_ROWMODE ||
            !(lpSS->lpBook->wButtonDrawMode & SS_BDM_ALWAYSCOMBO))))
         {
         SetRect(&Rect, x, y, x + cx + 1, y + cy + 1);
         SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
         }

      y += cy;
      }

   x += cx;
   }
}
#endif // 0

#endif


BOOL DLLENTRY SSGetFirstValidCell(HWND hWnd, LPSS_COORD lpCol,
                                  LPSS_COORD lpRow)
{
LPSPREADSHEET lpSS;
BOOL          bFound;

lpSS = SS_SheetLock(hWnd);
bFound = SS_GetFirstValidCell(lpSS, lpCol, lpRow);
SS_SheetUnlock(hWnd);
return bFound;
}


BOOL SS_GetFirstValidCell(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                          LPSS_COORD lpRow)
{
SS_CELLTYPE   CellType;
SS_COORD      dRowAtTemp = lpSS->Row.HeaderCnt;
SS_COORD      dColAtTemp = lpSS->Col.HeaderCnt - 1;
BOOL          bFound = FALSE;

while (!bFound && dRowAtTemp < SS_GetRowCnt(lpSS) &&
       (!lpSS->RestrictRows ||
       (lpSS->RestrictRows && dRowAtTemp <= lpSS->Row.DataCnt)))
   {
   if (SS_GetRowHeightInPixels(lpSS, dRowAtTemp) > 0)
      {
      while (!bFound && dColAtTemp < SS_GetColCnt(lpSS) - 1 &&
             (!lpSS->RestrictCols || (lpSS->RestrictCols &&
              dColAtTemp <= lpSS->Col.DataCnt)))
			{
			dColAtTemp++;

			SS_RetrieveCellType(lpSS, &CellType, NULL, dColAtTemp,
						  dRowAtTemp);

			if (SS_GetColWidthInPixels(lpSS, dColAtTemp) > 0 &&
				 (!lpSS->lpBook->fEditModePermanent ||
				 (CellType.Type != SS_TYPE_STATICTEXT &&
				 CellType.Type != SS_TYPE_OWNERDRAW &&
				 CellType.Type != SS_TYPE_PICTURE &&
				 !SS_GetLock(lpSS, dColAtTemp, dRowAtTemp, FALSE))))
				bFound = TRUE;
			}
      }

   if (!bFound)
      {
      dRowAtTemp++;
      dColAtTemp = lpSS->Col.HeaderCnt - 1;
      }
   }

if (bFound)
   {
   if (lpCol)
      *lpCol = dColAtTemp;
   if (lpCol)
      *lpRow = dRowAtTemp;
   }

return bFound;
}


BOOL DLLENTRY SSGetLastValidCell(HWND hWnd, LPSS_COORD lpCol,
                                 LPSS_COORD lpRow)
{
LPSPREADSHEET lpSS;
BOOL          bFound;

lpSS = SS_SheetLock(hWnd);
bFound = SS_GetLastValidCell(lpSS, lpCol, lpRow);
SS_SheetUnlock(hWnd);
return bFound;
}


BOOL SS_GetLastValidCell(LPSPREADSHEET lpSS, LPSS_COORD lpCol,
                         LPSS_COORD lpRow)
{
SS_CELLTYPE   CellType;
SS_COORD      dColAtTemp;
SS_COORD      dRowAtTemp;
BOOL          bFound = FALSE;

dColAtTemp = SS_GetColCnt(lpSS);
dRowAtTemp = SS_GetRowCnt(lpSS) - 1;

while (!bFound && dRowAtTemp >= lpSS->Row.HeaderCnt)
   {
   if (SS_GetRowHeightInPixels(lpSS, dRowAtTemp) > 0)
      {
      while (!bFound && dColAtTemp > lpSS->Col.HeaderCnt)
     {
     dColAtTemp--;

     SS_RetrieveCellType(lpSS, &CellType, NULL, dColAtTemp, dRowAtTemp);

     if (SS_GetColWidthInPixels(lpSS, dColAtTemp) > 0 &&
         (!lpSS->lpBook->fEditModePermanent ||
         (CellType.Type != SS_TYPE_STATICTEXT &&
         CellType.Type != SS_TYPE_OWNERDRAW &&
         CellType.Type != SS_TYPE_PICTURE &&
         !SS_GetLock(lpSS, dColAtTemp, dRowAtTemp, FALSE))))
        bFound = TRUE;
     }
      }

   if (!bFound)
      {
      dRowAtTemp--;
      dColAtTemp = SS_GetColCnt(lpSS);
      }
   }

if (bFound)
   {
   if (lpCol)
      *lpCol = dColAtTemp;
   if (lpRow)
      *lpRow = dRowAtTemp;
   }

return bFound;
}


BOOL SS_GetFirstValidCol(LPSPREADSHEET lpSS, SS_COORD Row, LPSS_COORD lpCol)
{
SS_CELLTYPE   CellType;
SS_COORD      dColAtTemp = lpSS->Col.HeaderCnt - 1;
BOOL          fFound = FALSE;

while (!fFound && dColAtTemp < SS_GetColCnt(lpSS) - 1 &&
       (!lpSS->RestrictCols || dColAtTemp <= lpSS->Col.DataCnt))
   {
  if (dColAtTemp == lpSS->Col.Frozen)
      dColAtTemp = max(lpSS->Col.UL, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);
   else
      dColAtTemp++;

   SS_RetrieveCellType(lpSS, &CellType, NULL, dColAtTemp, Row);

   if (SS_GetColWidthInPixels(lpSS, dColAtTemp) > 0 &&
       (!lpSS->lpBook->fEditModePermanent ||
       (CellType.Type != SS_TYPE_STATICTEXT &&
       CellType.Type != SS_TYPE_OWNERDRAW &&
       CellType.Type != SS_TYPE_PICTURE &&
       !SS_GetLock(lpSS, dColAtTemp, Row, FALSE))))
      fFound = TRUE;
   }

if (fFound)
   *lpCol = dColAtTemp;

return (fFound);
}


BOOL SS_GetFirstValidRow(LPSPREADSHEET lpSS, SS_COORD Col, LPSS_COORD lpRow)
{
SS_CELLTYPE   CellType;
SS_COORD      dRowAtTemp = lpSS->Row.HeaderCnt - 1;
BOOL          fFound = FALSE;

while (!fFound && dRowAtTemp < SS_GetRowCnt(lpSS) - 1 &&
       (!lpSS->RestrictRows || dRowAtTemp <= lpSS->Row.DataCnt))
   {
  if (dRowAtTemp == lpSS->Row.Frozen)
      dRowAtTemp = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
   else
      dRowAtTemp++;

   SS_RetrieveCellType(lpSS, &CellType, NULL, Col, dRowAtTemp);

   if (SS_GetRowHeightInPixels(lpSS, dRowAtTemp) > 0 &&
       (!lpSS->lpBook->fEditModePermanent ||
       (CellType.Type != SS_TYPE_STATICTEXT &&
       CellType.Type != SS_TYPE_OWNERDRAW &&
       CellType.Type != SS_TYPE_PICTURE &&
       !SS_GetLock(lpSS, Col, dRowAtTemp, FALSE))))
      fFound = TRUE;
   }

if (fFound)
   *lpRow = dRowAtTemp;

return (fFound);
}


void SS_UpdateRestrictCnt(LPSPREADSHEET lpSS,
                                LPSS_COORD lpDataColCnt,
                                LPSS_COORD lpDataRowCnt)
{
SS_CELLTYPE   CellTypeTemp;
LPSS_CELLTYPE lpCellType;
LPSS_CELL     lpCell;
HWND          hWndCtrl = 0;

if (lpSS->lpBook->EditModeOn && ((lpSS->RestrictRows &&
    lpSS->Row.CurAt >= lpSS->Row.DataCnt) ||
    (lpSS->RestrictCols && lpSS->Col.CurAt >= lpSS->Col.DataCnt)))
   {
   lpCell = SS_LockCellItem(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt);
   lpCellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, lpCell,
                                    lpSS->Col.CurAt, lpSS->Row.CurAt);
   if (lpCell)
      SS_UnlockCellItem(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt);

   if (lpCellType->ControlID)
      hWndCtrl = SS_GetControlhWnd(lpSS, lpCellType->ControlID);

   if (hWndCtrl &&
       (lpCellType->Type == SS_TYPE_EDIT ||
        lpCellType->Type == SS_TYPE_DATE ||
        lpCellType->Type == SS_TYPE_TIME ||
        lpCellType->Type == SS_TYPE_INTEGER ||
        lpCellType->Type == SS_TYPE_FLOAT ||
#ifdef SS_V40
        lpCellType->Type == SS_TYPE_CURRENCY ||
        lpCellType->Type == SS_TYPE_NUMBER ||
        lpCellType->Type == SS_TYPE_PERCENT ||
#endif // SS_V40
#ifdef SS_V70
        lpCellType->Type == SS_TYPE_SCIENTIFIC ||
#endif // SS_V70
        lpCellType->Type == SS_TYPE_PIC))
      if (SendMessage(hWndCtrl, EM_GETMODIFY, 0, 0L))
         {
         if (lpSS->RestrictCols && lpSS->Col.CurAt >= lpSS->Col.DataCnt)
            (*lpDataColCnt)++;

         if (lpSS->RestrictRows && lpSS->Row.CurAt >= lpSS->Row.DataCnt)
            (*lpDataRowCnt)++;
         }
   }
}


#ifdef SS_UTP
void SS_CalcHScrollArrowArea(HWND hWnd, LPSPREADSHEET lpSS)
{
SS_COORD RightVisCol;
RECT     Rect;
BOOL     fRightScrollArrowVis = FALSE;
BOOL     fLeftScrollArrowVis = FALSE;
BOOL     fEditModeOn;
short    dScrollArrowWidthOld;
short    x;

if (!hWnd) // handle an OCX control without a HWND
   return;

if (lpSS->fUseScrollArrows)
   {
   fEditModeOn = lpSS->EditModeOn;
   dScrollArrowWidthOld = lpSS->dScrollArrowWidth;
   lpSS->dScrollArrowWidth = 0;

   RightVisCol = SS_GetRightVisCell(lpSS, lpSS->Col.UL);
   if (RightVisCol < SS_GetColCnt(lpSS) - 1)
      fRightScrollArrowVis = TRUE;

   lpSS->dScrollArrowWidth = SS_SCROLLARROW_WIDTH;

   if (lpSS->Col.UL > lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      {
      SS_COORD ColTemp;

      for (ColTemp = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
           ColTemp < lpSS->Col.UL && !fLeftScrollArrowVis; ColTemp++)
         if (SS_GetColWidthInPixels(lpSS, ColTemp))
            fLeftScrollArrowVis = TRUE;
      }

   if (fRightScrollArrowVis != (BOOL)lpSS->fRightScrollArrowVis)
      {
      SS_CellEditModeOff(lpSS, 0);
      lpSS->fRightScrollArrowVis = fRightScrollArrowVis;
      GetClientRect(hWnd, &Rect);
      Rect.left = Rect.right - max(dScrollArrowWidthOld,
                   lpSS->dScrollArrowWidth);
      SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
      }

   if (fLeftScrollArrowVis != (BOOL)lpSS->fLeftScrollArrowVis)
      {
      lpSS->fLeftScrollArrowVis = fLeftScrollArrowVis;
      SS_CellEditModeOff(lpSS, 0);

      if (lpSS->fLeftScrollArrowVis)
         {
         SSSetColWidthInPixels(hWnd, lpSS->Col.HeaderCnt +
                       lpSS->Col.Frozen - 1, SS_SCROLLARROW_WIDTH);

         if (!SS_IsBlockSelected(lpSS))
            SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
         }
      else
         SSShowCol(hWnd, lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1, FALSE);
      }

   if (lpSS->fRightScrollArrowVis)
      {
      RightVisCol = max(SS_GetRightVisCell(lpSS, lpSS->Col.UL), lpSS->Col.UL);
      GetClientRect(hWnd, &Rect);
      x = max(Rect.right - (SS_GetCellPosX(lpSS, lpSS->Col.UL, RightVisCol) +
          SS_GetColWidthInPixels(lpSS, RightVisCol)) - 1, SS_SCROLLARROW_WIDTH);

      if (dScrollArrowWidthOld != x)
         {
         SS_CellEditModeOff(lpSS, 0);
         GetClientRect(hWnd, &Rect);
         lpSS->dScrollArrowWidth = x;
         Rect.left = Rect.right - max(lpSS->dScrollArrowWidth,
                          dScrollArrowWidthOld);
         SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
         }
      else
     lpSS->dScrollArrowWidth = dScrollArrowWidthOld;
      }

   if (fEditModeOn)
      SS_CellEditModeOn(lpSS, 0, 0, 0L);
   }
}


void SS_CalcVScrollArrowArea(HWND hWnd, LPSPREADSHEET lpSS)
{
SS_COORD BottomVisRow;
RECT     Rect;
BOOL     fBottomScrollArrowVis = FALSE;
BOOL     fTopScrollArrowVis = FALSE;
short    dScrollArrowHeightOld;
short    y;

if (!hWnd)  // handle an OCX control without a HWND
   return;

if (lpSS->fUseScrollArrows)
   {
   dScrollArrowHeightOld = lpSS->dScrollArrowHeight;
   lpSS->dScrollArrowHeight = 0;

   if (SS_GetBottomVisCell(lpSS, lpSS->Row.UL) < SS_GetRowCnt(lpSS) - 1)
      fBottomScrollArrowVis = TRUE;

   lpSS->dScrollArrowHeight = SS_SCROLLARROW_HEIGHT;

   if (lpSS->Row.UL > lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      {
      SS_COORD RowTemp;

      for (RowTemp = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
           RowTemp < lpSS->Row.UL && !fTopScrollArrowVis; RowTemp++)
         if (SS_GetRowHeightInPixels(lpSS, RowTemp))
            fTopScrollArrowVis = TRUE;
      }

   if (fBottomScrollArrowVis != (BOOL)lpSS->fBottomScrollArrowVis)
      {
      lpSS->fBottomScrollArrowVis = fBottomScrollArrowVis;
      GetClientRect(hWnd, &Rect);
      lpSS->dScrollArrowHeight = SS_SCROLLARROW_HEIGHT;
      Rect.top = Rect.bottom - max(dScrollArrowHeightOld,
                   lpSS->dScrollArrowHeight);
      SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
      //SS_UpdateWindow(lpSS->lpBook);
      }

   if (fTopScrollArrowVis != (BOOL)lpSS->fTopScrollArrowVis)
      {
      lpSS->fTopScrollArrowVis = fTopScrollArrowVis;

      {
      SUPERBTNCOLOR SuperBtnColor;
      SS_CELLTYPE   CellType;
      COLORREF      Color;
      HICON         hIconUp = 0;
      HICON         hIconDown = 0;
      WORD          wIconUpType = 0;
      WORD          wIconDownType = 0;

      if (lpSS->fTopScrollArrowVis)
         {
         SS_RetrieveCellType(lpSS, &CellType, NULL, 0, 0);
         _fmemcpy(&lpSS->CellType00, &CellType, sizeof(SS_CELLTYPE));

         SSGetColor(hWnd, 0, 0, &Color, NULL);

         SuperBtnColor.Color = Color;
         SuperBtnColor.ColorBorder = Color;
         SuperBtnColor.ColorShadow = Color;
         SuperBtnColor.ColorHighlight = Color;
         SuperBtnColor.ColorText = Color;

         if (lpSS->hBitmapScrollArrowTopUp)
            {
            hIconUp = lpSS->hBitmapScrollArrowTopUp;
            wIconUpType = lpSS->uScrollArrowTopUpType | BT_HANDLE;
            }

         if (lpSS->hBitmapScrollArrowTopDown)
            {
            hIconDown = lpSS->hBitmapScrollArrowTopDown;
            wIconDownType = lpSS->uScrollArrowTopDownType | BT_HANDLE;
            }

         SSSetTypeButton(hWnd, &CellType, 0L, NULL,
                 (LPTSTR)(hIconUp ? &hIconUp : NULL), wIconUpType,
                 (LPTSTR)(hIconDown ? &hIconDown : NULL), wIconDownType,
                 SUPERBTN_TYPE_NORMAL, 0, &SuperBtnColor);

         SSSetCellType(hWnd, 0, 0, &CellType);

         if (!SS_IsBlockSelected(lpSS))
            SS_ShowActiveCell(lpSS, SS_SHOW_NEAREST);
         }
      else
         {
         /*
         SSSetTypeStaticText(hWnd, &CellType, 0);
         SSSetCellType(hWnd, 0, 0, &CellType);
         */
         SSSetCellType(hWnd, 0, 0, &lpSS->CellType00);
         }
      }
      }

   if (lpSS->fBottomScrollArrowVis)
      {
      BottomVisRow = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);
      GetClientRect(hWnd, &Rect);
      y = Rect.bottom - (SS_GetCellPosY(lpSS, lpSS->Row.UL, BottomVisRow) +
             SS_GetRowHeightInPixels(lpSS, BottomVisRow)) - 1;

      if (dScrollArrowHeightOld != y)
         {
         GetClientRect(hWnd, &Rect);
         lpSS->dScrollArrowHeight = y;
         Rect.top = Rect.bottom - max(lpSS->dScrollArrowHeight,
                          dScrollArrowHeightOld);
         SS_InvalidateRect(lpSS->lpBook, &Rect, TRUE);
         }
      else
         lpSS->dScrollArrowHeight = dScrollArrowHeightOld;
      }
   }
}
#endif


void SS_TopRowChange(LPSPREADSHEET lpSS)
{
#ifdef SS_UTP
SS_CalcVScrollArrowArea(lpSS->lpBook->hWnd, lpSS);
#endif
SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_TOPROWCHANGE, FALSE);
}


void SS_LeftColChange(LPSPREADSHEET lpSS)
{
#ifdef SS_UTP
SS_CalcHScrollArrowArea(lpSS->lpBook->hWnd, lpSS);
#endif
SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_LEFTCOLCHANGE, FALSE);
}


void SS_TopLeftChange(LPSPREADSHEET lpSS)
{
#ifdef SS_UTP
SS_CalcHScrollArrowArea(lpSS->lpBook->hWnd, lpSS);
SS_CalcVScrollArrowArea(lpSS->lpBook->hWnd, lpSS);
#endif
SS_SendMsgCommand(lpSS->lpBook, lpSS, SSN_TOPLEFTCHANGE, FALSE);
}


short SS_GetCellPosYInRect(LPSPREADSHEET lpSS, SS_COORD RowTop, SS_COORD Row,
               LPRECT lpRect)
{
SS_COORD i;
short    y;

for (i = 0, y = -1; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen && i < Row; i++)
   y += SS_GetRowHeightInPixels(lpSS, i);

for (i = max(i, RowTop); i < Row && y < lpRect->bottom; i++)
   y += SS_GetRowHeightInPixels(lpSS, i);

return (y);
}


short SS_GetCellPosXInRect(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD Col,
               LPRECT lpRect)
{
SS_COORD i;
short    x;

for (i = 0, x = -1; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen && i < Col; i++)
   x += SS_GetColWidthInPixels(lpSS, i);

for (i = max(i, ColLeft); i < Col && x < lpRect->right; i++)
   x += SS_GetColWidthInPixels(lpSS, i);

return (x);
}


void SS_ResetCellSizeList(LPSS_ROWCOL lpRC)
{
if (lpRC->hCellSizeList)
   tbGlobalFree(lpRC->hCellSizeList);

lpRC->lCellSizeListAllocCnt = 0;
lpRC->lCellSizeListCnt = 0;
lpRC->hCellSizeList = 0;
}

#ifdef SS_V35
short SS_GetShowScrollTips(LPSS_BOOK lpBook)
{
  return lpBook->nShowScrollTips;
}

void SS_SetShowScrollTips(LPSS_BOOK lpBook, short nNewValue)
{
  if (nNewValue >=0 && nNewValue <=3)
  {  
    lpBook->nShowScrollTips = nNewValue;

#ifdef SS_OCX
    if (!(SS_IsDesignTime(lpBook)) && lpBook->hWnd)
#else
	  if (lpBook->hWnd)
#endif
    {
      FP_TT_INFO Info;
      WORD wItem = FP_TT_ITEM_SCROLLTIPSTATUS;
      
      if (nNewValue == 0)
        Info.fShowScrollTips = FALSE;
      else
        Info.fShowScrollTips = TRUE;
      fpTextTipSetInfo(lpBook->hWnd, wItem, &Info);
    }
  }
}

WORD DLLENTRY SSGetShowScrollTips(HWND hWnd)
{
  LPSS_BOOK lpBook = SS_BookLock(hWnd);
  short     nShowScrollTips = SS_GetShowScrollTips(lpBook);

  SS_BookUnlock(hWnd);

  return nShowScrollTips;
}

void DLLENTRY SSSetShowScrollTips(HWND hWnd, WORD wScrollTips)
{
  LPSS_BOOK lpBook = SS_BookLock(hWnd);

  SS_SetShowScrollTips(lpBook, wScrollTips);

  SS_BookUnlock(hWnd);
}

#endif

