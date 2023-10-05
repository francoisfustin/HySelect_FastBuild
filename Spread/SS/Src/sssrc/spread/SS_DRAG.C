/*********************************************************
* SS_DRAG.C
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
#include <string.h>
#include <process.h>
#include <math.h>
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_drag.h"
#include "ss_draw.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_span.h"
#include "ss_type.h"
#include "ss_win.h"


#ifndef SS_NODRAGDROP
void SS_DragDropDrawBox(HWND hWnd, LPSPREADSHEET lpSS, BOOL fBoxOn)
{
SS_CELLCOORD  BlockCellUL;
SS_CELLCOORD  BlockCellLR;
RECT          Rect;
RECT          Rect2;
HRGN          hRgn;
HRGN          hRgn2;
HDC           hDC;

if (lpSS->lpBook->fDragDropBoxOn == (unsigned)fBoxOn)
   return;

if (SS_USESELBAR(lpSS))
    {
    BlockCellUL.Col = SS_ALLCOLS;
    BlockCellUL.Row = lpSS->Row.CurAt;
    BlockCellLR.Col = SS_ALLCOLS;
    BlockCellLR.Row = lpSS->Row.CurAt;
    }
else if (!SS_IsBlockSelected(lpSS))
    {
    BlockCellUL.Col = lpSS->Col.CurAt;
    BlockCellUL.Row = lpSS->Row.CurAt;
    BlockCellLR.Col = lpSS->Col.CurAt;
    BlockCellLR.Row = lpSS->Row.CurAt;
    }
else
    {
    _fmemcpy(&BlockCellUL, &lpSS->BlockCellUL, sizeof(SS_CELLCOORD));
    _fmemcpy(&BlockCellLR, &lpSS->BlockCellLR, sizeof(SS_CELLCOORD));
    }

if (SS_GetDragDropRect(lpSS, &Rect, &Rect2,
                       lpSS->DragDropBlockUL.Col,
                       lpSS->DragDropBlockUL.Row,
                       BlockCellLR.Col == -1 ? -1 :
                       lpSS->DragDropBlockUL.Col +
                       (BlockCellLR.Col - BlockCellUL.Col),
                       BlockCellLR.Row == -1 ? -1 :
                       lpSS->DragDropBlockUL.Row +
                       (BlockCellLR.Row -
                       BlockCellUL.Row), 0, 3, 0))
   {
   hRgn = CreateRectRgnIndirect(&Rect);
   hRgn2 = CreateRectRgnIndirect(&Rect2);
   CombineRgn(hRgn, hRgn, hRgn2, RGN_XOR);
   DeleteObject(hRgn2);

   hDC = SS_GetDC(lpSS->lpBook);

   SelectClipRgn(hDC, hRgn);
   InvertRgn(hDC, hRgn);
   SelectClipRgn(hDC, 0);

   SS_ReleaseDC(lpSS->lpBook, hDC);

   DeleteObject(hRgn);
   }

lpSS->lpBook->fDragDropBoxOn = fBoxOn;
}


void SS_DragDropFinish(HWND hWnd, LPSPREADSHEET lpSS)
{
SS_CELLCOORD  BlockCellUL;
SS_CELLCOORD  BlockCellLR;
LPSS_CELLTYPE lpCellType;
SS_CELLTYPE   CellType;
GLOBALHANDLE  hData;
SS_DRAGDROP   DragDrop;
LPTSTR        lpData;
LPSS_COL      lpCol;
LPSS_ROW      lpRow;
SS_DATA       Data;
SS_COORD      dWidth;
SS_COORD      dHeight;
SS_COORD      Col;
SS_COORD      Row;
SS_COORD      Col2;
SS_COORD      Row2;
SS_COORD      ColTemp;
SS_COORD      RowTemp;
SS_COORD      ColDest;
SS_COORD      RowDest;
SS_COORD      i;
SS_COORD      j;
HDC           hDC;
BOOL          fBlockSelected = FALSE;
BOOL          fRedraw;
BOOL          fData;
BOOL          fRet;
#ifdef SS_V40
long          NumCols;
long          NumRows;
#endif // SS_V40

SS_DragDropDrawBox(hWnd, lpSS, FALSE);

if (SS_USESELBAR(lpSS))
    {
    BlockCellUL.Col = SS_ALLCOLS;
    BlockCellUL.Row = lpSS->Row.CurAt;
    BlockCellLR.Col = SS_ALLCOLS;
    BlockCellLR.Row = lpSS->Row.CurAt;
    }
else if (!SS_IsBlockSelected(lpSS))
    {
    BlockCellUL.Col = lpSS->Col.CurAt;
    BlockCellUL.Row = lpSS->Row.CurAt;
    BlockCellLR.Col = lpSS->Col.CurAt;
    BlockCellLR.Row = lpSS->Row.CurAt;
    }
else
    {
    fBlockSelected = TRUE;
    _fmemcpy(&BlockCellUL, &lpSS->BlockCellUL, sizeof(SS_CELLCOORD));
    _fmemcpy(&BlockCellLR, &lpSS->BlockCellLR, sizeof(SS_CELLCOORD));
    }

if (BlockCellUL.Col == lpSS->DragDropBlockUL.Col &&
    BlockCellUL.Row == lpSS->DragDropBlockUL.Row)
   return;

dWidth = BlockCellLR.Col - BlockCellUL.Col;
dHeight = BlockCellLR.Row - BlockCellUL.Row;

// If this is the top left cell of a span then do not allow
// it to overlap another span.
#ifdef SS_V40
if (SS_SPAN_ANCHOR == SS_GetCellSpan(lpSS, BlockCellUL.Col, BlockCellUL.Row, NULL, NULL, &NumCols, &NumRows) &&
    SS_IsSpanOverlap(lpSS, lpSS->DragDropBlockUL.Col, lpSS->DragDropBlockUL.Row, NumCols, NumRows) == SS_SPANOVERLAP_YES)
	return;
#endif // SS_V40

_fmemset(&DragDrop, '\0', sizeof(SS_DRAGDROP));

if (HIBYTE(GetKeyState(VK_CONTROL)))
   DragDrop.fCopy = TRUE;

DragDrop.BlockPrev.UL.Col = BlockCellUL.Col;
DragDrop.BlockPrev.UL.Row = BlockCellUL.Row;
DragDrop.BlockPrev.LR.Col = BlockCellLR.Col;
DragDrop.BlockPrev.LR.Row = BlockCellLR.Row;

DragDrop.BlockNew.UL.Col = lpSS->DragDropBlockUL.Col;
DragDrop.BlockNew.UL.Row = lpSS->DragDropBlockUL.Row;
DragDrop.BlockNew.LR.Col = BlockCellLR.Col == -1 ? -1 :
                           lpSS->DragDropBlockUL.Col + dWidth;
DragDrop.BlockNew.LR.Row = BlockCellLR.Row == -1 ? -1 :
                           lpSS->DragDropBlockUL.Row + dHeight;

/*****************************************
* Determine if data is being overwritten
*****************************************/

if (lpSS->DragDropBlockUL.Row == 0)
   RowTemp = lpSS->Row.DataCnt;
else
   RowTemp = lpSS->DragDropBlockUL.Row + dHeight;

if (lpSS->DragDropBlockUL.Col == 0)
   ColTemp = lpSS->Col.DataCnt;
else
   ColTemp = lpSS->DragDropBlockUL.Col + dWidth;

for (fData = FALSE, i = lpSS->DragDropBlockUL.Row; !fData && i <= RowTemp;
     i++)
   if (lpRow = SS_LockRowItem(lpSS, i))
      {
      for (j = lpSS->DragDropBlockUL.Col; !fData && j <= ColTemp; j++)
         {
         lpCol = SS_LockColItem(lpSS, j);
         fData = SS_GetDataStruct(lpSS, lpCol, lpRow, NULL, &Data, j, i);

			// RFW - 6/11/04 - 14380
			if (!fData)
				{
				lpCellType = SSx_RetrieveCellType(lpSS, &CellType, lpCol, lpRow, NULL,
                                              j, i);

				if (lpCellType)
					if (lpCellType->Type == SS_TYPE_COMBOBOX ||
						 lpCellType->Type == SS_TYPE_PICTURE ||
						 lpCellType->Type == SS_TYPE_BUTTON ||
						 lpCellType->Type == SS_TYPE_CHECKBOX ||
						 lpCellType->Type == SS_TYPE_OWNERDRAW)
						fData = TRUE;
				}

         if (lpCol)
            SS_UnlockColItem(lpSS, j);
         }

      SS_UnlockRowItem(lpSS, i);
      }

if (fData)
   DragDrop.fDataBeingOverwritten = TRUE;

SS_AdjustCellCoordsOut(lpSS, &DragDrop.BlockPrev.UL.Col, &DragDrop.BlockPrev.UL.Row);
SS_AdjustCellCoordsOut(lpSS, &DragDrop.BlockPrev.LR.Col, &DragDrop.BlockPrev.LR.Row);
SS_AdjustCellCoordsOut(lpSS, &DragDrop.BlockNew.UL.Col, &DragDrop.BlockNew.UL.Row);
SS_AdjustCellCoordsOut(lpSS, &DragDrop.BlockNew.LR.Col, &DragDrop.BlockNew.LR.Row);

/*****************************************
* Notify parent that drop is about occur
*****************************************/

SS_SendMsg(lpSS->lpBook, lpSS, SSM_DRAGDROP, GetWindowID(hWnd), (LPARAM)(LPSS_DRAGDROP)&DragDrop);

Col = BlockCellUL.Col;
Row = BlockCellUL.Row;
Col2 = BlockCellLR.Col;
Row2 = BlockCellLR.Row;
ColDest = lpSS->DragDropBlockUL.Col;
RowDest = lpSS->DragDropBlockUL.Row;

if (!DragDrop.fCancel)
   {
   SSx_ResetBlock(lpSS, TRUE);

   if (!DragDrop.fDataOnly)
      {
      fRedraw = lpSS->lpBook->Redraw;
      lpSS->lpBook->Redraw = FALSE;

      SS_CopySwapRange(lpSS, Col, Row, Col2, Row2,
                       lpSS->DragDropBlockUL.Col,
                       lpSS->DragDropBlockUL.Row, 
                       (WORD)(DragDrop.fCopy ? SS_CMD_COPY : SS_CMD_MOVE),
                       TRUE, TRUE);

      //SS_CopySwapRange(lpSS, Col, Row, Col2, Row2,
      //                 lpSS->DragDropBlockUL.Col,
      //                 lpSS->DragDropBlockUL.Row, SS_CMD_COPY, TRUE);
      //
      //if (!DragDrop.fCopy)
      //   {
      //   if (Row2 == SS_ALLROWS)
      //   {
      //      Row = SS_ALLROWS;
      //      RowDest = SS_ALLROWS;
      //   }
      //
      //   if (Col2 == SS_ALLCOLS)
      //   {
      //      Col = SS_ALLCOLS;
      //      ColDest = SS_ALLCOLS;
      //   }
      //
      //   for (i = Row; i <= Row2; i++)
      //      for (j = Col; j <= Col2; j++)
      //         if (i < RowDest ||
      //             i > RowDest + (Row2 - Row) ||
      //             j < ColDest ||
      //             j > ColDest + (Col2 - Col))
      //            SS_ClearRange(lpSS, j, i, j, i, TRUE);
      //   }

		SS_BookSetRedraw(lpSS->lpBook, fRedraw);
      }

   else
      {
      fRet = FALSE;

		/* RFW - 5/21/04 - 14295 
      if (hData = SSClipOut(hWnd, Col, Row, Col2, Row2))
		*/
      if (hData = SS_ClipOut(lpSS, Col, Row, Col2, Row2, FALSE, FALSE))
         {
         lpData = (LPTSTR)GlobalLock(hData);

         fRet = SS_ClipIn(lpSS, lpSS->DragDropBlockUL.Col,
                          lpSS->DragDropBlockUL.Row,
                          Col2 == -1 ? -1 : lpSS->DragDropBlockUL.Col +
                          dWidth,
                          Row2 == -1 ? -1 : lpSS->DragDropBlockUL.Row +
                          dHeight, lpData, lstrlen(lpData), FALSE, TRUE, FALSE,
                          NULL, NULL);

         GlobalUnlock(hData);
         }

      fRedraw = lpSS->lpBook->Redraw;
      lpSS->lpBook->Redraw = FALSE;

      if (fRet && !DragDrop.fCopy)
         {
         if (Row2 == SS_ALLROWS)
            {
            Row = SS_ALLROWS;
            RowDest = SS_ALLROWS;
            }

         if (Col2 == SS_ALLCOLS)
            {
            Col = SS_ALLCOLS;
            ColDest = SS_ALLCOLS;
            }

         for (i = Row; i <= Row2; i++)
            for (j = Col; j <= Col2; j++)
               if (i < RowDest ||
                   i > RowDest + (Row2 - Row) ||
                   j < ColDest ||
                   j > ColDest + (Col2 - Col))
                  SS_ClearDataRange(lpSS, j, i, j, i, TRUE, TRUE);
         }
         
		SS_BookSetRedraw(lpSS->lpBook, fRedraw);
      }

   SS_HighlightCell(lpSS, FALSE);

   if (Col2 != -1)
      lpSS->Col.CurAt = lpSS->DragDropBlockUL.Col;

   if (Row2 != -1)
      lpSS->Row.CurAt = lpSS->DragDropBlockUL.Row;

   if (fBlockSelected)
      {
      lpSS->BlockCellUL.Col = lpSS->DragDropBlockUL.Col;
      lpSS->BlockCellUL.Row = lpSS->DragDropBlockUL.Row;
      lpSS->BlockCellLR.Col = Col2 == -1 ? -1 :
                                  lpSS->DragDropBlockUL.Col + dWidth;
      lpSS->BlockCellLR.Row = Row2 == -1 ? -1 :
                                  lpSS->DragDropBlockUL.Row + dHeight;
      }
   // Fix for bug report SCS2209
   else
      {
      lpSS->BlockCellUL.Col = lpSS->BlockCellLR.Col = lpSS->Col.CurAt;
      lpSS->BlockCellUL.Row = lpSS->BlockCellLR.Row = lpSS->Row.CurAt;
      }

   SS_HighlightCell(lpSS, TRUE);

   hDC = SS_GetDC(lpSS->lpBook);
   SS_InvertBlock(hDC, lpSS);
   SS_ReleaseDC(lpSS->lpBook, hDC);
   }
}


BOOL SS_IsMouseInDragDrop(hWnd, lpSS, RectClient, MouseX, MouseY, lpCol,
                          lpRow)

HWND          hWnd;
LPSPREADSHEET lpSS;
LPRECT        RectClient;
int           MouseX;
int           MouseY;
LPSS_COORD    lpCol;
LPSS_COORD    lpRow;
{
SS_CELLCOORD  BlockCellUL;
SS_CELLCOORD  BlockCellLR;
SS_COORD      Col, ColRet;
SS_COORD      Row, RowRet;
int           x;
int           y;
BOOL          fRet = FALSE;

if (lpSS->lpBook->fAllowDragDrop && !lpSS->fMultipleBlocksSelected && !lpSS->lpBook->EditModeOn)
   {
	SS_GetCellFromPixel(lpSS, &Col, &Row, NULL, NULL, MouseX, MouseY);

   if (SS_USESELBAR(lpSS))
       {
       BlockCellUL.Col = SS_ALLCOLS;
       BlockCellUL.Row = lpSS->Row.CurAt;
       BlockCellLR.Col = SS_ALLCOLS;
       BlockCellLR.Row = lpSS->Row.CurAt;
       }
   else if (!SS_IsBlockSelected(lpSS))
       {
       BlockCellUL.Col = lpSS->Col.CurAt;
       BlockCellUL.Row = lpSS->Row.CurAt;
       BlockCellLR.Col = lpSS->Col.CurAt;
       BlockCellLR.Row = lpSS->Row.CurAt;
       }
   else
       {
       _fmemcpy(&BlockCellUL, &lpSS->BlockCellUL, sizeof(SS_CELLCOORD));
       _fmemcpy(&BlockCellLR, &lpSS->BlockCellLR, sizeof(SS_CELLCOORD));
       }

   if (BlockCellLR.Row != -1 &&
       SS_IsRowVisible(lpSS, BlockCellUL.Row, SS_VISIBLE_PARTIAL))
      {
      y = SS_GetCellPosY(lpSS, lpSS->Row.UL,
                         BlockCellUL.Row);

      if (MouseY >= y && MouseY <= y + 3 && (BlockCellLR.Col == -1 ||
          (Col >= BlockCellUL.Col && Col <= BlockCellLR.Col)))
         {
         RowRet = BlockCellUL.Row;
         ColRet = Col;

         fRet = TRUE;
         }
      }

   if (!fRet && BlockCellLR.Col != -1 &&
       SS_IsColVisible(lpSS, BlockCellUL.Col, SS_VISIBLE_PARTIAL))
      {
      x = SS_GetCellPosX(lpSS, lpSS->Col.UL, BlockCellUL.Col);

      if (MouseX >= x && MouseX <= x + 3 && (BlockCellLR.Row == -1 ||
          (Row >= BlockCellUL.Row &&
           Row <= BlockCellLR.Row)))
         {
         ColRet = BlockCellUL.Col;
         RowRet = Row;

         fRet = TRUE;
         }
      }

   if (!fRet && BlockCellLR.Row != -1 &&
       SS_IsRowVisible(lpSS, BlockCellLR.Row, SS_VISIBLE_ALL))
      {
      y = SS_GetCellPosY(lpSS, lpSS->Row.UL,
                         BlockCellLR.Row) +
                         SS_GetRowHeightInPixels(lpSS, BlockCellLR.Row);

      if (MouseY <= y && MouseY >= y - 3 && (BlockCellLR.Col == -1 ||
          (Col >= BlockCellUL.Col &&
           Col <= BlockCellLR.Col)))
         {
         RowRet = BlockCellLR.Row;
         ColRet = Col;

         fRet = TRUE;
         }
      }

   if (!fRet && BlockCellLR.Col != -1 &&
       SS_IsColVisible(lpSS, BlockCellLR.Col, SS_VISIBLE_ALL))
      {
      x = SS_GetCellPosX(lpSS, lpSS->Col.UL,
                         BlockCellLR.Col) +
                         SS_GetColWidthInPixels(lpSS, BlockCellLR.Col);

      if (MouseX <= x && MouseX >= x - 3 && (BlockCellLR.Row == -1 ||
          (Row >= BlockCellUL.Row &&
           Row <= BlockCellLR.Row)))
         {
         ColRet = BlockCellLR.Col;
         RowRet = Row;

         fRet = TRUE;
         }
      }
   }

// Do not allow drag/drop to be performed on a header cell.
if (fRet)
	{
	if (lpCol && ColRet < lpSS->Col.HeaderCnt)
		fRet = FALSE;
	else if (lpRow && RowRet < lpSS->Row.HeaderCnt)
		fRet = FALSE;
	// RFW - 8/27/03 - Do not allow dragging locked cells.
	/* RFW - 8/24/05 - I commented this out because of a complaint from a user
	else if (SS_GetLock(lpSS, ColRet, RowRet, TRUE))
		fRet = FALSE;
	else
		{
		SS_COORD Col1 = BlockCellUL.Col == -1 ? 0 : BlockCellUL.Col;
		SS_COORD Col2 = BlockCellLR.Col == -1 ? lpSS->Col.AllocCnt - 1 : BlockCellLR.Col;
		SS_COORD Row1 = BlockCellUL.Row == -1 ? 0 : BlockCellUL.Row;
		SS_COORD Row2 = BlockCellLR.Row == -1 ? lpSS->Row.AllocCnt - 1 : BlockCellLR.Row;
		SS_COORD c, r;

		for (r = Row1; r <= Row2 && fRet; r++)
			for (c = Col1; c <= Col2 && fRet; c++)
				fRet = !SS_GetLock(lpSS, c, r, TRUE);
		}
	*/

   if (lpRow)
      *lpRow = RowRet;

   if (lpCol)
      *lpCol = ColRet;
	}

return (fRet);
}


void SS_AddDragDropRectToBlock(LPSPREADSHEET lpSS, HRGN hRgn)
{
HRGN hRgn2;

if (SS_INVERTSEL(lpSS))
   if (hRgn2 = SS_CreateDragDropRect(lpSS))
      {
      CombineRgn(hRgn, hRgn, hRgn2, RGN_DIFF);
      DeleteObject(hRgn2);
      }
}


HRGN SS_CreateDragDropRect(LPSPREADSHEET lpSS)
{
RECT Rect;
RECT Rect2;
HRGN hRgn2 = 0;
HRGN hRgn3;

if (SS_GetDragDropRect(lpSS, &Rect, &Rect2,
                       lpSS->BlockCellUL.Col,
                       lpSS->BlockCellUL.Row,
                       lpSS->BlockCellLR.Col,
                       lpSS->BlockCellLR.Row, 3, 1, 1))
   {
   hRgn2 = CreateRectRgnIndirect(&Rect);
   hRgn3 = CreateRectRgnIndirect(&Rect2);
   CombineRgn(hRgn2, hRgn2, hRgn3, RGN_XOR);
   DeleteObject(hRgn3);

   if (SS_CreateBlockRect(lpSS, &Rect, lpSS->Col.CurAt,
                          lpSS->Row.CurAt, lpSS->Col.CurAt,
                          lpSS->Row.CurAt, TRUE))
      {
      hRgn3 = CreateRectRgnIndirect(&Rect);
      CombineRgn(hRgn2, hRgn2, hRgn3, RGN_DIFF);
      DeleteObject(hRgn3);
      }
   }

return (hRgn2);
}


BOOL SS_GetDragDropRect(LPSPREADSHEET lpSS, LPRECT lpRect,
                        LPRECT lpRect2, SS_COORD Col, SS_COORD Row,
                        SS_COORD Col2, SS_COORD Row2, short dOffset,
                        short dOffset2, short dOffset3)
{
BOOL fRet = FALSE;

if (SS_CreateBlockRect(lpSS, lpRect, Col, Row, Col2, Row2, FALSE))
   {
   CopyRect(lpRect2, lpRect);

   if (Col2 == -1)
      {
      lpRect->left = SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt);
      lpRect2->left = lpRect->left;
      }

   else if (Col == lpSS->Col.UL)
      {
      lpRect->left += dOffset3;
      lpRect2->left = lpRect->left + 1;
      }

// RFW - 5/6/04 - 14203
//   else if (Col > lpSS->Col.UL ||
   else if (Col > lpSS->Col.UL ||
            (Col > lpSS->Col.HeaderCnt && Col < lpSS->Col.HeaderCnt + lpSS->Col.Frozen))
      {
      lpRect->left += dOffset;
      lpRect2->left = lpRect->left + dOffset2;
      }

   if (Row2 == -1)
      {
      lpRect->top = SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt);
      lpRect2->top = lpRect->top;
      }

   else if (Row == lpSS->Row.UL)
      {
      lpRect->top += dOffset3;
      lpRect2->top = lpRect->top + 1;
      }

// RFW - 5/6/04 - 14203
//   else if (Row > lpSS->Row.UL)
   else if (Row > lpSS->Row.UL ||
            (Row > lpSS->Row.HeaderCnt && Row < lpSS->Row.HeaderCnt + lpSS->Row.Frozen))
      {
      lpRect->top += dOffset;
      lpRect2->top = lpRect->top + dOffset2;
      }

   if (Col2 != -1 && Col2 <= lpSS->Col.LRAllVis)
      {
      if (Col2 == SS_GetColCnt(lpSS) - 1)
         lpRect->right -= dOffset3;
      else
         lpRect->right = SS_GetCellPosX(lpSS, lpSS->Col.UL, Col2) +
                                        SS_GetColWidthInPixels(lpSS, Col2)
                                        + 2 - dOffset;

      lpRect2->right = lpRect->right - dOffset2;
      }

   if (Row2 != -1 && Row2 <= lpSS->Row.LRAllVis)
      {
      if (Row2 == SS_GetRowCnt(lpSS) - 1)
         lpRect->bottom -= dOffset3;
      else
         lpRect->bottom = SS_GetCellPosY(lpSS, lpSS->Row.UL, Row2) +
                                         SS_GetRowHeightInPixels(lpSS, Row2) + 2 -
                                         dOffset;

      lpRect2->bottom = lpRect->bottom - dOffset2;
      }

   fRet = TRUE;
   }

return (fRet);
}

#endif
