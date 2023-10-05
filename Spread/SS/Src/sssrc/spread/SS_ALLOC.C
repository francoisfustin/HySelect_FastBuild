/*********************************************************
* SS_ALLOC.C
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
void SSOCX_FreePict(LPPICTURE lpPict);
#endif

#include "spread.h"
#include "ss_alloc.h"
#include "ss_bord.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_formu.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_type.h"
#include "ss_user.h"

#ifdef SS_VB
//#include <vbapi.h>
void SSVB_FreePic(HANDLE hPic);
#endif

#ifdef SS_NOSUPERCLASS
#include "..\..\vbsrc\vbspread.h"
#endif

#ifdef SS_V40
#include "ss_span.h"
#endif // SS_V40

#if defined(SS_V70) && defined(SS_OCX)
#include "ssocxdb.h"
#endif

#if SS_V80
#include "ss_data.h"
#endif

#define BUFFER_ALLOC_CNT 200
#define ALLOC_INC        30

#ifndef SS_NO_USE_SH
extern OMEM_POOL tbOmemPoolCol;
extern OMEM_POOL tbOmemPoolRow;
extern OMEM_POOL tbOmemPoolCell;
#endif

void           SS_FreeSheetsWithHwnd(LPSS_BOOK lpBook);
void           SS_FreeSheetWithHwnd(LPSPREADSHEET lpSS);
void           SS_FreeCalc(LPSPREADSHEET lpSS, TBGLOBALHANDLE hCalc);
void           SS_FreeAllocItem(LPSS_ALLOCITEM lpAllocItem);
void           SS_FreeRowItem(LPSPREADSHEET lpSS,
                              LPSS_ALLOCITEM lpAllocItem);
TBGLOBALHANDLE SS_GrabItemHandle(LPSPREADSHEET lpSS,
                                 LPSS_ALLOCITEM lpAllocItem, SS_COORD wItemNum,
                                 SS_COORD wMaxItems);
void           SSx_FreeCells(LPSPREADSHEET lpSS);
void           SSx_FreeRowCells(LPSPREADSHEET lpSS, LPSS_ALLOCITEM lpCells);
void           SSx_FreeCalc(LPSPREADSHEET lpSS, TBGLOBALHANDLE hCalc);
BOOL           SSx_DeAllocCell(LPSPREADSHEET lpSS,
                               LPSS_ROW lpRow, SS_COORD CellCol,
                               SS_COORD CellRow, BOOL fSendChangeMsg);
void           SS_PicListFree(LPSS_BOOK lpBook);
#if (defined(SS_VB) || defined(SS_OCX))
// This frees the bookmark if there is one (func is in VBBOUND.C)
void DLLENTRY vbSS_SetBookmark(LPVOID lpDestBookMark, LPVOID srcBookMark);   
#endif

#ifdef SS_OCX
BOOL DLLENTRY SSOCXGetMessageReflect(LPSS_BOOK lpBook);
#endif


BOOL SS_Alloc(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
if (Col != SS_ALLCOLS && Row != SS_ALLROWS)
   return (SS_AllocCell(lpSS, Col, Row) > 0);

else if (Col != SS_ALLCOLS && Row == SS_ALLROWS)
   return (SS_AllocCol(lpSS, Col) > 0);

else if (Col == SS_ALLCOLS && Row != SS_ALLROWS)
   return (SS_AllocRow(lpSS, Row) > 0);

return (FALSE);
}


BOOL SS_AllocRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2)
{
BOOL fRet = TRUE;
SS_COORD i, j;

for (i = Row; i <= Row2 && fRet; i++)
	for (j = Col; j <= Col2 && fRet; j++)
		fRet = SS_Alloc(lpSS, j, i);

return (fRet);
}


TBGLOBALHANDLE SS_AllocCol(LPSPREADSHEET lpSS, SS_COORD CellCol)
{
TBGLOBALHANDLE hColItem;
LPSS_COL       lpColItem;
BOOL           fIsNew;

if (CellCol == -1)
   return (0);

/****************************
* Make room for column item
****************************/

if ((hColItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolCol, &lpSS->Col.Items, CellCol,
                              sizeof(SS_COL), 0, SS_GetColCnt(lpSS),
                              &fIsNew)) && hColItem != (TBGLOBALHANDLE)-1 && fIsNew)
   {
   lpColItem = (LPSS_COL)tbGlobalLock(hColItem);

   lpColItem->FontId = SS_FONT_NONE;
   lpColItem->dColWidthX100 = SS_WIDTH_DEFAULT;
   lpColItem->dColWidthInPixels = SS_WIDTH_DEFAULT;
#ifdef SS_V35
   lpColItem->nSortIndicator = -1;
   lpColItem->hCellNote = 0;
#endif

   tbGlobalUnlock(hColItem);
   }

else if (hColItem == (TBGLOBALHANDLE)-1)
   return (FALSE);

if (CellCol >= lpSS->Col.AllocCnt)
   lpSS->Col.AllocCnt = CellCol + 1;

lpSS->Col.LastRefCoord.Coord.Col = CellCol;
lpSS->Col.LastRefCoord.hItem = hColItem;

return (hColItem);
}


TBGLOBALHANDLE SS_AllocRow(LPSPREADSHEET lpSS, SS_COORD CellRow)
{
TBGLOBALHANDLE hRowItem;
LPSS_ROW       lpRowItem;
BOOL           fIsNew;

if (CellRow == -1)
   return (0);

/*************************
* Make room for Row item
*************************/

if ((hRowItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolRow, &lpSS->Row.Items, CellRow,
                              sizeof(SS_ROW), 0, SS_GetRowCnt(lpSS),
                              &fIsNew)) && (LPARAM)hRowItem != (LPARAM)-1 && fIsNew)
   {
   lpRowItem = (LPSS_ROW)tbGlobalLock(hRowItem);

   lpRowItem->FontId = SS_FONT_NONE;
   lpRowItem->RowMaxFontId = SS_FONT_NONE;
   lpRowItem->dRowHeightX100 = SS_HEIGHT_DEFAULT;
   lpRowItem->dRowHeightInPixels = SS_HEIGHT_DEFAULT;
#ifdef SS_V35
   lpRowItem->hCellNote = 0;
#endif

   tbGlobalUnlock(hRowItem);
   }

else if ((LPARAM)hRowItem == -1)
   return (FALSE);

if (CellRow >= lpSS->Row.AllocCnt)
   lpSS->Row.AllocCnt = CellRow + 1;

lpSS->Row.LastRefCoord.Coord.Row = CellRow;
lpSS->Row.LastRefCoord.hItem = hRowItem;

return (hRowItem);
}


TBGLOBALHANDLE SS_AllocCell(LPSPREADSHEET lpSS, SS_COORD CellCol,
                            SS_COORD CellRow)
{
TBGLOBALHANDLE hCellItem = 0;
LPSS_ROW       lpRow;
BOOL           fIsNew;

if (CellCol == -1 || CellRow == -1)
   return (0);

if (lpRow = SS_AllocLockRow(lpSS, CellRow))
   {
   hCellItem = SS_AllocItems(lpSS, lpSS->lpBook->OmemPoolCell, &lpRow->Cells,
                             CellCol, sizeof(SS_CELL), 0,
                             SS_GetColCnt(lpSS), &fIsNew);

   if (fIsNew && hCellItem && (LPARAM)hCellItem != -1L)
      {
      LPSS_CELL lpCell = (LPSS_CELL)tbGlobalLock(hCellItem);
      lpCell->FontId = SS_FONT_NONE;
      lpCell = NULL;    // This is to get around an internal compiler
                        // error in MSVC 8.0
      tbGlobalUnlock(hCellItem);

      if (CellCol >= lpSS->Col.AllocCnt)
         lpSS->Col.AllocCnt = CellCol + 1;
      }

   else if ((LPARAM)hCellItem == -1)
      hCellItem = 0;

   lpSS->LastCell.Coord.Row = CellRow;
   lpSS->LastCell.Coord.Col = CellCol;
   lpSS->LastCell.hItem = hCellItem;

   SS_UnlockRowItem(lpSS, CellRow);
   }

return (hCellItem);
}


BOOL SS_DeAllocCol(LPSPREADSHEET  lpSS, SS_COORD Col, BOOL fSendChangeMsg)
{
LPSS_ALLOCITEM lpAllocItem;
LPSS_COL       lpCol;
SS_COORD       lMaxCols;
SS_COORD       i;

if (Col >= lpSS->Col.AllocCnt)
   return (FALSE);

for (i = 0; i < lpSS->Row.AllocCnt; i++)
   SS_DeAllocCell(lpSS, Col, i, fSendChangeMsg);

if (lpCol = SS_LockColItem(lpSS, Col))
   {
   SSx_FreeCol(lpSS, lpCol);

   SS_UnlockColItem(lpSS, Col);
   lMaxCols = SS_GetColCnt(lpSS);
   lpAllocItem = &lpSS->Col.Items;
   SS_DeleteItem(lpSS, lpAllocItem, Col, lMaxCols);
   }

lpSS->Col.LastRefCoord.Coord.Col = -1;
return (TRUE);
}


void SSx_FreeCol(LPSPREADSHEET lpSS, LPSS_COL lpCol)
{
if (lpCol->hCellType)
   SS_DeAllocCellType(lpSS, lpCol->hCellType);
#ifdef SS_V35
if (lpCol->hCellNote)
{
  tbGlobalFree(lpCol->hCellNote);
  lpCol->hCellNote = 0;
}
#endif
#ifdef SS_V40
if (lpCol->hColID)
{
  tbGlobalFree(lpCol->hColID);
  lpCol->hColID = 0;
}
if (lpCol->hCellTag)
{
  tbGlobalFree(lpCol->hCellTag);
  lpCol->hCellTag = 0;
}
#endif

SSx_FreeData(&lpCol->Data);
SS_FreeBorder(lpSS, &lpCol->hBorder);

#ifndef SS_NOCALC
#ifdef SS_OLDCALC
SS_FreeCalcList(&lpCol->Dependents);
#endif

if (lpCol->hCalc)
   {
   SS_FreeCalc(lpSS, lpCol->hCalc);
   lpCol->hCalc = 0;
   }
#endif

if (lpCol->hDBFieldName)
   {
   tbGlobalFree(lpCol->hDBFieldName);
   lpSS->DataFieldNameCnt--;
   }
}


BOOL SS_DeAllocRow(LPSPREADSHEET lpSS, SS_COORD Row, BOOL fSendChangeMsg)
{
LPSS_ROW      lpRow;
SS_COORD      i;

if (Row >= lpSS->Row.AllocCnt)
   {
   return (FALSE);
   }

if (lpRow = SS_LockRowItem(lpSS, Row))
   {
   for (i = 0; i < lpSS->Col.AllocCnt; i++)
      SSx_DeAllocCell(lpSS, lpRow, i, Row, fSendChangeMsg);

   if (lpRow->fRowSelected)
      lpSS->MultiSelCnt--;

   SSx_FreeRow(lpSS, lpRow);

   SS_UnlockRowItem(lpSS, Row);
   SS_DeleteItem(lpSS, &lpSS->Row.Items, Row, SS_GetRowCnt(lpSS));

   lpSS->Row.LastRefCoord.Coord.Row = -1;
   }

return (TRUE);
}


void SSx_FreeRow(LPSPREADSHEET lpSS, LPSS_ROW lpRow)
{
if (lpRow->hCellType)
   SS_DeAllocCellType(lpSS, lpRow->hCellType);

#ifdef SS_V35
if (lpRow->hCellNote)
{
  tbGlobalFree(lpRow->hCellNote);
  lpRow->hCellNote = 0;
}
#endif
#ifdef SS_V40
if (lpRow->hCellTag)
{
  tbGlobalFree(lpRow->hCellTag);
  lpRow->hCellTag = 0;
}
#endif // SS_V40

#ifndef SS_NOCALC
#ifdef SS_OLDCALC
SS_FreeCalcList(&lpRow->Dependents);
#endif

if (lpRow->hCalc)
   {
   SS_FreeCalc(lpSS, lpRow->hCalc);
   lpRow->hCalc = 0;
   }
#endif

SSx_FreeData(&lpRow->Data);
SS_FreeBorder(lpSS, &lpRow->hBorder);

if (lpRow->Cells.wItemCnt)
	SSx_FreeRowCells(lpSS, &lpRow->Cells);
   // SS_FreeAllocItem(&lpRow->Cells); RFW - 1/26/02 - 9730

#if (defined(SS_VB) || defined(SS_OCX))
// This frees the bookmark if there is one (func is in VBBOUND.C)
if (lpRow->hlstrBookMark)
  vbSS_SetBookmark((LPVOID)&lpRow->hlstrBookMark, NULL);   
#endif
}


BOOL SS_DeAllocCell(LPSPREADSHEET lpSS, SS_COORD CellCol, SS_COORD CellRow,
                    BOOL fSendChangeMsg)
{
LPSS_ROW      lpRow;

if (CellRow >= lpSS->Row.AllocCnt || CellCol >= lpSS->Col.AllocCnt)
   return (FALSE);

lpRow = SS_LockRowItem(lpSS, CellRow);
SSx_DeAllocCell(lpSS, lpRow, CellCol, CellRow, fSendChangeMsg);

if (lpRow)
   SS_UnlockRowItem(lpSS, CellRow);

return (TRUE);
}


BOOL SSx_DeAllocCell(LPSPREADSHEET lpSS, LPSS_ROW lpRow, SS_COORD CellCol,
                     SS_COORD CellRow, BOOL fSendChangeMsg)
{
LPSS_CELL     lpCell;
BOOL          fDataChange = FALSE;

if (CellRow >= lpSS->Row.AllocCnt ||
    CellCol >= lpSS->Col.AllocCnt)
   {
   return (FALSE);
   }

if (lpCell = SSx_LockCellItem(lpSS, lpRow, CellCol, CellRow))
   {
   if (fSendChangeMsg && lpCell->Data.bDataType)
      fDataChange = TRUE;

   SS_ResetCellOverflow(lpSS, CellCol, CellRow);

   SSx_FreeCell(lpSS, lpCell);

   SS_UnlockCellItem(lpSS, CellCol, CellRow);

   if (lpRow)
      SS_DeleteItem(lpSS, &lpRow->Cells, CellCol, SS_GetColCnt(lpSS));

   lpSS->LastCell.Coord.Col = -1;
   lpSS->LastCell.Coord.Row = -1;

   if (fDataChange && lpSS->lpBook->hWnd)
      SS_SendMsgCoords(lpSS, SSM_DATACHANGE, GetDlgCtrlID(lpSS->lpBook->hWnd),
                       CellCol, CellRow);
   }

return (TRUE);
}


void SSx_FreeCell(LPSPREADSHEET lpSS, LPSS_CELL lpCell)
{
SSx_FreeData(&lpCell->Data);
SS_FreeBorder(lpSS, &lpCell->hBorder);

#ifndef SS_NOCALC
if (lpCell->hCalc)
   {
   SS_FreeCalc(lpSS, lpCell->hCalc);
   lpCell->hCalc = 0;
   }

#ifdef SS_OLDCALC
SS_FreeCalcList(&lpCell->Dependents);
#endif
#endif

if (lpCell->hCellType)
   SS_DeAllocCellType(lpSS, lpCell->hCellType);

#ifdef SS_V35
if (lpCell->hCellNote)
{
  tbGlobalFree(lpCell->hCellNote);
  lpCell->hCellNote = 0;
}
#endif
#ifdef SS_V40
if (lpCell->hCellTag)
{
  tbGlobalFree(lpCell->hCellTag);
  lpCell->hCellTag = 0;
}
#endif // SS_V40
}


#ifndef SS_NOCALC

void SS_FreeCalc(LPSPREADSHEET lpSS, TBGLOBALHANDLE hCalc)
{
SSx_FreeCalc(lpSS, hCalc);
}


#ifdef SS_OLDCALC
void SS_FreeCalcList(LPSS_CALCLIST lpCalcList)
{
if (lpCalcList->hItems)
   tbGlobalFree(lpCalcList->hItems);

lpCalcList->hItems = 0;
lpCalcList->ItemCnt = 0;
lpCalcList->ItemAllocCnt = 0;
lpCalcList->ListBuilt = FALSE;
}
#endif // SS_OLDCALC

#endif

// BJO 25Sep96 SEL6314 - Before fix

//BOOL SS_IsDestroyed(HWND hWnd)
//{
//return (!IsWindow(hWnd));
//}

// BJO 25Sep96 SEL6314 - Begin fix

BOOL SS_IsDestroyed(HWND hWnd)
{
LPSS_BOOK lpBook;
BOOL bRet = TRUE;

if (IsWindow(hWnd) && (lpBook = SS_BookLock(hWnd)))
   {
   	bRet = (hWnd != lpBook->hWnd);
	SS_BookUnlock(hWnd);
   }

return bRet;
}

// BJO 25Sep96 SEL6314 - End fix


#if 0

#ifdef SS_NOSUPERCLASS
static HWND g_hwndSSLastLocked = 0;
static LPSPREADSHEET g_lpSSLastLocked = 0;
#endif

LPSPREADSHEET SS_Lock(HWND hWnd)
{
#ifdef SS_NOSUPERCLASS
/*
LPVBSPREAD lpSpread;

lpSpread = (LPVBSPREAD)VBDerefControl(VBGetHwndControl(hWnd));
return ((LPSPREADSHEET)tbGlobalLock(lpSpread->hClassExtraBytes));
*/

if (hWnd != g_hwndSSLastLocked)
   {
   g_lpSSLastLocked = ((LPSPREADSHEET)MAKELONG(GetProp(hWnd, "Xtra1"),
                                               GetProp(hWnd, "Xtra2")));
   if (g_lpSSLastLocked)
      g_hwndSSLastLocked = hWnd;
   else
      g_hwndSSLastLocked = 0;
   }
return g_lpSSLastLocked;

#else
return (hWnd ? (LPSPREADSHEET)tbGlobalLock(GetWindowLong(hWnd, 0)) : NULL);
#endif
}


BOOL SSx_Unlock(HWND hWnd)
{
#ifndef SS_NOSUPERCLASS
TBGLOBALHANDLE hSpread;

if (hSpread = GetWindowLong(hWnd, 0))
   return (tbGlobalUnlock(hSpread));
#endif

return (FALSE);
}
#endif


LPSS_BOOK SS_BookLock(HWND hWnd)
{
#if defined(_WIN64) || defined(_IA64)
return (hWnd && IsWindow(hWnd) ? (LPSS_BOOK)tbGlobalLock(GetWindowLongPtr(hWnd, 0)) : NULL);
#else
return (hWnd && IsWindow(hWnd) ? (LPSS_BOOK)tbGlobalLock(GetWindowLong(hWnd, 0)) : NULL);
#endif
}


BOOL SSx_BookUnlock(HWND hWnd)
{
TBGLOBALHANDLE hBook = (TBGLOBALHANDLE)0;
if (hWnd && IsWindow(hWnd))
#if defined(_WIN64) || defined(_IA64)
	hBook = (TBGLOBALHANDLE)GetWindowLongPtr(hWnd, 0);
#else
	hBook = (TBGLOBALHANDLE)GetWindowLong(hWnd, 0);
#endif
if (hBook)
	return(tbGlobalUnlock(hBook));
return(FALSE);
}


TBGLOBALHANDLE SS_BookGetSheetHandleFromIndex(LPSS_BOOK lpBook, short nIndex)
{
LPTBGLOBALHANDLE lpSheets;
TBGLOBALHANDLE   hSS = 0;

if (lpBook && lpBook->hSheets && nIndex >= 0 && nIndex < lpBook->nSheetCnt)
	{
	lpSheets = (LPTBGLOBALHANDLE)tbGlobalLock(lpBook->hSheets);
	hSS = lpSheets[nIndex];
	tbGlobalUnlock(lpBook->hSheets);
	}

return (hSS);
}

LPSPREADSHEET SS_BookLockSheetIndex(LPSS_BOOK lpBook, short nIndex)
{
LPTBGLOBALHANDLE lpSheets;
LPSPREADSHEET    lpSS = NULL;

if (lpBook && lpBook->hSheets && nIndex >= 0 && nIndex < lpBook->nSheetCnt)
	{
	lpSheets = (LPTBGLOBALHANDLE)tbGlobalLock(lpBook->hSheets);
  lpSS = (LPSPREADSHEET)tbGlobalLock(lpSheets[nIndex]);
	tbGlobalUnlock(lpBook->hSheets);
	}

return (lpSS);
}


BOOL SSx_BookUnlockSheetIndex(LPSS_BOOK lpBook, short nIndex)
{
BOOL fRet = FALSE;

if (lpBook && lpBook->hSheets && nIndex >= 0 && nIndex < lpBook->nSheetCnt)
	{
	LPTBGLOBALHANDLE lpSheets = (LPTBGLOBALHANDLE)tbGlobalLock(lpBook->hSheets);
	fRet = tbGlobalUnlock(lpSheets[nIndex]);
	tbGlobalUnlock(lpBook->hSheets);
	}

return (fRet);
}

LPSPREADSHEET SS_BookLockActiveSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookLockSheetIndex(lpBook, lpBook->nActiveSheet));

return (NULL);
}


BOOL SSx_BookUnlockActiveSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookUnlockSheetIndex(lpBook, lpBook->nActiveSheet));

return (FALSE);
}


LPSPREADSHEET SS_BookLockSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookLockSheetIndex(lpBook, lpBook->nSheet));

return (NULL);
}


BOOL SSx_BookUnlockSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookUnlockSheetIndex(lpBook, lpBook->nSheet));

return (FALSE);
}

#ifdef SS_NO_USE_SH
BOOL SS_UnlockColItem(LPSPREADSHEET lpSS, SS_COORD dCol)
{
BOOL ret = FALSE;
TBGLOBALHANDLE hCol;
LPSS_COL       lpCol = NULL;

if (lpSS->Col.LastRefCoord.Coord.Col == dCol)
   ret = GlobalUnlock(lpSS->Col.LastRefCoord.hItem);

else if ((hCol = SS_GetItem(lpSS, &lpSS->Col.Items, dCol,
                            SS_GetColCnt(lpSS))) && (LPARAM)hCol != (LPARAM)-1L)
   ret = GlobalUnlock(hCol);

return ret;
}

BOOL SS_UnlockRowItem(LPSPREADSHEET lpSS, SS_COORD dRow)
{
BOOL ret = FALSE;	//RMA fix uninit variable report in BC
TBGLOBALHANDLE hRow;
LPSS_ROW       lpRow = NULL;

if (dRow == -1)
   return FALSE;

if (lpSS->Row.LastRefCoord.Coord.Row == dRow)
   ret = GlobalUnlock(lpSS->Row.LastRefCoord.hItem);

else if ((hRow = SS_GetItem(lpSS, &lpSS->Row.Items, dRow,
                            SS_GetRowCnt(lpSS))) && (LPARAM)hRow != (LPARAM)-1L)
   ret = GlobalUnlock(hRow);

return ret;
}

BOOL SS_UnlockCellItem(LPSPREADSHEET lpSS, SS_COORD dCol, SS_COORD dRow)
{
return (SSx_UnlockCellItem(lpSS, NULL, dCol, dRow));
}

BOOL SSx_UnlockCellItem(LPSPREADSHEET lpSS, LPSS_ROW lpRow,
                           SS_COORD dCol, SS_COORD dRow)
{
BOOL ret = FALSE;		// RMA fix BC uninit memory error
TBGLOBALHANDLE hCell;
LPSS_CELL      lpCell = NULL;
LPSS_ROW       lpRowOld = lpRow;

if (dCol == SS_ALLCOLS || dRow == SS_ALLROWS)
   return FALSE;

if (lpSS->LastCell.Coord.Row == dRow && lpSS->LastCell.Coord.Col == dCol)
   ret = GlobalUnlock(lpSS->LastCell.hItem);

else if (lpRow || (lpRow = SS_LockRowItem(lpSS, dRow)))
   {
   if ((hCell = SS_GetItem(lpSS, &lpRow->Cells, dCol, SS_GetColCnt(lpSS))) &&
                           (LPARAM)hCell != (LPARAM)-1L)
      ret = GlobalUnlock(hCell);

   if (!lpRowOld)
      SS_UnlockRowItem(lpSS, dRow);
   }

return ret;
}

#endif // SS_NO_USE_SH

#if 0

LPSPREADSHEET SS_BookLockSheetIndex(LPSS_BOOK lpBook, short nIndex)
{
return (lpBook);
}


BOOL SSx_BookUnlockSheetIndex(LPSS_BOOK lpBook, short nIndex)
{
return (FALSE);
}


LPSPREADSHEET SS_BookLockActiveSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookLockSheetIndex(lpBook, 0));

return (NULL);
}


BOOL SSx_BookUnlockActiveSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookUnlockSheetIndex(lpBook, 0));

return (FALSE);
}


LPSPREADSHEET SS_BookLockSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookLockSheetIndex(lpBook, 0));

return (NULL);
}


BOOL SSx_BookUnlockSheet(LPSS_BOOK lpBook)
{
if (lpBook)
	return (SS_BookUnlockSheetIndex(lpBook, 0));

return (FALSE);
}
#endif // 0

LPSPREADSHEET SS_SheetLockIndex(HWND hWnd, short nIndex)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
LPSPREADSHEET lpSheet = SS_BookLockSheetIndex(lpBook, nIndex);
SS_BookUnlock(hWnd);
return (lpSheet);
}


BOOL SSx_SheetUnlockIndex(HWND hWnd, short nIndex)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet = SS_BookUnlockSheetIndex(lpBook, nIndex);
SS_BookUnlock(hWnd);
return (fRet);
}


LPSPREADSHEET SS_SheetLock(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
LPSPREADSHEET lpSheet = SS_BookLockSheet(lpBook);
SS_BookUnlock(hWnd);
return (lpSheet);
}


BOOL SSx_SheetUnlock(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet = SS_BookUnlockSheet(lpBook);
SS_BookUnlock(hWnd);
return (fRet);
}


LPSPREADSHEET SS_SheetLockActive(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
LPSPREADSHEET lpSheet = SS_BookLockActiveSheet(lpBook);
SS_BookUnlock(hWnd);
return (lpSheet);
}


BOOL SSx_SheetUnlockActive(HWND hWnd)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet = SS_BookUnlockActiveSheet(lpBook);
SS_BookUnlock(hWnd);
return (fRet);
}


LRESULT SS_SendMsg(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, LPARAM lParam)
{
HWND hWnd;
LRESULT lRet;

if (!lpBook->hWnd)  // handle an OCX control without a HWND
   return 0;

hWnd = lpBook->hWnd;

lpBook->wMessageBeingSent++;
lRet = SSx_SendMsg(lpBook, lpSS, Msg, wParam, lParam);

if (SS_IsDestroyed(hWnd))
   return (lRet);

if (lpBook->wMessageBeingSent > 0)
   lpBook->wMessageBeingSent--;

return (lRet);
}


LRESULT SS_SendMsgActiveSheet(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPSPREADSHEET lpSS = SS_SheetLockActive(hWnd);
LRESULT lRet;

lRet = SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, lParam);

SS_SheetUnlockActive(hWnd);
return (lRet);
}


LRESULT SSx_SendMsgActiveSheet(LPSS_BOOK lpBook, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
LRESULT lRet;

lRet = SS_SendMsg(lpBook, lpSS, Msg, wParam, lParam);

SS_BookUnlockActiveSheet(lpBook);
return (lRet);
}


LRESULT SSx_SendMsg(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LRESULT  lRet = 0;
BOOL  bSelfMsg = TRUE;
BOOL  bSendThru = TRUE;

/* RFW - 6/4/04 - 14309
if (!lpSS || !lpBook || !lpBook->hWnd)  // handle an OCX control without a HWND
   return 0;
*/
if (!lpBook || !lpBook->hWnd)  // handle an OCX control without a HWND

#ifdef SS_OCX
if (Msg == WM_COMMAND)
   bSelfMsg = !(SSOCXGetMessageReflect(lpBook));
#endif

if (lpSS)
	lpBook->nSheetSendingMsg = lpSS->nSheetIndex;
else
	lpBook->nSheetSendingMsg = -1;

if ((bSelfMsg) && (!lpBook->lpfnCallBack))
   {
	HWND hWnd = lpBook->hWnd;

   lRet = SendMessage(hWnd, Msg, wParam, lParam);

   if (SS_IsDestroyed(hWnd))
      return (lRet);
#ifdef SS_OCX
// CTF 
// for OCX do not send messages that fire events to the Owner
// or else your return code will be obliterated
//CTF
   if ((Msg >= SSM_DBLCLK) && (Msg <= SSM_KEYPRESS))
      bSendThru = FALSE;
#endif
   }

if ((bSendThru) && (lpBook->hWndOwner))
   {
	HWND hWnd = lpBook->hWnd;

   lRet = SendMessage(lpBook->hWndOwner, Msg, wParam, lParam);

   if (SS_IsDestroyed(hWnd))
      return (lRet);
   }

if (lpBook->lpfnCallBack)
   lRet = (*lpBook->lpfnCallBack)(lpBook->hWnd, Msg, wParam, lParam);

return (lRet);
}


LRESULT SS_PostMsg(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LRESULT lRet;
HWND hWnd;

if (!lpSS || !lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   return 0;

hWnd = lpSS->lpBook->hWnd;

lRet = SendMessage(lpSS->lpBook->hWnd, Msg, wParam, lParam);

if (SS_IsDestroyed(hWnd))
   return (lRet);

if (!lRet)
   {
   if (lpSS->lpBook->hWndOwner)
      lRet = PostMessage(lpSS->lpBook->hWndOwner, Msg, wParam, lParam);

   if (lpSS->lpBook->lpfnCallBack)
      lRet = (*lpSS->lpBook->lpfnCallBack)(lpSS->lpBook->hWnd, Msg, wParam, lParam);
   }

return (lRet);
}


LRESULT SS_SendMsgColCoordRange(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, SS_COORD Col, SS_COORD Col2)
{
SS_AdjustCellCoordsOut(lpSS, &Col, NULL);
SS_AdjustCellCoordsOut(lpSS, &Col2, NULL);
#ifdef SS_USE16BITCOORDS
return (SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, MAKELONG(Col, Col2)));
#else
{
SS_COORDRANGE Range;

Range.Coord1 = Col;
Range.Coord2 = Col2;

return (SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, (LPARAM)(LPSS_COORDRANGE)&Range));
}
#endif
}


LRESULT SS_SendMsgRowCoordRange(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, SS_COORD Row, SS_COORD Row2)
{
SS_AdjustCellCoordsOut(lpSS, NULL, &Row);
SS_AdjustCellCoordsOut(lpSS, NULL, &Row2);
#ifdef SS_USE16BITCOORDS
return (SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, MAKELONG(Row, Row2)));
#else
{
SS_COORDRANGE Range;

Range.Coord1 = Row;
Range.Coord2 = Row2;

return (SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, (LPARAM)(LPSS_COORDRANGE)&Range));
}
#endif
}


LRESULT SS_SendMsgCoords(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, SS_COORD Col,
                         SS_COORD Row)
{
#ifdef SS_USE16BITCOORDS
SS_AdjustCellCoordsOut(lpSS, &Col, &Row);
return (SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, MAKELONG(Col, Row)));
#else
SS_CELLCOORD CellCoord;

SS_AdjustCellCoordsOut(lpSS, &Col, &Row);
CellCoord.Col = Col;
CellCoord.Row = Row;

if (Msg == SSM_DATACHANGE)
   lpSS->fChangeMade = TRUE;

return (SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, (LPARAM)(LPSS_CELLCOORD)&CellCoord));
#endif
}


LRESULT SS_SendMsgCommand(LPSS_BOOK lpBook, LPSPREADSHEET lpSS, WORD wCmd, BOOL fNoMsgCnt)
{
WPARAM wParam;
LPARAM lParam;

#ifdef WIN32
  wParam = MAKEWPARAM(GetWindowID(lpBook->hWnd), wCmd);
  lParam = (LPARAM)lpBook->hWnd;
#else
  wParam = (WPARAM)GetWindowID(lpBook->hWnd);
  lParam = MAKELPARAM(lpBook->hWnd, wCmd);
#endif

if (fNoMsgCnt)
   return (SSx_SendMsg(lpBook, lpSS, WM_COMMAND, wParam, lParam));
else
   return (SS_SendMsg(lpBook, lpSS, WM_COMMAND, wParam, lParam));
}


LRESULT SS_SendMsgCommandActiveSheet(HWND hWnd, WORD wCmd, BOOL fNoMsgCnt)
{
LPSPREADSHEET lpSS = SS_SheetLockActive(hWnd);
LRESULT   lRet;

lRet = SS_SendMsgCommand(lpSS->lpBook, lpSS, wCmd, fNoMsgCnt);

SS_SheetUnlockActive(hWnd);
return (lRet);
}


LRESULT SS_SendMsgEnterRow(LPSPREADSHEET lpSS, UINT Msg, WPARAM wParam, SS_COORD Row,
                           BOOL fNewRow)
{
#ifdef SS_USE16BITCOORDS
SS_AdjustCellCoordsOut(lpSS, NULL, &Row);
return (SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, MAKELONG(Row, (WORD)fNewRow)));
#else
SS_ENTERROW EnterRow;

SS_AdjustCellCoordsOut(lpSS, NULL, &Row);
EnterRow.Row = Row;
EnterRow.fNewRow = fNewRow;

return (SS_SendMsg(lpSS->lpBook, lpSS, Msg, wParam, (LPARAM)(LPSS_ENTERROW)&EnterRow));
#endif
}


#ifdef SS_UTP
LRESULT SS_SendMsgInsDel(LPSPREADSHEET lpSS, WORD wType, WORD wOperation,
                         WORD wDirection, SS_COORD Coord1, SS_COORD Coord2)
{
SS_INSDEL InsDel;

InsDel.wType = wType;
InsDel.wOperation = wOperation;
InsDel.wDirection = wDirection;
InsDel.Coord1 = Coord1;
InsDel.Coord2 = Coord2;

return (SS_SendMsg(lpSS->lpBook, lpSS, SSM_INSDEL, GetWindowID(hWnd),
                   (LPARAM)(LPSS_INSDEL)&InsDel));
}
#endif // SS_UTP


LRESULT SS_SendMsgComboCloseUp(LPSPREADSHEET lpSS, short dSelChange, SS_COORD Col,
                               SS_COORD Row)
{
SS_COMBOCLOSEUP CloseUp;

SS_AdjustCellCoordsOut(lpSS, &Col, &Row);
CloseUp.Col = Col;
CloseUp.Row = Row;
CloseUp.dSelChange = dSelChange;

return (SS_SendMsg(lpSS->lpBook, lpSS, SSM_COMBOCLOSEUP, GetWindowID(lpSS->lpBook->hWnd),
                   (LPARAM)(LPSS_COMBOCLOSEUP)&CloseUp));
}


#ifdef SS_V70
LRESULT SS_SendMsgBeforeEditMode(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                                 WORD wUserAction, LPWORD lpwCursorPos, LPBOOL lpfCancel)
{
SS_BEFOREEDITMODE BeforeEditMode;
LRESULT           lRet;

SS_AdjustCellCoordsOut(lpSS, &Col, &Row);
BeforeEditMode.Col = Col;
BeforeEditMode.Row = Row;
BeforeEditMode.wUserAction = wUserAction;
BeforeEditMode.wCursorPos = *lpwCursorPos;
BeforeEditMode.fCancel = *lpfCancel;

lRet = SS_SendMsg(lpSS->lpBook, lpSS, SSM_BEFOREEDITMODE, GetWindowID(lpSS->lpBook->hWnd),
                  (LPARAM)(LPSS_BEFOREEDITMODE)&BeforeEditMode);

*lpwCursorPos = BeforeEditMode.wCursorPos;
*lpfCancel = BeforeEditMode.fCancel;

return (lRet);
}


LRESULT SS_SendMsgBeforeScrollTip(LPSPREADSHEET lpSS, BOOL fIsVertical, SS_COORD Index,
                                  LPTSTR lpText, LPSHORT lpnWidth)
{
SS_BEFORESCROLLTIP BeforeScrollTip;
LRESULT            lRet;

_fmemset(&BeforeScrollTip, '\0', sizeof(SS_BEFORESCROLLTIP));

BeforeScrollTip.fIsVertical = fIsVertical;
BeforeScrollTip.Index = Index;
BeforeScrollTip.nWidth = *lpnWidth;
lstrcpyn(BeforeScrollTip.szText, lpText, SS_SCROLLTIP_TEXTMAX);

lRet = SS_SendMsg(lpSS->lpBook, lpSS, SSM_BEFORESCROLLTIP, GetWindowID(lpSS->lpBook->hWnd),
                  (LPARAM)(LPSS_BEFORESCROLLTIP)&BeforeScrollTip);

*lpnWidth = BeforeScrollTip.nWidth;
lstrcpy(lpText, BeforeScrollTip.szText);

return (lRet);
}


BOOL SS_SendMsgBeforeColMove(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Col2,
                             SS_COORD ColDest)
{
SS_BEFORECOLMOVE BeforeColMove;
LRESULT          lRet;

_fmemset(&BeforeColMove, '\0', sizeof(SS_BEFORECOLMOVE));

SS_AdjustCellCoordsOut(lpSS, &Col, NULL);
SS_AdjustCellCoordsOut(lpSS, &Col2, NULL);
SS_AdjustCellCoordsOut(lpSS, &ColDest, NULL);

BeforeColMove.Col = Col;
BeforeColMove.Col2 = Col2;
BeforeColMove.ColDest = ColDest;
BeforeColMove.fCancel = FALSE;

lRet = SS_SendMsg(lpSS->lpBook, lpSS, SSM_BEFORECOLMOVE, GetWindowID(lpSS->lpBook->hWnd),
                  (LPARAM)(LPSS_BEFORECOLMOVE)&BeforeColMove);

return (BeforeColMove.fCancel);
}


BOOL SS_SendMsgBeforeRowMove(LPSPREADSHEET lpSS, SS_COORD Row, SS_COORD Row2,
                             SS_COORD RowDest)
{
SS_BEFOREROWMOVE BeforeRowMove;
LRESULT          lRet;

_fmemset(&BeforeRowMove, '\0', sizeof(SS_BEFOREROWMOVE));

SS_AdjustCellCoordsOut(lpSS, NULL, &Row);
SS_AdjustCellCoordsOut(lpSS, NULL, &Row2);
SS_AdjustCellCoordsOut(lpSS, NULL, &RowDest);

BeforeRowMove.Row = Row;
BeforeRowMove.Row2 = Row2;
BeforeRowMove.RowDest = RowDest;
BeforeRowMove.fCancel = FALSE;

lRet = SS_SendMsg(lpSS->lpBook, lpSS, SSM_BEFOREROWMOVE, GetWindowID(lpSS->lpBook->hWnd),
                  (LPARAM)(LPSS_BEFOREROWMOVE)&BeforeRowMove);

return (BeforeRowMove.fCancel);
}

#endif // SS_V70


HPBYTE SS_HugeBufferAlloc(HPBYTE Buffer, LPLONG BufferLen, LPVOID Value,
                          long ValueLen, LPLONG AllocLen, LPHANDLE hBuffer)
{
if (*BufferLen + ValueLen > *AllocLen)
   {
   *AllocLen += max(ValueLen, BUFFER_ALLOC_CNT);

   if (!(*hBuffer))
      {
      if (!(*hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, *AllocLen)))
         {
         *BufferLen = 0;
         return (FALSE);
         }
      }

   else
      {
      GlobalUnlock(*hBuffer);
      if (!(*hBuffer = GlobalReAlloc(*hBuffer, *AllocLen,
                                     GMEM_MOVEABLE | GMEM_ZEROINIT)))
         {
         *BufferLen = 0;
         return (FALSE);
         }
      }

   Buffer = (HPBYTE)GlobalLock(*hBuffer);
   }

if (ValueLen > 0)
   {
   if (Value)
      MemHugeCpy(&Buffer[*BufferLen], Value, ValueLen);
   else
      MemHugeSet(&Buffer[*BufferLen], '\0', (short)ValueLen);
   }

*BufferLen += ValueLen;

return (Buffer);
}


#if 0
LPBYTE SS_BufferAlloc(LPBYTE Buffer, long FAR *BufferLen, void FAR *Value,
                     long ValueLen, LPLONG AllocLen, LPHANDLE hBuffer, short nAllocInc)
{
if (*BufferLen + ValueLen > *AllocLen)
   {
   *AllocLen += max(ValueLen, nAllocInc);

   if (!(*hBuffer))
      {
      if (!(*hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, *AllocLen)))
         {
         *BufferLen = 0;
         return (FALSE);
         }
      }

   else
      {
      GlobalUnlock(*hBuffer);
      if (!(*hBuffer = GlobalReAlloc(*hBuffer, *AllocLen,
                                     GMEM_MOVEABLE | GMEM_ZEROINIT)))
         {
         *BufferLen = 0;
         return (FALSE);
         }
      }

   Buffer = (LPBYTE)GlobalLock(*hBuffer);
   }

if (ValueLen > 0)
   {
   if (Value)
      _fmemcpy(&Buffer[*BufferLen], Value, (short)ValueLen);
   else
      _fmemset(&Buffer[*BufferLen], '\0', (short)ValueLen);
   }

*BufferLen += ValueLen;

return (Buffer);
}
#endif


void SS_FreeWithHwnd(HWND hWnd)
{
LPSS_BOOK lpBook;

if (lpBook = SS_BookLock(hWnd))
   {
   SetCursor(LoadCursor(NULL, IDC_ARROW));
#if defined(_WIN64) || defined(_IA64)
   SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));
#elif WIN32
   SetClassLong(hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
#else
   SetClassWord(hWnd, GCW_HCURSOR, (WORD)LoadCursor(NULL, IDC_ARROW));
#endif

	SS_FreeSheetsWithHwnd(lpBook);

#ifndef SS_NOCHILDSCROLL
   if (lpBook->hWndVScroll)
		{
      DestroyWindow(lpBook->hWndVScroll);
      lpBook->hWndVScroll = 0;
		}

   if (lpBook->hWndHScroll)
		{
      DestroyWindow(lpBook->hWndHScroll);
      lpBook->hWndHScroll = 0;
		}
#endif

   if (lpBook->hControls)
      {
		LPSS_CONTROL lpControls;
		SS_COORD i;

      lpControls = (LPSS_CONTROL)tbGlobalLock(lpBook->hControls);
      for (i = 0; i < lpBook->ControlsCnt; i++)
         if (lpControls[i].CtrlID && IsWindow(lpControls[i].hWnd))
            DestroyWindow(lpControls[i].hWnd);
      tbGlobalUnlock(lpBook->hControls);
      tbGlobalFree(lpBook->hControls);
      lpBook->hControls = 0;
      lpBook->ControlsCnt = 0;
      lpBook->ControlsAllocCnt = 0;

      #ifdef SS_OCX
      // COleControl class destroys/recreates the spreadsheet HWND when
      // the Visible property is toggled off/on.  The controls table needs
      // to be cleared, since the child controls are being destroyed.
      lpBook->FormulaCellType.ControlID = 0;
      #endif
      }

	lpBook->hWnd = 0;
   SS_BookUnlock(hWnd);
   }

#ifdef SS_NOSUPERCLASS
{
RemoveProp(hWnd, "Xtra1");
RemoveProp(hWnd, "Xtra2");
g_hwndSSLastLocked = 0;
g_lpSSLastLocked = 0;
}
#else
SetWindowLong(hWnd, 0, 0);
#endif
}


void SS_FreeSheetsWithHwnd(LPSS_BOOK lpBook)
{
LPSPREADSHEET lpSS;
short         nSheet;

for (nSheet = 0; nSheet < lpBook->nSheetCnt; nSheet++)
	{
	lpSS = SS_BookLockSheetIndex(lpBook, nSheet);
	SS_FreeSheetWithHwnd(lpSS);
	SS_BookUnlockSheetIndex(lpBook, nSheet);
	}
}


void SS_FreeSheetWithHwnd(LPSPREADSHEET lpSS)
{
#ifdef SS_OCX
SS_COORD i;
LPSS_CELLTYPE lpCellType;
LPSS_CELL lpCell;
LPSS_COL lpCol;
LPSS_ROW lpRow;
SS_COORD j;
#ifdef SS_V80
LPSS_CONTROL lpControls;
#endif

if (lpSS->lpBook->hControls)
   {
   // COleControl class destroys/recreates the spreadsheet HWND when
   // the Visible property is toggled off/on.  The controls table needs
   // to be cleared, since the child controls are being destroyed.
#if SS_V80
   lpControls = (LPSS_CONTROL)tbGlobalLock(lpSS->lpBook->hControls);
   if( lpSS->DefaultCellType.Type == SS_TYPE_CUSTOM )
   {
      SS_CT_Unref(lpSS, &lpSS->DefaultCellType);
      if( lpSS->DefaultCellType.ControlID )
      {
         lpControls[lpSS->DefaultCellType.ControlID - SS_CONTROL_BASEID].CtrlID = 0;
         lpControls[lpSS->DefaultCellType.ControlID - SS_CONTROL_BASEID].hWnd = 0;
      }
      if( lpSS->DefaultCellType.Spec.Custom.RendererControlID )
      {
         lpControls[lpSS->DefaultCellType.Spec.Custom.RendererControlID - SS_CONTROL_BASEID].CtrlID = 0;
         lpControls[lpSS->DefaultCellType.Spec.Custom.RendererControlID - SS_CONTROL_BASEID].hWnd = 0;
      }
      lpSS->DefaultCellType.Spec.Custom.RendererControlID = 0;
   }
#endif
   lpSS->DefaultCellType.ControlID = 0;

   if (!lpSS->lpBook->fObjectBeingDestroyed)
      {
      for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
         if (lpCol = SS_LockColItem(lpSS, i))
            {
            if (lpCol->hCellType)
               {
               lpCellType = (LPSS_CELLTYPE)tbGlobalLock(lpCol->hCellType);
#if SS_V80
               if( lpCellType->Type == SS_TYPE_CUSTOM )
               {
                  SS_CT_Unref(lpSS, lpCellType);
                  if( lpCellType->ControlID )
                  {
                     lpControls[lpCellType->ControlID - SS_CONTROL_BASEID].CtrlID = 0;
                     lpControls[lpCellType->ControlID - SS_CONTROL_BASEID].hWnd = 0;
                  }
                  if( lpCellType->Spec.Custom.RendererControlID )
                  {
                     lpControls[lpCellType->Spec.Custom.RendererControlID - SS_CONTROL_BASEID].CtrlID = 0;
                     lpControls[lpCellType->Spec.Custom.RendererControlID - SS_CONTROL_BASEID].hWnd = 0;
                  }
                  lpCellType->Spec.Custom.RendererControlID = 0;
               }
#endif
               lpCellType->ControlID = 0;
               tbGlobalUnlock(lpCol->hCellType);
               }
            SS_UnlockColItem(lpSS, i);
            }

      for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
         if (lpRow = SS_LockRowItem(lpSS, i))
            {
            if (lpRow->hCellType)
               {
               lpCellType = (LPSS_CELLTYPE)tbGlobalLock(lpRow->hCellType);
#if SS_V80
               if( lpCellType->Type == SS_TYPE_CUSTOM )
               {
                  SS_CT_Unref(lpSS, lpCellType);
                  if( lpCellType->ControlID )
                  {
                     lpControls[lpCellType->ControlID - SS_CONTROL_BASEID].CtrlID = 0;
                     lpControls[lpCellType->ControlID - SS_CONTROL_BASEID].hWnd = 0;
                  }
                  if( lpCellType->Spec.Custom.RendererControlID )
                  {
                     lpControls[lpCellType->Spec.Custom.RendererControlID - SS_CONTROL_BASEID].CtrlID = 0;
                     lpControls[lpCellType->Spec.Custom.RendererControlID - SS_CONTROL_BASEID].hWnd = 0;
                  }
                  lpCellType->Spec.Custom.RendererControlID = 0;
               }
#endif
               lpCellType->ControlID = 0;
               tbGlobalUnlock(lpRow->hCellType);
               }
            for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
               if (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i))
                  {
                  if (lpCell->hCellType)
                     {
                     lpCellType = (LPSS_CELLTYPE)tbGlobalLock(lpCell->hCellType);
#if SS_V80
                     if( lpCellType->Type == SS_TYPE_CUSTOM )
                     {
                        SS_CT_Unref(lpSS, lpCellType);
                        if( lpCellType->ControlID )
                        {
                           lpControls[lpCellType->ControlID - SS_CONTROL_BASEID].CtrlID = 0;
                           lpControls[lpCellType->ControlID - SS_CONTROL_BASEID].hWnd = 0;
                        }
                        if( lpCellType->Spec.Custom.RendererControlID )
                        {
                           lpControls[lpCellType->Spec.Custom.RendererControlID - SS_CONTROL_BASEID].CtrlID = 0;
                           lpControls[lpCellType->Spec.Custom.RendererControlID - SS_CONTROL_BASEID].hWnd = 0;
                        }
                        lpCellType->Spec.Custom.RendererControlID = 0;
                     }
#endif
                     lpCellType->ControlID = 0;
                     tbGlobalUnlock(lpRow->hCellType);
                     }
                  SS_UnlockCellItem(lpSS, j, i);
                  }
            SS_UnlockRowItem(lpSS, i);
            }
      }
#if SS_V80
   tbGlobalUnlock(lpSS->lpBook->hControls);
#endif
   }
#endif
}


void SS_FreePostHwnd(TBGLOBALHANDLE hBook)
{
LPSS_BOOK lpBook;

if (lpBook = (LPSS_BOOK)tbGlobalLock(hBook))
   {
   SS_ClearAll(lpBook);

   DestroyCursor(lpBook->hCursorPointer);
   DestroyCursor(lpBook->hCursorResizeH);
   DestroyCursor(lpBook->hCursorResizeV);

   DeleteObject(lpBook->hBrushLines);
   DeleteObject(lpBook->hBrushLines2);
   DeleteObject(lpBook->hBitmapLines);
   DeleteObject(lpBook->hBitmapLines2);
   DeleteObject(lpBook->hBitmapComboBtn);

#ifdef SS_UTP
   if (lpBook->hBitmapScrollArrowTopDef)
      DeleteObject(lpBook->hBitmapScrollArrowTopDef);

   if (lpBook->hBitmapScrollArrowBottomDef)
      DeleteObject(lpBook->hBitmapScrollArrowBottomDef);

   if (lpBook->hBitmapScrollArrowLeftDef)
      DeleteObject(lpBook->hBitmapScrollArrowLeftDef);

   if (lpBook->hBitmapScrollArrowRightDef)
      DeleteObject(lpBook->hBitmapScrollArrowRightDef);
#endif

#ifdef SS_OLDCALC
   SS_FreeCalcFunctions(lpSS);
#endif

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
   CalcFreeBook(&lpBook->CalcInfo);
#endif

   tbGlobalUnlock(hBook);
   }

if (hBook)
	tbGlobalFree(hBook);

/*
MemPoolShrink(tbOmemPoolRow);
MemPoolShrink(tbOmemPoolCol);
MemPoolShrink(tbOmemPoolCell);
MemPoolShrink(tbStringPool);
*/

/*
{
TCHAR Buffer[300];

wsprintf(Buffer, "Row  = %ld  %ld\nCol  = %ld  %ld\nCell = %ld  %ld\nStr  = %ld  %ld",
         MemPoolCount(tbOmemPoolRow),
         MemPoolSize(tbOmemPoolRow),
         MemPoolCount(tbOmemPoolCol),
         MemPoolSize(tbOmemPoolCol),
         MemPoolCount(tbOmemPoolCell),
         MemPoolSize(tbOmemPoolCell),
         MemPoolCount(tbStringPool),
         MemPoolSize(tbStringPool));

MessageBox(hWnd, Buffer, "Test", MB_OK);
}
*/
}


void SS_ClearAll(LPSS_BOOK lpBook)
{
#if SS_V80
if( lpBook->CustCellTypes.nElemCnt > 0 )
{  // remove custom celltypes
   CT_HANDLE hCT = CustCellTypeFirst(&lpBook->CustCellTypes);
   CT_HANDLE hCTPrev = hCT;
   while( hCT != NULL )
   {
      LPCTSTR lpszName = CustCellTypeLockText(hCT);
      long lRefs = CustCellTypeGetRefCnt(hCT);
      hCT = CustCellTypeNext(&lpBook->CustCellTypes, lpszName);
      CustCellTypeUnlockText(hCTPrev);
      SS_TypeControlRemove(hCTPrev, &lpBook->hControls, &lpBook->ControlsAllocCnt,
                              &lpBook->ControlsCnt);
      for( ; lRefs > 1; lRefs-- )
         CustCellTypeDestroy(hCTPrev);
      // last ref released by CustCellTypeRemove
      CustCellTypeRemove(&lpBook->CustCellTypes, hCTPrev);
      hCTPrev = hCT;
   }
}
#endif
SS_SetSheetCount(lpBook, 0);

if (lpBook->hControls)
   {
   LPSS_CONTROL Controls = (LPSS_CONTROL)tbGlobalLock(lpBook->hControls);
	SS_COORD     i;

   for (i = 0; i < lpBook->ControlsCnt; i++)
      if (Controls[i].CtrlID)
         DestroyWindow(Controls[i].hWnd);

   tbGlobalUnlock(lpBook->hControls);
   tbGlobalFree(lpBook->hControls);
   lpBook->hControls = 0;
   lpBook->ControlsCnt = 0;
   lpBook->ControlsAllocCnt = 0;
   }

if (lpBook->hBuffer)
   {
   tbGlobalFree(lpBook->hBuffer);
   lpBook->hBuffer = 0;                     // 9333
   }

// This will delete all of the cursors
SS_PicListFree(lpBook);

if (lpBook->hXtra)
   {
   GlobalFree(lpBook->hXtra);
   lpBook->hXtra = 0;
   lpBook->lXtraLen = 0;
   lpBook->bXtraVer = 0;
   }

#ifdef SS_V35
if (lpBook->hMonthLongNames)
   {
   tbGlobalFree(lpBook->hMonthLongNames);
   lpBook->hMonthLongNames = 0;                   // 9333
   }   
if (lpBook->hMonthShortNames)
   {
   tbGlobalFree(lpBook->hMonthShortNames);
   lpBook->hMonthShortNames = 0;                   // 9333
   }
if (lpBook->hDayLongNames)
   {
   tbGlobalFree(lpBook->hDayLongNames);
   lpBook->hDayLongNames = 0;                     // 9333
   }
if (lpBook->hDayShortNames)
   {
   tbGlobalFree(lpBook->hDayShortNames);
   lpBook->hDayShortNames = 0;                   // 9333
   }
if (lpBook->hOkText)
   {
   tbGlobalFree(lpBook->hOkText);
   lpBook->hOkText = 0;                          // 9333
   }
if (lpBook->hCancelText)
   {
   tbGlobalFree(lpBook->hCancelText);
   lpBook->hCancelText = 0;                      // 9333
   }
#endif

if (lpBook->hPrintAbortMsg)
	{
	tbGlobalFree(lpBook->hPrintAbortMsg);
	lpBook->hPrintAbortMsg = 0;
	}

if (lpBook->hPrintJobName)
	{
	tbGlobalFree(lpBook->hPrintJobName);
	lpBook->hPrintJobName = 0;
	}

lpBook->EditModeOn = FALSE;
_fmemset(&lpBook->CurVisCell, '\0', sizeof(SS_CELLCOORD));
lpBook->wMode = SS_MODE_NONE;
lpBook->ResizeOffset = 0;
lpBook->ResizeCurrentPos = 0;
lpBook->ResizeStartPos = 0;
lpBook->ResizeMinPos = 0;
lpBook->ResizeCoord = 0;
lpBook->EditModeTurningOff = FALSE;
lpBook->bBackColorStyle = SS_BACKCOLORSTYLE_OVERGRID;

#ifdef SS_V80
SS_AlphaBlendFree(lpBook);
#endif // SS_V80

}


void SS_ClearSheet(LPSPREADSHEET lpSS, BOOL bFreeCalcInfo)
{
SS_CellEditModeOff(lpSS, 0);

SS_ClearUndoBuffer(lpSS);

SS_ResetBlock(lpSS);
SSx_FreeCells(lpSS);

#if !defined(SS_NOCALC) && defined(SS_OLDCALC)
SS_FreeCalcList(&lpSS->CalcAllDependents);

if (lpSS->hCalcTable)
   tbGlobalFree(lpSS->hCalcTable);
#endif

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
if (bFreeCalcInfo)
  CalcFreeSheet(&lpSS->CalcInfo);
#endif

SSx_FreeData(&lpSS->DefaultData);
SS_FreeBorder(lpSS, &lpSS->hBorder);

#ifdef SS_V35
if (lpSS->hCellNote)
   {
   tbGlobalFree(lpSS->hCellNote);
   lpSS->hCellNote = 0;                       // 9333
   }
#endif // SS_V35

lpSS->HighlightOn = FALSE;
lpSS->FreezeHighlight = FALSE;
lpSS->NoEraseBkgnd = FALSE;
lpSS->Row.UL = 0;
lpSS->Col.UL = 0;
lpSS->Row.LR = 0;
lpSS->Col.LR = 0;
lpSS->Row.AllocCnt = 0;
lpSS->Col.AllocCnt = 0;
lpSS->Row.DataCnt = 0;
lpSS->Col.DataCnt = 0;
lpSS->DefaultData.bDataType = 0;
#ifdef SS_OLDCALC
lpSS->CalcTableCnt = 0;
lpSS->CalcTableAllocCnt = 0;
lpSS->hCalcTable = 0;            
_fmemset(&lpSS->CalcAllDependents, '\0', sizeof(SS_CALCLIST));
_fmemset(&lpSS->CurCalcCell, '\0', sizeof(SS_CELLCOORD));
#endif
_fmemset(&lpSS->BlockCellUL, '\0', sizeof(SS_CELLCOORD));
_fmemset(&lpSS->BlockCellLR, '\0', sizeof(SS_CELLCOORD));
lpSS->Col.BlockCellCurrentPos = 0;
lpSS->Row.BlockCellCurrentPos = 0;

lpSS->Col.LastRefCoord.Coord.Col = -1;
lpSS->Row.LastRefCoord.Coord.Row = -1;
lpSS->LastCell.Coord.Col = -1;
lpSS->LastCell.Coord.Row = -1;
lpSS->Col.lHeaderDisplayIndex = -1;
lpSS->Row.lHeaderDisplayIndex = -1;

lpSS->Col.HeaderCnt = 0;
lpSS->Row.HeaderCnt = 0;
lpSS->Row.CurAt = 1;
lpSS->Col.CurAt = 1;

if (lpSS->Col.hCellSizeList)
	{
   tbGlobalFree(lpSS->Col.hCellSizeList);
   lpSS->Col.lCellSizeListAllocCnt = 0;
   lpSS->Col.lCellSizeListCnt = 0;
   lpSS->Col.hCellSizeList = 0;
	}

if (lpSS->Row.hCellSizeList)
	{
   tbGlobalFree(lpSS->Row.hCellSizeList);
   lpSS->Row.lCellSizeListAllocCnt = 0;
   lpSS->Row.lCellSizeListCnt = 0;
   lpSS->Row.hCellSizeList = 0;
	}

SS_SetPrintOptions(lpSS, NULL, NULL, 0);

#ifdef SS_V40
SS_FreeSpanTable(lpSS);
SS_SpanFreePaintList(lpSS);
lpSS->lColHeadersUserSortIndex = -1;
if (lpSS->hCellTag)
{
  tbGlobalFree(lpSS->hCellTag);
  lpSS->hCellTag = 0;
}
#endif // SS_V40

#ifdef SS_V70
if (lpSS->hSheetName)
	{
	tbGlobalFree(lpSS->hSheetName);
	lpSS->hSheetName = 0;
	}

lpSS->fVisible = TRUE;
#if SS_V70 && FP_ADB
SSOcxFreeSheetOleBinding(lpSS); // 18029 -scl
#endif
#endif // SS_V70
}


void SS_DeAllocCellType(LPSPREADSHEET lpSS, TBGLOBALHANDLE hCellType)
{
LPSS_CELLTYPE CellType;

if (hCellType)
   {
   CellType = (LPSS_CELLTYPE)tbGlobalLock(hCellType);

   switch (CellType->Type)
      {
      case SS_TYPE_COMBOBOX:
         if (CellType->Spec.ComboBox.hItems)
            tbGlobalFree(CellType->Spec.ComboBox.hItems);

         break;

      case SS_TYPE_PIC:
         if (CellType->Spec.Pic.hMask)
            tbGlobalFree(CellType->Spec.Pic.hMask);

         break;

      case SS_TYPE_PICTURE:
         #ifdef SS_VB
         SSVB_FreePic(CellType->Spec.ViewPict.hPic);
         #endif

         #ifdef SS_OCX
         SSOCX_FreePict((LPPICTURE)CellType->Spec.ViewPict.lpPict);
         #endif

         if (CellType->Spec.ViewPict.hPictName &&
             !(CellType->Style & VPS_HANDLE))
            tbGlobalFree(CellType->Spec.ViewPict.hPictName);

         if (CellType->Spec.ViewPict.hPictName &&
             (CellType->Style & VPS_HANDLE) &&
             CellType->Spec.ViewPict.fDeleteHandle)
            SS_PicDeref(lpSS->lpBook, (HANDLE)CellType->Spec.ViewPict.hPictName);

         /*
         if (CellType->ControlID && lpSS->hControls)
            {
            Controls = (LPSS_CONTROL)tbGlobalLock(lpSS->hControls);

            DestroyWindow(SS_GetControlhWnd(lpSS, CellType->ControlID));
            Controls[CellType->ControlID - SS_CONTROL_BASEID].CtrlID = 0;
            Controls[CellType->ControlID - SS_CONTROL_BASEID].hWnd = 0;

            tbGlobalUnlock(lpSS->hControls);
            }
         */

         break;

      case SS_TYPE_BUTTON:
         if (CellType->Spec.Button.hText)
            GlobalFree(CellType->Spec.Button.hText);

         #ifdef SS_VB
         SSVB_FreePic(CellType->Spec.Button.hPic);
         SSVB_FreePic(CellType->Spec.Button.hPicDown);
         #endif

         #ifdef SS_OCX
         SSOCX_FreePict((LPPICTURE)CellType->Spec.Button.lpPict);
         SSOCX_FreePict((LPPICTURE)CellType->Spec.Button.lpPictDown);
         #endif

         if (CellType->Spec.Button.hPictName &&
             !(CellType->Spec.Button.nPictureType & SUPERBTN_PICT_HANDLE))
            GlobalFree(CellType->Spec.Button.hPictName);

         if (CellType->Spec.Button.hPictDownName &&
             !(CellType->Spec.Button.nPictureDownType & SUPERBTN_PICT_HANDLE))
            GlobalFree(CellType->Spec.Button.hPictDownName);

         if (CellType->Spec.Button.hPictName &&
             (CellType->Spec.Button.nPictureType & SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.Button.fDelHandle)
            SS_PicDeref(lpSS->lpBook, (HANDLE)CellType->Spec.Button.hPictName);

         if (CellType->Spec.Button.hPictDownName &&
             (CellType->Spec.Button.nPictureDownType & SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.Button.fDelDownHandle)
            SS_PicDeref(lpSS->lpBook, (HANDLE)CellType->Spec.Button.hPictDownName);

         break;

      case SS_TYPE_CHECKBOX:
         if (CellType->Spec.CheckBox.hText)
            GlobalFree(CellType->Spec.CheckBox.hText);

         #ifdef SS_VB
         SSVB_FreePic(CellType->Spec.CheckBox.hPicUp);
         SSVB_FreePic(CellType->Spec.CheckBox.hPicDown);
         SSVB_FreePic(CellType->Spec.CheckBox.hPicFocusUp);
         SSVB_FreePic(CellType->Spec.CheckBox.hPicFocusDown);
         SSVB_FreePic(CellType->Spec.CheckBox.hPicGray);
         SSVB_FreePic(CellType->Spec.CheckBox.hPicFocusGray);
         #endif

         #ifdef SS_OCX
         SSOCX_FreePict((LPPICTURE)CellType->Spec.CheckBox.lpPictUp);
         SSOCX_FreePict((LPPICTURE)CellType->Spec.CheckBox.lpPictDown);
         SSOCX_FreePict((LPPICTURE)CellType->Spec.CheckBox.lpPictFocusUp);
         SSOCX_FreePict((LPPICTURE)CellType->Spec.CheckBox.lpPictFocusDown);
         SSOCX_FreePict((LPPICTURE)CellType->Spec.CheckBox.lpPictGray);
         SSOCX_FreePict((LPPICTURE)CellType->Spec.CheckBox.lpPictFocusGray);
         #endif

         if (CellType->Spec.CheckBox.hPictUpName &&
             !(CellType->Spec.CheckBox.bPictUpType & BT_HANDLE))
            tbGlobalFree(CellType->Spec.CheckBox.hPictUpName);

         if (CellType->Spec.CheckBox.hPictDownName &&
             !(CellType->Spec.CheckBox.bPictDownType & BT_HANDLE))
            tbGlobalFree(CellType->Spec.CheckBox.hPictDownName);

         if (CellType->Spec.CheckBox.hPictFocusUpName &&
             !(CellType->Spec.CheckBox.bPictFocusUpType & BT_HANDLE))
            tbGlobalFree(CellType->Spec.CheckBox.hPictFocusUpName);

         if (CellType->Spec.CheckBox.hPictFocusDownName &&
             !(CellType->Spec.CheckBox.bPictFocusDownType & BT_HANDLE))
            tbGlobalFree(CellType->Spec.CheckBox.hPictFocusDownName);

         if (CellType->Spec.CheckBox.hPictGrayName &&
             !(CellType->Spec.CheckBox.bPictGrayType & BT_HANDLE))
            tbGlobalFree(CellType->Spec.CheckBox.hPictGrayName);

         if (CellType->Spec.CheckBox.hPictFocusGrayName &&
             !(CellType->Spec.CheckBox.bPictFocusGrayType & BT_HANDLE))
            tbGlobalFree(CellType->Spec.CheckBox.hPictFocusGrayName);

         if (CellType->Spec.CheckBox.hPictUpName &&
             (CellType->Spec.CheckBox.bPictUpType & SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.CheckBox.fDelUpHandle)
            SS_PicDeref(lpSS->lpBook, (HANDLE)CellType->Spec.CheckBox.hPictUpName);

         if (CellType->Spec.CheckBox.hPictDownName &&
             (CellType->Spec.CheckBox.bPictDownType & SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.CheckBox.fDelDownHandle)
            SS_PicDeref(lpSS->lpBook, (HANDLE)CellType->Spec.CheckBox.hPictDownName);

         if (CellType->Spec.CheckBox.hPictFocusUpName &&
             (CellType->Spec.CheckBox.bPictFocusUpType &
              SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.CheckBox.fDelFocusUpHandle)
            SS_PicDeref(lpSS->lpBook,
                        (HANDLE)CellType->Spec.CheckBox.hPictFocusUpName);

         if (CellType->Spec.CheckBox.hPictFocusDownName &&
             (CellType->Spec.CheckBox.bPictFocusDownType &
              SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.CheckBox.fDelFocusDownHandle)
            SS_PicDeref(lpSS->lpBook,
                        (HANDLE)CellType->Spec.CheckBox.hPictFocusDownName);

         if (CellType->Spec.CheckBox.hPictGrayName &&
             (CellType->Spec.CheckBox.bPictGrayType &
              SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.CheckBox.fDelGrayHandle)
            SS_PicDeref(lpSS->lpBook,
                        (HANDLE)CellType->Spec.CheckBox.hPictGrayName);

         if (CellType->Spec.CheckBox.hPictFocusGrayName &&
             (CellType->Spec.CheckBox.bPictFocusGrayType &
              SUPERBTN_PICT_HANDLE) &&
             CellType->Spec.CheckBox.fDelFocusGrayHandle)
            SS_PicDeref(lpSS->lpBook,
                        (HANDLE)CellType->Spec.CheckBox.hPictFocusGrayName);

         break;

#if SS_V80
      case SS_TYPE_CUSTOM:
         SS_CT_Unref(lpSS, CellType);
         if( CellType->Spec.Custom.hName )
         {
            GlobalFree(CellType->Spec.Custom.hName);
            CellType->Spec.Custom.hName = NULL;
         }
         SS_FreeCTValue(&CellType->Spec.Custom.ItemData);
#endif
         break;
      }

   tbGlobalUnlock(hCellType);
   tbGlobalFree(hCellType);
   }
}


//--------------------------------------------------------------------
//
//  The SSVB_FreePic() function frees a Visual Basic picture handle.
//

#ifdef SS_VB
void SSVB_FreePic(HANDLE hPic)
{
if (hPic)
   VBFreePic(hPic);
}
#endif


//--------------------------------------------------------------------
//
//  The SSOCX_FreePict() function releases an OCX picture interface.
//

#ifdef SS_OCX
void SSOCX_FreePict(LPPICTURE lpPict)
{
if (lpPict)
   lpPict->lpVtbl->Release(lpPict);
}
#endif


// BJO SCS7424 1998Sep29
// When SmartHeap allocates huge (>= 64K) memory blocks, the memory
// block begins 20 bytes into a 64K page of memory.  The 16-bit MSVC
// optimized code generator assumes that huge arrays of structures
// are aligned so that no structure is split by a 64K boundary.
// This assumption allows the optimized code generator to use far
// pointer arithematic (instead of huge pointer arithematic) when
// evaluating the structure member operators ("." and "->").  The
// SS_ALLOCITEM structure contains 8 bytes.  Thus, in 16-bits the
// array must be offset by 4 bytes to obtain the correct alignment.

#ifdef WIN32
  #define tbGlobalAllocHuge(uFlags,ulSize) tbGlobalAlloc(uFlags,ulSize)
  #define tbGlobalReAllocHuge(hMem,ulSize,uFlags) tbGlobalReAlloc(hMem,ulSize,uFlags)
  #define tbGlobalLockHuge(hMem) tbGlobalLock(hMem)
#else
  #define ARRAY1_OFFSET 4
  #define tbGlobalAllocHuge(uFlags,ulSize) tbGlobalAlloc(uFlags,(ulSize)+ARRAY1_OFFSET)
  #define tbGlobalReAllocHuge(hMem,ulSize,uFlags) tbGlobalReAlloc(hMem,(ulSize)+ARRAY1_OFFSET,uFlags)
  #define tbGlobalLockHuge(hMem) ((LPBYTE)tbGlobalLock(hMem)+ARRAY1_OFFSET)
#endif

TBGLOBALHANDLE SS_AllocItems(LPSPREADSHEET lpSS, OMEM_POOL OmemPool,
                             LPSS_ALLOCITEM lpAllocItem, SS_COORD wItemNum,
                             WORD wItemSize, TBGLOBALHANDLE hItem,
                             SS_COORD wMaxItems, LPBOOL lpfIsNew)
{
LPSS_ALLOCITEM   lpItems;
LPTBGLOBALHANDLE lpItems2;
MEMSIZE          wItemAt;
MEMSIZE          wItemCnt;
WORD             wItem2At;
WORD             wItem2Cnt;

if (lpfIsNew)
   *lpfIsNew = FALSE;

if (wItemNum + 1 > wMaxItems)
   return ((TBGLOBALHANDLE)-1L);

wItemAt = CALCALLOCITEM(wItemNum);
wItemCnt = max(wItemAt + 1, lpAllocItem->wItemCnt + ARRAY1_ALLOC_INC);

wItemCnt = wItemAt + 1;
if (wItemCnt > lpAllocItem->wItemCnt)
   wItemCnt = wItemCnt + ARRAY1_ALLOC_INC;

if (wItemCnt > lpAllocItem->wItemCnt)
   {
   if (lpAllocItem->wItemCnt == 0)
      {
      if (!(lpAllocItem->hItems = tbGlobalAllocHuge(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                                    (long)(wItemCnt *
                                                    sizeof(SS_ALLOCITEM)))))
         return ((TBGLOBALHANDLE)-1);
      }

   else
      {
      if (!(lpAllocItem->hItems = tbGlobalReAllocHuge(lpAllocItem->hItems,
                                  (long)(wItemCnt * sizeof(SS_ALLOCITEM)),
                                  GMEM_MOVEABLE | GMEM_ZEROINIT)))
         return ((TBGLOBALHANDLE)-1);
      }

   lpAllocItem->wItemCnt = wItemCnt;
   }

/******************************
* Allocate 2nd Array of items
******************************/

lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpAllocItem->hItems);

wItem2At = CALCALLOCITEMREM(wItemNum);

wItem2Cnt = wItem2At + 1;
if (wItem2Cnt > lpItems[wItemAt].wItemCnt)
   wItem2Cnt = min(wItem2Cnt + ARRAY2_ALLOC_INC, SS_ALLOCITEMCNT);

if (wItem2Cnt > lpItems[wItemAt].wItemCnt)
   {
   if (lpItems[wItemAt].wItemCnt == 0)
      {
      if (!(lpItems[wItemAt].hItems = tbGlobalAlloc(GMEM_MOVEABLE |
                                                    GMEM_ZEROINIT,
                                                    (long)(wItem2Cnt *
                                                    sizeof(TBGLOBALHANDLE)))))
         {
         tbGlobalUnlock(lpAllocItem->hItems);
         return ((TBGLOBALHANDLE)-1);
         }
      }

   else
      {
      if (!(lpItems[wItemAt].hItems = tbGlobalReAlloc(lpItems[wItemAt].hItems,
                                      (long)(wItem2Cnt *
                                      sizeof(TBGLOBALHANDLE)),
                                      GMEM_MOVEABLE | GMEM_ZEROINIT)))
         {
         tbGlobalUnlock(lpAllocItem->hItems);
         return ((TBGLOBALHANDLE)-1);
         }

      lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[wItemAt].hItems);
      _fmemset(&lpItems2[lpItems[wItemAt].wItemCnt], '\0',
               sizeof(TBGLOBALHANDLE) * (wItem2Cnt -
               (WORD)lpItems[wItemAt].wItemCnt));
      tbGlobalUnlock(lpItems[wItemAt].hItems);
      }

   lpItems[wItemAt].wItemCnt = wItem2Cnt;
   }

lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[wItemAt].hItems);

if (!lpItems2[wItem2At])
   {
   if (hItem)
      lpItems2[wItem2At] = hItem;
#ifdef SS_NO_USE_SH
   else if (hItem = (TBGLOBALHANDLE)tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, wItemSize))
#else
   else if (hItem = (TBGLOBALHANDLE)MemAllocFS(OmemPool))
#endif
      {
		LPBYTE lpItem = (LPBYTE)tbGlobalLock(hItem);
      _fmemset(lpItem, '\0', wItemSize);
		tbGlobalUnlock(hItem);
      lpItems2[wItem2At] = hItem;
      }
   else
      hItem = (TBGLOBALHANDLE)-1;

   if (lpfIsNew)
      *lpfIsNew = TRUE;
   }
else
   hItem = lpItems2[wItem2At];

tbGlobalUnlock(lpItems[wItemAt].hItems);
tbGlobalUnlock(lpAllocItem->hItems);

return (hItem);
}


BOOL SS_DeleteItem(LPSPREADSHEET lpSS, LPSS_ALLOCITEM lpAllocItem,
                   SS_COORD wItemNum, SS_COORD wMaxItems)
{
LPSS_ALLOCITEM   lpItems;
LPTBGLOBALHANDLE lpItems2;
MEMSIZE          wItemAt;
WORD             wItem2At;
BOOL             fRet = 0;

if (wItemNum + 1 > wMaxItems)
   return (FALSE);

wItemAt = CALCALLOCITEM(wItemNum);

if (wItemAt < lpAllocItem->wItemCnt)
   {
   lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpAllocItem->hItems);
   wItem2At = CALCALLOCITEMREM(wItemNum);

   if (wItem2At < lpItems[wItemAt].wItemCnt)
      {
      if (lpItems[wItemAt].hItems)
         {
         lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[wItemAt].hItems);

         if (lpItems2[wItem2At])
            {
#ifdef SS_NO_USE_SH
            tbGlobalFree((TBGLOBALHANDLE)lpItems2[wItem2At]);
#else
            MemFreeFS((LPVOID)lpItems2[wItem2At]);
#endif
            lpItems2[wItem2At] = 0;
            }

         tbGlobalUnlock(lpItems[wItemAt].hItems);
         fRet = TRUE;
         }
      }

   tbGlobalUnlock(lpAllocItem->hItems);
   }

return (fRet);
}


void SS_FreeAllocItem(LPSS_ALLOCITEM lpAllocItem)
{
LPSS_ALLOCITEM   lpItems;
LPTBGLOBALHANDLE lpItems2;
MEMSIZE          i;
WORD             j;

if (lpAllocItem->hItems)
   {
   lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpAllocItem->hItems);

   for (i = 0; i < lpAllocItem->wItemCnt; i++)
      if (lpItems[i].hItems)
         {
         lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[i].hItems);

         for (j = 0; j < lpItems[i].wItemCnt; j++)
            if (lpItems2[j])
#ifdef SS_NO_USE_SH
               tbGlobalFree((TBGLOBALHANDLE)lpItems2[j]);
#else
               MemFreeFS((LPVOID)lpItems2[j]);
#endif

         tbGlobalUnlock(lpItems[i].hItems);
         tbGlobalFree(lpItems[i].hItems);
         }

   tbGlobalUnlock(lpAllocItem->hItems);
   tbGlobalFree(lpAllocItem->hItems);

   lpAllocItem->hItems = 0;
   lpAllocItem->wItemCnt = 0;
   }
}


void SS_FreeRowItem(LPSPREADSHEET lpSS, LPSS_ALLOCITEM lpAllocItem)
{
LPSS_ALLOCITEM   lpItems;
LPTBGLOBALHANDLE lpItems2;
LPSS_ROW         lpRow;
MEMSIZE          i;
WORD             j;

if (lpAllocItem->hItems)
   {
   lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpAllocItem->hItems);

   for (i = 0; i < lpAllocItem->wItemCnt; i++)
      if (lpItems[i].hItems)
         {
         lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[i].hItems);

         for (j = 0; j < lpItems[i].wItemCnt; j++)
            if (lpItems2[j] && (lpRow = (LPSS_ROW)tbGlobalLock(lpItems2[j])))
               {
               SS_FreeAllocItem(&lpRow->Cells);
               tbGlobalUnlock(lpItems2[j]);
               }

         tbGlobalUnlock(lpItems[i].hItems);
         tbGlobalFree(lpItems[i].hItems);
         }

   tbGlobalUnlock(lpAllocItem->hItems);
   tbGlobalFree(lpAllocItem->hItems);

   lpAllocItem->hItems = 0;
   lpAllocItem->wItemCnt = 0;
   }
}


SS_COORD SS_GetRowAllocCnt(LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ALLOCITEM    lpAllocItem;
LPSS_ALLOCITEM    lpItems;
LPSS_ROW          lpRow;
SS_COORD          RowAllocCnt = 0;

if (lpRow = SS_AllocLockRow(lpSS, Row))
   {
   lpAllocItem = &lpRow->Cells;

   if (lpAllocItem->wItemCnt)
      {
      RowAllocCnt = (lpAllocItem->wItemCnt - 1) * SS_ALLOCITEMCNT;

      lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpAllocItem->hItems);
      RowAllocCnt += lpItems[lpAllocItem->wItemCnt - 1].wItemCnt;
      tbGlobalUnlock(lpAllocItem->hItems);
      }
   }

return (RowAllocCnt);
}


TBGLOBALHANDLE SS_GetItem(LPSPREADSHEET lpSS, LPSS_ALLOCITEM lpAllocItem,
                          register SS_COORD wItemNum, SS_COORD wMaxItems)
{
LPSS_ALLOCITEM    lpItems;
LPTBGLOBALHANDLE  lpItems2;
TBGLOBALHANDLE    hItem = 0;
register MEMSIZE  wItemAt;
register WORD     wItem2At;

if (wItemNum + 1 > wMaxItems)
   return ((TBGLOBALHANDLE)-1L);

wItemAt = CALCALLOCITEM(wItemNum);

if (wItemAt < lpAllocItem->wItemCnt)
   {
   lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpAllocItem->hItems);
   wItem2At = CALCALLOCITEMREM(wItemNum);

   if (wItem2At < lpItems[wItemAt].wItemCnt)
      {
      lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[wItemAt].hItems);
      hItem = lpItems2[wItem2At];
      tbGlobalUnlock(lpItems[wItemAt].hItems);
      }

   tbGlobalUnlock(lpAllocItem->hItems);
   }

return (hItem);
}


LPSS_COL SS_LockColItem(LPSPREADSHEET lpSS, SS_COORD dCol)
{
TBGLOBALHANDLE hCol;
LPSS_COL       lpCol = NULL;

if (lpSS->Col.LastRefCoord.Coord.Col == dCol)
   lpCol = (LPSS_COL)tbGlobalLock(lpSS->Col.LastRefCoord.hItem);

else if ((hCol = SS_GetItem(lpSS, &lpSS->Col.Items, dCol,
                            SS_GetColCnt(lpSS))) && (LPARAM)hCol != (LPARAM)-1L)
   {
   lpSS->Col.LastRefCoord.Coord.Col = dCol;
   lpSS->Col.LastRefCoord.hItem = hCol;
   lpCol = (LPSS_COL)tbGlobalLock(hCol);
   }

return (lpCol);
}


LPSS_COL SS_AllocLockCol(LPSPREADSHEET lpSS, SS_COORD dCol)
{
TBGLOBALHANDLE hCol;
LPSS_COL       lpCol = NULL;

if (lpSS->Col.LastRefCoord.Coord.Col == dCol && lpSS->Col.LastRefCoord.hItem)
   lpCol = (LPSS_COL)tbGlobalLock(lpSS->Col.LastRefCoord.hItem);

else if ((hCol = SS_AllocCol(lpSS, dCol)) && (LPARAM)hCol != (LPARAM)-1L)
   lpCol = (LPSS_COL)tbGlobalLock(hCol);

return (lpCol);
}


#if 0
BOOL SS_UnlockColItem(lpSS, dCol)

LPSPREADSHEET  lpSS;
SS_COORD       dCol;
{
TBGLOBALHANDLE hCol;

if (lpSS->Col.LastRefCoord.Coord.Col == dCol)
   if (lpSS->Col.LastRefCoord.hItem)
      return (tbGlobalUnlock(lpSS->Col.LastRefCoord.hItem));
   else
      return (0);

if (hCol = SS_GetItem(lpSS, &lpSS->Col.Items, dCol,
                      SS_GetColCnt(lpSS)))
   {
   tbGlobalUnlock(hCol);
   return (TRUE);
   }

return (FALSE);
}
#endif


LPSS_ROW SS_LockRowItem(LPSPREADSHEET lpSS, SS_COORD dRow)
{
TBGLOBALHANDLE hRow;
LPSS_ROW       lpRow = NULL;

if (dRow == -1)
   return (NULL);

if (lpSS->Row.LastRefCoord.Coord.Row == dRow)
   lpRow = (LPSS_ROW)tbGlobalLock(lpSS->Row.LastRefCoord.hItem);

else if ((hRow = SS_GetItem(lpSS, &lpSS->Row.Items, dRow,
                            SS_GetRowCnt(lpSS))) && (LPARAM)hRow != (LPARAM)-1L)
   {
   lpSS->Row.LastRefCoord.Coord.Row = dRow;
   lpSS->Row.LastRefCoord.hItem = hRow;
   lpRow = (LPSS_ROW)tbGlobalLock(hRow);
   }

return (lpRow);
}


LPSS_ROW SS_AllocLockRow(LPSPREADSHEET lpSS, SS_COORD dRow)
{
TBGLOBALHANDLE hRow;
LPSS_ROW       lpRow = NULL;

if (lpSS->Row.LastRefCoord.Coord.Row == dRow && lpSS->Row.LastRefCoord.hItem)
   lpRow = (LPSS_ROW)tbGlobalLock(lpSS->Row.LastRefCoord.hItem);

else if ((hRow = SS_AllocRow(lpSS, dRow)) && (LPARAM)hRow != (LPARAM)-1L)
   lpRow = (LPSS_ROW)tbGlobalLock(hRow);

return (lpRow);
}


#if 0
BOOL SS_UnlockRowItem(lpSS, dRow)

LPSPREADSHEET  lpSS;
SS_COORD       dRow;
{
TBGLOBALHANDLE hRow;

if (lpSS->Row.LastRefCoord.Coord.Row == dRow)
   if (lpSS->Row.LastRefCoord.hItem)
      return (tbGlobalUnlock(lpSS->Row.LastRefCoord.hItem));
   else
      return (0);

if (hRow = SS_GetItem(lpSS, &lpSS->Row.Items, dRow,
                      SS_GetRowCnt(lpSS)))
   {
   tbGlobalUnlock(hRow);
   return (TRUE);
   }

return (FALSE);
}
#endif


LPSS_CELL SS_LockCellItem(LPSPREADSHEET lpSS, SS_COORD dCol, SS_COORD dRow)
{
return (SSx_LockCellItem(lpSS, NULL, dCol, dRow));
}


LPSS_CELL SSx_LockCellItem(LPSPREADSHEET lpSS, LPSS_ROW lpRow,
                           SS_COORD dCol, SS_COORD dRow)
{
TBGLOBALHANDLE hCell;
LPSS_CELL      lpCell = NULL;
LPSS_ROW       lpRowOld = lpRow;

if (dCol == SS_ALLCOLS || dRow == SS_ALLROWS)
   return (NULL);

if (lpSS->LastCell.Coord.Row == dRow && lpSS->LastCell.Coord.Col == dCol)
   lpCell = (LPSS_CELL)tbGlobalLock(lpSS->LastCell.hItem);

else if (lpRow || (lpRow = SS_LockRowItem(lpSS, dRow)))
   {
   if ((hCell = SS_GetItem(lpSS, &lpRow->Cells, dCol, SS_GetColCnt(lpSS))) &&
                           (LPARAM)hCell != (LPARAM)-1L)
      {
      lpSS->LastCell.Coord.Row = dRow;
      lpSS->LastCell.Coord.Col = dCol;
      lpSS->LastCell.hItem = hCell;

      lpCell = (LPSS_CELL)tbGlobalLock(hCell);
      }

   if (!lpRowOld)
      SS_UnlockRowItem(lpSS, dRow);
   }

return (lpCell);
}


LPSS_CELL SS_AllocLockCell(LPSPREADSHEET lpSS, LPSS_ROW lpRow,
                           SS_COORD dCol, SS_COORD dRow)
{
TBGLOBALHANDLE hCell;
LPSS_CELL      lpCell = NULL;

if (lpSS->LastCell.Coord.Row == dRow && lpSS->LastCell.Coord.Col == dCol &&
    lpSS->LastCell.hItem)
   lpCell = (LPSS_CELL)tbGlobalLock(lpSS->LastCell.hItem);

else if ((hCell = SS_AllocCell(lpSS, dCol, dRow)) && (LPARAM)hCell != (LPARAM)-1L)
   lpCell = (LPSS_CELL)tbGlobalLock(hCell);

return (lpCell);
}


#if 0
BOOL SS_UnlockCellItem(lpSS, dCol, dRow)

LPSPREADSHEET  lpSS;
SS_COORD       dCol;
SS_COORD       dRow;
{
return (SSx_UnlockCellItem(lpSS, NULL, dCol, dRow));
}


BOOL SSx_UnlockCellItem(lpSS, lpRow, dCol, dRow)

LPSPREADSHEET  lpSS;
LPSS_ROW       lpRow;
SS_COORD       dCol;
SS_COORD       dRow;
{
TBGLOBALHANDLE hCell;
LPSS_ROW       lpRowOld;

if (lpSS->LastCell.Coord.Row == dRow &&
    lpSS->LastCell.Coord.Col == dCol)
   if (lpSS->LastCell.hItem)
      return (tbGlobalUnlock(lpSS->LastCell.hItem));
   else
      return (0);

lpRowOld = lpRow;

if (lpRow || (lpRow = SS_LockRowItem(lpSS, dRow)))
   {
   if (hCell = SS_GetItem(lpSS, &lpRow->Cells, dCol,
                          SS_GetColCnt(lpSS)))
      {
      if (!lpRowOld)
         SS_UnlockRowItem(lpSS, dRow);

      tbGlobalUnlock(hCell);
      return (TRUE);
      }

   if (!lpRowOld)
      SS_UnlockRowItem(lpSS, dRow);
   }

return (FALSE);
}
#endif


BOOL SS_MoveAllocItems(LPSPREADSHEET lpSS, OMEM_POOL OmemPool,
                       LPSS_ALLOCITEM lpAllocItem, SS_COORD wItemNumSrc1,
                       SS_COORD wItemNumSrc2, SS_COORD wItemNumDest,
                       SS_COORD wMaxItems)
{
TBGLOBALHANDLE hItem;
SS_COORD       wSrc;
SS_COORD       wDest;

wDest = wItemNumDest;

if (wItemNumSrc1 == wItemNumSrc2 + 1)
   return (TRUE);

else if (wItemNumDest == wItemNumSrc1 || wItemNumSrc1 > wItemNumSrc2)
   return (FALSE);

else if (wItemNumDest < wItemNumSrc1)
   {
   for (wSrc = wItemNumSrc1; wSrc <= wItemNumSrc2; wSrc++, wDest++)
      {
      if (hItem = SS_GrabItemHandle(lpSS, lpAllocItem, wSrc,
                                    wMaxItems))
         if ((hItem = SS_AllocItems(lpSS, OmemPool, lpAllocItem,
                                    wDest, 0, hItem, wMaxItems, NULL)) == 0 ||
                                    (LPARAM)hItem == (LPARAM)-1)
            return (FALSE);
      }
   }

else
   {
   wDest = wItemNumSrc2 + (wItemNumDest - wItemNumSrc1);

   for (wSrc = wItemNumSrc2; wSrc >= wItemNumSrc1; wSrc--, wDest--)
      {
      if (wDest < wMaxItems)
         if (hItem = SS_GrabItemHandle(lpSS, lpAllocItem, wSrc,
                                       wMaxItems))
            if ((hItem = SS_AllocItems(lpSS, OmemPool, lpAllocItem,
                                       wDest, 0, hItem, wMaxItems,
                                       NULL)) == 0 || (LPARAM)hItem == (LPARAM)-1)
               return (FALSE);
      }
   }

return (TRUE);
}


TBGLOBALHANDLE SS_GrabItemHandle(LPSPREADSHEET lpSS,
                                 LPSS_ALLOCITEM lpAllocItem, SS_COORD wItemNum,
                                 SS_COORD wMaxItems)
{
LPSS_ALLOCITEM   lpItems;
LPTBGLOBALHANDLE lpItems2;
TBGLOBALHANDLE   hItem = 0;
MEMSIZE          wItemAt;
WORD             wItem2At;

if (wItemNum + 1 <= wMaxItems)
   {
   wItemAt = CALCALLOCITEM(wItemNum);

   if (wItemAt < lpAllocItem->wItemCnt)
      {
      lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpAllocItem->hItems);
      wItem2At = CALCALLOCITEMREM(wItemNum);

      if (wItem2At < lpItems[wItemAt].wItemCnt)
         {
         if (lpItems[wItemAt].hItems)
            {
            lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[wItemAt].hItems);

            if (lpItems2[wItem2At])
               {
               hItem = lpItems2[wItem2At];
               lpItems2[wItem2At] = 0;
               }

            tbGlobalUnlock(lpItems[wItemAt].hItems);
            }
         }

      tbGlobalUnlock(lpAllocItem->hItems);
      }
   }

return (hItem);
}


void SSx_FreeCells(LPSPREADSHEET lpSS)
{
LPSS_ALLOCITEM   lpItems;
LPTBGLOBALHANDLE lpItems2;
LPSS_ROW         lpRow;
LPSS_COL         lpCol;
SS_COORD         i;
short            j;

if (lpSS->Row.Items.hItems)
   {
   lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpSS->Row.Items.hItems);

   for (i = 0; i < (SS_COORD)lpSS->Row.Items.wItemCnt; i++)
      if (lpItems[i].wItemCnt)
         {
         lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[i].hItems);

         for (j = 0; j < (short)lpItems[i].wItemCnt; j++)
            {
            if (lpItems2[j])
               {
               lpRow = (LPSS_ROW)tbGlobalLock(lpItems2[j]);
               SSx_FreeRow(lpSS, lpRow);
               tbGlobalUnlock(lpItems2[j]);
#ifdef SS_NO_USE_SH
               tbGlobalFree((TBGLOBALHANDLE)lpItems2[j]);
#else
               MemFreeFS((LPVOID)lpItems2[j]);
#endif
               }
            }

         tbGlobalUnlock(lpItems[i].hItems);
         tbGlobalFree(lpItems[i].hItems);
         }

   tbGlobalUnlock(lpSS->Row.Items.hItems);
   tbGlobalFree(lpSS->Row.Items.hItems);
   lpSS->Row.Items.hItems = 0;
   lpSS->Row.Items.wItemCnt = 0;
   }

if (lpSS->Col.Items.hItems)
   {
   lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpSS->Col.Items.hItems);

   for (i = 0; i < (SS_COORD)lpSS->Col.Items.wItemCnt; i++)
      if (lpItems[i].wItemCnt)
         {
         lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[i].hItems);

         for (j = 0; j < (short)lpItems[i].wItemCnt; j++)
            {
            if (lpItems2[j])
               {
               lpCol = (LPSS_COL)tbGlobalLock(lpItems2[j]);
               SSx_FreeCol(lpSS, lpCol);
               tbGlobalUnlock(lpItems2[j]);
#ifdef SS_NO_USE_SH
               tbGlobalFree((TBGLOBALHANDLE)lpItems2[j]);
#else
               MemFreeFS((LPVOID)lpItems2[j]);
#endif
               }
            }

         tbGlobalUnlock(lpItems[i].hItems);
         tbGlobalFree(lpItems[i].hItems);
         }

   tbGlobalUnlock(lpSS->Col.Items.hItems);
   tbGlobalFree(lpSS->Col.Items.hItems);
   lpSS->Col.Items.hItems = 0;
   lpSS->Col.Items.wItemCnt = 0;
   }
}


void SSx_FreeRowCells(LPSPREADSHEET lpSS, LPSS_ALLOCITEM lpCells)
{
LPSS_ALLOCITEM   lpItems;
LPTBGLOBALHANDLE lpItems2;
LPSS_CELL        lpCell;

if (lpCells->hItems)
   {
   SS_COORD i;
   short    j;

   lpItems = (LPSS_ALLOCITEM)tbGlobalLockHuge(lpCells->hItems);

   for (i = 0; i < (SS_COORD)lpCells->wItemCnt; i++)
      if (lpItems[i].wItemCnt)
         {
         lpItems2 = (LPTBGLOBALHANDLE)tbGlobalLock(lpItems[i].hItems);

         for (j = 0; j < (short)lpItems[i].wItemCnt; j++)
            {
            if (lpItems2[j])
               {
               lpCell = (LPSS_CELL)tbGlobalLock(lpItems2[j]);
               SSx_FreeCell(lpSS, lpCell);
               tbGlobalUnlock(lpItems2[j]);
#ifdef SS_NO_USE_SH
               tbGlobalFree((TBGLOBALHANDLE)lpItems2[j]);
#else
               MemFreeFS((LPVOID)lpItems2[j]);
#endif
               }
            }

         tbGlobalUnlock(lpItems[i].hItems);
         tbGlobalFree(lpItems[i].hItems);
         }

   tbGlobalUnlock(lpCells->hItems);
   tbGlobalFree(lpCells->hItems);
   lpCells->hItems = 0;
   lpCells->wItemCnt = 0;
   }
}


#ifndef SS_NOCALC

void SSx_FreeCalc(LPSPREADSHEET SpreadSheet, TBGLOBALHANDLE hCalc)
{
LPSS_CALC Calc;

if (hCalc)
   {
   Calc = (LPSS_CALC)tbGlobalLock(hCalc);
   
   #ifdef SS_OLDCALC
   SSx_CalcTableDelete(SpreadSheet, &Calc->CellCoord);
   #endif

   if (Calc->hFormula)
      tbGlobalFree(Calc->hFormula);

   if (Calc->Ancestors.hItems)
      tbGlobalFree(Calc->Ancestors.hItems);

   tbGlobalUnlock(hCalc);
   tbGlobalFree(hCalc);
   }
}

#endif


void SSx_FreeData(LPSS_DATA lpData)
{
//#ifdef SS_V80
//if( lpData->bDataType == SS_DATATYPE_HANDLE && lpData->Data.Handle.hValue )
//{
//  if( lpData->Data.Handle.pFreeFunc )
//    lpData->Data.Handle.pFreeFunc(lpData->Data.Handle.hValue);
//  else if( lpData->Data.Handle.fOwn )
//    GlobalFree(lpData->Data.Handle.hValue);
//  lpData->Data.Handle.hValue = NULL;
//  lpData->Data.Handle.fOwn = FALSE;
//  lpData->Data.Handle.pFreeFunc = NULL;
//}
//#endif
#if SS_V80
if( lpData->bDataType == SS_DATATYPE_BUFFER && lpData->Data.Buffer.hBuffer )
{
   tbGlobalFree(lpData->Data.Buffer.hBuffer);
   lpData->Data.Buffer.hBuffer = 0;
}
#endif
if (lpData->bDataType == SS_TYPE_EDIT && lpData->Data.hszData)
   tbGlobalFree(lpData->Data.hszData);

lpData->bDataType = 0;
lpData->Data.hszData = 0;
}


TBGLOBALHANDLE SSx_AllocList(TBGLOBALHANDLE hList, LPSHORT lpdItemCnt,
                             short dItemSize)
{
*lpdItemCnt += 10;

if (!hList)
   hList = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, *lpdItemCnt *
                         dItemSize);

else
   hList = tbGlobalReAlloc(hList, *lpdItemCnt * dItemSize,
                           GMEM_MOVEABLE | GMEM_ZEROINIT);

if (!hList)
   *lpdItemCnt = 0;

return (hList);
}


void SS_PicRef(LPSS_BOOK lpBook, HANDLE hPic)
{
LPSS_PICLIST  lpPicList;
BOOL          fFound = FALSE;
short         i;

if (lpBook->hPicList && lpBook->dPicListCnt)
   {
   lpPicList = (LPSS_PICLIST)tbGlobalLock(lpBook->hPicList);

   for (i = 0; i < lpBook->dPicListCnt && !fFound; i++)
      if (lpPicList[i].hPic == hPic)
         {
         lpPicList[i].dRefCnt++;
         fFound = TRUE;
         }

   if (!fFound)
      for (i = 0; i < lpBook->dPicListCnt && !fFound; i++)
         if (!lpPicList[i].hPic)
            {
            lpPicList[i].hPic = hPic;
            lpPicList[i].dRefCnt = 1;
            fFound = TRUE;
            }

   tbGlobalUnlock(lpBook->hPicList);
   }

if (!fFound)
   {
   if (lpBook->dPicListCnt >= lpBook->dPicListAllocCnt)
		lpBook->hPicList = SSx_AllocList(lpBook->hPicList,
		  										&lpBook->dPicListAllocCnt,
												sizeof(SS_PICLIST));

	if (lpBook->hPicList)
		{
		lpPicList = (LPSS_PICLIST)tbGlobalLock(lpBook->hPicList);

		lpPicList[lpBook->dPicListCnt].hPic = hPic;
		lpPicList[lpBook->dPicListCnt].dRefCnt = 1;
		lpBook->dPicListCnt++;

		tbGlobalUnlock(lpBook->hPicList);
		}
   }
}


void SS_PicDeref(LPSS_BOOK lpBook, HANDLE hPic)
{
LPSS_PICLIST lpPicList;
BOOL         fFound = FALSE;
short        i;

if (lpBook->hPicList && lpBook->dPicListCnt)
   {
   lpPicList = (LPSS_PICLIST)tbGlobalLock(lpBook->hPicList);

   for (i = 0; i < lpBook->dPicListCnt && !fFound; i++)
      if (lpPicList[i].hPic == hPic)
         {
         lpPicList[i].dRefCnt--;

         if (lpPicList[i].dRefCnt == 0)
            {
            if (GetObjectType(lpPicList[i].hPic))  // If returns 0, assume icon
               DeleteObject(lpPicList[i].hPic);
            else
               DestroyIcon(lpPicList[i].hPic);

            lpPicList[i].hPic = 0;
            }

         fFound = TRUE;
         }

   tbGlobalUnlock(lpBook->hPicList);
   }

/* RFW - 12/28/02
if (!fFound)
  DeleteObject(hPic);
*/
}


void SS_PicListFree(LPSS_BOOK lpBook)
{
LPSS_PICLIST lpPicList;
short        i;

if (lpBook->hPicList && lpBook->dPicListCnt)
   {
   lpPicList = (LPSS_PICLIST)tbGlobalLock(lpBook->hPicList);

   for (i = 0; i < lpBook->dPicListCnt; i++)
      if (lpPicList[i].hPic)
         {
         if (GetObjectType(lpPicList[i].hPic))  // If returns 0, assume icon
            DeleteObject(lpPicList[i].hPic);
         else
            DestroyIcon(lpPicList[i].hPic);
         }

   tbGlobalUnlock(lpBook->hPicList);
   tbGlobalFree(lpBook->hPicList);
   lpBook->hPicList = 0;
   lpBook->dPicListCnt = 0;
   lpBook->dPicListAllocCnt = 0;
   }
}


void SS_PostSetEditMode(LPSPREADSHEET lpSS, BOOL fOn, BOOL fDontSendEditModeMsg)
{
lpSS->lpBook->fSetEditModeSent = TRUE;
PostMessage(lpSS->lpBook->hWnd, SSM_SETEDITMODE, (WPARAM)fOn, (LPARAM)fDontSendEditModeMsg);
}
