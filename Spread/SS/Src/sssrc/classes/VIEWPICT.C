/*********************************************************
* VIEWPICT.C
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
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <math.h>
#include "toolbox.h"
#include "fphdc.h"
#include "viewpict.h"
#include "wintools.h"

#define VIEWPICT_OFFSET_LEFT     0
#define VIEWPICT_SCROLL_INC      16
#define MAPREADBUFFCNT           800
#define MAXREAD                  32767
#define BFT_BITMAP               0x4d42 /* 'BM' */

#define PALVERSION               0x300
#define MAXPALETTE               256   /* max. # supported palette entries */

#define WIDTHBYTES(i)            ((i+31)/32*4)
#define ISDIB(bft)               ((bft) == BFT_BITMAP)
#define BOUND(x, Size, Min, Max) ((x) < (Min) ? (Min) : ((x) > ((Max) - \
                                 (Size)) ? ((Max) - (Size)) : (x)))

/**********************
* Function prototypes
**********************/

static LPVIEWPICT       ViewPictInit(GLOBALHANDLE FAR *hGlobal);
WINENTRY                ViewPictProc(HWND hWnd, UINT Msg, WPARAM wParam,
                                     LPARAM lParam);
void                    ViewPictPaint(HWND hWnd, HDC hDC, HDC hDCDisplay,
                                      LPRECT lpRect, BOOL fErase, BOOL fUseDib,
                                      LPRECT lpRectOrig);
void                    ViewPictFree(LPVIEWPICT ViewPict);
static void             ViewPictSetVScrollBar(HWND hWnd, LPVIEWPICT ViewPict);
static void             ViewPictSetHScrollBar(HWND hWnd, LPVIEWPICT ViewPict);
static void             ViewPictSetScrollBar(HWND hWnd, LPVIEWPICT ViewPict);
static short            MaxLine(LPVIEWPICT ViewPict);
static short            MaxCol(LPVIEWPICT ViewPict);
static short            ViewPictScrollWidth(LPVIEWPICT ViewPict);
static short            ViewPictScrollHeight(LPVIEWPICT ViewPict);
static void             ViewPictSetPtr(HWND hWnd, GLOBALHANDLE hGlobal);
static short            ViewPictSetPicture(HWND hWnd, LPVIEWPICT ViewPict,
                                           LPVIEWPICT_PICTURE Picture,
                                           LPCTSTR FileName,
                                           BOOL SetPictureSize);
static void             ViewGetBitMapInfoHeader(LPVIEWPICT_PICTURE Picture);
static short            ViewBmpLoadImage(HWND hWnd, LPVIEWPICT_PICTURE Picture,
                                         LPCTSTR FileName, LPBYTE BmpResourceData,
                                         long BmpResourceDataSize);
static BOOL             ViewPictAddPicture(HWND hWnd, LPVIEWPICT ViewPict,
                                           LPVIEWPICT_ANIMATEPICT AnimatePict);
static BOOL             ViewPictInsertFrame(HWND hWnd, LPVIEWPICT ViewPict,
                                            LPVIEWPICT_ANIMATEPICT AnimatePict,
                                            short Index);
static BOOL             ViewPictShowCurFrame(HWND hWnd, LPVIEWPICT ViewPict);
static BOOL             ViewPictGetCurFrame(LPVIEWPICT ViewPict,
                                            LPVIEWPICT_PICTURE Picture,
                                            LPVIEWPICT_FRAME Frame);
static BOOL             ViewPictCreateBitmap2(HDC hDC, LPVIEWPICT lpViewPict,
                                              LPVIEWPICT_PICTURE lpPicture);


BOOL RegisterViewPict(hInstance, lpszClassName)

HANDLE     hInstance;
LPCTSTR      lpszClassName;
{
WNDCLASS wc;
BOOL     bRet = TRUE;

wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
wc.lpfnWndProc   = ViewPictProc;
wc.cbClsExtra    = 0;
wc.cbWndExtra    = 4;
wc.hInstance     = hInstance;
wc.hIcon         = NULL;
wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName  = NULL;
wc.lpszClassName = lpszClassName;
if (!RegisterClass(&wc))
   bRet = FALSE;
return bRet;
}


static LPVIEWPICT ViewPictInit(hGlobal)

GLOBALHANDLE FAR *hGlobal;
{
LPVIEWPICT        ViewPict;

if (!(*hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                             (long)sizeof(VIEWPICT))))
   return (NULL);

ViewPict = (LPVIEWPICT)GlobalLock(*hGlobal);

ViewPict->ScrollIncX = 10;
ViewPict->ScrollIncY = 10;
ViewPict->PictStyle  = VPS_NORMAL;

return (ViewPict);
}

//--------------------------------------------------------------------
//
//  The ViewPict_OnHScroll() function handles WM_HSCROLL messages.
//

void ViewPict_OnHScroll(HWND hWnd, HWND hWndCtl, UINT nCode, int nPos)
{
GLOBALHANDLE       hGlobal;
LPVIEWPICT         ViewPict;
short              ScrollIt;

ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

if ((ViewPict->PictStyle & VPS_SCROLL) == 0)
   {
   GlobalUnlock(hGlobal);
   return;
   }

ScrollIt = 0;
switch (nCode)
   {
   /******************************
   * scroll one line to the left
   ******************************/

   case SB_LINEUP:
      ScrollIt = -ViewPict->ScrollIncX;
      break;

   /*******************************
   * scroll one line to the right
   *******************************/

   case SB_LINEDOWN:
      ScrollIt = ViewPict->ScrollIncX;
      break;

   /************************
   * Scroll one page right
   ************************/

   case SB_PAGEDOWN:
      ScrollIt = ViewPict->cxClient / 2;
      break;

   /***********************
   * Scroll one page left
   ***********************/

   case SB_PAGEUP:
      ScrollIt = -ViewPict->cxClient / 2;
      break;

   case SB_TOP:
      ScrollIt = -ViewPict->CurCol;
      break;

   case SB_BOTTOM:
      ScrollIt = ViewPict->PictureWidth;
      break;

   /*****************************************
   * The slider or slider track was clicked
   *****************************************/

   case SB_THUMBTRACK:
   case SB_THUMBPOSITION:
      ScrollIt = ((short)nPos * ViewPict->ScrollIncX) -
                 ViewPict->CurCol;
      break;
   }

/****************************
* Scroll the desired region
****************************/

if (((ScrollIt < 0 && ViewPict->CurCol > 0) || (ScrollIt > 0 &&
    ViewPict->PictureWidth - ViewPict->CurCol > ViewPict->cxClient)) &&
    (ScrollIt = BOUND(ViewPict->CurCol + ScrollIt, ViewPict->cxClient, 0,
    ViewPict->PictureWidth) - ViewPict->CurCol))
   {
   ViewPict->CurCol += ScrollIt;

   ScrollWindow(hWnd, -ScrollIt, 0, NULL, NULL);
   UpdateWindow(hWnd);
   ViewPictSetHScrollBar(hWnd, ViewPict);
   }

GlobalUnlock(hGlobal);
}

//--------------------------------------------------------------------
//
//  The ViewPict_OnVScroll() function handles WM_VSCROLL messages.
//

void ViewPict_OnVScroll(HWND hWnd, HWND hWndCtl, UINT nCode, int nPos)
{
GLOBALHANDLE       hGlobal;
LPVIEWPICT         ViewPict;
short              ScrollIt;

ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

if ((ViewPict->PictStyle & VPS_SCROLL) == 0)
   {
   GlobalUnlock(hGlobal);
   return;
   }

ScrollIt = 0;
switch (nCode)
   {
   /*********************
   * scroll one line up
   *********************/

   case SB_LINEUP:
      ScrollIt = -ViewPict->ScrollIncY;
      break;

   /***********************
   * scroll one line down
   ***********************/

   case SB_LINEDOWN:
      ScrollIt = ViewPict->ScrollIncY;
      break;

   /***********************
   * Scroll one page down
   ***********************/

   case SB_PAGEDOWN:
      ScrollIt = ViewPict->cyClient / 2;
      break;

   /*********************
   * Scroll one page up
   *********************/

   case SB_PAGEUP:
      ScrollIt = -ViewPict->cyClient / 2;
      break;

   /******
   * Top
   ******/

   case SB_TOP:
      ScrollIt = -ViewPict->CurLine;
      break;

   /*********
   * Bottom
   *********/

   case SB_BOTTOM:
      ScrollIt = ViewPict->PictureHeight;
      break;

   /*****************************************
   * The slider or slider track was clicked
   *****************************************/

   case SB_THUMBTRACK:
   case SB_THUMBPOSITION:
      ScrollIt = ((short)nPos * ViewPict->ScrollIncY) -
                 ViewPict->CurLine;
      break;
   }

/****************************
* Scroll the desired region
****************************/

if (((ScrollIt < 0 && ViewPict->CurLine > 0) || (ScrollIt > 0 &&
    ViewPict->PictureHeight - ViewPict->CurLine > ViewPict->cyClient)) &&
    (ScrollIt = BOUND(ViewPict->CurLine + ScrollIt, ViewPict->cyClient,
    0, ViewPict->PictureHeight) - ViewPict->CurLine))
   {
   ViewPict->CurLine += ScrollIt;

   ScrollWindow(hWnd, 0, -ScrollIt, NULL, NULL);
   UpdateWindow(hWnd);
   ViewPictSetVScrollBar(hWnd, ViewPict);
   }

GlobalUnlock(hGlobal);
}

//--------------------------------------------------------------------

WINENTRY ViewPictProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
LPCREATESTRUCT     CreateStruct;
GLOBALHANDLE       hGlobal;
LPVIEWPICT         ViewPict;
LPVIEWPICT_FRAME   Frames;
LPVIEWPICT_PICTURE Pictures;
LPVIEWPICT_ANIMATE Animate;
LPVIEWPICT_PAINT   lpViewPictPaint;
LPRECT             lpRect;
RECT               Rect;
RECT               RectTemp;
HDC                hDC;
#ifndef BUGS         
// Bug-018
HRGN               hRgn;
#endif
PAINTSTRUCT        Paint;
POINT              Point;
BITMAP             bm;
HWND               hWndTemp;
long               lPos;
long               lRet;
short              CurFrame;
short              Ret;
short              i;

switch (Msg)
   {
   /********************
   * Create new window
   ********************/

   case WM_CREATE:
      ViewPict = ViewPictInit(&hGlobal);
      ViewPictSetPtr(hWnd, hGlobal);

      ViewPict->PictStyle = GetWindowLong(hWnd, GWL_STYLE);

      CreateStruct = (LPCREATESTRUCT)lParam;
      Ret = (short)SendMessage(hWnd, WM_SETTEXT, 0,
                               (LPARAM)CreateStruct->lpszName);

      GlobalUnlock(hGlobal);
      return (0);

   case WM_GETDLGCODE:
      if (GetWindowLong(hWnd, GWL_STYLE) & VPS_STATIC)
         return (DLGC_STATIC);
      else
         return (DLGC_WANTARROWS);

   case WM_NCHITTEST:
      if (GetWindowLong(hWnd, GWL_STYLE) & VPS_STATIC)
         return (HTTRANSPARENT);

      break;

   case WM_TIMER:
      SendMessage(hWnd, VPM_SHOWNEXTFRAME, 0, 0L);
      return (0);

   /*************************************************************
   * Each time a character is pressed, this message is received
   *************************************************************/

   case WM_KEYDOWN:
      switch (wParam)
         {
         /**********************************************
         * Left or Right keys map to horz scroll codes
         **********************************************/

         case VK_LEFT:
            FORWARD_WM_HSCROLL(hWnd, 0, SB_LINEUP, 0, SendMessage);
            return 0;

         case VK_RIGHT:
            FORWARD_WM_HSCROLL(hWnd, 0, SB_LINEDOWN, 0, SendMessage);
            return 0;

         /********************************************
         * Up and down keys map to vert scroll codes
         ********************************************/

         case VK_UP:
            FORWARD_WM_VSCROLL(hWnd, 0, SB_LINEUP, 0, SendMessage);
            return 0;

         case VK_DOWN:
            FORWARD_WM_VSCROLL(hWnd, 0, SB_LINEDOWN, 0, SendMessage);
            return 0;

         case VK_NEXT:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               FORWARD_WM_HSCROLL(hWnd, 0, SB_PAGEDOWN, 0, SendMessage);
            else
               FORWARD_WM_VSCROLL(hWnd, 0, SB_PAGEDOWN, 0, SendMessage);
            return 0;

         case VK_PRIOR:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               FORWARD_WM_HSCROLL(hWnd, 0, SB_PAGEUP, 0, SendMessage);
            else
               FORWARD_WM_VSCROLL(hWnd, 0, SB_PAGEUP, 0, SendMessage);
            return 0;

         case VK_HOME:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               FORWARD_WM_VSCROLL(hWnd, 0, SB_TOP, 0, SendMessage);
            else
               FORWARD_WM_HSCROLL(hWnd, 0, SB_TOP, 0, SendMessage);
            return (0);

         case VK_END:
            if (HIBYTE(GetKeyState(VK_CONTROL)))
               FORWARD_WM_VSCROLL(hWnd, 0, SB_BOTTOM, 0, SendMessage);
            else
               FORWARD_WM_HSCROLL(hWnd, 0, SB_BOTTOM, 0, SendMessage);
            return (0);

         }

      break;

   /******************************
   * The window has been resized
   ******************************/

   case WM_SIZE:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

      ViewPict->cxClient = LOWORD(lParam);
      ViewPict->cyClient = HIWORD(lParam);

      if (ViewPict->Picture.hBitMap2)
         {
         hDC = fpGetDC(hWnd);
         if (!ViewPictCreateBitmap2(hDC, ViewPict, &ViewPict->Picture))
            Ret = VIEWPICT_ERR_MEMORY;
         ReleaseDC(hWnd, hDC);
         }

      /************************************************
      * scroll bars may have different dimensions now
      ************************************************/

      ViewPictSetScrollBar(hWnd, ViewPict);

      GlobalUnlock(hGlobal);
      return (0);

   /*********************************************************
   * Any horizontal scroll (with mouse) causes this message
   *********************************************************/

   case WM_HSCROLL:
      HANDLE_WM_HSCROLL(hWnd, wParam, lParam, ViewPict_OnHScroll);
      return (0);

   /***********************************
   * A vertical scroll action occured
   ***********************************/

   case WM_VSCROLL:
      HANDLE_WM_VSCROLL(hWnd, wParam, lParam, ViewPict_OnVScroll);
      return (0);

   case WM_MOUSEACTIVATE:
      SetFocus(hWnd);
      break;

   case WM_LBUTTONDOWN:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      ViewPict->xPointer = LOWORD(lParam);
      ViewPict->yPointer = HIWORD(lParam);
      FORWARD_WM_COMMAND(hWnd, GetWindowID(hWnd), hWnd, VPN_BUTTONDOWN,
                         tbSendMessageToParent);
      GlobalUnlock(hGlobal);
      break;

   case WM_LBUTTONDBLCLK:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      ViewPict->xPointer = LOWORD(lParam);
      ViewPict->yPointer = HIWORD(lParam);
      FORWARD_WM_COMMAND(hWnd, GetWindowID(hWnd), hWnd, VPN_DBLCLK,
                         tbSendMessageToParent);
      GlobalUnlock(hGlobal);
      break;

   case WM_CLOSE:
      DestroyWindow(hWnd);
      return (0);

   /*********************************************************
   * This message is received when the window is destroyed,
   * either under program control or external direction
   *********************************************************/

   case WM_DESTROY:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      ViewPictFree(ViewPict);
      GlobalFree(hGlobal);
      return (0);

   case WM_SETTEXT:
      if (lParam)
         return (SendMessage(hWnd, VPM_SETPICTURE,
                 (short)GetWindowLong(hWnd, GWL_STYLE), lParam));

      return (0);

   case WM_SETFOCUS:
      FORWARD_WM_COMMAND(hWnd, GetWindowID(hWnd), hWnd, VPN_SETFOCUS,
                         tbSendMessageToParent);
      break;

   case WM_KILLFOCUS:
      FORWARD_WM_COMMAND(hWnd, GetWindowID(hWnd), hWnd, VPN_KILLFOCUS,
                         tbSendMessageToParent);
      break;

   case VPM_GETPOINTERPOS:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      lPos = MAKELONG(ViewPict->xPointer, ViewPict->yPointer);
      GlobalUnlock(hGlobal);
      return (lPos);

   case VPM_SETPICTURE:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      ViewPictFree(ViewPict);

      ViewPict->Picture.PictStyle = (long)wParam;
      ViewPict->PictStyle = (long)wParam;

      Ret = ViewPictSetPicture(hWnd, ViewPict, &ViewPict->Picture,
                               (LPCTSTR)lParam, TRUE);

      ViewPict->ScrollWidth = (ViewPict->PictureWidth + ViewPict->ScrollIncX -
                              1) / ViewPict->ScrollIncX;

      ViewPict->ScrollHeight = (ViewPict->PictureHeight +
                               ViewPict->ScrollIncY - 1) /
                               ViewPict->ScrollIncY;

      ViewPictSetScrollBar(hWnd, ViewPict);

      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);

      GlobalUnlock(hGlobal);
      return (Ret);

   case VPM_CLEAR:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      ViewPictFree(ViewPict);

      _fmemset(&ViewPict->Picture, '\0', sizeof(VIEWPICT_PICTURE));
      ViewPict->hGlobalAnimatePictures = 0;
      ViewPict->hGlobalAnimateFrames = 0;
      ViewPict->AnimationFrozen = 0;
      ViewPict->AnimationCurFrame = 0;
      ViewPict->AnimationPictureCnt = 0;
      ViewPict->AnimationFrameCnt = 0;
      ViewPict->CurLine = 0;
      ViewPict->CurCol = 0;

      ViewPictSetScrollBar(hWnd, ViewPict);

      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);

      GlobalUnlock(hGlobal);
      return (TRUE);

   case VPM_SETSTYLE:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

      if (ViewPict->PictStyle == lParam)
         {
         GlobalUnlock(hGlobal);
         return (TRUE);
         }

      ViewPict->PictStyle = lParam;
      Ret = TRUE;

      if (ViewPict->Picture.hBitMap)
         {
         GetObject(ViewPict->Picture.hBitMap, sizeof(BITMAP), (LPVOID)&bm);

         if ((ViewPict->PictStyle & VPS_STRETCH) == 0)
            {
            if (ViewPict->Picture.hBitMap2)
               DeleteObject(ViewPict->Picture.hBitMap2);

            ViewPict->Picture.hBitMap2 = NULL;

            ViewPict->PictureWidth = (short)bm.bmWidth;
            ViewPict->PictureHeight = (short)bm.bmHeight;
            }
         else
            {
            hDC = fpGetDC(hWnd);
            Ret = ViewPictCreateBitmap2(hDC, ViewPict, &ViewPict->Picture);
            ReleaseDC(hWnd, hDC);

            ViewPict->OldcxClient = 0;
            ViewPict->OldcyClient = 0;
            }

         ViewPict->CurCol = 0;
         ViewPict->CurLine = 0;

         ViewPictSetScrollBar(hWnd, ViewPict);

         InvalidateRect(hWnd, NULL, TRUE);
         UpdateWindow(hWnd);
         }

      GlobalUnlock(hGlobal);
      return (Ret);
/*
   case VPM_SETBKCOLOR:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      ViewPict->BkColor = (COLORREF)lParam;
      GlobalUnlock(hGlobal);

      InvalidateRect(hWnd, NULL, TRUE);
      UpdateWindow(hWnd);
      return (0);
*/
   case VPM_SETFRAMES:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

      Animate = (LPVIEWPICT_ANIMATE)lParam;
      ViewPict->AnimationTimeDelay = Animate->AnimateHdr.TimeDelay;

      for (i = 0; i < Animate->AnimateHdr.PictureCnt; i++)
         if (!ViewPictInsertFrame(hWnd, ViewPict, &Animate->AnimatePict[i],
                                  VIEWPICT_END))
            Ret = VIEWPICT_ERR_MEMORY;

      ViewPict->ScrollWidth = (ViewPict->PictureWidth + ViewPict->ScrollIncX -
                              1) / ViewPict->ScrollIncX;

      ViewPict->ScrollHeight = (ViewPict->PictureHeight +
                               ViewPict->ScrollIncY - 1) /
                               ViewPict->ScrollIncY;

      ViewPictSetScrollBar(hWnd, ViewPict);

      ViewPict->AnimationCurTimeDelay = ViewPict->AnimationTimeDelay;
      SetTimer(hWnd, 1, ViewPict->AnimationTimeDelay, NULL);

      GlobalUnlock(hGlobal);
      return (Ret);
/*
   case VPM_SETTIMEDELAY:
      break;
*/
   case VPM_FREEZEANIMATION:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

      if (!ViewPict->AnimationFrozen)
         {
         KillTimer(hWnd, 1);
         ViewPict->AnimationFrozen = TRUE;
         }

      GlobalUnlock(hGlobal);
      return (0);

   case VPM_STARTANIMATION:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

      if (ViewPict->AnimationFrozen)
         {
         ViewPict->AnimationCurTimeDelay = ViewPict->AnimationTimeDelay;
         SetTimer(hWnd, 1, ViewPict->AnimationTimeDelay, NULL);
         ViewPict->AnimationFrozen = FALSE;
         }

      GlobalUnlock(hGlobal);
      return (0);

   case VPM_SHOWNEXTFRAME:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      if (ViewPict->hGlobalAnimateFrames)
         {
         Frames = (LPVIEWPICT_FRAME)GlobalLock(ViewPict->hGlobalAnimateFrames);

         CurFrame = ViewPict->AnimationCurFrame;
         do
            {
            if (ViewPict->AnimationCurFrame < ViewPict->AnimationFrameCnt - 1)
               ViewPict->AnimationCurFrame++;
            else
               ViewPict->AnimationCurFrame = 0;
            } while (Frames[ViewPict->AnimationCurFrame].FrameDeleted &&
                     ViewPict->AnimationCurFrame != CurFrame);

         GlobalUnlock(ViewPict->hGlobalAnimateFrames);
         ViewPictShowCurFrame(hWnd, ViewPict);
         }

      GlobalUnlock(hGlobal);
      return (0);

   case VPM_SHOWPREVFRAME:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      if (ViewPict->hGlobalAnimateFrames)
         {
         Frames = (LPVIEWPICT_FRAME)GlobalLock(ViewPict->hGlobalAnimateFrames);

         CurFrame = ViewPict->AnimationCurFrame;
         do
            {
            if (ViewPict->AnimationCurFrame > 0 &&
                ViewPict->AnimationCurFrame <= ViewPict->AnimationFrameCnt)
               ViewPict->AnimationCurFrame--;
            else
               ViewPict->AnimationCurFrame = ViewPict->AnimationFrameCnt - 1;
            } while (Frames[ViewPict->AnimationCurFrame].FrameDeleted &&
                     ViewPict->AnimationCurFrame != CurFrame);

         GlobalUnlock(ViewPict->hGlobalAnimateFrames);
         ViewPictShowCurFrame(hWnd, ViewPict);
         }

      GlobalUnlock(hGlobal);
      return (0);

   case VPM_SETPALETTE:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

      if (ViewPict->AnimationPictureCnt)
         {
         if (wParam < (WORD)ViewPict->AnimationPictureCnt)
            {
            Pictures = (LPVIEWPICT_PICTURE)GlobalLock(
                                           ViewPict->hGlobalAnimatePictures);

            if (Pictures[wParam].hPal && !Pictures[wParam].fNoDeletePal)
               DeleteObject(Pictures[wParam].hPal);

            Pictures[wParam].hPal = (HPALETTE)lParam;
            Pictures[wParam].fNoDeletePal = TRUE;
            GlobalUnlock(ViewPict->hGlobalAnimatePictures);
            }
         }

      else
         {
         if (ViewPict->Picture.hPal && !ViewPict->Picture.fNoDeletePal)
           DeleteObject(ViewPict->Picture.hPal);

         ViewPict->Picture.hPal = (HPALETTE)lParam;
         ViewPict->Picture.fNoDeletePal = TRUE;
         }

      GlobalUnlock(hGlobal);
      return (TRUE);

   case VPM_GETPALETTE:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      lRet = 0L;

      if (ViewPict->AnimationPictureCnt)
         {
         if (wParam < (WORD)ViewPict->AnimationPictureCnt)
            {
            Pictures = (LPVIEWPICT_PICTURE)GlobalLock(
                                           ViewPict->hGlobalAnimatePictures);

            lRet = (LRESULT)Pictures[wParam].hPal;
            GlobalUnlock(ViewPict->hGlobalAnimatePictures);
            }
         }

      else
         {
         if (ViewPict->Picture.hPal && !ViewPict->Picture.fNoDeletePal)
           DeleteObject(ViewPict->Picture.hPal);

         lRet = (LRESULT)ViewPict->Picture.hPal;
         }

      GlobalUnlock(hGlobal);
      return (lRet);

   case VPM_PAINT:
      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      lpViewPictPaint = (LPVIEWPICT_PAINT)lParam;
      lpRect = &lpViewPictPaint->Rect;

      if (lpViewPictPaint->fUseDib)
         {
         hDC = fpGetDC(hWnd);
         ViewPictPaint(hWnd, (HDC)wParam, hDC, lpRect, TRUE, TRUE,
                       &lpViewPictPaint->RectOrig);
         ReleaseDC(hWnd, hDC);
         }
      else
         {
         if (ViewPict->Picture.hBitMap &&
             (ViewPict->cxClient != lpRect->right - lpRect->left ||
             ViewPict->cyClient != lpRect->bottom - lpRect->top ||
             ((ViewPict->PictStyle & VPS_STRETCH) &&
             !ViewPict->Picture.hBitMap2)))
            {
            ViewPict->cxClient = (short)(lpRect->right - lpRect->left);
            ViewPict->cyClient = (short)(lpRect->bottom - lpRect->top);

            if (ViewPict->PictStyle & VPS_STRETCH)
               {
               hDC = fpGetDC(hWnd);
               ViewPictCreateBitmap2(hDC, ViewPict, &ViewPict->Picture);
               ReleaseDC(hWnd, hDC);
               }
            }

#ifndef  BUGS
// Bug-018
         hRgn = CreateRectRgnIndirect(lpRect);

         SelectClipRgn((HDC)wParam, hRgn);
#endif
         ViewPictPaint(hWnd, (HDC)wParam, (HDC)wParam, lpRect, TRUE, FALSE,
                       NULL);
#ifndef  BUGS
// Bug-018
         SelectClipRgn((HDC)wParam, 0);
         DeleteObject(hRgn);
#endif
         }

      GlobalUnlock(hGlobal);
      return (0);

   case WM_ERASEBKGND:
      return (TRUE);

   case WM_PAINT:
      hDC = fpBeginPaint(hWnd, &Paint);

      if (GetWindowLong(hWnd, GWL_STYLE) & VPS_STATIC)
         {
         GetClientRect(hWnd, &Rect);

         /***************************************************************
         * Check the sibling windows that may lie within the boundaries
         * of this control.  If so, exclude them from the update region
         ***************************************************************/

         Point.x = 0;
         Point.y = 0;
         ClientToScreen(hWnd, &Point);

         for (hWndTemp = GetWindow(GetParent(hWnd), GW_CHILD); hWndTemp;
              hWndTemp = GetNextWindow(hWndTemp, GW_HWNDNEXT))
            {
            if (hWndTemp != hWnd)
               {
               GetWindowRect(hWndTemp, &RectTemp);

               RectTemp.left -= Point.x;
               RectTemp.right -= Point.x;

               RectTemp.top -= Point.y;
               RectTemp.bottom -= Point.y;

               if (RectTemp.left >= 0 &&
                   RectTemp.top >= 0 &&
                   RectTemp.right <= Rect.right &&
                   RectTemp.bottom <= Rect.bottom && IsWindowVisible(hWndTemp))
                  ExcludeClipRect(hDC, RectTemp.left, RectTemp.top,
                                  RectTemp.right, RectTemp.bottom);
               }
            }
         }

      GetClientRect(hWnd, &Rect);

      ViewPict = ViewPictGetPtr(hWnd, &hGlobal);
      ViewPictPaint(hWnd, hDC, hDC, &Rect, !ViewPict->fNoErase, FALSE, NULL);
      GlobalUnlock(hGlobal);

      EndPaint(hWnd, &Paint);
      return (0);

   }

return (DefWindowProc(hWnd, Msg, wParam, lParam));
}


void ViewPictPaint(HWND hWnd, HDC hDC, HDC hDCDisplay, LPRECT lpRect,
                   BOOL fErase, BOOL fUseDib, LPRECT lpRectOrig)
{
GLOBALHANDLE       hGlobal;
LPVIEWPICT         ViewPict;
HBITMAP            hOldBitMap = 0;
HBITMAP            hOldBitMap2 = 0;
HBRUSH             hBrush;
HCURSOR            hCursor;
VIEWPICT_PICTURE   Picture;
VIEWPICT_FRAME     Frame;
HPALETTE           hPalette;
HPALETTE           hPal = 0;
HPALETTE           hPalOld = 0;
HBITMAP            hBitmap;
BITMAP             bm;
HANDLE             hDIB;
HDC                hDCMem;
HDC                hDCMemory = 0;
HDC                hDCMemory2 = 0;
short              PictureWidth;
short              PictureHeight;
short              PictureWidthOrig;
short              PictureHeightOrig;
short              ColorCnt = 0;
short              x;
short              y;
short              xOrig;
short              yOrig;
int                iMapModeOld;

ViewPict = ViewPictGetPtr(hWnd, &hGlobal);

if (!fUseDib)
   {
   ViewPict->cxClient = (short)(lpRect->right - lpRect->left);
   ViewPict->cyClient = (short)(lpRect->bottom - lpRect->top);
   }

#ifdef WIN32
hBrush = FORWARD_WM_CTLCOLORSTATIC(GetParent(hWnd), hDC, hWnd, SendMessage);
#else
hBrush = FORWARD_WM_CTLCOLOR(GetParent(hWnd), hDC, hWnd, CTLCOLOR_STATIC,
                             SendMessage);
#endif

if (fErase)
   FillRect(hDC, lpRect, hBrush);

if (!fUseDib && (ViewPict->cxClient == 0 || ViewPict->cyClient == 0))
   {
   GlobalUnlock(hGlobal);
   return;
   }

SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
SetBkColor(hDC, GetSysColor(COLOR_WINDOW));

if (ViewPict->AnimationFrameCnt == 0)
   {
   _fmemcpy(&Picture, &ViewPict->Picture, sizeof(VIEWPICT_PICTURE));
   _fmemset(&Frame, '\0', sizeof(VIEWPICT_FRAME));
   }
else
   ViewPictGetCurFrame(ViewPict, &Picture, &Frame);

if (Picture.hBitMap)
   {
   hBitmap = Picture.hBitMap;

   if (!fUseDib)
      {
      hDCMemory = CreateCompatibleDC(hDCDisplay);
      hOldBitMap = SelectObject(hDCMemory, Picture.hBitMap);

      if (Picture.hBitMap2)
         {
         hDCMemory2 = CreateCompatibleDC(hDCDisplay);
         hOldBitMap2 = SelectObject(hDCMemory2, Picture.hBitMap2);
         }

      if (Picture.hPal)
         {
         SelectPalette(hDCMemory, Picture.hPal, TRUE);
         RealizePalette(hDCMemory);

         if (Picture.hBitMap2)
            {
            SelectPalette(hDCMemory2, Picture.hPal, TRUE);
            RealizePalette(hDCMemory2);
            }
         }
      }

   if (Picture.hPal)
      {
      hPal = Picture.hPal;
      hPalOld = SelectPalette(hDC, Picture.hPal, TRUE);
      RealizePalette(hDC);
      }

   GetObject(Picture.hBitMap, sizeof(BITMAP), (LPVOID)&bm);
   hDCMem = hDCMemory;

   if (!fUseDib && (ViewPict->PictStyle & VPS_STRETCH))
      {
      if (ViewPict->cxClient != ViewPict->OldcxClient ||
          ViewPict->cyClient != ViewPict->OldcyClient)
         {
         ColorCnt = 0;

         if (Picture.hPal)
            GetObject(Picture.hPal, 2, (LPVOID)&ColorCnt);

         if ((bm.bmPlanes == 1 && bm.bmBitsPixel == 1) || ColorCnt == 2)
            SetStretchBltMode(hDCMemory2, BLACKONWHITE);
         else
            SetStretchBltMode(hDCMemory2, COLORONCOLOR);

         hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
         ShowCursor(TRUE);

         StretchBlt(hDCMemory2, 0, 0, ViewPict->PictureWidth,
                    ViewPict->PictureHeight, hDCMemory, 0, 0,
                    bm.bmWidth, bm.bmHeight, SRCCOPY);

         ViewPict->OldcxClient = ViewPict->cxClient;
         ViewPict->OldcyClient = ViewPict->cyClient;

         ShowCursor(FALSE);
         SetCursor(hCursor);
         }

      hDCMem = hDCMemory2;
      }

   if (fUseDib)
      {
      if (hPal)
         hPalette = hPal;
      else
         hPalette = tbGetSystemPalette();

      /******************************
      * convert the bitmap to a DIB
      ******************************/

      hDIB = tbBitmapToDIB(hBitmap, hPalette);

      if (!hPal)
         DeleteObject(hPalette);

      SetStretchBltMode(hDC, COLORONCOLOR);
      iMapModeOld = SetMapMode(hDC, MM_TEXT);

      if ((ViewPict->PictStyle & VPS_STRETCH) &&
          !(ViewPict->PictStyle & VPS_MAINTAINSCALE))
         {
         tbStretchDibBlt(hDC, lpRect->left, lpRect->top,
                         lpRect->right - lpRect->left, lpRect->bottom -
                         lpRect->top, hDIB, 0, 0, bm.bmWidth, bm.bmHeight,
                         SRCCOPY);
         }

      else
         {
         if ((ViewPict->PictStyle & VPS_STRETCH) &&
             (ViewPict->PictStyle & VPS_MAINTAINSCALE))
            {
            PictureHeight = (short)(((long)(lpRect->right - lpRect->left) *
                            (long)ViewPict->PictureHeight) /
                            (long)ViewPict->PictureWidth);

            if (PictureHeight <= lpRect->bottom - lpRect->top)
               PictureWidth = (short)(lpRect->right - lpRect->left);

            else
               {
               PictureWidth = (short)(((long)(lpRect->bottom - lpRect->top) *
                              (long)ViewPict->PictureWidth) /
                              (long)ViewPict->PictureHeight);
               PictureHeight = (short)(lpRect->bottom - lpRect->top);
               }

            PictureWidthOrig = (short)bm.bmWidth;
            PictureHeightOrig = (short)bm.bmHeight;
            }
         else
            {
            PictureWidth = (short)(((long)ViewPict->PictureWidth *
                           (long)(lpRect->right - lpRect->left)) /
                           (long)(lpRectOrig->right - lpRectOrig->left));
            PictureHeight = (short)(((long)ViewPict->PictureHeight *
                            (long)(lpRect->bottom - lpRect->top)) /
                            (long)(lpRectOrig->bottom - lpRectOrig->top));

            PictureWidthOrig = (short)min(ViewPict->PictureWidth, lpRectOrig->right -
                                          lpRectOrig->left);
            PictureHeightOrig = (short)min(ViewPict->PictureHeight,
                                           lpRectOrig->bottom - lpRectOrig->top);
            }

         if (ViewPict->PictStyle & VPS_CENTER)
            {
            x = max(((lpRect->right - lpRect->left) - PictureWidth) / 2, 0);
            y = max(((lpRect->bottom - lpRect->top) - PictureHeight) / 2, 0);

            if ((ViewPict->PictStyle & VPS_STRETCH) &&
                (ViewPict->PictStyle & VPS_MAINTAINSCALE))
               {
               xOrig = 0;
               yOrig = 0;
               }
            else
               {
               xOrig = max((ViewPict->PictureWidth - (lpRectOrig->right -
                            lpRectOrig->left)) / 2, 0);
               yOrig = max((ViewPict->PictureHeight - (lpRectOrig->bottom -
                            lpRectOrig->top)) / 2, 0);
               }
            }

         else
            {
            x = 0;
            y = 0;
            xOrig = 0;

            if ((ViewPict->PictStyle & VPS_STRETCH) &&
                (ViewPict->PictStyle & VPS_MAINTAINSCALE))
               yOrig = 0;
            else
               yOrig = max(ViewPict->PictureHeight - (lpRectOrig->bottom -
                           lpRectOrig->top), 0);
            }

         tbStretchDibBlt(hDC, lpRect->left + x, lpRect->top + y,
                         min(lpRect->right - lpRect->left, PictureWidth),
                         min(lpRect->bottom - lpRect->top, PictureHeight),
                         hDIB, xOrig, yOrig, PictureWidthOrig, PictureHeightOrig,
                         SRCCOPY);
         }

      SetMapMode(hDC, iMapModeOld);
      }

   else
      {
      if (ViewPict->PictStyle & VPS_CENTER)
         {
         x = (ViewPict->cxClient - ViewPict->PictureWidth) / 2;
         y = (ViewPict->cyClient - ViewPict->PictureHeight) / 2;
         }

      else
         {
         x = ViewPict->CurCol;
         y = ViewPict->CurLine;
         }

      BitBlt(hDC, lpRect->left + x, lpRect->top + y,
             min(ViewPict->PictureWidth, ViewPict->cxClient - x),
             min(ViewPict->PictureHeight, ViewPict->cyClient - y),
             hDCMem, 0, 0, SRCCOPY);
      }

   if (hPalOld)
      SelectPalette(hDC, hPalOld, TRUE);

   if (hDCMemory)
      {
      SelectObject(hDCMemory, hOldBitMap);
      DeleteDC(hDCMemory);
      }

   if (hDCMemory2)
      {
      SelectObject(hDCMemory2, hOldBitMap2);
      DeleteDC(hDCMemory2);
      }
   }

GlobalUnlock(hGlobal);
}


void ViewPictFree(ViewPict)

LPVIEWPICT         ViewPict;
{
LPVIEWPICT_PICTURE Pictures;
short              i;

if (ViewPict->AnimationPictureCnt)
   {
   Pictures = (LPVIEWPICT_PICTURE)GlobalLock(ViewPict->hGlobalAnimatePictures);

   for (i = 0; i < ViewPict->AnimationPictureCnt; i++)
      {
      if (Pictures[i].hPal && !Pictures[i].fNoDeletePal)
        DeleteObject(Pictures[i].hPal);

      if ((!(Pictures[i].PictStyle & VPS_HANDLE) ||
          (Pictures[i].PictStyle & VPS_ICON)) && Pictures[i].hBitMap)
         DeleteObject(Pictures[i].hBitMap);

      if (Pictures[i].hBitMap2)
         DeleteObject(Pictures[i].hBitMap2);

      _fmemset(&Pictures[i], '\0', sizeof(VIEWPICT_PICTURE));
      }

   GlobalUnlock(ViewPict->hGlobalAnimatePictures);
   GlobalFree(ViewPict->hGlobalAnimatePictures);
   GlobalFree(ViewPict->hGlobalAnimateFrames);
   }

else
   {
   if (ViewPict->Picture.hPal && !ViewPict->Picture.fNoDeletePal)
     DeleteObject(ViewPict->Picture.hPal);

   if ((!(ViewPict->Picture.PictStyle & VPS_HANDLE) ||
       (ViewPict->Picture.PictStyle & VPS_ICON)) && ViewPict->Picture.hBitMap)
      DeleteObject(ViewPict->Picture.hBitMap);

   else if ((ViewPict->Picture.PictStyle & VPS_HANDLE) &&
            (ViewPict->Picture.PictStyle & VPS_ICON) &&
            ViewPict->Picture.hBitMap)
      DeleteObject(ViewPict->Picture.hBitMap);

   if (ViewPict->Picture.hBitMap2)
      DeleteObject(ViewPict->Picture.hBitMap2);

   _fmemset(&ViewPict->Picture, '\0', sizeof(VIEWPICT_PICTURE));
   }
}


static void ViewPictSetVScrollBar(hWnd, ViewPict)

HWND       hWnd;
LPVIEWPICT ViewPict;
{
if ((ViewPict->PictStyle & VPS_SCROLL) && ViewPict->PictureHeight >
    ViewPict->cyClient)
   SetScrollPos(hWnd, SB_VERT, (ViewPict->CurLine + ViewPict->ScrollIncY - 1) /
                ViewPict->ScrollIncY, TRUE);
}


static void ViewPictSetHScrollBar(hWnd, ViewPict)

HWND       hWnd;
LPVIEWPICT ViewPict;
{
if ((ViewPict->PictStyle & VPS_SCROLL) && ViewPict->PictureWidth >
    ViewPict->cxClient)
   SetScrollPos(hWnd, SB_HORZ, (ViewPict->CurCol + ViewPict->ScrollIncX - 1) /
                ViewPict->ScrollIncX, TRUE);
}


static void ViewPictSetScrollBar(hWnd, ViewPict)

HWND       hWnd;
LPVIEWPICT ViewPict;
{
if ((ViewPict->PictStyle & VPS_SCROLL) == 0 || ViewPict->PictureWidth <=
    ViewPict->cxClient)
   {
   SetScrollRange(hWnd, SB_HORZ, 0, 0, FALSE);
   SetScrollPos(hWnd, SB_HORZ, 0, TRUE);
   }
else
   {
   SetScrollRange(hWnd, SB_HORZ, 0, (ViewPict->PictureWidth -
                  ViewPict->cxClient + ViewPict->ScrollIncX - 1) /
                  ViewPict->ScrollIncX, FALSE);
   SetScrollPos(hWnd, SB_HORZ, (ViewPict->CurCol + ViewPict->ScrollIncX - 1) /
                ViewPict->ScrollIncX, TRUE);
   }

if ((ViewPict->PictStyle & VPS_SCROLL) == 0 || ViewPict->PictureHeight <=
    ViewPict->cyClient)
   {
   SetScrollRange(hWnd, SB_VERT, 0, 0, FALSE);
   SetScrollPos(hWnd, SB_VERT, 0, TRUE);
   }
else
   {
   SetScrollRange(hWnd, SB_VERT, 0, (ViewPict->PictureHeight -
                  ViewPict->cyClient + ViewPict->ScrollIncY - 1) /
                  ViewPict->ScrollIncY, FALSE);
   SetScrollPos(hWnd, SB_VERT, (ViewPict->CurLine + ViewPict->ScrollIncY - 1) /
                ViewPict->ScrollIncY, TRUE);
   }
}


static short MaxLine(ViewPict)

LPVIEWPICT ViewPict;
{
short      mLine;

if (ViewPict->PictureHeight <= ViewPict->cyClient)
   return (0);

mLine = ViewPict->ScrollHeight - (ViewPict->cyClient / ViewPict->ScrollIncY);

return (max(mLine, 0));
}


static short MaxCol(ViewPict)

LPVIEWPICT ViewPict;
{
short      mCol;

if (ViewPict->PictureWidth <= ViewPict->cxClient)
   return (0);

mCol = ViewPict->ScrollWidth - (ViewPict->cxClient / ViewPict->ScrollIncX);

return (max(mCol, 0));
}


static short ViewPictScrollWidth(ViewPict)

LPVIEWPICT ViewPict;
{
return (max(0, ((short)((ViewPict->cxClient - VIEWPICT_OFFSET_LEFT) /
            ViewPict->ScrollIncX))));
}


static short ViewPictScrollHeight(ViewPict)

LPVIEWPICT ViewPict;
{
return (max(0, ((ViewPict->cyClient / ViewPict->ScrollIncY))));
}


static void ViewPictSetPtr(hWnd, hGlobal)

HWND         hWnd;
GLOBALHANDLE hGlobal;
{
//#if _WIN64
//SetWindowLongPtr(hWnd, 0, (LONG_PTR)hGlobal);
//#else
SetWindowLong(hWnd, 0, (long)hGlobal);
//#endif
}


LPVIEWPICT ViewPictGetPtr(hWnd, hGlobal)

HWND              hWnd;
GLOBALHANDLE FAR *hGlobal;
{
//#if _WIN64
//*hGlobal = (GLOBALHANDLE)GetWindowLongPtr(hWnd, 0);
//#else
*hGlobal = (GLOBALHANDLE)GetWindowLong(hWnd, 0);
//#endif
return ((LPVIEWPICT)GlobalLock(*hGlobal));
}


static short ViewPictSetPicture(hWnd, ViewPict, Picture, FileName,
                                SetPictureSize)

HWND               hWnd;
LPVIEWPICT         ViewPict;
LPVIEWPICT_PICTURE Picture;
LPCTSTR            FileName;
BOOL               SetPictureSize;
{
HCURSOR            hCursor;
HBITMAP            hBitmapOld;
HANDLE             hResource;
HANDLE             hFindResource;
HDC                hDC;
HDC                hDCMemory;
LPBYTE             ResourceData = NULL;
LPCTSTR            Ptr;
RECT               Rect;
long               ResourceDataSize = 0;
short              PictType;
short              Ret = 0;

if (!FileName)
   return (VIEWPICT_ERR_NOTFOUND);

if (!(Picture->PictStyle & VPS_HANDLE) && !FileName[0])
   return (VIEWPICT_ERR_NOTFOUND);

if (Picture->PictStyle & VPS_BMP)
   PictType = VPS_BMP;

else if (Picture->PictStyle & VPS_ICON)
   PictType = VPS_ICON;

else if (Picture->PictStyle & VPS_PCX)
   PictType = VPS_PCX;

else if (Picture->PictStyle & VPS_GIF)
   PictType = VPS_GIF;

else
   {
   for (Ptr = &FileName[lstrlen(FileName) - 1]; Ptr >= FileName &&
        Ptr >= FileName + lstrlen(FileName) - 4 && *Ptr != '\\' &&
        *Ptr != '/' && *Ptr != '.'; Ptr--)
      ;

   if (Ptr >= FileName && *Ptr == '.')
      {
      if (lstrcmpi(Ptr + 1, _T("bmp")) == 0)
         PictType = VPS_BMP;

      else if (lstrcmpi(Ptr + 1, _T("pcx")) == 0)
         PictType = VPS_PCX;

      else if (lstrcmpi(Ptr + 1, _T("gif")) == 0)
         PictType = VPS_GIF;

      else
         return (VIEWPICT_ERR_NOTFOUND);
      }

   else
      return (VIEWPICT_ERR_NOTFOUND);
   }

hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
ShowCursor(TRUE);

hDC = fpGetDC(hWnd);

if (PictType != VPS_HANDLE && (Picture->PictStyle & VPS_RESOURCE))
   {
   if (!(hFindResource = FindResource(GetWindowInstance(hWnd),
                                      FileName, _T("VIEWPICT"))))
      Ret = VIEWPICT_ERR_NOTFOUND;

   else if (!(hResource = LoadResource(GetWindowInstance(hWnd),
                                       hFindResource)))
      Ret = VIEWPICT_ERR_NOTFOUND;

   else
      {
      ResourceDataSize = SizeofResource(GetWindowInstance(hWnd),
                                        hFindResource);
      ResourceData = LockResource(hResource);
      }
   }

if (Ret == 0)
   {
   switch (PictType)
      {
      case VPS_ICON:
         if (Picture->PictStyle & VPS_HANDLE)
            {
            Picture->bi.biWidth = tbGetIconWidth((HICON)(LONG)FileName);
            Picture->bi.biHeight = tbGetIconHeight((HICON)(LONG)FileName);

            Picture->hBitMap = CreateCompatibleBitmap(hDC,
                                                    (int)Picture->bi.biWidth,
                                                    (int)Picture->bi.biHeight);

            hDCMemory = CreateCompatibleDC(hDC);
            hBitmapOld = SelectObject(hDCMemory, Picture->hBitMap);

            SetRect(&Rect, 0, 0, (int)Picture->bi.biWidth,
                    (int)Picture->bi.biHeight);

#ifdef WIN32
            FORWARD_WM_CTLCOLORSTATIC(GetParent(hWnd), hDC, hWnd,
                                      SendMessage);
#else
            FORWARD_WM_CTLCOLOR(GetParent(hWnd), hDC, hWnd,
                                CTLCOLOR_STATIC, SendMessage);
#endif
            tbDrawIcon(hDCMemory, &Rect, GetBkColor(hDC), 0, 0,
                       (HICON)(LONG)FileName);

            SelectObject(hDCMemory, hBitmapOld);
            DeleteDC(hDCMemory);

            ViewGetBitMapInfoHeader(Picture);
            }

         break;

      case VPS_GIF:
         Ret = ViewGifLoadImage(hWnd, Picture, FileName, ResourceData,
                                ResourceDataSize);
         break;

      case VPS_PCX:
         Ret = ViewPcxLoadImage(hWnd, Picture, FileName, ResourceData,
                                ResourceDataSize);
         break;

      case VPS_BMP:
         if (Picture->PictStyle & VPS_HANDLE)
            {
            Picture->hBitMap = (HBITMAP)(LONG)FileName;
            ViewGetBitMapInfoHeader(Picture);
            }

         else
            Ret = ViewBmpLoadImage(hWnd, Picture, FileName, ResourceData,
                                   ResourceDataSize);

         break;
      }

   if (PictType != VPS_HANDLE && (Picture->PictStyle & VPS_RESOURCE))
      {
      UnlockResource(hResource);
      FreeResource(hResource);
      }

   if (Ret == 0)
      {
      if (ViewPict->PictStyle & VPS_STRETCH)
         {
         if (!ViewPictCreateBitmap2(hDC, ViewPict, &ViewPict->Picture))
            Ret = VIEWPICT_ERR_MEMORY;
         }

      else if (SetPictureSize)
         {
         ViewPict->PictureWidth = (short)Picture->bi.biWidth;
         ViewPict->PictureHeight = (short)Picture->bi.biHeight;
         }
      }
   }

ShowCursor(FALSE);
SetCursor(hCursor);

ReleaseDC(hWnd, hDC);

ViewPict->OldcxClient = 0;
ViewPict->OldcyClient = 0;

return (Ret);
}

static void ViewGetBitMapInfoHeader(Picture)

LPVIEWPICT_PICTURE Picture;
{
BITMAP             Bitmap;
DWORD dwCount;

GetObject(Picture->hBitMap, sizeof(BITMAP), (LPVOID)&Bitmap);

Picture->bi.biSize = sizeof(BITMAPINFOHEADER);
Picture->bi.biWidth = Bitmap.bmWidth;
Picture->bi.biHeight = Bitmap.bmHeight;
Picture->bi.biPlanes = 1;
Picture->bi.biBitCount = Bitmap.bmPlanes * Bitmap.bmBitsPixel;
Picture->bi.biCompression = BI_RGB;
Picture->bi.biSizeImage =
   WIDTHBYTES(Picture->bi.biWidth * Picture->bi.biBitCount) *
              Picture->bi.biHeight;
Picture->bi.biXPelsPerMeter = 0L;
Picture->bi.biYPelsPerMeter = 0L;
Picture->bi.biClrImportant = (DWORD)0;
// bug in MSVC++ 1.52
// This was replaced with the for loop below due to an "internal compiler error" 
//Picture->bi.biClrUsed = (short)pow((double)2,
//                        (double)Picture->bi.biBitCount *
//                        (double)Picture->bi.biPlanes);

Picture->bi.biClrUsed = 1;
for (dwCount=0; dwCount<(Picture->bi.biBitCount * Picture->bi.biPlanes) ;dwCount++)
	Picture->bi.biClrUsed *= 2;
}


static short ViewBmpLoadImage(hWnd, Picture, FileName, BmpResourceData,
                              BmpResourceDataSize)

HWND               hWnd;
LPVIEWPICT_PICTURE Picture;
LPCTSTR            FileName;
LPBYTE             BmpResourceData;
long               BmpResourceDataSize;
{
BITMAPINFOHEADER   bi;
GLOBALHANDLE       hDib;
short              Ret = 0;

if (!(hDib = tbOpenDIB(FileName, BmpResourceData, BmpResourceDataSize)))
   Ret = VIEWPICT_ERR_NOTFOUND;

else
   {
   Picture->hPal = tbCreateDibPalette(hDib);

   if (!(Picture->hBitMap = tbBitmapFromDib(hDib, Picture->hPal, TRUE)))
      Ret = VIEWPICT_ERR_NOTFOUND;

   else
      {
      tbDibInfo(hDib, &bi);
      _fmemcpy(&Picture->bi, &bi, sizeof(BITMAPINFOHEADER));
      }

   GlobalFree(hDib);
   }

return (Ret);
}


static BOOL ViewPictAddPicture(hWnd, ViewPict, AnimatePict)

HWND                   hWnd;
LPVIEWPICT             ViewPict;
LPVIEWPICT_ANIMATEPICT AnimatePict;
{
LPVIEWPICT_PICTURE     Pictures;
short                  Index;
short                  Ret;

Index = ViewPict->AnimationPictureCnt;

if (ViewPict->AnimationPictureCnt == 0)
   {
   if (!(ViewPict->hGlobalAnimatePictures = GlobalAlloc(GMEM_MOVEABLE |
         GMEM_ZEROINIT, (long)(sizeof(VIEWPICT_PICTURE) *
         (ViewPict->AnimationPictureCnt + 1)))))
      return (FALSE);
   }

else
   {
   if (!(ViewPict->hGlobalAnimatePictures =
         GlobalReAlloc(ViewPict->hGlobalAnimatePictures,
         (long)(sizeof(VIEWPICT_PICTURE) * (ViewPict->AnimationPictureCnt +
         1)), GMEM_MOVEABLE | GMEM_ZEROINIT)))
      return (FALSE);
   }

Pictures = (LPVIEWPICT_PICTURE)GlobalLock(ViewPict->hGlobalAnimatePictures);

_fmemset(&Pictures[Index], '\0', sizeof(VIEWPICT_PICTURE));

lstrcpy(Pictures[Index].PictName, AnimatePict->PictName);
Pictures[Index].PictStyle = AnimatePict->PictStyle;

Ret = ViewPictSetPicture(hWnd, ViewPict, &Pictures[Index],
                         Pictures[Index].PictName,
                         ViewPict->AnimationPictureCnt == 0 ? TRUE : FALSE);

GlobalUnlock(ViewPict->hGlobalAnimatePictures);

ViewPict->AnimationPictureCnt++;
return (Ret);
}


static BOOL ViewPictInsertFrame(hWnd, ViewPict, AnimatePict, Index)

HWND                   hWnd;
LPVIEWPICT             ViewPict;
LPVIEWPICT_ANIMATEPICT AnimatePict;
short                  Index;
{
LPVIEWPICT_PICTURE     Pictures;
LPVIEWPICT_FRAME       Frames;
short                  i;

if (Index == VIEWPICT_END)
   Index = ViewPict->AnimationFrameCnt;

if (ViewPict->AnimationFrameCnt == 0)
   {
   if (!(ViewPict->hGlobalAnimateFrames = GlobalAlloc(GMEM_MOVEABLE |
         GMEM_ZEROINIT, (long)(sizeof(VIEWPICT_FRAME) *
         (ViewPict->AnimationFrameCnt + 1)))))
      return (FALSE);
   }

else
   {
   if (!(ViewPict->hGlobalAnimateFrames =
         GlobalReAlloc(ViewPict->hGlobalAnimateFrames,
         (long)(sizeof(VIEWPICT_PICTURE) * (ViewPict->AnimationPictureCnt +
         1)), GMEM_MOVEABLE | GMEM_ZEROINIT)))
      return (FALSE);
   }

Frames = (LPVIEWPICT_FRAME)GlobalLock(ViewPict->hGlobalAnimateFrames);

for (i = ViewPict->AnimationFrameCnt; i > Index; i--)
   _fmemcpy(&Frames[i], &Frames[i - 1], sizeof(VIEWPICT_FRAME));

_fmemset(&Frames[Index], '\0', sizeof(VIEWPICT_FRAME));

if (ViewPict->hGlobalAnimatePictures)
   {
   Pictures = (LPVIEWPICT_PICTURE)GlobalLock(ViewPict->hGlobalAnimatePictures);

   for (; Frames[Index].PictureIndex < ViewPict->AnimationPictureCnt;
        Frames[Index].PictureIndex++)
      if (lstrcmpi(Pictures[Frames[Index].PictureIndex].PictName,
                   AnimatePict->PictName) == 0)
         break;

   GlobalUnlock(ViewPict->hGlobalAnimatePictures);
   }

if (Frames[Index].PictureIndex == ViewPict->AnimationPictureCnt)
   ViewPictAddPicture(hWnd, ViewPict, AnimatePict);

if (AnimatePict->PictStyle & VPS_SHOWFRAMEONCE)
   Frames[Index].ShowFrameOnce = 1;

Frames[Index].x = AnimatePict->x;
Frames[Index].y = AnimatePict->y;
Frames[Index].TimeDelay = AnimatePict->TimeDelay;

GlobalUnlock(ViewPict->hGlobalAnimateFrames);

ViewPict->AnimationFrameCnt++;
return (TRUE);
}


#if 0
static BOOL ViewPictDeleteFrame(ViewPict, Index)

LPVIEWPICT         ViewPict;
short              Index;
{
LPVIEWPICT_PICTURE Pictures;
short              i;

if (ViewPict->AnimateHdr.PictureCnt > 0)
   {
   Pictures = (LPVIEWPICT_PICTURE)GlobalLock(ViewPict->hGlobalAnimatePict);

   if (Pictures[Index].hPal && !Pictures[Index].fNoDeletePal)
     DeleteObject(Pictures[Index].hPal);

   if (Pictures[Index].hBitMap)
      DeleteObject(Pictures[Index].hBitMap);

   if (Pictures[Index].hBitMap2)
      DeleteObject(Pictures[Index].hBitMap2);

   GlobalUnlock(ViewPict->hGlobalAnimatePict);

   if (ViewPict->AnimateHdr.PictureCnt == 1)
      {
      GlobalFree(ViewPict->hGlobalAnimatePict);
      ViewPict->hGlobalAnimatePict = NULL;
      ViewPict->AnimateHdr.PictureCnt = 0;
      }

   else
      {
      Pictures = (LPVIEWPICT_PICTURE)GlobalLock(ViewPict->hGlobalAnimatePict);

      for (i = Index; i < ViewPict->AnimateHdr.PictureCnt - 1; i++)
         _fmemcpy(&Pictures[i], &Pictures[i + 1], sizeof(VIEWPICT_PICTURE));

      GlobalUnlock(ViewPict->hGlobalAnimatePict);
      ViewPict->AnimateHdr.PictureCnt--;

      if (!(ViewPict->hGlobalAnimatePict =
            GlobalReAlloc(ViewPict->hGlobalAnimatePict, sizeof(VIEWPICT_PICTURE) *
            ViewPict->AnimateHdr.PictureCnt, GMEM_MOVEABLE | GMEM_ZEROINIT)))
         return (FALSE);
      }
   }

return (TRUE);
}
#endif


static BOOL ViewPictShowCurFrame(hWnd, ViewPict)

HWND             hWnd;
LPVIEWPICT       ViewPict;
{
LPVIEWPICT_FRAME Frames;
short            TimeDelay;

ViewPict->fNoErase = TRUE;
InvalidateRect(hWnd, NULL, TRUE);
UpdateWindow(hWnd);
ViewPict->fNoErase = FALSE;

Frames = (LPVIEWPICT_FRAME)GlobalLock(ViewPict->hGlobalAnimateFrames);

if (Frames[ViewPict->AnimationCurFrame].ShowFrameOnce)
   Frames[ViewPict->AnimationCurFrame].FrameDeleted = TRUE;

if (!ViewPict->AnimationFrozen)
   {
   if (Frames[ViewPict->AnimationCurFrame].TimeDelay == VIEWPICT_DEFDELAY)
      TimeDelay = ViewPict->AnimationTimeDelay;
   else
      TimeDelay = Frames[ViewPict->AnimationCurFrame].TimeDelay;

   if (TimeDelay != ViewPict->AnimationCurTimeDelay)
      {
      ViewPict->AnimationCurTimeDelay = TimeDelay;
      KillTimer(hWnd, 1);
      SetTimer(hWnd, 1, ViewPict->AnimationCurTimeDelay, NULL);
      }
   }

GlobalUnlock(ViewPict->hGlobalAnimateFrames);
return (TRUE);
}


static BOOL ViewPictGetCurFrame(ViewPict, Picture, Frame)

LPVIEWPICT         ViewPict;
LPVIEWPICT_PICTURE Picture;
LPVIEWPICT_FRAME   Frame;
{
LPVIEWPICT_PICTURE Pictures;
LPVIEWPICT_FRAME   Frames;

Frames = (LPVIEWPICT_FRAME)GlobalLock(ViewPict->hGlobalAnimateFrames);
Pictures = (LPVIEWPICT_PICTURE)GlobalLock(ViewPict->hGlobalAnimatePictures);

_fmemcpy(Frame, &Frames[ViewPict->AnimationCurFrame], sizeof(VIEWPICT_FRAME));
_fmemcpy(Picture, &Pictures[Frames[ViewPict->AnimationCurFrame].PictureIndex],
       sizeof(VIEWPICT_PICTURE));

GlobalUnlock(ViewPict->hGlobalAnimatePictures);
GlobalUnlock(ViewPict->hGlobalAnimateFrames);
return (TRUE);
}


static LRESULT tbSendMessageToParent(hWnd, Msg, wParam, lParam)

HWND   hWnd;
UINT   Msg;
WPARAM wParam;
LPARAM lParam;
{
SendMessage(hWnd, Msg, wParam, lParam);
return (SendMessage(GetParent(hWnd), Msg, wParam, lParam));
}


static BOOL ViewPictCreateBitmap2(HDC hDC, LPVIEWPICT lpViewPict,
                           LPVIEWPICT_PICTURE lpPicture)
{
BITMAPINFOHEADER bi2;
short            Height;
BOOL             fRet = TRUE;

if (lpPicture->hBitMap2)
   DeleteObject(lpPicture->hBitMap2);

_fmemcpy(&bi2, &lpPicture->bi, sizeof(BITMAPINFOHEADER));

if (lpViewPict->PictStyle & VPS_MAINTAINSCALE)
   {
   Height = (short)((lpViewPict->cxClient * lpPicture->bi.biHeight) /
            lpPicture->bi.biWidth);

   if (Height <= lpViewPict->cyClient)
      {
      bi2.biWidth = lpViewPict->cxClient;
      bi2.biHeight = Height;
      }

   else
      {
      bi2.biWidth = (lpViewPict->cyClient * lpPicture->bi.biWidth) /
                    lpPicture->bi.biHeight;
      bi2.biHeight = lpViewPict->cyClient;
      }
   }
else
   {
   bi2.biWidth = lpViewPict->cxClient;
   bi2.biHeight = lpViewPict->cyClient;
   }

bi2.biSizeImage = 0;

lpViewPict->PictureWidth = (short)bi2.biWidth;
lpViewPict->PictureHeight = (short)bi2.biHeight;

if (!(lpPicture->hBitMap2 = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)&bi2,
                                           0L, NULL, NULL, 0)))
   fRet = FALSE;
return (fRet);
}
