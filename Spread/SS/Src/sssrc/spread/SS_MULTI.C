/*********************************************************
* SS_MULTI.C
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
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_drag.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_multi.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_user.h"
#include "ss_win.h"

GLOBALHANDLE    SS_ListAddItem(GLOBALHANDLE hList, LPSHORT lpdItemCnt,
                               LPSHORT lpdItemAllocCnt, LPVOID lpItem,
                               short dItemLen, short dAllocInc);
GLOBALHANDLE    SS_ListDup(GLOBALHANDLE hList, short dItemCnt,
                           short dItemSize);
void            SS_ListDelItem(GLOBALHANDLE hList, short dItemNum,
                               LPSHORT lpdItemCnt, short dItemLen);
GLOBALHANDLE    SS_SelBlockSplitItem(GLOBALHANDLE hList, LPSHORT lpdItemCnt,
                                     LPSHORT lpdItemAllocCnt,
                                     LPSS_SELBLOCK lpSelBlockOrig,
                                     LPSS_SELBLOCK lpSelBlockSplit);
void            SSx_SetSelBlockStruct(LPSS_SELBLOCK lpSelBlockDest,
                                      SS_COORD ULCol, SS_COORD ULRow,
                                      SS_COORD LRCol, SS_COORD LRRow);
BOOL            SS_IntersectSelBlock(LPSPREADSHEET lpSS, LPSS_SELBLOCK lpSelBlock1,
                                     LPSS_SELBLOCK lpSelBlock2);
void            SS_MultiSelCompleteBlock(HWND hWnd, LPSPREADSHEET lpSS);

#ifndef SS_NOLISTBOX

#if defined(_WIN64) || defined(_IA64)
long DLLENTRY SSSelModeSendMessage(HWND hWnd, UINT uMsg, long lParam1,
                                   LONG_PTR lParam2, long lParam3)
#else
long DLLENTRY SSSelModeSendMessage(HWND hWnd, UINT uMsg, long lParam1,
                                   long lParam2, long lParam3)
#endif
{
LPSPREADSHEET lpSS;
long          lRet;

lpSS = SS_SheetLock(hWnd);

switch (uMsg)
	{
	case SS_SELMODE_GETSEL:
	case SS_SELMODE_GETSELITEM:
   case SS_SELMODE_SETCURSEL:
		SS_AdjustCellCoords(lpSS, NULL, &lParam1);
		break;

	case SS_SELMODE_SELITEMRANGE:
#if defined(_WIN64) || defined(_IA64)
		{
			SS_COORD coord = (SS_COORD)lParam2;
			SS_AdjustCellCoords(lpSS, NULL, &coord);
			lParam2 = (LONG_PTR)coord;
		}
#else
		SS_AdjustCellCoords(lpSS, NULL, &lParam2);
#endif
		SS_AdjustCellCoords(lpSS, NULL, &lParam3);
	}

lRet = SS_SelModeSendMessage(lpSS, uMsg, lParam1, lParam2, lParam3);

switch (uMsg)
	{
	case SS_SELMODE_GETCURSEL:
	case SS_SELMODE_GETSELITEM:
		SS_AdjustCellCoordsOut(lpSS, NULL, &lRet);
		break;
	}

SS_SheetUnlock(hWnd);
  return lRet;
}

#if defined(_WIN64) || defined(_IA64)
long SS_SelModeSendMessage(LPSPREADSHEET lpSS, UINT uMsg, long lParam1,
                           LONG_PTR lParam2, long lParam3)
#else
long SS_SelModeSendMessage(LPSPREADSHEET lpSS, UINT uMsg, long lParam1,
                           long lParam2, long lParam3)
#endif
{
LPSS_ROW      lpRow;
long          lRet = -1;
SS_COORD      Sel1;
SS_COORD      Sel2;
SS_COORD      dCnt;
SS_COORD      i;
BOOL          RedrawOld;

if (lpSS->wOpMode == SS_OPMODE_SINGLESEL ||
    lpSS->wOpMode == SS_OPMODE_MULTISEL ||
    lpSS->wOpMode == SS_OPMODE_EXTSEL)
   {
   switch (uMsg)
      {
      case SS_SELMODE_CLEARSEL:
         if (lpSS->wOpMode != SS_OPMODE_SINGLESEL)
            {
            RedrawOld = lpSS->lpBook->Redraw;
            lpSS->lpBook->Redraw = FALSE;

            for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
               if (lpRow = SS_LockRowItem(lpSS, i))
                  {
                  if (lpRow->fRowSelected)
                     {
                     lpRow->fRowSelected = FALSE;
                     SS_InvalidateRow(lpSS, i);
                     }

                  SS_UnlockRowItem(lpSS, i);
                  }

            lpSS->MultiSelCnt = 0;
				SS_BookSetRedraw(lpSS->lpBook, RedrawOld);
            lRet = TRUE;
            }

         break;

      case SS_SELMODE_GETCURSEL:
         if (lpSS->wOpMode == SS_OPMODE_SINGLESEL)
            lRet = lpSS->Row.CurAt;

         break;

      case SS_SELMODE_GETSEL:
         if (lpSS->wOpMode == SS_OPMODE_SINGLESEL)
            {
            if (lpSS->Row.CurAt == lParam1)
               lRet = TRUE;
            else
               lRet = FALSE;
            }

         else
            {
            lRet = FALSE;

            if (lpRow = SS_LockRowItem(lpSS, (SS_COORD)lParam1))
               lRet = lpRow->fRowSelected;
            }

         break;

      case SS_SELMODE_GETSELCOUNT:
         if (lpSS->wOpMode != SS_OPMODE_SINGLESEL)
            lRet = lpSS->MultiSelCnt;

         break;

      case SS_SELMODE_GETSELITEMS:   // The array returned is in external coords
         if (lpSS->wOpMode != SS_OPMODE_SINGLESEL)
            {
            SS_COORD huge *lpdBuffer = (SS_COORD huge *)lParam2;

            for (i = lpSS->Row.HeaderCnt, dCnt = 0; i < lpSS->Row.AllocCnt && dCnt < lParam1; i++)
               {
               if (lpRow = SS_LockRowItem(lpSS, i))
                  {
                  if (lpRow->fRowSelected)
                     {
                     *(lpdBuffer + dCnt) = i - (lpSS->Row.HeaderCnt - 1);
                     dCnt++;
                     }

                  SS_UnlockRowItem(lpSS, i);
                  }
               }

            lRet = (long)dCnt;
            }

         break;

      case SS_SELMODE_GETSELITEM:
         if (lpSS->wOpMode != SS_OPMODE_SINGLESEL)
            {
            for (i = max((SS_COORD)lParam1 + 1, lpSS->Row.HeaderCnt); lRet == -1 &&
                 i < lpSS->Row.AllocCnt; i++)
               {
               if (lpRow = SS_LockRowItem(lpSS, i))
                  {
                  if (lpRow->fRowSelected)
                     lRet = i;

                  SS_UnlockRowItem(lpSS, i);
                  }
               }
            }

         break;

      case SS_SELMODE_SELITEMRANGE:
         if (lpSS->wOpMode != SS_OPMODE_SINGLESEL)
            {
            Sel1 = (SS_COORD)lParam2;
            Sel2 = (SS_COORD)lParam3;

            RedrawOld = lpSS->lpBook->Redraw;
            lpSS->lpBook->Redraw = FALSE;

            for (i = Sel1; i <= Sel2; i++)
               {
               if (!lParam1)
                  {
                  if (i >= lpSS->Row.AllocCnt)
                     break;

                  if (lpRow = SS_LockRowItem(lpSS, i))
                     {
                     if (lpRow->fRowSelected)
                        {
                        lpSS->MultiSelCnt--;
                        lpRow->fRowSelected = FALSE;
                        SS_InvalidateRow(lpSS, i);
                        }
                     }
                  }

               else
                  {
                  if (lpRow = SS_AllocLockRow(lpSS, i))
                     {
                     if (!lpRow->fRowSelected)
                        {
                        lpSS->MultiSelCnt++;
                        lpRow->fRowSelected = TRUE;
                        SS_InvalidateRow(lpSS, i);
                        }

                     SS_UnlockRowItem(lpSS, i);
                     }
                  }
               }

            if (lParam1)
					{
					lpSS->BlockCellUL.Col = -1;
					lpSS->BlockCellUL.Row = Sel1;
					lpSS->BlockCellLR.Col = -1;
					lpSS->BlockCellLR.Row = Sel2;
					}

				SS_BookSetRedraw(lpSS->lpBook, RedrawOld);
            }

         break;

      case SS_SELMODE_SETCURSEL:
         if (lpSS->wOpMode == SS_OPMODE_SINGLESEL)
            {
            SS_SetActiveCell(lpSS, lpSS->Col.HeaderCnt, (SS_COORD)lParam1);
            lRet = 0;
            }

         break;

      case SS_SELMODE_SETSEL:
         if (lpSS->wOpMode != SS_OPMODE_SINGLESEL)
            {
            if (lParam2 == SS_ALLROWS || lParam2 == 0)
               SS_SelModeSendMessage(lpSS, SS_SELMODE_SELITEMRANGE,
                                     lParam1, lpSS->Row.HeaderCnt, SS_GetRowCnt(lpSS) - 1);

            else
               SS_SelModeSendMessage(lpSS, SS_SELMODE_SELITEMRANGE,
                                     lParam1, lParam2, (long)lParam2);

            lRet = 0;
            }

         break;
      }
   }

return (lRet);
}


BOOL DLLENTRY SSSetMultiSelBlocks(HWND hWnd, LPSS_SELBLOCK lpSelBlockList, short dSelBlockCnt)
{
LPSPREADSHEET lpSS;
BOOL          fRet = TRUE;
short         i;

lpSS = SS_SheetLock(hWnd);

SS_DeSelectBlock(lpSS);

/*
if (lpSS->MultiSelBlock.hItemList)
   GlobalFree(lpSS->MultiSelBlock.hItemList);

lpSS->MultiSelBlock.dItemCnt = 0;
lpSS->MultiSelBlock.dItemAllocCnt = 0;
lpSS->fMultipleBlocksSelected = FALSE;
*/

if (lpSelBlockList && dSelBlockCnt > 0)
   {
	SS_CELLCOORD CellUL, CellLR;

   lpSS->fMultipleBlocksSelected = TRUE;

   for (i = 0; i < dSelBlockCnt && fRet; i++)
		{
		CellUL = lpSelBlockList[i].UL;
		CellLR = lpSelBlockList[i].LR;
		SS_AdjustCellCoords(lpSS, &CellUL.Col, &CellUL.Row);
		SS_AdjustCellCoords(lpSS, &CellLR.Col, &CellLR.Row);
      if (!SS_MultiSelBlockAddItem(lpSS, &CellUL, &CellLR))
         fRet = FALSE;
		}

   if (fRet)
      {
		CellUL = lpSelBlockList[dSelBlockCnt - 1].UL;
		CellLR = lpSelBlockList[dSelBlockCnt - 1].LR;
		SS_AdjustCellCoords(lpSS, &CellUL.Col, &CellUL.Row);
		SS_AdjustCellCoords(lpSS, &CellLR.Col, &CellLR.Row);

      lpSS->BlockCellUL.Col = CellUL.Col;
      lpSS->BlockCellUL.Row = CellUL.Row;

      lpSS->BlockCellLR.Col = CellLR.Col;
      lpSS->BlockCellLR.Row = CellLR.Row;

      lpSS->Col.CurAt = max(lpSS->BlockCellUL.Col, lpSS->Col.HeaderCnt);
      lpSS->Row.CurAt = max(lpSS->BlockCellUL.Row, lpSS->Row.HeaderCnt);

      SS_InvertMultiSelBlock(0, lpSS);
      }
   }

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL DLLENTRY SSAddMultiSelBlocks(HWND hWnd, LPSS_SELBLOCK lpSelBlock)
{
LPSPREADSHEET lpSS;
SS_SELBLOCK   selBlock;
BOOL          fRet = FALSE;

lpSS = SS_SheetLock(hWnd);

if (lpSelBlock)
   {
   _fmemcpy(&selBlock, lpSelBlock, sizeof(selBlock));
   if (SS_ALLCOLS == selBlock.UL.Col || SS_ALLCOLS == selBlock.LR.Col)
      selBlock.UL.Col = selBlock.LR.Col = SS_ALLCOLS;
   if (SS_ALLROWS == selBlock.UL.Row || SS_ALLROWS == selBlock.LR.Row)
      selBlock.UL.Row = selBlock.LR.Row = SS_ALLROWS;

	SS_AdjustCellCoords(lpSS, &selBlock.UL.Col, &selBlock.UL.Row);
	SS_AdjustCellCoords(lpSS, &selBlock.LR.Col, &selBlock.LR.Row);

	/* RFW - 6/23/04 - 14231
   if (selBlock.UL.Col <= selBlock.LR.Col &&
       selBlock.UL.Row <= selBlock.LR.Row)
	*/
   if (selBlock.UL.Col <= selBlock.LR.Col && selBlock.UL.Col < SS_GetColCnt(lpSS) &&
       selBlock.UL.Row <= selBlock.LR.Row && selBlock.UL.Row < SS_GetRowCnt(lpSS))
      {
	   if (selBlock.UL.Col < lpSS->Col.HeaderCnt || selBlock.LR.Col < lpSS->Col.HeaderCnt)
	      selBlock.UL.Col = selBlock.LR.Col = SS_ALLCOLS;

	   if (selBlock.UL.Row < lpSS->Row.HeaderCnt || selBlock.LR.Row < lpSS->Row.HeaderCnt)
	      selBlock.UL.Row = selBlock.LR.Row = SS_ALLROWS;

		// RFW - 6/23/04 - 14231
		selBlock.LR.Col = min(SS_GetColCnt(lpSS) - 1, selBlock.LR.Col);
		selBlock.LR.Row = min(SS_GetRowCnt(lpSS) - 1, selBlock.LR.Row);

      if (lpSS->fMultipleBlocksSelected || SS_IsBlockSelected(lpSS))
         SS_BeginMultipleSelection(hWnd, lpSS);

      lpSS->BlockCellLR.Col = lpSS->Col.CurAt;
      lpSS->BlockCellUL.Row = lpSS->Row.CurAt;
      lpSS->BlockCellLR.Col = lpSS->Col.CurAt;
      lpSS->BlockCellLR.Row = lpSS->Row.CurAt;

      SS_HighlightCell(lpSS, FALSE);

      lpSS->Col.CurAt = max(selBlock.UL.Col, lpSS->Col.HeaderCnt);
      lpSS->Row.CurAt = max(selBlock.UL.Row, lpSS->Row.HeaderCnt);

      SS_HighlightCell(lpSS, TRUE);

      lpSS->Col.BlockCellCurrentPos = max(selBlock.UL.Col, lpSS->Col.HeaderCnt);
      lpSS->Row.BlockCellCurrentPos = max(selBlock.UL.Row, lpSS->Row.HeaderCnt);

      lpSS->BlockCellUL.Col = lpSS->Col.CurAt;
      lpSS->BlockCellUL.Row = lpSS->Row.CurAt;
      lpSS->BlockCellLR.Col = lpSS->Col.CurAt;
      lpSS->BlockCellLR.Row = lpSS->Row.CurAt;

      SSx_SelectBlock(lpSS, selBlock.LR.Col, selBlock.LR.Row);

      lpSS->fMultipleBlocksSelected = TRUE;
      fRet = SS_MultiSelBlockAddItem(lpSS, &selBlock.UL, &selBlock.LR);
      }
   }

SS_SheetUnlock(hWnd);
return (fRet);
}


GLOBALHANDLE DLLENTRY SSGetMultiSelBlocks(HWND hWnd, LPSHORT lpdSelBlockCnt)
{
LPSPREADSHEET lpSS;
GLOBALHANDLE  hListNew;

lpSS = SS_SheetLock(hWnd);
hListNew = SS_GetMultiSelBlocks(lpSS, lpdSelBlockCnt);

if (hListNew)
	{
	LPSS_SELBLOCK lpItemList = (LPSS_SELBLOCK)GlobalLock(hListNew);
	short i;

	for (i = 0; i < *lpdSelBlockCnt; i++)
		{
		SS_AdjustCellCoordsOut(lpSS, &lpItemList[i].UL.Col, &lpItemList[i].UL.Row);
		SS_AdjustCellCoordsOut(lpSS, &lpItemList[i].LR.Col, &lpItemList[i].LR.Row);
		}

	GlobalUnlock(hListNew);
	}

SS_SheetUnlock(hWnd);

return (hListNew);
}


GLOBALHANDLE SS_GetMultiSelBlocks(LPSPREADSHEET lpSS, LPSHORT lpdSelBlockCnt)
{
LPSS_SELBLOCK lpItemList;
SS_SELBLOCK   SelBlockTemp;
SS_SELBLOCK   SelBlockSplit;
GLOBALHANDLE  hList;
GLOBALHANDLE  hListNew = 0;
long          lTemp;
long          lMaxCells;
short         dItemCntOld;
short         dItemAllocCnt;
short         dItemCnt;
short         dItemAt;
short         dMaxItemNum;
short         dItemCntNew = 0;
short         dItemAllocCntNew = 0;
short         i;

if (lpSS->MultiSelBlock.dItemCnt > 0)
   {
   if (hList = SS_ListDup(lpSS->MultiSelBlock.hItemList,
       lpSS->MultiSelBlock.dItemCnt, sizeof(SS_SELBLOCK)))
      {
      dItemCnt = lpSS->MultiSelBlock.dItemCnt;
      dItemAllocCnt = lpSS->MultiSelBlock.dItemCnt;

      while (dItemCnt > 0)
         {
         lMaxCells = 0;
         dMaxItemNum = -1;

         lpItemList = (LPSS_SELBLOCK)GlobalLock(hList);

         for (i = 0; i < dItemCnt; i++)
            {
            if (lpItemList[i].LR.Col == -1)
               lTemp = SS_GetColCnt(lpSS) - lpItemList[i].UL.Col;
            else
               lTemp = lpItemList[i].LR.Col - lpItemList[i].UL.Col + 1;

            if (lpItemList[i].LR.Row == -1)
               lTemp = lTemp * (SS_GetRowCnt(lpSS) - lpItemList[i].UL.Row);
            else
               lTemp = lTemp * (lpItemList[i].LR.Row - lpItemList[i].UL.Row + 1);

            if (lTemp > lMaxCells)
               {
               dMaxItemNum = i;
               lMaxCells = lTemp;
               }
            }

         GlobalUnlock(hList);

         _fmemcpy(&SelBlockTemp, &lpItemList[dMaxItemNum], sizeof(SS_SELBLOCK));

         hListNew = SS_ListAddItem(hListNew, &dItemCntNew, &dItemAllocCntNew,
                                   &SelBlockTemp, sizeof(SS_SELBLOCK), 50);

         SS_ListDelItem(hList, dMaxItemNum, &dItemCnt, sizeof(SS_SELBLOCK));

         /*********************
         * Split rest of list
         *********************/

         for (dItemAt = 0, dItemCntOld = dItemCnt; dItemAt < dItemCntOld; )
            {
            lpItemList = (LPSS_SELBLOCK)GlobalLock(hList);

            if (SS_IntersectSelBlock(lpSS, &SelBlockTemp, &lpItemList[dItemAt]))
               {
               _fmemcpy(&SelBlockSplit, &lpItemList[dItemAt], sizeof(SS_SELBLOCK));

               SS_ListDelItem(hList, dItemAt, &dItemCnt, sizeof(SS_SELBLOCK));

               hList = SS_SelBlockSplitItem(hList, &dItemCnt, &dItemAllocCnt,
                                            &SelBlockTemp, &SelBlockSplit);

               dItemCntOld--;
               }
            else
               dItemAt++;

            GlobalUnlock(hList);
            }
         }

      GlobalFree(hList);
      }
   }

else if (SS_IsBlockSelected(lpSS))
   {
   _fmemcpy(&SelBlockTemp.UL, &lpSS->BlockCellUL, sizeof(SS_CELLCOORD));
   _fmemcpy(&SelBlockTemp.LR, &lpSS->BlockCellLR, sizeof(SS_CELLCOORD));

   hListNew = SS_ListAddItem(hListNew, &dItemCntNew, &dItemAllocCntNew,
                             &SelBlockTemp, sizeof(SS_SELBLOCK), 1);
   }

// BJO 25Apr96 SEL3651 - Begin of fix
if (hListNew && (lpItemList = (LPSS_SELBLOCK)GlobalLock(hListNew)))
   {
   for (i = 0; i < dItemCntNew; i++, lpItemList++)
      {
      if (lpSS->Col.HeaderCnt > lpItemList->UL.Col)
         lpItemList->UL.Col = -1;
      if (lpSS->Row.HeaderCnt > lpItemList->UL.Row)
         lpItemList->UL.Row = -1;
      }
   GlobalUnlock(hListNew);
   }
// BJO 25Apr96 SEL3651 - End of fix

*lpdSelBlockCnt = dItemCntNew;

return (hListNew);
}


GLOBALHANDLE SS_ListAddItem(GLOBALHANDLE hList, LPSHORT lpdItemCnt,
                            LPSHORT lpdItemAllocCnt, LPVOID lpItem,
                            short dItemLen, short dAllocInc)
{
LPBYTE lpList;

if (*lpdItemCnt + 1 > *lpdItemAllocCnt)
   {
   if (*lpdItemAllocCnt == 0)
      hList = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (long)(dItemLen *
                          (*lpdItemAllocCnt + dAllocInc)));
   else
      hList = GlobalReAlloc(hList, (long)(dItemLen * (*lpdItemAllocCnt +
                            dAllocInc)), GMEM_MOVEABLE | GMEM_ZEROINIT);

   if (!hList)
      return (FALSE);

   *lpdItemAllocCnt += SS_MULTISEL_ALLOC_CNT;
   }

lpList = (LPBYTE)GlobalLock(hList);

_fmemcpy(&lpList[*lpdItemCnt * dItemLen], lpItem, dItemLen);
(*lpdItemCnt)++;

GlobalUnlock(hList);
return (hList);
}


GLOBALHANDLE SS_ListDup(GLOBALHANDLE hList, short dItemCnt, short dItemSize)
{
GLOBALHANDLE hListNew = 0;
LPVOID       lpList;
LPVOID       lpListNew;

if (hList)
   {
   if (hListNew = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (long)(dItemSize *
                              dItemCnt)))
      {
      lpList = (LPVOID)GlobalLock(hList);
      lpListNew = (LPVOID)GlobalLock(hListNew);

      _fmemcpy(lpListNew, lpList, dItemSize * dItemCnt);

      GlobalUnlock(hList);
      GlobalUnlock(hListNew);
      }
   }

return (hListNew);
}


void SS_ListDelItem(GLOBALHANDLE hList, short dItemNum, LPSHORT lpdItemCnt,
                    short dItemLen)
{
LPBYTE lpList;

if (hList && *lpdItemCnt)
   {
   lpList = (LPBYTE)GlobalLock(hList);

   _fmemcpy(&lpList[dItemNum * dItemLen], &lpList[(dItemNum + 1) * dItemLen],
            dItemLen * (*lpdItemCnt - (dItemNum + 1)));
   (*lpdItemCnt)--;

   GlobalUnlock(hList);
   }
}


GLOBALHANDLE SS_SelBlockSplitItem(GLOBALHANDLE hList, LPSHORT lpdItemCnt,
                                  LPSHORT lpdItemAllocCnt,
                                  LPSS_SELBLOCK lpSelBlockOrig,
                                  LPSS_SELBLOCK lpSelBlockSplit)
{
SS_SELBLOCK SelBlockTemp;
SS_COORD    ULCol;
SS_COORD    ULRow;
SS_COORD    LRCol;
SS_COORD    LRRow;

ULCol = lpSelBlockSplit->UL.Col;
ULRow = lpSelBlockSplit->UL.Row;
LRCol = lpSelBlockSplit->LR.Col;
LRRow = lpSelBlockSplit->LR.Row;

if (ULRow < lpSelBlockOrig->UL.Row)
   {
   SSx_SetSelBlockStruct(&SelBlockTemp, ULCol, ULRow, LRCol,
                         min(LRRow, lpSelBlockOrig->UL.Row - 1));

   hList = SS_ListAddItem(hList, lpdItemCnt, lpdItemAllocCnt,
                          &SelBlockTemp, sizeof(SS_SELBLOCK), 50);

   ULRow = lpSelBlockOrig->UL.Row;
   }

if (ULCol < lpSelBlockOrig->UL.Col)
   {
   SSx_SetSelBlockStruct(&SelBlockTemp, ULCol, ULRow,
                         min(LRCol, lpSelBlockOrig->UL.Col - 1), LRRow);

   hList = SS_ListAddItem(hList, lpdItemCnt, lpdItemAllocCnt,
                          &SelBlockTemp, sizeof(SS_SELBLOCK), 50);

   ULCol = lpSelBlockOrig->UL.Col;
   }

//#ifdef SPREAD_JPN
//- JPNFIX0016 - (Masanori Iwasa)
if (LRCol > lpSelBlockOrig->LR.Col && lpSelBlockOrig->LR.Col != -1)
//#else
//if (LRCol > lpSelBlockOrig->LR.Col)
//#endif
   {
   SSx_SetSelBlockStruct(&SelBlockTemp, max(ULCol, lpSelBlockOrig->LR.Col + 1),
                         ULRow, LRCol, LRRow);

   hList = SS_ListAddItem(hList, lpdItemCnt, lpdItemAllocCnt,
                          &SelBlockTemp, sizeof(SS_SELBLOCK), 50);

   LRCol = lpSelBlockOrig->LR.Col;
   }

//#ifdef SPREAD_JPN
//- JPNFIX0016 - (Masanori Iwasa)
if (LRRow > lpSelBlockOrig->LR.Row && lpSelBlockOrig->LR.Row != -1)
//#else
//if (LRRow > lpSelBlockOrig->LR.Row)
//#endif
   {
   SSx_SetSelBlockStruct(&SelBlockTemp, ULCol, max(ULRow,
                         lpSelBlockOrig->LR.Row + 1), LRCol, LRRow);

   hList = SS_ListAddItem(hList, lpdItemCnt, lpdItemAllocCnt,
                          &SelBlockTemp, sizeof(SS_SELBLOCK), 50);

   LRCol = lpSelBlockOrig->LR.Col;
   }

return (hList);
}


void SSx_SetSelBlockStruct(LPSS_SELBLOCK lpSelBlockDest, SS_COORD ULCol,
                           SS_COORD ULRow, SS_COORD LRCol, SS_COORD LRRow)
{
lpSelBlockDest->UL.Col = ULCol;
lpSelBlockDest->UL.Row = ULRow;
lpSelBlockDest->LR.Col = LRCol;
lpSelBlockDest->LR.Row = LRRow;
}


BOOL SS_IntersectSelBlock(LPSPREADSHEET lpSS, LPSS_SELBLOCK lpSelBlock1, LPSS_SELBLOCK lpSelBlock2)
{
if (lpSelBlock1->LR.Row >= lpSelBlock2->UL.Row &&
    lpSelBlock1->UL.Row <= lpSelBlock2->LR.Row &&
    lpSelBlock1->LR.Col >= lpSelBlock2->UL.Col &&
    lpSelBlock1->UL.Col <= lpSelBlock2->LR.Col)
   return (TRUE);

//#ifdef SPREAD_JPN
//- JPNFIX0016 - (Masanori Iwasa)
if (lpSelBlock1->LR.Row == lpSelBlock2->LR.Row &&
    lpSelBlock1->UL.Row == lpSelBlock2->UL.Row &&
    lpSelBlock1->LR.Col == lpSelBlock2->LR.Col &&
    lpSelBlock1->UL.Col == lpSelBlock2->UL.Col)
   return (TRUE);

if (lpSelBlock1->LR.Row >= lpSelBlock2->UL.Row &&
    lpSelBlock1->UL.Row <= lpSelBlock2->LR.Row &&
    lpSelBlock1->LR.Col == -1 &&
    lpSelBlock1->UL.Col < lpSS->Col.HeaderCnt)
   return (TRUE);

if (lpSelBlock1->LR.Row == -1 &&
    lpSelBlock1->UL.Row < lpSS->Row.HeaderCnt &&
    lpSelBlock1->LR.Col >= lpSelBlock2->UL.Col &&
    lpSelBlock1->UL.Col <= lpSelBlock2->LR.Col)
   return (TRUE);
//#endif

return (FALSE);
}

#endif


WORD DLLENTRY SSSetSelBlockOptions(HWND hWnd, WORD wOption)
{
LPSS_BOOK lpBook;
WORD      wSelBlockOptionOld;

lpBook = SS_BookLock(hWnd);
wSelBlockOptionOld = SS_SetSelBlockOptions(lpBook, wOption);
SS_BookUnlock(hWnd);

return (wSelBlockOptionOld);
}


WORD SS_SetSelBlockOptions(LPSS_BOOK lpBook, WORD wOption)
{
WORD wSelBlockOptionOld;

wSelBlockOptionOld = lpBook->wSelBlockOption;

if (!lpBook->fEditModePermanent && wOption <= 0x0F)
   lpBook->wSelBlockOption = wOption;

return (wSelBlockOptionOld);
}


WORD DLLENTRY SSGetSelBlockOptions(HWND hWnd)
{
LPSS_BOOK lpBook;
WORD      wSelBlockOptionOld;

lpBook = SS_BookLock(hWnd);
wSelBlockOptionOld = SS_GetSelBlockOptions(lpBook);
SS_BookUnlock(hWnd);

return (wSelBlockOptionOld);
}


WORD SS_GetSelBlockOptions(LPSS_BOOK lpBook)
{
return lpBook->wSelBlockOption;
}


BOOL SS_MultiSelBlockAddItem(LPSPREADSHEET lpSS, LPSS_CELLCOORD lpUL,
                             LPSS_CELLCOORD lpLR)
{
LPSS_SELBLOCK lpItemList;

/*
if (lpSS->MultiSelBlock.hItemList)
   {
   short i;

   lpItemList = (LPSS_SELBLOCK)GlobalLock(lpSS->MultiSelBlock.hItemList);

   for (i = 0; i < lpSS->MultiSelBlock.dItemCnt; i++)
      {
      if (lpUL->Col >= lpItemList[i].UL.Col &&
          lpLR->Col <= lpItemList[i].LR.Col &&
          lpUL->Row >= lpItemList[i].UL.Row &&
          lpLR->Row <= lpItemList[i].LR.Row)
         {
         GlobalUnlock(lpSS->MultiSelBlock.hItemList);
         return (TRUE);
         }
      }

   GlobalUnlock(lpSS->MultiSelBlock.hItemList);
   }
*/

if (lpSS->MultiSelBlock.dItemCnt + 1 > lpSS->MultiSelBlock.dItemAllocCnt)
   {
   if (lpSS->MultiSelBlock.dItemAllocCnt == 0)
      lpSS->MultiSelBlock.hItemList = GlobalAlloc(GMEM_MOVEABLE |
                                       GMEM_ZEROINIT,
                                       (long)(sizeof(SS_SELBLOCK) *
                                       (lpSS->MultiSelBlock.dItemAllocCnt +
                                       SS_MULTISEL_ALLOC_CNT)));
   else
      lpSS->MultiSelBlock.hItemList = GlobalReAlloc(
                                       lpSS->MultiSelBlock.hItemList,
                                       (long)(sizeof(SS_SELBLOCK) *
                                       (lpSS->MultiSelBlock.dItemAllocCnt +
                                       SS_MULTISEL_ALLOC_CNT)),
                                       GMEM_MOVEABLE | GMEM_ZEROINIT);

   if (!lpSS->MultiSelBlock.hItemList)
      return (FALSE);

   lpSS->MultiSelBlock.dItemAllocCnt += SS_MULTISEL_ALLOC_CNT;
   }

lpItemList = (LPSS_SELBLOCK)GlobalLock(lpSS->MultiSelBlock.hItemList);

_fmemcpy(&lpItemList[lpSS->MultiSelBlock.dItemCnt].UL, lpUL, sizeof(SS_CELLCOORD));
_fmemcpy(&lpItemList[lpSS->MultiSelBlock.dItemCnt].LR, lpLR, sizeof(SS_CELLCOORD));
lpSS->MultiSelBlock.dItemCnt++;

GlobalUnlock(lpSS->MultiSelBlock.hItemList);
return (TRUE);
}


BOOL SS_BeginMultipleSelection(HWND hWnd, LPSPREADSHEET lpSS)
{
SS_CELLCOORD BlockCellUL;
SS_CELLCOORD BlockCellLR;

if (!lpSS->fMultipleBlocksSelected)
   {
   if (SS_IsBlockSelected(lpSS))
      {
      if (!SS_MultiSelBlockAddItem(lpSS, &lpSS->BlockCellUL,
                                   &lpSS->BlockCellLR))
         return (FALSE);
      }
   else
      {
      BlockCellUL.Col = lpSS->Col.CurAt;
      BlockCellUL.Row = lpSS->Row.CurAt;
      BlockCellLR.Col = lpSS->Col.CurAt;
      BlockCellLR.Row = lpSS->Row.CurAt;

      if (!SS_MultiSelBlockAddItem(lpSS, &BlockCellUL, &BlockCellLR))
         return (FALSE);
      }

   SS_MultiSelCompleteBlock(hWnd, lpSS);
   lpSS->fMultipleBlocksSelected = TRUE;
   }
else
   SS_MultiSelCompleteBlock(hWnd, lpSS);

return (TRUE);
}


void SS_MultiSelCompleteBlock(HWND hWnd, LPSPREADSHEET lpSS)
{
RECT Rect;
HRGN hRgn;

if (!SS_IsActiveSheet(lpSS))
	return;

if (!SS_CreateBlockRect(lpSS, &Rect, lpSS->Col.CurAt,
                        lpSS->Row.CurAt, lpSS->Col.CurAt, lpSS->Row.CurAt,
                        FALSE))
   SetRectEmpty(&Rect);

hRgn = CreateRectRgnIndirect(&Rect);

#ifndef SS_NODRAGDROP
{
HRGN hRgn2;

if (lpSS->lpBook->fAllowDragDrop && !lpSS->fMultipleBlocksSelected)
   if (hRgn2 = SS_CreateDragDropRect(lpSS))
      {
      CombineRgn(hRgn, hRgn, hRgn2, RGN_XOR);
      DeleteObject(hRgn2);
      }
}
#endif

hRgn = SSxx_SelectBlock(lpSS, hRgn, &Rect,
                        (short)(lpSS->MultiSelBlock.dItemCnt - 1));

if ((lpSS->lpBook->hWnd && GetFocus() == lpSS->lpBook->hWnd) || lpSS->lpBook->fRetainSelBlock)
	{
	if (SS_INVERTSEL(lpSS))
		{
		HDC hDC = SS_GetDC(lpSS->lpBook);
		SelectClipRgn(hDC, hRgn);
		InvertRgn(hDC, hRgn);
		SelectClipRgn(hDC, 0);
		SS_ReleaseDC(lpSS->lpBook, hDC);
		}
	else
		InvalidateRgn(lpSS->lpBook->hWnd, hRgn, FALSE);
	}

DeleteObject(hRgn);
}


BOOL DLLENTRY SSIsCellInSelection(HWND hWnd, SS_COORD Col, SS_COORD Row)
{
LPSPREADSHEET lpSS;
BOOL          bRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
bRet = SS_IsCellInSelection(lpSS, Col, Row);
SS_SheetUnlock(hWnd);
return bRet;
}


BOOL SS_IsCellInSelection(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
BOOL bRet = FALSE;

if (lpSS->MultiSelBlock.dItemCnt > 0)
   {
	LPSS_SELBLOCK lpItemList;
	SS_COORD      i;

   lpItemList = (LPSS_SELBLOCK)GlobalLock(lpSS->MultiSelBlock.hItemList);

   for (i = 0; !bRet && i < lpSS->MultiSelBlock.dItemCnt; i++)
      if ((Col == -1 || lpItemList[i].UL.Col == -1 || lpItemList[i].LR.Col == -1 ||
           (Col >= lpItemList[i].UL.Col &&
            Col <= lpItemList[i].LR.Col)) &&
          (Row == -1 || lpItemList[i].UL.Row == -1 || lpItemList[i].LR.Row == -1 ||
           (Row >= lpItemList[i].UL.Row &&
            Row <= lpItemList[i].LR.Row)))
         bRet = TRUE;

   GlobalUnlock(lpSS->MultiSelBlock.hItemList);
   }

else if (SS_IsBlockSelected(lpSS))
   if ((Col == -1 || lpSS->BlockCellUL.Col == -1 || lpSS->BlockCellLR.Col == -1 ||
        (Col >= lpSS->BlockCellUL.Col &&
         Col <= lpSS->BlockCellLR.Col)) &&
       (Row == -1 || lpSS->BlockCellUL.Row == -1 || lpSS->BlockCellLR.Row == -1 ||
        (Row >= lpSS->BlockCellUL.Row &&
         Row <= lpSS->BlockCellLR.Row)))
      bRet = TRUE;

return bRet;
}


//#ifdef SS_V40
void SS_KillExtSelF8Mode(LPSPREADSHEET lpSS)
{
if (lpSS && lpSS->nExtSelF8Mode)
	{
	KillTimer(lpSS->lpBook->hWnd, SS_F8EXTSEL_TIMER_ID);
	if (1 == lpSS->nExtSelF8Mode)
		SS_HighlightCell(lpSS, FALSE);

	lpSS->nExtSelF8Mode = 0;
	}
}
//#endif // SS_V40
