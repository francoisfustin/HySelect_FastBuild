/*********************************************************
* SS_SORT.C
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
#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_emode.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_span.h"
#include "ss_type.h"

#ifndef SS_NOSORT

extern HANDLE hDynamicInst;

typedef struct ss_sortparam
   {
   WORD          wSortBy;
   short         nSortKeyCnt;
   LPSS_SORTKEY  lpSortKeys;
   LPSPREADSHEET lpSS;
   SS_COORD      Col;
   SS_COORD      Row;
   SS_COORD      Col2;
   SS_COORD      Row2;
   } SS_SORTPARAM, FAR *LPSS_SORTPARAM;

#if defined(_WIN64) || defined(_IA64)
int _export DLLENTRY    SS_SortCompare(LPVOID Arg1, LPVOID Arg2,
                                       LONG_PTR lUserData);
#else
int _export DLLENTRY    SS_SortCompare(LPVOID Arg1, LPVOID Arg2,
                                       long lUserData);
#endif
int                     SSx_SortCompare(LPSPREADSHEET lpSS, WORD wSortBy,
                                        SS_COORD wKeyReference,
                                        WORD wSortOrder, SS_COORD Col,
                                        SS_COORD Row, SS_COORD Col2,
                                        SS_COORD Row2);


BOOL DLLENTRY SSSort(HWND hWnd, SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                     SS_COORD Row2, LPSS_SORT lpSort)
{
SS_SORTKEY SortKeys[3];
short      nKeyCnt;

SortKeys[0].Reference = lpSort->Key1Reference;
SortKeys[0].wOrder = lpSort->wKey1Order;
SortKeys[1].Reference = lpSort->Key2Reference;
SortKeys[1].wOrder = lpSort->wKey2Order;
SortKeys[2].Reference = lpSort->Key3Reference;
SortKeys[2].wOrder = lpSort->wKey3Order;
nKeyCnt = 3;

while (nKeyCnt > 0)
	{
	if (SortKeys[nKeyCnt - 1].Reference == -1)
		nKeyCnt--;
	else
		break;
	}

if (nKeyCnt)
	return (SSSortEx(hWnd, Col, Row, Col2, Row2, lpSort->wSortBy, SortKeys, nKeyCnt));

return (FALSE);
}


BOOL DLLENTRY SSSortEx(HWND hWnd, SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                       SS_COORD Row2, WORD wSortBy, LPSS_SORTKEY lpSortKeys,
                       short nSortKeyCnt)
{
LPSPREADSHEET lpSS;
short i;
BOOL fRet;

lpSS = SS_SheetLock(hWnd);

SS_AdjustCellCoords(lpSS, &Col, &Row);
SS_AdjustCellCoords(lpSS, &Col2, &Row2);

for (i = 0; i < nSortKeyCnt; i++)
	{
	if (wSortBy == SS_SORT_ROW)
		SS_AdjustCellCoords(lpSS, &lpSortKeys[i].Reference, NULL);
	else
		SS_AdjustCellCoords(lpSS, NULL, &lpSortKeys[i].Reference);
	}

// 9283
if (lpSS->lpBook->EditModeOn)
   SS_CellEditModeOff(lpSS, 0);

fRet = SS_SortEx(lpSS, Col, Row, Col2, Row2, wSortBy, lpSortKeys, nSortKeyCnt);

if (lpSS->lpBook->fEditModePermanent)
   SS_CellEditModeOn(lpSS, 0, 0, 0);

for (i = 0; i < nSortKeyCnt; i++)
	{
	if (wSortBy == SS_SORT_ROW)
		SS_AdjustCellCoordsOut(lpSS, &lpSortKeys[i].Reference, NULL);
	else
		SS_AdjustCellCoordsOut(lpSS, NULL, &lpSortKeys[i].Reference);
	}

SS_SheetUnlock(hWnd);
return (fRet);
}

#if defined(_WIN64) || defined(_IA64)
LONG_PTR         lUserData;
#else
long             lUserData;
#endif
CRITICAL_SECTION csSort; // for Apartment model threading
BOOL             csInitSort = 0;

int SS_SortCompare2( const void *arg1, const void *arg2 )
{
return(SS_SortCompare((void *)arg1, (void *)arg2, lUserData));
}


SS_QuickSort(LPVOID lpList, size_t num, size_t width, LPSS_SORTPARAM lpSortParam)
{
if (!csInitSort)
	{
   InitializeCriticalSection(&csSort);
	csInitSort = TRUE;
	}

EnterCriticalSection(&csSort);

#if defined(_WIN64) || defined(_IA64)
lUserData = (LONG_PTR)lpSortParam;
#else
lUserData = (long)lpSortParam;
#endif
qsort((void *)lpList, num, width, SS_SortCompare2);

LeaveCriticalSection(&csSort);

/*
FARPROC ProcInst = MakeProcInstance((FARPROC)SS_SortCompare, hDynamicInst);
QuickSortHuge((HPBYTE)lpList, num, width,
              (long)lpSortParam, (SS_SORTCOMPARE)ProcInst);
FreeProcInstance(ProcInst);
*/
}


// RFW - 4/8/08 - 21963
void SS_SortDestroy()
{
if (csInitSort)
	{
   DeleteCriticalSection(&csSort);
	csInitSort = FALSE;
	}
}



BOOL SS_SortEx(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2,
               SS_COORD Row2, WORD wSortBy, LPSS_SORTKEY lpSortKeys,
               short nSortKeyCnt)
{
GLOBALHANDLE  hList;
SS_SORTPARAM  SortParam;
RECT          Rect;
HPSS_COORD    lpList;
SS_COORD      Base;
SS_COORD      Source;
SS_COORD      Dest;
SS_COORD      RefEmpty;
SS_COORD      RefNext;
SS_COORD      Ref1;
SS_COORD      Ref2;
SS_COORD      Max;
SS_COORD      Cnt;
SS_COORD      At;
SS_COORD      i;
SS_COORD      lAllocCntOld;
BOOL          RedrawOld;
#if !defined(SS_OLDCALC)
BOOL          bFormulaSyncOld;
#else
BOOL          fCalcBuildDependencies;
#endif

if (Col == SS_ALLCOLS || Col2 == SS_ALLCOLS)
   {
   Col = SS_ALLCOLS;
   Col2 = SS_ALLCOLS;
   }

if (Row == SS_ALLROWS || Row2 == SS_ALLROWS)
   {
   Row = SS_ALLROWS;
   Row2 = SS_ALLROWS;
   }

if (Row > Row2 || Col > Col2 || nSortKeyCnt == 0 || !lpSortKeys ||
    lpSortKeys[0].wOrder == SS_SORT_NONE)
   return (FALSE);

if (lpSortKeys)
   {
	if (Row == -1)
		Row2 = -1;

	if (Col == -1)
		Col2 = -1;

   if (lpSS->fVirtualMode)
      Row = max(Row, lpSS->Virtual.VTop);

#ifdef SS_V40
	{
	SS_COORD ColSpan = max(Col, lpSS->Col.HeaderCnt);
	SS_COORD ColSpan2 = Col2 == -1 ? SS_GetColCnt(lpSS) - 1 : Col2;
	SS_COORD RowSpan = max(Row, lpSS->Row.HeaderCnt);
	SS_COORD RowSpan2 = Row2 == -1 ? SS_GetRowCnt(lpSS) - 1 : Row2;

	if (SS_IsSpanOverlap(lpSS, ColSpan, RowSpan, ColSpan2 - ColSpan + 1, RowSpan2 - RowSpan + 1))
		return (FALSE);
	}
#endif // SS_V40

   if (wSortBy == SS_SORT_ROW)
      {
      if (Row < lpSS->Row.HeaderCnt)
         Ref1 = lpSS->Row.HeaderCnt;
      else
         Ref1 = Row;

      if (Row2 == -1)
         Ref2 = lpSS->Row.AllocCnt - 1;
//         Ref2 = lpSS->Row.DataCnt - 1;
      else
         Ref2 = min(Row2, SS_GetRowCnt(lpSS) - 1);
      }
   else
      {
      if (Col < lpSS->Col.HeaderCnt)
         Ref1 = lpSS->Col.HeaderCnt;
      else
         Ref1 = Col;

      if (Col2 == -1)
         Ref2 = lpSS->Col.AllocCnt - 1;
//         Ref2 = lpSS->Col.DataCnt - 1;
      else
         Ref2 = min(Col2, SS_GetColCnt(lpSS) - 1);
      }

   Cnt = Ref2 - Ref1 + 1;

	if (Cnt <= 0)
		return TRUE;

   if (!(hList = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                             (Cnt + 1) * sizeof(SS_COORD))))
      return (FALSE);

   lpList = (HPSS_COORD)GlobalLock(hList);

   for (At = 0, i = Ref1; i <= Ref2; i++, At++)
      *(lpList + At) = i;

   SortParam.wSortBy = wSortBy;
   SortParam.nSortKeyCnt = nSortKeyCnt;
   SortParam.lpSortKeys = lpSortKeys;
   SortParam.lpSS = lpSS;
   SortParam.Col = Col;
   SortParam.Row = Row;
   SortParam.Col2 = Col2;
   SortParam.Row2 = Row2;

	SS_QuickSort((void *)lpList, Cnt, sizeof(SS_COORD), &SortParam);

   /****************************************************
   * Copy the rows or columns to their new destination
   ****************************************************/

   RedrawOld = lpSS->lpBook->Redraw;
   lpSS->lpBook->Redraw = FALSE;

   if (wSortBy == SS_SORT_ROW)
      {
		lAllocCntOld = lpSS->Row.AllocCnt;
      lpSS->Row.MaxBeforeSort = lpSS->Row.Max;
      lpSS->Row.Max++;
      Max = SS_GetRowCnt(lpSS) - 1;
      }
   else
      {
		lAllocCntOld = lpSS->Col.AllocCnt;
      lpSS->Col.MaxBeforeSort = lpSS->Col.Max;
      lpSS->Col.Max++;
      Max = SS_GetColCnt(lpSS) - 1;
      }

   Base = 0;

   RefEmpty = Cnt;
   *(lpList + Cnt) = -1;

   #ifdef SS_OLDCALC
   fCalcBuildDependencies = lpSS->fCalcBuildDependencies;
   lpSS->fCalcBuildDependencies = FALSE;
   #else
   bFormulaSyncOld = lpSS->lpBook->CalcInfo.bFormulaSync;
   lpSS->lpBook->CalcInfo.bFormulaSync = FALSE;
   #endif
   
   while (Base < Cnt)
      {
      At = Base;

      if (*(lpList + At) != -1 && *(lpList + At) != Ref1 + At)
         {
         if (wSortBy == SS_SORT_ROW)
            SS_CopySwapRange(lpSS, Col, Ref1 + At, Col2, Ref1 + At, Col,
                             Max, SS_CMD_MOVE, FALSE, FALSE);
         else
            SS_CopySwapRange(lpSS, Ref1 + At, Row, Ref1 + At, Row2,
                             Max, Row, SS_CMD_MOVE, FALSE, FALSE);

         for (i = At + 1; i < Cnt; i++)
            if (*(lpList + i) == At + Ref1)
               {
               *(lpList + i) = Cnt + Ref1;
               break;
               }

         while (*(lpList + At) != -1 && *(lpList + At) != Ref1 + At)
            {
            RefNext = *(lpList + At) - Ref1;
            *(lpList + At) = -1;

            if (At == Cnt)
               Dest = Max;
            else
               Dest = Ref1 + At;

            if (RefNext == Cnt)
               Source = Max;
            else
               Source = Ref1 + RefNext;

            if (wSortBy == SS_SORT_ROW)
               SS_CopySwapRange(lpSS, Col, Source, Col2, Source, Col, Dest, SS_CMD_MOVE, FALSE, FALSE);
            else
               SS_CopySwapRange(lpSS, Source, Row, Source, Row2, Dest, Row, SS_CMD_MOVE, FALSE, FALSE);

            At = RefNext;
            }
         }

      Base++;
      }

   #if !defined(SS_OLDCALC)
   lpSS->lpBook->CalcInfo.bFormulaSync = bFormulaSyncOld;
   #endif

   if (wSortBy == SS_SORT_ROW)
      {
      // RFW - 7/1/99
      SS_SetMaxRows(lpSS, lpSS->Row.Max - 1);
//      lpSS->Row.Max--;
      lpSS->Row.MaxBeforeSort = -1;
		lpSS->Row.AllocCnt = lAllocCntOld;
      }
   else
      {
      // RFW - 7/1/99
      SS_SetMaxCols(lpSS, lpSS->Col.Max - 1);
//      lpSS->Col.Max--;
      lpSS->Col.MaxBeforeSort = -1;
		lpSS->Col.AllocCnt = lAllocCntOld;
      }

	// RFW - 8/19/04 - 15058
   if (Col2 >= lpSS->Col.DataCnt - 1)
      SS_SetDataColCnt(lpSS, Col2 + 1);

   if (Row2 >= lpSS->Row.DataCnt - 1)
      SS_SetDataRowCnt(lpSS, Row2 + 1);

   if (Col2 >= lpSS->Col.DataCnt - 1)
      SS_AdjustDataColCnt(lpSS, Col, Col2);

   if (Row2 >= lpSS->Row.DataCnt - 1)
      SS_AdjustDataRowCnt(lpSS, Row, Row2);

   GlobalUnlock(hList);
   GlobalFree(hList);

   #ifdef SS_OLDCALC
   lpSS->fCalcBuildDependencies = fCalcBuildDependencies;
   #endif
   SS_BuildDependencies(lpSS);

   if (GetUpdateRect(lpSS->lpBook->hWnd, &Rect, FALSE))
		SS_BookSetRedraw(lpSS->lpBook, RedrawOld);
   else
      lpSS->lpBook->Redraw = RedrawOld;
   }

return (TRUE);
}

#if defined(_WIN64) || defined(_IA64)
int _export DLLENTRY SS_SortCompare(LPVOID Arg1, LPVOID Arg2, LONG_PTR lUserData)
#else
int _export DLLENTRY SS_SortCompare(LPVOID Arg1, LPVOID Arg2, long lUserData)
#endif
{
LPSS_SORTPARAM lpSortParam = (LPSS_SORTPARAM)lUserData;
SS_COORD       Ref1 = *(LPSS_COORD)Arg1;
SS_COORD       Ref2 = *(LPSS_COORD)Arg2;
SS_COORD       Col;
SS_COORD       Row;
SS_COORD       Col2;
SS_COORD       Row2;
short          i;
int            iRet;

if (lpSortParam->wSortBy == SS_SORT_ROW)
   {
   Row = Ref1;
   Row2 = Ref2;
   }
else
   {
   Col = Ref1;
   Col2 = Ref2;
   }

for (i = 0, iRet = 0; i < lpSortParam->nSortKeyCnt && iRet == 0; i++)
   {
   if (lpSortParam->lpSortKeys[i].wOrder &&
       lpSortParam->lpSortKeys[i].Reference != -1)
      iRet = SSx_SortCompare(lpSortParam->lpSS, lpSortParam->wSortBy,
                             lpSortParam->lpSortKeys[i].Reference,
                             lpSortParam->lpSortKeys[i].wOrder,
                             Col, Row, Col2, Row2);
   }

return (iRet);
}


int SSx_SortCompare(LPSPREADSHEET lpSS, WORD wSortBy, SS_COORD KeyReference,
                    WORD wSortOrder, SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                    SS_COORD Row2)
{
LPSS_COL lpCol;
LPSS_COL lpCol2;
LPSS_ROW lpRow;
LPSS_ROW lpRow2;
int      iRet;

if (wSortBy == SS_SORT_ROW)
   Col = Col2 = KeyReference;
else
   Row = Row2 = KeyReference;

lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);
lpCol2 = SS_LockColItem(lpSS, Col2);
lpRow2 = SS_LockRowItem(lpSS, Row2);

iRet = SS_Compare(lpSS, lpCol, Col, lpRow, Row, lpCol2, Col2, lpRow2, Row2, wSortOrder, TRUE);

SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);
SS_UnlockColItem(lpSS, Col2);
SS_UnlockRowItem(lpSS, Row2);

return (iRet);
}


void SS_TextToNativeData(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                         LPSS_DATA lpData, LPTSTR lpszText)
{
if (SS_IsCellTypeFloatNum(lpCellType))
   {
   lpData->bDataType = SS_TYPE_FLOAT;
   SS_StringToNum(lpSS, lpCellType, lpszText, &lpData->Data.dfValue);
   }

else if (lpCellType->Type == SS_TYPE_INTEGER)
   {
   lpData->bDataType = SS_TYPE_INTEGER;
   lpData->Data.lValue = StringToLong(lpszText);
   }
}


LPTSTR SS_NativeDataToText(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType,
                           LPSS_DATA lpData, LPTSTR lpszText, LPTSTR lpszBuffer)
{
LPTSTR lpszRet;

if (lpData->bDataType == SS_TYPE_INTEGER)
   lpszRet = LongToString(lpData->Data.lValue, lpszBuffer);
else if (lpData->bDataType == SS_TYPE_FLOAT)
   {
	BYTE bRight = 0;

	if (lpCellType->Type == SS_TYPE_FLOAT)
		bRight = (BYTE)lpCellType->Spec.Float.Right;
#ifdef SS_V40
	else if (lpCellType->Type == SS_TYPE_CURRENCY)
		bRight = (BYTE)lpCellType->Spec.Currency.Right;
	else if (lpCellType->Type == SS_TYPE_NUMBER)
		bRight = (BYTE)lpCellType->Spec.Number.Right;
	else if (lpCellType->Type == SS_TYPE_PERCENT)
		bRight = (BYTE)lpCellType->Spec.Percent.Right + 2;
#endif // SS_V40
#ifdef SS_V70
   else if (lpCellType->Type == SS_TYPE_SCIENTIFIC)
		bRight = (BYTE)lpCellType->Spec.Scientific.Right;
#endif // SS_V70

   StrPrintf(lpszBuffer, _T("%.*f"), bRight, lpData->Data.dfValue);
	// RFW - 6/13/04 - 14447
	if (bRight == 0 && *lpszBuffer && lpszBuffer[lstrlen(lpszBuffer) - 1] == '.')
		lpszBuffer[lstrlen(lpszBuffer) - 1] = '\0';

   lpszRet = lpszBuffer;
   }
else
   lpszRet = lpszText;

return (lpszRet);
}


int SS_Compare(LPSPREADSHEET lpSS, LPSS_COL lpCol, SS_COORD Col, LPSS_ROW lpRow, SS_COORD Row,
               LPSS_COL lpCol2, SS_COORD Col2, LPSS_ROW lpRow2, SS_COORD Row2, WORD wSortOrder, BOOL fUseButtonText)
{
TBGLOBALHANDLE hszData1 = 0;
TBGLOBALHANDLE hszData2 = 0;
LPSS_CELL      lpCell;
HGLOBAL        hButtonData1 = 0;
HGLOBAL        hButtonData2 = 0;
SS_CELLTYPE    CellType1;
SS_CELLTYPE    CellType2;
SS_DATA        Data;
SS_DATA        Data2;
LPTSTR         lpData1 = NULL;
LPTSTR         lpData2 = NULL;
LPTSTR         lpText1;
LPTSTR         lpText2;
//GAB 8/19/04 Bug 15052 Made the buffer size the same as what is defined for 
// CVTBUFSIZE in output.h
TCHAR          szBuffer1[349]; 
TCHAR          szBuffer2[349];
BOOL           fData1;
BOOL           fData2;
int            iRet;

lpCell = SSx_LockCellItem(lpSS, lpRow, Col, Row);
fData1 = SS_GetDataStruct(lpSS, lpCol, lpRow, lpCell, &Data, Col, Row);
SSx_RetrieveCellType(lpSS, &CellType1, lpCol, lpRow, lpCell, Col, Row);
SS_UnlockCellItem(lpSS, Col, Row);

lpCell = SSx_LockCellItem(lpSS, lpRow2, Col2, Row2);
fData2 = SS_GetDataStruct(lpSS, lpCol2, lpRow2, lpCell, &Data2, Col2, Row2);
SSx_RetrieveCellType(lpSS, &CellType2, lpCol2, lpRow2, lpCell, Col2, Row2);
SS_UnlockCellItem(lpSS, Col2, Row2);

// Determine the Data for the first key
if (fUseButtonText)
	{
	if (CellType1.Type == SS_TYPE_CHECKBOX && CellType1.Spec.CheckBox.hText)
		hButtonData1 = CellType1.Spec.CheckBox.hText;
	else if (CellType1.Type == SS_TYPE_BUTTON && CellType1.Spec.Button.hText)
		hButtonData1 = CellType1.Spec.Button.hText;

	if (hButtonData1)
		{
		lpData1 = (LPTSTR)GlobalLock(hButtonData1);
		if (*lpData1)
			fData1 = TRUE;
		else
			lpData1 = NULL;
		}
	}

if (lpData1)
   ;
else if (Data.bDataType == SS_TYPE_EDIT)
   {
   hszData1 = Data.Data.hszData;
   lpData1 = (LPTSTR)tbGlobalLock(hszData1);
   }
else if (CellType1.Type == SS_TYPE_CHECKBOX || CellType1.Type == SS_TYPE_BUTTON)
   {
   lpData1 = _T("0");
   fData1 = TRUE;
   }

if (fData1 && Data.bDataType == SS_TYPE_EDIT && !lpData1)
   fData1 = FALSE;

// Determine the Data for the second key
if (fUseButtonText)
	{
	if (CellType2.Type == SS_TYPE_CHECKBOX && CellType2.Spec.CheckBox.hText)
		hButtonData2 = CellType2.Spec.CheckBox.hText;
	else if (CellType2.Type == SS_TYPE_BUTTON && CellType2.Spec.Button.hText)
		hButtonData2 = CellType2.Spec.Button.hText;

	if (hButtonData2)
		{
		lpData2 = (LPTSTR)GlobalLock(hButtonData2);
		if (*lpData2)
			fData2 = TRUE;
		else
			lpData2 = NULL;
		}
	}

if (lpData2)
   ;
else if (Data2.bDataType == SS_TYPE_EDIT)
   {
   hszData2 = Data2.Data.hszData;
   lpData2 = (LPTSTR)tbGlobalLock(hszData2);
   }
else if (CellType2.Type == SS_TYPE_CHECKBOX || CellType2.Type == SS_TYPE_BUTTON)
   {
   lpData2 = _T("0");
   fData2 = TRUE;
   }

if (fData2 && Data2.bDataType == SS_TYPE_EDIT && !lpData2)
   fData2 = FALSE;

if (fData1 && fData2)
   {
   if (Data.bDataType == SS_TYPE_EDIT)
		SS_TextToNativeData(lpSS, &CellType1, &Data, lpData1);

   if (Data2.bDataType == SS_TYPE_EDIT)
		SS_TextToNativeData(lpSS, &CellType2, &Data2, lpData2);

   /**************
   * Do Compares
   **************/

   if (Data.bDataType == SS_TYPE_FLOAT && Data2.bDataType == SS_TYPE_FLOAT)
      iRet = Data.Data.dfValue > Data2.Data.dfValue ? 1 :
             Data.Data.dfValue < Data2.Data.dfValue ? -1 : 0;

   else if (Data.bDataType == SS_TYPE_INTEGER &&
            Data2.bDataType == SS_TYPE_INTEGER)
      iRet = Data.Data.lValue > Data2.Data.lValue ? 1 :
             Data.Data.lValue < Data2.Data.lValue ? -1 : 0;

   else if (Data.bDataType == SS_TYPE_FLOAT &&
            Data2.bDataType == SS_TYPE_INTEGER)
      iRet = Data.Data.dfValue > (double)Data2.Data.lValue ? 1 :
             Data.Data.dfValue < (double)Data2.Data.lValue ? -1 : 0;

   else if (Data.bDataType == SS_TYPE_INTEGER &&
            Data2.bDataType == SS_TYPE_FLOAT)
      iRet = (double)Data.Data.lValue > Data2.Data.dfValue ? 1 :
             (double)Data.Data.lValue < Data2.Data.dfValue ? -1 : 0;

   else
      {
		lpText1 = SS_NativeDataToText(lpSS, &CellType1, &Data, lpData1, szBuffer1);
		lpText2 = SS_NativeDataToText(lpSS, &CellType2, &Data2, lpData2, szBuffer2);

      if (CellType1.Type == SS_TYPE_DATE && CellType2.Type == SS_TYPE_DATE)
         {
         TB_DATE Date1 = {0, 0, 0};
         TB_DATE Date2 = {0, 0, 0};

         if (lpText1 && *lpText1)
            DateStringToDMYEx(lpText1, &Date1, &CellType1.Spec.Date.Format,
                              lpSS->lpBook->nTwoDigitYearMax);

         if (lpText2 && *lpText2)
            DateStringToDMYEx(lpText2, &Date2, &CellType2.Spec.Date.Format,
                              lpSS->lpBook->nTwoDigitYearMax);

         if (Date1.nYear > Date2.nYear)
            iRet = 1;
         else if (Date1.nYear < Date2.nYear)
            iRet = -1;
         else
            {
            if (Date1.nMonth > Date2.nMonth)
               iRet = 1;
            else if (Date1.nMonth < Date2.nMonth)
               iRet = -1;
            else
               iRet = (Date1.nDay > Date2.nDay ? 1 : Date1.nDay < Date2.nDay ? -1 : 0);
            }
         }

      else if (CellType1.Type == SS_TYPE_TIME &&
               CellType2.Type == SS_TYPE_TIME)
         {
			TIME Time;
			long lSeconds1;
			long lSeconds2;

         TimeStringToHMS(lpText1, &Time, &CellType1.Spec.Time.Format);
         lSeconds1 = TimeHMSToSeconds(&Time);

         TimeStringToHMS(lpText2, &Time, &CellType2.Spec.Time.Format);
         lSeconds2 = TimeHMSToSeconds(&Time);

         iRet = lSeconds1 > lSeconds2 ? 1 : lSeconds1 < lSeconds2 ? -1 : 0;
         }

      // Always sort Time cells before Date cells.
      else if (CellType1.Type == SS_TYPE_DATE &&
               CellType2.Type == SS_TYPE_TIME)
         iRet = 1;

      else if (CellType1.Type == SS_TYPE_TIME &&
               CellType2.Type == SS_TYPE_DATE)
         iRet = -1;

      else
         iRet = lstrcmp(lpText1, lpText2);
      }

	if (wSortOrder == SS_SORT_DESCENDING)
		iRet = (iRet == 1 ? -1 : (iRet == -1 ? 1 : 0));
   }

else if (fData1 && !fData2)
   iRet = -1;

else if (!fData1 && fData2)
   iRet = 1;

else
   iRet = 0;

if (hszData1)
   tbGlobalUnlock(hszData1);
if (hszData2)
   tbGlobalUnlock(hszData2);
if (hButtonData1)
   GlobalUnlock(hButtonData1);
if (hButtonData2)
   GlobalUnlock(hButtonData2);

return (iRet);
}


#else


BOOL DLLENTRY SSSort(HWND hWnd, SS_COORD Col, SS_COORD Row, SS_COORD Col2,
                     SS_COORD Row2, LPSS_SORT lpSort)
{
return (TRUE);
}


#endif
