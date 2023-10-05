/*********************************************************
* SS_COL.C
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
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_draw.h"                   /* Needed for colors */
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_type.h"
#include "ss_win.h"
#include "ss_user.h"
#ifdef SS_V40
#include "ss_span.h"
#endif


BOOL DLLENTRY SSSetColWidth(HWND hWnd, SS_COORD Col, double dfWidth)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
bRet = SS_SetColWidth(lpSS, Col, dfWidth);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_SetColWidth(LPSPREADSHEET lpSS, SS_COORD Col, double dfWidth)
{
return (SSx_SetColWidth(lpSS, Col, dfWidth, FALSE));
}


BOOL SSx_SetColWidth(LPSPREADSHEET lpSS, SS_COORD Col, double dfWidth, BOOL fUserAction)
{
LPSS_COL      lpCol;
SS_COORD      i;
long          dColWidthX100;

if (dfWidth < 0.0 && dfWidth != -1.0)
	return (FALSE);

if (dfWidth == -1)
   dColWidthX100 = -1;
else
   dColWidthX100 = (long)(dfWidth * 100.0);

if (Col == -1)
   {
   for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
      if (lpCol = SS_LockColItem(lpSS, i))
         {
			// RFW - 11/29/05 - 17759
			if (!fUserAction || (SS_GetMergedUserResizeCol(lpSS, i) != SS_RESIZE_OFF))
				{
	         lpCol->dColWidthX100 = SS_WIDTH_DEFAULT;
	         lpCol->dColWidthInPixels = SS_WIDTH_DEFAULT;
				}

         SS_UnlockColItem(lpSS, i);
         }

   lpSS->Col.dCellSizeX100 = dColWidthX100;

   /*
   if (lpSS->Col.dCellSizeX100 == -1)
      lpSS->Col.dCellSizeInPixels = SS_WIDTH_DEFAULT;
   else
      lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, dfWidth);
   */

   lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, dfWidth);
   }

else
   {
   if (Col >= SS_GetColCnt(lpSS))
      {
      return (FALSE);
      }

   if (lpCol = SS_AllocLockCol(lpSS, Col))
      {
		//short dColWidthInPixels;
	    long dColWidthInPixels;

      if (lpCol->dColWidthX100 == dColWidthX100)
         {
         SS_UnlockColItem(lpSS, Col);
			// RFW - 4/22/04 - 14131
			// I changed this to return FALSE when the value does not change
         return (FALSE);
         }

      if (dColWidthX100 == -1)
         dColWidthInPixels = SS_WIDTH_DEFAULT;
      else
         dColWidthInPixels = SS_ColWidthToPixels(lpSS, dfWidth);

		// If the original value was unset and the user is trying to set it
		// to the default value, then leave it unset.
      if (lpCol->dColWidthX100 == -1 && dColWidthInPixels == lpSS->Col.dCellSizeInPixels)
         {
         SS_UnlockColItem(lpSS, Col);
         return (FALSE);
         }

      lpCol->dColWidthX100 = dColWidthX100;
      lpCol->dColWidthInPixels = dColWidthInPixels;

      SS_UnlockColItem(lpSS, Col);
      }

   if (lpSS->lpBook->fAllowCellOverflow)
      {
      SS_OverflowAdjustNeighbor(lpSS, Col, 0);
      for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
         SS_OverflowAdjustNeighbor(lpSS, Col, i);
      }
   }

SS_CellEditModeOff(lpSS, 0);

if (!SS_AutoSize(lpSS->lpBook, FALSE))
   {
   SS_SetHScrollBar(lpSS);

   if (Col == -1)
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

   else if (SS_IsColVisible(lpSS, Col, SS_VISIBLE_PARTIAL))
      SS_InvalidateColRange(lpSS, Col, -1);
   }

#ifdef SS_UTP
if (!lpSS->fUseScrollArrows || Col != lpSS->Col.HeaderCnt +
    lpSS->Col.Frozen - 1)
#endif
if (Col >= lpSS->Col.HeaderCnt && dfWidth == 0.0 && lpSS->Col.CurAt == Col)
   {
   if (SS_IsBlockSelected(lpSS))
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
	// RFW - 7/21/03 - 10961
	// I am very nervous about changing this since it has never fired the LeaveCell event
   //SS_ScrollLineLR(lpSS, SS_F_SCROLL_RIGHT | SS_F_MOVEACTIVECELL | SS_F_NOLEAVECELL);
   SS_ScrollLineLR(lpSS, SS_F_SCROLL_RIGHT | SS_F_MOVEACTIVECELL);
   if (lpSS->Col.CurAt == Col)
      //SS_ScrollLineUL(lpSS, SS_F_SCROLL_LEFT | SS_F_MOVEACTIVECELL | SS_F_NOLEAVECELL);
      SS_ScrollLineUL(lpSS, SS_F_SCROLL_LEFT | SS_F_MOVEACTIVECELL);
   }

// RFW - 7/23/03
if (Col >= lpSS->Col.HeaderCnt && dfWidth == 0.0 && lpSS->Col.UL == Col)
	{
   for (; Col < SS_GetColCnt(lpSS) - 1 && SS_GetColWidthInPixels(lpSS, Col) == 0; Col++)
		;

	lpSS->Col.UL = Col;
	}

return (TRUE);
}


BOOL DLLENTRY SSSetColWidthInPixels(HWND hWnd, SS_COORD Col, int dWidth)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
bRet = SS_SetColWidthInPixels(lpSS, Col, dWidth);
SS_SheetUnlock(hWnd);
return (bRet);
}


BOOL SS_SetColWidthInPixels(LPSPREADSHEET lpSS, SS_COORD Col, int dWidth)
{
double dfWidth;

SSLogUnitsToColWidth(lpSS->lpBook->hWnd, dWidth, &dfWidth);
return (SS_SetColWidth(lpSS, Col, dfWidth));

}


BOOL DLLENTRY SSGetColWidth(hWnd, Col, lpdfColWidth)

HWND          hWnd;
SS_COORD      Col;
LPDOUBLE      lpdfColWidth;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
fRet = SS_GetColWidth(lpSS, Col, lpdfColWidth);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetColWidth(lpSS, Col, lpdfColWidth)

LPSPREADSHEET lpSS;
SS_COORD      Col;
LPDOUBLE      lpdfColWidth;
{
BOOL          fRet = FALSE;

if (Col < SS_GetColCnt(lpSS))
   {
   SS_CalcPixelsToColWidth(lpSS, Col, SS_GetColWidthInPixels(lpSS,
                           Col), lpdfColWidth);
   fRet = TRUE;
   }

return (fRet);
}


BOOL DLLENTRY SSGetColWidthInPixels(HWND hWnd, SS_COORD Col,
                                    LPINT lpdWidthPixels)
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);

if (Col < SS_GetColCnt(lpSS))
   {
   *lpdWidthPixels = SS_GetColWidthInPixels(lpSS, Col);
   fRet = TRUE;
   }

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetColLock(lpSS, Col, Lock)

LPSPREADSHEET lpSS;
SS_COORD      Col;
BOOL          Lock;
{
LPSS_COL      lpCol;
LPSS_CELL     lpCell;
SS_COORD      i;

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, Col, i))
      lpCell->CellLocked = SS_LOCKED_DEF;

if (lpCol = SS_AllocLockCol(lpSS, Col))
   {
   lpCol->ColLocked = Lock;
   SS_UnlockColItem(lpSS, Col);

   if (lpSS->LockColor.ForegroundId || lpSS->LockColor.BackgroundId)
      SS_InvalidateCol(lpSS, Col);
   }

return (TRUE);
}


BOOL DLLENTRY SSInsCol(hWnd, Col)

HWND             hWnd;
SS_COORD         Col;
{
return (SSInsColRange(hWnd, Col, Col));
}


BOOL DLLENTRY SSInsColRange(hWnd, Col, Col2)

HWND             hWnd;
SS_COORD         Col;
SS_COORD         Col2;
{
LPSPREADSHEET    lpSS;
BOOL             bRet;

lpSS = SS_SheetLock(hWnd);

if (Col == SS_FIRST)
   Col = lpSS->Col.HeaderCnt;
else if (Col == SS_LAST)
   Col = lpSS->Col.AllocCnt;
else
	SS_AdjustCellCoords(lpSS, &Col, NULL);

if (Col <= 0)
	{
	SS_SheetUnlock(hWnd);
   return (FALSE);
	}

if (Col2 == SS_FIRST)
   Col2 = lpSS->Col.HeaderCnt;
else if (Col2 == SS_LAST)
   Col2 = lpSS->Col.AllocCnt;
else
	SS_AdjustCellCoords(lpSS, &Col2, NULL);

if (Col2 <= 0)
	{
	SS_SheetUnlock(hWnd);
   return (FALSE);
	}

bRet = SS_InsColRange(lpSS, Col, Col2);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_InsColRange(lpSS, Col, Col2)

LPSPREADSHEET    lpSS;
SS_COORD         Col;
SS_COORD         Col2;
{
#ifdef SS_OLDCALC
LPTBGLOBALHANDLE CalcTable;
LPSS_CALC        lpCalc;
#endif
LPSS_ROW         lpRow;
SS_COORD         i;
BOOL             fVisible = FALSE;

//BJO 2003Aug11 bug #11870 - quick exit skips update of custom names
//if (Col >= lpSS->Col.AllocCnt || Col >= SS_GetColCnt(lpSS))
//   return (FALSE);

if (Col > Col2)
   return (FALSE);

SS_CellEditModeOff(lpSS, 0);

for (i = max(SS_GetColCnt(lpSS) - 1 - (Col2 - Col), Col);
     i <= SS_GetColCnt(lpSS) - 1; i++)
   SS_DeAllocCol(lpSS, i, FALSE);

if (SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolCol,
                      &lpSS->Col.Items, Col, lpSS->Col.AllocCnt - 1,
                      Col2 + 1, SS_GetColCnt(lpSS)))
   {
   for (i = 0; i < lpSS->Row.AllocCnt; i++)
      {
      if (lpRow = SS_LockRowItem(lpSS, i))
         {
			SS_OverflowInvalidateCell(lpSS, Col, i);
			SS_ResetCellOverflowNeighbor(lpSS, Col, i);

         SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolCell,
                           &lpRow->Cells, Col, lpSS->Col.AllocCnt - 1,
                           Col2 + 1, SS_GetColCnt(lpSS));

			/* RFW - 8/4/04 - 14998
	      SS_OverflowAdjustNeighbor(lpSS, Col2, i);
			*/
			lpSS->Col.LastRefCoord.Coord.Col = -1;
			lpSS->LastCell.Coord.Col = -1;
			lpSS->LastCell.Coord.Row = -1;
	      SS_OverflowAdjustNeighbor(lpSS, Col2 + 1, i);

         SS_UnlockRowItem(lpSS, i);
         }
      }

   if (Col < lpSS->Col.DataCnt)
      SS_SetDataColCnt(lpSS, min(SS_GetColCnt(lpSS),
                       lpSS->Col.DataCnt + (Col2 - Col + 1)));

   if (Col < lpSS->Col.AllocCnt)
      lpSS->Col.AllocCnt = min(SS_GetColCnt(lpSS),
                             lpSS->Col.AllocCnt + (Col2 - Col + 1));

   lpSS->Col.LastRefCoord.Coord.Col = -1;
   lpSS->LastCell.Coord.Col = -1;
   lpSS->LastCell.Coord.Row = -1;

   #ifdef SS_OLDCALC
   if (lpSS->hCalcTable)
      {
      CalcTable = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->hCalcTable);
      for (i = 0; i < lpSS->CalcTableCnt; i++)
         {
         lpCalc = (LPSS_CALC)tbGlobalLock(CalcTable[i]);

         if (lpCalc->CellCoord.Col >= Col2)
            lpCalc->CellCoord.Col += Col2 - Col + 1;

         tbGlobalUnlock(CalcTable[i]);
         }

      tbGlobalUnlock(lpSS->hCalcTable);
      }
   #endif

#ifdef SS_V40
   SS_InsCellSpanColRange(lpSS, Col, Col2 - Col + 1);
#endif // SS_V40

   for (i = Col, fVisible = FALSE; i <= Col2; i++)
      if (SS_IsColVisible(lpSS, i, SS_VISIBLE_PARTIAL))
         fVisible = TRUE;

	/* RFW - 1/4/05 - 15241
	// RFW - 6/10/04 - 14021 
	// Adjust ActiveCell and UpperLeft
	if (Col <= lpSS->Col.CurAt)
		{
		lpSS->Col.CurAt += Col2 - Col + 1;
		lpSS->Col.CurAt = min(lpSS->Col.CurAt, SS_GetColCnt(lpSS) - 1);
		}
	*/

	if (Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen && Col < lpSS->Col.UL)
		{
		lpSS->Col.UL += Col2 - Col + 1;
		lpSS->Col.UL = min(lpSS->Col.UL, SS_GetColCnt(lpSS) - 1);
		lpSS->Col.LR = SS_GetRightCell(lpSS, lpSS->Col.UL);
		}

	// RFW - 6/10/04 - 14021-3
	SS_DeSelectBlock(lpSS);

   if (fVisible)
      /*
      SS_HScrollCol(lpSS, Col, TRUE, TRUE);
      */
      SS_InvalidateColRange(lpSS, Col, -1);
   else if (Col < lpSS->Col.UL)
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_COLS);
   }

#ifdef SS_OLDCALC
SS_BuildDependencies(lpSS);
#elif !defined(SS_NOCALC)
if (Col >= lpSS->Col.HeaderCnt)
	{
	SS_COORD lColOut = Col;
	SS_COORD lCol2Out = Col2;

	SS_AdjustCellCoordsOut(lpSS, &lColOut, NULL);
	SS_AdjustCellCoordsOut(lpSS, &lCol2Out, NULL);
	CalcInsColRange(&lpSS->CalcInfo, lColOut, lCol2Out);
	}
if (lpSS->lpBook->CalcAuto )
  CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif

#ifdef SS_UTP
if (Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
   SS_SendMsgInsDel(lpSS->lpBook->hWnd, SS_INSDEL_TYPE_INS, SS_INSDEL_OP_POST, SS_INSDEL_COL,
                    Col, Col2);
#endif

SS_HighlightCell(lpSS, TRUE);

return (TRUE);
}


BOOL DLLENTRY SSDelCol(hWnd, Col)

HWND             hWnd;
SS_COORD         Col;
{
return (SSDelColRange(hWnd, Col, Col));
}


BOOL DLLENTRY SSDelColRange(hWnd, Col, Col2)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Col2;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SSxDelColRange(lpSS, Col, Col2);
SS_SheetUnlock(hWnd);

return (fRet);
}


BOOL SSxDelColRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2)
{
BOOL fRet;

if (Col == SS_FIRST)
   Col = lpSS->Col.HeaderCnt;
else if (Col == SS_LAST)
   Col = lpSS->Col.AllocCnt - 1;
else
	SS_AdjustCellCoords(lpSS, &Col, NULL);

if (Col <= 0)
   return (FALSE);

if (Col2 == SS_FIRST)
   Col2 = lpSS->Col.HeaderCnt;
else if (Col2 == SS_LAST)
   Col2 = lpSS->Col.AllocCnt - 1;
else
	SS_AdjustCellCoords(lpSS, &Col2, NULL);

if (Col2 <= 0)
   return (FALSE);

fRet = SS_DelColRange(lpSS, Col, Col2);
return (fRet);
}


BOOL SS_DelColRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2)
{
LPTBGLOBALHANDLE lpCellList;
#ifdef SS_OLDCALC
LPTBGLOBALHANDLE CalcTable;
LPSS_CALC        lpCalc;
#endif
LPSS_UNDOCOL     lpList;
LPSS_ROW         lpRow;
SS_COORD         Index;
SS_COORD         i;
SS_COORD         j;
BOOL             fVisible = FALSE;
BOOL             RedrawOld = lpSS->lpBook->Redraw;

lpSS->lpBook->Redraw = FALSE;

//BJO 2003Aug11 bug #11870 - quick exit skips update of custom names
//if (Col >= lpSS->Col.AllocCnt || Col >= SS_GetColCnt(lpSS))
//   return (FALSE);
//
//Col2 = min(Col2, lpSS->Col.AllocCnt - 1);

if (Col > Col2)
   return (FALSE);

SS_CellEditModeOff(lpSS, 0);

if (lpSS->lpBook->fAllowUndo)
   {
   SS_ClearUndoBuffer(lpSS);

   if (lpSS->UndoBuffer.hData = tbGlobalAlloc(GMEM_MOVEABLE |
                                    GMEM_ZEROINIT,
                                    (long)sizeof(SS_UNDOCOL) *
                                    (Col2 - Col + 1)))
      {
      lpList = (LPSS_UNDOCOL)tbGlobalLock(lpSS->UndoBuffer.hData);

      for (Index = 0, i = Col; i <= Col2; i++, Index++)
         {
         lpList[Index].hCol = SS_GrabItemHandle(lpSS, &lpSS->Col.Items, i,
                                                SS_GetColCnt(lpSS));

         if (lpList[Index].hCellList = tbGlobalAlloc(GMEM_MOVEABLE |
                                       GMEM_ZEROINIT,
                                       (long)sizeof(TBGLOBALHANDLE) *
                                       lpSS->Row.AllocCnt))

         lpCellList = (LPTBGLOBALHANDLE)tbGlobalLock(lpList[Index].hCellList);

         for (j = 0; j < lpSS->Row.AllocCnt; j++)
            if (lpRow = SS_LockRowItem(lpSS, j))
               {
               lpCellList[j] = SS_GrabItemHandle(lpSS, &lpRow->Cells, i,
                                                 SS_GetColCnt(lpSS));
               SS_UnlockRowItem(lpSS, j);
               }

         tbGlobalUnlock(lpList[Index].hCellList);
         }

      tbGlobalUnlock(lpSS->UndoBuffer.hData);
      }

   lpSS->UndoBuffer.bUndoType = SS_UNDOTYPE_DELCOL;
   lpSS->UndoBuffer.ItemAllocCnt = lpSS->Row.AllocCnt;
   lpSS->UndoBuffer.Col = Col;
   lpSS->UndoBuffer.Col2 = Col2;
   }

else
   for (i = Col; i <= Col2; i++)
      SS_DeAllocCol(lpSS, i, FALSE);

SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolCol, &lpSS->Col.Items, Col2 + 1,
                  lpSS->Col.AllocCnt - 1, Col, SS_GetColCnt(lpSS));

for (i = 0; i < lpSS->Row.AllocCnt; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
		// RFW - 3/11/04 - 13832
		SS_OverflowInvalidateCell(lpSS, Col, i);
		SS_ResetCellOverflowNeighbor(lpSS, Col, i);
      SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolCell, &lpRow->Cells, Col2 + 1,
                        lpSS->Col.AllocCnt - 1, Col, SS_GetColCnt(lpSS));
      SS_OverflowAdjustNeighbor(lpSS, Col, i);
      SS_UnlockRowItem(lpSS, i);
      }
   }

lpSS->Col.LastRefCoord.Coord.Col = -1;
lpSS->LastCell.Coord.Col = -1;
lpSS->LastCell.Coord.Row = -1;

#ifdef SS_OLDCALC
if (lpSS->hCalcTable)
   {
   CalcTable = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->hCalcTable);
   for (i = 0; i < lpSS->CalcTableCnt; i++)
      {
      lpCalc = (LPSS_CALC)tbGlobalLock(CalcTable[i]);

      if (lpCalc->CellCoord.Col > Col2)
         lpCalc->CellCoord.Col -= Col2 - Col + 1;

      tbGlobalUnlock(CalcTable[i]);
      }

   tbGlobalUnlock(lpSS->hCalcTable);
   }
#endif

#ifdef SS_V40
SS_DelCellSpanColRange(lpSS, Col, Col2 - Col + 1, TRUE);
#endif // SS_V40

for (i = Col, fVisible = FALSE; i <= Col2; i++)
   if (SS_IsColVisible(lpSS, i, SS_VISIBLE_PARTIAL))
      fVisible = TRUE;

// RFW - 4/27/00 - SEL5028
//if (Col < lpSS->Col.DataCnt - 1 && Col2 < lpSS->Col.DataCnt - 1)
if (Col < lpSS->Col.DataCnt && Col2 < lpSS->Col.DataCnt)
   SS_SetDataColCnt(lpSS, max(lpSS->Col.DataCnt -
                    min(Col2 - Col + 1, lpSS->Col.DataCnt - Col), 1));

if (Col < lpSS->Col.AllocCnt)
   lpSS->Col.AllocCnt = max(lpSS->Col.AllocCnt - min(Col2 - Col + 1,
                          lpSS->Col.AllocCnt - Col), 1);

// RFW - 6/10/02 - RUN_SPR_003_013
if (Col < lpSS->Col.HeaderCnt)
	lpSS->Col.HeaderCnt -= min(Col2, lpSS->Col.HeaderCnt) - Col + 1;

// RFW - 4/15/04 - 14021 
// Adjust ActiveCell and UpperLeft
if (Col < lpSS->Col.CurAt)
	{
	/* RFW - 1/4/05 - 15241
	lpSS->Col.CurAt -= min(lpSS->Col.CurAt - 1, Col2) - Col + 1;
	*/
	lpSS->Col.CurAt = max(lpSS->Col.HeaderCnt, lpSS->Col.CurAt);
	}

if (Col < lpSS->Col.UL)
	{
	lpSS->Col.UL -= min(lpSS->Col.UL - 1, Col2) - Col + 1;
	lpSS->Col.UL = max(lpSS->Col.HeaderCnt + lpSS->Col.Frozen, lpSS->Col.UL);
	}

// RFW - 6/10/04 - 14021-3
SS_DeSelectBlock(lpSS);

SS_AdjustDataColCnt(lpSS, Col, Col2);
SS_AdjustDataRowCnt(lpSS, -1, -1);

// RFW - 4/6/04 - 14021 
// This block was move down from above
if (fVisible)
   /*
   SS_HScrollCol(lpSS, Col, FALSE, TRUE);
   */
   SS_InvalidateColRange(lpSS, Col, -1);
else if (Col < lpSS->Col.UL)
   SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_COLS);

#ifdef SS_OLDCALC
SS_BuildDependencies(lpSS);
#elif !defined(SS_NOCALC)
if (Col >= lpSS->Col.HeaderCnt)
	{
	SS_COORD lColOut = Col;
	SS_COORD lCol2Out = Col2;

	SS_AdjustCellCoordsOut(lpSS, &lColOut, NULL);
	SS_AdjustCellCoordsOut(lpSS, &lCol2Out, NULL);
	CalcDelColRange(&lpSS->CalcInfo, lColOut, lCol2Out);
	}
if (lpSS->lpBook->CalcAuto)
   CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif

SS_HighlightCell(lpSS, TRUE);

SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

return (TRUE);
}


BOOL SS_ClearCol(LPSPREADSHEET lpSS, SS_COORD Col, BOOL Paint,
                 BOOL fSendChangeMsg)
{
if (Col >= lpSS->Col.AllocCnt || Col >= SS_GetColCnt(lpSS))
   {
   return (FALSE);
   }

SS_DeAllocCol(lpSS, Col, fSendChangeMsg);

if (Paint && SS_IsColVisible(lpSS, Col, SS_VISIBLE_PARTIAL))
   SS_InvalidateColRange(lpSS, Col, -1);

return (TRUE);
}


BOOL SSIsColVisible(hWnd, Col, Visible)

HWND          hWnd;
SS_COORD      Col;
short         Visible;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);

SS_AdjustCellCoords(lpSS, &Col, NULL);
fRet = SS_IsColVisible(lpSS, Col, Visible);

if (fRet && !SS_GetColWidthInPixels(lpSS, Col))
   fRet = FALSE;

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_IsColVisible(LPSPREADSHEET lpSS, SS_COORD Col, short Visible)
{
BOOL fRet = FALSE;

switch (Visible)
   {
   case SS_VISIBLE_ALL:
      if (Col < lpSS->Col.HeaderCnt + lpSS->Col.Frozen ||
          (Col >= lpSS->Col.UL && Col <=
          SS_GetRightVisCell(lpSS, lpSS->Col.UL)))
         fRet = TRUE;

      break;

   case SS_VISIBLE_PARTIAL:
      if (Col < lpSS->Col.HeaderCnt + lpSS->Col.Frozen ||
          (Col >= lpSS->Col.UL && Col <= lpSS->Col.LR))
         fRet = TRUE;

      break;
   }

return (fRet);
}


BOOL SS_SetColFont(lpSS, Col, hFont, DeleteFont, lpfDeleteFontObject)

LPSPREADSHEET lpSS;
SS_COORD      Col;
HFONT         hFont;
BOOL          DeleteFont;
LPBOOL        lpfDeleteFontObject;
{
LPSS_COL      lpCol;
LPSS_CELL     lpCell;
SS_COORD      i;

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, Col, i))
      lpCell->FontId = SS_FONT_NONE;

if (lpCol = SS_AllocLockCol(lpSS, Col))
   {
   lpCol->FontId = SS_InitFont(lpSS->lpBook, hFont, DeleteFont, TRUE,
                               lpfDeleteFontObject);
   SS_UnlockColItem(lpSS, Col);

   if (lpSS->lpBook->fAllowCellOverflow)
      for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
         SS_OverflowAdjustNeighbor(lpSS, Col, i);
   }

SS_InvalidateCol(lpSS, Col);
return (TRUE);
}

#if SS_V80
BOOL SS_SetColLogFont(LPSPREADSHEET lpSS, SS_COORD Col, LOGFONT *pLogFont)

{
LPSS_COL      lpCol;
LPSS_CELL     lpCell;
SS_COORD      i;

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, Col, i))
      lpCell->FontId = SS_FONT_NONE;

if (lpCol = SS_AllocLockCol(lpSS, Col))
   {
   lpCol->FontId = SS_InitFontLogFont(lpSS->lpBook, pLogFont);
   SS_UnlockColItem(lpSS, Col);

   if (lpSS->lpBook->fAllowCellOverflow)
      for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
         SS_OverflowAdjustNeighbor(lpSS, Col, i);
   }

SS_InvalidateCol(lpSS, Col);
return (TRUE);
}
#endif

BOOL SS_SetColCellType(LPSPREADSHEET lpSS, SS_COORD Col,
                       LPSS_CELLTYPE CellType)
{
TBGLOBALHANDLE hData;
LPSS_CELLTYPE  CellTypeTemp;
SS_CELLTYPE    CellTypeOld;
LPSS_COL       lpCol;
LPSS_CELL      lpCell;
LPTSTR         lpData;
SS_DATA        Data;
SS_COORD       i;
BOOL           fTurnEditModeOn = FALSE;
BOOL           fValue = FALSE;
BOOL           fSetData = FALSE;

#ifdef SPREAD_JPN
// BUG SPR_VIR_001_009 (2-1)
// When DAutoCellType Is True, 
// Spread Calls SS_SetColCellType() To Set Each Column's Cell Type From Database.
// But If In Virtual Mode, It Does Not Set Cell Type Of The Whole Column,
// It Only Set Cells Up To The Last Row With Data (lpSS->Row.DataCnt).
// Since Some Cells May Have Different Cell Type But Do Not Contain Data,
// Their Data Type Will Not Be Reset.
// The Way To Solve This Problem Is Let It Reset All Rows (lpSS->Row.AllocCnt),
// But Not Only Up To The Last Row With Data.
// I Think The Will Not Cause Other Problems But Only Effect Performance.
// Modified By HaHa 1999.11.12
//for (i = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : 1);
//     i < (lpSS->fVirtualMode ? lpSS->Row.DataCnt : lpSS->Row.AllocCnt); i++)
for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
#else
for (i = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : lpSS->Row.HeaderCnt);
     i < (lpSS->fVirtualMode ? lpSS->Row.DataCnt : lpSS->Row.AllocCnt); i++)
#endif
   if (lpCell = SS_LockCellItem(lpSS, Col, i))
      {
      if (lpCell->hCellType)
         {
         SS_DeAllocCellType(lpSS, lpCell->hCellType);
         lpCell->hCellType = 0;
         }

      SS_UnlockCellItem(lpSS, Col, i);
      }

if (lpCol = SS_AllocLockCol(lpSS, Col))
   {
	// RFW - 11/17/08 - 23689
   if (lpSS->lpBook->EditModeOn && !lpSS->lpBook->fLeaveEditModeOn && Col == lpSS->Col.CurAt)
      {
      WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

      lpSS->lpBook->wMessageBeingSent = FALSE;
      SS_CellEditModeOff(lpSS, 0);
      lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;

      fTurnEditModeOn = TRUE;
      }

	// RFW - 6/14/04 - 14584
   SS_RetrieveCellType(lpSS, &CellTypeOld, NULL, Col, SS_ALLROWS);
   SS_DeAllocCellType(lpSS, lpCol->hCellType);
   lpCol->hCellType = 0;

   if (CellType)
      {
      if (!(lpCol->hCellType = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                             (long)sizeof(SS_CELLTYPE))))
         {
         SS_UnlockColItem(lpSS, Col);
         return (FALSE);
         }

      if ((lpCol->Data.bDataType == SS_TYPE_FLOAT &&
           !SS_IsCellTypeFloatNum(CellType)) ||
          (lpCol->Data.bDataType != SS_TYPE_FLOAT &&
           SS_IsCellTypeFloatNum(CellType)) ||
          (CellType->Type == SS_TYPE_COMBOBOX &&
           CellType->Spec.ComboBox.hItems))
         fSetData = TRUE;

      if ((SS_TYPE_DATE == CellTypeOld.Type &&
           SS_TYPE_DATE == CellType->Type) ||
          (SS_TYPE_TIME == CellTypeOld.Type &&
           SS_TYPE_TIME == CellType->Type))
         {
         fSetData = TRUE;
         fValue = SS_VALUE_VALUE;
         }

      CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpCol->hCellType);
      _fmemcpy(CellTypeTemp, CellType, sizeof(SS_CELLTYPE));
#if SS_V80
      SS_CT_Ref(lpSS, CellTypeTemp);
#endif
      tbGlobalUnlock(lpCol->hCellType);
      }

   else if (lpCol->Data.bDataType == SS_TYPE_FLOAT)
      fSetData = TRUE;

   if (fSetData)
      {
		/* RFW - 7/24/04 - 14931
      if (hData = SS_GetData(lpSS, CellType, Col, -1, FALSE))
		*/
      if (hData = SS_GetData(lpSS, &CellTypeOld, Col, -1, FALSE))
         {
			TBGLOBALHANDLE hValue;

         lpData = (LPTSTR)tbGlobalLock(hData);

         if (SS_VALUE_VALUE == fValue)
				{
            if (hValue = SS_UnFormatData(lpSS, Col, -1, &CellTypeOld, lpData))
               lpData = (LPTSTR)tbGlobalLock(hValue);
            else
               lpData = NULL;
				}

         SS_SetDataRange(lpSS, Col, -1, Col, -1, lpData, fValue, FALSE, FALSE);

         if (SS_VALUE_VALUE == fValue && hValue)
	         {
            tbGlobalUnlock(hValue);
            tbGlobalFree(hValue);
	         }

         tbGlobalUnlock(hData);
         }
      else
			/* RFW - 6/16/04 - 14530
			lpCol->Data.bDataType = SS_TYPE_EDIT;
			*/
			lpCol->Data.bDataType = 0;
      }

#ifdef SPREAD_JPN
	// BUG SPR_VIR_001_009 (2-2)
	// Modified By HaHa 1999.11.12
	//for (i = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : 1);
	//     i < (lpSS->fVirtualMode ? lpSS->Row.DataCnt : lpSS->Row.AllocCnt); i++)
	for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
#else
   for (i = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : lpSS->Row.HeaderCnt);
        i < (lpSS->fVirtualMode ? lpSS->Row.DataCnt : lpSS->Row.AllocCnt); i++)
#endif
      if (lpCell = SS_LockCellItem(lpSS, Col, i))
			{
			if (SS_IsCellTypeFloatNum(CellType) &&
				 SS_GetDataStruct(lpSS, lpCol, NULL, NULL, &Data, Col, i) &&
				 Data.bDataType == SS_TYPE_FLOAT)
				;  // No need to reformat data
			else if (hData = SS_GetData(lpSS, &CellTypeOld, Col, i, FALSE))
				{
				lpData = (LPTSTR)tbGlobalLock(hData);
				SS_SetDataRange(lpSS, Col, i, Col, i, lpData, FALSE, FALSE, FALSE);
				tbGlobalUnlock(hData);
				}
			else
				/* RFW - 6/16/04 - 14530
				lpCell->Data.bDataType = SS_TYPE_EDIT;
				*/
				lpCell->Data.bDataType = 0;

         SS_UnlockCellItem(lpSS, Col, i);
         }

   SS_InvalidateCol(lpSS, Col);

	// RFW - 11/17/08 - 23689
   if (fTurnEditModeOn)
      {
      SS_UpdateWindow(lpSS->lpBook);
      SS_CellEditModeOn(lpSS, 0, 0, 0L);
      }

   SS_UnlockColItem(lpSS, Col);
   }

return (TRUE);
}


void SS_SetDataColCnt(LPSPREADSHEET lpSS, SS_COORD ColCnt)
{
if (lpSS->Col.MaxBeforeSort == -1 || ColCnt <= lpSS->Col.MaxBeforeSort + 1)
   lpSS->Col.DataCnt = ColCnt;
}


BOOL SS_SetColColor(lpSS, Col, Background, Foreground)

LPSPREADSHEET lpSS;
SS_COORD      Col;
COLORREF      Background;
COLORREF      Foreground;
{
LPSS_COL      lpCol;
LPSS_CELL     lpCell;
SS_COORD      i;

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, Col, i))
      SS_ClearColorItem(&lpCell->Color, Background, Foreground);

if (lpCol = SS_AllocLockCol(lpSS, Col))
   {
//   if (!SS_ISDEFCOLOR(Background))
   if (Background != SPREAD_COLOR_IGNORE)
      lpCol->Color.BackgroundId = SS_AddColor(Background);

//   if (!SS_ISDEFCOLOR(Foreground))
   if (Foreground != SPREAD_COLOR_IGNORE)
      lpCol->Color.ForegroundId = SS_AddColor(Foreground);

   SS_UnlockColItem(lpSS, Col);

	// RFW - 2/6/02 - 9762
   if (lpSS->lpBook->EditModeOn && !lpSS->lpBook->fLeaveEditModeOn && Col == lpSS->Col.CurAt)
      {
      WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

      lpSS->lpBook->wMessageBeingSent = FALSE;
      SS_CellEditModeOff(lpSS, 0);
      lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;
      SS_UpdateWindow(lpSS->lpBook);
      SS_CellEditModeOn(lpSS, 0, 0, 0L);
      }

   if (lpSS->lpBook->fAllowCellOverflow)
      for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
         // RFW - 3/18/00 - GRB9138
         SS_OverflowInvalidateCell(lpSS, Col, i);
   }

SS_InvalidateCol(lpSS, Col);
return (TRUE);
}


#ifndef SS_NOMAXTEXT

BOOL DLLENTRY SSGetMaxTextColWidth(hWnd, Col, lpdfColWidth)

HWND          hWnd;
SS_COORD      Col;
LPDOUBLE      lpdfColWidth;
{
LPSPREADSHEET lpSS;
BOOL       fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
fRet = SS_GetMaxTextColWidth(lpSS, Col, lpdfColWidth);
SS_SheetUnlock(hWnd);
return fRet;
}


BOOL SS_GetMaxTextColWidth(LPSPREADSHEET lpSS, SS_COORD Col,
                           LPDOUBLE lpdfColWidth)
{
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
HDC           hDC;
long          lWidth = 0;
int           iWidthNew;
SS_COORD      Top;
SS_COORD      i;
SS_COORD      RowCnt;
BOOL          fRet = FALSE;

*lpdfColWidth = 0.0;

hDC = SS_GetDC(lpSS->lpBook);

if (Col < SS_GetColCnt(lpSS))
   {
   /*
   SS_CalcPixelsToColWidth(lpSS, Col, SS_GetColWidthInPixels(lpSS, Col),
                           lpdfColWidth);
   */

   lpCol = SS_LockColItem(lpSS, Col);

   if (lpSS->fVirtualMode)
      Top = max(lpSS->Row.HeaderCnt, lpSS->Virtual.VTop); // RFW - 8/8/05 - 16572
   else
      Top = lpSS->Row.HeaderCnt;

   if (Col < lpSS->Col.HeaderCnt)
      if (lpSS->fVirtualMode)
         RowCnt = lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize;
      else
         RowCnt = SS_GetRowCnt(lpSS);
   else
      RowCnt = lpSS->Row.AllocCnt;
		// RFW - 6/2/03      RowCnt = lpSS->Row.DataCnt;

   for (i = 0; i < RowCnt; )
      {
      lpRow = SS_LockRowItem(lpSS, i);

      SS_CalcCellMetrics(hDC, lpSS, lpCol, lpRow, NULL, NULL, NULL, Col, i, TRUE,
                         FALSE, NULL, NULL, &iWidthNew, NULL);
      lWidth = max(lWidth, iWidthNew + 1);

      if (lpRow)
         SS_UnlockRowItem(lpSS, i);

      if (i == lpSS->Row.HeaderCnt - 1)
         i = Top;
      else
         i++;
      }

   if (lWidth)
      SS_CalcPixelsToColWidth(lpSS, Col, lWidth, lpdfColWidth);

   if (lpCol)
      SS_UnlockColItem(lpSS, Col);

   fRet = TRUE;
   }

SS_ReleaseDC(lpSS->lpBook, hDC);

return (fRet);
}

#endif


#ifndef SS_NOUSERDATA
#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSSetColUserData(HWND hWnd, SS_COORD Col, LONG_PTR lUserData)
#else
BOOL DLLENTRY SSSetColUserData(HWND hWnd, SS_COORD Col, LONG lUserData)
#endif
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
bRet = SS_SetColUserData(lpSS, Col, lUserData);
SS_SheetUnlock(hWnd);
return bRet;
}

#if defined(_WIN64) || defined(_IA64)
BOOL SS_SetColUserData(LPSPREADSHEET lpSS, SS_COORD Col, LONG_PTR lUserData)
#else
BOOL SS_SetColUserData(LPSPREADSHEET lpSS, SS_COORD Col, LONG lUserData)
#endif
{
LPSS_COL lpCol;
BOOL     bRet = FALSE;

if (Col == SS_ALLCOLS)
   return bRet;

if (Col < SS_GetColCnt(lpSS) && (lpCol = SS_AllocLockCol(lpSS, Col)))
   {
   lpCol->lUserData = lUserData;
   SS_UnlockColItem(lpSS, Col);
   bRet = TRUE;
   }
return bRet;
}

#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSGetColUserData(HWND hWnd, SS_COORD Col, LONG_PTR* lplUserData)
#else
BOOL DLLENTRY SSGetColUserData(HWND hWnd, SS_COORD Col, LPLONG lplUserData)
#endif
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
bRet = SS_GetColUserData(lpSS, Col, lplUserData);
SS_SheetUnlock(hWnd);
return bRet;
}

#if defined(_WIN64) || defined(_IA64)
BOOL SS_GetColUserData(LPSPREADSHEET lpSS, SS_COORD Col, LONG_PTR* lplUserData)
#else
BOOL SS_GetColUserData(LPSPREADSHEET lpSS, SS_COORD Col, LPLONG lplUserData)
#endif
{
LPSS_COL lpCol;
BOOL     bRet = FALSE;

if (lplUserData)
   *lplUserData = 0;

if (Col == SS_ALLCOLS)
   return bRet;

if (Col < SS_GetColCnt(lpSS) &&
    (lpCol = SS_LockColItem(lpSS, Col)))
   {
   if (lplUserData)
      *lplUserData = lpCol->lUserData;
   if (lpCol->lUserData)
      bRet = TRUE;
   SS_UnlockColItem(lpSS, Col);
   }
return bRet;
}

#endif


#ifndef SS_NOPRINT

BOOL DLLENTRY SSSetColPageBreak(HWND hWnd, SS_COORD Col, BOOL fPageBreak)
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
fRet = SS_SetColPageBreak(lpSS, Col, fPageBreak);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetColPageBreak(LPSPREADSHEET lpSS, SS_COORD Col, BOOL fPageBreak)
{
LPSS_COL      lpCol;
BOOL          fRet = FALSE;

if (Col < SS_GetColCnt(lpSS) && (lpCol = SS_AllocLockCol(lpSS, Col)))
   {
   lpCol->fPageBreak = fPageBreak;
   SS_UnlockColItem(lpSS, Col);
   fRet = TRUE;
   }

return (fRet);
}


BOOL DLLENTRY SSGetColPageBreak(HWND hWnd, SS_COORD Col)
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
fRet = SS_GetColPageBreak(lpSS, Col);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetColPageBreak(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL      lpCol;
BOOL          fRet = FALSE;

if (Col < SS_GetColCnt(lpSS) &&
    (lpCol = SS_LockColItem(lpSS, Col)))
   {
   fRet = lpCol->fPageBreak;
   SS_UnlockColItem(lpSS, Col);
   }

return (fRet);
}

#endif


void SS_AdjustDataColCnt(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2)
{
LPSS_CELL lpCell;
SS_COORD  Top;
SS_COORD  i;
SS_COORD  j;
BOOL      fHasData = FALSE;

if (Col == SS_ALLCOLS || (Col <= lpSS->Col.DataCnt &&
    lpSS->Col.DataCnt <= Col2 + 1))
   {
   if (lpSS->fVirtualMode)
      Top = lpSS->Virtual.VTop;
   else
      Top = lpSS->Row.HeaderCnt;

   for (i = lpSS->Col.DataCnt - 1; i >= lpSS->Col.HeaderCnt && !fHasData; i--)
      {
      for (j = Top; j < lpSS->Row.DataCnt && !fHasData; j++)
         if (lpCell = SS_LockCellItem(lpSS, i, j))
            {
            if (lpCell->Data.bDataType)
               fHasData = TRUE;

            SS_UnlockCellItem(lpSS, i, j);
            }

      if (!fHasData)
         lpSS->Col.DataCnt--;
      }
   }
}


BOOL SS_SetColFieldName(LPSPREADSHEET lpSS, SS_COORD Col, LPCTSTR lpText)
{
LPSS_COL lpCol;
LPTSTR   lpFieldName;

if (Col >= SS_GetColCnt(lpSS))
   return (FALSE);

if (lpText && *lpText)
	lpCol = SS_AllocLockCol(lpSS, Col);
else
	lpCol = SS_LockColItem(lpSS, Col);

if (lpCol)
   {
   if (lpCol->hDBFieldName)
      {
      tbGlobalFree(lpCol->hDBFieldName);
      lpCol->hDBFieldName = 0;
      lpSS->DataFieldNameCnt--;
      }

   if (lpText && *lpText)
      if (lpCol->hDBFieldName = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                              (lstrlen(lpText) + 1) * sizeof(TCHAR)))
         {
         lpFieldName = (LPTSTR)tbGlobalLock(lpCol->hDBFieldName);
         lstrcpy(lpFieldName, lpText);
         tbGlobalUnlock(lpCol->hDBFieldName);
         lpSS->DataFieldNameCnt++;
         }

   SS_UnlockColItem(lpSS, Col);
   }
 
return (TRUE);
}


TBGLOBALHANDLE SS_GetColFieldName(LPSPREADSHEET lpSS, SS_COORD Col)
{
TBGLOBALHANDLE hFieldName = 0;
LPSS_COL       lpCol;

if (lpCol = SS_LockColItem(lpSS, Col))
   {
   hFieldName = lpCol->hDBFieldName;
   SS_UnlockColItem(lpSS, Col);
   }

return (hFieldName);
}


BOOL SS_SetColDataFillEvent(LPSPREADSHEET lpSS, SS_COORD Col, BYTE bDataFillEvent)
{
LPSS_COL lpCol;

if (Col >= SS_GetColCnt(lpSS))
   return (FALSE);

if (bDataFillEvent)
	lpCol = SS_AllocLockCol(lpSS, Col);
else
	lpCol = SS_LockColItem(lpSS, Col);

if (lpCol)
   {
   lpCol->bDataFillEvent = bDataFillEvent;
   SS_UnlockColItem(lpSS, Col);
   }

return (TRUE);
}


BYTE SS_GetColDataFillEvent(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL lpCol;
BYTE     bRet = 0;

if (lpCol = SS_LockColItem(lpSS, Col))
   {
   bRet = lpCol->bDataFillEvent;
   SS_UnlockColItem(lpSS, Col);
   }

return (bRet);
}


#ifdef SS_V35

short SS_GetMergedColUserSortIndicator(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL lpCol;
short    nSortIndicator = SS_COLUSERSORTINDICATOR_NONE;

if (lpSS)
   nSortIndicator = lpSS->nAllColsSortIndicator;

if (lpCol = SS_LockColItem(lpSS, Col))
   {
   if (lpCol->nSortIndicator != -1)
      nSortIndicator = lpCol->nSortIndicator;

   SS_UnlockColItem(lpSS, Col);
   }  

return (nSortIndicator);
}

short SS_GetColUserSortIndicator(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL lpCol;
short    nSortIndicator = SS_COLUSERSORTINDICATOR_NONE;

if (Col == SS_ALLCOLS)
   nSortIndicator = lpSS->nAllColsSortIndicator;
else
   {
   if (lpCol = SS_LockColItem(lpSS, Col))
      {
      if (lpCol->nSortIndicator != -1)
         nSortIndicator = lpCol->nSortIndicator;
      SS_UnlockColItem(lpSS, Col);
      }
   }
return (nSortIndicator);
}

short DLLENTRY SSGetColUserSortIndicator(HWND hWnd, SS_COORD Col)
{
LPSPREADSHEET lpSS;
short     nSortIndicator;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
nSortIndicator = SS_GetColUserSortIndicator(lpSS, Col);
SS_SheetUnlock(hWnd);

return (nSortIndicator);
}


void SS_SetColUserSortIndicator(LPSPREADSHEET lpSS, SS_COORD Col, short nColUserSortIndicator)
{
LPSS_COL lpCol;

if (SS_COLUSERSORTINDICATOR_MINVAL <= nColUserSortIndicator && nColUserSortIndicator <= SS_COLUSERSORTINDICATOR_MAXVAL)
   {
// 9126, 9044...
   if (Col == -1)
      {
		SS_COORD i;

      lpSS->nAllColsSortIndicator = nColUserSortIndicator;

		for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
			if (lpCol = SS_LockColItem(lpSS, i))
				{
	         if (lpCol->nSortIndicator != SS_COLUSERSORTINDICATOR_DISABLED)
					lpCol->nSortIndicator = -1;
	         SS_UnlockColItem(lpSS, i);
	         }

		SS_InvalidateRow(lpSS, SS_GetColHeadersUserSortRow(lpSS));
      }
   else
// ... 9126, 9044
      { 
      if (Col >= lpSS->Col.HeaderCnt && Col < SS_GetColCnt(lpSS) && (lpCol = SS_AllocLockCol(lpSS, Col)))
         {
         lpCol->nSortIndicator = nColUserSortIndicator;
			SS_InvalidateCell(lpSS, Col, SS_GetColHeadersUserSortRow(lpSS));
         SS_UnlockColItem(lpSS, Col);
         }
      }
   }
}


void DLLENTRY SSSetColUserSortIndicator(HWND hWnd, SS_COORD Col, short nColUserSortIndicator)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
SS_SetColUserSortIndicator(lpSS, Col, nColUserSortIndicator);
SS_SheetUnlock(hWnd);

}

#endif // SS_V35

#ifdef SS_V40

short DLLENTRY SSGetColMerge(HWND hWnd, SS_COORD Col)
{
LPSPREADSHEET lpSS;
short         nMerge;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
nMerge = SS_GetColMerge(lpSS, Col);
SS_SheetUnlock(hWnd);

return (nMerge);
}


short SS_GetColMerge(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL lpCol;
short    nMerge = SS_MERGE_NONE;

if (lpCol = SS_LockColItem(lpSS, Col))
   {
   nMerge = (short)lpCol->bMerge;
   SS_UnlockColItem(lpSS, Col);
   }

return (nMerge);
}


void DLLENTRY SSSetColMerge(HWND hWnd, SS_COORD Col, short nColMerge)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
SS_SetColMerge(lpSS, Col, nColMerge);
SS_SheetUnlock(hWnd);

}


void SS_SetColMerge(LPSPREADSHEET lpSS, SS_COORD Col, short nColMerge)
{
LPSS_COL lpCol;

if (0 <= nColMerge && nColMerge <= SS_MERGE_CNT)
   {
   if (Col < SS_GetColCnt(lpSS) && (lpCol = SS_AllocLockCol(lpSS, Col)))
      {
      lpCol->bMerge = (BYTE)nColMerge;
      SS_InvalidateColRange(lpSS, Col, -1);
      SS_UnlockColItem(lpSS, Col);
      }
   }
}


short DLLENTRY SSGetColID(HWND hWnd, SS_COORD Col, LPTSTR ColID)
{
LPSPREADSHEET lpSS;
short         nRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
nRet = SS_GetColID(lpSS, Col, ColID);
SS_SheetUnlock(hWnd);

return (nRet);
}


short SS_GetColID(LPSPREADSHEET lpSS, SS_COORD Col, LPTSTR ColID)
{
LPSS_COL lpCol;
BOOL     nRet = 0;

if (lpCol = SS_LockColItem(lpSS, Col))
	{
	TBGLOBALHANDLE hColID = lpCol->hColID;   

	if (hColID)
		{
		LPTSTR lpszColID = (LPTSTR)tbGlobalLock(hColID);
		nRet = lstrlen(lpszColID);
		if (nRet && ColID)
			_tcscpy(ColID, lpszColID);     

		tbGlobalUnlock(hColID);  
		}

	SS_UnlockColItem(lpSS, Col);
	}

return nRet;
}


BOOL DLLENTRY SSSetColID(HWND hWnd, SS_COORD Col, LPCTSTR ColID)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, NULL);
bRet = SS_SetColID(lpSS, Col, ColID);
SS_SheetUnlock(hWnd);

return (bRet);
}


BOOL SS_SetColID(LPSPREADSHEET lpSS, SS_COORD Col, LPCTSTR ColID)
{
LPSS_COL lpCol;
BOOL     fRet = FALSE;

if (lpCol = SS_AllocLockCol(lpSS, Col))
   {
   LPTSTR lpsz;
   if (lpCol->hColID)
      tbGlobalFree(lpCol->hColID);
   lpCol->hColID = tbGlobalAlloc(GHND, (lstrlen(ColID)+1)*sizeof(TCHAR));
   lpsz = (LPTSTR)tbGlobalLock(lpCol->hColID);
   _tcscpy(lpsz, ColID);
   tbGlobalUnlock(lpCol->hColID);
   SS_UnlockColItem(lpSS, Col);
   fRet = TRUE;
   } 

return (fRet);
}


SS_COORD DLLENTRY SSGetColFromID(HWND hWnd, LPCTSTR lpszColID)
{
LPSPREADSHEET lpSS;
SS_COORD      lRet;

lpSS = SS_SheetLock(hWnd);
lRet = SS_GetColFromID(lpSS, lpszColID);
SS_AdjustCellCoordsOut(lpSS, &lRet, NULL);
SS_SheetUnlock(hWnd);

return (lRet);
}

SS_COORD SS_GetColFromID(LPSPREADSHEET lpSS, LPCTSTR lpszColID)
{
LPSS_COL lpCol;
SS_COORD Col;
SS_COORD lRet = -1;

if (lpszColID && *lpszColID)
	{
   for (Col = 0; lRet == -1 && Col < lpSS->Col.AllocCnt; Col++)
      if (lpCol = SS_LockColItem(lpSS, Col))
         {
		   if (lpCol->hColID)
				{
				LPTSTR lpsz = (LPTSTR)tbGlobalLock(lpCol->hColID);
				if (lstrcmp(lpsz, lpszColID) == 0)
					lRet = Col;
				tbGlobalUnlock(lpCol->hColID);
				}

         SS_UnlockColItem(lpSS, Col);
         }
	}

return (lRet);
}

#endif // SS_V40

void SS_UnhideActiveCol(LPSPREADSHEET lpSS, SS_COORD Col)
{
if (lpSS->Col.CurAt == Col)
	{
	lpSS->lpBook->fSuspendExit = TRUE;
	SS_ScrollLineLR(lpSS, SS_F_SCROLL_RIGHT | SS_F_MOVEACTIVECELL | SS_F_NOLEAVECELL);
	if (lpSS->Col.CurAt == Col)
		SS_ScrollLineUL(lpSS, SS_F_SCROLL_LEFT | SS_F_MOVEACTIVECELL | SS_F_NOLEAVECELL);
	lpSS->lpBook->fSuspendExit = FALSE;
	}

// This code will hide the hidden col if it is the first col

if (lpSS->Col.UL == Col)
	{
	lpSS->lpBook->fSuspendExit = TRUE;
	SS_ScrollLineLR(lpSS, SS_F_SCROLL_RIGHT | SS_F_NOLEAVECELL);
	if (lpSS->Col.UL == Col)
		SS_ScrollLineUL(lpSS, SS_F_SCROLL_LEFT | SS_F_NOLEAVECELL);
	lpSS->lpBook->fSuspendExit = FALSE;
	SS_SetHScrollBar(lpSS);
	}
}
