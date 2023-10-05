/*********************************************
* pvprops.c
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

#define SS_ISDEFCOLOR(clr) (clr == (0x80000000L | COLOR_INACTIVEBORDER) || clr == (COLORREF)-1L || clr == -1)


LRESULT PASCAL PV_SetProp(LPSSPREVIEW lpSSPreview, WORD wProp, LPARAM lParam)
{
HWND hWnd = lpSSPreview->hWnd;
long lRet = 0L;

switch (wProp)
   {
#ifdef NO_MFC
  case PVPROP_MOUSEICON:
	  fpPM_SetProp(0, hWnd, FP_CTLTYPE_OCX, &lpSSPreview->idMouseIcon, lParam);
	  lRet = SendMessage(hWnd, PP_MOUSEICONCHANGED, 0, 0);
      break;
#endif
   case PVPROP_ALLOWUSERZOOM:
      lpSSPreview->fAllowUserZoom = (BOOL)lParam;
      break;

   case PVPROP_GRAYAREACOLOR:
      if (SS_ISDEFCOLOR(lParam))
         lpSSPreview->GrayAreaColor = RGBCOLOR_DARKGRAY;
      else
         lpSSPreview->GrayAreaColor = (COLORREF)lParam;

      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case PVPROP_GRAYAREAMARGINH:
      if (lParam >= 0)
         {
         lpSSPreview->lGrayAreaMarginH = (long)lParam;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

   case PVPROP_GRAYAREAMARGINTYPE:
      lpSSPreview->wGrayAreaMarginType = (WORD)lParam;
      SSPreviewCalcPage(lpSSPreview);
      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case PVPROP_GRAYAREAMARGINV:
      if (lParam >= 0)
         {
         lpSSPreview->lGrayAreaMarginV = (long)lParam;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

   case PVPROP_HWNDSPREAD:
      SSPreviewSrcClose(lpSSPreview);
      lpSSPreview->hWndSpread = (HWND)lParam;
      SSPreviewSrcOpen(lpSSPreview);
      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case PVPROP_PAGEBORDERCOLOR:
      if (SS_ISDEFCOLOR(lParam))
         lpSSPreview->PageBorderColor = RGBCOLOR_DARKBLUE;
      else
         lpSSPreview->PageBorderColor = (COLORREF)lParam;

      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case PVPROP_PAGEBORDERWIDTH:
      if ((short)lParam >= 0 && (short)lParam <= 100)
         {
         lpSSPreview->nPageBorderWidth = (short)lParam;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

   case PVPROP_PAGESHADOWCOLOR:
      if (SS_ISDEFCOLOR(lParam))
         lpSSPreview->PageShadowColor = RGBCOLOR_BLACK;
      else
         lpSSPreview->PageShadowColor = (COLORREF)lParam;

      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case PVPROP_PAGESHADOWWIDTH:
      if ((short)lParam >= 0 && (short)lParam <= 100)
         {
         lpSSPreview->nPageShadowWidth = (short)lParam;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

   case PVPROP_PAGEVIEWPERCENTAGE:
      if ((short)lParam > 0 && (short)lParam < 1000)
         {
         lpSSPreview->nPageViewPercentage = (short)lParam;
         if (lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE)
            PV_SetProp(lpSSPreview, PVPROP_PAGEVIEWTYPE,
                       (LPARAM)lpSSPreview->wPageViewType);
         }

      break;

   case PVPROP_PAGEVIEWTYPE:
      lpSSPreview->wPageViewType = (WORD)lParam;
      SSPreviewCalcPage(lpSSPreview);

      /************************
      * Save Old PageViewType
      ************************/

      {
      RECT  RectClient;
      RECT  RectPage;
      short nMarginH;
      short nMarginV;

      GetClientRect(lpSSPreview->hWnd, &RectClient);

      SSPreviewGetMargins(lpSSPreview, &nMarginH, &nMarginV);
      SSPreviewGetPageSize(lpSSPreview, &RectPage, nMarginH, nMarginV);

      if ((RectPage.right + nMarginH + (2 * lpSSPreview->nPageBorderWidth) +
           lpSSPreview->nPageShadowWidth <= RectClient.right) &&
          (RectPage.bottom + nMarginV + (2 * lpSSPreview->nPageBorderWidth) +
           lpSSPreview->nPageShadowWidth <= RectClient.bottom))
         {
         lpSSPreview->wPageViewTypePrev = lpSSPreview->wPageViewType;
         lpSSPreview->nPageMultiCntHPrev = lpSSPreview->nPageMultiCntH;
         lpSSPreview->nPageMultiCntVPrev = lpSSPreview->nPageMultiCntV;
         lpSSPreview->wZoomState = SSPRVW_ZOOMSTATE_OUT;

/*
         if (lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_MULTIPLEPAGES)
            {
            short nPagesPerScreen = SSPreviewGetPagesPerScreen(lpSSPreview);

            lpSSPreview->lPageCurrent =
               (((lpSSPreview->lPageCurrent - lpSSPreview->lPageBeg) /
                nPagesPerScreen) * nPagesPerScreen) + lpSSPreview->lPageBeg;

            SSPreviewCalcPage(lpSSPreview);
            }
*/
         }
      else if (lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_NORMALSIZE ||
          (lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_PERCENTAGE &&
           lpSSPreview->nPageViewPercentage == 100))
         lpSSPreview->wZoomState = SSPRVW_ZOOMSTATE_IN;
      else
         lpSSPreview->wZoomState = SSPRVW_ZOOMSTATE_INDETERMINATE;
      }

      InvalidateRect(hWnd, NULL, TRUE);
      break;

   case PVPROP_SCROLLBARH:
      lpSSPreview->wScrollBarH = (WORD)lParam;
      SSPreviewCalcPage(lpSSPreview);
      break;

   case PVPROP_SCROLLBARV:
      lpSSPreview->wScrollBarV = (WORD)lParam;
      SSPreviewCalcPage(lpSSPreview);
      break;

   case PVPROP_SCROLLINCH:
      if (lParam > 0)
         {
         lpSSPreview->lScrollIncH = (long)lParam;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

   case PVPROP_SCROLLINCV:
      if (lParam > 0)
         {
         lpSSPreview->lScrollIncV = (long)lParam;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

#ifdef SS_V35
   case PVPROP_SCRIPTENHANCED:
      lpSSPreview->bScriptEnhanced = (BOOL)lParam;
      break;

#endif

/*
   case PVPROP_SHOWMARGINS:
      if (lpSSPreview->fAllowMarginResize)
         {
         lpSSPreview->fShowMargins = (BOOL)lParam;
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;
*/

   case PVPROP_PAGEMULTICNTH:
   case PVPROP_PAGEMULTICNTV:
      if ((short)lParam > 0 && (short)lParam <= 10)
         {
         if (wProp == PVPROP_PAGEMULTICNTH)
            lpSSPreview->nPageMultiCntH = (short)lParam;
         else
            lpSSPreview->nPageMultiCntV = (short)lParam;

/*
         if (lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_MULTIPLEPAGES)
            {
            short nPagesPerScreen = SSPreviewGetPagesPerScreen(lpSSPreview);

            lpSSPreview->lPageCurrent =
               (((lpSSPreview->lPageCurrent - lpSSPreview->lPageBeg) /
                nPagesPerScreen) * nPagesPerScreen) + lpSSPreview->lPageBeg;
            }
*/

         SSPreviewCalcPage(lpSSPreview);

         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

   case PVPROP_PAGEGUTTERH:
      if ((long)lParam >= -1)
         {
         lpSSPreview->lPageGutterH = (long)lParam;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

   case PVPROP_PAGEGUTTERV:
      if ((long)lParam >= -1)
         {
         lpSSPreview->lPageGutterV = (long)lParam;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;

   case PVPROP_ZOOMSTATE:
      if (lpSSPreview->wZoomState != (WORD)lParam)
         {
         switch ((WORD)lParam)
            {
            case SSPRVW_ZOOMSTATE_OUT:
               lpSSPreview->wZoomState = (WORD)lParam;

               PV_SetProp(lpSSPreview, PVPROP_PAGEVIEWTYPE,
                          lpSSPreview->wPageViewTypePrev);
               PV_SendWMCommand(lpSSPreview, SPN_ZOOM);
               break;

            case SSPRVW_ZOOMSTATE_IN:
               lpSSPreview->wZoomState = (WORD)lParam;
               lpSSPreview->wPageViewTypePrev = lpSSPreview->wPageViewType;

               PV_SetProp(lpSSPreview, PVPROP_PAGEVIEWTYPE,
                          SSPRVW_VIEWTYPE_NORMALSIZE);
               PV_SendWMCommand(lpSSPreview, SPN_ZOOM);
               break;

            case SSPRVW_ZOOMSTATE_INDETERMINATE:
               break;

            case SSPRVW_ZOOMSTATE_SWITCH:
               SSPreviewZoom(lpSSPreview);
               break;
            }
         }

      break;

   case PVPROP_PAGECURRENT:
      if (lParam >= lpSSPreview->lPageBeg && lParam <= lpSSPreview->lPageEnd)
         {
         long lScrollPosVOld = lpSSPreview->lScrollPosV;

         SSPreviewSetPageCurrent(lpSSPreview, (long)lParam);

         if (lpSSPreview->wPageViewType != SSPRVW_VIEWTYPE_MULTIPLEPAGES ||
             lpSSPreview->lScrollPosV != lScrollPosVOld)
            {
            SSPreviewFreePagesInMem(lpSSPreview);
            //SSPreviewCalcPage(lpSSPreview);
            InvalidateRect(hWnd, NULL, TRUE);
            }
         }

      break;
/*#ifdef PP4
      case PVPROP_PAGEHEIGHT:
      if (lParam >= 0)
         {
         lpSSPreview->fPageHeight = (double)((long)YTwipsToPixels(lParam))/(double)lpSSPreview->nPixelsPerInchY;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;
   case PVPROP_PAGEWIDTH:
      if (lParam >= 0)
         {
         lpSSPreview->fPageWidth = (double)((long)XTwipsToPixels(lParam))/(double)lpSSPreview->nPixelsPerInchX;
         SSPreviewCalcPage(lpSSPreview);
         InvalidateRect(hWnd, NULL, TRUE);
         }

      break;
#endif
	  */
}

 return (lRet);
}


LRESULT PASCAL PV_GetProp(LPSSPREVIEW lpSSPreview, WORD wProp, LPARAM lParam)
{
LRESULT lRet = 0L;

switch (wProp)
   {
#ifdef NO_MFC
  case PVPROP_MOUSEICON:
    lRet = fpPM_GetProp(0, FP_CTLTYPE_OCX, lpSSPreview->idMouseIcon, lParam);
    break;
#endif
   case PVPROP_ALLOWUSERZOOM:
      lRet = (long)lpSSPreview->fAllowUserZoom;
      break;

   case PVPROP_GRAYAREACOLOR:
      lRet = (long)lpSSPreview->GrayAreaColor;
      break;

   case PVPROP_GRAYAREAMARGINH:
      lRet = (long)lpSSPreview->lGrayAreaMarginH;
      break;

   case PVPROP_GRAYAREAMARGINTYPE:
      lRet = (long)lpSSPreview->wGrayAreaMarginType;
      break;

   case PVPROP_GRAYAREAMARGINV:
      lRet = (long)lpSSPreview->lGrayAreaMarginV;
      break;

   case PVPROP_HWNDSPREAD:
      lRet = (LRESULT)lpSSPreview->hWndSpread;
      break;

   case PVPROP_PAGEBORDERCOLOR:
      lRet = (long)lpSSPreview->PageBorderColor;
      break;

   case PVPROP_PAGEBORDERWIDTH:
      lRet = (long)lpSSPreview->nPageBorderWidth;
      break;

   case PVPROP_PAGESHADOWCOLOR:
      lRet = (long)lpSSPreview->PageShadowColor;
      break;

   case PVPROP_PAGESHADOWWIDTH:
      lRet = (long)lpSSPreview->nPageShadowWidth;
      break;

   case PVPROP_PAGEVIEWPERCENTAGE:
      lRet = (long)lpSSPreview->nPageViewPercentage;
      break;

   case PVPROP_PAGEVIEWTYPE:
      lRet = (long)lpSSPreview->wPageViewType;
      break;

   case PVPROP_SCROLLBARH:
      lRet = (long)lpSSPreview->wScrollBarH;
      break;

   case PVPROP_SCROLLBARV:
      lRet = (long)lpSSPreview->wScrollBarV;
      break;

   case PVPROP_SCROLLINCH:
      lRet = (long)lpSSPreview->lScrollIncH;
      break;

   case PVPROP_SCROLLINCV:
      lRet = (long)lpSSPreview->lScrollIncV;
      break;

/*
   case PVPROP_SHOWMARGINS:
      lRet = (long)lpSSPreview->fShowMargins;
      break;
*/

   case PVPROP_PAGEMULTICNTH:
      lRet = (long)lpSSPreview->nPageMultiCntH;
      break;

   case PVPROP_PAGEMULTICNTV:
      lRet = (long)lpSSPreview->nPageMultiCntV;
      break;

   case PVPROP_PAGEGUTTERH:
      lRet = (long)lpSSPreview->lPageGutterH;
      break;

   case PVPROP_PAGEGUTTERV:
      lRet = (long)lpSSPreview->lPageGutterV;
      break;

   case PVPROP_PAGECURRENT:
      lRet = (long)lpSSPreview->lPageCurrent;
      break;

   case PVPROP_PAGESPERSCREEN:
      if (lpSSPreview->wPageViewType == SSPRVW_VIEWTYPE_MULTIPLEPAGES)
         lRet = lpSSPreview->nPageMultiCntH * lpSSPreview->nPageMultiCntV;
      else
         lRet = 1;

      break;

#ifdef SS_V35
   case PVPROP_SCRIPTENHANCED:
      lRet = (long)lpSSPreview->bScriptEnhanced;
      break;

#endif

   case PVPROP_ZOOMSTATE:
      lRet = (long)lpSSPreview->wZoomState;
      break;

   case PVPROP_PAGEPERCENTAGEACTUAL:
      {
      RECT   RectPage;
      short  nMarginH;
      short  nMarginV;
      double dfNormalPageV = (double)lpSSPreview->fPageHeight *
                             (double)lpSSPreview->nPixelsPerInchY;

      SSPreviewGetMargins(lpSSPreview, &nMarginH, &nMarginV);
      SSPreviewGetPageSize(lpSSPreview, &RectPage, nMarginH, nMarginV);

      lRet = (long)(((double)(RectPage.bottom - RectPage.top) / dfNormalPageV *
                             100.0) + 0.5);
      }
      break;

/*#ifdef PP4   

   case PVPROP_PAGEHEIGHT:
	   {
      long lVal = (long)((double)lpSSPreview->fPageHeight *
                         (double)lpSSPreview->nPixelsPerInchY);
	  lRet = YPixelsToTwips(lVal);
	   }
      break;

   case PVPROP_PAGEWIDTH:
	   {
      long lVal = (long)((double)lpSSPreview->fPageWidth *
                         (double)lpSSPreview->nPixelsPerInchX);
	   lRet = XPixelsToTwips(lVal);
	   }
      break;

#endif*/
}

return (lRet);
}


#ifdef SS_DLL

enum 
  {
  INT_TYPE,
  LONG_TYPE,
  SHORT_TYPE,
  BOOL_TYPE,
  COLOR_TYPE,
  FLOAT_TYPE,
  HWND_TYPE,
  TEXT_TYPE,             // should use "GetAnyText()" instead.
  };

static int GetAnyValue( HWND hWnd, int parmType, WORD property, void FAR *lpValue)
{
  LRESULT rc;
  
  if (lpValue == NULL)
     return(-1);
     
  rc = SendMessage(hWnd, SPM_GETPROPERTY, (WPARAM)property, (LPARAM)lpValue);
  
  switch(parmType)
    {
    case INT_TYPE:      *(int FAR *)lpValue      = (int)rc;       break;
    case LONG_TYPE:     *(long FAR *)lpValue     = (long)rc;      break;
    case SHORT_TYPE:    *(short FAR *)lpValue    = (short)rc;     break;
    case BOOL_TYPE:     *(BOOL FAR *)lpValue     = (BOOL)rc;      break;
    case COLOR_TYPE:    *(COLORREF FAR *)lpValue = (COLORREF)rc;  break;
    case HWND_TYPE:     *(HWND FAR *)lpValue   = (HANDLE)rc;    break;
    case FLOAT_TYPE:    break;      //lParam is already treated as a PTR
    case TEXT_TYPE:     break;      //Use "GetAnyText()" instead
    default:            break;
    } //switch
    
  return(0);  
}

int FP_API SpvGetAllowUserZoom(HWND hWnd, BOOL FAR *lpValue) 
  { return GetAnyValue(hWnd, BOOL_TYPE, PVPROP_ALLOWUSERZOOM, (LPVOID)lpValue); }
int FP_API SpvGetGrayAreaColor(HWND hWnd, COLORREF FAR *lpValue) 
  { return GetAnyValue(hWnd, COLOR_TYPE, PVPROP_GRAYAREACOLOR, (LPVOID)lpValue); }
int FP_API SpvGetGrayAreaMarginH(HWND hWnd, long FAR *lpValue) 
  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_GRAYAREAMARGINH, (LPVOID)lpValue); }
int FP_API SpvGetGrayAreaMarginType(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_GRAYAREAMARGINTYPE, (LPVOID)lpValue); }
int FP_API SpvGetGrayAreaMarginV(HWND hWnd, long FAR *lpValue) 
  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_GRAYAREAMARGINV, (LPVOID)lpValue); }
int FP_API SpvGetPageBorderColor(HWND hWnd, COLORREF FAR *lpValue) 
  { return GetAnyValue(hWnd, COLOR_TYPE, PVPROP_PAGEBORDERCOLOR, (LPVOID)lpValue); }
int FP_API SpvGetPageBorderWidth(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_PAGEBORDERWIDTH, (LPVOID)lpValue); }
int FP_API SpvGetPageCurrent(HWND hWnd, long FAR *lpValue) 
  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_PAGECURRENT, (LPVOID)lpValue); }
int FP_API SpvGetPageGutterH(HWND hWnd, long FAR *lpValue) 
  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_PAGEGUTTERH, (LPVOID)lpValue); }
int FP_API SpvGetPageGutterV(HWND hWnd, long FAR *lpValue) 
  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_PAGEGUTTERV, (LPVOID)lpValue); }
int FP_API SpvGetPageMultiCntH(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_PAGEMULTICNTH, (LPVOID)lpValue); }
int FP_API SpvGetPageMultiCntV(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_PAGEMULTICNTV, (LPVOID)lpValue); }
int FP_API SpvGetPagePercentageActual(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_PAGEPERCENTAGEACTUAL, (LPVOID)lpValue); }
int FP_API SpvGetPageShadowColor(HWND hWnd, COLORREF FAR *lpValue) 
  { return GetAnyValue(hWnd, COLOR_TYPE, PVPROP_PAGESHADOWCOLOR, (LPVOID)lpValue); }
int FP_API SpvGetPageShadowWidth(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_PAGESHADOWWIDTH, (LPVOID)lpValue); }
int FP_API SpvGetPageViewPercentage(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_PAGEVIEWPERCENTAGE, (LPVOID)lpValue); }
int FP_API SpvGetPageViewType(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_PAGEVIEWTYPE, (LPVOID)lpValue); }
int FP_API SpvGetPagesPerScreen(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_PAGESPERSCREEN, (LPVOID)lpValue); }
int FP_API SpvGetScrollBarH(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_SCROLLBARH, (LPVOID)lpValue); }
int FP_API SpvGetScrollBarV(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_SCROLLBARV, (LPVOID)lpValue); }
int FP_API SpvGetScrollIncH(HWND hWnd, long FAR *lpValue) 
  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_SCROLLINCH, (LPVOID)lpValue); }
int FP_API SpvGetScrollIncV(HWND hWnd, long FAR *lpValue) 
  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_SCROLLINCV, (LPVOID)lpValue); }
int FP_API SpvGetZoomState(HWND hWnd, short FAR *lpValue) 
  { return GetAnyValue(hWnd, SHORT_TYPE, PVPROP_ZOOMSTATE, (LPVOID)lpValue); }
int FP_API SpvGethWndSpread(HWND hWnd, HWND FAR *lphWnd)
  { return GetAnyValue(hWnd, HWND_TYPE, PVPROP_HWNDSPREAD, (LPVOID)lphWnd); }
//#ifdef PP4
//int FP_API SpvGetPageHeight(HWND hWnd, long FAR *lpValue) 
//  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_PAGEHEIGHT, (LPVOID)lpValue); }
//int FP_API SpvGetPageWidth(HWND hWnd, long FAR *lpValue) 
//  { return GetAnyValue(hWnd, LONG_TYPE, PVPROP_PAGEWIDTH, (LPVOID)lpValue); }
//#endif
int FP_API SpvSetAllowUserZoom(HWND hWnd, BOOL value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_ALLOWUSERZOOM, (LPARAM)value); }
int FP_API SpvSetGrayAreaColor(HWND hWnd, COLORREF value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_GRAYAREACOLOR, (LPARAM)value); }
int FP_API SpvSetGrayAreaMarginH(HWND hWnd, long value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_GRAYAREAMARGINH, (LPARAM)value); }
int FP_API SpvSetGrayAreaMarginType(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_GRAYAREAMARGINTYPE, (LPARAM)value); }
int FP_API SpvSetGrayAreaMarginV(HWND hWnd, long value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_GRAYAREAMARGINV, (LPARAM)value); }
int FP_API SpvSetPageBorderColor(HWND hWnd, COLORREF value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEBORDERCOLOR, (LPARAM)value); }
int FP_API SpvSetPageBorderWidth(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEBORDERWIDTH, (LPARAM)value); }
int FP_API SpvSetPageCurrent(HWND hWnd, long value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGECURRENT, (LPARAM)value); }
int FP_API SpvSetPageGutterH(HWND hWnd, long value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEGUTTERH, (LPARAM)value); }
int FP_API SpvSetPageGutterV(HWND hWnd, long value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEGUTTERV, (LPARAM)value); }
int FP_API SpvSetPageMultiCntH(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEMULTICNTH, (LPARAM)value); }
int FP_API SpvSetPageMultiCntV(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEMULTICNTV, (LPARAM)value); }
int FP_API SpvSetPageShadowColor(HWND hWnd, COLORREF value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGESHADOWCOLOR, (LPARAM)value); }
int FP_API SpvSetPageShadowWidth(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGESHADOWWIDTH, (LPARAM)value); }
int FP_API SpvSetPageViewPercentage(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEVIEWPERCENTAGE, (LPARAM)value); }
int FP_API SpvSetPageViewType(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEVIEWTYPE, (LPARAM)value); }
int FP_API SpvSetScrollBarH(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_SCROLLBARH, (LPARAM)value); }
int FP_API SpvSetScrollBarV(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_SCROLLBARV, (LPARAM)value); }
int FP_API SpvSetScrollIncH(HWND hWnd, long value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_SCROLLINCH, (LPARAM)value); }
int FP_API SpvSetScrollIncV(HWND hWnd, long value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_SCROLLINCV, (LPARAM)value); }
int FP_API SpvSetZoomState(HWND hWnd, short value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_ZOOMSTATE, (LPARAM)value); }
int FP_API SpvSethWndSpread(HWND hWnd, HWND hWndSpread)
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_HWNDSPREAD, (LPARAM)hWndSpread); }

//#ifdef PP4
//int FP_API SpvSetPageHeight(HWND hWnd, long value) 
//  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEHEIGHT, (LPARAM)value); }
//int FP_API SpvSetPageWidth(HWND hWnd, long value) 
//  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_PAGEWIDTH, (LPARAM)value); }
//#endif

#ifdef SS_V35
int FP_API SpvGetScriptEnhanced(HWND hWnd, BOOL FAR *lpValue) 
  { return GetAnyValue(hWnd, BOOL_TYPE, PVPROP_SCRIPTENHANCED, (LPVOID)lpValue); }
int FP_API SpvSetScriptEnhanced(HWND hWnd, BOOL value) 
  { return (int)SendMessage(hWnd, SPM_SETPROPERTY, PVPROP_SCRIPTENHANCED, (LPARAM)value); }
#endif

#endif // SS_DLL
;
