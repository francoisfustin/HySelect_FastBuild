/*    PortTool v2.2     FPTOOL50.C          */

// 
// FarPoint FontTable Manager
//

#include "fptools.h"
#include "string.h"
#include "fphdc.h"
#include <tchar.h>

FPGLOBALDATAMANAGER dmFontTable = { 0 };
int dFontsInUse = 0;

BOOL FPLIB fpGetFontTable ()
{
   fpCreateGlobalData (&dmFontTable, NULL, sizeof(FPFONTENTRY));
   return TRUE;
}

BOOL FPLIB fpGetFontTableEx (HINSTANCE hInst)
{
   fpCreateGlobalData (&dmFontTable, hInst, sizeof(FPFONTENTRY));
   return TRUE;
}

HFONT FPLIB fpLockFont (FPFONT fpFont, LOGFONT FAR *lpLogFont)
{
   LPFPFONTENTRY lpFontTable;
   UINT i, fuSize = 0;
   HFONT hFont = NULL;
   
   if (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize))
      {   
      if ((UINT)fpFont < fuSize)
         {
         lpFontTable[fpFont].lastUsed = GetTickCount();
         if (lpFontTable[fpFont].hFont == NULL && dFontsInUse > 10)
            {
            FPFONT oldest = NOFONT;

            // search for font to release
            for (i=0; i<fuSize; i++)
   //            if (lpFontTable[i].hFont && lpFontTable[i].useCount == 0)
               if (lpFontTable[i].hFont)
                  if (oldest == NOFONT ||
                      lpFontTable[i].lastUsed < lpFontTable[oldest].lastUsed)
                     oldest = i;

            // delete oldest font
            if (oldest != NOFONT && oldest != fpFont)
               {
               #ifdef _DEBUG
               //DebugOutput (DBF_TRACE, "Release Font #%d (last used: %ld)",
               //   oldest, lpFontTable[oldest].lastUsed);
               OutputDebugString(_T("Release Font"));
               #endif

               dFontsInUse--;
               DeleteObject (lpFontTable[oldest].hFont);
               lpFontTable[oldest].hFont = NULL;
               }
            }

         if (lpFontTable[fpFont].hFont == NULL)
            {
            if (lpLogFont == NULL)
               lpLogFont = &lpFontTable[fpFont].LogFont;
            hFont = CreateFontIndirect (lpLogFont);
            if (hFont)
               {
               lpFontTable[fpFont].hFont = hFont;
               dFontsInUse++;
               }
            }
         else
            hFont = lpFontTable[fpFont].hFont;
         }

      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   return hFont;
}

HFONT FPLIB fpLockFontEx (HINSTANCE hInst, FPFONT fpFont, LOGFONT FAR *lpLogFont)
{
   LPFPFONTENTRY lpFontTable;
   UINT i, fuSize = 0;
   HFONT hFont = NULL;
   
   if (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize))
      {   
      if ((UINT)fpFont < fuSize)
         {
         lpFontTable[fpFont].lastUsed = GetTickCount();
         if (lpFontTable[fpFont].hFont == NULL && dFontsInUse > 10)
            {
            FPFONT oldest = NOFONT;

            // search for font to release
            for (i=0; i<fuSize; i++)
   //            if (lpFontTable[i].hFont && lpFontTable[i].useCount == 0)
               if (lpFontTable[i].hFont)
                  if (oldest == NOFONT ||
                      lpFontTable[i].lastUsed < lpFontTable[oldest].lastUsed)
                     oldest = i;

            // delete oldest font
            if (oldest != NOFONT && oldest != fpFont)
               {
               #ifdef _DEBUG
               //DebugOutput (DBF_TRACE, "Release Font #%d (last used: %ld)",
               //   oldest, lpFontTable[oldest].lastUsed);
               OutputDebugString(_T("Release Font"));
               #endif

               dFontsInUse--;
               DeleteObject (lpFontTable[oldest].hFont);
               lpFontTable[oldest].hFont = NULL;
               }
            }

         if (lpFontTable[fpFont].hFont == NULL)
            {
            if (lpLogFont == NULL)
               lpLogFont = &lpFontTable[fpFont].LogFont;
            hFont = CreateFontIndirect (lpLogFont);
            if (hFont)
               {
               lpFontTable[fpFont].hFont = hFont;
               dFontsInUse++;
			   // update LogFont struct
			   GetObject(hFont, sizeof(LOGFONT), (LPVOID)&lpFontTable[fpFont].LogFont);
               }
            }
         else
            hFont = lpFontTable[fpFont].hFont;
         }

      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   return hFont;
}

FPFONT FPLIB fpGetLogFont (LOGFONT FAR *lpLogFont)
{
  return fpGetLogFontPrim (lpLogFont, NULL);
}

FPFONT FPLIB fpGetLogFontEx (HINSTANCE hInst, LOGFONT FAR *lpLogFont)
{
  return fpGetLogFontPrimEx (hInst, lpLogFont, NULL);
}

FPFONT FPLIB fpGetLogFontPrim(LOGFONT FAR *lpLogFont, HDC hdc)
{
return (fp_GetLogFontPrim(lpLogFont, hdc, NULL));
}

FPFONT FPLIB fpGetLogFontPrimEx(HINSTANCE hInst, LOGFONT FAR *lpLogFont, HDC hdc)
{
return (fp_GetLogFontPrimEx(hInst, lpLogFont, hdc, NULL));
}

FPFONT FPLIB fp_GetLogFontPrim(LOGFONT FAR *lpLogFont, HDC hdc,
                                  HFONT FAR *lphFont)
{
   LPFPFONTENTRY lpFontTable;
   FPFONT fpFont = NOFONT;
   UINT i, fuSize = 0;
   UINT fuEntryNew = (UINT)-1;
   TEXTMETRIC tm;
   HFONT oldFont;
   HFONT hFont = 0;
   HDC tmpHdc;
   
   if (!lpLogFont)
      return NOFONT;
   
   if (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize))
      {   
      // search for font in table
      for (i=0; i<fuSize; i++)
         if (lpFontTable[i].useCount == 0)
            {
            if (fuEntryNew == (UINT)-1)
               fuEntryNew = i;
            }
         else if (lstrcmp(lpFontTable[i].LogFont.lfFaceName, lpLogFont->lfFaceName) == 0 &&
                  _fmemcmp (&lpFontTable[i].LogFont, lpLogFont, sizeof(LOGFONT)-LF_FACESIZE) == 0)
            {
            fpFont = i;
            break;
            }
            
      fpUnlockGlobalData (&dmFontTable, NULL);
      }
      
   if (fpFont == NOFONT)
      {
      // add entry
      if (fuEntryNew == (UINT)-1)
         {
         i = fuSize;
         fuSize++;
         }
      else
         i = fuEntryNew;
      
      // initialize new entry
      if (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize))
         {
         _fmemcpy(&lpFontTable[i].LogFont, lpLogFont, sizeof(LOGFONT));
         lpFontTable[i].hFont = NULL;
         fpLockFont ((FPFONT)i, lpLogFont);
      
         // get char widths
         tmpHdc = hdc;
         if (!tmpHdc)
           tmpHdc = fpGetDC(NULL);
         oldFont = SelectObject(tmpHdc, lpFontTable[i].hFont);
         GetTextMetrics(tmpHdc, &tm);

#ifndef _UNICODE // RFW - 12/27/05 - 17903
			{
		   int FAR *lpCharWidths;
         UINT nCh, j;

         nCh = max(tm.tmLastChar,255);
         lpFontTable[i].hCharWidths = GlobalAlloc(GSHARE, sizeof(int)*(nCh + 1));
         lpCharWidths = (int FAR *)GlobalLock(lpFontTable[i].hCharWidths);
         if (!lpCharWidths) { fpUnlockGlobalData (&dmFontTable, NULL); return NOFONT; }
      
         GetCharWidth(tmpHdc, 0, nCh, lpCharWidths);
			if (tm.tmOverhang)
	         for (j=0; j <= nCh; j++)	
	            lpCharWidths[j] -= tm.tmOverhang;

         GlobalUnlock(lpFontTable[i].hCharWidths);
			}
#endif //_UNICODE
         
         _fmemcpy(&lpFontTable[i].tm, &tm, sizeof(TEXTMETRIC));
         SelectObject(tmpHdc, oldFont);
         if (!hdc)
           ReleaseDC(NULL, tmpHdc);
      
         hFont = lpFontTable[i].hFont;
         fpUnlockGlobalData (&dmFontTable, NULL);
         fpFont = i;
         }
      }
   else
      hFont = fpLockFont (fpFont, NULL);

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {
      lpFontTable[fpFont].useCount++;
      fpUnlockGlobalData (&dmFontTable, NULL);
      }

   if (lphFont)
      *lphFont = hFont;
   
   return (fpFont);
}

FPFONT FPLIB fp_GetLogFontPrimEx(HINSTANCE hInst, LOGFONT FAR *lpLogFont, HDC hdc,
                                  HFONT FAR *lphFont)
{
   LPFPFONTENTRY lpFontTable;
   FPFONT fpFont = NOFONT;
   int FAR *lpCharWidths;
   UINT i, j, fuSize = 0;
   UINT fuEntryNew = (UINT)-1;
   TEXTMETRIC tm;
   HFONT oldFont;
   HFONT hFont = 0;
   HDC tmpHdc;
   
   if (!lpLogFont)
      return NOFONT;
   
   if (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize))
      {   
      // search for font in table
      for (i=0; i<fuSize; i++)
         if (lpFontTable[i].useCount == 0)
            {
            if (fuEntryNew == (UINT)-1)
               fuEntryNew = i;
            }
         else if (lstrcmp(lpFontTable[i].LogFont.lfFaceName, lpLogFont->lfFaceName) == 0 &&
                  _fmemcmp (&lpFontTable[i].LogFont, lpLogFont, sizeof(LOGFONT)-LF_FACESIZE) == 0)
            {
            fpFont = i;
            break;
            }
            
      fpUnlockGlobalData (&dmFontTable, hInst);
      }
      
   if (fpFont == NOFONT)
      {
      // add entry
      if (fuEntryNew == (UINT)-1)
         {
         i = fuSize;
         fuSize++;
         }
      else
         i = fuEntryNew;
      
      // initialize new entry
      if (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize))
         {
         UINT nCh;

         _fmemcpy(&lpFontTable[i].LogFont, lpLogFont, sizeof(LOGFONT));
         lpFontTable[i].hFont = NULL;
         fpLockFontEx (hInst, (FPFONT)i, lpLogFont);
      
         // get char widths
         tmpHdc = hdc;
         if (!tmpHdc)
           tmpHdc = fpGetDC(NULL);
         oldFont = SelectObject(tmpHdc, lpFontTable[i].hFont);
         GetTextMetrics(tmpHdc, &tm);
         nCh = max(tm.tmLastChar,255);
         lpFontTable[i].hCharWidths = GlobalAlloc(GSHARE, sizeof(int)*(nCh + 1));
         lpCharWidths = (int FAR *)GlobalLock(lpFontTable[i].hCharWidths);
         if (!lpCharWidths) { fpUnlockGlobalData (&dmFontTable, NULL); return NOFONT; }
      
         GetCharWidth(tmpHdc, 0, nCh, lpCharWidths);
         for (j=0; j <= nCh; j++)	
            lpCharWidths[j] -= tm.tmOverhang;
         
         _fmemcpy(&lpFontTable[i].tm, &tm, sizeof(TEXTMETRIC));
            
         GlobalUnlock(lpFontTable[i].hCharWidths);
         SelectObject(tmpHdc, oldFont);
         if (!hdc)
           ReleaseDC(NULL, tmpHdc);
      
         hFont = lpFontTable[i].hFont;
         fpUnlockGlobalData (&dmFontTable, hInst);
         fpFont = i;
         }
      }
   else
      hFont = fpLockFontEx (hInst, fpFont, NULL);

   if (fpFont != NOFONT && (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {
      lpFontTable[fpFont].useCount++;
      fpUnlockGlobalData (&dmFontTable, hInst);
      }

   if (lphFont)
      *lphFont = hFont;
   
   return (fpFont);
}

FPFONT FPLIB fpGetFont (HFONT hFont)
{
   LOGFONT LogFont;

   if (!hFont)
      hFont = GetStockObject(SYSTEM_FONT);

   GetObject (hFont, sizeof(LOGFONT), &LogFont);

   return fpGetLogFont(&LogFont);
}

FPFONT FPLIB fpGetFontEx (HINSTANCE hInst, HFONT hFont)
{
   LOGFONT LogFont;

   if (!hFont)
      hFont = GetStockObject(SYSTEM_FONT);

   GetObject (hFont, sizeof(LOGFONT), &LogFont);

   return fpGetLogFontEx(hInst, &LogFont);
}

FPFONT FPLIB fpGetFontFromHDC (HFONT hFont, HDC hdc)
{
   LOGFONT LogFont;
   HFONT tmpFont = hFont;
   FPFONT ret;
   
   if (!tmpFont)
      tmpFont = SelectObject(hdc, GetStockObject(SYSTEM_FONT));

   GetObject (tmpFont, sizeof(LOGFONT), &LogFont);

   ret = fpGetLogFontPrim(&LogFont, hdc);
   
   if (!hFont)
     SelectObject(hdc, tmpFont);
     
   return ret;
}

FPFONT FPLIB fpGetFontFromHDCEx (HINSTANCE hInst, HFONT hFont, HDC hdc)
{
   LOGFONT LogFont;
   HFONT tmpFont = hFont;
   FPFONT ret;
   
   if (!tmpFont)
      tmpFont = SelectObject(hdc, GetStockObject(SYSTEM_FONT));

   GetObject (tmpFont, sizeof(LOGFONT), &LogFont);

   ret = fpGetLogFontPrimEx(hInst, &LogFont, hdc);
   
   if (!hFont)
     SelectObject(hdc, tmpFont);
     
   return ret;
}


// Callback
//
void DestroyFontTable (GLOBALHANDLE hTable, UINT fuSize)
{
   LPFPFONTENTRY lpFontTable;
   UINT i;
   
   if (hTable && (lpFontTable = (LPFPFONTENTRY)GlobalLock (hTable)))
      {
      for (i=0; i<fuSize; i++)
         {
         if (lpFontTable[i].hFont)
            DeleteObject(lpFontTable[i].hFont);
         lpFontTable[i].hFont = NULL;
         if (lpFontTable[i].hCharWidths)
            GlobalFree(lpFontTable[i].hCharWidths);
         lpFontTable[i].hCharWidths = NULL;
         }
      GlobalUnlock (hTable);
      dFontsInUse = 0;
      }
   return;
}

// Must be called by each object that uses the font table
//
void FPLIB fpReleaseFontTable ()
{
   fpDestroyGlobalData (&dmFontTable, NULL, DestroyFontTable);
   return;
}

void FPLIB fpReleaseFontTableEx (HINSTANCE hInst)
{
   fpDestroyGlobalData (&dmFontTable, hInst, DestroyFontTable);
   return;
}

void FPLIB fpReleaseFont (FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;
   UINT fuSize = 0;

   if (fpFont != NOFONT &&
       (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {                    
      if (lpFontTable[fpFont].useCount > 0)                        //AXXX
         {                                                         //AXXX
         lpFontTable[fpFont].useCount--;                           //AXXX
   
         if (lpFontTable[fpFont].useCount == 0)
            {

            if (lpFontTable[fpFont].hFont)
               {
               dFontsInUse--;
               DeleteObject(lpFontTable[fpFont].hFont);
               lpFontTable[fpFont].hFont = NULL;
               }
            if (lpFontTable[fpFont].hCharWidths)
               GlobalFree(lpFontTable[fpFont].hCharWidths);
            lpFontTable[fpFont].hCharWidths = NULL;
            }
         }

      fpUnlockGlobalData (&dmFontTable, NULL);
      }
}

void FPLIB fpReleaseFontEx (HINSTANCE hInst, FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;
   UINT fuSize = 0;

   if (fpFont != NOFONT &&
       (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {                    
      if (lpFontTable[fpFont].useCount > 0)                        //AXXX
         {                                                         //AXXX
         lpFontTable[fpFont].useCount--;                           //AXXX
   
         if (lpFontTable[fpFont].useCount == 0)
            {

            if (lpFontTable[fpFont].hFont)
               {
               dFontsInUse--;
               DeleteObject(lpFontTable[fpFont].hFont);
               lpFontTable[fpFont].hFont = NULL;
               }
            if (lpFontTable[fpFont].hCharWidths)
               GlobalFree(lpFontTable[fpFont].hCharWidths);
            lpFontTable[fpFont].hCharWidths = NULL;
            }
         }

      fpUnlockGlobalData (&dmFontTable, hInst);
      }
}

void FPLIB fpRefFont (FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;
   UINT fuSize = 0;

   if (fpFont != NOFONT &&
       (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize)))
      {                    
      if (lpFontTable[fpFont].useCount > 0)
         lpFontTable[fpFont].useCount++;

      fpUnlockGlobalData (&dmFontTable, NULL);
      }
}

void FPLIB fpRefFontEx (HINSTANCE hInst, FPFONT fpFont)
{
   LPFPFONTENTRY lpFontTable;
   UINT fuSize = 0;

   if (fpFont != NOFONT &&
       (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize)))
      {                    
      if (lpFontTable[fpFont].useCount > 0)
         lpFontTable[fpFont].useCount++;

      fpUnlockGlobalData (&dmFontTable, hInst);
      }
}

/****************
* Color Manager
****************/

#define FPCOLORMAN_THRESHOLD 10

FPGLOBALDATAMANAGER dmColorTable = { 0 };
short               nColorMan_BrushRefCnt = 0;
short               nColorMan_PenRefCnt = 0;

void fpxColorManFree(GLOBALHANDLE hTable, UINT fuSize);

typedef struct fpColorItem
{
COLORREF clr;
HBRUSH   hBrush;
HPEN     hPen;
LONG     lLastUsedBrush;
LONG     lLastUsedPen;
LONG     lRefCnt;
short    nLocked;
} FPCOLORITEM, FAR * LPFPCOLORITEM;


BOOL FPLIB fpCM_Init(void)
{
fpCreateGlobalData(&dmColorTable, NULL, sizeof(FPCOLORITEM));
return (TRUE);
}

BOOL FPLIB fpCM_InitEx(HINSTANCE hInst)
{
fpCreateGlobalData(&dmColorTable, hInst, sizeof(FPCOLORITEM));
return (TRUE);
}

void FPLIB fpCM_Free(void)
{
fpDestroyGlobalData(&dmColorTable, NULL, fpxColorManFree);
}

void FPLIB fpCM_FreeEx(HINSTANCE hInst)
{
fpDestroyGlobalData(&dmColorTable, hInst, fpxColorManFree);
}

void fpxColorManFree(GLOBALHANDLE hTable, UINT fuSize)
{
LPFPCOLORITEM lpColorTable;
UINT          i;

if (hTable && (lpColorTable = (LPFPCOLORITEM)GlobalLock(hTable)))
   {
   for (i = 0; i < fuSize; i++)
      {
      if (lpColorTable[i].hBrush)
         DeleteObject(lpColorTable[i].hBrush);

      if (lpColorTable[i].hPen)
         DeleteObject(lpColorTable[i].hPen);

      _fmemset(&lpColorTable[i], '\0', sizeof(FPCOLORITEM));
      }

   GlobalUnlock (hTable);
   }
}


FPCOLORID FPLIB fpCM_AddItem(COLORREF clr)
{
LPFPCOLORITEM lpColorTable;
FPCOLORID     fpColorID = -1;
UINT          uSize = 0;
UINT          i;

if (clr == RGBCOLOR_DEFAULT)
   return (0);

if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
   {
   // search for color in table
   for (i = 0; i < uSize; i++)
      if (lpColorTable[i].clr == clr && lpColorTable[i].lRefCnt)
         {
         lpColorTable[i].lRefCnt++;
         fpColorID = i;
         break;
         }

   fpUnlockGlobalData(&dmColorTable, NULL);
   }

if (fpColorID == -1)
   {
   if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
      {
      for (i = 0; i < uSize; i++)
         if (!lpColorTable[i].lRefCnt)
            {
            fpColorID = i;
            break;
            }

      fpUnlockGlobalData(&dmColorTable, NULL);
      }

   if (fpColorID == -1)
      {
      fpColorID = uSize;
      uSize++;
      }

   // Initialize new entry
   if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
      {
      _fmemset(&lpColorTable[fpColorID], '\0', sizeof(FPCOLORITEM));
      lpColorTable[fpColorID].lRefCnt++;
      lpColorTable[fpColorID].clr = clr;
      fpUnlockGlobalData(&dmColorTable, NULL);
      }
   }

return (fpColorID + 1);
}

FPCOLORID FPLIB fpCM_AddItemEx(HINSTANCE hInst, COLORREF clr)
{
LPFPCOLORITEM lpColorTable;
FPCOLORID     fpColorID = -1;
UINT          uSize = 0;
UINT          i;

if (clr == RGBCOLOR_DEFAULT)
   return (0);

if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
   {
   // search for color in table
   for (i = 0; i < uSize; i++)
      if (lpColorTable[i].clr == clr && lpColorTable[i].lRefCnt)
         {
         lpColorTable[i].lRefCnt++;
         fpColorID = i;
         break;
         }

   fpUnlockGlobalData(&dmColorTable, hInst);
   }

if (fpColorID == -1)
   {
   if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
      {
      for (i = 0; i < uSize; i++)
         if (!lpColorTable[i].lRefCnt)
            {
            fpColorID = i;
            break;
            }

      fpUnlockGlobalData(&dmColorTable, hInst);
      }

   if (fpColorID == -1)
      {
      fpColorID = uSize;
      uSize++;
      }

   // Initialize new entry
   if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
      {
      _fmemset(&lpColorTable[fpColorID], '\0', sizeof(FPCOLORITEM));
      lpColorTable[fpColorID].lRefCnt++;
      lpColorTable[fpColorID].clr = clr;
      fpUnlockGlobalData(&dmColorTable, hInst);
      }
   }

return (fpColorID + 1);
}

void FPLIB fpCM_FreeItem(FPCOLORID fpColorID)
{
if (fpColorID > 0)
   {
   LPFPCOLORITEM lpColorTable;
   UINT          uSize = 0;

   fpColorID--;

   if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
      {
      if (fpColorID < (FPCOLORID)uSize && lpColorTable[fpColorID].lRefCnt)
         {
         lpColorTable[fpColorID].lRefCnt--;
         if (lpColorTable[fpColorID].lRefCnt == 0)
            {
            if (lpColorTable[fpColorID].hBrush)
               {
               DeleteObject(lpColorTable[fpColorID].hBrush);
               nColorMan_BrushRefCnt--;
               }

            if (lpColorTable[fpColorID].hPen)
               {
               DeleteObject(lpColorTable[fpColorID].hPen);
               nColorMan_PenRefCnt--;
               }

            _fmemset(&lpColorTable[fpColorID], '\0', sizeof(FPCOLORITEM));
            }
         }

      fpUnlockGlobalData(&dmColorTable, NULL);
      }
   }
}

void FPLIB fpCM_FreeItemEx(HINSTANCE hInst, FPCOLORID fpColorID)
{
if (fpColorID > 0)
   {
   LPFPCOLORITEM lpColorTable;
   UINT          uSize = 0;

   fpColorID--;

   if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
      {
      if (fpColorID < (FPCOLORID)uSize && lpColorTable[fpColorID].lRefCnt)
         {
         lpColorTable[fpColorID].lRefCnt--;
         if (lpColorTable[fpColorID].lRefCnt == 0)
            {
            if (lpColorTable[fpColorID].hBrush)
               {
               DeleteObject(lpColorTable[fpColorID].hBrush);
               nColorMan_BrushRefCnt--;
               }

            if (lpColorTable[fpColorID].hPen)
               {
               DeleteObject(lpColorTable[fpColorID].hPen);
               nColorMan_PenRefCnt--;
               }

            _fmemset(&lpColorTable[fpColorID], '\0', sizeof(FPCOLORITEM));
            }
         }

      fpUnlockGlobalData(&dmColorTable, hInst);
      }
   }
}

void FPLIB fpCM_RefItem(FPCOLORID fpColorID)
{
LPFPCOLORITEM lpColorTable;
UINT          uSize = 0;

if (fpColorID > 0)
   {
   fpColorID--;

   if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
      {
      if (fpColorID < (FPCOLORID)uSize && lpColorTable[fpColorID].lRefCnt)
         lpColorTable[fpColorID].lRefCnt++;

      fpUnlockGlobalData(&dmColorTable, NULL);
      }
   }
}

void FPLIB fpCM_RefItemEx(HINSTANCE hInst, FPCOLORID fpColorID)
{
LPFPCOLORITEM lpColorTable;
UINT          uSize = 0;

if (fpColorID > 0)
   {
   fpColorID--;

   if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
      {
      if (fpColorID < (FPCOLORID)uSize && lpColorTable[fpColorID].lRefCnt)
         lpColorTable[fpColorID].lRefCnt++;

      fpUnlockGlobalData(&dmColorTable, hInst);
      }
   }
}

void FPLIB fpCM_ChangeItem(LPFPCOLORID lpfpColorID, COLORREF clr)
{
if (lpfpColorID)
   {
   if (*lpfpColorID > 0)
      fpCM_FreeItem(*lpfpColorID);

   *lpfpColorID = fpCM_AddItem(clr);
   }
}

void FPLIB fpCM_ChangeItemEx(HINSTANCE hInst, LPFPCOLORID lpfpColorID, COLORREF clr)
{
if (lpfpColorID)
   {
   if (*lpfpColorID > 0)
      fpCM_FreeItemEx(hInst, *lpfpColorID);

   *lpfpColorID = fpCM_AddItemEx(hInst, clr);
   }
}

COLORREF FPLIB fpCM_GetItem(FPCOLORID fpColorID, LPHANDLE lphBrush,
                                    LPHANDLE lphPen)
{
COLORREF clr = RGBCOLOR_DEFAULT;

if (fpColorID > 0)
   {
   LPFPCOLORITEM lpColorTable;
   UINT          uSize = 0;

   fpColorID--;

   if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
      {
      if (fpColorID < (FPCOLORID)uSize && lpColorTable[fpColorID].lRefCnt)
         {
         clr = lpColorTable[fpColorID].clr;

         if (lphBrush)
            {
            if (!lpColorTable[fpColorID].hBrush)
               {
               if (nColorMan_BrushRefCnt >= FPCOLORMAN_THRESHOLD)
                  {
                  FPCOLORID oldest = -1;
                  FPCOLORID i;

                  // search for brush to release
                  for (i = 0; i < (FPCOLORID)uSize; i++)
                     if (lpColorTable[i].hBrush)
                        if ((oldest == -1 ||
                             lpColorTable[i].lLastUsedBrush <
                             lpColorTable[oldest].lLastUsedBrush) &&
                             !lpColorTable[i].nLocked)
                           oldest = i;

                  // Delete oldest Brush
                  if (oldest != (FPCOLORID)-1)
                     {
                     DeleteObject(lpColorTable[oldest].hBrush);
                     lpColorTable[oldest].hBrush = 0;
                     nColorMan_BrushRefCnt--;
                     }
                  }

               lpColorTable[fpColorID].hBrush =
                  CreateSolidBrush(FPCOLOR(lpColorTable[fpColorID].clr));
               nColorMan_BrushRefCnt++;
               }

            lpColorTable[fpColorID].lLastUsedBrush = GetTickCount();
            *lphBrush = lpColorTable[fpColorID].hBrush;
            }

         if (lphPen)
            {
            if (!lpColorTable[fpColorID].hPen)
               {
               if (nColorMan_PenRefCnt >= FPCOLORMAN_THRESHOLD)
                  {
                  FPCOLORID oldest = -1;
                  FPCOLORID i;

                  // search for brush to release
                  for (i = 0; i < (FPCOLORID)uSize; i++)
                     if (lpColorTable[i].hPen && !lpColorTable[oldest].nLocked)
                        if (oldest == (FPCOLORID)-1 ||
                            lpColorTable[i].lLastUsedPen <
                            lpColorTable[oldest].lLastUsedPen)
                           oldest = i;

                  // Delete oldest Pen
                  if (oldest != (FPCOLORID)-1)
                     {
                     DeleteObject(lpColorTable[oldest].hPen);
                     lpColorTable[oldest].hPen = 0;
                     nColorMan_PenRefCnt--;
                     }
                  }

               lpColorTable[fpColorID].hPen =
                  CreatePen(PS_SOLID, 1, FPCOLOR(lpColorTable[fpColorID].clr));
               nColorMan_PenRefCnt++;
               }

            lpColorTable[fpColorID].lLastUsedPen = GetTickCount();
            *lphPen = lpColorTable[fpColorID].hPen;
            }
         }

      fpUnlockGlobalData(&dmColorTable, NULL);
      }
   }

return (clr);
}

COLORREF FPLIB fpCM_GetItemEx(HINSTANCE hInst, FPCOLORID fpColorID, LPHANDLE lphBrush,
                                    LPHANDLE lphPen)
{
COLORREF clr = RGBCOLOR_DEFAULT;

if (fpColorID > 0)
   {
   LPFPCOLORITEM lpColorTable;
   UINT          uSize = 0;

   fpColorID--;

   if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
      {
      if (fpColorID < (FPCOLORID)uSize && lpColorTable[fpColorID].lRefCnt)
         {
         clr = lpColorTable[fpColorID].clr;

         if (lphBrush)
            {
            if (!lpColorTable[fpColorID].hBrush)
               {
               if (nColorMan_BrushRefCnt >= FPCOLORMAN_THRESHOLD)
                  {
                  FPCOLORID oldest = -1;
                  FPCOLORID i;

                  // search for brush to release
                  for (i = 0; i < (FPCOLORID)uSize; i++)
                     if (lpColorTable[i].hBrush)
                        if ((oldest == -1 ||
                             lpColorTable[i].lLastUsedBrush <
                             lpColorTable[oldest].lLastUsedBrush) &&
                             !lpColorTable[i].nLocked)
                           oldest = i;

                  // Delete oldest Brush
                  if (oldest != (FPCOLORID)-1)
                     {
                     DeleteObject(lpColorTable[oldest].hBrush);
                     lpColorTable[oldest].hBrush = 0;
                     nColorMan_BrushRefCnt--;
                     }
                  }

               lpColorTable[fpColorID].hBrush =
                  CreateSolidBrush(FPCOLOR(lpColorTable[fpColorID].clr));
               nColorMan_BrushRefCnt++;
               }

            lpColorTable[fpColorID].lLastUsedBrush = GetTickCount();
            *lphBrush = lpColorTable[fpColorID].hBrush;
            }

         if (lphPen)
            {
            if (!lpColorTable[fpColorID].hPen)
               {
               if (nColorMan_PenRefCnt >= FPCOLORMAN_THRESHOLD)
                  {
                  FPCOLORID oldest = -1;
                  FPCOLORID i;

                  // search for brush to release
                  for (i = 0; i < (FPCOLORID)uSize; i++)
                     if (lpColorTable[i].hPen && !lpColorTable[oldest].nLocked)
                        if (oldest == (FPCOLORID)-1 ||
                            lpColorTable[i].lLastUsedPen <
                            lpColorTable[oldest].lLastUsedPen)
                           oldest = i;

                  // Delete oldest Pen
                  if (oldest != (FPCOLORID)-1)
                     {
                     DeleteObject(lpColorTable[oldest].hPen);
                     lpColorTable[oldest].hPen = 0;
                     nColorMan_PenRefCnt--;
                     }
                  }

               lpColorTable[fpColorID].hPen =
                  CreatePen(PS_SOLID, 1, FPCOLOR(lpColorTable[fpColorID].clr));
               nColorMan_PenRefCnt++;
               }

            lpColorTable[fpColorID].lLastUsedPen = GetTickCount();
            *lphPen = lpColorTable[fpColorID].hPen;
            }
         }

      fpUnlockGlobalData(&dmColorTable, hInst);
      }
   }

return (clr);
}


COLORREF FPLIB fpCM_LockItem(FPCOLORID fpColorID, LPHANDLE lphBrush,
                                LPHANDLE lphPen)
{
LPFPCOLORITEM lpColorTable;
COLORREF      clr;
UINT          uSize = 0;

clr = fpCM_GetItem(fpColorID, lphBrush, lphPen);

fpColorID--;

if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
   {
   if (fpColorID >= 0 && fpColorID < (FPCOLORID)uSize &&
       lpColorTable[fpColorID].lRefCnt)
      lpColorTable[fpColorID].nLocked++;

   fpUnlockGlobalData(&dmColorTable, NULL);
   }

return (clr);
}

COLORREF FPLIB fpCM_LockItemEx(HINSTANCE hInst, FPCOLORID fpColorID, LPHANDLE lphBrush,
                                LPHANDLE lphPen)
{
LPFPCOLORITEM lpColorTable;
COLORREF      clr;
UINT          uSize = 0;

clr = fpCM_GetItemEx(hInst, fpColorID, lphBrush, lphPen);

fpColorID--;

if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
   {
   if (fpColorID >= 0 && fpColorID < (FPCOLORID)uSize &&
       lpColorTable[fpColorID].lRefCnt)
      lpColorTable[fpColorID].nLocked++;

   fpUnlockGlobalData(&dmColorTable, hInst);
   }

return (clr);
}


void FPLIB fpCM_UnlockItem(FPCOLORID fpColorID)
{
LPFPCOLORITEM lpColorTable;
UINT          uSize = 0;

fpColorID--;

if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
   {
   if (fpColorID >= 0 && fpColorID < (FPCOLORID)uSize &&
       lpColorTable[fpColorID].lRefCnt &&
       lpColorTable[fpColorID].nLocked)
      lpColorTable[fpColorID].nLocked--;

   fpUnlockGlobalData(&dmColorTable, NULL);
   }
}

void FPLIB fpCM_UnlockItemEx(HINSTANCE hInst, FPCOLORID fpColorID)
{
LPFPCOLORITEM lpColorTable;
UINT          uSize = 0;

fpColorID--;

if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
   {
   if (fpColorID >= 0 && fpColorID < (FPCOLORID)uSize &&
       lpColorTable[fpColorID].lRefCnt &&
       lpColorTable[fpColorID].nLocked)
      lpColorTable[fpColorID].nLocked--;

   fpUnlockGlobalData(&dmColorTable, hInst);
   }
}

UINT FPLIB fpCM_GetCount(void)
{
UINT uSize = 0;

if (fpLockGlobalData(&dmColorTable, NULL, &uSize))
   fpUnlockGlobalData(&dmColorTable, NULL);

return (uSize);
}

UINT FPLIB fpCM_GetCountEx(HINSTANCE hInst)
{
UINT uSize = 0;

if (fpLockGlobalData(&dmColorTable, hInst, &uSize))
   fpUnlockGlobalData(&dmColorTable, hInst);

return (uSize);
}


void FPLIB fpCM_SysColorChange(void)
{
LPFPCOLORITEM lpColorTable;
UINT          uSize = 0;
UINT          i;

if (lpColorTable = fpLockGlobalData(&dmColorTable, NULL, &uSize))
   {
   for (i = 0; i < uSize; i++)
      {
      if (lpColorTable[i].clr & 0x80000000)
         {
         if (lpColorTable[i].hBrush)
            {
            DeleteObject(lpColorTable[i].hBrush);
            lpColorTable[i].hBrush = 0;
            }

         if (lpColorTable[i].hPen)
            {
            DeleteObject(lpColorTable[i].hPen);
            lpColorTable[i].hPen = 0;
            }
         }
      }

   fpUnlockGlobalData(&dmColorTable, NULL);
   }
}

void FPLIB fpCM_SysColorChangeEx(HINSTANCE hInst)
{
LPFPCOLORITEM lpColorTable;
UINT          uSize = 0;
UINT          i;

if (lpColorTable = fpLockGlobalData(&dmColorTable, hInst, &uSize))
   {
   for (i = 0; i < uSize; i++)
      {
      if (lpColorTable[i].clr & 0x80000000)
         {
         if (lpColorTable[i].hBrush)
            {
            DeleteObject(lpColorTable[i].hBrush);
            lpColorTable[i].hBrush = 0;
            }

         if (lpColorTable[i].hPen)
            {
            DeleteObject(lpColorTable[i].hPen);
            lpColorTable[i].hPen = 0;
            }
         }
      }

   fpUnlockGlobalData(&dmColorTable, hInst);
   }
}

