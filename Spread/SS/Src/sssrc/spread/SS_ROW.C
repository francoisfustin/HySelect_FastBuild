/*********************************************************
* SS_ROW.C
*
* Copyright (C) 1991-2001 - FarPoint Technologies
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
#include "ss_user.h"
#include "ss_win.h"
#ifdef SS_V40
#include "ss_span.h"
#endif
#ifdef SS_V80
#include "ss_type.h"
#endif

BOOL DLLENTRY SSSetRowHeight(HWND hWnd, SS_COORD Row, double dfHeight)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
bRet = SS_SetRowHeight(lpSS, Row, dfHeight);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_SetRowHeight(LPSPREADSHEET lpSS, SS_COORD Row, double dfHeight)
{
return (SSx_SetRowHeight(lpSS, Row, dfHeight, FALSE));
}


BOOL SSx_SetRowHeight(LPSPREADSHEET lpSS, SS_COORD Row, double dfHeight, BOOL fUserAction)
{
LPSS_ROW      lpRow;
SS_COORD      i;
long          HeightOld = -2;
long          dRowHeightX100;
WORD          wMessageBeingSent;
BOOL          fTurnEditModeOn = FALSE;
BOOL          fRet = TRUE;

if (dfHeight < 0.0 && dfHeight != -1.0)
	return (FALSE);

if (lpSS->lpBook->EditModeOn)
   {
   wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;
   lpSS->lpBook->wMessageBeingSent = 0;
   SS_CellEditModeOff(lpSS, 0);
   fTurnEditModeOn = TRUE;
   }

if (dfHeight == -1)
   dRowHeightX100 = -1;
else
   dRowHeightX100 = (long)(dfHeight * 100.0 + 0.5);

if (Row == -1)
   {
   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
      if (lpRow = SS_LockRowItem(lpSS, i))
         {
			// RFW - 6/1/04 - 14501
			if (!fUserAction || (SS_GetMergedUserResizeRow(lpSS, i) != SS_RESIZE_OFF))
				{
				lpRow->dRowHeightX100 = SS_WIDTH_DEFAULT;
				lpRow->dRowHeightInPixels = SS_WIDTH_DEFAULT;

				// RFW - 10-5-02
				// if (lpRow->FontId != SS_FONT_NONE)
				if (lpRow->FontId != SS_FONT_NONE && dfHeight == -1)
					lpRow->dRowHeightInPixels = SS_CalcRowHeight(lpSS, i);
				}

			SS_UnlockRowItem(lpSS, i);
         }

   HeightOld = lpSS->Row.dCellSizeX100;
   lpSS->Row.dCellSizeX100 = dRowHeightX100;

   /*
   if (lpSS->Row.dCellSizeX100 == SS_WIDTH_DEFAULT)
      lpSS->Row.dCellSizeInPixels = SS_WIDTH_DEFAULT;
   else
      lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, Row, dfHeight);
   */
   lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, Row, dfHeight);
   }

else
   {
   if (Row >= SS_GetRowCnt(lpSS))
      return (FALSE);

   if (lpRow = SS_AllocLockRow(lpSS, Row))
      {
		short dRowHeightInPixels;

      if (dRowHeightX100 == -1 && lpRow->FontId == SS_FONT_NONE)
         dRowHeightInPixels = SS_WIDTH_DEFAULT;
      else
			/* RFW - 1/6/05 - 15427
         dRowHeightInPixels = SS_RowHeightToPixels(lpSS, Row, dfHeight);
			*/
         dRowHeightInPixels = SS_RowHeightToPixels(lpSS, Row, dRowHeightX100 == SS_HEIGHT_DEFAULT ? -1.0 :
                                                   (double)dRowHeightX100 / 100.0);

      HeightOld = lpRow->dRowHeightX100;

		/* RFW - 8/15/06 - 19391
		// If the size is unchanged or the original value was unset and the
      // user is trying to set it to the default value, then leave it unset.
      if (lpRow->dRowHeightX100 == dRowHeightX100 ||
          (lpRow->dRowHeightX100 == -1 &&
           dRowHeightInPixels == lpSS->Row.dCellSizeInPixels))
			{
			// RFW - 5/2/06 - 18838
			lpRow->dRowHeightX100 = SS_WIDTH_DEFAULT;
			lpRow->dRowHeightInPixels = SS_WIDTH_DEFAULT;

			fRet = FALSE;
			}
		*/

		// If the size is unchanged then leave it unset.
      if (lpRow->dRowHeightX100 == dRowHeightX100)
			fRet = FALSE;

		// If the original value was unset and the user is trying
      // to set it to the default value, then leave it unset.
      else if (lpRow->dRowHeightX100 == -1 &&
               dRowHeightInPixels == lpSS->Row.dCellSizeInPixels)
			{
			// RFW - 5/2/06 - 18838
			lpRow->dRowHeightX100 = SS_WIDTH_DEFAULT;
			lpRow->dRowHeightInPixels = SS_WIDTH_DEFAULT;

			fRet = FALSE;
			}

		else
         {
         lpRow->dRowHeightX100 = dRowHeightX100;
         lpRow->dRowHeightInPixels = dRowHeightInPixels;
         }

      SS_UnlockRowItem(lpSS, Row);
      }
   }

if ((HeightOld != -2 && HeightOld != dfHeight) || Row == -1)
   {
   if (!SS_AutoSize(lpSS->lpBook, FALSE))
      {
      if (Row == -1)
         SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

      else if (SS_IsRowVisible(lpSS, Row, SS_VISIBLE_PARTIAL))
         SS_InvalidateRowRange(lpSS, Row, -1);
      }
   }

// RFW - 7/29/03 - I removed the else
if (Row >= lpSS->Row.HeaderCnt && dfHeight == 0.0 && lpSS->Row.CurAt == Row)
   {
	// RFW - 7/21/03 - 10961
	// I am very nervous about changing this since it has never fired the LeaveCell event
   //SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN | SS_F_MOVEACTIVECELL | SS_F_NOLEAVECELL);
   SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN | SS_F_MOVEACTIVECELL);
   if (lpSS->Row.CurAt == Row)
      //SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP | SS_F_MOVEACTIVECELL | SS_F_NOLEAVECELL);
      SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP | SS_F_MOVEACTIVECELL);
   }

// RFW - 7/23/03
if (Row >= lpSS->Row.HeaderCnt && dfHeight == 0.0 && lpSS->Row.UL == Row)
	{
   for (; Row < SS_GetRowCnt(lpSS) - 1 && SS_GetRowHeightInPixels(lpSS, Row) == 0; Row++)
		;

	lpSS->Row.UL = Row;
	}

if (fTurnEditModeOn)
   {
   SS_CellEditModeOn(lpSS, 0, 0, 0L);
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;
   }

return (fRet);
}


BOOL DLLENTRY SSSetRowHeightInPixels(HWND hWnd, SS_COORD Row, int dHeight)
{
double dfHeight;

SSLogUnitsToRowHeight(hWnd, Row, dHeight, &dfHeight);
return (SSSetRowHeight(hWnd, Row, dfHeight));
}


BOOL DLLENTRY SSGetRowHeight(hWnd, Row, lpdfRowHeight)

HWND          hWnd;
SS_COORD      Row;
LPDOUBLE      lpdfRowHeight;
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
fRet = SS_GetRowHeight(lpSS, Row, lpdfRowHeight);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetRowHeight(lpSS, Row, lpdfRowHeight)

LPSPREADSHEET lpSS;
SS_COORD      Row;
LPDOUBLE      lpdfRowHeight;
{
BOOL          fRet = FALSE;

if (Row < SS_GetRowCnt(lpSS))
   {
   SS_CalcPixelsToRowHeight(lpSS, Row, SS_GetRowHeightInPixels(lpSS,
                            Row), lpdfRowHeight);
   fRet = TRUE;
   }

return (fRet);
}


BOOL DLLENTRY SSGetRowHeightInPixels(HWND hWnd, SS_COORD Row,
                                     LPINT lpdHeightPixels)
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);

if (Row < SS_GetRowCnt(lpSS))
   {
   *lpdHeightPixels = SS_GetRowHeightInPixels(lpSS, Row);
   fRet = TRUE;
   }

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetRowLock(lpSS, Row, Lock)

LPSPREADSHEET lpSS;
SS_COORD      Row;
BOOL          Lock;
{
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
SS_COORD      i;

for (i = 0; i < lpSS->Col.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, i, Row))
      {
      lpCell->CellLocked = SS_LOCKED_DEF;
      SS_UnlockCellItem(lpSS, i, Row);
      }

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
   lpRow->RowLocked = Lock;
   SS_UnlockRowItem(lpSS, Row);

   if (lpSS->LockColor.ForegroundId || lpSS->LockColor.BackgroundId)
      SS_InvalidateRow(lpSS, Row);
   }

return (TRUE);
}


BOOL DLLENTRY SSInsRow(hWnd, Row)

HWND             hWnd;
SS_COORD         Row;
{
return (SSInsRowRange(hWnd, Row, Row));
}


BOOL DLLENTRY SSInsRowRange(hWnd, Row, Row2)

HWND             hWnd;
SS_COORD         Row;
SS_COORD         Row2;
{
LPSPREADSHEET    lpSS;
BOOL             fRet;

lpSS = SS_SheetLock(hWnd);

if (Row == SS_FIRST)
   Row = lpSS->Row.HeaderCnt;
else if (Row == SS_LAST)
   Row = lpSS->Row.AllocCnt;
else
	SS_AdjustCellCoords(lpSS, NULL, &Row);

if (Row <= 0)
   {
   SS_SheetUnlock(hWnd);
   return (FALSE);
   }

if (Row2 == SS_FIRST)
   Row2 = lpSS->Row.HeaderCnt;
else if (Row2 == SS_LAST)
   Row2 = lpSS->Row.AllocCnt;
else
	SS_AdjustCellCoords(lpSS, NULL, &Row2);

if (Row2 <= 0)
   {
   SS_SheetUnlock(hWnd);
   return (FALSE);
   }

fRet = SS_InsRowRange(lpSS, Row, Row2);

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_InsRowRange(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2)
{
#ifdef SS_OLDCALC
LPTBGLOBALHANDLE CalcTable;
LPSS_CALC        lpCalc;
#endif
BOOL             fRet;
SS_COORD         i;
BOOL             fVisible = FALSE;

//BJO 2003Aug11 bug #11870 - quick exit skips update of custom names
//if (Row >= lpSS->Row.AllocCnt || Row >= SS_GetRowCnt(lpSS))
//   return (TRUE);

if (Row > Row2)
   return (FALSE);

SS_CellEditModeOff(lpSS, 0);

for (i = SS_GetRowCnt(lpSS) - 1 - (Row2 - Row);
     i <= SS_GetRowCnt(lpSS) - 1; i++)
   SS_DeAllocRow(lpSS, i, FALSE);

if (fRet = SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolRow,
                             &lpSS->Row.Items, Row, lpSS->Row.AllocCnt - 1,
                             Row2 + 1, SS_GetRowCnt(lpSS)))
   {
   if (Row < lpSS->Row.DataCnt)
      SS_SetDataRowCnt(lpSS, min(SS_GetRowCnt(lpSS),
                       lpSS->Row.DataCnt + (Row2 - Row + 1)));

   if (Row < lpSS->Row.AllocCnt)
      lpSS->Row.AllocCnt = min(SS_GetRowCnt(lpSS),
                             lpSS->Row.AllocCnt + (Row2 - Row + 1));

   lpSS->Row.LastRefCoord.Coord.Row = -1;
   lpSS->LastCell.Coord.Col = -1;
   lpSS->LastCell.Coord.Row = -1;

   #ifdef SS_OLDCALC
   if (lpSS->hCalcTable)
      {
      CalcTable = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->hCalcTable);
      for (i = 0; i < lpSS->CalcTableCnt; i++)
         {
         lpCalc = (LPSS_CALC)tbGlobalLock(CalcTable[i]);

         if (lpCalc->CellCoord.Row >= Row)
            lpCalc->CellCoord.Row += Row2 - Row + 1;

         tbGlobalUnlock(CalcTable[i]);
         }

      tbGlobalUnlock(lpSS->hCalcTable);
      }
   #endif

   if (lpSS->fVirtualMode)
      {
      if (Row <= lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1 &&
          Row2 >= lpSS->Virtual.VTop)
         lpSS->Virtual.VPhysSize +=
            min(Row2, lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1) -
            max(Row, lpSS->Virtual.VTop) + 1;
      }

   for (i = Row, fVisible = FALSE; i <= Row2 && !fVisible; i++)
      if (SS_IsRowVisible(lpSS, i, SS_VISIBLE_PARTIAL))
         fVisible = TRUE;

#ifdef SS_V40
   SS_InsCellSpanRowRange(lpSS, Row, Row2 - Row + 1);
#endif // SS_V40

	/* RFW - 1/4/05 - 15241
	// RFW - 6/21/04 - 14600 
	// Adjust ActiveCell and UpperLeft
	if (Row <= lpSS->Row.CurAt)
		{
		lpSS->Row.CurAt += Row2 - Row + 1;
		lpSS->Row.CurAt = min(lpSS->Row.CurAt, SS_GetRowCnt(lpSS) - 1);
		}
	*/

	if (Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen && Row < lpSS->Row.UL)
		{
		lpSS->Row.UL += Row2 - Row + 1;
		lpSS->Row.UL = min(lpSS->Row.UL, SS_GetRowCnt(lpSS) - 1);
		lpSS->Row.LR = SS_GetBottomCell(lpSS, lpSS->Row.UL);
		// RFW - 12/27/04 - 15360
		lpSS->Row.LRAllVis = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);
		}

	// RFW - 1/5/05 - 15193
	if (!SS_USESELBAR(lpSS))
		SS_DeSelectBlock(lpSS);
	/*
	// RFW - 6/10/04 - 14021-3
	SS_DeSelectBlock(lpSS);
	*/

   if (fVisible)
      /*
      SS_VScrollRow(lpSS, Row, TRUE, TRUE);
      */
      SS_InvalidateRowRange(lpSS, Row, -1);
   else if (Row < lpSS->Row.UL)
      SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ROWS);
   }

#ifdef SS_OLDCALC
SS_BuildDependencies(lpSS);
#elif !defined(SS_NOCALC)
if (Row >= lpSS->Row.HeaderCnt)
	{
	SS_COORD lRowOut = Row;
	SS_COORD lRow2Out = Row2;

	SS_AdjustCellCoordsOut(lpSS, NULL, &lRowOut);
	SS_AdjustCellCoordsOut(lpSS, NULL, &lRow2Out);
	CalcInsRowRange(&lpSS->CalcInfo, lRowOut, lRow2Out);
	}
if (lpSS->lpBook->CalcAuto)
   CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif


#ifdef SS_UTP
if (Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
   SS_SendMsgInsDel(hWnd, SS_INSDEL_TYPE_INS, SS_INSDEL_OP_POST, SS_INSDEL_ROW,
                    Row, Row2);
#endif

SS_HighlightCell(lpSS, TRUE);
return (fRet);
}


BOOL DLLENTRY SSDelRow(hWnd, Row)

HWND             hWnd;
SS_COORD         Row;
{
return (SSDelRowRange(hWnd, Row, Row));
}


BOOL DLLENTRY SSDelRowRange(hWnd, Row, Row2)

HWND          hWnd;
SS_COORD      Row;
SS_COORD      Row2;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
fRet = SSxDelRowRange(lpSS, Row, Row2);
SS_SheetUnlock(hWnd);

return (fRet);
}


BOOL SSxDelRowRange(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2)
{
BOOL fRet;

if (Row == SS_FIRST)
   Row = lpSS->Row.HeaderCnt;
else if (Row == SS_LAST)
   Row = lpSS->Row.AllocCnt - 1;
else
	SS_AdjustCellCoords(lpSS, NULL, &Row);

if (Row <= 0)
   return (FALSE);

if (Row2 == SS_FIRST)
   Row2 = lpSS->Row.HeaderCnt;
else if (Row2 == SS_LAST)
   Row2 = lpSS->Row.AllocCnt - 1;
else
	SS_AdjustCellCoords(lpSS, NULL, &Row2);

if (Row2 <= 0)
   return (FALSE);

fRet = SS_DelRowRange(lpSS, Row, Row2);
return (fRet);
}


BOOL SS_DelRowRange(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2)
{
LPTBGLOBALHANDLE lpList;
#ifdef SS_OLDCALC
LPTBGLOBALHANDLE CalcTable;
LPSS_CALC        lpCalc;
#endif
BOOL             fRet = TRUE;
SS_COORD         i;
BOOL             fVisible = FALSE;

//BJO 2003Aug11 bug #11870 - quick exit skips update of custom names
//if (Row >= lpSS->Row.AllocCnt || Row >= SS_GetRowCnt(lpSS))
//   return (FALSE);
//
//Row2 = min(Row2, lpSS->Row.AllocCnt - 1);

if (Row > Row2)
   return (FALSE);

SS_CellEditModeOff(lpSS, 0);

if (lpSS->lpBook->fAllowUndo)
   {
   SS_ClearUndoBuffer(lpSS);

   if (lpSS->UndoBuffer.hData = tbGlobalAlloc(GMEM_MOVEABLE |
                                    GMEM_ZEROINIT,
                                    (long)sizeof(TBGLOBALHANDLE) *
                                    (Row2 - Row + 1)))
      {
      lpList = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->UndoBuffer.hData);

      for (i = Row; i <= Row2; i++)
         lpList[i - Row] = SS_GrabItemHandle(lpSS, &lpSS->Row.Items, i,
                                             SS_GetRowCnt(lpSS));

      tbGlobalUnlock(lpSS->UndoBuffer.hData);
      }

   lpSS->UndoBuffer.bUndoType = SS_UNDOTYPE_DELROW;
   lpSS->UndoBuffer.ItemAllocCnt = lpSS->Col.AllocCnt;
   lpSS->UndoBuffer.Row = Row;
   lpSS->UndoBuffer.Row2 = Row2;
   }

else
   for (i = Row; i <= Row2; i++)
      SS_DeAllocRow(lpSS, i, FALSE);

SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolRow, &lpSS->Row.Items, Row2 + 1,
                  lpSS->Row.AllocCnt - 1, Row, SS_GetRowCnt(lpSS));

lpSS->Row.LastRefCoord.Coord.Row = -1;
lpSS->LastCell.Coord.Col = -1;
lpSS->LastCell.Coord.Row = -1;

#ifdef SS_OLDCALC
if (lpSS->hCalcTable)
   {
   CalcTable = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->hCalcTable);
   for (i = 0; i < lpSS->CalcTableCnt; i++)
      {
      lpCalc = (LPSS_CALC)tbGlobalLock(CalcTable[i]);

      if (lpCalc->CellCoord.Row > Row)
         lpCalc->CellCoord.Row -= Row2 - Row + 1;

      tbGlobalUnlock(CalcTable[i]);
      }

   tbGlobalUnlock(lpSS->hCalcTable);
   }
#endif

if (lpSS->fVirtualMode)
   {
   if (Row <= lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1 &&
       Row2 >= lpSS->Virtual.VTop)
      lpSS->Virtual.VPhysSize -=
         min(Row2, lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize - 1) -
         max(Row, lpSS->Virtual.VTop) + 1;
   }

for (i = Row, fVisible = FALSE; i <= Row2 && !fVisible; i++)
   if (SS_IsRowVisible(lpSS, i, SS_VISIBLE_PARTIAL))
      fVisible = TRUE;

// RFW - 4/27/00 - SEL5028
//if (Row < lpSS->Row.DataCnt - 1 && Row2 < lpSS->Row.DataCnt - 1)
if (Row < lpSS->Row.DataCnt && Row2 < lpSS->Row.DataCnt)
   SS_SetDataRowCnt(lpSS, max(lpSS->Row.DataCnt -
                    min(Row2 - Row + 1, lpSS->Row.DataCnt - Row), 1));

if (Row < lpSS->Row.AllocCnt)
   lpSS->Row.AllocCnt = max(lpSS->Row.AllocCnt - min(Row2 - Row + 1,
                          lpSS->Row.AllocCnt - Row), 1);

#ifdef SS_V40
SS_DelCellSpanRowRange(lpSS, Row, Row2 - Row + 1);
#endif // SS_V40

#ifdef SS_OLDCALC
SS_BuildDependencies(lpSS);
#elif !defined(SS_NOCALC)
if (Row >= lpSS->Row.HeaderCnt)
	{
	SS_COORD lRowOut = Row;
	SS_COORD lRow2Out = Row2;

	SS_AdjustCellCoordsOut(lpSS, NULL, &lRowOut);
	SS_AdjustCellCoordsOut(lpSS, NULL, &lRow2Out);
	CalcDelRowRange(&lpSS->CalcInfo, lRowOut, lRow2Out);
	}
if (lpSS->lpBook->CalcAuto)
   CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif

if (Row < lpSS->Row.HeaderCnt)
	lpSS->Row.HeaderCnt -= min(Row2, lpSS->Row.HeaderCnt) - Row + 1;

// RFW - 4/15/04 - 14021 
// Adjust ActiveCell and UpperLeft
if (Row < lpSS->Row.CurAt)
	{
	/* RFW - 1/4/05 - 15241
	lpSS->Row.CurAt -= min(lpSS->Row.CurAt - 1, Row2) - Row + 1;
	*/
	lpSS->Row.CurAt = max(lpSS->Row.HeaderCnt, lpSS->Row.CurAt);
	}

if (Row < lpSS->Row.UL)
	{
	lpSS->Row.UL -= min(lpSS->Row.UL - 1, Row2) - Row + 1;
	lpSS->Row.UL = max(lpSS->Row.HeaderCnt + lpSS->Row.Frozen, lpSS->Row.UL);
	}

SS_AdjustDataColCnt(lpSS, -1, -1);
SS_AdjustDataRowCnt(lpSS, Row, Row2);

// RFW - 1/5/05 - 15193
if (!SS_USESELBAR(lpSS))
	SS_DeSelectBlock(lpSS);
/*
// RFW - 6/10/04 - 14021-3
SS_DeSelectBlock(lpSS);
*/

if (fVisible)
   /*
   SS_VScrollRow(lpSS, Row, FALSE, TRUE);
   */
   SS_InvalidateRowRange(lpSS, Row, -1);
else if (Row < lpSS->Row.UL)
   SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ROWS);

SS_HighlightCell(lpSS, TRUE);

return (fRet);
}


BOOL SS_ClearRow(LPSPREADSHEET lpSS, SS_COORD Row, BOOL Paint,
                 BOOL fSendChangeMsg)
{
if (Row >= lpSS->Row.AllocCnt || Row >= SS_GetRowCnt(lpSS))
   {
   return (FALSE);
   }

SS_DeAllocRow(lpSS, Row, fSendChangeMsg);

if (Paint && SS_IsRowVisible(lpSS, Row, SS_VISIBLE_PARTIAL))
   SS_InvalidateRowRange(lpSS, Row, -1);

return (TRUE);
}


BOOL SSIsRowVisible(hWnd, Row, Visible)

HWND          hWnd;
SS_COORD      Row;
short         Visible;
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);

SS_AdjustCellCoords(lpSS, NULL, &Row);

if (SS_GetRowHeightInPixels(lpSS, Row))
	fRet = SS_IsRowVisible(lpSS, Row, Visible);

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_IsRowVisible(LPSPREADSHEET lpSS, SS_COORD Row, short Visible)
{
BOOL fRet = FALSE;

switch (Visible)
   {
   case SS_VISIBLE_ALL:
      if (Row < lpSS->Row.HeaderCnt + lpSS->Row.Frozen ||
			/* RFW - 12/27/04 - 15360
          (Row >= lpSS->Row.UL && Row <= SS_GetBottomVisCell(lpSS, lpSS->Row.UL)))
			*/
			/* RFW - 3/9/05 - 15880
          (Row >= lpSS->Row.UL && Row <= (lpSS->fVirtualMode ?
           SS_GetBottomVisCell(lpSS, lpSS->Row.UL) : lpSS->Row.LRAllVis)))
			*/
          (Row >= lpSS->Row.UL && Row <= ((lpSS->fVirtualMode || lpSS->Row.LRAllVis == -1) ?
           SS_GetBottomVisCell(lpSS, lpSS->Row.UL) : lpSS->Row.LRAllVis)))
         fRet = TRUE;

      break;

   case SS_VISIBLE_PARTIAL:
      if (Row < lpSS->Row.HeaderCnt + lpSS->Row.Frozen ||
          (Row >= lpSS->Row.UL && Row <= (lpSS->fVirtualMode ?
           SS_GetBottomCell(lpSS, lpSS->Row.UL) : lpSS->Row.LR)))
         fRet = TRUE;

      break;
   }

return (fRet);
}


/*
SS_COORD SS_IsRowVisible(LPSPREADSHEET lpSS, SS_COORD Row, short Visible)
{
RECT     Rect;
SS_COORD RowAt;
SS_COORD RowTop = lpSS->Row.UL;
short    y;
short    cy;
BOOL     fRet = FALSE;

if (Row < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
	return (TRUE);

if (Row >= lpSS->Row.UL)
	{
	SS_GetClientRect(lpSS->lpBook, &Rect);

	y = SS_GetCellPosY(lpSS, RowTop, RowTop);
	for (RowAt = RowTop; RowAt <= Row &&
		  y + cy <= Rect.bottom - Rect.top; RowAt++)
		{
		cy = SS_GetRowHeightInPixels(lpSS, RowAt);

		if (RowAt == Row)
			if (Visible == SS_VISIBLE_PARTIAL || y + cy <= Rect.bottom - Rect.top)
				fRet = TRUE;

		y += cy;
		}
	}

return (fRet);
}
*/


BOOL SS_SetRowFont(lpSS, Row, hFont, DeleteFont, lpfDeleteFontObject)

LPSPREADSHEET lpSS;
SS_COORD      Row;
HFONT         hFont;
BOOL          DeleteFont;
LPBOOL        lpfDeleteFontObject;
{
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
SS_COORD      i;
BOOL          fTurnEditModeOn = FALSE;

if (lpSS->lpBook->EditModeOn && !lpSS->lpBook->fLeaveEditModeOn && Row == lpSS->Row.CurAt)
   {
   WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

   lpSS->lpBook->wMessageBeingSent = FALSE;
   SS_CellEditModeOff(lpSS, 0);
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;

   fTurnEditModeOn = TRUE;
   }

for (i = 0; i < lpSS->Col.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, i, Row))
      lpCell->FontId = SS_FONT_NONE;

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
   BOOL fRowHeightChanged;

   // 26551 -scl
	//lpRow->dRowHeightX100 = SS_WIDTH_DEFAULT;
   //lpRow->dRowHeightInPixels = SS_WIDTH_DEFAULT;
   lpRow->FontId = SS_InitFont(lpSS->lpBook, hFont, DeleteFont, TRUE, lpfDeleteFontObject);
   fRowHeightChanged = SS_SetRowMaxFont(lpSS, lpRow, Row, lpRow->FontId);
   SS_UnlockRowItem(lpSS, Row);

   if (lpSS->lpBook->fAllowCellOverflow)
      for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.DataCnt; i++)
         SS_OverflowAdjustNeighbor(lpSS, i, Row);

   if (SS_IsRowVisible(lpSS, Row, SS_VISIBLE_PARTIAL))
      SS_InvalidateRowRange(lpSS, Row, fRowHeightChanged ? -1 : Row);
   }

if (fTurnEditModeOn)
   {
   SS_UpdateWindow(lpSS->lpBook);
   SS_CellEditModeOn(lpSS, 0, 0, 0L);
   }

return (TRUE);
}

#if SS_V80
BOOL SS_SetRowLogFont(LPSPREADSHEET lpSS, SS_COORD Row, LOGFONT *pLogFont)

{
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
SS_COORD      i;
BOOL          fTurnEditModeOn = FALSE;

if (lpSS->lpBook->EditModeOn && !lpSS->lpBook->fLeaveEditModeOn && Row == lpSS->Row.CurAt)
   {
   WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

   lpSS->lpBook->wMessageBeingSent = FALSE;
   SS_CellEditModeOff(lpSS, 0);
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;

   fTurnEditModeOn = TRUE;
   }

for (i = 0; i < lpSS->Col.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, i, Row))
      lpCell->FontId = SS_FONT_NONE;

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
   BOOL fRowHeightChanged;

	lpRow->dRowHeightX100 = SS_WIDTH_DEFAULT;
   lpRow->dRowHeightInPixels = SS_WIDTH_DEFAULT;
   lpRow->FontId = SS_InitFontLogFont(lpSS->lpBook, pLogFont);
   fRowHeightChanged = SS_SetRowMaxFont(lpSS, lpRow, Row, lpRow->FontId);
   SS_UnlockRowItem(lpSS, Row);

   if (lpSS->lpBook->fAllowCellOverflow)
      for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.DataCnt; i++)
         SS_OverflowAdjustNeighbor(lpSS, i, Row);

   if (SS_IsRowVisible(lpSS, Row, SS_VISIBLE_PARTIAL))
      SS_InvalidateRowRange(lpSS, Row, fRowHeightChanged ? -1 : Row);
   }

if (fTurnEditModeOn)
   {
   SS_UpdateWindow(lpSS->lpBook);
   SS_CellEditModeOn(lpSS, 0, 0, 0L);
   }

return (TRUE);
}
#endif

BOOL SS_SetRowCellType(LPSPREADSHEET lpSS, SS_COORD Row,
                       LPSS_CELLTYPE CellType)
{
TBGLOBALHANDLE hData;
LPSS_CELLTYPE  CellTypeTemp;
LPSS_ROW       lpRow;
LPSS_CELL      lpCell;
SS_DATA        Data;
LPTSTR         lpData;
SS_COORD       i;
BOOL           fSetData = FALSE;

for (i = 0; i < lpSS->Col.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, i, Row))
      {
      if (lpCell->hCellType)
         {
         SS_DeAllocCellType(lpSS, lpCell->hCellType);
         lpCell->hCellType = 0;
         }
      SS_UnlockCellItem(lpSS, Row, i);
      }

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
   SS_DeAllocCellType(lpSS, lpRow->hCellType);
   lpRow->hCellType = 0;

   if (CellType)
      {
      if (!(lpRow->hCellType = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                             (long)sizeof(SS_CELLTYPE))))
         {
         SS_UnlockRowItem(lpSS, Row);
         return (FALSE);
         }

      if ((lpRow->Data.bDataType == SS_TYPE_FLOAT &&
           !SS_IsCellTypeFloatNum(CellType)) ||
          (CellType->Type == SS_TYPE_COMBOBOX &&
           CellType->Spec.ComboBox.hItems))
         fSetData = TRUE;

      CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpRow->hCellType);
      _fmemcpy(CellTypeTemp, CellType, sizeof(SS_CELLTYPE));
#if SS_V80
      SS_CT_Ref(lpSS, CellTypeTemp);
#endif
      tbGlobalUnlock(lpRow->hCellType);
      }
   
   else if (lpRow->Data.bDataType == SS_TYPE_FLOAT)
      fSetData = TRUE;

   if (fSetData)
      {
      if (hData = SS_GetData(lpSS, CellType, -1, Row, FALSE))
         {
         lpData = (LPTSTR)tbGlobalLock(hData);
         SS_SetDataRange(lpSS, -1, Row, -1, Row, lpData, FALSE, FALSE, FALSE);
         tbGlobalUnlock(hData);
         }
      else
         lpRow->Data.bDataType = SS_TYPE_EDIT;
      }

   for (i = 0; i < lpSS->Col.AllocCnt; i++)
      {
      if (lpCell = SS_LockCellItem(lpSS, i, Row))
			{
			if (SS_IsCellTypeFloatNum(CellType) &&
				 SS_GetDataStruct(lpSS, NULL, lpRow, NULL, &Data, i, Row) &&
				 Data.bDataType == SS_TYPE_FLOAT)
				;  // No need to reformat data
			else if (hData = SS_GetData(lpSS, CellType, i, Row, FALSE))
				{
				lpData = (LPTSTR)tbGlobalLock(hData);
				SS_SetDataRange(lpSS, i, Row, i, Row, lpData, FALSE, FALSE, FALSE);
				tbGlobalUnlock(hData);
				}
			else
				lpCell->Data.bDataType = SS_TYPE_EDIT;

			SS_UnlockCellItem(lpSS, i, Row);
			}
      }

   SS_UnlockRowItem(lpSS, Row);
   SS_InvalidateRow(lpSS, Row);
   }

return (TRUE);
}


void SS_SetDataRowCnt(LPSPREADSHEET lpSS, SS_COORD RowCnt)
{
if (lpSS->Row.MaxBeforeSort == -1 || RowCnt <= lpSS->Row.MaxBeforeSort + 1)
   lpSS->Row.DataCnt = RowCnt;
}


BOOL SS_SetRowColor(lpSS, Row, Background, Foreground)

LPSPREADSHEET lpSS;
SS_COORD      Row;
COLORREF      Background;
COLORREF      Foreground;
{
LPSS_ROW      lpRow;
LPSS_CELL     lpCell;
SS_COORD      i;

for (i = 0; i < lpSS->Col.AllocCnt; i++)
   if (lpCell = SS_LockCellItem(lpSS, i, Row))
      SS_ClearColorItem(&lpCell->Color, Background, Foreground);

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
//   if (!SS_ISDEFCOLOR(Background))
   if (Background != SPREAD_COLOR_IGNORE)
      lpRow->Color.BackgroundId = SS_AddColor(Background);

//   if (!SS_ISDEFCOLOR(Foreground))
   if (Foreground != SPREAD_COLOR_IGNORE)
      lpRow->Color.ForegroundId = SS_AddColor(Foreground);

   SS_UnlockRowItem(lpSS, Row);
   }

SS_InvalidateRow(lpSS, Row);
return (TRUE);
}


#ifndef SS_NOUSERDATA
#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSSetRowUserData(HWND hWnd, SS_COORD Row, LONG_PTR lUserData)
#else
BOOL DLLENTRY SSSetRowUserData(HWND hWnd, SS_COORD Row, long lUserData)
#endif
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
bRet = SS_SetRowUserData(lpSS, Row, lUserData);
SS_SheetUnlock(hWnd);
return bRet;
}

#if defined(_WIN64) || defined(_IA64)
BOOL SS_SetRowUserData(LPSPREADSHEET lpSS, SS_COORD Row, LONG_PTR lUserData)
#else
BOOL SS_SetRowUserData(LPSPREADSHEET lpSS, SS_COORD Row, long lUserData)
#endif
{
LPSS_ROW lpRow;
BOOL     bRet = FALSE;

if (Row == SS_ALLROWS)
   return bRet;

if (Row < SS_GetRowCnt(lpSS) && (lpRow = SS_AllocLockRow(lpSS, Row)))
   {
   lpRow->lUserData = lUserData;
   SS_UnlockRowItem(lpSS, Row);
   bRet = TRUE;
   }
return bRet;
}

#if defined(_WIN64) || defined(_IA64)
BOOL DLLENTRY SSGetRowUserData(HWND hWnd, SS_COORD Row, LONG_PTR* lplUserData)
#else
BOOL DLLENTRY SSGetRowUserData(HWND hWnd, SS_COORD Row, LPLONG lplUserData)
#endif
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
bRet = SS_GetRowUserData(lpSS, Row, lplUserData);
SS_SheetUnlock(hWnd);
return bRet;
}

#if defined(_WIN64) || defined(_IA64)
BOOL SS_GetRowUserData(LPSPREADSHEET lpSS, SS_COORD Row, LONG_PTR* lplUserData)
#else
BOOL SS_GetRowUserData(LPSPREADSHEET lpSS, SS_COORD Row, LPLONG lplUserData)
#endif
{
LPSS_ROW lpRow;
BOOL     bRet = FALSE;

if (Row == SS_ALLROWS)
   return bRet;

if (lplUserData)
   *lplUserData = 0;
if (Row < SS_GetRowCnt(lpSS) && (lpRow = SS_LockRowItem(lpSS, Row)))
   {
   if (lplUserData)
      *lplUserData = lpRow->lUserData;
   if (lpRow->lUserData)
      bRet = TRUE;
   SS_UnlockRowItem(lpSS, Row);
   }

return bRet;
}

#endif


#ifndef SS_NOMAXTEXT

BOOL DLLENTRY SSGetMaxTextRowHeight(hWnd, Row, lpdfRowHeight)

HWND          hWnd;
SS_COORD      Row;
LPDOUBLE      lpdfRowHeight;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
fRet = SS_GetMaxTextRowHeight(lpSS, Row, lpdfRowHeight);
SS_SheetUnlock(hWnd);
return fRet;
}


BOOL SS_GetMaxTextRowHeight(lpSS, Row, lpdfRowHeight)

LPSPREADSHEET lpSS;
SS_COORD      Row;
LPDOUBLE      lpdfRowHeight;
{
LPSS_ROW      lpRow;
HDC           hDC;
int           dHeight = 0;
int           dHeightNew;
SS_COORD      i;
SS_COORD      ColCnt;
BOOL          fRet = FALSE;

*lpdfRowHeight = 0.0;

hDC = SS_GetDC(lpSS->lpBook);

if (Row < SS_GetRowCnt(lpSS))
   {
   /*
   SS_CalcPixelsToRowHeight(lpSS, Row, SS_GetRowHeightInPixels(lpSS, Row),
                            lpdfRowHeight);
   */

   lpRow = SS_LockRowItem(lpSS, Row);

   if (Row < lpSS->Row.HeaderCnt)
      ColCnt = SS_GetColCnt(lpSS);
   else
	   ColCnt = lpSS->Col.AllocCnt;
		// RFW - 6/2/03      ColCnt = lpSS->Col.DataCnt;
      
   for (i = 0; i < ColCnt; i++)
      {
		// If cell is part of a span but not the anchor, then ignore it
#ifdef SS_V40
		if (SS_GetCellSpan(lpSS, i, Row, NULL, NULL, NULL, NULL) != SS_SPAN_YES)
#endif // SS_V40
			{
			SS_CalcCellMetrics(hDC, lpSS, NULL, lpRow, NULL, NULL, NULL, i,
						          Row, FALSE, TRUE, NULL, NULL, NULL, &dHeightNew);
			dHeight = max(dHeight, dHeightNew);
			}
      }

   if (dHeight)
      SS_CalcPixelsToRowHeight(lpSS, Row, dHeight, lpdfRowHeight);

   SS_UnlockRowItem(lpSS, Row);

   fRet = TRUE;
   }

SS_ReleaseDC(lpSS->lpBook, hDC);

return (fRet);
}

#endif


#ifndef SS_NOPRINT

BOOL DLLENTRY SSSetRowPageBreak(HWND hWnd, SS_COORD Row, BOOL fPageBreak)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
fRet = SS_SetRowPageBreak(lpSS, Row, fPageBreak);
SS_SheetUnlock(hWnd);
return fRet;
}


BOOL SS_SetRowPageBreak(LPSPREADSHEET lpSS, SS_COORD Row, BOOL fPageBreak)
{
LPSS_ROW      lpRow;
BOOL          fRet = FALSE;

if (Row < SS_GetRowCnt(lpSS) && (lpRow = SS_AllocLockRow(lpSS, Row)))
   {
   lpRow->fPageBreak = fPageBreak;
   SS_UnlockRowItem(lpSS, Row);
   fRet = TRUE;
   }

return (fRet);
}


BOOL DLLENTRY SSGetRowPageBreak(HWND hWnd, SS_COORD Row)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
fRet = SS_GetRowPageBreak(lpSS, Row);
SS_SheetUnlock(hWnd);
return fRet;
}


BOOL SS_GetRowPageBreak(LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ROW      lpRow;
BOOL          fRet = FALSE;

if (Row < SS_GetRowCnt(lpSS) &&
    (lpRow = SS_LockRowItem(lpSS, Row)))
   {
   fRet = lpRow->fPageBreak;
   SS_UnlockRowItem(lpSS, Row);
   }

return (fRet);
}

#endif


void SS_AdjustDataRowCnt(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2)
{
LPSS_ROW  lpRow;
LPSS_CELL lpCell;
SS_COORD  Top;
SS_COORD  i;
SS_COORD  j;
BOOL      fHasData = FALSE;

if (Row == SS_ALLROWS || (Row <= lpSS->Row.DataCnt &&
    lpSS->Row.DataCnt <= Row2 + 1))
   {
   if (lpSS->fVirtualMode)
      Top = lpSS->Virtual.VTop;
   else
      Top = lpSS->Row.HeaderCnt;

   for (i = lpSS->Row.DataCnt - 1; i >= Top && !fHasData; i--)
      {
      if (lpRow = SS_LockRowItem(lpSS, i))
         {
         for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.DataCnt && !fHasData; j++)
            if (lpCell = SS_LockCellItem(lpSS, j, i))
               {
               if (lpCell->Data.bDataType)
                  fHasData = TRUE;

               SS_UnlockCellItem(lpSS, j, i);
               }

         SS_UnlockRowItem(lpSS, i);
         }

      if (!fHasData)
         lpSS->Row.DataCnt--;
      }

   if (lpSS->fVirtualMode && !fHasData)
      lpSS->Row.DataCnt = lpSS->Row.HeaderCnt;
   }
}

#ifdef SS_V40

short SS_GetRowMerge(LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ROW lpRow;
short    nMerge = SS_MERGE_NONE;

if (lpRow = SS_LockRowItem(lpSS, Row))
   {
   nMerge = (short)lpRow->bMerge;
   SS_UnlockRowItem(lpSS, Row);
   }

return (nMerge);
}

short DLLENTRY SSGetRowMerge(HWND hWnd, SS_COORD Row)
{
LPSPREADSHEET lpSS;
short         nMerge;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
nMerge = SS_GetRowMerge(lpSS, Row);
SS_SheetUnlock(hWnd);

return (nMerge);
}


void SS_SetRowMerge(LPSPREADSHEET lpSS, SS_COORD Row, short nRowMerge)
{
LPSS_ROW lpRow;

if (0 <= nRowMerge && nRowMerge <= SS_MERGE_CNT)
   {
   if (Row < SS_GetRowCnt(lpSS) && (lpRow = SS_AllocLockRow(lpSS, Row)))
      {
      lpRow->bMerge = (BYTE)nRowMerge;
      SS_InvalidateRowRange(lpSS, Row, -1);
      SS_UnlockRowItem(lpSS, Row);
      }
   }
}


void DLLENTRY SSSetRowMerge(HWND hWnd, SS_COORD Row, short nRowMerge)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, NULL, &Row);
SS_SetRowMerge(lpSS, Row, nRowMerge);
SS_SheetUnlock(hWnd);
}

#endif // SS_V40


void SS_UnhideActiveRow(LPSPREADSHEET lpSS, SS_COORD Row)
{
if (lpSS->Row.CurAt == Row)
	{
	lpSS->lpBook->fSuspendExit = TRUE;
	SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN | SS_F_MOVEACTIVECELL | SS_F_NOLEAVECELL);
	if (lpSS->Row.CurAt == Row)
		SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP | SS_F_MOVEACTIVECELL | SS_F_NOLEAVECELL);
	lpSS->lpBook->fSuspendExit = FALSE;
	}

// This code will hide the hidden row if it is the first row

if (lpSS->Row.UL == Row)
	{
	lpSS->lpBook->fSuspendExit = TRUE;
	SS_ScrollLineLR(lpSS, SS_F_SCROLL_DOWN | SS_F_NOLEAVECELL);
	if (lpSS->Row.UL == Row)
		SS_ScrollLineUL(lpSS, SS_F_SCROLL_UP | SS_F_NOLEAVECELL);
	lpSS->lpBook->fSuspendExit = FALSE;
	SS_SetVScrollBar(lpSS);
	}
}
