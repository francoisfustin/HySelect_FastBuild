/*********************************************************
* SS_BOOL.C
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

//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_user.h"
#include "ss_win.h"
#ifdef SS_V80
#include "ss_scbar.h"
#endif

BOOL DLLENTRY SSSetBool(HWND hWnd, short nIndex, BOOL fNewVal)
{
LPSS_BOOK     lpBook = SS_BookLock(hWnd);
LPSPREADSHEET lpSS = SS_BookLockSheet(lpBook);
BOOL          fPrevVal;

fPrevVal = SS_SetBool(lpBook, lpSS, nIndex, fNewVal);

SS_BookUnlockSheet(lpBook);
SS_BookUnlock(hWnd);

return (fPrevVal);
}


BOOL SS_SetBool(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, short nIndex, BOOL fNewVal)
{
LPSPREADSHEET lpSSActive = SS_BookLockActiveSheet(lpBook);
RECT          Rect;
BOOL          fPrevVal = 0;

switch (nIndex)
   {
   case SSB_PROTECT:
      fPrevVal = lpBook->DocumentProtected;
      lpBook->DocumentProtected = fNewVal;
      SS_InvalidateScrollArea(lpSSActive, SS_SCROLLAREA_ALL);
      break;

   case SSB_RESTRICTROWS:
      fPrevVal = lpSS->RestrictRows;
      lpSS->RestrictRows = fNewVal;
      break;

   case SSB_RESTRICTCOLS:
      fPrevVal = lpSS->RestrictCols;
      lpSS->RestrictCols = fNewVal;
      break;

   case SSB_SHOWCOLHEADERS:
		{
		SS_COORD Row;

		for (Row = 0; Row < lpSS->Row.HeaderCnt; Row++)
			{
#ifdef SS_V40
			SS_ShowRow(lpSS, Row, fNewVal);
			if (fNewVal)
				if (SS_GetRowHeightInPixels(lpSS, Row) == 0)
					SS_SetRowHeight(lpSS, Row, SS_HEIGHT_DEFAULT);
#else
			if (fNewVal)
				{
				if (SS_GetRowHeightInPixels(lpSS, Row) == 0)
					SS_SetRowHeight(lpSS, Row, SS_HEIGHT_DEFAULT);
				}

			else
				SS_SetRowHeight(lpSS, Row, 0.0);
#endif // SS_V40
			}
		}
      break;

   case SSB_SHOWROWHEADERS:
		{
		SS_COORD Col;

		for (Col = 0; Col < lpSS->Col.HeaderCnt; Col++)
			{
#ifdef SS_V40
			SS_ShowCol(lpSS, Col, fNewVal);
			if (fNewVal)
				if (SS_GetColWidthInPixels(lpSS, Col) == 0)
					SS_SetColWidth(lpSS, Col, -2.0);
#else
			if (fNewVal)
				{
				if (SS_GetColWidthInPixels(lpSS, Col) == 0)
					SS_SetColWidth(lpSS, Col, -2.0);
				}
			else
				SS_SetColWidth(lpSS, Col, 0.0);
#endif // SS_V40
			}
		}
      break;

   case SSB_ALLOWUSERRESIZE:
      if (lpSS->wUserResize & (SS_USERRESIZE_COL | SS_USERRESIZE_ROW))
         fPrevVal = TRUE;
      else
         fPrevVal = FALSE;

      if (fPrevVal != fNewVal)
         {
         if (fNewVal)
            lpSS->wUserResize |= SS_USERRESIZE_COL | SS_USERRESIZE_ROW;
         else
            lpSS->wUserResize &= ~(SS_USERRESIZE_COL | SS_USERRESIZE_ROW);
         }

      break;

   case SSB_ALLOWUSERSELBLOCK:
      if (!lpBook->fEditModePermanent)
         {
         if (lpBook->wSelBlockOption & (SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
             SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL))
            fPrevVal = TRUE;
         else
            fPrevVal = FALSE;

         if (fPrevVal != fNewVal)
            {
            if (fNewVal)
               lpBook->wSelBlockOption |= SS_SELBLOCK_COLS |
                                        SS_SELBLOCK_ROWS |
                                        SS_SELBLOCK_BLOCKS |
                                        SS_SELBLOCK_ALL;
            else
               lpBook->wSelBlockOption &= ~(SS_SELBLOCK_COLS |
                                          SS_SELBLOCK_ROWS |
                                          SS_SELBLOCK_BLOCKS |
                                          SS_SELBLOCK_ALL);

            SS_InvalidateScrollArea(lpSSActive, SS_SCROLLAREA_ALL);
            }

         }

      break;

   case SSB_AUTOCALC:
		fPrevVal = SS_BookSetAutoCalc(lpBook, fNewVal);
      break;

   case SSB_REDRAW:
      fPrevVal = lpBook->Redraw;
      if (fPrevVal != fNewVal)
         {
         lpBook->Redraw = fNewVal;
         if (fNewVal)
            {
            if (lpBook->fUpdateAutoSize)
               SS_AutoSize(lpBook, FALSE);
            else
               {
               SS_SetVScrollBar(lpSSActive);
               SS_SetHScrollBar(lpSSActive);
               }

            if( !lpBook->hWnd )  // handle an OCX control without an HWND ????
              lpBook->fInvalidated = TRUE;
            else
               if (IsWindowVisible(lpBook->hWnd))
                  {
                  if (GetUpdateRect(lpBook->hWnd, &Rect, FALSE))
                     SS_UpdateWindow(lpBook);
                  else if (fPrevVal)
                     SS_InvalidateScrollArea(lpSSActive, SS_SCROLLAREA_ALL);
                  }
            }
         }

      break;

   case SSB_GRIDLINES:
      if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
         fPrevVal = TRUE;
      else
         fPrevVal = FALSE;

      if (fPrevVal != fNewVal)
         {
         if (fNewVal)
            lpSS->wGridType |= SS_GRID_HORIZONTAL | SS_GRID_VERTICAL;
         else
            lpSS->wGridType &= ~(SS_GRID_HORIZONTAL | SS_GRID_VERTICAL);

         SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
         }

      break;

   case SSB_HORZSCROLLBAR:
      fPrevVal = lpBook->HorzScrollBar;
      if (fPrevVal != fNewVal)
         {
         lpBook->HorzScrollBar = fNewVal;
         if (fNewVal)
            SS_SetHScrollBar(lpSSActive);
         else if( lpBook->hWnd ) // handle an OCX control without a HWND ???
            SetScrollRange(lpBook->hWnd, SB_HORZ, 0, 0, TRUE);

         SS_AutoSize(lpBook, FALSE);
         }

      break;

   case SSB_VERTSCROLLBAR:
      fPrevVal = lpBook->VertScrollBar;
      if (fPrevVal != fNewVal)
         {
         lpBook->VertScrollBar = fNewVal;
         if (fNewVal)
            SS_SetVScrollBar(lpSSActive);
         else if( lpBook->hWnd ) // handle an OCX control without a HWND ???
            SetScrollRange(lpBook->hWnd, SB_VERT, 0, 0, TRUE);

         SS_AutoSize(lpBook, FALSE);
         }

      break;

   case SSB_ALLOWUSERFORMULAS:
      fPrevVal = lpBook->AllowUserFormulas;
      lpBook->AllowUserFormulas = fNewVal;
      break;

   case SSB_EDITMODEPERMANENT:
      fPrevVal = lpBook->fEditModePermanent;
      
      // Added !lpSS->wMessageBeingSent to fix a painting problem
      // when EDITMODEPERMANENT is called from the LEAVECELL message.

      if (fPrevVal != fNewVal && !lpBook->wMessageBeingSent)
         SS_HighlightCell(lpSSActive, FALSE);

      lpBook->fEditModePermanent = fNewVal;

      if (fPrevVal != fNewVal && !lpBook->wMessageBeingSent &&
          !lpBook->EditModeOn)
         SS_HighlightCell(lpSSActive, TRUE);

		/*
      if (fNewVal)
         {
         lpBook->wSelBlockOption = 0;
#ifdef SS_UTP
         if (lpBook->fAllowEditModePermSel)
            lpBook->wSelBlockOption = SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS;
#endif
         }
      else
			{
			if (lpSS->wOpMode == SS_OPMODE_ROWMODE || lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
			    lpSS->wOpMode == SS_OPMODE_MULTISEL || lpSS->wOpMode == SS_OPMODE_EXTSEL)
				lpBook->wSelBlockOption = 0;
			else
				lpBook->wSelBlockOption = SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
				                          SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL;
			}
		*/

      break;

   case SSB_AUTOSIZE:
      fPrevVal = lpBook->fAutoSize;
      lpBook->fAutoSize = fNewVal;

      if (fPrevVal != fNewVal)
         {
         SS_AutoSize(lpBook, FALSE);
         SS_InvalidateRect(lpBook, NULL, TRUE);
         SS_UpdateWindow(lpBook);
         }

      break;

   case SSB_SCROLLBAREXTMODE:
      fPrevVal = lpBook->fScrollBarExtMode;
      lpBook->fScrollBarExtMode = fNewVal;

      if (fPrevVal != fNewVal)
         {
         SS_AutoSize(lpBook, FALSE);
         SS_InvalidateRect(lpBook, NULL, TRUE);
         SS_UpdateWindow(lpBook);
         }

      break;

   case SSB_SCROLLBARSHOWMAX:
      fPrevVal = lpBook->fScrollBarShowMax;
      lpBook->fScrollBarShowMax = fNewVal;
      SS_SetVScrollBar(lpSSActive);
      SS_SetHScrollBar(lpSSActive);
      break;

   case SSB_SCROLLBARMAXALIGN:
      fPrevVal = lpBook->fScrollBarMaxAlign;
      lpBook->fScrollBarMaxAlign = fNewVal;
      SS_SetVScrollBar(lpSSActive);
      SS_SetHScrollBar(lpSSActive);
      break;

   case SSB_PROCESSTAB:
      fPrevVal = lpBook->fProcessTab;
      lpBook->fProcessTab = fNewVal;
      break;

   case SSB_ARROWSEXITEDITMODE:
      fPrevVal = lpBook->fArrowsExitEditMode;
      lpBook->fArrowsExitEditMode = fNewVal;
      break;

   case SSB_MOVEACTIVEONFOCUS:
      fPrevVal = lpBook->fMoveActiveOnFocus;
      lpBook->fMoveActiveOnFocus = fNewVal;
      break;

   case SSB_RETAINSELBLOCK:
      fPrevVal = lpBook->fRetainSelBlock;
      lpBook->fRetainSelBlock = fNewVal;

      {
      HWND hWndFocus = GetFocus();
      if (!hWndFocus || (hWndFocus != lpBook->hWnd &&
          GetParent(hWndFocus) != lpBook->hWnd))
         {
         // RFW - 6/12/01 - 9023
         if (!lpBook->EditModeTurningOn &&
             !lpBook->EditModeTurningOff &&
             !lpBook->fRetainSelBlock &&
             !SS_USESELBAR(lpSSActive))
            {
            lpBook->fSelBlockInvisible = TRUE;
            if (!lpBook->fEditModePermanent)
               SS_HighlightCell(lpSSActive, FALSE);
            }
         else
            lpBook->fSelBlockInvisible = FALSE;

         SS_InvalidateScrollArea(lpSSActive, SS_SCROLLAREA_ALL);
         }
      }
      break;

   case SSB_EDITMODEREPLACE:
      fPrevVal = lpBook->fEditModeReplace;
      lpBook->fEditModeReplace = fNewVal;
      break;

   case SSB_NOBORDER:
      fPrevVal = lpBook->fNoBorder;
      lpBook->fNoBorder = fNewVal;

      if (fPrevVal != fNewVal)
         SS_InvalidateScrollArea(lpSSActive, SS_SCROLLAREA_ALL);

      break;

   case SSB_NOBEEP:
      fPrevVal = lpBook->fNoBeep;
      lpBook->fNoBeep = fNewVal;
      break;

   case SSB_AUTOCLIPBOARD:
      fPrevVal = lpBook->fAutoClipboard;
      lpBook->fAutoClipboard = fNewVal;
      break;

   case SSB_ALLOWMULTIBLOCKS:
      fPrevVal = lpBook->fAllowMultipleSelBlocks;
      lpBook->fAllowMultipleSelBlocks = fNewVal;
      break;

#ifndef SS_NOOVERFLOW
   case SSB_ALLOWCELLOVERFLOW:
      fPrevVal = lpBook->fAllowCellOverflow;
      lpBook->fAllowCellOverflow = fNewVal;

      if (lpBook->fAllowCellOverflow && !fPrevVal)
      {
			LPSPREADSHEET lpSSTemp;
         LPSS_CELL     lpCell;
         SS_COORD      i = -1;
         SS_COORD      j = -1;
         short         nSheet;

#if SS_V80
			for (nSheet = 0; nSheet < lpBook->nSheetCnt; nSheet++)
			{
				lpSSTemp = SS_BookLockSheetIndex(lpBook, nSheet);
            i = SS_GetNextNonEmptyRow(lpSSTemp, -1);
            for( ; i != -1; i = SS_GetNextNonEmptyRow(lpSSTemp, i) )
            {
               j = SS_GetNextNonEmptyColumnInRow(lpSSTemp, i, j);
               for( ; j != -1; j = SS_GetNextNonEmptyColumnInRow(lpSSTemp, i, j) )
						if (lpCell = SS_LockCellItem(lpSSTemp, j, i))
							{
							if (!lpCell->Data.bOverflow)
								SS_CalcCellOverflow(lpSSTemp, lpCell, NULL, j, i);
							SS_UnlockCellItem(lpSSTemp, j, i);
							}
               j = -1;
            }   
         }
#else
			for (nSheet = 0; nSheet < lpBook->nSheetCnt; nSheet++)
				{
				lpSSTemp = SS_BookLockSheetIndex(lpBook, nSheet);
				for (i = lpSSTemp->Row.HeaderCnt; i < lpSSTemp->Row.DataCnt; i++)
					for (j = lpSSTemp->Col.HeaderCnt; j < lpSSTemp->Col.DataCnt; j++)
						if (lpCell = SS_LockCellItem(lpSSTemp, j, i))
							{
							if (!lpCell->Data.bOverflow)
								SS_CalcCellOverflow(lpSSTemp, lpCell, NULL, j, i);
							SS_UnlockCellItem(lpSSTemp, j, i);
							}

                  SS_BookUnlockSheetIndex(lpBook, nSheet);
				}
#endif
      }
      if (fPrevVal != fNewVal)
         SS_InvalidateScrollArea(lpSSActive, SS_SCROLLAREA_ALL);

      break;
#endif

#ifndef SS_NOVIRTUAL

   case SSB_VIRTUALMODE:
      fPrevVal = lpSS->fVirtualMode;
      lpSS->fVirtualMode = fNewVal;

      if (fNewVal)
         {
         BOOL fRedraw;

         lpSS->Virtual.VPhysSize = 0;
         lpSS->Virtual.VTop = lpSS->Row.HeaderCnt;
         lpSS->Row.UL = lpSS->Row.HeaderCnt;
         lpSS->Row.CurAt = lpSS->Row.HeaderCnt;
         lpBook->wScrollDirection = 0;

         fRedraw = lpBook->Redraw;
         lpBook->Redraw = FALSE;

         if (lpSS->Virtual.VMax == -1)
            SS_SetMaxRows(lpSS, SS_VMAXCOORD);
         else
            SS_SetMaxRows(lpSS, lpSS->Virtual.VMax);

         lpBook->Redraw = fRedraw;
         SS_InvalidateRect(lpBook, NULL, TRUE);
         SS_UpdateWindow(lpBook);
         }

      break;

#endif

   case SSB_ALLOWDRAGDROP:
      fPrevVal = lpBook->fAllowDragDrop;
      SS_InvertBlock(NULL, lpSSActive);
      lpBook->fAllowDragDrop = fNewVal;
      SS_InvertBlock(NULL, lpSSActive);
      break;

   case SSB_CHANGEMADE:
      fPrevVal = lpSS->fChangeMade;
      lpSS->fChangeMade = fNewVal;
      break;

#ifndef SS_OLDCALC
   case SSB_FORMULASYNC:
      fPrevVal = lpBook->CalcInfo.bFormulaSync;
      lpBook->CalcInfo.bFormulaSync = fNewVal;
      break;
#endif

   case SSB_HSCROLLBARTRACK:
      fPrevVal = lpBook->fHScrollBarTrack;
      lpBook->fHScrollBarTrack = fNewVal;
#ifdef SS_V80
	  SendMessage(lpBook->hWndHScroll, SBR_TRACKMOUSE, fNewVal, 0);
#endif
      break;

   case SSB_VSCROLLBARTRACK:
      fPrevVal = lpBook->fVScrollBarTrack;
      lpBook->fVScrollBarTrack = fNewVal;
#ifdef SS_V80
     // 22960 -scl
	  //SendMessage(lpBook->hWndHScroll, SBR_TRACKMOUSE, fNewVal, 0);
     SendMessage(lpBook->hWndVScroll, SBR_TRACKMOUSE, fNewVal, 0);
#endif
      break;

#ifdef SS_V40
   case SSB_ALLOWEDITOVERFLOW:
      fPrevVal = lpBook->fEditOverflow;
      lpBook->fEditOverflow = fNewVal;
      break;
#endif

#ifdef SS_V70
	case SSB_ALLOWCOLMOVE:
      fPrevVal = lpSS->fAllowColMove;
      lpSS->fAllowColMove = fNewVal;
      break;

	case SSB_ALLOWROWMOVE:
      fPrevVal = lpSS->fAllowRowMove;
      lpSS->fAllowRowMove = fNewVal;
      break;
#endif // SS_V70
   }

SS_BookUnlockActiveSheet(lpBook);
return (fPrevVal);
}


BOOL DLLENTRY SSGetBool(HWND hWnd, short nIndex)
{
LPSS_BOOK     lpBook = SS_BookLock(hWnd);
LPSPREADSHEET lpSS = SS_BookLockSheet(lpBook);
BOOL          fPrevVal;

fPrevVal = SS_GetBool(lpBook, lpSS, nIndex);

SS_BookUnlockSheet(lpBook);
SS_BookUnlock(hWnd);

return (fPrevVal);
}


BOOL SS_GetBool(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, short nIndex)
{
BOOL fPrevVal = 0;

switch (nIndex)
   {
   case SSB_PROTECT:
      fPrevVal = lpBook->DocumentProtected;
      break;

   case SSB_RESTRICTROWS:
      fPrevVal = lpSS->RestrictRows;
      break;

   case SSB_RESTRICTCOLS:
      fPrevVal = lpSS->RestrictCols;
      break;

   case SSB_SHOWCOLHEADERS:
		{
		SS_COORD Row;
		double   dfRowHeight;

		fPrevVal = FALSE;

		for (Row = 0; Row < lpSS->Row.HeaderCnt && !fPrevVal; Row++)
	      if (!SS_GetRowHeight(lpSS, Row, &dfRowHeight) || dfRowHeight != 0.0)
				fPrevVal = TRUE;
		}
      break;

   case SSB_SHOWROWHEADERS:
		{
		SS_COORD Col;
		double   dfColWidth;

		fPrevVal = FALSE;

		for (Col = 0; Col < lpSS->Col.HeaderCnt && !fPrevVal; Col++)
	      if (!SS_GetColWidth(lpSS, Col, &dfColWidth) || dfColWidth != 0.0)
				fPrevVal = TRUE;
		}
      break;

   case SSB_ALLOWUSERRESIZE:
      if (lpSS->wUserResize & (SS_USERRESIZE_COL | SS_USERRESIZE_ROW))
         fPrevVal = TRUE;
      else
         fPrevVal = FALSE;

      break;

   case SSB_ALLOWUSERSELBLOCK:
      if (lpBook->wSelBlockOption & (SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
                                       SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL))
         fPrevVal = TRUE;
      else
         fPrevVal = FALSE;

      break;

   case SSB_AUTOCALC:
      fPrevVal = lpBook->CalcAuto;
      break;

   case SSB_REDRAW:
      fPrevVal = lpBook->Redraw;
      break;

   case SSB_GRIDLINES:
      if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
         fPrevVal = TRUE;
      else
         fPrevVal = FALSE;

      break;

   case SSB_HORZSCROLLBAR:
      fPrevVal = lpBook->HorzScrollBar;
      break;

   case SSB_VERTSCROLLBAR:
      fPrevVal = lpBook->VertScrollBar;
      break;

   case SSB_ALLOWUSERFORMULAS:
      fPrevVal = lpBook->AllowUserFormulas;
      break;

   case SSB_EDITMODEPERMANENT:
      fPrevVal = lpBook->fEditModePermanent;
      break;

   case SSB_AUTOSIZE:
      fPrevVal = lpBook->fAutoSize;
      break;

   case SSB_SCROLLBAREXTMODE:
      fPrevVal = lpBook->fScrollBarExtMode;
      break;

   case SSB_SCROLLBARSHOWMAX:
      fPrevVal = lpBook->fScrollBarShowMax;
      break;

   case SSB_SCROLLBARMAXALIGN:
      fPrevVal = lpBook->fScrollBarMaxAlign;
      break;

   case SSB_PROCESSTAB:
      fPrevVal = lpBook->fProcessTab;
      break;

   case SSB_ARROWSEXITEDITMODE:
      fPrevVal = lpBook->fArrowsExitEditMode;
      break;

   case SSB_MOVEACTIVEONFOCUS:
      fPrevVal = lpBook->fMoveActiveOnFocus;
      break;

   case SSB_RETAINSELBLOCK:
      fPrevVal = lpBook->fRetainSelBlock;
      break;

   case SSB_EDITMODEREPLACE:
      fPrevVal = lpBook->fEditModeReplace;
      break;

   case SSB_NOBORDER:
      fPrevVal = lpBook->fNoBorder;
      break;

   case SSB_NOBEEP:
      fPrevVal = lpBook->fNoBeep;
      break;

   case SSB_AUTOCLIPBOARD:
      fPrevVal = lpBook->fAutoClipboard;
      break;

   case SSB_ALLOWMULTIBLOCKS:
      fPrevVal = lpBook->fAllowMultipleSelBlocks;
      break;

   case SSB_ALLOWCELLOVERFLOW:
      fPrevVal = lpBook->fAllowCellOverflow;
      break;

#ifndef SS_NOVIRTUAL
   case SSB_VIRTUALMODE:
      fPrevVal = lpSS->fVirtualMode;
      break;
#endif

   case SSB_ALLOWDRAGDROP:
      fPrevVal = lpBook->fAllowDragDrop;
      break;

   case SSB_CHANGEMADE:
      fPrevVal = lpSS->fChangeMade;
      break;

#ifndef SS_OLDCALC
   case SSB_FORMULASYNC:
      fPrevVal = lpBook->CalcInfo.bFormulaSync;
      break;
#endif

   case SSB_HSCROLLBARTRACK:
      fPrevVal = lpBook->fHScrollBarTrack;
      break;

   case SSB_VSCROLLBARTRACK:
      fPrevVal = lpBook->fVScrollBarTrack;
      break;

#ifdef SS_V40
   case SSB_ALLOWEDITOVERFLOW:
      fPrevVal = lpBook->fEditOverflow;
      break;
#endif

#ifdef SS_V70
	case SSB_ALLOWCOLMOVE:
      fPrevVal = lpSS->fAllowColMove;
      break;

	case SSB_ALLOWROWMOVE:
      fPrevVal = lpSS->fAllowRowMove;
      break;
#endif // SS_V70
   }

return (fPrevVal);
}


BOOL SS_BookSetRedraw(LPSS_BOOK lpBook, BOOL fRedraw)
{
BOOL fPrevVal = lpBook->Redraw;

if (fPrevVal != fRedraw)
   {
   lpBook->Redraw = fRedraw;
   if (fRedraw)
      {
      if (lpBook->fUpdateAutoSize)
         SS_AutoSize(lpBook, FALSE);
      else
         {
			LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
         SS_SetVScrollBar(lpSS);
         SS_SetHScrollBar(lpSS);
			SS_BookUnlockActiveSheet(lpBook);
         }

      if( !lpBook->hWnd )  // handle an OCX control without an HWND ????
        lpBook->fInvalidated = TRUE;
      else
         if (IsWindowVisible(lpBook->hWnd))
            {
				RECT Rect;

            if (GetUpdateRect(lpBook->hWnd, &Rect, FALSE))
               SS_UpdateWindow(lpBook);
            else if (fPrevVal)
               SS_InvalidateRect(lpBook, NULL, TRUE);
            }
      }
   }

return (fPrevVal);
}


BOOL SS_BookSetAutoCalc(LPSS_BOOK lpBook, BOOL fAutoCalc)
{
BOOL fPrevVal = lpBook->CalcAuto;
lpBook->CalcAuto = fAutoCalc;
#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
if (fAutoCalc)
   CalcEvalNeededCells(&lpBook->CalcInfo);
#endif

return (fPrevVal);
}