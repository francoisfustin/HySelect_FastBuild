//--------------------------------------------------------------------
//
//  File: ss_tab.c
//
//  Description: Tab control object
//
//  Copyright (c) 2003-2004 by FarPoint Technologies, Inc.
//
//  All rights reserved.  No part of this source code may be
//  copied, modified or reproduced in any form without retaining
//  the above copyright notice.  This source code, or source code
//  derived from it, may not be redistributed without express
//  written permission of FarPoint Technologies, Inc.
//
//--------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <malloc.h>
#include "spread.h"
#include "ss_book.h"
#include "ss_draw.h"
#include "ss_tab.h"
#include "..\..\vbx\stringrc.h"
#ifdef SS_V80
#include "uxtheme.h"
#include "ss_theme.h"
#endif

extern HANDLE hDynamicInst;

#define SPRD_ZOOM_INITVAL 100

#define TABBTN_INITIALDELAY 350
#define TABBTN_TIMEDELAY    100

void SS_DrawScrollHomeLeft(HDC hdc, RECT *prc, short nZoom, BOOL bPushed);
void SS_DrawScrollEndRight(HDC hdc, RECT *prc, short nZoom, BOOL bPushed);
void SS_DrawScrollLineLeft(HDC hdc, RECT *prc, short nZoom, BOOL bPushed);
void SS_DrawScrollLineRight(HDC hdc, RECT *prc, short nZoom, BOOL bPushed);
#ifdef SS_V80
void SS_DrawScrollHomeLeftVisualStyles(HDC hdc, RECT *prc, short nZoom, BOOL bPushed, BOOL bMouseOver);
void SS_DrawScrollEndRightVisualStyles(HDC hdc, RECT *prc, short nZoom, BOOL bPushed, BOOL bMouseOver);
void SS_DrawScrollLineLeftVisualStyles(HDC hdc, RECT *prc, short nZoom, BOOL bPushed, BOOL bMouseOver);
void SS_DrawScrollLineRightVisualStyles(HDC hdc, RECT *prc, short nZoom, BOOL bPushed, BOOL bMouseOver);

void SS_DrawScrollButtonEnhanced(HDC hdc, LPSS_BOOK lpBook, RECT *prc, int button, short nZoom, BOOL bPushed, BOOL bMouseOver);

#endif
BOOL SSTabx_GetTabRect(LPSS_BOOK lpBook, HDC hDC, short nSheet, LPPOINT lpMouse, LPRECT lpRectBounds, LPRECT lpRectTab, LPSHORT lpnTabLast);
BOOL SSTab_GetTabRect(LPSS_BOOK lpBook, HDC hDC, short nSheet, LPRECT lpRectBounds, LPRECT lpRectTab);
BOOL SSTab_GetTabRectFromMouse(LPSS_BOOK lpBook, HDC hDC, LPPOINT lpMouse, LPRECT lpRectBounds, LPRECT lpRectTab, LPSHORT lpnSheet);
short SSTab_GetRightSheet(LPSS_BOOK lpBook, HDC hDC, LPRECT lpRectBounds);
short SSTab_GetTabWidth(LPSS_BOOK lpBook, HDC hDC, short nSheetIndex);

//--------------------------------------------------------------------
//
//  The following structure represents a tab control object.
//

typedef struct tagSS_TAB
{
  long lRefCnt;
  HWND hWndSS;
  int nBtnDown;    // (0=none, 1=Btn1, 2=Btn2, 3=Btn3, 4=Btn4)
  int nBtnOver;    // (0=none, 1=Btn1, 2=Btn2, 3=Btn3, 4=Btn4)
} SS_TAB;

//--------------------------------------------------------------------
//
//  The following macros handle memory management for tab objects.
//

#define SSTab_MemAlloc() (HSS_TAB)tbGlobalAlloc(GHND, sizeof(SS_TAB))
#define SSTab_MemFree(hMem) tbGlobalFree(hMem)
#define SSTab_MemLock(hMem) (SS_TAB *)tbGlobalLock(hMem)
#define SSTab_MemUnlock(hMem) tbGlobalUnlock(hMem)

//--------------------------------------------------------------------
//
//  The following function creates a tab object.
//

HSS_TAB SSTab_Create(void)
{
  SS_TAB *pThis;
  HSS_TAB hThis;

  if( (hThis = SSTab_MemAlloc()) && (pThis = SSTab_MemLock(hThis)) )
  {
    pThis->lRefCnt = 1;
    pThis->hWndSS = NULL;
    pThis->nBtnDown = 0;
    pThis->nBtnOver = 0;
    SSTab_MemUnlock(hThis);
  }
  return hThis;
}

//--------------------------------------------------------------------
//
//  The following function adds a reference to the tab object.
//

long SSTab_AddRef(HSS_TAB hThis)
{
  SS_TAB *pThis;
  long lRefCnt = 0;

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    lRefCnt = ++pThis->lRefCnt;
    SSTab_MemUnlock(hThis);
  }
  return lRefCnt;
}

//--------------------------------------------------------------------
//
//  The following function releases a reference to a tab object.
//  When the last reference is released, the object is destroyed.
//

long SSTab_Release(HSS_TAB hThis)
{
  SS_TAB *pThis;
  long lRefCnt = 0;

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    lRefCnt = --pThis->lRefCnt;
    SSTab_MemUnlock(hThis);
    if( 0 == lRefCnt )
      SSTab_MemFree(hThis);
  }
  return lRefCnt;
}

//--------------------------------------------------------------------
//
//  The following function attachs the tab to the book.  This
//  function should be called by the book when the book is
//  ready to receive communication from the tab (i.e. after
//  the book is fully initialized).
//

void SSTab_Attach(HSS_TAB hThis, HWND hWndSS)
{
  SS_TAB *pThis;

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    pThis->hWndSS = hWndSS;
    SSTab_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------
//
//  The following function detaches the tab from the book.  This
//  function should be called by the book when the book no longer
//  wishes to receive communication from the tab (i.e. before
//  the book starts to free itself).
//

void SSTab_Detach(HSS_TAB hThis)
{
  SS_TAB *pThis;

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    pThis->hWndSS = NULL;
    SSTab_MemUnlock(hThis);
  }
}

//--------------------------------------------------------------------
//
//  The following function divides the tab rectangle into smaller
//  sub rectangles for buttons and sheet tabs.
//

BOOL SSTabPtr_CalcRects(LPSS_BOOK lpBook, RECT *prc, RECT *prcBtn1, RECT *prcBtn2, RECT *prcBtn3, RECT *prcBtn4, RECT *prcTabs)
{
  int cxHScroll = GetSystemMetrics(SM_CXHSCROLL);
  int cxMaxBtn = (prc->right - prc->left) / 8;
  int cxBtn = min(cxHScroll, cxMaxBtn);
  short width = 0;

#ifdef SS_V80
  if (lpBook->wTabStripButtonPolicy == SS_TABSTRIPBUTTONPOLICY_ASNEEDED)
  {
  HDC hdc = GetDC(lpBook->hWnd);
  short i;
  for (i = 0; i < lpBook->nSheetCnt; i++)
	  width += SSTab_GetTabWidth(lpBook, hdc, i);
  ReleaseDC(lpBook->hWnd, hdc);

  if (width <= prc->right - prc->left)
	  cxBtn = 0;
  }
  else if (lpBook->wTabStripButtonPolicy == SS_TABSTRIPBUTTONPOLICY_NEVER)
	  cxBtn = 0;
#endif
  if( prcBtn1 )
	{
		prcBtn1->left = prc->left;
		prcBtn1->top = prc->top;;
		prcBtn1->right = prc->left + cxBtn;
		prcBtn1->bottom = prc->bottom;
	}
	if( prcBtn2 )
	{
		prcBtn2->left = prc->left + cxBtn;
		prcBtn2->top = prc->top;;
		prcBtn2->right = prc->left + 2 * cxBtn;
		prcBtn2->bottom = prc->bottom;
	}
	if( prcBtn3 )
	{
	    prcBtn3->left = prc->left + 2 * cxBtn;
		prcBtn3->top = prc->top;;
		prcBtn3->right = prc->left + 3 * cxBtn;
		prcBtn3->bottom = prc->bottom;
	}
	if( prcBtn4 )
	{
		prcBtn4->left = prc->left + 3 * cxBtn;
		prcBtn4->top = prc->top;;
		prcBtn4->right = prc->left + 4 * cxBtn;
		prcBtn4->bottom = prc->bottom;
	}

    if( prcTabs )
	{
		LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
		if (lpSS)
		{
			int x = SS_GetCellPosX(lpSS, lpSS->Col.Frozen != 0 ? lpSS->Col.HeaderCnt : lpSS->Col.UL, lpSS->Col.Frozen != 0 ? lpSS->Col.HeaderCnt :lpSS->Col.UL);
			SS_BookUnlockActiveSheet(lpBook);
			
			prcTabs->left = cxBtn == 0 ? x : prc->left + 4*cxBtn;
			prcTabs->top = prc->top;;
			prcTabs->right = prc->right;
			prcTabs->bottom = prc->bottom;
		}
	}
  return cxBtn > 0;
}

//--------------------------------------------------------------------

//#define SS_TABWIDTH     75
#define SS_TABOVERLAP   7
#define SS_TABMARGIN    5

//--------------------------------------------------------------------
/*
void RotBlt(HDC destDC, int srcx1, int srcy1, int srcx2, int srcy2,
  HDC srcDC , int destx1, int desty1 ,int thetaInDegrees ,DWORD mode)
{
  double theta = thetaInDegrees * (3.14159/180);
  //multiply degrees by PI/180 to convert to radians

  //determine width and height of source
  int width = srcx2 - srcx1;
  int height = srcy2 - srcy1;

  //determine centre/pivot point ofsource
  int centreX = int(float(srcx2 + srcx1)/2);
  int centreY = int(float(srcy2 + srcy1)/2);

  //since image is rotated we need to allocate a rectangle
  //which can hold the image in any orientation
  if(width>height)height = width;
  else
    width = height;


  //allocate memory and blah blah
  HDC memDC = CreateCompatibleDC(destDC);
  HBITMAP memBmp = CreateCompatibleBitmap(destDC, width, height);

  HBITMAP obmp = (HBITMAP) SelectObject(memDC, memBmp);

  //pivot point of our mem DC
  int newCentre = int(float(width)/2);

  //hmmm here's the rotation code. X std maths :|
  for(int x = srcx1; x<=srcx2; x++)
    for(int y = srcy1; y<=srcy2; y++)
    {
      COLORREF col = GetPixel(srcDC,x,y);

      int newX = int((x-centreX)*sin(theta)+(y-centreY)*cos(theta));
      int newY = int((x-centreX)*cos(theta)-(y-centreY)*sin(theta));


      SetPixel(memDC , newX + newCentre, newY + newCentre, col);
    }

  //splash onto the destination
  BitBlt(destDC, destx1, desty1, width, height, memDC, 0,0,mode);


  //free mem and blah
  SelectObject(memDC, obmp);

  DeleteDC(memDC);
  DeleteObject(memBmp);
}
*/
#ifdef SS_V80
void SSTabPtr_DrawSheetTabVisualStyles(SS_TAB *pThis, HDC hdc, LPTSTR pszSheetName, RECT *prcSheetTab, BOOL bActive, BOOL bBottom, BOOL bMouseOver)
{
  int tabType;
  int tabState;
  HDC memDC;
  HBITMAP membmpDC;
  HBITMAP hbmpOld;
  BOOL b = S_OK;
  int nFlags = 0;
  RECT rcText;
  LPWSTR lpWStr = NULL;
  int nlen = 0;
  int strLen = 0;
  RECT rc = {0,0, prcSheetTab->right-prcSheetTab->left, prcSheetTab->bottom-prcSheetTab->top};
  HTHEME hTheme = NULL;

  hTheme = SS_OpenThemeData(pThis->hWndSS, L"Tab");
  tabType = bBottom ? 5 : 1;
  tabState = bActive || bMouseOver  ? 2 : 1;
  memDC = CreateCompatibleDC(hdc);
  membmpDC = CreateCompatibleBitmap(hdc, prcSheetTab->right-prcSheetTab->left, prcSheetTab->bottom-prcSheetTab->top);
  hbmpOld = SelectObject(memDC, membmpDC);
  b = SS_DrawThemeBackground( hTheme, memDC, 1, tabState, &rc, &rc);
  SS_FlipBitmap(memDC, rc, FALSE);
  StretchBlt(hdc, prcSheetTab->left, prcSheetTab->top, prcSheetTab->right-prcSheetTab->left, prcSheetTab->bottom-prcSheetTab->top, memDC, 0, 0, rc.right-rc.left, rc.bottom-rc.top, SRCCOPY);
  if( pszSheetName )
  {
    rcText.left = prcSheetTab->left + SS_TABOVERLAP + 1;
    rcText.top = prcSheetTab->top + 1;
    rcText.right = prcSheetTab->right - SS_TABOVERLAP - 1;
    rcText.bottom = prcSheetTab->bottom - 1;
	
	strLen = lstrlen(pszSheetName);
#ifndef _UNICODE
	nlen = MultiByteToWideChar(CP_ACP, 0, pszSheetName, -1, NULL, 0);
    //lpWStr = (LPWSTR) SysAllocStringLen(NULL, nlen - 1);
    lpWStr = (LPWSTR)malloc(nlen * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, pszSheetName, strLen, lpWStr, nlen);
#else
   lpWStr = (LPWSTR)pszSheetName;
#endif

	nFlags = DT_SINGLELINE | DT_CENTER;

	SS_DrawThemeText(hTheme, hdc, 1, tabState, lpWStr, strLen, nFlags, 0, &rcText);
	//SysFreeString(lpWStr);
#ifndef _UNICODE
	free(lpWStr);
#endif
  }
 
  SS_CloseThemeData(hTheme);
  SelectObject(memDC, hbmpOld);
  DeleteDC(memDC);
  DeleteObject(membmpDC);
}

void SSTabPtr_DrawSheetTabEnhanced(SS_TAB *pThis, HDC hdc, WORD nStyle, LPTSTR pszSheetName, RECT *prcSheetTab, BOOL bActive, BOOL bBottom, BOOL bMouseOver,
								  	COLORREF sheetTabUpperNormalStartColor, COLORREF sheetTabUpperNormalEndColor, COLORREF sheetTabLowerNormalStartColor ,
									COLORREF sheetTabLowerNormalEndColor, COLORREF sheetTabUpperHoverStartColor,  COLORREF sheetTabUpperHoverEndColor,
									COLORREF sheetTabLowerHoverStartColor, COLORREF sheetTabLowerHoverEndColor, COLORREF sheetTabOuterBorderColor, COLORREF sheetTabInnerBorderColor, 
									COLORREF sheetTabForeColor, COLORREF sheetTabActiveForeColor)
{

	POINT ptOuter[11];
	POINT ptOuter2[11];
	POINT ptInner[11];
	HPEN pen, penOld;
	RECT rcText;
	COLORREF crOld;
	int	pts;
	int offset, width2;
	BOOL enabled = TRUE;
	HRGN rgn = CreateRectRgn(0,0,0,0);
	int height = prcSheetTab->bottom - prcSheetTab->top;
	int width = prcSheetTab->right - prcSheetTab->left;

	RECT rc =  {prcSheetTab->left, prcSheetTab->top, prcSheetTab->right - 1, prcSheetTab->bottom};
	RECT rcInner = {rc.left+1, rc.top+1, rc.right-1, rc.bottom-1};

	RECT rcUpper = {rc.left, rc.top, rc.right, rc.top + (rc.bottom-rc.top) / 2};
	RECT rcLower = {rc.left, rcUpper.bottom, rc.right, rc.bottom};

	COLORREF lightGray = RGB(192,192,192);
    COLORREF crText = bActive ? sheetTabActiveForeColor : sheetTabForeColor;
       
	HBRUSH upperBrush1 = CreateSolidBrush(sheetTabUpperNormalStartColor);
	HBRUSH upperBrush2 = CreateSolidBrush(sheetTabUpperNormalEndColor);
	HBRUSH lowerBrush1 = CreateSolidBrush(sheetTabLowerNormalStartColor);
	HBRUSH lowerBrush2 = CreateSolidBrush(sheetTabLowerNormalEndColor);

    if ((bMouseOver || bActive) && enabled)
        {
          DeleteObject(upperBrush1);
          DeleteObject(upperBrush2);
          DeleteObject(lowerBrush1);
          DeleteObject(lowerBrush2);
		  upperBrush1 = CreateSolidBrush(sheetTabUpperHoverStartColor);
		  upperBrush2 = CreateSolidBrush(sheetTabUpperHoverEndColor);
		  lowerBrush1 = CreateSolidBrush(sheetTabLowerHoverStartColor);
		  lowerBrush2 = CreateSolidBrush(sheetTabLowerHoverEndColor);
        }
    else if (!enabled)
        {
          DeleteObject(upperBrush1);
          DeleteObject(upperBrush2);
          DeleteObject(lowerBrush1);
          DeleteObject(lowerBrush2);
		  upperBrush1 = CreateSolidBrush(lightGray);
		  upperBrush2 = CreateSolidBrush(lightGray);
		  lowerBrush1 = CreateSolidBrush(lightGray);
		  lowerBrush2 = CreateSolidBrush(lightGray);
        }

	if (nStyle == SS_TABENHANCEDSHAPE_RECTANGLE)
	   {
        int x2 = prcSheetTab->left;
        width2 = width-8;
        offset = 8;
		pts = 5;

		ptOuter[0].x = x2;
		ptOuter[0].y = prcSheetTab->top;
		ptOuter[1].x = x2 + width2 - 1;
		ptOuter[1].y = prcSheetTab->top ;
		ptOuter[2].x = x2+ width2- 1;
		ptOuter[2].y = prcSheetTab->top + height - 1;
		ptOuter[3].x = x2;
		ptOuter[3].y = prcSheetTab->top + height - 1;
		ptOuter[4].x = x2;
		ptOuter[4].y = prcSheetTab->top;
		ptOuter2[0].x = x2;
		ptOuter2[0].y = prcSheetTab->top;
		ptOuter2[1].x = x2 + width2;
		ptOuter2[1].y = prcSheetTab->top;
		ptOuter2[2].x = x2 + width2;
		ptOuter2[2].y = prcSheetTab->top + height;
		ptOuter2[3].x = x2;
		ptOuter2[3].y = prcSheetTab->top + height;
		ptOuter2[4].x = x2;
		ptOuter2[4].y = prcSheetTab->top;
		ptInner[0].x = x2 + 1;
		ptInner[0].y = prcSheetTab->top + 1;
		ptInner[1].x = x2 + width2-2;
		ptInner[1].y = prcSheetTab->top + 1 ;
		ptInner[2].x = x2 + width2 - 2;
		ptInner[2].y = prcSheetTab->top + height - 2;
		ptInner[3].x = x2 + 1;
		ptInner[3].y = prcSheetTab->top + height - 2;        
		ptInner[4].x = x2 + 1;
		ptInner[4].y = prcSheetTab->top  + 1;   

	   }
	   else if (nStyle == SS_TABENHANCEDSHAPE_ROUNDEDRECTANGLE)
	   {
        int x2 = prcSheetTab->left;
        width2 = width-8;
		pts = 11;
		offset = 8;

		ptOuter[0].x = x2;
		ptOuter[0].y = prcSheetTab->top;
		ptOuter[1].x = x2;
		ptOuter[1].y = prcSheetTab->top + height - 6;
		ptOuter[2].x = x2 + 1;
		ptOuter[2].y = prcSheetTab->top + height - 3;
		ptOuter[3].x = x2 + 3;
		ptOuter[3].y = prcSheetTab->top + height - 1;
		ptOuter[4].x = x2 + 6;
		ptOuter[4].y = prcSheetTab->top + height;
		ptOuter[5].x = x2+ width2 - 7;
		ptOuter[5].y = prcSheetTab->top + height;
		ptOuter[6].x = x2+ width2 - 4;
		ptOuter[6].y = prcSheetTab->top + height - 1;
		ptOuter[7].x = x2 + width2 - 2;
		ptOuter[7].y = prcSheetTab->top + height - 3;
		ptOuter[8].x = x2 + width2 - 1;
		ptOuter[8].y = prcSheetTab->top + height - 6;
		ptOuter[9].x = x2 + width2 - 1;
		ptOuter[9].y = prcSheetTab->top ;
		ptOuter[10].x = x2;
		ptOuter[10].y = prcSheetTab->top;

		ptOuter2[0].x = x2;
		ptOuter2[0].y = prcSheetTab->top;
		ptOuter2[1].x = x2;
		ptOuter2[1].y = prcSheetTab->top + height - 6;
		ptOuter2[2].x = x2 + 1;
		ptOuter2[2].y = prcSheetTab->top + height - 3;
		ptOuter2[3].x = x2 + 3;
		ptOuter2[3].y = prcSheetTab->top + height - 1;
		ptOuter2[4].x = x2 + 6;
		ptOuter2[4].y = prcSheetTab->top + height;
		ptOuter2[5].x = x2+ width2 - 7;
		ptOuter2[5].y = prcSheetTab->top + height;
		ptOuter2[6].x = x2+ width2 - 4;
		ptOuter2[6].y = prcSheetTab->top + height - 1;
		ptOuter2[7].x = x2 + width2 - 2;
		ptOuter2[7].y = prcSheetTab->top + height - 3;
		ptOuter2[8].x = x2 + width2 - 1;
		ptOuter2[8].y = prcSheetTab->top + height - 6;
		ptOuter2[9].x = x2 + width2 - 1;
		ptOuter2[9].y = prcSheetTab->top ;
		ptOuter2[10].x = x2;
		ptOuter2[10].y = prcSheetTab->top;


		
		ptInner[0].x = x2 + 1;
		ptInner[0].y = prcSheetTab->top + 1;
		ptInner[1].x = x2+ width2 - 2;
		ptInner[1].y = prcSheetTab->top + 1;
		ptInner[2].x = x2+ width2 - 2;
		ptInner[2].y = prcSheetTab->top + height - 7;
		ptInner[3].x = x2+ width2 - 3;
		ptInner[3].y = prcSheetTab->top + height - 4;
		ptInner[4].x = x2+ width2 - 5;
		ptInner[4].y = prcSheetTab->top + height - 2;
		ptInner[5].x = x2+ width2 - 8;
		ptInner[5].y = prcSheetTab->top + height - 1;
		ptInner[6].x = x2+7;
		ptInner[6].y = prcSheetTab->top + height - 1;
		ptInner[7].x = x2+4;
		ptInner[7].y = prcSheetTab->top + height - 2;
		ptInner[8].x = x2+2;
		ptInner[8].y = prcSheetTab->top + height - 4;
		ptInner[9].x = x2+1;
		ptInner[9].y = prcSheetTab->top + height - 7 ;
		ptInner[10].x = x2 + 1;
		ptInner[10].y = prcSheetTab->top+1;
	   }
	   else if (nStyle == SS_TABENHANCEDSHAPE_TRAPEZOID)
	   {   
        int slope = height;
		offset = 10;
		width2 = 0;
		pts = 6;

		ptOuter[0].x = prcSheetTab->left;
		ptOuter[0].y = prcSheetTab->top;
		ptOuter[1].x = prcSheetTab->left + width - 1;
		ptOuter[1].y = prcSheetTab->top ;
		ptOuter[2].x = prcSheetTab->left + width - slope;
		ptOuter[2].y = prcSheetTab->top + height - 1;
		ptOuter[3].x = prcSheetTab->left + 3;
		ptOuter[3].y = prcSheetTab->top + height - 1;
		ptOuter[4].x = prcSheetTab->left;
		ptOuter[4].y = prcSheetTab->top + height - 4;
		ptOuter[5].x = prcSheetTab->left;
		ptOuter[5].y = prcSheetTab->top;
		ptOuter2[0].x = prcSheetTab->left;
		ptOuter2[0].y = prcSheetTab->top;
		ptOuter2[1].x = prcSheetTab->left + width;
		ptOuter2[1].y = prcSheetTab->top ;
		ptOuter2[2].x = prcSheetTab->left + width - slope + 1;
		ptOuter2[2].y = prcSheetTab->top + height - 1;
		ptOuter2[3].x = prcSheetTab->left + 3;
		ptOuter2[3].y = prcSheetTab->top + height - 1;
		ptOuter2[4].x = prcSheetTab->left;
		ptOuter2[4].y = prcSheetTab->top + height - 4;
		ptOuter2[5].x = prcSheetTab->left;
		ptOuter2[5].y = prcSheetTab->top;
		ptInner[0].x = prcSheetTab->left + 1;
		ptInner[0].y = prcSheetTab->top + 1;
		ptInner[1].x = prcSheetTab->left + width - 3;
		ptInner[1].y = prcSheetTab->top + 1 ;
		ptInner[2].x = prcSheetTab->left + width - slope;
		ptInner[2].y = prcSheetTab->top + height - 2;
		ptInner[3].x = prcSheetTab->left + 3;
		ptInner[3].y = prcSheetTab->top + height - 2;        
		ptInner[4].x = prcSheetTab->left + 1;
		ptInner[4].y = prcSheetTab->top + height - 4;        
		ptInner[5].x = prcSheetTab->left + 1;
		ptInner[5].y = prcSheetTab->top  + 1;   
	   }
	   else if (nStyle == SS_TABENHANCEDSHAPE_ISOSCELESTRAPEZOID)
	   {
		int height = prcSheetTab->bottom - prcSheetTab->top;
		int width = prcSheetTab->right - prcSheetTab->left;
		int slope = height / 2;
		pts = 5;
        offset = 0;
		width2 = 0;

		ptOuter[0].x = prcSheetTab->left;
		ptOuter[0].y = prcSheetTab->top;
		ptOuter[1].x = prcSheetTab->left + width - 1;
		ptOuter[1].y = prcSheetTab->top ;
		ptOuter[2].x = prcSheetTab->left + width - slope - 1;
		ptOuter[2].y = prcSheetTab->top + height - 1;
		ptOuter[3].x = prcSheetTab->left + slope;
		ptOuter[3].y = prcSheetTab->top + height - 1;
		ptOuter[4].x = prcSheetTab->left;
		ptOuter[4].y = prcSheetTab->top;
		ptOuter2[0].x = prcSheetTab->left;
		ptOuter2[0].y = prcSheetTab->top;
		ptOuter2[1].x = prcSheetTab->left + width - 1;
		ptOuter2[1].y = prcSheetTab->top;
		ptOuter2[2].x = prcSheetTab->left + width - slope - 1;
		ptOuter2[2].y = prcSheetTab->top + height;
		ptOuter2[3].x = prcSheetTab->left + slope;
		ptOuter2[3].y = prcSheetTab->top + height;
		ptOuter2[4].x = prcSheetTab->left;
		ptOuter2[4].y = prcSheetTab->top;
		ptInner[0].x = prcSheetTab->left + 2;
		ptInner[0].y = prcSheetTab->top + 1;
		ptInner[1].x = prcSheetTab->left + width - 2;
		ptInner[1].y = prcSheetTab->top + 1 ;
		ptInner[2].x = prcSheetTab->left + width - slope - 2;
		ptInner[2].y = prcSheetTab->top + height - 2;
		ptInner[3].x = prcSheetTab->left + slope + 1;
		ptInner[3].y = prcSheetTab->top + height - 2;        
		ptInner[4].x = prcSheetTab->left + 2;
		ptInner[4].y = prcSheetTab->top  + 1;   
	   }
	BeginPath(hdc);
	Polygon(hdc, ptOuter2, pts);
	EndPath(hdc);
	GetClipRgn(hdc, rgn);
	SelectClipPath(hdc, RGN_AND);
	rcUpper.left = prcSheetTab->left;
	rcUpper.top = prcSheetTab->top;
	rcUpper.right = rcUpper.left + width;
	rcUpper.bottom = rcUpper.top + (height/2);
	rcLower.left = prcSheetTab->left;
	rcLower.top = prcSheetTab->top + (height/2);
	rcLower.right = rcUpper.left + width;
	rcLower.bottom = prcSheetTab->bottom;
    fpGradientFill(hdc, rcUpper, upperBrush1, upperBrush2, GRADIENT_FILL_RECT_V);
    fpGradientFill(hdc, rcLower, lowerBrush1, lowerBrush2, GRADIENT_FILL_RECT_V);
 	SelectClipRgn(hdc, rgn);
	pen = CreatePen(PS_SOLID, 0, sheetTabOuterBorderColor);
	penOld = SelectObject(hdc, pen);
    Polyline(hdc, ptOuter, pts);
	DeleteObject(pen);
	pen = CreatePen(PS_SOLID, 0, sheetTabInnerBorderColor);
	SelectObject(hdc, pen);
	Polyline(hdc, ptInner, pts);

    if (bActive)
		SS_DrawLine(hdc, prcSheetTab->left, prcSheetTab->top, prcSheetTab->right - (width2 != 0 ? offset : 1), prcSheetTab->top);
	SelectObject(hdc, penOld);
   
	if( pszSheetName )
		{
			rcText.left = prcSheetTab->left;
			rcText.top = prcSheetTab->top;
			rcText.right = prcSheetTab->right - offset;
			rcText.bottom = prcSheetTab->bottom;
			crOld = SetTextColor(hdc, crText);
			DrawText(hdc, pszSheetName, -1, &rcText, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
		    SetTextColor(hdc, crOld);
		}

	 DeleteObject(pen);
     DeleteObject(rgn);
     DeleteObject(upperBrush1);
     DeleteObject(upperBrush2);
     DeleteObject(lowerBrush1);
     DeleteObject(lowerBrush2);

}
#endif

#ifdef SS_V80
void SSTabPtr_DrawSheetTabClassic(SS_TAB *pThis, HDC hdc, LPTSTR pszSheetName, RECT *prcSheetTab, BOOL bActive, BOOL bBottom)
#else
void SSTabPtr_DrawSheetTab(SS_TAB *pThis, HDC hdc, LPTSTR pszSheetName, RECT *prcSheetTab, BOOL bActive, BOOL bBottom)
#endif
{
  COLORREF crText = GetSysColor(bActive ? COLOR_WINDOWTEXT : COLOR_BTNTEXT);
  COLORREF crFace = GetSysColor(bActive ? COLOR_WINDOW : COLOR_3DFACE);
  COLORREF crHighlight = GetSysColor(COLOR_3DHIGHLIGHT);
  COLORREF crShadow = GetSysColor(COLOR_3DSHADOW);
  COLORREF crOld;
  HPEN hpenFace = CreatePen(PS_SOLID, 0, crFace);
  HPEN hpenHighlight = CreatePen(PS_SOLID, 0, crHighlight);
  HPEN hpenShadow = CreatePen(PS_SOLID, 0, crShadow);
  HPEN hpenOld;
  HBRUSH hbrFace = CreateSolidBrush(crFace);
  HBRUSH hbrOld;
  POINT pt[4];
  RECT rcText;

  if( bBottom )
  {
    pt[0].x = prcSheetTab->left;
    pt[0].y = prcSheetTab->top;
    pt[1].x = prcSheetTab->left + SS_TABOVERLAP;
    pt[1].y = prcSheetTab->bottom -1 ;
    pt[2].x = prcSheetTab->right - SS_TABOVERLAP - 1;
    pt[2].y = prcSheetTab->bottom - 1;
    pt[3].x = prcSheetTab->right - 1;
    pt[3].y = prcSheetTab->top;
  }
  else
  {
    pt[0].x = prcSheetTab->left;
    pt[0].y = prcSheetTab->bottom - 1;
    pt[1].x = prcSheetTab->left + SS_TABOVERLAP;
    pt[1].y = prcSheetTab->top;
    pt[2].x = prcSheetTab->right - SS_TABOVERLAP - 1;
    pt[2].y = prcSheetTab->top;
    pt[3].x = prcSheetTab->right - 1;
    pt[3].y = prcSheetTab->bottom - 1;
  }
  crOld = SetTextColor(hdc, crText);
  hbrOld = SelectObject(hdc, hbrFace);
  hpenOld = SelectObject(hdc, bBottom ? hpenShadow : hpenHighlight);
  Polygon(hdc, pt, 4);
  if( pszSheetName )
  {
    rcText.left = prcSheetTab->left + SS_TABOVERLAP + 1;
    rcText.top = prcSheetTab->top + 1;
    rcText.right = prcSheetTab->right - SS_TABOVERLAP - 1;
    rcText.bottom = prcSheetTab->bottom - 1;
    DrawText(hdc, pszSheetName, -1, &rcText, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
  }
  SelectObject(hdc, hpenHighlight);
  SS_DrawLine(hdc, pt[1].x, pt[1].y, pt[0].x, pt[0].y);
  SelectObject(hdc, bBottom ? hpenShadow : hpenHighlight);
  SS_DrawLine(hdc, pt[1].x, pt[1].y, pt[2].x, pt[2].y);
  SelectObject(hdc, hpenShadow);
  SS_DrawLine(hdc, pt[2].x, pt[2].y, pt[3].x, pt[3].y);
  if( bActive )
  {
    SelectObject(hdc, hpenFace);
    SS_DrawLine(hdc, pt[0].x+1, pt[0].y, pt[3].x, pt[3].y);
  }
  else
  {
    SelectObject(hdc, bBottom ? hpenShadow : hpenHighlight);
    SS_DrawLine(hdc, pt[0].x+1, pt[0].y, pt[3].x+1, pt[3].y);
  }
  SetTextColor(hdc, crOld);
  SelectObject(hdc, hpenOld);
  SelectObject(hdc, hbrOld);
  DeleteObject(hpenFace);
  DeleteObject(hpenHighlight);
  DeleteObject(hpenShadow);
  DeleteObject(hbrFace);
}

#ifdef SS_V80
 void SSTabPtr_DrawSheetTab(SS_TAB *pThis, HDC hdc, LPTSTR pszSheetName, RECT *prcSheetTab, BOOL bActive, BOOL bBottom, BOOL bMouseOver)
 {
    LPSS_BOOK lpBook = SS_BookLock(pThis->hWndSS);
	 if (lpBook->wAppearanceStyle == 2)
	 {
		 SSTabPtr_DrawSheetTabEnhanced(pThis, hdc, lpBook->wTabEnhancedShape, pszSheetName, prcSheetTab, bActive, bBottom, bMouseOver, lpBook->sheetTabUpperNormalStartColor,
			 lpBook->sheetTabUpperNormalEndColor, lpBook->sheetTabLowerNormalStartColor, lpBook->sheetTabLowerNormalEndColor, lpBook->sheetTabUpperHoverStartColor,
			 lpBook->sheetTabUpperHoverEndColor, lpBook->sheetTabLowerHoverStartColor, lpBook->sheetTabLowerHoverEndColor, lpBook->sheetTabOuterBorderColor,
			 lpBook->sheetTabInnerBorderColor, lpBook->sheetTabForeColor, lpBook->sheetTabActiveForeColor);
	 }
	 else if (lpBook->wAppearanceStyle == 1 && lpBook->fThemesActive)
		 SSTabPtr_DrawSheetTabVisualStyles(pThis, hdc, pszSheetName, prcSheetTab, bActive, bBottom, bMouseOver);
	 else
		 SSTabPtr_DrawSheetTabClassic(pThis, hdc, pszSheetName, prcSheetTab, bActive, bBottom);
	 SS_BookUnlock(pThis->hWndSS);
 }
#endif
 //--------------------------------------------------------------------

void SSTabPtr_DrawGrayArea(SS_TAB *pThis, HDC hdc, RECT *prc, BOOL bBottom)
{
#ifdef SS_V80
  LPSS_BOOK lpBook = SS_BookLock(pThis->hWndSS);
  if (lpBook->wAppearanceStyle == 2)
  {
	  HBRUSH hBrush1 = CreateSolidBrush(lpBook->sheetGrayAreaStartColor);
	  HBRUSH hBrush2 = CreateSolidBrush(lpBook->sheetGrayAreaEndColor);
	  RECT rc =  {prc->left, prc->top, prc->right, prc->top + (2 * ((prc->bottom-prc->top) / 5))};
	  RECT rc2 = {prc->left, rc.bottom, prc->right, prc->bottom};
      fpGradientFill(hdc, rc, hBrush1, hBrush2, GRADIENT_FILL_RECT_V);
  
      FillRect(hdc, &rc2, hBrush2);

	  DeleteObject(hBrush1);
	  DeleteObject(hBrush2);

  }
  else
  {
#endif

  COLORREF crFace = GetSysColor(COLOR_3DFACE);
  COLORREF crShadow = GetSysColor(bBottom ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT);
  HPEN hpenShadow = CreatePen(PS_SOLID, 0, crShadow);
  HPEN hpenOld;
  HBRUSH hbrFace = CreateSolidBrush(crFace);
  RECT rc = *prc;

  hpenOld = SelectObject(hdc, hpenShadow);
  if( bBottom )
  {
    SS_DrawLine(hdc, rc.left, rc.top, rc.right, rc.top);
    rc.top++;
  }
  else
  {
    SS_DrawLine(hdc, rc.left, rc.bottom-1, rc.right, rc.bottom-1);
    rc.bottom--;
  }
  FillRect(hdc, &rc, hbrFace);
  SelectObject(hdc, hpenOld);
  DeleteObject(hpenShadow);
  DeleteObject(hbrFace);
#ifdef SS_V80
  }
  SS_BookUnlock(pThis->hWndSS);
#endif
}

//--------------------------------------------------------------------
//
//  The following function draws the tab object.
//

BOOL SSTab_Draw(HSS_TAB hThis, HDC hdc, RECT *prc, BOOL bBottom)
{
  SS_TAB *pThis;
  short lActiveSheet;
  short nSheet;
  short lLeftExtra;
  short nLeftSheet;
  short lRightSheet;
  TCHAR sheetName[SS_MAXSHEETNAME + 1];
  RECT rcBtn1;
  RECT rcBtn2;
  RECT rcBtn3;
  RECT rcBtn4;
  RECT rcTabs;
  RECT rcSheetTab;
  HFONT hFontTab;
  HFONT hFontOld;
  BOOL bDrawBtns;

  BOOL bRet = FALSE;

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    LPSS_BOOK lpBook = SS_BookLock(pThis->hWndSS);
    SSTabPtr_DrawGrayArea(pThis, hdc, prc, bBottom);

    bDrawBtns = SSTabPtr_CalcRects(lpBook, prc, &rcBtn1, &rcBtn2, &rcBtn3, &rcBtn4, &rcTabs);
	if (bDrawBtns)
	{
#ifdef SS_V80
    if (lpBook->wAppearanceStyle == 2)
	{
    SS_DrawScrollButtonEnhanced(hdc, lpBook, &rcBtn1, 1, SPRD_ZOOM_INITVAL, 1 == pThis->nBtnOver, lpBook->mouseTabBtn == 1);
    SS_DrawScrollButtonEnhanced(hdc, lpBook, &rcBtn2, 2, SPRD_ZOOM_INITVAL, 2 == pThis->nBtnOver, lpBook->mouseTabBtn == 2);
    SS_DrawScrollButtonEnhanced(hdc, lpBook, &rcBtn3, 3, SPRD_ZOOM_INITVAL, 3 == pThis->nBtnOver, lpBook->mouseTabBtn == 3);
    SS_DrawScrollButtonEnhanced(hdc, lpBook, &rcBtn4, 4, SPRD_ZOOM_INITVAL, 4 == pThis->nBtnOver, lpBook->mouseTabBtn == 4);
	}
	else if (lpBook->wAppearanceStyle == 1 && lpBook->fThemesActive)
	{
    SS_DrawScrollHomeLeftVisualStyles(hdc, &rcBtn1, SPRD_ZOOM_INITVAL, 1 == pThis->nBtnOver, lpBook->mouseTabBtn == 1);
    SS_DrawScrollLineLeftVisualStyles(hdc, &rcBtn2, SPRD_ZOOM_INITVAL, 2 == pThis->nBtnOver, lpBook->mouseTabBtn == 2);
    SS_DrawScrollLineRightVisualStyles(hdc, &rcBtn3, SPRD_ZOOM_INITVAL, 3 == pThis->nBtnOver, lpBook->mouseTabBtn == 3);
    SS_DrawScrollEndRightVisualStyles(hdc, &rcBtn4, SPRD_ZOOM_INITVAL, 4 == pThis->nBtnOver, lpBook->mouseTabBtn == 4);
	}
	else
	{
    SS_DrawScrollHomeLeft(hdc, &rcBtn1, SPRD_ZOOM_INITVAL, 1 == pThis->nBtnOver);
    SS_DrawScrollLineLeft(hdc, &rcBtn2, SPRD_ZOOM_INITVAL, 2 == pThis->nBtnOver);
    SS_DrawScrollLineRight(hdc, &rcBtn3, SPRD_ZOOM_INITVAL, 3 == pThis->nBtnOver);
    SS_DrawScrollEndRight(hdc, &rcBtn4, SPRD_ZOOM_INITVAL, 4 == pThis->nBtnOver);
	}
#else
    SS_DrawScrollHomeLeft(hdc, &rcBtn1, SPRD_ZOOM_INITVAL, 1 == pThis->nBtnOver);
    SS_DrawScrollLineLeft(hdc, &rcBtn2, SPRD_ZOOM_INITVAL, 2 == pThis->nBtnOver);
    SS_DrawScrollLineRight(hdc, &rcBtn3, SPRD_ZOOM_INITVAL, 3 == pThis->nBtnOver);
    SS_DrawScrollEndRight(hdc, &rcBtn4, SPRD_ZOOM_INITVAL, 4 == pThis->nBtnOver);
#endif
	}
    SaveDC(hdc);
    IntersectClipRect(hdc, rcTabs.left, rcTabs.top, rcTabs.right, rcTabs.bottom);
    //SSTabPtr_DrawGrayArea(pThis, hdc, &rcTabs, bBottom);
    lActiveSheet = lpBook->nActiveSheet;
    nLeftSheet = SS_GetTabStripLeftSheet(lpBook);
    hFontTab = SS_GetTabStripFont(lpBook);
    hFontOld = SelectObject(hdc, hFontTab);
    SetBkMode(hdc, TRANSPARENT);

    lLeftExtra = SSTab_GetPrevVisibleTab(lpBook, nLeftSheet);
    lLeftExtra = max(lLeftExtra, 0);
    lRightSheet = SSTab_GetRightSheet(lpBook, hdc, &rcTabs);

    for( nSheet = lRightSheet; nSheet >= lLeftExtra; nSheet-- )
    {
      if( nSheet != lActiveSheet &&
          SS_GetSheetVisible(lpBook, nSheet) )
      {
        SSTab_GetTabRect(lpBook, hdc, nSheet, &rcTabs, &rcSheetTab);
        SSTab_GetSheetDisplayName(lpBook, nSheet, sheetName, SS_MAXSHEETNAME + 1);
#ifdef SS_V80
        SSTabPtr_DrawSheetTab(pThis, hdc, sheetName, &rcSheetTab, FALSE, bBottom, lpBook->mouseTabSheet == nSheet);
#else
        SSTabPtr_DrawSheetTab(pThis, hdc, sheetName, &rcSheetTab, FALSE, bBottom);
#endif
      }
    }
    if( lLeftExtra <= lActiveSheet && lActiveSheet <= lRightSheet &&
        SS_GetSheetVisible(lpBook, lActiveSheet) )
    {
      SSTab_GetTabRect(lpBook, hdc, lActiveSheet, &rcTabs, &rcSheetTab);
      SSTab_GetSheetDisplayName(lpBook, lActiveSheet, sheetName, SS_MAXSHEETNAME + 1);
#ifdef SS_V80
      SSTabPtr_DrawSheetTab(pThis, hdc, sheetName, &rcSheetTab, TRUE, bBottom, lpBook->mouseTabSheet == lActiveSheet);
#else
      SSTabPtr_DrawSheetTab(pThis, hdc, sheetName, &rcSheetTab, TRUE, bBottom);
#endif
    }
    SelectObject(hdc, hFontOld);
    RestoreDC(hdc, -1);
    bRet = TRUE;

    SS_BookUnlock(pThis->hWndSS);
    SSTab_MemUnlock(hThis);
  }
  return bRet;
}

//--------------------------------------------------------------------

LRESULT SSTab_OnLButtonDown(HSS_TAB hThis, WPARAM wParam, LPARAM lParam, RECT *prc)
{
  SS_TAB *pThis;
  short nNewSheet;
  int nBtn = 0;
  int x = LOWORD(lParam);
  int y = HIWORD(lParam);

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    LPSS_BOOK lpBook = SS_BookLock(pThis->hWndSS);

    SSTab_HitTest(hThis, x, y, prc, &nNewSheet, &nBtn, NULL);

    if( nBtn )
    {
      pThis->nBtnDown = nBtn;
      pThis->nBtnOver = nBtn;
      SS_InvalidateTabStrip(lpBook);
      UpdateWindow(lpBook->hWnd);

      if ( nBtn == 2 || nBtn == 3 ) // Prev or Next
      {
        long  lMouseButton;
        short nNewLeftSheet;
        short nOldLeftSheet = SS_GetTabStripLeftSheet(lpBook);
        short nSheets = SS_GetSheetCount(lpBook);
        DWORD dwTime, dwDelay = 0;
        BOOL  fFirstTime;

        lMouseButton = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
        dwTime = GetTickCount();

        SetCapture(lpBook->hWnd);

        for (fFirstTime = TRUE;;)
        {
          if (dwTime + dwDelay <= GetTickCount() || fFirstTime)
          {
            if( nBtn == 2 )
              nNewLeftSheet = SSTab_GetPrevVisibleTab(lpBook, nOldLeftSheet);
            else // nBtn == 3
              nNewLeftSheet = SSTab_GetNextVisibleTab(lpBook, nOldLeftSheet);

            if( 0 <= nNewLeftSheet && nNewLeftSheet < nSheets && nNewLeftSheet != nOldLeftSheet )
            {
              SS_FireTabScrolling(lpBook, nOldLeftSheet, nNewLeftSheet);
              SS_SetTabStripLeftSheet(lpBook, nNewLeftSheet);
              SS_FireTabScrolled(lpBook, nOldLeftSheet, nNewLeftSheet);
              UpdateWindow(lpBook->hWnd);
              nOldLeftSheet = nNewLeftSheet;
            }

            dwTime = GetTickCount();
            dwDelay = fFirstTime ? TABBTN_INITIALDELAY : TABBTN_TIMEDELAY;
            fFirstTime = FALSE;
          }

          if ((GetAsyncKeyState(lMouseButton) & 0x8000) == 0)
            break;
        }

        ReleaseCapture();

        pThis->nBtnDown = 0;
        pThis->nBtnOver = 0;
        SS_InvalidateTabStrip(lpBook);
        UpdateWindow(lpBook->hWnd);
      }

    // RFW - 7/7/04 - 14833
    else if ( nBtn == 1 || nBtn == 4 ) // First or Last
        SetCapture(lpBook->hWnd);
    }

    else if( nNewSheet != -1 )
    {
      short nOldSheet = SS_GetActiveSheet(lpBook);
      if( nOldSheet != nNewSheet )
        SS_SetActiveSheet(lpBook, nNewSheet);
    }

    SS_BookUnlock(pThis->hWndSS);
    SSTab_MemUnlock(hThis);
  }
  return 0;
}

//--------------------------------------------------------------------

LRESULT SSTab_OnLButtonUp(HSS_TAB hThis, WPARAM wParam, LPARAM lParam, RECT *prc)
{
  SS_TAB *pThis;
  short nSheets;
  short nOldLeftSheet;
  short nNewLeftSheet = -1;
  int nBtn = 0;
  int x = LOWORD(lParam);
  int y = HIWORD(lParam);

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    LPSS_BOOK lpBook = SS_BookLock(pThis->hWndSS);

    nSheets = SS_GetSheetCount(lpBook);
    nOldLeftSheet = SS_GetTabStripLeftSheet(lpBook);
    if( pThis->nBtnDown )
    {
		// RFW - 7/7/04 - 14833
		ReleaseCapture();

      SSTab_HitTest(hThis, x, y, prc, NULL, &nBtn, NULL);
      if( nBtn == 1 )
        nNewLeftSheet = SSTab_GetFirstVisibleTab(lpBook);
      /*
      else if( nBtn == 2 )
        nNewLeftSheet = SSTab_GetPrevVisibleTab(lpBook, nOldLeftSheet);
      else if( nBtn == 3 )
        nNewLeftSheet = SSTab_GetNextVisibleTab(lpBook, nOldLeftSheet);
      */
      else if( nBtn == 4 )
        nNewLeftSheet = SSTab_GetLastVisibleTab(lpBook);

      if( nBtn == pThis->nBtnDown && 0 <= nNewLeftSheet && nNewLeftSheet < nSheets && nNewLeftSheet != nOldLeftSheet )
      {
        SS_FireTabScrolling(lpBook, nOldLeftSheet, nNewLeftSheet);
        SS_SetTabStripLeftSheet(lpBook, nNewLeftSheet);
        SS_FireTabScrolled(lpBook, nOldLeftSheet, nNewLeftSheet);
      }
      pThis->nBtnDown = 0;
      pThis->nBtnOver = 0;
      SS_InvalidateTabStrip(lpBook);
    }

    SS_BookUnlock(pThis->hWndSS);
    SSTab_MemUnlock(hThis);
  }
  return 0;
}

//--------------------------------------------------------------------

LRESULT SSTab_OnMouseMove(HSS_TAB hThis, WPARAM wParam, LPARAM lParam, RECT *prc)
{
  SS_TAB *pThis;
  int x = LOWORD(lParam);
  int y = HIWORD(lParam);
  int nBtn = 0;

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    LPSS_BOOK lpBook = SS_BookLock(pThis->hWndSS);

    SSTab_HitTest(hThis, x, y, prc, NULL, &nBtn, NULL);
    if( nBtn == pThis->nBtnDown )
    {
      if( pThis->nBtnOver != nBtn )
      {
        pThis->nBtnOver = nBtn;
        SS_InvalidateTabStrip(lpBook);
      }
    }
    else
    {
      if( pThis->nBtnOver != 0 )
      {
        pThis->nBtnOver = 0;
        SS_InvalidateTabStrip(lpBook);
      }
    }

    SS_BookUnlock(pThis->hWndSS);
    SSTab_MemUnlock(hThis);
  }
  return 0;
}


BOOL SSTab_HitTest(HSS_TAB hThis, int x, int y, RECT *prc, short *pnSheet, int *pnBtn, RECT *prcHit)
{
  SS_TAB *pThis;
  RECT rcBtn1;
  RECT rcBtn2;
  RECT rcBtn3;
  RECT rcBtn4;
  RECT rcTabs;
  RECT rcTemp;
  int  nBtn = 0;
  BOOL bRet = FALSE;

  if (!prcHit)
    prcHit = &rcTemp;

  *pnBtn = 0;
  if (pnSheet)
    *pnSheet = -1;

  if( hThis && (pThis = SSTab_MemLock(hThis)) )
  {
    LPSS_BOOK lpBook = SS_BookLock(pThis->hWndSS);

    SSTabPtr_CalcRects(lpBook, prc, &rcBtn1, &rcBtn2, &rcBtn3, &rcBtn4, &rcTabs);
    if( rcBtn1.left <= x && x < rcBtn1.right && rcBtn1.top <= y && y < rcBtn1.bottom )
    {
      nBtn = 1;
      *prcHit = rcBtn1;
    }
    else if( rcBtn2.left <= x && x < rcBtn2.right && rcBtn2.top <= y && y < rcBtn2.bottom )
    {
      nBtn = 2;
      *prcHit = rcBtn2;
    }
    else if( rcBtn3.left <= x && x < rcBtn3.right && rcBtn3.top <= y && y < rcBtn3.bottom )
    {
      nBtn = 3;
      *prcHit = rcBtn3;
    }
    else if( rcBtn4.left <= x && x < rcBtn4.right && rcBtn4.top <= y && y < rcBtn4.bottom )
    {
      nBtn = 4;
      *prcHit = rcBtn4;
    }
    if( nBtn )
    {
      *pnBtn = nBtn;
    }
    else if( rcTabs.left <= x && x < rcTabs.right && rcTabs.top <= y && y < rcTabs.bottom )
    {
      POINT ptMouse = {x, y};
      if (!SSTab_GetTabRectFromMouse(lpBook, 0, &ptMouse, &rcTabs, prcHit, pnSheet))
			{
			if (pnSheet)
				*pnSheet = -1;
			}
      IntersectRect(prcHit, prcHit, &rcTabs);
    }

    SS_BookUnlock(pThis->hWndSS);
    SSTab_MemUnlock(hThis);
  
    bRet = TRUE;
  }

  return bRet;
}

//--------------------------------------------------------------------

void SS_Draw3DSurfaceEx(HDC hdc, RECT *prc, COLORREF cr3DFace, COLORREF cr3DHighlight, COLORREF cr3DShadow, int nWidth, BOOL bSunken, BOOL bAdjust)
{
  HBRUSH hbr3DFace;
  HPEN hpen3DHighlight;
  HPEN hpen3DShadow;
  HPEN hpenOld;
  RECT rc = *prc;

  hbr3DFace = CreateSolidBrush(cr3DFace);
  hpen3DHighlight = CreatePen(PS_SOLID, 0, bSunken ? cr3DShadow : cr3DHighlight);
  hpen3DShadow = CreatePen(PS_SOLID, 0, bSunken ? cr3DHighlight : cr3DShadow);
  if( rc.left < rc.right && rc.top < rc.bottom )
  {
    for( ; nWidth > 0 && rc.left < rc.right && rc.top < rc.bottom ; nWidth--)
    {
      hpenOld = SelectObject(hdc, hpen3DHighlight);
      SS_DrawLine(hdc, rc.left, rc.bottom-1, rc.left, rc.top);
      SS_DrawLine(hdc, rc.left, rc.top, rc.right-1, rc.top);
      SelectObject(hdc, hpen3DShadow);
      SS_DrawLine(hdc, rc.right-1, rc.top, rc.right-1, rc.bottom);
      SS_DrawLine(hdc, rc.right-1, rc.bottom-1, rc.left-1, rc.bottom-1);
      SelectObject(hdc, hpenOld);
      InflateRect(&rc, -1, -1);
    }
    if( rc.left < rc.right && rc.top < rc.bottom )
    {
      FillRect(hdc, &rc, hbr3DFace);
    }
  }
  DeleteObject(hbr3DFace);
  DeleteObject(hpen3DHighlight);
  DeleteObject(hpen3DShadow);
  if( bAdjust )
    *prc = rc;
}

//--------------------------------------------------------------------

void SS_Draw3DSurface(HDC hdc, RECT *prc, BOOL bSunken, BOOL bAdjust)
{
  COLORREF cr3DFace = GetSysColor(COLOR_3DFACE);
  COLORREF cr3DHighlight = GetSysColor(COLOR_3DHILIGHT);
  COLORREF cr3DShadow = GetSysColor(COLOR_3DSHADOW);
  SS_Draw3DSurfaceEx(hdc, prc, cr3DFace, cr3DHighlight, cr3DShadow, 1, bSunken, bAdjust);
}

#ifdef SS_V80
void SS_DrawSplitBoxEnhanced(HDC hdc, RECT *prc, LPSS_BOOK lpBook)
{
	int yTemp;
	int heightTemp;
	int len;
	int startY;
	int x;
	int width;
	HPEN penOld;

	RECT rc =  {prc->left, prc->top, prc->right, prc->bottom};
	RECT rcInner = {rc.left+1, rc.top+1, rc.right-1, rc.bottom-1};

	RECT rcUpper = {rc.left, rc.top, rc.right, rc.top + (rc.bottom-rc.top) / 2};
	RECT rcLower = {rc.left, rcUpper.bottom, rc.right, rc.bottom};
  
	HBRUSH upperBrush1 = CreateSolidBrush(lpBook->sheetTabUpperNormalStartColor);
	HBRUSH upperBrush2 = CreateSolidBrush(lpBook->sheetTabUpperNormalEndColor);
	HBRUSH lowerBrush1 = CreateSolidBrush(lpBook->sheetTabLowerNormalStartColor);
	HBRUSH lowerBrush2 = CreateSolidBrush(lpBook->sheetTabLowerNormalEndColor);

	HBRUSH outerBrush = CreateSolidBrush(lpBook->sheetTabOuterBorderColor);
	HPEN   dashPen = CreatePen(PS_SOLID, 1, lpBook->sheetTabOuterBorderColor);
       
	fpGradientFill(hdc, rcUpper, upperBrush1, upperBrush2, GRADIENT_FILL_RECT_V);
    fpGradientFill(hdc, rcLower, lowerBrush1, lowerBrush2, GRADIENT_FILL_RECT_V);
    FrameRect(hdc, &rc, outerBrush );
    
	yTemp = rc.top + 1;
    heightTemp = (rc.bottom-rc.top) - 2;
	x = rc.left;
	width = rc.right-rc.left;
    len = heightTemp / 2;
    if ((len % 2) != 0)
        len--;
    startY = yTemp + ((heightTemp - len) / 2);
	penOld = SelectObject(hdc, dashPen);
    SS_DrawLine(hdc, x + (width / 2), startY, x + (width / 2), startY + len);
	SelectObject(hdc, penOld);
	DeleteObject(dashPen);
	DeleteObject(upperBrush1);
	DeleteObject(upperBrush2);
	DeleteObject(lowerBrush1);
	DeleteObject(lowerBrush2);

}

void SS_DrawSplitBoxVisualStyles(HDC hdc, RECT *prc)
{
	HTHEME hTheme = SS_OpenThemeData(WindowFromDC(hdc), L"ScrollBar");
	BOOL isHorz = ((prc->right-prc->left) > (prc->bottom-prc->top));
	SS_DrawThemeBackground( hTheme, hdc, isHorz ? 2 : 3, 1, prc, prc);
   SS_CloseThemeData(hTheme);
}
#endif
#ifdef SS_V80
void SS_DrawSplitBox(HDC hdc, RECT *prc, LPSS_BOOK lpBook)
#else
void SS_DrawSplitBox(HDC hdc, RECT *prc)
#endif
{
#ifdef SS_V80
  if (lpBook->wAppearanceStyle == 2)
  {
	  SS_DrawSplitBoxEnhanced(hdc, prc, lpBook);
  }
  else if (lpBook->wAppearanceStyle == 1 && lpBook->fThemesActive)
  {
	  SS_DrawSplitBoxVisualStyles(hdc, prc);
  }
  else
  {
#endif
	  SS_Draw3DSurface(hdc, prc, FALSE, FALSE);
#ifdef SS_V80
  }
#endif
}

//--------------------------------------------------------------------
#ifdef SS_V80
void SS_DrawScrollHomeLeftVisualStyles(HDC hdc, RECT *prc, short nZoom, BOOL bPushed, BOOL bMouseOver)
{
  RECT rc = *prc;
// 99914280 -scl
//  POINT pt;
  POINT pt = {7, 10};
  COLORREF clr;
  HRESULT res = S_OK;
  HGLOBAL gh1 = 0;
  LPWSTR lpwstrFile = NULL;
  HGLOBAL gh2 = 0;
  LPWSTR lpwstrColor = NULL;
  HGLOBAL gh3 = 0;
  LPWSTR lpwstrSize = NULL;
  HPEN hPen;
  HPEN hPenOld;

  HTHEME hTheme = SS_OpenThemeData(WindowFromDC(hdc), L"ScrollBar");
    gh1 = GlobalAlloc(GHND,255);
    lpwstrFile = (LPWSTR)GlobalLock(gh1);
    gh2 = GlobalAlloc(GHND,255);
    lpwstrColor = (LPWSTR)GlobalLock(gh2);
    gh3 = GlobalAlloc(GHND,255);
    lpwstrSize = (LPWSTR)GlobalLock(gh3);
	SS_DrawThemeBackground( hTheme, hdc, 1, bPushed ? 11 : bMouseOver ? 10 : 9, &rc, &rc);
	res = SS_GetCurrentThemeName(lpwstrFile, 255, lpwstrColor, 255, lpwstrSize, 255);
	SS_GetThemePosition(hTheme, 1, bPushed ? 11 : 9, 3409, &pt );

	if (lstrcmpW((LPCWSTR)lpwstrColor, L"Metallic")== 0)
	  clr = RGB(0,0,0);
	else if (lstrcmpW((LPCWSTR)lpwstrColor, L"HomeStead") == 0)
	  clr = RGB(255,255,255);
	else if (lstrcmpW((LPCWSTR)lpwstrColor, L"NormalColor") == 0)
	  clr = RGB(77,97,133);
	else
	  clr = RGB(0,0,0);

	hPen = CreatePen(PS_SOLID, 1, clr);
	hPenOld = SelectObject(hdc, hPen);

    SS_DrawLine(hdc, rc.left + ((rc.right-rc.left-pt.x)/2)-3, rc.top + ((rc.bottom-rc.top-pt.y)/2)+1, rc.left + ((rc.right-rc.left-pt.x)/2)-3, rc.bottom - ((rc.bottom-rc.top-pt.y)/2)-1);

	SelectObject(hdc, hPenOld);
	DeleteObject(hPen);

    SS_CloseThemeData(hTheme);
    GlobalUnlock(gh1);
    GlobalFree(gh1);
    GlobalUnlock(gh2);
    GlobalFree(gh2);
    GlobalUnlock(gh3);
    GlobalFree(gh3);
}
void SS_DrawScrollButtonEnhanced(HDC hdc, LPSS_BOOK lpBook, RECT *prc, int button, short nZoom, BOOL bPushed, BOOL bMouseOver )
{
	    HPEN hPen;
		HPEN hPenOld;
        int arrowSize;
        int arrowX;
        int arrowY;
		int i;

	    BOOL enabled = TRUE;
		BOOL mouseOver = FALSE;
        RECT rc = *prc;		
		int x = rc.left;
		int y = rc.top;
		int width = rc.right-rc.left;
		int height = rc.bottom-rc.top;
		RECT topRC = {rc.left, rc.top, rc.right, rc.top + (height/2)};
		RECT bottomRC = {rc.left, rc.top + (height/2), rc.right, rc.bottom};

		HBRUSH disBrush = CreateSolidBrush(RGB(192,192,192));
		HBRUSH upperBrush1 = CreateSolidBrush(lpBook->sheetScrollUpperNormalStartColor);
		HBRUSH upperBrush2 = CreateSolidBrush(lpBook->sheetScrollUpperNormalEndColor);
		HBRUSH bottomBrush1 = CreateSolidBrush(lpBook->sheetScrollLowerNormalStartColor);
		HBRUSH bottomBrush2 = CreateSolidBrush(lpBook->sheetScrollLowerNormalEndColor);

        if (!enabled)
        {
          fpGradientFill(hdc, topRC, disBrush, disBrush, GRADIENT_FILL_RECT_V);
          fpGradientFill(hdc, bottomRC, disBrush, disBrush, GRADIENT_FILL_RECT_V);
        }
        else
        {
          fpGradientFill(hdc, topRC, upperBrush1, upperBrush2, GRADIENT_FILL_RECT_V);
          fpGradientFill(hdc, bottomRC, bottomBrush1, bottomBrush2, GRADIENT_FILL_RECT_V);
        }
		DeleteObject(disBrush);
		DeleteObject(upperBrush1);
		DeleteObject(upperBrush2);
		DeleteObject(bottomBrush1);
		DeleteObject(bottomBrush2);

      if (bMouseOver && enabled)
        {
		 upperBrush1 = CreateSolidBrush(lpBook->sheetScrollUpperHoverStartColor);
		 upperBrush2 = CreateSolidBrush(lpBook->sheetScrollUpperHoverEndColor);
		 bottomBrush1 = CreateSolidBrush(lpBook->sheetScrollLowerHoverStartColor);
		 bottomBrush2 = CreateSolidBrush(lpBook->sheetScrollLowerHoverEndColor);
        }
        if (bPushed && enabled)
        {
		 upperBrush1 = CreateSolidBrush(lpBook->sheetScrollUpperPushedStartColor);
		 upperBrush2 = CreateSolidBrush(lpBook->sheetScrollUpperPushedEndColor);
		 bottomBrush1 = CreateSolidBrush(lpBook->sheetScrollLowerPushedStartColor);
		 bottomBrush2 = CreateSolidBrush(lpBook->sheetScrollLowerPushedEndColor);
        }
        if (bMouseOver || bPushed)
        {
		  topRC.left = x + 2;
		  topRC.right -= 2;
		  bottomRC.left = x + 2;
		  bottomRC.right -= 2;
          fpGradientFill(hdc, topRC, upperBrush1, upperBrush2, GRADIENT_FILL_RECT_V);
          fpGradientFill(hdc, bottomRC, bottomBrush1, bottomBrush2, GRADIENT_FILL_RECT_V);
        }

		hPen = CreatePen(PS_SOLID, 1, RGB(255,255,255));
		hPenOld = SelectObject(hdc, hPen);

        SS_DrawLine(hdc, x, y + height - 2, x + width - 1, y + height - 2);
        SS_DrawLine(hdc, x, y + 1, x + width - 1, y + 1);

		SelectObject(hdc, hPenOld);
		DeleteObject(hPen);
		DeleteObject(upperBrush1);
		DeleteObject(upperBrush2);
		DeleteObject(bottomBrush1);
		DeleteObject(bottomBrush2);

        if ((bMouseOver || bPushed) && enabled)
        {
		  HBRUSH brush = CreateSolidBrush(RGB(146, 161, 189));
		  rc.left += 2;
		  rc.right -= 2;
		  rc.top += 1;
		  rc.bottom -= 1;
		  SelectObject(hdc, brush);
          FrameRect(hdc, &rc, brush);
		  DeleteObject(brush);
        }
        

        arrowSize = min((width - 2) / 3, (height - 2) / 3);
        arrowX = x + (width - arrowSize) / 2 + (bPushed ? 1 : 0);
        arrowY = y + (height - 1) / 2 + (bPushed ? 1 : 0);
        if (width > 2 && height > 2)
        {
	      RECT arrowRect = {0,0,0,0};
          HBRUSH enabledBrush = CreateSolidBrush(lpBook->sheetScrollArrowColor);//mark
          HBRUSH lightBrush = GetSysColorBrush(COLOR_3DHIGHLIGHT);
          HBRUSH darkBrush = GetSysColorBrush(COLOR_3DSHADOW);

          if (button == 1)
          {
            if (enabled)
            {
              for (i = 0; i < arrowSize; i++)
			  {
				arrowRect.left = arrowX + i;
				arrowRect.top = arrowY - i;
				arrowRect.right = arrowRect.left+1;
				arrowRect.bottom = arrowRect.top + ( 2 * i + 1);
                FillRect(hdc, &arrowRect, enabledBrush);
			  }
			  arrowRect.left = arrowX - 2;
			  arrowRect.top = arrowY - arrowSize + 1,
			  arrowRect.right = arrowRect.left + 1;
			  arrowRect.bottom = arrowRect.top + (2 * arrowSize - 1);
              FillRect(hdc, &arrowRect, enabledBrush);
            }
            else
            {
              for (i = 0; i < arrowSize; i++)
			  {
				arrowRect.left = arrowX + i + 1;
				arrowRect.top = arrowY - i + 1;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, lightBrush);
			  }
			  arrowRect.left = arrowX - 1;
			  arrowRect.top = arrowY - arrowSize + 2;
			  arrowRect.right = arrowRect.left + 1;
			  arrowRect.bottom = arrowRect.top + (2 * arrowSize - 1);
              FillRect(hdc, &arrowRect, lightBrush);
              for (i = 0; i < arrowSize; i++)
			  {
 				arrowRect.left = arrowX + i;
				arrowRect.top = arrowY - i;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, darkBrush);
			  }
			  arrowRect.left = arrowX - 2;
			  arrowRect.top = arrowY - arrowSize + 1;
			  arrowRect.right = arrowRect.left + 1;
			  arrowRect.bottom = arrowRect.top + (2 * arrowSize - 1);
              FillRect(hdc, &arrowRect, darkBrush);
            }
          }
          else if (button == 2)
          {
           if (enabled)
            {
              for (i = 0; i < arrowSize; i++)
			  {
				arrowRect.left = arrowX + i;
				arrowRect.top = arrowY - i;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, enabledBrush);
			  }
            }
            else
            {
              for (i = 0; i < arrowSize; i++)
			  {
				arrowRect.left = arrowX + i + 1;
				arrowRect.top = arrowY - i + 1;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, lightBrush);
			  }
              for (i = 0; i < arrowSize; i++)
			  {
				arrowRect.left = arrowX + i;
				arrowRect.top = arrowY - i;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, darkBrush);
			  }
            }
          }
          else if (button == 3)
          {
            if (enabled)
            {
              for (i = 0; i < arrowSize; i++)
			  {
 				arrowRect.left = arrowX + arrowSize - 1 - i;
				arrowRect.top = arrowY - i;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, enabledBrush);
			  }
            }
            else
            {
              for (i = 0; i < arrowSize; i++)
			  {
  				arrowRect.left = arrowX + arrowSize - i;
				arrowRect.top = arrowY - i + 1;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, lightBrush);
			  }
              for (i = 0; i < arrowSize; i++)
			  {
  				arrowRect.left = arrowX + arrowSize - 1 - i;
				arrowRect.top = arrowY - i;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, darkBrush);
			  }
            }
          }
          else if (button == 4)
          {
            if (enabled)
            {
              for (i = 0; i < arrowSize; i++)
			  {
   				arrowRect.left = arrowX + arrowSize - 1 - i;
				arrowRect.top = arrowY - i;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
               FillRect(hdc, &arrowRect, enabledBrush);
			  }
  				arrowRect.left = arrowX + arrowSize + 1;
				arrowRect.top = arrowY - arrowSize + 1;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * arrowSize - 1);
              FillRect(hdc, &arrowRect, enabledBrush);
            }
            else
            {
              for (i = 0; i < arrowSize; i++)
			  {
   				arrowRect.left = arrowX + arrowSize - i;
				arrowRect.top = arrowY - i + 1;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, lightBrush);
			  }
  			  arrowRect.left = arrowX + arrowSize + 2;
			  arrowRect.top = arrowY - arrowSize + 2;
			  arrowRect.right = arrowRect.left + 1;
			  arrowRect.bottom = arrowRect.top + (2 * i + 1);
              FillRect(hdc, &arrowRect, lightBrush);
              for (i = 0; i < arrowSize; i++)
			  {
  				arrowRect.left = arrowX + arrowSize - 1 - i;
				arrowRect.top = arrowY - i;
				arrowRect.right = arrowRect.left + 1;
				arrowRect.bottom = arrowRect.top + (2 * i + 1);
                FillRect(hdc, &arrowRect, darkBrush);
			  }
   			 arrowRect.left = arrowX + arrowSize + 1;
			 arrowRect.top = arrowY - arrowSize + 1;
			 arrowRect.right = arrowRect.left + 1;
			 arrowRect.bottom = arrowRect.top + (2 * i + 1);
             FillRect(hdc, &arrowRect, darkBrush);
            }
          }
          DeleteObject(enabledBrush);
        }
		hPen = CreatePen(PS_SOLID, 1, RGB(146, 161, 189));
		hPenOld = SelectObject(hdc, hPen);
        SS_DrawLine(hdc, x, y, x + width, y);
        SS_DrawLine(hdc, x, y + height - 1, x + width, y + height - 1);
		SelectObject(hdc, hPenOld);
		DeleteObject(hPen);


}
#endif
void SS_DrawScrollHomeLeft(HDC hdc, RECT *prc, short nZoom, BOOL bPushed)
{
  RECT rc = *prc;

  COLORREF cr3DText = GetSysColor(COLOR_BTNTEXT);
  HPEN hpen3DText = CreatePen(PS_SOLID, 0, cr3DText);
  HPEN hpenOld;
  int x, y;       // location of arrow tip
  int nSize = (4 * nZoom) / SPRD_ZOOM_INITVAL;  // height of arrow in pixels
  int i;

  SS_Draw3DSurface(hdc, &rc, bPushed, TRUE);
  hpenOld = SelectObject(hdc, hpen3DText);
  nSize = min(nSize, (rc.right - rc.left + 1) / 2);
  nSize = min(nSize, rc.bottom - rc.top - 2);
  if( nSize > 0 )
  {
    x = (rc.left + rc.right - nSize + 2) / 2;
    y = (rc.top + rc.bottom - 1) / 2;
    for( i = 0; i < nSize; i++ )
    {
      SS_DrawLine(hdc, x+i, y-i, x+i, y+i+1);
    }
    SS_DrawLine(hdc, x-2, y-nSize+1, x-2, y+nSize);
  }
  SelectObject(hdc, hpenOld);
  DeleteObject(hpen3DText);
}

//--------------------------------------------------------------------
#ifdef SS_V80
void SS_DrawScrollEndRightVisualStyles(HDC hdc, RECT *prc, short nZoom, BOOL bPushed, BOOL bMouseOver)
{
    RECT rc = *prc;
// 99914280 -scl
//    POINT pt;
    POINT pt = {7, 10};
    COLORREF clr;
	HRESULT res = S_OK;
	HGLOBAL gh1 = 0;
	LPWSTR lpwstrFile = NULL;
	HGLOBAL gh2 = 0;
	LPWSTR lpwstrColor = NULL;
	HGLOBAL gh3 = 0;
	LPWSTR lpwstrSize = NULL;
	HPEN hPen;
	HPEN hPenOld;
	
	HTHEME hTheme = SS_OpenThemeData(WindowFromDC(hdc), L"ScrollBar");
    gh1 = GlobalAlloc(GHND,255);
    lpwstrFile = (LPWSTR)GlobalLock(gh1);
    gh2 = GlobalAlloc(GHND,255);
    lpwstrColor = (LPWSTR)GlobalLock(gh2);
    gh3 = GlobalAlloc(GHND,255);
    lpwstrSize = (LPWSTR)GlobalLock(gh3);
	SS_DrawThemeBackground( hTheme, hdc, 1, bPushed ? 15 : bMouseOver ? 14 : 13, &rc, &rc);
	res = SS_GetCurrentThemeName(lpwstrFile, 255, lpwstrColor, 255, lpwstrSize, 255);
	SS_GetThemePosition(hTheme, 1, bPushed ? 15 : 13, 3409, &pt );

	if (lstrcmpW((LPCWSTR)lpwstrColor, L"Metallic")== 0)
	  clr = RGB(0,0,0);
	else if (lstrcmpW ((LPCWSTR)lpwstrColor, L"HomeStead") == 0)
	  clr = RGB(255,255,255);
	else if (lstrcmpW ((LPCWSTR)lpwstrColor, L"NormalColor") == 0)
	  clr = RGB(77,97,133);
	else
	  clr = RGB(0,0,0);

	hPen = CreatePen(PS_SOLID, 1, clr);
	hPenOld = SelectObject(hdc, hPen);

    SS_DrawLine(hdc, rc.right - ((rc.right-rc.left-pt.x)/2)+1, rc.top + ((rc.bottom-rc.top-pt.y)/2)+1, rc.right - ((rc.right-rc.left-pt.x)/2)+1, rc.bottom - ((rc.bottom-rc.top-pt.y)/2)-1);

	SelectObject(hdc, hPenOld);
	DeleteObject(hPen);

    SS_CloseThemeData(hTheme);
    GlobalUnlock(gh1);
    GlobalFree(gh1);
    GlobalUnlock(gh2);
    GlobalFree(gh2);
    GlobalUnlock(gh3);
    GlobalFree(gh3);

}

#endif
void SS_DrawScrollEndRight(HDC hdc, RECT *prc, short nZoom, BOOL bPushed)
{
  RECT rc = *prc;
  COLORREF cr3DText = GetSysColor(COLOR_BTNTEXT);
  HPEN hpen3DText = CreatePen(PS_SOLID, 0, cr3DText);
  HPEN hpenOld;
  int x, y;       // location of arrow tip
  int nSize = (4 * nZoom) / SPRD_ZOOM_INITVAL;  // height of arrow in pixels
  int i;

  SS_Draw3DSurface(hdc, &rc, bPushed, TRUE);
  nSize = min(nSize, (rc.right - rc.left + 1) / 2);
  nSize = min(nSize, rc.bottom - rc.top - 2);
  if( nSize > 0 )
  {
    x = (rc.left + rc.right + nSize - 3) / 2;
    y = (rc.top + rc.bottom - 1) / 2;
    hpenOld = SelectObject(hdc, hpen3DText);
    for( i = 0; i < nSize; i++ )
    {
      SS_DrawLine(hdc, x-i, y-i, x-i, y+i+1);
    }
    SS_DrawLine(hdc, x+2, y-nSize+1, x+2, y+nSize);
    SelectObject(hdc, hpenOld);
    DeleteObject(hpen3DText);
  }
}

//--------------------------------------------------------------------
#ifdef SS_V80
void SS_DrawScrollLineLeftVisualStyles(HDC hdc, RECT *prc, short nZoom, BOOL bPushed, BOOL bMouseOver)
{
  RECT rc = *prc;
	HTHEME hTheme = SS_OpenThemeData(WindowFromDC(hdc), L"ScrollBar");
	SS_DrawThemeBackground( hTheme, hdc, 1, bPushed ? 11 : bMouseOver ? 10 : 9, &rc, &rc);
	SS_CloseThemeData(hTheme);
}
#endif
void SS_DrawScrollLineLeft(HDC hdc, RECT *prc, short nZoom, BOOL bPushed)
{
  RECT rc = *prc;

  COLORREF cr3DText = GetSysColor(COLOR_BTNTEXT);
  HPEN hpen3DText = CreatePen(PS_SOLID, 0, cr3DText);
  HPEN hpenOld;
  int x, y;       // location of arrow tip
  int nSize = (4 * nZoom) / SPRD_ZOOM_INITVAL;  // height of arrow in pixels
  int i;

  SS_Draw3DSurface(hdc, &rc, bPushed, TRUE);
  nSize = min(nSize, (rc.right - rc.left + 1) / 2);
  nSize = min(nSize, rc.bottom - rc.top);
  x = (rc.left + rc.right - nSize) / 2;
  y = (rc.top + rc.bottom - 1) / 2;
  hpenOld = SelectObject(hdc, hpen3DText);
  for( i = 0; i < nSize; i++ )
  {
    SS_DrawLine(hdc, x+i, y-i, x+i, y+i+1);
  }
  SelectObject(hdc, hpenOld);
  DeleteObject(hpen3DText);
}

//--------------------------------------------------------------------
#ifdef SS_V80
void SS_DrawScrollLineRightVisualStyles(HDC hdc, RECT *prc, short nZoom, BOOL bPushed, BOOL bMouseOver)
{
  RECT rc = *prc;
	HTHEME hTheme = SS_OpenThemeData(WindowFromDC(hdc), L"ScrollBar");
	SS_DrawThemeBackground( hTheme, hdc, 1, bPushed ? 15 : bMouseOver ? 14 : 13, &rc, &rc);
	SS_CloseThemeData(hTheme);
}
#endif
void SS_DrawScrollLineRight(HDC hdc, RECT *prc, short nZoom, BOOL bPushed)
{
  RECT rc = *prc;
  COLORREF cr3DText = GetSysColor(COLOR_BTNTEXT);
  HPEN hpen3DText = CreatePen(PS_SOLID, 0, cr3DText);
  HPEN hpenOld;
  int x, y;       // location of arrow tip
  int nSize = (4 * nZoom) / SPRD_ZOOM_INITVAL;  // height of arrow in pixels
  int i;

  SS_Draw3DSurface(hdc, &rc, bPushed, TRUE);
  nSize = min(nSize, (rc.right - rc.left + 1) / 2);
  nSize = min(nSize, rc.bottom - rc.top);
  x = (rc.left + rc.right + nSize - 1) / 2;
  y = (rc.top + rc.bottom - 1) / 2;
  hpenOld = SelectObject(hdc, hpen3DText);
  for( i = 0; i < nSize; i++ )
  {
    SS_DrawLine(hdc, x-i, y-i, x-i, y+i+1);
  }
  SelectObject(hdc, hpenOld);
  DeleteObject(hpen3DText);
}


short SSTab_GetSheetDisplayName(LPSS_BOOK lpBook, short nSheet, LPTSTR lpszName, int nLen)
{
TBGLOBALHANDLE hSheetName = SS_GetSheetName(lpBook, nSheet);

*lpszName = '\0';

if (!hSheetName)
	{
  TCHAR szSheetName[SS_MAXSHEETNAME + 1];
	LoadString(hDynamicInst, IDS_SHEET_STRING, szSheetName, 16);
	_stprintf(&szSheetName[lstrlen(szSheetName)], _T("%ld"), nSheet + 1);
  lstrcpyn(lpszName, szSheetName, nLen);
	}
else
	{
	LPTSTR pszSheetName = (LPTSTR)tbGlobalLock(hSheetName);
	lstrcpyn(lpszName, pszSheetName, nLen);
	tbGlobalUnlock(hSheetName);
	}

return (lstrlen(lpszName));
}


short SSTab_GetTabWidth(LPSS_BOOK lpBook, HDC hDC, short nSheetIndex)
{
TCHAR szSheetName[SS_MAXSHEETNAME + 1];
RECT  rcTemp;
short nWidth = 0;

if (SS_GetSheetVisible(lpBook, nSheetIndex))
	{
	SSTab_GetSheetDisplayName(lpBook, nSheetIndex, szSheetName, SS_MAXSHEETNAME + 1);
	SetRectEmpty(&rcTemp);
	DrawText(hDC, szSheetName, -1, &rcTemp, DT_SINGLELINE | DT_CALCRECT);

	nWidth = (short)((rcTemp.right - rcTemp.left) + ((SS_TABOVERLAP + SS_TABMARGIN) * 2));
	}

return (nWidth);
}


BOOL SSTabx_GetTabRect(LPSS_BOOK lpBook, HDC hDC, short nSheet, LPPOINT lpMouse, LPRECT lpRectBounds, LPRECT lpRectTab, LPSHORT lpnTabLast)
{
short nPos = (short)lpRectBounds->left;
short nWidth = 0;
short nSheetCount = SS_GetSheetCount(lpBook);
short nLeftSheet = SS_GetTabStripLeftSheet(lpBook);
short nSheetIndex = nLeftSheet;
HFONT hFontOld;
int   iBkModeOld;
HDC   hDCOrig = hDC;
BOOL  fRet = FALSE;

if (!hDCOrig)
	{
	HFONT hFontTab;

	hDC = GetDC(lpBook->hWnd);
	hFontTab = SS_GetTabStripFont(lpBook);
	hFontOld = (HFONT)SelectObject(hDC, hFontTab);
	iBkModeOld = SetBkMode(hDC, TRANSPARENT);
	}

SetRectEmpty(lpRectTab);

if (nSheet != -1 && nSheet == SSTab_GetPrevVisibleTab(lpBook, nLeftSheet))
	{
	nSheetIndex = SSTab_GetNextVisibleTab(lpBook, nSheet);
	nWidth = SSTab_GetTabWidth(lpBook, hDC, nSheetIndex);

	if (lpRectTab)
		{
		lpRectTab->right = nPos + SS_TABOVERLAP;
		lpRectTab->top = lpRectBounds->top;
		lpRectTab->left = nPos - nWidth;
		lpRectTab->bottom = lpRectBounds->bottom;
		}

	fRet = TRUE;
	}

else
	{
	for (; nSheetIndex < nSheetCount && nPos < lpRectBounds->right && !fRet; nSheetIndex++)
		{
		nWidth = SSTab_GetTabWidth(lpBook, hDC, nSheetIndex);

		if (nSheetIndex == nSheet || (lpMouse && lpMouse->x >= nPos && lpMouse->x < nPos + nWidth))
			{
			if (lpRectTab)
				{
				lpRectTab->left = nPos;
				lpRectTab->top = lpRectBounds->top;
				lpRectTab->right = nPos + nWidth;
				lpRectTab->bottom = lpRectBounds->bottom;
				}

			fRet = TRUE;
			}

		if (nWidth)
			nPos += nWidth - SS_TABOVERLAP;
		}
	}

if (lpnTabLast)
	*lpnTabLast = nSheetIndex - 1;

if (!hDCOrig)
	{
	hFontOld = SelectObject(hDC, hFontOld);
	SetBkMode(hDC, iBkModeOld);
	ReleaseDC(lpBook->hWnd, hDC);
	}

return (fRet);
}


BOOL SSTab_GetTabRect(LPSS_BOOK lpBook, HDC hDC, short nSheet, LPRECT lpRectBounds, LPRECT lpRectTab)
{
return SSTabx_GetTabRect(lpBook, hDC, nSheet, NULL, lpRectBounds, lpRectTab, NULL);
}


BOOL SSTab_GetTabRectFromMouse(LPSS_BOOK lpBook, HDC hDC, LPPOINT lpMouse, LPRECT lpRectBounds, LPRECT lpRectTab, LPSHORT lpnSheet)
{
return SSTabx_GetTabRect(lpBook, hDC, -1, lpMouse, lpRectBounds, lpRectTab, lpnSheet);
}


short SSTab_GetRightSheet(LPSS_BOOK lpBook, HDC hDC, LPRECT lpRectBounds)
{
short nSheets = SS_GetSheetCount(lpBook);
short nSheetRight = 0;

SSTabx_GetTabRect(lpBook, hDC, (short)(nSheets - 1), NULL, lpRectBounds, NULL, &nSheetRight);

return (nSheetRight);
}


void SSTab_DisplayActiveTab(LPSS_BOOK lpBook)
{
RECT  rcTabs;
RECT  rcTab;
RECT  Rect;
short nLeftSheet = SS_GetTabStripLeftSheet(lpBook);
short nSheet = lpBook->nActiveSheet;

SS_GetTabStripRect(lpBook, &Rect);
SSTabPtr_CalcRects(lpBook, &Rect, NULL, NULL, NULL, NULL, &rcTabs);

if (nSheet < nLeftSheet)
	{
   SS_FireTabScrolling(lpBook, nLeftSheet, nSheet);
	lpBook->nTabStripLeftSheet = nSheet;
	SS_FireTabScrolled(lpBook, nLeftSheet, nSheet);
	}
else
	{
	while (nSheet != lpBook->nTabStripLeftSheet &&
          (!SSTab_GetTabRect(lpBook, 0, nSheet, &rcTabs, &rcTab) ||
           rcTab.right > rcTabs.right))
		lpBook->nTabStripLeftSheet++;

	if (nLeftSheet != lpBook->nTabStripLeftSheet)
		{
		SS_FireTabScrolling(lpBook, nLeftSheet, lpBook->nTabStripLeftSheet);
		SS_FireTabScrolled(lpBook, nLeftSheet, lpBook->nTabStripLeftSheet);
		}
	}

if (nLeftSheet != lpBook->nTabStripLeftSheet)
	SS_InvalidateTabStrip(lpBook);
}


// This function will return -1 if no sheets are visible.
short SSTab_GetFirstVisibleTab(LPSS_BOOK lpBook)
{
short nSheets = SS_GetSheetCount(lpBook);
short nSheet = 0;

while (nSheet < nSheets && !SS_GetSheetVisible(lpBook, nSheet))
	nSheet++;

return (nSheet >= nSheets ? -1 : nSheet);
}


// This function will return -1 if no sheets are visible.
short SSTab_GetLastVisibleTab(LPSS_BOOK lpBook)
{
short nSheets = SS_GetSheetCount(lpBook);
short nSheet = nSheets - 1;

while (nSheet >= 0 && !SS_GetSheetVisible(lpBook, nSheet))
	nSheet--;

return (nSheet);
}


// This function will return -1 if no sheets are visible.
short SSTab_GetNextVisibleTab(LPSS_BOOK lpBook, short nSheet)
{
short nSheets = SS_GetSheetCount(lpBook);

nSheet++;

while (nSheet < nSheets && !SS_GetSheetVisible(lpBook, nSheet))
	nSheet++;

return (nSheet >= nSheets ? -1 : nSheet);
}


// This function will return -1 if no sheets are visible.
short SSTab_GetPrevVisibleTab(LPSS_BOOK lpBook, short nSheet)
{
nSheet--;

while (nSheet >= 0 && !SS_GetSheetVisible(lpBook, nSheet))
	nSheet--;

return (nSheet);
}
