/*********************************************************
* SS_COPY.C
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
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>

#ifdef SS_OCX
#ifndef WIN32
#include <ole2.h>
#include <olenls.h>
#include <dispatch.h>
#endif
#include <olectl.h>
#endif

#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_span.h"
#include "ss_type.h"

#ifndef SS_NOCOPYMOVESWAP

#ifdef SS_VB
//#include <vbapi.h>
#endif

BOOL            SS_CopySwapCol(LPSPREADSHEET lpSS, SS_COORD x,
                               SS_COORD xDest, WORD wCmd, BOOL fSendChangeMsg, BOOL fAdjustDataCnt);
BOOL            SS_CopySwapCell(LPSPREADSHEET lpSS, SS_COORD x,
                                SS_COORD y, SS_COORD xDest, SS_COORD yDest,
                                WORD wCmd, BOOL  fSendChangeMsg, BOOL fAdjustDataCnt);
BOOL            SS_CopySwapRow(LPSPREADSHEET lpSS, SS_COORD y,
                               SS_COORD yDest, WORD wCmd, BOOL fSendChangeMsg, BOOL fAdjustDataCnt);
BOOL            SS_CopyRow(LPSPREADSHEET lpSS, SS_COORD Row,
                           LPSS_ROW lpRowDest, LPSS_ROW lpRowSrc);
BOOL            SS_CopyCol(LPSPREADSHEET lpSS, SS_COORD Col,
                           LPSS_COL lpColDest, LPSS_COL lpColSrc);
BOOL            SS_CopyCell(LPSPREADSHEET lpSS, SS_COORD ColDest, SS_COORD RowDest,
                            SS_COORD ColSrc, SS_COORD RowSrc, LPSS_CELL lpCellDest,
                            LPSS_CELL lpCellSrc);
BOOL            SSx_CopyData(LPSS_DATA lpDataDest, LPSS_DATA lpDataSrc,
                             LPSS_CELLTYPE lpCellType);
BOOL            SSx_tbCopyItem(LPTBGLOBALHANDLE lphItemDest,
                               LPTBGLOBALHANDLE lphItemSrc);
BOOL            SSxx_tbCopyItem(LPTBGLOBALHANDLE lphItemDest,
                                LPTBGLOBALHANDLE lphItemSrc, short nStringCnt);
BOOL            SSx_CopyItem(LPGLOBALHANDLE lphItemDest,
                             LPGLOBALHANDLE lphItemSrc);
TBGLOBALHANDLE  SS_DupBorderStruct(TBGLOBALHANDLE hBorder);
TBGLOBALHANDLE  SSx_CopyCellType(LPSPREADSHEET lpSS,
                                 TBGLOBALHANDLE hCellTypeSrc);
TBGLOBALHANDLE  SSx_CopyCalc(LPSPREADSHEET lpSS, TBGLOBALHANDLE hCalcSrc,
                             SS_COORD Col, SS_COORD Row);
void            SS_CopyAdjustDataCnt(LPSPREADSHEET lpSS, BYTE bDataType,
                                     SS_COORD Col, SS_COORD Row);
void            SS_CopyAdjustDataColCnt(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
void            SS_CopyAdjustDataRowCnt(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);


BOOL DLLENTRY SSCopyRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                          SS_COORD Col2, SS_COORD Row2, SS_COORD ColDest,
                          SS_COORD RowDest)
{
LPSPREADSHEET lpSS;
BOOL       bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
SS_AdjustCellCoords(lpSS, &Col2, &Row2);
SS_AdjustCellCoords(lpSS, &ColDest, &RowDest);
bRet = SS_CopySwapRange(lpSS, Col, Row, Col2, Row2, ColDest, RowDest,
                        SS_CMD_COPY, FALSE, TRUE);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL DLLENTRY SSSwapRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                          SS_COORD Col2, SS_COORD Row2, SS_COORD ColDest,
                          SS_COORD RowDest)
{
LPSPREADSHEET lpSS;
BOOL       bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
SS_AdjustCellCoords(lpSS, &Col2, &Row2);
SS_AdjustCellCoords(lpSS, &ColDest, &RowDest);
bRet = SS_CopySwapRange(lpSS, Col, Row, Col2, Row2, ColDest, RowDest,
                        SS_CMD_SWAP, FALSE, TRUE);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL DLLENTRY SSMoveRange(HWND hWnd, SS_COORD Col, SS_COORD Row,
                          SS_COORD Col2, SS_COORD Row2, SS_COORD ColDest,
                          SS_COORD RowDest)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
SS_AdjustCellCoords(lpSS, &Col2, &Row2);
SS_AdjustCellCoords(lpSS, &ColDest, &RowDest);
bRet = SS_MoveRange(lpSS, Col, Row, Col2, Row2, ColDest, RowDest);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_MoveRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                  SS_COORD Row2, SS_COORD ColDest, SS_COORD RowDest)
{
return (SS_CopySwapRange(lpSS, Col, Row, Col2, Row2, ColDest, RowDest, SS_CMD_MOVE, FALSE, TRUE));
}


BOOL SS_CopySwapRange(LPSPREADSHEET lpSS,
                      SS_COORD Col, SS_COORD Row,
                      SS_COORD Col2, SS_COORD Row2,
                      SS_COORD ColDest, SS_COORD RowDest,
                      WORD wCmd, BOOL fSendChangeMsg, BOOL fAdjustDataCnt)
{
SS_COORD      x;
SS_COORD      xInc;
SS_COORD      xDest;
SS_COORD      xDestEnd;
SS_COORD      y;
SS_COORD      yInc;
SS_COORD      yDest;
SS_COORD      yDestEnd;
BOOL          RedrawOld;
BOOL          fTurnEditModeOn = FALSE;
BOOL          fRet = TRUE;

if (SS_ALLCOLS == Col || SS_ALLCOLS == Col2)
   Col = Col2 = SS_ALLCOLS;
if (SS_ALLROWS == Row || SS_ALLROWS == Row2)
   Row = Row2 = SS_ALLROWS;

if ((Row != SS_ALLROWS && RowDest == SS_ALLROWS) ||
    (Col != SS_ALLCOLS && ColDest == SS_ALLCOLS))
   return (FALSE);

// RFW - 8/30/04 - 15117
if ((SS_ALLCOLS != Col && Col > Col2) ||
    (SS_ALLROWS != Row && Row > Row2))
	return (FALSE);

// RFW - 10/26/06 - 19609
if (lpSS->lpBook->EditModeOn)
   {
   SS_CellEditModeOff(lpSS, 0);
   fTurnEditModeOn = TRUE;
   }

/************************
* Calculate coordinates
************************/

if (Row != SS_ALLROWS)
   {
   if (RowDest >= Row && RowDest <= Row2)
      {
      y = Row2;
      yInc = -1;
      yDest = RowDest + Row2 - Row;
      yDestEnd = RowDest - 1;
      if (yDest >= SS_GetRowCnt(lpSS))
         return FALSE;
      }
   else
      {
      y = Row;
      yInc = 1;
      yDest = RowDest;
      yDestEnd = RowDest + Row2 - Row + 1;
      if (yDestEnd > SS_GetRowCnt(lpSS))
        return FALSE;
      }
   }
else
   {
   y = -1;
   yInc = 1;
   yDest = -1;
   yDestEnd = 0;
   }

lpSS->Col.LastRefCoord.Coord.Col = -1;
lpSS->Row.LastRefCoord.Coord.Row = -1;
lpSS->LastCell.Coord.Col = -1;
lpSS->LastCell.Coord.Row = -1;

RedrawOld = lpSS->lpBook->Redraw;
lpSS->lpBook->Redraw = FALSE;

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
{
SS_COORD lColOut = Col;
SS_COORD lRowOut = Row;
SS_COORD lCol2Out = Col2;
SS_COORD lRow2Out = Row2;
SS_COORD lColDestOut = ColDest;
SS_COORD lRowDestOut = RowDest;

SS_AdjustCellCoordsOut(lpSS, &lColOut, &lRowOut);
SS_AdjustCellCoordsOut(lpSS, &lCol2Out, &lRow2Out);
SS_AdjustCellCoordsOut(lpSS, &lColDestOut, &lRowDestOut);

switch(wCmd)
{
  case SS_CMD_COPY:
    CalcCopyRange(&lpSS->CalcInfo, lColOut, lRowOut, lCol2Out, lRow2Out, lColDestOut, lRowDestOut);
    break;
  case SS_CMD_MOVE:
    CalcMoveRange(&lpSS->CalcInfo, lColOut, lRowOut, lCol2Out, lRow2Out, lColDestOut, lRowDestOut);
    break;
  case SS_CMD_SWAP:
    CalcSwapRange(&lpSS->CalcInfo, lColOut, lRowOut, lCol2Out, lRow2Out, lColDestOut, lRowDestOut);
    break;
}
}
#endif

for (; yDest != yDestEnd && fRet; y += yInc, yDest += yInc)
   {
   if (Col != SS_ALLCOLS)
      {
      if (ColDest >= Col && ColDest <= Col2)
         {
         x = Col2;
         xInc = -1;
         xDest = ColDest + Col2 - Col;
         xDestEnd = ColDest - 1;
         if (xDest >= SS_GetColCnt(lpSS))
            return FALSE;
         }
      else
         {
         x = Col;
         xInc = 1;
         xDest = ColDest;
         xDestEnd = ColDest + Col2 - Col + 1;
         if (xDestEnd > SS_GetColCnt(lpSS))
            return FALSE;
         }
      }
   else
      {
      x = -1;
      xInc = 1;
      xDest = -1;
      xDestEnd = 0;
      }

   for (; xDest != xDestEnd && fRet; x += xInc, xDest += xInc)
      {
      if (x != SS_ALLCOLS && y != SS_ALLROWS)          // Cell
         fRet = SS_CopySwapCell(lpSS, x, y, xDest, yDest, wCmd,
                                fSendChangeMsg, fAdjustDataCnt);

      else if (x != SS_ALLCOLS && y == SS_ALLROWS)     // Col
         fRet = SS_CopySwapCol(lpSS, x, xDest, wCmd, fSendChangeMsg, fAdjustDataCnt);

      else if (x == SS_ALLCOLS && y != SS_ALLROWS)     // Row
         fRet = SS_CopySwapRow(lpSS, y, yDest, wCmd, fSendChangeMsg, fAdjustDataCnt);
      }
   }

// RFW - 2/10/05 - 15731
if (fAdjustDataCnt)
	{
	if (max(ColDest + (Col2 - Col), Col2) >= lpSS->Col.DataCnt - 1)
		SS_SetDataColCnt(lpSS, max(ColDest + (Col2 - Col), Col2) + 1);

	if (max(RowDest + (Row2 - Row), Row2) >= lpSS->Row.DataCnt - 1)
		SS_SetDataRowCnt(lpSS, max(RowDest + (Row2 - Row), Row2) + 1);

	if (max(ColDest + (Col2 - Col), Col2) >= lpSS->Col.DataCnt - 1)
		SS_AdjustDataColCnt(lpSS, min(ColDest, Col), max(ColDest + (Col2 - Col), Col2));

	if (max(RowDest + (Row2 - Row), Row2) >= lpSS->Row.DataCnt - 1)
		SS_AdjustDataRowCnt(lpSS, min(RowDest, Row), max(RowDest + (Row2 - Row), Row2));
	}

#ifdef SS_OLDCALC
SS_BuildDependencies(lpSS);
#elif !defined(SS_NOCALC)
if( lpSS->lpBook->CalcAuto )
  CalcEvalNeededCells(&lpSS->lpBook->CalcInfo);
#endif

if (lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
	SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

if (fTurnEditModeOn)
   SS_CellEditModeOn(lpSS, 0, 0, 0L);

return (fRet);
}


BOOL SS_CopySwapCol(LPSPREADSHEET lpSS, SS_COORD x,
                    SS_COORD xDest, WORD wCmd, BOOL fSendChangeMsg, BOOL fAdjustDataCnt)
{
TBGLOBALHANDLE hItem;
LPSS_COL       lpCol;
LPSS_COL       lpColDest;
LPSS_ROW       lpRow;
LPSS_CELL      lpCell;
LPSS_CELL      lpCellDest;
BOOL           fRet = TRUE;
SS_COORD       i;

if (x == xDest)
	return (TRUE);

/*************
* Copy a col
*************/

if (wCmd == SS_CMD_COPY)
   {
   SS_DeAllocCol(lpSS, xDest, FALSE);

   if (lpCol = SS_LockColItem(lpSS, x))
      {
      if (lpColDest = SS_AllocLockCol(lpSS, xDest))
         {
         SS_CopyCol(lpSS, xDest, lpColDest, lpCol);
         SS_UnlockColItem(lpSS, xDest);
         }

      SS_UnlockColItem(lpSS, x);
      }

   for (i = 0; i < lpSS->Row.AllocCnt; i++)
      {
      if (lpCell = SSx_LockCellItem(lpSS, NULL, x, i))
         {
         if (lpCellDest = SS_AllocLockCell(lpSS, NULL, xDest, i))
            {
            SS_CopyCell(lpSS, xDest, i, x, i, lpCellDest, lpCell);
            SS_UnlockCellItem(lpSS, xDest, i);
            }

         SS_UnlockCellItem(lpSS, x, i);
         }
      }

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       xDest, SS_ALLROWS);

   SS_InvalidateColRange(lpSS, xDest, -1);
   }

/*************
* Swap a col
*************/

else if (wCmd == SS_CMD_SWAP)
   {
   hItem = SS_GrabItemHandle(lpSS, &lpSS->Col.Items, xDest,
                             SS_GetColCnt(lpSS));

   fRet = SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolCol,
                            &lpSS->Col.Items, x, x, xDest,
                            SS_GetColCnt(lpSS));

   if (hItem)
      if ((hItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolCol,
                                 &lpSS->Col.Items, x, 0, hItem,
                                 SS_GetColCnt(lpSS), NULL)) == 0 ||
                                 (LPARAM)hItem == (LPARAM)-1)
         fRet = FALSE;

   for (i = 0; i < lpSS->Row.AllocCnt; i++)
      {
      if (lpRow = SS_LockRowItem(lpSS, i))
         {
         hItem = SS_GrabItemHandle(lpSS, &lpRow->Cells, xDest,
                                   SS_GetColCnt(lpSS));

         fRet = SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolCell,
                                  &lpRow->Cells, x, x, xDest,
                                  SS_GetColCnt(lpSS));

         if (hItem)
            if ((hItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolCell,
                                       &lpRow->Cells, x, 0, hItem,
                                       SS_GetColCnt(lpSS), NULL)) == 0 ||
                                       (LPARAM)hItem == (LPARAM)-1)
               fRet = FALSE;

         SS_UnlockRowItem(lpSS, i);
         }
      }

	if (max(x, xDest) >= lpSS->Col.AllocCnt)
		lpSS->Col.AllocCnt = max(x, xDest) + 1;

	if (fAdjustDataCnt)
		{
		if (max(x, xDest) >= lpSS->Col.DataCnt - 1)
			{
			lpSS->Col.DataCnt = max(x, xDest) + 1;
			SS_AdjustDataColCnt(lpSS, min(x, xDest), max(x, xDest));
			}
		}

   SS_InvalidateColRange(lpSS, x, -1);
   SS_InvalidateColRange(lpSS, xDest, -1);

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      {
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       x, SS_ALLROWS);
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       xDest, SS_ALLROWS);
      }
   }

/*************
* Move a col
*************/

else
   {
   SS_DeAllocCol(lpSS, xDest, FALSE);

   fRet = SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolCol,
                            &lpSS->Col.Items, x, x, xDest,
                            SS_GetColCnt(lpSS));

   for (i = 0; i < lpSS->Row.AllocCnt; i++)
		fRet = SS_CopySwapCell(lpSS, x, i, xDest, i, SS_CMD_MOVE, FALSE, fAdjustDataCnt);
		/* RFW - 1/2/04 - 13337
      {
      if (lpRow = SS_LockRowItem(lpSS, i))
         {
         SS_DeAllocCell(lpSS, xDest, i, FALSE);

         fRet = SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolCell,
                                  &lpRow->Cells, x, x, xDest,
                                  SS_GetColCnt(lpSS));

         SS_UnlockRowItem(lpSS, i);
         }
      }
		*/

#ifdef SS_V40
//	SS_MoveSpan(lpSS, xDest, -1, x, -1);
#endif // SS_V40

	if (max(x, xDest) >= lpSS->Col.AllocCnt)
		lpSS->Col.AllocCnt = max(x, xDest) + 1;

	if (fAdjustDataCnt)
		{
		if (max(x, xDest) >= lpSS->Col.DataCnt - 1)
			{
			lpSS->Col.DataCnt = max(x, xDest) + 1;
			SS_AdjustDataColCnt(lpSS, min(x, xDest), max(x, xDest));
			}
		}

   SS_InvalidateColRange(lpSS, x, -1);
   SS_InvalidateColRange(lpSS, xDest, -1);

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      {
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       x, SS_ALLROWS);
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       xDest, SS_ALLROWS);
      }
   }

return (fRet);
}


BOOL SS_CopySwapCell(LPSPREADSHEET lpSS, SS_COORD x, SS_COORD y,
                     SS_COORD xDest, SS_COORD yDest, WORD wCmd,
                     BOOL fSendChangeMsg, BOOL fAdjustDataCnt)
{
TBGLOBALHANDLE hItem = 0;
TBGLOBALHANDLE hItemDest = 0;
LPSS_ROW       lpRow;
LPSS_ROW       lpRowDest;
LPSS_CELL      lpCell;
LPSS_CELL      lpCellDest;
BOOL           fRet = TRUE;

if (x == xDest && y == yDest)
	return (TRUE);

/**************
* Copy a cell
**************/

if (wCmd == SS_CMD_COPY)
   {
   SS_DeAllocCell(lpSS, xDest, yDest, FALSE);

   if (lpCell = SS_LockCellItem(lpSS, x, y))
      {
      if (lpCellDest = SS_AllocLockCell(lpSS, NULL, xDest, yDest))
         {
         SS_CopyCell(lpSS, xDest, yDest, x, y, lpCellDest, lpCell);
         SS_UnlockCellItem(lpSS, xDest, yDest);
         }

      SS_UnlockCellItem(lpSS, x, y);
      }

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       xDest, yDest);

   SS_InvalidateCell(lpSS, xDest, yDest);
   }

/**************
* Swap a cell
**************/

else if (wCmd == SS_CMD_SWAP)
   {
   if (lpRowDest = SS_LockRowItem(lpSS, yDest))
      {
      hItemDest = SS_GrabItemHandle(lpSS, &lpRowDest->Cells, xDest,
                                    SS_GetColCnt(lpSS));
      SS_UnlockRowItem(lpSS, yDest);
      }

   if (lpRow = SS_LockRowItem(lpSS, y))
      {
      hItem = SS_GrabItemHandle(lpSS, &lpRow->Cells, x,
                                SS_GetColCnt(lpSS));
      SS_UnlockRowItem(lpSS, y);
      }

   if (lpRowDest = SS_AllocLockRow(lpSS, yDest))
      {
      if (hItem)
			{
         if ((hItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolCell,
                                    &lpRowDest->Cells, xDest, 0, hItem,
                                    SS_GetColCnt(lpSS), NULL)) == 0 ||
                                    (LPARAM)hItem == (LPARAM)-1)
            fRet = FALSE;
         else
            {
#ifndef SS_NOOVERFLOW
            SS_OverflowAdjustNeighbor(lpSS, xDest, yDest);
#endif
            }
			}
		else // RFW - 6/21/04 - 14604
	      SS_ResetCellOverflow(lpSS, xDest, yDest);

      SS_UnlockRowItem(lpSS, yDest);
      }

   if (lpRow = SS_AllocLockRow(lpSS, y))
      {
      if (hItemDest)
			{
         if ((hItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolCell,
                                    &lpRow->Cells, x, 0, hItemDest,
                                    SS_GetColCnt(lpSS), NULL)) == 0 ||
                                    (LPARAM)hItem == (LPARAM)-1)
            fRet = FALSE;
         else
            {
#ifndef SS_NOOVERFLOW
            SS_OverflowAdjustNeighbor(lpSS, x, y);
#endif
            }
			}
		else // RFW - 6/21/04 - 14604
	      SS_ResetCellOverflow(lpSS, x, y);

      SS_UnlockRowItem(lpSS, y);
      }

	lpSS->Col.LastRefCoord.Coord.Col = -1;
	lpSS->Row.LastRefCoord.Coord.Row = -1;
	lpSS->LastCell.Coord.Col = -1;
	lpSS->LastCell.Coord.Row = -1;

	if (max(x, xDest) >= lpSS->Col.AllocCnt)
		lpSS->Col.AllocCnt = max(x, xDest) + 1;

   SS_InvalidateCell(lpSS, x, y);
   SS_InvalidateCell(lpSS, xDest, yDest);

/* RFW - 5/19/04 - 14262
   bDataType = 0;

   if (lpCellDest = SS_LockCellItem(lpSS, xDest, yDest))
      {
      bDataType = lpCellDest->Data.bDataType;
      SS_UnlockCellItem(lpSS, xDest, yDest);
      }

   SS_CopyAdjustDataCnt(lpSS, bDataType, xDest, yDest);
*/
	if (fAdjustDataCnt)
		{
		if (x > xDest)
			SS_CopyAdjustDataColCnt(lpSS, x, y);
		else
			SS_CopyAdjustDataColCnt(lpSS, xDest, yDest);

		if (y > yDest)
			SS_CopyAdjustDataRowCnt(lpSS, x, y);
		else
			SS_CopyAdjustDataRowCnt(lpSS, xDest, yDest);
		}

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      {
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       x, y);
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       xDest, yDest);
      }
   }

/**************
* Move a cell
**************/

else
   {
   SS_DeAllocCell(lpSS, xDest, yDest, FALSE);

   if (lpRow = SS_LockRowItem(lpSS, y))
      {
      SS_OverflowInvalidateCell(lpSS, x, y);
      SSx_ResetCellOverflow(lpSS, x, y, NULL, NULL);

#ifndef SS_NOOVERFLOW
		{
      LPSS_CELL lpCell = SS_LockCellItem(lpSS, x, y);
		if (lpCell)
			{
			lpCell->Data.bOverflow = 0;
			SS_UnlockCellItem(lpSS, x, y);
			}
		}
#endif

      hItem = SS_GrabItemHandle(lpSS, &lpRow->Cells, x,
                                SS_GetColCnt(lpSS));

      if (lpRowDest = SS_AllocLockRow(lpSS, yDest))
         {
         if (hItem)
            if ((hItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolCell,
                                       &lpRowDest->Cells, xDest, 0, hItem,
                                       SS_GetColCnt(lpSS), NULL)) == 0 ||
                                       (LPARAM)hItem == (LPARAM)-1)
               fRet = FALSE;
				else
					{
					lpSS->Col.AllocCnt = max(lpSS->Col.AllocCnt, xDest + 1);
#ifndef SS_NOOVERFLOW
					SS_OverflowAdjustNeighbor(lpSS, xDest, yDest);
#endif
					}

			// RFW - 8/29/03
	      SS_OverflowAdjustNeighbor(lpSS, x, y);

			SS_InvalidateCell(lpSS, x, y);
#ifdef SS_V40
			SS_MoveSpan(lpSS, xDest, yDest, x, y);
#endif // SS_V40
         SS_UnlockRowItem(lpSS, yDest);
         }

      SS_UnlockRowItem(lpSS, y);
      }

	lpSS->Col.LastRefCoord.Coord.Col = -1;
	lpSS->Row.LastRefCoord.Coord.Row = -1;
	lpSS->LastCell.Coord.Col = -1;
	lpSS->LastCell.Coord.Row = -1;

	if (max(x, xDest) >= lpSS->Col.AllocCnt)
		lpSS->Col.AllocCnt = max(x, xDest) + 1;

   SS_InvalidateCell(lpSS, xDest, yDest);

/* RFW - 5/19/04 - 14262
   bDataType = 0;

   if (lpCellDest = SS_LockCellItem(lpSS, xDest, yDest))
      {
      bDataType = lpCellDest->Data.bDataType;
      SS_UnlockCellItem(lpSS, xDest, yDest);
      }

   SS_CopyAdjustDataCnt(lpSS, bDataType, xDest, yDest);
*/

	/* RFW - 7/2/04 - 14781
	if (x > xDest)
		SS_CopyAdjustDataColCnt(lpSS, x, y);
	else
		SS_CopyAdjustDataColCnt(lpSS, xDest, yDest);

	if (y > yDest)
		SS_CopyAdjustDataRowCnt(lpSS, x, y);
	else
		SS_CopyAdjustDataRowCnt(lpSS, xDest, yDest);
	*/

	/* RFW - 2/10/05 - 15731
	if (fAdjustDataCnt)
		{
		if (max(x, xDest) >= lpSS->Col.DataCnt - 1)
			SS_SetDataColCnt(lpSS, max(x, xDest) + 1);

		if (max(y, yDest) >= lpSS->Row.DataCnt - 1)
			SS_SetDataRowCnt(lpSS, max(y, yDest) + 1);

		if (max(x, xDest) >= lpSS->Col.DataCnt - 1)
			SS_AdjustDataColCnt(lpSS, min(x, xDest), max(x, xDest));

		if (max(y, yDest) >= lpSS->Row.DataCnt - 1)
			SS_AdjustDataRowCnt(lpSS, min(y, yDest), max(y, yDest));
		}
	*/

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      {
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       x, y);
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       xDest, yDest);
      }
   }

return (fRet);
}


BOOL SS_CopySwapRow(LPSPREADSHEET lpSS, SS_COORD y, SS_COORD yDest,
                    WORD wCmd, BOOL fSendChangeMsg, BOOL fAdjustDataCnt)
{
TBGLOBALHANDLE hItem;
LPSS_ROW       lpRow;
LPSS_ROW       lpRowDest;
LPSS_CELL      lpCell;
LPSS_CELL      lpCellDest;
BOOL           fRet = TRUE;
SS_COORD       i;

if (y == yDest)
	return (TRUE);

/*************
* Copy a row
*************/

if (wCmd == SS_CMD_COPY)
   {
   SS_DeAllocRow(lpSS, yDest, FALSE);

   if (lpRow = SS_LockRowItem(lpSS, y))
      {
      if (lpRowDest = SS_AllocLockRow(lpSS, yDest))
         {
         SS_CopyRow(lpSS, yDest, lpRowDest, lpRow);
         SS_UnlockRowItem(lpSS, yDest);
         }

      /********************************
      * Copy each cell within the row
      ********************************/

      for (i = 0; i < lpSS->Col.AllocCnt; i++)
         {
         if (lpCell = SSx_LockCellItem(lpSS, lpRow, i, y))
            {
            if (lpCellDest = SS_AllocLockCell(lpSS, lpRow, i, yDest))
               {
               SS_CopyCell(lpSS, i, yDest, i, y, lpCellDest, lpCell);
               SS_UnlockCellItem(lpSS, i, yDest);
               }

            SS_UnlockCellItem(lpSS, i, y);
            }
         }

      SS_UnlockRowItem(lpSS, y);
      }

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       SS_ALLCOLS, yDest);

   SS_InvalidateRowRange(lpSS, yDest, -1);
   }

/*************
* Swap a row
*************/

else if (wCmd == SS_CMD_SWAP)
   {
   hItem = SS_GrabItemHandle(lpSS, &lpSS->Row.Items, yDest,
                             SS_GetRowCnt(lpSS));

   fRet = SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolRow, &lpSS->Row.Items, y, y, yDest,
                            SS_GetRowCnt(lpSS));

   if (hItem)
      if ((hItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolRow,
                                 &lpSS->Row.Items, y, 0, hItem,
                                 SS_GetRowCnt(lpSS), NULL)) == 0 ||
                                 (LPARAM)hItem == (LPARAM)-1)
         fRet = FALSE;

	if (max(y, yDest) >= lpSS->Row.AllocCnt)
		lpSS->Row.AllocCnt = max(y, yDest) + 1;

	if (fAdjustDataCnt)
		{
		if (max(y, yDest) >= lpSS->Row.DataCnt - 1)
			{
			lpSS->Row.DataCnt = max(y, yDest) + 1;
			SS_AdjustDataRowCnt(lpSS, min(y, yDest), max(y, yDest));
			}
		}

   SS_InvalidateRowRange(lpSS, y, -1);
   SS_InvalidateRowRange(lpSS, yDest, -1);

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      {
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       SS_ALLCOLS, y);
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       SS_ALLCOLS, yDest);
      }
   }

/*************
* Move a row
*************/

else
   {
   SS_DeAllocRow(lpSS, yDest, FALSE);

   fRet = SS_MoveAllocItems(lpSS, lpSS->lpBook->OmemPoolRow,
                            &lpSS->Row.Items, y, y, yDest,
                            SS_GetRowCnt(lpSS));

	if (max(y, yDest) >= lpSS->Row.AllocCnt)
		lpSS->Row.AllocCnt = max(y, yDest) + 1;

	if (fAdjustDataCnt)
		{
		if (max(y, yDest) >= lpSS->Row.DataCnt - 1)
			{
			lpSS->Row.DataCnt = max(y, yDest) + 1;
			SS_AdjustDataRowCnt(lpSS, min(y, yDest), max(y, yDest));
			}
		}

#ifdef SS_V40
	SS_MoveSpan(lpSS, -1, yDest, -1, y);
#endif // SS_V40

   SS_InvalidateRowRange(lpSS, y, -1);
   SS_InvalidateRowRange(lpSS, yDest, -1);

   if (fSendChangeMsg && lpSS->lpBook->hWnd)
      {
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       SS_ALLCOLS, y);
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       SS_ALLCOLS, yDest);
      }
   }

return (fRet);
}


BOOL SS_CopyRow(lpSS, Row, lpRowDest, lpRowSrc)

LPSPREADSHEET lpSS;
SS_COORD      Row;
LPSS_ROW      lpRowDest;
LPSS_ROW      lpRowSrc;
{
LPSS_CELLTYPE lpCellType;
SS_CELLTYPE   CellTypeTemp;

_fmemcpy(lpRowDest, lpRowSrc, sizeof(SS_ROW));

if (lpRowDest->Data.bDataType == SS_TYPE_EDIT)
   {
   lpCellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL, -1, Row);
   SSx_CopyData(&lpRowDest->Data, &lpRowSrc->Data, lpCellType);
   }

#ifdef SS_V35
SSx_tbCopyItem(&lpRowDest->hCellNote, &lpRowSrc->hCellNote);
#endif
#ifdef SS_V40
SSx_tbCopyItem(&lpRowDest->hCellTag, &lpRowSrc->hCellTag);
#endif

lpRowDest->hCellType = SSx_CopyCellType(lpSS, lpRowSrc->hCellType);
lpRowDest->hBorder = SS_DupBorderStruct(lpRowSrc->hBorder);

#ifndef SS_NOCALC
#ifdef SS_OLDCALC
lpRowDest->hCalc = SSx_CopyCalc(lpSS, lpRowSrc->hCalc, -1, Row);
_fmemset(&lpRowDest->Dependents, '\0', sizeof(SS_CALCLIST));
#endif
#endif

// BJO 29Apr97 JOK2402 - Begin fix
#ifdef SS_BOUNDCONTROL
lpRowDest->hlstrBookMark = 0;
lpRowDest->bDirty = FALSE;
#endif
// BJO 29Apr97 JOK2402 - End fix

lpRowDest->Cells.wItemCnt = 0;
lpRowDest->Cells.hItems = 0;

SS_CopyAdjustDataCnt(lpSS, lpRowDest->Data.bDataType, SS_ALLCOLS, Row);

return (TRUE);
}


BOOL SS_CopyCol(lpSS, Col, lpColDest, lpColSrc)

LPSPREADSHEET lpSS;
SS_COORD      Col;
LPSS_COL      lpColDest;
LPSS_COL      lpColSrc;
{
LPSS_CELLTYPE lpCellType;
SS_CELLTYPE   CellTypeTemp;

_fmemcpy(lpColDest, lpColSrc, sizeof(SS_COL));

if (lpColDest->Data.bDataType == SS_TYPE_EDIT)
   {
   lpCellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL, Col, -1);
   SSx_CopyData(&lpColDest->Data, &lpColSrc->Data, lpCellType);
   }

SSx_tbCopyItem(&lpColDest->hDBFieldName, &lpColSrc->hDBFieldName);
#ifdef SS_V35
SSx_tbCopyItem(&lpColDest->hCellNote, &lpColSrc->hCellNote);
#endif
#ifdef SS_V40
SSx_tbCopyItem(&lpColDest->hColID, &lpColSrc->hColID);
SSx_tbCopyItem(&lpColDest->hCellTag, &lpColSrc->hCellTag);
#endif

lpColDest->hCellType = SSx_CopyCellType(lpSS, lpColSrc->hCellType);
lpColDest->hBorder = SS_DupBorderStruct(lpColSrc->hBorder);

#ifndef SS_NOCALC
#ifdef SS_OLDCALC
lpColDest->hCalc = SSx_CopyCalc(lpSS, lpColSrc->hCalc, Col, -1);
_fmemset(&lpColDest->Dependents, '\0', sizeof(SS_CALCLIST));
#endif
#endif

SS_CopyAdjustDataCnt(lpSS, lpColDest->Data.bDataType, Col, SS_ALLROWS);

return (TRUE);
}


BOOL SS_CopyCell(LPSPREADSHEET lpSS, SS_COORD ColDest, SS_COORD RowDest, SS_COORD ColSrc,
                 SS_COORD RowSrc, LPSS_CELL lpCellDest, LPSS_CELL lpCellSrc)
{
LPSS_CELLTYPE lpCellType;
SS_CELLTYPE   CellTypeTemp;

_fmemcpy(lpCellDest, lpCellSrc, sizeof(SS_CELL));

if (lpCellDest->Data.bDataType == SS_TYPE_EDIT)
   {
   lpCellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL, ColDest, RowDest);
   SSx_CopyData(&lpCellDest->Data, &lpCellSrc->Data, lpCellType);
   }

// RFW - 2/6/07 - 19785
lpCellDest->Data.bOverflow &= ~(SS_OVERFLOW_LEFT | SS_OVERFLOW_RIGHT);

#ifdef SS_V35
SSx_tbCopyItem(&lpCellDest->hCellNote, &lpCellSrc->hCellNote);
#endif
#ifdef SS_V40
SSx_tbCopyItem(&lpCellDest->hCellTag, &lpCellSrc->hCellTag);
SS_CopySpan(lpSS, ColDest, RowDest, ColSrc, RowSrc);
#endif

lpCellDest->hCellType = SSx_CopyCellType(lpSS, lpCellSrc->hCellType);
lpCellDest->hBorder = SS_DupBorderStruct(lpCellSrc->hBorder);

#ifndef SS_NOCALC
#ifdef SS_OLDCALC
lpCellDest->hCalc = SSx_CopyCalc(lpSS, lpCellSrc->hCalc, ColDest, RowDest);
_fmemset(&lpCellDest->Dependents, '\0', sizeof(SS_CALCLIST));
#endif
#endif

SS_CopyAdjustDataCnt(lpSS, lpCellDest->Data.bDataType, ColDest, RowDest);

#ifndef SS_NOOVERFLOW
SS_OverflowAdjustNeighbor(lpSS, ColDest, RowDest);
#endif

return (TRUE);
}


BOOL SSx_CopyData(LPSS_DATA lpDataDest, LPSS_DATA lpDataSrc,
                  LPSS_CELLTYPE lpCellType)
{
BOOL fRet = TRUE;

if (lpDataSrc->bDataType == SS_TYPE_EDIT)
   {
   if (!(fRet = SSxx_tbCopyItem(&lpDataDest->Data.hszData,
         &lpDataSrc->Data.hszData, (short)((lpCellType &&
         lpCellType->Type == SS_TYPE_COMBOBOX) ? 2 : 1))))
      lpDataDest->bDataType = 0;
   }

return (fRet);
}


BOOL SSx_tbCopyItem(lphItemDest, lphItemSrc)

LPTBGLOBALHANDLE lphItemDest;
LPTBGLOBALHANDLE lphItemSrc;
{
return (SSxx_tbCopyItem(lphItemDest, lphItemSrc, 1));
}


BOOL SSxx_tbCopyItem(LPTBGLOBALHANDLE lphItemDest, LPTBGLOBALHANDLE lphItemSrc,
                     short nStringCnt)
{
TBGLOBALHANDLE hItemDest;
LPTSTR         lpText;
LPTSTR         lpTextDest;
// fix for #9006 -scl
//short          nLen = 0;
long           nLen = 0;
short          i;
BOOL           fRet = FALSE;

if (*lphItemSrc)
   {
   lpText = (LPTSTR)tbGlobalLock(*lphItemSrc);

   for (i = 0; i < nStringCnt; i++)
      nLen += lstrlen(&lpText[nLen]) + 1;

   if (hItemDest = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                 nLen * sizeof(TCHAR)))
      {
      lpTextDest = (LPTSTR)tbGlobalLock(hItemDest);
      _fmemcpy(lpTextDest, lpText, nLen * sizeof(TCHAR));
      tbGlobalUnlock(hItemDest);
      fRet = TRUE;
      }

   tbGlobalUnlock(*lphItemSrc);

   *lphItemDest = hItemDest;
   }

return (fRet);
}


BOOL SSx_CopyItem(lphItemDest, lphItemSrc)

LPGLOBALHANDLE lphItemDest;
LPGLOBALHANDLE lphItemSrc;
{
LPTSTR         lpText;
LPTSTR         lpTextDest;
BOOL           fRet = FALSE;

if (*lphItemSrc)
   {
   lpText = (LPTSTR)GlobalLock(*lphItemSrc);

   if (*lphItemDest = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  (lstrlen(lpText) + 1) * sizeof(TCHAR)))
      {
      lpTextDest = (LPTSTR)GlobalLock(*lphItemDest);
      lstrcpy(lpTextDest, lpText);
      GlobalUnlock(*lphItemDest);
      fRet = TRUE;
      }

   GlobalUnlock(*lphItemSrc);
   }

return (fRet);
}


TBGLOBALHANDLE SS_DupBorderStruct(TBGLOBALHANDLE hBorder)
{
TBGLOBALHANDLE  hBorderNew = 0;
LPSS_CELLBORDER lpBorder;
LPSS_CELLBORDER lpBorderNew;

if (hBorder)
   {
   lpBorder = (LPSS_CELLBORDER)tbGlobalLock(hBorder);

   if (hBorderNew = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  sizeof(SS_CELLBORDER)))
      {
      lpBorderNew = (LPSS_CELLBORDER)tbGlobalLock(hBorderNew);
      _fmemcpy(lpBorderNew, lpBorder, sizeof(SS_CELLBORDER));
      tbGlobalUnlock(hBorderNew);
      }

   tbGlobalUnlock(hBorder);
   }

return (hBorderNew);
}


#ifdef SS_OCX
void SSOCX_AddRefPict(LPPICTURE lpPict)
{
if (lpPict)
   lpPict->lpVtbl->AddRef(lpPict);
}
#endif


TBGLOBALHANDLE SSx_CopyCellType(LPSPREADSHEET lpSS,
                                TBGLOBALHANDLE hCellTypeSrc)
{
TBGLOBALHANDLE hCellTypeDest = 0;
LPSS_CELLTYPE  lpCellTypeDest;
LPSS_CELLTYPE  lpCellTypeSrc;

if (hCellTypeSrc)
   {
   lpCellTypeSrc = (LPSS_CELLTYPE)tbGlobalLock(hCellTypeSrc);

   if (hCellTypeDest = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                     sizeof(SS_CELLTYPE)))
      {
      lpCellTypeDest = (LPSS_CELLTYPE)tbGlobalLock(hCellTypeDest);
      SS_CopyCellType(lpSS, lpCellTypeDest, lpCellTypeSrc);
      tbGlobalUnlock(hCellTypeDest);
      }

   tbGlobalUnlock(hCellTypeSrc);
   }

return (hCellTypeDest);
}


BOOL SS_CopyCellType(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellTypeDest,
                     LPSS_CELLTYPE lpCellTypeSrc)
{
BOOL fRet = TRUE;

if (lpCellTypeSrc)
   {
   _fmemcpy(lpCellTypeDest, lpCellTypeSrc, sizeof(SS_CELLTYPE));

   switch (lpCellTypeSrc->Type)
      {
      case SS_TYPE_PIC:
         SSx_tbCopyItem(&lpCellTypeDest->Spec.Pic.hMask,
                        &lpCellTypeSrc->Spec.Pic.hMask);

         break;

      case SS_TYPE_BUTTON:
         SSx_CopyItem(&lpCellTypeDest->Spec.Button.hText,
                      &lpCellTypeSrc->Spec.Button.hText);
         if (!(lpCellTypeDest->Spec.Button.nPictureType &
             SUPERBTN_PICT_HANDLE))
            SSx_CopyItem(&lpCellTypeDest->Spec.Button.hPictName,
                         &lpCellTypeSrc->Spec.Button.hPictName);
         if (!(lpCellTypeDest->Spec.Button.nPictureDownType &
             SUPERBTN_PICT_HANDLE))
            SSx_CopyItem(&lpCellTypeDest->Spec.Button.hPictDownName,
                         &lpCellTypeSrc->Spec.Button.hPictDownName);

         #if defined(SS_VB)
         if (lpCellTypeDest->Spec.Button.hPic)
            VBRefPic(lpCellTypeDest->Spec.Button.hPic);
         if (lpCellTypeDest->Spec.Button.hPicDown)
            VBRefPic(lpCellTypeDest->Spec.Button.hPicDown);
         #elif defined(SS_OCX)
         SSOCX_AddRefPict(lpCellTypeDest->Spec.Button.lpPict);
         SSOCX_AddRefPict(lpCellTypeDest->Spec.Button.lpPictDown);
         #else
         if (lpCellTypeDest->Spec.Button.fDelHandle)
            SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.Button.hPictName);
         if (lpCellTypeDest->Spec.Button.fDelDownHandle)
            SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.Button.hPictDownName);
         #endif

         break;

      case SS_TYPE_CHECKBOX:
         SSx_CopyItem(&lpCellTypeDest->Spec.CheckBox.hText,
                      &lpCellTypeSrc->Spec.CheckBox.hText);

         if (!(lpCellTypeDest->Spec.CheckBox.bPictUpType & BT_HANDLE))
            SSx_tbCopyItem(&lpCellTypeDest->Spec.CheckBox.hPictUpName,
                           &lpCellTypeSrc->Spec.CheckBox.hPictUpName);
         if (!(lpCellTypeDest->Spec.CheckBox.bPictDownType & BT_HANDLE))
            SSx_tbCopyItem(&lpCellTypeDest->Spec.CheckBox.hPictDownName,
                           &lpCellTypeSrc->Spec.CheckBox.hPictDownName);
         if (!(lpCellTypeDest->Spec.CheckBox.bPictFocusUpType & BT_HANDLE))
            SSx_tbCopyItem(&lpCellTypeDest->Spec.CheckBox.hPictFocusUpName,
                           &lpCellTypeSrc->Spec.CheckBox.hPictFocusUpName);
         if (!(lpCellTypeDest->Spec.CheckBox.bPictFocusDownType &
             BT_HANDLE))
            SSx_tbCopyItem(&lpCellTypeDest->Spec.CheckBox.hPictFocusDownName,
                           &lpCellTypeSrc->Spec.CheckBox.hPictFocusDownName);
         if (!(lpCellTypeDest->Spec.CheckBox.bPictGrayType &
             BT_HANDLE))
            SSx_tbCopyItem(&lpCellTypeDest->Spec.CheckBox.hPictGrayName,
            &lpCellTypeSrc->Spec.CheckBox.hPictGrayName);
         if (!(lpCellTypeDest->Spec.CheckBox.bPictFocusGrayType &
             BT_HANDLE))
            SSx_tbCopyItem(&lpCellTypeDest->Spec.CheckBox.hPictFocusGrayName,
            &lpCellTypeSrc->Spec.CheckBox.hPictFocusGrayName);

         #if defined(SS_VB)
         if (lpCellTypeDest->Spec.CheckBox.hPicUp)
            VBRefPic(lpCellTypeDest->Spec.CheckBox.hPicUp);
         if (lpCellTypeDest->Spec.CheckBox.hPicDown)
            VBRefPic(lpCellTypeDest->Spec.CheckBox.hPicDown);
         if (lpCellTypeDest->Spec.CheckBox.hPicFocusUp)
            VBRefPic(lpCellTypeDest->Spec.CheckBox.hPicFocusUp);
         if (lpCellTypeDest->Spec.CheckBox.hPicFocusDown)
            VBRefPic(lpCellTypeDest->Spec.CheckBox.hPicFocusDown);
         if (lpCellTypeDest->Spec.CheckBox.hPicGray)
            VBRefPic(lpCellTypeDest->Spec.CheckBox.hPicGray);
         if (lpCellTypeDest->Spec.CheckBox.hPicFocusGray)
            VBRefPic(lpCellTypeDest->Spec.CheckBox.hPicFocusGray);
         #elif defined(SS_OCX)
         SSOCX_AddRefPict(lpCellTypeDest->Spec.CheckBox.lpPictUp);
         SSOCX_AddRefPict(lpCellTypeDest->Spec.CheckBox.lpPictDown);
         SSOCX_AddRefPict(lpCellTypeDest->Spec.CheckBox.lpPictFocusUp);
         SSOCX_AddRefPict(lpCellTypeDest->Spec.CheckBox.lpPictFocusDown);
         SSOCX_AddRefPict(lpCellTypeDest->Spec.CheckBox.lpPictGray);
         SSOCX_AddRefPict(lpCellTypeDest->Spec.CheckBox.lpPictFocusGray);
         #else
         if (lpCellTypeDest->Spec.CheckBox.fDelUpHandle)
           SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.CheckBox.hPictUpName);
         if (lpCellTypeDest->Spec.CheckBox.fDelDownHandle)
           SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.CheckBox.hPictDownName);
         if (lpCellTypeDest->Spec.CheckBox.fDelFocusUpHandle)
           SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.CheckBox.hPictFocusUpName);
         if (lpCellTypeDest->Spec.CheckBox.fDelFocusDownHandle)
           SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.CheckBox.hPictFocusDownName);
         if (lpCellTypeDest->Spec.CheckBox.fDelGrayHandle)
           SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.CheckBox.hPictGrayName);
         if (lpCellTypeDest->Spec.CheckBox.fDelFocusGrayHandle)
           SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.CheckBox.hPictFocusGrayName);
         #endif

         break;

      case SS_TYPE_PICTURE:
         if (lpCellTypeDest->Style & VPS_HANDLE)
            {
            //lpCellTypeDest->Spec.ViewPict.fDeleteHandle = FALSE;
            lpCellTypeDest->ControlID = 0;
            }
         else
            {
            LPTSTR lpPict = NULL;

            if (lpCellTypeDest->Spec.ViewPict.hPictName)
               lpPict = (LPTSTR)tbGlobalLock(
                        lpCellTypeDest->Spec.ViewPict.hPictName);
            SS_SetTypePicture(lpSS, lpCellTypeDest, lpCellTypeDest->Style,
                              lpPict);
            if (lpPict)
               tbGlobalUnlock(lpCellTypeDest->Spec.ViewPict.hPictName);
            }

         #if defined(SS_VB)
         if (lpCellTypeDest->Spec.ViewPict.hPic)
            VBRefPic(lpCellTypeDest->Spec.ViewPict.hPic);
         #elif defined(SS_OCX)
         SSOCX_AddRefPict(lpCellTypeDest->Spec.ViewPict.lpPict);
         #else
         if (lpCellTypeDest->Spec.ViewPict.fDeleteHandle)
           SS_PicRef(lpSS->lpBook, (HANDLE)lpCellTypeDest->Spec.ViewPict.hPictName);
         #endif

         break;

      case SS_TYPE_COMBOBOX:
         SSxx_tbCopyItem(&lpCellTypeDest->Spec.ComboBox.hItems,
                         &lpCellTypeSrc->Spec.ComboBox.hItems,
                         lpCellTypeSrc->Spec.ComboBox.dItemCnt);

         break;
#if SS_V80
      case SS_TYPE_CUSTOM:
         {
            LPTSTR lpszName;
            CT_HANDLE hCT;

            if( lpCellTypeDest->Type == SS_TYPE_CUSTOM )
            {
               lpszName = SS_CT_LockName(lpCellTypeDest);
               if( hCT = CustCellTypeLookup(&lpSS->lpBook->CustCellTypes, lpszName) )
               {
                  long lRefs = CustCellTypeGetRefCnt(hCT);
                  if( lRefs > 1 )
                     CustCellTypeDestroy(hCT);
               }
               SS_CT_UnlockName(lpCellTypeDest);
            }
            SSx_CopyItem(&lpCellTypeDest->Spec.Custom.hName,
                         &lpCellTypeSrc->Spec.Custom.hName);
         }
         break;
#endif
      }
   }
else
   fRet = FALSE;

return (fRet);
}


#ifndef SS_NOCALC
#ifdef SS_OLDCALC

TBGLOBALHANDLE SSx_CopyCalc(lpSS, hCalcSrc, Col, Row)

LPSPREADSHEET  lpSS;
TBGLOBALHANDLE hCalcSrc;
SS_COORD       Col;
SS_COORD       Row;
{
TBGLOBALHANDLE hCalcDest = 0;
LPSS_CALC      lpCalcDest;
LPSS_CALC      lpCalcSrc;
LPTSTR         lpFormulaSrc;
LPTSTR         lpFormulaDest;

if (hCalcSrc)
   {
   lpCalcSrc = (LPSS_CALC)tbGlobalLock(hCalcSrc);

   if (hCalcDest = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                 sizeof(SS_CALC)))
      {
      lpCalcDest = (LPSS_CALC)tbGlobalLock(hCalcDest);

      lpCalcDest->CellCoord.Col = Col;
      lpCalcDest->CellCoord.Row = Row;
      _fmemset(&lpCalcDest->Ancestors, '\0', sizeof(SS_CALCLIST));

      if (lpCalcSrc->hFormula)
         {
         lpFormulaSrc = (LPTSTR)tbGlobalLock(lpCalcSrc->hFormula);

         if (lpCalcDest->hFormula = tbGlobalAlloc(GMEM_MOVEABLE |
                                    GMEM_ZEROINIT,
                                    (lstrlen(lpFormulaSrc) + 1)
                                    * sizeof(TCHAR)))

            {
            lpFormulaDest = (LPTSTR)tbGlobalLock(lpCalcDest->hFormula);
            lstrcpy(lpFormulaDest, lpFormulaSrc);
            tbGlobalUnlock(lpCalcDest->hFormula);

            SSx_CalcTableAdd(lpSS, hCalcDest);
            }

         tbGlobalUnlock(lpCalcSrc->hFormula);
         }

      tbGlobalUnlock(hCalcDest);
      }

   tbGlobalUnlock(hCalcSrc);
   }

return (hCalcDest);
}

#endif
#endif


void SS_CopyAdjustDataCnt(LPSPREADSHEET lpSS, BYTE bDataType,
                          SS_COORD Col, SS_COORD Row)
{
if (bDataType)
   {
   if (Col != SS_ALLCOLS && Col >= lpSS->Col.DataCnt)
      {
      SS_SetDataColCnt(lpSS, Col + 1);
      SS_SetHScrollBar(lpSS);
      }

   if (Row != SS_ALLROWS && Row >= lpSS->Row.DataCnt)
      {
      SS_SetDataRowCnt(lpSS, Row + 1);
      SS_SetVScrollBar(lpSS);
      }
   }
else
   {
   if (Col != SS_ALLCOLS)
      SS_AdjustDataColCnt(lpSS, Col, Col);

   if (Row != SS_ALLROWS)
      SS_AdjustDataRowCnt(lpSS, Row, Row);
   }
}


void SS_CopyAdjustDataColCnt(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
LPSS_CELL lpCell;
BYTE      bDataType = 0;

if (lpCell = SS_LockCellItem(lpSS, Col, Row))
   {
   bDataType = lpCell->Data.bDataType;
   SS_UnlockCellItem(lpSS, Col, Row);
   }

if (bDataType)
   {
   if (Col != SS_ALLCOLS && Col >= lpSS->Col.DataCnt)
      {
      SS_SetDataColCnt(lpSS, Col + 1);
      SS_SetHScrollBar(lpSS);
      }
   }
else
   {
   if (Col != SS_ALLCOLS)
      SS_AdjustDataColCnt(lpSS, Col, Col);
   }
}


void SS_CopyAdjustDataRowCnt(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
LPSS_CELL lpCell;
BYTE      bDataType = 0;

if (lpCell = SS_LockCellItem(lpSS, Col, Row))
   {
   bDataType = lpCell->Data.bDataType;
   SS_UnlockCellItem(lpSS, Col, Row);
   }

if (bDataType)
   {
   if (Row != SS_ALLROWS && Row >= lpSS->Row.DataCnt)
      {
      SS_SetDataRowCnt(lpSS, Row + 1);
      SS_SetVScrollBar(lpSS);
      }
   }
else
   {
   if (Row != SS_ALLROWS)
      SS_AdjustDataRowCnt(lpSS, Row, Row);
   }
}

#endif
