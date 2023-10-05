/*********************************************************
* SS_CELL.C
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
#include "ss_draw.h"                   /* Needed for colors */
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_type.h"
#include "ss_win.h"

extern BOOL SSxx_SetCellData(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow,
                             LPTSTR Data, short Len);


BOOL SS_SetCellLock(lpSS, CellCol, CellRow, Lock)

LPSPREADSHEET lpSS;
SS_COORD      CellCol;
SS_COORD      CellRow;
BOOL          Lock;
{
LPSS_CELL     lpCell;
BOOL          fRet = FALSE;

if (lpCell = SS_AllocLockCell(lpSS, NULL, CellCol, CellRow))
   {
   lpCell->CellLocked = Lock;
   SS_UnlockCellItem(lpSS, CellCol, CellRow);
   fRet = TRUE;

   if (lpSS->LockColor.ForegroundId || lpSS->LockColor.BackgroundId)
      SS_InvalidateCell(lpSS, CellCol, CellRow);
   }

return (fRet);
}


BOOL SS_SetCellData(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow,
                    LPTSTR Data, short Len)
{
SS_COORD      DataColCntOld;
SS_COORD      DataRowCntOld;
BOOL          fRet = FALSE;

DataColCntOld = lpSS->Col.DataCnt;
DataRowCntOld = lpSS->Row.DataCnt;

if (fRet = SSxx_SetCellData(lpSS, CellCol, CellRow, Data, Len))
   {
   if (CellCol >= DataColCntOld)
      SS_SetHScrollBar(lpSS);

   if (CellRow >= DataRowCntOld)
      SS_SetVScrollBar(lpSS);

	/* RFW - 9/9/05 - 16925
   if (!lpSS->lpBook->EditModeTurningOff || CellCol != lpSS->Col.CurAt ||
       CellRow != lpSS->Row.CurAt || lpSS->fFormulaMode)
	*/
   if (!lpSS->lpBook->EditModeTurningOff || CellCol != lpSS->Col.CurAt ||
       CellRow != lpSS->Row.CurAt || lpSS->lpBook->nActiveSheet != lpSS->nSheetIndex ||
       lpSS->fFormulaMode)
      if (lpSS->lpBook->hWnd)
         SS_SendMsgCoords(lpSS, SSM_DATACHANGE,
                          GetDlgCtrlID(lpSS->lpBook->hWnd),
                          CellCol, CellRow);
   }

return (fRet);
}


BOOL SSx_SetCellData(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow,
                     LPTSTR Data, short Len, BOOL fFireChangeEvent)
{
SS_COORD      DataColCntOld;
SS_COORD      DataRowCntOld;
BOOL          fRet = FALSE;

DataColCntOld = lpSS->Col.DataCnt;
DataRowCntOld = lpSS->Row.DataCnt;

if (fRet = SSxx_SetCellData(lpSS, CellCol, CellRow, Data, Len))
   {
   if (CellCol >= DataColCntOld)
      SS_SetHScrollBar(lpSS);

   if (CellRow >= DataRowCntOld)
      SS_SetVScrollBar(lpSS);

   if (!lpSS->lpBook->EditModeTurningOff || CellCol != lpSS->Col.CurAt ||
       CellRow != lpSS->Row.CurAt || lpSS->fFormulaMode)
      if (lpSS->lpBook->hWnd && fFireChangeEvent)
         SS_SendMsgCoords(lpSS, SSM_DATACHANGE,
                          GetDlgCtrlID(lpSS->lpBook->hWnd),
                          CellCol, CellRow);
   }

return (fRet);
}


BOOL SSxx_SetCellData(lpSS, CellCol, CellRow, Data, Len)

LPSPREADSHEET lpSS;
SS_COORD      CellCol;
SS_COORD      CellRow;
LPTSTR        Data;
short         Len;
{
LPSS_CELLTYPE lpCellType = NULL;
SS_CELLTYPE   CellType;
LPSS_CELL     lpCell;
LPTSTR        lpDataTemp;
short         LenTemp;
BOOL          fRet = FALSE;

if (lpCell = SS_AllocLockCell(lpSS, NULL, CellCol, CellRow))
   {
   if (lpCell->Data.bDataType || Len)
      {
      if (lpCell->Data.bDataType == SS_TYPE_EDIT && lpCell->Data.Data.hszData)
         {
         lpDataTemp = (LPTSTR)tbGlobalLock(lpCell->Data.Data.hszData);

         if (lstrlen(lpDataTemp) == Len && StrnCmp(lpDataTemp, Data, Len) == 0)
            fRet = TRUE;

         tbGlobalUnlock(lpCell->Data.Data.hszData);

         if (fRet)
            return (TRUE);
         }

      SSx_FreeData(&lpCell->Data);
      SS_ResetCellOverflow(lpSS, CellCol, CellRow);

      if (Len)
         {
         lpCellType = SS_RetrieveCellType(lpSS, &CellType, lpCell,
                                          CellCol, CellRow);

#ifndef SS_NOOVERFLOW
         lpCell->Data.bOverflow = 0;
#endif

         if (SS_IsCellTypeFloatNum(&CellType))
            {
            lpCell->Data.bDataType = SS_TYPE_FLOAT;
            SS_StringToNum(lpSS, &CellType, Data, &lpCell->Data.Data.dfValue);
            }

         else
            {
            lpCell->Data.bDataType = SS_TYPE_EDIT;
            #ifndef SS_NOCT_COMBO
            if (CellType.Type == SS_TYPE_COMBOBOX)
               {
               lpCell->Data.Data.hszData = SS_FormatComboBox(lpSS, &CellType, Data, NULL, FALSE, FALSE);
               }
            else
            #endif
               {
               LenTemp = Len + 1;
               if (!(lpCell->Data.Data.hszData = tbGlobalAlloc(GMEM_MOVEABLE |
                                                               GMEM_ZEROINIT,
                                                               LenTemp * sizeof(TCHAR))))
                  return (FALSE);
               lpDataTemp = (LPTSTR)tbGlobalLock(lpCell->Data.Data.hszData);
               _fmemcpy(lpDataTemp, Data, Len * sizeof(TCHAR));
               tbGlobalUnlock(lpCell->Data.Data.hszData);
               }
            }
         }

      if (lpCell->Data.bDataType && CellCol >= lpSS->Col.DataCnt)
         SS_SetDataColCnt(lpSS, CellCol + 1);

      if (lpCell->Data.bDataType && CellRow >= lpSS->Row.DataCnt)
         SS_SetDataRowCnt(lpSS, CellRow + 1);

      if (!lpCell->Data.bDataType)
         {
         SS_AdjustDataColCnt(lpSS, CellCol, CellCol);
         SS_AdjustDataRowCnt(lpSS, CellRow, CellRow);
         }
      }

   SS_CalcCellOverflow(lpSS, lpCell, lpCellType, CellCol, CellRow);

   if (!lpCell->Data.bDataType)
      SS_OverflowAdjustNeighbor(lpSS, CellCol, CellRow);

   SS_UnlockCellItem(lpSS, CellCol, CellRow);
   fRet = TRUE;
   }

return (fRet);
}


BOOL SS_SetCellDataItem(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow,
                        LPSS_DATA lpDataItem)
{
return (SSx_SetCellDataItem(lpSS, CellCol, CellRow, lpDataItem, TRUE));
}


BOOL SSx_SetCellDataItem(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow,
                         LPSS_DATA lpDataItem, BOOL fFireChangeEvent)

{
SS_COORD DataColCntOld;
SS_COORD DataRowCntOld;
BOOL     fRet = FALSE;

DataColCntOld = lpSS->Col.DataCnt;
DataRowCntOld = lpSS->Row.DataCnt;

if (fRet = SSxx_SetCellDataItem(lpSS, CellCol, CellRow, lpDataItem))
   {
   if (CellCol >= DataColCntOld)
      SS_SetHScrollBar(lpSS);

   if (CellRow >= DataRowCntOld)
      SS_SetVScrollBar(lpSS);

   if (!lpSS->lpBook->EditModeTurningOff || CellCol != lpSS->Col.CurAt ||
       CellRow != lpSS->Row.CurAt || lpSS->fFormulaMode)
      if (lpSS->lpBook->hWnd && fFireChangeEvent)
         SS_SendMsgCoords(lpSS, SSM_DATACHANGE,
                          GetDlgCtrlID(lpSS->lpBook->hWnd),
                          CellCol, CellRow);
   }

return (fRet);
}


BOOL SSxx_SetCellDataItem(lpSS, CellCol, CellRow, lpDataItem)

LPSPREADSHEET lpSS;
SS_COORD      CellCol;
SS_COORD      CellRow;
LPSS_DATA     lpDataItem;
{
LPSS_CELL     lpCell;
BOOL          fRet = FALSE;

if (lpCell = SS_AllocLockCell(lpSS, NULL, CellCol, CellRow))
   {
   if (lpCell->Data.bDataType || lpDataItem->bDataType)
      {
      SSx_FreeData(&lpCell->Data);
      SS_ResetCellOverflow(lpSS, CellCol, CellRow);
      _fmemcpy(&lpCell->Data, lpDataItem, sizeof(SS_DATA));

      if (lpCell->Data.bDataType && CellCol >= lpSS->Col.DataCnt)
         SS_SetDataColCnt(lpSS, CellCol + 1);

      if (lpCell->Data.bDataType && CellRow >= lpSS->Row.DataCnt)
         SS_SetDataRowCnt(lpSS, CellRow + 1);

      if (!lpCell->Data.bDataType)
         {
         SS_AdjustDataColCnt(lpSS, CellCol, CellCol);
         SS_AdjustDataRowCnt(lpSS, CellRow, CellRow);
         }

      SS_CalcCellOverflow(lpSS, lpCell, NULL, CellCol, CellRow);

      if (!lpCell->Data.bDataType)
         SS_OverflowAdjustNeighbor(lpSS, CellCol, CellRow);
      }

   fRet = TRUE;
   SS_UnlockCellItem(lpSS, CellCol, CellRow);
   }

return (fRet);
}


BOOL SSIsCellVisible(hWnd, CellCol, CellRow, Visible)

HWND          hWnd;
SS_COORD      CellCol;
SS_COORD      CellRow;
short         Visible;
{
LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &CellCol, &CellRow);
fRet = SS_IsCellVisible(lpSS, CellCol, CellRow, Visible);

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_IsCellVisible(LPSPREADSHEET lpSS, SS_COORD CellCol,
                      SS_COORD CellRow, short Visible)
{
BOOL fRet = FALSE;

if (SS_IsRowVisible(lpSS, CellRow, Visible) &&
    SS_IsColVisible(lpSS, CellCol, Visible))
   fRet = TRUE;

return (fRet);
}


void SS_AdjustCellCoords(lpSS, lpCellCol, lpCellRow)

LPSPREADSHEET lpSS;
LPSS_COORD    lpCellCol;
LPSS_COORD    lpCellRow;
{
if (lpCellRow && *lpCellRow != SS_ALLROWS)
   {
   if (*lpCellRow < 0)
      *lpCellRow += -SS_HEADER;
   else if (*lpCellRow == 0)
      *lpCellRow = 0;
	else
      *lpCellRow += lpSS->Row.HeaderCnt - 1;

//   *lpCellRow = min(*lpCellRow, SS_GetRowCnt(lpSS) - 1);
   }

if (lpCellCol && *lpCellCol != SS_ALLROWS)
   {
   if (*lpCellCol < 0)
      *lpCellCol += -SS_HEADER;
   else if (*lpCellCol == 0)
      *lpCellCol = 0;
   else
      *lpCellCol += lpSS->Col.HeaderCnt - 1;

//   *lpCellCol = min(*lpCellCol, SS_GetColCnt(lpSS) - 1);
   }
}


void SS_AdjustCellRangeCoords(LPSPREADSHEET lpSS,
                              LPSS_COORD lpCellCol, LPSS_COORD lpCellRow,
                              LPSS_COORD lpCellCol2, LPSS_COORD lpCellRow2)
{
if (lpCellRow)
   {
   if (*lpCellRow == -1)
      {
      if (lpCellRow2)
         *lpCellRow2 = -1;
      }
   else
      {
      if (*lpCellRow < 0)
			{
         *lpCellRow += -SS_HEADER;
			*lpCellRow = max(0, *lpCellRow);
			*lpCellRow = min(lpSS->Row.HeaderCnt - 1, *lpCellRow);
			}
		else if (*lpCellRow == 0)
			*lpCellRow = 0;
      else
         *lpCellRow += lpSS->Row.HeaderCnt - 1;

      *lpCellRow = min(*lpCellRow, SS_GetRowCnt(lpSS) - 1);

      if (lpCellRow2)
         {
         if (*lpCellRow2 != -1)
            {
            if (*lpCellRow2 < 0)
					{
					*lpCellRow2 += -SS_HEADER;
					*lpCellRow2 = max(0, *lpCellRow2);
					*lpCellRow2 = min(lpSS->Row.HeaderCnt - 1, *lpCellRow2);
					}
				else if (*lpCellRow2 == 0)
					*lpCellRow2 = 0;
            else
               *lpCellRow2 += lpSS->Row.HeaderCnt - 1;

            *lpCellRow2 = min(*lpCellRow2, SS_GetRowCnt(lpSS) - 1);
            }
//         else
//            *lpCellRow = -1;
         }
      }
   }

if (lpCellCol)
   {
   if (*lpCellCol == -1)
      {
      if (lpCellCol2)
         *lpCellCol2 = -1;
      }
   else
      {
      if (*lpCellCol < 0)
			{
         *lpCellCol += -SS_HEADER;
			*lpCellCol = max(0, *lpCellCol);
			*lpCellCol = min(lpSS->Col.HeaderCnt - 1, *lpCellCol);
			}
		else if (*lpCellCol == 0)
			*lpCellCol = 0;
      else
         *lpCellCol += lpSS->Col.HeaderCnt - 1;

      *lpCellCol = min(*lpCellCol, SS_GetColCnt(lpSS) - 1);

      if (lpCellCol2)
         {
         if (*lpCellCol2 != -1)
            {
            if (*lpCellCol2 < 0)
					{
					*lpCellCol2 += -SS_HEADER;
					*lpCellCol2 = max(0, *lpCellCol2);
					*lpCellCol2 = min(lpSS->Col.HeaderCnt - 1, *lpCellCol2);
					}
				else if (*lpCellCol2 == 0)
					*lpCellCol2 = 0;
            else
               *lpCellCol2 += lpSS->Col.HeaderCnt - 1;

            *lpCellCol2 = min(*lpCellCol2, SS_GetColCnt(lpSS) - 1);
            }
//         else
//            *lpCellCol = -1;
         }
      }
   }
}


void SS_AdjustCellCoordsOut(lpSS, lpCellCol, lpCellRow)

LPSPREADSHEET lpSS;
LPSS_COORD    lpCellCol;
LPSS_COORD    lpCellRow;
{
if (lpCellRow && *lpCellRow != SS_ALLROWS)
   {
   if (*lpCellRow < lpSS->Row.HeaderCnt)
		{
#ifdef SS_V40
		if (*lpCellRow > 0)
			*lpCellRow += SS_HEADER;
#endif
      }
   else
      *lpCellRow -= lpSS->Row.HeaderCnt - 1;
   }

if (lpCellCol && *lpCellCol != SS_ALLROWS)
   {
   if (*lpCellCol < lpSS->Col.HeaderCnt)
		{
#ifdef SS_V40
		if (*lpCellCol > 0)
	      *lpCellCol += SS_HEADER;
#endif
      }
   else
      *lpCellCol -= lpSS->Col.HeaderCnt - 1;
   }
}


void SS_LimitRangeCoords(LPSPREADSHEET lpSS,
                         LPSS_COORD lplCol, LPSS_COORD lplRow,
                         LPSS_COORD lplCol2, LPSS_COORD lplRow2)
{
if (SS_ALLCOLS == *lplCol || SS_ALLCOLS == *lplCol2)
   {
   *lplCol = *lplCol2 = SS_ALLCOLS;
   }
else
   {
   *lplCol = min(*lplCol, SS_GetColCnt(lpSS) - 1);
   *lplCol2 = min(*lplCol2, SS_GetColCnt(lpSS) - 1);
   }
if (SS_ALLROWS == *lplRow || SS_ALLROWS == *lplRow2)
   {
   *lplRow = *lplRow2 = SS_ALLROWS;
   }
else
   {
   *lplRow = min(*lplRow, SS_GetRowCnt(lpSS) - 1);
   *lplRow2 = min(*lplRow2, SS_GetRowCnt(lpSS) - 1);
   }
}


BOOL SS_ClearCell(lpSS, CellCol, CellRow, Paint, fSendChangeMsg)

LPSPREADSHEET lpSS;
SS_COORD      CellCol;
SS_COORD      CellRow;
BOOL          Paint;
BOOL          fSendChangeMsg;
{
if (CellCol >= SS_GetColCnt(lpSS) ||
    CellRow >= SS_GetRowCnt(lpSS))
   {
   return (FALSE);
   }

// RFW - 11/21/05 - 17646
if (CellRow >= lpSS->Row.AllocCnt || CellCol >= lpSS->Col.AllocCnt)
   return (FALSE);

SS_DeAllocCell(lpSS, CellCol, CellRow, fSendChangeMsg);
SS_OverflowAdjustNeighbor(lpSS, CellCol, CellRow);  // RFW - 8/2/04 - 14665

if (Paint)
   SS_InvalidateCell(lpSS, CellCol, CellRow);

return (TRUE);
}


BOOL SS_ClearCellData(lpSS, CellCol, CellRow)

LPSPREADSHEET lpSS;
SS_COORD      CellCol;
SS_COORD      CellRow;
{
LPSS_CELL     lpCell;
BOOL          fRet = FALSE;

if (lpCell = SS_LockCellItem(lpSS, CellCol, CellRow))
   {
   SSx_FreeData(&lpCell->Data);
   SS_ResetCellOverflow(lpSS, CellCol, CellRow);

   SS_UnlockCellItem(lpSS, CellCol, CellRow);

   SS_InvalidateCell(lpSS, CellCol, CellRow);

   fRet = TRUE;
   }

return (fRet);
}


BOOL SS_SetCellFont(lpSS, CellCol, CellRow, hFont, DeleteFont,
                    lpfDeleteFontObject)

LPSPREADSHEET lpSS;
SS_COORD      CellCol;
SS_COORD      CellRow;
HFONT         hFont;
BOOL          DeleteFont;
LPBOOL        lpfDeleteFontObject;
{
LPSS_CELL     lpCell;
BOOL          fRet = FALSE;

if (CellCol < SS_GetColCnt(lpSS) && CellRow < SS_GetRowCnt(lpSS))
   if (lpCell = SS_AllocLockCell(lpSS, NULL, CellCol, CellRow))
      {
		LPSS_FONT lpFontTable;
		LPSS_ROW  lpRow;
		short     dFontHeight = 0;
		short     dFontHeightCell = 0;
		short     dFontHeightCellOld = 0;
      BOOL      fRowHeightChanged;
      BOOL      fTurnEditModeOn = FALSE;

      if (lpSS->lpBook->EditModeOn && !lpSS->lpBook->fLeaveEditModeOn &&
        CellCol == lpSS->Col.CurAt && CellRow == lpSS->Row.CurAt)
         {
         WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

         lpSS->lpBook->wMessageBeingSent = FALSE;
         SS_CellEditModeOff(lpSS, 0);
         lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;

         fTurnEditModeOn = TRUE;
         }

		lpFontTable = SS_FontTableLock();

		if (lpCell->FontId > 0)
			dFontHeightCellOld = lpFontTable[lpCell->FontId - 1].FontHeight;

      lpCell->FontId = SS_InitFont(lpSS->lpBook, hFont, DeleteFont, TRUE, lpfDeleteFontObject);

		if (lpCell->FontId > 0)
			dFontHeightCell = lpFontTable[lpCell->FontId - 1].FontHeight;

	   lpRow = SS_LockRowItem(lpSS, CellRow);

		if (lpRow->FontId > 0)
			dFontHeight = lpFontTable[lpRow->FontId - 1].FontHeight;

		if (dFontHeightCell > dFontHeight ||
          (dFontHeightCellOld == dFontHeight && dFontHeightCell < dFontHeightCellOld))
			fRowHeightChanged = SS_SetRowMaxFont(lpSS, lpRow, CellRow, lpCell->FontId);

	   SS_UnlockRowItem(lpSS, CellRow);
		SS_FontTableUnlock();

      SS_OverflowAdjustNeighbor(lpSS, CellCol, CellRow);

      if (SS_IsRowVisible(lpSS, CellRow, SS_VISIBLE_PARTIAL))
         {
         if (fRowHeightChanged)
            SS_InvalidateRowRange(lpSS, CellRow, -1);
         else
            SS_InvalidateCell(lpSS, CellCol, CellRow);
         }

      if (fTurnEditModeOn)
         {
         SS_UpdateWindow(lpSS->lpBook);
         SS_CellEditModeOn(lpSS, 0, 0, 0L);
         }

      SS_UnlockCellItem(lpSS, CellCol, CellRow);
      fRet = TRUE;
      }

return (fRet);
}

#if SS_V80
BOOL SS_SetCellLogFont(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, LOGFONT *pLogFont)
{
LPSS_CELL     lpCell;
BOOL          fRet = FALSE;

if (CellCol < SS_GetColCnt(lpSS) && CellRow < SS_GetRowCnt(lpSS))
   if (lpCell = SS_AllocLockCell(lpSS, NULL, CellCol, CellRow))
      {
		LPSS_FONT lpFontTable;
		LPSS_ROW  lpRow;
		short     dFontHeight = 0;
		short     dFontHeightCell = 0;
		short     dFontHeightCellOld = 0;
      BOOL      fRowHeightChanged;
      BOOL      fTurnEditModeOn = FALSE;

      if (lpSS->lpBook->EditModeOn && !lpSS->lpBook->fLeaveEditModeOn &&
        CellCol == lpSS->Col.CurAt && CellRow == lpSS->Row.CurAt)
         {
         WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

         lpSS->lpBook->wMessageBeingSent = FALSE;
         SS_CellEditModeOff(lpSS, 0);
         lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;

         fTurnEditModeOn = TRUE;
         }

		lpFontTable = SS_FontTableLock();

		if (lpCell->FontId > 0)
			dFontHeightCellOld = lpFontTable[lpCell->FontId - 1].FontHeight;

      lpCell->FontId = SS_InitFontLogFont(lpSS->lpBook, pLogFont);

		if (lpCell->FontId > 0)
			dFontHeightCell = lpFontTable[lpCell->FontId - 1].FontHeight;

	   lpRow = SS_LockRowItem(lpSS, CellRow);

		if (lpRow->FontId > 0)
			dFontHeight = lpFontTable[lpRow->FontId - 1].FontHeight;

		if (dFontHeightCell > dFontHeight ||
          (dFontHeightCellOld == dFontHeight && dFontHeightCell < dFontHeightCellOld))
			fRowHeightChanged = SS_SetRowMaxFont(lpSS, lpRow, CellRow, lpCell->FontId);

	   SS_UnlockRowItem(lpSS, CellRow);
		SS_FontTableUnlock();

      SS_OverflowAdjustNeighbor(lpSS, CellCol, CellRow);

      if (SS_IsRowVisible(lpSS, CellRow, SS_VISIBLE_PARTIAL))
         {
         if (fRowHeightChanged)
            SS_InvalidateRowRange(lpSS, CellRow, -1);
         else
            SS_InvalidateCell(lpSS, CellCol, CellRow);
         }

      if (fTurnEditModeOn)
         {
         SS_UpdateWindow(lpSS->lpBook);
         SS_CellEditModeOn(lpSS, 0, 0, 0L);
         }

      SS_UnlockCellItem(lpSS, CellCol, CellRow);
      fRet = TRUE;
      }

return (fRet);
}
#endif

BOOL SS_SetCellCellType(LPSPREADSHEET lpSS,
                        SS_COORD CellCol, SS_COORD CellRow,
                        LPSS_CELLTYPE CellType)
{
LPSS_CELL      lpCell;
LPSS_CELLTYPE  CellTypeTemp;
SS_CELLTYPE    CellTypeOld;
TBGLOBALHANDLE hValue = 0;
TBGLOBALHANDLE hData;
LPTSTR         lpData;
BOOL           fValue = FALSE;
BOOL           fSetData = SS_VALUE_TEXT;
BOOL           fTurnEditModeOn = FALSE;
BOOL           fRedrawOld = FALSE;

if (lpCell = SS_AllocLockCell(lpSS, NULL, CellCol, CellRow))
   {
   if (lpSS->lpBook->EditModeOn && !lpSS->lpBook->fLeaveEditModeOn &&
		 CellCol == lpSS->Col.CurAt && CellRow == lpSS->Row.CurAt)
      {
      WORD wMessageBeingSent = lpSS->lpBook->wMessageBeingSent;

      lpSS->lpBook->wMessageBeingSent = FALSE;
      SS_CellEditModeOff(lpSS, 0);
      lpSS->lpBook->wMessageBeingSent = wMessageBeingSent;

      fTurnEditModeOn = TRUE;
      }

   SS_RetrieveCellType(lpSS, &CellTypeOld, lpCell, CellCol, CellRow);

	// RFW - 7/31/06 - 19221

   if( SS_BACKCOLORSTYLE_UNDERGRID == lpSS->lpBook->bBackColorStyle &&
	    (CellTypeOld.Type == SS_TYPE_BUTTON || CellTypeOld.Type == SS_TYPE_COMBOBOX))
		{
	   fRedrawOld = lpSS->lpBook->Redraw;
	   lpSS->lpBook->Redraw = FALSE;
      SS_InvalidateCellRange(lpSS, CellCol, CellRow, CellCol, CellRow);
		}

   SS_DeAllocCellType(lpSS, lpCell->hCellType);
   lpCell->hCellType = 0;

   if (CellType)
      {
      if (!(lpCell->hCellType = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                              (long)sizeof(SS_CELLTYPE))))
         return (FALSE);

      if ((lpCell->Data.bDataType == SS_TYPE_FLOAT &&
           !SS_IsCellTypeFloatNum(CellType)) ||
          (lpCell->Data.bDataType != SS_TYPE_FLOAT &&
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

      CellTypeTemp = (LPSS_CELLTYPE)tbGlobalLock(lpCell->hCellType);
      _fmemcpy(CellTypeTemp, CellType, sizeof(SS_CELLTYPE));
#if SS_V80
      SS_CT_Ref(lpSS, CellTypeTemp);
#endif
      tbGlobalUnlock(lpCell->hCellType);
      }

   if (fSetData)
      {
      if (hData = SS_GetData(lpSS, &CellTypeOld, CellCol, CellRow, fValue))
         {
         lpData = (LPTSTR)tbGlobalLock(hData);
         if (SS_VALUE_VALUE == fValue)
         {
           if (hValue = SS_UnFormatData(lpSS, CellCol, CellRow, &CellTypeOld, lpData))
              lpData = (LPTSTR)tbGlobalLock(hValue);
           else
              lpData = NULL;
         }
         SS_SetDataRange(lpSS, CellCol, CellRow, CellCol, CellRow, lpData,
                         fValue, FALSE, FALSE);
         if (SS_VALUE_VALUE == fValue && hValue)
         {
           tbGlobalUnlock(hValue);
           tbGlobalFree(hValue);
         }
         tbGlobalUnlock(hData);
         }
      else
         {
         // BJO 18Oct96 ARR748 - Before fix
         //lpCell->Data.bDataType = SS_TYPE_EDIT;
         // BJO 18Oct96 ARR748 - Begin fix
         lpCell->Data.bDataType = 0;
         // BJO 18Oct96 ARR748 - End fix
         }
      }

   if (lpSS->lpBook->fAllowCellOverflow)
      {
      SS_ResetCellOverflow(lpSS, CellCol, CellRow);
      SS_CalcCellOverflow(lpSS, lpCell, CellType, CellCol, CellRow);
      SS_OverflowAdjustNeighbor(lpSS, CellCol, CellRow);
      }

	if (!lpSS->lpBook->fLeaveEditModeOn)
      SS_InvalidateCellRange(lpSS, CellCol, CellRow, CellCol, CellRow);
		/* RFW - 7/1/04 - 14593
		SS_InvalidateCell(lpSS, CellCol, CellRow);
		*/

	// RFW - 7/31/06 - 19221
	if (fRedrawOld)
		SS_BookSetRedraw(lpSS->lpBook, fRedrawOld);

   if (fTurnEditModeOn)
      {
      SS_UpdateWindow(lpSS->lpBook);
      SS_CellEditModeOn(lpSS, 0, 0, 0L);
      }

   SS_UnlockCellItem(lpSS, CellCol, CellRow);
   }

return (TRUE);
}


BOOL SS_SetCellColor(lpSS, CellCol, CellRow, Background, Foreground)

LPSPREADSHEET lpSS;
SS_COORD      CellCol;
SS_COORD      CellRow;
COLORREF      Background;
COLORREF      Foreground;
{
LPSS_CELL     lpCell;
BOOL          fRet = FALSE;

if (lpCell = SS_AllocLockCell(lpSS, NULL, CellCol, CellRow))
   {
//   if (!SS_ISDEFCOLOR(Background))
   if (Background != SPREAD_COLOR_IGNORE)
      lpCell->Color.BackgroundId = SS_AddColor(Background);

//   if (!SS_ISDEFCOLOR(Foreground))
   if (Foreground != SPREAD_COLOR_IGNORE)
      lpCell->Color.ForegroundId = SS_AddColor(Foreground);

   SS_UnlockCellItem(lpSS, CellCol, CellRow);
   // RFW - 3/18/00 - GRB9138
   if (!lpSS->lpBook->fAllowCellOverflow ||
       !SS_OverflowInvalidateCell(lpSS, CellCol, CellRow))
      SS_InvalidateCell(lpSS, CellCol, CellRow);

   fRet = TRUE;
   }

return (fRet);
}


BOOL DLLENTRY SSSetSel(hWnd, SelStart, SelEnd)

HWND            hWnd;
int             SelStart;
int             SelEnd;
{
HWND            hWndCtl;
LPSPREADSHEET   lpSS;
SS_CELLTYPE     CellType;
BOOL            fRet = FALSE;

lpSS = SS_SheetLockActive(hWnd);

if (lpSS->lpBook->EditModeOn)
   {
   if (SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.CurAt,
                           lpSS->Row.CurAt) && CellType.ControlID)
      {
      hWndCtl = SS_GetControlhWnd(lpSS, CellType.ControlID);

      switch (CellType.Type)
         {
         case SS_TYPE_DATE:
         case SS_TYPE_EDIT:
         case SS_TYPE_PIC:
         case SS_TYPE_FLOAT:
         case SS_TYPE_INTEGER:
         case SS_TYPE_TIME:
#ifdef SS_V40
         case SS_TYPE_CURRENCY:
         case SS_TYPE_NUMBER:
         case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
         case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
            Edit_SetSel(hWndCtl, SelStart, SelEnd);
#ifdef WIN32
            SendMessage(hWndCtl, EM_SCROLLCARET, 0, 0);
#endif
            fRet = TRUE;
            break;

         case SS_TYPE_COMBOBOX:
            ComboBox_SetEditSel(hWndCtl, SelStart, SelEnd);
            fRet = TRUE;
            break;
         }
      }
   }

SS_SheetUnlockActive(hWnd);
return (fRet);
}


BOOL DLLENTRY SSGetSel(hWnd, SelStart, SelEnd)

HWND            hWnd;
LPINT           SelStart;
LPINT           SelEnd;
{
LPSPREADSHEET   lpSS;
SS_CELLTYPE     CellType;
HWND            hWndCtl;
long            Sel;
BOOL            fRet = FALSE;

if (SelStart)
   *SelStart = 0;
if (SelEnd)
   *SelEnd = 0;

lpSS = SS_SheetLockActive(hWnd);

if (lpSS->lpBook->EditModeOn)
   {
   if (SS_RetrieveCellType(lpSS, &CellType, NULL, lpSS->Col.CurAt,
                           lpSS->Row.CurAt) && CellType.ControlID)
      {
      hWndCtl = SS_GetControlhWnd(lpSS, CellType.ControlID);

      switch (CellType.Type)
         {
         case SS_TYPE_DATE:
         case SS_TYPE_EDIT:
         case SS_TYPE_PIC:
         case SS_TYPE_FLOAT:
         case SS_TYPE_INTEGER:
         case SS_TYPE_TIME:
#ifdef SS_V40
         case SS_TYPE_CURRENCY:
         case SS_TYPE_NUMBER:
         case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
         case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
            Sel = Edit_GetSel(hWndCtl);
            fRet = TRUE;
            break;

         case SS_TYPE_COMBOBOX:
            Sel = ComboBox_GetEditSel(hWndCtl);
            fRet = TRUE;
            break;
         }

      if (fRet)
         {
         if (SelStart)
            *SelStart = LOWORD(Sel);
         if (SelEnd)
            *SelEnd = HIWORD(Sel);
         }
      }
   }

SS_SheetUnlockActive(hWnd);
return (fRet);
}


BOOL DLLENTRY SSReplaceSel(hWnd, lpText)

HWND            hWnd;
LPCTSTR         lpText;
{
LPSPREADSHEET   lpSS;
SS_CELLTYPE     CellType;
BOOL            fRet = FALSE;

lpSS = SS_SheetLockActive(hWnd);

if (lpSS->lpBook->EditModeOn)
   {
   if (SS_RetrieveCellType(lpSS, &CellType, NULL,
                           lpSS->Col.CurAt, lpSS->Row.CurAt) &&
                           CellType.ControlID)
      {
      switch (CellType.Type)
         {
         case SS_TYPE_DATE:
         case SS_TYPE_EDIT:
         case SS_TYPE_PIC:
         case SS_TYPE_FLOAT:
         case SS_TYPE_INTEGER:
         case SS_TYPE_TIME:
#ifdef SS_V40
         case SS_TYPE_CURRENCY:
         case SS_TYPE_NUMBER:
         case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
         case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
            SendMessage(SS_GetControlhWnd(lpSS, CellType.ControlID),
                        EM_REPLACESEL, 0, (LPARAM)lpText);
            fRet = TRUE;
            break;

         case SS_TYPE_COMBOBOX:
            SendMessage(SS_GetControlhWnd(lpSS, CellType.ControlID),
                        SS_CB_REPLACESEL, 0, (LPARAM)lpText);
            fRet = TRUE;
            break;
         }
      }
   }

SS_SheetUnlockActive(hWnd);
return (fRet);
}


GLOBALHANDLE DLLENTRY SSGetSelText(hWnd)

HWND            hWnd;
{
LPSPREADSHEET   lpSS;
SS_CELLTYPE     CellType;
HANDLE          hData = 0;
LPTSTR          Data;
short           TextLen;
int             dStart;
int             dEnd;
short           i;

lpSS = SS_SheetLockActive(hWnd);

if (lpSS->lpBook->EditModeOn)
   {
   if (SSGetSel(hWnd, &dStart, &dEnd) && dStart < dEnd)
      {
      if (SS_RetrieveCellType(lpSS, &CellType, NULL,
                              lpSS->Col.CurAt, lpSS->Row.CurAt) &&
                              CellType.ControlID)
         {
         TextLen = (short)SendMessage(
                          SS_GetControlhWnd(lpSS, CellType.ControlID),
                          WM_GETTEXTLENGTH, 0, 0L);

         if (TextLen)
            {
            if (hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                    (TextLen + 1) * sizeof(TCHAR)))
               {
               Data = GlobalLock(hData);

               SendMessage(SS_GetControlhWnd(lpSS, CellType.ControlID),
                           WM_GETTEXT, TextLen + 1, (LPARAM)Data);

               for (i = 0; i < min(lstrlen(Data), dEnd) - dStart; i++)
                  Data[i] = Data[dStart + i];

               Data[i] = '\0';
               GlobalUnlock(hData);
               }
            }
         }
      }
   }

SS_SheetUnlockActive(hWnd);
return (hData);
}


TBGLOBALHANDLE SS_GetData(lpSS, lpCellType, Col, Row, fValue)

LPSPREADSHEET  lpSS;
LPSS_CELLTYPE  lpCellType;
SS_COORD       Col;
SS_COORD       Row;
BOOL           fValue;
{
LPSS_ROW       lpRow;
TBGLOBALHANDLE hData;

lpRow = SS_LockRowItem(lpSS, Row);
hData = SSx_GetData(lpSS, NULL, lpRow, NULL, lpCellType, NULL, Col, Row,
                    fValue);
SS_UnlockRowItem(lpSS, Row);

return (hData);
}


TBGLOBALHANDLE SSx_GetData(lpSS, lpCol, lpRow, lpCell, lpCellType,
                           lpData, Col, Row, fValue)

LPSPREADSHEET  lpSS;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
LPSS_CELL      lpCell;
LPSS_CELLTYPE  lpCellType;
LPSS_DATA      lpData;
SS_COORD       Col;
SS_COORD       Row;
BOOL           fValue;
{
SS_CELLTYPE    CellType;
SS_DATA        Data;
TBGLOBALHANDLE hData = 0;
LPSS_CELL      lpCellOrig;
LPSS_COL       lpColOrig;
LPTSTR         lpszBuffer;

lpCellOrig = lpCell;
lpColOrig = lpCol;

/* RFW - 6/1/04 - 14257
if (Row != SS_ALLROWS && Col != SS_ALLCOLS &&
    (lpCell || (lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row))) &&
    lpCell->Data.bDataType)
   _fmemcpy(&Data, &lpCell->Data, sizeof(SS_DATA));

else if (Row != SS_ALLROWS && lpRow && lpRow->Data.bDataType)
   _fmemcpy(&Data, &lpRow->Data, sizeof(SS_DATA));

else if (Col != SS_ALLCOLS && (lpCol ||
         (lpCol = SS_LockColItem(lpSS, Col))) && lpCol->Data.bDataType)
   _fmemcpy(&Data, &lpCol->Data, sizeof(SS_DATA));

else
   _fmemcpy(&Data, &lpSS->DefaultData, sizeof(SS_DATA));
*/

if (Row != SS_ALLROWS && Col != SS_ALLCOLS &&
    (lpCell || (lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row))) &&
    lpCell->Data.bDataType)
   _fmemcpy(&Data, &lpCell->Data, sizeof(SS_DATA));

else if (Row != SS_ALLROWS && lpRow && lpRow->Data.bDataType && (Col == -1 || Col >= lpSS->Col.HeaderCnt))
   _fmemcpy(&Data, &lpRow->Data, sizeof(SS_DATA));

else if (Col != SS_ALLCOLS && (lpCol ||
         (lpCol = SS_LockColItem(lpSS, Col))) && lpCol->Data.bDataType &&
         (Row == -1 || Row >= lpSS->Row.HeaderCnt))
   _fmemcpy(&Data, &lpCol->Data, sizeof(SS_DATA));

else
   _fmemcpy(&Data, &lpSS->DefaultData, sizeof(SS_DATA));

if (!lpCellOrig && lpCell)
   SSx_UnlockCellItem(lpSS, lpRow, Col, Row);

if (!lpColOrig && lpCol)
   SS_UnlockColItem(lpSS, Col);

if (lpData && Data.bDataType == SS_TYPE_FLOAT)
   _fmemcpy(lpData, &Data, sizeof(SS_DATA));

else if (Data.bDataType)
   switch (Data.bDataType)
      {
      case SS_TYPE_EDIT:
         hData = Data.Data.hszData;
         break;

      case SS_TYPE_FLOAT:
         lpszBuffer = (LPTSTR)tbGlobalLock(lpSS->lpBook->hBuffer);

         if (!lpCellType)
            {
            SS_RetrieveCellType(lpSS, &CellType, lpCell, Col, Row);
            lpCellType = &CellType;
            }

         SS_FloatFormat(lpSS, lpCellType, Data.Data.dfValue,
                        lpszBuffer, fValue);
         tbGlobalUnlock(lpSS->lpBook->hBuffer);
         hData = lpSS->lpBook->hBuffer;
         break;

      case SS_TYPE_INTEGER:
         lpszBuffer = (LPTSTR)tbGlobalLock(lpSS->lpBook->hBuffer);
         wsprintf(lpszBuffer, _T("%ld"), Data.Data.lValue);
         tbGlobalUnlock(lpSS->lpBook->hBuffer);
         hData = lpSS->lpBook->hBuffer;
         break;

      }

return (hData);
}


BOOL SS_GetDataStruct(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow, LPSS_CELL lpCell,
                      LPSS_DATA lpData, SS_COORD Col, SS_COORD Row)
{
LPSS_CELL lpCellOrig;
LPSS_COL lpColOrig;
LPSS_ROW lpRowOrig;

lpCellOrig = lpCell;
lpColOrig = lpCol;
lpRowOrig = lpRow;

if (Row != SS_ALLROWS && Col != SS_ALLCOLS &&
    (lpCell || (lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row))) &&
    lpCell->Data.bDataType)
   _fmemcpy(lpData, &lpCell->Data, sizeof(SS_DATA));

else if (Row != SS_ALLROWS && (lpRow || (lpRow = SS_LockRowItem(lpSS, Row))) && lpRow->Data.bDataType)
   _fmemcpy(lpData, &lpRow->Data, sizeof(SS_DATA));

else if (Col != SS_ALLCOLS && (lpCol || (lpCol = SS_LockColItem(lpSS, Col))) && lpCol->Data.bDataType)
   _fmemcpy(lpData, &lpCol->Data, sizeof(SS_DATA));

else
   _fmemcpy(lpData, &lpSS->DefaultData, sizeof(SS_DATA));

if (lpCell && !lpCellOrig)
   SS_UnlockCellItem(lpSS, Col, Row);
if( lpCol && !lpColOrig )
  SS_UnlockColItem(lpSS, Col);
if( lpRow && !lpRowOrig )
  SS_UnlockRowItem(lpSS, Row);
return (lpData->bDataType);
}


BOOL SS_IsCellDataEmpty(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow, LPSS_CELL lpCell,
                        SS_COORD Col, SS_COORD Row)
{
SS_DATA Data;

return (!SS_GetDataStruct(lpSS, lpCol, lpRow, lpCell, &Data, Col, Row));
}


#ifndef SS_NOMAXTEXT

BOOL DLLENTRY SSGetMaxTextCellSize(HWND hWnd, SS_COORD Col, SS_COORD Row,
                                   LPDOUBLE lpdfWidth, LPDOUBLE lpdfHeight)
{
LPSPREADSHEET lpSS;
BOOL       fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
fRet = SS_GetMaxTextCellSize(lpSS, Col, Row, lpdfWidth, lpdfHeight);
SS_SheetUnlock(hWnd);
return fRet;
}


BOOL SS_GetMaxTextCellSize(LPSPREADSHEET lpSS, SS_COORD Col,
                           SS_COORD Row, LPDOUBLE lpdfWidth, LPDOUBLE lpdfHeight)
{
LPSS_COL lpCol;
LPSS_ROW lpRow;
HDC      hDC;
int      iWidth;
int      iHeight;
BOOL     fRet = FALSE;

if (lpdfWidth)
   *lpdfWidth = 0.0;

if (lpdfHeight)
   *lpdfHeight = 0.0;

hDC = SS_GetDC(lpSS->lpBook);

if (Row < SS_GetRowCnt(lpSS) && Col < SS_GetColCnt(lpSS))
   {
   lpRow = SS_LockRowItem(lpSS, Row);
   lpCol = SS_LockColItem(lpSS, Col);

   SS_CalcCellMetrics(hDC, lpSS, lpCol, lpRow, NULL, NULL,
                      NULL, Col, Row, TRUE, TRUE, NULL, NULL, &iWidth, &iHeight);

   SS_UnlockColItem(lpSS, Col);
   SS_UnlockRowItem(lpSS, Row);

   if (lpdfWidth && iWidth)
      SS_CalcPixelsToColWidth(lpSS, Col, iWidth + 1, lpdfWidth);

   if (lpdfHeight && iHeight)
      SS_CalcPixelsToRowHeight(lpSS, Row, iHeight, lpdfHeight);

   fRet = TRUE;
   }

SS_ReleaseDC(lpSS->lpBook, hDC);

return (fRet);
}

#ifdef SS_V35

short SS_GetCellNote(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, LPTSTR Note)
{
LPSS_CELL lpCell = SS_LockCellItem(lpSS, CellCol, CellRow);
LPSS_COL  lpCol = SS_LockColItem(lpSS, CellCol);
LPSS_ROW  lpRow = SS_LockRowItem(lpSS, CellRow);
TBGLOBALHANDLE hCellNote = 0;
BOOL      nRet = 0;

if (lpCell && lpCell->hCellNote)
   hCellNote = lpCell->hCellNote;
else if (lpCol && lpCol->hCellNote)
   hCellNote = lpCol->hCellNote;   
else if (lpRow && lpRow->hCellNote)
   hCellNote = lpRow->hCellNote;   
else
   hCellNote = lpSS->hCellNote;

if (hCellNote)
   {
   LPTSTR lpszNote = (LPTSTR)tbGlobalLock(hCellNote);
   if (lpszNote && lstrlen(lpszNote))
      {
      nRet = lstrlen(lpszNote);
      if (Note)
        _tcscpy(Note, lpszNote);     
      tbGlobalUnlock(lpCell->hCellNote);  
      }
   }

if (lpCell) SS_UnlockCellItem(lpSS, CellCol, CellRow);
if (lpCol) SS_UnlockColItem(lpSS, CellCol);
if (lpRow) SS_UnlockRowItem(lpSS, CellRow);

return nRet;
}


BOOL SS_SetCellCellNote(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow, LPCTSTR Note)
{
LPSS_COL  lpCol;
LPSS_ROW  lpRow;
LPSS_CELL lpCell;
LPTSTR    lpsz;
BOOL      fRet = FALSE;

if (CellRow == SS_ALLROWS && CellCol == SS_ALLCOLS)
{
   if (lpSS->hCellNote)
   {
      tbGlobalFree(lpSS->hCellNote);
      lpSS->hCellNote = 0;
   }
   if (Note && *Note)
   {
     lpSS->hCellNote = tbGlobalAlloc(GHND, (lstrlen(Note)+1)*sizeof(TCHAR));
     lpsz = (LPTSTR)tbGlobalLock(lpSS->hCellNote);
     _tcscpy(lpsz, Note);
     tbGlobalUnlock(lpSS->hCellNote);
   }
   SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
   fRet = TRUE;
}

else if (CellRow == SS_ALLROWS)
{
   if (lpCol = SS_AllocLockCol(lpSS, CellCol))
   {
     if (lpCol->hCellNote)
     {
        tbGlobalFree(lpCol->hCellNote);
        lpCol->hCellNote = 0;
     }
     if (Note && *Note)
     {
       lpCol->hCellNote = tbGlobalAlloc(GHND, (lstrlen(Note)+1)*sizeof(TCHAR));
       lpsz = (LPTSTR)tbGlobalLock(lpCol->hCellNote);
       _tcscpy(lpsz, Note);
       tbGlobalUnlock(lpCol->hCellNote);
     }
     SS_InvalidateCol(lpSS, CellCol);
     SS_UnlockColItem(lpSS, CellCol);
     fRet = TRUE;
   }
}
else if (CellCol == SS_ALLCOLS)
{
   if (lpRow = SS_AllocLockRow(lpSS, CellRow))
   {
     if (lpRow->hCellNote)
     {
        tbGlobalFree(lpRow->hCellNote);
        lpRow->hCellNote = 0;
     }
     if (Note && *Note)
     {
       lpRow->hCellNote = tbGlobalAlloc(GHND, (lstrlen(Note)+1)*sizeof(TCHAR));
       lpsz = (LPTSTR)tbGlobalLock(lpRow->hCellNote);
       _tcscpy(lpsz, Note);
       tbGlobalUnlock(lpRow->hCellNote);
     }
     SS_InvalidateRow(lpSS, CellRow);
     SS_UnlockRowItem(lpSS, CellRow);
     fRet = TRUE;
   }
}

else if (lpCell = SS_AllocLockCell(lpSS, NULL, CellCol, CellRow))
   {
   if (lpCell->hCellNote)
   {
      tbGlobalFree(lpCell->hCellNote);
      lpCell->hCellNote = 0;
   }
   if (Note && *Note)
   {
     lpCell->hCellNote = tbGlobalAlloc(GHND, (lstrlen(Note)+1)*sizeof(TCHAR));
     lpsz = (LPTSTR)tbGlobalLock(lpCell->hCellNote);
     _tcscpy(lpsz, Note);
     tbGlobalUnlock(lpCell->hCellNote);
   }
   SS_InvalidateRange(lpSS, CellCol, CellRow, CellCol, CellRow);
   SS_UnlockCellItem(lpSS, CellCol, CellRow);
   fRet = TRUE;
   } 

return (fRet);
}

#endif //SS_V35

#endif
