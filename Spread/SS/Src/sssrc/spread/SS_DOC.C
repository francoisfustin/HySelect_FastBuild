/*********************************************************
* SS_DOC.C       
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
#include "..\..\..\..\fplibs\fptools\src\fptools.h"
#include "..\classes\wintools.h"
#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_main.h"
#include "ss_multi.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_type.h"
#include "ss_win.h"
#ifdef SS_V30
#include "ss_user.h"
#endif
#ifdef SS_V40
#include "ss_span.h"
#endif

#include "..\edit\editfld.h"
#ifdef SS_VB
//#include <vbapi.h>
#include "..\..\vbsrc\vbspread.h"

// RFW - 8/3/94
void VBSS_SetMax(HWND hWnd);
#endif

#ifdef SS_V30
#define SS_IsPasteRowHeader(lpBook) (lpBook->bClipboardOptions & SS_CLIP_PASTEROWHEADERS)

#define SS_IsCopyRowHeader(lpBook)  (lpBook->bClipboardOptions & SS_CLIP_COPYROWHEADERS)

#define SS_IsPasteColHeader(lpBook) (lpBook->bClipboardOptions & SS_CLIP_PASTECOLHEADERS)

#define SS_IsCopyColHeader(lpBook)  (lpBook->bClipboardOptions & SS_CLIP_COPYCOLHEADERS)
#else
#define SS_IsPasteHeader(lpBook) TRUE
#define SS_IsCopyHeader(lpBook)  TRUE
#endif


BOOL DLLENTRY SS_ClipboardCopy(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, BOOL fCut);
BOOL          SSx_ValidatePasteData(LPSPREADSHEET lpSS, LPTSTR lpszText,
                                    LPSS_CELLCOORD lpCellUL,
                                    LPSS_CELLCOORD lpCellLR,
                                    BOOL fValidate);
long          hStrLen(HPTSTR hpStr);
short         SS_GetPrintAbortMsg(LPSS_BOOK lpBook, LPTSTR lpszText, short nLen);
short         SS_GetPrintJobName(LPSS_BOOK lpBook, LPTSTR lpszText, short nLen);


void SS_SetDocLock(LPSPREADSHEET lpSS, BOOL Lock)
{
LPSS_CELL     lpCell;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
SS_COORD      i;
SS_COORD      j;

/**********************************
* Clear All Cols, Rows, and Cells
**********************************/

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      lpRow->RowLocked = SS_LOCKED_DEF;

      for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
         if (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i))
            lpCell->CellLocked = SS_LOCKED_DEF;
      }
   }

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   if (lpCol = SS_LockColItem(lpSS, i))
      lpCol->ColLocked = SS_LOCKED_DEF;

lpSS->DocumentLocked = Lock;

if (lpSS->LockColor.ForegroundId || lpSS->LockColor.BackgroundId)
   SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
}


BOOL SSGetDocLock(HWND hWnd)
{
LPSPREADSHEET lpSS;
BOOL       DocumentLocked;

lpSS = SS_SheetLock(hWnd);
DocumentLocked = lpSS->DocumentLocked;
SS_SheetUnlock(hWnd);
return (DocumentLocked);
}


BOOL SSGetDocVisible(hWnd)

HWND hWnd;
{
return (IsWindowVisible(hWnd));
}


BOOL SS_ClearDoc(LPSPREADSHEET lpSS)
{
#ifdef SS_V70
   return (SS_ResetSheet(lpSS->lpBook, lpSS->nSheetIndex));
#else
	SS_Reset(lpSS->lpBook);
   return TRUE;
#endif

/* RFW - 3/10/04 - 13842
SS_COORD Col;
SS_COORD Row;

#ifdef SS_UTP
if (lpSS->fUseScrollArrows)
   {
   for (Col = lpSS->Col.HeaderCnt;
        Col < lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1; Col++)
      SS_DeAllocCol(lpSS, Col, FALSE);

   for (Col = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
        Col < lpSS->Col.AllocCnt; Col++)
      SS_DeAllocCol(lpSS, Col, FALSE);
   }
else
   for (Col = lpSS->Col.HeaderCnt; Col < lpSS->Col.AllocCnt; Col++)
      SS_DeAllocCol(lpSS, Col, FALSE);

#else

for (Col = lpSS->Col.HeaderCnt; Col < lpSS->Col.AllocCnt; Col++)
   SS_DeAllocCol(lpSS, Col, FALSE);

#endif

for (Row = lpSS->Row.HeaderCnt; Row < lpSS->Row.AllocCnt; Row++)
   SS_DeAllocRow(lpSS, Row, FALSE);

for (Row = 0; Row < lpSS->Row.HeaderCnt; Row++)
	for (Col = 0; Col < lpSS->Col.HeaderCnt; Col++)
		SS_DeAllocCell(lpSS, Col, Row, FALSE);

#ifdef SS_UTP
lpSS->Col.AllocCnt = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;
#else
lpSS->Col.AllocCnt = lpSS->Col.HeaderCnt;
#endif

lpSS->Row.AllocCnt = lpSS->Row.HeaderCnt;

lpSS->Col.DataCnt = lpSS->Col.HeaderCnt;
lpSS->Row.DataCnt = lpSS->Row.HeaderCnt;

// RFW - 2/5/04 - 13842
#ifdef SS_V70
SS_SetTypeEdit(&lpSS->DefaultCellType, ES_LEFT |
               ES_AUTOHSCROLL, 32000, SS_CHRSET_CHR, SS_CASE_NOCASE);
#else
SS_SetTypeEdit(&lpSS->DefaultCellType, ES_LEFT |
               ES_AUTOHSCROLL, 60, SS_CHRSET_CHR, SS_CASE_NOCASE);
#endif // SS_V70

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

return (TRUE);
*/
}


BOOL SS_SetDocDefaultFont(LPSPREADSHEET lpSS, HFONT hFont, BOOL DeleteFont, LPBOOL lpfDeleteFontObject)
{
LPSS_CELL     lpCell;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
SS_COORD      i;
SS_COORD      j;

lpSS->DefaultFontId = SS_InitFont(lpSS->lpBook, hFont, DeleteFont, TRUE,
                                  lpfDeleteFontObject);

lpSS->Row.dCellSizeInPixels = SS_CalcRowHeight(lpSS, -1);

/**********************************
* Clear All Cols, Rows, and Cells
**********************************/

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      lpRow->FontId = SS_FONT_NONE;
      if (lpRow->dRowHeightX100 != SS_WIDTH_DEFAULT)
         lpRow->dRowHeightInPixels = SS_CalcRowHeight(lpSS, i);
      else
         lpRow->dRowHeightInPixels = SS_WIDTH_DEFAULT;

      for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
         if (lpCell = SS_LockCellItem(lpSS, j, i))
            lpCell->FontId = SS_FONT_NONE;
      }
   }

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   if (lpCol = SS_LockColItem(lpSS, i))
      lpCol->FontId = SS_FONT_NONE;

if (lpSS->lpBook->fAllowCellOverflow)
   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
      for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.DataCnt; j++)
         SS_OverflowAdjustNeighbor(lpSS, j, i);

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
return (TRUE);
}

#if SS_V80
BOOL SS_SetDocDefaultLogFont(LPSPREADSHEET lpSS, LOGFONT *pLogFont)
{
LPSS_CELL     lpCell;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
SS_COORD      i;
SS_COORD      j;

lpSS->DefaultFontId = SS_InitFontLogFont(lpSS->lpBook, pLogFont);

lpSS->Row.dCellSizeInPixels = SS_CalcRowHeight(lpSS, -1);

/**********************************
* Clear All Cols, Rows, and Cells
**********************************/

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      lpRow->FontId = SS_FONT_NONE;
      if (lpRow->dRowHeightX100 != SS_WIDTH_DEFAULT)
         lpRow->dRowHeightInPixels = SS_CalcRowHeight(lpSS, i);
      else
         lpRow->dRowHeightInPixels = SS_WIDTH_DEFAULT;

      for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
         if (lpCell = SS_LockCellItem(lpSS, j, i))
            lpCell->FontId = SS_FONT_NONE;
      }
   }

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   if (lpCol = SS_LockColItem(lpSS, i))
      lpCol->FontId = SS_FONT_NONE;

if (lpSS->lpBook->fAllowCellOverflow)
   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
      for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.DataCnt; j++)
         SS_OverflowAdjustNeighbor(lpSS, j, i);

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
return (TRUE);
}
#endif

BOOL SSGetDocDefaultFont(HWND hWnd, LPSS_FONT Font)
{
LPSPREADSHEET lpSS;

if (lpSS = SS_SheetLock(hWnd))
   {
   if (!SS_GetFont(lpSS->lpBook, Font, lpSS->DefaultFontId))
      {
      SS_SheetUnlock(hWnd);
      return (FALSE);
      }

   SS_SheetUnlock(hWnd);
   }

return (TRUE);
}


BOOL SS_SetDocDefaultCellType(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
LPSS_CELL      lpCell;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
SS_DATA        Data;
TBGLOBALHANDLE hData;
LPTSTR         lpData;
SS_COORD       i;
SS_COORD       j;
BOOL           fClear = FALSE;

/**********************************
* Clear All Cols, Rows, and Cells
**********************************/

/* RFW - 1/6/05 - 15433
if (CellType->Type != lpSS->DefaultCellType.Type)
	fClear = TRUE;
*/
#if SS_V80
SS_CT_Unref(lpSS, &lpSS->DefaultCellType);
#endif
_fmemcpy(&lpSS->DefaultCellType, CellType, sizeof(SS_CELLTYPE));
#if SS_V80
SS_CT_Ref(lpSS, &lpSS->DefaultCellType);
#endif

/* RFW - 1/6/05 - 15433
// RFW - 5/21/04 - 14273
// Only clear all celltypes if the default celltype changes
if (fClear)
*/
	{
	for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
		if (lpCol = SS_LockColItem(lpSS, i))
			{
			if (lpCol->hCellType)
				SS_DeAllocCellType(lpSS, lpCol->hCellType);

			lpCol->hCellType = 0;
			}

	for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
		{
		if (lpRow = SS_LockRowItem(lpSS, i))
			{
			if (lpRow->hCellType)
				SS_DeAllocCellType(lpSS, lpRow->hCellType);

			lpRow->hCellType = 0;

			for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
				if (lpCell = SS_LockCellItem(lpSS, j, i))
					{
					if (lpCell->hCellType)
						SS_DeAllocCellType(lpSS, lpCell->hCellType);

					lpCell->hCellType = 0;

					// Clear the data if incompatible celltypes.

					if (SS_IsCellTypeFloatNum(CellType) &&
						 SS_GetDataStruct(lpSS, NULL, lpRow, lpCell, &Data, j, i) &&
						 Data.bDataType == SS_TYPE_FLOAT)
						;  // No need to reformat data
					else if (hData = SS_GetData(lpSS, CellType, j, i, FALSE))
						{
						lpData = (LPTSTR)tbGlobalLock(hData);
						SS_SetDataRange(lpSS, j, i, j, i, lpData, FALSE, FALSE, FALSE);
						tbGlobalUnlock(hData);
						}
					/* RFW - 2/7/06 - 18083
					else 
				// BUG 01805 (1-1)
				// If The Cell Is Overflowed By Other Cell
				// It Is Still Empty But Not Real Edit Type
				// So We Can't Set It To SS_TYPE_EDIT Here
				// Modified By HaHa 1999.10.19
				if (!lpCell->Data.bOverflow)
						lpCell->Data.bDataType = SS_TYPE_EDIT;
					*/

					SS_UnlockCellItem(lpSS, j, i);
					}

			SS_UnlockRowItem(lpSS, i);
			}
		}
	}

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

return (TRUE);
}


BOOL SS_GetDocDefaultCellType(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType)
{
_fmemcpy(CellType, &lpSS->DefaultCellType, sizeof(SS_CELLTYPE));
return (TRUE);
}


BOOL DLLENTRY SSGetEditMode(HWND hWnd)
{
LPSS_BOOK lpBook;
BOOL      EditModeOn;

lpBook = SS_BookLock(hWnd);
EditModeOn = SS_GetEditMode(lpBook);
SS_BookUnlock(hWnd);
return (EditModeOn);
}


BOOL SS_GetEditMode(LPSS_BOOK lpBook)
{
return lpBook->EditModeOn;
}


BOOL DLLENTRY SSSetEditMode(HWND hWnd, BOOL fEditModeOn)
{
LPSS_BOOK lpBook;
BOOL      fEditModeOnOld;

lpBook = SS_BookLock(hWnd);
fEditModeOnOld = SS_SetEditMode(lpBook, fEditModeOn);
SS_BookUnlock(hWnd);
return (fEditModeOnOld);
}


BOOL SS_SetEditMode(LPSS_BOOK lpBook, BOOL fEditModeOn)
{
BOOL fEditModeOnOld;

fEditModeOnOld = lpBook->EditModeOn;

if (lpBook->hWnd && IsWindowVisible(lpBook->hWnd))
#ifdef SS_GP
   SendMessage(lpBook->hWnd, SSM_SETEDITMODE, fEditModeOn, 0L);
#else
	{
	LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
   SS_PostSetEditMode(lpSS, fEditModeOn, 0L);
	SS_BookUnlockActiveSheet(lpBook);
	}
#endif

return (fEditModeOnOld);
}


BOOL DLLENTRY SSGetActiveCell(HWND hWnd, LPSS_COORD CellCol, LPSS_COORD CellRow)
{
LPSPREADSHEET lpSS;
BOOL       bRet;

lpSS = SS_SheetLock(hWnd);

bRet = SS_GetActiveCell(lpSS, CellCol, CellRow);
SS_AdjustCellCoordsOut(lpSS, CellCol, CellRow);
if (CellCol && lpSS->Col.Max == 0 && *CellCol > 0)
	*CellCol = 0;
if (CellRow && lpSS->Row.Max == 0 && *CellRow > 0)
	*CellRow = 0;

SS_SheetUnlock(hWnd);
return (bRet);
}


BOOL SS_GetActiveCell(LPSPREADSHEET lpSS, LPSS_COORD CellCol, LPSS_COORD CellRow)
{
if (CellRow)
   *CellRow = lpSS->Row.CurAt;
if (CellCol)
   *CellCol = lpSS->Col.CurAt;
return (TRUE);
}


BOOL DLLENTRY SSSetActiveCell(HWND hWnd, SS_COORD CellCol, SS_COORD CellRow)
{
LPSPREADSHEET lpSS;
BOOL       bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &CellCol, &CellRow);
bRet = SS_SetActiveCell(lpSS, CellCol, CellRow);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_SetActiveCell(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow)
{
SS_COORD ColOld;
SS_COORD RowOld;
BOOL     fRet;

// RFW - 8/8/02 - 10742
//if (CellCol < lpSS->Col.HeaderCnt || CellCol >= SS_GetColCnt(lpSS) ||
//    CellRow < lpSS->Row.HeaderCnt || CellRow >= SS_GetRowCnt(lpSS))
if (CellCol >= SS_GetColCnt(lpSS) ||
    CellRow >= SS_GetRowCnt(lpSS))
   return (FALSE);

lpSS->fSetActiveCellCalled = TRUE;

ColOld = lpSS->Col.CurAt;
RowOld = lpSS->Row.CurAt;

if (!lpSS->fSuspendInvert)
   SS_ResetBlock(lpSS);

fRet = SS_ActivateCell(lpSS, CellCol, CellRow, TRUE);

lpSS->Row.BlockCellCurrentPos = lpSS->Row.CurAt;
lpSS->ExtSelAnchorRow = lpSS->Row.CurAt;

#ifndef SS_NOBUTTONDRAWMODE
if (lpSS->lpBook->wButtonDrawMode)
   SS_InvalidateButtons(lpSS, ColOld, lpSS->Col.UL, RowOld, lpSS->Row.UL);
#endif

return (fRet);
}


void DLLENTRY SSSetMaxCols(HWND hWnd, SS_COORD MaxCols)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetMaxCols(lpSS, MaxCols);
SS_SheetUnlock(hWnd);
}


void SS_SetMaxCols(LPSPREADSHEET lpSS, SS_COORD MaxCols)
{
SS_COORD ColLeft;
SS_COORD ColULOld;
SS_COORD MaxColsOld;
SS_COORD i;
BOOL     fTurnHighlightOn = FALSE;

if (MaxCols < 0)
// RFW - 10/20/98 - BOM47   MaxCols = SS_MAXCOORD;
   MaxCols = 500;
else
   MaxCols = min(MaxCols, SS_MAXCOORD);

MaxCols += lpSS->Col.HeaderCnt - 1;

if (lpSS->Col.CurAt == 0 && MaxCols > 0)
   lpSS->Col.CurAt = lpSS->Col.HeaderCnt;
else if (lpSS->Col.CurAt < lpSS->Col.HeaderCnt)
   lpSS->Col.CurAt = min(lpSS->Col.HeaderCnt, MaxCols);

if (lpSS->Col.UL == 0 && MaxCols > 0)
   lpSS->Col.UL = lpSS->Col.HeaderCnt;
else if (lpSS->Col.UL < lpSS->Col.HeaderCnt)
   lpSS->Col.UL = min(lpSS->Col.HeaderCnt, MaxCols);

if (lpSS->Col.AllocCnt < lpSS->Col.HeaderCnt)
   lpSS->Col.AllocCnt = min(lpSS->Col.HeaderCnt, MaxCols + 1);

if (lpSS->Col.DataCnt < lpSS->Col.HeaderCnt)
   lpSS->Col.DataCnt = min(lpSS->Col.HeaderCnt, MaxCols + 1);

if (MaxCols < lpSS->Col.AllocCnt - 1)
   {
   SS_ClearRange(lpSS, MaxCols + 1, SS_ALLROWS, lpSS->Col.AllocCnt - 1,
                 SS_ALLROWS, FALSE);

   lpSS->Col.AllocCnt = min(lpSS->Col.AllocCnt, MaxCols + 1);
   lpSS->Col.DataCnt = min(lpSS->Col.DataCnt, MaxCols + 1);
   }

MaxColsOld = SS_GetColCnt(lpSS);
ColULOld = lpSS->Col.UL;

if (lpSS->Col.CurAt > MaxCols)
   {
   WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

   lpSS->lpBook->wMessageBeingSent = FALSE;
   SS_CellEditModeOff(lpSS, 0);
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;

   SS_HighlightCell(lpSS, FALSE);
   fTurnHighlightOn = TRUE;
   }

#ifdef SS_V40
if (MaxCols < MaxColsOld)
   SS_DelCellSpanColRange(lpSS, MaxCols + 1, MaxColsOld - MaxCols, FALSE);
#endif // SS_V40

lpSS->Col.Max = MaxCols - (lpSS->Col.HeaderCnt - 1);
lpSS->Col.CurAt = min(lpSS->Col.CurAt, SS_GetColCnt(lpSS) - 1);
lpSS->Col.UL = min(lpSS->Col.UL, SS_GetColCnt(lpSS) - 1);

if (lpSS->Col.UL > (ColLeft = SS_GetLastPageLeftCol(lpSS)))
   lpSS->Col.UL = ColLeft;

if (lpSS->Col.UL < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
   lpSS->Col.UL = min(MaxCols, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);

// RFW - 3/21/02 - 9873
lpSS->Col.LR = SS_GetRightCell(lpSS, lpSS->Col.UL);

if (ColULOld != lpSS->Col.UL)
   SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);

if (SS_IsBlockSelected(lpSS))
   {
   if (lpSS->BlockCellUL.Col >= SS_GetColCnt(lpSS))
      {
      lpSS->BlockCellUL.Row = -1;
      lpSS->BlockCellUL.Col = -1;

      lpSS->BlockCellLR.Row = -1;
      lpSS->BlockCellLR.Col = -1;
      }

   else if (lpSS->BlockCellLR.Col >= SS_GetColCnt(lpSS))
      lpSS->BlockCellLR.Col = SS_GetColCnt(lpSS) - 1;
  }

SS_BuildDependencies(lpSS);

if (lpSS->lpBook->fAllowCellOverflow)
   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.DataCnt; i++)
      SS_OverflowAdjustNeighbor(lpSS, min(MaxCols, MaxColsOld), i);

SS_AutoSize(lpSS->lpBook, FALSE);
SS_InvalidateColRange(lpSS, min(SS_GetColCnt(lpSS), MaxColsOld),
                      max(SS_GetColCnt(lpSS) - 1, MaxColsOld));

if (fTurnHighlightOn)
   SS_HighlightCell(lpSS, TRUE);

// RFW - 8/3/94
#ifdef SS_VB
VBSS_SetMax(lpSS->lpBook->hWnd);
#endif
}


SS_COORD DLLENTRY SSGetMaxCols(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD   ColsMax;

lpSS = SS_SheetLock(hWnd);
ColsMax = SS_GetMaxCols(lpSS);
SS_SheetUnlock(hWnd);
return (ColsMax);
}


SS_COORD SS_GetMaxCols(LPSPREADSHEET lpSS)
{
return lpSS->Col.Max;
}


void DLLENTRY SSSetMaxRows(HWND hWnd, SS_COORD MaxRows)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);

if (!lpSS->fVirtualMode)
   SS_SetMaxRows(lpSS, MaxRows);

SS_SheetUnlock(hWnd);
}


void SS_SetMaxRows(LPSPREADSHEET lpSS, SS_COORD MaxRows)
{
SS_COORD RowTop;
SS_COORD RowULOld;
SS_COORD MaxRowsOld;
BOOL     fTurnHighlightOn = FALSE;

if (MaxRows < 0)
// RFW - 10/20/98 - BOM47   MaxRows = SS_MAXCOORD;
   MaxRows = 500;
else
   MaxRows = min(MaxRows, SS_MAXCOORD);

MaxRows += lpSS->Row.HeaderCnt - 1;

if (lpSS->Row.CurAt == 0 && MaxRows > 0)
   lpSS->Row.CurAt = lpSS->Row.HeaderCnt;
else if (lpSS->Row.CurAt < lpSS->Row.HeaderCnt)
   lpSS->Row.CurAt = min(lpSS->Row.HeaderCnt, MaxRows);

if (lpSS->Row.UL == 0 && MaxRows > 0)
   lpSS->Row.UL = lpSS->Row.HeaderCnt;
else if (lpSS->Row.UL < lpSS->Row.HeaderCnt)
   lpSS->Row.UL = min(lpSS->Row.HeaderCnt, MaxRows);

if (lpSS->Row.AllocCnt < lpSS->Row.HeaderCnt)
   lpSS->Row.AllocCnt = min(lpSS->Row.HeaderCnt, MaxRows + 1);

if (lpSS->Row.DataCnt < lpSS->Row.HeaderCnt)
   lpSS->Row.DataCnt = min(lpSS->Row.HeaderCnt, MaxRows + 1);

if (MaxRows < lpSS->Row.AllocCnt - 1)
   {
   SS_ClearRange(lpSS, SS_ALLCOLS, MaxRows + 1, SS_ALLCOLS,
                lpSS->Row.AllocCnt - 1, FALSE);

   lpSS->Row.AllocCnt = min(lpSS->Row.AllocCnt, MaxRows + 1);
   lpSS->Row.DataCnt = min(lpSS->Row.DataCnt, MaxRows + 1);
   }

MaxRowsOld = SS_GetRowCnt(lpSS);
RowULOld = lpSS->Row.UL;

if (lpSS->Row.CurAt > MaxRows)
   {
   WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

   lpSS->lpBook->wMessageBeingSent = FALSE;
   SS_CellEditModeOff(lpSS, 0);
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;

   SS_HighlightCell(lpSS, FALSE);
   fTurnHighlightOn = TRUE;
   }

#ifdef SS_V40
if (MaxRows < MaxRowsOld)
   SS_DelCellSpanRowRange(lpSS, MaxRows + 1, MaxRowsOld - MaxRows);
#endif // SS_V40

lpSS->Row.Max = MaxRows - (lpSS->Row.HeaderCnt - 1);
lpSS->Row.CurAt = min(lpSS->Row.CurAt, SS_GetRowCnt(lpSS) - 1);
lpSS->Row.UL = min(lpSS->Row.UL, SS_GetRowCnt(lpSS) - 1);

if (lpSS->Row.UL > (RowTop = SS_GetLastPageTopRow(lpSS)))
   lpSS->Row.UL = RowTop;

if (lpSS->Row.UL < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
   lpSS->Row.UL = min(MaxRows, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);

// RFW - 3/21/02 - 9873
lpSS->Row.LR = SS_GetBottomCell(lpSS, lpSS->Row.UL);
// RFW - 12/27/04 - 15360
lpSS->Row.LRAllVis = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);

if (RowULOld != lpSS->Row.UL)
   SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);

if (SS_IsBlockSelected(lpSS))
   {
   if (lpSS->BlockCellUL.Row >= SS_GetRowCnt(lpSS))
      {
      lpSS->BlockCellUL.Row = -1;
      lpSS->BlockCellUL.Col = -1;

      lpSS->BlockCellLR.Row = -1;
      lpSS->BlockCellLR.Col = -1;
      }

   else if (lpSS->BlockCellLR.Row >= SS_GetRowCnt(lpSS))
      lpSS->BlockCellLR.Row = SS_GetRowCnt(lpSS) - 1;
   }

SS_BuildDependencies(lpSS);

SS_AutoSize(lpSS->lpBook, FALSE);

#ifdef SS_VB
VBSS_SetMax(lpSS->lpBook->hWnd);
#endif

#ifdef SS_UTP
SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);

if (lpSS->Redraw)
   SS_UpdateWindow(lpSS->lpBook);

#else
SS_InvalidateRowRange(lpSS, min(SS_GetRowCnt(lpSS), MaxRowsOld),
                      max(SS_GetRowCnt(lpSS) - 1, MaxRowsOld));
#endif

if (fTurnHighlightOn)
   SS_HighlightCell(lpSS, TRUE);

// RFW - 8/3/94
#ifdef SS_VB
VBSS_SetMax(lpSS->lpBook->hWnd);
#endif
}


SS_COORD DLLENTRY SSGetMaxRows(HWND hWnd)
{
LPSPREADSHEET lpSS;
SS_COORD   RowsMax;

lpSS = SS_SheetLock(hWnd);
RowsMax = SS_GetMaxRows(lpSS);
SS_SheetUnlock(hWnd);
return (RowsMax);
}


SS_COORD SS_GetMaxRows(LPSPREADSHEET lpSS)
{
return lpSS->Row.Max;
}


BOOL DLLENTRY SSGetSelectBlock(HWND hWnd, LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
{
LPSPREADSHEET lpSS;
BOOL       bRet;

lpSS = SS_SheetLock(hWnd);
bRet = SS_GetSelectBlock(lpSS, CellUL, CellLR);
if (CellUL)
	SS_AdjustCellCoordsOut(lpSS, &CellUL->Col, &CellUL->Row);
if (CellLR)
	SS_AdjustCellCoordsOut(lpSS, &CellLR->Col, &CellLR->Row);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_GetSelectBlock(LPSPREADSHEET lpSS, LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
{
if (CellUL)
   {
   CellUL->Col = -1;
   CellUL->Row = -1;
   }

if (CellLR)
   {
   CellLR->Col = -1;
   CellLR->Row = -1;
   }

if (lpSS->BlockCellUL.Col == -1 &&
    lpSS->BlockCellUL.Row == -1)
	return (FALSE);

if (lpSS->BlockCellUL.Col == lpSS->BlockCellLR.Col &&
    lpSS->BlockCellUL.Row == lpSS->BlockCellLR.Row &&
    lpSS->BlockCellUL.Row != -1 && lpSS->BlockCellUL.Col != -1)
   return (FALSE);

if (CellUL)
   {
   CellUL->Row = lpSS->BlockCellUL.Row;
   CellUL->Col = lpSS->BlockCellUL.Col;

   // BJO 25Apr96 SEL3651 - Begin of fix
   if (lpSS->Row.HeaderCnt > CellUL->Row )
      CellUL->Row = -1;
   if (lpSS->Col.HeaderCnt > CellUL->Col )
      CellUL->Col = -1;
   // BJO 25Apr96 SEL3651 - End of fix
   }

if (CellLR)
   {
   CellLR->Row = lpSS->BlockCellLR.Row;
   CellLR->Col = lpSS->BlockCellLR.Col;
   }

return (TRUE);
}


BOOL DLLENTRY SSSetSelectBlock(HWND hWnd, LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
{
LPSPREADSHEET lpSS;
BOOL       bRet;

lpSS = SS_SheetLock(hWnd);

if (CellUL->Row == SS_HEADER)
   CellUL->Row = -1;
if (CellUL->Col == SS_HEADER)
   CellUL->Col = -1;
if (CellLR->Row == SS_HEADER)
   CellLR->Row = -1;
if (CellLR->Col == SS_HEADER)
   CellLR->Col = -1;

SS_AdjustCellCoords(lpSS, &CellUL->Col, &CellUL->Row);
SS_AdjustCellCoords(lpSS, &CellLR->Col, &CellLR->Row);

bRet = SS_SetSelectBlock(lpSS, CellUL, CellLR);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_SetSelectBlock(LPSPREADSHEET lpSS, LPSS_CELLCOORD CellUL, LPSS_CELLCOORD CellLR)
{
SS_ResetBlock(lpSS);

if (CellUL->Row < lpSS->Row.HeaderCnt)
	{
   lpSS->BlockCellUL.Row = 0;
	CellLR->Row = -1;
	}
else
   lpSS->BlockCellUL.Row = CellUL->Row;

if (CellUL->Col < lpSS->Col.HeaderCnt)
	{
	lpSS->BlockCellUL.Col = 0;
	CellLR->Col = -1;
	}
else
   lpSS->BlockCellUL.Col = CellUL->Col;

if (CellLR->Row == -1)
   lpSS->BlockCellLR.Row = -1;
else
   lpSS->BlockCellLR.Row = CellLR->Row;

if (CellLR->Col == -1)
   lpSS->BlockCellLR.Col = -1;
else
   lpSS->BlockCellLR.Col = CellLR->Col;

lpSS->Col.BlockCellCurrentPos = lpSS->BlockCellLR.Col;
lpSS->Row.BlockCellCurrentPos = lpSS->BlockCellLR.Row;

lpSS->fSuspendInvert = TRUE;
SS_SetActiveCell(lpSS, max(lpSS->Col.HeaderCnt, CellUL->Col),
                 max(lpSS->Row.HeaderCnt, CellUL->Row));
lpSS->fSuspendInvert = FALSE;

if (lpSS->lpBook->wMessageBeingSent && (lpSS->lpBook->wMsgLast == WM_KEYDOWN ||
    lpSS->lpBook->wMsgLast == WM_LBUTTONDOWN))
   {
   lpSS->fSuspendInvert = TRUE;
   SS_HighlightCell(lpSS, FALSE);
   lpSS->fInvertBlock = TRUE;
   }

else if ((lpSS->lpBook->hWnd && GetFocus() == lpSS->lpBook->hWnd) || lpSS->lpBook->fRetainSelBlock)
   SS_InvertBlock(0, lpSS);

else
   lpSS->lpBook->fSelBlockInvisible = TRUE;

#ifdef SS_V80
SS_InvalidateActiveHeaders(lpSS);
#endif // SS_V80

return (TRUE);
}


void SSGetDocColor(HWND hWnd,LPCOLORREF Background, LPCOLORREF Foreground)
{
LPSPREADSHEET      lpSS;
SS_COLORTBLITEM ColorTblItem;

lpSS = SS_SheetLock(hWnd);

if (Background)
   {
   SS_GetColorItem(&ColorTblItem,
                   lpSS->Color.BackgroundId);
   *Background = ColorTblItem.Color;
   }

if (Foreground)
   {
   SS_GetColorItem(&ColorTblItem,
                   lpSS->Color.ForegroundId);
   *Foreground = ColorTblItem.Color;
   }

SS_SheetUnlock(hWnd);
}


void SS_SetDocColor(LPSPREADSHEET lpSS, COLORREF Background, COLORREF Foreground)
{
LPSS_CELL lpCell;
LPSS_COL  lpCol;
LPSS_ROW  lpRow;
SS_COORD  i;
SS_COORD  j;

/**********************************
* Clear All Cols, Rows, and Cells
**********************************/

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
//      _fmemset(&lpRow->Color, '\0', sizeof(SS_COLORITEM));
      SS_ClearColorItem(&lpRow->Color, Background, Foreground);

      for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
         if (lpCell = SS_LockCellItem(lpSS, j, i))
            SS_ClearColorItem(&lpCell->Color, Background, Foreground);
      }
   }

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   if (lpCol = SS_LockColItem(lpSS, i))
      SS_ClearColorItem(&lpCol->Color, Background, Foreground);

//if (!SS_ISDEFCOLOR(Background))
if (Background != SPREAD_COLOR_IGNORE)
	/* RFW - 2/2/09 - 24662
   lpSS->Color.BackgroundId = SS_AddColor(Background);
	*/
   lpSS->Color.BackgroundId = SS_AddColor(Background == SPREAD_COLOR_NONE ? RGBCOLOR_WHITE : Background);

//if (!SS_ISDEFCOLOR(Foreground))
if (Foreground != SPREAD_COLOR_IGNORE)
   lpSS->Color.ForegroundId = SS_AddColor(Foreground == SPREAD_COLOR_NONE ? RGBCOLOR_BLACK : Foreground);

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
}

#ifdef SS_V35
void SSGetDocCellNote(HWND hWnd, LPTSTR Note, short *pnLen)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);

if (lpSS->hCellNote)
   {
      LPTSTR lpszNote = (LPTSTR)tbGlobalLock(lpSS->hCellNote);
      *pnLen = lstrlen(lpszNote);
      if (Note)
        _tcscpy(Note, lpszNote);
      tbGlobalUnlock(lpSS->hCellNote); 
   }

SS_SheetUnlock(hWnd);
}

void SS_SetDocCellNote(LPSPREADSHEET lpSS, LPTSTR Note)
{
LPSS_CELL lpCell;
LPSS_COL  lpCol;
LPSS_ROW  lpRow;
SS_COORD  i;
SS_COORD  j;

/**********************************
* Clear All Cols, Rows, and Cells
**********************************/

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   {
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      if (lpRow->hCellNote)
        {
        tbGlobalFree(lpRow->hCellNote);
        lpRow->hCellNote = 0;
        }

      for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
         if (lpCell = SS_LockCellItem(lpSS, j, i))
           if (lpCell->hCellNote)
           {
           tbGlobalFree(lpCell->hCellNote);
           lpCell->hCellNote = 0;
           }
      }
   }

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   if (lpCol = SS_LockColItem(lpSS, i))
      if (lpCol->hCellNote)
        {
        tbGlobalFree(lpCol->hCellNote);
        lpCol->hCellNote = 0;
        }

if (lpSS->hCellNote)
  {
  tbGlobalFree(lpSS->hCellNote);
  lpSS->hCellNote = 0;
  }

if (Note && Note[0])
  {
    LPTSTR lpszNote;
    lpSS->hCellNote = tbGlobalAlloc(GHND, (lstrlen(Note)+1)*sizeof(TCHAR));
    lpszNote = (LPTSTR)tbGlobalLock(lpSS->hCellNote);
    _tcscpy(lpszNote, Note);
    tbGlobalUnlock(lpSS->hCellNote);
  }

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
}
#endif

void SS_ClearColorItem(LPSS_COLORITEM lpColor, COLORREF Background,
                       COLORREF Foreground)
{
// RFW - 3/30/00 - WEB921768345656
//if (!SS_ISDEFCOLOR(Background))
if (Background != SPREAD_COLOR_IGNORE)
   lpColor->BackgroundId = 0;

//if (!SS_ISDEFCOLOR(Foreground))
if (Foreground != SPREAD_COLOR_IGNORE)
   lpColor->ForegroundId = 0;
}


void DLLENTRY SSGetShadowColor(HWND hWnd, LPCOLORREF ShadowColor, LPCOLORREF ShadowText,
                               LPCOLORREF ShadowDark, LPCOLORREF ShadowLight)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_GetShadowColor(lpSS, ShadowColor, ShadowText, ShadowDark, ShadowLight);
SS_SheetUnlock(hWnd);
}

void SS_GetShadowColor(LPSPREADSHEET lpSS, LPCOLORREF ShadowColor, LPCOLORREF ShadowText,
                       LPCOLORREF ShadowDark, LPCOLORREF ShadowLight)
{
SS_COLORTBLITEM ColorTblItem;

if (ShadowColor)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->Color.ShadowColorId);
   *ShadowColor = ColorTblItem.Color;
   }

if (ShadowText)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->Color.ShadowTextId);
   *ShadowText = ColorTblItem.Color;
   }

if (ShadowDark)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->Color.ShadowDarkId);
   *ShadowDark = ColorTblItem.Color;
   }

if (ShadowLight)
   {
   SS_GetColorItem(&ColorTblItem, lpSS->Color.ShadowLightId);
   *ShadowLight = ColorTblItem.Color;
   }
}


void DLLENTRY SSSetShadowColor(HWND hWnd, COLORREF ShadowColor, COLORREF ShadowText,
                               COLORREF ShadowDark, COLORREF ShadowLight)
{
LPSPREADSHEET lpSS;

#ifdef SPREAD_JPN
// JPNFIX0001 - (Masanori Iwasa)
if(ShadowColor & 0x80000000L)
   ShadowColor = GetSysColor((short)(ShadowColor & 0x00FFFFFFL));

if(ShadowText & 0x80000000L)
   ShadowText = GetSysColor((short)(ShadowText & 0x00FFFFFFL));

if(ShadowDark & 0x80000000L)
   ShadowDark = GetSysColor((short)(ShadowDark & 0x00FFFFFFL));

if(ShadowLight & 0x80000000L)
   ShadowLight = GetSysColor((short)(ShadowLight & 0x00FFFFFFL));
#endif

lpSS = SS_SheetLock(hWnd);
SS_SetShadowColor(lpSS, ShadowColor, ShadowText, ShadowDark, ShadowLight);
SS_SheetUnlock(hWnd);
}

void SS_SetShadowColor(LPSPREADSHEET lpSS, COLORREF ShadowColor, COLORREF ShadowText,
                       COLORREF ShadowDark, COLORREF ShadowLight)
{
if (!SS_ISDEFCOLOR(ShadowColor))
   lpSS->Color.ShadowColorId = SS_AddColor(ShadowColor);
if (!SS_ISDEFCOLOR(ShadowText))
   lpSS->Color.ShadowTextId  = SS_AddColor(ShadowText);
if (!SS_ISDEFCOLOR(ShadowDark))
   lpSS->Color.ShadowDarkId  = SS_AddColor(ShadowDark);
if (!SS_ISDEFCOLOR(ShadowLight))
   lpSS->Color.ShadowLightId = SS_AddColor(ShadowLight);

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
}


void DLLENTRY SSGetGrayAreaColor(HWND hWnd, LPCOLORREF lpBackground, LPCOLORREF lpForeground)
{
LPSS_BOOK lpBook;

lpBook = SS_BookLock(hWnd);
SS_GetGrayAreaColor(lpBook, lpBackground, lpForeground);
SS_BookUnlock(hWnd);
}


void SS_GetGrayAreaColor(LPSS_BOOK lpBook, LPCOLORREF lpBackground, LPCOLORREF lpForeground)
{
if (lpBackground)
   *lpBackground = lpBook->GrayAreaBackground;

if (lpForeground)
   *lpForeground = lpBook->GrayAreaForeground;
}


void DLLENTRY SSSetGrayAreaColor(HWND hWnd, COLORREF Background, COLORREF Foreground)
{
LPSS_BOOK lpBook;

lpBook = SS_BookLock(hWnd);
SS_SetGrayAreaColor(lpBook, Background, Foreground);
SS_BookUnlock(hWnd);
}


void SS_SetGrayAreaColor(LPSS_BOOK lpBook, COLORREF Background, COLORREF Foreground)
{
LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);

lpBook->GrayAreaBackground = Background;
lpBook->GrayAreaForeground = Foreground;

SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
SS_BookUnlockActiveSheet(lpBook);
}


BOOL DLLENTRY SS_ClipboardCopy(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, BOOL fCut)
{
GLOBALHANDLE hClip = 0; // RFW - 2/7/06 - 18129
LPTSTR       lpstrTemp;
SS_CELLCOORD CellUL;
SS_CELLCOORD CellLR;
SS_COORD     Col;
SS_COORD     Row;
BOOL         fRet = FALSE;

// BJO 12Mar98 GIC3265 - Before fix
if (SS_USESINGLESELBAR(lpSS) ||
    (lpSS->wOpMode == SS_OPMODE_MULTISEL && lpSS->MultiSelCnt))
   {
   if (SS_GetActiveCell(lpSS, &Col, &Row))
      hClip = SS_ClipOut(lpSS, 1, Row, lpSS->Col.AllocCnt - 1, Row, FALSE, TRUE);
   }

else if (SS_USESELBAR(lpSS) && lpSS->MultiSelCnt == 0)
   return (0);

else if (SS_GetSelectBlock(lpSS, &CellUL, &CellLR))
   hClip = SS_ClipOut(lpSS,
                     CellUL.Col < lpSS->Col.HeaderCnt && !SS_IsCopyRowHeader(lpBook) ? lpSS->Col.HeaderCnt : CellUL.Col,
                     CellUL.Row < lpSS->Row.HeaderCnt && !SS_IsCopyColHeader(lpBook) ? lpSS->Row.HeaderCnt : CellUL.Row, 
                     CellLR.Col == 0 ? -1 : CellLR.Col, CellLR.Row == 0 ? -1 : CellLR.Row, FALSE, TRUE);

else if (lpSS->wOpMode == SS_OPMODE_READONLY)
   return (0);

else if (SS_GetActiveCell(lpSS, &Col, &Row))
   {
	SS_CELLTYPE CellType;

   SS_RetrieveCellType(lpSS, &CellType, NULL, Col, Row);

	if (SS_TYPE_EDIT == CellType.Type && (ES_PASSWORD & CellType.Style))
		; // Ignore clipboard copy for password cells
	else
		{
		CellUL.Col = Col;
		CellUL.Row = Row;
		CellLR.Col = Col;
		CellLR.Row = Row;

		if (hClip = GlobalAlloc(GMEM_MOVEABLE, (SS_GetDataLen(lpSS, Col, Row)
										+ 1) * sizeof(TCHAR)))
			{
			lpstrTemp = GlobalLock(hClip);
			SS_GetDataEx(lpSS, Col, Row, lpstrTemp, -1);
				// 25724 -scl //>>99918148 Leon 20101025
         if( SS_TYPE_EDIT == CellType.Type && (ES_MULTILINE & CellType.Style) && (_tcstok(lpstrTemp, _T("\t")) || _tcstok(lpstrTemp, _T("\r"))))
         {
					GLOBALHANDLE temp = SS_DoubleQuotes_Encapsulation(lpstrTemp);					
               GlobalUnlock(hClip);
               GlobalFree(hClip);
               hClip = temp;
               GlobalLock(hClip);
            }
				//<<99918148 Leon 20101025
			GlobalUnlock(hClip);
			}
		}
	}

#ifdef SS_UTP
if (fCut && !hClip)
   hClip = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(TCHAR));
#endif

if (hClip && OpenClipboard(lpBook->hWnd))
   {
   EmptyClipboard();
   #if defined(UNICODE) || defined(_UNICODE)
   SetClipboardData(CF_UNICODETEXT, hClip);
   #else
   SetClipboardData(CF_TEXT, hClip);
   #endif
   CloseClipboard();
   fRet = TRUE;

#ifdef SS_UTP
   if (fCut)
      SS_VBDelBlock(hWnd, lpSS, SS_INSDEL_OP_CLIPBOARD);
#else
// RFW - 4/5/01
//   if (fCut && !SS_USESINGLESELBAR(lpSS))
	/* RFW - 8/25/05 - 16759
   if (fCut && !SS_USESELBAR(lpSS))
	*/
   if (fCut && !SS_USESELBAR(lpSS) && lpSS->wOpMode != SS_OPMODE_READONLY)
      SS_ClearDataRange(lpSS, CellUL.Col, CellUL.Row, CellLR.Col, CellLR.Row,
                        TRUE, TRUE);
#endif
   }

return (fRet);
}


HANDLE SS_GetTextFromClipboard(HWND hWnd)
{
HANDLE hMem;
HANDLE hText = 0;
LPTSTR lpszMem;
LPTSTR lpszText;

OpenClipboard(hWnd);

#if defined(UNICODE) || defined(_UNICODE)
if (hMem = GetClipboardData(CF_UNICODETEXT))
#else
if (hMem = GetClipboardData(CF_TEXT))
#endif
   {
   lpszMem = (LPTSTR)GlobalLock(hMem);

   if (lstrlen(lpszMem))
      if (hText = GlobalAlloc(GHND, (lstrlen(lpszMem) + 1) * sizeof(TCHAR)))
         {
         lpszText = (LPTSTR)GlobalLock(hText);
         StrCpy(lpszText, lpszMem);
         GlobalUnlock(hText);
         }

   GlobalUnlock(hMem);
   }

CloseClipboard();

return (hText);
}


BOOL DLLENTRY SSClipboardCopy(HWND hWnd)
{
LPSS_BOOK  lpBook;
LPSPREADSHEET lpSS;
BOOL       fRet;

lpBook = SS_BookLock(hWnd);
lpSS = SS_BookLockActiveSheet(lpBook);
fRet = SS_ClipboardCopy(lpBook, lpSS, FALSE);
SS_BookUnlockActiveSheet(lpBook);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL DLLENTRY SSClipboardCut(HWND hWnd)
{
LPSS_BOOK  lpBook;
LPSPREADSHEET lpSS;
BOOL       fRet;

lpBook = SS_BookLock(hWnd);
lpSS = SS_BookLockActiveSheet(lpBook);
fRet = SS_ClipboardCopy(lpBook, lpSS, TRUE);
SS_BookUnlockActiveSheet(lpBook);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL DLLENTRY SSClipboardPaste(HWND hWnd)
{
LPSS_BOOK    lpBook;
LPSPREADSHEET   lpSS;
GLOBALHANDLE hClip;
LPTSTR       lpStrOut;
SS_CELLCOORD CellUL;
SS_CELLCOORD CellLR;
SS_CELLCOORD CellLRVal;
BOOL         fCancel = FALSE;
BOOL         fRet = FALSE;

lpBook = SS_BookLock(hWnd);
lpSS = SS_BookLockActiveSheet(lpBook);

// RFW - 4/5/01
//if (!SS_USESINGLESELBAR(lpSS))
/* RFW - 8/1/04 - 14864
if (!SS_USESELBAR(lpSS))
*/
if (!SS_USESELBAR(lpSS) || lpSS->wOpMode == SS_OPMODE_ROWMODE)
   {
	/* RFW - 3/12/04 - 13869
   if ((lpSS->wOpMode != SS_OPMODE_READONLY ||
       SS_GetSelectBlock(lpSS, &CellUL, &CellLR)) &&
       OpenClipboard(hWnd))
	*/
   if (lpSS->wOpMode != SS_OPMODE_READONLY &&
       OpenClipboard(hWnd))
      {
      if (IsClipboardFormatAvailable(CF_TEXT) ||
          IsClipboardFormatAvailable(CF_OEMTEXT))
         {
         #if defined(UNICODE) || defined(_UNICODE)
         hClip = GetClipboardData(CF_UNICODETEXT);
         #else
         hClip = GetClipboardData(CF_TEXT);
         #endif

         if (hClip)
            {
            if (lpStrOut = GlobalLock(hClip))
               {
               if (!SS_GetSelectBlock(lpSS, &CellUL, &CellLR))
                  {
                  CellLR.Col = SS_GetColCnt(lpSS) - 1;
                  CellLR.Row = SS_GetRowCnt(lpSS) - 1;

                  SS_GetActiveCell(lpSS, &CellUL.Col, &CellUL.Row);
#ifdef SS_UTP
                  if (CellUL.Row == SS_HEADER)
                      {
                      CellUL.Row = 1;
                      CellUL.Col = 0;
                      CellLR.Col = -1;
                      }

                  if (CellUL.Col == SS_HEADER)
                      {
                      CellUL.Col = 1;
                      CellUL.Row = 0;
                      CellLR.Row = -1;
                      }
#endif

                  _fmemcpy(&CellLRVal, &CellLR, sizeof(SS_CELLCOORD));
                  }
               else
                  _fmemcpy(&CellLRVal, &CellLR, sizeof(SS_CELLCOORD));

               if (SS_ValidatePasteData(hWnd, lpSS, lpStrOut, &CellUL,
                                        &CellLRVal, FALSE))
                  {
#ifdef SS_UTP
                  BOOL fRedraw = lpSS->Redraw;
                  lpSS->Redraw = FALSE;

                  if (lpSS->fValidateClipboardPaste)
                     {
                     SS_COORD InsCnt;

                     if (CellUL.Col == -1 || CellLRVal.Col == -1)
                        {
                        InsCnt = CellLRVal.Row - CellUL.Row + 1;
                        if (InsCnt > 0 &&
                            !(fCancel = (BOOL)SS_SendMsgInsDel(hWnd,
                                        SS_INSDEL_TYPE_INS,
                                        SS_INSDEL_OP_CLIPBOARD,
                                        SS_INSDEL_ROW, CellUL.Row,
                                        CellLRVal.Row)))
                           {
                           SSSetMaxRows(hWnd, SS_GetRowCnt(lpSS) - 1 +
                                        InsCnt);
                           SS_InsRowRange(lpSS, CellUL.Row, CellLRVal.Row);
                           }
                        }

                     else if (CellUL.Row == -1 || CellLRVal.Row == -1)
                        {
                        InsCnt = CellLRVal.Col - CellUL.Col + 1;
                        if (InsCnt > 0 &&
                            !(fCancel = (BOOL)SS_SendMsgInsDel(hWnd,
                                        SS_INSDEL_TYPE_INS,
                                        SS_INSDEL_OP_CLIPBOARD,
                                        SS_INSDEL_COL, CellUL.Col,
                                        CellLRVal.Col)))
                           {
                           SSSetMaxCols(hWnd, SS_GetColCnt(lpSS) - 1 +
                                        InsCnt);
                           SS_InsColRange(lpSS, CellUL.Col, CellLRVal.Col);
                           }
                        }
                     }

                  if (!fCancel)
                     {
                     lpSS->fInvalidDataClippedIn = FALSE;
                     fRet = SS_ClipIn(lpSS, CellUL.Col == 0 ? 1 : CellUL.Col,
                                      CellUL.Row == 0 ? 1 : CellUL.Row,
                                      CellLRVal.Col, CellLRVal.Row, lpStrOut,
                                      hStrLen(lpStrOut), FALSE, TRUE, TRUE,
                                      NULL, NULL);

                     if (CellUL.Col == -1 || CellLRVal.Col == -1 &&
                         lpSS->fInvalidDataClippedIn)
                        SS_SendMsgCommand(hWnd, NULL, SSN_INVALIDCOLTYPE,
                                          FALSE);
                     }

						SS_BookSetRedraw(lpSS->lpBook, fRedraw)
#else
//Modify By BOC 99.7.30 (hyt)----------------------------
//for clipboard action already changed at 3.0, 2.5J fixed
//now is not correct so discard.

/*// For Yagi No.3 Wei Feng 1997/11/04
#ifdef  SPREAD_JPN
				if ( (CellUL.Col == CellLRVal.Col && CellUL.Col == -1 || 
					  CellUL.Row == CellLRVal.Row && CellUL.Row == -1) && 
					  ( (char)(*(lpStrOut)) != 0x0d || (char)(*(lpStrOut + 1)) != 0x0a )

// Sp25_005 [1-1]
// When A Whole Column In Clipboard, The Separator Is '0d0a'
// But When A Whole Row In Clipboard, The Separator Is '09'
// So Add More Judgement For Whole Row :)
// -- HaHa 1998.4.22
					  && ((char)(*(lpStrOut))!=0x09))
				;
				else
#endif*/
//--------------------------------------------------------------------------------
                  fRet = SS_ClipIn(lpSS,
                                   CellUL.Col <= 0 && !SS_IsPasteRowHeader(lpBook) ? lpSS->Col.HeaderCnt : CellUL.Col,
                                   CellUL.Row <= 0 && !SS_IsPasteColHeader(lpBook) ? lpSS->Row.HeaderCnt : CellUL.Row,
                                   CellLRVal.Col, CellLRVal.Row, lpStrOut,
                                   hStrLen(lpStrOut), FALSE, TRUE, TRUE, NULL,
                                   NULL);
#endif
                  }

               GlobalUnlock(hClip);
               }
            }
         }

      CloseClipboard();
      }
   }

SS_BookUnlockActiveSheet(lpBook);
SS_BookUnlock(hWnd);
return (fRet);
}


WORD DLLENTRY SSSetButtonDrawMode(HWND hWnd, WORD wMode)
{
LPSS_BOOK lpBook;
WORD      wModeOld;

lpBook = SS_BookLock(hWnd);
wModeOld = SS_SetButtonDrawMode(lpBook, wMode);
SS_BookUnlock(hWnd);

return (wModeOld);
}


WORD SS_SetButtonDrawMode(LPSS_BOOK lpBook, WORD wMode)
{
LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
WORD       wModeOld;

wModeOld = lpBook->wButtonDrawMode;
lpBook->wButtonDrawMode = wMode;
SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);

SS_BookUnlockActiveSheet(lpBook);
return (wModeOld);
}


WORD DLLENTRY SSGetButtonDrawMode(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wModeOld;

lpBook = SS_BookLock(hWnd);
wModeOld = SS_GetButtonDrawMode(lpBook);
SS_BookUnlock(hWnd);

return (wModeOld);
}


WORD SS_GetButtonDrawMode(LPSS_BOOK lpBook)
{
return lpBook->wButtonDrawMode;
}

#ifdef SS_V35

short SS_GetUserColAction(LPSPREADSHEET lpSS)
{
return lpSS->nUserColAction;
}

short DLLENTRY SSGetUserColAction(HWND hWnd)
{
LPSPREADSHEET lpSS;
short      nUserColAction;

lpSS = SS_SheetLock(hWnd);
nUserColAction = SS_GetUserColAction(lpSS);
SS_SheetUnlock(hWnd);

return (nUserColAction);
}


void SS_SetUserColAction(LPSPREADSHEET lpSS, short nUserColAction)
{
if (SS_USERCOLACTION_MINVAL <= nUserColAction && nUserColAction <= SS_USERCOLACTION_MAXVAL)
   {
   lpSS->nUserColAction = nUserColAction;

   if (SS_USERCOLACTION_SORT == nUserColAction ||
       SS_USERCOLACTION_SORTNOINDICATOR == nUserColAction)
      {
      SS_COORD lCol;
      short nIndicator = SS_COLUSERSORTINDICATOR_NONE;

#ifdef SS_V40
		for (lCol = lpSS->Col.HeaderCnt; lCol < lpSS->Col.AllocCnt; lCol++)
			{
	      nIndicator = SS_GetMergedColUserSortIndicator(lpSS, lCol);
			if (SS_COLUSERSORTINDICATOR_ASCENDING == nIndicator ||
			    SS_COLUSERSORTINDICATOR_DESCENDING == nIndicator)
				break;
			}
#else
      SS_GetActiveCell(lpSS, &lCol, NULL);
      nIndicator = SS_GetMergedColUserSortIndicator(lpSS, lCol);

#endif // SS_V40

      if (SS_COLUSERSORTINDICATOR_ASCENDING == nIndicator ||
          SS_COLUSERSORTINDICATOR_DESCENDING == nIndicator)
         {
         SS_BEFOREUSERSORT beforeSort;

         // if column is selected, clear selection
         //   column can't be selected and in sort mode at the same time
         if (SS_IsCellInSelection(lpSS, lCol, 0))
            SS_DeSelectBlock(lpSS);
 
         beforeSort.lCol = lCol;
         beforeSort.lState = nIndicator;
         beforeSort.lDefaultAction = BEFOREUSERSORT_DEFAULTACTION_AUTOSORT;
			SS_AdjustCellCoordsOut(lpSS, &beforeSort.lCol, NULL);

         SS_SendMsg(lpSS->lpBook, lpSS, SSM_BEFOREUSERSORT, GetWindowID(lpSS->lpBook->hWnd), 
                    (LPARAM)(LPSS_BEFOREUSERSORT)&beforeSort);
                   
         if (BEFOREUSERSORT_DEFAULTACTION_AUTOSORT == beforeSort.lDefaultAction)
            {
            SS_SORT sort = {SS_SORT_ROW, lCol, nIndicator == SS_COLUSERSORTINDICATOR_ASCENDING ? SS_SORT_ASCENDING : SS_SORT_DESCENDING, -1, SS_SORT_NONE, -1, SS_SORT_NONE };
				BOOL fRet;
				SS_AdjustCellCoordsOut(lpSS, &sort.Key1Reference, NULL);
            fRet = SSSort(lpSS->lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS, SS_ALLCOLS, SS_ALLROWS, &sort);
#ifdef SS_V40
				if (fRet)
					{
					SS_SetColUserSortIndicator(lpSS, -1, SS_COLUSERSORTINDICATOR_NONE);
					SS_SetColUserSortIndicator(lpSS, lCol, nIndicator);
					}
#endif // SS_V40
            }

			SS_AdjustCellCoordsOut(lpSS, &lCol, NULL);
         SS_SendMsg(lpSS->lpBook, lpSS, SSM_AFTERUSERSORT, GetWindowID(lpSS->lpBook->hWnd), (LPARAM)lCol);
         }
// 9283...
      if (lpSS->lpBook->EditModeOn)
         {
         SS_CellEditModeOff(lpSS, 0);
         SS_CellEditModeOn(lpSS, 0, 0, 0);
         }
// ... 9283
      }
   
   SS_InvalidateRect(lpSS->lpBook, NULL, TRUE);
   }
}


void DLLENTRY SSSetUserColAction(HWND hWnd, short nUserColAction)
{
LPSPREADSHEET lpSS;

lpSS = SS_SheetLock(hWnd);
SS_SetUserColAction(lpSS, nUserColAction);
SS_SheetUnlock(hWnd);
}


BOOL SS_GetScriptEnhanced(LPSS_BOOK lpBook)
{
return lpBook->bScriptEnhanced;
}


BOOL DLLENTRY SSGetScriptEnhanced(HWND hWnd)
{
LPSS_BOOK lpBook;
BOOL      bScriptEnhanced;

lpBook = SS_BookLock(hWnd);
bScriptEnhanced = SS_GetScriptEnhanced(lpBook);
SS_BookUnlock(hWnd);

return (bScriptEnhanced);
}


void SS_SetScriptEnhanced(LPSS_BOOK lpBook, BOOL bScriptEnhanced)
{
lpBook->bScriptEnhanced = bScriptEnhanced;
}


void DLLENTRY SSSetScriptEnhanced(HWND hWnd, BOOL bScriptEnhanced)
{
LPSS_BOOK lpBook;

lpBook = SS_BookLock(hWnd);
SS_SetScriptEnhanced(lpBook, bScriptEnhanced);
SS_BookUnlock(hWnd);
}


#endif // SS_V35

BOOL SS_ProcessMnemonic(LPSPREADSHEET lpSS, TCHAR cChar)
{
TBGLOBALHANDLE hData;
BOOL           fRet = FALSE;
LPTSTR         lpData;
LPTSTR         lpPtr;
SS_COORD       ColFound;
SS_COORD       RowFound;
SS_COORD       Col;
SS_COORD       Row;
SS_COORD       i;
SS_CELLTYPE    CellType;

for (i = lpSS->Row.HeaderCnt, RowFound = 0; !RowFound && i < lpSS->Row.AllocCnt; i++)
   {
   SS_RetrieveCellType(lpSS, &CellType, NULL, 0, i);
   if (SS_TYPE_STATICTEXT == CellType.Type &&
       SS_TEXT_PREFIX & CellType.Style)
      {
      if (hData = SS_GetData(lpSS, NULL, 0, i, FALSE))
         {
         lpData = (LPTSTR)tbGlobalLock(hData);

         //???? problem under unicode ????
         if (lpPtr = _ftcschr(lpData, '&'))
      #if defined(_WIN64) || defined(_IA64)
           if (AnsiLower((LPSTR)MAKELONG_PTR(*(lpPtr + 1), 0)) ==
                AnsiLower((LPSTR)MAKELONG_PTR(cChar, 0)))
      #else
           if (AnsiLower((LPSTR)MAKELONG(*(lpPtr + 1), 0)) ==
                AnsiLower((LPSTR)MAKELONG(cChar, 0)))
      #endif
               RowFound = i;

         tbGlobalUnlock(hData);
         }
      }
   }

if (RowFound)
   {
   for (Col = lpSS->Col.HeaderCnt; Col < SS_GetColCnt(lpSS) - 1 &&
        SS_GetColWidthInPixels(lpSS, Col) == 0; Col++)
      ;

   SS_ActivateCell(lpSS, Col, RowFound, TRUE);
   fRet = TRUE;
   }

if (!fRet)
   {
   for (i = lpSS->Col.HeaderCnt, ColFound = 0; !ColFound && i < lpSS->Col.AllocCnt; i++)
      {
      SS_RetrieveCellType(lpSS, &CellType, NULL, i, 0);
      if (SS_TYPE_STATICTEXT == CellType.Type &&
          SS_TEXT_PREFIX & CellType.Style)
         {
         if (hData = SS_GetData(lpSS, NULL, i, 0, FALSE))
            {
            lpData = (LPTSTR)tbGlobalLock(hData);

            //???? problem under unicode
            if (lpPtr = _ftcschr(lpData, '&'))
              #if defined(_WIN64) || defined(_IA64)
              if (AnsiLower((LPSTR)MAKELONG_PTR(*(lpPtr + 1), 0)) ==
                   AnsiLower((LPSTR)MAKELONG_PTR(cChar, 0)))
              #else
              if (AnsiLower((LPSTR)MAKELONG(*(lpPtr + 1), 0)) ==
                   AnsiLower((LPSTR)MAKELONG(cChar, 0)))
              #endif
                  ColFound = i;

            tbGlobalUnlock(hData);
            }
         }
      }

   if (ColFound)
      {
      for (Row = lpSS->Row.HeaderCnt; Row < SS_GetRowCnt(lpSS) - 1 &&
           SS_GetRowHeightInPixels(lpSS, Row) == 0; Row++)
         ;

      SS_ActivateCell(lpSS, ColFound, Row, TRUE);
      fRet = TRUE;
      }
   }

return (fRet);
}


#if defined(SS_OCX)
HACCEL SS_CreateAcceleratorTable(LPSPREADSHEET lpSS, USHORT FAR* lpcAccel)
{
TBGLOBALHANDLE hData;
LPTSTR         lpData;
LPTSTR         lpPtr;
SS_COORD       i;
SS_CELLTYPE    CellType;
TCHAR          ch;
ACCEL          accel;
HACCEL         hAccel = 0;
USHORT         cAccel = 0;
LPBYTE         lpBuff = 0;
HANDLE         hBuff = 0;
long           lBuffLen = 0;
long           lBuffAlloc = 0;

for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
   {
   SS_RetrieveCellType(lpSS, &CellType, NULL, 0, i);
   if (SS_TYPE_STATICTEXT == CellType.Type &&
       SS_TEXT_PREFIX & CellType.Style)
      {
      if (hData = SS_GetData(lpSS, NULL, 0, i, FALSE))
         {
         lpData = (LPTSTR)tbGlobalLock(hData);

         if (lpPtr = _ftcschr(lpData, '&'))
            if (ch = *(lpPtr + 1))
               {
               accel.fVirt = FALT | FVIRTKEY;
               #if defined(WIN32)
               accel.key = (TCHAR)CharUpper((LPTSTR)ch);
               #else
               accel.key = (TCHAR)(long)AnsiUpper((LPSTR)ch);
               #endif
               accel.cmd = TRUE;
               cAccel++;
               lpBuff = SS_HugeBufferAlloc(lpBuff, &lBuffLen,
                                           &accel, sizeof(accel),
                                           &lBuffAlloc, &hBuff);
               }

         tbGlobalUnlock(hData);
         }
      }
   }

for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
   {
   SS_RetrieveCellType(lpSS, &CellType, NULL, i, 0);
   if (SS_TYPE_STATICTEXT == CellType.Type &&
       SS_TEXT_PREFIX & CellType.Style)
      {
      if (hData = SS_GetData(lpSS, NULL, i, 0, FALSE))
         {
         lpData = (LPTSTR)tbGlobalLock(hData);

         if (lpPtr = _ftcschr(lpData, '&'))
            if (ch = *(lpPtr + 1))
               {
               accel.fVirt = FALT | FVIRTKEY;
               #if defined(WIN32)
               accel.key = (TCHAR)CharUpper((LPTSTR)ch);
               #else
               accel.key = (TCHAR)(long)AnsiUpper((LPSTR)ch);
               #endif
               accel.cmd = TRUE;
               cAccel++;
               lpBuff = SS_HugeBufferAlloc(lpBuff, &lBuffLen,
                                            &accel, sizeof(accel),
                                            &lBuffAlloc, &hBuff);
               }

         tbGlobalUnlock(hData);
         }
      }
   }

if (cAccel)
   {
   #if defined(WIN32)
   hAccel = CreateAcceleratorTable((LPACCEL)lpBuff, cAccel);
   GlobalUnlock(hBuff);
   GlobalFree(hBuff);
   #else
   ((LPACCEL)lpBuff)[cAccel-1].fVirt |= 0x80;
   GlobalUnlock(hBuff);
   hAccel = hBuff;
   #endif
   }

if( lpcAccel )
   *lpcAccel = cAccel;

return hAccel;
}
#endif  // defined(SSOCX)


#if defined(SS_OCX)
void SS_DestroyAcceleratorTable(HACCEL hAccel)
{
  #if defined(WIN32)
  DestroyAcceleratorTable(hAccel);
  #else
  GlobalFree(hAccel);
  #endif
}
#endif  // defined(SS_OCX)


#if (!defined(SS_NOMAXTEXT) || !defined(SS_NOOVERFLOW))

void SS_CalcCellMetrics(HDC hDC, LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow, LPSS_CELL Cell,
                        LPSS_CELLTYPE CellType, LPRECT lpRect, SS_COORD CellCol, SS_COORD CellRow, BOOL fCalcWidth,
                        BOOL fCalcHeight, LPSHORT lpJust, LPTSTR lpData, LPINT lpiWidth, LPINT lpiHeight)
{
TBGLOBALHANDLE    hGlobalData = 0;
LPSS_CELL         lpCellOrig = Cell;
LPSS_COL          lpColOrig = lpCol;
LPSS_ROW          lpRowOrig = lpRow;
HFONT             hFontOld;
HFONT             hFont;
RECT              RectTemp;
RECT              RectNew;
SS_CELLTYPE       CellTypeTemp;
SS_DATA           SSData;
LPSS_FONT         lpFont;
SS_FONT           FontTemp;
SS_FONTID         FontId;
TCHAR             Temp[10];
LPTSTR            Data = NULL;

#ifdef  BUGS
// Bug-002
TCHAR             Buffer[330];
#else
TCHAR             Buffer[200];
#endif

HWND              hWndCtrl = 0;
double            dfValue;
int               iWidth = 0;
int               iHeight = 0;
SS_COORD          Number;
short             dXMargin = lpSS->lpBook->dXMargin;
short             dYMargin = lpSS->lpBook->dYMargin;
short             Just;
short             xExtra;
short             yExtra;
short             JustRet = DT_LEFT;
short             i;
WORD              wMessageBeingSent;
SIZE              Size;
UINT              uTextAlignOld;
BOOL              fTurnEditModeOn = FALSE;

if (!lpCol && CellCol != -1)
   lpCol = SS_LockColItem(lpSS, CellCol);

if (!lpRow && CellRow != -1)
   lpRow = SS_LockRowItem(lpSS, CellRow);

if (!Cell)
   Cell = SSx_LockCellItem(lpSS, lpRow, CellCol, CellRow);

// RFW - 2/8/05 - 15697
if (lpSS->lpBook->EditModeOn && lpSS->Col.CurAt == CellCol && lpSS->Row.CurAt == CellRow)
   {
	// RFW - 4/20/05 - 16070
   wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;
   lpSS->lpBook->wMessageBeingSent = 0;
   SS_CellEditModeOff(lpSS, 0);
   fTurnEditModeOn = TRUE;
   }

if (!lpRect)
   {
   lpRect = &RectNew;
   lpRect->left = 0;
   lpRect->top = 0;
	SS_GetCellActualSize(lpSS, CellCol, CellRow, &lpRect->right, &lpRect->bottom);
	lpRect->right--;
	lpRect->bottom--;
//   lpRect->bottom = SS_GetRowHeightInPixels(lpSS, CellRow) - 1;
//   lpRect->right = SS_GetColWidthInPixels(lpSS, CellCol) - 1;
   }
else
	{
	RectNew = *lpRect;
	lpRect = &RectNew;
	}

iWidth = 0;
iHeight = 0;

if (!CellType)
   CellType = SSx_RetrieveCellType(lpSS, &CellTypeTemp, lpCol, lpRow, Cell,
                                   CellCol, CellRow);

#ifdef SS_V40
if ((CellType->Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_DOWN ||
    (CellType->Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_UP)
	{
	RECT RectTemp = *lpRect;
	lpRect->left = RectTemp.top;
	lpRect->top = RectTemp.left;
	lpRect->right = RectTemp.bottom;
	lpRect->bottom = RectTemp.right;
	}
#endif  //SS_V40

uTextAlignOld = SetTextAlign(hDC, TA_NOUPDATECP);

SSData.bDataType = 0;

if (lpData)
   Data = lpData;

else
   {
   if (SS_IsCellTypeFloatNum(CellType))
      {
      if (hGlobalData = SSx_GetData(lpSS, lpCol, lpRow, Cell, CellType,
                                    &SSData, CellCol, CellRow, FALSE))
         Data = (LPTSTR)tbGlobalLock(hGlobalData);
      else if (SSData.bDataType == SS_TYPE_FLOAT)
         Data = (LPTSTR)1;
      }
   else
      {
      if (hGlobalData = SSx_GetData(lpSS, lpCol, lpRow, Cell, CellType,
                                    NULL, CellCol, CellRow, FALSE))
         Data = (LPTSTR)tbGlobalLock(hGlobalData);
      }

   /*****************
   * Format Headers
   *****************/

   if (CellCol >= lpSS->Col.HeaderCnt && CellRow < lpSS->Row.HeaderCnt &&
       (CellRow == lpSS->Row.lHeaderDisplayIndex || (lpSS->Row.lHeaderDisplayIndex == -1 && CellRow == lpSS->Row.HeaderCnt - 1)) &&
       (!hGlobalData || !Cell || !Cell->Data.bDataType))
      {
      if (CellRow < lpSS->Row.HeaderCnt)
         {
         Number = CellCol - lpSS->Col.HeaderCnt + lpSS->Col.NumStart;

         if (lpSS->ColHeaderDisplay == SS_HEADERDISPLAY_LETTERS)
            SS_DrawFormatHeaderLetter(Temp, Number);
         else if (lpSS->ColHeaderDisplay == SS_HEADERDISPLAY_NUMBERS)
            SS_DrawFormatHeaderNumber(Temp, Number);
         else
            Temp[0] = '\0';

         Data = Temp;
         }
      }

   else if (CellRow >= lpSS->Row.HeaderCnt && CellCol < lpSS->Col.HeaderCnt &&
            (CellCol == lpSS->Col.lHeaderDisplayIndex || (lpSS->Col.lHeaderDisplayIndex == -1 && CellCol == lpSS->Col.HeaderCnt - 1)) &&
            (!hGlobalData || !Cell || !Cell->Data.bDataType))
      {
      if (CellCol < lpSS->Col.HeaderCnt)
         {
         if (lpSS->fVirtualMode &&
             (lpSS->Virtual.lVStyle & SSV_NOROWNUMBERS))
            Data = _T("");
         else
            {
            Number = CellRow - lpSS->Row.HeaderCnt + lpSS->Row.NumStart;

            if (lpSS->RowHeaderDisplay == SS_HEADERDISPLAY_LETTERS)
               SS_DrawFormatHeaderLetter(Temp, Number);
            else if (lpSS->RowHeaderDisplay == SS_HEADERDISPLAY_NUMBERS)
               SS_DrawFormatHeaderNumber(Temp, Number);
            else
               Temp[0] = '\0';

            Data = Temp;
            }
         }
      }

   else if (CellCol < lpSS->Col.HeaderCnt && CellRow < lpSS->Row.HeaderCnt && (!Cell || !Cell->Data.bDataType))
      Data = _T("");
   }

FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol, CellRow);
lpFont = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);
hFont = lpFont->hFont;
hFontOld = SelectObject(hDC, hFont);

#ifdef SS_V40
if ((CellType->Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_DOWN ||
    (CellType->Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_UP)
	{
	BOOL fTemp = fCalcHeight;
	fCalcHeight = fCalcWidth;
	fCalcWidth = fTemp;
	}
#endif //SS_V40

switch (CellType->Type)
   {
   case SS_TYPE_STATICTEXT:
      if (Data)
         {
         short dShadowSize = 0;

         Just = SS_RetrieveJust((short)CellType->Style);
         JustRet = Just;

         if (CellType->Style & (SS_TEXT_SHADOW | SS_TEXT_SHADOWIN))
            {
				/* RFW - 5/22/07 - 20480
            if (CellCol < lpSS->Col.HeaderCnt && CellRow < lpSS->Row.HeaderCnt)
               dShadowSize = 2;
            else
               dShadowSize = 1;
				*/

#ifdef SS_V35
	         dShadowSize = 1;
#else
				dShadowSize = (CellCol < lpSS->Col.HeaderCnt && CellRow < lpSS->Row.HeaderCnt) ? 2 : 1;
#endif
            }

			dXMargin = 1; // RFW - 5/22/07 - 20480

         if (fCalcHeight)
            {
            if (Just & DT_WORDBREAK)
               {
               SetRect(&RectTemp, lpRect->left + dXMargin + dShadowSize,
                       lpRect->top + dYMargin,
                       lpRect->right - dXMargin - dShadowSize,
                       lpRect->bottom - dYMargin);

					// RFW - 5/22/07 - 20480
#ifdef SS_V35
#ifndef SS_V40
		         if (CellCol < lpSS->Col.HeaderCnt || CellRow < lpSS->Row.HeaderCnt)
#endif // SS_V40
						{
						RectTemp.right++;
						RectTemp.bottom++;
						}
#endif


#ifdef SPREAD_JPN
					// BUG 01862 (1-1)
					// It Uses Different Way To Adjust Rectangle Of Cell,
					// When Draw Text And Calculate Height Of Text.
					// Make Them Same To Solve The Problem Of This Bug.
					// Modified By HaHa 1999.11.30
					if (Just & DT_RIGHT)
						RectTemp.left -= dXMargin;
					else if (Just & DT_CENTER)
					{
						RectTemp.left -= dXMargin;
						RectTemp.right += dXMargin;
					}
					else
						RectTemp.right += dXMargin;

					// BUG 01862 Refix (3-1)
					// DrawText Function Will Cause New Problem In Bug 01862.
					// Now I Remove All Fixing Of Bug SPR_JPN_001_020 And Still Use fpDrawText.
					// Modified By HaHa 1999.12.16

					// SPR_JPN_001_020 Refix (2-2)
					// Modified By HaHa 1999.12.15
					//DrawText(hDC, Data, -1, &RectTemp, Just | DT_CALCRECT);
					//DrawText(hDC, Data, lstrlen(Data), &RectTemp, DT_LEFT |
					//		DT_WORDBREAK | DT_CALCRECT | (Just & DT_NOPREFIX));
#endif
					{
					FPDRAWTEXT dt;
						
					_fmemset(&dt, '\0', sizeof(dt));
					dt.dwFontLanguageInfo = lpFont->dwFontLanguageInfo;

					if (lpFont->hCharWidths)
						dt.lpCharWidths = (int FAR *)tbGlobalLock(lpFont->hCharWidths);

					fpDrawText(lpSS->lpBook->hWnd, hDC, Data, -1, &RectTemp, DT_LEFT |
								  DT_WORDBREAK | DT_CALCRECT | (Just & DT_NOPREFIX),
								  &dt);

					if (lpFont->hCharWidths)
						tbGlobalUnlock(lpFont->hCharWidths);
					}

               iHeight = RectTemp.bottom - RectTemp.top +
                         ((dYMargin + dShadowSize) * 2) + 1;
               }
            else
               {
               GetTextExtentPoint(hDC, Data, lstrlen(Data), &Size);
					/* RFW - 4/20/04 - 14364
               iHeight = Size.cy + ((dYMargin + dShadowSize) * 2);
					*/
               iHeight = Size.cy + ((dYMargin + dShadowSize) * 2) + 1;
               }
            }

         if (fCalcWidth)
            {
				/* RFW - 6/2/04 - 14481
            GetTextExtentPoint(hDC, Data, lstrlen(Data), &Size);
            iWidth = Size.cx;
				*/
				FPDRAWTEXT dt;
					
				_fmemset(&dt, '\0', sizeof(dt));

				dt.dwFontLanguageInfo = lpFont->dwFontLanguageInfo;

				if (lpFont->hCharWidths)
					dt.lpCharWidths = (int FAR *)tbGlobalLock(lpFont->hCharWidths);

				SetRectEmpty(&RectTemp);
				fpDrawText(lpSS->lpBook->hWnd, hDC, Data, -1, &RectTemp, DT_LEFT |
							  DT_SINGLELINE | DT_CALCRECT | (Just & DT_NOPREFIX), &dt);

				if (lpFont->hCharWidths)
					tbGlobalUnlock(lpFont->hCharWidths);

				iWidth = RectTemp.right - RectTemp.left;

            iWidth += (dXMargin + dShadowSize) * 2;
            if (Just & DT_WORDBREAK)
               iWidth = min(iWidth, lpRect->right - lpRect->left);
            }
         }

      break;

   case SS_TYPE_PICTURE:
      // BJO 28Aug96 ARR157 - Begin fix (remove SS_RetrieveControlhWnd)
      //hWndCtrl = SS_RetrieveControlhWnd(lpSS, CellCol, CellRow);
      // BJO 28Aug96 ARR157 - End fix      

/* RFW 12/8/98 TEL1872
      if (fCalcWidth)
         iWidth = lpRect->right - lpRect->left;

      if (fCalcHeight)
         iHeight = lpRect->bottom - lpRect->top + 1;
*/


      if (CellType->Style & VPS_ICON)
         {
         iWidth = tbGetIconWidth((HICON)CellType->Spec.ViewPict.hPictName);
         iHeight = tbGetIconHeight((HICON)CellType->Spec.ViewPict.hPictName);
         }
      else if (CellType->Spec.ViewPict.hPictName)
         {
         BITMAP bm;

         GetObject((HANDLE)CellType->Spec.ViewPict.hPictName, sizeof(BITMAP),
                   (LPVOID)&bm);
         iWidth = (short)bm.bmWidth;
         iHeight = (short)bm.bmHeight;
         }

      /*
      if (hWndCtrl)
         {
         lpViewPict = ViewPictGetPtr(hWndCtrl, &hGlobal);

         if (fCalcWidth)
            iWidth = (short)lpViewPict->Picture.bi.biWidth;

         if (fCalcHeight)
            iWidth = (short)lpViewPict->Picture.bi.biHeight;

         GlobalUnlock(hGlobal);
         }
      */

      break;

   case SS_TYPE_BUTTON:
      {
#ifdef SS_V80
	  BOOL fUseVisualStyles = (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1);
      long lRet = SS_BtnDraw(lpSS->lpBook->hWnd, hDC, NULL, CellType, hFont, 0, 0, 0,
                             NULL, TRUE, lpSS->lpBook->fIs95, TRUE, FALSE, fUseVisualStyles);
#else
      long lRet = SS_BtnDraw(lpSS->lpBook->hWnd, hDC, NULL, CellType, hFont, 0, 0, 0,
                             NULL, TRUE, lpSS->lpBook->fIs95, TRUE);
#endif

      if (fCalcWidth)
         iWidth = LOWORD(lRet) + 2;

      if (fCalcHeight)
         iHeight = HIWORD(lRet);
      }

      break;

   case SS_TYPE_CHECKBOX:
      {
      HWND hWndCtrl = SS_RetrieveControlhWnd(lpSS, CellCol, CellRow);
      long lRet;
#ifdef SS_V80
	  BOOL fUseVisualStyles = (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1);
      lRet = SS_CheckBoxDraw(lpSS->lpBook->hWnd, hWndCtrl, hDC, NULL, CellType, hFont,
                             0, 0, NULL, TRUE, fUseVisualStyles);
#else
      lRet = SS_CheckBoxDraw(lpSS->lpBook->hWnd, hWndCtrl, hDC, NULL, CellType, hFont,
                             0, 0, NULL, TRUE);
#endif
      if (fCalcWidth)
         iWidth = LOWORD(lRet) + 2;

      if (fCalcHeight)
         iHeight = HIWORD(lRet);
      }

      break;

   case SS_TYPE_COMBOBOX:
      if (Data)
         {
         if (fCalcHeight)
            {
            GetTextExtentPoint(hDC, Data, lstrlen(Data), &Size);
            iHeight = Size.cy + 2 * dYMargin;
            }

         if (fCalcWidth)
            {
            GetTextExtentPoint(hDC, Data, lstrlen(Data), &Size);
            iWidth =  Size.cx  + dXMargin;
            }
         }

      if (fCalcWidth)
         iWidth += lpSS->lpBook->dComboButtonBitmapWidth;

      break;

#if SS_V80
      case SS_TYPE_CUSTOM:
        {
          SIZE sizePreferred;
          LPSS_CELLTYPE CellTypeTemp = SS_CT_LockCellType(lpSS, CellCol, CellRow);
          if( SS_CT_GetPreferredSize(lpSS, CellTypeTemp, hDC, lpRect, CellCol, CellRow, &sizePreferred) )
          {
            if( fCalcWidth )
              iWidth = sizePreferred.cx;
            if( fCalcHeight )
              iHeight = sizePreferred.cy;
          }
          SS_CT_UnlockCellType(lpSS, CellCol, CellRow);
        }
        break;
#endif
   case SS_TYPE_EDIT:
   case SS_TYPE_PIC:
   case SS_TYPE_DATE:
   case SS_TYPE_TIME:
   case SS_TYPE_INTEGER:
   case SS_TYPE_FLOAT:
#ifdef SS_V40
   case SS_TYPE_CURRENCY:
   case SS_TYPE_NUMBER:
   case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
   case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
/* RFW - 6/7/02 - RUN_CEL_006_015
      if ((CellType->Type == SS_TYPE_EDIT &&
          (CellType->Style & ES_RIGHT)) ||
          CellType->Type == SS_TYPE_INTEGER ||
          SS_IsCellTypeFloatNum(CellType))
         JustRet = DT_RIGHT;
      else if (CellType->Style & ES_CENTER)
         JustRet = DT_CENTER;
*/

      if (CellType->Style & ES_RIGHT)
         JustRet = DT_RIGHT;
      else if (CellType->Style & ES_CENTER)
         JustRet = DT_CENTER;
      else if (CellType->Style & ES_LEFTALIGN)
         JustRet = DT_LEFT;
      else if (CellType->Type == SS_TYPE_INTEGER || SS_IsCellTypeFloatNum(CellType))
         JustRet = DT_RIGHT;
      else
         JustRet = DT_LEFT;

      if (CellType->Type == SS_TYPE_EDIT &&
          ((CellType->Style & ES_MULTILINE) || (lpSS->lpBook->fPrintExpandMultiLine &&
          lpSS->fPrintingInProgress)))
         {
         lpRect->left += 2;
         lpRect->top += 1;
         lpRect->right -= 2;
         lpRect->bottom -= 1;

         xExtra = 5;
         yExtra = 4;
         }

      else
         {
         if (JustRet & DT_RIGHT)
            lpRect->right -= dXMargin;
         else if (!(JustRet & DT_CENTER))
            lpRect->left += dXMargin;
         //lpRect->right -= dXMargin;
         //lpRect->left += dXMargin;
         lpRect->top += dYMargin;

         xExtra = (2 * dXMargin);
         yExtra = (2 * dYMargin);
         }

      if (SSData.bDataType == SS_TYPE_FLOAT)
         {
         _fmemset(Buffer, '\0', sizeof(Buffer));
         SS_FloatFormat(lpSS, CellType, SSData.Data.dfValue, Buffer,
                        FALSE);
         Data = Buffer;
         }

      else if (SS_IsCellTypeFloatNum(CellType))
         {
         if (Data && *Data)
            {
            _fmemset(Buffer, '\0', sizeof(Buffer));
            SS_StringToNum(lpSS, CellType, Data, &dfValue),
            SS_FloatFormat(lpSS, CellType, dfValue, Buffer, FALSE);
            Data = Buffer;
            }
         }

      else if (CellType->Type == SS_TYPE_EDIT &&
               (CellType->Style & ES_PASSWORD) &&
               !(CellType->Style & ES_MULTILINE))
         {
         for (i = 0; i < lstrlen(Data); i++)
            Buffer[i] = '*';

         Buffer[lstrlen(Data)] = '\0';

         Data = Buffer;
         }

      if (Data)
         {
         if (fCalcHeight)
            {
            if (CellType->Type == SS_TYPE_EDIT &&
                ((CellType->Style & ES_MULTILINE) ||
                (lpSS->lpBook->fPrintExpandMultiLine && lpSS->fPrintingInProgress)))
               {
               CopyRect(&RectTemp, lpRect);

					// BUG 01862 Refix (3-2)
					// Modified By HaHa 1999.12.16

					// SPR_JPN_001_020 Refix (2-1)
					// When Editing Text, Edit Box Takes Over The Control Of Drawing Text,
					// It Must Use DrawText Function To Show Text.
					// To Make Editing, Showing And Calculating Height Have Same Result,
					// They Should Use Same Function.
					// fpDrawText Is Little Different To DrawText, While They Should Have Same Result.
					// Since fpDrawText Function Is Difficult To Change, 
					// Use DrawText Here Instead Of fpDrawText Is An Effective Way To Avoid Bug.
					// Modified By HaHa 1999.12.15
					//DrawText(hDC, Data, -1, &RectTemp, DT_LEFT | DT_WORDBREAK |
					//		DT_CALCRECT | DT_NOPREFIX);
					{
					FPDRAWTEXT dt;
						
					_fmemset(&dt, '\0', sizeof(dt));
					dt.dwFontLanguageInfo = lpFont->dwFontLanguageInfo;

					if (lpFont->hCharWidths)
						dt.lpCharWidths = (int FAR *)tbGlobalLock(lpFont->hCharWidths);

					fpDrawText(lpSS->lpBook->hWnd, hDC, Data, -1, &RectTemp, DT_LEFT |
								  DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX, &dt);

					if (lpFont->hCharWidths)
						tbGlobalUnlock(lpFont->hCharWidths);				}

               iHeight = RectTemp.bottom - RectTemp.top;
               }
            else 
               {
               GetTextExtentPoint(hDC, Data, lstrlen(Data), &Size);
               iHeight = Size.cy;
               }

            iHeight += yExtra;
            }

         if (fCalcWidth)
            {
				int iLen = lstrlen(Data);
				ABCFLOAT abcf;
				int abcfXtra = 0;

				if (iLen)
					if (GetCharABCWidthsFloat(hDC, Data[iLen - 1], Data[iLen - 1], &abcf) &&
                   abcf.abcfC < 0)
						abcfXtra = -(int)abcf.abcfC;

            GetTextExtentPoint32(hDC, Data, iLen, &Size);
            iWidth = Size.cx + xExtra + abcfXtra;

            if (CellType->Type == SS_TYPE_EDIT &&
                ((CellType->Style & ES_MULTILINE) ||
                (lpSS->lpBook->fPrintExpandMultiLine && lpSS->fPrintingInProgress)))
               iWidth = min(iWidth, lpRect->right - lpRect->left + xExtra - 1);
            }
         }

      break;

   }

SelectObject(hDC, hFontOld);

if (hGlobalData)
   tbGlobalUnlock(hGlobalData);

// RFW - 2/8/05 - 15697
if (fTurnEditModeOn)
   {
   SS_CellEditModeOn(lpSS, 0, 0, 0L);
   lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;
   }

if (!lpCellOrig)
   SSx_UnlockCellItem(lpSS, lpRow, CellCol, CellRow);

if (!lpColOrig)
   SS_UnlockColItem(lpSS, CellCol);

if (!lpRowOrig)
   SS_UnlockRowItem(lpSS, CellRow);

if (lpJust)
   *lpJust = JustRet;

SetTextAlign(hDC, uTextAlignOld);

#ifdef SS_V40
if ((CellType->Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_DOWN ||
    (CellType->Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_UP)
	{
	int dTemp = iHeight;
	iHeight = iWidth;
	iWidth = dTemp;
	}
#endif //SS_V40

if (lpiWidth)
	*lpiWidth = iWidth;

if (lpiHeight)
	*lpiHeight = iHeight;
}

#endif


BOOL SS_ValidatePasteData(HWND hWnd, LPSPREADSHEET lpSS, LPTSTR lpszText,
                          LPSS_CELLCOORD lpCellUL, LPSS_CELLCOORD lpCellLR,
                          BOOL fValidate)
{
#ifdef SS_UTP

BOOL fRet = FALSE;

if (!lpSS->fAutoClipboard || !lpSS->fValidateClipboardPaste)
   return (TRUE);

else if (!lpszText)
   {
   GLOBALHANDLE  hClip;
   LPTSTR        lpStrOut;

   if (!SS_USESINGLESELBAR(lpSS))
      {
      if (OpenClipboard(hWnd))
         {
         if (IsClipboardFormatAvailable(CF_TEXT) ||
             IsClipboardFormatAvailable(CF_OEMTEXT))
            {
            #if defined(UNICODE) || defined(_UNICODE)
            hClip = GetClipboardData(CF_UNICODETEXT);
            #else
            hClip = GetClipboardData(CF_TEXT);
            #endif

            if (hClip)
               {
               if (lpStrOut = GlobalLock(hClip))
                  {
                  fRet = SSx_ValidatePasteData(lpSS, lpStrOut,
                                               lpCellUL, lpCellLR, fValidate);

                  GlobalUnlock(hClip);
                  }
               }
            }

         CloseClipboard();
         }
      }
   }

else
   fRet = SSx_ValidatePasteData(lpSS, lpszText, lpCellUL, lpCellLR,
                                fValidate);

return (fRet);

#else

return (TRUE);

#endif
}


BOOL SSx_ValidatePasteData(LPSPREADSHEET lpSS, LPTSTR lpszText,
                           LPSS_CELLCOORD lpCellUL, LPSS_CELLCOORD lpCellLR,
                           BOOL fValidate)
{
#ifdef SS_UTP

SS_COORD ColCntMax = 0;
BOOL     fRet = FALSE;

if (!lpSS->fAutoClipboard || !lpSS->fValidateClipboardPaste)
   return (TRUE);

/************************
* Everthing is selected
************************/

if ((lpCellUL->Col == -1 || lpCellLR->Col == -1) &&
    (lpCellUL->Row == -1 || lpCellLR->Row == -1))
   fRet = TRUE;

else
   {
   SS_COORD RowCnt = -1;
   SS_COORD ColCnt = -1;
   SS_COORD RowCntTemp = 0;
   SS_COORD ColCntTemp = 0;

   /****************
   * Rows selected
   ****************/

   if (lpCellUL->Col == -1 || lpCellLR->Col == -1)
      RowCnt = lpCellLR->Row - lpCellUL->Row + 1;

   /****************
   * Cols selected
   ****************/

   else if (lpCellUL->Row == -1 || lpCellLR->Row == -1)
      ColCnt = lpCellLR->Col - lpCellUL->Col + 1;

   /****************
   * Cells selected
   ****************/

   else
      {
      ColCnt = lpCellLR->Col - lpCellUL->Col + 1;
      RowCnt = lpCellLR->Row - lpCellUL->Row + 1;
      }

   while (TRUE)
      {
      if (*lpszText == '\t')
         ColCntTemp++;

      else if (*lpszText == '\0')
         {
         ColCntTemp++;
         RowCntTemp++;
         ColCntMax = max(ColCntMax, ColCntTemp);
         break;
         }

      ColCntMax = max(ColCntMax, ColCntTemp);

      if (fValidate && ColCnt != -1 && ColCntTemp > ColCnt)
         break;

      else if (*lpszText == '\r')
         {
         ColCntTemp++;

         if (fValidate && ColCnt != -1 && ColCntTemp > ColCnt)
            break;

         ColCntMax = max(ColCntMax, ColCntTemp);

         ColCntTemp = 0;
         RowCntTemp++;

         if (*(lpszText + 1) == '\0' ||
             (*(lpszText + 1) == '\n' && *(lpszText + 2) == '\0'))
            break;
         }

      if (fValidate && RowCnt != -1 && RowCntTemp > RowCnt)
         break;

      lpszText++;
      }

   if ((ColCnt == -1 || ColCntTemp <= ColCnt) &&
       (RowCnt == -1 || RowCntTemp <= RowCnt))
      fRet = TRUE;

   if (!fValidate)
      {
      if (ColCnt != -1)
         lpCellLR->Col = lpCellUL->Col + ColCntMax - 1;

      if (RowCnt != -1)
         lpCellLR->Row = lpCellUL->Row + RowCntTemp - 1;

      fRet = TRUE;
      }
   }

if (!fRet)
   if (SS_SendMsgCommand(lpSS->lpBook->hWnd, NULL, SSN_INVALIDPASTEDATA, FALSE))
      fRet = TRUE;

return (fRet);

#else

return (TRUE);

#endif
}


void SS_ClearUndoBuffer(LPSPREADSHEET lpSS)
{
#ifdef SS_UTP
LPTBGLOBALHANDLE lpList;
LPTBGLOBALHANDLE lpCellList;
TBGLOBALHANDLE   hCell;
LPSS_UNDOCOL     lpColList;
LPSS_ROW         lpRow;
LPSS_COL         lpCol;
LPSS_CELL        lpCell;
SS_COORD         Index;
SS_COORD         i;
SS_COORD         j;

if (lpSS->fAllowUndo)
   {
   switch (lpSS->UndoBuffer.bUndoType)
      {
      case SS_UNDOTYPE_DELROW:
         if (lpSS->UndoBuffer.hData)
            {
            lpList = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->UndoBuffer.hData);

            for (i = lpSS->UndoBuffer.Row; i <= lpSS->UndoBuffer.Row2;
                 i++)
               {
               if (lpList[i - lpSS->UndoBuffer.Row])
                  {
                  lpRow = (LPSS_ROW)tbGlobalLock(lpList[i -
                                                 lpSS->UndoBuffer.Row]);

                  for (j = 0; j < lpSS->UndoBuffer.ItemAllocCnt; j++)
                     if ((hCell = SS_GetItem(lpSS, &lpRow->Cells, j,
                                             SS_GetColCnt(lpSS))) &&
                                             hCell != (ULONG)-1L)
                        {
                        lpCell = (LPSS_CELL)tbGlobalLock(hCell);
                        SSx_FreeCell(lpSS, lpCell);
                        tbGlobalUnlock(hCell);
                        SS_DeleteItem(lpSS, &lpRow->Cells, j,
                                      SS_GetColCnt(lpSS));
                        }

                  SSx_FreeRow(lpSS, lpRow);
                  tbGlobalUnlock(lpList[i - lpSS->UndoBuffer.Row]);
                  MemFreeFS((LPVOID)lpList[i - lpSS->UndoBuffer.Row]);
                  }
               }

            tbGlobalUnlock(lpSS->UndoBuffer.hData);
            tbGlobalFree(lpSS->UndoBuffer.hData);
            }

         break;

      case SS_UNDOTYPE_DELCOL:
         if (lpSS->UndoBuffer.hData)
            {
            lpColList = (LPSS_UNDOCOL)tbGlobalLock(lpSS->UndoBuffer.hData);

            for (Index = 0, i = lpSS->UndoBuffer.Col;
                 i <= lpSS->UndoBuffer.Col2; i++, Index++)
               {
               lpCellList = (LPTBGLOBALHANDLE)tbGlobalLock(
                                              lpColList[Index].hCellList);

               for (j = 0; j < lpSS->UndoBuffer.ItemAllocCnt; j++)
                  if (lpCellList[j])
                     {
                     lpCell = (LPSS_CELL)tbGlobalLock(lpCellList[j]);
                     SSx_FreeCell(lpSS, lpCell);
                     tbGlobalUnlock(lpCellList[j]);
                     MemFreeFS((LPVOID)lpCellList[j]);
                     }

               if (lpColList[Index].hCol)
                  {
                  lpCol = (LPSS_COL)tbGlobalLock(lpColList[Index].hCol);
                  SSx_FreeCol(lpSS, lpCol);
                  tbGlobalUnlock(lpColList[Index].hCol);
                  MemFreeFS((LPVOID)lpColList[Index].hCol);
                  }
               }

            tbGlobalUnlock(lpSS->UndoBuffer.hData);
            tbGlobalFree(lpSS->UndoBuffer.hData);
            }

         break;

      }

   _fmemset(&lpSS->UndoBuffer, '\0', sizeof(SS_UNDOBUFFER));
   }

#endif
}


void SS_Undo(HWND hWnd, LPSPREADSHEET lpSS)
{
#ifdef SS_UTP
LPTBGLOBALHANDLE lpList;
LPTBGLOBALHANDLE lpCellList;
LPSS_UNDOCOL     lpColList;
LPSS_CELL        lpCell;
LPSS_ROW         lpRow;
SS_COORD         Index;
SS_COORD         RowCnt;
SS_COORD         ColCnt;
SS_COORD         i;
SS_COORD         j;
BOOL             fRedrawOld;

if (lpSS->fAllowUndo)
   {
   switch (lpSS->UndoBuffer.bUndoType)
      {
      case SS_UNDOTYPE_DELROW:
         if (lpSS->UndoBuffer.hData &&
             !SS_SendMsgInsDel(hWnd, SS_INSDEL_TYPE_INS, SS_INSDEL_OP_UNDO,
                               SS_INSDEL_ROW, lpSS->UndoBuffer.Row,
                               lpSS->UndoBuffer.Row2))
            {
            lpList = (LPTBGLOBALHANDLE)tbGlobalLock(lpSS->UndoBuffer.hData);

            fRedrawOld = lpSS->Redraw;
            lpSS->Redraw = FALSE;
            SSSetMaxRows(hWnd, SS_GetRowCnt(lpSS) - 1 +
                         (lpSS->UndoBuffer.Row2 -
                         lpSS->UndoBuffer.Row + 1));

            if (lpSS->UndoBuffer.Row >= lpSS->Row.AllocCnt)
               {
               if (lpSS->UndoBuffer.Row >= lpSS->Row.UL)
                  SS_InvalidateRowRange(lpSS, lpSS->UndoBuffer.Row, -1);
               else
                  SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ROWS);
               }
            else
               SS_InsRowRange(lpSS, lpSS->UndoBuffer.Row, lpSS->UndoBuffer.Row2);

            ColCnt = 0;

            for (i = lpSS->UndoBuffer.Row; i <= lpSS->UndoBuffer.Row2; i++)
               {
               if (lpList[i - lpSS->UndoBuffer.Row])
                  SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolRow, &lpSS->Row.Items,
                                i, sizeof(SS_ROW),
                                lpList[i - lpSS->UndoBuffer.Row],
                                SS_GetRowCnt(lpSS), NULL);

               ColCnt = max(ColCnt, SS_GetRowAllocCnt(lpSS, i) - 1);

               if (lpRow = SS_AllocLockRow(lpSS, i))
                  {
                  lpRow->bDirty = SS_ROWDIRTY_ALL;
                  lpRow->hlstrBookMark = 0;
                  VBSSBoundChangeMade(lpSS, VBGetHwndControl(hWnd), hWnd, SS_ALLCOLS,
                                      i);
                  SS_UnlockRowItem(lpSS, i);
                  }
               }

            lpSS->Col.AllocCnt = max(lpSS->Col.AllocCnt, ColCnt + 1);
            lpSS->Row.AllocCnt = max(lpSS->Row.AllocCnt,
                                     lpSS->UndoBuffer.Row2 + 1);

            SS_SetDataColCnt(lpSS, ColCnt + 1);
            SS_SetDataRowCnt(lpSS, lpSS->UndoBuffer.Row2 + 1);

            SS_AdjustDataColCnt(lpSS, 1, ColCnt);
            SS_AdjustDataRowCnt(lpSS, lpSS->UndoBuffer.Row,
                                lpSS->UndoBuffer.Row2);

            tbGlobalUnlock(lpSS->UndoBuffer.hData);
            tbGlobalFree(lpSS->UndoBuffer.hData);
            // RFW - 9/6/94
            _fmemset(&lpSS->UndoBuffer, '\0', sizeof(SS_UNDOBUFFER));
				SS_BookSetRedraw(lpSS->lpBook, fRedrawOld)
            }

         break;

      case SS_UNDOTYPE_DELCOL:
         if (lpSS->UndoBuffer.hData &&
             !SS_SendMsgInsDel(hWnd, SS_INSDEL_TYPE_INS, SS_INSDEL_OP_UNDO,
                               SS_INSDEL_COL, lpSS->UndoBuffer.Col,
                               lpSS->UndoBuffer.Col2))
            {
            fRedrawOld = lpSS->Redraw;
            lpSS->Redraw = FALSE;
            SSSetMaxCols(hWnd, SS_GetColCnt(lpSS) - 1 +
                         (lpSS->UndoBuffer.Col2 -
                         lpSS->UndoBuffer.Col + 1));

            if (lpSS->UndoBuffer.Col >= lpSS->Col.AllocCnt)
               {
               if (lpSS->UndoBuffer.Col >= lpSS->Col.UL)
                  SS_InvalidateColRange(lpSS, lpSS->UndoBuffer.Col, -1);
               else
                  SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_COLS);
               }
            else
               SS_InsColRange(lpSS, lpSS->UndoBuffer.Col,
                             lpSS->UndoBuffer.Col2);

            lpColList = (LPSS_UNDOCOL)tbGlobalLock(lpSS->UndoBuffer.hData);

            for (Index = 0, RowCnt = 0, i = lpSS->UndoBuffer.Col;
                 i <= lpSS->UndoBuffer.Col2; i++, Index++)
               {
               if (lpColList[Index].hCol)
                  SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolCol,
                                &lpSS->Col.Items, i, sizeof(SS_COL),
                                lpColList[Index].hCol,
                                SS_GetColCnt(lpSS), NULL);

               lpCellList = (LPTBGLOBALHANDLE)tbGlobalLock(
                                              lpColList[Index].hCellList);

               RowCnt = max(RowCnt, lpSS->UndoBuffer.ItemAllocCnt);

               for (j = 0; j < lpSS->UndoBuffer.ItemAllocCnt; j++)
                  if (lpCellList[j])
                     {
                     if (lpRow = SS_AllocLockRow(lpSS, j))
                        {
                        SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolRow,
                                      &lpRow->Cells, i, sizeof(SS_CELL),
                                      lpCellList[j],
                                      SS_GetColCnt(lpSS), NULL);

                        if (lpCell = SS_AllocLockCell(lpSS, lpRow, i, j))
                           {
                           if (!lpRow->bDirty)
                              lpRow->bDirty = SS_ROWDIRTY_SOME;

                           lpCell->fDirty = TRUE;
                           SS_UnlockCellItem(lpSS, i, j);
                           }

                        SS_UnlockRowItem(lpSS, j);
                        }
                     }

               tbGlobalUnlock(lpColList[Index].hCellList);
               tbGlobalFree(lpColList[Index].hCellList);
               }

            lpSS->Row.AllocCnt = max(lpSS->Row.AllocCnt, RowCnt + 1);
            lpSS->Col.AllocCnt = max(lpSS->Col.AllocCnt,
                                     lpSS->UndoBuffer.Col2 + 1);

            SS_SetDataColCnt(lpSS, lpSS->UndoBuffer.Col2 + 1);
            SS_SetDataRowCnt(lpSS, RowCnt + 1);

            SS_AdjustDataColCnt(lpSS, lpSS->UndoBuffer.Col,
                                lpSS->UndoBuffer.Col2);
            SS_AdjustDataRowCnt(lpSS, 1, RowCnt);

            tbGlobalUnlock(lpSS->UndoBuffer.hData);
            tbGlobalFree(lpSS->UndoBuffer.hData);
            // RFW - 9/6/94
            _fmemset(&lpSS->UndoBuffer, '\0', sizeof(SS_UNDOBUFFER));
				SS_BookSetRedraw(lpSS->lpBook, fRedrawOld)
            }

         break;

      }
   }

#endif
}


void SS_VBDelBlock(HWND hWnd, LPSPREADSHEET lpSS, WORD wInsDelOperation)
{
#ifdef SS_UTP
if (SS_IsBlockSelected(lpSS))
   {
   SS_COORD DelCnt;
   BOOL     fRedrawOld;

   fRedrawOld = lpSS->Redraw;
   lpSS->Redraw = FALSE;

   if (lpSS->BlockCellUL.Col == -1 || lpSS->BlockCellLR.Col == -1)
      {
      DelCnt = lpSS->BlockCellLR.Row - lpSS->BlockCellUL.Row + 1;
      if (DelCnt > 0)
         {
         BOOL fDel = TRUE;

         if (!SS_SendMsgInsDel(hWnd, SS_INSDEL_TYPE_DEL, wInsDelOperation,
                               SS_INSDEL_ROW, lpSS->BlockCellUL.Row,
                               lpSS->BlockCellLR.Row))
            {
#ifdef SS_UTP
            HCURSOR hCursOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
#endif

#ifdef SS_BOUNDCONTROL
            if (VBSSBoundIsBound(VBGetHwndControl(hWnd), lpSS))
               if (VBSSBoundDel(lpSS, VBGetHwndControl(hWnd), hWnd,
                                lpSS->BlockCellUL.Row, lpSS->BlockCellLR.Row))
                  fDel = FALSE;
#endif

#if (defined(SS_VB) && defined(SS_QE))
            if (lpSpread->DBInfo.fConnected)
               if (DBSS_BoundDel(hWnd, lpSpread, lpSS->BlockCellUL.Row,
                                 lpSS->BlockCellLR.Row))
                  fDel = FALSE;
#endif

            if (fDel)
               SSDelRowRange(hWnd, lpSS->BlockCellUL.Row,
                             lpSS->BlockCellLR.Row);

            if (lpSS->BlockCellUL.Row >= lpSS->Row.AllocCnt)
               SS_InvalidateRowRange(lpSS, lpSS->BlockCellUL.Row, -1);

            SSSetMaxRows(hWnd, SS_GetRowCnt(lpSS) - 1 - DelCnt);
#ifdef SS_UTP
            SetCursor(hCursOld);
#endif
            }
         }
      }

   else if (lpSS->BlockCellUL.Row == -1 || lpSS->BlockCellLR.Row == -1)
      {
      DelCnt = lpSS->BlockCellLR.Col - lpSS->BlockCellUL.Col + 1;
      if (DelCnt > 0)
         {
         if (!SS_SendMsgInsDel(hWnd, SS_INSDEL_TYPE_DEL, wInsDelOperation,
                               SS_INSDEL_COL, lpSS->BlockCellUL.Col,
                               lpSS->BlockCellLR.Col))
            {
            SSDelColRange(hWnd, lpSS->BlockCellUL.Col, lpSS->BlockCellLR.Col);

            if (lpSS->BlockCellUL.Col >= lpSS->Col.AllocCnt)
               SS_InvalidateColRange(lpSS, lpSS->BlockCellUL.Col, -1);

            SSSetMaxCols(hWnd, SS_GetColCnt(lpSS) - 1 - DelCnt);
            }
         }
      }

	SS_BookSetRedraw(lpSS->lpBook, fRedrawOld)
   }
#endif
}


long hStrLen(HPTSTR hpStr)
{
long lLen = 0;

while (*hpStr++)
   lLen++;

return (lLen);
}


BOOL DLLENTRY SSSetPrintAbortMsg(HWND hWnd, LPTSTR lpszText)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet = SS_SetPrintAbortMsg(lpBook, lpszText);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetPrintAbortMsg(LPSS_BOOK lpBook, LPTSTR lpszText)
{
if (lpBook->hPrintAbortMsg)
	{
	tbGlobalFree(lpBook->hPrintAbortMsg);
	lpBook->hPrintAbortMsg = 0;
	}

if (lpszText && *lpszText)
	{
	if (lpBook->hPrintAbortMsg = tbGlobalAlloc(GHND, (lstrlen(lpszText) + 1) * sizeof(TCHAR)))
		{
		LPTSTR lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);
		lstrcpy(lpszTemp, lpszText);
		tbGlobalUnlock(lpBook->hPrintAbortMsg);
		}
	}

return (TRUE);
}


short DLLENTRY SSGetPrintAbortMsg(HWND hWnd, LPTSTR lpszText, short nLen)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
short nRet = SS_GetPrintAbortMsg(lpBook, lpszText, nLen);
SS_BookUnlock(hWnd);
return (nRet);
}


short SS_GetPrintAbortMsg(LPSS_BOOK lpBook, LPTSTR lpszText, short nLen)
{
short nRet = 0;

if (lpBook->hPrintAbortMsg)
	{
	LPTSTR lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);
	if (lpszText)
		lstrcpyn(lpszText, lpszTemp, nLen);
	nRet = lstrlen(lpszTemp);
	tbGlobalUnlock(lpBook->hPrintAbortMsg);
	}

return (nRet);
}

BOOL DLLENTRY SSSetPrintJobName(HWND hWnd, LPTSTR lpszText)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet = SS_SetPrintJobName(lpBook, lpszText);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_SetPrintJobName(LPSS_BOOK lpBook, LPTSTR lpszText)
{
if (lpBook->hPrintJobName)
	{
	tbGlobalFree(lpBook->hPrintJobName);
	lpBook->hPrintJobName = 0;
	}

if (lpszText && *lpszText)
	{
	if (lpBook->hPrintJobName = tbGlobalAlloc(GHND, (lstrlen(lpszText) + 1) * sizeof(TCHAR)))
		{
		LPTSTR lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hPrintJobName);
		lstrcpy(lpszTemp, lpszText);
		tbGlobalUnlock(lpBook->hPrintJobName);
		}
	}

return (TRUE);
}


short DLLENTRY SSGetPrintJobName(HWND hWnd, LPTSTR lpszText, short nLen)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
short nRet = SS_GetPrintJobName(lpBook, lpszText, nLen);
SS_BookUnlock(hWnd);
return (nRet);
}


short SS_GetPrintJobName(LPSS_BOOK lpBook, LPTSTR lpszText, short nLen)
{
short nRet = 0;

if (lpBook->hPrintJobName)
	{
	LPTSTR lpszTemp = (LPTSTR)tbGlobalLock(lpBook->hPrintJobName);
	if (lpszText)
		lstrcpyn(lpszText, lpszTemp, nLen);
	nRet = lstrlen(lpszTemp);
	tbGlobalUnlock(lpBook->hPrintJobName);
	}

return (nRet);
}
