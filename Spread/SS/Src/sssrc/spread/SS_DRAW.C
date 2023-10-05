/*********************************************************
* SS_DRAW.C
*                
* Copyright (C) 1991-2002 - FarPoint Technologies                  
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*********************************************************/

#ifdef SS_V40
//#define SS_PAINTTOSCREEN // Uncomment this for testing to paint directly to screen
#else
#define SS_PAINTTOSCREEN
#endif

#if defined(SS_PAINTTOSCREEN) && defined(SS_V40)
#pragma message( "--------------------Warning - Turn off SS_PAINTTOSCREEN--------------------" )
#ifndef _DEBUG
#undef SS_PAINTTOSCREEN
#endif
#endif

#define OEMRESOURCE
#define DRV_NOPATTERN
#define DRV_NOGAUGE

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include <limits.h>
#include "..\..\..\..\fplibs\fptools\src\fptools.h"
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_bord.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
#include "ss_doc.h"
#include "ss_drag.h"
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_main.h"
#include "ss_multi.h"
#include "ss_row.h"
#include "ss_scrol.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_virt.h"
#include "ss_win.h"
#include "..\classes\checkbox.h"
#include "..\classes\superbtn.h"
#include "..\classes\viewpict.h"
#include "..\classes\wintools.h"
#include "..\edit\editfld.h"
#ifdef SS_V40
#include "ss_span.h"
#endif // SS_V40
#ifdef SS_V70
#include "ss_book.h"
#include "ss_tab.h"
#endif // SS_V70
#ifdef SS_V80
#include "uxtheme.h"
#include "ss_theme.h"
#include "ss_data.h"
#endif
#define SS_LINETYPE_SOLID       0
#define SS_LINETYPE_DOT         1
#define SS_LINETYPE_NONE        2

#define DEFAULT_CELLWIDTH       8
#define DEFAULT_CELLWIDTH_TWIPS 950

#define STANDARDFONTWIDTH       8
#define XPIXELSPERINCH          96
#define YPIXELSPERINCH          96

#define DIVUP(Var1, Var2)      (((Var1) + (Var2) - 1) / (Var2))

static DWORD TickCountLast = 0;
HBRUSH hBrushFocusRect;
extern HANDLE hDynamicInst;

SS_PAINTBITMAP PaintBitmap;

static BOOL SS_PaintCell(HDC hDC, HWND hWnd, LPSPREADSHEET lpSS, LPSS_COL lpCol,
                  LPSS_ROW lpRow, LPRECT Rect, SS_COORD CellCol,
                  SS_COORD CellRow, BOOL fErase, LPSSX_PRINTFORMAT lpPFormat,
                  LPRECT lpRectUpdate, LPRECT lpCellOverflowRectPrev,
                  LPRECT lpCellOverflowRectNext, BOOL fOverflowToLeft,
                  BOOL fCellSelected, BOOL fMergeScan, BOOL IsLastCol, BOOL IsLastRow);
BOOL           SSx_Paint(LPSPREADSHEET lpSS, HDC hDC, HDC hDCDev, LPRECT lpRectUpdate);
void           SS_DrawCellBorder(LPSPREADSHEET lpSS, HDC hDC, SS_COORD Col, SS_COORD Row, LPRECT lpRect, LPSSX_PRINTFORMAT lpPFormat, SS_COORD CellLeft, SS_COORD CellTop);
static short   SS_RetrieveVertTextJust(short Justification);
static void    SS_DrawShadows(HDC hDC, LPRECT lpRect, short iShadowSize,
                              HBRUSH hBrushShadow, HBRUSH hBrushHighlight,
                              BOOL fNoLeft);
static void    SS_ShrinkRect(LPRECT Rect, int x, int y);
void           SS_CheckBoxDrawSetPict(HANDLE hInst, LPCHECKBOXPICTS lpPicts,
                                      TBGLOBALHANDLE hPictName,
                                      short bPictType);
void           SS_DrawBorder(HWND hWnd, LPSPREADSHEET lpSS, HDC hDC,
                             LPSS_COL lpCol, LPSS_ROW lpRow, LPRECT lpRect,
                             SS_COORD Col, SS_COORD Row,
                           LPSSX_PRINTFORMAT lpPFormat);
void           SSx_DrawBorder(LPSPREADSHEET lpSS, HDC hDC, int left,
                              int top, int right, int bottom,
                              LPSSX_PRINTFORMAT lpPFormat, WORD wBorderStyle,
                              COLORREF Color);
void           SS_EraseBkgnd(HWND hWnd, LPSPREADSHEET lpSS, HDC hDC,
                             LPRECT lpRectClient, LPRECT lpRectUpdate,
                             SS_COORD RightVisCell, SS_COORD BottomVisCell);
#ifdef SS_UTP
void           SS_PaintArrow(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS,
                             LPRECT lpRect, HICON hIconUp, WORD wIconUpType,
                             HICON hIconDown, WORD wIconDownType,
                             BOOL fButtonDown);
#endif
void           SSx_DrawText(HWND hWnd, HDC hDC, LPSSX_PRINTFORMAT lpPFormat,
                            LPTSTR lpData, int iLen, LPRECT lpRect,
                            LPRECT lpRectOrig, UINT Just, LPSS_FONT lpFont, DWORD dwTextOrient);
void           SS_DrawFormatHeaderLetter(LPTSTR lpBuffer, long lHeaderNumber);
void           SS_DrawFormatHeaderNumber(LPTSTR lpBuffer, long lHeaderNumber);
void           SS_ScaleRect(LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect);
void           SS_GetEditModeShadowRects(LPSPREADSHEET lpSS, LPRECT lpRect);
#ifdef SPREAD_JPN
//- JPNFIX0015 - (Masanori Iwasa)
//void           SS_AdjustHiddenRowCol(LPSPREADSHEET lpSS, LPSS_ROWCOL Target, BOOL fRow);
//BOOL           SSx_AdjustHiddenRowCol(LPSPREADSHEET lpSS, SS_COORD nCell, BOOL fRow);
#endif
void           SS_DrawCellBlock(LPSPREADSHEET lpSS, SS_COORD Left, SS_COORD Top,
                                SS_COORD Right, SS_COORD Bottom);
BOOL           SS_FocusRect(LPSPREADSHEET lpSS, HDC hDC, LPRECT lpRect, BYTE bFlags);
//void           DrawBorderEx(LPLINEDATA lpLineData, int X1, int Y1, int X2, int Y2, 
//                              LPSSX_PRINTFORMAT lpPFormat);
int SS_GetCellOffsetX(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD Col, LPSSX_PRINTFORMAT lpPFormat, SS_COORD LeftCol);
int SS_GetCellOffsetY(LPSPREADSHEET lpSS, SS_COORD RowTop, SS_COORD Row, LPSSX_PRINTFORMAT lpPFormat, SS_COORD TopRow);
short SS_GetColWidthFromList(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat, SS_COORD LeftCol, SS_COORD Col);
short SS_GetRowHeightFromList(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat, SS_COORD TopRow, SS_COORD Row);

#ifdef SS_V80
void SS_AlphaBlendSelection(LPSS_BOOK lpBook, HDC hDC, LPRECT lpRect);
void SS_CT_Render(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT CellRect, LPRECT CellRectOrigNoScale, SS_COORD CellCol, SS_COORD CellRow, BOOL fCellSelected);
#endif // SS_V80

//--------------------------------------------------------------------

struct ss_drawborder
	{
	SS_COORD Col;
	SS_COORD Row;
	RECT     Rect;
	} SS_DRAWBORDER, FAR *LPSS_DRAWBORDER;

void SS_DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
  POINT pt[2] = {{x1, y1}, {x2, y2}};

  Polyline(hdc, pt, 2);
}
//--------------------------------------------------------------------

void SS_DrawSortIndicator(HDC hdc, RECT *prc, BOOL bAscending)
{
  RECT rc = *prc;
  COLORREF cr3DHiLite = GetSysColor(COLOR_BTNHIGHLIGHT);
  COLORREF cr3DShadow = GetSysColor(COLOR_BTNSHADOW);

  HPEN hpen3DHiLite = CreatePen(PS_SOLID, 0, cr3DHiLite);
  HPEN hpen3DShadow = CreatePen(PS_SOLID, 0, cr3DShadow);
  HPEN hpenOld;
  int x, y;       // location of arrow tip
#if defined(_WIN64) || defined(_IA64)
  long  nXAmt, nYAmt;
#else
  short nXAmt, nYAmt;
#endif

  nXAmt = (short)max(min(2, rc.right - rc.left), 4);
  nYAmt = (short)max(min(3, rc.bottom - rc.top), 6);

  x = (rc.left + rc.right + nXAmt) / 2;
  y = (rc.top + rc.bottom - nYAmt) / 2;

  if (bAscending)
     {
     hpenOld = SelectObject(hdc, hpen3DShadow);
     // draw line  
     SS_DrawLine(hdc, x - 1, y + 1, x - nXAmt, y + nYAmt);
     SelectObject(hdc, hpen3DHiLite);
     SS_DrawLine(hdc, x - nXAmt, y + nYAmt, x + nXAmt, y + nYAmt);
     // draw line 
     SS_DrawLine(hdc, x + nXAmt - 1, y + nYAmt, x, y);
     SelectObject(hdc, hpenOld);
     }
  else
     {
     // draw line  
     hpenOld = SelectObject(hdc, hpen3DShadow);
     SS_DrawLine(hdc, x - nXAmt, y + 1, x + nXAmt, y + 1);
     SS_DrawLine(hdc, x - 1, y + nYAmt, x - nXAmt, y);
     // draw line 
     SelectObject(hdc, hpen3DHiLite);
     SS_DrawLine(hdc, x, y + nYAmt, x + nXAmt - 1, y + 1);
     SelectObject(hdc, hpenOld);
     }

  DeleteObject(hpen3DHiLite);
  DeleteObject(hpen3DShadow);
}


BOOL SS_DrawText(HDC hDC, HWND hWnd, LPSPREADSHEET lpSS, LPSS_COL lpCol,
                 LPSS_ROW lpRow, LPRECT Rect, SS_COORD CellCol,
                 SS_COORD CellRow, BOOL fErase, LPSSX_PRINTFORMAT lpPFormat,
                 LPRECT lpRectUpdate, LPRECT lpCellOverflowRectPrev,
                 LPRECT lpCellOverflowRectNext, BOOL fOverflowToLeft,
                 BOOL fCellSelected, SS_COORD CellLeft, SS_COORD CellTop,
                 BOOL IsLastCol, BOOL IsLastRow)
{
#ifdef SS_V40
SS_SELBLOCK SpanBlock;
#endif // SS_V40
int         iSaveDC = 0;
RECT        RectOrig;
BOOL        fDrawSelection = FALSE;
BOOL			fMergeScan = FALSE;
BOOL			fIsCellAlreadyPainted = FALSE;
BOOL        fRet;

if (Rect->left >= Rect->right || Rect->top >= Rect->bottom)
   return (TRUE);

#ifdef SS_V30
// Determine if cells should be drawn as selected
if (!SS_INVERTSEL(lpSS) && !lpPFormat && 
	(lpSS->lpBook->fRetainSelBlock || SS_USESELBAR(lpSS) || (!lpSS->lpBook->fRetainSelBlock && GetFocus()==hWnd)))
   {
   if (fCellSelected == TRUE ||
       (fCellSelected == SS_DRAWTEXTSEL_DEF &&
        SS_ShouldCellBeDrawnSelected(lpSS, CellCol, CellRow, lpRow)))
      fDrawSelection = TRUE;
   }
#endif // SS_V30
if (!lpPFormat)
	fCellSelected = fDrawSelection;
else if (fCellSelected == SS_DRAWTEXTSEL_DEF)
	fCellSelected = FALSE;

#ifdef SS_V40
if (SS_SpanIsCellAlreadyPainted(lpSS, CellCol, CellRow))
	{
	if (!fCellSelected)
		return (TRUE);
	fIsCellAlreadyPainted = TRUE;
	}

/***********************
* Calculate Merge Info
***********************/

if (SS_SpanMergeCalcBlock(lpSS, CellRow, lpRow, CellCol, lpCol, &SpanBlock))
   {
	RECT RectOrig;

	CopyRect(&RectOrig, Rect);

   Rect->right = Rect->left + SS_GetCellOffsetX(lpSS, CellCol, SpanBlock.LR.Col + 1, lpPFormat, CellLeft) - 1;
   Rect->bottom = Rect->top + SS_GetCellOffsetY(lpSS, CellRow, SpanBlock.LR.Row + 1, lpPFormat, CellTop) - 1;
   Rect->left -= SS_GetCellOffsetX(lpSS, SpanBlock.UL.Col, CellCol, lpPFormat, CellLeft);
   Rect->top -= SS_GetCellOffsetY(lpSS, SpanBlock.UL.Row, CellRow, lpPFormat, CellTop);
   //Rect->left -= SS_GetCellOffsetX(lpSS, SpanBlock.UL.Col, CellCol, lpPFormat);
   //Rect->top -= SS_GetCellOffsetY(lpSS, SpanBlock.UL.Row, CellRow, lpPFormat);
   //Rect->right += SS_GetCellOffsetX(lpSS, CellCol + 1, SpanBlock.LR.Col + 1, lpPFormat);
   //Rect->bottom += SS_GetCellOffsetY(lpSS, CellRow + 1, SpanBlock.LR.Row + 1, lpPFormat);
   fErase = TRUE;

   if ((SpanBlock.UL.Col < CellLeft && SpanBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen) ||
       (SpanBlock.UL.Row < CellTop && SpanBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen))
      {
      RECT RectTemp;

      CopyRect(&RectTemp, Rect);

		/* RFW - 5/3/06 - 18850
      if (SpanBlock.UL.Col < CellLeft && SpanBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
         RectTemp.left = (int)SS_DFSCREEN2PRINTERX(lpPFormat, SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt + lpSS->Col.Frozen)) + 1;
      if (SpanBlock.UL.Row < CellTop && SpanBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
         RectTemp.top = (int)SS_DFSCREEN2PRINTERY(lpPFormat, SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt + lpSS->Row.Frozen)) + 1;
		*/
		/* RFW - 11/21/06 - 19640
      if (SpanBlock.UL.Col < CellLeft && SpanBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
          (!lpPFormat || lpPFormat->pf.fShowRowHeaders))
         RectTemp.left = (int)SS_DFSCREEN2PRINTERX(lpPFormat, SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt + lpSS->Col.Frozen)) + 1;
      if (SpanBlock.UL.Row < CellTop && SpanBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
          (!lpPFormat || lpPFormat->pf.fShowColHeaders))
         RectTemp.top = (int)SS_DFSCREEN2PRINTERY(lpPFormat, SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt + lpSS->Row.Frozen)) + 1;
		*/
		/* RFW - 4/8/08 - 21902
      if (SpanBlock.UL.Col < CellLeft && SpanBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
          (!lpPFormat || lpPFormat->pf.fShowRowHeaders || lpSS->Col.Frozen))
         RectTemp.left = (int)SS_DFSCREEN2PRINTERX(lpPFormat, SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt + lpSS->Col.Frozen)) + 1;
      if (SpanBlock.UL.Row < CellTop && SpanBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
          (!lpPFormat || lpPFormat->pf.fShowColHeaders || lpSS->Row.Frozen))
         RectTemp.top = (int)SS_DFSCREEN2PRINTERY(lpPFormat, SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt + lpSS->Row.Frozen)) + 1;
		*/
      if (SpanBlock.UL.Col < CellLeft && SpanBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
          (!lpPFormat || lpPFormat->pf.fShowRowHeaders || lpSS->Col.Frozen))
         RectTemp.left = (int)SS_DFSCREEN2PRINTERX(lpPFormat, SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt + lpSS->Col.Frozen) + 1);
      if (SpanBlock.UL.Row < CellTop && SpanBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
          (!lpPFormat || lpPFormat->pf.fShowColHeaders || lpSS->Row.Frozen))
         RectTemp.top = (int)SS_DFSCREEN2PRINTERY(lpPFormat, SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt + lpSS->Row.Frozen) + 1);

		RectTemp.left = max(0, RectTemp.left);
		RectTemp.top = max(0, RectTemp.top);

		if (!iSaveDC)
			iSaveDC = SaveDC(hDC);

      IntersectClipRect(hDC, RectTemp.left, RectTemp.top, RectTemp.right + 1, RectTemp.bottom + 1);
      }

	CellRow = SpanBlock.UL.Row;
	CellCol = SpanBlock.UL.Col;
	lpRow = SS_LockRowItem(lpSS, CellRow);
	lpCol = SS_LockColItem(lpSS, CellCol);
	fMergeScan = TRUE;

	if (fCellSelected)
		{
		if (!fIsCellAlreadyPainted)
			{
			RECT RectBefore = *Rect;
			SS_PaintCell(hDC, hWnd, lpSS, lpCol, lpRow, Rect, CellCol, CellRow, fErase, lpPFormat,
                      lpRectUpdate, lpCellOverflowRectPrev, lpCellOverflowRectNext, fOverflowToLeft,
                      FALSE, fMergeScan, IsLastCol, IsLastRow);
			*Rect = RectBefore;
			}

		if (!iSaveDC)
			iSaveDC = SaveDC(hDC);
      IntersectClipRect(hDC, RectOrig.left, RectOrig.top, RectOrig.right + 1, RectOrig.bottom + 1);
		}
   }
#endif // SS_V40

CopyRect(&RectOrig, Rect);
fRet = SS_PaintCell(hDC, hWnd, lpSS, lpCol, lpRow, Rect, CellCol, CellRow, fErase, lpPFormat,
                    lpRectUpdate, lpCellOverflowRectPrev, lpCellOverflowRectNext, fOverflowToLeft,
                    fCellSelected, fMergeScan, IsLastCol, IsLastRow);

#ifdef SS_V35
{
SS_COORD lColHeadersUserSortIndex = SS_GetColHeadersUserSortRow(lpSS);

// if col header painting, check user sort flags
// draw sort indicator if necessary
#ifdef SS_V40
if ((lColHeadersUserSortIndex == CellRow) &&
    (SS_USERCOLACTION_SORT == SS_GetUserColAction(lpSS)))
#else
if ((lColHeadersUserSortIndex == CellRow) && (CellCol == lpSS->Col.CurAt) &&
    (SS_USERCOLACTION_SORT == SS_GetUserColAction(lpSS)))
#endif // SS_V40
   {
   RECT rcSort;
   short nIndicator = SS_GetMergedColUserSortIndicator(lpSS, CellCol);

   CopyRect(&rcSort, Rect);
   InflateRect(&rcSort, -4, 0);
   rcSort.left = rcSort.right - min(10, rcSort.right - rcSort.left);

   if (SS_COLUSERSORTINDICATOR_ASCENDING == nIndicator)
      SS_DrawSortIndicator(hDC, &rcSort, TRUE);
   else if (SS_COLUSERSORTINDICATOR_DESCENDING == nIndicator)
      SS_DrawSortIndicator(hDC, &rcSort, FALSE);
   }
}

if (!lpPFormat && SS_GetCellNote(lpSS, CellCol, CellRow, NULL) &&
    CellCol >= lpSS->Col.HeaderCnt && CellRow >= lpSS->Row.HeaderCnt &&
    (SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT == lpSS->lpBook->nCellNoteIndicator ||
     SS_CELLNOTEINDICATOR_SHOWANDDONOTFIREEVENT == lpSS->lpBook->nCellNoteIndicator))
   SS_DrawCellNote(lpSS, hDC, &RectOrig, CellCol, CellRow);

#endif // SS_V35

if (fMergeScan)
	{
	SS_UnlockRowItem(lpSS, CellRow);
	SS_UnlockColItem(lpSS, CellCol);
	}

if (iSaveDC)
   RestoreDC(hDC, iSaveDC);

return (fRet);
}

#ifdef SS_V80
void SS_FlipBitmap(HDC srcDC, RECT srcRect, BOOL bLateral)
{
    HDC destDC = NULL;
	HBITMAP hbmResult = NULL;
    int nHeight, nWidth;

	destDC = CreateCompatibleDC(srcDC );

	nWidth = srcRect.right - srcRect.left;
	nHeight = srcRect.bottom - srcRect.top;

    hbmResult = CreateCompatibleBitmap(srcDC, nWidth, nHeight);
	SelectObject(destDC, hbmResult);
	if( bLateral )
		StretchBlt(destDC, 0, 0, nWidth, nHeight, srcDC,
				srcRect.right-1, srcRect.top, -nWidth+1, nHeight, SRCCOPY );
	else
		StretchBlt(destDC, 0, 0, nWidth, nHeight, srcDC,
				srcRect.left, srcRect.bottom-1, nWidth, -nHeight, SRCCOPY );
				
   
	StretchBlt(srcDC, srcRect.left, srcRect.top, nWidth, nHeight, destDC, 0, 0, nWidth, nHeight, SRCCOPY);
	
	DeleteDC(destDC);
	DeleteObject(hbmResult);


}

void SS_RotateBitmap(HDC bitmapDC, RECT bmpRect, HDC destDC, RECT rc, float angle)
 {
         float lSine;
         float lCosine;
         float X1;
         float X2;
         float X3;
         float Y1;
         float Y2;
         float Y3;
         float lMinX;
         float lMaxX;
         float lMinY;
         float lMaxY;
         long lNewWidth;
         long lNewHeight;
         long I;
         long J;
         float lSourceX;
         float lSourceY;
		 long lHeight;
         long lWidth;
		 HDC bmpDC = NULL;
		 HBITMAP hbmp = NULL;

		 bmpDC = CreateCompatibleDC(bitmapDC);

		 if (angle == 180.0)
		 {
			 lSine = 0.0;
			 lCosine = -1.0;
		 }
		 else if (angle == 90.0)
		 {
		 lSine = 1.0;
         lCosine = 0.0;
		 }
		 else
		 {
		 lSine = (FLOAT)sin(RADIANS(angle));
         lCosine = (FLOAT)cos(RADIANS(angle));
		 }

		 lHeight = bmpRect.bottom-bmpRect.top;
		 lWidth = bmpRect.right-bmpRect.left;

         X1 = -lHeight * lSine;
         Y1 = lHeight * lCosine;
         X2 = lWidth * lCosine - lHeight * lSine;
         Y2 = lHeight * lCosine + lWidth * lSine;
         X3 = lWidth * lCosine;
         Y3 = lWidth * lSine;

         lMinX = min(0, min(X1, min(X2, X3)));
         lMinY = min(0, min(Y1, min(Y2, Y3)));
         lMaxX = max(X1, max(X2, X3));
         lMaxY = max(Y1, max(Y2, Y3));

         lNewWidth = (LONG)(lMaxX - lMinX);
         lNewHeight = (LONG)(lMaxY - lMinY);

 		 hbmp = CreateCompatibleBitmap(bitmapDC, lNewWidth, lNewHeight);
		 SelectObject(bmpDC, hbmp);

         for (I = 0; I <= lNewHeight; I++)
		 {
			 for (J = 0; J <= lNewWidth; J++)
			 {
               lSourceX = (J + lMinX) * lCosine + (I + lMinY) * lSine;
               lSourceY = (I + lMinY) * lCosine - (J + lMinX) * lSine;
               if ((lSourceX >= 0) && (lSourceX <= lWidth) && (lSourceY >= 0) && (lSourceY <= lHeight))
                  BitBlt(bmpDC, J, I, 1, 1, bitmapDC, (int)lSourceX, (int)lSourceY, SRCCOPY);
			 }
		 }
         StretchBlt(destDC, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, bmpDC, 0, 0, lNewWidth, lNewHeight, SRCCOPY);

         lWidth = lNewWidth;
         lHeight = lNewHeight;
		 StretchBlt(bitmapDC, 0, 0, lHeight, lWidth, destDC, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, SRCCOPY);
		 DeleteDC(bmpDC);
		 DeleteObject(hbmp);


 }

void SS_DrawTriangle(LPSPREADSHEET lpSS, HDC hdc, RECT rcTriangle, COLORREF triangleColor, COLORREF triangleBorderColor)
{
	HPEN pen, penOld;
    float zoomFactor = 1.0;
	int TRIANGLE_SIZE_WIDTH = 12;
	int TRIANGLE_SIZE_HEIGHT = 12;
    int x1,y1, x2, y2;
    int scaleX, scaleY;

	HBRUSH fillBrush, oldBrush;

	RECT r;
    POINT ptsOutline[3];
    POINT ptsInner[3];
	HDC sdc = GetDC(lpSS->lpBook->hWnd);
  	
    x1 = GetDeviceCaps(hdc, LOGPIXELSX);
    y1 = GetDeviceCaps(hdc, LOGPIXELSY);
    x2 = GetDeviceCaps(sdc, LOGPIXELSX);
    y2 = GetDeviceCaps(sdc, LOGPIXELSY);

	ReleaseDC(lpSS->lpBook->hWnd, sdc);

    scaleX = x1/x2;
    scaleY = y1/y2;

    r.left = rcTriangle.right - (int)((4 + TRIANGLE_SIZE_WIDTH) * zoomFactor * scaleX);
	r.top =  rcTriangle.bottom - (int)((3 + TRIANGLE_SIZE_HEIGHT) * zoomFactor * scaleY);
	r.right = r.left + (int)(TRIANGLE_SIZE_WIDTH * zoomFactor * scaleX);
	r.bottom = r.top + (int)(TRIANGLE_SIZE_HEIGHT * zoomFactor * scaleY);

    ptsOutline[0].x = r.right;
	ptsOutline[0].y = r.top + 1 ; 
    ptsOutline[1].x = r.right;
	ptsOutline[1].y = r.bottom ; 
    ptsOutline[2].x = r.left + 1;
	ptsOutline[2].y = r.bottom; 
    ptsInner[0].x = r.right;
	ptsInner[0].y = r.top + 2; 
    ptsInner[1].x = r.right;
	ptsInner[1].y = r.bottom; 
    ptsInner[2].x = r.left + 2;
	ptsInner[2].y = r.bottom; 

    pen = CreatePen(PS_SOLID, 1, triangleBorderColor);
	penOld = SelectObject(hdc, pen);

	fillBrush = CreateSolidBrush(triangleColor);
	oldBrush = SelectObject(hdc, fillBrush);
	SetPolyFillMode(hdc, 1);
    Polygon(hdc, ptsOutline, 3);
    Polygon(hdc, ptsInner, 3);
	SelectObject(hdc, penOld);
	SelectObject(hdc,oldBrush);
	DeleteObject(fillBrush);
	DeleteObject(pen);
}
void SS_DrawEnhancedCorner(LPSPREADSHEET lpSS, HDC hdc, RECT rc, BOOL mouseOver)
{
	 HBRUSH brush;
	 HPEN pen;
	 HPEN penOld;

        if (mouseOver)
		{
			brush = CreateSolidBrush(lpSS->lpBook->cornerHoverColor);
		}
		else
		{
			brush = CreateSolidBrush(lpSS->lpBook->cornerBackColor);
		}

      FillRect(hdc, &rc, brush);
		DeleteObject(brush);

        SS_DrawTriangle(lpSS, hdc, rc, mouseOver ? lpSS->lpBook->cornerTriangleHoverColor : lpSS->lpBook->cornerTriangleColor,
			mouseOver ? lpSS->lpBook->cornerTriangleHoverBorderColor : lpSS->lpBook->cornerTriangleBorderColor);

		pen = CreatePen(PS_SOLID, 1, lpSS->lpBook->cornerTriangleColor /*RGB(213, 228, 242)*/);
		penOld = SelectObject(hdc, pen);
		SS_DrawLine(hdc, rc.left, rc.top, rc.right, rc.top);
		SS_DrawLine(hdc, rc.left, rc.top, rc.left, rc.bottom);
		SelectObject(hdc,penOld);
		DeleteObject(pen);
		pen = CreatePen(PS_SOLID, 1, lpSS->lpBook->cornerTriangleBorderColor/*RGB(176, 207, 247)*/);
		penOld = SelectObject(hdc, pen);
		SS_DrawLine(hdc, rc.right-1, rc.top+1, rc.right-1, rc.bottom-1);
		SS_DrawLine(hdc, rc.left, rc.bottom-1, rc.right, rc.bottom-1);
		SelectObject(hdc,penOld);
		DeleteObject(pen);
		
}
void SS_DrawHeaderEnhanced(LPSPREADSHEET lpSS, HDC hdc, BOOL isColHeader, BOOL isRowHeader, RECT RectTemp, BOOL selected, BOOL active)
{

	HBRUSH upperBrush;
	HBRUSH lowerBrush;
	HBRUSH borderBrush;

	if ((!lpSS->lpBook->fRetainSelBlock && GetFocus() != lpSS->lpBook->hWnd && !lpSS->lpBook->EditModeOn) || !SS_SEL_HIGHLIGHTHEADERS(lpSS))
		selected = FALSE;

	if (isColHeader && !isRowHeader)
	{
		if (selected && active)
		{
		upperBrush = CreateSolidBrush(lpSS->lpBook->colSelectedHoverUpperColor);       
		lowerBrush = CreateSolidBrush(lpSS->lpBook->colSelectedHoverLowerColor);
		}
		else if (selected)
		{
		upperBrush = CreateSolidBrush(lpSS->lpBook->colSelectedUpperColor);       
		lowerBrush = CreateSolidBrush(lpSS->lpBook->colSelectedLowerColor);
		}
		else if (active)
		{
		upperBrush = CreateSolidBrush(lpSS->lpBook->colHoverUpperColor);       
		lowerBrush = CreateSolidBrush(lpSS->lpBook->colHoverLowerColor);
		}
		else 
		{
		upperBrush = CreateSolidBrush(lpSS->lpBook->colUpperColor);       
		lowerBrush = CreateSolidBrush(lpSS->lpBook->colLowerColor);
		}
	fpGradientFill(hdc, RectTemp, upperBrush, lowerBrush, GRADIENT_FILL_RECT_V);
		DeleteObject(upperBrush);
		DeleteObject(lowerBrush);
		if (selected)
			borderBrush = CreateSolidBrush(lpSS->lpBook->colHeaderSelectedBorderColor);
		else
			borderBrush = CreateSolidBrush(lpSS->lpBook->colHeaderBorderColor);
	}
	else if (isRowHeader && !isColHeader)
	{
		if (selected && active)
		{
		upperBrush = CreateSolidBrush(lpSS->lpBook->rowSelectedHoverUpperColor);       
		lowerBrush = CreateSolidBrush(lpSS->lpBook->rowSelectedHoverLowerColor);
		}
		else if (selected)
		{
		upperBrush = CreateSolidBrush(lpSS->lpBook->rowSelectedUpperColor);       
		lowerBrush = CreateSolidBrush(lpSS->lpBook->rowSelectedLowerColor);
		}
		else if (active)
		{
		upperBrush = CreateSolidBrush(lpSS->lpBook->rowHoverUpperColor);       
		lowerBrush = CreateSolidBrush(lpSS->lpBook->rowHoverLowerColor);
		}
		else 
		{
		upperBrush = CreateSolidBrush(lpSS->lpBook->rowUpperColor);       
		lowerBrush = CreateSolidBrush(lpSS->lpBook->rowLowerColor);
		}
		//FillRect(hdc, &RectTemp, upperBrush);
		fpGradientFill(hdc, RectTemp, upperBrush, lowerBrush, GRADIENT_FILL_RECT_V);
		DeleteObject(upperBrush);
		DeleteObject(lowerBrush);
		if (selected)
			borderBrush = CreateSolidBrush(lpSS->lpBook->rowHeaderSelectedBorderColor);
		else
			borderBrush = CreateSolidBrush(lpSS->lpBook->rowHeaderBorderColor);
	}
	else
	{
		SS_DrawEnhancedCorner(lpSS, hdc, RectTemp, active);
		if (selected)
			borderBrush = CreateSolidBrush(lpSS->lpBook->colHeaderSelectedBorderColor);
		else
			borderBrush = CreateSolidBrush(lpSS->lpBook->colHeaderBorderColor);
	}

	RectTemp.left--;
	RectTemp.right++;
	RectTemp.top--;
	RectTemp.bottom++;
	FrameRect(hdc, &RectTemp, borderBrush);

	DeleteObject(borderBrush);

}


void SS_DrawVisualStyleHeader(LPSS_BOOK lpBook, HDC hdc, HWND hWnd, BOOL isColHeader, BOOL isRowHeader, RECT rect, BOOL selected, BOOL active)
{
	HTHEME hTheme = NULL;
	HBITMAP membmpDC = NULL;
	HDC memDC = NULL;
	HBITMAP hbmpOld = NULL;
   RECT rc;

	if (isRowHeader)
		rect.left--;
	else
		rect.top--;

	if ((lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERHORZGRIDONLY ||
		  lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID))
		rect.bottom++;

	if ((lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERVERTGRIDONLY ||
		  lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID))
		rect.right++;

   rc.left = 0;
   rc.right = rect.right-rect.left;
   rc.top = 0;
   rc.bottom = rect.bottom-rect.top;
	
	if (isRowHeader)
	{
		rc.right = rc.left + (rect.bottom-rect.top);
		rc.bottom = rc.top + (rect.right-rect.left);
	}

	hTheme = SS_OpenThemeData(hWnd, L"Header");
	memDC = CreateCompatibleDC(hdc);
	membmpDC = CreateCompatibleBitmap(hdc, rc.right-rc.left, rc.bottom-rc.top);
	hbmpOld = SelectObject(memDC, membmpDC);
	if (isRowHeader)
	{
		SS_DrawThemeBackground( hTheme, memDC, 1, active ? 2 : 1, &rc, &rc);
		 SS_RotateBitmap(memDC, rc, hdc, rect, 90.0);
		 SS_FlipBitmap(hdc, rect, TRUE);
	}
	else
	{
		SS_DrawThemeBackground( hTheme, memDC, 1, active ? 2 : 1, &rc, &rc);
		BitBlt(hdc, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, memDC, 0,0, SRCCOPY);
	}
	SS_CloseThemeData(hTheme);
	SelectObject(memDC, hbmpOld);
	DeleteDC(memDC);
	DeleteObject(membmpDC);
}


#if 0 // RFW - 4/3/09 - 24942
void SS_DrawVisualStyleHeader(HDC hdc, HWND hWnd, BOOL isColHeader, BOOL isRowHeader, RECT rect, BOOL selected, BOOL active)
{
	HTHEME hTheme = NULL;
	HBITMAP membmpDC = NULL;
	HDC memDC = NULL;
	HBITMAP hbmpOld = NULL;

   RECT rc = {0,0, rect.right-rect.left, rect.bottom-rect.top};	
	
	if (isColHeader)
	{
		rect.right++;
		rect.bottom++;
		rc.right++;
		rc.bottom++;
	}
	else
	{
		/* RFW - 4/3/09 - 24942
		rect.left--;
		*/
		rect.right++;
		rect.bottom++;
		rc.right = rc.left + (rect.bottom-rect.top);
		rc.bottom = rc.top + (rect.right-rect.left);
	}


	hTheme = SS_OpenThemeData(hWnd, L"Header");
	memDC = CreateCompatibleDC(hdc);
	membmpDC = CreateCompatibleBitmap(hdc, rc.right-rc.left, rc.bottom-rc.top);
	hbmpOld = SelectObject(memDC, membmpDC);
	if (isRowHeader)
	{
		SS_DrawThemeBackground( hTheme, memDC, 1, active ? 2 : 1, &rc, &rc);
		 SS_RotateBitmap(memDC, rc, hdc, rect, 90.0);
		 SS_FlipBitmap(hdc, rect, TRUE);
	}
	else
	{
		SS_DrawThemeBackground( hTheme, memDC, 1, active ? 2 : 1, &rc, &rc);
		BitBlt(hdc, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, memDC, 0,0, SRCCOPY);
	}
	SS_CloseThemeData(hTheme);
	SelectObject(memDC, hbmpOld);
	DeleteDC(memDC);
	DeleteObject(membmpDC);
}
#endif

BOOL SS_IsAnyCellInRowSelected(LPSPREADSHEET lpSS, SS_COORD Row)
{
	BOOL ret = FALSE;

	if (lpSS->MultiSelBlock.dItemCnt > 0)
	{
		ret = SS_IsCellInSelection(lpSS, -1, Row);
		if (!ret && Row >= lpSS->BlockCellUL.Row)
			if (Row <= lpSS->BlockCellLR.Row || lpSS->BlockCellLR.Row == -1)
				ret = TRUE;
	}
	else if (SS_IsBlockSelected(lpSS))
	{
		if (lpSS->BlockCellUL.Col == 0 && lpSS->BlockCellLR.Col == -1)
			return FALSE;
		if (Row >= lpSS->BlockCellUL.Row)
			if (Row <= lpSS->BlockCellLR.Row || lpSS->BlockCellLR.Row == -1)
				ret = TRUE;
	}
	else if (lpSS->Row.CurAt == Row)
		ret = TRUE;
	return ret;

}

BOOL SS_IsAnyCellInColumnSelected(LPSPREADSHEET lpSS, SS_COORD Col)
{
	BOOL ret = FALSE;
	if (lpSS->wOpMode > 2)
		ret = TRUE;
	else if (lpSS->wOpMode == 2 && !lpSS->fRowModeEditing)
		ret = TRUE;
	else if (lpSS->MultiSelBlock.dItemCnt > 0)
	{
		ret = SS_IsCellInSelection(lpSS, Col, -1);
		if (!ret && Col >= lpSS->BlockCellUL.Col)
			if (Col <= lpSS->BlockCellLR.Col || lpSS->BlockCellLR.Col == -1)
				ret = TRUE;
	}
	else if (SS_IsBlockSelected(lpSS))
	{
		if (lpSS->BlockCellUL.Row == 0 && lpSS->BlockCellLR.Row == -1)
			return FALSE;
		if (Col >= lpSS->BlockCellUL.Col)
			if (Col <= lpSS->BlockCellLR.Col || lpSS->BlockCellLR.Col == -1)
				ret = TRUE;
	}
	else if (lpSS->Col.CurAt == Col)
		ret = TRUE;
	return ret;
}
#endif

static BOOL SS_PaintCell(HDC hDC, HWND hWnd, LPSPREADSHEET lpSS, LPSS_COL lpCol,
                  LPSS_ROW lpRow, LPRECT Rect, SS_COORD CellCol,
                  SS_COORD CellRow, BOOL fErase, LPSSX_PRINTFORMAT lpPFormat,
                  LPRECT lpRectUpdate, LPRECT lpCellOverflowRectPrev,
                  LPRECT lpCellOverflowRectNext, BOOL fOverflowToLeft,
                  BOOL fCellSelected, BOOL fMergeScan, BOOL IsLastCol, BOOL IsLastRow)
{
TBGLOBALHANDLE    hGlobalData = 0;
//COLORREF lpColors[4] = {RGB(249, 252, 253), RGB(211, 220, 233), RGB(249, 252, 253), RGB(211, 220, 233)};
HFONT             hFontOld = 0;
SS_COLORTBLITEM   ColorTblItem;
SS_COLORTBLITEM   ColorTblItemFore;
SS_COLORTBLITEM   ColorTblItemLight;
COLORREF          ForegroundOld;
COLORREF          Background;
COLORREF          BackgroundActual;
HBRUSH            hBrush;
HBRUSH            hBrush1;
HBRUSH            hBrush2;
HFONT             hFont;
RECT              RectOrig;
RECT              RectOrigNoScale;
RECT              RectTemp;
#ifndef SS_V30
RECT              RectUpdateTemp;
RECT              CellOverflowRectPrevTemp;
RECT              CellOverflowRectNextTemp;
#endif
LPSS_CELL         Cell;
LPSS_CELLTYPE     CellType;
SS_CELLTYPE       CellTypeTemp;
SS_DATA           SSData;
LPSS_FONT         Font;
SS_FONT           FontTemp;
SS_FONTID         FontId;
BOOL              fButtonState;
TCHAR             Temp[10];
LPTSTR            Data = NULL;
HWND              hWndCtrl = 0;
short             Attr = 0;
SS_COORD          Number;
SS_COORD          CurVisCellRow;
SS_COORD          CurVisCellCol;
SS_COORD          lHeaderRowsDisplayIndex;
SS_COORD          lHeaderColsDisplayIndex;
short             dShadowSize;
UINT              Just;
short             dXMargin = SS_SCREEN2PRINTERX(lpPFormat, lpSS->lpBook->dXMargin);
short             dYMargin = SS_SCREEN2PRINTERY(lpPFormat, lpSS->lpBook->dYMargin);
short             i;
BOOL              bBackDiff;
BOOL              fDrawSelBack = FALSE;
BOOL              fDrawSelFore = FALSE;
BOOL              fDrawSelBoth = FALSE;
BOOL              fAllowCellOverflow = (lpSS->lpBook->fAllowCellOverflow && !fMergeScan);
BOOL              fSelAlpha = FALSE;
BOOL              fSelAlpha2 = FALSE; // This is the current method, which applies the alpha after the cell has painted
#ifdef SS_V80
RECT RectAlphaFill; // RFW - 4/17/09 - 25434
BOOL toScreen = GetDeviceCaps(hDC, TECHNOLOGY) == DT_RASDISPLAY;
BOOL isColHeader = FALSE;
BOOL isRowHeader = FALSE;
BOOL bDrawEnhanced = lpSS->lpBook->bEnhanceStaticCells;
BOOL isCorner = FALSE;
BOOL isVisualStyles =lpSS->lpBook->fThemesActive;
HTHEME hTheme = NULL;
BOOL colSelected = FALSE;
BOOL rowSelected = FALSE;
#endif
#ifdef SS_V30
// Determine if cells should be drawn as selected
if (fCellSelected == TRUE)
   {
	if (SS_SEL_ALPHA(lpSS))
		{
		fErase = TRUE;
		fSelAlpha2 = TRUE;
		}
	else
		{
		if (lpSS->lpBook->SelColor.ForegroundId)
			fDrawSelFore = TRUE;
		if (lpSS->lpBook->SelColor.BackgroundId)
			fDrawSelBack = TRUE;

		fDrawSelBoth = (fDrawSelBack && fDrawSelFore);
		}
   }
#endif // SS_V30

SetBkMode(hDC, TRANSPARENT);

CopyRect(&RectOrigNoScale, Rect);

if (lpPFormat)
   {
   SS_ScaleRect(lpPFormat, Rect);
   CopyRect(&RectOrig, Rect);

#ifdef SS_V30
   RectOrigNoScale.left = SS_PRINTER2SCREENX(lpPFormat, Rect->left);
   RectOrigNoScale.top = SS_PRINTER2SCREENY(lpPFormat, Rect->top);
   RectOrigNoScale.right = SS_PRINTER2SCREENX(lpPFormat, Rect->right);
   RectOrigNoScale.bottom = SS_PRINTER2SCREENY(lpPFormat, Rect->bottom);
#else
   RectUpdateTemp.left = SS_SCREEN2PRINTERX(lpPFormat, lpRectUpdate->left);
   RectUpdateTemp.top = SS_SCREEN2PRINTERY(lpPFormat, lpRectUpdate->top);
   RectUpdateTemp.right = SS_SCREEN2PRINTERX(lpPFormat, lpRectUpdate->right);
   RectUpdateTemp.bottom = SS_SCREEN2PRINTERY(lpPFormat, lpRectUpdate->bottom);
   lpRectUpdate = &RectUpdateTemp;

   if (lpCellOverflowRectPrev)
      {
      CopyRect(&CellOverflowRectPrevTemp, lpCellOverflowRectPrev);
      lpCellOverflowRectPrev = &CellOverflowRectPrevTemp;

      SS_ScaleRect(lpPFormat, lpCellOverflowRectPrev);
      }

   if (lpCellOverflowRectNext)
      {
      CopyRect(&CellOverflowRectNextTemp, lpCellOverflowRectNext);
      lpCellOverflowRectNext = &CellOverflowRectNextTemp;

      SS_ScaleRect(lpPFormat, lpCellOverflowRectNext);
      }
#endif
   }

else
   CopyRect(&RectOrig, Rect);

Cell = SSx_LockCellItem(lpSS, lpRow, CellCol, CellRow);

#ifndef SS_NOOVERFLOW
if (fAllowCellOverflow)
   if (Cell && Cell->Data.bOverflow)
      {
      SSx_UnlockCellItem(lpSS, lpRow, CellCol, CellRow);
      return (TRUE);
      }
#endif

CellType = SSx_RetrieveCellType(lpSS, &CellTypeTemp, lpCol, lpRow, Cell,
                                CellCol, CellRow);

#ifdef SS_UTP
if (CellType->Type == SS_TYPE_EDIT && lpSS->fPrintExpandMultiLine &&
    lpSS->fPrintingInProgress)
   CellType->Style |= ES_MULTILINE;
#endif

SSData.bDataType = 0;
if (SS_IsCellTypeFloatNum(CellType))
   {
   if (hGlobalData = SSx_GetData(lpSS, lpCol, lpRow, Cell, CellType,
                                 &SSData, CellCol, CellRow, TRUE))
      Data = (LPTSTR)tbGlobalLock(hGlobalData);
   else if (SSData.bDataType == SS_TYPE_FLOAT)
      Data = (LPTSTR)1;
   }
else
   {
   if (hGlobalData = SSx_GetData(lpSS, lpCol, lpRow, Cell, CellType,
                                 NULL, CellCol, CellRow, TRUE))
      Data = (LPTSTR)tbGlobalLock(hGlobalData);
   }

/*****************
* Format Headers
*****************/

lHeaderRowsDisplayIndex = lpSS->Row.lHeaderDisplayIndex >= 0 &&
                          lpSS->Row.lHeaderDisplayIndex < lpSS->Row.HeaderCnt ?
                          lpSS->Row.lHeaderDisplayIndex : lpSS->Row.HeaderCnt - 1;
lHeaderColsDisplayIndex = lpSS->Col.lHeaderDisplayIndex >= 0 &&
                          lpSS->Col.lHeaderDisplayIndex < lpSS->Col.HeaderCnt ?
                          lpSS->Col.lHeaderDisplayIndex : lpSS->Col.HeaderCnt - 1;

if (CellCol >= lpSS->Col.HeaderCnt && CellRow < lpSS->Row.HeaderCnt &&
    CellRow == lHeaderRowsDisplayIndex &&
    (!hGlobalData || !Cell || !Cell->Data.bDataType))
   {
   if (CellRow < lpSS->Row.HeaderCnt)
      {
      if (CellType->Type == SS_TYPE_CHECKBOX)
         Data = _T("");
      else
         {
         Number = CellCol - lpSS->Col.HeaderCnt + lpSS->Col.NumStart;

         if (lpSS->ColHeaderDisplay == SS_HEADERDISPLAY_LETTERS)
            SS_DrawFormatHeaderLetter(Temp, Number);
         else if (lpSS->ColHeaderDisplay == SS_HEADERDISPLAY_NUMBERS)
            SS_DrawFormatHeaderNumber(Temp, Number);
         else
            _fmemset(Temp, '\0', sizeof(Temp));

         Data = Temp;
         }
      }
   }

else if (CellRow >= lpSS->Row.HeaderCnt && CellCol < lpSS->Col.HeaderCnt &&
         CellCol == lHeaderColsDisplayIndex &&
         (!hGlobalData || !Cell || !Cell->Data.bDataType))
   {
   if (CellCol < lpSS->Col.HeaderCnt)
      {
      if (CellType->Type == SS_TYPE_CHECKBOX)
         Data = _T("");
      else if (lpSS->fVirtualMode &&
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
            _fmemset(Temp, '\0', sizeof(Temp));

         Data = Temp;
         }
      }
   }

// RFW - 3/28/02
//else if (CellCol < lpSS->Col.HeaderCnt && CellRow < lpSS->Row.HeaderCnt && (!Cell || !Cell->Data.bDataType))
else if ((CellCol < lpSS->Col.HeaderCnt || CellRow < lpSS->Row.HeaderCnt) && (!Cell || !Cell->Data.bDataType))
   Data = _T("");

#ifdef SS_V80
isColHeader = CellRow < lpSS->Row.HeaderCnt;
isRowHeader = CellCol < lpSS->Col.HeaderCnt;
isCorner = isColHeader && isRowHeader;
if (isColHeader && !lpPFormat)
	colSelected = SS_IsAnyCellInColumnSelected(lpSS, CellCol);
if (isRowHeader  && !lpPFormat)
	rowSelected = SS_IsAnyCellInRowSelected(lpSS, CellRow);

#endif

CurVisCellRow = lpSS->lpBook->CurVisCell.Row;
CurVisCellCol = lpSS->lpBook->CurVisCell.Col;
lpSS->lpBook->CurVisCell.Row = CellRow;
lpSS->lpBook->CurVisCell.Col = CellCol;

SS_GetColorItem(&ColorTblItem, lpSS->Color.BackgroundId);
Background = ColorTblItem.Color;
BackgroundActual = ColorTblItem.Color;

#ifdef SS_V30
if (CellRow >= lpSS->Row.HeaderCnt)
   {
   SS_COLORTBLITEM ColorTblItemTemp;

   if ((CellRow - lpSS->Row.HeaderCnt + 1) % 2) // Odd
      {
      if (lpSS->OddRowColor.BackgroundId)
         {
         SS_GetColorItem(&ColorTblItemTemp, lpSS->OddRowColor.BackgroundId);
//         BackgroundActual = ColorTblItemTemp.Color;
         }
      }
   else         // Even
      {
      if (lpSS->EvenRowColor.BackgroundId)
         {
         SS_GetColorItem(&ColorTblItemTemp, lpSS->EvenRowColor.BackgroundId);
//         BackgroundActual = ColorTblItemTemp.Color;
         }
      }
   }
#endif

#ifdef SS_V80
	CopyRect(&RectAlphaFill, Rect);
#endif

switch (CellType->Type)
   {
   case SS_TYPE_STATICTEXT:

      /************************
      * Paint Cell Background
      ************************/     
      hBrush = ColorTblItem.hBrush;
#ifdef SS_V80
	if (bDrawEnhanced && !(isColHeader || isRowHeader))
		isColHeader = TRUE;
#endif
      if (!fDrawSelBoth)
         SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                             CellRow, &ColorTblItem, &ColorTblItemFore);

      if (fDrawSelBack)
         {
         SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
         if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
            BackgroundActual = ColorTblItem.Color;
         }

		else if (lpSS->LockColor.BackgroundId &&
               SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
         SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);

      if (fAllowCellOverflow)
         {
         Just = SS_RetrieveJust((short)CellType->Style);
         if (!(Just & DT_WORDBREAK))
            if (SS_CalcOverflowRect(hDC, lpSS, Data, CellCol, Just, Rect,
                                    lpCellOverflowRectPrev,
                                    lpCellOverflowRectNext, Rect))
               fErase = TRUE;
         }

      /*******************
      * Draw static text
      *******************/

      if ((!lpPFormat || lpPFormat->pf.fDrawShadows) &&
          (CellType->Style & (SS_TEXT_SHADOW | SS_TEXT_SHADOWIN)))
         {
         SS_GetColorItem(&ColorTblItem, lpSS->Color.ShadowColorId);

#ifdef SS_V80
         if (ColorTblItem.Color != Background || fErase || bDrawEnhanced || isColHeader || isRowHeader)
#else
         if (ColorTblItem.Color != Background || fErase)
#endif
            {
				RECT RectTemp;

				if (!(CellType->Style & (SS_TEXT_SHADOW | SS_TEXT_SHADOWIN)))
				   SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                               ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);
				else
				   CopyRect(&RectTemp, Rect);

				// RFW - 5/24/07 - 20492
#ifdef SS_V80
				if (lpSS->lpBook->wAppearanceStyle == 0 && lpPFormat)
					SS_ShrinkRect(&RectTemp, 1, 1);
#else
				if (lpPFormat)
					SS_ShrinkRect(&RectTemp, 1, 1);
#endif

				//Modify by BOC 99.6.1(hyt)------------------
				//for not print color when PrintColor=false
				if (lpPFormat && !lpPFormat->pf.fDrawColors)
				{
					hBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
#ifdef SS_V80
					if (lpSS->lpBook->wAppearanceStyle == 2 && (isColHeader || isRowHeader || bDrawEnhanced))
						SS_DrawHeaderEnhanced(lpSS, hDC, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
					else if ( lpSS->lpBook->wAppearanceStyle == 1 && (isColHeader || isRowHeader) && !isCorner && isVisualStyles && toScreen)
						SS_DrawVisualStyleHeader(lpSS->lpBook, hDC, hWnd, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
					else
#endif
					FillRect(hDC, &RectTemp, hBrush);

				}
				else
				{
					hBrush = ColorTblItem.hBrush;
					UnrealizeObject(hBrush);
#ifdef SS_V80
//					if ((lpSS->lpBook->wAppearanceStyle == 2 || (SS_SEL_HIGHLIGHTHEADERS(lpSS) && (isColHeader ? colSelected : rowSelected))) &&
//                   (isColHeader || isRowHeader))
					if (lpSS->lpBook->wAppearanceStyle == 2 && (isColHeader || isRowHeader || bDrawEnhanced))
					{
						SS_DrawHeaderEnhanced(lpSS, hDC, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
					}
					else if (lpSS->lpBook->wAppearanceStyle && (isColHeader || isRowHeader) && !isCorner && isVisualStyles && toScreen)
					{
						SS_DrawVisualStyleHeader(lpSS->lpBook, hDC, hWnd, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
					}
					else if (fSelAlpha)
					   fpAlphaFill(hDC, RectTemp, ColorTblItem.hBrush, SS_TranslateColor(lpSS->lpBook->clrHighlightAlphaBlend), lpSS->lpBook->bHighlightAlphaBlend);
				   else
#endif
					FillRect(hDC, &RectTemp, ColorTblItem.hBrush);
				}
				//-----------------------------------------------
            }

         SS_GetColorItem(&ColorTblItem, lpSS->Color.ShadowDarkId);
         SS_GetColorItem(&ColorTblItemLight, lpSS->Color.ShadowLightId);

         if (CellType->Style & SS_TEXT_SHADOW)
            {
            hBrush1 = ColorTblItem.hBrush;
            hBrush2 = ColorTblItemLight.hBrush;
            }
         else
            {
            hBrush1 = ColorTblItemLight.hBrush;
            hBrush2 = ColorTblItem.hBrush;
            }

         CopyRect(&RectTemp, Rect);
#ifdef SS_V35
         dShadowSize = 1;
#ifndef SS_V40
         if (CellCol < lpSS->Col.HeaderCnt || CellRow < lpSS->Row.HeaderCnt)
#endif // SS_V40
         {
           RectTemp.right++;
           RectTemp.bottom++;
         }
#else
         dShadowSize = (CellCol == 0 && CellRow == 0) ? 2 : 1;
#endif

         if (lpPFormat)
            dShadowSize = SS_SCREEN2PRINTERX(lpPFormat, dShadowSize);
#ifdef SS_V80
		 else if (lpSS->lpBook->wAppearanceStyle == 0 || !isVisualStyles || ! (isColHeader || isRowHeader || bDrawEnhanced))
#else
         else
#endif
           SS_DrawShadows(hDC, &RectTemp, dShadowSize, hBrush1, hBrush2,
                           fOverflowToLeft ? TRUE : FALSE);

			CopyRect(Rect, &RectTemp); // RFW - -8/5/04
         SS_ShrinkRect(Rect, dShadowSize, dShadowSize);
         if (fOverflowToLeft)
            Rect->left -= dShadowSize;
         }
#ifdef SS_V80
      else if ((!lpPFormat || lpPFormat->pf.fDrawColors || fErase) &&
               (ColorTblItem.Color != Background || fErase || bDrawEnhanced || isColHeader || isRowHeader))
#else
      else if ((!lpPFormat || lpPFormat->pf.fDrawColors || fErase) &&
               (ColorTblItem.Color != Background || fErase))
#endif
         {
         RECT RectTemp;

         if (lpPFormat && !lpPFormat->pf.fDrawColors && fErase)
            hBrush = GetStockObject(WHITE_BRUSH);
         else
            {
            hBrush = ColorTblItem.hBrush;
            UnrealizeObject(hBrush);
            }

         SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                         ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);
#ifdef SS_V80
         //if ((lpSS->lpBook->wAppearanceStyle == 2 || (SS_SEL_HIGHLIGHTHEADERS(lpSS) && !lpPFormat && (isColHeader ? colSelected : rowSelected))) &&
           //  (isColHeader || isRowHeader))
			if (lpSS->lpBook->wAppearanceStyle == 2 && (isColHeader || isRowHeader || bDrawEnhanced))
            {
            SS_DrawHeaderEnhanced(lpSS, hDC, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
            }
         else if (lpSS->lpBook->wAppearanceStyle == 1 && (isColHeader || isRowHeader) && !isCorner && isVisualStyles && toScreen)
            {
            SS_DrawVisualStyleHeader(lpSS->lpBook, hDC, hWnd, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
            }	
         else if (fSelAlpha)
            fpAlphaFill(hDC, RectTemp, ColorTblItem.hBrush, SS_TranslateColor(lpSS->lpBook->clrHighlightAlphaBlend), lpSS->lpBook->bHighlightAlphaBlend);
         else
#endif

				FillRect(hDC, &RectTemp, hBrush);
         }

      if (Data && *Data)
         {
         FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol,
                                     CellRow);
         Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);

         if (lpPFormat)
            hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, lpPFormat);
         else
            hFont = Font->hFont;

         hFontOld = SelectObject(hDC, hFont);

         if (CellType->Style & (SS_TEXT_SHADOW | SS_TEXT_SHADOWIN))
            SS_GetColorItem(&ColorTblItemFore, lpSS->Color.ShadowTextId);

         else if (fDrawSelFore)
            SS_GetColorItem(&ColorTblItemFore, lpSS->lpBook->SelColor.ForegroundId);

         else if (lpSS->LockColor.ForegroundId &&
                  SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol,
                              CellRow, TRUE))
            SS_GetColorItem(&ColorTblItemFore,
                            lpSS->LockColor.ForegroundId);

         if (!lpPFormat || lpPFormat->pf.fDrawColors)
            ForegroundOld = SetTextColor(hDC, SS_TranslateColor(ColorTblItemFore.Color));

         /*
         if (CellType->Style & SS_TEXT_VERTICALDOWN)
            {
            Just = SS_RetrieveVertTextJust((short)CellType->Style);
            Just = SS_RetrieveJust((short)CellType->Style);
            SS_VertDrawText(hDC, hBrush, Font->hFont, Data, Rect, Just);
            }

         else
         */
            {
            Just = SS_RetrieveJust((short)CellType->Style);
#ifdef SS_V35
#ifdef SS_V40
				if (!(CellType->Style & SSS_TEXTORIENT_MASK))
#endif // SS_V40
	            if ((CellType->Style & SSS_ELLIPSES) && 
		             (!((Just & DT_CENTER) || (Just & DT_RIGHT))))
			         Just |= DTX_ELLIPSES;  
#endif
//-------------Commented by Dean 2000/03/16------------
//-To fix the bug that appearence does not change when change TypeVAlign.
//#if 0 // RFW - 6/21/99 - Bottom alignment with wordwrap and text with
      //                 no spaces didn't wrap
//-----------------------------------------------------
#ifdef SS_V40
				if ((Just & DT_WORDBREAK) && !(CellType->Style & SSS_TEXTORIENT_MASK))
#else
            if (Just & DT_WORDBREAK)
#endif // SS_V40
               {
					dXMargin = 1; // RFW - -8/5/04

               if (fOverflowToLeft)
                  RectTemp.left = Rect->left;
               else
                  RectTemp.left = Rect->left + dXMargin;

               RectTemp.right = max(RectTemp.left, Rect->right - dXMargin);
               RectTemp.top = Rect->top + dYMargin;
               RectTemp.bottom = max(RectTemp.top, Rect->bottom - dYMargin);

/*
               RectTemp.bottom = RectTemp.top +
						               DrawText(hDC, Data, -1, &RectTemp,
											(Just & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM)) |
											DT_CALCRECT | (lpPFormat ? DT_EXTERNALLEADING : 0));
*/
               SSx_DrawText(hWnd, hDC, lpPFormat, Data, -1, &RectTemp,
                            &RectOrig, (Just & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM)) |
                            DT_CALCRECT, Font, CellType->Style);
               if (Just & DT_BOTTOM)
                  RectTemp.top = max(Rect->top + max((Rect->bottom -
                                     Rect->top) - (RectTemp.bottom -
                                     RectTemp.top), 0), Rect->top +
                                     dYMargin);
               else if (Just & DT_VCENTER)
                  RectTemp.top = max(Rect->top + max((Rect->bottom -
                                     Rect->top) - (RectTemp.bottom -
                                     RectTemp.top), 0) / 2,
                                     Rect->top + dYMargin);

               RectTemp.bottom = Rect->bottom;
               RectTemp.right = Rect->right - dXMargin;

               Just &= ~(DT_BOTTOM | DT_VCENTER);

               SSx_DrawText(hWnd, hDC, lpPFormat, Data, -1, &RectTemp,
                            &RectOrig, Just, Font, CellType->Style);
               }

            else
//-------------Commented by Dean 2000/03/16------------
//#endif
//-----------------------------------------------------
               {
               CopyRect(&RectTemp, Rect);

#ifdef SS_V40
					if (CellType->Style & SSS_TEXTORIENT_MASK)
	               SS_ShrinkRect(&RectTemp, fOverflowToLeft ? 0 : dXMargin, dYMargin);
#endif // SS_V40
						{
						if (Just & DT_RIGHT)
							RectTemp.right -= dXMargin;
						else if (!(Just & DT_CENTER) && !fOverflowToLeft)
							RectTemp.left += dXMargin;

						if (!(Just & DT_VCENTER))
							{
							RectTemp.top += dYMargin;
							RectTemp.bottom -= dYMargin;
							}
						}

               SSx_DrawText(hWnd, hDC, lpPFormat, Data, -1, &RectTemp,
                            &RectOrig, Just, Font, CellType->Style);
               }
            }

         if (!lpPFormat || lpPFormat->pf.fDrawColors)
            SetTextColor(hDC, ForegroundOld);

         SelectObject(hDC, hFontOld);

//         if (lpPFormat)
//            DeleteObject(hFont);
         }

      break;

   case SS_TYPE_PICTURE:
      /**********************
      * Get Cell Background
      **********************/

      hBrush = 0;

      if (fDrawSelBack)
         {
         SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
         if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
            BackgroundActual = ColorTblItem.Color;
         }
      else
         {
         SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                             CellRow, &ColorTblItem, &ColorTblItemFore);

         if (lpSS->LockColor.BackgroundId && SSx_GetLock(lpSS, lpCol, lpRow,
             Cell, CellCol, CellRow, TRUE))
            SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);
         }

      bBackDiff = ColorTblItem.Color != BackgroundActual;

      if ((!lpPFormat || lpPFormat->pf.fDrawColors || fErase) &&
          (ColorTblItem.Color != Background || fErase))
         {
         hBrush = ColorTblItem.hBrush;
         Background = ColorTblItem.Color;
         UnrealizeObject(hBrush);
         }

      {
      HDC hDCDisp = fpGetDC(hWnd);
      SS_PaintPicture(lpSS, hDC, hDCDisp, Rect,
                      lpPFormat ? SS_DFSCREEN2PRINTERX(lpPFormat, 1) : 0.0,
                      lpPFormat ? SS_DFSCREEN2PRINTERY(lpPFormat, 1) : 0.0,
                      CellType->Spec.ViewPict.hPal,
                      (HANDLE)CellType->Spec.ViewPict.hPictName, CellType->Style,
                      Background, hBrush, bBackDiff, IsLastCol, IsLastRow, fSelAlpha);
      ReleaseDC(hWnd, hDCDisp);
      }

      break;

#if (!defined(SS_NOCT_BUTTON) || !defined(SS_NOCT_CHECK))
   case SS_TYPE_BUTTON:
   case SS_TYPE_CHECKBOX:
		{
		COLORREF clrBack = Background;

      RectTemp.left = Rect->left;
      RectTemp.top = Rect->top;
      RectTemp.right = Rect->right;
      RectTemp.bottom = Rect->bottom;

      if (!lpSS->lpBook->fIs95)
         InflateRect(&RectTemp, 1, 1);
      else
         {
         RectTemp.right++;
         RectTemp.bottom++;
         }

      FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol,
                                  CellRow);

      hFont = 0;

      if (Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId))
         {
         if (lpPFormat)
            hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, lpPFormat);
         else
            hFont = Font->hFont;
         }

      fButtonState = 0;

      if (CellType->Type == SS_TYPE_BUTTON &&
          CellType->Spec.Button.ButtonType == SUPERBTN_TYPE_2STATE)
         {
         if (Data && Data[0] == '1')
            fButtonState = 1;
         }

      else if (CellType->Type == SS_TYPE_CHECKBOX)
         {
         if (Data)
            {
            if ((CellType->Style & 0x0F) == BS_3STATE ||
                (CellType->Style & 0x0F) == BS_AUTO3STATE)
               {
               if (Data[0] == '1')
                  fButtonState = 1;
               else if (Data[0] == '2')
                  fButtonState = 2;
               }
            else
               {
               if (!Data[0] || (!Data[1] && Data[0] == '0'))
                  fButtonState = 0;
               else
                  fButtonState = 1;
               }
            }
         }

#ifndef SS_NOCT_BUTTON
#ifdef SPREAD_JPN
      //- JPNFIX0014 - (Masanori Iwasa)
      if (!SS_USESELBAR(lpSS) && CellType->Type == SS_TYPE_BUTTON && (CellCol < lpSS->Col.HeaderCnt || CellRow < lpSS->Row.HeaderCnt ||
#else
      if (CellType->Type == SS_TYPE_BUTTON && (CellCol < lpSS->Col.HeaderCnt || CellRow < lpSS->Row.HeaderCnt ||
#endif
          SS_ShouldButtonBeDrawn(lpSS, CellCol, CellRow, CellType->Type)))
#ifdef SS_V80
		           SS_BtnDraw(hWnd, hDC, &RectTemp, CellType, hFont, fButtonState, TRUE,
                    (BOOL)(lpPFormat ? 1 : 0), &RectOrigNoScale,
                    TRUE, lpSS->lpBook->fIs95, FALSE, FALSE, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#else
         SS_BtnDraw(hWnd, hDC, &RectTemp, CellType, hFont, fButtonState, TRUE,
                    (BOOL)(lpPFormat ? 1 : 0), &RectOrigNoScale,
                    TRUE, lpSS->lpBook->fIs95, FALSE);
#endif
      // BJO 24Feb96 SCS4714 - Begin fix
      else
         {
         if (!fDrawSelBoth)
            SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                                CellRow, &ColorTblItem, &ColorTblItemFore);

         if (fDrawSelBack)
            {
            SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
            if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
               BackgroundActual = ColorTblItem.Color;
            }
         // RFW - 2/20/00 - GIC11209
         else if (lpSS->LockColor.BackgroundId &&
             SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
            SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);

         if (fDrawSelFore)
            SS_GetColorItem(&ColorTblItemFore, lpSS->lpBook->SelColor.ForegroundId);

			clrBack = ColorTblItem.Color;

			// RFW - 6/11/04 - 14358
			if (lpPFormat && !lpPFormat->pf.fDrawColors && fErase)
				{
				CopyRect(&RectTemp, Rect);
				hBrush = GetStockObject(WHITE_BRUSH);
#ifdef SS_V80
					if (lpSS->lpBook->wAppearanceStyle == 2 && (isColHeader || isRowHeader))
					{
						SS_DrawHeaderEnhanced(lpSS, hDC, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
					}
					else if (lpSS->lpBook->wAppearanceStyle == 1 && (isColHeader || isRowHeader) && !isCorner && isVisualStyles && toScreen)
					{
						SS_DrawVisualStyleHeader(lpSS->lpBook, hDC, hWnd, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
					}
					else
#endif
					FillRect(hDC, &RectTemp, hBrush);
				}

         else if ((!lpPFormat || lpPFormat->pf.fDrawColors) &&
                  (ColorTblItem.Color != Background || fErase))
            {
            SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                            ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);

            hBrush = ColorTblItem.hBrush;
            UnrealizeObject(hBrush);
#ifdef SS_V80
					if (lpSS->lpBook->wAppearanceStyle == 2 && (isColHeader || isRowHeader))
					{
						SS_DrawHeaderEnhanced(lpSS, hDC, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
					}
					else if (lpSS->lpBook->wAppearanceStyle == 1 && (isColHeader || isRowHeader) && !isCorner && isVisualStyles && toScreen)
					{
						SS_DrawVisualStyleHeader(lpSS->lpBook, hDC, hWnd, isColHeader, isRowHeader, RectTemp, isColHeader ? colSelected : rowSelected, CellRow == lpSS->lpBook->mouseRow && CellCol == lpSS->lpBook->mouseCol);
					}
					else

				if (fSelAlpha)
					fpAlphaFill(hDC, RectTemp, ColorTblItem.hBrush, SS_TranslateColor(lpSS->lpBook->clrHighlightAlphaBlend), lpSS->lpBook->bHighlightAlphaBlend);
				else
#endif
					FillRect(hDC, &RectTemp, ColorTblItem.hBrush);
            }
         }
      // BJO 24Feb96 SCS4714 - End fix
#endif

#ifndef SS_NOCT_CHECK
      if (CellType->Type == SS_TYPE_CHECKBOX)
         {
			COLORREF clrBackOld;

         hWndCtrl = SS_RetrieveControlhWnd(lpSS, CellCol, CellRow);

			clrBackOld = SetBkColor(hDC, SS_TranslateColor(clrBack));
         if (!lpPFormat || lpPFormat->pf.fDrawColors)
            ForegroundOld = SetTextColor(hDC, SS_TranslateColor(ColorTblItemFore.Color));

#ifdef SS_V80
        SS_CheckBoxDraw(hWnd, hWndCtrl, hDC, Rect, CellType, hFont,
                         fButtonState, (BOOL)(lpPFormat ? 1 : 0),
                         &RectOrigNoScale, FALSE, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#else
         SS_CheckBoxDraw(hWnd, hWndCtrl, hDC, Rect, CellType, hFont,
                         fButtonState, (BOOL)(lpPFormat ? 1 : 0),
                         &RectOrigNoScale, FALSE);
#endif
			SetBkColor(hDC, clrBackOld);
         if (!lpPFormat || lpPFormat->pf.fDrawColors)
            SetTextColor(hDC, ForegroundOld);
         }
#endif

//      if (lpPFormat)
//         DeleteObject(hFont);
		}

      break;
#endif

#ifndef SS_NOCT_COMBO
   case SS_TYPE_COMBOBOX:
      /************************
      * Paint Cell Background
      ************************/

      hBrush = ColorTblItem.hBrush;
      if (!fDrawSelBoth)
         SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                             CellRow, &ColorTblItem, &ColorTblItemFore);

      if (fDrawSelBack)
         {
         SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
         if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
            BackgroundActual = ColorTblItem.Color;
         }
      else if (lpSS->LockColor.BackgroundId &&
          SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
         SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);

		// RFW - 6/11/04 - 14358
		if (lpPFormat && !lpPFormat->pf.fDrawColors && fErase)
			{
			CopyRect(&RectTemp, Rect);
			hBrush = GetStockObject(WHITE_BRUSH);
			FillRect(hDC, &RectTemp, hBrush);
			}

      else if ((!lpPFormat || lpPFormat->pf.fDrawColors) &&
               (ColorTblItem.Color != Background || fErase))
         {
         RECT RectTemp;

         SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                         ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);

         hBrush = ColorTblItem.hBrush;
         UnrealizeObject(hBrush);

#ifdef S_V80
			CopyRect(&RectAlphaFill, &RectTemp);
			if (fSelAlpha)
				fpAlphaFill(hDC, RectTemp, ColorTblItem.hBrush, SS_TranslateColor(lpSS->lpBook->clrHighlightAlphaBlend), lpSS->lpBook->bHighlightAlphaBlend);
			else
#endif
	         FillRect(hDC, &RectTemp, ColorTblItem.hBrush);
         }

      if (Data)
         {
         UINT uiFlags = 0;

         FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol,
                                     CellRow);
         Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);

         if (lpPFormat)
            hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, lpPFormat);
         else
            hFont = Font->hFont;

         hFontOld = SelectObject(hDC, hFont);

         if (fDrawSelFore)
            SS_GetColorItem(&ColorTblItemFore, lpSS->lpBook->SelColor.ForegroundId);
         else if (lpSS->LockColor.ForegroundId &&
                  SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
            SS_GetColorItem(&ColorTblItemFore,
                            lpSS->LockColor.ForegroundId);

         if (!lpPFormat || lpPFormat->pf.fDrawColors)
            ForegroundOld = SetTextColor(hDC, SS_TranslateColor(ColorTblItemFore.Color));

         _fmemcpy(&RectTemp, Rect, sizeof(RECT));

         if (!lpPFormat && SS_ShouldButtonBeDrawn(lpSS, CellCol, CellRow, CellType->Type))
            {
            RectTemp.right -= lpSS->lpBook->dComboButtonBitmapWidth;
            if (!lpSS->lpBook->fIs95)
               RectTemp.right++;
            }

         RectTemp.left += dXMargin;
         RectTemp.top += dYMargin;

         if (CellType->Style & SSS_ALIGN_CENTER)
            uiFlags |= DT_CENTER;
         else if (CellType->Style & SSS_ALIGN_RIGHT)
            uiFlags |= DT_RIGHT;
         else
            uiFlags |= DT_LEFT;

         if (CellType->Style & SSS_ALIGN_VCENTER)
            uiFlags |= DT_VCENTER;
         else if (CellType->Style & SSS_ALIGN_BOTTOM)
            uiFlags |= DT_BOTTOM;
         else
            uiFlags |= DT_TOP;

         DrawText(hDC, Data, -1, &RectTemp, uiFlags | DT_SINGLELINE |
                  DT_NOPREFIX);

         if (!lpPFormat || lpPFormat->pf.fDrawColors)
            SetTextColor(hDC, ForegroundOld);

         //SelectObject(hDC, hFontOld); //#99918159 Leon 20101104

//         if (lpPFormat)
//            DeleteObject(hFont);
         }

      /**************
      * Draw Button
      **************/

#ifdef SPREAD_JPN
      //- JPNFIX0014 - (Masanori Iwasa)
      if  ( (!SS_USESELBAR(lpSS) && !lpPFormat && SS_ShouldButtonBeDrawn(lpSS, CellCol, CellRow, CellType->Type)))
	  // SP25_009 Zhang HaoNan 1998/4/23
      //      || (lpSS->wOpMode == SS_OPMODE_ROWMODE && !lpPFormat && SS_ShouldButtonBeDrawn(lpSS, CellCol, CellRow, CellType->Type)))
#else
      if (!lpPFormat && SS_ShouldButtonBeDrawn(lpSS, CellCol, CellRow, CellType->Type))
#endif
#ifdef SS_V80
            SS_DrawComboBtn(hWnd, lpSS, hDC, Rect,
                            lpSS->lpBook->dComboButtonBitmapWidth, FALSE, FALSE, (lpSS->lpBook->wAppearanceStyle != 0 && lpSS->lpBook->wUseVisualStyles != 2) || (lpSS->lpBook->wAppearanceStyle == 0 && lpSS->lpBook->wUseVisualStyles == 1));
#else
		  SS_DrawComboBtn(hWnd, lpSS, hDC, Rect,
                            lpSS->lpBook->dComboButtonBitmapWidth, FALSE);

#endif
			SelectObject(hDC, hFontOld); //#99918159 Leon 20101104
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
      if (CellType->Type == SS_TYPE_EDIT && (CellType->Style & ES_RIGHT) &&
          !(CellType->Style & ES_MULTILINE))
         Just = DT_RIGHT | DT_SINGLELINE;
      else if (CellType->Type == SS_TYPE_EDIT &&
              (CellType->Style & ES_MULTILINE))
         {
         if (CellType->Style & ES_RIGHT)
            Just = DT_RIGHT | DT_WORDBREAK;
         else if (CellType->Style & ES_CENTER)
            Just = DT_CENTER | DT_WORDBREAK;
         else
            Just = DT_LEFT | DT_WORDBREAK;
         }
      else if (CellType->Style & ES_RIGHT)
         Just = DT_RIGHT | DT_SINGLELINE;
      else if (CellType->Style & ES_CENTER)
         Just = DT_CENTER | DT_SINGLELINE;
      else if (CellType->Style & ES_LEFTALIGN)
         Just = DT_LEFT | DT_SINGLELINE;
      else if (CellType->Type == SS_TYPE_INTEGER ||
               SS_IsCellTypeFloatNum(CellType))
         Just = DT_RIGHT | DT_SINGLELINE;
      else
         Just = DT_LEFT | DT_SINGLELINE;

      if (CellType->Style & SSS_ALIGN_BOTTOM)
         Just |= DT_BOTTOM;
      else if (CellType->Style & SSS_ALIGN_VCENTER)
         Just |= DT_VCENTER;
      else
         Just |= DT_TOP;

      Just |= DT_NOPREFIX;

#ifdef SS_V35
#ifdef SS_V40
		if (!(CellType->Style & SSS_TEXTORIENT_MASK))
#endif // SS_V40
	      if ((CellType->Style & SSS_ELLIPSES) && 
	          (!((Just & DT_CENTER) || (Just & DT_RIGHT))))
	         Just |= DTX_ELLIPSES;  
#endif

      if (fAllowCellOverflow)
         {
         if (CellType->Type != SS_TYPE_EDIT ||
             !(CellType->Style & ES_MULTILINE))
// BJO 19Jun96 GRB1964, GRB1965 - removed Bug-011 fix
//#ifdef  BUGS
//// Bug-011
//            if ( !lpPFormat ||                                          // for display
//              // if overflow cell on the right side of block.
//                 (lpPFormat && lpPFormat->pf.x2CellRange > CellCol) ||  
//                 lpPFormat->pf.x2CellRange == 0 )                       // for non-block mode

               if (SS_CalcOverflowRect(hDC, lpSS, Data, CellCol, Just,
                                       Rect, lpCellOverflowRectPrev,
                                       lpCellOverflowRectNext, Rect))
                  fErase = TRUE;
         }

      /************************
      * Paint Cell Background
      ************************/

      if (lpPFormat && !lpPFormat->pf.fDrawColors && fErase)
         {
			CopyRect(&RectTemp, Rect);
			// RFW - 10/21/02 - The following line was added for the case where
			// postscript drivers draw lines as 2 pixels instead of 1.
			if (lpPFormat->wAction != SS_PRINT_ACTION_PREVIEW)
				RectTemp.left++;
         hBrush = GetStockObject(WHITE_BRUSH);
         FillRect(hDC, &RectTemp, hBrush);
         }

      else if (!lpPFormat || lpPFormat->pf.fDrawColors || fErase)
         {
         hBrush = ColorTblItem.hBrush;

         if (!fDrawSelBoth)
            SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                                CellRow, &ColorTblItem, &ColorTblItemFore);

         if (fDrawSelBack)
            {
            SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
            if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
               BackgroundActual = ColorTblItem.Color;
            }
         else if (lpSS->LockColor.BackgroundId &&
                  SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
            SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);

         if (ColorTblItem.Color != Background || fErase ||
             (CellType->Type == SS_TYPE_EDIT &&
              (CellType->Style & ES_PASSWORD) &&
              !(CellType->Style & ES_MULTILINE)))
            {
            RECT RectTemp;

				// RFW - 7/21/03 - 11820
            //SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
            //                ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);
				/* RFW - 4/17/09 - 25434
            SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                            !lpCellOverflowRectPrev && ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);
				*/
            SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                            ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);

            hBrush = ColorTblItem.hBrush;
            UnrealizeObject(hBrush);

#ifdef SS_V80
				CopyRect(&RectAlphaFill, &RectTemp);
				if (fSelAlpha)
					fpAlphaFill(hDC, RectTemp, ColorTblItem.hBrush, SS_TranslateColor(lpSS->lpBook->clrHighlightAlphaBlend), lpSS->lpBook->bHighlightAlphaBlend);
				else
#endif
					FillRect(hDC, &RectTemp, ColorTblItem.hBrush);
            }
         }

      if (Data)
         {
			double dfValue = 0.0;
         TCHAR  Buffer[330];

         FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol, CellRow);
         Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);

         if (lpPFormat)
            hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, lpPFormat);
         else
            hFont = Font->hFont;

         if (SSData.bDataType == SS_TYPE_FLOAT)
            {
				dfValue = SSData.Data.dfValue;

            _fmemset(Buffer, '\0', sizeof(Buffer));
            SS_FloatFormat(lpSS, CellType, dfValue, Buffer, FALSE);
            Data = Buffer;
            }

         else if (SS_IsCellTypeFloatNum(CellType))
            {
            if (*Data)
               {
               _fmemset(Buffer, '\0', sizeof(Buffer));
               SS_StringToNum(lpSS, CellType, Data, &dfValue);
#ifdef SS_V40
					if (CellType->Type == SS_TYPE_PERCENT)
						dfValue *= 100.0;
#endif // SS_V40
               SS_FloatFormat(lpSS, CellType, dfValue, Buffer, FALSE);

               Data = Buffer;
               }
            }

         else if (CellType->Type == SS_TYPE_EDIT &&
                  (CellType->Style & ES_PASSWORD) &&
                  !(CellType->Style & ES_MULTILINE))
            {
            for (i = 0; i < lstrlen(Data) && i < 100; i++)
               Buffer[i] = '*';

            Buffer[lstrlen(Data)] = '\0';

            Data = Buffer;
            }

#ifdef SS_V40
         else if (CellType->Type == SS_TYPE_INTEGER)
            {
	         if (SSData.bDataType == SS_TYPE_INTEGER)
					dfValue = (double)SSData.Data.lValue;
				else if (Data)
//GAB 12/10/01 Changed for unicode
//					dfValue = (double)atoi(Data);
					dfValue = (double)_ttoi(Data);
				}
#endif // SS_V40

         hFontOld = SelectObject(hDC, hFont);

         if (!lpPFormat || lpPFormat->pf.fDrawColors)
				{
				if (fDrawSelFore)
					SS_GetColorItem(&ColorTblItemFore, lpSS->lpBook->SelColor.ForegroundId);
				else if (lpSS->LockColor.ForegroundId &&
							SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
					SS_GetColorItem(&ColorTblItemFore,
										 lpSS->LockColor.ForegroundId);
#ifdef SS_V40
				else if (dfValue < 0.0 &&
							(CellType->Type == SS_TYPE_CURRENCY || CellType->Type == SS_TYPE_NUMBER ||
							 CellType->Type == SS_TYPE_PERCENT || CellType->Type == SS_TYPE_FLOAT ||
#ifdef SS_V70
				          CellType->Type == SS_TYPE_SCIENTIFIC ||
#endif // SS_V70
							 CellType->Type == SS_TYPE_INTEGER))
					if (CellType->Style & SSS_NEGRED)
						ColorTblItemFore.Color = RGB(255, 0, 0);
#endif // SS_V40

				ForegroundOld = SetTextColor(hDC, SS_TranslateColor(ColorTblItemFore.Color));
				}

			CopyRect(&RectTemp, Rect);

         if (CellType->Type == SS_TYPE_EDIT && (CellType->Style & ES_MULTILINE))
            {
            RectTemp.left += SS_SCREEN2PRINTERX(lpPFormat, 2);
            RectTemp.top += SS_SCREEN2PRINTERY(lpPFormat, 1);
            RectTemp.right -= SS_SCREEN2PRINTERX(lpPFormat, 2);
            RectTemp.bottom -= SS_SCREEN2PRINTERY(lpPFormat, 1);
            }

         else
            {
            if (Just & DT_RIGHT)
               RectTemp.right -= dXMargin;
            else if (!(Just & DT_CENTER))
               RectTemp.left += dXMargin;

            RectTemp.top += dYMargin;
            }

         SSx_DrawText(hWnd, hDC, lpPFormat, Data, -1, &RectTemp, &RectOrig,
                      (short)(Just | DT_NOPREFIX), Font, CellType->Style);

         if (!lpPFormat || lpPFormat->pf.fDrawColors)
            SetTextColor(hDC, ForegroundOld);

         SelectObject(hDC, hFontOld);
         }
      break;

#ifndef SS_NOCT_OWNERDRAW
   case SS_TYPE_OWNERDRAW:
      {
      SS_DRAWITEMSTRUCT DrawItem;
      int               iSaveDC = 0;

      if (!fDrawSelBoth)
         SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                             CellRow, &ColorTblItem, &ColorTblItemFore);

      if (fDrawSelBack)
         {
         SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
         if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
            BackgroundActual = ColorTblItem.Color;
         }

		/* RFW - 11/12/07 - OwnerDraw ignored PrintColor
      if (ColorTblItem.Color != Background || fErase)
		*/
		if (lpPFormat && !lpPFormat->pf.fDrawColors && fErase)
			{
			CopyRect(&RectTemp, Rect);
			hBrush = GetStockObject(WHITE_BRUSH);
			FillRect(hDC, &RectTemp, hBrush);
			}

      else if ((!lpPFormat || lpPFormat->pf.fDrawColors) &&
               (ColorTblItem.Color != Background || fErase))
         {
         RECT RectTemp;
         hBrush = ColorTblItem.hBrush;
         UnrealizeObject(hBrush);

         SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                         ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);

#ifdef SS_V80
			CopyRect(&RectAlphaFill, &RectTemp);
			if (fSelAlpha)
				fpAlphaFill(hDC, RectTemp, ColorTblItem.hBrush, SS_TranslateColor(lpSS->lpBook->clrHighlightAlphaBlend), lpSS->lpBook->bHighlightAlphaBlend);
			else
#endif
	         FillRect(hDC, &RectTemp, ColorTblItem.hBrush);
         }

      FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol,
                                  CellRow);
      Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);

      if (lpPFormat)
         hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, lpPFormat);
      else
         hFont = Font->hFont;

      hFontOld = SelectObject(hDC, hFont);

      if (fDrawSelFore)
         SS_GetColorItem(&ColorTblItemFore, lpSS->lpBook->SelColor.ForegroundId);
      else if (lpSS->LockColor.ForegroundId &&
               SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
         SS_GetColorItem(&ColorTblItemFore, lpSS->LockColor.ForegroundId);

      if (!lpPFormat || lpPFormat->pf.fDrawColors)
         ForegroundOld = SetTextColor(hDC, SS_TranslateColor(ColorTblItemFore.Color));

      DrawItem.CtlID = GetWindowID(hWnd);
      DrawItem.Col = CellCol;
      DrawItem.Row = CellRow;
      DrawItem.hWnd = hWnd;
      DrawItem.hDC = hDC;
      DrawItem.lStyle = CellType->Style;
      DrawItem.lpData = Data;
      CopyRect(&DrawItem.Rect, Rect);
		SS_AdjustCellCoordsOut(lpSS, &DrawItem.Col, &DrawItem.Row);

/*
      if (!lpPFormat && lpRectUpdate != NULL)
         {
         HRGN hRgn;

         iSaveDC = SaveDC(hDC);
         IntersectRect(&RectTemp, Rect, lpRectUpdate);
         hRgn = CreateRectRgnIndirect(&RectTemp);
         SelectClipRgn(hDC, hRgn);
         DeleteObject(hRgn);
         }
*/

      iSaveDC = SaveDC(hDC);
      if (!lpPFormat && lpRectUpdate != NULL)
         IntersectRect(&RectTemp, Rect, lpRectUpdate);
      else
         CopyRect(&RectTemp, Rect);

      IntersectClipRect(hDC, RectTemp.left, RectTemp.top, RectTemp.right,
                        RectTemp.bottom);

      SS_SendMsg(lpSS->lpBook, lpSS, SSM_DRAWITEM, 0, (LPARAM)(LPSS_DRAWITEMSTRUCT)&DrawItem);

      if (iSaveDC)
         {
//         SelectClipRgn(hDC, 0);
         RestoreDC(hDC, iSaveDC);
         }

      if (!lpPFormat || lpPFormat->pf.fDrawColors)
         SetTextColor(hDC, SS_TranslateColor(ForegroundOld));

      SelectObject(hDC, hFontOld);

//      if (lpPFormat)
//         DeleteObject(hFont);
      }

      break;
#endif
#ifdef SS_V80
   case SS_TYPE_CUSTOM:
      {
        int iSaveDC;
        RECT CellRect;
        LPSS_CELLTYPE CellTypeTemp = SS_CT_LockCellType(lpSS, CellCol, CellRow);

        iSaveDC = SaveDC(hDC);
        if (fAllowCellOverflow)
         {
         Just = SS_RetrieveJust((short)CellType->Style);
         if (SS_CalcOverflowRect(hDC, lpSS, Data, CellCol, Just, Rect,
                                    lpCellOverflowRectPrev,
                                    lpCellOverflowRectNext, Rect))
			{
            fErase = TRUE;
			SS_CopyCellRect(lpSS, &CellRect, lpCellOverflowRectNext, lpPFormat, FALSE, IsLastCol, IsLastRow);
			}
		 else
			SS_CopyCellRect(lpSS, &CellRect, Rect, lpPFormat, FALSE, IsLastCol, IsLastRow);
         }
		else
		
			SS_CopyCellRect(lpSS, &CellRect, Rect, lpPFormat, FALSE, IsLastCol, IsLastRow);

        if (!lpPFormat && lpRectUpdate != NULL)
           IntersectRect(&RectTemp, &CellRect, lpRectUpdate);
        else
           CopyRect(&RectTemp, &CellRect);
        IntersectClipRect(hDC, RectTemp.left, RectTemp.top, RectTemp.right, RectTemp.bottom);
        if( !SS_CT_PaintCell(lpSS, CellTypeTemp, hDC, lpPFormat, &CellRect, CellCol, CellRow, fCellSelected) )
           SS_CT_Render(lpSS, CellTypeTemp, hDC, lpPFormat, &CellRect, &RectOrigNoScale, CellCol, CellRow, fCellSelected);
        SS_CT_UnlockCellType(lpSS, CellCol, CellRow);
        if (iSaveDC)
           RestoreDC(hDC, iSaveDC);
      }
#endif // SS_V80

#ifdef SS_V80R
   case SS_TYPE_CUSTOM:
      {
      SS_CCT_PAINTCELLSTRUCT PaintCell;
      int                    iSaveDC = 0;

      if (!fDrawSelBoth)
         SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                             CellRow, &ColorTblItem, &ColorTblItemFore);

      if (fDrawSelBack)
         {
         SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
         if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
            BackgroundActual = ColorTblItem.Color;
         }

		if (lpPFormat && !lpPFormat->pf.fDrawColors && fErase)
			{
			CopyRect(&RectTemp, Rect);
			hBrush = GetStockObject(WHITE_BRUSH);
			FillRect(hDC, &RectTemp, hBrush);
			}

      else if ((!lpPFormat || lpPFormat->pf.fDrawColors) &&
               (ColorTblItem.Color != Background || fErase))
         {
         RECT RectTemp;
         hBrush = ColorTblItem.hBrush;
         UnrealizeObject(hBrush);

         SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                         ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);

         FillRect(hDC, &RectTemp, ColorTblItem.hBrush);
         }

      FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol, CellRow);
      Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);

      if (lpPFormat)
         hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, lpPFormat);
      else
         hFont = Font->hFont;

      hFontOld = SelectObject(hDC, hFont);

      if (fDrawSelFore)
         SS_GetColorItem(&ColorTblItemFore, lpSS->lpBook->SelColor.ForegroundId);
      else if (lpSS->LockColor.ForegroundId &&
               SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
         SS_GetColorItem(&ColorTblItemFore, lpSS->LockColor.ForegroundId);

      if (!lpPFormat || lpPFormat->pf.fDrawColors)
         ForegroundOld = SetTextColor(hDC, SS_TranslateColor(ColorTblItemFore.Color));

      PaintCell.CtlID = GetWindowID(hWnd);
      PaintCell.Col = CellCol;
      PaintCell.Row = CellRow;
      PaintCell.hWnd = hWnd;
      PaintCell.hDC = hDC;
      PaintCell.lStyle = CellType->Style;
      PaintCell.lpData = Data;
      CopyRect(&PaintCell.Rect, Rect);
		SS_AdjustCellCoordsOut(lpSS, &PaintCell.Col, &PaintCell.Row);

      iSaveDC = SaveDC(hDC);
      if (!lpPFormat && lpRectUpdate != NULL)
         IntersectRect(&RectTemp, Rect, lpRectUpdate);
      else
         CopyRect(&RectTemp, Rect);

      IntersectClipRect(hDC, RectTemp.left, RectTemp.top, RectTemp.right, RectTemp.bottom);

      SS_SendMsg(lpSS->lpBook, lpSS, SSM_DRAWITEM, 0, (LPARAM)(LPSS_DRAWITEMSTRUCT)&PaintCell);

      if (iSaveDC)
         RestoreDC(hDC, iSaveDC);

      if (!lpPFormat || lpPFormat->pf.fDrawColors)
         SetTextColor(hDC, SS_TranslateColor(ForegroundOld));

      SelectObject(hDC, hFontOld);
      }

      break;
#endif // SS_V80

#ifdef SS_V80R
   case SS_TYPE_CUSTOM:
      {
      SS_CCT_PAINTCELLSTRUCT PaintCell;
      int                    iSaveDC = 0;

      if (!fDrawSelBoth)
         SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                             CellRow, &ColorTblItem, &ColorTblItemFore);

      if (fDrawSelBack)
         {
         SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
         if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
            BackgroundActual = ColorTblItem.Color;
         }

		if (lpPFormat && !lpPFormat->pf.fDrawColors && fErase)
			{
			CopyRect(&RectTemp, Rect);
			hBrush = GetStockObject(WHITE_BRUSH);
			FillRect(hDC, &RectTemp, hBrush);
			}

      else if ((!lpPFormat || lpPFormat->pf.fDrawColors) &&
               (ColorTblItem.Color != Background || fErase))
         {
         RECT RectTemp;
         hBrush = ColorTblItem.hBrush;
         UnrealizeObject(hBrush);

         SS_CopyCellRect(lpSS, &RectTemp, Rect, lpPFormat,
                         ColorTblItem.Color != BackgroundActual, IsLastCol, IsLastRow);

         FillRect(hDC, &RectTemp, ColorTblItem.hBrush);
         }

      FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol, CellRow);
      Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);

      if (lpPFormat)
         hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, lpPFormat);
      else
         hFont = Font->hFont;

      hFontOld = SelectObject(hDC, hFont);

      if (fDrawSelFore)
         SS_GetColorItem(&ColorTblItemFore, lpSS->lpBook->SelColor.ForegroundId);
      else if (lpSS->LockColor.ForegroundId &&
               SSx_GetLock(lpSS, lpCol, lpRow, Cell, CellCol, CellRow, TRUE))
         SS_GetColorItem(&ColorTblItemFore, lpSS->LockColor.ForegroundId);

      if (!lpPFormat || lpPFormat->pf.fDrawColors)
         ForegroundOld = SetTextColor(hDC, SS_TranslateColor(ColorTblItemFore.Color));

      PaintCell.CtlID = GetWindowID(hWnd);
      PaintCell.Col = CellCol;
      PaintCell.Row = CellRow;
      PaintCell.hWnd = hWnd;
      PaintCell.hDC = hDC;
      PaintCell.lStyle = CellType->Style;
      PaintCell.lpData = Data;
      CopyRect(&PaintCell.Rect, Rect);
		SS_AdjustCellCoordsOut(lpSS, &PaintCell.Col, &PaintCell.Row);

      iSaveDC = SaveDC(hDC);
      if (!lpPFormat && lpRectUpdate != NULL)
         IntersectRect(&RectTemp, Rect, lpRectUpdate);
      else
         CopyRect(&RectTemp, Rect);

      IntersectClipRect(hDC, RectTemp.left, RectTemp.top, RectTemp.right, RectTemp.bottom);

      SS_SendMsg(lpSS->lpBook, lpSS, SSM_DRAWITEM, 0, (LPARAM)(LPSS_DRAWITEMSTRUCT)&PaintCell);

      if (iSaveDC)
         RestoreDC(hDC, iSaveDC);

      if (!lpPFormat || lpPFormat->pf.fDrawColors)
         SetTextColor(hDC, SS_TranslateColor(ForegroundOld));

      SelectObject(hDC, hFontOld);
      }

      break;
#endif // SS_V80

#if 0
   case SS_TYPE_CUSTOM:
      if (Data)
         {
         FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, Cell, CellCol,
                                     CellRow);
         Font = SS_GetFont(lpSS, &FontTemp, FontId);
         SendMessage(hWndCtrl, WM_SETFONT, Font->hFont, 1L);

         SendMessage(hWndCtrl, WM_SETTEXT, 0, (long)Data);
         /*
         SetWindowText(hWndCtrl, Data);
         */
         MoveWindow(hWndCtrl, Rect->left, Rect->top, Rect->right -
                    Rect->left, Rect->bottom - Rect->top, TRUE);

         lpSS->NoEraseBkgnd = TRUE;
         ShowWindow(hWndCtrl, SW_SHOWNA);
         UpdateWindow(hWndCtrl);
         lpSS->NoEraseBkgnd = TRUE;
         ShowWindow(hWndCtrl, SW_HIDE);
         ValidateRect(hWnd, NULL);
         lpSS->NoEraseBkgnd = FALSE;
         }

      else
         {
         SSx_GetColorTblItem(lpSS, lpCol, lpRow, Cell, CellCol,
                             CellRow, &ColorTblItem, NULL);

         if (ColorTblItem.Color != Background || fErase)
            FillRect(hDC, Rect, ColorTblItem.hBrush);
         }

      break;
#endif
   }

if (hGlobalData)
   tbGlobalUnlock(hGlobalData);

SSx_UnlockCellItem(lpSS, lpRow, CellCol, CellRow);

lpSS->lpBook->CurVisCell.Row = CurVisCellRow;
lpSS->lpBook->CurVisCell.Col = CurVisCellCol;

#ifdef SS_V80
if (fSelAlpha2)
	SS_AlphaBlendSelection(lpSS->lpBook, hDC, &RectAlphaFill);
#endif

return (TRUE);
}


short SS_RetrieveJust(Justification)

short Justification;
{
short Just;

if (Justification & SS_TEXT_WORDWRAP)
   Just = DT_WORDBREAK;
else
   Just = DT_SINGLELINE;

if (!(Justification & SS_TEXT_PREFIX))
   Just |= DT_NOPREFIX;

/*************************************
* Determine Horizontal justification
*************************************/

if (Justification & SS_TEXT_RIGHT)
   Just |= DT_RIGHT;

else if (Justification & SS_TEXT_CENTER)
   Just |= DT_CENTER;

else
   Just |= DT_LEFT;

/***********************************
* Determine Vertical justification
***********************************/

if (Justification & SS_TEXT_BOTTOM)
   Just |= DT_BOTTOM;

else if (Justification & SS_TEXT_VCENTER)
   Just |= DT_VCENTER;

else
   Just |= DT_TOP;

return (Just);
}


static short SS_RetrieveVertTextJust(short Justification)
{
short Just = 0;

if (Justification & SS_TEXT_WORDWRAP)
   Just |= DT_WORDBREAK;

/*************************************
* Determine Horizontal justification
*************************************/

if (Justification & SS_TEXT_RIGHT)
   Just |= DT_TOP;

else if (Justification & SS_TEXT_CENTER)
   Just |= DT_VCENTER | DT_SINGLELINE;

else
   Just |= DT_BOTTOM | DT_SINGLELINE;

/***********************************
* Determine Vertical justification
***********************************/

if (Justification & SS_TEXT_BOTTOM)
   Just |= DT_RIGHT;

else if (Justification & SS_TEXT_VCENTER)
   Just |= DT_CENTER;

else
   Just |= DT_LEFT;

return (Just);
}


static void SS_DrawShadows(hDC, lpRect, iShadowSize, hBrushShadow,
                           hBrushHighlight, fNoLeft)

HDC    hDC;
LPRECT lpRect;
short  iShadowSize;
HBRUSH hBrushShadow;
HBRUSH hBrushHighlight;
BOOL   fNoLeft;
{
HBRUSH hBrushOld;
short  i;

/**********************
* Create left and top
**********************/

hBrushOld = SelectObject(hDC, hBrushHighlight);

/*******
* Left
*******/

if (!fNoLeft)
   for (i = 0; i < iShadowSize; i++)
      PatBlt(hDC, lpRect->left + i, lpRect->top, 1, lpRect->bottom -
             lpRect->top - i, PATCOPY);

/******
* Top
******/

for (i = 0; i < iShadowSize; i++)
   PatBlt(hDC, lpRect->left, lpRect->top + i, lpRect->right -
          lpRect->left - i, 1, PATCOPY);

SelectObject(hDC, hBrushOld);

/**************************
* Create right and bottom
**************************/

hBrushOld = SelectObject(hDC, hBrushShadow);

/********
* Right
********/

for (i = 0; i < iShadowSize; i++)
   PatBlt(hDC, lpRect->right - i - 1, lpRect->top + i, 1, lpRect->bottom -
          lpRect->top - i, PATCOPY);

/*********
* Bottom
*********/

for (i = 0; i < iShadowSize; i++)
   PatBlt(hDC, lpRect->left + i, lpRect->bottom - i - 1, lpRect->right -
          lpRect->left - i, 1, PATCOPY);

SelectObject(hDC, hBrushOld);
}


void SS_ShrinkRect(Rect, x, y)

LPRECT Rect;
int    x;
int    y;
{
Rect->left += x;
Rect->top += y;
Rect->right -= x;
Rect->bottom -= y;
}


BOOL SS_GetColorTblItem(lpSS, Col, Row, BackColorTblItem, ForeColorTblItem)

LPSPREADSHEET     lpSS;
SS_COORD          Col;
SS_COORD          Row;
LPSS_COLORTBLITEM BackColorTblItem;
LPSS_COLORTBLITEM ForeColorTblItem;
{
LPSS_ROW          lpRow;
LPSS_COL          lpCol;
BOOL              fRet;

lpCol = SS_LockColItem(lpSS, Col);
lpRow = SS_LockRowItem(lpSS, Row);

fRet = SSx_GetColorTblItem(lpSS, lpCol, lpRow, NULL, Col, Row,
                           BackColorTblItem, ForeColorTblItem);

SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);

return (fRet);
}


BOOL SSx_GetColorTblItem(lpSS, lpCol, lpRow, lpCell, Col, Row,
                         BackColorTblItem, ForeColorTblItem)

LPSPREADSHEET     lpSS;
LPSS_COL          lpCol;
LPSS_ROW          lpRow;
LPSS_CELL         lpCell;
SS_COORD          Col;
SS_COORD          Row;
LPSS_COLORTBLITEM BackColorTblItem;
LPSS_COLORTBLITEM ForeColorTblItem;
{
LPSS_CELL         lpCellOrig;
SS_COLORID        BackgroundIdSave;
SS_COLORID        BackgroundId;
SS_COLORID        ForegroundId;

lpCellOrig = lpCell;

BackgroundId = lpSS->Color.BackgroundId;
ForegroundId = lpSS->Color.ForegroundId;

#ifdef SS_V30
if (Row >= lpSS->Row.HeaderCnt)
   {
   if ((Row - lpSS->Row.HeaderCnt + 1) % 2) // Odd
      {
      if (lpSS->OddRowColor.BackgroundId)
          BackgroundId = lpSS->OddRowColor.BackgroundId;
      if (lpSS->OddRowColor.ForegroundId)
          ForegroundId = lpSS->OddRowColor.ForegroundId;
      }
   else         // Even
      {
      if (lpSS->EvenRowColor.BackgroundId)
          BackgroundId = lpSS->EvenRowColor.BackgroundId;
      if (lpSS->EvenRowColor.ForegroundId)
          ForegroundId = lpSS->EvenRowColor.ForegroundId;
      }
   }
#endif

BackgroundIdSave = BackgroundId;

if (Col != -1 && lpCol)
   {
   if (lpCol->Color.BackgroundId != 0)
       BackgroundId = lpCol->Color.BackgroundId;

   if (lpCol->Color.ForegroundId != 0)
       ForegroundId = lpCol->Color.ForegroundId;
   }

if (Row != -1 && lpRow)
   {
   if (lpRow->Color.BackgroundId != 0)
       BackgroundId = lpRow->Color.BackgroundId;

   if (lpRow->Color.ForegroundId != 0)
       ForegroundId = lpRow->Color.ForegroundId;
   }

if (Row != -1 && Col != -1 && (lpCell ||
    (lpCell = SS_LockCellItem(lpSS, Col, Row))))
   {
   if (lpCell->Color.BackgroundId != 0)
      BackgroundId = lpCell->Color.BackgroundId;

   if (lpCell->Color.ForegroundId != 0)
      ForegroundId = lpCell->Color.ForegroundId;

   if (!lpCellOrig)
      SS_UnlockCellItem(lpSS, Col, Row);
   }

if (BackColorTblItem)
   {
   _fmemset(BackColorTblItem, '\0', sizeof(SS_COLORTBLITEM));
   SS_GetColorItem(BackColorTblItem, BackgroundId);
   }

if (ForeColorTblItem)
   {
   _fmemset(ForeColorTblItem, '\0', sizeof(SS_COLORTBLITEM));
   SS_GetColorItem(ForeColorTblItem, ForegroundId);
   }

return (BackgroundIdSave != BackgroundId);
}


SS_COLORID SS_AddColor(COLORREF Color)
{
LPSS_COLORTBLITEM lpColorTable;
SS_COLORID        ColorId = 0;
SS_COLORID        i;

if (SS_ISDEFCOLOR(Color))
   return (0);

if (!SS_ColorTableAlloc())
   return (-1);

lpColorTable = SS_ColorTableLock();

for (i = 0; i < ColorTable.dTableCnt; i++)
   if (lpColorTable[i].Color == Color)
      {
      SS_ColorTableUnlock();
      return (i + 1);
      }

/***************************
* Add Color to Color Table
***************************/

lpColorTable[ColorTable.dTableCnt].Color = Color;
//lpColorTable[ColorTable.dTableCnt].hBrush = CreateSolidBrush(Color);

ColorTable.dTableCnt++;

ColorId = ColorTable.dTableCnt;

SS_ColorTableUnlock();
return (ColorId);
}


BOOL SS_ColorTableAlloc(void)
{
if (ColorTable.dTableCnt >= ColorTable.dTableAllocCnt)
   {
   long lSize;

#ifdef WIN32
   if (!ColorTable.hTable)
      InitializeCriticalSection(&ColorTable.cs);

   EnterCriticalSection(&ColorTable.cs);
#endif

   ColorTable.dTableAllocCnt += 10;
   lSize = ColorTable.dTableAllocCnt * sizeof(SS_COLORTBLITEM);

   if (!ColorTable.hTable)
      ColorTable.hTable = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, lSize);

   else
      ColorTable.hTable = tbGlobalReAlloc(ColorTable.hTable, lSize,
                                          GMEM_MOVEABLE | GMEM_ZEROINIT);

   if (!ColorTable.hTable)
      {
      ColorTable.dTableCnt = 0;
      ColorTable.dTableAllocCnt = 0;
      }

#ifdef WIN32
   LeaveCriticalSection(&ColorTable.cs);

   if (!ColorTable.hTable)
      DeleteCriticalSection(&ColorTable.cs);
#endif

   return (ColorTable.hTable != 0);
   }

return (TRUE);
}


void SS_ColorTableDelete()
{
LPSS_COLORTBLITEM lpColorTable;
long             i;

if (ColorTable.hTable && ColorTable.dTableCnt)
   {
#ifdef WIN32
   EnterCriticalSection(&ColorTable.cs);
#endif
   lpColorTable = (LPSS_COLORTBLITEM)tbGlobalLock(ColorTable.hTable);

   for (i = 0; i < ColorTable.dTableCnt; i++)
      {
      if (lpColorTable[i].hBrush)
         DeleteObject(lpColorTable[i].hBrush);
      }

   tbGlobalUnlock(ColorTable.hTable);
   tbGlobalFree(ColorTable.hTable);

   ColorTable.hTable = 0;
   ColorTable.dTableCnt = 0;
   ColorTable.dTableAllocCnt = 0;
   ColorTable.dTableUseCnt = 0;

#ifdef WIN32
   LeaveCriticalSection(&ColorTable.cs);
   DeleteCriticalSection(&ColorTable.cs);
#endif
   }
}


void SS_ColorTableDeleteBrushes()
{
LPSS_COLORTBLITEM lpColorTable;
short             i;

if (ColorTable.hTable && ColorTable.dTableCnt)
   {
#ifdef WIN32
   EnterCriticalSection(&ColorTable.cs);
#endif
   lpColorTable = (LPSS_COLORTBLITEM)tbGlobalLock(ColorTable.hTable);

   for (i = 0; i < ColorTable.dTableCnt; i++)
      {
      if (lpColorTable[i].hBrush)
			{
         DeleteObject(lpColorTable[i].hBrush);
         lpColorTable[i].hBrush = 0;
			}
      }

   tbGlobalUnlock(ColorTable.hTable);
   ColorTable.dTableUseCnt = 0;

#ifdef WIN32
   LeaveCriticalSection(&ColorTable.cs);
#endif
   }
}


LPSS_COLORTBLITEM SS_ColorTableLock(void)
{
if (!ColorTable.hTable)
   return (NULL);

#ifdef WIN32
EnterCriticalSection(&ColorTable.cs);
#endif

return ((LPSS_COLORTBLITEM)tbGlobalLock(ColorTable.hTable));
}


void SS_ColorTableUnlock(void)
{
if (ColorTable.hTable)
   {
   tbGlobalUnlock(ColorTable.hTable);
#ifdef WIN32
   LeaveCriticalSection(&ColorTable.cs);
#endif
   }
}



LPSS_COLORTBLITEM SS_GetColorItem(LPSS_COLORTBLITEM ColorTblItem, SS_COLORID ColorId)
{
LPSS_COLORTBLITEM lpColorTable;

if (ColorId > 0 && ColorId <= ColorTable.dTableCnt)
   {
   lpColorTable = SS_ColorTableLock();

   #ifdef WIN32
   if (SS_IsEncodedSysColor(lpColorTable[ColorId - 1].Color))
      {
      _fmemcpy(ColorTblItem, &lpColorTable[ColorId - 1], sizeof(SS_COLORTBLITEM));
      ColorTblItem->hBrush = GetSysColorBrush(SS_DecodeSysColor(ColorTblItem->Color));
      }
   else
   #endif
      {
      #ifndef WIN32
      if (SS_IsEncodedSysColor(lpColorTable[ColorId - 1].Color))
         {
         if (lpColorTable[ColorId - 1].hBrush)
            {
            DeleteObject(lpColorTable[ColorId - 1].hBrush);
            lpColorTable[ColorId - 1].hBrush = 0;
            ColorTable.dTableUseCnt--;
            }
         }
      #endif
      if (!lpColorTable[ColorId - 1].hBrush)
         {
         if (ColorTable.dTableUseCnt >= SS_COLORTABLE_USEMAX)
            {
            SS_COLORID oldest = -1;
            SS_COLORID i;
  
            // search for brush to release
            for (i = 0; i < ColorTable.dTableCnt; i++)
               if (lpColorTable[i].hBrush)
                  if (oldest == -1 ||
                      lpColorTable[i].lLastUsed < lpColorTable[oldest].lLastUsed)
                     oldest = i;
  
            // Delete oldest Brush
            if (oldest != -1)
               {
               DeleteObject(lpColorTable[oldest].hBrush);
               lpColorTable[oldest].hBrush = 0;
               ColorTable.dTableUseCnt--;
               }
            }

			// RFW - 11/1/05 - 17337
			{
			COLORREF clr = SS_TranslateColor(lpColorTable[ColorId - 1].Color);
			HDC      hDC = GetDC(NULL);

			clr = GetNearestColor(hDC, clr);
         lpColorTable[ColorId - 1].hBrush = CreateSolidBrush(clr);
			ReleaseDC(NULL, hDC);
			}

         ColorTable.dTableUseCnt++;
         }

      lpColorTable[ColorId - 1].lLastUsed = GetTickCount();

      // The following code handles the case where there can be
      // multiple entries in the table with the same TickCount.
      if ((DWORD)lpColorTable[ColorId - 1].lLastUsed <= TickCountLast)
         {
         TickCountLast++;
         lpColorTable[ColorId - 1].lLastUsed = (long)TickCountLast;
         }
      else
         TickCountLast = lpColorTable[ColorId - 1].lLastUsed;

      _fmemcpy(ColorTblItem, &lpColorTable[ColorId - 1], sizeof(SS_COLORTBLITEM));
      }
   
   SS_ColorTableUnlock();

   return (ColorTblItem);
   }

else
   {
   ColorTblItem->Color = RGBCOLOR_DEFAULT;
   ColorTblItem->hBrush = 0;
   ColorTblItem->lLastUsed = 0;
   }

return (NULL);
}


BOOL SS_Paint(LPSPREADSHEET lpSS, HDC hDC)
{                                           
PAINTSTRUCT Paint;
HDC         hDCOrig;
HDC         hDCSpread;
RECT        Rect;
RECT        RectClientAll;
RECT        RectClient;
LPSS_ROW    lpRow;
SS_COORD    RowAt;
SS_COORD    RowPosAt;
SS_COORD    TempRowAt;
SS_COORD    RowHeaderAt;
short       Height;
BOOL        fKillFocusReceived;
BOOL        fHighlightOnOld;
BOOL        fDrawSelBlock = FALSE;
BOOL        fHideHightlightCell = TRUE;
HWND        hWndCtrl = 0;
HWND        hWnd;

if (!lpSS)
	return (FALSE);

hWnd = lpSS->lpBook->hWnd;

hDCOrig = hDC;

if (lpSS->lpBook->fPaintingToScreen)
   {
   if (!hDCOrig)
      {
      fpBeginPaint(hWnd, (LPPAINTSTRUCT)&Paint);
      EndPaint(hWnd, &Paint);
      }

   return (FALSE);
   }
 
if (lpSS->lpBook->fUpdateAutoSize)
   SS_AutoSize(lpSS->lpBook, FALSE);

lpSS->lpBook->fPaintingToScreen = TRUE;

/********************************************************
* Check to see if the spreadsheet is in virtual mode.
* If so check to see if more data needs to be requested
* from the application
********************************************************/

#ifndef SS_NOVIRTUAL
if (lpSS->fVirtualMode && lpSS->Virtual.VMax != 0 && lpSS->Row.Max &&
    !lpSS->fProcessingVQueryData)
   {
   SS_COORD lBottomCell = SS_GetBottomCell(lpSS, lpSS->Row.UL);

   if (lpSS->Row.UL < lpSS->Virtual.VTop ||
       lBottomCell >= lpSS->Virtual.VTop + lpSS->Virtual.VPhysSize)
      {
      GetUpdateRect(hWnd, &Paint.rcPaint, FALSE);
      SS_VQueryData(lpSS, lBottomCell);
      SS_InvalidateRect(lpSS->lpBook, &Paint.rcPaint, FALSE);
      }
   }
#endif

GetClientRect(hWnd, &RectClientAll);
SS_GetClientRect(lpSS->lpBook, &RectClient);
CopyRect(&Rect, &RectClient);

if (hDCOrig)
   {
#ifdef SS_OCX
   if (lpSS->lpBook->fOnDraw)
      {
      CopyRect(&Paint.rcPaint, &lpSS->lpBook->RectPaint);
      hDCSpread = hDC;
      }
   else
      {
      CopyRect(&Paint.rcPaint, &RectClientAll);
      hDCSpread = GetDC(hWnd);
      }
#else
   CopyRect(&Paint.rcPaint, &RectClientAll);
   hDCSpread = GetDC(hWnd);
#endif
   }
else
   {
   hDC = fpBeginPaint(hWnd, (LPPAINTSTRUCT)&Paint);
   hDCSpread = hDC;
   }

// Note: The actual OCX control window maybe invisible at design time,
//       when OleControl::DoSuperclassPaint() sends WM_PAINT message.
#ifdef SS_OCX
if (lpSS->NoEraseBkgnd || lpSS->NoPaint)
#else
if (lpSS->NoEraseBkgnd || lpSS->NoPaint || (hDCOrig != hDC && !IsWindowVisible(hWnd)) || IsRectEmpty(&Paint.rcPaint))
#endif
   {
   if (lpSS->NoEraseBkgnd)
      lpSS->NoEraseBkgnd = FALSE;
   if (lpSS->NoPaint)
      lpSS->NoPaint = FALSE;
   lpSS->lpBook->fPaintingToScreen = FALSE;

   if (!hDCOrig)
      EndPaint(hWnd, &Paint);

   return (FALSE);
   }

if (lpSS->lpBook->EditModeOn)
   {
   SS_CELLTYPE   CellTypeTemp;
   LPSS_CELLTYPE CellType;

   CellType = SS_RetrieveCellType(lpSS, &CellTypeTemp, NULL,
                                  lpSS->Col.CurAt, lpSS->Row.CurAt);

   if (CellType && CellType->ControlID)
      {
      hWndCtrl = SS_GetControlhWnd(lpSS, CellType->ControlID);
      HideCaret(hWndCtrl);
      }
   }

if (SS_INVERTSEL(lpSS))
   if (GetFocus() == hWnd || lpSS->lpBook->fRetainSelBlock)
      SS_DrawSelBlock(hDC, lpSS);

if (SS_USESINGLESELBAR(lpSS) && lpSS->lpBook->fKillFocusReceived)
   lpSS->fNoDrawFocusRect = TRUE;

// Remove cell highlight

lpSS->fNoDrawFocusRect = FALSE;
fHighlightOnOld = lpSS->HighlightOn;


#ifdef SS_V80
// Check to see if the clipping region is outside of the headers
{
RECT RectNonHeader;

SS_GetScrollArea(lpSS, &RectNonHeader, SS_SCROLLAREA_NONHEADER);
fHideHightlightCell = RectVisible(hDC, &RectNonHeader);
}
#endif // SS_V80

#ifdef SS_OCX
if (TRUE)
#else
if (!hDCOrig)
#endif
   {
	if (fHideHightlightCell)
      {
      if (SS_USESINGLESELBAR(lpSS) && GetFocus() != hWnd && fHighlightOnOld)
         SSx_HighlightCell(lpSS, hDCSpread, FALSE);
      else
         SS_HighlightCell(lpSS, FALSE);
      }
   }

lpSS->fNoDrawFocusRect = FALSE;

#ifdef SS_PAINTTOSCREEN
SSx_Paint(lpSS, hDC, hDC, &Paint.rcPaint);
#else
{
HBITMAP hBitmapOld;
HDC     hDCMem;
HBITMAP hPaintBitmap;
BOOL    fDeletePaintBitmap = FALSE;

if (!PaintBitmap.csInit)
	{
   InitializeCriticalSection(&PaintBitmap.cs);
	PaintBitmap.csInit = TRUE;
	}

EnterCriticalSection(&PaintBitmap.cs);

// If another window is using our bitmap, then create and destroy a temp one.
if (PaintBitmap.fInUse)
	{
   hPaintBitmap = CreateCompatibleBitmap(hDC, RectClientAll.right, RectClientAll.bottom);
	fDeletePaintBitmap = TRUE;
	}

else if (!PaintBitmap.hPaintBitmap || PaintBitmap.cxPaintBitmap < RectClientAll.right ||
	PaintBitmap.cyPaintBitmap < RectClientAll.bottom)
   {
   if (PaintBitmap.hPaintBitmap)
      DeleteObject(PaintBitmap.hPaintBitmap);

   PaintBitmap.cxPaintBitmap = RectClientAll.right;
   PaintBitmap.cyPaintBitmap = RectClientAll.bottom;
   PaintBitmap.hPaintBitmap = CreateCompatibleBitmap(hDC, PaintBitmap.cxPaintBitmap, PaintBitmap.cyPaintBitmap);
	hPaintBitmap = PaintBitmap.hPaintBitmap;
   }

else
	hPaintBitmap = PaintBitmap.hPaintBitmap;

PaintBitmap.fInUse = TRUE;
LeaveCriticalSection(&PaintBitmap.cs);

/*
if (!lpSS->hPaintBitmapDC || lpSS->cxPaintBitmap != RectClientAll.right || lpSS->cyPaintBitmap != RectClientAll.bottom)
   {
   if (lpSS->hPaintBitmapDC)
      DeleteObject(lpSS->hPaintBitmapDC);

   lpSS->cxPaintBitmap = RectClientAll.right;
   lpSS->cyPaintBitmap = RectClientAll.bottom;
   lpSS->hPaintBitmapDC = CreateCompatibleBitmap(hDC, lpSS->cxPaintBitmap, lpSS->cyPaintBitmap);
   }
*/

// Create a memory device context
hDCMem = CreateCompatibleDC(hDC);
hBitmapOld = SelectObject(hDCMem, hPaintBitmap);

SSx_Paint(lpSS, hDCMem, hDC, &Paint.rcPaint);

/*******************************************************
* Copy the memory device context bitmap to the display
*******************************************************/

BitBlt(hDC, Paint.rcPaint.left, Paint.rcPaint.top, Paint.rcPaint.right -
       Paint.rcPaint.left, Paint.rcPaint.bottom - Paint.rcPaint.top,
       hDCMem, Paint.rcPaint.left, Paint.rcPaint.top, SRCCOPY);

/***********
* Clean up
***********/

EnterCriticalSection(&PaintBitmap.cs);
PaintBitmap.fInUse = TRUE;
LeaveCriticalSection(&PaintBitmap.cs);

SelectObject(hDCMem, hBitmapOld);
DeleteDC(hDCMem);
if (fDeletePaintBitmap)
	DeleteObject(hPaintBitmap);
}
#endif // SS_PAINTTOSCREEN

#ifdef SS_OCX
if (TRUE)
#else
if (!hDCOrig)
#endif
   {
#ifndef SS_UTP
   if (!lpSS->lpBook->EditModeOn && (!SS_USESINGLESELBAR(lpSS) ||
       GetFocus() == hWnd))
      {
      if (lpSS->lpBook->fKillFocusReceived && SS_USESINGLESELBAR(lpSS) &&
          GetFocus() == hWnd)
         lpSS->lpBook->fKillFocusReceived = FALSE;
      
//      if (!SS_USESELBAR(lpSS) || SS_INVERTSEL(lpSS))
         SS_HighlightCell(lpSS, TRUE);
      }

   if (SS_IsDestroyed(hWnd))
   {
      if (!hDCOrig)
      	EndPaint(hWnd, &Paint);
      return (FALSE);
    }
#endif

   if (SS_INVERTSEL(lpSS))
      {
      if (lpSS->wOpMode == SS_OPMODE_MULTISEL ||
          lpSS->wOpMode == SS_OPMODE_EXTSEL)
         {
         RowPosAt = SS_GetCellPosY(lpSS, lpSS->Row.HeaderCnt,
                                   lpSS->Row.HeaderCnt);
         RowHeaderAt = lpSS->Row.HeaderCnt;
         RowAt = lpSS->Row.UL;

         do
            {
            if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
               TempRowAt = RowHeaderAt;
            else
               TempRowAt = RowAt;

            Height = SS_GetRowHeightInPixels(lpSS, TempRowAt);

            if (Rect.top + RowPosAt < Paint.rcPaint.bottom &&
                Rect.top + RowPosAt + Height > Paint.rcPaint.top)
               if (lpRow = SS_LockRowItem(lpSS, TempRowAt))
                  {
                  if (lpRow->fRowSelected == TRUE || lpRow->fRowSelected == 3)
                     SS_InvertRowBlock(hWnd, hDC, lpSS, TempRowAt, TempRowAt);

                  SS_UnlockRowItem(lpSS, TempRowAt);
                  }

            if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
               RowHeaderAt++;
            else
               RowAt++;

            RowPosAt += Height;

            } while (RowPosAt < Rect.bottom && TempRowAt <
                     SS_GetRowCnt(lpSS) - 1);
         }

      else if (GetFocus() == hWnd || lpSS->lpBook->fRetainSelBlock)
         SS_DrawSelBlock(hDC, lpSS);

      else if (!lpSS->lpBook->fSelBlockInvisible)
			{
			HWND hWndFocus = GetFocus();
			if (!hWndFocus || GetParent(hWndFocus) != hWnd)
				fDrawSelBlock = TRUE;
			}
      }

   if (SS_USESINGLESELBAR(lpSS))
      {
      if (GetFocus() != hWnd)
         {
         if (!lpSS->FreezeHighlight)
//         if (!lpSS->FreezeHighlight && fHighlightOnOld)
            {
            fKillFocusReceived = lpSS->lpBook->fKillFocusReceived;
            // BJO 21Apr98 GRB363 - Before fix
            //SSx_HighlightCell(lpSS, hDCSpread, TRUE);
            // BJO 21Apr98 GRB363 - Begin fix
            if( fHighlightOnOld )
              SSx_HighlightCell(lpSS, hDCSpread, TRUE);
            else
              SS_HighlightCell(lpSS, TRUE);
            // BJO 21Apr98 GRB363 - End fix

            /*
            if (fKillFocusReceived)
               {
               lpSS->fProcessingKillFocus = TRUE;
               SSx_HighlightCell(lpSS, hDCSpread, FALSE);
               lpSS->fProcessingKillFocus = FALSE;
               }
            */
            }
         }
      else
         lpSS->lpBook->fKillFocusReceived = FALSE;
      }

   else if (!lpSS->lpBook->fEditModePermanent && fHighlightOnOld &&
            !lpSS->HighlightOn && GetFocus() != hWnd)
      {
//      if (!SS_USESELBAR(lpSS) || SS_INVERTSEL(lpSS))
         SSx_HighlightCell(lpSS, hDCSpread, TRUE);
      }

   if (hWndCtrl)
      ShowCaret(hWndCtrl);

   if (!hDCOrig)
      EndPaint(hWnd, &Paint);

   if (fDrawSelBlock)
      if (SS_INVERTSEL(lpSS))
         SS_DrawSelBlock(0, lpSS);

   lpSS->lpBook->fPaintingToScreen = FALSE;

#ifdef SS_UTP
   if (!lpSS->EditModeOn && (!SS_USESINGLESELBAR(lpSS) ||
       GetFocus() == hWnd))
      SS_HighlightCell(lpSS, TRUE);

   if (lpSS->EditModeOn && lpSS->fUseEditModeShadow)
      {
      short x;
      short y;
      short cx;
      short cy;

      SS_GetClientRect(lpSS->lpBook, &Rect);
      SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL,
                      lpSS->Col.CurAt, lpSS->Row.CurAt, &x, &y, &cx, &cy);

      /*
      if (x + cx > Rect.right)
         cx = Rect.right - x;

      if (y + cy > Rect.bottom)
         cy = Rect.bottom - y;
      */

      SS_PaintEditModeShadow(hWnd, lpSS, &Rect, x, y, cx, cy);
      }
#endif
   }

else
   lpSS->lpBook->fPaintingToScreen = FALSE;

if (!SS_INVERTSEL(lpSS) && !lpSS->FreezeHighlight && SS_USESELBAR(lpSS))
   lpSS->HighlightOn = TRUE;

if (hDCOrig && !lpSS->lpBook->fOnDraw)
   ReleaseDC(hWnd, hDCSpread);

return (0);
}


void SS_ClipClientRect(LPSS_BOOK lpBook, HDC hDC, LPRECT lpRectClient)
{
RECT RectTemp;

GetClientRect(lpBook->hWnd, &RectTemp);

#ifdef WIN32
// RFW 12/10/98 scs7717
if (RectTemp.right > lpRectClient->right && RectTemp.bottom > lpRectClient->bottom)
   {
   HRGN hRgn;
   POINT pts[2] = { lpRectClient->left, lpRectClient->top, lpRectClient->right,
                    lpRectClient->bottom };
   LPtoDP(hDC, pts, 2);
   SetRect(&RectTemp, pts[0].x, pts[0].y, pts[1].x, pts[1].y);
   hRgn = CreateRectRgnIndirect(&RectTemp);
   ExtSelectClipRgn(hDC, hRgn, RGN_AND);
   DeleteObject(hRgn);
   }
#else
if (RectTemp.right > lpRectClient->right)
   ExcludeClipRect(hDC, lpRectClient->right, 0, RectTemp.right,
                   RectTemp.bottom);

if (RectTemp.bottom > lpRectClient->bottom)
   ExcludeClipRect(hDC, 0, lpRectClient->bottom, RectTemp.right,
                   RectTemp.bottom);
#endif
}

#ifdef SS_V80
void SS_DrawSplitBoxArea(HDC hDC, LPSPREADSHEET lpSS)
{
	HBRUSH hBrush1, hBrush2;
	RECT ClientRect;
	RECT BoxRect;

	if (lpSS->lpBook->fVScrollVisible && lpSS->lpBook->fHScrollVisible)
	{
		hBrush1 = CreateSolidBrush(lpSS->lpBook->sheetSplitBoxStartColor);
		hBrush2 = CreateSolidBrush(lpSS->lpBook->sheetSplitBoxEndColor);

		GetClientRect(lpSS->lpBook->hWnd, &ClientRect);

		BoxRect.left = ClientRect.right - lpSS->lpBook->dVScrollWidth + 1;
		BoxRect.right = ClientRect.right;
		BoxRect.top = ClientRect.bottom - lpSS->lpBook->dHScrollHeight + 1;
		BoxRect.bottom = ClientRect.bottom;

		fpGradientFill(hDC, BoxRect, hBrush1, hBrush2, GRADIENT_FILL_RECT_V);

		DeleteObject(hBrush1);
		DeleteObject(hBrush2);
	}
}
#endif

BOOL SSx_Paint(LPSPREADSHEET lpSS, HDC hDC, HDC hDCDev, LPRECT lpRectUpdate)
{                                           
RECT     RectClient;
HWND     hWnd = lpSS->lpBook->hWnd;
SS_COORD RightVisCell;
SS_COORD BottomVisCell;
int      iDCSave;

RightVisCell = SS_GetRightVisCell(lpSS, lpSS->Col.UL);
BottomVisCell = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);

iDCSave = SaveDC(hDC);

/*******************
* Erase Background
*******************/

SS_GetClientRect(lpSS->lpBook, &RectClient);
if (!lpRectUpdate)
   lpRectUpdate = &RectClient;

SS_EraseBkgnd(hWnd, lpSS, hDC, &RectClient, lpRectUpdate, RightVisCell, BottomVisCell);

lpSS->lpBook->fInvalidated = FALSE;
lpSS->lpBook->fWindowPainted = TRUE;

SS_ClipClientRect(lpSS->lpBook, hDC, &RectClient);

SS_Draw(hWnd, hDC, hDCDev, lpSS, &RectClient, lpRectUpdate, lpSS->Col.UL, lpSS->Row.UL,
        RightVisCell, BottomVisCell, SS_GetColCnt(lpSS), SS_GetRowCnt(lpSS), NULL);

RestoreDC(hDC, iDCSave);

#ifdef SS_V70
if (SS_IsTabStripVisible(lpSS->lpBook))
	{
	RECT rcTemp;

	if (IntersectRect(&rcTemp, lpRectUpdate, &lpSS->lpBook->TabStripRect))
		SSTab_Draw(lpSS->lpBook->hTabStrip, hDC, &lpSS->lpBook->TabStripRect, TRUE);

	if (IntersectRect(&rcTemp, lpRectUpdate, &lpSS->lpBook->TabSplitBoxRect))
#ifdef SS_V80
		SS_DrawSplitBox(hDC, &lpSS->lpBook->TabSplitBoxRect, lpSS->lpBook);
#else
		SS_DrawSplitBox(hDC, &lpSS->lpBook->TabSplitBoxRect);
#endif
	}
#ifdef SS_V80
	if (lpSS->lpBook->wAppearanceStyle == 2)
		SS_DrawSplitBoxArea(hDC, lpSS);
#endif
#endif // SS_V70

return (0);
}


BOOL SS_Draw(HWND hWnd, HDC hDC, HDC hDCDev, LPSPREADSHEET lpSS, LPRECT lpRect, LPRECT lpRectUpdate,
             SS_COORD CellLeft, SS_COORD CellTop, SS_COORD CellRight, SS_COORD CellBottom,
             SS_COORD xCellMax, SS_COORD yCellMax, LPSSX_PRINTFORMAT lpPFormat)
{
SS_COLORTBLITEM   BackColorTblItem;
SS_COLORTBLITEM   ForeColorTblItem;
TBGLOBALHANDLE    hColWidthList = 0;
TBGLOBALHANDLE    hRowHeightList = 0;
COLORREF          TextColorOld;
COLORREF          BkColorOld;
// 99914148 -scl
//LPSHORT           lpColWidthList = 0;
//LPSHORT           lpRowHeightList = 0;
LPINT             lpColWidthList = 0;
LPINT             lpRowHeightList = 0;
LPSS_ROW          lpRow;
LPSS_COL          lpCol;
long              lColWidthListAt;
long              lColWidthListCnt = 0;
long              lColWidthListAllocCnt = 0;
long              lRowHeightListCnt = 0;
long              lRowHeightListAllocCnt = 0;
RECT              RectTemp;
RECT              RectTempBorder;
RECT              RectLast;
RECT              RectIsVisible;
LPRECT            lpCellOverflowRectPrev;
LPRECT            lpCellOverflowRectNext;
#ifndef SS_NOOVERFLOW
RECT              CellOverflowRectPrev;
RECT              CellOverflowRectNext;
#endif
HPEN              hPen;
HPEN              hPenOld;
HPEN              hPenGray = 0;
BYTE              LineType;
BOOL              fGridLinesVert = FALSE;
BOOL              fGridLinesHorz = FALSE;
BOOL              fRightColVis;
BOOL              fBottomRowVis;
BOOL              fOverflowToLeft;
BOOL              fFirstCellPrintedInRow;
int               xRightCol;
int               HeaderRowHeight;
int               HeaderColWidth;
int               yBottomRow;
SS_COORD          RowHeaderAt;
SS_COORD          ColHeaderAt;
SS_COORD          ColHeaderAtSave;
SS_COORD          TempRowAt;
SS_COORD          TempColAt;
SS_COORD          RowAt;
SS_COORD          ColAt;
SS_COORD          LastColPrinted;
SS_COORD          CellOverflowColPrev;
SS_COORD          CellOverflowColNext;
int               CellOverflowPosPrev;
int               CellOverflowPosNext;
int               RowPosAt;
int               ColPosAt;
SS_COORD          RowsMax;
SS_COORD          ColsMax;
SS_COORD          ColLast;
int               Height;
int               Width;
POINT             Point;
SS_COORD          i;
BOOL              bCellOverflowClipPrev;     // BJO 10Jun97 SCS4154 - Added
int               xCellOverflowClipPosPrev;  // BJO 10Jun97 SCS4154 - Added
double            dfWidth;
double            dfHeight;
double            dfRowPosAt;
double            dfColPosAt;
BOOL              fDrawSelected;
BOOL              fDrawBorder;


#ifdef SS_V40
SS_SpanFreePaintList(lpSS);
#endif // SS_V40

if (!lpPFormat)
   {
   UnrealizeObject(lpSS->lpBook->hBrushLines);

   SS_GetColorItem(&ForeColorTblItem, lpSS->Color.ForegroundId);
#ifdef SS_V35
   {
   SS_COLORTBLITEM ColorTblItem;
   SS_GetColorItem(&ColorTblItem, lpSS->Color.ShadowDarkId);
   hPen = CreatePen(PS_SOLID, 1, SS_TranslateColor(ColorTblItem.Color));
   }
#else
   hPen = CreatePen(PS_SOLID, 1, SS_TranslateColor(ForeColorTblItem.Color));
#endif

   #ifdef SS_GRIDGRAY
      lpSS->wGridType |= SS_GRID_SOLID;
      lpSS->GridColor = RGBCOLOR_DARKGRAY;
   #endif

   if (lpSS->wGridType & SS_GRID_SOLID)
      {
      if (GetDeviceCaps(hDC, NUMCOLORS) == 2)
         hPenGray = CreatePen(PS_SOLID, 1, RGBCOLOR_BLACK);
      else
         hPenGray = CreatePen(PS_SOLID, 1, SS_TranslateColor(lpSS->GridColor));
      }
   }
else
//Modify by BOC 99.5.18 (hyt)------------------------------
//GridColor should print color when PrintColor set
   {
   if (lpPFormat->pf.fDrawColors &&
       lpPFormat->wPrinterType != SS_PRINTER_HP4000PCL6)
      {
      SS_GetColorItem(&ForeColorTblItem, lpSS->Color.ForegroundId);
      hPen = CreatePen(PS_SOLID, 1, SS_TranslateColor(ForeColorTblItem.Color));
      hPenGray = CreatePen(PS_SOLID, 1, SS_TranslateColor(lpSS->GridColor));
      }
   else
      {
      hPen = CreatePen(PS_SOLID, 1, RGBCOLOR_BLACK);
      hPenGray = CreatePen(PS_SOLID, 1, RGBCOLOR_BLACK);
      }
   }
//---------------------------------------------------------

SS_GetColorItem(&BackColorTblItem, lpSS->Color.BackgroundId);

if (!lpPFormat || lpPFormat->pf.fDrawColors)
   BkColorOld = SetBkColor(hDC, SS_TranslateColor(BackColorTblItem.Color));

if (!lpPFormat)
   TextColorOld = SetTextColor(hDC, SS_TranslateColor(lpSS->GridColor));

hPenOld = SelectObject(hDC, hPen);
LineType = SS_LINETYPE_SOLID;
SetBkMode(hDC, TRANSPARENT);

if (!lpPFormat && !lpSS->fPrintingInProgress)
   {
   hColWidthList = lpSS->Col.hCellSizeList;
   lColWidthListCnt = lpSS->Col.lCellSizeListCnt;
   lColWidthListAllocCnt = lpSS->Col.lCellSizeListAllocCnt;

   hRowHeightList = lpSS->Row.hCellSizeList;
   lRowHeightListCnt = lpSS->Row.lCellSizeListCnt;
   lRowHeightListAllocCnt = lpSS->Row.lCellSizeListAllocCnt;
   }

if (hColWidthList)
// 99914148 -scl
//   lpColWidthList = (LPSHORT)tbGlobalLock(hColWidthList);
   lpColWidthList = (LPINT)tbGlobalLock(hColWidthList);

if (hRowHeightList)
// 99914148 -scl
//   lpRowHeightList = (LPSHORT)tbGlobalLock(hRowHeightList);
   lpRowHeightList = (LPINT)tbGlobalLock(hRowHeightList);

if (lpSS->wGridType & SS_GRID_HORIZONTAL)
   fGridLinesHorz = TRUE;

if (lpSS->wGridType & SS_GRID_VERTICAL)
   fGridLinesVert = TRUE;

if (lpPFormat && !lpPFormat->pf.fShowGrid)
   {
   fGridLinesHorz = FALSE;
   fGridLinesVert = FALSE;
   }

CellTop = max(CellTop, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
CellLeft = max(CellLeft, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);
RowsMax = yCellMax;
ColsMax = xCellMax;

/*****************
* Setup row info
*****************/

RowHeaderAt = 0;
RowAt = CellTop;
dfRowPosAt = -1;
RowPosAt = -1;
lRowHeightListCnt = 0;
HeaderRowHeight = 0;
fBottomRowVis = (CellBottom == yCellMax - 1 ? TRUE : FALSE);

do
   {
   if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      TempRowAt = RowHeaderAt;
   else
      TempRowAt = RowAt;

   if (lpPFormat && !lpPFormat->pf.fShowColHeaders && TempRowAt < lpSS->Row.HeaderCnt)
     Height = 0;
   else
     Height = SS_GetRowHeightInPixels(lpSS, TempRowAt);

#ifdef SS_V30
   dfHeight = SS_DFSCREEN2PRINTERY(lpPFormat, Height);
#else
   dfHeight = (double)Height;
#endif

   dfRowPosAt += dfHeight;
   Height = (int)dfRowPosAt - RowPosAt;
   RowPosAt = (int)dfRowPosAt;

   if (TempRowAt < lpSS->Row.HeaderCnt)
      HeaderRowHeight += Height;

   /*****************************
   * Create List of row heights
   *****************************/

   if (lRowHeightListCnt >= lRowHeightListAllocCnt)
      {
      if (lRowHeightListAllocCnt == 0)
         {
         lRowHeightListAllocCnt += 20;
// 99914148 -scl
//         hRowHeightList = tbGlobalAlloc(GMEM_MOVEABLE |
//                      GMEM_ZEROINIT,
//                      (long)(lRowHeightListAllocCnt *
//                      sizeof(short)));
         hRowHeightList = tbGlobalAlloc(GMEM_MOVEABLE |
                      GMEM_ZEROINIT,
                      (long)(lRowHeightListAllocCnt *
                      sizeof(int)));
         }

      else
         {
         tbGlobalUnlock(hRowHeightList);
         lRowHeightListAllocCnt += 20;
// 99914148 -scl
//         hRowHeightList = tbGlobalReAlloc(hRowHeightList,
//                              (long)(lRowHeightListAllocCnt *
//                              sizeof(short)), GMEM_MOVEABLE | GMEM_ZEROINIT);
         hRowHeightList = tbGlobalReAlloc(hRowHeightList,
                              (long)(lRowHeightListAllocCnt *
                              sizeof(int)), GMEM_MOVEABLE | GMEM_ZEROINIT);
         }

      if (hRowHeightList)
// 99914148 -scl
//         lpRowHeightList = (LPSHORT)tbGlobalLock(hRowHeightList);
         lpRowHeightList = (LPINT)tbGlobalLock(hRowHeightList);
      else
         return (0);
      }

   lpRowHeightList[lRowHeightListCnt] = Height;

   if (!lpPFormat)
      {
      lpSS->Row.LR = TempRowAt;

      if (TempRowAt == CellTop || lpRect->top + RowPosAt <= lpRect->bottom)
         lpSS->Row.LRAllVis = TempRowAt;
      }

   if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      RowHeaderAt++;
   else
      RowAt++;

   lRowHeightListCnt++;
   if (fBottomRowVis)
      yBottomRow = lpRect->top + RowPosAt;

   } while (RowPosAt < lpRect->bottom && TempRowAt < RowsMax - 1);

/*****************
* Setup col info
*****************/

ColHeaderAtSave = 0;

if (lpPFormat && !lpPFormat->pf.fShowRowHeaders)
   ColHeaderAtSave = lpSS->Col.HeaderCnt;

ColAt = CellLeft;
dfColPosAt = -1;
ColPosAt = -1;
ColHeaderAt = ColHeaderAtSave;
lColWidthListCnt = 0;
HeaderColWidth = 0;
fRightColVis = (CellRight == xCellMax - 1 ? TRUE : FALSE);

do
   {
   if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      TempColAt = ColHeaderAt;
   else
      TempColAt = ColAt;

   Width = SS_GetColWidthInPixels(lpSS, TempColAt);

#ifdef SS_V30
   dfWidth = SS_DFSCREEN2PRINTERX(lpPFormat, Width);
#else
   dfWidth = (double)Width;
#endif

#ifdef SS_UTP
   if (lpPFormat &&
       TempColAt == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
      dfWidth = 0;
#endif

   dfColPosAt += dfWidth;
   Width = (int)dfColPosAt - ColPosAt;
   ColPosAt = (int)dfColPosAt;

   if ((!lpPFormat || lpPFormat->pf.fShowRowHeaders) &&
       TempColAt < lpSS->Col.HeaderCnt)
      HeaderColWidth += Width;

   /*******************************
   * Create List of column widths
   *******************************/

   if (lColWidthListCnt >= lColWidthListAllocCnt)
      {
      if (lColWidthListAllocCnt == 0)
         {
         lColWidthListAllocCnt += 20;
// 99914148 -scl
//         hColWidthList = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
//                         (long)(lColWidthListAllocCnt *
//                         sizeof(short)));
         hColWidthList = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                         (long)(lColWidthListAllocCnt *
                         sizeof(int)));
         }

      else
         {
         tbGlobalUnlock(hColWidthList);
         lColWidthListAllocCnt += 20;
// 99914148 -scl
//         hColWidthList = tbGlobalReAlloc(hColWidthList,
//                         (long)(lColWidthListAllocCnt *
//                         sizeof(short)), GMEM_MOVEABLE |
//                         GMEM_ZEROINIT);
         hColWidthList = tbGlobalReAlloc(hColWidthList,
                         (long)(lColWidthListAllocCnt *
                         sizeof(int)), GMEM_MOVEABLE |
                         GMEM_ZEROINIT);
         }

      if (hColWidthList)
// 99914148 -scl
//         lpColWidthList = (LPSHORT)tbGlobalLock(hColWidthList);
         lpColWidthList = (LPINT)tbGlobalLock(hColWidthList);
      else
         lpColWidthList = 0;
      }

   if (lpColWidthList)
      lpColWidthList[lColWidthListCnt] = Width;

   if (!lpPFormat)
      {
      lpSS->Col.LR = TempColAt;

      if (TempColAt == CellLeft || lpRect->left + ColPosAt <= lpRect->right)
         lpSS->Col.LRAllVis = TempColAt;
      }

   if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      ColHeaderAt++;
   else
      ColAt++;

   lColWidthListCnt++;

   if (fRightColVis)
      xRightCol = lpRect->left + ColPosAt;

   } while (ColPosAt < lpRect->right - lpRect->left && TempColAt < ColsMax - 1);

if (lpPFormat)
	{
	lpPFormat->lpColWidthList = lpColWidthList;
	lpPFormat->lpRowHeightList = lpRowHeightList;
	lpPFormat->lColWidthListCnt = lColWidthListCnt;
	lpPFormat->lRowHeightListCnt = lRowHeightListCnt;
	}

/*****************
* Draw Row Lines
*****************/

RowAt = CellTop;
RowPosAt = -1;
RowHeaderAt = 0;
lRowHeightListCnt = 0;

do
   {
   if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      TempRowAt = RowHeaderAt;
   else
      TempRowAt = RowAt;

   RowPosAt += lpRowHeightList[lRowHeightListCnt];

   if (lpRect->top + RowPosAt >= lpRectUpdate->top &&
       lpRect->top + RowPosAt < lpRectUpdate->bottom &&
       lpRowHeightList[lRowHeightListCnt] > 0)
      {
      /*****************
      * Draw Row Lines
      *****************/

      Point.x = lpRect->left;
      Point.y = lpRect->top + RowPosAt;
      SS_MoveTo(hDC, Point.x, Point.y, lpPFormat);

      if (HeaderColWidth)
         {
         if (TempRowAt >= RowsMax - 1 && lpPFormat)
            LineType = SS_LINETYPE_NONE;
         else if (lpPFormat && !lpPFormat->pf.fDrawShadows)
            LineType = SS_LINETYPE_DOT;
         else
            LineType = SS_LINETYPE_SOLID;

         if (lpPFormat)
            Point.x = lpRect->left + min(HeaderColWidth - 1,
                      lpRect->right - lpRect->left - 1);
         else
            Point.x = lpRect->left + min(HeaderColWidth - 2,
                      lpRect->right - lpRect->left - 1);

         if (Point.x >= lpRectUpdate->left)
            SS_LineTo(lpSS, hDC, lpSS->lpBook->hBrushLines,
                      lpSS->lpBook->hBrushLines2, hPenGray, Point.x, Point.y,
                      LineType, fGridLinesHorz, lpPFormat);

         else
            SS_MoveTo(hDC, Point.x + 1, Point.y, lpPFormat); // RFW 4/25/93
			
         }

      if (RowHeaderAt < lpSS->Row.HeaderCnt)
         {
         if (lpPFormat && !lpPFormat->pf.fDrawShadows)
            LineType = SS_LINETYPE_DOT;
         else
            LineType = SS_LINETYPE_SOLID;
         }
      else if (TempRowAt >= RowsMax - 1)
         {
         if (lpPFormat)
            LineType = SS_LINETYPE_NONE;
         else if (lpSS->lpBook->fNoBorder)
            LineType = SS_LINETYPE_DOT;
#ifdef SS_UTP
         else if (lpSS->fUseScrollArrows)
            LineType = SS_LINETYPE_DOT;
#endif
         else
            LineType = SS_LINETYPE_SOLID;
         }

      else
         LineType = SS_LINETYPE_DOT;

      if (HeaderColWidth < lpRect->right - lpRect->left)
         {
         Point.x = lpRect->right - 1;

         if (Point.x >= lpRectUpdate->left)
            {
            if (Point.x >= lpRectUpdate->right)
               Point.x = lpRectUpdate->right - 1;

            if (fRightColVis)
               Point.x = min(Point.x,
                             xRightCol + (TempRowAt >= RowsMax - 1 ? 1 : 0));

            SS_LineTo(lpSS, hDC, lpSS->lpBook->hBrushLines,
                      lpSS->lpBook->hBrushLines2, hPenGray, Point.x, Point.y,
                      LineType, fGridLinesHorz, lpPFormat);
            }
         }
      }

   if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      RowHeaderAt++;
   else
      RowAt++;

   lRowHeightListCnt++;

   } while (RowPosAt < lpRect->bottom && TempRowAt < RowsMax - 1);

/*****************
* Draw Col Lines
*****************/

ColAt = CellLeft;
ColPosAt = -1;
ColHeaderAt = ColHeaderAtSave;
lColWidthListAt = 0;

ColLast = ColsMax - 1;
while (ColLast >= lpSS->Col.HeaderCnt && SS_IsColHidden(lpSS, ColLast))
   ColLast--;

do
   {
   if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      TempColAt = ColHeaderAt;
   else
      TempColAt = ColAt;

   ColPosAt += lpColWidthList[lColWidthListAt];

   if (lpRect->left + ColPosAt - lpColWidthList[lColWidthListAt] < lpRectUpdate->right &&
       lpRect->left + ColPosAt >= lpRectUpdate->left &&
       lpColWidthList[lColWidthListAt] > 0)
      {
      /*****************
      * Draw Col Lines
      *****************/

      Point.x = lpRect->left + ColPosAt;
      Point.y = lpRect->top;
      SS_MoveTo(hDC, Point.x, Point.y, lpPFormat);

      if (HeaderRowHeight)
         {
         if (TempColAt >= ColLast && lpPFormat)
            LineType = SS_LINETYPE_NONE;
         else if (lpPFormat && !lpPFormat->pf.fDrawShadows)
            LineType = SS_LINETYPE_DOT;
         else
            LineType = SS_LINETYPE_SOLID;

         Point.y = lpRect->top + min(HeaderRowHeight - 1,
                   lpRect->bottom - lpRect->top - 1);

         if (Point.y >= lpRectUpdate->top)
            SS_LineTo(lpSS, hDC, lpSS->lpBook->hBrushLines,
                      lpSS->lpBook->hBrushLines2, hPenGray,
                      Point.x, Point.y, LineType, fGridLinesVert,
                      lpPFormat);
         else                                    // RFW 4/25/93
            SS_MoveTo(hDC, Point.x, Point.y + 1, lpPFormat);
         }

      if (ColHeaderAt < lpSS->Col.HeaderCnt)
         {
         if (lpPFormat && !lpPFormat->pf.fDrawShadows)
            LineType = SS_LINETYPE_DOT;
         else
            LineType = SS_LINETYPE_SOLID;
         }
      else if (TempColAt >= ColLast)
         {
         if (lpPFormat)
            LineType = SS_LINETYPE_NONE;
         else if (lpSS->lpBook->fNoBorder)
            LineType = SS_LINETYPE_DOT;
#ifdef SS_UTP
         else if (lpSS->fUseScrollArrows)
            LineType = SS_LINETYPE_DOT;
#endif
         else
            LineType = SS_LINETYPE_SOLID;
         }

      else
         LineType = SS_LINETYPE_DOT;

      Point.y = lpRect->bottom - 1;

      if (Point.y > lpRectUpdate->top)
         {
         if (Point.y >= lpRectUpdate->bottom)
            Point.y = lpRectUpdate->bottom - 1;

         if (fBottomRowVis)
            Point.y = min(Point.y, yBottomRow);

         SS_LineTo(lpSS, hDC, lpSS->lpBook->hBrushLines,
                   lpSS->lpBook->hBrushLines2, hPenGray, Point.x,
                   Point.y, LineType, fGridLinesVert, lpPFormat);
         }
      }

   if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      ColHeaderAt++;
   else
      ColAt++;

   lColWidthListAt++;

   } while (ColPosAt < lpRect->right - lpRect->left && TempColAt < ColsMax - 1);

/*************
* Draw Cells
*************/

RowHeaderAt = 0;
RowAt = CellTop;
RowPosAt = 0;
lRowHeightListCnt = 0;

do
   {
   if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      TempRowAt = RowHeaderAt;
   else
      TempRowAt = RowAt;

   Height = lpRowHeightList[lRowHeightListCnt];
 
   RowPosAt += Height;

   lpRow = SS_LockRowItem(lpSS, TempRowAt);

   if (lpRect->top + RowPosAt - Height - 1 < lpRectUpdate->bottom &&
       lpRect->top + RowPosAt - 1 >= lpRectUpdate->top)
      {
      /**********************************
      * Draw each column within the row
      **********************************/

      ColAt = CellLeft;
      ColPosAt = -1;
      ColHeaderAt = ColHeaderAtSave;

      lColWidthListAt = 0;

      CellOverflowColPrev = -1;
      CellOverflowPosPrev = 0;
      CellOverflowColNext = -1;
      CellOverflowPosNext = 0;
      fFirstCellPrintedInRow = FALSE;
      LastColPrinted = -1;
      fOverflowToLeft = FALSE;
      bCellOverflowClipPrev = FALSE;  // BJO 10Jun97 SCS4154 - Add

      fDrawSelected = SS_ShouldCellBeDrawnSelected(lpSS, -1, TempRowAt, lpRow);

      do
         {
         if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
            TempColAt = ColHeaderAt;
         else
            TempColAt = ColAt;

         Width = lpColWidthList[lColWidthListAt];

         ColPosAt += Width;

         if (lpRect->left + ColPosAt - Width < lpRectUpdate->right &&
             lpRect->left + ColPosAt >= lpRectUpdate->left)
            {
            lpCol = SS_LockColItem(lpSS, TempColAt);

            /************
            * Draw Cell
            ************/

            RectTemp.left = lpRect->left + ColPosAt - Width + 1;
            RectTemp.top = lpRect->top + RowPosAt - Height;
            RectTemp.right = lpRect->left + ColPosAt;
            RectTemp.bottom = lpRect->top + RowPosAt - 1;
            CopyRect(&RectTempBorder, &RectTemp);
				fDrawBorder = TRUE;

            // 24769 (border is drawn after cell contents) -scl
            //SS_DrawBorder(hWnd, lpSS, hDC, lpCol, lpRow,
            //              &RectTempBorder, TempColAt, TempRowAt, lpPFormat);
#ifdef SS_V40
				if (SS_SpanIsCellAlreadyPainted(lpSS, TempColAt, TempRowAt))
					// If cell is not part of span, then it is a merger, so set fDrawBorder to FALSE
					if (!SS_GetCellSpan(lpSS, TempColAt, TempRowAt, NULL, NULL, NULL, NULL))
						fDrawBorder = FALSE;
#endif // SS_V40

				CopyRect(&RectIsVisible, &RectTemp);
				if ((lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERHORZGRIDONLY ||
					  lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID))
					RectIsVisible.bottom++;

				if ((lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERVERTGRIDONLY ||
					  lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID))
					RectIsVisible.right++;

				if (RectTemp.top < lpRectUpdate->bottom &&
                RectTemp.bottom >= lpRectUpdate->top &&
                RectTemp.left < lpRectUpdate->right &&
                RectTemp.right >= lpRectUpdate->left &&
                TempRowAt < RowsMax && TempColAt < ColsMax &&
				    (!hDCDev || lpPFormat || RectVisible(hDCDev, &RectIsVisible)))
               {
               /**********************************************
               * Determine next empty cell for text overflow
               **********************************************/

               lpCellOverflowRectPrev = NULL;
               lpCellOverflowRectNext = NULL;

#ifndef SS_NOOVERFLOW
               if (lpSS->lpBook->fAllowCellOverflow)
                  {
                  LPSS_CELL lpCellTemp;
                  RECT      RectOverflow;
                  BOOL      fOverflow;
						SS_COORD  lColCnt = SS_GetColCnt(lpSS);

                  if (CellOverflowColNext == -1 ||
                      CellOverflowColNext <= TempColAt)
                     {
                     long lColWidthListAtTemp = lColWidthListAt;

                     CellOverflowColNext = -1;
                     CellOverflowPosNext = lpRect->left + ColPosAt;

                     for (i = TempColAt + 1; i < lColCnt; i++)
                        {
                        lColWidthListAtTemp++;
                        fOverflow = FALSE;

// RFW - 11/9/98 - CHF4
                        if (i == lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
                            i != CellLeft)
                           ;
                        else if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, i,
                                 TempRowAt))
                           {
                           if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_LEFT)
                              {
                              fOverflow = TRUE;
                              if (lColWidthListAtTemp < lColWidthListCnt)
                                 CellOverflowPosNext += lpColWidthList[lColWidthListAtTemp];
                              else
                                 CellOverflowPosNext +=
                                    (int)SS_GetColWidthInPixelsExDF(lpSS, i, lpPFormat);
                              CellOverflowColNext = i;
                              }

                           SS_UnlockCellItem(lpSS, i, TempRowAt);
                           }

                        if (!fOverflow)
                           break;
                        }
                     }

// RFW - 11/9/98 - CHF4
//                  else
//                     CellOverflowColNext = -1;

                  if (CellOverflowColPrev == -1)
                     {
							// RFW - 3/2/00 - GIC11727
                     long lColWidthListAtTemp = lColWidthListAt;

                     CellOverflowPosPrev = RectTemp.left;

                     for (i = TempColAt - 1; i >= lpSS->Col.HeaderCnt +
                          lpSS->Col.Frozen; i--)
                        {
                        lColWidthListAtTemp--;
                        fOverflow = FALSE;
                        if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, i,
                            TempRowAt))
                           {
                           if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_RIGHT)
                              {
                              fOverflow = TRUE;
                              // BJO 10Jun97 SCS4154 - Begin fix
                              if( i == CellLeft - 1)
                                 {
                                 bCellOverflowClipPrev = TRUE;
                                 xCellOverflowClipPosPrev = (int)CellOverflowPosPrev;
                                 }
                              // BJO 10Jun97 SCS4154 - End fix
                              if (lColWidthListAtTemp >= (i >= lpSS->Col.Frozen + lpSS->Col.HeaderCnt ? lpSS->Col.Frozen : 0) + lpSS->Col.HeaderCnt) // RFW - 3/2/00 - GIC11727
                                 CellOverflowPosPrev -= lpColWidthList[lColWidthListAtTemp];
                              else
                                 CellOverflowPosPrev -= (int)SS_GetColWidthInPixelsExDF(lpSS, i, lpPFormat);

                              CellOverflowColPrev = i;
                              }

                           SS_UnlockCellItem(lpSS, i, TempRowAt);
                           }

                        if (!fOverflow)
                           break;
                        }
                     }

                  if (CellOverflowColPrev != -1)
                     {
                     SetRect(&CellOverflowRectPrev, (int)CellOverflowPosPrev,
                             RectTemp.top, RectTemp.right, RectTemp.bottom);
                     lpCellOverflowRectPrev = &CellOverflowRectPrev;
                     }

                  if (CellOverflowColNext != -1)
                     {
                     SetRect(&CellOverflowRectNext, RectTemp.left,
                             RectTemp.top, (int)CellOverflowPosNext,
                             RectTemp.bottom);
                     lpCellOverflowRectNext = &CellOverflowRectNext;
                     }

                  /* RFW - 4/25/00 - SCS9638
                  if (!fFirstCellPrintedInRow && TempColAt >=
                      lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
                  */
                  if (!fFirstCellPrintedInRow)
                     {
                     if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, TempColAt, TempRowAt))
                        {
                        if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_LEFT)
                           {
                           if (CellOverflowColNext == -1)
                              CopyRect(&RectOverflow, &RectTemp);
                           else
                              UnionRect(&RectOverflow, &RectTemp, lpCellOverflowRectNext);

                           SS_DrawHiddenOverflowCellLeft(hWnd, hDC, lpSS, lpRow, &RectOverflow,
                                                         TempColAt, TempRowAt, lpPFormat,
                                                         lpRectUpdate, CellLeft, CellTop,
                                                         TempColAt >= ColLast, TempRowAt >= RowsMax - 1);
                           }

                        SS_UnlockCellItem(lpSS, TempColAt, TempRowAt);
                        }

                     if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, TempColAt - 1, TempRowAt))
                        {
                        if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_RIGHT)
                           fOverflowToLeft = TRUE;

                        SS_UnlockCellItem(lpSS, TempColAt, TempRowAt);
                        }

                     if (!fDrawSelected && TempColAt >= lpSS->Col.HeaderCnt)
                        fFirstCellPrintedInRow = TRUE;
                     }

						// RFW - 1/2/02 - 9434
						if (TempColAt == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
                     fFirstCellPrintedInRow = FALSE;

                  LastColPrinted = TempColAt;

                  // BJO 10Jun97 SCS4154 - Begin fix
                  if (bCellOverflowClipPrev)
                     {
                     SaveDC(hDC);
                     IntersectClipRect(hDC, xCellOverflowClipPosPrev,
                                      lpRectUpdate->top, lpRectUpdate->right,
                                      lpRectUpdate->bottom);
                     }
                  // BJO 10Jun97 SCS4154 - End fix
                  }
#endif

               /*********************
               * Draw cell contents
               *********************/

               CopyRect(&RectLast, &RectTemp);

#ifdef SS_UTP
               if (!lpPFormat || TempColAt != 0 || TempRowAt != 0)
#endif
//CTF          SS_DrawText(hDC, hWnd, lpSS, lpCol, lpRow, &RectTemp,
//CTF                      TempColAt, TempRowAt, FALSE, lpPFormat,
//CTF                      lpRectUpdate, lpCellOverflowRectPrev,
//CTF                      lpCellOverflowRectNext, fOverflowToLeft);
//RFW-10/28/97 SS_DrawText(hDC, hWnd, lpSS, lpCol, lpRow, &RectTemp,
//                         TempColAt, TempRowAt, TRUE, lpPFormat,
//                         lpRectUpdate, lpCellOverflowRectPrev,
//                         lpCellOverflowRectNext, fOverflowToLeft);

               if (fDrawSelected)
                  {
                  if (lpCellOverflowRectNext &&
                      lpCellOverflowRectNext->right > RectTemp.right)
                     lpCellOverflowRectNext->right = RectTemp.right + 1;

                  if (lpCellOverflowRectPrev &&
                      lpCellOverflowRectPrev->left < RectTemp.left)
                     lpCellOverflowRectPrev->left = RectTemp.left - 1;

                  SS_DrawText(hDC, hWnd, lpSS, lpCol, lpRow, &RectTemp,
                              TempColAt, TempRowAt, FALSE, lpPFormat,
                              lpRectUpdate, lpCellOverflowRectPrev,
                              lpCellOverflowRectNext, fOverflowToLeft,
                              SS_DRAWTEXTSEL_DEF, CellLeft, CellTop,
                              TempColAt >= ColLast, TempRowAt >= RowsMax - 1);
                  }
               else
                  {
                  SS_DrawText(hDC, hWnd, lpSS, lpCol, lpRow, &RectTemp,
                              TempColAt, TempRowAt, FALSE, lpPFormat,
                              lpRectUpdate, lpCellOverflowRectPrev,
                              lpCellOverflowRectNext, fOverflowToLeft,
                              SS_DRAWTEXTSEL_DEF, CellLeft, CellTop,
                              TempColAt >= ColLast, TempRowAt >= RowsMax - 1);
                  }

               fOverflowToLeft = FALSE;

               /**************************************************
               * Determine previous empty cell for text overflow
               **************************************************/

#ifndef SS_NOOVERFLOW
               if (lpSS->lpBook->fAllowCellOverflow)
                  {
                  LPSS_CELL lpCellTemp;
                  BOOL      fOverflow = FALSE;

                  // BJO 10Jun97 SCS4154 - Begin fix
                  if (bCellOverflowClipPrev)
                     {
                     RestoreDC(hDC, -1);
                     }
                  // BJO 10Jun97 SCS4154 - End fix

                  if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, TempColAt,
                      TempRowAt))
                     {
                     if ((lpCellTemp->Data.bOverflow & SS_OVERFLOW_RIGHT))
                        fOverflow = TRUE;

                     SS_UnlockCellItem(lpSS, TempColAt, TempRowAt);
                     }

                  if (fOverflow)
                     {
                     // This code was added to fix the problem with
                     // selection colors and right justified cell overflow

                     if (fDrawSelected)
                        {
                        RECT RectOverflow;

                        if (!lpCellOverflowRectPrev)
                           CopyRect(&RectOverflow, &RectTemp);
                        else
                           UnionRect(&RectOverflow, &RectTemp,
                                     lpCellOverflowRectPrev);

                        SS_DrawHiddenOverflowCellRight(hWnd, hDC, lpSS, lpRow, &RectOverflow,
                                                       TempColAt, TempRowAt, lpPFormat,
                                                       lpRectUpdate, CellLeft, CellTop,
                                                       TempColAt >= ColLast, TempRowAt >= RowsMax - 1);
                        }

                     if (CellOverflowColPrev == -1)
                        {
                        // BOOL fDone = FALSE;

                        CellOverflowColPrev = TempColAt;
                        CellOverflowPosPrev = RectLast.left;

                        /*
                        for (i = TempColAt - 1; i >= lpSS->Col.HeaderCnt &&
                             !fDone; i--)
                           {
                           fDone = TRUE;

                           if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow, i,
                               TempRowAt))
                              {
                              if (lpCellTemp->Data.bOverflow &
                                  SS_OVERFLOW_RIGHT)
                                 {
                                 CellOverflowColPrev = i;
                                 CellOverflowPosPrev -=
                                    SS_GetColWidthInPixelsExDF(lpSS, i, lpPFormat);
                                 fDone = FALSE;
                                 }

                              SS_UnlockCellItem(lpSS, i, TempRowAt);
                              }
                           }
                        */
                        }

                     fOverflowToLeft = TRUE;
                     }
                  else
                     CellOverflowColPrev = -1;

                  }
#endif
               }

// BJO 01Apr98 JEH383 - Begin fix (code moved from before cell contents drawn)
#ifndef SS_NOBORDERS
				if (fDrawBorder)
					SS_DrawCellBorder(lpSS, hDC, TempColAt, TempRowAt, &RectTempBorder, lpPFormat, CellLeft, CellTop);
/*
				if (fDrawBorder)
					{
					int iSaveDCTemp = 0;
					SS_COORD CellCol = TempColAt;
					SS_COORD CellRow = TempRowAt;
					LPSS_COL lpColTemp = lpCol;
					LPSS_ROW lpRowTemp = lpRow;
//#if 0 // RFW - 2/26/02 - 9780
#ifdef SS_V40
					SS_SELBLOCK MergeBlock;

					// RFW - 5/5/02 - 9938
					if (SS_MergeCalcBlock(lpSS, CellRow, lpRow, CellCol, lpCol, &MergeBlock))
						{
						RectTempBorder.left -= SS_GetCellOffsetX(lpSS, MergeBlock.UL.Col, CellCol, lpPFormat, 0);
						RectTempBorder.top -= SS_GetCellOffsetY(lpSS, MergeBlock.UL.Row, CellRow, lpPFormat, 0);
						RectTempBorder.right += SS_GetCellOffsetX(lpSS, CellCol + 1, MergeBlock.LR.Col + 1, lpPFormat, 0);
						RectTempBorder.bottom += SS_GetCellOffsetY(lpSS, CellRow + 1, MergeBlock.LR.Row + 1, lpPFormat, 0);

						if ((MergeBlock.UL.Col < CellLeft && MergeBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen) ||
							 (MergeBlock.UL.Row < CellTop && MergeBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen))
							{
							RECT RectTemp;

							CopyRect(&RectTemp, &RectTempBorder);
							RectTemp.left--;
							RectTemp.top--;

							if (MergeBlock.UL.Col < CellLeft && MergeBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
								RectTemp.left = (int)SS_DFSCREEN2PRINTERX(lpPFormat, SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt + lpSS->Col.Frozen) + 1);
							if (MergeBlock.UL.Row < CellTop && MergeBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
								RectTemp.top = (int)SS_DFSCREEN2PRINTERY(lpPFormat, SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt + lpSS->Row.Frozen) + 1);

							iSaveDCTemp = SaveDC(hDC);
							IntersectClipRect(hDC, RectTemp.left, RectTemp.top, RectTemp.right + 1, RectTemp.bottom + 1);
							}

						CellRow = MergeBlock.UL.Row;
						CellCol = MergeBlock.UL.Col;
						lpRowTemp = SS_LockRowItem(lpSS, CellRow);
						lpColTemp = SS_LockColItem(lpSS, CellCol);
						}
#endif // SS_V40
//#endif // 0

					SS_DrawBorder(hWnd, lpSS, hDC, lpColTemp, lpRowTemp, &RectTempBorder, CellCol, CellRow, lpPFormat);

					if (iSaveDCTemp)
						RestoreDC(hDC, iSaveDCTemp);
					}
*/
#endif
// BJO 01Apr98 JEH383 - End fix

            SS_UnlockColItem(lpSS, TempColAt);
            }

         if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
            ColHeaderAt++;
         else
            ColAt++;

         lColWidthListAt++;

         } while (ColPosAt < lpRect->right - lpRect->left &&
                  TempColAt < ColsMax - 1);

#ifndef SS_NOOVERFLOW
      if (lpSS->lpBook->fAllowCellOverflow && LastColPrinted != -1 &&
          LastColPrinted < SS_GetColCnt(lpSS) - 1 && !fDrawSelected)
         {
         LPSS_CELL lpCellTemp;
         RECT      RectOverflow;

         if (lpCellTemp = SSx_LockCellItem(lpSS, lpRow,
                                           LastColPrinted, TempRowAt))
            {
            if (lpCellTemp->Data.bOverflow & SS_OVERFLOW_RIGHT)
               {
               if (!lpCellOverflowRectPrev)
                  CopyRect(&RectOverflow, &RectLast);
               else
                  UnionRect(&RectOverflow, &RectLast,
                            lpCellOverflowRectPrev);

               SS_DrawHiddenOverflowCellRight(hWnd, hDC, lpSS, lpRow, &RectOverflow,
                                              LastColPrinted, TempRowAt, lpPFormat,
                                              lpRectUpdate, CellLeft, CellTop,
                                              TempColAt >= ColLast, TempRowAt >= RowsMax - 1);
               }

            SS_UnlockCellItem(lpSS, LastColPrinted, TempRowAt);
            }

         fFirstCellPrintedInRow = TRUE;
         }
#endif
      }

   SS_UnlockRowItem(lpSS, TempRowAt);

   if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      RowHeaderAt++;
   else
      RowAt++;

   lRowHeightListCnt++;

   } while (RowPosAt <= lpRect->bottom && TempRowAt < RowsMax - 1);

if (!lpPFormat && ColPosAt < lpRect->right - lpRect->left)
   {
   RectTemp.left = max(lpRectUpdate->left, lpRect->left + ColPosAt + 1);
   RectTemp.top = max(lpRectUpdate->top, lpRect->top);
   RectTemp.right = min(lpRectUpdate->right, lpRect->right);
   RectTemp.bottom = min(lpRectUpdate->bottom, lpRect->bottom);

   SS_HatchRect(hDC, lpSS, &RectTemp, lpPFormat);
   }

RowPosAt--;

if (!lpPFormat && RowPosAt < lpRect->bottom - lpRect->top)
   {
   RectTemp.left = max(lpRectUpdate->left, lpRect->left);
   RectTemp.top = max(lpRectUpdate->top, lpRect->top + RowPosAt + 1);
   RectTemp.right = min(lpRectUpdate->right, lpRect->right);
   RectTemp.bottom = min(lpRectUpdate->bottom, lpRect->bottom);

   SS_HatchRect(hDC, lpSS, &RectTemp, lpPFormat);
   }

if (hColWidthList)
   {
   tbGlobalUnlock(hColWidthList);

   if (lpPFormat || lpSS->fPrintingInProgress)
      {
      if (lpSS->Col.hCellSizeList)
         tbGlobalFree(lpSS->Col.hCellSizeList);

      tbGlobalFree(hColWidthList);
      lpSS->Col.lCellSizeListAllocCnt = 0;
      lpSS->Col.lCellSizeListCnt = 0;
      lpSS->Col.hCellSizeList = 0;
      }
   else
      {
      lpSS->Col.hCellSizeList = hColWidthList;
      lpSS->Col.lCellSizeListCnt = lColWidthListCnt;
      lpSS->Col.lCellSizeListAllocCnt = lColWidthListAllocCnt;
      }
   }

if (hRowHeightList)
   {
   tbGlobalUnlock(hRowHeightList);

   if (lpPFormat || lpSS->fPrintingInProgress)
      {
      if (lpSS->Row.hCellSizeList)
         tbGlobalFree(lpSS->Row.hCellSizeList);

      tbGlobalFree(hRowHeightList);
      lpSS->Row.lCellSizeListAllocCnt = 0;
      lpSS->Row.lCellSizeListCnt = 0;
      lpSS->Row.hCellSizeList = 0;
      }
   else
      {
      lpSS->Row.hCellSizeList = hRowHeightList;
      lpSS->Row.lCellSizeListCnt = lRowHeightListCnt;
      lpSS->Row.lCellSizeListAllocCnt = lRowHeightListAllocCnt;
      }
   }

if (!lpPFormat)
   SetTextColor(hDC, TextColorOld);

if (!lpPFormat || lpPFormat->pf.fDrawColors)
   SetBkColor(hDC, BkColorOld);

SelectObject(hDC, hPenOld);
DeleteObject(hPen);

if (hPenGray)
   DeleteObject(hPenGray);

#ifdef SS_V40
SS_SpanFreePaintList(lpSS);
#endif // SS_V40

return (0);
}


#ifndef SS_NOBORDERS
void SS_DrawCellBorder(LPSPREADSHEET lpSS, HDC hDC, SS_COORD Col, SS_COORD Row, LPRECT lpRect, LPSSX_PRINTFORMAT lpPFormat, SS_COORD CellLeft, SS_COORD CellTop)
{
int iSaveDCTemp = 0;
SS_COORD CellCol = Col;
SS_COORD CellRow = Row;
LPSS_COL lpCol = SS_LockColItem(lpSS, Col);
LPSS_ROW lpRow = SS_LockRowItem(lpSS, Row);
LPSS_COL lpColTemp = lpCol;
LPSS_ROW lpRowTemp = lpRow;

#ifdef SS_V40
{
SS_SELBLOCK MergeBlock;

// RFW - 5/5/02 - 9938
if (SS_MergeCalcBlock(lpSS, Row, lpRow, Col, lpCol, &MergeBlock))
	{
	lpRect->left -= SS_GetCellOffsetX(lpSS, MergeBlock.UL.Col, CellCol, lpPFormat, 0);
	lpRect->top -= SS_GetCellOffsetY(lpSS, MergeBlock.UL.Row, CellRow, lpPFormat, 0);
	lpRect->right += SS_GetCellOffsetX(lpSS, CellCol + 1, MergeBlock.LR.Col + 1, lpPFormat, 0);
	lpRect->bottom += SS_GetCellOffsetY(lpSS, CellRow + 1, MergeBlock.LR.Row + 1, lpPFormat, 0);

	if ((MergeBlock.UL.Col < CellLeft && MergeBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen) ||
		 (MergeBlock.UL.Row < CellTop && MergeBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen))
		{
		RECT RectTemp;

		CopyRect(&RectTemp, lpRect);
		RectTemp.left--;
		RectTemp.top--;

		if (MergeBlock.UL.Col < CellLeft && MergeBlock.UL.Col >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
			RectTemp.left = (int)SS_DFSCREEN2PRINTERX(lpPFormat, SS_GetCellPosX(lpSS, 0, lpSS->Col.HeaderCnt + lpSS->Col.Frozen) + 1);
		if (MergeBlock.UL.Row < CellTop && MergeBlock.UL.Row >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
			RectTemp.top = (int)SS_DFSCREEN2PRINTERY(lpPFormat, SS_GetCellPosY(lpSS, 0, lpSS->Row.HeaderCnt + lpSS->Row.Frozen) + 1);

		iSaveDCTemp = SaveDC(hDC);
		IntersectClipRect(hDC, RectTemp.left, RectTemp.top, RectTemp.right + 1, RectTemp.bottom + 1);
		}

	CellRow = MergeBlock.UL.Row;
	CellCol = MergeBlock.UL.Col;
	lpRowTemp = SS_LockRowItem(lpSS, CellRow);
	lpColTemp = SS_LockColItem(lpSS, CellCol);
	}
}
#endif // SS_V40

SS_DrawBorder(lpSS->lpBook->hWnd, lpSS, hDC, lpColTemp, lpRowTemp, lpRect, CellCol, CellRow, lpPFormat);

if (iSaveDCTemp)
	RestoreDC(hDC, iSaveDCTemp);

SS_UnlockColItem(lpSS, Col);
SS_UnlockRowItem(lpSS, Row);
}
#endif


BOOL SS_DrawSelBlock(hDC, lpSS)

HDC           hDC;
LPSPREADSHEET lpSS;
{
BOOL          fRet;

if (lpSS->fMultipleBlocksSelected)
   fRet = SS_InvertMultiSelBlock(hDC, lpSS);
else
   fRet = SS_InvertBlock(hDC, lpSS);

return (fRet);
}


BOOL SS_InvertBlock(hDC, lpSS)

HDC           hDC;
LPSPREADSHEET lpSS;
{
LPSS_SELBLOCK lpItemList;
HDC           hDCOrig;
RECT          Rect;
RECT          Rect2;
RECT          RectClip;
HRGN          hRgn;
HRGN          hRgn2;
HRGN          hRgnClip;
short         i;
int           iDCSave;
int           iRet;

hDCOrig = hDC;

if (!SS_IsActiveSheet(lpSS))
	return (FALSE);

if (lpSS->fSuspendInvert)
   return (FALSE);

if (!SS_IsBlockSelected(lpSS))
   return (FALSE);

if (lpSS->Row.LR == 0 || lpSS->Col.LR == 0)
   return (FALSE);

if (SS_CreateBlockRect(lpSS, &Rect, lpSS->BlockCellUL.Col,
                       lpSS->BlockCellUL.Row,
                       lpSS->BlockCellLR.Col,
                       lpSS->BlockCellLR.Row, FALSE))
   {
   if (!hDCOrig)
      hDC = SS_GetDC(lpSS->lpBook);

   hRgn = CreateRectRgnIndirect(&Rect);

// RFW - jap6666   if (!lpSS->fNoShowActiveWithSel && SS_IsCellVisible(lpSS, lpSS->Col.CurAt,
//       lpSS->Row.CurAt, SS_VISIBLE_PARTIAL))

#ifdef SS_UTP
   if (!lpSS->fNoShowActiveWithSel)
#endif
      if ((lpSS->Row.CurAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen ||
          (lpSS->Row.CurAt >= lpSS->Row.UL - 1 &&
           lpSS->Row.CurAt <= (lpSS->fVirtualMode ?
           SS_GetBottomCell(lpSS, lpSS->Row.UL) : lpSS->Row.LR) + 1)) &&
          (lpSS->Col.CurAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen ||
           (lpSS->Col.CurAt >= lpSS->Col.UL - 1 &&
            lpSS->Col.CurAt <= lpSS->Col.LR + 1)))
			{
			if (SS_CreateBlockRect(lpSS, &Rect2, lpSS->Col.CurAt,
										  lpSS->Row.CurAt, lpSS->Col.CurAt,
										  lpSS->Row.CurAt, TRUE))
				{
				hRgn2 = CreateRectRgnIndirect(&Rect2);
				CombineRgn(hRgn, hRgn, hRgn2, RGN_XOR);
				DeleteObject(hRgn2);
				}
			}

   if (lpSS->fMultipleBlocksSelected)
      {
      lpItemList = (LPSS_SELBLOCK)GlobalLock(lpSS->MultiSelBlock.hItemList);

      for (i = 0; i < lpSS->MultiSelBlock.dItemCnt; i++)
         if (SS_CreateBlockRect(lpSS, &Rect, lpItemList[i].UL.Col,
                                lpItemList[i].UL.Row, lpItemList[i].LR.Col,
                                lpItemList[i].LR.Row, FALSE))
            {
            hRgn2 = CreateRectRgnIndirect(&Rect);
            CombineRgn(hRgn, hRgn, hRgn2, RGN_DIFF);
            DeleteObject(hRgn2);
            }

      GlobalUnlock(lpSS->MultiSelBlock.hItemList);
      }

#ifndef SS_NODRAGDROP
   else if (lpSS->lpBook->fAllowDragDrop)
      SS_AddDragDropRectToBlock(lpSS, hRgn);
#endif

#ifdef SS_UTP
   if (lpSS->fUseScrollArrows)
      {
      if (SS_CreateBlockRect(lpSS, &Rect, lpSS->Col.HeaderCnt +
                            lpSS->Col.Frozen - 1, 0, lpSS->Col.HeaderCnt +
                            lpSS->Col.Frozen - 1, -1, FALSE))
         {
         hRgn2 = CreateRectRgnIndirect(&Rect);
         CombineRgn(hRgn, hRgn, hRgn2, RGN_DIFF);
         DeleteObject(hRgn2);
         }
      }

#endif

   iDCSave = SaveDC(hDC);

   if ((iRet = GetClipBox(hDC, &RectClip)) != ERROR && iRet != NULLREGION)
      {
      hRgnClip = CreateRectRgnIndirect(&RectClip);
      CombineRgn(hRgn, hRgn, hRgnClip, RGN_AND);
      DeleteObject(hRgnClip);
      }

   if (SS_INVERTSEL(lpSS))
      {
#ifndef SS_UTP
      SelectClipRgn(hDC, hRgn);
#endif
      InvertRgn(hDC, hRgn);
      }
   else
      InvalidateRgn(lpSS->lpBook->hWnd, hRgn, FALSE);

   DeleteObject(hRgn);
   RestoreDC(hDC, iDCSave);

   if (!hDCOrig)
      SS_ReleaseDC(lpSS->lpBook, hDC);
   }

return (TRUE);
}


BOOL SS_InvertMultiSelBlock(hDC, lpSS)

HDC           hDC;
LPSPREADSHEET lpSS;
{
LPSS_SELBLOCK lpItemList;
HDC           hDCOrig;
RECT          RectActive;
RECT          Rect;
HRGN          hRgn = 0;
HRGN          hRgn2;
HRGN          hRgnActive;
short         i;
BOOL          fActive = FALSE;

hDCOrig = hDC;

if (!SS_IsActiveSheet(lpSS))
	return (FALSE);

if (lpSS->fSuspendInvert)
   return (FALSE);

if (!SS_IsBlockSelected(lpSS))
   return (FALSE);

if (lpSS->Row.LR == 0 || lpSS->Col.LR == 0)
   return (FALSE);

if (!hDCOrig)
   hDC = SS_GetDC(lpSS->lpBook);

if (SS_CreateBlockRect(lpSS, &RectActive, lpSS->Col.CurAt,
                       lpSS->Row.CurAt, lpSS->Col.CurAt,
                       lpSS->Row.CurAt, TRUE))
   fActive = TRUE;

lpItemList = (LPSS_SELBLOCK)GlobalLock(lpSS->MultiSelBlock.hItemList);

for (i = 0; i < lpSS->MultiSelBlock.dItemCnt; i++)
   if (SS_CreateBlockRect(lpSS, &Rect, lpItemList[i].UL.Col,
                          lpItemList[i].UL.Row, lpItemList[i].LR.Col,
                          lpItemList[i].LR.Row, FALSE))
      {
      if (!hRgn)
         {
         hRgn = CreateRectRgnIndirect(&Rect);

         if (!(lpSS->lpBook->wMode == SS_MODE_BLOCK) && i == lpSS->MultiSelBlock.dItemCnt - 1 && fActive)
            {
            hRgnActive = CreateRectRgnIndirect(&RectActive);
            CombineRgn(hRgn, hRgn, hRgnActive, RGN_XOR);
            DeleteObject(hRgnActive);
            }
         }

      else
         {
         hRgn2 = CreateRectRgnIndirect(&Rect);

         if (!(lpSS->lpBook->wMode == SS_MODE_BLOCK) && i == lpSS->MultiSelBlock.dItemCnt - 1 && fActive)
            {
            hRgnActive = CreateRectRgnIndirect(&RectActive);
            CombineRgn(hRgn2, hRgn2, hRgnActive, RGN_XOR);
            DeleteObject(hRgnActive);
            }

         CombineRgn(hRgn, hRgn, hRgn2, RGN_OR);
         DeleteObject(hRgn2);
         }
      }

if (lpSS->lpBook->wMode == SS_MODE_BLOCK)
   if (SS_CreateBlockRect(lpSS, &Rect, lpSS->BlockCellUL.Col,
                          lpSS->BlockCellUL.Row, lpSS->BlockCellLR.Col,
                          lpSS->BlockCellLR.Row, FALSE))
      {
      if (!hRgn)
         {
         hRgn = CreateRectRgnIndirect(&Rect);

			/* RFW - 5/29/08
         if (!(lpSS->lpBook->wMode == SS_MODE_BLOCK) && i == lpSS->MultiSelBlock.dItemCnt - 1)
			*/
            {
            hRgnActive = CreateRectRgnIndirect(&RectActive);
            CombineRgn(hRgn, hRgn, hRgnActive, RGN_XOR);
            DeleteObject(hRgnActive);
            }
         }

      else
         {
         hRgn2 = CreateRectRgnIndirect(&Rect);

			/* RFW - 5/29/08
         if (!(lpSS->lpBook->wMode == SS_MODE_BLOCK) && i == lpSS->MultiSelBlock.dItemCnt - 1)
			*/
            {
            hRgnActive = CreateRectRgnIndirect(&RectActive);
            CombineRgn(hRgn2, hRgn2, hRgnActive, RGN_XOR);
            DeleteObject(hRgnActive);
            }

         CombineRgn(hRgn, hRgn, hRgn2, RGN_OR);
         DeleteObject(hRgn2);
         }
      }

if (hRgn)
   {
   int iDCSave = SaveDC(hDC);

   if (SS_INVERTSEL(lpSS))
      {
#ifndef SS_UTP
      SelectClipRgn(hDC, hRgn);
#endif
      InvertRgn(hDC, hRgn);
      }
   else
      InvalidateRgn(lpSS->lpBook->hWnd, hRgn, FALSE);

   DeleteObject(hRgn);

   RestoreDC(hDC, iDCSave);
   }

if (!hDCOrig)
   SS_ReleaseDC(lpSS->lpBook, hDC);

GlobalUnlock(lpSS->MultiSelBlock.hItemList);

return (TRUE);
}


BOOL SS_CreateBlockRect(lpSS, Rect, Left, Top, Right, Bottom, fCell)

LPSPREADSHEET lpSS;
LPRECT        Rect;
SS_COORD      Left;
SS_COORD      Top;
SS_COORD      Right;
SS_COORD      Bottom;
BOOL          fCell;
{
RECT          RectClient;
SS_COORD      Col;
SS_COORD      Row;
BOOL          fRetRow = TRUE;
BOOL          fRetCol = TRUE;
BOOL          fRetRow2 = TRUE;
BOOL          fRetCol2 = TRUE;
short         Offset;
int           x;
int           y;
short         xTraUL;
short         xTraLR;
SS_COORD      i;

if ((Left >= lpSS->Col.HeaderCnt && Left > lpSS->Col.LR + 1) ||
    (Top >= lpSS->Row.HeaderCnt && Top > lpSS->Row.LR + 1) ||
    (Left >= lpSS->Col.HeaderCnt + lpSS->Col.Frozen &&
     Right >= lpSS->Col.HeaderCnt && Right < lpSS->Col.UL - 1) ||
    (Top >= lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
     Bottom >= lpSS->Row.HeaderCnt && Bottom < lpSS->Row.UL - 1))
   return (FALSE);

SS_GetClientRect(lpSS->lpBook, &RectClient);

if (Right == -1 && Bottom == -1)
   ;

else if (Right == -1 && Bottom != -1)
   {
   for (i = 0, Offset = 0; i < lpSS->Row.HeaderCnt; i++)
      Offset += SS_GetRowHeightInPixels(lpSS, i);

   RectClient.top += Offset;
   }

else if (Right != -1 && Bottom == -1)
   {
   for (i = 0, Offset = 0; i < lpSS->Col.HeaderCnt; i++)
      Offset += SS_GetColWidthInPixels(lpSS, i);

   RectClient.left += Offset;
   }

else if (Right != -1 && Bottom != -1)
   {
   /*****************
   * Determine Left
   *****************/

   for (i = 0, Offset = 0; i < lpSS->Col.HeaderCnt; i++)
      Offset += SS_GetColWidthInPixels(lpSS, i);

   RectClient.left += Offset;

   /****************
   * Determine Top
   ****************/

   for (i = 0, Offset = 0; i < lpSS->Row.HeaderCnt; i++)
      Offset += SS_GetRowHeightInPixels(lpSS, i);

   RectClient.top += Offset;
   }

if (Right == -1)
   {
   Rect->left = RectClient.left;
   Rect->right = min(SS_GetCellPosX(lpSS, lpSS->Col.UL,
                     lpSS->Col.LR) + SS_GetColWidthInPixels(lpSS,
                     lpSS->Col.LR), RectClient.right);
   }
else
   {
   if (lpSS->Col.Frozen && Left < lpSS->Col.HeaderCnt +
       lpSS->Col.Frozen)
      {
      Col = Left;
      Rect->left = SS_GetCellPosX(lpSS, 0, Left);
      }
   else
      {
      Col = max(Left, lpSS->Col.UL); 
//      Col = max(Left, lpSS->Col.UL - 1); RFW - JAP6666
      Rect->left = SS_GetCellPosX(lpSS, lpSS->Col.UL, Col);
      }

   if (Right == lpSS->Col.UL - 1 && Rect->left < RectClient.right && !fCell &&
       lpSS->Col.UL > lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
      x = Rect->left;
   else
      {
      for (x = Rect->left; Col <= Right && x < RectClient.right; )
         {
         x += SS_GetColWidthInPixels(lpSS, Col);

         if (lpSS->Col.Frozen && Col == lpSS->Col.HeaderCnt +
             lpSS->Col.Frozen - 1)
            Col = max(lpSS->Col.UL, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);
         else
            Col++;
         }

      if (Rect->left == x)
         fRetCol = FALSE;

      if (x == RectClient.right)
         fRetCol2 = FALSE;
      }

   if (SS_INVERTSEL(lpSS))
      {
      xTraUL = 1;
      xTraLR = 1;
      }
   else
      {
      xTraUL = -1;
      xTraLR = 0;
      }

   Rect->right = x + (xTraLR + 1);
   Rect->left = max(Rect->left - xTraUL, RectClient.left);

   if (Col == SS_GetColCnt(lpSS))
      Rect->right -= (xTraLR + 1);
// RFW - 5/6/04 - 14203
//   else if (Left <= lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1 &&
//            Right < lpSS->Col.UL)
   else if (Left <= lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1 &&
            Right == lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1)
      Rect->right -= xTraLR;
   else if (lpSS->Col.Frozen && Left >= lpSS->Col.HeaderCnt +
            lpSS->Col.Frozen && Left <= lpSS->Col.UL)
      Rect->left += (xTraUL + 1);
/*
      {
      Rect->right = x + 2;
      Rect->left = max(Rect->left - 1, RectClient.left);
      }
   else
      {
      Rect->right = x;
      Rect->left = max(Rect->left + 1, RectClient.left);
      }

   if (Col == SS_GetColCnt(lpSS))
      Rect->right -= 2;
   else if (Left <= lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1 &&
            Right < lpSS->Col.UL && SS_INVERTSEL(lpSS))
      Rect->right--;
   else if (lpSS->Col.Frozen && Left >= lpSS->Col.HeaderCnt +
            lpSS->Col.Frozen && Left <= lpSS->Col.UL && SS_INVERTSEL(lpSS))
      Rect->left += 2;
*/

   Rect->right = min(Rect->right, RectClient.right);
   }

if (Bottom == -1)
   {
   Rect->top = RectClient.top;
   Rect->bottom = min(SS_GetCellPosY(lpSS, lpSS->Row.UL,
                      lpSS->Row.LR) + SS_GetRowHeightInPixels(lpSS,
                      lpSS->Row.LR), RectClient.bottom);
   }
else
   {
   if (lpSS->Row.Frozen && Top < lpSS->Row.HeaderCnt +
       lpSS->Row.Frozen)
      {
      Row = Top;
      Rect->top = SS_GetCellPosY(lpSS, 0, Top);
      }
   else
      {
      Row = max(Top, lpSS->Row.UL);
//      Row = max(Top, lpSS->Row.UL - 1); RFW - JAP6666
      Rect->top = SS_GetCellPosY(lpSS, lpSS->Row.UL, Row);
      }

   if (Bottom == lpSS->Row.UL - 1 && Rect->top < RectClient.bottom && !fCell &&
       lpSS->Row.UL > lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
      y = Rect->top;
   else
      {
      for (y = Rect->top; Row <= Bottom && y < RectClient.bottom; )
         {
         y += SS_GetRowHeightInPixels(lpSS, Row);

         if (lpSS->Row.Frozen && Row == lpSS->Row.HeaderCnt +
             lpSS->Row.Frozen - 1)
            Row = max(lpSS->Row.UL, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
         else
            Row++;
         }

      if (Rect->top == y)
         fRetRow = FALSE;

      if (y == RectClient.bottom)
         fRetRow2 = FALSE;
      }

   if (SS_USESELBAR(lpSS))
      {
      Rect->top = max(Rect->top, RectClient.top);

      Rect->bottom = y;
		/* RFW - 8/3/04 - 14904
      if (Row != SS_GetRowCnt(lpSS))
         Rect->bottom += 1;
		*/
		// RFW - 2/1/05 - 15363
		if ((lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERHORZGRIDONLY ||
			  lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID) &&
			  Row != SS_GetRowCnt(lpSS))
			Rect->bottom += 1;

      Rect->bottom = min(Rect->bottom, RectClient.bottom);
      }

   else
      {
      if (SS_INVERTSEL(lpSS))
         {
         xTraUL = 1;
         xTraLR = 1;
         }
      else
         {
         xTraUL = -1;
         xTraLR = 0;
         }

      Rect->bottom = y + (xTraLR + 1);
      Rect->top = max(Rect->top - xTraUL, RectClient.top);

      if (Row == SS_GetRowCnt(lpSS))
         Rect->bottom -= (xTraLR + 1);
      else if (Top <= lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1 &&
               Bottom < lpSS->Row.UL)
         Rect->bottom -= xTraLR;
      else if (lpSS->Row.Frozen && Top >= lpSS->Row.HeaderCnt +
               lpSS->Row.Frozen && Top <= lpSS->Row.UL)
         Rect->top += (xTraUL + 1);
/*
      if (SS_INVERTSEL(lpSS))
         {
         Rect->bottom = y + 2;
         Rect->top = max(Rect->top - 1, RectClient.top);
         }
      else
         {
         Rect->bottom = y;
         Rect->top = max(Rect->top + 1, RectClient.top);
         }

      if (Row == SS_GetRowCnt(lpSS))
         Rect->bottom -= 2;
      else if (Top <= lpSS->Row.HeaderCnt + lpSS->Row.Frozen - 1 &&
               Bottom < lpSS->Row.UL && SS_INVERTSEL(lpSS))
         Rect->bottom--;
      else if (lpSS->Row.Frozen && Top >= lpSS->Row.HeaderCnt +
               lpSS->Row.Frozen && Top <= lpSS->Row.UL && SS_INVERTSEL(lpSS))
         Rect->top += 2;
//         Rect->top++; // RFW 5/8/98  GIC5063
*/

      Rect->bottom = min(Rect->bottom, RectClient.bottom);
      }
   }

if (fCell)
   {
   if (fRetRow && !fRetCol && lpSS->Col.CurAt == lpSS->Col.UL - 1)
      {
      Rect->right = Rect->left + 1;
      fRetCol = TRUE;
      }

   if (fRetCol && !fRetRow && lpSS->Row.CurAt == lpSS->Row.UL - 1)
      {
      Rect->bottom = Rect->top + 1;
      fRetRow = TRUE;
      }

   if (fRetRow2 && !fRetCol2 && lpSS->Col.CurAt == lpSS->Col.LR + 1)
      {
      Rect->left = Rect->right - 1;
      fRetCol = TRUE;
      }

   if (fRetCol2 && !fRetRow2 && lpSS->Row.CurAt == lpSS->Row.LR + 1)
      {
      Rect->top = Rect->bottom - 1;
      fRetRow = TRUE;
      }
   }

return (fRetRow && fRetCol);
}


void SS_LineTo(lpSS, hDC, hBrush, hBrush2, hPenGrid, x, y, LineType,
               fGridLines, lpPFormat)

LPSPREADSHEET     lpSS;
HDC               hDC;
HBRUSH            hBrush;
HBRUSH            hBrush2;
HPEN              hPenGrid;
int               x;
int               y;
BYTE              LineType;
BOOL              fGridLines;
LPSSX_PRINTFORMAT lpPFormat;
{
HBRUSH            hBrushOld;
HBRUSH            hBrushTemp;
POINT             ptCurPos;
int               x1;
int               y1;
int               xOrig;
int               yOrig;
int               dx = 1;
int               dy = 1;

#ifndef SS_V30
if (lpPFormat)
   {
   x = SS_SCREEN2PRINTERX(lpPFormat, x);
   dx = max(SS_SCREEN2PRINTERX(lpPFormat, 1), 1);
   y = SS_SCREEN2PRINTERY(lpPFormat, y);
   dy = max(SS_SCREEN2PRINTERY(lpPFormat, 1), 1);
   }
#endif

if (LineType == SS_LINETYPE_NONE || (!fGridLines &&
    (LineType == SS_LINETYPE_DOT || (lpPFormat &&
    !lpPFormat->pf.fDrawBorder))))
	{
//	EndPath(hDC);  // RFW-REMOVE
   return;
	}

xOrig = x;
yOrig = y;

GetCurrentPositionEx(hDC, &ptCurPos);

x1 = ptCurPos.x;
y1 = ptCurPos.y;

if (y == y1)
	{
	// RFW - 10/21/02 - 11087
	if (!lpPFormat || PtVisible(hDC, x + 1, y))
		x++;
	}
else
   y++;

if (lpPFormat)
   {
#ifndef SS_V30
   short i;

   if (y == y1)
      {
      for (i = 0; i < dy; i++)
         {
         LineTo(hDC, x, y + i);
         MoveToEx(hDC, x1, y1 + i + 1, NULL);
         }

      MoveToEx(hDC, x, y1, NULL);
      }
   else
      {
      for (i = 0; i < dx; i++)
         {
         LineTo(hDC, x + i, y);
         MoveToEx(hDC, x1 + i + 1, y1, NULL);
         }

      MoveToEx(hDC, x1, y, NULL);
      }
#else
   if (hPenGrid && LineType != SS_LINETYPE_SOLID)
      {
      HPEN hPenOld;

      hPenOld = SelectObject(hDC, hPenGrid);
      LineTo(hDC, x, y);

      if (lpPFormat->wPrinterType == SS_PRINTER_HP4000PCL6)
         {
         if (y == y1)
            {
            MoveToEx(hDC, x1, y1+1, NULL);
            LineTo(hDC, x, y+1);
            }
         else if (x == x1)
            {
            MoveToEx(hDC, x1 + 1, y1, NULL);
            LineTo(hDC, x + 1, y);
            }

         MoveToEx(hDC, x, y, NULL);
         }

      SelectObject(hDC, hPenOld);
      }
   else
      LineTo(hDC, x, y);
#endif
   }

else if (LineType == SS_LINETYPE_SOLID)
   LineTo(hDC, x, y);

else
   {
   if (hPenGrid)
      {
      HPEN hPenOld;

      hPenOld = SelectObject(hDC, hPenGrid);
      LineTo(hDC, x, y);
      SelectObject(hDC, hPenOld);
      }

   else
      {
      RECT Rect;

      Rect.left = x1;
      Rect.top = y1;
      Rect.right = xOrig + 1;
      Rect.bottom = yOrig + 1;

      /*
      if (x == x1)
         if (y1 % 2)
            hBrush = hBrush2;

      if (y == y1)
         if (x1 % 2)
            hBrush = hBrush2;
      */
#if 0
      if (x == x1)
         if ((BYTE)(y1 % 2) == lpSS->bVPixelOffset)
            hBrush = hBrush2;

      if (y == y1)
         if ((BYTE)(x1 % 2) == lpSS->bHPixelOffset)
            hBrush = hBrush2;

      /*
      UnrealizeObject(hBrush);
      FillRect(hDC, &Rect, hBrush);
      */

      UnrealizeObject(hBrush);
#ifdef WIN32
      SetBrushOrgEx(hDC, 0, 0, NULL);
#else
      SetBrushOrg(hDC, 0, 0);
#endif
      hBrushOld = SelectObject(hDC, hBrush);

      PatBlt(hDC, Rect.left, Rect.top, Rect.right - Rect.left,
             Rect.bottom - Rect.top, PATCOPY);

      SelectObject(hDC, hBrushOld);
#endif

      hBrushOld = SelectObject(hDC, hBrush2);

      UnrealizeObject(hBrush);
#ifdef WIN32
      SetBrushOrgEx(hDC, 0, 0, NULL);
#else
      SetBrushOrg(hDC, 0, 0);
#endif

      if (x == x1)
         if ((BYTE)(y1 % 2) == lpSS->bVPixelOffset)
#ifdef WIN32
            SetBrushOrgEx(hDC, (x % 2) ? 0 : 1, 0, NULL);
#else
            SetBrushOrg(hDC, (x % 2) ? 0 : 1, 0);
#endif
         else
#ifdef WIN32
            SetBrushOrgEx(hDC, (x % 2) ? 0 : 1, 1, NULL);
#else
            SetBrushOrg(hDC, (x % 2) ? 0 : 1, 1);
#endif
      if (y == y1)
         if ((BYTE)(x1 % 2) == lpSS->bHPixelOffset)
#ifdef WIN32
            SetBrushOrgEx(hDC, 0, (y % 2) ? 0 : 1, NULL);
#else
            SetBrushOrg(hDC, 0, (y % 2) ? 0 : 1);
#endif
         else
#ifdef WIN32
            SetBrushOrgEx(hDC, 1, (y % 2) ? 0 : 1, NULL);
#else
            SetBrushOrg(hDC, 1, (y % 2) ? 0 : 1);
#endif
      hBrushTemp = SelectObject(hDC, hBrush);

      PatBlt(hDC, Rect.left, Rect.top, Rect.right - Rect.left,
             Rect.bottom - Rect.top, PATCOPY);

      SelectObject(hDC, hBrushTemp);
      SelectObject(hDC, hBrushOld);
      }
   }

//EndPath(hDC);  // RFW-REMOVE
//StrokePath(hDC);
}


void SS_MoveTo(hDC, x, y, lpPFormat)

HDC               hDC;
int               x;
int               y;
LPSSX_PRINTFORMAT lpPFormat;
{
#ifndef SS_V30
if (lpPFormat)
   {
   x = SS_SCREEN2PRINTERX(lpPFormat, x);
   y = SS_SCREEN2PRINTERY(lpPFormat, y);
   }
#endif

//BeginPath(hDC); // RFW-REMOVE
MoveToEx(hDC, x, y, NULL);
}


void SS_HatchRect(hDC, lpSS, lpRect, lpPFormat)

HDC               hDC;
LPSPREADSHEET     lpSS;
LPRECT            lpRect;
LPSSX_PRINTFORMAT lpPFormat;
{
HBRUSH            hBrush;

if (!lpPFormat)
   {
#ifdef SS_V80
/*	if (lpSS->lpBook->wAppearanceStyle == 2)
		hBrush = CreateSolidBrush(RGB(173,216,230));
	else
	*/
		hBrush = CreateSolidBrush(SS_TranslateColor(lpSS->lpBook->GrayAreaBackground));

#else
   hBrush = CreateSolidBrush(SS_TranslateColor(lpSS->lpBook->GrayAreaBackground));
#endif
   UnrealizeObject(hBrush);
#ifdef WIN32
   SetBrushOrgEx(hDC, 0, 0, NULL);
#else
   SetBrushOrg(hDC, 0, 0);
#endif
   FillRect(hDC, lpRect, hBrush);
   DeleteObject(hBrush);
   }
}


#if 0
void SS_HatchRect(hDC, lpSS, lpRect, lpPFormat)

HDC               hDC;
LPSPREADSHEET     lpSS;
LPRECT            lpRect;
LPSSX_PRINTFORMAT lpPFormat;
{
HBITMAP           hBitmap;
HBRUSH            hBrush;
HBRUSH            hBrushOld;
COLORREF          BkColorOld;
COLORREF          TextColorOld;
static WORD       Bits[] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

if (!lpPFormat)
   {
   BkColorOld = SetBkColor(hDC, SS_TranslateColor(lpSS->GrayAreaBackground));
   TextColorOld = SetTextColor(hDC, SS_TranslateColor(lpSS->GrayAreaForeground));
   }

hBitmap = CreateBitmap(8, 8, 1, 1, Bits);
hBrush = CreatePatternBrush(hBitmap);

UnrealizeObject(hBrush);
#ifdef WIN32
SetBrushOrgEx(hDC, lpRect->left % 2 ? 1 : 0, lpRect->top % 2 ? 1 : 0, NULL);
#else
SetBrushOrg(hDC, lpRect->left % 2 ? 1 : 0, lpRect->top % 2 ? 1 : 0);
#endif
hBrushOld = SelectObject(hDC, hBrush);

PatBlt(hDC, lpRect->left, lpRect->top, lpRect->right - lpRect->left,
       lpRect->bottom - lpRect->top, PATCOPY);

SelectObject(hDC, hBrushOld);

/*
FillRect(hDC, lpRect, hBrush);
*/

DeleteObject(hBrush);
DeleteObject(hBitmap);

if (!lpPFormat)
   {
   SetBkColor(hDC, BkColorOld);
   SetTextColor(hDC, TextColorOld);
   }
}
#endif


int SS_GetColWidthInPixelsEx(LPSPREADSHEET lpSS, SS_COORD Col,
                             LPSSX_PRINTFORMAT lpPFormat)
{
#ifdef SS_V30
return (SS_SCREEN2PRINTERX(lpPFormat, SS_GetColWidthInPixels(lpSS, Col)));
#else
return (SS_GetColWidthInPixels(lpSS, Col));
#endif
}


double SS_GetColWidthInPixelsExDF(LPSPREADSHEET lpSS, SS_COORD Col,
                                  LPSSX_PRINTFORMAT lpPFormat)
{
#ifdef SS_V30
return (SS_DFSCREEN2PRINTERX(lpPFormat, SS_GetColWidthInPixels(lpSS, Col)));
#else
return ((double)SS_GetColWidthInPixels(lpSS, Col));
#endif
}


int SS_GetColWidthInPixels(LPSPREADSHEET lpSS, SS_COORD Col)
{
LPSS_COL lpCol;
HPSHORT  hpPrintColWidthList;
int      dColWidth;

if (lpSS->hPrintColWidthList && Col < lpSS->lPrintColWidthListCnt &&
    !lpSS->lpBook->fPaintingToScreen)
   {
   hpPrintColWidthList = (HPSHORT)tbGlobalLock(lpSS->hPrintColWidthList);
   dColWidth = hpPrintColWidthList[Col];
   tbGlobalUnlock(lpSS->hPrintColWidthList);
   return (dColWidth);
   }

dColWidth = lpSS->Col.dCellSizeInPixels;

if (Col != -1 && (lpCol = SS_LockColItem(lpSS, Col)))
   {
   if (lpCol->fColHidden)
      dColWidth = 0;
   else if (lpCol->dColWidthInPixels != SS_WIDTH_DEFAULT)
      dColWidth = lpCol->dColWidthInPixels;

   SS_UnlockColItem(lpSS, Col);
   }

return (dColWidth);
}


//int SS_ColWidthToPixels(lpSS, ColWidth)
long SS_ColWidthToPixels(lpSS, ColWidth)

LPSPREADSHEET lpSS;
double        ColWidth;
{
double        dfTemp;
//int           Width;
long           Width;

switch (lpSS->lpBook->dUnitType)
   {
   case SS_UNITTYPE_NORMAL:
      if (ColWidth == SS_WIDTH_DEFAULT)
         Width = lpSS->lpBook->StandardFontWidth * DEFAULT_CELLWIDTH;
      else if (ColWidth < -1)
         Width = (long)((double)(lpSS->lpBook->StandardFontWidth *
                 DEFAULT_CELLWIDTH) / -ColWidth);
      else
         Width = (long)((double)lpSS->lpBook->StandardFontWidth * ColWidth);

      break;

   case SS_UNITTYPE_DEFAULT:
      if (ColWidth == SS_WIDTH_DEFAULT)
         ColWidth = DEFAULT_CELLWIDTH;
      else if (ColWidth < -1)
         ColWidth = DEFAULT_CELLWIDTH / (-1 * ColWidth);

      dfTemp = (double)STANDARDFONTWIDTH * ColWidth *
               (double)lpSS->lpBook->dxPixelsPerInch / (double)XPIXELSPERINCH;
      Width = (long)SS_Round(dfTemp, 0);
      break;

   case SS_UNITTYPE_TWIPS:
      if (ColWidth == SS_WIDTH_DEFAULT)
         ColWidth = DEFAULT_CELLWIDTH_TWIPS;
      else if (ColWidth < -1)
         ColWidth = DEFAULT_CELLWIDTH_TWIPS / (-1 * ColWidth);

      Width = (long)SS_Round((double)((long)ColWidth *
                             (long)lpSS->lpBook->dxPixelsPerInch) / 1440.0, 0);

      break;
   }

if (Width > 0)
   {
   /*
   if (Width % 2 == 0)
      Width++;
   */

   // BJO 27Feb97 SCS4761 - Begin removal of code
   ////***********************************************************
   ////* Make special case for 1024x768 - So much for Microsoft's
   ////* claim that Windows is device independent
   ////***********************************************************
   //if (lpSS->dxPixelsPerInch == 120 &&
   //    lpSS->dUnitType != SS_UNITTYPE_TWIPS)
   //   Width++;
   // BJO 27Feb97 SCS4761 - End removal of code

   Width += lpSS->lpBook->dBorderSize;
   }

return (Width);
}


int SS_GetRowHeightInPixelsEx(LPSPREADSHEET lpSS, SS_COORD Row,
                             LPSSX_PRINTFORMAT lpPFormat)
{
#ifdef SS_V30
return (SS_SCREEN2PRINTERY(lpPFormat, SS_GetRowHeightInPixels(lpSS, Row)));
#else
return (SS_GetRowHeightInPixels(lpSS, Row));
#endif
}


int SS_GetRowHeightInPixels(LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ROW lpRow;
int      dRowHeight;

#ifdef SS_UTP
if (lpSS->hPrintRowHeightList && Row < lpSS->lPrintRowHeightListCnt &&
    !lpSS->fPaintingToScreen)
   {
   HPSHORT hpPrintRowHeightList = (HPSHORT)tbGlobalLock(
                                           lpSS->hPrintRowHeightList);
   dRowHeight = hpPrintRowHeightList[Row];
   tbGlobalUnlock(lpSS->hPrintRowHeightList);
   return (dRowHeight);
   }
#endif

dRowHeight = lpSS->Row.dCellSizeInPixels;

if (lpRow = SS_LockRowItem(lpSS, Row))
   {
   if (lpRow->fRowHidden)
      dRowHeight = 0;
   else if (lpRow->dRowHeightInPixels != SS_WIDTH_DEFAULT)
      dRowHeight = lpRow->dRowHeightInPixels;

   SS_UnlockRowItem(lpSS, Row);
   }

return (dRowHeight);
}


long SS_CalcRowHeight(LPSPREADSHEET lpSS, SS_COORD Row)
{
LPSS_ROW lpRow;
double   RowHeight = SS_HEIGHT_DEFAULT;
long    dPixels = -1;

if (Row == -1)
   dPixels = SS_RowHeightToPixels(lpSS, Row,
                                  lpSS->Row.dCellSizeX100 == -1 ? -1 :
                                  (double)lpSS->Row.dCellSizeX100 / 100.0);

else if (lpRow = SS_LockRowItem(lpSS, Row))
   {
   if (lpRow->dRowHeightX100 == 0)
      dPixels = 0;
   else
      dPixels = SS_RowHeightToPixels(lpSS, Row, lpRow->dRowHeightX100 ==
                                     SS_HEIGHT_DEFAULT ? -1.0 :
                                     (double)lpRow->dRowHeightX100 / 100.0);

   SS_UnlockRowItem(lpSS, Row);
   }

return (dPixels);
}


int SS_RowHeightToPixels(lpSS, Row, RowHeight)

LPSPREADSHEET lpSS;
SS_COORD      Row;
double        RowHeight;
{
LPSS_FONT     Font;
SS_FONT       FontTemp;
LPSS_ROW      lpRow;
double        FontCellHeight;
double        Height;
double        dfTemp;
int           FontHeight = -1;

if (Row != -1 && (lpRow = SS_LockRowItem(lpSS, Row)))
   {
   Font = SS_GetFont(lpSS->lpBook, &FontTemp, lpRow->RowMaxFontId);

   if (Font && Font->FontHeight != SS_HEIGHT_DEFAULT)
      {
      FontHeight = Font->FontHeight;
      FontCellHeight = Font->FontCellHeight;
      }

   SS_UnlockRowItem(lpSS, Row);
   }

if (FontHeight == -1)
   {
   Font = SS_GetFont(lpSS->lpBook, &FontTemp, lpSS->DefaultFontId);

   FontHeight = Font->FontHeight;
   FontCellHeight = Font->FontCellHeight;
   }

switch (lpSS->lpBook->dUnitType)
   {
   case SS_UNITTYPE_NORMAL:
      if (RowHeight == SS_HEIGHT_DEFAULT)
         Height = FontHeight;
      else
         Height = (int)(RowHeight * (double)FontHeight) / FontCellHeight;

      break;

   case SS_UNITTYPE_DEFAULT:
      if (RowHeight == SS_HEIGHT_DEFAULT)
         Height = FontHeight;
      else
         {
         dfTemp = RowHeight * (double)FontHeight;
         dfTemp /= FontCellHeight;
         dfTemp *= (double)lpSS->lpBook->dyPixelsPerInch;
         dfTemp /= (double)YPIXELSPERINCH;
         Height = (int)(dfTemp + 0.5);
         }

      break;

   case SS_UNITTYPE_TWIPS:
      if (RowHeight == SS_HEIGHT_DEFAULT)
         Height = FontHeight;
      else
         Height = (short)SS_Round((double)((long)RowHeight *
                         (long)lpSS->lpBook->dyPixelsPerInch) / 1440.0, 0);

      break;
   }

if (Height > 0)
   {
#ifdef SS_GP
   if ((short)Height % 2 == 0)
//      if (lpSS->dyPixelsPerInch == 72 &&
//          lpSS->dUnitType != SS_UNITTYPE_TWIPS)
      if (lpSS->dyPixelsPerInch == 72)
         Height--;
      else
         Height++;
#endif

   Height += lpSS->lpBook->dBorderSize;
   }

return ((short)Height);
}


void SS_CalcPixelsToColWidth(lpSS, Col, PixelWidth, lpdfColWidth)

LPSPREADSHEET lpSS;
SS_COORD      Col;
int           PixelWidth;
LPDOUBLE      lpdfColWidth;
{
Col;

if (PixelWidth <= 0)
   *lpdfColWidth = 0;
else
   {
   PixelWidth -= lpSS->lpBook->dBorderSize;

   switch (lpSS->lpBook->dUnitType)
      {
      case SS_UNITTYPE_NORMAL:
         *lpdfColWidth = (double)PixelWidth /
                         (double)lpSS->lpBook->StandardFontWidth;
         break;

      case SS_UNITTYPE_DEFAULT:
         *lpdfColWidth = (double)PixelWidth * (double)XPIXELSPERINCH /
                         ((double)STANDARDFONTWIDTH *
                         (double)lpSS->lpBook->dxPixelsPerInch);
         break;

      case SS_UNITTYPE_TWIPS:
         *lpdfColWidth = ((double)PixelWidth * 1440.0) /
                         (double)lpSS->lpBook->dxPixelsPerInch;
         break;
      }
   }
}


void SS_CalcPixelsToRowHeight(lpSS, Row, PixelHeight, lpdfRowHeight)

LPSPREADSHEET lpSS;
SS_COORD      Row;
int           PixelHeight;
LPDOUBLE      lpdfRowHeight;
{
LPSS_FONT     Font;
SS_FONT       FontTemp;
LPSS_ROW      lpRow;
double        FontCellHeight;
int           FontHeight;

if (PixelHeight <= 0)
   *lpdfRowHeight = 0;

else
   {
   PixelHeight -= lpSS->lpBook->dBorderSize;

   switch (lpSS->lpBook->dUnitType)
      {
      case SS_UNITTYPE_NORMAL:
      case SS_UNITTYPE_DEFAULT:
         Font = SS_GetFont(lpSS->lpBook, &FontTemp, lpSS->DefaultFontId);

         FontHeight = Font->FontHeight;
         FontCellHeight = Font->FontCellHeight;

         if (lpRow = SS_LockRowItem(lpSS, Row))
            {
            Font = SS_GetFont(lpSS->lpBook, &FontTemp, lpRow->RowMaxFontId);

            if (Font && Font->FontHeight != SS_HEIGHT_DEFAULT)
               {
               FontHeight = Font->FontHeight;
               FontCellHeight = Font->FontCellHeight;
               }

            SS_UnlockRowItem(lpSS, Row);
            }

         if (lpSS->lpBook->dUnitType == SS_UNITTYPE_NORMAL)
            *lpdfRowHeight = ((double)PixelHeight * FontCellHeight /
                             (double)FontHeight);
         else
            *lpdfRowHeight = ((double)PixelHeight * FontCellHeight  *
                             (double)YPIXELSPERINCH / ((double)FontHeight *
                             (double)lpSS->lpBook->dyPixelsPerInch));

         break;

      case SS_UNITTYPE_TWIPS:
         *lpdfRowHeight = ((double)PixelHeight * 1440.0) /
                          (double)lpSS->lpBook->dyPixelsPerInch;
         break;
      }
   }
}


short SS_GetCellPosX(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD Col)
{
SS_COORD i;
short    x;

for (i = 0, x = -1; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen && i < Col; i++)
   x += SS_GetColWidthInPixels(lpSS, i);

//if (Col == ColLeft - 1)
//   x -= SS_GetColWidthInPixels(lpSS, Col);
//else
   for (i = max(i, ColLeft); i < Col; i++)
      x += SS_GetColWidthInPixels(lpSS, i);

return (x);
}


int SS_GetCellOffsetX(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD Col, LPSSX_PRINTFORMAT lpPFormat, SS_COORD LeftCol)
{
SS_COORD i;
int      iWidth;
int      x = 0;

if (lpPFormat)
	{
	for (i = ColLeft; i < Col; i++)
		{
		iWidth = SS_GetColWidthFromList(lpSS, lpPFormat, LeftCol, i);
		if (iWidth == -1)
			{
			iWidth = SS_GetColWidthInPixels(lpSS, i);
			iWidth = (int)SS_DFSCREEN2PRINTERX(lpPFormat, iWidth);
			}

		x += iWidth;
		}
	}
else
	{
	for (i = ColLeft; i < Col; i++)
		x += SS_GetColWidthInPixels(lpSS, i);
	}

return (x);
}


int SS_GetCellOffsetY(LPSPREADSHEET lpSS, SS_COORD RowTop, SS_COORD Row, LPSSX_PRINTFORMAT lpPFormat, SS_COORD TopRow)
{
SS_COORD i;
int      iHeight;

if (lpPFormat)
	{
	// RFW - 2/13/07 - 19769
	double dfHeight;
	double dfy = 0;

	for (i = RowTop; i < Row; i++)
		{
		dfHeight = (double)SS_GetRowHeightFromList(lpSS, lpPFormat, TopRow, i);
		if (dfHeight == -1)
			{
			iHeight = SS_GetRowHeightInPixels(lpSS, i);
			dfHeight = SS_DFSCREEN2PRINTERY(lpPFormat, iHeight);
			}

		dfy += dfHeight;
		}

	return ((int)dfy);
	}
else
	{
	int y = 0;

	for (i = RowTop; i < Row; i++)
		y += SS_GetRowHeightInPixels(lpSS, i);

	return (y);
	}
}


short SS_GetCellPosXExt(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD Col,
                        LPRECT lpRect)
{
SS_COORD      i;
short         x;

for (i = 0, x = -1; i < lpSS->Col.HeaderCnt + lpSS->Col.Frozen && i < Col; i++)
   x += SS_GetColWidthInPixels(lpSS, i);

//if (Col == ColLeft - 1)
//   x -= SS_GetColWidthInPixels(lpSS, Col);
//else
   for (i = max(i, ColLeft); i < Col; i++)
      {
      x += SS_GetColWidthInPixels(lpSS, i);
      if (x > lpRect->right - lpRect->left)
        break;
      }

return (x);
}


short SS_GetCellPosY(LPSPREADSHEET lpSS, SS_COORD RowTop, SS_COORD Row)
{
SS_COORD      i;
short         y;

for (i = 0, y = -1; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen && i < Row; i++)
   y += SS_GetRowHeightInPixels(lpSS, i);

//if (Row == RowTop - 1)
//   y -= SS_GetRowHeightInPixels(lpSS, Row);
//else
   for (i = max(i, RowTop); i < Row; i++)
      y += SS_GetRowHeightInPixels(lpSS, i);

return (y);
}


short SS_GetCellPosYExt(LPSPREADSHEET lpSS, SS_COORD RowTop, SS_COORD Row,
                        LPRECT lpRect)
{
SS_COORD i;
short    y;

for (i = 0, y = -1; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen && i < Row; i++)
   y += SS_GetRowHeightInPixels(lpSS, i);

//if (Row == RowTop - 1)
//   y -= SS_GetRowHeightInPixels(lpSS, Row);
//else
   for (i = max(i, RowTop); i < Row; i++)
      {
      y += SS_GetRowHeightInPixels(lpSS, i);
      if (y > lpRect->bottom - lpRect->top)
         break;
      }

return (y);
}


BOOL SS_GetCellCoord(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD RowTop, SS_COORD Col,
                     SS_COORD Row, LPINT lpx, LPINT lpy, LPINT lpcx, LPINT lpcy)
{
*lpx  = SS_GetCellPosX(lpSS, ColLeft, Col);
*lpy  = SS_GetCellPosY(lpSS, RowTop, Row);
*lpcx = SS_GetColWidthInPixels(lpSS, Col);
*lpcy = SS_GetRowHeightInPixels(lpSS, Row);

return (TRUE);
}


BOOL SS_GetCellRangeCoord(LPSPREADSHEET lpSS, SS_COORD ColLeft, SS_COORD RowTop, SS_COORD Col1,
                          SS_COORD Row1, SS_COORD Col2, SS_COORD Row2, LPRECT lpRect, LPINT lpx, LPINT lpy,
                          LPINT lpcx, LPINT lpcy)
{
*lpx  = SS_GetCellPosX(lpSS, ColLeft, Col1);
*lpy  = SS_GetCellPosY(lpSS, RowTop, Row1);

if (lpRect)
	{
	*lpcx  = SS_GetCellPosXInRect(lpSS, ColLeft, Col2 + 1, lpRect) - *lpx;
	*lpcy  = SS_GetCellPosYInRect(lpSS, RowTop, Row2 + 1, lpRect) - *lpy;
	}
else
	{
	*lpcx  = SS_GetCellPosX(lpSS, ColLeft, Col2 + 1) - *lpx;
	*lpcy  = SS_GetCellPosY(lpSS, RowTop, Row2 + 1) - *lpy;
	}

return (TRUE);
}


// Take Spans into account
BOOL SS_GetCellActualSize(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPINT lpcx, LPINT lpcy)
{
#ifdef SS_V40
SS_COORD lSpanCols, lSpanRows;

if (SS_GetCellSpan(lpSS, Col, Row, NULL, NULL, &lSpanCols, &lSpanRows) == SS_SPAN_ANCHOR)
	{
	*lpcx  = SS_GetCellOffsetX(lpSS, Col, Col + lSpanCols, NULL, 0);
	*lpcy  = SS_GetCellOffsetY(lpSS, Row, Row + lSpanRows, NULL, 0);
	}
else
#endif
	{
	*lpcx = SS_GetColWidthInPixels(lpSS, Col);
	*lpcy = SS_GetRowHeightInPixels(lpSS, Row);
	}

return (TRUE);
}


BOOL SS_HighlightCell(LPSPREADSHEET lpSS, BOOL Highlight)
{
HDC  hDC;
HWND hWnd = lpSS->lpBook->hWnd;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
     lpSS->lpBook->fInvalidated = TRUE;
     return TRUE;  // safe to return TRUE since return value is not checked ?
   }

if (!SS_IsActiveSheet(lpSS))
	return (FALSE);

//if (lpSS->FreezeHighlight || lpSS->HighlightOn == (unsigned)Highlight ||
//    (Highlight && GetFocus() != lpSS->lpBook->hWnd && !lpSS->fRetainSelBlock) ||
//    lpSS->lpBook->EditModeTurningOff || lpSS->Col.Max == 0 ||
//    lpSS->Row.Max == 0 || lpSS->wOpMode == SS_OPMODE_READONLY)
if (lpSS->FreezeHighlight || lpSS->HighlightOn == (unsigned)Highlight ||
    (Highlight && GetFocus() != lpSS->lpBook->hWnd && !lpSS->lpBook->fRetainSelBlock &&
     !SS_USESINGLESELBAR(lpSS)) ||    // BJO 3Apr96 SEL3553
    lpSS->lpBook->EditModeTurningOff || lpSS->Col.Max == 0 ||
    lpSS->Row.Max == 0 || lpSS->wOpMode == SS_OPMODE_READONLY)
   {

#ifdef SS_V80
	if (lpSS->wOpMode == SS_OPMODE_READONLY && !lpSS->lpBook->fPaintingToScreen)
		SS_InvalidateActiveHeaders(lpSS);
#endif
    return (FALSE);
   }

hDC = SS_GetDC(lpSS->lpBook);
SSx_HighlightCell(lpSS, hDC, Highlight);

if (SS_IsDestroyed(hWnd))
   return (FALSE);

SS_ReleaseDC(lpSS->lpBook, hDC);

return (TRUE);
}


BOOL SSx_HighlightCell(LPSPREADSHEET lpSS, HDC hDC, BOOL Highlight)
{
RECT RclBound;
RECT Rect;
BOOL fNormal;
int  x;
int  y;
int  cx;
int  cy;

if (!lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   {
	lpSS->lpBook->fInvalidated = TRUE;
	return TRUE;  // safe to return TRUE since return value is not checked ?
   }

if (!SS_IsActiveSheet(lpSS))
	return (FALSE);

/* RFW - 6/24/03 - 12119 
if (lpSS->Row.Max == 0 || lpSS->wOpMode == SS_OPMODE_READONLY || lpSS->Row.CurAt < lpSS->Row.HeaderCnt ||
    lpSS->Col.CurAt < lpSS->Col.HeaderCnt)
*/
if (lpSS->Row.Max == 0 || lpSS->wOpMode == SS_OPMODE_READONLY)
   return (FALSE);

if (lpSS->lpBook->wMode == SS_MODE_COLMOVE ||
	 lpSS->lpBook->wMode == SS_MODE_BEGINCOLMOVE ||
	 lpSS->lpBook->wMode == SS_MODE_ROWMOVE ||
	 lpSS->lpBook->wMode == SS_MODE_BEGINROWMOVE)
   return (FALSE);

/* RFW - 6/25/04 - 14305 - I moved this below
#ifdef SS_V70
if (Highlight && lpSS->bActiveCellHighlightStyle == SS_ACTIVECELLHIGHLIGHTSTYLE_OFF)
	return (FALSE);
#endif // SS_V70
*/

if (SS_USESELBAR(lpSS))
   {
   if (!SS_IsRowVisible(lpSS, lpSS->Row.CurAt, SS_VISIBLE_PARTIAL))
      {
      if (!Highlight)
         {
         lpSS->HighlightOn = Highlight;
         lpSS->lpBook->fFocusHighlightOn = Highlight;
         }

      return (FALSE);
      }
   }

#ifdef SS_GP
else if (!SS_IsCellVisible(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt,
                           SS_VISIBLE_PARTIAL))
#else
else if (!lpSS->lpBook->fEditModePermanent && !SS_IsCellVisible(lpSS,
         lpSS->Col.CurAt, lpSS->Row.CurAt, SS_VISIBLE_PARTIAL))
#endif
   {
	// RFW - 6/22/04 - 14599
	if (!Highlight)
      {
      lpSS->HighlightOn = Highlight;
      lpSS->lpBook->fFocusHighlightOn = Highlight;
      }

   return (FALSE);
   }

else if (lpSS->lpBook->fEditModePermanent)
   {
	SS_COORD ColEdit;
	SS_COORD RowEdit;

   //if (!SS_IsCellVisible(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt,
   //                      SS_VISIBLE_ALL) && (lpSS->Col.CurAt != lpSS->Col.UL ||
   //                      lpSS->Col.UL != lpSS->Col.LR) &&
   //                      (lpSS->Row.CurAt != lpSS->Row.UL ||
   //                      lpSS->Row.UL != lpSS->Row.LR))
	SS_GetCellBeingEdited(lpSS, &ColEdit, &RowEdit);

   if ((!SS_IsCellVisible(lpSS, ColEdit, RowEdit, SS_VISIBLE_ALL)) &&
       (!SS_IsRowVisible(lpSS, RowEdit, SS_VISIBLE_ALL) ||
        ColEdit != lpSS->Col.UL ||
        lpSS->Col.UL != lpSS->Col.LR) &&
       (!SS_IsColVisible(lpSS, ColEdit, SS_VISIBLE_ALL) ||
        RowEdit != lpSS->Row.UL ||
        lpSS->Row.UL != lpSS->Row.LR) &&
       (ColEdit != lpSS->Col.UL ||
        lpSS->Col.UL != lpSS->Col.LR ||
        RowEdit != lpSS->Row.UL ||
        lpSS->Row.UL != lpSS->Row.LR))
      {
      return (FALSE);
      }
   }

if (lpSS->lpBook->fEditModePermanent && !SS_USESELBAR(lpSS))
   {
#ifdef SS_UTP
   if (lpSS->fAllowEditModePermSel && SS_IsBlockSelected(lpSS))
      lpSS->HighlightOn = FALSE;
	else 
#endif

   if (Highlight)
      {
      if (!lpSS->lpBook->EditModeOn && !lpSS->lpBook->EditModeTurningOn &&
//          !lpSS->wMessageBeingSent && lpSS->lpBook->hWnd == GetFocus()) // RFW - GRB316 - 1/22/96
          lpSS->lpBook->hWnd == GetFocus())
         {
         lpSS->HighlightOn = Highlight;

         if (lpSS->lLButtonDownlParam)
            SS_CellEditModeOn(lpSS, WM_LBUTTONDOWN, 0,
                              lpSS->lLButtonDownlParam);
			// RFW - 10/20/03 - 12451
         else if (lpSS->lpBook->wMessageBeingSent || lpSS->lpBook->fPaintingToScreen)  // RFW - GRB316 - 1/22/96
            SS_PostSetEditMode(lpSS, TRUE, 0);
         else
            SS_CellEditModeOn(lpSS, 0, 0, 0L);
         }

#ifdef SS_V80
      if (!lpSS->lpBook->fPaintingToScreen)
         SS_InvalidateActiveHeaders(lpSS);
#endif
      }

   else
      lpSS->HighlightOn = Highlight;

   return (FALSE);
   }

fNormal = lpSS->wOpMode == SS_OPMODE_NORMAL ||
          (lpSS->wOpMode == SS_OPMODE_ROWMODE &&
          lpSS->fRowModeEditing);
//Modify By BOC 99.6.18 (hyt)--------------------------------
//for Draw Error if set current col or row hidden or decrease
//width or heigh to 0,old bug seem fixed by farpoint at other
//position
#ifdef SPREAD_JPN
//- JPNFIX0015 - (Masanori Iwasa)
//- Adjust columns or rows when frozen
//SS_AdjustHiddenRowCol(lpSS, &lpSS->Col, FALSE);
//SS_AdjustHiddenRowCol(lpSS, &lpSS->Row, TRUE );
#endif
//------------------------------------------------------------

/*
if ((SS_USESINGLESELBAR(lpSS) ||
    (lpSS->wOpMode == SS_OPMODE_EXTSEL &&
    !SSSelModeSendMessage(hWnd, SS_SELMODE_GETSEL, lpSS->Row.CurAt, 0L, 0L))) &&
    !lpSS->fProcessingKillFocus)
*/

if (SS_USESELBAR(lpSS) && !SS_INVERTSEL(lpSS) && lpSS->lpBook->fPaintingToScreen)
   ;
else if (SS_USESINGLESELBAR(lpSS) && !lpSS->lpBook->fProcessingKillFocus)
   SS_InvertRow(lpSS->lpBook->hWnd, hDC, lpSS);

if (lpSS->FreezeHighlight || lpSS->HighlightOn == (unsigned)Highlight ||
    (Highlight && GetFocus() != lpSS->lpBook->hWnd && !lpSS->lpBook->fRetainSelBlock &&
     !SS_USESINGLESELBAR(lpSS)) ||    // BJO 3Apr96 SEL3553
    lpSS->lpBook->EditModeTurningOff || lpSS->Col.Max == 0 ||
    lpSS->Row.Max == 0 || lpSS->wOpMode == SS_OPMODE_READONLY)
   {
   return (FALSE);
   }

// RFW - 7/25/04 - 14895 - 14315 - 15192
if (Highlight && SS_USESELBAR(lpSS) && GetFocus() != lpSS->lpBook->hWnd)
    lpSS->lpBook->fFocusHighlightOn = FALSE;

else if (SS_USESELBAR(lpSS) && lpSS->lpBook->fFocusHighlightOn == (unsigned)Highlight)
	;

#if 0
else if (SS_USESELBAR(lpSS) && (lpSS->lpBook->fFocusHighlightOn == (unsigned)Highlight ||
    lpSS->lpBook->fKillFocusReceived))
    /* RFW - 7/25/04 - 14895
    (SS_USESINGLESELBAR(lpSS) && lpSS->lpBook->fKillFocusReceived)))
    */
   lpSS->lpBook->fFocusHighlightOn = FALSE; // RFW - 6/24/04 - 14315
#endif
else if ((SS_USESELBAR(lpSS) || fNormal) && !lpSS->fNoDrawFocusRect)
   {
   if (SS_USESELBAR(lpSS))
      lpSS->lpBook->fFocusHighlightOn = Highlight;

#ifdef SS_V70
	if (lpSS->bActiveCellHighlightStyle == SS_ACTIVECELLHIGHLIGHTSTYLE_OFF)
		{
		if (!lpSS->lpBook->fProcessingKillFocus)
			lpSS->HighlightOn = Highlight;

		return (FALSE);
		}
#endif // SS_V70

   /*
   if (lpSS->fKillFocusReceived)
      lpSS->fKillFocusReceived = FALSE;
   */

   SS_GetClientRect(lpSS->lpBook, &Rect);
   CopyRect(&RclBound, &Rect);

#if 0
   /*************************
   * Calculate Row position
   *************************/

   if (lpSS->Row.hCellSizeList)
      {
      lpRowHeightList = (LPSHORT)tbGlobalLock(lpSS->Row.hCellSizeList);

      for (i = 0, y = -1, ListAt = 0; i < lpSS->Row.HeaderCnt +
           lpSS->Row.Frozen && i < lpSS->Row.CurAt; i++, ListAt++)
         y += lpRowHeightList[ListAt];

      for (i = max(i, lpSS->Row.UL); i < lpSS->Row.CurAt;
           i++, ListAt++)
         y += lpRowHeightList[ListAt];

      cy = lpRowHeightList[ListAt];
      tbGlobalUnlock(lpSS->Row.hCellSizeList);
      }

   else
      {
      for (i = 0, y = -1; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
           i < lpSS->Row.CurAt; i++)
         y += SS_GetRowHeightInPixels(lpSS, i);

      for (i = max(i, lpSS->Row.UL); i < lpSS->Row.CurAt; i++)
         y += SS_GetRowHeightInPixels(lpSS, i);

      cy = SS_GetRowHeightInPixels(lpSS, lpSS->Row.CurAt);
      }

   /*************************
   * Calculate Col position
   *************************/

   if (fNormal)
      {
      if (lpSS->Col.hCellSizeList)
         {
         lpColWidthList = (LPSHORT)tbGlobalLock(lpSS->Col.hCellSizeList);

         for (i = 0, x = -1, ListAt = 0; i < lpSS->Col.HeaderCnt +
              lpSS->Col.Frozen && i < lpSS->Col.CurAt; i++, ListAt++)
            x += lpColWidthList[ListAt];

         for (i = max(i, lpSS->Col.UL); i < lpSS->Col.CurAt;
              i++, ListAt++)
            x += lpColWidthList[ListAt];

         cx = lpColWidthList[ListAt];

         tbGlobalUnlock(lpSS->Col.hCellSizeList);
         }

      else
         {
         for (i = 0, x = -1; i < lpSS->Col.HeaderCnt +
              lpSS->Col.Frozen && i < lpSS->Col.CurAt; i++)
            x += SS_GetColWidthInPixels(lpSS, i);

         for (i = max(i, lpSS->Col.UL); i < lpSS->Col.CurAt; i++)
            x += SS_GetColWidthInPixels(lpSS, i);

         cx = SS_GetColWidthInPixels(lpSS, lpSS->Col.CurAt);
         }
      }

   else
      {
      x = SS_GetCellPosX(lpSS, lpSS->Col.HeaderCnt, lpSS->Col.HeaderCnt);
      cx = min(SS_GetCellPosX(lpSS, lpSS->Col.UL,
               lpSS->Col.LR) + SS_GetColWidthInPixels(lpSS,
               lpSS->Col.LR), Rect.right) - x;
      }
#endif // 0

   if (fNormal)
		{
#ifdef SS_V40
//		SS_COORD lSpanCols, lSpanRows;
//		if (SS_GetCellSpan(lpSS, CellCol, CellRow, NULL, NULL, &lSpanCols, &lSpanRows) == SS_SPAN_ANCHOR)
//			SS_InvalidateCellRange(lpSS, CellCol, CellRow, CellCol + lSpanCols - 1,
//	                             CellRow + lSpanRows - 1);
#endif // SS_V40
	   SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, lpSS->Col.CurAt, lpSS->Row.CurAt,
	                   &x, &y, &cx, &cy);
		}
	else
	   SS_GetCellRangeCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, lpSS->Col.HeaderCnt, lpSS->Row.CurAt,
                           lpSS->Col.LR, lpSS->Row.CurAt, &Rect, &x, &y, &cx, &cy);

	
	// RFW - 8/9/02
//	   SS_GetCellRangeCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL, lpSS->Col.UL, lpSS->Row.CurAt,
//                           lpSS->Col.LR, lpSS->Row.CurAt, &Rect, &x, &y, &cx, &cy);

   x += Rect.left;
   y += Rect.top;

   if ((lpSS->Row.CurAt == lpSS->Row.UL && lpSS->Row.Frozen == 0) ||
        lpSS->Row.CurAt == lpSS->Row.HeaderCnt)
      RclBound.top = max(RclBound.top, y + 1);
   else
      RclBound.top = max(RclBound.top, y - 1);

   if (!fNormal ||
       ((lpSS->Col.CurAt == lpSS->Col.UL && lpSS->Col.Frozen == 0) ||
       lpSS->Col.CurAt == lpSS->Col.HeaderCnt))
      RclBound.left = max(RclBound.left, x + 1);
   else
      RclBound.left = max(RclBound.left, x - 1);

   if (lpSS->Row.CurAt == SS_GetRowCnt(lpSS) - 1)
      RclBound.bottom = min(RclBound.bottom, y + cy);
   else
      RclBound.bottom = min(RclBound.bottom, y + cy + 2);

   if (!fNormal || lpSS->Col.CurAt == SS_GetColCnt(lpSS) - 1)
      RclBound.right = min(RclBound.right, x + cx);
   else
      RclBound.right = min(RclBound.right, x + cx + 2);

   if (SS_USESELBAR(lpSS) && !SS_INVERTSEL(lpSS))
      {
      RECT RectTemp;
      SetRect(&RectTemp, x + 1, y + 1, x + cx, y + cy);

      if (Highlight)
         {
         if (!lpSS->lpBook->fPaintingToScreen)
            UpdateWindow(lpSS->lpBook->hWnd);
         SS_FocusRect(lpSS, hDC, &RectTemp, 0);
         }
      else
			{
         if (!lpSS->lpBook->fPaintingToScreen)
           SS_InvertRow(lpSS->lpBook->hWnd, hDC, lpSS);
         SS_FocusRect(lpSS, hDC, &RectTemp, 0); // RICHARD
         }
      }
   else
      {
      SS_ReverseArea(hDC, x + 2, y - 1, cx - 3, 3, &RclBound);
      SS_ReverseArea(hDC, x - 1, y - 1, 3, cy + 3, &RclBound);
      SS_ReverseArea(hDC, x + 2, y + cy - 1, cx - 3, 3, &RclBound);
      SS_ReverseArea(hDC, x + cx - 1, y - 1, 3, cy + 3, &RclBound);
      }
   }

if (!lpSS->lpBook->fProcessingKillFocus)
   lpSS->HighlightOn = Highlight;

#ifdef SS_V80
if (!lpSS->lpBook->fPaintingToScreen)
	{
	BOOL fRedraw = lpSS->lpBook->Redraw;
	lpSS->lpBook->Redraw = FALSE;
	SS_InvalidateActiveHeaders(lpSS);
	lpSS->lpBook->Redraw = fRedraw;
	}
#endif // SS_V80

return (TRUE);
}


#if 0
#ifdef SPREAD_JPN
//---------------------------------------------------------------------
// This function adjusts the row or column when it's frozen and picks
// up the closest one.
// JPNFIX0015 - (Masanori Iwasa)
//---------------------------------------------------------------------
void SS_AdjustHiddenRowCol(LPSPREADSHEET lpSS, LPSS_ROWCOL Target, BOOL fRow)
{
    int         fOk;
    SS_COORD    nAdjust = 0;
    SS_COORD    nMax, nMin;

    fOk = SSx_AdjustHiddenRowCol(lpSS, Target->CurAt, fRow);
    while (!fOk)
        {
        nAdjust++;
        nMax = Target->CurAt + nAdjust;
        nMin = Target->CurAt - nAdjust;
 
        //- Check if there's any nearby downwards.
        if(nMax < Target->Max)
            fOk = SSx_AdjustHiddenRowCol(lpSS, nMax, fRow);
        
        //- Check if there's any nearby upwards.
        if(!fOk && nMin > 0)
            fOk = SSx_AdjustHiddenRowCol(lpSS, nMin, fRow) ? 2 : 0;
        
        //- Check to see if all the rest are hidden.
        if(nMax >= Target->Max && nMin <= 0 )
            {
            nAdjust = 0;
            fOk = TRUE;
            }
        }
    Target->CurAt = (fOk == 1) ? Target->CurAt + nAdjust : Target->CurAt - nAdjust;
}

BOOL SSx_AdjustHiddenRowCol(LPSPREADSHEET lpSS, SS_COORD nCell, BOOL fRow)
{
    double  dfVal;

    if(fRow)
        SS_GetRowHeight(lpSS, nCell, &dfVal);
    else
        SS_GetColWidth(lpSS, nCell, &dfVal);
    
    return (dfVal ? TRUE : FALSE);
}
#endif
#endif // 0


/*
void SS_HighlightDatePermanent(hWnd)

HWND          hWnd;
{
LPSPREADSHEET lpSS;

lpSS = SS_Lock(hWnd);

if (lpSS->FreezeHighlight || lpSS->HighlightOn == TRUE ||
    (GetFocus() != hWnd && !lpSS->fRetainSelBlock) ||
    lpSS->lpBook->EditModeTurningOff || lpSS->Col.Max == 0 ||
    lpSS->Row.Max == 0 || lpSS->wOpMode == SS_OPMODE_READONLY)
   {
   SS_Unlock(hWnd);
   return;
   }

else if (!SS_IsCellVisible(lpSS, lpSS->Col.CurAt, lpSS->Row.CurAt,
         SS_VISIBLE_PARTIAL))
   {
   SS_Unlock(hWnd);
   return;
   }

if (lpSS->fEditModePermanent && !SS_USESELBAR(lpSS))
   {
   lpSS->HighlightOn = TRUE;
   if (!lpSS->lpBook->EditModeOn && !lpSS->lpBook->EditModeTurningOn &&
       !lpSS->wMessageBeingSent)
      SS_CellEditModeOn(lpSS, WM_LBUTTONDOWN, 0, 0L);
   }

SS_Unlock(hWnd);
}
*/


static void SS_ReverseArea(hDC, x, y, cx, cy, RclBound)

HDC    hDC;
int    x;
int    y;
int    cx;
int    cy;
LPRECT RclBound;
{
RECT   Rect;

Rect.left   = max(x, RclBound->left);
Rect.right  = min(x + cx, RclBound->right);
Rect.top    = max(y, RclBound->top);
Rect.bottom = min(y + cy, RclBound->bottom);

if (Rect.left < Rect.right && Rect.top < Rect.bottom)
   InvertRect(hDC, &Rect);
}


SS_COORD SS_GetBottomVisCell(lpSS, RowTop)

LPSPREADSHEET lpSS;
SS_COORD      RowTop;
{
RECT          Rect;
SS_COORD      RowAt;
SS_COORD      MaxRows;
short         y;
short         cy;

SS_GetClientRect(lpSS->lpBook, &Rect);
MaxRows = SS_GetRowCnt(lpSS);

y = SS_GetCellPosY(lpSS, RowTop, RowTop);
for (cy = 0, RowAt = RowTop; RowAt < MaxRows &&
     y + cy <= Rect.bottom - Rect.top; RowAt++)
   {
   y += cy;
   cy = SS_GetRowHeightInPixels(lpSS, RowAt);

   if (y + cy > Rect.bottom - Rect.top)
      break;
   }

RowAt--;

if (RowAt < RowTop)
   RowAt = -1;

return (RowAt);
}


SS_COORD SS_GetBottomCell(lpSS, RowTop)

LPSPREADSHEET lpSS;
SS_COORD      RowTop;
{
RECT          Rect;
SS_COORD      RowAt;
SS_COORD      MaxRows;
short         y;

SS_GetClientRect(lpSS->lpBook, &Rect);
MaxRows = SS_GetRowCnt(lpSS);

y = SS_GetCellPosY(lpSS, RowTop, RowTop);
for (RowAt = RowTop; RowAt < MaxRows && y < Rect.bottom - Rect.top; RowAt++)
   y += SS_GetRowHeightInPixels(lpSS, RowAt);

RowAt--;

if (RowAt < RowTop)
   RowAt = -1;

return (RowAt);
}


SS_COORD SS_GetRightVisCell(lpSS, ColLeft)

LPSPREADSHEET lpSS;
SS_COORD      ColLeft;
{
RECT          Rect;
SS_COORD      ColAt;
SS_COORD      MaxCols;
short         x;
short         cx;

SS_GetClientRect(lpSS->lpBook, &Rect);
MaxCols = SS_GetColCnt(lpSS);

x = SS_GetCellPosX(lpSS, ColLeft, ColLeft);
for (cx = 0, ColAt = ColLeft;
     ColAt < MaxCols && x + cx <= Rect.right - Rect.left; ColAt++)
   {
   x += cx;
   cx = SS_GetColWidthInPixels(lpSS, ColAt);

   if (x + cx > Rect.right - Rect.left)
      break;
   }

ColAt--;

if (ColAt < ColLeft)
   ColAt = -1;

return (ColAt);
}


SS_COORD SS_GetRightCell(LPSPREADSHEET lpSS, SS_COORD ColLeft)
{
RECT          Rect;
SS_COORD      ColAt;
SS_COORD      MaxCols;
short         x;

SS_GetClientRect(lpSS->lpBook, &Rect);
MaxCols = SS_GetColCnt(lpSS);

x = SS_GetCellPosX(lpSS, ColLeft, ColLeft);
for (ColAt = ColLeft; ColAt < MaxCols && x < Rect.right - Rect.left; ColAt++)
   x += SS_GetColWidthInPixels(lpSS, ColAt);

ColAt--;

if (ColAt < ColLeft)
   ColAt = -1;

return (ColAt);
}


#if (!defined(SS_NOCT_BUTTON) || !defined(SS_NOCT_COMBO))
#ifdef SS_V80
long SS_BtnDraw(HWND hWnd, HDC hDC, LPRECT lpRect, LPSS_CELLTYPE lpCellType,
                HFONT hFont, BOOL fButtonDown, BOOL fRoundCorners,
                BOOL fUseDib, LPRECT lpRectOrig, BOOL fIsButton, BOOL fIs95,
                BOOL fCalcSize, BOOL fMouseOver, BOOL fEnhanced)
#else
long SS_BtnDraw(HWND hWnd, HDC hDC, LPRECT lpRect, LPSS_CELLTYPE lpCellType,
                HFONT hFont, BOOL fButtonDown, BOOL fRoundCorners,
                BOOL fUseDib, LPRECT lpRectOrig, BOOL fIsButton, BOOL fIs95,
                BOOL fCalcSize)
#endif
{
SUPERBTN SuperBtn;
LPTSTR   lpText;
BOOL     fSetBtnUp = FALSE;
BOOL     fSetBtnDown = FALSE;
long     lRet = 0;

_fmemset(&SuperBtn, '\0', sizeof(SUPERBTN));

SuperBtn.hText          = lpCellType->Spec.Button.hText;
SuperBtn.hBrushFocus    = 0;
SuperBtn.hBitmapDC      = 0;
SuperBtn.hFont          = hFont;
SuperBtn.lStyle         = lpCellType->Style;
SuperBtn.ShadowSize     = lpCellType->Spec.Button.ShadowSize;
SuperBtn.BtnFrameOffset = 0;
SuperBtn.ButtonType     = 0;
SuperBtn.ButtonDown     = fButtonDown;
SuperBtn.MouseCaptureOn = FALSE;
SuperBtn.fStretch       = FALSE;
SuperBtn.fIs95          = fIs95;
#ifdef SS_V80
SuperBtn.MouseOver      = fMouseOver;
SuperBtn.Enhanced       = fEnhanced;
#endif

if ((!fButtonDown || !lpCellType->Spec.Button.hPictDownName || fCalcSize) &&
     lpCellType->Spec.Button.hPictName)
   {
   if (lpCellType->Spec.Button.nPictureType & SUPERBTN_PICT_HANDLE)
      SuperBtnSetPict(0, 0, &SuperBtn.Pict,
                      (WORD)lpCellType->Spec.Button.nPictureType,
                      (LPARAM)lpCellType->Spec.Button.hPictName);
   else
      {
      fSetBtnUp = TRUE;
      lpText = (LPTSTR)GlobalLock(lpCellType->Spec.Button.hPictName);

      SuperBtnSetPict(0, GetWindowInstance(hWnd), &SuperBtn.Pict,
                      (WORD)lpCellType->Spec.Button.nPictureType,
                      (LPARAM)lpText);

      GlobalUnlock(lpCellType->Spec.Button.hPictName);
      }
   }

else if ((fButtonDown || fCalcSize) && lpCellType->Spec.Button.hPictDownName)
   {
   if (lpCellType->Spec.Button.nPictureDownType & SUPERBTN_PICT_HANDLE)
      SuperBtnSetPict(0, 0, &SuperBtn.PictBtnDown,
                      (WORD)lpCellType->Spec.Button.nPictureDownType,
                      (LPARAM)lpCellType->Spec.Button.hPictDownName);
   else
      {
      fSetBtnDown = TRUE;
      lpText = (LPTSTR)GlobalLock(lpCellType->Spec.Button.hPictDownName);

      SuperBtnSetPict(0, GetWindowInstance(hWnd),
                      &SuperBtn.PictBtnDown,
                      (WORD)lpCellType->Spec.Button.nPictureDownType,
                      (LPARAM)lpText);

      GlobalUnlock(lpCellType->Spec.Button.hPictDownName);
      }
   }

if (fCalcSize)
   {
   SIZE  Size = {0, 0};
   short nWidth = 0;

   if (SuperBtn.hText)
      {
      LPTSTR lpszText = (LPTSTR)GlobalLock(SuperBtn.hText);
      GetTextExtentPoint(hDC, lpszText, lstrlen(lpszText), &Size);
      GlobalUnlock(SuperBtn.hText);
      }

   if ((SuperBtn.lStyle & SBS_TEXTLEFT_PICTRIGHT) ||
       (SuperBtn.lStyle & SBS_TEXTRIGHT_PICTLEFT))
      {
      Size.cx += max(SuperBtn.Pict.PictWidth,
                     SuperBtn.PictBtnDown.PictWidth);
      Size.cy = max(Size.cy, max(SuperBtn.Pict.PictHeight,
                    SuperBtn.PictBtnDown.PictHeight));
      }

   else
      {
      Size.cx = max(Size.cx, max(SuperBtn.Pict.PictWidth,
                    SuperBtn.PictBtnDown.PictWidth));
      Size.cy += max(SuperBtn.Pict.PictHeight,
                     SuperBtn.PictBtnDown.PictHeight);
      }

   if (SuperBtn.fIs95)
      nWidth = max(1, SuperBtn.ShadowSize / 2) +
               max(1, DIVUP(SuperBtn.ShadowSize, 2));
   else
      nWidth = SuperBtn.ShadowSize;

   nWidth *= 2;
   lRet = MAKELONG(Size.cx + nWidth + 1, Size.cy + nWidth);
   }
else
   {
   _fmemcpy(&SuperBtn.Color, &lpCellType->Spec.Button.Color,
            sizeof(SUPERBTNCOLOR));
   SuperBtn.Color.Color = SS_TranslateColor(SuperBtn.Color.Color);
   SuperBtn.Color.ColorBorder = SS_TranslateColor(SuperBtn.Color.ColorBorder);
   SuperBtn.Color.ColorShadow = SS_TranslateColor(SuperBtn.Color.ColorShadow);
   SuperBtn.Color.ColorHighlight = SS_TranslateColor(SuperBtn.Color.ColorHighlight);
   SuperBtn.Color.ColorText = SS_TranslateColor(SuperBtn.Color.ColorText);

   SuperBtnPaint(hWnd, &SuperBtn, hDC, lpRect, TRUE, FALSE, fRoundCorners,
                 fUseDib, lpRectOrig, fIsButton);
   }

if (fSetBtnUp)
   SuperBtnFreePict(&SuperBtn.Pict);

if (fSetBtnDown)
   SuperBtnFreePict(&SuperBtn.PictBtnDown);

return (lRet);
}

#endif


#ifndef SS_NOCT_CHECK
#ifdef SS_V80
long SS_CheckBoxDraw(hWnd, hWndCtrl, hDC, lpRect, lpCellType, hFont,
                     fButtonDown, fUseDib, lpRectOrig, fCalcSize, fEnhanced)
#else
long SS_CheckBoxDraw(hWnd, hWndCtrl, hDC, lpRect, lpCellType, hFont,
                     fButtonDown, fUseDib, lpRectOrig, fCalcSize)
#endif

HWND          hWnd;
HWND          hWndCtrl;
HDC           hDC;
LPRECT        lpRect;
LPSS_CELLTYPE lpCellType;
HFONT         hFont;
BOOL          fButtonDown;
BOOL          fUseDib;
LPRECT        lpRectOrig;
BOOL          fCalcSize;
{
GLOBALHANDLE  hGlobal;
LPCHECKBOX    lpCheckBox;
CHECKBOX      CheckBox;
long          lRet = 0;

lpCheckBox = CheckBoxGetPtr(hWndCtrl, &hGlobal);
_fmemset(&CheckBox, '\0', sizeof(CHECKBOX));

if (!hWndCtrl || !lpCheckBox)
	return (0);

CheckBox.hText          = lpCellType->Spec.CheckBox.hText;
CheckBox.hBrushFocus    = lpCheckBox->hBrushFocus;
CheckBox.hBitmapFocus   = lpCheckBox->hBitmapFocus;
CheckBox.hBitmapDC      = 0;
CheckBox.hFont          = hFont;
CheckBox.cPaint         = CHECKBOX_PAINTALL;
CheckBox.dCtlColorCode  = CTLCOLOR_BTN;
CheckBox.lStyle         = lpCellType->Style;
CheckBox.fFocus         = FALSE;
CheckBox.fDisabled      = FALSE;
CheckBox.fChecked       = fButtonDown;
#ifdef SS_V80
CheckBox.Enhanced		= fEnhanced;
#endif

GlobalUnlock(hGlobal);

SS_CheckBoxDrawSetPict(GetWindowInstance(hWnd), &CheckBox.Picts,
                       lpCellType->Spec.CheckBox.hPictGrayName,
                       (short)(lpCellType->Spec.CheckBox.bPictGrayType |
                               BT_PICT_GRAY));

SS_CheckBoxDrawSetPict(GetWindowInstance(hWnd), &CheckBox.Picts,
                       lpCellType->Spec.CheckBox.hPictDownName,
                       (short)(lpCellType->Spec.CheckBox.bPictDownType |
                               BT_PICT_DOWN));

SS_CheckBoxDrawSetPict(GetWindowInstance(hWnd), &CheckBox.Picts,
                       lpCellType->Spec.CheckBox.hPictUpName,
                       (short)(lpCellType->Spec.CheckBox.bPictUpType |
                       BT_PICT_UP));

if (fCalcSize)
   {
   SIZE  Size = {0, 0};

   if (CheckBox.hText)
      {
      LPTSTR lpszText = (LPTSTR)GlobalLock(CheckBox.hText);
      GetTextExtentPoint(hDC, lpszText, lstrlen(lpszText), &Size);
      GlobalUnlock(CheckBox.hText);
      }

   lRet = MAKELONG(Size.cx + BTNTEXTPICT_INC + CheckBox.Picts.dMaxPictWidth,
                   max(Size.cy, CheckBox.Picts.dMaxPictHeight));
   }
else
   CheckBoxPaint(hWndCtrl, &CheckBox, hDC, lpRect, fUseDib, lpRectOrig, FALSE);

#ifdef SS_W
CheckBoxFreePict(&CheckBox.Picts.PictUp);
CheckBoxFreePict(&CheckBox.Picts.PictDown);
CheckBoxFreePict(&CheckBox.Picts.PictGray);
CheckBoxFreePict(&CheckBox.Picts.PictFocusUp);
CheckBoxFreePict(&CheckBox.Picts.PictFocusDown);
CheckBoxFreePict(&CheckBox.Picts.PictFocusGray);
CheckBoxFreePict(&CheckBox.Picts.PictDisabledUp);
CheckBoxFreePict(&CheckBox.Picts.PictDisabledDown);
CheckBoxFreePict(&CheckBox.Picts.PictDisabledGray);
#endif

return (lRet);
}


void SS_CheckBoxDrawSetPict(HANDLE hInst, LPCHECKBOXPICTS lpPicts,
                            TBGLOBALHANDLE hPictName, short bPictType)
{
LPTSTR lpText;

if (bPictType & BT_HANDLE)
   CheckBoxSetPict(0, 0, lpPicts, (WORD)bPictType,
                   (LPARAM)hPictName);
else
   {
   lpText = (LPTSTR)tbGlobalLock(hPictName);
   CheckBoxSetPict(0, hInst, lpPicts, (WORD)bPictType, (LPARAM)lpText);
   tbGlobalUnlock(hPictName);
   }
}

#endif


#ifndef SS_NOCT_COMBO

#ifdef SS_V80
void SS_DrawComboBtn(hWnd, lpSS, hDC, lpRect, dButtonWidth, fButtonDown, fMouseOver, fEnhanced)
#else
void SS_DrawComboBtn(hWnd, lpSS, hDC, lpRect, dButtonWidth, fButtonDown)
#endif

HWND          hWnd;
LPSPREADSHEET lpSS;
HDC           hDC;
LPRECT        lpRect;
short         dButtonWidth;
BOOL          fButtonDown;
{
SS_CELLTYPE   CellType;
RECT          RectTemp;

if (lpSS->lpBook->fIs95)
   {
   RectTemp.left = max(lpRect->left, lpRect->right - dButtonWidth);
#if SS_V80
   if( !fEnhanced )
   {
      RectTemp.top = lpRect->top;
      RectTemp.right = lpRect->right - 1;
      RectTemp.bottom = lpRect->bottom - 1;
   }
   else
   {
      RectTemp.top = lpRect->top - 1;
      RectTemp.right = lpRect->right + 1;
      RectTemp.bottom = lpRect->bottom + 1;
   }
#else
   RectTemp.top = lpRect->top;
   RectTemp.right = lpRect->right + 1;
   RectTemp.bottom = lpRect->bottom + 1;
#endif
   }
else
   {
   RectTemp.left = max(lpRect->left, lpRect->right - dButtonWidth + 1);
   RectTemp.top = lpRect->top - 1;
   RectTemp.right = lpRect->right + 1;
   RectTemp.bottom = lpRect->bottom + 1;
   }

CellType.Style                            = 0;
CellType.Spec.Button.hText                = 0;
CellType.Spec.Button.hPictName            = lpSS->lpBook->hBitmapComboBtn;
CellType.Spec.Button.hPictDownName        = 0;
CellType.Spec.Button.nPictureType         = SUPERBTN_PICT_HANDLE |
                                            SUPERBTN_PICT_BITMAP;
CellType.Spec.Button.nPictureDownType     = 0;
CellType.Spec.Button.ButtonType           = SUPERBTN_TYPE_NORMAL;
CellType.Spec.Button.ShadowSize           = 1;
CellType.Spec.Button.Color.Color          = GetSysColor(COLOR_BTNFACE);
CellType.Spec.Button.Color.ColorBorder    = GetSysColor(COLOR_BTNTEXT);
CellType.Spec.Button.Color.ColorShadow    = GetSysColor(COLOR_BTNSHADOW);
CellType.Spec.Button.Color.ColorHighlight = GetSysColor(COLOR_BTNHIGHLIGHT);
CellType.Spec.Button.Color.ColorText      = GetSysColor(COLOR_BTNTEXT);

#ifdef SS_V80
SS_BtnDraw(hWnd, hDC, &RectTemp, &CellType, 0, fButtonDown, FALSE, FALSE,
           NULL, FALSE, lpSS->lpBook->fIs95, FALSE, fMouseOver, fEnhanced);
#else
SS_BtnDraw(hWnd, hDC, &RectTemp, &CellType, 0, fButtonDown, FALSE, FALSE,
           NULL, FALSE, lpSS->lpBook->fIs95, FALSE);
#endif
}

#endif


HFONT SS_CreatePrinterFont(LPSPREADSHEET lpSS, HDC hDCPrinter, LPSS_FONT lpFont,
                           SS_FONTID FontId, LPSSX_PRINTFORMAT lpPFormat)
{
HDC     hDC = GetDC(lpSS->lpBook->hWnd);
LOGFONT LogFont;
double  dfHeight;
short   nHeight;
//UINT    nCh;                 // SCP 12/31/98
//int FAR *lpCharWidths;       // SCP 12/31/98

_fmemcpy(&LogFont, &lpFont->LogFont, sizeof(LOGFONT));

// Calculate the exact height of the font
{
TEXTMETRIC tm;
HFONT      hFontOld = SelectObject(hDC, lpFont->hFont);

GetTextMetrics(hDC, &tm);
LogFont.lfHeight = tm.tmHeight - tm.tmInternalLeading;

SelectObject(hDC, hFontOld);
}

dfHeight = (double)abs(LogFont.lfHeight) * 72.0 / (double)lpPFormat->TextHeightScreen;
if (lpPFormat->wAction == SS_PRINT_ACTION_PREVIEW)
   nHeight = (short)((dfHeight * (double)lpPFormat->TextHeightPrinter / 72.0 *
                      lpPFormat->dfScalePreviewY * lpPFormat->dfScale));
//                      lpPFormat->dfScalePreviewY) + 0.5);
else
   nHeight = (short)((dfHeight * (double)lpPFormat->TextHeightPrinter / 72.0 *
//                      lpPFormat->dfScale));  RFW - 1/16/02 - 9654
                      lpPFormat->dfScale) + 0.5);

if (lpFont->nFontHeightPrinter != nHeight || !lpFont->hFontPrinter)
   {
   HFONT hFontOld;
   TEXTMETRIC tm;

   LogFont.lfHeight = -nHeight;
   LogFont.lfWidth = 0;
   LogFont.lfOutPrecision = OUT_CHARACTER_PRECIS;

   // if we have an array of char widths, then free it.  SCP 12/31/98
   if (lpFont->hCharWidthsPrinter)
      {
      tbGlobalFree(lpFont->hCharWidthsPrinter);
      lpFont->hCharWidthsPrinter = 0;
      }

   if (lpFont->hFontPrinter)
      DeleteObject(lpFont->hFontPrinter);

   lpFont->hFontPrinter = CreateFontIndirect(&LogFont);
   hFontOld = SelectObject(hDC, lpFont->hFontPrinter);
   GetTextMetrics(hDC, &tm);

   if (lpPFormat->wAction == SS_PRINT_ACTION_PREVIEW)
      {
      // If the font is not scaled small enough then have Windows
      // choose any font of this size.

      if (tm.tmAscent > nHeight)
         {
         LogFont.lfPitchAndFamily = tm.tmPitchAndFamily & 0xF0;
         if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH)
            LogFont.lfPitchAndFamily &= ~FIXED_PITCH;
         else
            LogFont.lfPitchAndFamily |= FIXED_PITCH;

         LogFont.lfCharSet = tm.tmCharSet;
         _fmemset(LogFont.lfFaceName, '\0', LF_FACESIZE);

         SelectObject(hDC, hFontOld);
         DeleteObject(lpFont->hFontPrinter);
         lpFont->hFontPrinter = CreateFontIndirect(&LogFont);
         // Select new font and get new TextMetrics  SCP 12/31/98
         SelectObject(hDC, lpFont->hFontPrinter);
         GetTextMetrics(hDC, &tm);
         }

		hDCPrinter = hDC;
		}

   SelectObject(hDC, hFontOld);

   // Create array for Char widths.  SCP 12/31/98

   /* RFW - 7/30/04 - 14874
   nCh = max(tm.tmLastChar,255);
   lpFont->hCharWidthsPrinter = tbGlobalAlloc(GSHARE, sizeof(int)*(nCh + 1));
   lpCharWidths = (int FAR *)tbGlobalLock(lpFont->hCharWidthsPrinter);
   if (lpCharWidths)
		{
		UINT j;
      SelectObject(hDCPrinter, lpFont->hFontPrinter);
		GetCharWidth(hDCPrinter, 0, nCh, lpCharWidths);
	   SelectObject(hDCPrinter, hFontOld);
		if (tm.tmOverhang)
			for (j = 0; j <= nCh; j++)	
				lpCharWidths[j] -= tm.tmOverhang;
		tbGlobalUnlock(lpFont->hCharWidthsPrinter);
		}
   */

   lpFont->nFontHeightPrinter = nHeight;
   SS_SetFont(lpSS->lpBook, lpFont, FontId);
   }

ReleaseDC(lpSS->lpBook->hWnd, hDC);
return (lpFont->hFontPrinter);
}


#if 0
HFONT SS_CreatePrinterFont(LPSPREADSHEET lpSS, LPSS_FONT lpFont,
                           SS_FONTID FontId, LPSSX_PRINTFORMAT lpPFormat)
{
LOGFONT LogFont;
short   nHeight;
UINT    nCh;                 // SCP 12/31/98
int FAR *lpCharWidths;       // SCP 12/31/98

_fmemcpy(&LogFont, &lpFont->LogFont, sizeof(LOGFONT));

// Calculate the exact height of the font
{
TEXTMETRIC tm;
HDC        hDC = GetDC(lpSS->lpBook->hWnd);
HFONT      hFontOld = SelectObject(hDC, lpFont->hFont);

GetTextMetrics(hDC, &tm);
LogFont.lfHeight = tm.tmHeight - tm.tmInternalLeading;

SelectObject(hDC, hFontOld);
ReleaseDC(lpSS->lpBook->hWnd, hDC);
}

#if 0
nHeight = (short)abs(MulDiv(LogFont.lfHeight, 72, lpPFormat->TextHeightScreen));
if (lpPFormat->wAction == SS_PRINT_ACTION_PREVIEW)
   nHeight = (short)(((double)((long)nHeight *
                               (long)lpPFormat->TextHeightPrinter) / 72.0 *
                               lpPFormat->dfScalePreviewY) + 0.5);
else
   nHeight = (short)(((double)((long)nHeight *
                               (long)lpPFormat->TextHeightPrinter) / 72.0 *
                               lpPFormat->dfScale) + 0.5);
//   nHeight = (short)MulDiv((int)nHeight, lpPFormat->TextHeightPrinter, 72);
#endif

{
double dfHeight = (double)abs(LogFont.lfHeight) * 72.0 / (double)lpPFormat->TextHeightScreen;
if (lpPFormat->wAction == SS_PRINT_ACTION_PREVIEW)
   nHeight = (short)((dfHeight * (double)lpPFormat->TextHeightPrinter / 72.0 *
                      lpPFormat->dfScalePreviewY * lpPFormat->dfScale));
//                      lpPFormat->dfScalePreviewY) + 0.5);
else
   nHeight = (short)((dfHeight * (double)lpPFormat->TextHeightPrinter / 72.0 *
//                      lpPFormat->dfScale));  RFW - 1/16/02 - 9654
                      lpPFormat->dfScale) + 0.5);
//   nHeight = (short)MulDiv((int)nHeight, lpPFormat->TextHeightPrinter, 72);
}

if (lpFont->nFontHeightPrinter != nHeight || !lpFont->hFontPrinter)
   {
   LogFont.lfHeight = -nHeight;
   LogFont.lfWidth = 0;
   LogFont.lfOutPrecision = OUT_CHARACTER_PRECIS;

   // if we have an array of char widths, then free it.  SCP 12/31/98
   if (lpFont->hCharWidthsPrinter)
      {
      tbGlobalFree(lpFont->hCharWidthsPrinter);
      lpFont->hCharWidthsPrinter = 0;
      }

   if (lpFont->hFontPrinter)
      DeleteObject(lpFont->hFontPrinter);

   lpFont->hFontPrinter = CreateFontIndirect(&LogFont);

   if (lpPFormat->wAction == SS_PRINT_ACTION_PREVIEW)
      {
      HDC        hDC = GetDC(lpSS->lpBook->hWnd);
      TEXTMETRIC tm;
      HFONT      hFontOld = SelectObject(hDC, lpFont->hFontPrinter);

      GetTextMetrics(hDC, &tm);

      // If the font is not scaled small enough then have Windows
      // choose any font of this size.

      if (tm.tmAscent > nHeight)
         {
         LogFont.lfPitchAndFamily = tm.tmPitchAndFamily & 0xF0;
         if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH)
            LogFont.lfPitchAndFamily &= ~FIXED_PITCH;
         else
            LogFont.lfPitchAndFamily |= FIXED_PITCH;

         LogFont.lfCharSet = tm.tmCharSet;
         _fmemset(LogFont.lfFaceName, '\0', LF_FACESIZE);

         SelectObject(hDC, hFontOld);
         DeleteObject(lpFont->hFontPrinter);
         lpFont->hFontPrinter = CreateFontIndirect(&LogFont);
         // Select new font and get new TextMetrics  SCP 12/31/98
         SelectObject(hDC, lpFont->hFontPrinter);
         GetTextMetrics(hDC, &tm);
         }

      // Create array for Char widths.  SCP 12/31/98
      nCh = max(tm.tmLastChar,255);
      lpFont->hCharWidthsPrinter = tbGlobalAlloc(GSHARE, sizeof(int)*(nCh + 1));
      lpCharWidths = (int FAR *)tbGlobalLock(lpFont->hCharWidthsPrinter);
      if (lpCharWidths)
        {
        UINT j;
        GetCharWidth(hDC, 0, nCh, lpCharWidths);
        for (j = 0; j <= nCh; j++)	
          lpCharWidths[j] -= tm.tmOverhang;
        tbGlobalUnlock(lpFont->hCharWidthsPrinter);
        }
      // end of Char widths code

      SelectObject(hDC, hFontOld);
      ReleaseDC(lpSS->lpBook->hWnd, hDC);
      }

   lpFont->nFontHeightPrinter = nHeight;
   SS_SetFont(lpSS->lpBook, lpFont, FontId);
   }

return (lpFont->hFontPrinter);
}
#endif // 0


BOOL SS_InvertRow(hWnd, hDC, lpSS)

HWND          hWnd;
HDC           hDC;
LPSPREADSHEET lpSS;
{
return (SS_InvertRowBlock(hWnd, hDC, lpSS, lpSS->Row.CurAt,
        lpSS->Row.CurAt));
}


BOOL SS_InvertRowBlock(hWnd, hDC, lpSS, Row, Row2)

HWND          hWnd;
HDC           hDC;
LPSPREADSHEET lpSS;
SS_COORD      Row;
SS_COORD      Row2;
{
RECT          Rect;
HRGN          hRgn;

if (!SS_IsActiveSheet(lpSS))
	return (FALSE);

if (lpSS->Row.LR == 0 || lpSS->Col.LR == 0)
   return (FALSE);

if (SS_CreateBlockRect(lpSS, &Rect, 0, Row, -1, Row2, FALSE))
   {
   if (SS_INVERTSEL(lpSS))
      {
      int iDCSave = SaveDC(hDC);

      hRgn = CreateRectRgnIndirect(&Rect);

      // SelectClipRgn() assumes that the coordinates for a region are
      // specified in device units.  InvertRgn() assumes that the coordinates
      // for a region are specified in logical units.  At design time, logical
      // units of an OCX control may not be equivalent to device units.
#ifndef SS_UTP
      {
      HRGN hRgnClip;
      LPtoDP(hDC, (LPPOINT)&Rect, 2);
      hRgnClip = CreateRectRgnIndirect(&Rect);
      SelectClipRgn(hDC, hRgnClip);
      DeleteObject(hRgnClip);
      }
#endif
      InvertRgn(hDC, hRgn);
      DeleteObject(hRgn);
      RestoreDC(hDC, iDCSave);
      }
   else
      {
      SS_COORD i;

      for (i = 0; i < lpSS->Col.HeaderCnt; i++)
         Rect.left += SS_GetColWidthInPixels(lpSS, i);

      InvalidateRect(lpSS->lpBook->hWnd, &Rect, FALSE);
      }
   }

return (TRUE);
}


BOOL SS_ShouldButtonBeDrawn(LPSPREADSHEET lpSS, SS_COORD Col,
                            SS_COORD Row, WORD wCellType)
{
BOOL fRet = FALSE;

if (lpSS->wOpMode == SS_OPMODE_ROWMODE)
//#ifdef SPREAD_JPN
  // SP25_009 Zhang HaoNan 1998/4/23
//	return TRUE;
//#else
   return (lpSS->fRowModeEditing && Row == lpSS->Row.CurAt ? TRUE :
           FALSE);
//#endif

if (lpSS->lpBook->wButtonDrawMode == SS_BDM_ALWAYS)
   fRet = TRUE;

else if (wCellType == SS_TYPE_BUTTON &&
         (lpSS->lpBook->wButtonDrawMode & SS_BDM_ALWAYSBUTTON))
   fRet = TRUE;

else if (wCellType == SS_TYPE_COMBOBOX &&
         (lpSS->lpBook->wButtonDrawMode & SS_BDM_ALWAYSCOMBO))
   fRet = TRUE;

else if ((lpSS->lpBook->wButtonDrawMode & SS_BDM_CURRENTCELL) &&
    lpSS->Col.CurAt == Col && lpSS->Row.CurAt == Row)
   fRet = TRUE;

else if ((lpSS->lpBook->wButtonDrawMode & SS_BDM_CURRENTCOL) &&
    lpSS->Col.CurAt == Col)
   fRet = TRUE;

else if ((lpSS->lpBook->wButtonDrawMode & SS_BDM_CURRENTROW) &&
    lpSS->Row.CurAt == Row)
   fRet = TRUE;

return (fRet);
}


#ifndef SS_NOBORDERS
void SS_DrawBorder(HWND hWnd, LPSPREADSHEET lpSS, HDC hDC, LPSS_COL lpCol,
                   LPSS_ROW lpRow, LPRECT lpRect, SS_COORD Col, SS_COORD Row,
                   LPSSX_PRINTFORMAT lpPFormat)
{
RECT     RectTemp;
WORD     wStyleLeft = 0;
COLORREF clrLeft = 0;
WORD     wStyleTop = 0;
COLORREF clrTop = 0;
WORD     wStyleRight = 0;
COLORREF clrRight = 0;
WORD     wStyleBottom = 0;
COLORREF clrBottom = 0;

if (!lpSS->fSetBorder)
   return;

if (lpRect->right < lpRect->left || lpRect->bottom < lpRect->top)
	return;

if (SSx_GetBorder(lpSS, lpCol, lpRow, NULL, Col, Row,
                  &wStyleLeft, &clrLeft, &wStyleTop, &clrTop,
                  &wStyleRight, &clrRight, &wStyleBottom, &clrBottom))
   {
   SetRect(&RectTemp, lpRect->left - 1, lpRect->top - 1, lpRect->right,
           lpRect->bottom);

#ifdef SS_V40
	{
	SS_SELBLOCK SpanBlock;

	if (SS_SpanCalcBlock(lpSS, Col, Row, &SpanBlock))
		{
		if (Col != SpanBlock.UL.Col)
			wStyleLeft = 0;
		if (Col != SpanBlock.LR.Col)
			wStyleRight = 0;
		if (Row != SpanBlock.UL.Row)
			wStyleTop = 0;
		if (Row != SpanBlock.LR.Row)
			wStyleBottom = 0;
		}
	}
#endif // SS_V40

#ifdef  BUGS
// Bug-013
   if (wStyleLeft  && ( lpPFormat ? TRUE : (Col != lpSS->Col.UL) ) )
#else
   if (wStyleLeft  && Col != lpSS->Col.UL)
#endif

      SSx_DrawBorder(lpSS, hDC, RectTemp.left, max(RectTemp.top, 0),
                     RectTemp.left, RectTemp.bottom, lpPFormat,
                     wStyleLeft, clrLeft);

#ifdef  BUGS
// Bug-013
   if (wStyleTop  && ( lpPFormat ? TRUE : (Row != lpSS->Row.UL) ) )
#else
   if (wStyleTop  && Row != lpSS->Row.UL)
#endif

      SSx_DrawBorder(lpSS, hDC, max(RectTemp.left, 0), RectTemp.top,
                     RectTemp.right, RectTemp.top, lpPFormat,
                     wStyleTop, clrTop);

   if (wStyleRight)
      SSx_DrawBorder(lpSS, hDC, RectTemp.right, max(RectTemp.top, 0),
                     RectTemp.right, RectTemp.bottom, lpPFormat,
                     wStyleRight, clrRight);

   if (wStyleBottom)
      SSx_DrawBorder(lpSS, hDC, max(RectTemp.left, 0), RectTemp.bottom,
                     RectTemp.right, RectTemp.bottom, lpPFormat,
                     wStyleBottom, clrBottom);
   }
}


void SSx_DrawBorder(LPSPREADSHEET lpSS, HDC hDC, int left, int top,
                    int right, int bottom, LPSSX_PRINTFORMAT lpPFormat,
                    WORD wBorderStyle, COLORREF Color)
{
SS_COLORTBLITEM BackColorTblItem;
int             nWidth;
int             nPenStyle;

if (wBorderStyle == SS_BORDERSTYLE_NONE)
   return;

nWidth = 1;
nPenStyle = PS_SOLID;

SS_GetColorItem(&BackColorTblItem, lpSS->Color.BackgroundId);

//Modify by BOC 99.7.19(hyt)--------------------------
//for not print color when PrintColor=false
if (lpPFormat && !lpPFormat->pf.fDrawColors)
{
	Color = RGBCOLOR_BLACK;
	BackColorTblItem.Color = RGBCOLOR_WHITE;
}
//----------------------------------------------------				

switch (wBorderStyle)
   {
   case SS_BORDERSTYLE_SOLID:
      nPenStyle = PS_SOLID;
      break;

   case SS_BORDERSTYLE_DASH:
      nPenStyle = PS_DASH;
      break;

   case SS_BORDERSTYLE_DOT:
      nPenStyle = PS_DOT;
      break;

   case SS_BORDERSTYLE_DASHDOT:
      nPenStyle = PS_DASHDOT;
      break;

   case SS_BORDERSTYLE_DASHDOTDOT:
      nPenStyle = PS_DASHDOTDOT;
      break;

   case SS_BORDERSTYLE_BLANK:
      Color = BackColorTblItem.Color;
      nPenStyle = PS_SOLID;
      break;

   case SS_BORDERSTYLE_DOUBLESOLID:
   case SS_BORDERSTYLE_SOLID2:
   case SS_BORDERSTYLE_SOLID3:
      break;

//#ifdef  SPREAD_JPN
   case SS_BORDERSTYLE_FINE_SOLID:
      if (!lpPFormat)
         {
         static BYTE LineType[] = {0, 1, 3};

         DrawBorderEx(hDC, left, top, right, bottom, Color, BackColorTblItem.Color,
                                        (LPBYTE)LineType, lpPFormat);
         return;
         }
      else
         nPenStyle = PS_SOLID;

      break;

   case SS_BORDERSTYLE_FINE_DASH:
      if (!lpPFormat)
         {
         static BYTE LineType[] = {0, 0, 0, 0, 1, 1, 3};

         DrawBorderEx(hDC, left, top, right, bottom, Color, BackColorTblItem.Color,
                                        (LPBYTE)LineType, lpPFormat);
         return;
         }
      else
         nPenStyle = PS_DASH;

      break;

   case SS_BORDERSTYLE_FINE_DOT:
      if (!lpPFormat)
         {
         static BYTE LineType[] = {0, 0, 1, 1, 3};

         DrawBorderEx(hDC, left, top, right, bottom, Color, BackColorTblItem.Color,
                                        (LPBYTE)LineType, lpPFormat);
         return;
         }
      else
         nPenStyle = PS_DOT;

      break;

   case SS_BORDERSTYLE_FINE_DASH_DOT:
      if (!lpPFormat)
         {
         static BYTE LineType[] = {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 3};

         DrawBorderEx(hDC, left, top, right, bottom, Color, BackColorTblItem.Color,
                                        (LPBYTE)LineType, lpPFormat);
         return;
         }
      else
         nPenStyle = PS_DASHDOT;

      break;

   case SS_BORDERSTYLE_FINE_DASH_DOT_DOT:
      if (!lpPFormat)
         {
         static BYTE LineType[] = {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 3};

         DrawBorderEx(hDC, left, top, right, bottom, Color, BackColorTblItem.Color,
                                        (LPBYTE)LineType, lpPFormat);
         return;
         }
      else
         nPenStyle = PS_DASHDOTDOT;

      break;
//#endif
   }

Color = SS_TranslateColor(Color);

if (nPenStyle == PS_SOLID && !lpPFormat &&
    (Color == RGBCOLOR_BLACK || Color == RGBCOLOR_WHITE))
   {
   PatBlt(hDC, left, top, right - left + 1, bottom - top + 1,
          Color == RGBCOLOR_BLACK ? BLACKNESS : WHITENESS);
   }

else
   {
   int      BkMode = SetBkMode(hDC, OPAQUE);
   COLORREF BkColorOld = SetBkColor(hDC, SS_TranslateColor(BackColorTblItem.Color));
   HPEN     hPen = CreatePen(nPenStyle, nWidth, Color);
   HPEN     hPenOld = SelectObject(hDC, hPen);

// RFW-REMOVE
/*
HBRUSH hBrush = CreateSolidBrush(Color);
RECT Rect = {left, top, right + 1, bottom + 1};
UnrealizeObject(hBrush);
FillRect(hDC, &Rect, hBrush);
DeleteObject(hBrush);
*/
   SS_MoveTo(hDC, left, top, lpPFormat);
   SS_LineTo(lpSS, hDC, 0, 0, 0, right, bottom, SS_LINETYPE_SOLID, TRUE, lpPFormat);

   SelectObject(hDC, hPenOld);
   DeleteObject(hPen);

   SetBkColor(hDC, BkColorOld);
   SetBkMode(hDC, BkMode);
   }

}
#endif


void SS_EraseBkgnd(HWND hWnd, LPSPREADSHEET lpSS, HDC hDC, LPRECT lpRectClient,
                   LPRECT lpRectUpdate, SS_COORD RightVisCell,
                   SS_COORD BottomVisCell)
{
SS_COLORTBLITEM BackColorTblItem;
RECT            RectTemp;
RECT            Rect;

CopyRect(&Rect, lpRectClient);

if (RightVisCell == SS_GetColCnt(lpSS) - 1)
   Rect.right = min(Rect.right, Rect.left + SS_GetCellPosX(lpSS,
                lpSS->Col.UL, SS_GetColCnt(lpSS) - 1) +
                SS_GetColWidthInPixels(lpSS, SS_GetColCnt(lpSS) - 1) + 1);

if (BottomVisCell == SS_GetRowCnt(lpSS) - 1)
   Rect.bottom = min(Rect.bottom, Rect.top + SS_GetCellPosY(lpSS,
                 lpSS->Row.UL, SS_GetRowCnt(lpSS) - 1) +
                 SS_GetRowHeightInPixels(lpSS, SS_GetRowCnt(lpSS) - 1) + 1);

SS_GetColorItem(&BackColorTblItem, lpSS->Color.BackgroundId);

IntersectRect(&RectTemp, &Rect, lpRectUpdate);

UnrealizeObject(BackColorTblItem.hBrush);
#ifdef WIN32
SetBrushOrgEx(hDC, lpSS->bHPixelOffset, lpSS->bVPixelOffset, NULL);
#else
SetBrushOrg(hDC, lpSS->bHPixelOffset, lpSS->bVPixelOffset);
#endif

FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);

#ifndef SS_NOCHILDSCROLL
if (lpSS->lpBook->fVScrollVisible && lpSS->lpBook->fHScrollVisible)
   {
   HBRUSH hBrush;
   RECT   RectTemp;

   SetRect(&RectTemp, lpRectClient->right, lpRectClient->bottom,
           lpRectClient->right + lpSS->lpBook->dVScrollWidth, lpRectClient->bottom +
           lpSS->lpBook->dHScrollHeight);
	hBrush = CreateSolidBrush(GetSysColor(COLOR_SCROLLBAR));
	FillRect(hDC, &RectTemp, hBrush);
	DeleteObject(hBrush);
   }
#endif

/*********************
* Draw Scroll Arrows
*********************/

#ifdef SS_UTP
if (lpSS->fUseScrollArrows)
   {
   RECT     RectClient;
   HPEN     hPenOld;
   HPEN     hPen;
   SS_COORD BottomVisCell;
   SS_COORD i;
   short    y;

   GetClientRect(hWnd, &RectClient);

   /*******************
   * Draw right arrow
   *******************/

   if (lpSS->fRightScrollArrowVis)
      {
      // Draw Arrow

      BottomVisCell = SS_GetBottomVisCell(lpSS, lpSS->Row.UL);

      SetRect(&RectTemp, RectClient.right - lpSS->dScrollArrowWidth,
              RectClient.top, RectClient.right, RectClient.bottom);

      if (BottomVisCell == SS_GetRowCnt(lpSS) - 1)
         RectTemp.bottom = Rect.bottom;

      FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);

      SetRect(&RectTemp, RectClient.right - lpSS->dScrollArrowWidth + 4,
              RectClient.top - 1, RectClient.right + 1, RectClient.top +
              SS_GetRowHeightInPixels(lpSS, 0) + 1);

      SS_DrawArrow(hWnd, hDC, lpSS, SS_MODE_ARROW_RIGHT, FALSE);

//      MoveToEx(hDC, RectTemp.left - 4, RectTemp.top, NULL);
//      LineTo(hDC, RectTemp.left - 4, RectTemp.bottom - 1);

      if (GetDeviceCaps(hDC, NUMCOLORS) == 2)
         hPen = CreatePen(PS_DOT, 1, RGBCOLOR_BLACK);
      else
         hPen = CreatePen(PS_DOT, 1, SS_TranslateColor(lpSS->GridColor));

      hPenOld = SelectObject(hDC, hPen);

      for (i = 0, y = -1; i < lpSS->Row.HeaderCnt + lpSS->Row.Frozen &&
           i <= BottomVisCell; i++)
         {
         y += SS_GetRowHeightInPixels(lpSS, i);

         if (i >= lpSS->Row.HeaderCnt)
            {
            MoveToEx(hDC, RectTemp.left, y, NULL);
            LineTo(hDC, RectTemp.right, y);
            }
         }

      for (i = max(i, lpSS->Row.UL); i <= BottomVisCell; i++)
         {
         y += SS_GetRowHeightInPixels(lpSS, i);
         MoveToEx(hDC, RectTemp.left, y, NULL);
         LineTo(hDC, RectTemp.right, y);
         }

      SelectObject(hDC, hPenOld);
      DeleteObject(hPen);
      }
   else
      {
      CopyRect(&RectTemp, &RectClient);
      RectTemp.left = Rect.right;
      FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);
      }

   /********************
   * Draw bottom arrow
   ********************/

   if (lpSS->fBottomScrollArrowVis)
      {
      // Draw Arrow

      SetRect(&RectTemp, RectClient.left, RectClient.bottom -
              lpSS->dScrollArrowHeight, RectClient.right,
              RectClient.bottom);

      if (SS_GetRightVisCell(lpSS, lpSS->Col.UL) == SS_GetColCnt(lpSS) - 1)
         RectTemp.right = Rect.right;

      FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);

      SetRect(&RectTemp, RectClient.left, RectClient.bottom -
              lpSS->dScrollArrowHeight, RectClient.left +
              SS_GetColWidthInPixels(lpSS, 0), RectClient.bottom);

      SS_DrawArrow(hWnd, hDC, lpSS, SS_MODE_ARROW_DOWN, FALSE);

//      MoveToEx(hDC, RectTemp.left, RectTemp.top, NULL);
//      LineTo(hDC, RectTemp.right, RectTemp.top);
      }
   else
      {
      CopyRect(&RectTemp, &RectClient);
      RectTemp.top = Rect.bottom;
      FillRect(hDC, &RectTemp, BackColorTblItem.hBrush);
      }
   }

#endif
}


#ifdef SS_UTP
void SS_GetScrollArrowRects(HWND hWnd, LPSPREADSHEET lpSS,
                            LPRECT lpRectRight, LPRECT lpRectDown,
                            LPRECT lpRectLeft, LPRECT lpRectUp)
{
RECT  RectClient;
short x;
short y;
short cx;
short cy;

if (lpSS->fUseScrollArrows)
   {
   GetClientRect(hWnd, &RectClient);

   /**************
   * Right Arrow
   **************/

   if (lpRectRight)
      if (SS_GetRightVisCell(lpSS, lpSS->Col.UL) < lpSS->Col.Max)
         SetRect(lpRectRight, RectClient.right - lpSS->dScrollArrowWidth +
                 4, RectClient.top - 1, RectClient.right + 1, RectClient.top +
                 SS_GetRowHeightInPixels(lpSS, 0) + 1);
      else
         SetRectEmpty(lpRectRight);

   /*************
   * Down Arrow
   *************/

   if (lpRectDown)
      if (SS_GetBottomVisCell(lpSS, lpSS->Row.UL) < lpSS->Row.Max)
         SetRect(lpRectDown, RectClient.left - 1, RectClient.bottom -
                 lpSS->dScrollArrowHeight + 1, RectClient.left +
                 SS_GetColWidthInPixels(lpSS, 0) + 1, RectClient.bottom + 1);
      else
         SetRectEmpty(lpRectDown);

   /*************
   * Left Arrow
   *************/

   if (lpRectLeft)
      if (lpSS->Col.UL > lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
         {
         SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL,
                         lpSS->Col.HeaderCnt + lpSS->Col.Frozen - 1, 0,
                         &x, &y, &cx, &cy);
         SetRect(lpRectLeft, x, y, x + cx + 1, y + cy + 1);
         }
      else
         SetRectEmpty(lpRectLeft);

   /***********
   * Up Arrow
   ***********/

   if (lpRectUp)
      if (lpSS->Row.UL > lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
         {
         SS_GetCellCoord(lpSS, 0, 0, 0, 0, &x, &y, &cx, &cy);
         SetRect(lpRectUp, x, y, x + cx + 1, y + cy);
         }
      else
         SetRectEmpty(lpRectUp);
   }
}


void SS_DrawArrow(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS, BYTE bArrow,
                  BOOL fButtonDown)
{
RECT  Rect;
HDC   hDCOrig = hDC;
HICON hIconUp = 0;
HICON hIconDown = 0;
WORD  wIconUpType = 0;
WORD  wIconDownType = 0;

if (!hDCOrig)
   hDC = SS_GetDC(lpSS->lpBook);

if (bArrow == SS_MODE_ARROW_RIGHT)
   {
   hIconUp = lpSS->hBitmapScrollArrowRightUp;
   wIconUpType = lpSS->uScrollArrowRightUpType | BT_HANDLE;
   hIconDown = lpSS->hBitmapScrollArrowRightDown;
   wIconDownType = lpSS->uScrollArrowRightDownType | BT_HANDLE;
   SS_GetScrollArrowRects(hWnd, lpSS, &Rect, NULL, NULL, NULL);

   if (Rect.right - Rect.left > SS_SCROLLARROW_WIDTH)
      Rect.left = Rect.right - SS_SCROLLARROW_WIDTH;
   }

else if (bArrow == SS_MODE_ARROW_DOWN)
   {
   hIconUp = lpSS->hBitmapScrollArrowBottomUp;
   wIconUpType = lpSS->uScrollArrowBottomUpType | BT_HANDLE;
   hIconDown = lpSS->hBitmapScrollArrowBottomDown;
   wIconDownType = lpSS->uScrollArrowBottomDownType | BT_HANDLE;
   SS_GetScrollArrowRects(hWnd, lpSS, NULL, &Rect, NULL, NULL);

   if (Rect.bottom - Rect.top > SS_SCROLLARROW_HEIGHT)
      Rect.top = Rect.bottom - SS_SCROLLARROW_HEIGHT;
   }

else if (bArrow == SS_MODE_ARROW_LEFT)
   {
   hIconUp = lpSS->hBitmapScrollArrowLeftUp;
   wIconUpType = lpSS->uScrollArrowLeftUpType | BT_HANDLE;
   hIconDown = lpSS->hBitmapScrollArrowLeftDown;
   wIconDownType = lpSS->uScrollArrowLeftDownType | BT_HANDLE;
   SS_GetScrollArrowRects(hWnd, lpSS, NULL, NULL, &Rect, NULL);
   }

else if (bArrow == SS_MODE_ARROW_UP)
   {
   hIconUp = lpSS->hBitmapScrollArrowTopUp;
   wIconUpType = lpSS->uScrollArrowTopUpType | BT_HANDLE;
   hIconDown = lpSS->hBitmapScrollArrowTopDown;
   wIconDownType = lpSS->uScrollArrowTopDownType | BT_HANDLE;
   SS_GetScrollArrowRects(hWnd, lpSS, NULL, NULL, NULL, &Rect);
   }

SS_PaintArrow(hWnd, hDC, lpSS, &Rect, hIconUp, wIconUpType, hIconDown,
              wIconDownType, fButtonDown);

if (!hDCOrig)
   SS_ReleaseDC(lpSS->lpBook, hDC);
}


void SS_PaintArrow(HWND hWnd, HDC hDC, LPSPREADSHEET lpSS, LPRECT lpRect,
                   HICON hIconUp, WORD wIconUpType, HICON hIconDown,
                   WORD wIconDownType, BOOL fButtonDown)
{
SS_COLORTBLITEM BackColorTblItem;
SUPERBTN        SuperBtn;

_fmemset(&SuperBtn, '\0', sizeof(SUPERBTN));

SuperBtn.ButtonDown = fButtonDown;

if (hIconUp && (!fButtonDown || !hIconDown))
   SuperBtnSetPict(0, 0, &SuperBtn.Pict, wIconUpType, (long)hIconUp);
                    
else if (fButtonDown && hIconDown)
   SuperBtnSetPict(0, 0, &SuperBtn.PictBtnDown, wIconDownType,
                   (long)hIconDown);

SS_GetColorItem(&BackColorTblItem, lpSS->Color.BackgroundId);

SuperBtn.Color.Color = BackColorTblItem.Color;
SuperBtn.Color.ColorBorder = BackColorTblItem.Color;
SuperBtn.Color.ColorShadow = BackColorTblItem.Color;
SuperBtn.Color.ColorHighlight = BackColorTblItem.Color;
SuperBtn.Color.ColorText = BackColorTblItem.Color;

SuperBtnPaint(hWnd, &SuperBtn, hDC, lpRect, TRUE, FALSE, FALSE, FALSE, NULL);
}
#endif

void SSx_DrawText(HWND hWnd, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPTSTR lpData,
                  int iLen, LPRECT lpRect, LPRECT lpRectOrig, UINT Just,
                  LPSS_FONT lpFont, DWORD dwTextOrient)
{
TBGLOBALHANDLE hCharWidths;
FPDRAWTEXT     dt;
short          JustOrig = Just;

//-Fengwei
#ifdef	SPREAD_JPN
// The bug occurs only on NT environment
// It'll hang up after set RowHeaderDisplay to 0 in VB Debug Window
//
if(lpData && *lpData)
#endif

// Sp25_008 [3-3]
// It Seems This Function Will Cause GDI Leak :(
// But The Function Is In The Lib And Can't Be Modify :(
// So Nothing Can Do :(
// -- HaHa 1998.4.25
  { // Moved above ifdef so nothing is done if no data - SCP 12/31/98.

 if (iLen == -1)
   iLen = lstrlen(lpData);

  hCharWidths = (lpPFormat ? lpFont->hCharWidthsPrinter : lpFont->hCharWidths);

  _fmemset(&dt, '\0', sizeof(dt));

  dt.fuStyle |= DTX_TRANSPARENT;

#ifdef SS_V35
  if (Just & DTX_ELLIPSES)
     {
     dt.fuStyle |= DTX_ELLIPSES;
     Just &= ~DTX_ELLIPSES;
     }
#endif

#ifdef SS_V40
	if ((dwTextOrient & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_VERT_LTR)
		dt.fuStyle |= DTX_VERTICAL_LTR;
	else if ((dwTextOrient & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_DOWN)
		{
		dt.fuStyle |= DTX_ROTATE | DTX_FLIP;
		Just &= ~(DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM);
		if (JustOrig & DT_RIGHT)
			Just |= DT_TOP;
		else if (JustOrig & DT_CENTER)
			Just |= DT_VCENTER;
		else
			Just |= DT_BOTTOM;

		if (JustOrig & DT_BOTTOM)
			Just |= DT_RIGHT;
		else if (JustOrig & DT_VCENTER)
			Just |= DT_CENTER;
		else
			Just |= DT_LEFT;
		}
	else if ((dwTextOrient & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_UP)
		{
		dt.fuStyle |= DTX_ROTATE;
		Just &= ~(DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM);
		if (JustOrig & DT_RIGHT)
			Just |= DT_BOTTOM;
		else if (JustOrig & DT_CENTER)
			Just |= DT_VCENTER;
		else
			Just |= DT_TOP;

		if (JustOrig & DT_BOTTOM)
			Just |= DT_LEFT;
		else if (JustOrig & DT_VCENTER)
			Just |= DT_CENTER;
		else
			Just |= DT_RIGHT;
		}
	else if ((dwTextOrient & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_INVERT)
		{
		dt.fuStyle |= DTX_FLIP;
		Just &= ~(DT_LEFT | DT_CENTER | DT_RIGHT | DT_TOP | DT_VCENTER | DT_BOTTOM);
		if (JustOrig & DT_RIGHT)
			Just |= DT_LEFT;
		else if (JustOrig & DT_CENTER)
			Just |= DT_CENTER;
		else
			Just |= DT_RIGHT;

		if (JustOrig & DT_BOTTOM)
			Just |= DT_TOP;
		else if (JustOrig & DT_VCENTER)
			Just |= DT_VCENTER;
		else
			Just |= DT_BOTTOM;
		}
	else if ((dwTextOrient & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_VERT_RTL)
		dt.fuStyle |= DTX_VERTICAL_RTL;
#endif // SS_V40

  // If we have char widths, then lock the array and put in "dt" struct. SCP 12/31/98
  if (hCharWidths)
    dt.lpCharWidths = (int FAR *)tbGlobalLock(hCharWidths);

  dt.dwFontLanguageInfo = lpFont->dwFontLanguageInfo;
	// BUG 01822 (1-1)
	// Rect Here Is Calculated From Adjacent Cells Of Current Cell,
	// Using It To Draw Center Alignment Text Is Incorrect,
	// Because It Is Relacted To Width Of Adjacent Cells.
	// Calculate The Rect Again, Make Origin Cell Be The Center Within It.
	// Modified By HaHa 1999.11.15
#ifdef SS_V40
	if ((Just & DT_CENTER) && (dwTextOrient & SSS_TEXTORIENT_MASK) == 0)
#else
	if (Just & DT_CENTER)
#endif
	{
		RECT DrawRect;	// Rect To Draw
		int SavedDCNumber;

		// Set Rect To Draw
		DrawRect.left=lpRect->left;
		DrawRect.right=lpRect->right;
		DrawRect.top=lpRect->top;
		DrawRect.bottom=lpRect->bottom;
		if (lpRectOrig->left-lpRect->left>lpRect->right-lpRectOrig->right)
			DrawRect.right=lpRectOrig->right+(lpRectOrig->left-lpRect->left);
		else
			DrawRect.left=lpRectOrig->left-(lpRect->right-lpRectOrig->right);

		// Use Old Rect To Clip
		SavedDCNumber=SaveDC(hDC);
		IntersectClipRect(hDC,lpRect->left,lpRect->top,lpRect->right,lpRect->bottom);
//    RFW - 5/13/02 - RUN_CEL_004_002
//		#ifdef  SPREAD_JPN
//		DrawText(hDC, lpData, iLen, &DrawRect, Just &= ~DT_NOCLIP);
//		#else
		fpDrawText(hWnd, hDC, lpData, iLen, &DrawRect, Just &= ~DT_NOCLIP, &dt);
//		#endif
		RestoreDC(hDC,SavedDCNumber);
	}
	else
	{
		// BUG 01862 Refix (3-3)
		// Modified By HaHa 1999.12.16

		// BUG SPR_JPN_001_012 & SPR_JPN_001_020 (1-1)
		// It Use An Edit Box To Edit A Cell And Paint The Cell Using The Default Method Of Edit Box,
		// The Default Paint Method Of Edit Box Most Probablely Calls DrawText() Windows API.
		// (Refer To The Call Of UpdateWindow() In SS_TypeEditOn() In SS_EMODE.C)
		// (Refer To SS_EditHScrollProc() In SS_EDIT.C)
		// When A Cell Not In Editing Needs Repaint, It Will Call fpDrawText() Here,
		// fpDrawText() Function Has Differnce With DrawText() Windows API In Wordwrap.
		// This Will Cause Bug SPR_JPN_001_012 & SPR_JPN_001_020 (The Two Bugs Are Same),
		// So I Think It's Better Also Use DrawText() Windows API Here Instead Of fpDrawText().
		// By The Way, DrawText() Is Not Add By Me But Already Exist As Comment
		// Modified By HaHa 1999.10.26
		fpDrawText(hWnd, hDC, lpData, iLen, lpRect, Just | (lpPFormat ? DT_EXTERNALLEADING : 0), &dt);
//		DrawText(hDC, lpData, iLen, lpRect, Just &= ~DT_NOCLIP);
	}

  if (hCharWidths)
    tbGlobalUnlock(hCharWidths);
  }
}


void SS_DrawFormatHeaderLetter(LPTSTR lpBuffer, long lHeaderNumber)
{
short j;

*lpBuffer = '\0';

if (lHeaderNumber >= 1)
   do
      {
      for (j = lstrlen(lpBuffer) + 1; j > 0; j--)
         lpBuffer[j] = lpBuffer[j - 1];

      lpBuffer[0] = (TCHAR)('A' + ((lHeaderNumber - 1) % 26));
      lHeaderNumber = (lHeaderNumber - 1) / 26;
      } while (lHeaderNumber > 0);
}


SS_COORD SS_ColLetterToNumber(LPCTSTR lpBuffer)
{
short    j;
short    nLen;
long     lMultiplier = 1;
SS_COORD lRet = 0;

if (lpBuffer && *lpBuffer)
	{
	nLen = lstrlen(lpBuffer);

	if (nLen <= 7)
		for (j = nLen - 1; j >= 0; j--, lMultiplier *= 26)
			lRet += (toupper((int)lpBuffer[j]) - 'A' + 1) * lMultiplier;

	// This will handle overflow cases
	if (lRet < 0)
		lRet = 0;
	}

return (lRet);
}


void SS_DrawFormatHeaderNumber(LPTSTR lpBuffer, long lHeaderNumber)
{
wsprintf(lpBuffer, _T("%ld"), lHeaderNumber);
}


void SS_ScaleRect(LPSSX_PRINTFORMAT lpPFormat, LPRECT lpRect)
{
#ifndef SS_V30
lpRect->left = SS_SCREEN2PRINTERX(lpPFormat, lpRect->left);
lpRect->top = SS_SCREEN2PRINTERY(lpPFormat, lpRect->top);
lpRect->right = SS_SCREEN2PRINTERX(lpPFormat, lpRect->right);
lpRect->bottom = SS_SCREEN2PRINTERY(lpPFormat, lpRect->bottom);
#endif
}


#ifdef SS_UTP

void SS_PaintEditModeShadow(HWND hWnd, LPSPREADSHEET lpSS,
                            LPRECT lpRectClient, short x, short y, short cx,
                            short cy)
{
SS_COLORTBLITEM ColorTblItem;
HBRUSH          hBrushOld;
RECT            Rect[6];
HDC             hDC;
short           i;

hDC = SS_GetDC(lpSS->lpBook);

SS_GetEditModeShadowRects(lpSS, Rect);

SS_GetColorItem(&ColorTblItem, lpSS->Color.ShadowDarkId);
hBrushOld = SelectObject(hDC, ColorTblItem.hBrush);

for (i = 0; i < 4; i++)
   if (Rect[i].right - Rect[i].left > 0 && Rect[i].bottom - Rect[i].top > 0)
      PatBlt(hDC, Rect[i].left, Rect[i].top, Rect[i].right - Rect[i].left,
             Rect[i].bottom - Rect[i].top, PATCOPY);

SelectObject(hDC, GetStockObject(BLACK_BRUSH));

for (i = 4; i < 6; i++)
   if (Rect[i].right - Rect[i].left > 0 && Rect[i].bottom - Rect[i].top > 0)
      PatBlt(hDC, Rect[i].left, Rect[i].top, Rect[i].right - Rect[i].left,
             Rect[i].bottom - Rect[i].top, PATCOPY);

SelectObject(hDC, hBrushOld);
SS_ReleaseDC(lpSS->lpBook, hDC);
}


void SS_ClearEditModeShadow(LPSPREADSHEET lpSS)
{
RECT  Rect[6];
short i;

SS_GetEditModeShadowRects(lpSS, Rect);

for (i = 0; i < 6; i++)
   if (Rect[i].right - Rect[i].left > 0 && Rect[i].bottom - Rect[i].top > 0)
      SS_InvalidateRect(lpSS->lpBook, &Rect[i], TRUE);

SS_UpdateWindow(lpSS->lpBook);
}


void SS_GetEditModeShadowRects(LPSPREADSHEET lpSS, LPRECT lpRect)
{
RECT  RectClient;
short dShadowSize = 2;
short dTop;
short dLeft;
short dRight;
short dBottom;
short x;
short y;
short cx;
short cy;

SS_GetClientRect(lpSS->lpBook, &RectClient);

SS_GetCellCoord(lpSS, lpSS->Col.UL, lpSS->Row.UL,
                lpSS->Col.CurAt, lpSS->Row.CurAt, &x, &y, &cx, &cy);

cx = min(cx, RectClient.right - x);

/**********
* Top - 0
**********/

if ((lpSS->Row.Frozen == 0 && lpSS->Row.CurAt == lpSS->Row.UL) ||
    (lpSS->Row.Frozen && lpSS->Row.CurAt == lpSS->Row.HeaderCnt))
   dTop = y + 1;
else
   dTop = y - 1;

SetRect(&lpRect[0], x + 1, dTop, x + cx, y + 1);

/***********
* Left - 1
***********/

if ((lpSS->Col.Frozen == 0 && lpSS->Col.CurAt == lpSS->Col.UL) ||
    (lpSS->Col.Frozen && lpSS->Col.CurAt == lpSS->Col.HeaderCnt))
   dLeft = x + 1;
else
   dLeft = x - 1;

SetRect(&lpRect[1], dLeft, dTop, x + 1, y + cy);

/*************
* Bottom - 2
*************/

//dBottom = (y + cy) + min(RectClient.bottom - (y + cy), 2);
dBottom = (y + cy) + 2;
SetRect(&lpRect[2], dLeft, y + cy, x + cx, dBottom);

/************
* Right - 3
************/

//dRight = (x + cx) + min(RectClient.right - (x + cx), 2);
dRight = (x + cx) + 2;
SetRect(&lpRect[3], x + cx, dTop, dRight, dBottom);

/**************
* Drop Shadow
**************/

x += 2;
y += 2;

//dRight = (x + cx) + min(RectClient.right - (x + cx), dShadowSize);
dRight = (x + cx) + dShadowSize;
//dBottom = (y + cy) + min(RectClient.bottom - (y + cy), dShadowSize);
dBottom = (y + cy) + dShadowSize;

/*******************
* Right Shadow - 4
*******************/

SetRect(&lpRect[4], x + cx, y + dShadowSize, dRight, y + cy);

/********************
* Bottom Shadow - 5
********************/

SetRect(&lpRect[5], x + dShadowSize, y + cy, dRight, dBottom);
}

#endif

//#ifdef  SPREAD_JPN
void FAR PASCAL LineProc(int X, int Y, LPSTR lpData)
{
    static int    nCount, nDots;
    BYTE          byType;
    LPLINEDATA    lpLineData;
    int           i;

    lpLineData = (LPLINEDATA)lpData;

    if ( lpLineData->fStart ) {
        lpLineData->fStart = FALSE;
        nCount = 0;
        nDots = lpLineData->n1PointDots;
    }
    
    byType = lpLineData->lpLineType[nCount];

    if(byType == 3) {
        nCount = 0;
        nDots = lpLineData->n1PointDots;
        byType = lpLineData->lpLineType[nCount];
    }

    for(i = 0; i < lpLineData->nWidth; i++)
        {
        SetPixel(lpLineData->hDC, X, Y, lpLineData->Color[byType]);
        if(lpLineData->fIsLeft2Right)
             Y++;
        else
             X++;
        }

    nDots--;
    if(nDots == 0)
        {
        nCount++;
        nDots = ((LPLINEDATA)lpData)->n1PointDots;
        }
}

void DrawBorderEx(HDC hDC, int X1, int Y1, int X2, int Y2, COLORREF ForeColor, 
                    COLORREF BackColor, LPBYTE LineType, LPSSX_PRINTFORMAT lpPFormat)
{
    LINEDATA LineData;
    FARPROC  lpfnLineDraw = MakeProcInstance((FARPROC)LineProc, hDynamicInst);
   
    LineData.hDC = hDC;
    LineData.Color[0] = ForeColor;
    LineData.Color[1] = BackColor;
    LineData.fStart = TRUE;
    LineData.fIsLeft2Right = (Y1 == Y2);
    LineData.nWidth = 1;
    LineData.n1PointDots = 1;
    LineData.lpLineType = LineType;

#ifndef SS_V30
    if(lpPFormat)
        {
        int dx, dy;

        X1 = SS_SCREEN2PRINTERX(lpPFormat, X1);
        X2 = SS_SCREEN2PRINTERX(lpPFormat, X2);
        Y1 = SS_SCREEN2PRINTERY(lpPFormat, Y1);
        Y2 = SS_SCREEN2PRINTERY(lpPFormat, Y2);
        
        dx = max(SS_SCREEN2PRINTERX(lpPFormat, 1) - 1, 1);
        dy = max(SS_SCREEN2PRINTERY(lpPFormat, 1) - 1, 1);
        LineData.nWidth = (LineData.fIsLeft2Right) ? dy : dx;
        LineData.n1PointDots = (LineData.fIsLeft2Right) ? dx : dy;
        }
#endif

    LineDDA(X1, Y1, X2, Y2, (LINEDDAPROC)lpfnLineDraw, (LPARAM)(LPLINEDATA)&LineData);
    FreeProcInstance(lpfnLineDraw);
}
//#endif
//-


void SS_CopyCellRect(LPSPREADSHEET lpSS, LPRECT lpRectNew, LPRECT lpRect,
                     LPSSX_PRINTFORMAT lpPFormat, BOOL fOverGrid,
                     BOOL IsLastCol, BOOL IsLastRow)
{
int xPixel = 1;
int yPixel = 1;

CopyRect(lpRectNew, lpRect);

#ifndef SS_V30
if (lpPFormat)
   {
   xPixel = max(SS_SCREEN2PRINTERX(lpPFormat, 1) + 1, 1);
   yPixel = max(SS_SCREEN2PRINTERY(lpPFormat, 1) + 1, 1);
   }
#endif

if ((lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERHORZGRIDONLY ||
     lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID) &&
     (fOverGrid || !(lpSS->wGridType & SS_GRID_HORIZONTAL)) && !IsLastRow)
   lpRectNew->bottom += yPixel;

if ((lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERVERTGRIDONLY ||
     lpSS->lpBook->bBackColorStyle == SS_BACKCOLORSTYLE_OVERGRID) &&
     (fOverGrid || !(lpSS->wGridType & SS_GRID_VERTICAL)) && !IsLastCol)
   lpRectNew->right += xPixel;
}


void SS_DrawCellBlock(LPSPREADSHEET lpSS, SS_COORD Left, SS_COORD Top,
                      SS_COORD Right, SS_COORD Bottom)
{
RECT Rect;

if (SS_CreateBlockRect(lpSS, &Rect, Left, Top, Right, Bottom, FALSE))
   {
   InvalidateRect(lpSS->lpBook->hWnd, &Rect, FALSE);
   UpdateWindow(lpSS->lpBook->hWnd);
   }
}


#ifdef SS_V30
BOOL SS_FocusRect(LPSPREADSHEET lpSS, HDC hDC, LPRECT lpRect, BYTE bFlags)
{
HBITMAP  hBitmap;
HBRUSH   hOldBrush;
COLORREF clrTextOld;
COLORREF clrBackOld;
RECT     RectClient;

if (!hBrushFocusRect)
   {
   static WORD FAR Bits[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

   hBitmap = CreateBitmap(8, 8, 1, 1, (LPSTR)Bits);
   hBrushFocusRect = CreatePatternBrush(hBitmap);
   DeleteObject(hBitmap);
   }

UnrealizeObject(hBrushFocusRect);
SetBrushOrgEx(hDC, lpSS->xFocusRectOrig, 0, 0);
hOldBrush = SelectObject(hDC, hBrushFocusRect);

clrTextOld = GetTextColor(hDC);
SetTextColor(hDC, RGBCOLOR_WHITE);
clrBackOld = GetBkColor(hDC);
SetBkColor(hDC, RGBCOLOR_BLACK);

// draw the dotted lines
SS_GetClientRect(lpSS->lpBook, &RectClient);

// do the top line
if (lpRect->top >= RectClient.top)
   PatBlt(hDC, lpRect->left, lpRect->top, lpRect->right - lpRect->left, 1, PATINVERT);

// do the bottom line
if (lpRect->bottom - 1 < RectClient.bottom)
   PatBlt(hDC, lpRect->left, lpRect->bottom - 1, lpRect->right - lpRect->left, 1, PATINVERT);

#if 0
// do the left vertical line
if (!(bFlags & LB_FOCUSRECT_NOLEFT))
   PatBlt(hDC, lpRect->left, lpRect->top + 1, 1, lpRect->bottom - lpRect->top - 2, PATINVERT);

// do the right vertical line
if (!(bFlags & LB_FOCUSRECT_NORIGHT))
   PatBlt(hDC, lpRect->right - 1, lpRect->top + 1, 1, lpRect->bottom - lpRect->top - 2, PATINVERT);
#endif

SelectObject(hDC, hOldBrush);
SetTextColor(hDC, clrTextOld);
SetBkColor(hDC, clrBackOld);

return (TRUE);
}
#endif // SS_V30


BOOL SS_ShouldCellBeDrawnSelected(LPSPREADSHEET lpSS, SS_COORD Col,
                                  SS_COORD Row, LPSS_ROW lpRow)
{
BOOL fSel = FALSE;

if (!SS_INVERTSEL(lpSS))
   {
   if (lpSS->wOpMode == SS_OPMODE_MULTISEL ||
       lpSS->wOpMode == SS_OPMODE_EXTSEL)
      {
      LPSS_ROW lpRowOrig = lpRow;

      if (!lpRow)
         lpRow = SS_LockRowItem(lpSS, Row);

      if (lpRow && (lpRow->fRowSelected == TRUE || lpRow->fRowSelected == 3))
         fSel = TRUE;

      if (!lpRowOrig)
         SS_UnlockRowItem(lpSS, Row);
      }
   else if (SS_USESINGLESELBAR(lpSS))
      {
      if (Row == lpSS->Row.CurAt)
         fSel = TRUE;
      }

   else if ((SS_IsBlockSelected(lpSS) && lpSS->Row.LR != 0 && lpSS->Col.LR != 0 &&
             ((lpSS->BlockCellLR.Col == -1 || Col == -1 || Col >= lpSS->BlockCellUL.Col) &&
              (lpSS->BlockCellLR.Col == -1 || Col == -1 || Col <= lpSS->BlockCellLR.Col) &&
              (lpSS->BlockCellLR.Row == -1 || Row >= lpSS->BlockCellUL.Row) &&
              (lpSS->BlockCellLR.Row == -1 || Row <= lpSS->BlockCellLR.Row))) ||
            SS_IsCellInSelection(lpSS, Col, Row))
      {
/* RFW - 12/16/98
   else if (SS_IsBlockSelected(lpSS) && lpSS->Row.LR != 0 && lpSS->Col.LR != 0)
      {
      if ((lpSS->BlockCellLR.Col == -1 || Col == -1 || Col >= lpSS->BlockCellUL.Col) &&
          (lpSS->BlockCellLR.Col == -1 || Col == -1 || Col <= lpSS->BlockCellLR.Col) &&
          (lpSS->BlockCellLR.Row == -1 || Row >= lpSS->BlockCellUL.Row) &&
          (lpSS->BlockCellLR.Row == -1 || Row <= lpSS->BlockCellLR.Row))
*/
         {
         fSel = TRUE;

         if (Col == lpSS->Col.CurAt && Row == lpSS->Row.CurAt)
				{
#ifdef SS_UTP
				if (!lpSS->fNoShowActiveWithSel)
#endif
					if ((lpSS->Row.CurAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen ||
					     (lpSS->Row.CurAt >= lpSS->Row.UL - 1 &&
							lpSS->Row.CurAt <= (lpSS->fVirtualMode ?
							SS_GetBottomCell(lpSS, lpSS->Row.UL) : lpSS->Row.LR) + 1)) &&
						 (lpSS->Col.CurAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen ||
						  (lpSS->Col.CurAt >= lpSS->Col.UL - 1 &&
							lpSS->Col.CurAt <= lpSS->Col.LR + 1)))
						fSel = FALSE;
				}
         }
      }
   }

return (fSel);
}

#ifdef SS_V35
BOOL SS_DrawCellNote(LPSPREADSHEET lpSS, HDC hdc, LPRECT lpRect, SS_COORD Col, SS_COORD Row)
{
  HBRUSH hbrNote;
  RECT rcNote;
  int xSize = MulDiv(45, dxPixelsPerInch, 1440);
  int ySize = MulDiv(45, dyPixelsPerInch, 1440);
  
  if (lpRect)
    CopyRect(&rcNote, lpRect);
  else
    SS_GetCellRect(lpSS, Col, Row, &rcNote);

  xSize = min(xSize, rcNote.right - rcNote.left);
  ySize = min(ySize, rcNote.bottom - rcNote.top);
  if( xSize > 0 && ySize > 0 )
  {
    rcNote.left = rcNote.right - xSize-1;
    rcNote.top = rcNote.top;  
    rcNote.right = rcNote.right;
    rcNote.bottom = rcNote.top + ySize+1;
#ifdef SS_V80
    hbrNote = CreateSolidBrush(lpSS->lpBook->CellNoteIndicatorColor);
#else
    hbrNote = CreateSolidBrush(RGB(255,0,0));
#endif
#ifdef SS_V80
	switch (lpSS->lpBook->wCellNoteIndicatorShape)
	{
	case SS_CELLNOTEINDICATORSHAPE_SQUARE:
		FillRect(hdc, &rcNote, hbrNote);
		break;
#if SS_V80
	case SS_CELLNOTEINDICATORSHAPE_TRIANGLE:
		{
			POINT pts[3];
			HPEN hPen, hPenOld;
			HBRUSH hbrOld;
			pts[0].x = rcNote.left;
			pts[0].y = rcNote.top; 
			pts[1].x = rcNote.right;
			pts[1].y = rcNote.top; 
			pts[2].x = rcNote.right;
			pts[2].y = rcNote.bottom; 
			hPen = CreatePen(PS_SOLID, 0, lpSS->lpBook->CellNoteIndicatorColor);
			hPenOld = SelectObject(hdc, hPen);
			hbrOld = SelectObject(hdc, hbrNote);
			Polygon(hdc, pts, 3);
			SelectObject(hdc, hbrOld);
			SelectObject(hdc, hPenOld);
			DeleteObject(hPen);
		}
		break;
#endif
	}
#else
		FillRect(hdc, &rcNote, hbrNote);
#endif
    DeleteObject(hbrNote);
  }
  return TRUE;
}
#endif // SS_V35


short SS_GetColWidthFromList(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat, SS_COORD LeftCol, SS_COORD Col)
{
short nRet = -1;

if (lpPFormat && lpPFormat->lpColWidthList && lpPFormat->lColWidthListCnt)
	{
	SS_COORD ColAt = max(LeftCol, lpSS->Col.HeaderCnt + lpSS->Col.Frozen);
	SS_COORD ColHeaderAt = lpPFormat->pf.fShowRowHeaders ? 0 : lpSS->Col.HeaderCnt;
	SS_COORD TempColAt;
   long     lColWidthListAt;

	for (lColWidthListAt = 0; lColWidthListAt < lpPFormat->lColWidthListCnt && nRet == -1; lColWidthListAt++)
		{
		if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
			TempColAt = ColHeaderAt;
		else
			TempColAt = ColAt;

		if (Col == TempColAt)
			nRet = lpPFormat->lpColWidthList[lColWidthListAt];

		if (ColHeaderAt < lpSS->Col.HeaderCnt + lpSS->Col.Frozen)
			ColHeaderAt++;
		else
			ColAt++;
		}
	}

return (nRet);
}


short SS_GetRowHeightFromList(LPSPREADSHEET lpSS, LPSSX_PRINTFORMAT lpPFormat, SS_COORD TopRow, SS_COORD Row)
{
short nRet = -1;

if (lpPFormat && lpPFormat->lpRowHeightList && lpPFormat->lRowHeightListCnt)
	{
	SS_COORD RowAt = max(TopRow, lpSS->Row.HeaderCnt + lpSS->Row.Frozen);
	SS_COORD RowHeaderAt = 0;
	SS_COORD TempRowAt;
   long     lRowHeightListAt;

	for (lRowHeightListAt = 0; lRowHeightListAt < lpPFormat->lRowHeightListCnt && nRet == -1; lRowHeightListAt++)
		{
		if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
			TempRowAt = RowHeaderAt;
		else
			TempRowAt = RowAt;

		if (Row == TempRowAt)
			nRet = lpPFormat->lpRowHeightList[lRowHeightListAt];

		if (RowHeaderAt < lpSS->Row.HeaderCnt + lpSS->Row.Frozen)
			RowHeaderAt++;
		else
			RowAt++;
		}
	}

return (nRet);
}

#if SS_V80

BOOL SS_CustCellTypeGetAppearance(LPSPREADSHEET lpSS, LPSS_CELLTYPE lpCellType, SS_COORD Col, SS_COORD Row, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPSS_CT_APPEARANCE app)
{
   SS_FONTID FontId;
   LPSS_FONT Font;
   SS_FONT   FontTemp;
   SS_COLORTBLITEM BackColorTblItem;
   SS_COLORTBLITEM ForeColorTblItem;
   SS_COLORTBLITEM ColorTblItem;
   LPSS_COL lpCol = SS_LockColItem(lpSS, Col);
   LPSS_ROW lpRow = SS_LockRowItem(lpSS, Row);
   LPSS_CELL lpCell = SS_LockCellItem(lpSS, Col, Row);

   SS_GetColorTblItem(lpSS, Col, Row, &BackColorTblItem, &ForeColorTblItem);
   app->clrBackColor = BackColorTblItem.Color;
   app->clrForeColor = ForeColorTblItem.Color;
   SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.BackgroundId);
   app->clrSelBackColor = ColorTblItem.Color;
   SS_GetColorItem(&ColorTblItem, lpSS->lpBook->SelColor.ForegroundId);
   app->clrSelForeColor = ColorTblItem.Color;
   SS_GetColorItem(&ColorTblItem, lpSS->LockColor.BackgroundId);
   app->clrLockBackColor = ColorTblItem.Color;
   SS_GetColorItem(&ColorTblItem, lpSS->LockColor.ForegroundId);
   app->clrLockForeColor = ColorTblItem.Color;
   app->lStyle = lpCellType->Style;
   app->fDrawPrimaryButton = SS_GetButtonDrawMode(lpSS->lpBook) & SS_BDM_ALWAYSBUTTON;
   app->fDrawSecondaryButton = SS_GetButtonDrawMode(lpSS->lpBook) & SS_BDM_ALWAYSCOMBO;
   app->fDrawColors = !lpPFormat || lpPFormat->pf.fDrawColors;

   FontId = SSx_RetrieveFontId(lpSS, lpCol, lpRow, lpCell, Col, Row);
   Font = SS_GetFont(lpSS->lpBook, &FontTemp, FontId);

   if (lpPFormat)
      app->hFont = SS_CreatePrinterFont(lpSS, hDC, Font, FontId, lpPFormat);
   else
      app->hFont = Font->hFont;

   SS_UnlockColItem(lpSS, Col);
   SS_UnlockRowItem(lpSS, Row);
   SS_UnlockCellItem(lpSS, Col, Row);
   return TRUE;
}


#if 0
void SS_AlphaBlendSelection(LPSPREADSHEET lpSS, HDC hDC)
{
BLENDFUNCTION bf;
RECT          rectClient;
int           iWidth, iHeight;

GetClientRect(lpSS->lpBook->hWnd, &rectClient);

iWidth = rectClient.right - rectClient.left;
iHeight = rectClient.bottom - rectClient.top;

bf.BlendOp = AC_SRC_OVER;
bf.BlendFlags = 0;
bf.SourceConstantAlpha = 100;
bf.AlphaFormat = 0;             // ignore source alpha channel

/*
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.AlphaFormat = AC_SRC_ALPHA;   // use source alpha 
    bf.SourceConstantAlpha = 0xbf;   // use constant alpha, with 
*/

AlphaBlend(hDC, 0, 0, iWidth, iHeight, 
           hDC, 0, 0, iWidth, iHeight, bf);
}
#endif

HBITMAP SS_AlphaBlendCreateBitmap(LPSS_BOOK lpBook, HDC hDC, int iWidth, int iHeight)
{
// RFW - 4/17/09 - 25440
iWidth = min(iWidth, lpBook->ClientRect.right - lpBook->ClientRect.left);
iHeight = min(iHeight, lpBook->ClientRect.bottom - lpBook->ClientRect.top);

if (!lpBook->hbmpAlpha || iWidth > lpBook->iAlphaBitmapWidth || iHeight > lpBook->iAlphaBitmapHeight)
	{
	BITMAPINFO bmi;        // bitmap header
	VOID      *pvBits;     // pointer to DIB section
	int        x,y;        // stepping variables
	COLORREF   clr = SS_TranslateColor(lpBook->clrHighlightAlphaBlend);

	// zero the memory for the bitmap info
	ZeroMemory(&bmi, sizeof(BITMAPINFO));

	iWidth = max(iWidth, lpBook->iAlphaBitmapWidth);
	iHeight = max(iHeight, lpBook->iAlphaBitmapHeight);

	// setup bitmap info 
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = iHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = iWidth * iHeight * 4;

	// Reverse the RGB values
	clr = ((clr & 0xff) << 16) + (clr & 0xff00) + ((clr & 0xff0000) >> 16);

	if (lpBook->hbmpAlpha)
		DeleteObject(lpBook->hbmpAlpha);

	// create our DIB section
	lpBook->hbmpAlpha = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);

	// set all pixels to specified color and set source alpha to zero
	for (y = 0; y < iHeight; y++)
		for (x = 0; x < iWidth; x++)
			((UINT32 *)pvBits)[x + y * iWidth] = clr;

	lpBook->iAlphaBitmapWidth = iWidth;
	lpBook->iAlphaBitmapHeight = iHeight;
	}

return (lpBook->hbmpAlpha);
}


void SS_AlphaBlendFree(LPSS_BOOK lpBook)
{
if (lpBook->hbmpAlpha)
	DeleteObject(lpBook->hbmpAlpha);

lpBook->hbmpAlpha = 0;
lpBook->iAlphaBitmapWidth = 0;
lpBook->iAlphaBitmapHeight = 0;
}

typedef BOOL (__stdcall *ALPHA_BLEND)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);

void SS_AlphaBlendSelection(LPSS_BOOK lpBook, HDC hDC, LPRECT lpRect)
{
HDC           hdcTemp; // handle of the DC we will create 
BLENDFUNCTION bf;      // structure for alpha blending
HBITMAP       hBitmap; // bitmap handle
int           iWidth = min(lpBook->ClientRect.right, lpRect->right) - lpRect->left;   // RFW - 4/17/09 - 25440
int           iHeight = min(lpBook->ClientRect.bottom, lpRect->bottom) - lpRect->top; // RFW - 4/17/09 - 25440
HINSTANCE     hInstMsimg;
ALPHA_BLEND   pAlphaBlend = NULL;

// make sure we have at least some window size
if ((!iWidth) || (!iHeight))
	return;

// make sure we can call AlphaBlend
hInstMsimg = LoadLibrary(_T("msimg32.dll"));
if( hInstMsimg != NULL )
   pAlphaBlend = (ALPHA_BLEND)GetProcAddress(hInstMsimg, "AlphaBlend");

if( pAlphaBlend )
{
   // create a DC for our bitmap -- the source DC for AlphaBlend 
   hdcTemp = CreateCompatibleDC(hDC);
   hBitmap = SS_AlphaBlendCreateBitmap(lpBook, hDC, iWidth, iHeight);
   SelectObject(hdcTemp, hBitmap);

   bf.BlendOp = AC_SRC_OVER;
   bf.BlendFlags = 0;
   bf.SourceConstantAlpha = lpBook->bHighlightAlphaBlend;
   bf.AlphaFormat = 0;             // ignore source alpha channel

   //AlphaBlend(hDC, lpRect->left, lpRect->top, iWidth, iHeight, 
   //           hdcTemp, 0, 0, iWidth, iHeight, bf);
   pAlphaBlend(hDC, lpRect->left, lpRect->top, iWidth, iHeight, 
               hdcTemp, 0, 0, iWidth, iHeight, bf);

   // do cleanup
   DeleteDC(hdcTemp);
}

if( hInstMsimg != NULL )
   FreeLibrary(hInstMsimg);
}

#define HIMETRIC_INCH   2540    // HIMETRIC units per inch
#define TWIPS_INCH      1440

void DPtoHIMETRIC(HDC hDC, LPSIZE lpSize)
{
	int nMapMode;
	if ((nMapMode = GetMapMode(hDC)) < MM_ISOTROPIC &&
		nMapMode != MM_TEXT)
	{
		// when using a constrained map mode, map against physical inch
		SetMapMode(hDC, MM_HIMETRIC);
		DPtoLP(hDC, (LPPOINT)lpSize, 1);
		SetMapMode(hDC, nMapMode);
	}
	else
	{
		// map against logical inch for non-constrained mapping modes
		int cxPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
      int cyPerInch = GetDeviceCaps(hDC, LOGPIXELSY);
		lpSize->cx = MulDiv(lpSize->cx, HIMETRIC_INCH, cxPerInch);
		lpSize->cy = MulDiv(lpSize->cy, HIMETRIC_INCH, cyPerInch);
	}
}

void SS_CT_Render(LPSPREADSHEET lpSS, LPSS_CELLTYPE CellType, HDC hDC, LPSSX_PRINTFORMAT lpPFormat, LPRECT CellRect, LPRECT CellRectOrigNoScale, SS_COORD CellCol, SS_COORD CellRow, BOOL fCellSelected)
{
   HWND hWndCT = SS_GetControlRenderer(lpSS, CellType);
   int iSaveDC;
   SS_CT_APPEARANCE app;
   HBRUSH hBrush;
   HDC hdctemp;
   HBITMAP hbmp, hbmpold;
   HDC hdcSpread = GetDC(lpSS->lpBook->hWnd);
   RECT rcPaint = {0, 0, RC_WIDTH(CellRectOrigNoScale), RC_HEIGHT(CellRectOrigNoScale)};
   
   if( hWndCT == NULL )
     return; // can't draw without hWndCT
   SS_CT_InitializeRendererControl(lpSS, CellType, CellCol, CellRow);
   SS_CT_SetRendererValue(lpSS, CellType, CellCol, CellRow);
   // in case it got recreated (e.g. set BorderStyle)
   hWndCT = SS_GetControlRenderer(lpSS, CellType);
   SetWindowPos(hWndCT, NULL, 0, 0, RC_WIDTH(CellRectOrigNoScale), RC_HEIGHT(CellRectOrigNoScale), SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
   iSaveDC = SaveDC(hDC);
   SS_CustCellTypeGetAppearance(lpSS, CellType, CellCol, CellRow, hDC, lpPFormat, &app);
   hBrush = CreateSolidBrush(app.clrBackColor);
   hdctemp = CreateCompatibleDC(hdcSpread);
   hbmp = CreateCompatibleBitmap(hdcSpread, RC_WIDTH(CellRectOrigNoScale), RC_HEIGHT(CellRectOrigNoScale));
   hbmpold = SelectObject(hdctemp, hbmp);
   FillRect(hdctemp, &rcPaint, hBrush);
   DeleteObject(hBrush);
   fpRenderWindow(hWndCT, hdctemp, &rcPaint, FALSE);
   SetStretchBltMode(hDC, COLORONCOLOR);
   StretchBlt(hDC, CellRect->left, CellRect->top, RC_WIDTH(CellRect), RC_HEIGHT(CellRect), hdctemp, 0, 0, RC_WIDTH(CellRectOrigNoScale), RC_HEIGHT(CellRectOrigNoScale), SRCCOPY);
   SelectObject(hdctemp, hbmpold);
   DeleteObject(hbmp);
   DeleteDC(hdctemp);
   ReleaseDC(lpSS->lpBook->hWnd, hdcSpread);
   RestoreDC(hDC, iSaveDC);
}

BOOL DLLENTRY PaintControl(HWND hWnd, HDC hDC, LPRECT lpRect, BOOL fClientOnly)
{
return fpRenderWindow(hWnd, hDC, lpRect, fClientOnly);
}
#endif // SS_V80
