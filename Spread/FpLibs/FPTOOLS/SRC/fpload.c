#define  NOCOMM
#include <windows.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "fpmemmgr.h"
#include "fptools.h"
#include "fphuge.h"
#include "fpsave.h"

short FPLIB fpIO_Read(LPFP_IO lpfpIO, HPVOID hpData, long lSize)
{
short rc = 0;

#ifdef FP_VB
if (lpfpIO->fVB)
   {
   if (lSize > 30000)
      {
      FPGLOBALHANDLE hTemp;
      LPSTR          lpszTemp;
      HPSTR          hpDataTemp = (HPSTR)hpData;
      long           lSizeTotal = lSize;
      long           lSizeTemp;

      if (hTemp = fpGlobalAlloc(GHND, 30000))
         {
         lpszTemp = (LPSTR)fpGlobalLock(hTemp);

         for (; lSizeTotal > 0 && rc == 0; lSizeTotal -= 30000,
              hpDataTemp += 30000)
            {
            lSizeTemp = min(lSizeTotal, 30000);

            rc = VBReadFormFile(lpfpIO->IO.hf, (LPVOID)lpszTemp,
                                 (WORD)lSizeTemp);
            fpMemHugeCpy(hpDataTemp, lpszTemp, lSizeTemp);
            }

         fpGlobalUnlock(hTemp);
         fpGlobalFree(hTemp);
         }
      else
         rc = -1;
      }
   else
      rc = VBReadFormFile(lpfpIO->IO.hf, (LPSTR)hpData, (WORD)lSize);

   if (!rc)
      lpfpIO->IO.Buffer.lBufferLen += lSize;
   }
else
#endif
   {       
	HPSTR hpBuffer = lpfpIO->IO.Buffer.hBuffer ? (HPSTR)GlobalLock(lpfpIO->IO.Buffer.hBuffer) : NULL;

   if (hpBuffer)
      {
      if (lpfpIO->IO.Buffer.lAllocLen + lSize <= lpfpIO->IO.Buffer.lBufferLen)
         {
         fpMemHugeCpy(hpData,
                      hpBuffer + lpfpIO->IO.Buffer.lAllocLen,
                      lSize);
         lpfpIO->IO.Buffer.lAllocLen += lSize;
         }
      else
         rc = 1;
      GlobalUnlock(lpfpIO->IO.Buffer.hBuffer);
      }   
    else
      rc = 1;
   }

return (rc);
}


short FPLIB FP_LoadTables(LPFP_IO lpIO, LPFP_LOAD_TABLES lpTables)
{
short          rc;          

_fmemset(lpTables, '\0', sizeof(FP_LOAD_TABLES));

rc = FP_LoadColorTable(lpIO, lpTables);
rc = FP_LoadFontTable(lpIO, lpTables);
rc = FP_LoadPictTable(lpIO, lpTables);

return (rc);
}


short FPLIB FP_LoadFontTable(LPFP_IO lpIO, LPFP_LOAD_TABLES lpTables)
{
FP_REC_TABLE RecTable;
short        i;
short        rc = 0;
long         lSize;

rc = fpIO_Read(lpIO, &lSize, sizeof(long));
if (!(rc = fpIO_Read(lpIO, &RecTable, sizeof(FP_REC_TABLE))))
   {
   if (RecTable.nCntSaved)
      {
      if ((lpTables->Font.hData = fpGlobalAlloc(GHND,
           RecTable.nCntSaved * sizeof(FP_REC_LOGFONT))) &&
          (lpTables->Font.hIndex = fpGlobalAlloc(GHND,
           RecTable.nCnt * sizeof(FP_LOAD_TABLEINDEX))))
         {
         LPFP_REC_LOGFONT     lpRec_FontItem = (LPFP_REC_LOGFONT)
                                 fpGlobalLock(lpTables->Font.hData);
         LPFP_LOAD_TABLEINDEX lpLoad_TableIndex = (LPFP_LOAD_TABLEINDEX)
                                 fpGlobalLock(lpTables->Font.hIndex);

         rc = fpIO_Read(lpIO, lpRec_FontItem, RecTable.nCntSaved *
                        sizeof(FP_REC_LOGFONT));

         for (i = 0; i < RecTable.nCnt; i++)
            lpLoad_TableIndex[i].nSavedID = i;

         fpGlobalUnlock(lpTables->Font.hData);
         fpGlobalUnlock(lpTables->Font.hIndex);

         lpTables->Font.nCnt = RecTable.nCnt;
         lpTables->Font.nCntSaved = RecTable.nCntSaved;
         }
      }
   }

return (rc);
}


short FPLIB FP_LoadColorTable(LPFP_IO lpIO, LPFP_LOAD_TABLES lpTables)
{
FP_REC_TABLE RecTable;
short        i;
short        rc = 0;
long         lSize;

rc = fpIO_Read(lpIO, &lSize, sizeof(long));
if (!(rc = fpIO_Read(lpIO, &RecTable, sizeof(FP_REC_TABLE))))
   {
   if (RecTable.nCntSaved)
      {
      if ((lpTables->Color.hData = fpGlobalAlloc(GHND,
           RecTable.nCntSaved * sizeof(COLORREF))) &&
          (lpTables->Color.hIndex = fpGlobalAlloc(GHND,
           RecTable.nCnt * sizeof(FP_LOAD_TABLEINDEX))))
         {
         LPCOLORREF           lpRec_ColorItem = (LPCOLORREF)
                                 fpGlobalLock(lpTables->Color.hData);
         LPFP_LOAD_TABLEINDEX lpLoad_TableIndex = (LPFP_LOAD_TABLEINDEX)
                                 fpGlobalLock(lpTables->Color.hIndex);

         rc = fpIO_Read(lpIO, lpRec_ColorItem, RecTable.nCntSaved *
                        sizeof(COLORREF));

         for (i = 0; i < RecTable.nCnt; i++)
            lpLoad_TableIndex[i].nSavedID = i + 1;

         fpGlobalUnlock(lpTables->Color.hData);
         fpGlobalUnlock(lpTables->Color.hIndex);

         lpTables->Color.nCnt = RecTable.nCnt;
         lpTables->Color.nCntSaved = RecTable.nCntSaved;
         }
      }
   }

return (rc);
}


short FPLIB FP_LoadPictTable(LPFP_IO lpIO, LPFP_LOAD_TABLES lpTables)
{
FP_REC_TABLE RecTable;
short        rc = 0;
long         lSize;

rc = fpIO_Read(lpIO, &lSize, sizeof(long));
if (!(rc = fpIO_Read(lpIO, &RecTable, sizeof(FP_REC_TABLE))))
   {
   if (RecTable.nCntSaved)
      {
      if ((lpTables->Pict.hData = fpGlobalAlloc(GHND,
           RecTable.nCntSaved * sizeof(FP_REC_PICT))) &&
          (lpTables->Pict.hIndex = fpGlobalAlloc(GHND,
           RecTable.nCnt * sizeof(FP_LOAD_TABLEINDEX))))
         {
         LPFP_REC_PICT        lpRec_PictItem = (LPFP_REC_PICT)
                                 fpGlobalLock(lpTables->Pict.hData);
         LPFP_LOAD_TABLEINDEX lpLoad_TableIndex = (LPFP_LOAD_TABLEINDEX)
                                 fpGlobalLock(lpTables->Pict.hIndex);
         long                 lLen;
         short                i;

         for (i = 0; i < RecTable.nCntSaved && !rc; i++)
             {
             if (!(rc = fpIO_Read(lpIO, (LPVOID)&lLen, sizeof(long))) &&
                  (lLen > 0))
                {
                if (lpRec_PictItem[i].hBuffer = GlobalAlloc(GHND, lLen))
                   {
                   HPSTR hpBuffer = (HPSTR)GlobalLock(lpRec_PictItem[i].hBuffer);
                   rc = fpIO_Read(lpIO, hpBuffer, lLen);
                   GlobalUnlock(lpRec_PictItem[i].hBuffer);

                   lpRec_PictItem[i].lLen = lLen;
                   }
                }
             }
         for (i = 0; i < RecTable.nCnt; i++)
            lpLoad_TableIndex[i].nSavedID = i + 1;

         fpGlobalUnlock(lpTables->Pict.hData);
         fpGlobalUnlock(lpTables->Pict.hIndex);

         lpTables->Pict.nCnt = RecTable.nCnt;
         lpTables->Pict.nCntSaved = RecTable.nCntSaved;
         }
      else
         rc = -1;
      }
   }

return (rc);
}


void FPLIB FP_LoadFreeTable(LPFP_LOAD_TABLE lpTable)
{
if (lpTable->hData)
   fpGlobalFree(lpTable->hData);

if (lpTable->hIndex)
   fpGlobalFree(lpTable->hIndex);
}

// fix for 9336 -scl
void FPLIB FP_LoadFreePictTable(LPFP_LOAD_TABLE lpTable)
{
  LPFP_REC_PICT lpRec_PictItem;
  int n;

  if( lpTable == NULL )
    return;
  lpRec_PictItem = (LPFP_REC_PICT)fpGlobalLock(lpTable->hData);
  for( n = 0; n < lpTable->nCnt; n++ )
    if( lpRec_PictItem[n].hBuffer != NULL )
      GlobalFree(lpRec_PictItem[n].hBuffer);

  if (lpTable->hData)
    fpGlobalFree(lpTable->hData);

  if (lpTable->hIndex)
    fpGlobalFree(lpTable->hIndex);
}

void FPLIB FP_LoadFreeTables(LPFP_LOAD_TABLES lpTables)
{
  FP_LoadFreeTable(&lpTables->Color);
  FP_LoadFreeTable(&lpTables->Font);
  FP_LoadFreePictTable(&lpTables->Pict);
}

FPCOLORID FPLIB FP_LoadConvertColorID(LPFP_LOAD_TABLES lpTables,
                                FPCOLORID idColorOld)
{
FPCOLORID idColor = 0;

if (idColorOld > 0 && idColorOld <= lpTables->Color.nCnt &&
    lpTables->Color.hData && lpTables->Color.hIndex)
   {
   LPFP_LOAD_TABLEINDEX lpLoad_TableIndex = (LPFP_LOAD_TABLEINDEX)
                           fpGlobalLock(lpTables->Color.hIndex);

   if (!lpLoad_TableIndex[idColorOld - 1].nNewID)
      {
      LPCOLORREF lpRec_Color = (LPCOLORREF)
                               fpGlobalLock(lpTables->Color.hData);

      lpRec_Color = &lpRec_Color[lpLoad_TableIndex[idColorOld - 1].nSavedID - 1];

      lpLoad_TableIndex[idColorOld - 1].nNewID =
         fpCM_AddItem(*lpRec_Color);

      fpGlobalUnlock(lpTables->Color.hData);
      }
   else
      fpCM_RefItem(lpLoad_TableIndex[idColorOld - 1].nNewID);

   idColor = lpLoad_TableIndex[idColorOld - 1].nNewID;
   fpGlobalUnlock(lpTables->Color.hIndex);
   }

return (idColor);
}

FPCOLORID FPLIB FP_LoadConvertColorIDEx(HINSTANCE hInst, LPFP_LOAD_TABLES lpTables,
                                FPCOLORID idColorOld)
{
FPCOLORID idColor = 0;

if (idColorOld > 0 && idColorOld <= lpTables->Color.nCnt &&
    lpTables->Color.hData && lpTables->Color.hIndex)
   {
   LPFP_LOAD_TABLEINDEX lpLoad_TableIndex = (LPFP_LOAD_TABLEINDEX)
                           fpGlobalLock(lpTables->Color.hIndex);

   if (!lpLoad_TableIndex[idColorOld - 1].nNewID)
      {
      LPCOLORREF lpRec_Color = (LPCOLORREF)
                               fpGlobalLock(lpTables->Color.hData);

      lpRec_Color = &lpRec_Color[lpLoad_TableIndex[idColorOld - 1].nSavedID - 1];

      lpLoad_TableIndex[idColorOld - 1].nNewID =
         fpCM_AddItemEx(hInst, *lpRec_Color);

      fpGlobalUnlock(lpTables->Color.hData);
      }
   else
      fpCM_RefItemEx(hInst, lpLoad_TableIndex[idColorOld - 1].nNewID);

   idColor = lpLoad_TableIndex[idColorOld - 1].nNewID;
   fpGlobalUnlock(lpTables->Color.hIndex);
   }

return (idColor);
}


FPFONT FPLIB FP_LoadConvertFontID(LPFP_LOAD_TABLES lpTables, FPFONT idFontOld)
{
FPFONT idFont = NOFONT;

if (idFontOld >= 0 && idFontOld < lpTables->Font.nCnt &&
    lpTables->Font.hData && lpTables->Font.hIndex)
   {
   LPFP_LOAD_TABLEINDEX lpLoad_TableIndex = (LPFP_LOAD_TABLEINDEX)
                           fpGlobalLock(lpTables->Font.hIndex);

//   if (!lpLoad_TableIndex[idFontOld].nNewID)
      {
      LPFP_REC_LOGFONT lpRec_Font = (LPFP_REC_LOGFONT)
                          fpGlobalLock(lpTables->Font.hData);
      LOGFONT          LogFont;

      lpRec_Font = &lpRec_Font[lpLoad_TableIndex[idFontOld].nSavedID];

      _fmemset(&LogFont, '\0', sizeof(LOGFONT));

      LogFont.lfHeight = MulDiv((int)lpRec_Font->lfHeight,
                                dyPixelsPerInch, HEIGHT_UNITS_PER_INCH);
      LogFont.lfWidth = 0;
      LogFont.lfEscapement = (int)lpRec_Font->lfEscapement;
      LogFont.lfOrientation = (int)lpRec_Font->lfOrientation;
      LogFont.lfWeight = (int)lpRec_Font->lfWeight;
      LogFont.lfItalic = lpRec_Font->lfItalic;
      LogFont.lfUnderline = lpRec_Font->lfUnderline;
      LogFont.lfStrikeOut = lpRec_Font->lfStrikeOut;
      LogFont.lfCharSet = lpRec_Font->lfCharSet;
      LogFont.lfOutPrecision = lpRec_Font->lfOutPrecision;
      LogFont.lfClipPrecision = lpRec_Font->lfClipPrecision;
      LogFont.lfQuality = lpRec_Font->lfQuality;
      LogFont.lfPitchAndFamily = lpRec_Font->lfPitchAndFamily;

#ifdef _UNICODE
assert(0);  // Fix this code for UNICODE!!!
//    lstrcpy(LogFont.lfFaceName, lpRec_Font->lfFaceName);
#endif
      lstrcpy((LPTSTR)LogFont.lfFaceName, (LPTSTR)lpRec_Font->lfFaceName);

      lpLoad_TableIndex[idFontOld].nNewID = fpGetLogFont(&LogFont);

      fpGlobalUnlock(lpTables->Font.hData);
      }

   idFont = lpLoad_TableIndex[idFontOld].nNewID;
   fpGlobalUnlock(lpTables->Font.hIndex);
   }

return (idFont);
}

FPFONT FPLIB FP_LoadConvertFontIDEx(HINSTANCE hInst, LPFP_LOAD_TABLES lpTables, FPFONT idFontOld)
{
FPFONT idFont = NOFONT;

if (idFontOld >= 0 && idFontOld < lpTables->Font.nCnt &&
    lpTables->Font.hData && lpTables->Font.hIndex)
   {
   LPFP_LOAD_TABLEINDEX lpLoad_TableIndex = (LPFP_LOAD_TABLEINDEX)
                           fpGlobalLock(lpTables->Font.hIndex);

//   if (!lpLoad_TableIndex[idFontOld].nNewID)
      {
      LPFP_REC_LOGFONT lpRec_Font = (LPFP_REC_LOGFONT)
                          fpGlobalLock(lpTables->Font.hData);
      LOGFONT          LogFont;

      lpRec_Font = &lpRec_Font[lpLoad_TableIndex[idFontOld].nSavedID];

      _fmemset(&LogFont, '\0', sizeof(LOGFONT));

      LogFont.lfHeight = MulDiv((int)lpRec_Font->lfHeight,
                                dyPixelsPerInch, HEIGHT_UNITS_PER_INCH);
      LogFont.lfWidth = 0;
      LogFont.lfEscapement = (int)lpRec_Font->lfEscapement;
      LogFont.lfOrientation = (int)lpRec_Font->lfOrientation;
      LogFont.lfWeight = (int)lpRec_Font->lfWeight;
      LogFont.lfItalic = lpRec_Font->lfItalic;
      LogFont.lfUnderline = lpRec_Font->lfUnderline;
      LogFont.lfStrikeOut = lpRec_Font->lfStrikeOut;
      LogFont.lfCharSet = lpRec_Font->lfCharSet;
      LogFont.lfOutPrecision = lpRec_Font->lfOutPrecision;
      LogFont.lfClipPrecision = lpRec_Font->lfClipPrecision;
      LogFont.lfQuality = lpRec_Font->lfQuality;
      LogFont.lfPitchAndFamily = lpRec_Font->lfPitchAndFamily;

#ifdef _UNICODE
assert(0);  // Fix this code for UNICODE!!!
//    lstrcpy(LogFont.lfFaceName, lpRec_Font->lfFaceName);
#endif
      lstrcpy((LPTSTR)LogFont.lfFaceName, (LPTSTR)lpRec_Font->lfFaceName);

      lpLoad_TableIndex[idFontOld].nNewID = fpGetLogFontEx(hInst, &LogFont);

      fpGlobalUnlock(lpTables->Font.hData);
      }

   idFont = lpLoad_TableIndex[idFontOld].nNewID;
   fpGlobalUnlock(lpTables->Font.hIndex);
   }

return (idFont);
}

FPPICTID FPLIB FP_LoadConvertPictID(HANDLE hInst, LPFP_LOAD_TABLES lpTables, FPPICTID idPictOld, BYTE bCtlType)
{
FPPICTID idPict = 0;

if (idPictOld > 0 && idPictOld <= (FPPICTID)lpTables->Pict.nCnt &&
    lpTables->Pict.hData && lpTables->Pict.hIndex)
   {
   LPFP_LOAD_TABLEINDEX lpLoad_TableIndex = (LPFP_LOAD_TABLEINDEX)
                           fpGlobalLock(lpTables->Pict.hIndex);

   if (!lpLoad_TableIndex[idPictOld - 1].nNewID)
      {
      LPFP_REC_PICT lpRec_Pict = (LPFP_REC_PICT)
                                 fpGlobalLock(lpTables->Pict.hData);

      lpRec_Pict = &lpRec_Pict[lpLoad_TableIndex[idPictOld - 1].nSavedID - 1];

      if (lpRec_Pict->hBuffer)
         {
         LPSTR lpszBuffer = (LPSTR)GlobalLock(lpRec_Pict->hBuffer);

         lpLoad_TableIndex[idPictOld - 1].nNewID =
            fpPM_Load(hInst, lpszBuffer, lpRec_Pict->lLen, bCtlType);

         GlobalUnlock(lpRec_Pict->hBuffer);
		   GlobalFree(lpRec_Pict->hBuffer); // free the handle! -scl
         lpRec_Pict->hBuffer = 0; // RFW - 2/10/05 - 15696
         }

      fpGlobalUnlock(lpTables->Pict.hData);
      }
   else
      fpPM_RefItem(hInst, lpLoad_TableIndex[idPictOld - 1].nNewID);

   idPict = lpLoad_TableIndex[idPictOld - 1].nNewID;
   fpGlobalUnlock(lpTables->Pict.hIndex);
   }

return (idPict);
}
