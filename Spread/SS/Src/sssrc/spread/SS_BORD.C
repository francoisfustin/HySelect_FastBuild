/*********************************************************
* SS_BORD.C
*
* Copyright (C) 1991-1998 - FarPoint Technologies
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
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_bord.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_user.h"

#ifndef SS_NOBORDERS

BOOL SS_BorderApply(LPSS_CELLBORDER lpCellBorder, TBGLOBALHANDLE hBorder);


BOOL DLLENTRY SSGetBorder(hWnd, Col, Row, lpwBorderType, lpwBorderStyle,
                          lpColor)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
LPWORD        lpwBorderType;
LPWORD        lpwBorderStyle;
LPCOLORREF    lpColor;
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
fRet = SS_GetBorder(lpSS, Col, Row, lpwBorderType, lpwBorderStyle, lpColor);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetBorder(lpSS, Col, Row, lpwBorderType, lpwBorderStyle, lpColor)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
LPWORD        lpwBorderType;
LPWORD        lpwBorderStyle;
LPCOLORREF    lpColor;
{
LPSS_ROW      lpRow;
LPSS_COL      lpCol;
WORD          wType[4] = {SS_BORDERTYPE_LEFT, SS_BORDERTYPE_TOP, SS_BORDERTYPE_RIGHT, SS_BORDERTYPE_BOTTOM};
WORD          wStyle[4];
COLORREF      Color[4];
short         i;
BOOL          fRet;

lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);

fRet = SSx_GetBorder(lpSS, lpCol, lpRow, NULL, Col, Row,
                     &(wStyle[0]), &(Color[0]), &(wStyle[1]), &(Color[1]),
                     &(wStyle[2]), &(Color[2]), &(wStyle[3]), &(Color[3]));

if (lpwBorderType)
   *lpwBorderType = 0;
if (lpwBorderStyle)
   *lpwBorderStyle = 0;

if (fRet)
	{
	for (i = 0; i < 4; i++)
		{
		if (wStyle[i])
			{
			if (lpwBorderType)
				*lpwBorderType |= wType[i];

			if (lpwBorderStyle)
				*lpwBorderStyle = wStyle[i];

			if (lpColor)
				*lpColor = Color[i];
			}
		}
	}

SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);

return (fRet);
}


BOOL DLLENTRY SSGetBorderEx(HWND hWnd, SS_COORD Col, SS_COORD Row,
                            LPWORD lpwStyleLeft, LPCOLORREF lpclrLeft,
                            LPWORD lpwStyleTop, LPCOLORREF lpclrTop,
                            LPWORD lpwStyleRight, LPCOLORREF lpclrRight,
                            LPWORD lpwStyleBottom, LPCOLORREF lpclrBottom)
{
LPSPREADSHEET lpSS;
BOOL          fRet;

lpSS = SS_SheetLock(hWnd);
SS_AdjustCellCoords(lpSS, &Col, &Row);
fRet = SS_GetBorderEx(lpSS, Col, Row, lpwStyleLeft, lpclrLeft, lpwStyleTop,
                      lpclrTop, lpwStyleRight, lpclrRight, lpwStyleBottom,
                      lpclrBottom);
SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_GetBorderEx(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                    LPWORD lpwStyleLeft, LPCOLORREF lpclrLeft,
                    LPWORD lpwStyleTop, LPCOLORREF lpclrTop,
                    LPWORD lpwStyleRight, LPCOLORREF lpclrRight,
                    LPWORD lpwStyleBottom, LPCOLORREF lpclrBottom)
{
LPSS_ROW lpRow;
LPSS_COL lpCol;
BOOL     fRet;

lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);

fRet = SSx_GetBorder(lpSS, lpCol, lpRow, NULL, Col, Row, lpwStyleLeft,
                     lpclrLeft, lpwStyleTop, lpclrTop, lpwStyleRight,
                     lpclrRight, lpwStyleBottom, lpclrBottom);

SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);

return (fRet);
}

BOOL SSx_GetBorder(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow,
                   LPSS_CELL lpCell, SS_COORD Col, SS_COORD Row,
                   LPWORD lpwStyleLeft, LPCOLORREF lpclrLeft,
                   LPWORD lpwStyleTop, LPCOLORREF lpclrTop,
                   LPWORD lpwStyleRight, LPCOLORREF lpclrRight,
                   LPWORD lpwStyleBottom, LPCOLORREF lpclrBottom)
{
SS_COLORTBLITEM ColorTblItem;
LPSS_CELL       lpCellOrig = lpCell;
SS_CELLBORDER   CellBorder;
BOOL            fRet = FALSE;

_fmemset(&CellBorder, '\0', sizeof(SS_CELLBORDER));

fRet |= SS_BorderApply(&CellBorder, lpSS->hBorder);

if (Col != SS_ALLCOLS && lpCol)
   fRet |= SS_BorderApply(&CellBorder, lpCol->hBorder);

if (Row != SS_ALLROWS && lpRow)
   fRet |= SS_BorderApply(&CellBorder, lpRow->hBorder);

if (Row != SS_ALLROWS && Col != SS_ALLCOLS &&
    (lpCell || (lpCell = SS_LockCellItem(lpSS, Col, Row))))
   {
   fRet |= SS_BorderApply(&CellBorder, lpCell->hBorder);

   if (lpCell && !lpCellOrig)
      SS_UnlockCellItem(lpSS, Col, Row);
   }

{
LPWORD     lpwStyle[4] = {lpwStyleLeft, lpwStyleTop, lpwStyleRight, lpwStyleBottom};
LPCOLORREF lpColor[4] = {lpclrLeft, lpclrTop, lpclrRight, lpclrBottom};
short      i;

for (i = 0; i < 4; i++)
   {
   SS_GetColorItem(&ColorTblItem, CellBorder.Border[i].idColor);

   if (lpwStyle[i])
		*(lpwStyle[i]) = fRet ? CellBorder.Border[i].bStyle : SS_BORDERSTYLE_NONE;

   if (lpColor[i])
      *(lpColor[i]) = fRet ? ColorTblItem.Color : RGBCOLOR_DEFAULT;
   }
}

return (fRet);
}


BOOL SS_BorderApply(LPSS_CELLBORDER lpCellBorder, TBGLOBALHANDLE hBorder)
{
BOOL fRet = FALSE;

if (hBorder)
   {
   LPSS_CELLBORDER lpCellBorderSrc = (LPSS_CELLBORDER)tbGlobalLock(hBorder);
   short           i;

   for (i = 0; i < 4; i++)
      {
      if (lpCellBorderSrc->Border[i].bStyle)
         lpCellBorder->Border[i].bStyle = lpCellBorderSrc->Border[i].bStyle;

      if (lpCellBorderSrc->Border[i].idColor)
         lpCellBorder->Border[i].idColor = lpCellBorderSrc->Border[i].idColor;
      }

   tbGlobalUnlock(hBorder);
	fRet = TRUE;
   }

return (fRet);
}
/*
BOOL SSx_GetBorder(LPSPREADSHEET lpSS, LPSS_COL lpCol, LPSS_ROW lpRow,
                   LPSS_CELL lpCell, SS_COORD Col, SS_COORD Row,
                   LPWORD lpwStyleLeft, LPCOLORREF lpclrLeft,
                   LPWORD lpwStyleTop, LPCOLORREF lpclrTop,
                   LPWORD lpwStyleRight, LPCOLORREF lpclrRight,
                   LPWORD lpwStyleBottom, LPCOLORREF lpclrBottom)
{
SS_COLORTBLITEM ColorTblItem;
LPSS_CELL       lpCellOrig = lpCell;
SS_CELLBORDER   CellBorder;
BOOL            fRet = TRUE;

_fmemset(&CellBorder, '\0', sizeof(SS_CELLBORDER));

SS_BorderApply(&CellBorder, lpSS->hBorder);

if (Col != SS_ALLCOLS && lpCol)
   SS_BorderApply(&CellBorder, lpCol->hBorder);

if (Row != SS_ALLROWS && lpRow)
   SS_BorderApply(&CellBorder, lpRow->hBorder);

if (Row != SS_ALLROWS && Col != SS_ALLCOLS &&
    (lpCell || (lpCell = SS_LockCellItem(lpSS, Col, Row))))
   {
   SS_BorderApply(&CellBorder, lpCell->hBorder);

   if (lpCell && !lpCellOrig)
      SS_UnlockCellItem(lpSS, Col, Row);
   }

if (fRet)
   {
   LPWORD     lpwStyle[4] = {lpwStyleLeft, lpwStyleTop, lpwStyleRight, lpwStyleBottom};
   LPCOLORREF lpColor[4] = {lpclrLeft, lpclrTop, lpclrRight, lpclrBottom};
   short      i;

   for (i = 0; i < 4; i++)
      {
      SS_GetColorItem(lpSS, &ColorTblItem, CellBorder.Border[i].idColor);

      if (lpwStyle[i])
         *(lpwStyle[i]) = CellBorder.Border[i].bStyle;

      if (lpColor[i])
         *(lpColor[i]) = ColorTblItem.Color;
      }
   }

return (fRet);
}


BOOL SS_BorderApply(LPSS_CELLBORDER lpCellBorder, TBGLOBALHANDLE hBorder)
{


if (hBorder)
   {
   LPSS_CELLBORDER lpCellBorderSrc = (LPSS_CELLBORDER)tbGlobalLock(hBorder);
   short           i;

   for (i = 0; i < 4; i++)
      {
      if (lpCellBorderSrc->Border[i].bStyle)
         lpCellBorder->Border[i].bStyle = lpCellBorderSrc->Border[i].bStyle;

      if (lpCellBorderSrc->Border[i].idColor)
         lpCellBorder->Border[i].idColor = lpCellBorderSrc->Border[i].idColor;
      }

   tbGlobalUnlock(hBorder);

   }

return (TRUE);
}
*/
BOOL DLLENTRY SSSetBorder(hWnd, Col, Row, wBorderType, wBorderStyle, Color)

HWND     hWnd;
SS_COORD Col;
SS_COORD Row;
WORD     wBorderType;
WORD     wBorderStyle;
COLORREF Color;
{
return (SSSetBorderRange(hWnd, Col, Row, Col, Row, wBorderType, wBorderStyle,
                         Color));
}


BOOL DLLENTRY SSSetBorderRange(hWnd, Col, Row, Col2, Row2, wBorderType,
                               wBorderStyle, Color)

HWND          hWnd;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
WORD          wBorderType;
WORD          wBorderStyle;
COLORREF      Color;
{
LPSPREADSHEET lpSS;
BOOL       fRet;

lpSS = SS_SheetLock(hWnd);

if (Col != SS_ALLCOLS)
   SS_AdjustCellCoords(lpSS, &Col, NULL);

if (Row != SS_ALLROWS)
   SS_AdjustCellCoords(lpSS, NULL, &Row);

if (Col2 != SS_ALLCOLS)
   SS_AdjustCellCoords(lpSS, &Col2, NULL);

if (Row2 != SS_ALLROWS)
   SS_AdjustCellCoords(lpSS, NULL, &Row2);

fRet = SS_SetBorderRange(lpSS, Col, Row, Col2, Row2, wBorderType,
                         wBorderStyle, Color);

SS_SheetUnlock(hWnd);
return (fRet);
}


BOOL SS_SetBorderRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                       SS_COORD Col2, SS_COORD Row2, WORD wBorderType, WORD wBorderStyle,
                       COLORREF Color)
{
SS_COORD      x;
SS_COORD      y;
BOOL          RedrawOld;
BOOL          fRet = TRUE;

SS_LimitRangeCoords(lpSS, &Col, &Row, &Col2, &Row2);

lpSS->fSetBorder = TRUE;

RedrawOld = lpSS->lpBook->Redraw;
lpSS->lpBook->Redraw = FALSE;

if (wBorderType & SS_BORDERTYPE_OUTLINE)
   {
   if (wBorderType == SS_BORDERTYPE_OUTLINE)
      wBorderType |= SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM |
                     SS_BORDERTYPE_LEFT | SS_BORDERTYPE_RIGHT;

   if ((wBorderType & SS_BORDERTYPE_TOP) ||
       (wBorderType & SS_BORDERTYPE_BOTTOM))
      for (x = Col; x <= Col2; x++)
         {
         if (wBorderType & SS_BORDERTYPE_TOP)
            fRet = SS_SetBorder(lpSS, x, Row, SS_BORDERTYPE_TOP,
                                wBorderStyle, Color);
         if (wBorderType & SS_BORDERTYPE_BOTTOM)
            fRet = SS_SetBorder(lpSS, x, Row2, SS_BORDERTYPE_BOTTOM,
                                wBorderStyle, Color);
         }

   if ((wBorderType & SS_BORDERTYPE_LEFT) ||
       (wBorderType & SS_BORDERTYPE_RIGHT))
      for (y = Row; y <= Row2; y++)
         {
         if (wBorderType & SS_BORDERTYPE_LEFT)
            fRet = SS_SetBorder(lpSS, Col, y, SS_BORDERTYPE_LEFT,
                                wBorderStyle, Color);
         if (wBorderType & SS_BORDERTYPE_RIGHT)
            fRet = SS_SetBorder(lpSS, Col2, y, SS_BORDERTYPE_RIGHT,
                                wBorderStyle, Color);
         }
   }

else
   for (y = Row; y <= Row2 && fRet; y++)
      for (x = Col; x <= Col2 && fRet; x++)
         fRet = SS_SetBorder(lpSS, x, y, wBorderType, wBorderStyle, Color);

SS_BookSetRedraw(lpSS->lpBook, RedrawOld);

return (fRet);
}


BOOL SS_SetBorder(lpSS, Col, Row, wBorderType, wBorderStyle, Color)

LPSPREADSHEET lpSS;
SS_COORD      Col;
SS_COORD      Row;
WORD          wBorderType;
WORD          wBorderStyle;
COLORREF      Color;
{
LPSS_CELL     lpCell;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
SS_COORD      i;
SS_COORD      j;
BOOL          fRet = FALSE;

if (Col != SS_ALLCOLS && Row != SS_ALLROWS)
   {
   if (lpCell = SS_AllocLockCell(lpSS, NULL, Col, Row))
      {
      SS_SetBorderItem(lpSS, &lpCell->hBorder, wBorderType,
                       wBorderStyle, Color);

      SS_UnlockCellItem(lpSS, Col, Row);
      SS_InvalidateCell(lpSS, Col, Row);
      fRet = TRUE;
      }
   }

else if (Col != SS_ALLCOLS && Row == SS_ALLROWS)
   {
   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
      if (lpCell = SS_LockCellItem(lpSS, Col, i))
         {
         SS_SetBorderItem(lpSS, &lpCell->hBorder, wBorderType,
                          SS_BORDERSTYLE_NONE, RGBCOLOR_DEFAULT);
         SS_UnlockCellItem(lpSS, Col, i);
         }

   if (lpCol = SS_AllocLockCol(lpSS, Col))
      {
      SS_SetBorderItem(lpSS, &lpCol->hBorder, wBorderType,
                       wBorderStyle, Color);

      SS_UnlockColItem(lpSS, Col);
      SS_InvalidateCol(lpSS, Col);
      fRet = TRUE;
      }
   }

else if (Col == SS_ALLCOLS && Row != SS_ALLROWS)
   {
   for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
      if (lpCell = SS_LockCellItem(lpSS, i, Row))
         {
         SS_SetBorderItem(lpSS, &lpCell->hBorder, wBorderType,
                          SS_BORDERSTYLE_NONE, RGBCOLOR_DEFAULT);
         SS_UnlockCellItem(lpSS, i, Row);
         }

   if (lpRow = SS_AllocLockRow(lpSS, Row))
      {
      SS_SetBorderItem(lpSS, &lpRow->hBorder, wBorderType,
                       wBorderStyle, Color);

      SS_UnlockRowItem(lpSS, Row);
      SS_InvalidateRow(lpSS, Row);
      fRet = TRUE;
      }
   }
else
   {
   /**********************************
   * Clear All Cols, Rows, and Cells
   **********************************/

   for (i = lpSS->Row.HeaderCnt; i < lpSS->Row.AllocCnt; i++)
      {
      if (lpRow = SS_LockRowItem(lpSS, i))
         {
         SS_SetBorderItem(lpSS, &lpRow->hBorder, wBorderType,
                          SS_BORDERSTYLE_NONE, RGBCOLOR_DEFAULT);

         for (j = lpSS->Col.HeaderCnt; j < lpSS->Col.AllocCnt; j++)
            if (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i))
               {
               SS_SetBorderItem(lpSS, &lpCell->hBorder, wBorderType,
                                SS_BORDERSTYLE_NONE, RGBCOLOR_DEFAULT);
               SS_UnlockCellItem(lpSS, j, i);
               }
         }
      }

   for (i = lpSS->Col.HeaderCnt; i < lpSS->Col.AllocCnt; i++)
      if (lpCol = SS_LockColItem(lpSS, i))
         {
         SS_SetBorderItem(lpSS, &lpCol->hBorder, wBorderType,
                          SS_BORDERSTYLE_NONE, RGBCOLOR_DEFAULT);
         SS_UnlockColItem(lpSS, i);
         }

   SS_SetBorderItem(lpSS, &lpSS->hBorder, wBorderType,
                    wBorderStyle, Color);
   SS_InvalidateScrollArea(lpSS, SS_SCROLLAREA_ALL);
   fRet = TRUE;
   }

return (fRet);
}


BOOL SS_SetBorderItem(LPSPREADSHEET lpSS, LPTBGLOBALHANDLE lphBorder,
                      WORD wBorderType, WORD wBorderStyle, COLORREF Color)
{
if (!wBorderType && *lphBorder)
   SS_FreeBorder(lpSS, lphBorder);

else
   {
   if (!(*lphBorder) && wBorderStyle != SS_BORDERSTYLE_NONE)
      *lphBorder = tbGlobalAlloc(GHND, sizeof(SS_CELLBORDER));

   if (*lphBorder)
      {
      LPSS_CELLBORDER lpCellBorder = (LPSS_CELLBORDER)tbGlobalLock(*lphBorder);
      SS_COLORID      idColor = SS_AddColor(Color);

      if (wBorderType & SS_BORDERTYPE_LEFT)
         {
         lpCellBorder->Border[SS_BORDERINDEX_LEFT].bStyle = (BYTE)wBorderStyle;
         lpCellBorder->Border[SS_BORDERINDEX_LEFT].idColor = idColor;
         }

      if (wBorderType & SS_BORDERTYPE_TOP)
         {
         lpCellBorder->Border[SS_BORDERINDEX_TOP].bStyle = (BYTE)wBorderStyle;
         lpCellBorder->Border[SS_BORDERINDEX_TOP].idColor = idColor;
         }

      if (wBorderType & SS_BORDERTYPE_RIGHT)
         {
         lpCellBorder->Border[SS_BORDERINDEX_RIGHT].bStyle = (BYTE)wBorderStyle;
         lpCellBorder->Border[SS_BORDERINDEX_RIGHT].idColor = idColor;
         }

      if (wBorderType & SS_BORDERTYPE_BOTTOM)
         {
         lpCellBorder->Border[SS_BORDERINDEX_BOTTOM].bStyle = (BYTE)wBorderStyle;
         lpCellBorder->Border[SS_BORDERINDEX_BOTTOM].idColor = idColor;
         }

      if (wBorderStyle == SS_BORDERSTYLE_NONE &&
          !lpCellBorder->Border[SS_BORDERINDEX_LEFT].bStyle &&
          !lpCellBorder->Border[SS_BORDERINDEX_TOP].bStyle &&
          !lpCellBorder->Border[SS_BORDERINDEX_RIGHT].bStyle &&
          !lpCellBorder->Border[SS_BORDERINDEX_BOTTOM].bStyle)
         {
         tbGlobalUnlock(*lphBorder);
         SS_FreeBorder(lpSS, lphBorder);
         }
      else
         tbGlobalUnlock(*lphBorder);
      }
   }

return (TRUE);
}


void SS_FreeBorder(LPSPREADSHEET lpSS, LPTBGLOBALHANDLE lphBorder)
{
if (*lphBorder)
   {
   tbGlobalFree(*lphBorder);
   *lphBorder = 0;
   }
}

#endif
