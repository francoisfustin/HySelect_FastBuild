/*********************************************************
* SS_SPAN.C
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
#include <process.h>
#include <math.h>
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_span.h"

void SS_RemoveCellSpanRange(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lNumCols, SS_COORD lNumRows, TBGLOBALHANDLE hSpanIgnore);
static BOOL SS_MergeColCalcBlock(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                                 SS_COORD lCol, LPSS_COL lpCol, LPSS_SELBLOCK lpBlock);
static BOOL SS_MergeColCompare(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                               SS_COORD lRowComp, SS_COORD lCol, LPSS_COL lpCol);
static BOOL SS_MergeRowCalcBlock(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                                 SS_COORD lCol, LPSS_COL lpCol, LPSS_SELBLOCK lpBlock);
static BOOL SS_MergeRowCompare(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                               SS_COORD lColComp, SS_COORD lCol, LPSS_COL lpCol);


BOOL DLLENTRY SSAddCellSpan(HWND hWnd, SS_COORD lCol, SS_COORD lRow, SS_COORD lNumCols, SS_COORD lNumRows)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &lCol, &lRow);
fRet = SS_AddCellSpan(lpSS, lCol, lRow, lNumCols, lNumRows);
SS_SheetUnlock(hWnd);
return (fRet);
}


WORD DLLENTRY SSGetCellSpan(HWND hWnd, SS_COORD lCol, SS_COORD lRow, LPSS_COORD lplColAnchor,
                            LPSS_COORD lplRowAnchor, LPSS_COORD lplNumCols, LPSS_COORD lplNumRows)
{
LPSPREADSHEET lpSS;
WORD          wRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &lCol, &lRow);
wRet = SS_GetCellSpan(lpSS, lCol, lRow, lplColAnchor, lplRowAnchor, lplNumCols, lplNumRows);
SS_AdjustCellCoordsOut(lpSS, lplColAnchor, lplRowAnchor);
SS_SheetUnlock(hWnd);
return wRet;
}


void DLLENTRY SSRemoveCellSpan(HWND hWnd, SS_COORD lCol, SS_COORD lRow)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &lCol, &lRow);
SS_RemoveCellSpan(lpSS, lCol, lRow);
SS_SheetUnlock(hWnd);
}

//--------------------------------------------------------------------

BOOL SS_AddCellSpan(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lNumCols, SS_COORD lNumRows)
{
BOOL fRet = FALSE;

if (0 <= lCol && lCol < SS_GetColCnt(lpSS) && 0 <= lRow && lRow < SS_GetRowCnt(lpSS) && 0 < lNumCols && 0 < lNumRows)
	{
	SS_RemoveCellSpanRange(lpSS, lCol, lRow, lNumCols, lNumRows, 0);
	fRet = TRUE;

	if (lCol + lNumCols > SS_GetColCnt(lpSS))
		lNumCols = SS_GetColCnt(lpSS) - lCol;

	if (lCol < lpSS->Col.HeaderCnt && lCol + lNumCols > lpSS->Col.HeaderCnt)
		lNumCols = lpSS->Col.HeaderCnt - lCol;

	if (lRow + lNumRows > SS_GetRowCnt(lpSS))
		lNumRows = SS_GetRowCnt(lpSS) - lRow;

	if (lRow < lpSS->Row.HeaderCnt && lRow + lNumRows > lpSS->Row.HeaderCnt)
		lNumRows = lpSS->Row.HeaderCnt - lRow;

	if ((1 < lNumCols || 1 < lNumRows) && !SS_IsSpanOverlap(lpSS, lCol, lRow, lNumCols, lNumRows))
		{
		TBGLOBALHANDLE hNew;
		LPSS_CELLSPAN  lpNew;

		if (hNew = tbGlobalAlloc(GHND, sizeof(SS_CELLSPAN)))
			{
			lpNew = (LPSS_CELLSPAN)tbGlobalLock(hNew);
			lpNew->lCol = lCol;
			lpNew->lRow = lRow;
			lpNew->lNumCols = lNumCols;
			lpNew->lNumRows = lNumRows;
			lpNew->hNext = lpSS->hCellSpan;
			tbGlobalUnlock(hNew);
			lpSS->hCellSpan = hNew;
			SS_Alloc(lpSS, lCol, lRow);
			SS_AllocRange(lpSS, lCol, SS_ALLROWS, lCol + lNumCols - 1, SS_ALLROWS);
			SS_AllocRange(lpSS, SS_ALLCOLS, lRow, SS_ALLCOLS, lRow + lNumRows - 1);
			SS_InvalidateCellRange(lpSS, lCol, lRow, lCol + lNumCols - 1, lRow + lNumRows - 1);
			fRet = TRUE;

			// RFW - 6/21/04 - 14605
			if (lpSS->lpBook->fAllowCellOverflow)
				{
				SS_COORD i;

				for (i = lRow; i < lRow + lNumRows; i++)
					{
					SS_ResetCellOverflow(lpSS, lCol, i);
					SS_CalcCellOverflow(lpSS, NULL, NULL, lCol, i);
					SS_OverflowAdjustNeighbor(lpSS, lCol, i);

					if (lNumCols > 1)
						{
						SS_ResetCellOverflow(lpSS, lCol + lNumCols - 1, i);
						SS_CalcCellOverflow(lpSS, NULL, NULL, lCol + lNumCols - 1, i);
						SS_OverflowAdjustNeighbor(lpSS, lCol + lNumCols - 1, i);
						}
					}
				}
			}
		else
			fRet = FALSE;
		}
	else
		fRet = FALSE;
	}

return (fRet);
}


WORD SS_GetCellSpan(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_COORD lplColAnchor,
                    LPSS_COORD lplRowAnchor, LPSS_COORD lplNumCols, LPSS_COORD lplNumRows)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hNextTemp;
LPSS_CELLSPAN  lpNext;
SS_CELLSPAN    Span;
WORD           wRet = SS_SPAN_NO;

for (hNext = lpSS->hCellSpan; hNext && !wRet; )
	{
	lpNext = (LPSS_CELLSPAN)tbGlobalLock(hNext);
	if ((lCol == -1 || (lCol >= lpNext->lCol && lCol < lpNext->lCol + lpNext->lNumCols)) &&
       (lRow == -1 || (lRow >= lpNext->lRow && lRow < lpNext->lRow + lpNext->lNumRows)))
		{
		Span = *lpNext;
		if ((lCol == -1 || lCol == lpNext->lCol) &&
          (lRow == -1 || lRow == lpNext->lRow))
			wRet = SS_SPAN_ANCHOR;
		else
			wRet = SS_SPAN_YES;
		}

   hNextTemp = lpNext->hNext;
	tbGlobalUnlock(hNext);
   hNext = hNextTemp;
	}

if (wRet)
	{
	if (lplColAnchor)
		*lplColAnchor = Span.lCol;
	if (lplRowAnchor)
		*lplRowAnchor = Span.lRow;
	if (lplNumCols)
		*lplNumCols = min(Span.lNumCols, SS_GetColCnt(lpSS) - Span.lCol);
	if (lplNumRows)
		*lplNumRows = min(Span.lNumRows, SS_GetRowCnt(lpSS) - Span.lRow);
	}

return wRet;
}


WORD SS_IsSpanOverlap(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lNumCols, SS_COORD lNumRows)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hNextTemp;
LPSS_CELLSPAN  lpNext;
WORD           wRet = SS_SPANOVERLAP_NONE;

// Loop through until a match is found.  If lCol or lRow == -1
// then loop through all unless an unequal match is found.
for (hNext = lpSS->hCellSpan; hNext && !(wRet == SS_SPANOVERLAP_YES || (wRet == SS_SPANOVERLAP_EQUAL && lCol != -1 && lRow != -1)); )
	{
	lpNext = (LPSS_CELLSPAN)tbGlobalLock(hNext);
	if ((lCol == -1 || (lCol < lpNext->lCol + lpNext->lNumCols && lCol + lNumCols > lpNext->lCol)) &&
       (lRow == -1 || (lRow < lpNext->lRow + lpNext->lNumRows && lRow + lNumRows > lpNext->lRow)))
		{
		if ((lCol == -1 || (lpNext->lCol == lCol && lpNext->lNumCols == lNumCols)) &&
          (lRow == -1 || (lpNext->lRow == lRow && lpNext->lNumRows == lNumRows)))
			wRet = SS_SPANOVERLAP_EQUAL;
		else
			wRet = SS_SPANOVERLAP_YES;
		}

   hNextTemp = lpNext->hNext;
	tbGlobalUnlock(hNext);
   hNext = hNextTemp;
	}

return wRet;
}


void SS_RemoveCellSpan(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
SS_RemoveCellSpanRange(lpSS, lCol, lRow, 1, 1, 0);
}


void SS_RemoveCellSpanRange(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, SS_COORD lNumCols, SS_COORD lNumRows, TBGLOBALHANDLE hSpanIgnore)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hPrev;
TBGLOBALHANDLE hCurr;
LPSS_CELLSPAN  lpCurr;
LPSS_CELLSPAN  lpPrev;

for (hPrev = 0, hCurr = lpSS->hCellSpan; hCurr; hCurr = hNext)
	{
	lpCurr = (LPSS_CELLSPAN)tbGlobalLock(hCurr);
	hNext = lpCurr->hNext;

	/* RFW - 6/18/04 - 14598
	if (lCol <= lpCurr->lCol && lpCurr->lCol < lCol + lNumCols && lRow <= lpCurr->lRow && lpCurr->lRow < lRow + lNumRows)
	*/
	if ((lCol == -1 || (lCol <= lpCurr->lCol && lpCurr->lCol < lCol + lNumCols)) &&
       (lRow == -1 || (lRow <= lpCurr->lRow && lpCurr->lRow < lRow + lNumRows)) &&
       hCurr != hSpanIgnore)
		{
		if (hPrev)
			{
			lpPrev = (LPSS_CELLSPAN)tbGlobalLock(hPrev);
         lpPrev->hNext = lpCurr->hNext;
			tbGlobalUnlock(hPrev);
			}
		else
			lpSS->hCellSpan = lpCurr->hNext;

		SS_InvalidateCellRange(lpSS, lpCurr->lCol, lpCurr->lRow, lpCurr->lCol + lpCurr->lNumCols - 1, lpCurr->lRow + lpCurr->lNumRows - 1);
		tbGlobalUnlock(hCurr);
      tbGlobalFree(hCurr);
      }
	else
		{
		tbGlobalUnlock(hCurr);
		hPrev = hCurr;
		}
	}
}


BOOL SS_DelCellSpanRowRange(LPSPREADSHEET lpSS, SS_COORD lRow, SS_COORD lNumRows)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hPrev;
TBGLOBALHANDLE hCurr;
LPSS_CELLSPAN  lpCurr;
LPSS_CELLSPAN  lpPrev;
BOOL           fRet = FALSE;

for (hPrev = 0, hCurr = lpSS->hCellSpan; hCurr; hCurr = hNext)
	{
	lpCurr = (LPSS_CELLSPAN)tbGlobalLock(hCurr);
	hNext = lpCurr->hNext;

	if (lpCurr->lRow >= lRow && lpCurr->lRow < lRow + lNumRows)
		{
		if (hPrev)
			{
			lpPrev = (LPSS_CELLSPAN)tbGlobalLock(hPrev);
         lpPrev->hNext = lpCurr->hNext;
			tbGlobalUnlock(hPrev);
			}
		else
			lpSS->hCellSpan = lpCurr->hNext;

      tbGlobalFree(hCurr);
		fRet = TRUE;
      }
	else
		{
		hPrev = hCurr;

		if (lRow < lpCurr->lRow)
			lpCurr->lRow -= lNumRows;
		else if (lRow > lpCurr->lRow && lRow < lpCurr->lRow + lpCurr->lNumRows)
			lpCurr->lNumRows -= min(lRow + lNumRows, lpCurr->lRow + lpCurr->lNumRows) - lRow;
		}

	tbGlobalUnlock(hCurr);
	}

return (fRet);
}


void SS_InsCellSpanRowRange(LPSPREADSHEET lpSS, SS_COORD lRow, SS_COORD lNumRows)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hCurr;
LPSS_CELLSPAN  lpCurr;

for (hCurr = lpSS->hCellSpan; hCurr; hCurr = hNext)
	{
	lpCurr = (LPSS_CELLSPAN)tbGlobalLock(hCurr);
	hNext = lpCurr->hNext;

	if (lpCurr->lRow >= lRow)
		lpCurr->lRow += lNumRows;
	else if (lRow > lpCurr->lRow && lRow < lpCurr->lRow + lpCurr->lNumRows)
		lpCurr->lNumRows += lNumRows;

	tbGlobalUnlock(hCurr);
	}
}


BOOL SS_DelCellSpanColRange(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lNumCols, BOOL fInvalidate)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hPrev;
TBGLOBALHANDLE hCurr;
LPSS_CELLSPAN  lpCurr;
LPSS_CELLSPAN  lpPrev;
BOOL           fRet = FALSE;

for (hPrev = 0, hCurr = lpSS->hCellSpan; hCurr; hCurr = hNext)
	{
	lpCurr = (LPSS_CELLSPAN)tbGlobalLock(hCurr);
	hNext = lpCurr->hNext;

	if (lpCurr->lCol >= lCol && lpCurr->lCol < lCol + lNumCols)
		{
		if (hPrev)
			{
			lpPrev = (LPSS_CELLSPAN)tbGlobalLock(hPrev);
         lpPrev->hNext = lpCurr->hNext;
			if (fInvalidate)
				SS_InvalidateCellRange(lpSS, lpCurr->lCol, lpCurr->lRow, lpCurr->lCol + lpCurr->lNumCols - 1, lpCurr->lRow + lpCurr->lNumRows - 1);

			tbGlobalUnlock(hPrev);
			}
		else
			lpSS->hCellSpan = lpCurr->hNext;

      tbGlobalFree(hCurr);
		fRet = TRUE;
      }
	else
		{
		hPrev = hCurr;

		if (lCol < lpCurr->lCol)
			lpCurr->lCol -= lNumCols;
		else if (lCol > lpCurr->lCol && lCol < lpCurr->lCol + lpCurr->lNumCols)
			{
			if (fInvalidate)
				SS_InvalidateCellRange(lpSS, lpCurr->lCol, lpCurr->lRow, lpCurr->lCol + lpCurr->lNumCols - 1, lpCurr->lRow + lpCurr->lNumRows - 1);
			lpCurr->lNumCols -= min(lCol + lNumCols, lpCurr->lCol + lpCurr->lNumCols) - lCol;
			}
		}

	tbGlobalUnlock(hCurr);
	}

return (fRet);
}


void SS_InsCellSpanColRange(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lNumCols)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hCurr;
LPSS_CELLSPAN  lpCurr;

for (hCurr = lpSS->hCellSpan; hCurr; hCurr = hNext)
	{
	lpCurr = (LPSS_CELLSPAN)tbGlobalLock(hCurr);
	hNext = lpCurr->hNext;

	if (lpCurr->lCol >= lCol)
		lpCurr->lCol += lNumCols;
	else if (lCol > lpCurr->lCol && lCol < lpCurr->lCol + lpCurr->lNumCols)
		lpCurr->lNumCols += lNumCols;

	tbGlobalUnlock(hCurr);
	}
}


void SS_CopySpan(LPSPREADSHEET lpSS, SS_COORD ColDest, SS_COORD RowDest, SS_COORD ColSrc,
                 SS_COORD RowSrc)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hCurr;
LPSS_CELLSPAN  lpCurr;

for (hCurr = lpSS->hCellSpan; hCurr; hCurr = hNext)
	{
	lpCurr = (LPSS_CELLSPAN)tbGlobalLock(hCurr);
	hNext = lpCurr->hNext;

	if (lpCurr->lCol == ColSrc && lpCurr->lRow == RowSrc)
		SS_AddCellSpan(lpSS, ColDest, RowDest, lpCurr->lNumCols, lpCurr->lNumRows);

	tbGlobalUnlock(hCurr);
	}
}


void SS_MoveSpan(LPSPREADSHEET lpSS, SS_COORD ColDest, SS_COORD RowDest, SS_COORD ColSrc,
                 SS_COORD RowSrc)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hCurr;
LPSS_CELLSPAN  lpCurr;

// RFW - 6/18/04 - 14598
// Delete spans that overlap the dest
SS_RemoveCellSpanRange(lpSS, ColDest, RowDest, 1, 1, 0);

for (hCurr = lpSS->hCellSpan; hCurr; hCurr = hNext)
	{
	lpCurr = (LPSS_CELLSPAN)tbGlobalLock(hCurr);

	if ((ColSrc == -1 || lpCurr->lCol == ColSrc) && (RowSrc == -1 || lpCurr->lRow == RowSrc))
		{
		if (ColDest != -1)
			lpCurr->lCol = ColDest;
		if (RowDest != -1)
			lpCurr->lRow = RowDest;

		// RFW - 6/18/04 - 14598
		// Delete spans that overlap the dest
		SS_RemoveCellSpanRange(lpSS, ColDest, RowDest, lpCurr->lNumCols, lpCurr->lNumRows, hCurr);
		}

	hNext = lpCurr->hNext;
	tbGlobalUnlock(hCurr);
	}
}


void SS_FreeSpanTable(LPSPREADSHEET lpSS)
{
while (lpSS->hCellSpan)
   {
   TBGLOBALHANDLE hFree = lpSS->hCellSpan;
   LPSS_CELLSPAN  lpFree;

   lpFree = (LPSS_CELLSPAN)tbGlobalLock(hFree);
   lpSS->hCellSpan = lpFree->hNext;
   tbGlobalUnlock(hFree);
   tbGlobalFree(hFree);
   }
}


BOOL SS_GetSpanLR(LPSPREADSHEET lpSS, LPSS_COORD lplCol, LPSS_COORD lplRow)
{
return (SS_GetSpanLREx(lpSS, lplCol, lplRow, -1, -1));
}


BOOL SS_GetSpanLREx(LPSPREADSHEET lpSS, LPSS_COORD lplCol, LPSS_COORD lplRow, SS_COORD lColLR, SS_COORD lRowLR)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hCurr;
LPSS_CELLSPAN  lpCurr;
SS_COORD       lCol = 0, lRow = 0;

for (hCurr = lpSS->hCellSpan; hCurr; hCurr = hNext)
	{
	lpCurr = (LPSS_CELLSPAN)tbGlobalLock(hCurr);
	hNext = lpCurr->hNext;

	if (lColLR == -1 || (lpCurr->lCol <= lColLR && lpCurr->lRow <= lRowLR))
		lCol = max(lCol, lpCurr->lCol + lpCurr->lNumCols - 1);
	if (lRowLR == -1 || (lpCurr->lRow <= lRowLR && lpCurr->lCol <= lColLR))
		lRow = max(lRow, lpCurr->lRow + lpCurr->lNumRows - 1);

	tbGlobalUnlock(hCurr);
	}

if (lplCol)
	*lplCol = lCol;

if (lplRow)
	*lplRow = lRow;

return (lpSS->hCellSpan != 0);
}


BOOL SS_SpanCalcBlock(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_SELBLOCK lpBlock)
{
TBGLOBALHANDLE hNext;
LPSS_CELLSPAN  lpNext;
BOOL bRet = FALSE;

for (hNext = lpSS->hCellSpan; hNext && !bRet; )
	{
	lpNext = (LPSS_CELLSPAN)tbGlobalLock(hNext);
	if (lCol >= lpNext->lCol && lCol < lpNext->lCol + lpNext->lNumCols &&
	    lRow >= lpNext->lRow && lRow < lpNext->lRow + lpNext->lNumRows)
		{
		lpBlock->UL.Col = lpNext->lCol;
		lpBlock->UL.Row = lpNext->lRow;
		lpBlock->LR.Col = min(lpNext->lCol + lpNext->lNumCols - 1, SS_GetColCnt(lpSS) - 1);
		lpBlock->LR.Row = min(lpNext->lRow + lpNext->lNumRows - 1, SS_GetRowCnt(lpSS) - 1);
		bRet = TRUE;
		}

   hNext = lpNext->hNext;
	tbGlobalUnlock(hNext);
	}

if (bRet)
	{
	// Assure that spanned cells stay inside or outside of Headers and Frozen Cols and Rows
	if (lpBlock->UL.Col < lpSS->Col.HeaderCnt)
		lpBlock->LR.Col = min(lpBlock->LR.Col, lpSS->Col.HeaderCnt - 1);
	else if (lpBlock->UL.Col < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
		lpBlock->LR.Col = min(lpBlock->LR.Col, lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1);

	if (lpBlock->UL.Row < lpSS->Row.HeaderCnt)
		lpBlock->LR.Row = min(lpBlock->LR.Row, lpSS->Row.HeaderCnt - 1);
	else if (lpBlock->UL.Row < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
		lpBlock->LR.Row = min(lpBlock->LR.Row, lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1);

	if (!(lCol >= lpNext->lCol && lCol <= lpBlock->LR.Col &&
	      lRow >= lpNext->lRow && lRow <= lpBlock->LR.Row))
		bRet = FALSE;
	}

return (bRet);
}


BOOL SS_SpanMergeCalcBlock(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                           SS_COORD lCol, LPSS_COL lpCol, LPSS_SELBLOCK lpBlock)
{
BOOL bRet = FALSE;

// RFW - 12/18/03 - 13339
// I changed the order so that the merges are checked
// before the spans.
bRet = SS_MergeCalcBlock(lpSS, lRow, lpRow, lCol, lpCol, lpBlock);

if (!bRet)
	bRet = SS_SpanCalcBlock(lpSS, lCol, lRow, lpBlock);

if (bRet)
	SS_AddBlockToPaintSpanList(lpSS, lpBlock);

return (bRet);
}


void SS_SpanInvalidateRange(LPSPREADSHEET lpSS, SS_COORD lCol1, SS_COORD lRow1, SS_COORD lCol2,
                            SS_COORD lRow2, BOOL fAnchorOnly)
{
TBGLOBALHANDLE hNext;
TBGLOBALHANDLE hNextTemp;
LPSS_CELLSPAN  lpNext;

if (lCol1 == SS_ALLCOLS)
	lCol1 = 0;
if (lRow1 == SS_ALLROWS)
	lRow1 = 0;
if (lCol2 == SS_ALLCOLS)
	lCol2 = SS_GetColCnt(lpSS) - 1;
if (lRow2 == SS_ALLROWS)
	lRow2 = SS_GetRowCnt(lpSS) - 1;

for (hNext = lpSS->hCellSpan; hNext; )
	{
	lpNext = (LPSS_CELLSPAN)tbGlobalLock(hNext);

	if (fAnchorOnly)
		{
		if (lpNext->lCol >= lCol1 && lpNext->lCol <= lCol2 &&
          lpNext->lRow >= lRow1 && lpNext->lRow <= lRow2)
			SSx_InvalidateCellRange(lpSS, lpNext->lCol, lpNext->lRow, lpNext->lCol + lpNext->lNumCols - 1,
                                 lpNext->lRow + lpNext->lNumRows - 1);
		}
	else
		{
			if (lCol1 < lpNext->lCol + lpNext->lNumCols && lRow1 < lpNext->lRow + lpNext->lNumRows &&
		       lCol2 >= lpNext->lCol && lRow2 >= lpNext->lRow)
			SSx_InvalidateCellRange(lpSS, lpNext->lCol, lpNext->lRow, lpNext->lCol + lpNext->lNumCols - 1,
                                 lpNext->lRow + lpNext->lNumRows - 1);
		}

   hNextTemp = lpNext->hNext;
	tbGlobalUnlock(hNext);
   hNext = hNextTemp;
	}
}

#define SS_HIGHESTPRECEDENCE_COL 0
#define SS_HIGHESTPRECEDENCE_ROW 1
#define SS_HIGHESTPRECEDENCE     SS_HIGHESTPRECEDENCE_ROW


BOOL SS_SpanIsCellAlreadyPaintedEx(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow, LPSS_SELBLOCK lpBlock)
{
BOOL fFound = FALSE;

if (lpSS->hPaintSpanList && lpSS->dPaintSpanListCnt)
   {
   LPSS_SELBLOCK lpPaintSpanList = (LPSS_SELBLOCK)tbGlobalLock(lpSS->hPaintSpanList);
   short         i;

   for (i = 0; i < lpSS->dPaintSpanListCnt && !fFound; i++)
      if (lCol >= lpPaintSpanList[i].UL.Col && lRow >= lpPaintSpanList[i].UL.Row &&
          lCol <= lpPaintSpanList[i].LR.Col && lRow <= lpPaintSpanList[i].LR.Row)
			{
			if (lpBlock)
				{
				lpBlock->UL.Col = lpPaintSpanList[i].UL.Col;
				lpBlock->LR.Col = lpPaintSpanList[i].LR.Col;
				lpBlock->UL.Row = lpPaintSpanList[i].UL.Row;
				lpBlock->LR.Row = lpPaintSpanList[i].LR.Row;
				}

         fFound = TRUE;
			}

   tbGlobalUnlock(lpSS->hPaintSpanList);
   }

return (fFound);
}


BOOL SS_SpanIsCellAlreadyPainted(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow)
{
return (SS_SpanIsCellAlreadyPaintedEx(lpSS, lCol, lRow, NULL));
}


BOOL SS_AddBlockToPaintSpanList(LPSPREADSHEET lpSS, LPSS_SELBLOCK lpBlock)
{
BOOL fRet = FALSE;

if (lpSS->dPaintSpanListCnt >= lpSS->dPaintSpanListAllocCnt)
	lpSS->hPaintSpanList = SSx_AllocList(lpSS->hPaintSpanList,
		  									&lpSS->dPaintSpanListAllocCnt,
											sizeof(SS_SELBLOCK));

if (lpSS->hPaintSpanList)
	{
	LPSS_SELBLOCK lpPaintSpanList = (LPSS_SELBLOCK)tbGlobalLock(lpSS->hPaintSpanList);

	lpPaintSpanList[lpSS->dPaintSpanListCnt] = *lpBlock;
	lpSS->dPaintSpanListCnt++;

	tbGlobalUnlock(lpSS->hPaintSpanList);
	}

return (fRet);
}


void SS_SpanFreePaintList(LPSPREADSHEET lpSS)
{
if (lpSS->hPaintSpanList)
   tbGlobalFree(lpSS->hPaintSpanList);
lpSS->hPaintSpanList = 0;
lpSS->dPaintSpanListCnt = 0;
lpSS->dPaintSpanListAllocCnt = 0;
}


BOOL SS_MergeCalcBlock(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                       SS_COORD lCol, LPSS_COL lpCol, LPSS_SELBLOCK lpBlock)
{
LPSS_ROW lpRowOrig = lpRow;
LPSS_COL lpColOrig = lpCol;
short    i;
BOOL     fRet = FALSE;

if (!lpRowOrig)
	lpRow = SS_LockRowItem(lpSS, lRow);

if (!lpColOrig)
	lpCol = SS_LockColItem(lpSS, lCol);

for (i = 0; i < 2 && !fRet; i++)
   {
   if ((SS_HIGHESTPRECEDENCE == SS_HIGHESTPRECEDENCE_COL && i == 0) ||
       (SS_HIGHESTPRECEDENCE != SS_HIGHESTPRECEDENCE_COL && i == 1))
      fRet = SS_MergeColCalcBlock(lpSS, lRow, lpRow, lCol, lpCol, lpBlock);
   else
      fRet = SS_MergeRowCalcBlock(lpSS, lRow, lpRow, lCol, lpCol, lpBlock);
   }

if (fRet)
	{
	// Assure that spanned cells stay inside or outside of Frozen Cols and Rows
	if (lCol < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
		lpBlock->LR.Col = min(lpBlock->LR.Col, lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1);
	else
		lpBlock->UL.Col = max(lpBlock->UL.Col, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);

	if (lRow < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
		lpBlock->LR.Row = min(lpBlock->LR.Row, lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1);
	else
		lpBlock->UL.Row = max(lpBlock->UL.Row, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
	}

if (!lpRowOrig)
	SS_UnlockRowItem(lpSS, lRow);

if (!lpColOrig)
	SS_UnlockColItem(lpSS, lCol);

return (fRet);
}


BOOL SS_MergeColCalcBlock(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                          SS_COORD lCol, LPSS_COL lpCol, LPSS_SELBLOCK lpBlock)
{
BOOL fRet = FALSE;

if (lpCol && lpCol->bMerge)
   {
   SS_COORD i;
	SS_COORD lMaxRows;
   BOOL     fContinue;

   if (!SS_SpanIsCellAlreadyPaintedEx(lpSS, lCol, lRow, lpBlock))
      {
      if (!SS_IsCellDataEmpty(lpSS, lpCol, lpRow, NULL, lCol, lRow))
         {
			// RFW - 3/9/04 - 13833
			SS_COORD lTop = lRow >= lpSS->Row.HeaderCnt ? lpSS->Row.HeaderCnt : 0;

         lpBlock->UL.Col = lCol;
         lpBlock->LR.Col = lCol;
         lpBlock->UL.Row = lRow;
         lpBlock->LR.Row = lRow;

         /**************************
         * Check all previous rows
         **************************/

         for (i = lRow - 1, fContinue = TRUE; i >= lTop && fContinue; i--)
            if (fContinue = SS_MergeColCompare(lpSS, lRow, lpRow, i, lCol, lpCol))
               lpBlock->UL.Row = i;

         /*******************************
         * Now Check all following rows
         *******************************/

         lMaxRows = SS_GetRowCnt(lpSS);
         for (i = lRow + 1, fContinue = TRUE; i < lMaxRows && fContinue; i++)
            if (fContinue = SS_MergeColCompare(lpSS, lRow, lpRow, i, lCol, lpCol))
               lpBlock->LR.Row = i;

         if (lpBlock->UL.Row < lpBlock->LR.Row)
            fRet = TRUE;
         }
      }
	else
		fRet = TRUE;
   }

return (fRet);
}


BOOL SS_MergeColCompare(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                        SS_COORD lRowComp, SS_COORD lCol, LPSS_COL lpCol)
{
LPSS_ROW lpRowComp;
BOOL     fRet = FALSE;

lpRowComp = SS_LockRowItem(lpSS, lRowComp);

if (!(SS_HIGHESTPRECEDENCE == SS_HIGHESTPRECEDENCE_ROW && lpRowComp && lpRowComp->bMerge &&
      SS_SpanIsCellAlreadyPainted(lpSS, lCol, lRowComp)))
   {
   if (!SS_Compare(lpSS, lpCol, lCol, lpRow, lRow, lpCol, lCol, lpRowComp, lRowComp, SS_SORT_ASCENDING, FALSE))
      {
      fRet = TRUE;

      if (lpCol->bMerge == SS_MERGE_RESTRICTED)
         {
			/* RFW - 8/3/05 - 16346
         if (lCol > lpSS->Col.HeaderCnt)
			*/
         if (lCol > lpSS->Col.HeaderCnt || (lCol > 0 && lCol < lpSS->Col.HeaderCnt))
            {
            LPSS_COL lpColPrev;
            SS_COORD lColPrev = lCol - 1;

            if (lpColPrev = SS_LockColItem(lpSS, lColPrev))
					{
					if (lpColPrev->bMerge)
						{
						if (!SS_IsCellDataEmpty(lpSS, lpColPrev, lpRow, NULL, lColPrev, lRow))
							fRet = SS_MergeColCompare(lpSS, lRow, lpRow, lRowComp, lColPrev, lpColPrev);
						else
							fRet = FALSE;
						}

					SS_UnlockColItem(lpSS, lColPrev);
					}
				}
         }

      /*******************************************************
      * Now check to see if the cell has already been merged
      *******************************************************/

      if (fRet == TRUE && lpRowComp && lpRowComp->bMerge &&
          SS_HIGHESTPRECEDENCE != SS_HIGHESTPRECEDENCE_COL)
         {
         SS_COORD lMaxCols = SS_GetColCnt(lpSS);

         /*********************
         * Check previous col
         *********************/

         if (lCol - 1 >= lpSS->Col.HeaderCnt)
            if (SS_MergeRowCompare(lpSS, lRowComp, lpRowComp,
                                   lCol - 1, lCol, lpCol))
               fRet = FALSE;

         /*********************
         * Now Check next col
         *********************/

         if (fRet && lCol + 1 < lMaxCols)
            if (SS_MergeRowCompare(lpSS, lRowComp, lpRowComp,
                                   lCol + 1, lCol, lpCol))
               fRet = FALSE;
         }
      }
   }

if (lpRowComp)
	SS_UnlockRowItem(lpSS, lRowComp);

return (fRet);
}


BOOL SS_MergeRowCalcBlock(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                          SS_COORD lCol, LPSS_COL lpCol, LPSS_SELBLOCK lpBlock)
{
BOOL fRet = FALSE;

if (lpRow && lpRow->bMerge)
   {
   SS_COORD lMaxCols;
   SS_COORD i;
   BOOL     fContinue;

   if (!SS_SpanIsCellAlreadyPaintedEx(lpSS, lCol, lRow, lpBlock))
      {
      if (!SS_IsCellDataEmpty(lpSS, lpCol, lpRow, NULL, lCol, lRow))
         {
			// RFW - 3/9/04 - 13833
			SS_COORD lLeft = lCol >= lpSS->Col.HeaderCnt ? lpSS->Col.HeaderCnt : 0;

         lpBlock->UL.Col = lCol;
         lpBlock->LR.Col = lCol;
         lpBlock->UL.Row = lRow;
         lpBlock->LR.Row = lRow;

         /**************************
         * Check all previous cols
         **************************/

//         if (lpSS->fColFirstDrawn)
            {
            for (i = lCol - 1, fContinue = TRUE; i >= lLeft && fContinue; i--)
               if (fContinue = SS_MergeRowCompare(lpSS, lRow, lpRow, i, lCol, lpCol))
                  lpBlock->UL.Col = i;
            }

         /*******************************
         * Now Check all following cols
         *******************************/

         lMaxCols = SS_GetColCnt(lpSS);
         for (i = lCol + 1, fContinue = TRUE; i < lMaxCols && fContinue; i++)
            if (fContinue = SS_MergeRowCompare(lpSS, lRow, lpRow, i,
                                               lCol, lpCol))
               lpBlock->LR.Col = i;

         if (lpBlock->UL.Col < lpBlock->LR.Col)
            fRet = TRUE;
         }
      }
	else
		fRet = TRUE;
   }

return (fRet);
}


BOOL SS_MergeRowCompare(LPSPREADSHEET lpSS, SS_COORD lRow, LPSS_ROW lpRow,
                        SS_COORD lColComp, SS_COORD lCol, LPSS_COL lpCol)
{
LPSS_COL lpColComp;
BOOL     fRet = FALSE;

lpColComp = SS_LockColItem(lpSS, lColComp);

if (!(SS_HIGHESTPRECEDENCE == SS_HIGHESTPRECEDENCE_COL && lpColComp && lpColComp->bMerge &&
      SS_SpanIsCellAlreadyPainted(lpSS, lColComp, lRow)))
   {
   if (!SS_Compare(lpSS, lpCol, lCol, lpRow, lRow, lpColComp, lColComp, lpRow, lRow, SS_SORT_ASCENDING, FALSE))
      {
      fRet = TRUE;

      if (lpRow->bMerge == SS_MERGE_RESTRICTED)
         {
         if (lRow > lpSS->Row.HeaderCnt)
            {
            LPSS_ROW lpRowPrev;
            SS_COORD lRowPrev = lRow - 1;

            if (lpRowPrev = SS_LockRowItem(lpSS, lRowPrev))
					{
					if (lpRowPrev->bMerge)
						{
						if (!SS_IsCellDataEmpty(lpSS, lpCol, lpRowPrev, NULL, lCol, lRowPrev))
							fRet = SS_MergeRowCompare(lpSS, lRowPrev, lpRowPrev, lColComp, lCol, lpCol);
						else
							fRet = FALSE;
						}

					SS_UnlockRowItem(lpSS, lRowPrev);
					}
            }
         }

      /*******************************************************
      * Now check to see if the cell has already been merged
      *******************************************************/

      if (fRet == TRUE && lpColComp && lpColComp->bMerge &&
          SS_HIGHESTPRECEDENCE == SS_HIGHESTPRECEDENCE_COL)
         {
         SS_COORD lMaxRows = SS_GetRowCnt(lpSS);

         /*********************
         * Check previous row
         *********************/

         if (lRow - 1 >= lpSS->Row.HeaderCnt)
            if (SS_MergeColCompare(lpSS, lRow, lpRow, lRow - 1,
                                   lColComp, lpColComp))
               fRet = FALSE;

         /*********************
         * Now Check next row
         *********************/

         if (fRet && lRow + 1 < lMaxRows)
            if (SS_MergeColCompare(lpSS, lRow, lpRow, lRow + 1,
                                   lColComp, lpColComp))
               fRet = FALSE;
         }
      }
   }

if (lpColComp)
	SS_UnlockColItem(lpSS, lColComp);

return (fRet);
}


void SS_MergeInvalidateRange(LPSPREADSHEET lpSS, SS_COORD lCol1, SS_COORD lRow1, SS_COORD lCol2,
                             SS_COORD lRow2)
{
LPSS_COL lpCol;
LPSS_ROW lpRow;
SS_COORD i;

lRow1 = max(lRow1, lpSS->Row.UL);
lRow2 = min(lRow2, SS_GetBottomCell(lpSS, lpSS->Row.UL)) + 1;

for (i = lRow1; i <=  lRow2; i++)
   {
	if (lpRow = SS_LockRowItem(lpSS, i))
		{
		if (lpRow->bMerge)
			SS_InvalidateRow(lpSS, i);

		SS_UnlockRowItem(lpSS, i);
		}
	}

lCol1 = max(lCol1, lpSS->Col.UL);
lCol2 = min(lCol2, SS_GetRightCell(lpSS, lpSS->Col.UL)) + 1;

for (i = lCol1; i <= lCol2; i++)
   {
	if (lpCol = SS_LockColItem(lpSS, i))
		{
		if (lpCol->bMerge)
			SS_InvalidateCol(lpSS, i);

		SS_UnlockColItem(lpSS, i);
		}
	}
}