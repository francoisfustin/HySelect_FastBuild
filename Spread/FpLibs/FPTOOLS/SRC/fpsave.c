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

extern FPGLOBALHANDLE hPictureTable;
extern FPPICTID       dPictureTableCnt;


short FPLIB fpIO_Write(LPFP_IO lpfpIO, HPVOID hpData, long lSize)
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
      long           lSizeTemp;

      if (hTemp = fpGlobalAlloc(GHND, 30000))
         {
         lpszTemp = (LPSTR)fpGlobalLock(hTemp);

         for (; lSize > 0 && rc == 0; lSize -= 30000, hpDataTemp += 30000)
            {
            lSizeTemp = min(lSize, 30000);
            fpMemHugeCpy(lpszTemp, hpDataTemp, lSizeTemp);
            rc = VBWriteFormFile(lpfpIO->IO.hf, (LPVOID)lpszTemp,
                                 (WORD)lSizeTemp);
            }

         fpGlobalUnlock(hTemp);
         fpGlobalFree(hTemp);
         }
      else
         rc = -1;
      }
   else
      rc = VBWriteFormFile(lpfpIO->IO.hf, (LPVOID)hpData, (WORD)lSize);
   }
else
#endif
   fpHugeBufferAlloc(&lpfpIO->IO.Buffer, hpData, lSize);

return (rc);
}

short FPLIB FP_SaveTables(HANDLE hInst, LPFP_IO lpIO)
{
short rc;

rc = FP_SaveColorTable(lpIO);
rc = FP_SaveFontTable(lpIO);
rc = FP_SavePictTable(hInst, lpIO);

return (rc);
}

short FPLIB FP_SaveTablesEx(HANDLE hInst, LPFP_IO lpIO)
{
short rc;

rc = FP_SaveColorTableEx(hInst, lpIO);
rc = FP_SaveFontTableEx(hInst, lpIO);
rc = FP_SavePictTable(hInst, lpIO);

return (rc);
}

short FPLIB FP_SaveValue(LPFP_IO lpIO, BYTE bTag, LPVOID lpValue, long lValSize)
{
short rc;

rc = fpIO_Write(lpIO, &bTag, sizeof(BYTE));
rc = fpIO_Write(lpIO, &lValSize, sizeof(long));

if (lValSize)
   rc = fpIO_Write(lpIO, lpValue, lValSize);

return (rc);
}


short FPLIB FP_SaveData(LPFP_IO lpIO, LPVOID lpValue, long lValSize)
{
short rc;

if (lValSize)
   rc = fpIO_Write(lpIO, lpValue, lValSize);

return (rc);
}


short FPLIB FP_SaveByte(LPFP_IO lpIO, BYTE bValue)
{
return (fpIO_Write(lpIO, &bValue, sizeof(BYTE)));
}


short FPLIB FP_SaveWord(LPFP_IO lpIO, WORD wValue)
{
return (fpIO_Write(lpIO, &wValue, sizeof(WORD)));
}


short FPLIB FP_SaveLong(LPFP_IO lpIO, LONG lValue)
{
return (fpIO_Write(lpIO, &lValue, sizeof(long)));
}


short FPLIB FP_SaveText(LPFP_IO lpIO, BYTE bTag, FPGLOBALHANDLE hText)
{
short rc = 0;

#ifdef _UNICODE
// NOTE: This code has not been properly converted for UNICODE - SCP 12/16/98
assert(0);
#endif

if (hText)
   {
   LPSTR lpszText = (LPSTR)fpGlobalLock(hText);
   long  lLen = 0;

   rc = fpIO_Write(lpIO, &bTag, sizeof(BYTE));

   lLen = lstrlen((LPTSTR)lpszText);
   rc = fpIO_Write(lpIO, &lLen, sizeof(long));

   if (lLen)
      rc = fpIO_Write(lpIO, lpszText, lLen);

   fpGlobalUnlock(hText);
   }

return (rc);
}

short FPLIB FP_SaveColorTable(LPFP_IO lpIO)
{
COLORREF clr;
short    nCnt = (short)fpCM_GetCount();
short    i;
short    rc = 0;

if (nCnt)
   {
   FP_REC_TABLE RecTable;

   RecTable.nCnt = nCnt;
   RecTable.nCntSaved = nCnt;
   rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE) +
                    (nCnt * sizeof(COLORREF)));
   rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));

   for (i = 1; i <= nCnt; i++)
      {
      clr = fpCM_GetItem(i, NULL, NULL);
      rc = fpIO_Write(lpIO, &clr, sizeof(COLORREF));
      }
   }
else
   {
   FP_REC_TABLE RecTable;

   RecTable.nCnt = 0;
   RecTable.nCntSaved = 0;
   rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE));
   rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));
   }
   
return (rc);
}

short FPLIB FP_SaveColorTableEx(HINSTANCE hInst, LPFP_IO lpIO)
{
COLORREF clr;
short    nCnt = (short)fpCM_GetCountEx(hInst);
short    i;
short    rc = 0;

if (nCnt)
   {
   FP_REC_TABLE RecTable;

   RecTable.nCnt = nCnt;
   RecTable.nCntSaved = nCnt;
   rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE) +
                    (nCnt * sizeof(COLORREF)));
   rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));

   for (i = 1; i <= nCnt; i++)
      {
      clr = fpCM_GetItemEx(hInst, i, NULL, NULL);
      rc = fpIO_Write(lpIO, &clr, sizeof(COLORREF));
      }
   }
else
   {
   FP_REC_TABLE RecTable;

   RecTable.nCnt = 0;
   RecTable.nCntSaved = 0;
   rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE));
   rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));
   }
   
return (rc);
}


short FPLIB FP_SaveFontTable(LPFP_IO lpIO)
{
LPFPFONTENTRY  lpFontTable;
FP_REC_TABLE   RecTable;
FP_REC_LOGFONT LogFont;
UINT           fuSize = 0;
UINT           i;
short          rc = 0;

if (lpFontTable = fpLockGlobalData(&dmFontTable, NULL, &fuSize))
   {
   if (fuSize)
      {
      RecTable.nCnt = fuSize;
      RecTable.nCntSaved = fuSize;
      rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE) +
                       (fuSize * sizeof(FP_REC_LOGFONT)));
      rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));

      for (i = 0; i < fuSize; i++)
         {
         _fmemset(&LogFont, '\0', sizeof(FP_REC_LOGFONT));
         LogFont.lfHeight = MulDiv(lpFontTable[i].LogFont.lfHeight,
                                   HEIGHT_UNITS_PER_INCH, dyPixelsPerInch);
         LogFont.lfEscapement = lpFontTable[i].LogFont.lfEscapement;
         LogFont.lfOrientation = lpFontTable[i].LogFont.lfOrientation;
         LogFont.lfWeight = lpFontTable[i].LogFont.lfWeight;
         LogFont.lfItalic = lpFontTable[i].LogFont.lfItalic;
         LogFont.lfUnderline = lpFontTable[i].LogFont.lfUnderline;
         LogFont.lfStrikeOut = lpFontTable[i].LogFont.lfStrikeOut;
         LogFont.lfCharSet = lpFontTable[i].LogFont.lfCharSet;
         LogFont.lfOutPrecision = lpFontTable[i].LogFont.lfOutPrecision;
         LogFont.lfClipPrecision = lpFontTable[i].LogFont.lfClipPrecision;
         LogFont.lfQuality = lpFontTable[i].LogFont.lfQuality;
         LogFont.lfPitchAndFamily = lpFontTable[i].LogFont.lfPitchAndFamily;

#ifdef _UNICODE
assert(0);  // Fix this code for UNICODE!!!
//       lstrcpy(LogFont.lfFaceName, lpFontTable[i].LogFont.lfFaceName);
#endif
         lstrcpy((LPTSTR)LogFont.lfFaceName, (LPTSTR)lpFontTable[i].LogFont.lfFaceName);

         rc = fpIO_Write(lpIO, &LogFont, sizeof(FP_REC_LOGFONT));
         }
      }

   fpUnlockGlobalData(&dmFontTable, NULL);
   }
else
   {
   FP_REC_TABLE RecTable;

   RecTable.nCnt = 0;
   RecTable.nCntSaved = 0;
   rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE));
   rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));
   }
   

return (rc);
}

short FPLIB FP_SaveFontTableEx(HINSTANCE hInst, LPFP_IO lpIO)
{
LPFPFONTENTRY  lpFontTable;
FP_REC_TABLE   RecTable;
FP_REC_LOGFONT LogFont;
UINT           fuSize = 0;
UINT           i;
short          rc = 0;

if (lpFontTable = fpLockGlobalData(&dmFontTable, hInst, &fuSize))
   {
   if (fuSize)
      {
      RecTable.nCnt = fuSize;
      RecTable.nCntSaved = fuSize;
      rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE) +
                       (fuSize * sizeof(FP_REC_LOGFONT)));
      rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));

      for (i = 0; i < fuSize; i++)
         {
         _fmemset(&LogFont, '\0', sizeof(FP_REC_LOGFONT));
         LogFont.lfHeight = MulDiv(lpFontTable[i].LogFont.lfHeight,
                                   HEIGHT_UNITS_PER_INCH, dyPixelsPerInch);
         LogFont.lfEscapement = lpFontTable[i].LogFont.lfEscapement;
         LogFont.lfOrientation = lpFontTable[i].LogFont.lfOrientation;
         LogFont.lfWeight = lpFontTable[i].LogFont.lfWeight;
         LogFont.lfItalic = lpFontTable[i].LogFont.lfItalic;
         LogFont.lfUnderline = lpFontTable[i].LogFont.lfUnderline;
         LogFont.lfStrikeOut = lpFontTable[i].LogFont.lfStrikeOut;
         LogFont.lfCharSet = lpFontTable[i].LogFont.lfCharSet;
         LogFont.lfOutPrecision = lpFontTable[i].LogFont.lfOutPrecision;
         LogFont.lfClipPrecision = lpFontTable[i].LogFont.lfClipPrecision;
         LogFont.lfQuality = lpFontTable[i].LogFont.lfQuality;
         LogFont.lfPitchAndFamily = lpFontTable[i].LogFont.lfPitchAndFamily;

#ifdef _UNICODE
assert(0);  // Fix this code for UNICODE!!!
//       lstrcpy(LogFont.lfFaceName, lpFontTable[i].LogFont.lfFaceName);
#endif
         lstrcpy((LPTSTR)LogFont.lfFaceName, (LPTSTR)lpFontTable[i].LogFont.lfFaceName);

         rc = fpIO_Write(lpIO, &LogFont, sizeof(FP_REC_LOGFONT));
         }
      }

   fpUnlockGlobalData(&dmFontTable, hInst);
   }
else
   {
   FP_REC_TABLE RecTable;

   RecTable.nCnt = 0;
   RecTable.nCntSaved = 0;
   rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE));
   rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));
   }
   

return (rc);
}


short FPLIB FP_SavePictTable(HANDLE hInst, LPFP_IO lpIO)
{
FP_REC_TABLE RecTable;
HGLOBAL      hBuffer;
HPSTR        hpBuffer;
FPPICTID     i;
long         lSize;
UINT         nPictTblCnt = fpPM_GetCount(hInst);
short        rc = 0;

if (nPictTblCnt)
   {
   RecTable.nCnt = nPictTblCnt;
   RecTable.nCntSaved = nPictTblCnt;
   rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE));
   rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));

   for (i = 0; i < nPictTblCnt; i++)
      {
      if (hBuffer = fpPM_Save(hInst, (FPPICTID)(i + 1), &lSize))
         {
         rc = FP_SaveLong(lpIO, lSize);

         hpBuffer = (HPSTR)GlobalLock(hBuffer);
         rc = fpIO_Write(lpIO, hpBuffer, lSize);

         GlobalUnlock(hBuffer);
         GlobalFree(hBuffer);
         }
      else
         rc = FP_SaveLong(lpIO, 0);
      }
   }
else
   {
   FP_REC_TABLE RecTable;

   RecTable.nCnt = 0;
   RecTable.nCntSaved = 0;
   rc = FP_SaveLong(lpIO, sizeof(FP_REC_TABLE));
   rc = FP_SaveData(lpIO, &RecTable, sizeof(FP_REC_TABLE));
   }
   

return (rc);
}

