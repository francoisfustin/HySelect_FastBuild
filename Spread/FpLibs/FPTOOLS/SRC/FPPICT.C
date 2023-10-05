#define OEMRESOURCE

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "fptools.h"
#include "fpmemmgr.h"
#include "fphuge.h"

#ifndef _WIN32
typedef struct _ICONINFO { // ii 
   BOOL    fIcon; 
   DWORD   xHotspot; 
   DWORD   yHotspot; 
   HBITMAP hbmMask; 
   HBITMAP hbmColor; 
} ICONINFO; 
#endif

void     fpxPictManFree(GLOBALHANDLE hTable, UINT fuSize);
BOOL     fpPMx_ItemInTable(HANDLE hInst, LPFP_PICT lpPict, HPBYTE lpMem, long lMemLen, BYTE bCtlType, FPPICTID *lpID);
FPPICTID fpPMx_AddItem(HANDLE hInst, LPFP_PICT lpPict);
void     fpPMx_ChangeItem(HANDLE hInst, LPFPPICTID lpidPict, LPFP_PICT lpPict);
void     fpPictFree(LPFP_PICT lpPict);
HANDLE   fpPictGet(LPFP_PICT lpPict, BYTE bCtlType, LPARAM lParam);
BOOL     fpPictSet(HWND hWnd, LPFP_PICT lpPict, BYTE bCtlType, LPARAM lParam);
BOOL     fpPictLoad(LPFP_PICT lpPict, HPBYTE lpMem, long lMemLen, BYTE bCtlType);
HGLOBAL  fpPictSave(LPFP_PICT lpPict, LPLONG lplSize);
HBITMAP  fpPictLoadBmp(HPBYTE lpMem, long lMemLen, HPALETTE FAR *lphPal);
HGLOBAL  fpPictSaveBmp(HBITMAP hBmp, HPALETTE hPal, LPLONG lplSize,
                       BOOL fSavePictType, BOOL fIconInverted);
HICON    fpPictLoadIcon(HPBYTE lpMem, long lMemLen);
HGLOBAL  fpPictSaveIcon(HICON hIcon, LPLONG lplSize, BOOL fSavePictType);
HMETAFILE fpPictLoadWMF(HPBYTE lpMem, long lMemLen, LPLONG lpXExt, LPLONG lpYExt);
HGLOBAL  fpPictSaveWMF(HMETAFILE hwmf, LONG lWidth, LONG lHeight, LPLONG lplSize,
                       BOOL fSavePictType);
void     fpGetIconInfo(HICON hIcon, ICONINFO FAR * lpIconInfo);
HBITMAP FPLIB fpIconToBitmap2(HICON hIcon, COLORREF Color);

static long fpPaletteSize(int nBitCount);

long fpGetIconHeight(HICON hIcon);
long fpGetIconWidth(HICON hIcon);

typedef struct tagFP_PICTTBLITEM
   {
   long    lRefCnt;
   FP_PICT Pict;
   } FP_PICTTBLITEM, FAR *LPFP_PICTTBLITEM;

FPGLOBALDATAMANAGER dmPictTable = { 0 };

#define WIDTHBYTES(i)   ((i + 15) / 16 * 2)      /* ULONG aligned ! */


BOOL FPLIB fpPM_Init(HANDLE hInst)
{
fpCreateGlobalData(&dmPictTable, hInst, sizeof(FP_PICTTBLITEM));
return (TRUE);
}


void FPLIB fpPM_Free(HANDLE hInst)
{
fpDestroyGlobalData(&dmPictTable, hInst, fpxPictManFree);
}

/*  fpPM Picture Manager utility functions      */

BOOL FPLIB fpPM_IsPicture(HANDLE hInst, FPPICTID idPict)
{
   FP_PICT Pict;
   BOOL bRet = FALSE;

   if (idPict)
      {
      if (fpPM_GetItem(hInst, idPict, &Pict) &&
          (Pict.bType != FP_PICT_TYPE_NONE) && 
          (Pict.hPict) &&
          ((Pict.nWidth  > 1) || 
           (Pict.nHeight > 1)))
         bRet = TRUE;
      }
   return bRet;
}

long FPLIB fpPM_GetRefCount(HANDLE hInst, FPPICTID idPict)
{
LPFP_PICTTBLITEM lpPictTable;
UINT             uSize = 0;                   
long             lRefCnt = -1;

if (idPict > 0)
   {
   idPict--;

   if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
      {
      if (idPict < (FPPICTID)uSize && lpPictTable[idPict].lRefCnt)
         lRefCnt = lpPictTable[idPict].lRefCnt;

      fpUnlockGlobalData(&dmPictTable, hInst);
      }
   }
return (lRefCnt);
}

// NOTE: use this function with caution
void FPLIB fpPM_SetRefCount(HANDLE hInst, FPPICTID idPict, long lRefCnt)
{
LPFP_PICTTBLITEM lpPictTable;
UINT             uSize = 0;                   

if (idPict > 0)
   {
   idPict--;

   if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
      {
      if (idPict < (FPPICTID)uSize)
         lpPictTable[idPict].lRefCnt = lRefCnt;

      fpUnlockGlobalData(&dmPictTable, hInst);
      }
   }
}

void FPLIB fpPM_UnrefItem(HANDLE hInst, FPPICTID idPict)
{
LPFP_PICTTBLITEM lpPictTable;
UINT             uSize = 0;

if (idPict > 0)
   {
   idPict--;

   if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
      {
      if (idPict < (FPPICTID)uSize && lpPictTable[idPict].lRefCnt)
         lpPictTable[idPict].lRefCnt--;

      fpUnlockGlobalData(&dmPictTable, hInst);
      }
   }
}


HPALETTE FPLIB fpPM_GetPalette(HANDLE hInst, FPPICTID idPict)
{     
   HPALETTE   hPalette = 0;
   FP_PICT    Pict;

   if (idPict)
      {
      if (fpPM_GetItem(hInst, idPict, &Pict))
         hPalette = Pict.hPal;
      }
   return hPalette;
}

BYTE FPLIB fpPM_GetPictType(HANDLE hInst, FPPICTID idPict)
{     
   BYTE    bType = FP_PICT_TYPE_NONE;
   FP_PICT Pict;

   if (idPict)
      {
      if (fpPM_GetItem(hInst, idPict, &Pict))
         bType = Pict.bType;
      }
   return bType;
}

long FPLIB fpPM_GetWidth(HANDLE hInst, FPPICTID idPict)
{     
   long    lWidth = 0;
   FP_PICT Pict;

   if (idPict)
      {
      if (fpPM_GetItem(hInst, idPict, &Pict) &&
          Pict.bType && 
          Pict.hPict)
         lWidth = Pict.nWidth;
      }
   return lWidth;
}

long FPLIB fpPM_GetHeight(HANDLE hInst, FPPICTID idPict)
{
   long    lHeight = 0;
   FP_PICT Pict;

   if (idPict)
      {
      if (fpPM_GetItem(hInst, idPict, &Pict) &&
          Pict.bType && 
          Pict.hPict)
         lHeight = Pict.nHeight;
      }
   return lHeight;
}

long FPLIB fpPM_GetStretchWidth(HANDLE hInst, FPPICTID idPict)
{     
   long    lWidth = 0;
   FP_PICT Pict;

   if (idPict)
      {
      if (fpPM_GetItem(hInst, idPict, &Pict) &&
          Pict.bType && 
          Pict.hPictStretch)
         lWidth = Pict.nPictStretchWidth;
      }
   return lWidth;
}

long FPLIB fpPM_GetStretchHeight(HANDLE hInst, FPPICTID idPict)
{
   long    lHeight = 0;
   FP_PICT Pict;

   if (idPict)
      {
      if (fpPM_GetItem(hInst, idPict, &Pict) &&
          Pict.bType && 
          Pict.hPictStretch)
         lHeight = Pict.nPictStretchHeight;
      }
   return lHeight;
}

void FPLIB fpPM_SetWidth(HANDLE hInst, FPPICTID idPict, long lWidth)
{     
   LPFP_PICT lpPict;

   if (idPict)
      {
      lpPict = fpPM_LockItem(hInst, idPict);
      if (lpPict->bType && lpPict->hPict)
         lpPict->nWidth = (short)lWidth;
      fpPM_UnlockItem(hInst, idPict);
      }
}

void FPLIB fpPM_SetHeight(HANDLE hInst, FPPICTID idPict, long lHeight)
{
   LPFP_PICT lpPict;

   if (idPict)
      {
      lpPict = fpPM_LockItem(hInst, idPict);
      if (lpPict->bType && lpPict->hPict)
         lpPict->nHeight = (short)lHeight;
      fpPM_UnlockItem(hInst, idPict);
      }
}

void FPLIB fpPM_SetPalette(HANDLE hInst, FPPICTID idPict, HPALETTE hPal)
{     
   LPFP_PICT lpPict;

   if (idPict)
      {
      lpPict = fpPM_LockItem(hInst, idPict);
      if (lpPict->bType && lpPict->hPict)
         lpPict->hPal = hPal;
      fpPM_UnlockItem(hInst, idPict);
      }
}

HBITMAP FPLIB fpPM_GetStretchBitmap(HANDLE hInst, FPPICTID idPict)
{
   FP_PICT   Pict;
   HBITMAP   hbmpStretch = 0;

   if (idPict)
      {
      if (fpPM_GetItem(hInst, idPict, &Pict))
         hbmpStretch = Pict.hPictStretch;
      }
   return hbmpStretch;
}

/*  fpPM Picture Manager utility functions      */

//////////////////////////////////////////////////////////////////////
void fpxPictManFree(GLOBALHANDLE hTable, UINT fuSize)
{
LPFP_PICTTBLITEM lpPictTable;
UINT             i;

if (hTable && (lpPictTable = (LPFP_PICTTBLITEM)GlobalLock(hTable)))
   {
   for (i = 0; i < fuSize; i++)
      {
      if (lpPictTable[i].lRefCnt)
         fpPictFree(&lpPictTable[i].Pict);

      _fmemset(&lpPictTable[i], '\0', sizeof(FP_PICTTBLITEM));
      }

   GlobalUnlock(hTable);
   }
}


BOOL FPLIB fpPM_SetProp(HANDLE hInst, HWND hWnd, BYTE bCtlType, LPFPPICTID lpidPict,
                           long lParam)
{
FP_PICT Pict;

fpPictSet(hWnd, &Pict, bCtlType, lParam);
fpPMx_ChangeItem(hInst, lpidPict, &Pict);

return (TRUE);
}


HANDLE FPLIB fpPM_GetProp(HANDLE hInst, BYTE bCtlType, FPPICTID idPict, long lParam)
{
FP_PICT Pict;
HANDLE    lRet = 0L;

if (fpPM_GetItem(hInst, idPict, &Pict))
   lRet = fpPictGet(&Pict, bCtlType, lParam);

return (lRet);
}

BOOL fpPMx_ItemInTable(HANDLE hInst, LPFP_PICT lpPict, HPBYTE lpMem, long lMemLen, BYTE bCtlType, FPPICTID *lpID)
{
LPFP_PICTTBLITEM lpPictTable;
BOOL             bInTable = FALSE;
UINT             uSize = 0;
UINT             i;

*lpID = 0;

if (lpPict->hPict == 0)
   return (bInTable);

if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
   {
   // search for item in table
   for (i = 0; i < uSize; i++)
      if (lpPictTable[i].Pict.hPict == lpPict->hPict)
	     {
         bInTable = TRUE;
         *lpID = i + 1;
         break;
         }
      else if ((lpPictTable[i].Pict.bType == lpPict->bType) &&
               (lpPictTable[i].Pict.nWidth == lpPict->nWidth) &&
               (lpPictTable[i].Pict.nHeight == lpPict->nHeight))               
         {
         HGLOBAL hBits = NULL;
         LPBYTE  lpBits;
         long lSize;

         switch (lpPict->bType)
           {
            
           case FP_PICT_TYPE_BITMAP:
             {
             hBits = fpPictSaveBmp(lpPictTable[i].Pict.hPict, 
                                   lpPictTable[i].Pict.hPal, &lSize, TRUE, FALSE);
             }
             break;
           } 
         
         if (lSize == lMemLen &&
             hBits && 
             (lpBits = (LPBYTE)GlobalLock(hBits)))
            {
            if (memcmp(lpBits, lpMem, lMemLen) == 0)   // bits are identical
              {
              bInTable = TRUE;
              *lpID = i + 1;
              GlobalUnlock(hBits);
              GlobalFree(hBits);  
              break;
              }
            GlobalUnlock(hBits);
            GlobalFree(hBits);             
            }
         
         }

   fpUnlockGlobalData(&dmPictTable, hInst);
   }

return (bInTable);
}

FPPICTID fpPMx_AddItem(HANDLE hInst, LPFP_PICT lpPict)
{
LPFP_PICTTBLITEM lpPictTable;
long             idPict = -1;
UINT             uSize = 0;
UINT             i;

if (lpPict->hPict == 0)
   return (0);

if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
   {
   // search for item in table
   for (i = 0; i < uSize; i++)
      if (lpPictTable[i].Pict.hPict == lpPict->hPict)
	     {
         lpPictTable[i].lRefCnt++;
         idPict = i;
		 // free unneeded pictureholder created in fpAttachPicture
#if (defined(FP_VB) || defined(FP_OCX)) && (!defined(FP_BTNDES) || defined(BM20) || defined(FP_VB)) // for 16-bit OCX button designer -scl
		 fpDetachPicture(&lpPict->hVBPic);
#endif
         break;
         }

   fpUnlockGlobalData(&dmPictTable, hInst);
   }

if (idPict == -1)
   {
   if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
      {
      for (i = 0; i < uSize; i++)
         if (!lpPictTable[i].lRefCnt)
            {
            idPict = i;
            break;
            }

      fpUnlockGlobalData(&dmPictTable, hInst);
      }

   if (idPict == -1)
      {
      idPict = uSize;
      uSize++;
      }

   // Initialize new entry
   if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
      {
      lpPictTable[idPict].lRefCnt = 1;
      _fmemcpy(&lpPictTable[idPict].Pict, lpPict, sizeof(FP_PICT));
      fpUnlockGlobalData(&dmPictTable, hInst);
      }
   }

return ((FPPICTID)(idPict + 1));
}


void FPLIB fpPM_FreeItem(HANDLE hInst, FPPICTID idPict)
{
if (idPict > 0)
   {
   LPFP_PICTTBLITEM lpPictTable;
   UINT             uSize = 0;

   idPict--;

   if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
      {
      if (idPict < (FPPICTID)uSize && lpPictTable[idPict].lRefCnt)
         {
         lpPictTable[idPict].lRefCnt--;
         if (lpPictTable[idPict].lRefCnt == 0)
            //{
            fpPictFree(&lpPictTable[idPict].Pict);
            //_fmemset(&lpPictTable[idPict], '\0', sizeof(FP_PICTTBLITEM)); // not needed, done in fpPictFree.
            //}
         }

      fpUnlockGlobalData(&dmPictTable, hInst);
      }
   }
}


void FPLIB fpPM_RefItem(HANDLE hInst, FPPICTID idPict)
{
LPFP_PICTTBLITEM lpPictTable;
UINT             uSize = 0;

if (idPict > 0)
   {
   idPict--;

   if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
      {
      if (idPict < (FPPICTID)uSize && lpPictTable[idPict].lRefCnt)
         lpPictTable[idPict].lRefCnt++;

      fpUnlockGlobalData(&dmPictTable, hInst);
      }
   }
}


void fpPMx_ChangeItem(HANDLE hInst, LPFPPICTID lpidPict, LPFP_PICT lpPict)
{
if (lpidPict)
   {
   if (*lpidPict > 0)
      fpPM_FreeItem(hInst, *lpidPict);

   *lpidPict = fpPMx_AddItem(hInst, lpPict);
   }
}


BOOL FPLIB fpPM_GetItem(HANDLE hInst, FPPICTID idPict, LPFP_PICT lpPict)
{
LPFP_PICT lpPictTemp;
BOOL      fRet = FALSE;

if (lpPictTemp = fpPM_LockItem(hInst, idPict))
   {
   _fmemcpy(lpPict, lpPictTemp, sizeof(FP_PICT));
   fpPM_UnlockItem(hInst, idPict);
   fRet = TRUE;
  }

return (fRet);
}


LPFP_PICT FPLIB fpPM_LockItem(HANDLE hInst, FPPICTID idPict)
{
LPFP_PICTTBLITEM lpPictTable;
LPFP_PICT        lpPict = 0;
UINT             uSize = 0;

idPict--;

if (lpPictTable = fpLockGlobalData(&dmPictTable, hInst, &uSize))
   {
   if (idPict >= 0 && idPict < (FPPICTID)uSize && lpPictTable[idPict].lRefCnt)
      lpPict = &lpPictTable[idPict].Pict;

   fpUnlockGlobalData(&dmPictTable, hInst);
   }

return (lpPict);
}


FPPICTID FPLIB fpPM_Load(HANDLE hInst, HPBYTE lpMem, long lMemLen, BYTE bCtlType)
{
FP_PICT Pict;
FPPICTID pID = 0;

fpPictLoad(&Pict, lpMem, lMemLen, bCtlType);
/*
if (fpPMx_ItemInTable(hInst, &Pict, lpMem, lMemLen, bCtlType, &pID) == TRUE)
   return pID;
else
   return (fpPMx_AddItem(hInst, &Pict));
*/
return (fpPMx_AddItem(hInst, &Pict));
}


HGLOBAL FPLIB fpPM_Save(HANDLE hInst, FPPICTID idPict, LPLONG lplSize)
{
FP_PICT Pict;

if (fpPM_GetItem(hInst, idPict, &Pict))
   return (fpPictSave(&Pict, lplSize));

return (0);
}


UINT FPLIB fpPM_GetCount(HANDLE hInst)
{
UINT uSize = 0;

if (fpLockGlobalData(&dmPictTable, hInst, &uSize))
   fpUnlockGlobalData(&dmPictTable, hInst);

return (uSize);
}


void fpPictFree(LPFP_PICT lpPict)
{
#ifdef FP_VB
if (lpPict->hVBPic)
   fpVBFreePic(lpPict->hVBPic);
#endif

#ifdef FP_OCX
   fpDetachPicture(&lpPict->hVBPic);
#endif

if (lpPict->fDeletePict && lpPict->hPict)
{
// fix for bug 9278 -scl
// ->
//   DeleteObject(lpPict->hPict);
   if( FP_PICT_TYPE_ICON == lpPict->bType )
     DestroyIcon(lpPict->hPict);
   else
     DeleteObject(lpPict->hPict);
// <-
// fix for bug 9278 -scl
   if (lpPict->hPal) // need to delete the palette, too -scl
      DeleteObject(lpPict->hPal);
}

if (lpPict->hPictStretch)
   DeleteObject(lpPict->hPictStretch);

_fmemset(lpPict, '\0', sizeof(FP_PICT));
}


HANDLE fpPictGet(LPFP_PICT lpPict, BYTE bCtlType, LPARAM lParam)
{
HANDLE lRet = NULL;

if (lpPict)
   {
   if (bCtlType == FP_CTLTYPE_DLL)
      {
      if (lParam)
         {
         LPFP_PICT_INFO lpfpPictInfo = (LPFP_PICT_INFO)lParam;

         _fmemset(lpfpPictInfo, '\0', sizeof(FP_PICT_INFO));

         lpfpPictInfo->pictType = lpPict->bType;
         lpfpPictInfo->hPalette = lpPict->hPal;
         lpfpPictInfo->u.handle = lpPict->hPict;
         if (lpPict->bType == FP_PICT_TYPE_METAFILE)
            {
            lpfpPictInfo->xExt = lpPict->nWidth;
            lpfpPictInfo->yExt = lpPict->nHeight;
            }
         }

      lRet = lpPict->hPict;
      }

#if (defined(FP_VB) || defined(FP_OCX))
   else
      {
#ifdef FP_OCX
      if (lpPict->hPict)
         lRet = lpPict->hVBPic;
      else
         lRet = 0;
#elif FP_VB
      lRet = lpPict->hVBPic;
#endif
      }
#endif
   }

return (lRet);
}


BOOL fpPictSet(HWND hWnd, LPFP_PICT lpPict, BYTE bCtlType, LPARAM lParam)
{
BITMAP bm;
short  nWidth = 0;
BOOL   fRet = TRUE;

_fmemset(lpPict, '\0', sizeof(FP_PICT));

if (lParam)
   {
   if (bCtlType == FP_CTLTYPE_DLL)
      {
      LPFP_PICT_INFO lpfpPictInfo = (LPFP_PICT_INFO)lParam;

      if (lpfpPictInfo->pictType == FP_PICT_TYPE_BITMAP ||
          lpfpPictInfo->pictType == FP_PICT_TYPE_ICON   ||
          lpfpPictInfo->pictType == FP_PICT_TYPE_METAFILE)
         {
         if (lpfpPictInfo->hndType != FP_HNDTYPE_HANDLE)
            {
            LPTSTR lpName;

            if (lpfpPictInfo->hndType == FP_HNDTYPE_RESNAME)
               lpName = lpfpPictInfo->u.resName.lpsz;
            else
               lpName = (LPTSTR)MAKEINTRESOURCE(lpfpPictInfo->u.resID);

            if (lpfpPictInfo->pictType == FP_PICT_TYPE_BITMAP)
               lpPict->hPict = LoadBitmap(GETGW_HINSTANCE(hWnd), lpName);
            else if (lpfpPictInfo->pictType == FP_PICT_TYPE_ICON)
               lpPict->hPict = LoadIcon(GETGW_HINSTANCE(hWnd), lpName);
            else         
               {
               // currently don't support metafiles as internal resources
               fRet = FALSE;
               }
            if (lpPict->hPict)
               lpPict->fDeletePict = TRUE;
            }
         else
            lpPict->hPict = (HANDLE)lpfpPictInfo->u.handle;

         if (lpPict->hPict)
            {
            if (lpfpPictInfo->pictType == FP_PICT_TYPE_BITMAP)  // CTF was & changed to ==
               {
               GetObject(lpPict->hPict, sizeof(BITMAP), (LPSTR)&bm);
               lpPict->bType = FP_PICT_TYPE_BITMAP;
               lpPict->hPal = lpfpPictInfo->hPalette;
               lpPict->nWidth = (short)bm.bmWidth;
               lpPict->nHeight = (short)bm.bmHeight;
               }
            else if (lpfpPictInfo->pictType == FP_PICT_TYPE_METAFILE)
               {
               lpPict->bType = FP_PICT_TYPE_METAFILE;
               // extents are in pixels for DLL
               lpPict->nWidth = (short)lpfpPictInfo->xExt;
               lpPict->nHeight = (short)lpfpPictInfo->yExt;
               }
            else
               {
               lpPict->bType = FP_PICT_TYPE_ICON;
               lpPict->nWidth = (short)fpGetIconWidth(lpPict->hPict);
               lpPict->nHeight = (short)fpGetIconHeight(lpPict->hPict);
               }
            }
         }
      }

#if (defined(FP_VB) || defined(FP_OCX))
   else
      {
      PIC Pic;

#ifdef FP_VB
      lpPict->hVBPic = VBRefPic((HPIC)lParam);
#else
      fpAttachPicture(&lpPict->hVBPic, lParam);
#endif

      _fmemset(&Pic, '\0', sizeof(PIC));

// CTF & SCL...
      if (fpVBGetVersion() < VB200_VERSION)
         fpVBGetPic(lpPict->hVBPic, &Pic);
      else
         fpVBGetPicEx(lpPict->hVBPic, &Pic, fpVBGetVersion());
// CTF & SCL

      switch (Pic.picType)
         {
         case PICTYPE_BITMAP:
            lpPict->hPict = Pic.picData.bmp.hbitmap;
            lpPict->bType = FP_PICT_TYPE_BITMAP;
            GetObject(lpPict->hPict, sizeof(BITMAP), (LPSTR)&bm);
            lpPict->nWidth = (short)bm.bmWidth;
            lpPict->nHeight = (short)bm.bmHeight;
            lpPict->hPal = Pic.picData.bmp.hpal;
            break;

         case PICTYPE_ICON:
            {
            lpPict->hPict = Pic.picData.icon.hicon;
            lpPict->bType = FP_PICT_TYPE_ICON;
            lpPict->nWidth = (short)fpGetIconWidth(lpPict->hPict);
            lpPict->nHeight = (short)fpGetIconHeight(lpPict->hPict);
            }
            break;

         case PICTYPE_METAFILE:
            lpPict->hPict = Pic.picData.wmf.hmeta;
            lpPict->bType = FP_PICT_TYPE_METAFILE;
            // extents are in twips for VBX / OCX
            lpPict->nWidth = XTwipsToPixels(Pic.picData.wmf.xExt);
            lpPict->nHeight = YTwipsToPixels(Pic.picData.wmf.yExt);
            break;

         case 0:
#ifdef FP_OCX
            fpDetachPicture(&lpPict->hVBPic);
#endif                   
            break;
         }
      }
#endif
   }

return (fRet);
}


BOOL fpPictLoad(LPFP_PICT lpPict, HPBYTE lpMem, long lMemLen, BYTE bCtlType)
{
BOOL fRet = FALSE;

_fmemset(lpPict, '\0', sizeof(FP_PICT));

if (*(LPBYTE)lpMem == FP_PICT_TYPE_BITMAP)
   {
   if (lpPict->hPict = fpPictLoadBmp(lpMem + sizeof(BYTE),
                                     lMemLen - sizeof(BYTE), &lpPict->hPal))
      {
      BITMAP bm;

      GetObject(lpPict->hPict, sizeof(BITMAP), (LPSTR)&bm);
      lpPict->bType = FP_PICT_TYPE_BITMAP;
      lpPict->nWidth = (short)bm.bmWidth;
      lpPict->nHeight = (short)bm.bmHeight;
// changed to keep ownership of GDI resource -scl 7/22/99
//      if (bCtlType == FP_CTLTYPE_DLL)
#ifndef FP_VB
         lpPict->fDeletePict = TRUE;
#endif
#if (defined(FP_VB) || defined(FP_OCX))
//      else
         {
         PIC Pic;

         Pic.picType = PICTYPE_BITMAP;
         Pic.picData.bmp.hbitmap = lpPict->hPict;
         Pic.picData.bmp.hpal = lpPict->hPal;
         lpPict->hVBPic = fpVBAllocPicEx(&Pic, fpVBGetVersion());
         fpVBRefPic(lpPict->hVBPic);
         }
#endif

      fRet = TRUE;
      }
   }

else if (*(LPBYTE)lpMem == FP_PICT_TYPE_ICON)
   {
   if (lpPict->hPict = fpPictLoadIcon(lpMem + sizeof(BYTE),
                                      lMemLen - sizeof(BYTE)))
      {
      lpPict->bType = FP_PICT_TYPE_ICON;
      lpPict->nWidth = (short)fpGetIconWidth(lpPict->hPict);
      lpPict->nHeight = (short)fpGetIconHeight(lpPict->hPict);

// changed to keep ownership of GDI resource -scl 7/22/99
//      if (bCtlType == FP_CTLTYPE_DLL)
#ifndef FP_VB
         lpPict->fDeletePict = TRUE;
#endif
#if (defined(FP_VB) || defined(FP_OCX))
//      else
         {
         PIC Pic;

         Pic.picType = PICTYPE_ICON;
         Pic.picData.icon.hicon = lpPict->hPict;
         lpPict->hVBPic = fpVBAllocPicEx(&Pic, fpVBGetVersion());
         fpVBRefPic(lpPict->hVBPic);
         }
#endif

      fRet = TRUE;
      }
   }
else if (*(LPBYTE)lpMem == FP_PICT_TYPE_METAFILE)
   {                                                  
   long xExt, yExt;
   if (lpPict->hPict = fpPictLoadWMF(lpMem + sizeof(BYTE),
                                     lMemLen - sizeof(BYTE), 
                                     &xExt, &yExt))
      {
      lpPict->bType = FP_PICT_TYPE_METAFILE;
      lpPict->nWidth = (short)xExt;
      lpPict->nHeight = (short)yExt;

// changed to keep ownership of GDI resource -scl 7/22/99
//      if (bCtlType == FP_CTLTYPE_DLL)
#ifndef FP_VB
         lpPict->fDeletePict = TRUE;
#endif
#if (defined(FP_VB) || defined(FP_OCX))
//      else
         {
         PIC Pic;

         Pic.picType = PICTYPE_METAFILE;
         Pic.picData.wmf.hmeta = lpPict->hPict;
         lpPict->hVBPic = fpVBAllocPicEx(&Pic, fpVBGetVersion());
         fpVBRefPic(lpPict->hVBPic);
         }
#endif

      fRet = TRUE;
      }
   }

return (fRet);
}


HGLOBAL fpPictSave(LPFP_PICT lpPict, LPLONG lplSize)
{
HGLOBAL hBuffer = 0;

if (lpPict->bType == FP_PICT_TYPE_BITMAP)
   hBuffer = fpPictSaveBmp(lpPict->hPict, lpPict->hPal, lplSize, TRUE, FALSE);

else if (lpPict->bType == FP_PICT_TYPE_ICON)
   hBuffer = fpPictSaveIcon(lpPict->hPict, lplSize, TRUE);

else if (lpPict->bType == FP_PICT_TYPE_METAFILE)
   hBuffer = fpPictSaveWMF(lpPict->hPict, lpPict->nWidth, 
                           lpPict->nHeight, lplSize, TRUE);

return (hBuffer);
}


//--------------------------------------------------------------------
//
//  Bitmaps are stored as...
//    BITMAPINFOHEADER bmih
//    RGBQUAD rgb[n]
//    BYTE bImage[m]
//

HBITMAP fpPictLoadBmp(HPBYTE lpMem, long lMemLen, HPALETTE FAR *lphPal)
{
LPBITMAPINFOHEADER lpbmih = (LPBITMAPINFOHEADER)(lpMem);
HBITMAP            hbmp = 0;
HPALETTE           hPalOld;
HDC                hDC;
long               lPaletteSize;

if (lMemLen)
   {
   lPaletteSize = fpPaletteSize(lpbmih->biBitCount);
   hDC = GetDC(0);

   if (lphPal)
      {
      if (*lphPal = CreateBIPalette(lpbmih))
         {
         hPalOld = SelectPalette(hDC, *lphPal, TRUE);
         RealizePalette(hDC);
         }
      }
        
   hbmp = CreateDIBitmap(hDC, lpbmih, CBM_INIT,
                         (LPBYTE)lpbmih + lpbmih->biSize + lPaletteSize,
                         (LPBITMAPINFO)lpbmih, DIB_RGB_COLORS);

   if (lphPal)
      SelectPalette(hDC, hPalOld, TRUE);

   ReleaseDC(0, hDC);
   }

return (hbmp);
}

//--------------------------------------------------------------------

HGLOBAL fpPictSaveBmp(HBITMAP hBmp, HPALETTE hPal, LPLONG lplSize,
                      BOOL fSavePictType, BOOL fIconInverted)
{
HGLOBAL            hMem = 0;
BITMAP             bm;
LPBITMAPINFOHEADER lpbmih;
HPALETTE           hPalOld;
int                nBitCount;
long               lMemSize;
long               lPaletteSize;
long               lOffset = (fSavePictType ? sizeof(BYTE) : 0);
HBITMAP            hbmTemp = hBmp;

if (hBmp)
   {
   HDC hDC = GetDC(0);

   if (hPal)
      {
      hPalOld = SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      }

   GetObject(hBmp, sizeof(bm), &bm);

   nBitCount = bm.bmPlanes * bm.bmBitsPixel;

// JIS3686...
#ifndef WIN32
   if( 16 == nBitCount || 32 == nBitCount )
     nBitCount = 24;  // 16 & 32 not supported in 16-bit windows
#endif
// JIS3686

   lPaletteSize = fpPaletteSize(nBitCount);

   lMemSize = sizeof(*lpbmih) + lPaletteSize;

   hMem = GlobalAlloc(GHND, lMemSize + lOffset);
   lpbmih = (LPBITMAPINFOHEADER)((LPSTR)GlobalLock(hMem) + lOffset);

   if (fSavePictType)
      *(LPBYTE)((LPSTR)lpbmih - sizeof(BYTE)) = FP_PICT_TYPE_BITMAP;

   lpbmih->biSize = sizeof(*lpbmih);
   lpbmih->biWidth = bm.bmWidth;
   lpbmih->biHeight = bm.bmHeight;
   lpbmih->biPlanes = 1;
   lpbmih->biBitCount = nBitCount;
   lpbmih->biCompression = BI_RGB;
   GetDIBits(hDC, hBmp, 0, (UINT)lpbmih->biHeight, NULL,
             (LPBITMAPINFO)lpbmih, DIB_RGB_COLORS);
   lMemSize += lpbmih->biSizeImage;
   GlobalUnlock(hMem);

   hMem = GlobalReAlloc(hMem, lMemSize + lOffset, 0);
   lpbmih = (LPBITMAPINFOHEADER)((LPSTR)GlobalLock(hMem) + lOffset);

#ifdef WIN32  
   if (fIconInverted)
      lpbmih->biHeight = -(lpbmih->biHeight);
#else
//   if (fIconInverted)
//      {
//      HBITMAP hbmRot, hbmRot2;
//      hbmRot = fpRotateBitmap(hDC, hBmp, hPal);
//      hbmRot2 = fpRotateBitmap(hDC, hbmRot, hPal);
//      hbmTemp = fpRotateBitmap(hDC, hbmRot2, hPal);
//      DeleteObject(hbmRot);
//      DeleteObject(hbmRot2);
//      }

#endif        

   GetDIBits(hDC, hbmTemp, 0, (UINT)labs(lpbmih->biHeight),
             (LPBYTE)lpbmih + lpbmih->biSize + lPaletteSize,
             (LPBITMAPINFO)lpbmih, DIB_RGB_COLORS);

#ifndef WIN32
//   if (fIconInverted)
//      DeleteObject(hbmTemp);
#endif

// leave height negative to denote bottom-up or top-down DIB
//   if (fIconInverted)
//      lpbmih->biHeight = -(lpbmih->biHeight);

   if (hPal)
      SelectPalette(hDC, hPalOld, TRUE);

   *lplSize = lMemSize + lOffset;
   GlobalUnlock(hMem);
   ReleaseDC(0, hDC);
   }

return (hMem);
}

//--------------------------------------------------------------------
//
//  Icons are stored as...
//    long lMaskLen = # bytes used to store mask bitmap
//    long lColorLen = # bytes used to store color bitmap
//    bmMask = mask bitmap stored via SS_Save4Bitmap()
//    bmColor = color bitmap stored via SS_Save4Bitmap()
//

HICON fpPictLoadIcon(HPBYTE lpMem, long lMemLen)
{
HICON hIcon = 0;

//#ifdef WIN32

LPBYTE   lpMask;
LPBYTE   lpColor;
long     lMaskLen;
long     lColorLen;

if (lMemLen)
   {
   lMaskLen = *(LPLONG)lpMem;
   lColorLen = *(LPLONG)(lpMem + sizeof(long));
   lpMask = lpMem + 2 * sizeof(long);
   lpColor = lpMask + lMaskLen;  
   
// JIS3686...
   {
#ifdef _WIN32
   DWORD dwVersion = GetVersion();
   DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
   DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));

   if (dwVersion < 0x80000000)                // Windows NT
      {
      ICONINFO iconInfo;
      iconInfo.fIcon = TRUE;
      iconInfo.xHotspot = 0;  // ignored for icons
      iconInfo.yHotspot = 0;  // ignored for icons

      iconInfo.hbmMask = fpPictLoadBmp(lpMask, lMaskLen, 0);
      iconInfo.hbmColor = fpPictLoadBmp(lpColor, lColorLen, 0);
      hIcon = CreateIconIndirect(&iconInfo);
      DeleteObject(iconInfo.hbmMask);
      DeleteObject(iconInfo.hbmColor);
      }
/*
   else if (dwWindowsMajorVersion < 4)        // Win32s
      { // this case should probably be included 
      } //under Win95 / 16-bit, below -scl
*/
   else                                       // Windows 95 or 16-bit
#endif
  {
      HBITMAP hbmColor = fpPictLoadBmp(lpColor, lColorLen, 0);
      BITMAP  bm;
      HGLOBAL hBits;
      long    lbmLen;

      GetObject(hbmColor, sizeof(BITMAP), &bm);
      lbmLen = bm.bmHeight * bm.bmWidthBytes;
      if (hBits = GlobalAlloc(GHND, lbmLen))
         {
         LPSTR lpBits = (LPSTR)GlobalLock(hBits);
         GetBitmapBits(hbmColor, lbmLen, lpBits);

         hIcon = CreateIcon(fpInstance,
                            (int)bm.bmWidth,
                            (int)bm.bmHeight,
                            (BYTE)bm.bmPlanes,
                            (BYTE)bm.bmBitsPixel,
                            lpMask + sizeof(BITMAPINFOHEADER) + fpPaletteSize(((LPBITMAPINFOHEADER)(lpMask))->biBitCount),
                            lpBits);

         GlobalUnlock(hBits);
         GlobalFree(hBits);
         }
// fix for bug 6130 -scl
      DeleteObject(hbmColor);
/*
      LPBITMAPINFOHEADER lpbmih = (LPBITMAPINFOHEADER)(lpMask);
      LPBITMAPINFOHEADER lpbmih2 = (LPBITMAPINFOHEADER)(lpColor);

// icon was saved inverted
      if (lpbmih->biHeight < 0)
         lpbmih->biHeight = -(lpbmih->biHeight);
           
      if (lpbmih2->biHeight < 0)
         lpbmih2->biHeight = -(lpbmih2->biHeight);
           
      hIcon = CreateIcon(fpInstance, 
                         (int)lpbmih2->biWidth,
                         (int)lpbmih2->biHeight,
                         (BYTE)lpbmih2->biPlanes, 
                         (BYTE)lpbmih2->biBitCount,
                         lpMask + sizeof(BITMAPINFOHEADER) + fpPaletteSize(lpbmih->biBitCount), 
                         lpColor + sizeof(BITMAPINFOHEADER)+ fpPaletteSize(lpbmih2->biBitCount));
*/
      }

   }
// JIS3686
   }

//#endif

return (hIcon);
}

//--------------------------------------------------------------------

HGLOBAL fpPictSaveIcon(HICON hIcon, LPLONG lplSize, BOOL fSavePictType)
{
HGLOBAL hMem = 0;
BYTE    bPictType = FP_PICT_TYPE_ICON;

if (hIcon)
   {
//#ifdef WIN32
   FP_HUGEBUFFER Buffer;
   HGLOBAL       hMemTemp;
   ICONINFO      iconInfo;
   LPSTR         lpMem;
   long          lMemSize = 0;
   long          lMaskLen = 0;
   long          lColorLen = 0;

   fpHugeBufferInit(&Buffer, 50);

#ifdef _WIN32 
   GetIconInfo(hIcon, &iconInfo);
#else
   fpGetIconInfo(hIcon, &iconInfo);
#endif

   if (fSavePictType)
      fpHugeBufferAlloc(&Buffer, (HPSTR)&bPictType, sizeof(BYTE));

   fpHugeBufferAlloc(&Buffer, (HPSTR)&lMaskLen, sizeof(lMaskLen));
   fpHugeBufferAlloc(&Buffer, (HPSTR)&lColorLen, sizeof(lColorLen));

#ifdef _WIN32
// JIS3686 added icon invert parameter to fpPictSaveBmp
   if (hMemTemp = fpPictSaveBmp(iconInfo.hbmMask, 0, &lMaskLen, FALSE, TRUE))
      {
      lpMem = (LPSTR)GlobalLock(hMemTemp);
      fpHugeBufferAlloc(&Buffer, lpMem, lMaskLen);
      GlobalUnlock(hMemTemp);
// pluggin' leaks . . . -scl
	  GlobalFree(hMemTemp);
// pluggin' leaks . . . -scl
      }

// JIS3686 added icon invert parameter to fpPictSaveBmp
   if (hMemTemp = fpPictSaveBmp(iconInfo.hbmColor, 0, &lColorLen, FALSE, TRUE))
      {
      lpMem = (LPSTR)GlobalLock(hMemTemp);
      fpHugeBufferAlloc(&Buffer, lpMem, lColorLen);
      GlobalUnlock(hMemTemp);
// pluggin' leaks . . . -scl
	  GlobalFree(hMemTemp);
// pluggin' leaks . . . -scl
      }
#else
   if (hMemTemp = fpPictSaveBmp(iconInfo.hbmMask, 0, &lMaskLen, FALSE, FALSE))
      {
      lpMem = (LPSTR)GlobalLock(hMemTemp);
      fpHugeBufferAlloc(&Buffer, lpMem, lMaskLen);
      GlobalUnlock(hMemTemp);
// pluggin' leaks . . . -scl
	  GlobalFree(hMemTemp);
// pluggin' leaks . . . -scl
      }

   if (hMemTemp = fpPictSaveBmp(iconInfo.hbmColor, 0, &lColorLen, FALSE, FALSE))
      {
      lpMem = (LPSTR)GlobalLock(hMemTemp);
      fpHugeBufferAlloc(&Buffer, lpMem, lColorLen);
      GlobalUnlock(hMemTemp);
// pluggin' leaks . . . -scl
	  GlobalFree(hMemTemp);
// pluggin' leaks . . . -scl
      }
#endif

   DeleteObject(iconInfo.hbmMask);
   DeleteObject(iconInfo.hbmColor);

   lpMem = (LPSTR)GlobalLock(Buffer.hBuffer);

   if (fSavePictType)                            // lMaskLen was overwriting
       lpMem += sizeof(BYTE);                    //   the PictType of ICON

   fpMemHugeCpy(lpMem, &lMaskLen, sizeof(long));
   fpMemHugeCpy(lpMem + sizeof(long), &lColorLen, sizeof(long));

   GlobalUnlock(Buffer.hBuffer);
   *lplSize = Buffer.lBufferLen;
   hMem = Buffer.hBuffer;

/*
#else

   HBITMAP hBmp;

   hBmp = fpIconToBitmap(GetDesktopWindow(), hIcon, RGB(255,255,255));
   hMem = fpPictSaveBmp(hBmp, 0, lplSize, TRUE, FALSE);
   DeleteObject(hBmp);

#endif
*/
   }

return (hMem);
}

//  Metafiles are stored as...
//    LONG xExt
//    LONG yExt
//    METAHEADER hdrWMF
//    METARECORD recWMF ('n' times)

HMETAFILE fpPictLoadWMF(HPBYTE lpMem, long lMemLen, LPLONG lpXExt, LPLONG lpYExt)
{
HMETAFILE  hwmf = 0;
METAHEADER hdrWMF;
HGLOBAL    hMemBits = 0;
HPBYTE     hpbTemp;
LPBYTE     lpMemBits;

*lpXExt = 0;
*lpYExt = 0;

if (lMemLen)
   {        
   *lpXExt = *(LPLONG)lpMem;
   *lpYExt = *(LPLONG)(lpMem + sizeof(long));
   hpbTemp = lpMem + (2 * sizeof(long));

   fpMemHugeCpy((LPBYTE)&hdrWMF, hpbTemp, sizeof(METAHEADER));
                                                         
   // mtSize is size of metafile in WORDs, multiply by 2 for bytecount                                                         
   hMemBits = GlobalAlloc(GHND, (2 * hdrWMF.mtSize));
   lpMemBits = (LPBYTE)GlobalLock(hMemBits);
   if (lpMemBits)
      {
      fpMemHugeCpy((LPBYTE)lpMemBits, hpbTemp, (2 * hdrWMF.mtSize));
#ifdef WIN32
      hwmf = SetMetaFileBitsEx((2 * hdrWMF.mtSize), lpMemBits);
      GlobalUnlock(hMemBits);
#else
      GlobalUnlock(hMemBits);
      hwmf = SetMetaFileBits(hMemBits);
#endif

      }
   }

return (hwmf);
}

//--------------------------------------------------------------------

HGLOBAL fpPictSaveWMF(HMETAFILE hwmf, LONG lWidth, LONG lHeight, LPLONG lplSize,
                      BOOL fSavePictType)
{
HGLOBAL            hMem = 0;
HGLOBAL            hMemBits = 0;
LPBYTE             lpMem;
LPBYTE             lpMemBits;
METAHEADER         hdrWMF;
long               lMemSize;
long               lOffset = (fSavePictType ? sizeof(BYTE) : 0);

*lplSize = 0;

if (hwmf)
   {

   HDC hdcMeta;
   HMETAFILE hmfLocal;
   UINT nSize = 0;

   hdcMeta = CreateMetaFile(NULL);
   PlayMetaFile(hdcMeta, hwmf);
   hmfLocal = CloseMetaFile(hdcMeta);

#ifdef WIN32
   nSize = GetMetaFileBitsEx(hmfLocal, 0, NULL);
   hMemBits = GlobalAlloc(GHND, nSize);
   lpMemBits = (LPBYTE)((LPSTR)GlobalLock(hMemBits));
   GetMetaFileBitsEx(hmfLocal, nSize, lpMemBits);
// 32 bit requires the metafile handle to be deleted and not
//     in 16 bit
   DeleteMetaFile(hmfLocal);
#else
   hMemBits = GetMetaFileBits(hmfLocal);
   lpMemBits = (LPBYTE)GlobalLock(hMemBits);
#endif

   if (lpMemBits)
      {
      fpMemHugeCpy((LPBYTE)&hdrWMF, lpMemBits, sizeof(METAHEADER));
                                                         
      // mtSize is size of metafile in WORDs, multiply by 2 for bytecount                                                         
      lMemSize = (2 * sizeof(long)) + (2 * hdrWMF.mtSize);

      hMem = GlobalAlloc(GHND, lMemSize + lOffset);
      lpMem = (LPBYTE)((LPSTR)GlobalLock(hMem) + lOffset);

      if (fSavePictType)
         *(LPBYTE)((LPSTR)lpMem - sizeof(BYTE)) = FP_PICT_TYPE_METAFILE;

      fpMemHugeCpy(lpMem, &lWidth, sizeof(long));
      lpMem += sizeof(long);
      fpMemHugeCpy(lpMem, &lHeight, sizeof(long));
      lpMem += sizeof(long);
      fpMemHugeCpy(lpMem, lpMemBits, (2 * hdrWMF.mtSize));
      GlobalUnlock(hMemBits);
      GlobalFree(hMemBits);
      GlobalUnlock(hMem);
      *lplSize = lMemSize + lOffset;

      }
   }

return (hMem);
}

static long fpPaletteSize(int nBitCount)
{
long lPalleteSize = 0;

if( nBitCount <= 8 )
  lPalleteSize = (long)(1 << nBitCount) * sizeof(RGBQUAD);
else if( nBitCount <= 16 )
  lPalleteSize = 3 * sizeof(DWORD);
else if( nBitCount <= 24 )
  lPalleteSize = 0;
else if( nBitCount <= 32 )
  lPalleteSize = 3 * sizeof(DWORD);

return (lPalleteSize);
}


HBITMAP FPLIB fpRotateBitmap(HDC hDCDev, HBITMAP hBitmap, HPALETTE hPal)
{                 
BITMAP       BitmapSrc;
BITMAP       BitmapDest;
HBITMAP      hBitmapDest = 0;
GLOBALHANDLE hBitsSrc;
GLOBALHANDLE hBitsDest;
HPSTR        lpBitsSrc;
HPSTR        lpBitsDest;
HPBYTE       lpByteSrc;
HPBYTE       lpByteDest;
BYTE         bByte;
long         lColDest;
long         lCol;
long         lRow;
long         lBitCnt;
long         lBitOffset;
long         lbmSrcWidthBytes;
long         lbmDestWidthBytes;
short        nPlane;
short        nNumBytes;
HBITMAP      hBitmapOld;
HPALETTE     hPalMemOld;
HDC          hDCMemory;

GetObject(hBitmap, sizeof(BITMAP), &BitmapSrc);
BitmapSrc.bmWidthBytes = (short)WIDTHBYTES((long)BitmapSrc.bmWidth *
                                           (long)BitmapSrc.bmBitsPixel);

_fmemcpy(&BitmapDest, &BitmapSrc, sizeof(BITMAP));
BitmapDest.bmWidth = BitmapSrc.bmHeight;
BitmapDest.bmHeight = BitmapSrc.bmWidth;
BitmapDest.bmWidthBytes = (short)WIDTHBYTES((long)BitmapDest.bmWidth *
                                            (long)BitmapDest.bmBitsPixel);

lbmSrcWidthBytes = (long)((long)BitmapSrc.bmWidthBytes * (long)BitmapSrc.bmPlanes);
lbmDestWidthBytes = (long)((long)BitmapDest.bmWidthBytes * (long)BitmapDest.bmPlanes);

if (hBitsSrc = GlobalAlloc(GHND,
                           (long)(lbmSrcWidthBytes *
                           (long)BitmapSrc.bmHeight) + 1))
   {
   if (hBitsDest = GlobalAlloc(GHND,
                               (long)(lbmDestWidthBytes *
                               (long)BitmapDest.bmHeight) + 1))
      {
      lpBitsSrc = (HPSTR)GlobalLock(hBitsSrc);
      lpBitsDest = (HPSTR)GlobalLock(hBitsDest);

      lBitCnt = BitmapSrc.bmBitsPixel;                
      nNumBytes = (short)(lBitCnt / 8);
        
      lRow = GetBitmapBits(hBitmap, 
                           (long)(lbmSrcWidthBytes * (long)BitmapSrc.bmHeight),
                           lpBitsSrc);

      for (lRow = 0; lRow < BitmapSrc.bmHeight; lRow++)
         {
         lColDest = BitmapDest.bmWidth - lRow - 1;

         for (lCol = 0; lCol < BitmapSrc.bmWidth; lCol++)
            {
            if (BitmapSrc.bmPlanes == 1)
               {
               lBitOffset = (lRow * lbmSrcWidthBytes) +
                             ((lCol * lBitCnt) / 8);
               lpByteSrc = (HPBYTE)(lpBitsSrc + lBitOffset);

               lBitOffset = (lCol * lbmDestWidthBytes) +
                             ((lColDest * lBitCnt) / 8);
               lpByteDest = (HPBYTE)(lpBitsDest + lBitOffset);

               if (nNumBytes == 0)
                  {
                  lBitOffset = (lCol * lBitCnt) % 8;
                  bByte = *lpByteSrc >> (8 - (lBitOffset + lBitCnt));
                  bByte = bByte << (8 - lBitCnt);

                  lBitOffset = (lColDest * lBitCnt) % 8;
                  *lpByteDest |= (bByte >> lBitOffset);
                  }
               else
                  _fmemmove(lpByteDest, lpByteSrc, nNumBytes);
               }
            else
               {
               /***************************************************
               * Read each plane and write it to the new location
               ***************************************************/

               for (nPlane = 0; nPlane < BitmapSrc.bmPlanes; nPlane++)
                  {
                  lBitOffset = (lRow * lbmSrcWidthBytes) + ((lCol / 8) +
                               (long)((long)BitmapSrc.bmWidthBytes * (long)nPlane));
                  bByte = *(HPBYTE)(lpBitsSrc + lBitOffset);
                  bByte = (bByte << (lCol % 8)) & 0x80;

                  lBitOffset = (lCol * lbmDestWidthBytes) + ((lColDest / 8) +
                               (long)((long)BitmapDest.bmWidthBytes * (long)nPlane));
                  lpByteDest = (HPBYTE)(lpBitsDest + lBitOffset);
                  *lpByteDest |= (bByte >> (lColDest % 8));
                  }
               }
            }
         }

      BitmapDest.bmBits = lpBitsDest;

      hDCMemory = CreateCompatibleDC(hDCDev);

      if (hPal)
         {
         hPalMemOld = SelectPalette(hDCMemory, hPal, 0);
         RealizePalette(hDCMemory);
         }

      hBitmapOld = SelectObject(hDCMemory, hBitmap);

      hBitmapDest = CreateCompatibleBitmap(hDCMemory, BitmapDest.bmWidth,
                                           BitmapDest.bmHeight);
      SetBitmapBits(hBitmapDest, lbmDestWidthBytes * (long)BitmapDest.bmHeight,
                    lpBitsDest);

      GetObject(hBitmapDest, sizeof(BITMAP), &BitmapDest);

      if (hPal)
         SelectPalette(hDCMemory, hPalMemOld, TRUE);

      SelectObject(hDCMemory, hBitmapOld);
      DeleteDC(hDCMemory);

/*
      if (hPal)
         {
         hDCMemory = CreateCompatibleDC(0);
         hPalMemOld = SelectPalette(hDCMemory, hPal, 0);
         RealizePalette(hDCMemory);

         hBitmapOld = SelectObject(hDCMemory, hBitmap);

         hBitmapDest = CreateCompatibleBitmap(hDCMemory, BitmapDest.bmWidth,
                                              BitmapDest.bmHeight);
         SetBitmapBits(hBitmapDest, lbmSrcWidthBytes * (long)BitmapDest.bmHeight,
                       lpBitsDest);

         SelectPalette(hDCMemory, hPalMemOld, TRUE);
         SelectObject(hDCMemory, hBitmapOld);
         DeleteDC(hDCMemory);
         }
      else
         hBitmapDest = CreateBitmapIndirect(&BitmapDest);
*/

      GlobalUnlock(hBitsDest);
      GlobalFree(hBitsDest);
      GlobalUnlock(hBitsSrc);
      }

   GlobalFree(hBitsSrc);
   }

return (hBitmapDest);
}

#if 0
void fpGetIconInfo(HICON hIcon, ICONINFO FAR * lpIconInfo)
{
    if (lpIconInfo)
    {
    HWND                    hWndDesktop = GetDesktopWindow();
    HDC             hDCtemp1 = CreateCompatibleDC(hWndDesktop);
    HDC             hDCtemp2 = CreateCompatibleDC(hWndDesktop);
    int             nWidth = xGetSystemMetrics(SM_CXICON);
    int             nHeight = xGetSystemMetrics(SM_CYICON);
    RECT            rectIcon = {0, 0, nWidth, nHeight};
    HBITMAP         hBmpMaskBlack = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
    HBITMAP         hBmpMaskWhite = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
    HBITMAP         hBmpOld1 = SelectObject(hDCtemp1, hBmpMaskBlack);
    HBITMAP         hBmpOld2 = SelectObject(hDCtemp2, hBmpMaskWhite);
    //HBRUSH          hBrBlack = GetStockObject(BLACK_BRUSH);
    //HBRUSH          hBrWhite = GetStockObject(WHITE_BRUSH);
    //HPALETTE        hPal = GetStockObject(DEFAULT_PALETTE);
          
      lpIconInfo->fIcon = TRUE;
        lpIconInfo->hbmColor = fpIconToBitmap(hWndDesktop, hIcon, RGB(255,255,255));
    

    // fill hBmpMaskBlack with black pixels, then
    // draw the icon in white using black as the mask color
    //SetMapMode(hDCtemp1, MM_TEXT);
    //SelectObject(hDCtemp1, hPal);
    //RealizePalette(hDCtemp1);
    //FillRect(hDCtemp1, &rectIcon, hBrBlack);
    //BitBlt(hDCtemp1, 0, 0, nWidth, nHeight, 0, 0, 0, BLACKNESS);
    //SetBkColor(hDCtemp1, RGB(0,0,0));
    //SetTextColor(hDCtemp1, RGB(255,255,255));
    //DrawIcon(hDCtemp1, 0, 0, hIcon);
    SelectObject(hDCtemp2, lpIconInfo->hbmColor);
    SetBkColor(hDCtemp2, RGB(255,255,255));
    BitBlt(hDCtemp1, 0, 0, nWidth, nHeight, hDCtemp2, 0, 0, NOTSRCCOPY);
    // hBmpMaskBlack now contains white pixels 
    // where the icon contains non-black pixels
    
    // fill hBmpMaskWhite with white pixels, then
    // draw the icon in black using white as the mask color
    //SetMapMode(hDCtemp2, MM_TEXT);
    //SelectObject(hDCtemp2, hPal);
    //RealizePalette(hDCtemp2);
    //FillRect(hDCtemp2, &rectIcon, hBrWhite);
    //BitBlt(hDCtemp2, 0, 0, nWidth, nHeight, 0, 0, 0, WHITENESS);
    //SetBkColor(hDCtemp2, RGB(255,255,255));
    //SetTextColor(hDCtemp2, RGB(0,0,0));
    //DrawIcon(hDCtemp2, 0, 0, hIcon);
    SelectObject(hDCtemp1, hBmpMaskWhite);
    SelectObject(hDCtemp2, hBmpOld2);
    DeleteObject(lpIconInfo->hbmColor);
        lpIconInfo->hbmColor = fpIconToBitmap(hWndDesktop, hIcon, RGB(0,0,0));
    SelectObject(hDCtemp2, lpIconInfo->hbmColor);
    SetBkColor(hDCtemp2, RGB(0,0,0));
    BitBlt(hDCtemp1, 0, 0, nWidth, nHeight, hDCtemp2, 0, 0, SRCCOPY);
    // hBmpMaskWhite now contains black pixels 
    // where the icon contains non-white pixels
    
    // now combine the two mask bitmaps 
    // invert each pixel in hBmpMaskBlack and
    // combine with the corresponding pixel in 
    // hBmpMaskWhite using the OR operator
    SelectObject(hDCtemp2, hBmpMaskBlack);
    BitBlt(hDCtemp1, 0, 0, nWidth, nHeight, hDCtemp2, 0, 0, MERGEPAINT);
    // hBmpMaskWhite now contains black pixels 
    // where the icon contains any pixels, and
    // white pixels elsewhere

    // test code, make the mask all black
    //FillRect(hDCtemp1, &rectIcon, hBrBlack);
    // test code, make the mask all white
    //FillRect(hDCtemp1, &rectIcon, hBrWhite);

    lpIconInfo->hbmMask = hBmpMaskWhite; 
    // cleanup
    SelectObject(hDCtemp1, hBmpOld1);
    SelectObject(hDCtemp2, hBmpOld2);
    DeleteObject(hBmpMaskBlack);
    DeleteDC(hDCtemp1);
    DeleteDC(hDCtemp2);
  }
}
#endif


void fpGetIconInfo(HICON hIcon, ICONINFO FAR * lpIconInfo)
{
if (lpIconInfo)
   {
   HDC     hDCtemp1 = CreateCompatibleDC(0);
   HDC     hDCtemp2 = CreateCompatibleDC(0);
   HDC     hDCtemp3 = CreateCompatibleDC(0);
   int     nWidth = (int)fpGetIconWidth(hIcon);
   int     nHeight = (int)fpGetIconHeight(hIcon);
   RECT    rectIcon = {0, 0, nWidth, nHeight};
   HBITMAP hBmpMaskBlack = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
   HBITMAP hBmpMaskWhite = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
   HBITMAP hBmpMaskFlip = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
   HBITMAP hBmpOld1 = SelectObject(hDCtemp1, hBmpMaskBlack);
   HBITMAP hBmpOld2 = SelectObject(hDCtemp2, hBmpMaskWhite);
   HBITMAP hBmpOld3 = SelectObject(hDCtemp3, hBmpMaskFlip);
   HBRUSH  hBrBlack = GetStockObject(BLACK_BRUSH);
   HBRUSH  hBrWhite = GetStockObject(WHITE_BRUSH);

   // Initialize Color bitmap.
   // The transparent portion is black.

   lpIconInfo->fIcon = TRUE;
   lpIconInfo->hbmColor = fpIconToBitmap(GetDesktopWindow(), hIcon, RGB(0,0,0));
//   lpIconInfo->hbmColor = fpIconToBitmap2(hIcon, RGB(0,0,0));
   lpIconInfo->xHotspot = 0;  // ignored for icons
   lpIconInfo->yHotspot = 0;  // ignored for icons

   // Fill hBmpMaskBlack with black pixels, then draw the icon
   SetMapMode(hDCtemp1, MM_TEXT);
   FillRect(hDCtemp1, &rectIcon, hBrBlack);
   SetBkColor(hDCtemp1, RGB(0,0,0));
   SetTextColor(hDCtemp1, RGB(255,255,255));
#ifdef WIN32
   DrawIconEx(hDCtemp1, 0, 0,
              hIcon, nWidth, nHeight,
              0,
              NULL, DI_NORMAL);
#else
   DrawIcon(hDCtemp1, 0, 0, hIcon);
#endif


   // Fill hBmpMaskWhite with white pixels, then draw the icon
   SetMapMode(hDCtemp2, MM_TEXT);
   FillRect(hDCtemp2, &rectIcon, hBrWhite);
   SetBkColor(hDCtemp2, RGB(0,0,0));
   SetTextColor(hDCtemp2, RGB(255,255,255));
#ifdef WIN32
   DrawIconEx(hDCtemp2, 0, 0,
              hIcon, nWidth, nHeight,
              0,
              NULL, DI_NORMAL);
#else
   DrawIcon(hDCtemp2, 0, 0, hIcon);
#endif

   // Now combine the two mask bitmaps turning
   // the transperent bits to 1 and the others 0.
   BitBlt(hDCtemp1, 0, 0, nWidth, nHeight, hDCtemp2, 0, 0, SRCINVERT);

   StretchBlt(hDCtemp3, nWidth - 1, 0, -nWidth, nHeight, hDCtemp1, 0, 0, nWidth, nHeight, SRCCOPY);

//   lpIconInfo->hbmMask = hBmpMaskBlack;
   lpIconInfo->hbmMask = hBmpMaskFlip;

   // Cleanup
   SelectObject(hDCtemp1, hBmpOld1);
   SelectObject(hDCtemp2, hBmpOld2);
   SelectObject(hDCtemp3, hBmpOld3);
   DeleteObject(hBmpMaskBlack);
//   DeleteObject(hBmpMaskFlip);
   DeleteObject(hBmpMaskWhite);

   DeleteDC(hDCtemp1);
   DeleteDC(hDCtemp2);
   DeleteDC(hDCtemp3);

// this code will rotate the mask bitmap of a 16 bit icon when saved to blob
   {
   HDC     hDCtempX = CreateCompatibleDC(0);
   HBITMAP hbmRot;
      
   hbmRot = fpRotateBitmap(hDCtempX, hBmpMaskFlip, 0);
   DeleteObject(hBmpMaskFlip);
   hBmpMaskFlip = fpRotateBitmap(hDCtempX, hbmRot, 0);
   DeleteObject(hbmRot);
     
   lpIconInfo->hbmMask = hBmpMaskFlip;
   }
//
   }
}


#if 0
HBITMAP FPLIB fpIconToBitmap2(HICON hIcon, COLORREF Color)
{
HBITMAP    hBitmap;
HBITMAP    hBitmapOld;
HDC        hDCMemory;
RECT       Rect;
HBRUSH     hBrush;
short      nWidth;
short      nHeight;

nWidth = xGetSystemMetrics(SM_CXICON);
nHeight = xGetSystemMetrics(SM_CYICON);

/*********************************
* Create a memory device context
*********************************/

hDCMemory = CreateCompatibleDC(0);

/*******************************************************
* Create Bitmap to select to the memory device context
*******************************************************/

hBitmap = CreateBitmap(nWidth, nHeight, 1, 4, NULL);
//hBitmap = CreateCompatibleBitmap(hDCMemory, nWidth, nHeight);

hBitmapOld = SelectObject(hDCMemory, hBitmap);
SetMapMode(hDCMemory, MM_TEXT);

/*****************************************
* Fill the hDCMemory background with the
* current background of the display
*****************************************/

hBrush = CreateSolidBrush(Color);
SetRect(&Rect, 0, 0, nWidth, nHeight);
FillRect(hDCMemory, &Rect, hBrush);
DeleteObject(hBrush);

/*********************************************
* Draw the icon on the memory device context
*********************************************/

#ifdef WIN32
DrawIconEx(hDCMemory, 0, 0,
           hIcon, Width, Height,
           0,
           NULL, DI_NORMAL);
#else
DrawIcon(hDCMemory, 0, 0, hIcon);
#endif

/***********
* Clean up
***********/

SelectObject(hDCMemory, hBitmapOld);
DeleteDC(hDCMemory);

return (hBitmap);
}
#endif
