/*********************************************
* pvscroll.c
*
* Copyright (c) 1998 - FarPoint Technologies
* All rights reserved.
*********************************************/

#include <windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fptools.h"
#include "ssprvw.h"


void SSPreviewSetScrollBars(LPSSPREVIEW lpSSPreview)
{
RECT  RectClient;
RECT  RectPage;
short nMarginH;
short nMarginV;
BOOL  fScrollBarSet;
SCROLLINFO si;

si.cbSize = sizeof(si);

GetClientRect(lpSSPreview->hWnd, &RectClient);

SSPreviewGetMargins(lpSSPreview, &nMarginH, &nMarginV);
SSPreviewGetPageSize(lpSSPreview, &RectPage, nMarginH, nMarginV);

/***********************
* Horizontal ScrollBar
***********************/

fScrollBarSet = FALSE;

if (lpSSPreview->wScrollBarH != SSPRVW_SCROLLBAR_HIDE)
   {
   if ((lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_NORMALSIZE ||
       lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE ||
       lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PAGEHEIGHT) &&
       (RectPage.right + (2 * (nMarginH + lpSSPreview->nPageBorderWidth)) +
       lpSSPreview->nPageShadowWidth > RectClient.right))
      {
      short nWidth;
      short nScrollInc;

      nWidth = (short)(RectPage.right + (2 * nMarginH) + (2 *
               lpSSPreview->nPageBorderWidth) + lpSSPreview->nPageShadowWidth);

      nScrollInc = max(TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lScrollIncH), 1);
      lpSSPreview->lScrollMaxH = DIVUP(nWidth - RectClient.right, nScrollInc);
		if (lpSSPreview->lScrollPosH)
			InvalidateRect(lpSSPreview->hWnd, NULL, TRUE);
      lpSSPreview->lScrollPosH = 0;

      ShowScrollBar(lpSSPreview->hWnd, SB_HORZ, TRUE);
		/*
      SetScrollRange(lpSSPreview->hWnd, SB_HORZ, 0,
                     (short)lpSSPreview->lScrollMaxH, FALSE);
      SetScrollPos(lpSSPreview->hWnd, SB_HORZ, (short)lpSSPreview->lScrollPosH,
                   TRUE);
		*/

      si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nMin = 0;
		si.nMax = lpSSPreview->lScrollMaxH;
		si.nPos = lpSSPreview->lScrollPosH;
      si.nPage = 1;
      SetScrollInfo(lpSSPreview->hWnd, SB_HORZ, &si, TRUE);

      fScrollBarSet = TRUE;
      }
   }

if (!fScrollBarSet)
   {

	/* RFW - 20149 - 8/17/07
   SetScrollRange(lpSSPreview->hWnd, SB_HORZ, 0, 0, FALSE);
	SetScrollPos(lpSSPreview->hWnd, SB_HORZ, 0, TRUE);
	*/

   si.fMask = SIF_DISABLENOSCROLL | SIF_POS | SIF_RANGE | SIF_PAGE;
	si.nMin = 0;
	si.nMax = 0;
	si.nPos = 0;
   si.nPage = 0;
   SetScrollInfo(lpSSPreview->hWnd, SB_HORZ, &si, FALSE);

   if (lpSSPreview->wScrollBarH == SSPRVW_SCROLLBAR_SHOW)
      ShowScrollBar(lpSSPreview->hWnd, SB_HORZ, TRUE);
   else
      ShowScrollBar(lpSSPreview->hWnd, SB_HORZ, FALSE);

   lpSSPreview->lScrollMaxH = 0;
   lpSSPreview->lScrollPosH = 0;
   }

/*********************
* Vertical ScrollBar
*********************/

fScrollBarSet = FALSE;

//if (lpSSPreview->wScrollBarV != SSPRVW_SCROLLBAR_HIDE)
   {
   short nPagesPerScreen = SSPreviewGetPagesPerScreen(lpSSPreview);
	short nMin;

   if ((lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_NORMALSIZE ||
       lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE ||
       lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PAGEWIDTH) &&
       (RectPage.bottom + nMarginV + (2 * lpSSPreview->nPageBorderWidth) +
       lpSSPreview->nPageShadowWidth > RectClient.bottom))
      {
      short nHeight;
      short nScrollInc;

      nHeight = (short)(RectPage.bottom + (2 * nMarginV) + (2 *
                lpSSPreview->nPageBorderWidth) + lpSSPreview->nPageShadowWidth);

      nScrollInc = max(TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lScrollIncV), 1);
      lpSSPreview->lScrollMaxV = DIVUP(nHeight - RectClient.bottom, nScrollInc);
      lpSSPreview->lScrollPosV = 0;

      if (lpSSPreview->wScrollBarV != SSPRVW_SCROLLBAR_HIDE)
         {
			nMin = 0;
         fScrollBarSet = TRUE;
         }
      }

   else if (PAGECNT(lpSSPreview) > nPagesPerScreen)
      {
      lpSSPreview->lScrollMaxV = DIVUP(PAGECNT(lpSSPreview), nPagesPerScreen);
      lpSSPreview->lScrollPosV = DIVUP(lpSSPreview->lPageCurrent - lpSSPreview->lPageBeg + 1,
                                       nPagesPerScreen);

      if (lpSSPreview->wScrollBarV != SSPRVW_SCROLLBAR_HIDE)
         {
			nMin = 1;
         fScrollBarSet = TRUE;
         }
      }

   if (fScrollBarSet)
      {
      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, TRUE);

      si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nMin = nMin;
		si.nMax = lpSSPreview->lScrollMaxV;
		si.nPos = lpSSPreview->lScrollPosV;
      si.nPage = 1;

      SetScrollInfo(lpSSPreview->hWnd, SB_VERT, &si, TRUE);
      }
   }

if (!fScrollBarSet)
   {
	/* RFW - 20149 - 8/17/07
   SetScrollRange(lpSSPreview->hWnd, SB_VERT, 0, 0, FALSE);
   SetScrollPos(lpSSPreview->hWnd, SB_VERT, 0, TRUE);
	*/
   SCROLLINFO si;

   si.cbSize = sizeof(si);
   si.fMask = SIF_DISABLENOSCROLL | SIF_POS | SIF_RANGE;
	si.nMin = 0;
	si.nMax = 0;
	si.nPos = 0;
   SetScrollInfo(lpSSPreview->hWnd, SB_VERT, &si, FALSE);

   if (lpSSPreview->wScrollBarV == SSPRVW_SCROLLBAR_SHOW)
      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, TRUE);
   else
      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, FALSE);

	// RFW - 8/25/03 - 12375
	if (lpSSPreview->wScrollBarV != SSPRVW_SCROLLBAR_HIDE)
		{
		// RFW - 3/18/00 These were uncommented to fix bug GIC11140
		lpSSPreview->lScrollMaxV = 0;
		lpSSPreview->lScrollPosV = 0;
		}
   }

#if 0
if (lpSSPreview->wScrollBarV != SSPRVW_SCROLLBAR_HIDE)
   {
   short nPagesPerScreen = SSPreviewGetPagesPerScreen(lpSSPreview);

   if ((lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_NORMALSIZE ||
       lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE ||
       lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PAGEWIDTH) &&
       (RectPage.bottom + nMarginV + (2 * lpSSPreview->nPageBorderWidth) +
       lpSSPreview->nPageShadowWidth > RectClient.bottom))
      {
      short nHeight;
      short nScrollInc;

      nHeight = RectPage.bottom + (2 * nMarginV) + (2 *
                lpSSPreview->nPageBorderWidth) + lpSSPreview->nPageShadowWidth;

      nScrollInc = max(TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lScrollIncV), 1);
      lpSSPreview->lScrollMaxV = DIVUP(nHeight - RectClient.bottom, nScrollInc);
      lpSSPreview->lScrollPosV = 0;

      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, TRUE);
      SetScrollRange(lpSSPreview->hWnd, SB_VERT, 0,
                     (short)lpSSPreview->lScrollMaxV, FALSE);
      fScrollBarSet = TRUE;
      }

   else if (PAGECNT(lpSSPreview) > nPagesPerScreen)
      {
      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, TRUE);
      lpSSPreview->lScrollMaxV = DIVUP(PAGECNT(lpSSPreview), nPagesPerScreen);
      lpSSPreview->lScrollPosV = DIVUP(lpSSPreview->lPageCurrent,
                                       nPagesPerScreen);

      SetScrollRange(lpSSPreview->hWnd, SB_VERT, 1,
                     (short)lpSSPreview->lScrollMaxV, FALSE);
      fScrollBarSet = TRUE;
      }

   if (fScrollBarSet)
      {
#ifdef WIN32
      SCROLLINFO si;

      si.cbSize = sizeof(si);
      si.fMask = SIF_PAGE;
      si.nPage = 1;
      SetScrollInfo(lpSSPreview->hWnd, SB_VERT, &si, FALSE);
#endif

      SetScrollPos(lpSSPreview->hWnd, SB_VERT, (short)lpSSPreview->lScrollPosV,
                   TRUE);
      }
   }

if (!fScrollBarSet)
   {
   SetScrollRange(lpSSPreview->hWnd, SB_VERT, 0, 0, FALSE);
   SetScrollPos(lpSSPreview->hWnd, SB_VERT, 0, TRUE);

   if (lpSSPreview->wScrollBarV == SSPRVW_SCROLLBAR_SHOW)
      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, TRUE);
   else
      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, FALSE);

   lpSSPreview->lScrollMaxV = 0;
   lpSSPreview->lScrollPosV = 0;
   }
#endif
}


void SSPreviewHScroll(LPSSPREVIEW lpSSPreview, WORD wParam, LONG lParam)
{
RECT  RectClient;
long  lScrollAmtH;
short nScrollInc;

GetClientRect(lpSSPreview->hWnd, &RectClient);

nScrollInc = max(TWIPSTOPIXELSX(lpSSPreview, lpSSPreview->lScrollIncH), 1);

switch (wParam)
   {
   case SB_LINEDOWN:
      lScrollAmtH = 1;
      break;

   case SB_LINEUP:
      lScrollAmtH = -1;
      break;

   case SB_BOTTOM:
      lScrollAmtH = lpSSPreview->lScrollMaxH - lpSSPreview->lScrollPosH;
      break;

   case SB_TOP:
      lScrollAmtH = -lpSSPreview->lScrollPosH;
      break;

   case SB_PAGEUP:
      lScrollAmtH = min(-1, -(RectClient.right / nScrollInc));
      break;

   case SB_PAGEDOWN:
      lScrollAmtH = max(1, RectClient.right / nScrollInc);
      break;

   case SB_THUMBPOSITION:
      lScrollAmtH = LOWORD(lParam) - lpSSPreview->lScrollPosH;
      break;

   default:
      lScrollAmtH = 0;
   } // case

if (lScrollAmtH = max(-lpSSPreview->lScrollPosH, min(lScrollAmtH,
    lpSSPreview->lScrollMaxH - lpSSPreview->lScrollPosH)))
   {
   long lPixH;

   lpSSPreview->lScrollPosH += lScrollAmtH;
   lPixH = -(long)nScrollInc * lScrollAmtH;

   if (labs(lPixH) > (long)RectClient.right)
      InvalidateRect(lpSSPreview->hWnd, &RectClient, TRUE);
   else
      ScrollWindow(lpSSPreview->hWnd, (short)lPixH, 0, &RectClient,
                   &RectClient);

   SetScrollPos(lpSSPreview->hWnd, SB_HORZ, (short)lpSSPreview->lScrollPosH,
                TRUE);
   UpdateWindow(lpSSPreview->hWnd);        // send a WM_PAINT message
   }
}


void SSPreviewVScroll(LPSSPREVIEW lpSSPreview, WORD wParam, LONG lParam)
{
RECT  RectClient;
RECT  RectPage;
long  lScrollAmtV;
short nPagesPerScreen;
short nScrollInc;
short nMarginH;
short nMarginV;
BOOL  fScrollPartial = FALSE;

GetClientRect(lpSSPreview->hWnd, &RectClient);

SSPreviewGetMargins(lpSSPreview, &nMarginH, &nMarginV);
SSPreviewGetPageSize(lpSSPreview, &RectPage, nMarginH, nMarginV);

nPagesPerScreen = SSPreviewGetPagesPerScreen(lpSSPreview);

if ((lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_NORMALSIZE ||
    lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE ||
    lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PAGEWIDTH) &&
    (RectPage.bottom + nMarginV + (2 * lpSSPreview->nPageBorderWidth) +
    lpSSPreview->nPageShadowWidth > RectClient.bottom))
   fScrollPartial = TRUE;

nScrollInc = max(TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lScrollIncV), 1);

switch (wParam)
   {
   case SB_LINEDOWN:
      lScrollAmtV = 1;
      break;

   case SB_LINEUP:
      lScrollAmtV = -1;
      break;

   case SB_BOTTOM:
      lScrollAmtV = lpSSPreview->lScrollMaxV - lpSSPreview->lScrollPosV;
      break;

   case SB_TOP:
      lScrollAmtV = -lpSSPreview->lScrollPosV + 1;
      break;

   case SB_PAGEUP:
      if (fScrollPartial)
         lScrollAmtV = min(-1, -(RectClient.bottom / nScrollInc));
      else
         lScrollAmtV = -1;

      break;

   case SB_PAGEDOWN:
      if (fScrollPartial)
         lScrollAmtV = max(1, RectClient.bottom / nScrollInc);
      else
         lScrollAmtV = 1;

      break;

   case SB_THUMBPOSITION:
      lScrollAmtV = LOWORD(lParam) - lpSSPreview->lScrollPosV;
      break;

   default:
      lScrollAmtV = 0;
   } // case

if ((lScrollAmtV = max(-lpSSPreview->lScrollPosV, min(lScrollAmtV,
    lpSSPreview->lScrollMaxV - lpSSPreview->lScrollPosV))) &&
    lpSSPreview->lScrollPosV + lScrollAmtV >= (fScrollPartial ? 0 : 1) &&
    lpSSPreview->lScrollPosV + lScrollAmtV <= lpSSPreview->lScrollMaxV)
   {
   long lPixV;

   lpSSPreview->lScrollPosV += lScrollAmtV;

   if (fScrollPartial)
      {
      lPixV = -(long)nScrollInc * lScrollAmtV;

      if (labs(lPixV) > (long)RectClient.bottom)
         InvalidateRect(lpSSPreview->hWnd, &RectClient, TRUE);
      else
         ScrollWindow(lpSSPreview->hWnd, 0, (short)lPixV, &RectClient,
                      &RectClient);
      }
   else
      {
      SSPreviewSetPageCurrent(lpSSPreview, ((lpSSPreview->lScrollPosV - 1) *
                              SSPreviewGetPagesPerScreen(lpSSPreview)) + lpSSPreview->lPageBeg);
      InvalidateRect(lpSSPreview->hWnd, &RectClient, TRUE);
      }

   SetScrollPos(lpSSPreview->hWnd, SB_VERT, (short)lpSSPreview->lScrollPosV,
                TRUE);
   UpdateWindow(lpSSPreview->hWnd);        // send a WM_PAINT message
   }

// If a partial page is displayed and the scroll bar is at the top
// and the up arrow is pressed then go to the previous page and
// visa versa for the down arrow.

else if (fScrollPartial)
   {
   if (wParam == SB_LINEDOWN)
      PV_SetProp(lpSSPreview, PVPROP_PAGECURRENT, lpSSPreview->lPageCurrent + 1);

   else if (wParam == SB_LINEUP)
      PV_SetProp(lpSSPreview, PVPROP_PAGECURRENT, lpSSPreview->lPageCurrent - 1);
   }

#if 0
RECT  RectClient;
RECT  RectPage;
short nMarginH;
short nMarginV;
BOOL  fScrollBarSet;

GetClientRect(lpSSPreview->hWnd, &RectClient);

SSPreviewGetMargins(lpSSPreview, &nMarginH, &nMarginV);
SSPreviewGetPageSize(lpSSPreview, &RectPage, nMarginH, nMarginV);

/*********************
* Vertical ScrollBar
*********************/

fScrollBarSet = FALSE;

if (lpSSPreview->wScrollBars == SSPRVW_SCROLLBARS_VERT ||
    lpSSPreview->wScrollBars == SSPRVW_SCROLLBARS_BOTH)
   {
   short nPagesPerScreen = SSPreviewGetPagesPerScreen(lpSSPreview);

   if ((lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_NORMALSIZE ||
       lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE ||
       lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PAGEWIDTH) &&
       (RectPage.bottom + nMarginV + (2 * lpSSPreview->nPageBorderWidth) +
       lpSSPreview->nPageShadowWidth > RectClient.bottom))
      {
      short nHeight;
      short nScrollInc;

      nHeight = RectPage.bottom + (2 * nMarginV) + (2 *
                lpSSPreview->nPageBorderWidth) + lpSSPreview->nPageShadowWidth;

      nScrollInc = TWIPSTOPIXELSY(lpSSPreview, lpSSPreview->lScrollIncV);
      lpSSPreview->lScrollMaxV = DIVUP(nHeight - RectClient.bottom, nScrollInc);

      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, TRUE);
      SetScrollRange(lpSSPreview->hWnd, SB_VERT, 0,
                     (short)lpSSPreview->lScrollMaxV, TRUE);
      fScrollBarSet = TRUE;
      }

   else if (PAGECNT(lpSSPreview) > nPagesPerScreen)
      {
      ShowScrollBar(lpSSPreview->hWnd, SB_VERT, TRUE);
      lpSSPreview->lScrollMaxV = DIVUP(PAGECNT(lpSSPreview), nPagesPerScreen);
      SetScrollRange(lpSSPreview->hWnd, SB_VERT, 1,
                     (short)lpSSPreview->lScrollMaxV, TRUE);
      fScrollBarSet = TRUE;
      }
   }

if (!fScrollBarSet)
   {
   ShowScrollBar(lpSSPreview->hWnd, SB_VERT, FALSE);
   lpSSPreview->lScrollMaxV = 0;
   }
#endif
}


BOOL SSPreviewIsScrollPartial(LPSSPREVIEW lpSSPreview)
{
RECT  RectClient;
RECT  RectPage;
short nMarginH;
short nMarginV;
BOOL  fScrollPartial = FALSE;

GetClientRect(lpSSPreview->hWnd, &RectClient);

SSPreviewGetMargins(lpSSPreview, &nMarginH, &nMarginV);
SSPreviewGetPageSize(lpSSPreview, &RectPage, nMarginH, nMarginV);

if ((lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_NORMALSIZE ||
    lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE ||
    lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PAGEWIDTH) &&
    (RectPage.bottom + nMarginV + (2 * lpSSPreview->nPageBorderWidth) +
    lpSSPreview->nPageShadowWidth > RectClient.bottom))
   fScrollPartial = TRUE;

return (fScrollPartial);
}