/*********************************************************
* SS_OVERF.C
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

#define OEMRESOURCE
#define DRV_NOPATTERN
#define DRV_NOGAUGE

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
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_span.h"
#include "ss_type.h"
#include "ss_win.h"
#ifdef SS_V80
#include "cust_celltype.h"
#endif

#ifndef SS_NOOVERFLOW
BOOL SSx_CalcCellOverflow(LPSPREADSHEET lpSS, BYTE bOption,
                          SS_COORD Col, SS_COORD Row);
BOOL SSxx_ResetCellOverflow(LPSPREADSHEET lpSS, BYTE bOption,
                            SS_COORD Col, SS_COORD Row);
BOOL SS_GetOverflowCoords(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                          LPSHORT lpdWidthPrev, LPSHORT lpdWidthNext);
BOOL SS_OverflowInvalidateData(LPSPREADSHEET lpSS,
                               SS_COORD Col, SS_COORD Row);

/* RFW - 6/22/04 - 14597
#define SS_OF_LEFTWALL(lpSS, ColStart) (lpSS->Col.HeaderCnt + (ColStart < \
                                        lpSS->Col.HeaderCnt + lpSS->Col.Frozen ? \
                                        0 : lpSS->Col.Frozen))
*/
#define SS_OF_LEFTWALL(lpSS, ColStart) (lpSS->Col.HeaderCnt + (ColStart < \
                                        lpSS->Col.HeaderCnt + lpSS->Col.Frozen || \
                                        ColStart == lpSS->Col.UL ? \
                                        0 : lpSS->Col.Frozen))

BOOL SS_CalcOverflowRect(HDC hDC, LPSPREADSHEET lpSS, LPTSTR Data,
                         SS_COORD Col, UINT Just, LPRECT lpRect,
                         LPRECT lpCellOverflowRectPrev,
                         LPRECT lpCellOverflowRectNext, LPRECT lpRectNew)
{
BOOL fRet = FALSE;

if (!lpSS->lpBook->fAllowCellOverflow)
   return (FALSE);

CopyRect(lpRectNew, lpRect);

if (Just & DT_RIGHT)
   {
   if (lpCellOverflowRectPrev)
      {
      CopyRect(lpRectNew, lpCellOverflowRectPrev);
      fRet = TRUE;
      }
   }

else if (Just & DT_CENTER)
   {
   fRet = TRUE;

   if (lpCellOverflowRectPrev && lpCellOverflowRectNext)
      UnionRect(lpRectNew, lpCellOverflowRectPrev, lpCellOverflowRectNext);

   else if (lpCellOverflowRectPrev)
      CopyRect(lpRectNew, lpCellOverflowRectPrev);

   else if (lpCellOverflowRectNext)
      CopyRect(lpRectNew, lpCellOverflowRectNext);

   else
      fRet = FALSE;
   }

else
   {
   if (lpCellOverflowRectNext)
      {
      CopyRect(lpRectNew, lpCellOverflowRectNext);
      fRet = TRUE;
      }
   }

return (fRet);
}


BOOL SS_CalcCellOverflow(LPSPREADSHEET lpSS, LPSS_CELL lpCell,
                         LPSS_CELLTYPE lpCellType, SS_COORD Col, SS_COORD Row)
{
TBGLOBALHANDLE hData = 0;
SS_CELLTYPE    CellType;
LPSS_COL       lpCol;
LPSS_ROW       lpRow;
RECT           Rect;
HDC            hDC;
BOOL           fRet = FALSE;
int            iWidthMax;
short          x;
int            dWidth;
short          Just;
SS_COORD       i;
#ifdef SS_V80  
LPTSTR lpszName;
BOOL   bCanOverflow = TRUE;
if( lpSS->lpBook->fLoading )
	return TRUE;
#endif
if( lpSS->lpBook->fLoading )
	return TRUE;

if (!lpSS->lpBook->fAllowCellOverflow)
   return (TRUE);

if (Row < SS_GetRowCnt(lpSS) && Col < SS_GetColCnt(lpSS))
   {
   if (!lpCellType)
      {
      SS_RetrieveCellType(lpSS, &CellType, lpCell, Col, Row);
      lpCellType = &CellType;
      }

   if ((lpCellType->Type == SS_TYPE_STATICTEXT &&
        (lpCellType->Style & SS_TEXT_WORDWRAP)) ||
       (lpCellType->Type == SS_TYPE_EDIT &&
        (lpCellType->Style & ES_MULTILINE)) ||
       lpCellType->Type == SS_TYPE_PICTURE ||
       lpCellType->Type == SS_TYPE_COMBOBOX ||
       lpCellType->Type == SS_TYPE_BUTTON ||
       lpCellType->Type == SS_TYPE_CHECKBOX)
      return (TRUE);

#ifdef SS_V80
	else if (lpCellType->Type == SS_TYPE_CUSTOM)
	{
		if( lpszName = (LPTSTR)GlobalLock(lpCellType->Spec.Custom.hName) )
			SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, &bCanOverflow, NULL, NULL, NULL);
		GlobalUnlock(CellType.Spec.Custom.hName);
		if (!bCanOverflow)
			return (TRUE);
	}
#endif


   SS_OverflowInvalidateData(lpSS, Col, Row);

#ifdef SS_V40
	// RFW - 4/22/02 - 9979
	if (SS_GetCellSpan(lpSS, Col, Row, NULL, NULL, NULL, NULL))
		return (TRUE);
#endif

   if (lpSS->lpBook->hWnd && (hDC = SS_GetDC(lpSS->lpBook)))
      {
      lpRow = SS_LockRowItem(lpSS, Row);
      lpCol = SS_LockColItem(lpSS, Col);

      Rect.left = 0;
      Rect.top = 0;
      Rect.right = SS_GetColWidthInPixels(lpSS, Col) - 1;
      Rect.bottom = SS_GetRowHeightInPixels(lpSS, Row) - 1;

      // BJO 23Oct96 GRB5117 - Before fix
      //if (Rect.right > -1 && Rect.bottom > -1)
      // BJO 23Oct96 GRB5117 - Begin fix
      if (Rect.right > -1)
      // BJO 23Oct96 GRB5117 - After fix
         {
         RECT RectTemp;

         CopyRect(&RectTemp, &Rect);
         SS_CalcCellMetrics(hDC, lpSS, lpCol, lpRow, lpCell,
                            lpCellType, &RectTemp, Col, Row, TRUE,
                            TRUE, &Just, NULL, &iWidthMax, NULL);

         if (iWidthMax >= Rect.right)
            {
            if (Just & DT_RIGHT)
               {
               for (dWidth = Rect.right + 1, i = Col - 1;
                    i >= SS_OF_LEFTWALL(lpSS, Col) &&
                    iWidthMax >= dWidth; i--)
                  {
                  if (!SSx_CalcCellOverflow(lpSS, SS_OVERFLOW_RIGHT, i, Row))
                     break;

                  dWidth += SS_GetColWidthInPixels(lpSS, i);
                  }
               }

            else if (Just & DT_CENTER)
               {
               x = (short)(Rect.right + ((iWidthMax - Rect.right) / 2));

               for (dWidth = Rect.right + 1, i = Col - 1;
                    i >= SS_OF_LEFTWALL(lpSS, Col) &&
                    x >= dWidth; i--)
                  {
                  if (!SSx_CalcCellOverflow(lpSS, SS_OVERFLOW_RIGHT, i, Row))
                     break;

                  dWidth += SS_GetColWidthInPixels(lpSS, i);
                  }

               for (dWidth = Rect.right + 1, i = Col + 1; i < SS_GetColCnt(lpSS) &&
                    x >= dWidth; i++)
                  {
                  if (!SSx_CalcCellOverflow(lpSS, SS_OVERFLOW_LEFT, i, Row))
                     break;

                  dWidth += SS_GetColWidthInPixels(lpSS, i);
                  }
               }

            else                             // DT_LEFT
               {
               for (dWidth = Rect.right + 1, i = Col + 1; i < SS_GetColCnt(lpSS) &&
                    iWidthMax >= dWidth; i++)
                  {
                  if (!SSx_CalcCellOverflow(lpSS, SS_OVERFLOW_LEFT, i, Row))
                     break;

                  dWidth += SS_GetColWidthInPixels(lpSS, i);
                  }
               }
            }
         }

      SS_UnlockColItem(lpSS, Col);
      SS_UnlockRowItem(lpSS, Row);

      SS_ReleaseDC(lpSS->lpBook, hDC);
		// RFW - 6/11/04 - 14369
		SS_OverflowInvalidateCell(lpSS, Col, Row);

      fRet = TRUE;
      }
   }

return (fRet);
}


BOOL SSx_CalcCellOverflow(LPSPREADSHEET lpSS, BYTE bOption,
                          SS_COORD Col, SS_COORD Row)
{
SS_CELLTYPE CellType;
LPSS_CELL   lpCell;
BOOL        fRet = FALSE;
#ifdef SS_V80  
LPTSTR lpszName;
BOOL   bCanBeOverflown = TRUE;
#endif
// RFW - 6/21/04 - 14605
#ifdef SS_V40
if (SS_GetCellSpan(lpSS, Col, Row, NULL, NULL, NULL, NULL))
	return (FALSE);
#endif

if (lpCell = SS_LockCellItem(lpSS, Col, Row))
   {
   SS_RetrieveCellType(lpSS, &CellType, lpCell, Col, Row);
#ifdef SS_V80
	if (!lpCell->Data.bDataType && (&CellType != NULL && CellType.Type == SS_TYPE_CUSTOM))
	{
		if( lpszName = (LPTSTR)GlobalLock(CellType.Spec.Custom.hName) )
			SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, &bCanBeOverflown, NULL, NULL);
		GlobalUnlock(CellType.Spec.Custom.hName);
		if (!lpCell->Data.bOverflow && bCanBeOverflown)
		{
			lpCell->Data.bOverflow |= bOption;
			fRet = TRUE;
		}

	}
   else if (!lpCell->Data.bDataType &&
#else
   if (!lpCell->Data.bDataType &&
#endif
       (&CellType == NULL ||
        (CellType.Type != SS_TYPE_PICTURE && CellType.Type != SS_TYPE_BUTTON &&
         CellType.Type != SS_TYPE_COMBOBOX &&
         CellType.Type != SS_TYPE_CHECKBOX &&
         CellType.Type != SS_TYPE_OWNERDRAW)))
      {
      if (!lpCell->Data.bOverflow)
			{
			lpCell->Data.bOverflow |= bOption;
			fRet = TRUE;
			}
      }

   SS_UnlockCellItem(lpSS, Col, Row);
   }

else
   {
   if (lpCell = SS_AllocLockCell(lpSS, NULL, Col, Row))
      {
     SS_RetrieveCellType(lpSS, &CellType, lpCell, Col, Row);
#ifdef SS_V80
	if (!lpCell->Data.bOverflow && (&CellType != NULL && CellType.Type == SS_TYPE_CUSTOM))
	{
		if( lpszName = (LPTSTR)GlobalLock(CellType.Spec.Custom.hName) )
			SS_CustCellTypeLookup(lpSS->lpBook, lpszName, NULL, NULL, &bCanBeOverflown, NULL, NULL);
		GlobalUnlock(CellType.Spec.Custom.hName);
		if (!lpCell->Data.bOverflow && bCanBeOverflown)
		{
			lpCell->Data.bOverflow |= bOption;
			fRet = TRUE;
		}

	}
	else if (!lpCell->Data.bOverflow &&
#else
	if (!lpCell->Data.bOverflow &&
#endif
			 (&CellType == NULL ||
			  (CellType.Type != SS_TYPE_PICTURE && CellType.Type != SS_TYPE_BUTTON &&
				CellType.Type != SS_TYPE_COMBOBOX &&
				CellType.Type != SS_TYPE_CHECKBOX &&
				CellType.Type != SS_TYPE_OWNERDRAW)))
			{
			lpCell->Data.bOverflow |= bOption;
			fRet = TRUE;
			}

      SS_UnlockCellItem(lpSS, Col, Row);
      }
   }

return (fRet);
}

BOOL SS_ResetCellOverflowNeighbor(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
BOOL fRet = SSx_ResetCellOverflow(lpSS, Col, Row, NULL, NULL);

SSxx_ResetCellOverflow(lpSS, SS_OVERFLOW_LEFT | SS_OVERFLOW_RIGHT, Col, Row);

return (fRet);
}


BOOL SS_ResetCellOverflow(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
return (SS_OverflowInvalidateData(lpSS, Col, Row));
}


BOOL SSx_ResetCellOverflow(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                           LPSHORT lpWidthPrev, LPSHORT lpWidthNext)
{
BOOL     fRet = FALSE;
SS_COORD i;

if (!lpSS->lpBook->fAllowCellOverflow)
   return (TRUE);

if (lpWidthPrev)
   *lpWidthPrev = 0;

if (lpWidthNext)
   *lpWidthNext = 0;

if (Row != -1 && Row < SS_GetRowCnt(lpSS) && Col != -1 &&
    Col < SS_GetColCnt(lpSS))
   {
   for (i = Col - 1; i >= SS_OF_LEFTWALL(lpSS, Col); i--)
      if (!SSxx_ResetCellOverflow(lpSS, SS_OVERFLOW_RIGHT, i, Row))
         break;
      else if (lpWidthPrev)
         *lpWidthPrev += SS_GetColWidthInPixels(lpSS, i);

   for (i = Col + 1; i < SS_GetColCnt(lpSS); i++)
      if (!SSxx_ResetCellOverflow(lpSS, SS_OVERFLOW_LEFT, i, Row))
         break;
      else if (lpWidthNext)
         *lpWidthNext += SS_GetColWidthInPixels(lpSS, i);

   fRet = TRUE;
   }

return (fRet);
}


BOOL SSxx_ResetCellOverflow(LPSPREADSHEET lpSS, BYTE bOption,
                            SS_COORD Col, SS_COORD Row)
{
LPSS_CELL lpCell;
BOOL      fRet = FALSE;

if (lpCell = SS_LockCellItem(lpSS, Col, Row))
   {
   if (lpCell->Data.bOverflow & bOption)
      {
      lpCell->Data.bOverflow &= ~bOption;
      fRet = TRUE;
      }

   SS_UnlockCellItem(lpSS, Col, Row);
   }

return (fRet);
}


void SS_DrawHiddenOverflowCellLeft(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                                   LPSS_ROW lpRow, LPRECT lpRect, SS_COORD Col,
                                   SS_COORD Row, LPSSX_PRINTFORMAT lpPFormat,
                                   LPRECT lpRectUpdate, SS_COORD CellLeft, SS_COORD CellTop,
                                   BOOL IsLastCol, BOOL IsLastRow)
{
LPSS_CELL lpCellTemp;
LPSS_COL  lpCol;
RECT      RectOverflowPrev;
RECT      RectOverflow;
RECT      RectTemp;
RECT      RectClip;
BOOL      fOverflow;
double    dfWidth;
double    dfWidthTotal;
SS_COORD  ColOrig = Col;
SS_COORD  i;

SaveDC(hDC);

// BJO 01Apr98 JEH383 - Before fix
//IntersectClipRect(hDC, lpRect->left, lpRect->top, lpRect->right,
//                  lpRect->bottom);
// BJO 01Apr98 JEH383 - Begin fix
SS_CopyCellRect(lpSS, &RectClip, lpRect, lpPFormat, TRUE, IsLastCol, IsLastRow);
IntersectClipRect(hDC, RectClip.left, RectClip.top, RectClip.right,
                  RectClip.bottom);
// BJO 01Apr98 JEH383 - End fix

for (i = Col - 1, dfWidth = 0, dfWidthTotal = 0, fOverflow = TRUE; fOverflow &&
     i >= lpSS->Col.HeaderCnt; i--)
   {
   dfWidth = SS_GetColWidthInPixelsExDF(lpSS, i, lpPFormat);
   dfWidthTotal += dfWidth;
   Col--;

   fOverflow = FALSE;

   if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, i, Row))
      {
      if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_LEFT)
         fOverflow = TRUE;

      SS_UnlockCellItem(lpSS, i, Row);
      }
   }

CopyRect(&RectOverflow, lpRect);
RectOverflow.left -= (int)dfWidthTotal;
CopyRect(&RectTemp, &RectOverflow);
RectTemp.right = RectTemp.left + (int)dfWidth - 1;

/***************************************************************
* RFW - 2/29/96
*
* For some unknown reason this code was commented out.  I am
* putting it back in to fix KEM3257, which is overflowing text
* that is centered.
***************************************************************/

for (i = Col - 1, dfWidthTotal = 0, fOverflow = TRUE; fOverflow &&
     i >= lpSS->Col.HeaderCnt; i--)
   {
   fOverflow = FALSE;

   if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, i, Row))
      {
      if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_RIGHT)
         {
         dfWidthTotal += SS_GetColWidthInPixelsExDF(lpSS, i, lpPFormat);
         fOverflow = TRUE;
         }

      SS_UnlockCellItem(lpSS, i, Row);
      }
   }

CopyRect(&RectOverflowPrev, &RectTemp);
RectOverflowPrev.left -= (int)dfWidthTotal;

lpCol = SS_LockColItem(lpSS, Col);

SS_DrawText(hDC, hWnd, lpSS, lpCol, lpRow, &RectTemp, Col, Row, FALSE,
            lpPFormat, lpRectUpdate, &RectOverflowPrev, &RectOverflow, FALSE,
            SS_ShouldCellBeDrawnSelected(lpSS, ColOrig, Row, lpRow),
            CellLeft, CellTop, IsLastCol, IsLastRow);

#ifdef SS_V35
if (lpCellTemp = SS_LockCellItem(lpSS, Col, Row) )
   {
   if (lpCellTemp->hCellNote)
      SS_DrawCellNote(lpSS, hDC, NULL, Col, Row);
   SS_UnlockCellItem(lpSS, Col, Row);
   }
#endif

SS_UnlockColItem(lpSS, Col);

RestoreDC(hDC, -1);
}


void SS_DrawHiddenOverflowCellRight(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                                    LPSS_ROW lpRow, LPRECT lpRect,
                                    SS_COORD Col, SS_COORD Row,
                                    LPSSX_PRINTFORMAT lpPFormat,
                                    LPRECT lpRectUpdate, SS_COORD CellLeft, SS_COORD CellTop,
                                    BOOL IsLastCol, BOOL IsLastRow)
{
LPSS_CELL lpCellTemp;
LPSS_COL  lpCol;
RECT      RectOverflow;
RECT      RectOverflowNext;
RECT      RectTemp;
RECT      RectClip;
BOOL      fOverflow;
double    dfWidth;
double    dfWidthTotal;
SS_COORD  ColOrig = Col;
SS_COORD  i;
#ifdef SS_V35
BOOL      fCellNote = FALSE;
#endif

SaveDC(hDC);

// BJO 01Apr98 JEH383 - Before fix
//IntersectClipRect(hDC, lpRect->left, lpRect->top, lpRect->right + 1,
//                  lpRect->bottom);
// BJO 01Apr98 JEH383 - Begin fix
SS_CopyCellRect(lpSS, &RectClip, lpRect, lpPFormat, TRUE, IsLastCol, IsLastRow);
IntersectClipRect(hDC, RectClip.left, RectClip.top, RectClip.right + 1,
                  RectClip.bottom);
// BJO 01Apr98 JEH383 - End fix

for (i = Col + 1, dfWidth = 0, dfWidthTotal = 0, fOverflow = TRUE; fOverflow &&
     i < SS_GetColCnt(lpSS); i++)
   {
   dfWidth = SS_GetColWidthInPixelsExDF(lpSS, i, lpPFormat);
   dfWidthTotal += dfWidth;
   Col++;

   fOverflow = FALSE;

   if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, i, Row))
      {
      if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_RIGHT)
         fOverflow = TRUE;

      SS_UnlockCellItem(lpSS, i, Row);
      }
   }

CopyRect(&RectOverflow, lpRect);
RectOverflow.right += (int)dfWidthTotal;
CopyRect(&RectTemp, &RectOverflow);
RectTemp.left = RectTemp.right - (int)dfWidth + 1;

for (i = Col + 1, dfWidthTotal = 0, fOverflow = TRUE; fOverflow &&
     i < SS_GetColCnt(lpSS); i++)
   {
   fOverflow = FALSE;

   if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, i, Row))
      {
      if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_LEFT)
         {
         dfWidthTotal += SS_GetColWidthInPixelsExDF(lpSS, i, lpPFormat);
         fOverflow = TRUE;
         }

      SS_UnlockCellItem(lpSS, i, Row);
      }
   }

CopyRect(&RectOverflowNext, &RectTemp);
RectOverflowNext.right += (int)dfWidthTotal;

fOverflow = FALSE;

if (ColOrig - 1 >= lpSS->Col.HeaderCnt)
   if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, ColOrig - 1, Row))
      {
      if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_RIGHT)
         fOverflow = TRUE;

      SS_UnlockCellItem(lpSS, ColOrig - 1, Row);
      }

lpCol = SS_LockColItem(lpSS, Col);

SS_DrawText(hDC, hWnd, lpSS, lpCol, lpRow, &RectTemp, Col, Row, FALSE,
            lpPFormat, lpRectUpdate, &RectOverflow, &RectOverflowNext,
            fOverflow, SS_ShouldCellBeDrawnSelected(lpSS, ColOrig, Row, lpRow),
            CellLeft, CellTop, IsLastCol, IsLastRow);
#ifdef SS_V35
if (lpCellTemp = SS_LockCellItem(lpSS, Col, Row) )
   {
   if (lpCellTemp->hCellNote)
      SS_DrawCellNote(lpSS, hDC, NULL, Col, Row);
   SS_UnlockCellItem(lpSS, Col, Row);
   }
#endif
SS_UnlockColItem(lpSS, Col);

RestoreDC(hDC, -1);
}


BOOL SS_OverflowInvalidateCell(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
RECT      Rect;
RECT      RectOverlap;
BOOL      fRet = FALSE;
short     dWidthPrev = 0;
short     dWidthNext = 0;
int       x;
int       y;
int       cx;
int       cy;

if (SS_GetOverflowCoords(lpSS, Col, Row, &dWidthPrev, &dWidthNext))
   {
   int xTra = 0;

   SS_GetClientRect(lpSS->lpBook, &Rect);
   SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, Col, Row,
                   &x, &y, &cx, &cy);

   if (lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID ||
       lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERVERTGRIDONLY)
      xTra = 1;

   SetRect(&RectOverlap, x, y + 1, x + cx + xTra, y + cy + xTra);

   if (dWidthPrev)
      RectOverlap.left = max(Rect.left, RectOverlap.left - dWidthPrev);

   if (dWidthNext)
      RectOverlap.right = min(Rect.right, RectOverlap.right + dWidthNext);

   SS_InvalidateRect(lpSS->lpBook, &RectOverlap, TRUE);
   fRet = TRUE;
   }

return (fRet);
}


WORD SS_GetOverflowRects(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                         LPRECT lpRectClient, LPRECT lpRect,
                         LPRECT lpRectOverflowPrev,
                         LPRECT lpRectOverflowNext,
                         LPBOOL lpfOverflowToLeft,
                         LPRECT lpRectOverflowAll)
{
WORD  wRet = 0;
short dWidthPrev = 0;
short dWidthNext = 0;

if (lpfOverflowToLeft)
   *lpfOverflowToLeft = FALSE;

if (SS_GetOverflowCoords(lpSS, Col, Row, &dWidthPrev, &dWidthNext))
   {
   if (lpRectOverflowAll)
      CopyRect(lpRectOverflowAll, lpRect);

   if (dWidthPrev)
      {
      if (lpRectOverflowPrev)
         SetRect(lpRectOverflowPrev, max(lpRectClient->left, lpRect->left -
                 dWidthPrev), lpRect->top + 1, lpRect->left,
                 min(lpRectClient->right, lpRect->bottom));

      if (lpfOverflowToLeft)
         if (lpRectClient->left > lpRect->left - dWidthPrev)
            *lpfOverflowToLeft = TRUE;

      if (lpRectOverflowAll)
         lpRectOverflowAll->left = max(lpRectClient->left, lpRect->left -
                                      dWidthPrev);
      }

   if (dWidthNext)
      {
      if (lpRectOverflowNext)
         SetRect(lpRectOverflowNext, lpRect->right, lpRect->top,
                 min(lpRectClient->right, lpRect->right + dWidthNext),
                 lpRect->right);

      if (lpRectOverflowAll)
         lpRectOverflowAll->right = min(lpRectClient->right, lpRect->right +
                                       dWidthNext);
      }

   wRet = MAKEWORD((BYTE)(dWidthPrev ? 1 : 0), (BYTE)(dWidthNext ? 1 : 0));
   }

return (wRet);
}


BOOL SS_GetOverflowCoords(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                          LPSHORT lpdWidthPrev, LPSHORT lpdWidthNext)
{
LPSS_CELL lpCell;
BOOL      fFound;
BOOL      fRet = FALSE;
SS_COORD  i;

*lpdWidthPrev = 0;
*lpdWidthNext = 0;

if (Row != -1 && Row < SS_GetRowCnt(lpSS) && Col != -1 &&
    Col < SS_GetColCnt(lpSS))
   {
   for (i = Col - 1, fFound = TRUE; fFound && i >= SS_OF_LEFTWALL(lpSS, Col); i--)
      {
      fFound = FALSE;

      if (lpCell = SS_LockCellItem(lpSS, i, Row))
         {
         if (lpCell->Data.bOverflow & SS_OVERFLOW_RIGHT)
            {
            *lpdWidthPrev += SS_GetColWidthInPixels(lpSS, i);
            fFound = TRUE;
            }

         SS_UnlockCellItem(lpSS, i, Row);
         }
      }

   for (i = Col + 1, fFound = TRUE; fFound && i < SS_GetColCnt(lpSS); i++)
      {
      fFound = FALSE;

      if (lpCell = SS_LockCellItem(lpSS, i, Row))
         {
         if (lpCell->Data.bOverflow & SS_OVERFLOW_LEFT)
            {
            *lpdWidthNext += SS_GetColWidthInPixels(lpSS, i);
            fFound = TRUE;
            }

         SS_UnlockCellItem(lpSS, i, Row);
         }
      }
   }

if (*lpdWidthPrev || *lpdWidthNext)
   fRet = TRUE;

return (fRet);
}


BOOL SS_OverflowInvalidateData(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row)
{
LPSS_CELL lpCell;
RECT      RectTemp;
RECT      RectOverlap;
short     dWidthPrev;
short     dWidthNext;
int       x;
int       y;
int       cx;
int       cy;
BOOL      fInvalidateCurrentCell = FALSE;
BOOL      fRet = FALSE;

if (lpSS->lpBook->fAllowCellOverflow)
   {
   if (lpCell = SS_LockCellItem(lpSS, Col, Row))
      {
      if (lpCell->Data.bOverflow)
         fInvalidateCurrentCell = TRUE;

      SS_UnlockCellItem(lpSS, Col, Row);
      }

   if (SSx_ResetCellOverflow(lpSS, Col, Row, &dWidthPrev, &dWidthNext))
      {
      if (dWidthPrev || dWidthNext)
         {
         SS_GetScrollArea(lpSS, &RectTemp, SS_SCROLLAREA_CELLS);

         SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL,
                         Col, Row, &x, &y, &cx, &cy);

         if (dWidthPrev)
            {
            SetRect(&RectOverlap, max(Col < lpSS->Col.HeaderCnt +
                    lpSS->Col.Frozen ? 0 : RectTemp.left, x - dWidthPrev), y,
                    min(RectTemp.right, x), y + cy);
            SS_InvalidateRect(lpSS->lpBook, &RectOverlap, TRUE);
            }

         if (dWidthNext)
            {
            SetRect(&RectOverlap, x + cx - 1, y, min(RectTemp.right,
                    x + cx + dWidthNext), y + cy);
            SS_InvalidateRect(lpSS->lpBook, &RectOverlap, TRUE);
            }

         if (fInvalidateCurrentCell)
            {
            SetRect(&RectOverlap, x, y + 1, x + cx, y + cy);
            SS_InvalidateRect(lpSS->lpBook, &RectOverlap, TRUE);
            }
         }

      fRet = TRUE;
      }
   }

return (fRet);
}


void SS_OverflowAdjustNeighbor(LPSPREADSHEET lpSS, SS_COORD Col,
                               SS_COORD Row)
{
SS_CELLTYPE CellType;
LPSS_CELL   lpCellCurrent;
LPSS_CELL   lpCell;
BOOL        fFound;
BOOL        fDone = FALSE;
SS_COORD    i;

if (lpSS->lpBook->fAllowCellOverflow)
   {
   lpCellCurrent = SS_LockCellItem(lpSS, Col, Row);

   if (!lpCellCurrent || !lpCellCurrent->Data.bDataType ||
       (lpCellCurrent->Data.bOverflow & SS_OVERFLOW_LEFT))
      {
      for (i = Col - 1, fFound = TRUE; fFound &&
           i >= SS_OF_LEFTWALL(lpSS, Col); i--)
         {
         fFound = FALSE;

         if (lpCell = SS_LockCellItem(lpSS, i, Row))
            {
            if (lpCell->Data.bOverflow & SS_OVERFLOW_LEFT)
               fFound = TRUE;
            else if (lpCell->Data.bDataType)
               if (SS_RetrieveCellType(lpSS, &CellType, lpCell, i, Row))
                  {
                  SS_CalcCellOverflow(lpSS, lpCell, &CellType, i, Row);
                  SS_OverflowInvalidateCell(lpSS, i, Row);
                  fDone = TRUE;
                  }

            SS_UnlockCellItem(lpSS, i, Row);
            }
         }
      }

   if (!fDone && (!lpCellCurrent || !lpCellCurrent->Data.bDataType ||
       (lpCellCurrent->Data.bOverflow & SS_OVERFLOW_RIGHT)))
      {
      for (i = Col + 1, fFound = TRUE; fFound && i < SS_GetColCnt(lpSS);
           i++)
         {
         fFound = FALSE;

         if (lpCell = SS_LockCellItem(lpSS, i, Row))
            {
            if (lpCell->Data.bOverflow & SS_OVERFLOW_RIGHT)
               fFound = TRUE;
            else if (lpCell->Data.bDataType)
               if (SS_RetrieveCellType(lpSS, &CellType, lpCell, i, Row))
                  {
                  SS_CalcCellOverflow(lpSS, lpCell, &CellType, i, Row);
                  SS_OverflowInvalidateCell(lpSS, i, Row);
                  fDone = TRUE;
                  }

            SS_UnlockCellItem(lpSS, i, Row);
            }
         }
      }

   if (!fDone && lpCellCurrent && lpCellCurrent->Data.bDataType)
      {
      if (SS_RetrieveCellType(lpSS, &CellType, lpCellCurrent, Col, Row))
         {
         SS_CalcCellOverflow(lpSS, lpCellCurrent, &CellType, Col, Row);
         SS_OverflowInvalidateCell(lpSS, Col, Row);
         }
      }

	if (lpCellCurrent)
		SS_UnlockCellItem(lpSS, Col, Row);
   }
}

#endif // SS_NOOVERFLOW
