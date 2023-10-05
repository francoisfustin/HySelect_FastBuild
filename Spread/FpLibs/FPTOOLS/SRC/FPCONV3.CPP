/****************************************************************************/
/* FPCONV3.CPP   FarPoint Support for Windows API in non-hWnd OCX's        */
/****************************************************************************/

#ifdef STRICT
#undef STRICT
#endif

#include <string.h>
#include <dos.h>

#ifdef FP_OCX
#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#else
//#include "ctlcore.h"
#include "stdafx.h"
#include "fpatl.h"
#endif

#ifdef WIN32

#ifndef NO_MFC
#include <afxpriv.h>
#endif

#ifdef NO_MFC
#if (defined( __T) && !defined(_INC_TCHAR))
#undef __T
#endif
#endif

#include "tchar.h"

#else

#define WM_NOTIFY                       0x004E

#define USES_CONVERSION int _convert; _convert

#define T2OLE  
#define OLE2T

#define NON_STANDARD_INDEX  12

#endif

#include "vbinterf.h"
// following lines taken from VBINTERF.H
#include <initguid.h>

#endif

#include "fptools.h"                             

#ifdef __cplusplus
extern "C" {
#endif

//#define fpScaleModeToUnits  FP_PREFIX(fpScaleModeToUnits) 
//#define fpPixelsToScaleMode FP_PREFIX(fpPixelsToScaleMode) 
#define fpGetPropIndex      FP_PREFIX(fpGetPropIndex) 
/*
void fpScaleModeToUnits(LPSCALEMODE lpScaleMode, float flX, float flY,
                        LPLONG lpXPixels, LPLONG lpYPixels, BOOL fSize);
void fpPixelsToScaleMode(LPSCALEMODE lpScaleMode, long lX, long lY,
                        float far * lpXUnits, float far * lpYUnits, BOOL fSize);
*/
USHORT fpGetPropIndex(LPFPCONTROL lpObject, LPSTR lpszProp);

#ifndef FP_DLL
BYTE FPLIB fpVBGetControlRect(LPFPCONTROL lpObject, LPRECT lprect)
{
#ifdef FP_OCX
  return (fpGetWindowRect(lpObject, lprect));
#else
  VBGetControlRect((HCTL)lpObject->hCtl, lprect);
  return (0);
#endif
}
#endif

BYTE FPLIB fpGetWindowRect(LPFPCONTROL lpObject, LPRECT lprect)
{
#ifdef FP_OCX 
#ifndef NO_MFC
  BYTE bReturn = FALSE;
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    ((CWnd *)pObject)->GetWindowRect(lprect);
  else
    bReturn = fpGetRectFromDispatch(lpObject, lprect);

  return bReturn;
#else			 // NO_MFC
  BYTE bReturn = FALSE;
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndBase = NULL;
  
  if (lpObject->hWnd)
	 hWndBase = lpObject->hWnd;
  
  if (pObject)
	 hWndBase = pObject->fpAtlGetHWnd();
  
  if ((hWndBase) && IsWindow(hWndBase))
    GetWindowRect(hWndBase, lprect);
  else
    bReturn = fpGetRectFromDispatch(lpObject, lprect);

  return bReturn;
#endif
#else
  GetWindowRect((HWND)lpObject->hWnd, lprect);
  return 0;
#endif
}   


BYTE FPLIB fpGetRectFromDispatch(LPFPCONTROL lpObject, LPRECT lpRect)
{
#ifdef FP_OCX
SCALEMODE ScaleMode;
FPCONTROL fpTemp = { 0, 0, 0L, 0L};
float     flLeft;
float     flTop;
float     flWidth;
float     flHeight;
float     flX1;
float     flY1;
float     flX2;
float     flY2;
long      lLeft;
long      lTop;
long      lWidth;
long      lHeight;
long      X1;
long      Y1;
long      X2;
long      Y2;
BYTE      bLineOrient = 0;

//fpVBGetScaleMode(lpObject, &ScaleMode);
fpVBGetScaleMode(lpObject, &ScaleMode, TRUE);

if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&flLeft,
                           _T("Left"), VT_R4) != (ERR)-1)
   {
   fpVBGetControlProperty(lpObject, 0, (LPVOID)&flTop, _T("Top"), VT_R4);
   fpScaleModeToUnits(&ScaleMode, flLeft, flTop, &lLeft, &lTop, FALSE);
   lpRect->left = (int)lLeft;
   lpRect->top = (int)lTop;

   if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&flWidth,
                              _T("Width"), VT_R4) != (ERR)-1)
      {
      fpVBGetControlProperty(lpObject, 0, (LPVOID)&flHeight,
                             _T("Height"), VT_R4);
      fpScaleModeToUnits(&ScaleMode, flWidth, flHeight, &lWidth, &lHeight,
                         TRUE);
      lpRect->right = lpRect->left + (int)lWidth;
      lpRect->bottom = lpRect->top + (int)lHeight;
      }
   else
      {
      // if no height or width, make them 1 pixel to satisfy rectangle
      // functions in API
      lpRect->right = lpRect->left + 1;
      lpRect->bottom = lpRect->top + 1;
      }
   }

else if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&flX1,
                                _T("X1"), VT_R4) != (ERR)-1)
   {
   fpVBGetControlProperty(lpObject, 0, (LPVOID)&flY1, _T("Y1"), VT_R4);
   fpVBGetControlProperty(lpObject, 0, (LPVOID)&flX2, _T("X2"), VT_R4);
   fpVBGetControlProperty(lpObject, 0, (LPVOID)&flY2, _T("Y2"), VT_R4);
   fpScaleModeToUnits(&ScaleMode, flX1, flY1, &X1, &Y1, FALSE);
   fpScaleModeToUnits(&ScaleMode, flX2, flY2, &X2, &Y2, FALSE);

   SetRect(lpRect, (int)min(X1, X2), (int)min(Y1, Y2), (int)(max(X1, X2) + 1L),
           (int)(max(Y1, Y2) + 1L));

   if (X1 == lpRect->left && Y1 == lpRect->top)
      bLineOrient = FP_LINEORIENT_UL;
   else if (X1 == lpRect->right - 1 && Y1 == lpRect->top)
      bLineOrient = FP_LINEORIENT_UR;
   else if (X1 == lpRect->right - 1 && Y1 == lpRect->bottom - 1)
      bLineOrient = FP_LINEORIENT_LR;
   else if (X1 == lpRect->left && Y1 == lpRect->bottom - 1)
      bLineOrient = FP_LINEORIENT_LL;

   }
else // make it a minimal rectangle
   SetRect(lpRect, 0, 0, 1, 1);

/*************************************************
* Now turn the rectangle into screen coordinates
*************************************************/

fpTemp.lpDispatch = fpGetControlDispatch(lpObject, GC_CONTAINER);

if (fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&fpTemp.hWnd,
    _T("hWnd"), VT_I4) != (ERR)-1)
   {
   POINT pt;

   pt.x = 0;
   pt.y = 0;
   ClientToScreen(fpTemp.hWnd, &pt);
   OffsetRect(lpRect, pt.x, pt.y);
   }

fpDispatchRelease(fpTemp.lpDispatch);

return (bLineOrient);
#else
return (0);
#endif
}

BYTE FPLIB fpGetRectFromDispatchEx(LPFPCONTROL lpObject, LPRECT lpRect)
{
#ifdef FP_OCX
SCALEMODE ScaleMode;
FPCONTROL fpTemp = { 0, 0, 0L, 0L};
float     flLeft;
float     flTop;
float     flWidth;
float     flHeight;
float     flX1;
float     flY1;
float     flX2;
float     flY2;
long      lLeft;
long      lTop;
long      lWidth;
long      lHeight;
long      X1;
long      Y1;
long      X2;
long      Y2;
BYTE      bLineOrient = 0;

//fpVBGetScaleMode(lpObject, &ScaleMode);
fpVBGetScaleModeEx(lpObject, &ScaleMode, TRUE, TRUE);

if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&flLeft,
                           _T("Left"), VT_R4) != (ERR)-1)
   {
   fpVBGetControlProperty(lpObject, 0, (LPVOID)&flTop, _T("Top"), VT_R4);
   fpScaleModeToUnits(&ScaleMode, flLeft, flTop, &lLeft, &lTop, FALSE);
   lpRect->left = (int)lLeft;
   lpRect->top = (int)lTop;

   if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&flWidth,
                              _T("Width"), VT_R4) != (ERR)-1)
      {
      fpVBGetControlProperty(lpObject, 0, (LPVOID)&flHeight,
                             _T("Height"), VT_R4);
      fpScaleModeToUnits(&ScaleMode, flWidth, flHeight, &lWidth, &lHeight,
                         TRUE);
      lpRect->right = lpRect->left + (int)lWidth;
      lpRect->bottom = lpRect->top + (int)lHeight;
      }
   else
      {
      // if no height or width, make them 1 pixel to satisfy rectangle
      // functions in API
      lpRect->right = lpRect->left + 1;
      lpRect->bottom = lpRect->top + 1;
      }
   }

else if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&flX1,
                                _T("X1"), VT_R4) != (ERR)-1)
   {
   fpVBGetControlProperty(lpObject, 0, (LPVOID)&flY1, _T("Y1"), VT_R4);
   fpVBGetControlProperty(lpObject, 0, (LPVOID)&flX2, _T("X2"), VT_R4);
   fpVBGetControlProperty(lpObject, 0, (LPVOID)&flY2, _T("Y2"), VT_R4);
   fpScaleModeToUnits(&ScaleMode, flX1, flY1, &X1, &Y1, FALSE);
   fpScaleModeToUnits(&ScaleMode, flX2, flY2, &X2, &Y2, FALSE);

   SetRect(lpRect, (int)min(X1, X2), (int)min(Y1, Y2), (int)(max(X1, X2) + 1L),
           (int)(max(Y1, Y2) + 1L));

   if (X1 == lpRect->left && Y1 == lpRect->top)
      bLineOrient = FP_LINEORIENT_UL;
   else if (X1 == lpRect->right - 1 && Y1 == lpRect->top)
      bLineOrient = FP_LINEORIENT_UR;
   else if (X1 == lpRect->right - 1 && Y1 == lpRect->bottom - 1)
      bLineOrient = FP_LINEORIENT_LR;
   else if (X1 == lpRect->left && Y1 == lpRect->bottom - 1)
      bLineOrient = FP_LINEORIENT_LL;

   }
else // make it a minimal rectangle
   SetRect(lpRect, 0, 0, 1, 1);

/*************************************************
* Now turn the rectangle into screen coordinates
*************************************************/

fpTemp.lpDispatch = fpGetControlDispatch(lpObject, GC_CONTAINER);

if (fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&fpTemp.hWnd,
    _T("hWnd"), VT_I4) != (ERR)-1)
   {
   POINT pt;

   pt.x = 0;
   pt.y = 0;
   ClientToScreen(fpTemp.hWnd, &pt);
   OffsetRect(lpRect, pt.x, pt.y);
   }

fpDispatchRelease(fpTemp.lpDispatch);

return (bLineOrient);
#else
return (0);
#endif
}


void FPLIB fpSetRectFromDispatch(LPFPCONTROL lpObject, LPRECT lprect,
                                  BYTE bLineOrient)
{
  fpSetRectPartsFromDispatch(lpObject, lprect, bLineOrient, TRUE, TRUE, TRUE, TRUE);
}

void FPLIB fpSetRectFromDispatchEx(LPFPCONTROL lpObject, LPRECT lprect,
                                  BYTE bLineOrient, BOOL fDefaultToPixels)
{
  fpSetRectPartsFromDispatchEx(lpObject, lprect, bLineOrient, TRUE, TRUE, TRUE, TRUE, fDefaultToPixels);
}

void FPLIB fpSetRectPartsFromDispatch(LPFPCONTROL lpObject, LPRECT lprect,
                                  BYTE bLineOrient, BOOL bTop, BOOL bLeft, BOOL bHeight, BOOL bWidth)
{
#ifdef FP_OCX
    SCALEMODE ScaleMode;

    FPCONTROL fpTemp = { 0, 0, 0L, 0L};
	long      lTemp;
	
//    fpVBGetScaleMode(lpObject, &ScaleMode);
    fpVBGetScaleMode(lpObject, &ScaleMode, TRUE);

    if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&lTemp,
                               _T("Top"), VT_I4) != (ERR)-1)
    {                           
      float     flLeft;
      float     flTop;
      float     flWidth;
      float     flHeight;
      
	  // right and bottom need to be converted to width and height, respectively
      lprect->right = lprect->right - lprect->left;
      lprect->bottom = lprect->bottom - lprect->top;

//      flLeft = (float)VBXPixelsToTwips(lprect->left);
//      flTop = (float)VBYPixelsToTwips(lprect->top);
//      flWidth = (float)VBXPixelsToTwips(lprect->right);
//      flHeight = (float)VBYPixelsToTwips(lprect->bottom);
      flLeft = (float)lprect->left;
      flTop = (float)lprect->top;
      flWidth = (float)lprect->right;
      flHeight = (float)lprect->bottom;

      fpPixelsToScaleMode(&ScaleMode, lprect->left, lprect->top, &flLeft, &flTop, FALSE);

      fpPixelsToScaleMode(&ScaleMode, lprect->right, lprect->bottom, &flWidth, &flHeight, TRUE);

      if (bHeight)
		  fpVBSetControlProperty(lpObject, 0, (long)flHeight, _T("Height"), VT_I4);
      if (bWidth)
		  fpVBSetControlProperty(lpObject, 0, (long)flWidth, _T("Width"), VT_I4);
      if (bTop)
		  fpVBSetControlProperty(lpObject, 0, (long)flTop, _T("Top"), VT_I4);
      if (bLeft)
		  fpVBSetControlProperty(lpObject, 0, (long)flLeft, _T("Left"), VT_I4);
    }

    else if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&lTemp,
                                    _T("X1"), VT_I4) != (ERR)-1)
    {
      long X1;
      long X2;
      long Y1;
      long Y2;
      // object supports X1, X2, Y1, Y2

      switch (bLineOrient)
         {
         case FP_LINEORIENT_UL:
            X1 = lprect->left;
            Y1 = lprect->top;
            X2 = lprect->right - 1;
            Y2 = lprect->bottom - 1;
            break;

         case FP_LINEORIENT_UR:
            X1 = lprect->right - 1;
            Y1 = lprect->top;
            X2 = lprect->left;
            Y2 = lprect->bottom - 1;
            break;

         case FP_LINEORIENT_LR:
            X1 = lprect->right - 1;
            Y1 = lprect->bottom - 1;
            X2 = lprect->left;
            Y2 = lprect->top;
            break;

         case FP_LINEORIENT_LL:
            X1 = lprect->left;
            Y1 = lprect->bottom - 1;
            X2 = lprect->right - 1;
            Y2 = lprect->top;
            break;

         }

	  // assume left = X1, right = X2, top = Y1, bottom = Y2
      fpVBSetControlProperty(lpObject, 0, VBXPixelsToTwips(X1), _T("X1"), VT_I4);
      fpVBSetControlProperty(lpObject, 0, VBYPixelsToTwips(Y1), _T("Y1"), VT_I4);
      fpVBSetControlProperty(lpObject, 0, VBXPixelsToTwips(X2), _T("X2"), VT_I4);
      fpVBSetControlProperty(lpObject, 0, VBYPixelsToTwips(Y2), _T("Y2"), VT_I4);
    }
    else
    {
      // control doesn't have top, left, or X1, Y1 to allow movement
      ;
    }  
#endif
}

void FPLIB fpSetRectPartsFromDispatchEx(LPFPCONTROL lpObject, LPRECT lprect,
                                  BYTE bLineOrient, BOOL bTop, BOOL bLeft, BOOL bHeight, BOOL bWidth, BOOL fDefaultToPixels)
{
#ifdef FP_OCX
    SCALEMODE ScaleMode;

    FPCONTROL fpTemp = { 0, 0, 0L, 0L};
	long      lTemp;
	
//    fpVBGetScaleMode(lpObject, &ScaleMode);
    fpVBGetScaleModeEx(lpObject, &ScaleMode, TRUE, fDefaultToPixels);

    if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&lTemp,
                               _T("Top"), VT_I4) != (ERR)-1)
    {                           
      float     flLeft;
      float     flTop;
      float     flWidth;
      float     flHeight;
      
	  // right and bottom need to be converted to width and height, respectively
      lprect->right = lprect->right - lprect->left;
      lprect->bottom = lprect->bottom - lprect->top;

//      flLeft = (float)VBXPixelsToTwips(lprect->left);
//      flTop = (float)VBYPixelsToTwips(lprect->top);
//      flWidth = (float)VBXPixelsToTwips(lprect->right);
//      flHeight = (float)VBYPixelsToTwips(lprect->bottom);
      flLeft = (float)lprect->left;
      flTop = (float)lprect->top;
      flWidth = (float)lprect->right;
      flHeight = (float)lprect->bottom;

      fpPixelsToScaleMode(&ScaleMode, lprect->left, lprect->top, &flLeft, &flTop, FALSE);

      fpPixelsToScaleMode(&ScaleMode, lprect->right, lprect->bottom, &flWidth, &flHeight, TRUE);

      if (bHeight)
		  fpVBSetControlProperty(lpObject, 0, (long)flHeight, _T("Height"), VT_I4);
      if (bWidth)
		  fpVBSetControlProperty(lpObject, 0, (long)flWidth, _T("Width"), VT_I4);
      if (bTop)
		  fpVBSetControlProperty(lpObject, 0, (long)flTop, _T("Top"), VT_I4);
      if (bLeft)
		  fpVBSetControlProperty(lpObject, 0, (long)flLeft, _T("Left"), VT_I4);
    }

    else if (fpVBGetControlProperty(lpObject, 0, (LPVOID)&lTemp,
                                    _T("X1"), VT_I4) != (ERR)-1)
    {
      long X1;
      long X2;
      long Y1;
      long Y2;
      // object supports X1, X2, Y1, Y2

      switch (bLineOrient)
         {
         case FP_LINEORIENT_UL:
            X1 = lprect->left;
            Y1 = lprect->top;
            X2 = lprect->right - 1;
            Y2 = lprect->bottom - 1;
            break;

         case FP_LINEORIENT_UR:
            X1 = lprect->right - 1;
            Y1 = lprect->top;
            X2 = lprect->left;
            Y2 = lprect->bottom - 1;
            break;

         case FP_LINEORIENT_LR:
            X1 = lprect->right - 1;
            Y1 = lprect->bottom - 1;
            X2 = lprect->left;
            Y2 = lprect->top;
            break;

         case FP_LINEORIENT_LL:
            X1 = lprect->left;
            Y1 = lprect->bottom - 1;
            X2 = lprect->right - 1;
            Y2 = lprect->top;
            break;

         }

	  // assume left = X1, right = X2, top = Y1, bottom = Y2
      fpVBSetControlProperty(lpObject, 0, VBXPixelsToTwips(X1), _T("X1"), VT_I4);
      fpVBSetControlProperty(lpObject, 0, VBYPixelsToTwips(Y1), _T("Y1"), VT_I4);
      fpVBSetControlProperty(lpObject, 0, VBXPixelsToTwips(X2), _T("X2"), VT_I4);
      fpVBSetControlProperty(lpObject, 0, VBYPixelsToTwips(Y2), _T("Y2"), VT_I4);
    }
    else
    {
      // control doesn't have top, left, or X1, Y1 to allow movement
      ;
    }  
#endif
}


#if (defined(FP_VB) || defined(FP_OCX))
void FPLIB fpScaleModeToUnits(LPSCALEMODE lpScaleMode, float flX, float flY,
                        LPLONG lpXPixels, LPLONG lpYPixels, BOOL fSize)
{
switch (lpScaleMode->nScaleMode)
   {
   case 0:                             // User
      if (!fSize)
         {
         flX -= lpScaleMode->fScaleLeft;
         flY -= lpScaleMode->fScaleTop;
         }

	  if (lpXPixels) // don't GPF if I only want to convert one coordinate instead of two -scl
         *lpXPixels = (short)((double)flX * lpScaleMode->lWidth /
                           (double)lpScaleMode->fScaleWidth);
	  if (lpYPixels)
         *lpYPixels = (short)((double)flY * lpScaleMode->lHeight /
                           (double)lpScaleMode->fScaleHeight);

      break;

   case 1:                             // Twip
	  if (lpXPixels)
         *lpXPixels = XTwipsToPixels(flX);
	  if (lpYPixels)
         *lpYPixels = YTwipsToPixels(flY);
      break;

   case 2:                             // Point
	  if (lpXPixels)
         *lpXPixels = XPointsToPixels(flX);
	  if (lpYPixels)
         *lpYPixels = YPointsToPixels(flY);
      break;

   case 3:                             // Pixel
	  if (lpXPixels)
         *lpXPixels = (short)flX;
	  if (lpYPixels)
         *lpYPixels = (short)flY;
      break;

   case 4:                             // Char
	  if (lpXPixels)
         *lpXPixels = XCharsToPixels(flX);
	  if (lpYPixels)
         *lpYPixels = YCharsToPixels(flY);
      break;

   case 5:                             // Inch
	  if (lpXPixels)
         *lpXPixels = XInchesToPixels(flX);
	  if (lpYPixels)
         *lpYPixels = YInchesToPixels(flY);
      break;

   case 6:                             // MM
	  if (lpXPixels)
         *lpXPixels = XMMToPixels(flX);
	  if (lpYPixels)
         *lpYPixels = YMMToPixels(flY);
      break;

   case 7:                             // CM
	  if (lpXPixels)
         *lpXPixels = XCMToPixels(flX);
	  if (lpYPixels)
         *lpYPixels = YCMToPixels(flY);
      break;

   }
}

void FPLIB fpPixelsToScaleMode(LPSCALEMODE lpScaleMode, long lX, long lY,
                        float far * lpXUnits, float far * lpYUnits, BOOL fSize)
{
switch (lpScaleMode->nScaleMode)
   {
   case 0:                             // User
      if (!fSize)
         {
         lX += (long)lpScaleMode->fScaleLeft;
         lY += (long)lpScaleMode->fScaleTop;
         }

	  if (lpXUnits) // don't GPF if I only want to convert one coordinate instead of two -scl
		 *lpXUnits = (short)((double)lX * lpScaleMode->fScaleWidth /
                           (double)lpScaleMode->lWidth);
      if (lpYUnits)
		 *lpYUnits = (short)((double)lY * lpScaleMode->fScaleHeight /
                           (double)lpScaleMode->lHeight);

      break;

   case 1:                             // Twip
      if (lpXUnits)
         *lpXUnits = (float)XPixelsToTwips(lX);
	  if (lpYUnits)
	     *lpYUnits = (float)YPixelsToTwips(lY);
      break;

   case 2:                             // Point
	  if (lpXUnits)
	     *lpXUnits = (float)XPixelsToPoints(lX);
      if (lpYUnits)
	     *lpYUnits = (float)XPixelsToPoints(lY);
      break;

   case 3:                             // Pixel
	  if (lpXUnits)
		 *lpXUnits = (float)lX;
	  if (lpYUnits)
		 *lpYUnits = (float)lY;
      break;

   case 4:                             // Char
	  if (lpXUnits)
         *lpXUnits = (float)XPixelsToChars(lX);
      if (lpYUnits)
         *lpYUnits = (float)YPixelsToChars(lY);
      break;

   case 5:                             // Inch
	  if (lpXUnits)
	     *lpXUnits = (float)XPixelsToInches(lX);
      if (lpYUnits)
         *lpYUnits = (float)YPixelsToInches(lY);
      break;

   case 6:                             // MM
	  if (lpXUnits)
	     *lpXUnits = (float)XPixelsToMM(lX);
      if (lpYUnits)
         *lpYUnits = (float)YPixelsToMM(lY);
      break;

   case 7:                             // CM
	  if (lpXUnits)
	     *lpXUnits = (float)XPixelsToCM(lX);
      if (lpYUnits)
         *lpYUnits = (float)YPixelsToCM(lY);
      break;

   }
}

#pragma warning(once : 4049 4018)
USHORT fpGetPropIndex(LPFPCONTROL lpObject, LPSTR lpszProp)
{
#ifdef FP_OCX  
 // OCX will use the "Property Name" not the index
 return 0;
#else
MODEL FAR     *lpModel;
LPPROPLIST lppl;
LPPROPINFO lppi;
char buf[256];
LPSTR lstr;
USHORT i;

lpModel = fpVBGetControlModel(lpObject);
if (lpModel)
  {
  FP_SEG(lppi) = FP_SEG(lpModel);
  FP_SEG(lstr) = FP_SEG(lpModel);
  FP_SEG(lppl) = FP_SEG(lpModel);
  FP_OFF(lppl) = (int)lpModel->npproplist;
  
  for (i = 0; lppl[i]; i++)
     {
        if (lppl[i] < PPROPINFO_STD_LAST)
        {
        FP_OFF(lppi) = (int)lppl[i];
        if (lppi)
           FP_OFF(lstr) = (int)lppi->npszName;  
       _fstrcpy(buf, lstr);
        if (_fstrcmp(lpszProp, buf ) == 0)
          return i;
        }
     }
  }
return ((USHORT)-1);
#endif
}

//fpVBGetScaleMode NOTE:
// An assumption was made incorrectly:
// If a control had a ScaleMode property, it was assumed its
//      Height, Left, Top and Width properties would be in those units.
// All objects Height, Left, Top and Width properties are in the units
//      of its container, whether a control or form.
// The bContainerOnly parameter was added to adjust for this oversight.
// Set bContainerOnly = FALSE to make the function work like it used to.
// Set bContainerOnly = TRUE to make the function work correctly.

BOOL FPLIB fpVBGetScaleMode(LPFPCONTROL lpObject, LPSCALEMODE lpScaleMode, BOOL bContainerOnly)
{
#ifdef FP_OCX
  LONG lpDispParent = 0;
  FPCONTROL fpTemp = {0, 0, 0L, lpObject->lpDispatch};
  SCALEMODE scale = {-1, -1, -1, (float)-1, (float)-1, (float)-1, (float)-1};

  // OCX - only function
  if (bContainerOnly)
     {
     fpTemp.lpDispatch = fpGetControlDispatch(lpObject, GC_CONTAINER);
     lpDispParent = fpTemp.lpDispatch;
    
     if (fpTemp.lpDispatch == NULL ||
         fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.nScaleMode),
                               _T("ScaleMode"), VT_I2) == (ERR)-1)
        {
        //parent does not have a ScaleMode property, return twips
        lpScaleMode->nScaleMode = 1;
        if (fpTemp.lpDispatch)
           fpDispatchRelease(fpTemp.lpDispatch);
        return TRUE;
        }
     }
  else
     {
     if (fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.nScaleMode),
                                _T("ScaleMode"), VT_I2) == (ERR)-1)
        {
        // current object doesn't have a ScaleMode property, get parent's
        fpTemp.lpDispatch = fpGetControlDispatch(lpObject, GC_CONTAINER);
        lpDispParent = fpTemp.lpDispatch;
    
        if (fpTemp.lpDispatch == NULL ||
           fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.nScaleMode),
                                  _T("ScaleMode"), VT_I2) == (ERR)-1)
        //parent does not have a ScaleMode property, return twips
           {
           lpScaleMode->nScaleMode = 1;
           if (fpTemp.lpDispatch)
              fpDispatchRelease(fpTemp.lpDispatch);
	       return TRUE;
	       }
        }
  
     }
  lpScaleMode->nScaleMode = scale.nScaleMode;
  
  if (lpScaleMode->nScaleMode == 0)   // User defined scale mode
     {
     long lWnd;

     if (fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&lWnd,
                                _T("hWnd"), VT_I4) != (ERR)-1)
        {
        RECT RectClient;
     
        GetClientRect((HWND)lWnd, &RectClient);
        scale.lWidth = RectClient.right - RectClient.left;
        scale.lHeight = RectClient.bottom - RectClient.top;
        }
     else
        {
        fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.lHeight),
                               _T("Height"), VT_I4);
        fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.lWidth),
                               _T("Width"), VT_I4);
        scale.lWidth = XTwipsToPixels(scale.lWidth);
        scale.lHeight = YTwipsToPixels(scale.lHeight);
        }
   
     fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.fScaleTop),
                            _T("ScaleTop"), VT_R4);
     fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.fScaleLeft),
                            _T("ScaleLeft"), VT_R4);
     fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.fScaleHeight),
                            _T("ScaleHeight"), VT_R4);
     fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.fScaleWidth),
                            _T("ScaleWidth"), VT_R4);
     _fmemcpy(lpScaleMode, &scale, sizeof(SCALEMODE));                       	  
     }

  if (lpDispParent)
     fpDispatchRelease(lpDispParent);

#elif defined(FP_VB)
  FPCONTROL fpTemp = {lpObject->hWnd, lpObject->hCtl, 0L, 0L};
  short nIndex;
  short nScaleMode;   
  nIndex = fpGetPropIndex(&fpTemp,(LPSTR)"ScaleMode"); 
  if (nIndex == -1)
  {
    fpTemp.hCtl = (HCTL)fpVBGetControl(&fpTemp, GC_CONTAINER);
    if (fpTemp.hCtl)
      nIndex = fpGetPropIndex((LPFPCONTROL)&fpTemp,(LPSTR)"ScaleMode"); 
  }

  if (nIndex == -1 ||
      fpVBGetControlProperty(&fpTemp, nIndex, (LPVOID)&nScaleMode,
                               "ScaleMode", VT_I2) == (ERR)-1)
      //parent does not have a ScaleMode property, return twips
    lpScaleMode->nScaleMode = 1;
  else
    lpScaleMode->nScaleMode  = (BYTE)nScaleMode;  

#endif
  return TRUE;
}

//fpVBGetScaleModeEx NOTE:
// An assumption was made incorrectly:
// If a control had a ScaleMode property, it was assumed its
//      Height, Left, Top and Width properties would be in those units.
// All objects Height, Left, Top and Width properties are in the units
//      of its container, whether a control or form.
// The bContainerOnly parameter was added to adjust for this oversight.
// Set bContainerOnly = FALSE to make the function work like it used to.
// Set bContainerOnly = TRUE to make the function work correctly.
// In the case where the container does not support the ScaleMode property,
// the scale mode was defaulting to twips.  In most cases it should instead
// default to pixels.
BOOL FPLIB fpVBGetScaleModeEx(LPFPCONTROL lpObject, LPSCALEMODE lpScaleMode, BOOL bContainerOnly, BOOL fDefaultToPixels)
{
#ifdef FP_OCX
  LONG lpDispParent = 0;
  FPCONTROL fpTemp = {0, 0, 0L, lpObject->lpDispatch};
  SCALEMODE scale = {-1, -1, -1, (float)-1, (float)-1, (float)-1, (float)-1};

  // OCX - only function
  if (bContainerOnly)
     {
     fpTemp.lpDispatch = fpGetControlDispatch(lpObject, GC_CONTAINER);
     lpDispParent = fpTemp.lpDispatch;
    
     if (fpTemp.lpDispatch == NULL ||
         fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.nScaleMode),
                               _T("ScaleMode"), VT_I2) == (ERR)-1)
        {
        //parent does not have a ScaleMode property, return twips
        if( fDefaultToPixels ) // unless fDefaultToPixels -scl
           lpScaleMode->nScaleMode = 3;
        else
           lpScaleMode->nScaleMode = 1;
        if (fpTemp.lpDispatch)
           fpDispatchRelease(fpTemp.lpDispatch);
        return TRUE;
        }
     }
  else
     {
     if (fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.nScaleMode),
                                _T("ScaleMode"), VT_I2) == (ERR)-1)
        {
        // current object doesn't have a ScaleMode property, get parent's
        fpTemp.lpDispatch = fpGetControlDispatch(lpObject, GC_CONTAINER);
        lpDispParent = fpTemp.lpDispatch;
    
        if (fpTemp.lpDispatch == NULL ||
           fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.nScaleMode),
                                  _T("ScaleMode"), VT_I2) == (ERR)-1)
        //parent does not have a ScaleMode property, return twips
           {
           if( fDefaultToPixels )
              lpScaleMode->nScaleMode = 3;
           else
              lpScaleMode->nScaleMode = 1;
           if (fpTemp.lpDispatch)
              fpDispatchRelease(fpTemp.lpDispatch);
	       return TRUE;
	       }
        }
  
     }
  lpScaleMode->nScaleMode = scale.nScaleMode;
  
  if (lpScaleMode->nScaleMode == 0)   // User defined scale mode
     {
     long lWnd;

     if (fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&lWnd,
                                _T("hWnd"), VT_I4) != (ERR)-1)
        {
        RECT RectClient;
     
        GetClientRect((HWND)lWnd, &RectClient);
        scale.lWidth = RectClient.right - RectClient.left;
        scale.lHeight = RectClient.bottom - RectClient.top;
        }
     else
        {
        fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.lHeight),
                               _T("Height"), VT_I4);
        fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.lWidth),
                               _T("Width"), VT_I4);
        scale.lWidth = XTwipsToPixels(scale.lWidth);
        scale.lHeight = YTwipsToPixels(scale.lHeight);
        }
   
     fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.fScaleTop),
                            _T("ScaleTop"), VT_R4);
     fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.fScaleLeft),
                            _T("ScaleLeft"), VT_R4);
     fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.fScaleHeight),
                            _T("ScaleHeight"), VT_R4);
     fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&(scale.fScaleWidth),
                            _T("ScaleWidth"), VT_R4);
     _fmemcpy(lpScaleMode, &scale, sizeof(SCALEMODE));                       	  
     }

  if (lpDispParent)
     fpDispatchRelease(lpDispParent);

#elif defined(FP_VB)
  FPCONTROL fpTemp = {lpObject->hWnd, lpObject->hCtl, 0L, 0L};
  short nIndex;
  short nScaleMode;   
  nIndex = fpGetPropIndex(&fpTemp,(LPSTR)"ScaleMode"); 
  if (nIndex == -1)
  {
    fpTemp.hCtl = (HCTL)fpVBGetControl(&fpTemp, GC_CONTAINER);
    if (fpTemp.hCtl)
      nIndex = fpGetPropIndex((LPFPCONTROL)&fpTemp,(LPSTR)"ScaleMode"); 
  }

  if (nIndex == -1 ||
      fpVBGetControlProperty(&fpTemp, nIndex, (LPVOID)&nScaleMode,
                               "ScaleMode", VT_I2) == (ERR)-1)
  {
     if( fDefaultToPixels )
        lpScaleMode->nScaleMode = 3;
     else
      //parent does not have a ScaleMode property, return twips
      lpScaleMode->nScaleMode = 1;
  }
  else
    lpScaleMode->nScaleMode  = (BYTE)nScaleMode;  

#endif
  return TRUE;
}

BYTE FPLIB fpVBGetRectInContainer(LPFPCONTROL lpObject, LPRECT lprect)
{
BYTE bLineOrient = 0;

#ifdef FP_OCX
#ifndef NO_MFC
  RECT tempRect;
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
    pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

  if (lpObject->lpDispatch)
  {
    FPCONTROL fpTemp = { 0, 0, 0L, 0L};
    POINT pt = { 0, 0 };

    bLineOrient = fpGetWindowRect(lpObject, lprect);

    fpTemp.lpDispatch = fpGetControlDispatch(lpObject, GC_CONTAINER);

    if (fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&fpTemp.hWnd,
        _T("hWnd"), VT_I4) != (ERR)-1)
       {
       ClientToScreen(fpTemp.hWnd, &pt);
       OffsetRect(lprect, -pt.x, -pt.y);
       }

    // RFW - 4/7/08 - 20261
    else
       {
	    fpTemp.hWnd = GetParent(fpGetParent(lpObject));
       ClientToScreen(fpTemp.hWnd, &pt);
       OffsetRect(lprect, -pt.x, -pt.y);
       }

    fpDispatchRelease(fpTemp.lpDispatch);
  }
  else if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(COleControl))) &&
      (((COleControl *)pObject)->GetRectInContainer(&tempRect)))
       ::CopyRect(lprect, &tempRect);
  else if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd)))) 
  {                  
     CWnd *pWndParent = ((CWnd *)pObject)->GetParent();
 
     fpGetWindowRect(lpObject, lprect);
     if (pWndParent)
     {
	   POINT pt;

       pt.x = 0;
       pt.y = 0;    
       pWndParent->ClientToScreen(&pt);
       ::OffsetRect(lprect, -(pt.x), -(pt.y));
     }  
  }

#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  
  HWND hWndBase = NULL;
  
  if (lpObject->hWnd)
	 hWndBase = lpObject->hWnd;
  
  if (pObject)
	 hWndBase = pObject->fpAtlGetHWnd();
  
  if (lpObject->lpDispatch)
  {
    FPCONTROL fpTemp = { 0, 0, 0L, 0L};

    bLineOrient = fpGetWindowRect(lpObject, lprect);

    fpTemp.lpDispatch = fpGetControlDispatch(lpObject, GC_CONTAINER);

    if (fpVBGetControlProperty(&fpTemp, 0, (LPVOID)&fpTemp.hWnd,
        _T("hWnd"), VT_I4) != (ERR)-1)
       {
       POINT pt;

       pt.x = 0;
       pt.y = 0;
       ClientToScreen(fpTemp.hWnd, &pt);
       OffsetRect(lprect, -pt.x, -pt.y);
       }

    fpDispatchRelease(fpTemp.lpDispatch);
  }
  else
  {                  
     HWND hWndParent = GetParent(hWndBase);
 
     fpGetWindowRect(lpObject, lprect);
     if (hWndParent)
     {
	   POINT pt;

       pt.x = 0;
       pt.y = 0;    
       ClientToScreen(hWndParent, &pt);
       ::OffsetRect(lprect, -(pt.x), -(pt.y));
     }  
  }

#endif
#else
  VBGetRectInContainer((HCTL)lpObject->hCtl, lprect);
#endif

return (bLineOrient);
} 

void FPLIB fpVBMoveControl(LPFPCONTROL lpObject, LPRECT lprect, BOOL fRepaint,
                            BYTE bLineOrient)
{
#ifdef FP_OCX
#ifndef NO_MFC
  CObject *pObject = (CObject *)lpObject->lObject;

  if (lpObject->lpDispatch)
  {
     fpSetRectFromDispatch(lpObject, lprect, bLineOrient);
	 if (fRepaint)
	   fpUpdateWindow(lpObject);
  }
  else
  {
    if (lpObject->hWnd && IsWindow((HWND)lpObject->hWnd))
      pObject = (CObject *)CWnd::FromHandle((HWND)lpObject->hWnd);

    if ((pObject) && (pObject->IsKindOf(RUNTIME_CLASS(CWnd))))
    {
       if (((CWnd *)pObject)->GetSafeHwnd())
  	   { 
	  	  if (pObject->IsKindOf(RUNTIME_CLASS(COleControl))) 
            ((COleControl *)pObject)->SetControlSize((lprect->right - lprect->left), 
                                                     (lprect->bottom - lprect->top));
          ((CWnd *)pObject)->MoveWindow(lprect, fRepaint);
	   }
    }
  }
  return;
#else			 // NO_MFC
  CfpAtlBase *pObject = (CfpAtlBase *)lpObject->lObject;  

  if (lpObject->lpDispatch)
  {
     fpSetRectFromDispatch(lpObject, lprect, bLineOrient);
	 if (fRepaint)
	   fpUpdateWindow(lpObject);
  }
  else
  {
     if (pObject)
	 {
	    pObject->fpAtlSetControlSize(lprect->right - lprect->left,
                                     lprect->bottom - lprect->top);
	 }
/*	 
	 if (hWndBase && IsWindow(hWndBase))
     {
       MoveWindow(hWndBase, lprect->left, lprect->top,
		          (lprect->right - lprect->left), 
                  (lprect->bottom - lprect->top), fRepaint);
     }
*/
  }
  return;
#endif
#else      
  VBMoveControl((HCTL)lpObject->hCtl, lprect, fRepaint);
#endif
}       
#endif

#ifdef __cplusplus
}
#endif
