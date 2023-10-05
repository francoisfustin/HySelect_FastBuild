/*********************************************************
* SS_SAVE.C          
*
* Copyright (C) 1991-1993 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* RAP01 - GIC8975 - Change Load & Save to read & write in chunks instead of   SEP.27.99
*         buffering the entire SS.
* 28.jul.01 - jb - Tabfile will call SaveRangeToTextFile.
*********************************************************/
#if ((defined(SS_OCX) || defined(FP_DLL)) && defined(WIN32))
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#endif
#endif

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <math.h>
#include "..\..\..\..\fplibs\fptools\src\fpconvrt.h"
#include "spread.h"
#include "fphdc.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_bord.h"
#include "ss_calc.h"
#include "ss_cell.h"
#include "ss_col.h"
// 97' 2/5 Modified by BOC Gao. for BUG 000739
// We should use SS_SetXXX instead of set it directly, shouldn't we?
#ifdef SPREAD_JPN
#include "ss_doc.h"
#endif
// ----------------------------------------<<
#include "ss_draw.h"
#include "ss_emode.h"
#include "ss_font.h"
#include "ss_formu.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_save.h"
#include "ss_save4.h"
#include "ss_type.h"
#include "ss_text.h"
#include "ss_user.h"
#include "ss_virt.h"
#include "ss_win.h"

#include "..\classes\wintools.h"
#include "..\..\vbsrc\vbspread.h"

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
#include "..\calc\cal_cust.h"
#include "..\calc\cal_func.h"
#include "..\calc\cal_tok.h"
#include "..\calc\calc.h"
#endif

#if (defined(SS_BOUNDCONTROL) || defined(SS_QE) || defined(SS_VB))
//DBOCX...
#ifdef SS_OCX
#include "..\..\ssocx\ssocxdb.h"
#endif //SS_OCX
//DBOCX.
#endif

#ifdef SS_OCX
#include "ss_spict.h"      //prototype for CreateOcxPict()
#endif

#ifndef SS_NOSAVE

#define BUFFER_ALLOC_CNT 4096

BOOL   SS_SaveBufferToFile(LPCTSTR lpFileName, GLOBALHANDLE hBuffer,
                           long lBufferLen);
#ifdef WIN32
BOOL   SSx_WriteBufferToFile(HANDLE hFile, GLOBALHANDLE hBuffer,
                             long lBufferLen);
#else
BOOL   SSx_WriteBufferToFile(HFILE hFile, GLOBALHANDLE hBuffer,
                             long lBufferLen);
#endif

HPBYTE SS_SaveFontTable(LPSPREADSHEET lpSS, HPBYTE lpBuffer,
                        LPLONG lpBufferLen, LPLONG lpAllocLen,
                        LPHANDLE lphBuffer);
HPBYTE SS_SaveColorTable(LPSPREADSHEET lpSS, HPBYTE lpBuffer,
                         LPLONG lpBufferLen, LPLONG lpAllocLen,
                         LPHANDLE lphBuffer);
HPBYTE SS_SaveFormula(LPSPREADSHEET lpSS, HPBYTE lpBuffer,
                      LPLONG lpBufferLen, LPLONG lpAllocLen,
                      LPHANDLE lphBuffer, TBGLOBALHANDLE hCalc);
HPBYTE SS_SaveCellData(LPSPREADSHEET lpSS, HPBYTE lpBuffer,
                       LPLONG lpBufferLen,
                       LPLONG lpAllocLen, LPHANDLE lphBuffer,
                       LPSS_CELL lpCell,
                       SS_COORD Col, SS_COORD Row);
HPBYTE SS_SaveButtonPicture(LPSPREADSHEET lpSS, HPBYTE Buffer,
                            LPLONG BufferLen, LPLONG AllocLen,
                            LPHANDLE hBuffer, COLORREF Color,
                            HANDLE hPicture, short nPictureType,
                            LPBOOL lpfPaletteSaved);
HPBYTE SS_SavePicture(LPSPREADSHEET lpSS, HPBYTE Buffer, LPLONG BufferLen,
                      LPLONG AllocLen, LPHANDLE hBuffer, COLORREF Color,
                      HANDLE hPicture, long lStyle,
                      LPBOOL lpfPaletteSaved);
HPBYTE SS_SaveIcon(LPSPREADSHEET lpSS, HPBYTE Buffer, LPLONG BufferLen,
                   LPLONG AllocLen, LPHANDLE hBuffer, HICON hIcon,
                   COLORREF Color, LPBOOL lpfPaletteSaved);
HPBYTE SS_SaveBitmap(HPBYTE Buffer, LPLONG BufferLen,
                     LPLONG AllocLen, LPHANDLE hBuffer,
                     HBITMAP hBitmap, LPBOOL lpfPaletteSaved);
void   SS_SaveAddColor(LPSPREADSHEET lpSS, LPCOLORREF lpSaveColorTable,
                       LPSS_COLORITEM lpColorNew,
                       LPSS_SAVECOLORITEM lpColorOld);
BOOL   SS_LoadAllFromBuffer2(LPSPREADSHEET lpSS,
                             HPBYTE lpBuffer, long lBufferLen);
BOOL   SS_LoadAllFromBuffer3(LPSPREADSHEET lpSS,
                             HPBYTE lpBuffer, long lBufferLen);
HPBYTE SS_LoadBitmap(HPBYTE lpBuffer, long lBufferLen,
                     LPHBITMAP lphBitmap, LPHPALETTE lphPalDefault,
                     LPSHORT lpdPaletteSize,
                     LPGLOBALHANDLE lphGlobalPalette);
LPTSTR SS_LoadData(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                   HPTSTR hpText, LPTSTR lpBuffer,
                   LPGLOBALHANDLE lphBuffer, LPSHORT lpdAllocLen);
LPTSTR SS_LoadCellData(HWND hWnd, SS_COORD Col, SS_COORD Row,
                       HPTSTR hpText, LPTSTR lpBuffer,
                       LPGLOBALHANDLE lphBuffer, LPSHORT lpdAllocLen);
short  SS_SaveInitFont(LPSPREADSHEET lpSS, LPSS_SAVEFONTTBL lpSaveFont);
HPBYTE SS_SaveButtonPict(LPSPREADSHEET lpSS, HPBYTE Buffer, LPLONG BufferLen,
                         LPLONG AllocLen, LPHANDLE hBuffer,
                         LPBOOL lpfPaletteSaved, GLOBALHANDLE hPictName,
                         short nPictureType, COLORREF Color);
HPBYTE SS_SaveCheckBoxPict(LPSPREADSHEET lpSS, HPBYTE Buffer,
                           LPLONG BufferLen, LPLONG AllocLen,
                           LPHANDLE hBuffer, LPBOOL lpfPaletteSaved,
                           TBGLOBALHANDLE hPictName, short nPictureType,
                           COLORREF Color);
HPBYTE SS_LoadButtonPict(LPSPREADSHEET lpSS,
                         HPBYTE lpBuffer, long lBufferLen,
                         LPHPALETTE lphPalDefault, LPSHORT lpdPaletteSize,
                         LPGLOBALHANDLE lphGlobalPalette,
                         HANDLE hPictName, LPSHORT lpnPictType,
                         LPTSTR FAR *lplpPict, HBITMAP FAR *lphBitmap);
HPBYTE SS_SaveDataAware(HWND hWnd, HPBYTE Buffer, LPLONG BufferLen,
                        LPLONG AllocLen, LPHANDLE hBuffer);
HPBYTE SS_LoadDataAware(HWND hWnd, HPBYTE lpBuffer, LONG lBufferLen);
void   SS_LoadBorder(LPSPREADSHEET lpSS, LPCOLORREF lpSaveColorTable,
                     short dColorTableCnt, LPTBGLOBALHANDLE hBorderNew,
                     LPSS_SAVEBORDER lpBorderOld);
SS_COLORID SS_LoadColor(LPSPREADSHEET lpSS, LPCOLORREF lpSaveColorTable,
                        SS_COLORID ColorIdOld);
long StrHugeLen(HPTSTR hpStr);

#ifdef SS_VB
#ifndef SS_NOPRINT
//void       vbSpreadPreSave(HWND hWnd, LPSPREADSHEET lpSS);
//void       vbSpreadPostLoad(HWND hWnd, LPSPREADSHEET lpSS);
#endif
#endif

#ifdef SS_QE
extern void vbSSClearDB(HCTL hCtl, HWND hWnd);
#endif

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
static int StrCopyToken(LPCALC_TOKEN lpToken,
                        LPTSTR lpszText, int nTextLen)
{
  int nLen = min(lpToken->nLen, nTextLen - 1);
  
  lstrcpyn(lpszText, lpToken->lpszText, nLen + 1);
  lpszText[nLen] = '\0';
  return nLen;
}
#endif

//--------------------------------------------------------------------

#ifndef SS_OLDCALC
void SS_RegisterCustFuncs(LPSPREADSHEET lpSS, LPCTSTR lpszFormula)
{
CALC_TOKENARRAY tokenArray;
LPCALC_TOKEN lpToken;
CALC_HANDLE hFunc;
TCHAR szFunc[CALC_MAX_NAME_LEN];
int i;

if (TokenArrayInit(&tokenArray))
   {
   if (TokenParse(lpszFormula, &tokenArray))
      {
      for (i = 0; i < tokenArray.nSize; i++)
         {
         lpToken = tokenArray.lpItem + i;
         if (CALC_TOKEN_FUNCTION == lpToken->nType)
            {
            StrCopyToken(lpToken, szFunc, sizeof(szFunc)/sizeof(TCHAR));
            if (FuncLookup(szFunc))
               ; // nothing to do
            else if (hFunc = CustFuncLookup(&lpSS->lpBook->CalcInfo.CustFuncs, szFunc))
               {
               if (lpToken->u.Func.nArgs < CustFuncGetMinArgs(hFunc))
                  CustFuncSetMinArgs(hFunc, (short)lpToken->u.Func.nArgs);
               if (lpToken->u.Func.nArgs > CustFuncGetMaxArgs(hFunc))
                  CustFuncSetMaxArgs(hFunc, (short)lpToken->u.Func.nArgs);
               }
            else if (hFunc = CustFuncCreate(szFunc, (short)lpToken->u.Func.nArgs, (short)lpToken->u.Func.nArgs, NULL, 0))
               {
               CustFuncAdd(&lpSS->lpBook->CalcInfo.CustFuncs, hFunc);
               CustFuncDestroy(hFunc);
               }
            }
         }
      }
   TokenArrayFree(&tokenArray);
   }
}
#endif

//--------------------------------------------------------------------

BOOL DLLENTRY SSSaveToFile(HWND hWnd, LPCTSTR lpFileName, BOOL fDataOnly)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL fRet = SS_SaveToFile(lpBook, lpFileName, fDataOnly);
SS_BookUnlock(hWnd);

return (fRet);
}

BOOL SS_SaveToFile(LPSS_BOOK lpBook, LPCTSTR lpFileName, BOOL fDataOnly)
{
GLOBALHANDLE hBuffer;
long         lBufferLen;
BOOL         fRet = TRUE;

#if defined(SS_V30) && defined(WIN32)

lpBook->hFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
if (INVALID_HANDLE_VALUE == lpBook->hFile)
	return (FALSE);

hBuffer = SS_SaveToBuffer(lpBook, &lBufferLen, fDataOnly, &fRet);

CloseHandle(lpBook->hFile);
lpBook->hFile = (HANDLE)0;

#else

if (hBuffer = SS_SaveToBuffer(lpBook, &lBufferLen, fDataOnly, &fRet))
   fRet = SS_SaveBufferToFile(lpFileName, hBuffer, lBufferLen);
#endif

return (fRet);
}

BOOL DLLENTRY SSSaveTabFile(HWND hWnd, LPCTSTR lpFileName)
{
	LPSPREADSHEET lpSS;
	BOOL          fRet;

	lpSS = SS_SheetLock(hWnd);
//GAB 01/10/02 This is the ANSI version of SSSaveTabFile so the fUnicode parameter
// is set to FALSE
	fRet = SS_SaveTabFile(lpSS, lpFileName, FALSE);
	SS_SheetUnlock(hWnd);
	return fRet;
}

//GAB 01/10/02 New function added to allow users to save a Tab delimited file as a Unicode text file.
#ifdef SS_V40
BOOL DLLENTRY SSSaveTabFileU(HWND hWnd, LPCTSTR lpFileName)
{
	LPSPREADSHEET lpSS;
	BOOL          fRet;

	lpSS = SS_SheetLock(hWnd);
	fRet = SS_SaveTabFile(lpSS, lpFileName, TRUE);
	SS_SheetUnlock(hWnd);
	return fRet;
}
#endif //SS_V40

#ifdef WIN32
	#ifdef SS_V35
// jb - 28.jul.01 ---------------------------------------------------------------
// Changed the SaveTabFile because we want to be consistent with file saving when
// it comes to checkboxes.  Before, when a checkbox was encountered, if it was 
// unchecked, it was blank, if it was checked, then unchecked, it was 0.  
// Now, the save methods save a BLANK for all unchecked and checked/unchecked checkboxes.
//
// I called the new function SS_ExportRangeToTextFile sending it NO cell delimiter, 
// tab (\t) for a column delimiter, and a carriage return/line feed for the row delimiter.  
// Save data *Un-Formatted* (4) since that what was happening in the SS_ClipOut function 
// that was called in the original SaveTabFile.  No log file will be generated (NULL)
//
//GAB 01/10/02 Added the new bUnicode parameter to allow users to use SaveTabFile (ANSI) or 
// SaveTabFileU (UNICODE)
BOOL SS_SaveTabFile(LPSPREADSHEET lpSS, LPCTSTR lpFileName, BOOL fUnicode)
{
	BOOL fRet = FALSE;
// GAB 03/28/02 Added SS_EXPORTTEXT_TABFILE define so the export would be recognized as a tab file
// and the cell delimiters would not be used. Also removed the flag to save unformatted data.Rick and I 
// discussed this one and he agreed it should be like the ActiveX and save formatted data.
//  fRet = SS_ExportRangeToTextFile (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, lpFileName, 0, _T("\t"), _T("\r\n"), 4, NULL, fUnicode);
fRet = SS_ExportRangeToTextFile (lpSS, SS_ALLCOLS, SS_ALLROWS, 0, 0, lpFileName, 0, _T("\t"), _T("\r\n"), SS_EXPORTTEXT_TABFILE, NULL, fUnicode);	return fRet;
}

	#else // SS_V35

//GAB 01/10/02 Changed to be consistent with function prototype. fUnicode is not
//used in this version.
BOOL SS_SaveTabFile(LPSPREADSHEET lpSS, LPCTSTR lpFileName, BOOL fUnicode)
{
BOOL fRet = FALSE;
GLOBALHANDLE  hBuffer;
SS_COORD      ColCnt;
SS_COORD      RowCnt;

if (SSGetDataCnt(lpSS->lpBook->hWnd, &ColCnt, &RowCnt) && ColCnt == 0 && RowCnt == 0)
{	fRet = SS_SaveBufferToFile(lpFileName, NULL, 0);
}
else
{
   long     lMaxCells = 1000;
   long     lRows = (lMaxCells / lpSS->Col.Max) + 1;
   SS_COORD Row;
   HANDLE   hFile;

   hFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

   if (INVALID_HANDLE_VALUE != hFile)
      {
      for (Row = 1; Row <= lpSS->Row.Max; Row += lRows)
         if (hBuffer = SS_ClipOut(lpSS, 1, Row, lpSS->Col.Max,
                                  min(Row + lRows - 1, lpSS->Row.Max), FALSE, FALSE))
            fRet = SSx_WriteBufferToFile(hFile, hBuffer, -1);
         else
            break; // fRet = FALSE;

      CloseHandle(hFile);
      }
   }
   return fRet;
}
	#endif // SS_V32
#else // not WIN32 and Spread 3

//GAB 01/10/02 Changed to be consistent with function prototype. fUnicode is not
//used in this version.
BOOL SS_SaveTabFile(LPSPREADSHEET lpSS, LPCTSTR lpFileName, BOOL fUnicode)
{
GLOBALHANDLE  hBuffer;
HPTSTR        hpBuffer;
long          lBufferLen;
BOOL          fRet = FALSE;
SS_COORD      ColCnt;
SS_COORD      RowCnt;

if (hBuffer = SS_ClipOut(lpSS, 1, 1, lpSS->Col.Max, lpSS->Row.Max, FALSE, FALSE))
   {
   hpBuffer = (HPTSTR)GlobalLock(hBuffer);
   lBufferLen = StrHugeLen(hpBuffer);
   GlobalUnlock(hBuffer);
   fRet = SS_SaveBufferToFile(lpFileName, hBuffer, lBufferLen);
   }
else if (SSGetDataCnt(lpSS->lpBook->hWnd, &ColCnt, &RowCnt)
         && ColCnt == 0 && RowCnt == 0)
   fRet = SS_SaveBufferToFile(lpFileName, NULL, 0);

return fRet;
}

#endif // win32


BOOL SS_SaveBufferToFile(LPCTSTR lpFileName, GLOBALHANDLE hBuffer,
                         long lBufferLen)
{
BOOL     Ret = TRUE;
#if defined(WIN32)
HANDLE   hFile;
#else
HFILE    hFile;
OFSTRUCT of;
#endif

#if defined(WIN32)
hFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                   NULL);
if (INVALID_HANDLE_VALUE == hFile)
#else
hFile = OpenFile(lpFileName, &of, OF_CREATE | OF_WRITE | OF_SHARE_EXCLUSIVE);
if (HFILE_ERROR == hFile)
#endif
   {
   if(hBuffer)
      GlobalFree(hBuffer);
   return (FALSE);
   }

Ret = SSx_WriteBufferToFile(hFile, hBuffer, lBufferLen);

#if defined(WIN32)
CloseHandle(hFile);
#else   
_lclose(hFile);
#endif
return (Ret);
}


#ifdef WIN32

BOOL SSx_WriteBufferToFile(HANDLE hFile, GLOBALHANDLE hBuffer,
                           long lBufferLen)
{
HPBYTE hpBuffer;
long   i;
BOOL   Ret = TRUE;
short  Bytes;
DWORD  BytesWritten;

if(hBuffer)
   {
   hpBuffer = (HPBYTE)GlobalLock(hBuffer);

   if (lBufferLen == -1)
//GAB 10/18/01 Modified for Unicode
      //lBufferLen = StrHugeLen(hpBuffer);
	  lBufferLen = StrHugeLen((HPTSTR)hpBuffer);

   for (i = 0; i < lBufferLen; i += SS_FILESAVESIZE)
      {
      Bytes = (short)min((long)SS_FILESAVESIZE, lBufferLen - i);

      if (!WriteFile(hFile, hpBuffer, Bytes, &BytesWritten, NULL) ||
          (DWORD)Bytes != BytesWritten)
         {
         Ret = FALSE;
         break;
         }

      hpBuffer += Bytes;
      }

   GlobalUnlock(hBuffer);
   GlobalFree(hBuffer);
   }

return (Ret);
}

#else

BOOL SSx_WriteBufferToFile(HFILE hFile, GLOBALHANDLE hBuffer,
                           long lBufferLen)
{
HPBYTE hpBuffer;
long   i;
BOOL   Ret = TRUE;
short  Bytes;

if(hBuffer)
   {
   hpBuffer = (HPBYTE)GlobalLock(hBuffer);

   if (lBufferLen == -1)
      lBufferLen = StrHugeLen(hpBuffer);

   for (i = 0; i < lBufferLen; i += SS_FILESAVESIZE)
      {
      Bytes = (short)min((long)SS_FILESAVESIZE, lBufferLen - i);

      if (_lwrite(hFile, hpBuffer, Bytes) != (WORD)Bytes)
         {
         Ret = FALSE;
         break;
         }

      hpBuffer += Bytes;
      }

   GlobalUnlock(hBuffer);
   GlobalFree(hBuffer);
   }

return (Ret);
}
#endif


GLOBALHANDLE DLLENTRY SSSaveToBuffer(hWnd, lpBufferLen, fDataOnly)

HWND          hWnd;
LPLONG        lpBufferLen;
BOOL          fDataOnly;
{
LPSS_BOOK     lpBook;
GLOBALHANDLE  hBuffer;

lpBook = SS_BookLock(hWnd);
hBuffer = SS_SaveToBuffer(lpBook, lpBufferLen, fDataOnly, NULL);
SS_BookUnlock(hWnd);
return (hBuffer);
}


GLOBALHANDLE SS_SaveToBuffer(LPSS_BOOK lpBook, LPLONG lpBufferLen,
                             BOOL fDataOnly, LPBOOL lpfRet)
{
LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
GLOBALHANDLE  hBuffer;

if (lpfRet)
	*lpfRet = TRUE;

SS_CellEditModeOff(lpSS, 0);
#ifdef SS_OLDSAVE
if (fDataOnly)
   hBuffer = SS_SaveData(lpSS, lpBufferLen);
else
   hBuffer = SS_SaveAll(lpSS, lpBufferLen);
#else
hBuffer = SS_SaveToBuffer4(lpBook, lpBufferLen, fDataOnly, lpfRet);
#endif

SS_BookUnlockActiveSheet(lpBook);
return hBuffer;
}

BOOL DLLENTRY SSLoadTabFile(HWND hWnd, LPCTSTR lpszFileName)
{
// GAB 03/27/02 - 9884 Start using SSLoadTextFile to load tab files now.
#if defined(SS_V40) || defined(SS_V35)
//	return SSLoadTextFile(hWnd, lpszFileName, _T(""), _T("\t"), _T("\r\n"), SS_LOADTEXT_NOHEADERS | SS_LOADTEXT_CLEARDATAONLY, NULL); 
	return SSLoadTextFile(hWnd, lpszFileName, _T(""), _T("\t"), _T("\r\n"), SS_LOADTEXT_TABFILE | SS_LOADTEXT_NOHEADERS, NULL); 
#else
  return SSLoadFromFile(hWnd, lpszFileName);
#endif
}

/* RAP01d
BOOL DLLENTRY SSLoadFromFile(hWnd, lpFileName)

HWND     hWnd;
LPCTSTR  lpFileName;
{
#if defined(WIN32)
HANDLE   hBuffer = 0;
DWORD    Bytes;
#else
HFILE    hBuffer = 0;
OFSTRUCT of;
WORD     Bytes;
TBGLOBALHANDLE hTmpRead = (TBGLOBALHANDLE)0;
LPSTR    lpTmpRead;
#endif
HPBYTE   lpBuffer;
long     lBufferLen = 0;
HANDLE   hFile;
BOOL     Ret;

#if defined(WIN32)
hFile = CreateFile(lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                   NULL);
if (INVALID_HANDLE_VALUE == hFile)
   return (FALSE);
#else
hFile = OpenFile(lpFileName, &of, OF_READ | OF_SHARE_EXCLUSIVE);
if (HFILE_ERROR == hFile)
   return (FALSE);

hTmpRead = tbGlobalAlloc(GHND, SS_FILEREADSIZE);
if (!hTmpRead)
  return (FALSE);
#endif

for (;;)
   {
   if (!hBuffer)
      {
      if (!(hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  SS_FILEREADSIZE)))
         {
         #if defined(WIN32)
         CloseHandle(hFile);
         #else
         _lclose(hFile);
         if (hTmpRead)
           tbGlobalFree(hTmpRead);
         #endif
         return (FALSE);
         }
      }

   else
      {
      GlobalUnlock(hBuffer);
      if (!(hBuffer = GlobalReAlloc(hBuffer, lBufferLen + SS_FILEREADSIZE,
                                    GMEM_MOVEABLE | GMEM_ZEROINIT)))
         {
         #if defined(WIN32)
         CloseHandle(hFile);
         #else
         _lclose(hFile);
         if (hTmpRead)
           tbGlobalFree(hTmpRead);
         #endif
         return (FALSE);
         }
      }

   lpBuffer = (HPBYTE)GlobalLock(hBuffer);

   #if defined(WIN32)
   ReadFile(hFile, &lpBuffer[lBufferLen], SS_FILEREADSIZE, &Bytes, NULL);
   #else
// Bytes = _lread(hFile, &lpBuffer[lBufferLen], SS_FILEREADSIZE);  //SCP+64
   lpTmpRead = (LPSTR)tbGlobalLock(hTmpRead);
   Bytes = _lread(hFile, lpTmpRead, SS_FILEREADSIZE);
   if ((Bytes != HFILE_ERROR) && (Bytes > 0))
     MemHugeCpy(lpBuffer+lBufferLen, (HPBYTE)lpTmpRead, (LONG)Bytes);
   tbGlobalUnlock(hTmpRead);
   #endif

   if (Bytes == -1)
      break;

   lBufferLen += Bytes;

   if (Bytes < SS_FILEREADSIZE)
      break;
   }

#if defined(WIN32)
CloseHandle(hFile);
#else
_lclose(hFile);
tbGlobalFree(hTmpRead);                              //SCP+64
#endif
GlobalUnlock(hBuffer);

Ret = SSLoadFromBuffer(hWnd, hBuffer, lBufferLen);

GlobalFree(hBuffer);
return (Ret);
}
*/

BOOL DLLENTRY SSLoadFromFile(HWND hWnd, LPCTSTR lpFileName)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      fRet;

fRet = SS_LoadFromFile(lpBook, lpFileName);
SS_BookUnlock(hWnd);
return (fRet);
}


BOOL SS_LoadFromFile(LPSS_BOOK lpBook, LPCTSTR lpFileName)
{
#if defined(WIN32)
HANDLE   hBuffer = 0;
DWORD    Bytes;
#else
HFILE    hBuffer = 0;
OFSTRUCT of;
WORD     Bytes;
TBGLOBALHANDLE hTmpRead = (TBGLOBALHANDLE)0;
LPSTR    lpTmpRead;
#endif
HPBYTE   lpBuffer;
long     lBufferLen = 0;
HANDLE   hFile;
BOOL     fAllAtOnce = -1;
BOOL     Ret;

lpBook->hFile = (HANDLE)0;

if (lpBook->EditModeOn)
	{
	LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
	SS_CellEditModeOff(lpSS, 0);
	SS_BookUnlockActiveSheet(lpBook);
	}

#if defined(WIN32)
hFile = CreateFile(lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
if (INVALID_HANDLE_VALUE == hFile)
	return (FALSE);
#else
hFile = OpenFile(lpFileName, &of, OF_READ | OF_SHARE_EXCLUSIVE);
if (HFILE_ERROR == hFile)
	return (FALSE);

hTmpRead = tbGlobalAlloc(GHND, SS_FILEREADSIZE);
if (!hTmpRead)
	return (FALSE);
#endif

for (;;)
	{
	if (!hBuffer)
		{
		if (!(hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, SS_FILEREADSIZE)))
			{
#if defined(WIN32)
			CloseHandle(hFile);
#else
			_lclose(hFile);
			if (hTmpRead)
				tbGlobalFree(hTmpRead);
#endif
			return (FALSE);
			}
		}
	else
		{
		GlobalUnlock(hBuffer);
		if (!(hBuffer = GlobalReAlloc(hBuffer, lBufferLen + SS_FILEREADSIZE,
                                    GMEM_MOVEABLE | GMEM_ZEROINIT)))
			{
#if defined(WIN32)
			CloseHandle(hFile);
#else
			_lclose(hFile);
			if (hTmpRead)
				tbGlobalFree(hTmpRead);
#endif
			return (FALSE);
			}
		}

	lpBuffer = (HPBYTE)GlobalLock(hBuffer);

#if defined(WIN32)
	ReadFile(hFile, &lpBuffer[lBufferLen], SS_FILEREADSIZE, &Bytes, NULL);
#else
	lpTmpRead = (LPSTR)tbGlobalLock(hTmpRead);
	Bytes = _lread(hFile, lpTmpRead, SS_FILEREADSIZE);
	if ((Bytes != HFILE_ERROR) && (Bytes > 0))
		MemHugeCpy(lpBuffer+lBufferLen, (HPBYTE)lpTmpRead, (LONG)Bytes);
	tbGlobalUnlock(hTmpRead);
#endif

	if (Bytes == -1)
		break;

	lBufferLen += Bytes;

#ifdef WIN32
	if (SS_SAVEVERSION4 == lpBuffer[1] && lBufferLen >= 6)
		{
		fAllAtOnce = FALSE;
		lpBook->hFile = hFile;
		Ret = SS_LoadFromBuffer(lpBook, lpBuffer, lBufferLen);
		lpBook->hFile = (HANDLE)0;
		break;
		}
#endif

	if (Bytes < SS_FILEREADSIZE)
		break;
	}

#if defined(WIN32)
CloseHandle(hFile);
#else
_lclose(hFile);
tbGlobalFree(hTmpRead);                              //SCP+64
#endif
GlobalUnlock(hBuffer);

if (fAllAtOnce)
	Ret = SS_LoadFromBuffer(lpBook, hBuffer, lBufferLen);

GlobalFree(hBuffer);
return (Ret);
}


BOOL DLLENTRY SSLoadFromBuffer(hWnd, hBuffer, lBufferLen)

HWND         hWnd;
GLOBALHANDLE hBuffer;
long         lBufferLen;
{
LPSS_BOOK    lpBook;
HPBYTE       lpBuffer;
BOOL         bRet = FALSE;

lpBook = SS_BookLock(hWnd);
if( hBuffer )
{
  lpBuffer = (HPBYTE)GlobalLock(hBuffer);
  bRet = SS_LoadFromBuffer(lpBook, lpBuffer, lBufferLen);
  GlobalUnlock(hBuffer);
}
SS_BookUnlock(hWnd);
return bRet;
}


BOOL SS_LoadFromBuffer(LPSS_BOOK lpBook, HPBYTE lpBuffer, long lBufferLen)
{
BOOL fRet = TRUE;

if (lpBuffer)
   {
   fRet = FALSE;

   lpBook->fNoSetScrollBars = TRUE;

   if (SS_SAVEVERSION2 == lpBuffer[1] && lBufferLen >= 6)
   {
		LPSPREADSHEET lpSS = SS_BookLockSheet(lpBook);
      if (SS_FILETYPE_DATA == lpBuffer[0])
         fRet = SS_LoadDataFromBuffer(lpSS, lpBuffer, lBufferLen);
      else if (SS_FILETYPE_ALL == lpBuffer[0])
         fRet = SS_LoadAllFromBuffer2(lpSS, lpBuffer, lBufferLen);
		SS_BookUnlockSheet(lpBook);
   }
   else if (SS_SAVEVERSION3 == lpBuffer[1] && lBufferLen >= 6)
   {
		LPSPREADSHEET lpSS = SS_BookLockSheet(lpBook);
      if (SS_FILETYPE_DATA == lpBuffer[0])
         fRet = SS_LoadDataFromBuffer(lpSS, lpBuffer, lBufferLen);
      else if (SS_FILETYPE_ALL == lpBuffer[0])
         fRet = SS_LoadAllFromBuffer3(lpSS, lpBuffer, lBufferLen);
		SS_BookUnlockSheet(lpBook);
   }   
#ifndef SS_OLDSAVE
   else if (SS_SAVEVERSION4 == lpBuffer[1] && lBufferLen >= 6)
      fRet = SS_LoadFromBuffer4(lpBook, lpBuffer, lBufferLen);
#endif

   if (!fRet)
		{
		LPSPREADSHEET lpSS = SS_BookLockSheet(lpBook);
      fRet = SS_LoadTabDataFromBuffer(lpSS, lpBuffer, lBufferLen);
		SS_BookUnlockSheet(lpBook);
		}

   lpBook->fNoSetScrollBars = FALSE;

	{
	LPSPREADSHEET lpSS = SS_BookLockActiveSheet(lpBook);
   SS_SetVScrollBar(lpSS);
   SS_SetHScrollBar(lpSS);
	SS_BookUnlockSheet(lpBook);
	}

   SS_InvalidateRect(lpBook, NULL, TRUE);
   SS_AutoSize(lpBook, FALSE);
   SS_UpdateWindow(lpBook);
   }

return (fRet);
}


GLOBALHANDLE SS_SaveData(lpSS, lpBufferLen)

LPSPREADSHEET   lpSS;
long far       *lpBufferLen;
{
GLOBALHANDLE    hPack = 0;
TBGLOBALHANDLE  hData;
HPBYTE          Pack = NULL;
LPTSTR          Data;
BYTE            FileType = SS_FILETYPE_DATA;
BYTE            Version = SS_SAVEVERSION2;
long            PackLen = 0L;
long            AllocLen = 0L;
SS_CELLTYPE     CellType;
LPSS_ROW        lpRow;
LPSS_CELL       lpCell;
SS_COORD        i;
SS_COORD        j;
BOOL            DataDone;

*lpBufferLen = 0;

if (lpSS && lpSS->Row.DataCnt && lpSS->Col.DataCnt)
   {
   if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen,
                             (void FAR *)&FileType, (long)sizeof(BYTE),
                             &AllocLen, (LPHANDLE)&hPack)))
      return (NULL);

   if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen,
                             (void FAR *)&Version, (long)sizeof(BYTE),
                             &AllocLen, (LPHANDLE)&hPack)))
      return (NULL);

   if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen, NULL,
                             (long)sizeof(long), &AllocLen, (LPHANDLE)&hPack)))
      return (NULL);

   if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen,
                             (void FAR *)&lpSS->Row.DataCnt,
                             (long)sizeof(SS_COORD), &AllocLen,
                             (GLOBALHANDLE FAR *)&hPack)))
      return (NULL);

   if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen,
                             (void FAR *)&lpSS->Col.DataCnt,
                             (long)sizeof(SS_COORD), &AllocLen,
                             (GLOBALHANDLE FAR *)&hPack)))
      return (NULL);

   for (i = 0; i < lpSS->Row.DataCnt; i++)
      {
      lpRow = SS_LockRowItem(lpSS, i);

      for (j = 0; j < lpSS->Col.DataCnt; j++)
         {
         DataDone = FALSE;

         if (lpRow && (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i)))
            {
            SS_RetrieveCellType(lpSS, &CellType, lpCell, j, i);

            if (hData = SS_GetData(lpSS, &CellType, j, i, FALSE))
               {
               if (CellType.Type != SS_TYPE_PICTURE)
                  {
                  Data = (LPTSTR)tbGlobalLock(hData);

                  if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen,
                                            Data, (long)(lstrlen(Data) + 1),
                                            &AllocLen,
                                            (GLOBALHANDLE FAR *)&hPack)))
                     return (NULL);

                  tbGlobalUnlock(hData);

                  DataDone = TRUE;
                  }
               }

            SSx_UnlockCellItem(lpSS, lpRow, j, i);
            }

         if (!DataDone)
            if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen, NULL,
                                      1L, &AllocLen,
                                      (GLOBALHANDLE FAR *)&hPack)))
               return (NULL);
         }

      SS_UnlockRowItem(lpSS, i);
      }

   MemHugeCpy(&Pack[2], &PackLen, sizeof(long));
   GlobalUnlock(hPack);
   }

*lpBufferLen = PackLen;
return (hPack);
}


#ifdef SS_OLDSAVE
GLOBALHANDLE SS_SaveAll(lpSS, lpBufferLen)

LPSPREADSHEET     lpSS;
long far         *lpBufferLen;
{
GLOBALHANDLE      hPack = 0;
TBGLOBALHANDLE    hData;
SS_SAVESPREAD3    SaveSpread;
SS_SAVEROW2       SaveRow;
SS_SAVECOL2       SaveCol;
SS_SAVECELL2      SaveCell;
SS_CELLTYPE       CellType;
HPBYTE            Pack = NULL;
LPTSTR            PtrTemp;
BYTE              FileType = SS_FILETYPE_ALL;
BYTE              Version = SS_SAVEVERSION3;
long              PackLen = 0L;
long              AllocLen = 0L;
SS_COORD          i;
SS_COORD          j;
#ifndef SS_NOCALC
LPSS_CALC         Calc;
#endif
LPSS_ROW          lpRow;
LPSS_COL          lpCol;
LPSS_CELL         lpCell;
BYTE              bSaved = TRUE;
BYTE              bNoSave = FALSE;
BOOL              fPaletteSaved = FALSE;

#ifdef SS_VB
#ifndef SS_NOPRINT
//vbSpreadPreSave(lpSS->lpBook->hWnd, lpSS);
#endif
#endif

*lpBufferLen = 0;

/***************
* Add FileType
***************/

if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen, (void FAR *)&FileType,
                          (long)sizeof(BYTE), &AllocLen, (LPHANDLE)&hPack)))
   return (NULL);

if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen,
                          (void FAR *)&Version, (long)sizeof(BYTE),
                          &AllocLen, (LPHANDLE)&hPack)))
   return (NULL);

if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen, NULL,
                          (long)sizeof(long), &AllocLen, (LPHANDLE)&hPack)))
   return (NULL);

_fmemset(&SaveSpread, '\0', sizeof(SS_SAVESPREAD3));

SaveSpread.fUseFontPointSize = TRUE;

SaveSpread.DocumentProtected = lpSS->DocumentProtected;
SaveSpread.DocumentLocked = lpSS->DocumentLocked;

if (lpSS->wUserResize & (SS_USERRESIZE_COL | SS_USERRESIZE_ROW))
   SaveSpread.AllowResize = TRUE;

if (lpSS->wSelBlockOption & (SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
                                 SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL))
   SaveSpread.AllowSelectBlock = TRUE;

SaveSpread.RestrictRows = lpSS->RestrictRows;
SaveSpread.RestrictCols = lpSS->RestrictCols;
SaveSpread.CalcAuto = lpSS->lpBook->CalcAuto;
SaveSpread.Redraw = lpSS->Redraw;

if (lpSS->wGridType & (SS_GRID_HORIZONTAL | SS_GRID_VERTICAL))
   SaveSpread.GridLines = TRUE;

SaveSpread.HorzScrollBar = lpSS->HorzScrollBar;
SaveSpread.VertScrollBar = lpSS->VertScrollBar;
_fmemcpy(&SaveSpread.Color, &lpSS->Color, sizeof(SS_COLOR));
SaveSpread.DefaultFontId = lpSS->DefaultFontId;
SaveSpread.HeaderRows = lpSS->Row.HeaderCnt;
SaveSpread.HeaderCols = lpSS->Col.HeaderCnt;
SaveSpread.RowsFrozen = lpSS->Row.Frozen;
SaveSpread.ColsFrozen = lpSS->Col.Frozen;
SaveSpread.RowsMax = lpSS->Row.Max;
SaveSpread.ColsMax = lpSS->Col.Max;
SaveSpread.RowCnt = lpSS->Row.AllocCnt;
SaveSpread.ColCnt = lpSS->Col.AllocCnt;
SaveSpread.DataRowCnt = lpSS->Row.DataCnt;
SaveSpread.DataColCnt = lpSS->Col.DataCnt;

if (lpSS->Col.dCellSizeX100 == -1)
   SaveSpread.ColWidth = -1;
else
   SaveSpread.ColWidth = (double)lpSS->Col.dCellSizeX100 / 100.0;

if (lpSS->Row.dCellSizeX100 == -1)
   SaveSpread.RowHeight = -1;
else
   SaveSpread.RowHeight = (double)lpSS->Row.dCellSizeX100 / 100.0;

SaveSpread.FontTableCnt = dFontTableCnt;
SaveSpread.ColorTableCnt = dColorTableCnt;

SaveSpread.DefaultDateFormat.bCentury = lpSS->DefaultDateFormat.bCentury;
SaveSpread.DefaultDateFormat.cSeparator = (char)lpSS->DefaultDateFormat.cSeparator;
SaveSpread.DefaultDateFormat.nFormat = lpSS->DefaultDateFormat.nFormat;
SaveSpread.DefaultDateFormat.bSpin = lpSS->DefaultDateFormat.cSeparator;
SaveSpread.DefaultTimeFormat.b24Hour = lpSS->DefaultTimeFormat.b24Hour;
SaveSpread.DefaultTimeFormat.bSeconds = lpSS->DefaultTimeFormat.bSeconds;
SaveSpread.DefaultTimeFormat.cSeparator = (char)lpSS->DefaultTimeFormat.cSeparator;
SaveSpread.DefaultTimeFormat.bSpin = lpSS->DefaultTimeFormat.bSpin;

SaveSpread.AllowUserFormulas = lpSS->AllowUserFormulas;
SaveSpread.fEditModePermanent = lpSS->fEditModePermanent;
SaveSpread.fAutoSize = lpSS->fAutoSize;
SaveSpread.fScrollBarExtMode = lpSS->fScrollBarExtMode;
SaveSpread.fProcessTab = lpSS->fProcessTab;
SaveSpread.fScrollBarShowMax = lpSS->fScrollBarShowMax;
SaveSpread.fScrollBarMaxAlign = lpSS->fScrollBarMaxAlign;
SaveSpread.fArrowsExitEditMode = lpSS->fArrowsExitEditMode;
SaveSpread.fMoveActiveOnFocus = lpSS->fMoveActiveOnFocus;
SaveSpread.fRetainSelBlock = lpSS->fRetainSelBlock;
SaveSpread.fEditModeReplace = lpSS->fEditModeReplace;
SaveSpread.fNoBorder = lpSS->fNoBorder;
SaveSpread.fNoBeep = lpSS->fNoBeep;
SaveSpread.fAutoClipboard = lpSS->fAutoClipboard;
SaveSpread.fAllowMultipleSelBlocks = lpSS->fAllowMultipleSelBlocks;
SaveSpread.fAllowCellOverflow = lpSS->fAllowCellOverflow;
SaveSpread.fVirtualMode = lpSS->fVirtualMode;
SaveSpread.fAllowDragDrop = lpSS->fAllowDragDrop;
SaveSpread.fUseSpecialVScroll = lpSS->fUseSpecialVScroll;
SaveSpread.fSetBorder = lpSS->fSetBorder;
SaveSpread.wButtonDrawMode = lpSS->wButtonDrawMode;
SaveSpread.wEnterAction = lpSS->wEnterAction;
SaveSpread.wOpMode = lpSS->wOpMode;
SaveSpread.wGridType = lpSS->wGridType;
SaveSpread.wUserResize = lpSS->wUserResize;
SaveSpread.wSelBlockOption = lpSS->wSelBlockOption;
SaveSpread.wSpecialVScrollOptions = lpSS->wSpecialVScrollOptions;
SaveSpread.GridColor = lpSS->GridColor;
SaveSpread.dUnitType = (BYTE)lpSS->dUnitType;
SaveSpread.ColNumStart = lpSS->Col.NumStart;
SaveSpread.RowNumStart = lpSS->Row.NumStart;
SaveSpread.AutoSizeVisibleCols = lpSS->Col.AutoSizeVisibleCnt;
SaveSpread.AutoSizeVisibleRows = lpSS->Row.AutoSizeVisibleCnt;
SaveSpread.ColHeaderDisplay = lpSS->ColHeaderDisplay;
SaveSpread.RowHeaderDisplay = lpSS->RowHeaderDisplay;
_fmemcpy(&SaveSpread.Virtual, &lpSS->Virtual, sizeof(SS_VIRTUAL));
_fmemcpy(&SaveSpread.LockColor, &lpSS->LockColor, sizeof(SS_COLOR));
_fmemcpy(&SaveSpread.DefaultFloatFormat, &lpSS->DefaultFloatFormat,
         sizeof(FLOATFORMAT));

if (lpSS->DefaultData.bDataType &&
    (lpSS->DefaultData.bDataType == SS_TYPE_INTEGER ||
     lpSS->DefaultData.bDataType == SS_TYPE_FLOAT ||
     lpSS->DefaultData.Data.hszData))
   SaveSpread.fDataProvided = TRUE;

if (lpSS->hXtra)
   SaveSpread.bMiscFlags = SS_SAVEMISC_XTRA;

#if ((defined(SS_BOUNDCONTROL) || defined(SS_QE)) && !defined(SS_OCX))
SaveSpread.bMiscFlags = SS_SAVEMISC_DATAAWARE;
#endif

/***************************
* Add SaveSpread structure
***************************/

if (!(Pack = SS_SaveAlloc(Pack, (long FAR *)&PackLen, (void FAR *)&SaveSpread,
                          (long)sizeof(SS_SAVESPREAD3), &AllocLen,
                          (LPHANDLE)&hPack)))
   return (NULL);

if (SaveSpread.bMiscFlags & SS_SAVEMISC_XTRA)
   {
   HPBYTE hpXtraBuff;
   if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &lpSS->lXtraLen, 4,
                             &AllocLen, (LPHANDLE)&hPack)))
      return (NULL);

   hpXtraBuff = (HPBYTE)GlobalLock(lpSS->hXtra);
   if (!(Pack = SS_SaveAlloc(Pack, &PackLen, hpXtraBuff, lpSS->lXtraLen,
                             &AllocLen, (LPHANDLE)&hPack)))
      return (NULL);

   GlobalUnlock(lpSS->hXtra);
   }

#if ((defined(SS_BOUNDCONTROL) || defined(SS_QE)) && !defined(SS_OCX))
if (!(Pack = SS_SaveDataAware(lpSS->lpBook->hWnd, Pack, &PackLen, &AllocLen,
                              (LPHANDLE)&hPack)))
   return (NULL);
#endif

if (!(Pack = SS_SaveCellTypeExt(lpSS, Pack, &PackLen, &AllocLen,
                                (LPHANDLE)&hPack, &lpSS->DefaultCellType,
                                -1, -1, &fPaletteSaved)))
   return (NULL);

if (SaveSpread.fDataProvided)
   {
   if (hData = SS_GetData(lpSS, &CellType, SS_ALLCOLS, SS_ALLROWS, FALSE))
      {
      PtrTemp = (LPTSTR)tbGlobalLock(hData);
      if (!(Pack = SS_SaveAlloc(Pack, &PackLen, PtrTemp,
                                (long)lstrlen(PtrTemp) + 1, &AllocLen,
                                (LPHANDLE)&hPack)))
         return (NULL);

      tbGlobalUnlock(hData);
      }
   }

/**************************
* Add FontTable structure
**************************/

if (!(Pack = SS_SaveFontTable(lpSS, Pack, &PackLen, &AllocLen, &hPack)))
   return (NULL);

/***************************
* Add ColorTable structure
***************************/

if (!(Pack = SS_SaveColorTable(lpSS, Pack, &PackLen, &AllocLen, &hPack)))
   return (NULL);

/******************
* Add each column
******************/

if (SaveSpread.ColCnt)
   {
   for (i = 0; i < SaveSpread.ColCnt; i++)
      {
      if (lpCol = SS_LockColItem(lpSS, i))
         {
         _fmemset(&SaveCol, '\0', sizeof(SS_SAVECOL2));
         SaveCol.FontId = SS_FONT_NONE;
         SaveCol.ColWidth = SS_WIDTH_DEFAULT;

         if (lpCol->hCellType)
            SaveCol.fCellTypeProvided = TRUE;
         else
            SaveCol.fCellTypeProvided = FALSE;

         SaveCol.fFormulaProvided = FALSE;

#ifndef SS_NOCALC

         if (lpCol->hCalc)
            {
            Calc = (LPSS_CALC)tbGlobalLock(lpCol->hCalc);
            if (Calc->hFormula)
               SaveCol.fFormulaProvided = TRUE;

            tbGlobalUnlock(lpCol->hCalc);
            }
#endif

         if (lpCol->Data.bDataType &&
             (lpCol->Data.bDataType == SS_TYPE_INTEGER ||
              lpCol->Data.bDataType == SS_TYPE_FLOAT ||
              lpCol->Data.Data.hszData))
            SaveCol.fDataProvided = TRUE;
         else
            SaveCol.fDataProvided = FALSE;

         SaveCol.ColLocked = lpCol->ColLocked;

         if (lpCol->dColWidthX100 == -1)
            SaveCol.ColWidth = -1;
         else
            SaveCol.ColWidth = (double)lpCol->dColWidthX100 / 100.0;

         SaveCol.RowLastNonEmpty = (SS_COORD16)lpCol->RowLastNonEmpty;
         SaveCol.FontId = lpCol->FontId;
         _fmemcpy(&SaveCol.Color, &lpCol->Color, sizeof(SS_COLORITEM));
#ifndef SS_NOBORDERS
         _fmemcpy(&SaveCol.Border, &lpCol->Border, sizeof(SS_BORDER));
#endif
         SaveCol.fPageBreak = lpCol->fPageBreak;
         SaveCol.bUserResize = lpCol->bUserResize;
#ifndef SS_NOUSERDATA
         SaveCol.lUserData = lpCol->lUserData;
#endif

         SaveCol.Offset = 0;

         if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &bSaved,
                                   (long)sizeof(BYTE), &AllocLen,
                                   (LPHANDLE)&hPack)))
            return (NULL);

         if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &SaveCol,
                                   (long)sizeof(SS_SAVECOL2), &AllocLen,
                                   (LPHANDLE)&hPack)))
            return (NULL);

         if (SaveCol.fCellTypeProvided)
            if (!(Pack = SS_SaveCellType(lpSS, Pack, &PackLen, &AllocLen,
                                         (LPHANDLE)&hPack, lpCol->hCellType,
                                         i, -1, &fPaletteSaved)))
               return (NULL);

#ifndef SS_NOCALC
         if (SaveCol.fFormulaProvided)
            if (!(Pack = SS_SaveFormula(lpSS, Pack, &PackLen, &AllocLen,
                                        &hPack, lpCol->hCalc)))
               return (NULL);
#endif

         if (SaveCol.fDataProvided)
            if (!(Pack = SS_SaveCellData(lpSS, Pack, &PackLen, &AllocLen,
                                         &hPack, NULL, i,
                                         SS_ALLROWS)))
               return (NULL);

         /******************************
         * Save Data Aware information
         ******************************/

#if ((defined(SS_BOUNDCONTROL) || defined(SS_QE)) && !defined(SS_OCX))
         if (SaveSpread.bMiscFlags & SS_SAVEMISC_DATAAWARE)
            {
            if (!(Pack = SS_SaveAlloc(Pack, &PackLen, "\0\0\0\0", 4, // Unused
                                      &AllocLen, (LPHANDLE)&hPack))) // Bytes
               return (NULL);

            if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &lpCol->fDataFillEvent,
                                      1, &AllocLen, (LPHANDLE)&hPack)))
               return (NULL);

            if (lpCol->hFieldName)
               {
               LPTSTR lpText;

               lpText = (LPTSTR)tbGlobalLock(lpCol->hFieldName);
               if (!(Pack = SS_SaveAlloc(Pack, &PackLen, lpText,
                                         (lstrlen(lpText) + 1) * sizeof(TCHAR),
                                         &AllocLen, &hPack)))
                  return (NULL);

               tbGlobalUnlock(lpCol->hFieldName);
               }
            else
               if (!(Pack = SS_SaveAlloc(Pack, &PackLen, _T(""), sizeof(TCHAR),
                                         &AllocLen, &hPack)))
                  return (NULL);
            }
#endif

         SS_UnlockColItem(lpSS, i);
         }

      else
         if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &bNoSave,
                                   (long)sizeof(BYTE), &AllocLen,
                                   (LPHANDLE)&hPack)))
            return (NULL);
      }
   }

/***************
* Add each row
***************/

if (lpSS->Row.AllocCnt)
   {
   for (i = 0; i < SaveSpread.RowCnt; i++)
      {
      _fmemset(&SaveRow, '\0', sizeof(SS_SAVEROW2));
      SaveRow.FontId = SS_FONT_NONE;
      SaveRow.RowMaxFontId = SS_FONT_NONE;
      SaveRow.RowHeight = SS_HEIGHT_DEFAULT;

      if (lpRow = SS_LockRowItem(lpSS, i))
         {
         if (lpRow->hCellType)
            SaveRow.fCellTypeProvided = TRUE;

#ifndef SS_NOCALC
         if (lpRow->hCalc)
            {
            Calc = (LPSS_CALC)tbGlobalLock(lpRow->hCalc);
            if (Calc->hFormula)
               SaveRow.fFormulaProvided = TRUE;

            tbGlobalUnlock(lpRow->hCalc);
            }
#endif

         if (lpRow->Data.bDataType &&
             (lpRow->Data.bDataType == SS_TYPE_INTEGER ||
              lpRow->Data.bDataType == SS_TYPE_FLOAT ||
              lpRow->Data.Data.hszData))
            SaveRow.fDataProvided = TRUE;

         SaveRow.RowLocked = lpRow->RowLocked;

         if (lpRow->dRowHeightX100 == -1)
            SaveRow.RowHeight = -1;
         else
            SaveRow.RowHeight = (double)lpRow->dRowHeightX100 / 100.0;

         SaveRow.FontId = lpRow->FontId;
         _fmemcpy(&SaveRow.Color, &lpRow->Color, sizeof(SS_COLORITEM));
#ifndef SS_NOBORDERS
         _fmemcpy(&SaveRow.Border, &lpRow->Border, sizeof(SS_BORDER));
#endif
         SaveRow.fRowSelected = lpRow->fRowSelected;
         SaveRow.fPageBreak = lpRow->fPageBreak;
         SaveRow.bUserResize = lpRow->bUserResize;
#ifndef SS_NOUSERDATA
         SaveRow.lUserData = lpRow->lUserData;
#endif

         if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &bSaved,
                                   (long)sizeof(BYTE), &AllocLen,
                                   (LPHANDLE)&hPack)))
            return (NULL);

         if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &SaveRow,
                                   (long)sizeof(SS_SAVEROW2), &AllocLen,
                                   (LPHANDLE)&hPack)))
            return (NULL);

         if (SaveRow.fCellTypeProvided)
            if (!(Pack = SS_SaveCellType(lpSS, Pack, &PackLen, &AllocLen,
                                         (LPHANDLE)&hPack, lpRow->hCellType,
                                         -1, i, &fPaletteSaved)))
               return (NULL);

#ifndef SS_NOCALC
         if (SaveRow.fFormulaProvided)
            if (!(Pack = SS_SaveFormula(lpSS, Pack, &PackLen, &AllocLen, &hPack,
                                        lpRow->hCalc)))
               return (NULL);
#endif

         if (SaveRow.fDataProvided)
            if (!(Pack = SS_SaveCellData(lpSS, Pack, &PackLen, &AllocLen, &hPack,
                                         NULL, SS_ALLCOLS, i)))
               return (NULL);

         /****************
         * Add each Cell
         ****************/

         if (lpSS->Col.AllocCnt)
            {
            for (j = 0; j < SaveSpread.ColCnt; j++)
               {
               _fmemset(&SaveCell, '\0', sizeof(SS_SAVECELL2));
               SaveCell.FontId = SS_FONT_NONE;

               if (lpCell = SSx_LockCellItem(lpSS, lpRow, j, i))
                  {
                  if (lpCell->hCellType)
                     SaveCell.fCellTypeProvided = TRUE;

#ifndef SS_NOCALC
                  if (lpCell->hCalc)
                     {
                     Calc = (LPSS_CALC)tbGlobalLock(lpCell->hCalc);
                     if (Calc->hFormula)
                        SaveCell.fFormulaProvided = TRUE;

                     tbGlobalUnlock(lpCell->hCalc);
                     }
#endif

                  if (lpCell->Data.bDataType &&
                      (lpCell->Data.bDataType == SS_TYPE_INTEGER ||
                       lpCell->Data.bDataType == SS_TYPE_FLOAT ||
                       lpCell->Data.Data.hszData))
                     SaveCell.fDataProvided = TRUE;

                  SaveCell.CellLocked = lpCell->CellLocked;
                  SaveCell.FontId = lpCell->FontId;
                  _fmemcpy(&SaveCell.Color, &lpCell->Color, sizeof(SS_COLORITEM));
#ifndef SS_NOBORDERS
                  _fmemcpy(&SaveCell.Border, &lpCell->Border, sizeof(SS_BORDER));
#endif

                  if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &bSaved,
                                            (long)sizeof(BYTE), &AllocLen,
                                            (LPHANDLE)&hPack)))
                     return (NULL);

                  if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &SaveCell,
                                            (long)sizeof(SS_SAVECELL2),
                                            &AllocLen, (LPHANDLE)&hPack)))
                     return (NULL);

                  if (SaveCell.fCellTypeProvided)
                     if (!(Pack = SS_SaveCellType(lpSS, Pack, &PackLen,
                                                  &AllocLen, &hPack,
                                                  lpCell->hCellType,
                                                  j, i, &fPaletteSaved)))
                        return (NULL);

#ifndef SS_NOCALC
                  if (SaveCell.fFormulaProvided)
                     if (!(Pack = SS_SaveFormula(lpSS, Pack, &PackLen,
                                                 &AllocLen, &hPack,
                                                 lpCell->hCalc)))
                        return (NULL);
#endif

                  if (SaveCell.fDataProvided)
                     if (!(Pack = SS_SaveCellData(lpSS, Pack, &PackLen,
                                                  &AllocLen, &hPack,
                                                  lpCell, j, i)))
                        return (NULL);

                  SSx_UnlockCellItem(lpSS, lpRow, j, i);
                  }

               else
                  if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &bNoSave,
                                            (long)sizeof(BYTE), &AllocLen,
                                            (LPHANDLE)&hPack)))
                     return (NULL);
               }
            }

         SS_UnlockRowItem(lpSS, i);
         }

      else
         if (!(Pack = SS_SaveAlloc(Pack, &PackLen, &bNoSave,
                                   (long)sizeof(BYTE), &AllocLen,
                                   (LPHANDLE)&hPack)))
            return (NULL);
      }
   }

MemHugeCpy(&Pack[2], &PackLen, sizeof(long));

GlobalUnlock(hPack);

*lpBufferLen = PackLen;
return (hPack);
}


HPBYTE SS_SaveFontTable(lpSS, lpBuffer, lpBufferLen, lpAllocLen, lphBuffer)

LPSPREADSHEET lpSS;
HPBYTE        lpBuffer;
LPLONG        lpBufferLen;
LPLONG        lpAllocLen;
LPHANDLE      lphBuffer;
{
LPSS_FONT  lpFontTable;
short      dPointSize;
short      i;

lpFontTable = (LPSS_FONT)tbGlobalLock(hFontTable);

for (i = 0; i < dFontTableCnt; i++)
   {
   dPointSize = (short)PIXELS_TO_INTPT(lpFontTable[i].LogFont.lfHeight);

   if (!(lpBuffer = SS_SaveAlloc(lpBuffer, lpBufferLen,
                                 (void FAR *)&dPointSize,
                                 (long)sizeof(dPointSize), lpAllocLen,
                                 lphBuffer)))
      {
      tbGlobalUnlock(hFontTable);
      return (NULL);
      }

   if (!(lpBuffer = SS_SaveAlloc(lpBuffer, lpBufferLen,
                                 (void FAR *)&lpFontTable[i].LogFont,
                                 (long)sizeof(LOGFONT), lpAllocLen,
                                 lphBuffer)))
      {
      tbGlobalUnlock(hFontTable);
      return (NULL);
      }
   }

tbGlobalUnlock(hFontTable);

return (lpBuffer);
}


HPBYTE SS_SaveColorTable(lpSS, lpBuffer, lpBufferLen, lpAllocLen,
                         lphBuffer)

LPSPREADSHEET lpSS;
HPBYTE        lpBuffer;
LPLONG        lpBufferLen;
LPLONG        lpAllocLen;
LPHANDLE      lphBuffer;
{
LPSS_COLORTBLITEM lpColorTable;
short             i;

lpColorTable = (LPSS_COLORTBLITEM)tbGlobalLock(hColorTable);

for (i = 0; i < dColorTableCnt; i++)
   if (!(lpBuffer = SS_SaveAlloc(lpBuffer, lpBufferLen,
                             (void FAR *)&lpColorTable[i].Color,
                             (long)sizeof(COLORREF), lpAllocLen, lphBuffer)))
      {
      tbGlobalUnlock(hColorTable);
      return (NULL);
      }

tbGlobalUnlock(hColorTable);

return (lpBuffer);
}


#ifndef SS_NOCALC

HPBYTE SS_SaveFormula(lpSS, lpBuffer, lpBufferLen, lpAllocLen, lphBuffer,
                      hCalc)

LPSPREADSHEET  lpSS;
HPBYTE         lpBuffer;
LPLONG         lpBufferLen;
LPLONG         lpAllocLen;
LPHANDLE       lphBuffer;
TBGLOBALHANDLE hCalc;
{
LPSS_CALC      lpCalc;
LPTSTR         lpFormula;

lpCalc = (LPSS_CALC)tbGlobalLock(hCalc);

lpFormula = (LPTSTR)tbGlobalLock(lpCalc->hFormula);
if (!(lpBuffer = SS_SaveAlloc(lpBuffer, lpBufferLen, lpFormula,
                          (lstrlen(lpFormula) + 1) * sizeof(TCHAR),
                          lpAllocLen, lphBuffer)))
   return (NULL);

tbGlobalUnlock(lpCalc->hFormula);
tbGlobalUnlock(hCalc);

return (lpBuffer);
}

#endif


HPBYTE SS_SaveCellData(lpSS, lpBuffer, lpBufferLen, lpAllocLen, lphBuffer,
                       lpCell, Col, Row)

LPSPREADSHEET  lpSS;
HPBYTE         lpBuffer;
LPLONG         lpBufferLen;
LPLONG         lpAllocLen;
LPHANDLE       lphBuffer;
LPSS_CELL      lpCell;
SS_COORD       Col;
SS_COORD       Row;
{
TBGLOBALHANDLE hData;
SS_CELLTYPE    CellType;
LPTSTR         lpData;

SS_RetrieveCellType(lpSS, &CellType, lpCell, Col, Row);

if (hData = SS_GetData(lpSS, &CellType, Col, Row, FALSE))
   {
   lpData = (LPTSTR)tbGlobalLock(hData);
   if (!(lpBuffer = SS_SaveAlloc(lpBuffer, lpBufferLen, lpData,
                             (lstrlen(lpData) + 1) * sizeof(TCHAR),
                             lpAllocLen, lphBuffer)))
      return (NULL);

   tbGlobalUnlock(hData);
   }
else
   {
   BYTE bZero = 0;

   if (!(lpBuffer = SS_SaveAlloc(lpBuffer, lpBufferLen, &bZero, 1,
                                 lpAllocLen, lphBuffer)))
      return (NULL);
   }

return (lpBuffer);
}


HPBYTE SS_SaveCellType(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                       hCellType, Col, Row, lpfPaletteSaved)
LPSPREADSHEET  lpSS;
HPBYTE         Buffer;
LPLONG         BufferLen;
LPLONG         AllocLen;
LPHANDLE       hBuffer;
TBGLOBALHANDLE hCellType;
SS_COORD       Col;
SS_COORD       Row;
LPBOOL         lpfPaletteSaved;
{
LPSS_CELLTYPE lpCellType;

lpCellType = (LPSS_CELLTYPE)tbGlobalLock(hCellType);
Buffer = SS_SaveCellTypeExt(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                            lpCellType, Col, Row, lpfPaletteSaved);
tbGlobalUnlock(hCellType);
return (Buffer);
}


HPBYTE SS_SaveCellTypeExt(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                          lpCellType, Col, Row, lpfPaletteSaved)

LPSPREADSHEET lpSS;
HPBYTE        Buffer;
LPLONG        BufferLen;
LPLONG        AllocLen;
LPHANDLE      hBuffer;
LPSS_CELLTYPE lpCellType;
SS_COORD      Col;
SS_COORD      Row;
LPBOOL        lpfPaletteSaved;
{
SS_SAVECELLTYPE3 SaveCellType;
SS_COLORTBLITEM  ColorTblItem;
LPTSTR           lpText;
LPTSTR           lpTemp;
short            dStringLen;
short            dLen;
short            i;

_fmemset(&SaveCellType, 0, sizeof(SaveCellType));
SaveCellType.Type = lpCellType->Type;
SaveCellType.ControlID = lpCellType->ControlID;
SaveCellType.Style = lpCellType->Style;
switch (lpCellType->Type)
   {
   case SS_TYPE_EDIT:
      SaveCellType.Spec.Edit.ChrSet = lpCellType->Spec.Edit.ChrSet;
      SaveCellType.Spec.Edit.Len = lpCellType->Spec.Edit.Len;
      SaveCellType.Spec.Edit.ChrCase = (char)lpCellType->Spec.Edit.ChrCase;
      SaveCellType.Spec.Edit.dfWidth = lpCellType->Spec.Edit.dfWidth;
      SaveCellType.Spec.Edit.nRowCnt = lpCellType->Spec.Edit.nRowCnt;
      break;
   case SS_TYPE_INTEGER:
      SaveCellType.Spec.Integer.Min = lpCellType->Spec.Integer.Min;
      SaveCellType.Spec.Integer.Max = lpCellType->Spec.Integer.Max;
      SaveCellType.Spec.Integer.fSpinWrap = lpCellType->Spec.Integer.fSpinWrap;
      SaveCellType.Spec.Integer.lSpinInc = lpCellType->Spec.Integer.lSpinInc;
      break;
   case SS_TYPE_FLOAT:
      SaveCellType.Spec.Float.Left = lpCellType->Spec.Float.Left;
      SaveCellType.Spec.Float.Right = lpCellType->Spec.Float.Right;
      SaveCellType.Spec.Float.Min = lpCellType->Spec.Float.Min;
      SaveCellType.Spec.Float.Max = lpCellType->Spec.Float.Max;
      SaveCellType.Spec.Float.fSetFormat = lpCellType->Spec.Float.fSetFormat;
      SaveCellType.Spec.Float.Format.cCurrencySign = (BYTE)lpCellType->Spec.Float.Format.cCurrencySign;
      SaveCellType.Spec.Float.Format.cDecimalSign = (BYTE)lpCellType->Spec.Float.Format.cDecimalSign;
      SaveCellType.Spec.Float.Format.cSeparator = (BYTE)lpCellType->Spec.Float.Format.cSeparator;
      break;
   case SS_TYPE_COMBOBOX:
      SaveCellType.Spec.ComboBox.dItemCnt = lpCellType->Spec.ComboBox.dItemCnt;
      SaveCellType.Spec.ComboBox.hItems = lpCellType->Spec.ComboBox.hItems;
      break;
   case SS_TYPE_TIME:
      SaveCellType.Spec.Time.SetFormat = lpCellType->Spec.Time.SetFormat;
      SaveCellType.Spec.Time.SetRange = lpCellType->Spec.Time.SetRange;
      SaveCellType.Spec.Time.Format.b24Hour = lpCellType->Spec.Time.Format.b24Hour;
      SaveCellType.Spec.Time.Format.bSeconds = lpCellType->Spec.Time.Format.bSeconds;
      SaveCellType.Spec.Time.Format.cSeparator = (char)lpCellType->Spec.Time.Format.cSeparator;
      SaveCellType.Spec.Time.Format.bSpin = lpCellType->Spec.Time.Format.bSpin;
      SaveCellType.Spec.Time.Min.nHour = lpCellType->Spec.Time.Min.nHour;
      SaveCellType.Spec.Time.Min.nMinute = lpCellType->Spec.Time.Min.nMinute;
      SaveCellType.Spec.Time.Min.nSecond = lpCellType->Spec.Time.Min.nSecond;
      SaveCellType.Spec.Time.Max.nHour = lpCellType->Spec.Time.Max.nHour;
      SaveCellType.Spec.Time.Max.nMinute = lpCellType->Spec.Time.Max.nMinute;
      SaveCellType.Spec.Time.Max.nSecond = lpCellType->Spec.Time.Max.nSecond;
      break;
   case SS_TYPE_DATE:
      SaveCellType.Spec.Date.SetFormat = lpCellType->Spec.Date.SetFormat;
      SaveCellType.Spec.Date.SetRange = lpCellType->Spec.Date.SetRange;
      SaveCellType.Spec.Date.Format.bCentury = lpCellType->Spec.Date.Format.bCentury;
      SaveCellType.Spec.Date.Format.cSeparator = (char)lpCellType->Spec.Date.Format.cSeparator;
      SaveCellType.Spec.Date.Format.nFormat = lpCellType->Spec.Date.Format.nFormat;
      SaveCellType.Spec.Date.Format.bSpin = lpCellType->Spec.Date.Format.bSpin;
      SaveCellType.Spec.Date.Min.nDay = lpCellType->Spec.Date.Min.nDay;
      SaveCellType.Spec.Date.Min.nMonth = lpCellType->Spec.Date.Min.nMonth;
      SaveCellType.Spec.Date.Min.nYear = lpCellType->Spec.Date.Min.nYear;
      SaveCellType.Spec.Date.Max.nDay = lpCellType->Spec.Date.Max.nDay;
      SaveCellType.Spec.Date.Max.nMonth = lpCellType->Spec.Date.Max.nMonth;
      SaveCellType.Spec.Date.Max.nYear = lpCellType->Spec.Date.Max.nYear;
      break;
   case SS_TYPE_PIC:
      SaveCellType.Spec.Pic.hMask = lpCellType->Spec.Pic.hMask;
      break;
   case SS_TYPE_PICTURE:
      SaveCellType.Spec.ViewPict.hPictName = lpCellType->Spec.ViewPict.hPictName;
      SaveCellType.Spec.ViewPict.hPal = lpCellType->Spec.ViewPict.hPal;
      #ifdef SS_VB
      SaveCellType.Spec.ViewPict.hPic = lpCellType->Spec.ViewPict.hPic;
      #endif
      SaveCellType.Spec.ViewPict.fDeleteHandle = lpCellType->Spec.ViewPict.fDeleteHandle;
      break;
   case SS_TYPE_BUTTON:
      SaveCellType.Spec.Button.hText = (SS_SAVEHGLOBAL)lpCellType->Spec.Button.hText;
      SaveCellType.Spec.Button.hPictName = (SS_SAVEHGLOBAL)lpCellType->Spec.Button.hPictName;
      SaveCellType.Spec.Button.hPictDownName = (SS_SAVEHGLOBAL)lpCellType->Spec.Button.hPictDownName;
      SaveCellType.Spec.Button.nPictureType = (short)lpCellType->Spec.Button.nPictureType;
      SaveCellType.Spec.Button.nPictureDownType = (short)lpCellType->Spec.Button.nPictureDownType;
      SaveCellType.Spec.Button.ButtonType = lpCellType->Spec.Button.ButtonType;
      SaveCellType.Spec.Button.ShadowSize = lpCellType->Spec.Button.ShadowSize;
      SaveCellType.Spec.Button.fDelHandle = lpCellType->Spec.Button.fDelHandle;
      SaveCellType.Spec.Button.fDelDownHandle = lpCellType->Spec.Button.fDelDownHandle;
      SaveCellType.Spec.Button.Color.Color = lpCellType->Spec.Button.Color.Color;
      SaveCellType.Spec.Button.Color.ColorBorder = lpCellType->Spec.Button.Color.ColorBorder;
      SaveCellType.Spec.Button.Color.ColorShadow = lpCellType->Spec.Button.Color.ColorShadow;
      SaveCellType.Spec.Button.Color.ColorHighlight = lpCellType->Spec.Button.Color.ColorHighlight;
      SaveCellType.Spec.Button.Color.ColorText = lpCellType->Spec.Button.Color.ColorText;
      #ifdef SS_VB
      SaveCellType.Spec.Button.hPic = lpCellType->Spec.Button.hPic;
      SaveCellType.Spec.Button.hPicDown = lpCellType->Spec.Button.hPicDown;
      #endif
      break;
   case SS_TYPE_CHECKBOX:
      SaveCellType.Spec.CheckBox.hText = (SS_SAVEHGLOBAL)lpCellType->Spec.CheckBox.hText;
      SaveCellType.Spec.CheckBox.hPictUpName = (SS_SAVETBHANDLE)lpCellType->Spec.CheckBox.hPictUpName;
      SaveCellType.Spec.CheckBox.hPictDownName = (SS_SAVETBHANDLE)lpCellType->Spec.CheckBox.hPictDownName;
      SaveCellType.Spec.CheckBox.hPictFocusUpName = (SS_SAVETBHANDLE)lpCellType->Spec.CheckBox.hPictFocusUpName;
      SaveCellType.Spec.CheckBox.hPictFocusDownName = (SS_SAVETBHANDLE)lpCellType->Spec.CheckBox.hPictFocusDownName;
      SaveCellType.Spec.CheckBox.hPictGrayName = (SS_SAVETBHANDLE)lpCellType->Spec.CheckBox.hPictGrayName;
      SaveCellType.Spec.CheckBox.hPictFocusGrayName = (SS_SAVETBHANDLE)lpCellType->Spec.CheckBox.hPictFocusGrayName;
      SaveCellType.Spec.CheckBox.bPictUpType = lpCellType->Spec.CheckBox.bPictUpType;
      SaveCellType.Spec.CheckBox.bPictDownType = lpCellType->Spec.CheckBox.bPictDownType;
      SaveCellType.Spec.CheckBox.bPictFocusUpType = lpCellType->Spec.CheckBox.bPictFocusUpType;
      SaveCellType.Spec.CheckBox.bPictFocusDownType = lpCellType->Spec.CheckBox.bPictFocusDownType;
      SaveCellType.Spec.CheckBox.bPictGrayType = lpCellType->Spec.CheckBox.bPictGrayType;
      SaveCellType.Spec.CheckBox.bPictFocusGrayType = lpCellType->Spec.CheckBox.bPictFocusGrayType;
      SaveCellType.Spec.CheckBox.fDelUpHandle = lpCellType->Spec.CheckBox.fDelUpHandle;
      SaveCellType.Spec.CheckBox.fDelDownHandle = lpCellType->Spec.CheckBox.fDelDownHandle;
      SaveCellType.Spec.CheckBox.fDelFocusUpHandle = lpCellType->Spec.CheckBox.fDelFocusUpHandle;
      SaveCellType.Spec.CheckBox.fDelFocusDownHandle = lpCellType->Spec.CheckBox.fDelFocusDownHandle;
      SaveCellType.Spec.CheckBox.fDelGrayHandle = lpCellType->Spec.CheckBox.fDelGrayHandle;
      SaveCellType.Spec.CheckBox.fDelFocusGrayHandle = lpCellType->Spec.CheckBox.fDelFocusGrayHandle;
      #ifdef SS_VB
      SaveCellType.Spec.CheckBox.hPicUp = lpCellType->Spec.CheckBox.hPicUp;
      SaveCellType.Spec.CheckBox.hPicDown = lpCellType->Spec.CheckBox.hPicDown;
      SaveCellType.Spec.CheckBox.hPicFocusUp = lpCellType->Spec.CheckBox.hPicFocusUp;
      SaveCellType.Spec.CheckBox.hPicFocusDown = lpCellType->Spec.CheckBox.hPicFocusDown;
      SaveCellType.Spec.CheckBox.hPicGray = lpCellType->Spec.CheckBox.hPicGray;
      SaveCellType.Spec.CheckBox.hPicFocusGray = lpCellType->Spec.CheckBox.hPicFocusGray;
      #endif
      break;
   }

if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, &SaveCellType,
                            (long)sizeof(SaveCellType), AllocLen, hBuffer)))

   return (NULL);

switch (lpCellType->Type)
   {
#ifndef SS_NOCT_PIC
   case SS_TYPE_PIC:
      if (lpCellType->Spec.Pic.hMask)
         {
         lpText = (LPTSTR)tbGlobalLock(lpCellType->Spec.Pic.hMask);

         if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpText,
                                     (lstrlen(lpText) + 1) * sizeof(TCHAR),
                                     AllocLen, hBuffer)))
            return (NULL);

         tbGlobalUnlock(lpCellType->Spec.Pic.hMask);
         }

      break;
#endif

   case SS_TYPE_PICTURE:
      if (lpCellType->Spec.ViewPict.hPictName &&
          !(lpCellType->Style & VPS_HANDLE))
         {
         lpText = (LPTSTR)tbGlobalLock(lpCellType->Spec.ViewPict.hPictName);

         if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpText,
                                     (lstrlen(lpText) + 1) * sizeof(TCHAR),
                                     AllocLen, hBuffer)))
            return (NULL);

         tbGlobalUnlock(lpCellType->Spec.ViewPict.hPictName);
         }

      else if (lpCellType->Spec.ViewPict.hPictName &&
               (lpCellType->Style & VPS_HANDLE))
         {
         SSx_GetColorTblItem(lpSS, NULL, NULL, NULL, Col,
                             Row, &ColorTblItem, NULL);

         Buffer = SS_SavePicture(lpSS, Buffer, BufferLen, AllocLen,
                                 hBuffer, ColorTblItem.Color,
                                 (HBITMAP)lpCellType->Spec.ViewPict.hPictName,
                                 lpCellType->Style, lpfPaletteSaved);
         }

      break;

#ifndef SS_NOCT_BUTTON
   case SS_TYPE_BUTTON:
      if (lpCellType->Spec.Button.hText)
         {
         lpText = (LPTSTR)GlobalLock(lpCellType->Spec.Button.hText);

         if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpText,
                                     (lstrlen(lpText) + 1) * sizeof(TCHAR),
                                     AllocLen, hBuffer)))
            return (NULL);

         GlobalUnlock(lpCellType->Spec.Button.hText);
         }

      if (!(Buffer = SS_SaveButtonPict(lpSS, Buffer, BufferLen, AllocLen,
                                       hBuffer, lpfPaletteSaved,
                                       lpCellType->Spec.Button.hPictName,
                                       lpCellType->Spec.Button.nPictureType,
                                       lpCellType->Spec.Button.Color.Color)))
         return (NULL);

      if (!(Buffer = SS_SaveButtonPict(lpSS, Buffer, BufferLen, AllocLen,
                                      hBuffer, lpfPaletteSaved,
                                      lpCellType->Spec.Button.hPictDownName,
                                      lpCellType->Spec.Button.nPictureDownType,
                                      lpCellType->Spec.Button.Color.Color)))
         return (NULL);

      break;
#endif

#ifndef SS_NOCT_COMBO
   case SS_TYPE_COMBOBOX:
      if (lpCellType->Spec.ComboBox.hItems &&
          lpCellType->Spec.ComboBox.dItemCnt)
         {
         lpText = (LPTSTR)tbGlobalLock(lpCellType->Spec.ComboBox.hItems);

         lpTemp = lpText;
         dLen = 0;
         for (i = 0; i < lpCellType->Spec.ComboBox.dItemCnt; i++)
            {
            dStringLen = lstrlen(lpTemp) + 1;
            dLen += dStringLen;
            lpTemp += dStringLen;
            }

         if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpText, dLen * sizeof(TCHAR),
                                     AllocLen, hBuffer)))
            return (NULL);

         tbGlobalUnlock(lpCellType->Spec.ComboBox.hItems);
         }

      break;
#endif

#ifndef SS_NOCT_CHECK
   case SS_TYPE_CHECKBOX:
      if (lpCellType->Spec.CheckBox.hText)
         {
         lpText = (LPTSTR)GlobalLock(lpCellType->Spec.CheckBox.hText);

         if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpText,
                                     (lstrlen(lpText) + 1) * sizeof(TCHAR),
                                     AllocLen, hBuffer)))
            return (NULL);

         GlobalUnlock(lpCellType->Spec.CheckBox.hText);
         }

      SSx_GetColorTblItem(lpSS, NULL, NULL, NULL, Col,
                          Row, &ColorTblItem, NULL);

      if (!(Buffer = SS_SaveCheckBoxPict(lpSS, Buffer, BufferLen, AllocLen,
                     hBuffer, lpfPaletteSaved,
                     lpCellType->Spec.CheckBox.hPictUpName,
                     lpCellType->Spec.CheckBox.bPictUpType,
                     ColorTblItem.Color)))
         return (NULL);

      if (!(Buffer = SS_SaveCheckBoxPict(lpSS, Buffer, BufferLen, AllocLen,
                     hBuffer, lpfPaletteSaved,
                     lpCellType->Spec.CheckBox.hPictDownName,
                     lpCellType->Spec.CheckBox.bPictDownType,
                     ColorTblItem.Color)))
         return (NULL);

      if (!(Buffer = SS_SaveCheckBoxPict(lpSS, Buffer, BufferLen, AllocLen,
                     hBuffer, lpfPaletteSaved,
                     lpCellType->Spec.CheckBox.hPictFocusUpName,
                     lpCellType->Spec.CheckBox.bPictFocusUpType,
                     ColorTblItem.Color)))
         return (NULL);

      if (!(Buffer = SS_SaveCheckBoxPict(lpSS, Buffer, BufferLen, AllocLen,
                     hBuffer, lpfPaletteSaved,
                     lpCellType->Spec.CheckBox.hPictFocusDownName,
                     lpCellType->Spec.CheckBox.bPictFocusDownType,
                     ColorTblItem.Color)))
         return (NULL);

      if (!(Buffer = SS_SaveCheckBoxPict(lpSS, Buffer, BufferLen, AllocLen,
                     hBuffer, lpfPaletteSaved,
                     lpCellType->Spec.CheckBox.hPictGrayName,
                     lpCellType->Spec.CheckBox.bPictGrayType,
                     ColorTblItem.Color)))
         return (NULL);

      if (!(Buffer = SS_SaveCheckBoxPict(lpSS, Buffer, BufferLen, AllocLen,
                     hBuffer, lpfPaletteSaved,
                     lpCellType->Spec.CheckBox.hPictFocusGrayName,
                     lpCellType->Spec.CheckBox.bPictFocusGrayType,
                     ColorTblItem.Color)))
         return (NULL);

      break;
#endif
   }

return (Buffer);
}


#ifndef SS_NOCT_BUTTON
HPBYTE SS_SaveButtonPict(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                         lpfPaletteSaved, hPictName, nPictureType, Color)

LPSPREADSHEET lpSS;
HPBYTE        Buffer;
LPLONG        BufferLen;
LPLONG        AllocLen;
LPHANDLE      hBuffer;
LPBOOL        lpfPaletteSaved;
GLOBALHANDLE  hPictName;
short         nPictureType;
COLORREF      Color;
{
LPTSTR        lpText;

if (hPictName && !(nPictureType & SUPERBTN_PICT_HANDLE))
   {
   lpText = (LPTSTR)GlobalLock(hPictName);

   if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpText,
                               (lstrlen(lpText) + 1) * sizeof(TCHAR),
                               AllocLen, hBuffer)))
      return (NULL);

   GlobalUnlock(hPictName);
   }

else if (hPictName && (nPictureType & SUPERBTN_PICT_HANDLE))
   Buffer = SS_SaveButtonPicture(lpSS, Buffer, BufferLen, AllocLen,
                                 hBuffer, Color, (HANDLE)hPictName,
                                 nPictureType, lpfPaletteSaved);

return (Buffer);
}

#endif


#ifndef SS_NOCT_CHECK

HPBYTE SS_SaveCheckBoxPict(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                           lpfPaletteSaved, hPictName, nPictureType, Color)

LPSPREADSHEET  lpSS;
HPBYTE         Buffer;
LPLONG         BufferLen;
LPLONG         AllocLen;
LPHANDLE       hBuffer;
LPBOOL         lpfPaletteSaved;
TBGLOBALHANDLE hPictName;
short          nPictureType;
COLORREF       Color;
{
LPTSTR         lpText;

if (hPictName && !(nPictureType & SUPERBTN_PICT_HANDLE))
   {
   lpText = (LPTSTR)tbGlobalLock(hPictName);

   if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpText,
                               (lstrlen(lpText) + 1) * sizeof(TCHAR),
                               AllocLen, hBuffer)))
      return (NULL);

   tbGlobalUnlock(hPictName);
   }

else if (hPictName && (nPictureType & SUPERBTN_PICT_HANDLE))
   Buffer = SS_SaveButtonPicture(lpSS, Buffer, BufferLen, AllocLen,
                                 hBuffer, Color, (HANDLE)hPictName,
                                 nPictureType, lpfPaletteSaved);

return (Buffer);
}

#endif

#endif // SS_OLDSAVE


BOOL SS_LoadDataFromBuffer(lpSS, lpBuffer, lBufferLen)

LPSPREADSHEET lpSS;
HPBYTE        lpBuffer;
long          lBufferLen;
{
short         dTextAllocLen = 0;
LPTSTR        lpTextBuffer = NULL;
GLOBALHANDLE  hTextBuffer = 0;
HPBYTE        Ptr;
short         Version;
SS_COORD16    RowCnt16;
SS_COORD16    ColCnt16;
SS_COORD      RowCnt;
SS_COORD      ColCnt;
SS_COORD      x;
SS_COORD      y;

Ptr = lpBuffer;
Ptr += sizeof(BYTE);                   // File Type

Version = *Ptr;

Ptr += sizeof(BYTE);                   // Version
Ptr += sizeof(long);                   // Buffer Len

if (Version >= SS_SAVEVERSION2)
   {
   MemHugeCpy(&RowCnt, Ptr, sizeof(SS_COORD));
   Ptr += sizeof(SS_COORD);

   MemHugeCpy(&ColCnt, Ptr, sizeof(SS_COORD));
   Ptr += sizeof(SS_COORD);
   }
else
   {
   MemHugeCpy(&RowCnt16, Ptr, sizeof(SS_COORD16));
   Ptr += sizeof(SS_COORD16);

   MemHugeCpy(&ColCnt16, Ptr, sizeof(SS_COORD16));
   Ptr += sizeof(SS_COORD16);

   ColCnt = ColCnt16;
   RowCnt = RowCnt16;
   }

for (y = 0; y < RowCnt; y++)
   for (x = 0; x < ColCnt && Ptr < lpBuffer + lBufferLen; x++)
      {
      if (*Ptr)
         lpTextBuffer = SS_LoadData(lpSS, x, y, (HPTSTR)Ptr, lpTextBuffer,
                                    &hTextBuffer, &dTextAllocLen);

      Ptr += (StrHugeLen((HPTSTR)Ptr) + 1) * sizeof(TCHAR);
      }

if (hTextBuffer)
   GlobalFree(hTextBuffer);

return (TRUE);
}


BOOL SS_LoadTabDataFromBuffer(lpSS, lpBuffer, lBufferLen)

LPSPREADSHEET lpSS;
HPBYTE        lpBuffer;
long          lBufferLen;
{
SS_COORD      ColCnt;
SS_COORD      RowCnt;
BOOL          fRet;
BOOL          fRedraw = SS_BookSetRedraw(lpSS->lpBook, FALSE);

SS_ClearDataRange(lpSS, SS_ALLCOLS, SS_ALLROWS, SS_ALLCOLS, SS_ALLROWS,
                  FALSE, FALSE);
lpSS->Col.Max = 999999;
lpSS->Row.Max = 999999;
fRet = SS_ClipIn(lpSS, 1, 1, SS_GetColCnt(lpSS) - 1, SS_GetRowCnt(lpSS) - 1, (LPTSTR)lpBuffer,
                 lBufferLen, FALSE, FALSE, FALSE, &ColCnt, &RowCnt);
//                 lBufferLen, TRUE, FALSE, FALSE, &ColCnt, &RowCnt);

// 97' 2/5 Modified by BOC Gao. for BUG 000739
// We should use SS_SetXXX instead of set it directly, shouldn't we?
#ifdef SPREAD_JPN
SS_SetMaxCols(lpSS, ColCnt);
SS_SetMaxRows(lpSS, RowCnt);
#else
lpSS->Col.Max = ColCnt;
if (lpSS->Col.CurAt == 0 && lpSS->Col.Max > 0)
   lpSS->Col.CurAt = lpSS->Col.HeaderCnt;
if (lpSS->Col.UL == 0 && lpSS->Col.Max > 0)
   lpSS->Col.UL = lpSS->Col.HeaderCnt;
lpSS->Row.Max = RowCnt;
if (lpSS->Row.CurAt == 0 && lpSS->Row.Max > 0)
   lpSS->Row.CurAt = lpSS->Row.HeaderCnt;
if (lpSS->Row.UL == 0 && lpSS->Row.Max > 0)
   lpSS->Row.UL = lpSS->Row.HeaderCnt;
#endif

SS_BookSetRedraw(lpSS->lpBook, fRedraw);

return (fRet);
}


BOOL SS_LoadAllFromBuffer(lpSS, lpBuffer, lBufferLen)

LPSPREADSHEET lpSS;
HPBYTE        lpBuffer;
long          lBufferLen;
{
HPBYTE        Ptr;

#ifndef SS_NOSAVEV1
LPSS_BOOK         lpBook = lpSS->lpBook;
SS_SAVESPREAD     SaveSpread;
GLOBALHANDLE      hSaveFontTable = 0;
GLOBALHANDLE      hSaveColorTable = 0;
SS_SAVEROW        SaveRow;
SS_SAVEROW        SaveRowNull;
SS_SAVEROW        SaveRowNull2;
SS_SAVECOL        SaveCol;
SS_SAVECOL        SaveColNull;
SS_SAVECOL        SaveColNull2;
SS_SAVECELL       SaveCell;
SS_SAVECELL       SaveCellNull;
SS_SAVECELL       SaveCellNull2;
LPCOLORREF        lpSaveColorTable = 0;
COLORREF          Color;
SS_SAVELOGFONT    SaveLogFont;
LOGFONT           LogFont;
LPSS_SAVEFONTTBL  lpSaveFontTable = 0;
HPTSTR            lpText;
LPSS_ROW          lpRow;
LPSS_COL          lpCol;
LPSS_CELL         lpCell;
HPALETTE          hPalDefault = 0;
short             dTextAllocLen = 0;
LPTSTR            lpTextBuffer = NULL;
GLOBALHANDLE      hTextBuffer = 0;
short             dPaletteSize = 0;
short             dPointSize;
GLOBALHANDLE      hGlobalPalette = 0;
SS_COORD          i;
SS_COORD          j;

#endif

Ptr = lpBuffer;
Ptr += sizeof(BYTE);                   // File Type

if (*Ptr == SS_SAVEVERSION2)
   return (SS_LoadAllFromBuffer2(lpSS, lpBuffer, lBufferLen));

if (*Ptr == SS_SAVEVERSION3)
   return (SS_LoadAllFromBuffer3(lpSS, lpBuffer, lBufferLen));

#ifndef SS_NOSAVEV1

Ptr += sizeof(BYTE);                   // Version
Ptr += sizeof(long);                   // Buffer Len

_fmemset(&SaveColNull, '\0', sizeof(SS_SAVECOL));
_fmemset(&SaveColNull2, '\0', sizeof(SS_SAVECOL));

SaveColNull2.FontId = SS_FONT_NONE;
SaveColNull2.ColWidth = SS_WIDTH_DEFAULT;

_fmemset(&SaveRowNull, '\0', sizeof(SS_SAVEROW));
_fmemset(&SaveRowNull2, '\0', sizeof(SS_SAVEROW));

SaveRowNull2.FontId = SS_FONT_NONE;
SaveRowNull2.RowMaxFontId = SS_FONT_NONE;
SaveRowNull2.RowHeight = SS_HEIGHT_DEFAULT;

_fmemset(&SaveCellNull, '\0', sizeof(SS_SAVECELL));
_fmemset(&SaveCellNull2, '\0', sizeof(SS_SAVECELL));

SaveCellNull2.FontId = SS_FONT_NONE;

SS_Reset(lpBook);
lpSS = SS_BookLockSheet(lpBook);

if (!(lpSS->lpBook->hBuffer = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 330 * sizeof(TCHAR))))
   ;

SS_BookSetRedraw(lpSS->lpBook, FALSE);
SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOCALC, FALSE);

MemHugeCpy(&SaveSpread, Ptr, sizeof(SS_SAVESPREAD));
Ptr += sizeof(SS_SAVESPREAD);

lpSS->lpBook->DocumentProtected = SaveSpread.DocumentProtected;
lpSS->DocumentLocked = SaveSpread.DocumentLocked;

if (SaveSpread.AllowResize)
   lpSS->wUserResize = SS_USERRESIZE_COL | SS_USERRESIZE_ROW;
else
   lpSS->wUserResize = 0;

if (SaveSpread.AllowSelectBlock)
   lpSS->lpBook->wSelBlockOption = SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
                               SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL;
else
   lpSS->lpBook->wSelBlockOption = 0;

lpSS->RestrictRows = SaveSpread.RestrictRows;
lpSS->RestrictCols = SaveSpread.RestrictCols;
lpSS->lpBook->CalcAuto = SaveSpread.CalcAuto;

if (SaveSpread.GridLines)
   lpSS->wGridType |= SS_GRID_HORIZONTAL | SS_GRID_VERTICAL;
else
   lpSS->wGridType &= ~(SS_GRID_HORIZONTAL | SS_GRID_VERTICAL);

lpSS->lpBook->HorzScrollBar = SaveSpread.HorzScrollBar;
lpSS->lpBook->VertScrollBar = SaveSpread.VertScrollBar;

lpSS->Color.BackgroundId = SaveSpread.Color.BackgroundId;
lpSS->Color.ForegroundId = SaveSpread.Color.ForegroundId;
lpSS->Color.ShadowColorId = SaveSpread.Color.ShadowColorId;
lpSS->Color.ShadowTextId = SaveSpread.Color.ShadowTextId;
lpSS->Color.ShadowDarkId = SaveSpread.Color.ShadowDarkId;
lpSS->Color.ShadowLightId = SaveSpread.Color.ShadowLightId;
lpSS->lpBook->GrayAreaBackground = SaveSpread.Color.GrayAreaBackground;
lpSS->lpBook->GrayAreaForeground = SaveSpread.Color.GrayAreaForeground;

lpSS->DefaultFontId = SaveSpread.DefaultFontId;
_fmemcpy(&lpSS->DefaultCellType, &SaveSpread.DefaultCellType,
         sizeof(SS_CELLTYPE1));
SS_LoadCellType(lpSS, (HPBYTE)&lpSS->DefaultCellType,
                sizeof(SS_CELLTYPE), SS_ALLCOLS, SS_ALLROWS, &hPalDefault,
                &dPaletteSize, &hGlobalPalette, sizeof(SS_CELLTYPE1));
lpSS->Row.HeaderCnt = SaveSpread.HeaderRows;
lpSS->Col.HeaderCnt = SaveSpread.HeaderCols;
lpSS->Row.Frozen = SaveSpread.RowsFrozen;
lpSS->Col.Frozen = SaveSpread.ColsFrozen;
lpSS->Row.Max = SaveSpread.RowsMax;
lpSS->Col.Max = SaveSpread.ColsMax;
lpSS->Row.DataCnt = SaveSpread.DataRowCnt;
lpSS->Col.DataCnt = SaveSpread.DataColCnt;

if (SaveSpread.ColWidth == -1)
   {
   lpSS->Col.dCellSizeX100 = -1;
   lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, -1);
   }
else
   {
   lpSS->Col.dCellSizeX100 = (long)(SaveSpread.ColWidth * 100.0);
   lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, SaveSpread.ColWidth);
   }

if (SaveSpread.RowHeight == -1)
   {
   lpSS->Row.dCellSizeX100 = -1;
   lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, -1, -1);
   }
else
   {
   lpSS->Row.dCellSizeX100 = (long)(SaveSpread.RowHeight * 100.0);
   lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, -1,
                                                      SaveSpread.RowHeight);
   }

lpSS->lpBook->DefaultDateFormat.bCentury = SaveSpread.DefaultDateFormat.bCentury;
lpSS->lpBook->DefaultDateFormat.cSeparator = SaveSpread.DefaultDateFormat.cSeparator;
lpSS->lpBook->DefaultDateFormat.nFormat = SaveSpread.DefaultDateFormat.nFormat;
lpSS->lpBook->DefaultDateFormat.bSpin = SaveSpread.DefaultDateFormat.cSeparator;
lpSS->lpBook->DefaultTimeFormat.b24Hour = SaveSpread.DefaultTimeFormat.b24Hour;
lpSS->lpBook->DefaultTimeFormat.bSeconds = SaveSpread.DefaultTimeFormat.bSeconds;
lpSS->lpBook->DefaultTimeFormat.cSeparator = SaveSpread.DefaultTimeFormat.cSeparator;
lpSS->lpBook->DefaultTimeFormat.bSpin = SaveSpread.DefaultTimeFormat.bSpin;

lpSS->BlockCellUL.Col = -1;
lpSS->BlockCellUL.Row = -1;

lpSS->Row.UL = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
lpSS->Col.UL = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;

lpSS->Row.CurAt = lpSS->Row.UL;
lpSS->Col.CurAt = lpSS->Col.UL;

if (SaveSpread.fDataProvided)
   {
   lpText = (HPTSTR)Ptr;
   Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

   lpTextBuffer = SS_LoadData(lpSS, SS_ALLCOLS, SS_ALLROWS, lpText,
                              lpTextBuffer, &hTextBuffer, &dTextAllocLen);
   }

/***************************
* Read FontTable structure
***************************/

if (!(hSaveFontTable = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   SaveSpread.FontTableCnt *
                                   sizeof(SS_SAVEFONTTBL))))
   return (FALSE);

lpSaveFontTable = (LPSS_SAVEFONTTBL)GlobalLock(hSaveFontTable);

for (i = 0; i < SaveSpread.FontTableCnt; i++)
   {
   if (SaveSpread.fUseFontPointSize)
      {
      MemHugeCpy(&dPointSize, Ptr, sizeof(dPointSize));
      Ptr += sizeof(dPointSize);
      }

   MemHugeCpy(&SaveLogFont, Ptr, sizeof(SS_SAVELOGFONT));
   Ptr += sizeof(SS_SAVELOGFONT);
   _fmemset(&LogFont, 0, sizeof(LOGFONT));
   LogFont.lfHeight = SaveLogFont.lfHeight;
   LogFont.lfWidth = SaveLogFont.lfWidth;
   LogFont.lfEscapement = SaveLogFont.lfEscapement;
   LogFont.lfOrientation = SaveLogFont.lfOrientation;
   LogFont.lfWeight = SaveLogFont.lfWeight;
   LogFont.lfItalic = SaveLogFont.lfItalic;
   LogFont.lfUnderline = SaveLogFont.lfUnderline;
   LogFont.lfStrikeOut = SaveLogFont.lfStrikeOut;
   LogFont.lfCharSet = SaveLogFont.lfCharSet;
   LogFont.lfOutPrecision = SaveLogFont.lfOutPrecision;
   LogFont.lfClipPrecision = SaveLogFont.lfClipPrecision;
   LogFont.lfQuality = SaveLogFont.lfQuality;
   LogFont.lfPitchAndFamily = SaveLogFont.lfPitchAndFamily;
   _fmemcpy(LogFont.lfFaceName, SaveLogFont.lfFaceName,
            sizeof(LogFont.lfFaceName));  //???? need to translate for Unicode DLL/OCX

   if (SaveSpread.fUseFontPointSize)
      {
      LogFont.lfHeight = INTPT_TO_PIXELS(dPointSize);
      LogFont.lfWidth = 0;
      }

   lpSaveFontTable[i].hFont = CreateFontIndirect(&LogFont);
   }

if (SaveSpread.DefaultFontId == -1)
   lpSS->DefaultFontId = -1;
else
   lpSS->DefaultFontId = SS_SaveInitFont(lpSS,
                             &lpSaveFontTable[SaveSpread.DefaultFontId - 1]);

/****************************
* Read ColorTable structure
****************************/

if (SaveSpread.ColorTableCnt > 0)
   {
   if (!(hSaveColorTable = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                       SaveSpread.ColorTableCnt *
                                       sizeof(COLORREF))))
      return (FALSE);

   lpSaveColorTable = (LPCOLORREF)GlobalLock(hSaveColorTable);

   for (i = 0; i < SaveSpread.ColorTableCnt; i++)
      {
      MemHugeCpy(&Color, Ptr, sizeof(COLORREF));
      lpSaveColorTable[i] = Color;

      Ptr += sizeof(COLORREF);
      }

   if (SaveSpread.Color.ForegroundId == 0)
      lpSS->Color.ForegroundId = 0;
   else
      lpSS->Color.ForegroundId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ForegroundId - 1]);

   if (SaveSpread.Color.BackgroundId == 0)
      lpSS->Color.BackgroundId = 0;
   else
      lpSS->Color.BackgroundId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.BackgroundId - 1]);

   if (SaveSpread.Color.ShadowColorId == 0)
      lpSS->Color.ShadowColorId = 0;
   else
      lpSS->Color.ShadowColorId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowColorId - 1]);

   if (SaveSpread.Color.ShadowTextId == 0)
      lpSS->Color.ShadowTextId = 0;
   else
      lpSS->Color.ShadowTextId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowTextId - 1]);

   if (SaveSpread.Color.ShadowDarkId == 0)
      lpSS->Color.ShadowDarkId = 0;
   else
      lpSS->Color.ShadowDarkId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowDarkId - 1]);

   if (SaveSpread.Color.ShadowLightId == 0)
      lpSS->Color.ShadowLightId = 0;
   else
      lpSS->Color.ShadowLightId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowLightId - 1]);
   }

/*******************
* Read each column
*******************/

for (i = 0; i < SaveSpread.ColCnt; i++)
   {
   MemHugeCpy(&SaveCol, Ptr, sizeof(SS_SAVECOL));
   Ptr += sizeof(SS_SAVECOL);

   if (_fmemcmp(&SaveCol, &SaveColNull, sizeof(SS_SAVECOL)) &&
       _fmemcmp(&SaveCol, &SaveColNull2, sizeof(SS_SAVECOL)))
      {
      if (lpCol = SS_AllocLockCol(lpSS, i))
         {
         lpCol->ColLocked = (BYTE)SaveCol.ColLocked;

         if (SaveCol.ColWidth == -1)
            {
            lpCol->dColWidthX100 = -1;
            lpCol->dColWidthInPixels = -1;
            }
         else
            {
            lpCol->dColWidthX100 = (long)(SaveCol.ColWidth * 100.0);
            lpCol->dColWidthInPixels = SS_ColWidthToPixels(lpSS,
                                                           SaveCol.ColWidth);
            }

         lpCol->RowLastNonEmpty = SaveCol.RowLastNonEmpty;
         if (SaveCol.FontId == -1)
            lpCol->FontId = -1;
         else
            lpCol->FontId = SS_SaveInitFont(lpSS,
                                        &lpSaveFontTable[SaveCol.FontId - 1]);

         SS_SaveAddColor(lpSS, lpSaveColorTable, &lpCol->Color,
                         &SaveCol.Color);

         if (SaveCol.fCellTypeProvided)
            Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen, i,
                                  SS_ALLROWS, &hPalDefault, &dPaletteSize,
                                  &hGlobalPalette, sizeof(SS_CELLTYPE1));

         if (SaveCol.fFormulaProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen((HPTSTR)Ptr) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
            SS_SetFormulaRange(lpSS, i, SS_ALLROWS, i, SS_ALLROWS,
                               lpText, FALSE);
#endif
            }

         if (SaveCol.fDataProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
            lpTextBuffer = SS_LoadData(lpSS, i, SS_ALLROWS, lpText,
                                       lpTextBuffer, &hTextBuffer,
                                       &dTextAllocLen);
            }

         SS_UnlockColItem(lpSS, i);
         }
      }
   }

/****************
* Read each row
****************/

for (i = 0; i < SaveSpread.RowCnt; i++)
   {
   lpRow = NULL;

   MemHugeCpy(&SaveRow, Ptr, sizeof(SS_SAVEROW));
   Ptr += sizeof(SS_SAVEROW);

   if (_fmemcmp(&SaveRow, &SaveRowNull, sizeof(SS_SAVEROW)) &&
       _fmemcmp(&SaveRow, &SaveRowNull2, sizeof(SS_SAVEROW)))
      {
      if (lpRow = SS_AllocLockRow(lpSS, i))
         {
         lpRow->RowLocked = (BYTE)SaveRow.RowLocked;

         if (SaveRow.RowHeight == -1)
            {
            lpRow->dRowHeightX100 = -1;
            lpRow->dRowHeightInPixels = -1;
            }
         else
            {
            lpRow->dRowHeightX100 = (long)(SaveRow.RowHeight * 100.0);
            lpRow->dRowHeightInPixels = SS_CalcRowHeight(lpSS, i);
            }

         if (SaveRow.FontId == -1)
            lpRow->FontId = -1;
         else
            lpRow->FontId = SS_SaveInitFont(lpSS,
                                        &lpSaveFontTable[SaveRow.FontId - 1]);

         SS_SaveAddColor(lpSS, lpSaveColorTable, &lpRow->Color,
                         &SaveRow.Color);

         if (SaveRow.fCellTypeProvided)
            Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen,
                                  SS_ALLCOLS, i, &hPalDefault, &dPaletteSize,
                                  &hGlobalPalette, sizeof(SS_CELLTYPE1));

         if (SaveRow.fFormulaProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
            SS_SetFormulaRange(lpSS, SS_ALLCOLS, i, SS_ALLCOLS, i,
                               lpText, FALSE);
#endif
            }

         if (SaveRow.fDataProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

            lpTextBuffer = SS_LoadData(lpSS, SS_ALLCOLS, i, lpText,
                                       lpTextBuffer, &hTextBuffer,
                                       &dTextAllocLen);
            }
         }
      }

   /*****************
   * Read each Cell
   *****************/

   for (j = 0; j < SaveSpread.ColCnt; j++)
      {
      MemHugeCpy(&SaveCell, Ptr, sizeof(SS_SAVECELL));

      Ptr += sizeof(SS_SAVECELL);

      if (_fmemcmp(&SaveCell, &SaveCellNull, sizeof(SS_SAVECELL)) &&
          _fmemcmp(&SaveCell, &SaveCellNull2, sizeof(SS_SAVECELL)))
         {
         if (lpCell = SS_AllocLockCell(lpSS, lpRow, j, i))
            {
            lpCell->CellLocked = (BYTE)SaveCell.CellLocked;

            if (SaveCell.FontId == -1)
               lpCell->FontId = -1;
            else
               lpCell->FontId = SS_SaveInitFont(lpSS,
                                &lpSaveFontTable[SaveCell.FontId - 1]);

            SS_SaveAddColor(lpSS, lpSaveColorTable, &lpCell->Color,
                            &SaveCell.Color);

            if (SaveCell.fCellTypeProvided)
               Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen,
                                     j, i, &hPalDefault, &dPaletteSize,
                                     &hGlobalPalette, sizeof(SS_CELLTYPE1));

            if (SaveCell.fFormulaProvided)
               {
               lpText = (HPTSTR)Ptr;
               Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
               SS_SetFormulaRange(lpSS, j, i, j, i, lpText, FALSE);
#endif
               }

            if (SaveCell.fDataProvided)
               {
               lpText = (HPTSTR)Ptr;
               Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

               lpTextBuffer = SS_LoadData(lpSS, j, i, lpText,
                                          lpTextBuffer, &hTextBuffer,
                                          &dTextAllocLen);
               }

            SSx_UnlockCellItem(lpSS, lpRow, j, i);
            }
         }
      }

   SS_UnlockRowItem(lpSS, i);   // This should be here
   }

if (lpSS->lpBook->HorzScrollBar)
   SS_SetHScrollBar(lpSS);
else if (lpSS->lpBook->hWnd)
   SetScrollRange(lpSS->lpBook->hWnd, SB_HORZ, 0, 0, TRUE);

if (lpSS->lpBook->VertScrollBar)
   SS_SetVScrollBar(lpSS);
else if (lpSS->lpBook->hWnd)
   SetScrollRange(lpSS->lpBook->hWnd, SB_VERT, 0, 0, TRUE);

SS_BuildDependencies(lpSS);
SS_BookSetRedraw(lpSS->lpBook, TRUE);
//lpSS->Redraw = SaveSpread.Redraw;

for (i = 0; i < SaveSpread.FontTableCnt; i++)
   if (!lpSaveFontTable[i].fNoDelete)
      DeleteObject(lpSaveFontTable[i].hFont);

GlobalUnlock(hSaveFontTable);
GlobalFree(hSaveFontTable);

if (hSaveColorTable)
   {
   GlobalUnlock(hSaveColorTable);
   GlobalFree(hSaveColorTable);
   }

if (hTextBuffer)
   GlobalFree(hTextBuffer);

if (hGlobalPalette)
   GlobalFree(hGlobalPalette);

if (hPalDefault)
   DeleteObject(hPalDefault);

return (TRUE);
#endif
}


BOOL SS_LoadAllFromBuffer2(lpSS, lpBuffer, lBufferLen)

LPSPREADSHEET     lpSS;
HPBYTE            lpBuffer;
long              lBufferLen;
{
LPSS_BOOK         lpBook = lpSS->lpBook;
HPBYTE            Ptr;
SS_SAVESPREAD2    SaveSpread;
GLOBALHANDLE      hSaveFontTable = 0;
GLOBALHANDLE      hSaveColorTable = 0;
SS_SAVEROW2       SaveRow;
SS_SAVECOL2       SaveCol;
SS_SAVECELL2      SaveCell;
LPCOLORREF        lpSaveColorTable = 0;
COLORREF          Color;
SS_SAVELOGFONT    SaveLogFont;
LOGFONT           LogFont;
LPSS_SAVEFONTTBL  lpSaveFontTable = 0;
HPTSTR            lpText;
LPSS_ROW          lpRow;
LPSS_COL          lpCol;
LPSS_CELL         lpCell;
HPALETTE          hPalDefault = 0;
short             dTextAllocLen = 0;
LPTSTR            lpTextBuffer = NULL;
GLOBALHANDLE      hTextBuffer = 0;
short             dPaletteSize = 0;
short             dPointSize;
GLOBALHANDLE      hGlobalPalette = 0;
GLOBALHANDLE      hBuffer;
SS_COORD          i;
SS_COORD          j;
BYTE              bSaved;

Ptr = lpBuffer;
Ptr += sizeof(BYTE);                   // File Type
Ptr += sizeof(BYTE);                   // Version
Ptr += sizeof(long);                   // Buffer Len

SS_Reset(lpBook);
lpSS = SS_BookLockSheet(lpBook);

#if (defined(SS_QE) && defined(SS_VB))
vbSSClearDB(VBGetHwndControl(lpSS->lpBook->hWnd), lpSS->lpBook->hWnd);
#endif

if (!(lpSS->lpBook->hBuffer = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 330 * sizeof(TCHAR))))
   ;

#ifndef SS_NOCALC
SS_SetTypeEdit(&lpSS->lpBook->FormulaCellType, ES_LEFT | ES_AUTOHSCROLL,
               SS_CALC_FORMULA_MAXLEN, SS_CHRSET_CHR, SS_CASE_NOCASE);

SS_CreateControl(lpSS, &lpSS->lpBook->FormulaCellType, FALSE);
#endif

SS_BookSetRedraw(lpSS->lpBook, FALSE);
SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOCALC, FALSE);

MemHugeCpy(&SaveSpread, Ptr, sizeof(SS_SAVESPREAD2));
Ptr += sizeof(SS_SAVESPREAD2);

lpSS->lpBook->DocumentProtected = SaveSpread.DocumentProtected;
lpSS->DocumentLocked = SaveSpread.DocumentLocked;

if (SaveSpread.AllowResize)
   lpSS->wUserResize = SS_USERRESIZE_COL | SS_USERRESIZE_ROW;
else
   lpSS->wUserResize = 0;

if (SaveSpread.AllowSelectBlock)
   lpSS->lpBook->wSelBlockOption = SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
                           SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL;
else
   lpSS->lpBook->wSelBlockOption = 0;

lpSS->RestrictRows = SaveSpread.RestrictRows;
lpSS->RestrictCols = SaveSpread.RestrictCols;
lpSS->lpBook->CalcAuto = SaveSpread.CalcAuto;

if (SaveSpread.GridLines)
   lpSS->wGridType |= SS_GRID_HORIZONTAL | SS_GRID_VERTICAL;
else
   lpSS->wGridType &= ~(SS_GRID_HORIZONTAL | SS_GRID_VERTICAL);

lpSS->lpBook->HorzScrollBar = SaveSpread.HorzScrollBar;
lpSS->lpBook->VertScrollBar = SaveSpread.VertScrollBar;

lpSS->Color.BackgroundId = SaveSpread.Color.BackgroundId;
lpSS->Color.ForegroundId = SaveSpread.Color.ForegroundId;
lpSS->Color.ShadowColorId = SaveSpread.Color.ShadowColorId;
lpSS->Color.ShadowTextId = SaveSpread.Color.ShadowTextId;
lpSS->Color.ShadowDarkId = SaveSpread.Color.ShadowDarkId;
lpSS->Color.ShadowLightId = SaveSpread.Color.ShadowLightId;
lpSS->lpBook->GrayAreaBackground = SaveSpread.Color.GrayAreaBackground;
lpSS->lpBook->GrayAreaForeground = SaveSpread.Color.GrayAreaForeground;

lpSS->DefaultFontId = SaveSpread.DefaultFontId;
_fmemcpy(&lpSS->DefaultCellType, &SaveSpread.DefaultCellType,
         sizeof(SS_CELLTYPE));
SS_LoadCellType(lpSS, (HPBYTE)&lpSS->DefaultCellType,
                sizeof(SS_CELLTYPE), SS_ALLCOLS, SS_ALLROWS, &hPalDefault,
                &dPaletteSize, &hGlobalPalette, sizeof(SS_SAVECELLTYPE3));
lpSS->Row.HeaderCnt = SaveSpread.HeaderRows;
lpSS->Col.HeaderCnt = SaveSpread.HeaderCols;
lpSS->Row.Frozen = SaveSpread.RowsFrozen;
lpSS->Col.Frozen = SaveSpread.ColsFrozen;
lpSS->Row.Max = SaveSpread.RowsMax;
lpSS->Col.Max = SaveSpread.ColsMax;
lpSS->Row.DataCnt = SaveSpread.DataRowCnt;
lpSS->Col.DataCnt = SaveSpread.DataColCnt;

lpSS->lpBook->DefaultDateFormat.bCentury = SaveSpread.DefaultDateFormat.bCentury;
lpSS->lpBook->DefaultDateFormat.cSeparator = SaveSpread.DefaultDateFormat.cSeparator;
lpSS->lpBook->DefaultDateFormat.nFormat = SaveSpread.DefaultDateFormat.nFormat;
lpSS->lpBook->DefaultDateFormat.bSpin = SaveSpread.DefaultDateFormat.cSeparator;
lpSS->lpBook->DefaultTimeFormat.b24Hour = SaveSpread.DefaultTimeFormat.b24Hour;
lpSS->lpBook->DefaultTimeFormat.bSeconds = SaveSpread.DefaultTimeFormat.bSeconds;
lpSS->lpBook->DefaultTimeFormat.cSeparator = SaveSpread.DefaultTimeFormat.cSeparator;
lpSS->lpBook->DefaultTimeFormat.bSpin = SaveSpread.DefaultTimeFormat.bSpin;

lpSS->lpBook->AllowUserFormulas = SaveSpread.AllowUserFormulas;
lpSS->lpBook->fEditModePermanent = SaveSpread.fEditModePermanent;
lpSS->lpBook->fAutoSize = SaveSpread.fAutoSize;
lpSS->lpBook->fScrollBarExtMode = SaveSpread.fScrollBarExtMode;
lpSS->lpBook->fProcessTab = SaveSpread.fProcessTab;
lpSS->lpBook->fScrollBarShowMax = SaveSpread.fScrollBarShowMax;
lpSS->lpBook->fScrollBarMaxAlign = SaveSpread.fScrollBarMaxAlign;
lpSS->lpBook->fArrowsExitEditMode = SaveSpread.fArrowsExitEditMode;
lpSS->lpBook->fMoveActiveOnFocus = SaveSpread.fMoveActiveOnFocus;
lpSS->lpBook->fRetainSelBlock = SaveSpread.fRetainSelBlock;
lpSS->lpBook->fEditModeReplace = SaveSpread.fEditModeReplace;
lpSS->lpBook->fNoBorder = SaveSpread.fNoBorder;
lpSS->lpBook->fNoBeep = SaveSpread.fNoBeep;
lpSS->lpBook->fAutoClipboard = SaveSpread.fAutoClipboard;
lpSS->lpBook->fAllowMultipleSelBlocks = SaveSpread.fAllowMultipleSelBlocks;
lpSS->lpBook->fAllowCellOverflow = SaveSpread.fAllowCellOverflow;
lpSS->fVirtualMode = SaveSpread.fVirtualMode;
lpSS->lpBook->fAllowDragDrop = SaveSpread.fAllowDragDrop;
lpSS->fSetBorder = SaveSpread.fSetBorder;
lpSS->lpBook->wButtonDrawMode = SaveSpread.wButtonDrawMode;
lpSS->lpBook->wEnterAction = SaveSpread.wEnterAction;
lpSS->wOpMode = SaveSpread.wOpMode;
lpSS->wGridType = SaveSpread.wGridType;
lpSS->wUserResize = SaveSpread.wUserResize;
lpSS->lpBook->wSelBlockOption = SaveSpread.wSelBlockOption;
lpSS->GridColor = SaveSpread.GridColor;
lpSS->lpBook->dUnitType = SaveSpread.dUnitType;
lpSS->Col.NumStart = SaveSpread.ColNumStart;
lpSS->Row.NumStart = SaveSpread.RowNumStart;
lpSS->Col.AutoSizeVisibleCnt = SaveSpread.AutoSizeVisibleCols;
lpSS->Row.AutoSizeVisibleCnt = SaveSpread.AutoSizeVisibleRows;
lpSS->ColHeaderDisplay = SaveSpread.ColHeaderDisplay;
lpSS->RowHeaderDisplay = SaveSpread.RowHeaderDisplay;
_fmemcpy(&lpSS->Virtual, &SaveSpread.Virtual, sizeof(SS_VIRTUAL));
lpSS->LockColor.BackgroundId = SaveSpread.LockColor.BackgroundId;
lpSS->LockColor.ForegroundId = SaveSpread.LockColor.ForegroundId;
_fmemcpy(&lpSS->lpBook->DefaultFloatFormat, &SaveSpread.DefaultFloatFormat,
         sizeof(FLOATFORMAT));

lpSS->DataFieldNameCnt = 0;

#ifndef SS_NOSCBAR
SS_VScrollSetSpecial(lpSS->lpBook, SaveSpread.fUseSpecialVScroll,
                     SaveSpread.wSpecialVScrollOptions);
#endif

lpSS->BlockCellUL.Col = -1;
lpSS->BlockCellUL.Row = -1;

lpSS->Row.UL = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
lpSS->Col.UL = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;

lpSS->Row.CurAt = lpSS->Row.UL;
lpSS->Col.CurAt = lpSS->Col.UL;

if (SaveSpread.bMiscFlags & SS_SAVEMISC_XTRA)
   {
   _fmemcpy(&lpSS->lpBook->lXtraLen, Ptr, 4);
   Ptr += 4;

   if (lpSS->lpBook->lXtraLen)
      {
      if (hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                lpSS->lpBook->lXtraLen))
         {
         HPBYTE hpXtraBuff;
         hpXtraBuff = (HPBYTE)GlobalLock(hBuffer);
         MemHugeCpy(hpXtraBuff, Ptr, lpSS->lpBook->lXtraLen);
         GlobalUnlock(hBuffer);

         lpSS->lpBook->hXtra = hBuffer;
         }

      Ptr += lpSS->lpBook->lXtraLen;
      }
   }

#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))
if (SaveSpread.bMiscFlags & SS_SAVEMISC_DATAAWARE)
   if (!(Ptr = SS_LoadDataAware(lpSS->lpBook->hWnd, Ptr, lBufferLen)))
      return ((BOOL)NULL);                //DBOCX
#endif

if (SaveSpread.fDataProvided)
   {
   lpText = (HPTSTR)Ptr;
   Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

   lpTextBuffer = SS_LoadData(lpSS, SS_ALLCOLS, SS_ALLROWS, lpText,
                              lpTextBuffer, &hTextBuffer, &dTextAllocLen);
   }

/***************************
* Read FontTable structure
***************************/

if (!(hSaveFontTable = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   SaveSpread.FontTableCnt *
                                   sizeof(SS_SAVEFONTTBL))))
   return (FALSE);

lpSaveFontTable = (LPSS_SAVEFONTTBL)GlobalLock(hSaveFontTable);

for (i = 0; i < SaveSpread.FontTableCnt; i++)
   {
   if (SaveSpread.fUseFontPointSize)
      {
      MemHugeCpy(&dPointSize, Ptr, sizeof(dPointSize));
      Ptr += sizeof(dPointSize);
      }

   MemHugeCpy(&SaveLogFont, Ptr, sizeof(SS_SAVELOGFONT));
   Ptr += sizeof(SS_SAVELOGFONT);
   _fmemset(&LogFont, 0, sizeof(LOGFONT));
   LogFont.lfHeight = SaveLogFont.lfHeight;
   LogFont.lfWidth = SaveLogFont.lfWidth;
   LogFont.lfEscapement = SaveLogFont.lfEscapement;
   LogFont.lfOrientation = SaveLogFont.lfOrientation;
   LogFont.lfWeight = SaveLogFont.lfWeight;
   LogFont.lfItalic = SaveLogFont.lfItalic;
   LogFont.lfUnderline = SaveLogFont.lfUnderline;
   LogFont.lfStrikeOut = SaveLogFont.lfStrikeOut;
   LogFont.lfCharSet = SaveLogFont.lfCharSet;
   LogFont.lfOutPrecision = SaveLogFont.lfOutPrecision;
   LogFont.lfClipPrecision = SaveLogFont.lfClipPrecision;
   LogFont.lfQuality = SaveLogFont.lfQuality;
   LogFont.lfPitchAndFamily = SaveLogFont.lfPitchAndFamily;
   _fmemcpy(LogFont.lfFaceName, SaveLogFont.lfFaceName,
            sizeof(LogFont.lfFaceName));  //???? need to translate for Unicode DLL/OCX

   if (SaveSpread.fUseFontPointSize)
      {
      LogFont.lfHeight = INTPT_TO_PIXELS(dPointSize);
      LogFont.lfWidth = 0;
      }

   lpSaveFontTable[i].hFont = CreateFontIndirect(&LogFont);
   }

if (SaveSpread.DefaultFontId == -1)
   lpSS->DefaultFontId = -1;
else
   lpSS->DefaultFontId = SS_SaveInitFont(lpSS,
                             &lpSaveFontTable[SaveSpread.DefaultFontId - 1]);

if (SaveSpread.ColWidth == -1)
   {
   lpSS->Col.dCellSizeX100 = -1;
   lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, -1);
   }
else
   {
   lpSS->Col.dCellSizeX100 = (long)(SaveSpread.ColWidth * 100.0);
   lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, SaveSpread.ColWidth);
   }

if (SaveSpread.RowHeight == -1)
   {
   lpSS->Row.dCellSizeX100 = -1;
   lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, -1, -1);
   }
else
   {
   lpSS->Row.dCellSizeX100 = (long)(SaveSpread.RowHeight * 100.0);
   lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, -1,
                                                   SaveSpread.RowHeight);
   }

/****************************
* Read ColorTable structure
****************************/

if (SaveSpread.ColorTableCnt > 0)
   {
   if (!(hSaveColorTable = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                       SaveSpread.ColorTableCnt *
                                       sizeof(COLORREF))))
      return (FALSE);

   lpSaveColorTable = (LPCOLORREF)GlobalLock(hSaveColorTable);

   for (i = 0; i < SaveSpread.ColorTableCnt; i++)
      {
      MemHugeCpy(&Color, Ptr, sizeof(COLORREF));
      lpSaveColorTable[i] = Color;

      Ptr += sizeof(COLORREF);
      }

   if (SaveSpread.Color.ForegroundId == 0 ||
       SaveSpread.Color.ForegroundId >= SaveSpread.ColorTableCnt)
      lpSS->Color.ForegroundId  = SS_AddColor(RGBCOLOR_BLACK);
   else
      lpSS->Color.ForegroundId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ForegroundId - 1]);

   if (SaveSpread.Color.BackgroundId == 0 ||
       SaveSpread.Color.BackgroundId >= SaveSpread.ColorTableCnt)
      lpSS->Color.BackgroundId  = SS_AddColor(RGBCOLOR_WHITE);
   else
      lpSS->Color.BackgroundId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.BackgroundId - 1]);

   if (SaveSpread.Color.ShadowColorId == 0 ||
       SaveSpread.Color.ShadowColorId >= SaveSpread.ColorTableCnt)
      lpSS->Color.ShadowColorId = SS_AddColor(GetSysColor(COLOR_BTNFACE));
   else
      lpSS->Color.ShadowColorId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowColorId - 1]);

   if (SaveSpread.Color.ShadowTextId == 0 ||
       SaveSpread.Color.ShadowTextId >= SaveSpread.ColorTableCnt)
      lpSS->Color.ShadowTextId  = SS_AddColor(GetSysColor(COLOR_BTNTEXT));
   else
      lpSS->Color.ShadowTextId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowTextId - 1]);

   if (SaveSpread.Color.ShadowDarkId == 0 ||
       SaveSpread.Color.ShadowDarkId >= SaveSpread.ColorTableCnt)
      lpSS->Color.ShadowDarkId  = SS_AddColor(GetSysColor(COLOR_BTNSHADOW));
   else
      lpSS->Color.ShadowDarkId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowDarkId - 1]);

   if (SaveSpread.Color.ShadowLightId == 0 ||
       SaveSpread.Color.ShadowLightId >= SaveSpread.ColorTableCnt)
      lpSS->Color.ShadowLightId = SS_AddColor(GetSysColor(COLOR_BTNHIGHLIGHT));
   else
      lpSS->Color.ShadowLightId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowLightId - 1]);

   if (SaveSpread.LockColor.BackgroundId == 0 ||
       SaveSpread.LockColor.BackgroundId >= SaveSpread.ColorTableCnt)
      lpSS->LockColor.BackgroundId = 0;
   else
      lpSS->LockColor.BackgroundId = SS_AddColor(lpSaveColorTable[SaveSpread.LockColor.BackgroundId - 1]);

   if (SaveSpread.LockColor.ForegroundId == 0 ||
       SaveSpread.LockColor.ForegroundId >= SaveSpread.ColorTableCnt)
      lpSS->LockColor.ForegroundId = 0;
   else
      lpSS->LockColor.ForegroundId = SS_AddColor(lpSaveColorTable[SaveSpread.LockColor.ForegroundId - 1]);
   }

/*******************
* Read each column
*******************/

for (i = 0; i < SaveSpread.ColCnt; i++)
   {
   bSaved = *Ptr;
   Ptr++;

   if (bSaved)
      {
      MemHugeCpy(&SaveCol, Ptr, sizeof(SS_SAVECOL2));
      Ptr += sizeof(SS_SAVECOL2);

      if (lpCol = SS_AllocLockCol(lpSS, i))
         {
         lpCol->ColLocked = (BYTE)SaveCol.ColLocked;

         if (SaveCol.ColWidth == -1)
            {
            lpCol->dColWidthX100 = -1;
            lpCol->dColWidthInPixels = -1;
            }
         else
            {
            lpCol->dColWidthX100 = (long)(SaveCol.ColWidth * 100.0);
            lpCol->dColWidthInPixels = SS_ColWidthToPixels(lpSS,
                                                           SaveCol.ColWidth);
            }

         lpCol->RowLastNonEmpty = SaveCol.RowLastNonEmpty;
         if (SaveCol.FontId == -1)
            lpCol->FontId = -1;
         else
            lpCol->FontId = SS_SaveInitFont(lpSS,
                                        &lpSaveFontTable[SaveCol.FontId - 1]);

         SS_SaveAddColor(lpSS, lpSaveColorTable, &lpCol->Color,
                         &SaveCol.Color);

#ifndef SS_NOBORDERS
         SS_LoadBorder(lpSS, lpSaveColorTable, SaveSpread.ColorTableCnt,
                       &lpCol->hBorder, &SaveCol.Border);
#endif
         lpCol->fPageBreak = SaveCol.fPageBreak;
         lpCol->bUserResize = SaveCol.bUserResize;
#ifndef SS_NOUSERDATA
         lpCol->lUserData = SaveCol.lUserData;
#endif

         if (SaveCol.fCellTypeProvided)
            Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen, i,
                                  SS_ALLROWS, &hPalDefault, &dPaletteSize,
                                  &hGlobalPalette, sizeof(SS_SAVECELLTYPE3));

         if (SaveCol.fFormulaProvided)
            {
            lpText = (LPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
            if ('=' == *lpText)
               lpText++;
            #ifndef SS_OLDCALC
            SS_RegisterCustFuncs(lpSS, lpText);
            #endif
            SS_SetFormulaRange(lpSS, i, SS_ALLROWS, i, SS_ALLROWS,
                               lpText, FALSE);
#endif
            }

         if (SaveCol.fDataProvided)
            {
            lpText = (LPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
            lpTextBuffer = SS_LoadData(lpSS, i, SS_ALLROWS, lpText,
                                       lpTextBuffer, &hTextBuffer,
                                       &dTextAllocLen);
            }

         /***********************
         * Read Data Aware Info
         ***********************/

#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))
         if (SaveSpread.bMiscFlags & SS_SAVEMISC_DATAAWARE)
            {
            long lLen;

            Ptr += 4;                  // Unused

            lpCol->bDataFillEvent = *Ptr;
            if (lpCol->bDataFillEvent)
               lpCol->bDataFillEvent = SS_DATAFILLEVENT_YES;

            Ptr += 1;

            lLen = StrHugeLen((HPTSTR)Ptr);

            if (lLen)
               {
               if (lpCol->hDBFieldName = tbGlobalAlloc(GMEM_MOVEABLE |
                                                       GMEM_ZEROINIT,
                                                       (lLen + 1)
                                                       * sizeof(TCHAR)))
                  {
                  lpText = (LPTSTR)tbGlobalLock(lpCol->hDBFieldName);
                  MemHugeCpy(lpText, Ptr, (lLen + 1) * sizeof(TCHAR));
                  tbGlobalUnlock(lpCol->hDBFieldName);
                  lpSS->DataFieldNameCnt++;
                  }
               }

            Ptr += lLen + 1;
            }
#endif

         SS_UnlockColItem(lpSS, i);
         }
      }
   }

/****************
* Read each row
****************/

for (i = 0; i < SaveSpread.RowCnt; i++)
   {
   lpRow = NULL;

   bSaved = *Ptr;
   Ptr++;

   if (bSaved)
      {
      MemHugeCpy(&SaveRow, Ptr, sizeof(SS_SAVEROW2));
      Ptr += sizeof(SS_SAVEROW2);

      if (lpRow = SS_AllocLockRow(lpSS, i))
         {
         lpRow->RowLocked = (BYTE)SaveRow.RowLocked;

         if (SaveRow.RowHeight == -1)
            {
            lpRow->dRowHeightX100 = -1;
            lpRow->dRowHeightInPixels = -1;
            }
         else
            {
            lpRow->dRowHeightX100 = (long)(SaveRow.RowHeight * 100.0);
            lpRow->dRowHeightInPixels = SS_CalcRowHeight(lpSS, i);
            }

         if (SaveRow.FontId == -1)
            lpRow->FontId = -1;
         else
            lpRow->FontId = SS_SaveInitFont(lpSS,
                                        &lpSaveFontTable[SaveRow.FontId - 1]);

         SS_SaveAddColor(lpSS, lpSaveColorTable, &lpRow->Color,
                         &SaveRow.Color);

#ifndef SS_NOBORDERS
         SS_LoadBorder(lpSS, lpSaveColorTable, SaveSpread.ColorTableCnt,
                       &lpRow->hBorder, &SaveRow.Border);
#endif
         lpRow->fRowSelected = SaveRow.fRowSelected;
         lpRow->fPageBreak = SaveRow.fPageBreak;
         lpRow->bUserResize = SaveRow.bUserResize;
#ifndef SS_NOUSERDATA
         lpRow->lUserData = SaveRow.lUserData;
#endif

         if (SaveRow.fCellTypeProvided)
            Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen,
                                  SS_ALLCOLS, i, &hPalDefault, &dPaletteSize,
                                  &hGlobalPalette, sizeof(SS_SAVECELLTYPE3));

         if (SaveRow.fFormulaProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
            if ('=' == *lpText)
               lpText++;
            #ifndef SS_OLDCALC
            SS_RegisterCustFuncs(lpSS, lpText);
            #endif
            SS_SetFormulaRange(lpSS, SS_ALLCOLS, i, SS_ALLCOLS, i,
                               lpText, FALSE);
#endif
            }

         if (SaveRow.fDataProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

            lpTextBuffer = SS_LoadData(lpSS, SS_ALLCOLS, i, lpText,
                                       lpTextBuffer, &hTextBuffer,
                                       &dTextAllocLen);
            }
         }

      /*****************
      * Read each Cell
      *****************/

      for (j = 0; j < SaveSpread.ColCnt; j++)
         {
         bSaved = *Ptr;
         Ptr++;

         if (bSaved)
            {
            MemHugeCpy(&SaveCell, Ptr, sizeof(SS_SAVECELL2));
            Ptr += sizeof(SS_SAVECELL2);

            if (lpCell = SS_AllocLockCell(lpSS, lpRow, j, i))
               {
               lpCell->CellLocked = (BYTE)SaveCell.CellLocked;

               if (SaveCell.FontId == -1)
                  lpCell->FontId = -1;
               else
                  lpCell->FontId = SS_SaveInitFont(lpSS,
                                   &lpSaveFontTable[SaveCell.FontId - 1]);

               SS_SaveAddColor(lpSS, lpSaveColorTable, &lpCell->Color,
                               &SaveCell.Color);
#ifndef SS_NOBORDERS
               SS_LoadBorder(lpSS, lpSaveColorTable, SaveSpread.ColorTableCnt,
                             &lpCell->hBorder, &SaveCell.Border);
#endif

               if (SaveCell.fCellTypeProvided)
                  Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen,
                                        j, i, &hPalDefault, &dPaletteSize,
                                        &hGlobalPalette, sizeof(SS_SAVECELLTYPE3));

               if (SaveCell.fFormulaProvided)
                  {
                  lpText = (HPTSTR)Ptr;
                  Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
                  if ('=' == *lpText)
                     lpText++;
                  #ifndef SS_OLDCALC
                  SS_RegisterCustFuncs(lpSS, lpText);
                  #endif
                  SS_SetFormulaRange(lpSS, j, i, j, i, lpText, FALSE);
#endif
                  }

               if (SaveCell.fDataProvided)
                  {
                  lpText = (HPTSTR)Ptr;
                  Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

                  lpTextBuffer = SS_LoadData(lpSS, j, i, lpText,
                                             lpTextBuffer, &hTextBuffer,
                                             &dTextAllocLen);
                  }

               SSx_UnlockCellItem(lpSS, lpRow, j, i);
               }
            }
         }
      }

   SS_UnlockRowItem(lpSS, i);   // This should be here
   }

if (lpSS->lpBook->HorzScrollBar)
   SS_SetHScrollBar(lpSS);
else if (lpSS->lpBook->hWnd)
   SetScrollRange(lpSS->lpBook->hWnd, SB_HORZ, 0, 0, TRUE);

if (lpSS->lpBook->VertScrollBar)
   SS_SetVScrollBar(lpSS);
else if (lpSS->lpBook->hWnd)
   SetScrollRange(lpSS->lpBook->hWnd, SB_VERT, 0, 0, TRUE);

SS_BuildDependencies(lpSS);
SS_BookSetRedraw(lpSS->lpBook, TRUE);
//lpSS->Redraw = SaveSpread.Redraw;

for (i = 0; i < SaveSpread.FontTableCnt; i++)
   if (!lpSaveFontTable[i].fNoDelete)
      DeleteObject(lpSaveFontTable[i].hFont);

GlobalUnlock(hSaveFontTable);
GlobalFree(hSaveFontTable);

if (hSaveColorTable)
   {
   GlobalUnlock(hSaveColorTable);
   GlobalFree(hSaveColorTable);
   }

if (hTextBuffer)
   GlobalFree(hTextBuffer);

if (hGlobalPalette)
   GlobalFree(hGlobalPalette);

if (hPalDefault)
   DeleteObject(hPalDefault);

#ifdef SS_VB
#ifndef SS_NOPRINT
//vbSpreadPostLoad(lpSS->lpBook->hWnd, lpSS);
#endif
#endif

return (TRUE);
}


BOOL SS_LoadAllFromBuffer3(lpSS, lpBuffer, lBufferLen)

LPSPREADSHEET     lpSS;
HPBYTE            lpBuffer;
long              lBufferLen;
{
LPSS_BOOK         lpBook = lpSS->lpBook;
HPBYTE            Ptr;
SS_SAVESPREAD3    SaveSpread;
GLOBALHANDLE      hSaveFontTable = 0;
GLOBALHANDLE      hSaveColorTable = 0;
SS_SAVEROW2       SaveRow;
SS_SAVECOL2       SaveCol;
SS_SAVECELL2      SaveCell;
LPCOLORREF        lpSaveColorTable = 0;
COLORREF          Color;
SS_SAVELOGFONT    SaveLogFont;
LOGFONT           LogFont;
LPSS_SAVEFONTTBL  lpSaveFontTable = 0;
HPTSTR            lpText;
LPSS_ROW          lpRow;
LPSS_COL          lpCol;
LPSS_CELL         lpCell;
HPALETTE          hPalDefault = 0;
short             dTextAllocLen = 0;
LPTSTR            lpTextBuffer = NULL;
GLOBALHANDLE      hTextBuffer = 0;
short             dPaletteSize = 0;
short             dPointSize;
GLOBALHANDLE      hGlobalPalette = 0;
GLOBALHANDLE      hBuffer;
SS_COORD          i;
SS_COORD          j;
BYTE              bSaved;

Ptr = lpBuffer;
Ptr += sizeof(BYTE);                   // File Type
Ptr += sizeof(BYTE);                   // Version
Ptr += sizeof(long);                   // Buffer Len

SS_Reset(lpBook);
lpSS = SS_BookLockSheet(lpBook);

#if (defined(SS_QE) && defined(SS_VB))
vbSSClearDB(VBGetHwndControl(lpSS->lpBook->hWnd), lpSS->lpBook->hWnd);
#endif

if (!(lpSS->lpBook->hBuffer = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 330 * sizeof(TCHAR))))
   ;

#ifndef SS_NOCALC
SS_SetTypeEdit(&lpSS->lpBook->FormulaCellType, ES_LEFT | ES_AUTOHSCROLL,
               SS_CALC_FORMULA_MAXLEN, SS_CHRSET_CHR, SS_CASE_NOCASE);

SS_CreateControl(lpSS, &lpSS->lpBook->FormulaCellType, FALSE);
#endif

SS_BookSetRedraw(lpSS->lpBook, FALSE);
SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOCALC, FALSE);

MemHugeCpy(&SaveSpread, Ptr, sizeof(SS_SAVESPREAD3));
Ptr += sizeof(SS_SAVESPREAD3);

lpSS->lpBook->DocumentProtected = SaveSpread.DocumentProtected;
lpSS->DocumentLocked = SaveSpread.DocumentLocked;

if (SaveSpread.AllowResize)
   lpSS->wUserResize = SS_USERRESIZE_COL | SS_USERRESIZE_ROW;
else
   lpSS->wUserResize = 0;

if (SaveSpread.AllowSelectBlock)
   lpSS->lpBook->wSelBlockOption = SS_SELBLOCK_COLS | SS_SELBLOCK_ROWS |
                           SS_SELBLOCK_BLOCKS | SS_SELBLOCK_ALL;
else
   lpSS->lpBook->wSelBlockOption = 0;

lpSS->RestrictRows = SaveSpread.RestrictRows;
lpSS->RestrictCols = SaveSpread.RestrictCols;
lpSS->lpBook->CalcAuto = SaveSpread.CalcAuto;

if (SaveSpread.GridLines)
   lpSS->wGridType |= SS_GRID_HORIZONTAL | SS_GRID_VERTICAL;
else
   lpSS->wGridType &= ~(SS_GRID_HORIZONTAL | SS_GRID_VERTICAL);

lpSS->lpBook->HorzScrollBar = SaveSpread.HorzScrollBar;
lpSS->lpBook->VertScrollBar = SaveSpread.VertScrollBar;

lpSS->Color.BackgroundId = SaveSpread.Color.BackgroundId;
lpSS->Color.ForegroundId = SaveSpread.Color.ForegroundId;
lpSS->Color.ShadowColorId = SaveSpread.Color.ShadowColorId;
lpSS->Color.ShadowTextId = SaveSpread.Color.ShadowTextId;
lpSS->Color.ShadowDarkId = SaveSpread.Color.ShadowDarkId;
lpSS->Color.ShadowLightId = SaveSpread.Color.ShadowLightId;
lpSS->lpBook->GrayAreaBackground = SaveSpread.Color.GrayAreaBackground;
lpSS->lpBook->GrayAreaForeground = SaveSpread.Color.GrayAreaForeground;

lpSS->DefaultFontId = SaveSpread.DefaultFontId;
lpSS->Row.HeaderCnt = SaveSpread.HeaderRows;
lpSS->Col.HeaderCnt = SaveSpread.HeaderCols;
lpSS->Row.Frozen = SaveSpread.RowsFrozen;
lpSS->Col.Frozen = SaveSpread.ColsFrozen;
lpSS->Row.Max = SaveSpread.RowsMax;
lpSS->Col.Max = SaveSpread.ColsMax;
lpSS->Row.DataCnt = SaveSpread.DataRowCnt;
lpSS->Col.DataCnt = SaveSpread.DataColCnt;

lpSS->lpBook->DefaultDateFormat.bCentury = SaveSpread.DefaultDateFormat.bCentury;
lpSS->lpBook->DefaultDateFormat.cSeparator = SaveSpread.DefaultDateFormat.cSeparator;
lpSS->lpBook->DefaultDateFormat.nFormat = SaveSpread.DefaultDateFormat.nFormat;
lpSS->lpBook->DefaultDateFormat.bSpin = SaveSpread.DefaultDateFormat.cSeparator;
lpSS->lpBook->DefaultTimeFormat.b24Hour = SaveSpread.DefaultTimeFormat.b24Hour;
lpSS->lpBook->DefaultTimeFormat.bSeconds = SaveSpread.DefaultTimeFormat.bSeconds;
lpSS->lpBook->DefaultTimeFormat.cSeparator = SaveSpread.DefaultTimeFormat.cSeparator;
lpSS->lpBook->DefaultTimeFormat.bSpin = SaveSpread.DefaultTimeFormat.bSpin;

lpSS->lpBook->AllowUserFormulas = SaveSpread.AllowUserFormulas;
lpSS->lpBook->fEditModePermanent = SaveSpread.fEditModePermanent;
lpSS->lpBook->fAutoSize = SaveSpread.fAutoSize;
lpSS->lpBook->fScrollBarExtMode = SaveSpread.fScrollBarExtMode;
lpSS->lpBook->fProcessTab = SaveSpread.fProcessTab;
lpSS->lpBook->fScrollBarShowMax = SaveSpread.fScrollBarShowMax;
lpSS->lpBook->fScrollBarMaxAlign = SaveSpread.fScrollBarMaxAlign;
lpSS->lpBook->fArrowsExitEditMode = SaveSpread.fArrowsExitEditMode;
lpSS->lpBook->fMoveActiveOnFocus = SaveSpread.fMoveActiveOnFocus;
lpSS->lpBook->fRetainSelBlock = SaveSpread.fRetainSelBlock;
lpSS->lpBook->fEditModeReplace = SaveSpread.fEditModeReplace;
lpSS->lpBook->fNoBorder = SaveSpread.fNoBorder;
lpSS->lpBook->fNoBeep = SaveSpread.fNoBeep;
lpSS->lpBook->fAutoClipboard = SaveSpread.fAutoClipboard;
lpSS->lpBook->fAllowMultipleSelBlocks = SaveSpread.fAllowMultipleSelBlocks;
lpSS->lpBook->fAllowCellOverflow = SaveSpread.fAllowCellOverflow;
lpSS->fVirtualMode = SaveSpread.fVirtualMode;
lpSS->lpBook->fAllowDragDrop = SaveSpread.fAllowDragDrop;
lpSS->fSetBorder = SaveSpread.fSetBorder;
lpSS->lpBook->wButtonDrawMode = SaveSpread.wButtonDrawMode;
lpSS->lpBook->wEnterAction = SaveSpread.wEnterAction;
lpSS->wOpMode = SaveSpread.wOpMode;
lpSS->wGridType = SaveSpread.wGridType;
lpSS->wUserResize = SaveSpread.wUserResize;
lpSS->lpBook->wSelBlockOption = SaveSpread.wSelBlockOption;
lpSS->GridColor = SaveSpread.GridColor;
lpSS->lpBook->dUnitType = SaveSpread.dUnitType;
lpSS->Col.NumStart = SaveSpread.ColNumStart;
lpSS->Row.NumStart = SaveSpread.RowNumStart;
lpSS->Col.AutoSizeVisibleCnt = SaveSpread.AutoSizeVisibleCols;
lpSS->Row.AutoSizeVisibleCnt = SaveSpread.AutoSizeVisibleRows;
lpSS->ColHeaderDisplay = SaveSpread.ColHeaderDisplay;
lpSS->RowHeaderDisplay = SaveSpread.RowHeaderDisplay;
_fmemcpy(&lpSS->Virtual, &SaveSpread.Virtual, sizeof(SS_VIRTUAL));
lpSS->LockColor.BackgroundId = SaveSpread.LockColor.BackgroundId;
lpSS->LockColor.ForegroundId = SaveSpread.LockColor.ForegroundId;
_fmemcpy(&lpSS->lpBook->DefaultFloatFormat, &SaveSpread.DefaultFloatFormat,
         sizeof(FLOATFORMAT));

lpSS->DataFieldNameCnt = 0;

#ifndef SS_NOSCBAR
SS_VScrollSetSpecial(lpSS->lpBook, SaveSpread.fUseSpecialVScroll,
                     SaveSpread.wSpecialVScrollOptions);
#endif

lpSS->BlockCellUL.Col = -1;
lpSS->BlockCellUL.Row = -1;

lpSS->Row.UL = lpSS->Row.HeaderCnt + lpSS->Row.Frozen;
lpSS->Col.UL = lpSS->Col.HeaderCnt + lpSS->Col.Frozen;

lpSS->Row.CurAt = lpSS->Row.UL;
lpSS->Col.CurAt = lpSS->Col.UL;

if (SaveSpread.bMiscFlags & SS_SAVEMISC_XTRA)
   {
   _fmemcpy(&lpSS->lpBook->lXtraLen, Ptr, 4);
   Ptr += 4;

   if (lpSS->lpBook->lXtraLen)
      {
      if (hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                lpSS->lpBook->lXtraLen))
         {
         HPBYTE hpXtraBuff;
         hpXtraBuff = (HPBYTE)GlobalLock(hBuffer);
         MemHugeCpy(hpXtraBuff, Ptr, lpSS->lpBook->lXtraLen);
         GlobalUnlock(hBuffer);

         lpSS->lpBook->hXtra = hBuffer;
         }

      Ptr += lpSS->lpBook->lXtraLen;
      }
   }

#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))
if (SaveSpread.bMiscFlags & SS_SAVEMISC_DATAAWARE)
   if (!(Ptr = SS_LoadDataAware(lpSS->lpBook->hWnd, Ptr, lBufferLen)))
      return ((BOOL)NULL);
#endif

Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen, SS_ALLCOLS,
                      SS_ALLROWS, &hPalDefault, &dPaletteSize,
                      &hGlobalPalette, sizeof(SS_SAVECELLTYPE3));

if (SaveSpread.fDataProvided)
   {
   lpText = (HPTSTR)Ptr;
   Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

   lpTextBuffer = SS_LoadData(lpSS, SS_ALLCOLS, SS_ALLROWS, lpText,
                              lpTextBuffer, &hTextBuffer, &dTextAllocLen);
   }

/***************************
* Read FontTable structure
***************************/

if (!(hSaveFontTable = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                   SaveSpread.FontTableCnt *
                                   sizeof(SS_SAVEFONTTBL))))
   return (FALSE);

lpSaveFontTable = (LPSS_SAVEFONTTBL)GlobalLock(hSaveFontTable);

for (i = 0; i < SaveSpread.FontTableCnt; i++)
   {
   if (SaveSpread.fUseFontPointSize)
      {
      MemHugeCpy(&dPointSize, Ptr, sizeof(dPointSize));
      Ptr += sizeof(dPointSize);
      }

   MemHugeCpy(&SaveLogFont, Ptr, sizeof(SS_SAVELOGFONT));
   Ptr += sizeof(SS_SAVELOGFONT);
   _fmemset(&LogFont, 0, sizeof(LOGFONT));
   LogFont.lfHeight = SaveLogFont.lfHeight;
   LogFont.lfWidth = SaveLogFont.lfWidth;
   LogFont.lfEscapement = SaveLogFont.lfEscapement;
   LogFont.lfOrientation = SaveLogFont.lfOrientation;
   LogFont.lfWeight = SaveLogFont.lfWeight;
   LogFont.lfItalic = SaveLogFont.lfItalic;
   LogFont.lfUnderline = SaveLogFont.lfUnderline;
   LogFont.lfStrikeOut = SaveLogFont.lfStrikeOut;
   LogFont.lfCharSet = SaveLogFont.lfCharSet;
   LogFont.lfOutPrecision = SaveLogFont.lfOutPrecision;
   LogFont.lfClipPrecision = SaveLogFont.lfClipPrecision;
   LogFont.lfQuality = SaveLogFont.lfQuality;
   LogFont.lfPitchAndFamily = SaveLogFont.lfPitchAndFamily;
   _fmemcpy(LogFont.lfFaceName, SaveLogFont.lfFaceName,
            sizeof(LogFont.lfFaceName));  //???? need to translate for Unicode DLL/OCX

   if (SaveSpread.fUseFontPointSize)
      {
      LogFont.lfHeight = INTPT_TO_PIXELS(dPointSize);
      LogFont.lfWidth = 0;
      }

   lpSaveFontTable[i].hFont = CreateFontIndirect(&LogFont);
   }

if (SaveSpread.DefaultFontId <= 0)
   lpSS->DefaultFontId = -1;
else
   lpSS->DefaultFontId = SS_SaveInitFont(lpSS,
                             &lpSaveFontTable[SaveSpread.DefaultFontId - 1]);

if (SaveSpread.ColWidth == -1)
   {
   lpSS->Col.dCellSizeX100 = -1;
   lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, -1);
   }
else
   {
   lpSS->Col.dCellSizeX100 = (long)(SaveSpread.ColWidth * 100.0);
   lpSS->Col.dCellSizeInPixels = SS_ColWidthToPixels(lpSS, SaveSpread.ColWidth);
   }

if (SaveSpread.RowHeight == -1)
   {
   lpSS->Row.dCellSizeX100 = -1;
   lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, -1, -1);
   }
else
   {
   lpSS->Row.dCellSizeX100 = (long)(SaveSpread.RowHeight * 100.0);
   lpSS->Row.dCellSizeInPixels = SS_RowHeightToPixels(lpSS, -1,
                                                   SaveSpread.RowHeight);
   }

/****************************
* Read ColorTable structure
****************************/

if (SaveSpread.ColorTableCnt > 0)
   {
   if (!(hSaveColorTable = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                       SaveSpread.ColorTableCnt *
                                       sizeof(COLORREF))))
      return (FALSE);

   lpSaveColorTable = (LPCOLORREF)GlobalLock(hSaveColorTable);

   for (i = 0; i < SaveSpread.ColorTableCnt; i++)
      {
      MemHugeCpy(&Color, Ptr, sizeof(COLORREF));
      lpSaveColorTable[i] = Color;

      Ptr += sizeof(COLORREF);
      }

   if (SaveSpread.Color.ForegroundId == 0 ||
       SaveSpread.Color.ForegroundId > SaveSpread.ColorTableCnt)
      lpSS->Color.ForegroundId  = SS_AddColor(RGBCOLOR_BLACK);
   else
      lpSS->Color.ForegroundId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ForegroundId - 1]);

   if (SaveSpread.Color.BackgroundId == 0 ||
       SaveSpread.Color.BackgroundId > SaveSpread.ColorTableCnt)
      lpSS->Color.BackgroundId  = SS_AddColor(RGBCOLOR_WHITE);
   else
      lpSS->Color.BackgroundId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.BackgroundId - 1]);

   if (SaveSpread.Color.ShadowColorId == 0 ||
       SaveSpread.Color.ShadowColorId > SaveSpread.ColorTableCnt)
      lpSS->Color.ShadowColorId = SS_AddColor(GetSysColor(COLOR_BTNFACE));
   else
      lpSS->Color.ShadowColorId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowColorId - 1]);

   if (SaveSpread.Color.ShadowTextId == 0 ||
       SaveSpread.Color.ShadowTextId > SaveSpread.ColorTableCnt)
      lpSS->Color.ShadowTextId  = SS_AddColor(GetSysColor(COLOR_BTNTEXT));
   else
      lpSS->Color.ShadowTextId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowTextId - 1]);

   if (SaveSpread.Color.ShadowDarkId == 0 ||
       SaveSpread.Color.ShadowDarkId > SaveSpread.ColorTableCnt)
      lpSS->Color.ShadowDarkId  = SS_AddColor(GetSysColor(COLOR_BTNSHADOW));
   else
      lpSS->Color.ShadowDarkId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowDarkId - 1]);

   if (SaveSpread.Color.ShadowLightId == 0 ||
       SaveSpread.Color.ShadowLightId > SaveSpread.ColorTableCnt)
      lpSS->Color.ShadowLightId = SS_AddColor(GetSysColor(COLOR_BTNHIGHLIGHT));
   else
      lpSS->Color.ShadowLightId = SS_AddColor(lpSaveColorTable[SaveSpread.Color.ShadowLightId - 1]);

   if (SaveSpread.LockColor.BackgroundId == 0 ||
       SaveSpread.LockColor.BackgroundId > SaveSpread.ColorTableCnt)
      lpSS->LockColor.BackgroundId = 0;
   else
      lpSS->LockColor.BackgroundId = SS_AddColor(lpSaveColorTable[SaveSpread.LockColor.BackgroundId - 1]);

   if (SaveSpread.LockColor.ForegroundId == 0 ||
       SaveSpread.LockColor.ForegroundId > SaveSpread.ColorTableCnt)
      lpSS->LockColor.ForegroundId = 0;
   else
      lpSS->LockColor.ForegroundId = SS_AddColor(lpSaveColorTable[SaveSpread.LockColor.ForegroundId - 1]);
   }

/*******************
* Read each column
*******************/

for (i = 0; i < SaveSpread.ColCnt; i++)
   {
   bSaved = *Ptr;
   Ptr++;

   if (bSaved)
      {
      MemHugeCpy(&SaveCol, Ptr, sizeof(SS_SAVECOL2));
      Ptr += sizeof(SS_SAVECOL2);

      if (lpCol = SS_AllocLockCol(lpSS, i))
         {
         lpCol->ColLocked = (BYTE)SaveCol.ColLocked;

         if (SaveCol.ColWidth == -1)
            {
            lpCol->dColWidthX100 = -1;
            lpCol->dColWidthInPixels = -1;
            }
         else
            {
            lpCol->dColWidthX100 = (long)(SaveCol.ColWidth * 100.0);
            lpCol->dColWidthInPixels = SS_ColWidthToPixels(lpSS,
                                                           SaveCol.ColWidth);
            }

         lpCol->RowLastNonEmpty = SaveCol.RowLastNonEmpty;
         if (SaveCol.FontId == -1)
            lpCol->FontId = -1;
         else
            lpCol->FontId = SS_SaveInitFont(lpSS,
                                        &lpSaveFontTable[SaveCol.FontId - 1]);

         SS_SaveAddColor(lpSS, lpSaveColorTable, &lpCol->Color,
                         &SaveCol.Color);

#ifndef SS_NOBORDERS
         SS_LoadBorder(lpSS, lpSaveColorTable, SaveSpread.ColorTableCnt,
                       &lpCol->hBorder, &SaveCol.Border);
#endif
         lpCol->fPageBreak = SaveCol.fPageBreak;
         lpCol->bUserResize = SaveCol.bUserResize;
#ifndef SS_NOUSERDATA
         lpCol->lUserData = SaveCol.lUserData;
#endif

         if (SaveCol.fCellTypeProvided)
            Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen, i,
                                  SS_ALLROWS, &hPalDefault, &dPaletteSize,
                                  &hGlobalPalette, sizeof(SS_SAVECELLTYPE3));

         if (SaveCol.fFormulaProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
            if ('=' == *lpText)
               lpText++;
            #ifndef SS_OLDCALC
            SS_RegisterCustFuncs(lpSS, lpText);
            #endif
            SS_SetFormulaRange(lpSS, i, SS_ALLROWS, i, SS_ALLROWS,
                               lpText, FALSE);
#endif
            }

         if (SaveCol.fDataProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
            lpTextBuffer = SS_LoadData(lpSS, i, SS_ALLROWS, lpText,
                                       lpTextBuffer, &hTextBuffer,
                                       &dTextAllocLen);
            }

         /***********************
         * Read Data Aware Info
         ***********************/

#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))
         if (SaveSpread.bMiscFlags & SS_SAVEMISC_DATAAWARE)
            {
            long lLen;

            Ptr += 4;                  // Unused

            lpCol->bDataFillEvent = *Ptr;
            if (lpCol->bDataFillEvent)
               lpCol->bDataFillEvent = SS_DATAFILLEVENT_YES;

            Ptr += 1;

            lLen = StrHugeLen((HPTSTR)Ptr);

            if (lLen)
               {
               if (lpCol->hDBFieldName = tbGlobalAlloc(GMEM_MOVEABLE |
                                                       GMEM_ZEROINIT,
                                                       (lLen + 1)
                                                       * sizeof(TCHAR)))
                  {
                  lpText = (LPTSTR)tbGlobalLock(lpCol->hDBFieldName);
                  MemHugeCpy(lpText, Ptr, (lLen + 1) * sizeof(TCHAR));
                  tbGlobalUnlock(lpCol->hDBFieldName);
                  lpSS->DataFieldNameCnt++;
                  }
               }

            Ptr += lLen + 1;
            }
#endif

         SS_UnlockColItem(lpSS, i);
         }
      }
   }

/****************
* Read each row
****************/

for (i = 0; i < SaveSpread.RowCnt; i++)
   {
   lpRow = NULL;

   bSaved = *Ptr;
   Ptr++;

   if (bSaved)
      {
      MemHugeCpy(&SaveRow, Ptr, sizeof(SS_SAVEROW2));
      Ptr += sizeof(SS_SAVEROW2);

      if (lpRow = SS_AllocLockRow(lpSS, i))
         {
         lpRow->RowLocked = (BYTE)SaveRow.RowLocked;

         if (SaveRow.RowHeight == -1)
            {
            lpRow->dRowHeightX100 = -1;
            lpRow->dRowHeightInPixels = -1;
            }
         else
            {
            lpRow->dRowHeightX100 = (long)(SaveRow.RowHeight * 100.0);
            lpRow->dRowHeightInPixels = SS_CalcRowHeight(lpSS, i);
            }

         if (SaveRow.FontId == -1)
            lpRow->FontId = -1;
         else
            lpRow->FontId = SS_SaveInitFont(lpSS,
                                        &lpSaveFontTable[SaveRow.FontId - 1]);

         SS_SaveAddColor(lpSS, lpSaveColorTable, &lpRow->Color,
                         &SaveRow.Color);

#ifndef SS_NOBORDERS
         SS_LoadBorder(lpSS, lpSaveColorTable, SaveSpread.ColorTableCnt,
                       &lpRow->hBorder, &SaveRow.Border);
#endif
         lpRow->fRowSelected = SaveRow.fRowSelected;
         lpRow->fPageBreak = SaveRow.fPageBreak;
         lpRow->bUserResize = SaveRow.bUserResize;
#ifndef SS_NOUSERDATA
         lpRow->lUserData = SaveRow.lUserData;
#endif

         if (SaveRow.fCellTypeProvided)
            Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen,
                                  SS_ALLCOLS, i, &hPalDefault, &dPaletteSize,
                                  &hGlobalPalette, sizeof(SS_SAVECELLTYPE3));

         if (SaveRow.fFormulaProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
            if ('=' == *lpText)
               lpText++;
            #ifndef SS_OLDCALC
            SS_RegisterCustFuncs(lpSS, lpText);
            #endif
            SS_SetFormulaRange(lpSS, SS_ALLCOLS, i, SS_ALLCOLS, i,
                               lpText, FALSE);
#endif
            }

         if (SaveRow.fDataProvided)
            {
            lpText = (HPTSTR)Ptr;
            Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

            lpTextBuffer = SS_LoadData(lpSS, SS_ALLCOLS, i, lpText,
                                       lpTextBuffer, &hTextBuffer,
                                       &dTextAllocLen);
            }
         }

      /*****************
      * Read each Cell
      *****************/

      for (j = 0; j < SaveSpread.ColCnt; j++)
         {
         bSaved = *Ptr;
         Ptr++;

         if (bSaved)
            {
            MemHugeCpy(&SaveCell, Ptr, sizeof(SS_SAVECELL2));
            Ptr += sizeof(SS_SAVECELL2);

            if (lpCell = SS_AllocLockCell(lpSS, lpRow, j, i))
               {
               lpCell->CellLocked = (BYTE)SaveCell.CellLocked;

               if (SaveCell.FontId == -1)
                  lpCell->FontId = -1;
               else
                  lpCell->FontId = SS_SaveInitFont(lpSS,
                                   &lpSaveFontTable[SaveCell.FontId - 1]);

               SS_SaveAddColor(lpSS, lpSaveColorTable, &lpCell->Color,
                               &SaveCell.Color);
#ifndef SS_NOBORDERS
               SS_LoadBorder(lpSS, lpSaveColorTable, SaveSpread.ColorTableCnt,
                             &lpCell->hBorder, &SaveCell.Border);
#endif

               if (SaveCell.fCellTypeProvided)
                  Ptr = SS_LoadCellType(lpSS, Ptr, lBufferLen,
                                        j, i, &hPalDefault, &dPaletteSize,
                                        &hGlobalPalette, sizeof(SS_SAVECELLTYPE3));

               if (SaveCell.fFormulaProvided)
                  {
                  lpText = (HPTSTR)Ptr;
                  Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
#ifndef SS_NOCALC
                  if ('=' == *lpText)
                     lpText++;
                  #ifndef SS_OLDCALC
                  SS_RegisterCustFuncs(lpSS, lpText);
                  #endif
                  SS_SetFormulaRange(lpSS, j, i, j, i, lpText, FALSE);
#endif
                  }

               if (SaveCell.fDataProvided)
                  {
                  lpText = (HPTSTR)Ptr;
                  Ptr += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);

                  lpTextBuffer = SS_LoadData(lpSS, j, i, lpText,
                                             lpTextBuffer, &hTextBuffer,
                                             &dTextAllocLen);
                  }

               SSx_UnlockCellItem(lpSS, lpRow, j, i);
               }
            }
         }
      }

   SS_UnlockRowItem(lpSS, i);   // This should be here
   }

if (lpSS->lpBook->HorzScrollBar)
   SS_SetHScrollBar(lpSS);
else if (lpSS->lpBook->hWnd)
   SetScrollRange(lpSS->lpBook->hWnd, SB_HORZ, 0, 0, TRUE);

if (lpSS->lpBook->VertScrollBar)
   SS_SetVScrollBar(lpSS);
else if (lpSS->lpBook->hWnd)
   SetScrollRange(lpSS->lpBook->hWnd, SB_VERT, 0, 0, TRUE);

SS_BuildDependencies(lpSS);
SS_BookSetRedraw(lpSS->lpBook, TRUE);
//lpSS->Redraw = SaveSpread.Redraw;

for (i = 0; i < SaveSpread.FontTableCnt; i++)
   if (!lpSaveFontTable[i].fNoDelete)
      DeleteObject(lpSaveFontTable[i].hFont);

GlobalUnlock(hSaveFontTable);
GlobalFree(hSaveFontTable);

if (hSaveColorTable)
   {
   GlobalUnlock(hSaveColorTable);
   GlobalFree(hSaveColorTable);
   }

if (hTextBuffer)
   GlobalFree(hTextBuffer);

if (hGlobalPalette)
   GlobalFree(hGlobalPalette);

if (hPalDefault)
   DeleteObject(hPalDefault);

#ifdef SS_VB
#ifndef SS_NOPRINT
//vbSpreadPostLoad(lpSS->lpBook->hWnd, lpSS);
#endif
#endif

return (TRUE);
}


//--------------------------------------------------------------------
//
//  The CreateVbPict() function creates a Visual Basic picture handle
//  from the given bitmap or icon.
//

#ifdef SS_VB
HANDLE CreateVbPict(HANDLE hPicture, BOOL bIsBitmap, BOOL bIsIcon)
{
HPIC hPic = 0;
PIC pic;

if (hPicture)
   {
   if (bIsBitmap)
      {
      pic.picType = PICTYPE_BITMAP;
      pic.picData.bmp.hbitmap = hPicture;
      hPic = VBRefPic(VBAllocPic(&pic));
      }
   else if (bIsIcon)
      {
      pic.picType = PICTYPE_ICON;
      pic.picData.icon.hicon = hPicture;
      hPic = VBRefPic(VBAllocPic(&pic));
      }
   }
return (HANDLE)hPic;
}
#endif


//--------------------------------------------------------------------
//
//  The CreateOcxPict() function creates an OCX picture interface
//  from the given bitmap or icon.
//

// The following was moved to SS_SPICT.C because it requires 
// ole2.h & olectl.h. Those files cause "redefinition" conflicts 
// when including fpvbsub.h.
//
//#ifdef SS_OCX
//LPVOID CreateOcxPict(HANDLE hPicture, BOOL bIsBitmap)

// BS_CENTER was orginally defined as 0x0800 in ToolBox spreadsheet.
// Win95 introducted BS_CENTER as 0x0300.  BS_CENTER_OLD is used
// to convert from the old value to the new value when loading an
// old spreadsheet file.
#define BS_CENTER_OLD 0x0800L

HPBYTE SS_LoadCellType(lpSS, lpBuffer, lBufferLen, Col, Row,
                       lphPalDefault, lpdPaletteSize, lphGlobalPalette,
                       dCellTypeSize)

LPSPREADSHEET  lpSS;
HPBYTE         lpBuffer;
long           lBufferLen;
SS_COORD       Col;
SS_COORD       Row;
LPHPALETTE     lphPalDefault;
LPSHORT        lpdPaletteSize;
LPGLOBALHANDLE lphGlobalPalette;
short          dCellTypeSize;
{
SS_SAVECELLTYPE3 SaveCellType;
SS_CELLTYPE      CellType;
SS_CELLTYPE      CellTypeNew;
LPSS_CELLTYPE    lpCellType = 0;
HBITMAP          hBitmapDown;
HBITMAP          hBitmap;
LPTSTR           lpText = NULL;
LPTSTR           lpPict = NULL;
LPTSTR           lpPictDown = NULL;
short            i;

MemHugeCpy(&SaveCellType, lpBuffer, dCellTypeSize);
lpBuffer += dCellTypeSize;
_fmemset(&CellType, 0, sizeof(CellType));
CellType.Type = SaveCellType.Type;
CellType.ControlID = SaveCellType.ControlID;
CellType.Style = SaveCellType.Style;
switch (SaveCellType.Type)
   {
   case SS_TYPE_EDIT:
      CellType.Spec.Edit.ChrSet = SaveCellType.Spec.Edit.ChrSet;
      CellType.Spec.Edit.Len = SaveCellType.Spec.Edit.Len;
      CellType.Spec.Edit.ChrCase = SaveCellType.Spec.Edit.ChrCase;
      CellType.Spec.Edit.dfWidth = SaveCellType.Spec.Edit.dfWidth;
      CellType.Spec.Edit.nRowCnt = SaveCellType.Spec.Edit.nRowCnt;
      break;
   case SS_TYPE_INTEGER:
      CellType.Spec.Integer.Min = SaveCellType.Spec.Integer.Min;
      CellType.Spec.Integer.Max = SaveCellType.Spec.Integer.Max;
      CellType.Spec.Integer.fSpinWrap = SaveCellType.Spec.Integer.fSpinWrap;
      CellType.Spec.Integer.lSpinInc = SaveCellType.Spec.Integer.lSpinInc;
      break;
   case SS_TYPE_FLOAT:
      CellType.Spec.Float.Left = SaveCellType.Spec.Float.Left;
      CellType.Spec.Float.Right = SaveCellType.Spec.Float.Right;
      CellType.Spec.Float.Min = SaveCellType.Spec.Float.Min;
      CellType.Spec.Float.Max = SaveCellType.Spec.Float.Max;
      CellType.Spec.Float.fSetFormat = SaveCellType.Spec.Float.fSetFormat;
      CellType.Spec.Float.Format.cCurrencySign = SaveCellType.Spec.Float.Format.cCurrencySign;
      CellType.Spec.Float.Format.cDecimalSign = SaveCellType.Spec.Float.Format.cDecimalSign;
      CellType.Spec.Float.Format.cSeparator = SaveCellType.Spec.Float.Format.cSeparator;
      break;
   case SS_TYPE_COMBOBOX:
      CellType.Spec.ComboBox.dItemCnt = SaveCellType.Spec.ComboBox.dItemCnt;
      CellType.Spec.ComboBox.hItems = (TBGLOBALHANDLE)SaveCellType.Spec.ComboBox.hItems;
      break;
   case SS_TYPE_TIME:
      CellType.Spec.Time.SetFormat = SaveCellType.Spec.Time.SetFormat;
      CellType.Spec.Time.SetRange = SaveCellType.Spec.Time.SetRange;
      CellType.Spec.Time.Format.b24Hour = SaveCellType.Spec.Time.Format.b24Hour;
      CellType.Spec.Time.Format.bSeconds = SaveCellType.Spec.Time.Format.bSeconds;
      CellType.Spec.Time.Format.cSeparator = SaveCellType.Spec.Time.Format.cSeparator;
      CellType.Spec.Time.Format.bSpin = SaveCellType.Spec.Time.Format.bSpin;
      CellType.Spec.Time.Min.nHour = SaveCellType.Spec.Time.Min.nHour;
      CellType.Spec.Time.Min.nMinute = SaveCellType.Spec.Time.Min.nMinute;
      CellType.Spec.Time.Min.nSecond = SaveCellType.Spec.Time.Min.nSecond;
      CellType.Spec.Time.Max.nHour = SaveCellType.Spec.Time.Max.nHour;
      CellType.Spec.Time.Max.nMinute = SaveCellType.Spec.Time.Max.nMinute;
      CellType.Spec.Time.Max.nSecond = SaveCellType.Spec.Time.Max.nSecond;
      break;
   case SS_TYPE_DATE:
      CellType.Spec.Date.SetFormat = SaveCellType.Spec.Date.SetFormat;
      CellType.Spec.Date.SetRange = SaveCellType.Spec.Date.SetRange;
      CellType.Spec.Date.Format.bCentury = SaveCellType.Spec.Date.Format.bCentury;
      CellType.Spec.Date.Format.cSeparator = SaveCellType.Spec.Date.Format.cSeparator;
      CellType.Spec.Date.Format.nFormat = SaveCellType.Spec.Date.Format.nFormat;
      CellType.Spec.Date.Format.bSpin = SaveCellType.Spec.Date.Format.bSpin;
      CellType.Spec.Date.Min.nDay = SaveCellType.Spec.Date.Min.nDay;
      CellType.Spec.Date.Min.nMonth = SaveCellType.Spec.Date.Min.nMonth;
      CellType.Spec.Date.Min.nYear = SaveCellType.Spec.Date.Min.nYear;
      CellType.Spec.Date.Max.nDay = SaveCellType.Spec.Date.Max.nDay;
      CellType.Spec.Date.Max.nMonth = SaveCellType.Spec.Date.Max.nMonth;
      CellType.Spec.Date.Max.nYear = SaveCellType.Spec.Date.Max.nYear;
      break;
   case SS_TYPE_PIC:
      CellType.Spec.Pic.hMask = (TBGLOBALHANDLE)SaveCellType.Spec.Pic.hMask;
      break;
   case SS_TYPE_PICTURE:
      CellType.Spec.ViewPict.hPictName = (TBGLOBALHANDLE)SaveCellType.Spec.ViewPict.hPictName;
      CellType.Spec.ViewPict.hPal = SaveCellType.Spec.ViewPict.hPal;
      #ifdef SS_VB
      CellType.Spec.ViewPict.hPic = SaveCellType.Spec.ViewPict.hPic;
      CellType.Spec.ViewPict.fDeleteHandle = FALSE;
      #elif SS_OCX
      CellType.Spec.ViewPict.fDeleteHandle = FALSE;
      #else
      CellType.Spec.ViewPict.fDeleteHandle = TRUE;
      #endif
      break;
   case SS_TYPE_BUTTON:
      CellType.Spec.Button.hText = (HGLOBAL)SaveCellType.Spec.Button.hText;
      CellType.Spec.Button.hPictName = (HGLOBAL)SaveCellType.Spec.Button.hPictName;
      CellType.Spec.Button.hPictDownName = (HGLOBAL)SaveCellType.Spec.Button.hPictDownName;
      CellType.Spec.Button.nPictureType = SaveCellType.Spec.Button.nPictureType;
      CellType.Spec.Button.nPictureDownType = SaveCellType.Spec.Button.nPictureDownType;
      CellType.Spec.Button.ButtonType = SaveCellType.Spec.Button.ButtonType;
      CellType.Spec.Button.ShadowSize = SaveCellType.Spec.Button.ShadowSize;
      CellType.Spec.Button.fDelHandle = SaveCellType.Spec.Button.fDelHandle;
      CellType.Spec.Button.fDelDownHandle = SaveCellType.Spec.Button.fDelDownHandle;
      CellType.Spec.Button.Color.Color = SaveCellType.Spec.Button.Color.Color;
      CellType.Spec.Button.Color.ColorBorder = SaveCellType.Spec.Button.Color.ColorBorder;
      CellType.Spec.Button.Color.ColorShadow = SaveCellType.Spec.Button.Color.ColorShadow;
      CellType.Spec.Button.Color.ColorHighlight = SaveCellType.Spec.Button.Color.ColorHighlight;
      CellType.Spec.Button.Color.ColorText = SaveCellType.Spec.Button.Color.ColorText;
      #ifdef SS_VB
      CellType.Spec.Button.hPic = SaveCellType.Spec.Button.hPic;
      CellType.Spec.Button.hPicDown = SaveCellType.Spec.Button.hPicDown;
      #endif
      break;
   case SS_TYPE_CHECKBOX:
      if (CellType.Style & BS_CENTER_OLD)
         CellType.Style = BS_CENTER | (CellType.Style & ~BS_CENTER_OLD);
      CellType.Spec.CheckBox.hText = (HGLOBAL)SaveCellType.Spec.CheckBox.hText;
      CellType.Spec.CheckBox.hPictUpName = (TBGLOBALHANDLE)SaveCellType.Spec.CheckBox.hPictUpName;
      CellType.Spec.CheckBox.hPictDownName = (TBGLOBALHANDLE)SaveCellType.Spec.CheckBox.hPictDownName;
      CellType.Spec.CheckBox.hPictFocusUpName = (TBGLOBALHANDLE)SaveCellType.Spec.CheckBox.hPictFocusUpName;
      CellType.Spec.CheckBox.hPictFocusDownName = (TBGLOBALHANDLE)SaveCellType.Spec.CheckBox.hPictFocusDownName;
      CellType.Spec.CheckBox.hPictGrayName = (TBGLOBALHANDLE)SaveCellType.Spec.CheckBox.hPictGrayName;
      CellType.Spec.CheckBox.hPictFocusGrayName = (TBGLOBALHANDLE)SaveCellType.Spec.CheckBox.hPictFocusGrayName;
      CellType.Spec.CheckBox.bPictUpType = SaveCellType.Spec.CheckBox.bPictUpType;
      CellType.Spec.CheckBox.bPictDownType = SaveCellType.Spec.CheckBox.bPictDownType;
      CellType.Spec.CheckBox.bPictFocusUpType = SaveCellType.Spec.CheckBox.bPictFocusUpType;
      CellType.Spec.CheckBox.bPictFocusDownType = SaveCellType.Spec.CheckBox.bPictFocusDownType;
      CellType.Spec.CheckBox.bPictGrayType = SaveCellType.Spec.CheckBox.bPictGrayType;
      CellType.Spec.CheckBox.bPictFocusGrayType = SaveCellType.Spec.CheckBox.bPictFocusGrayType;
      CellType.Spec.CheckBox.fDelUpHandle = SaveCellType.Spec.CheckBox.fDelUpHandle;
      CellType.Spec.CheckBox.fDelDownHandle = SaveCellType.Spec.CheckBox.fDelDownHandle;
      CellType.Spec.CheckBox.fDelFocusUpHandle = SaveCellType.Spec.CheckBox.fDelFocusUpHandle;
      CellType.Spec.CheckBox.fDelFocusDownHandle = SaveCellType.Spec.CheckBox.fDelFocusDownHandle;
      CellType.Spec.CheckBox.fDelGrayHandle = SaveCellType.Spec.CheckBox.fDelGrayHandle;
      CellType.Spec.CheckBox.fDelFocusGrayHandle = SaveCellType.Spec.CheckBox.fDelFocusGrayHandle;
      #ifdef SS_VB
      CellType.Spec.CheckBox.hPicUp = SaveCellType.Spec.CheckBox.hPicUp;
      CellType.Spec.CheckBox.hPicDown = SaveCellType.Spec.CheckBox.hPicDown;
      CellType.Spec.CheckBox.hPicFocusUp = SaveCellType.Spec.CheckBox.hPicFocusUp;
      CellType.Spec.CheckBox.hPicFocusDown = SaveCellType.Spec.CheckBox.hPicFocusDown;
      CellType.Spec.CheckBox.hPicGray = SaveCellType.Spec.CheckBox.hPicGray;
      CellType.Spec.CheckBox.hPicFocusGray = SaveCellType.Spec.CheckBox.hPicFocusGray;
      #endif
      break;
   }

switch (CellType.Type)
   {
#ifndef SS_NOCT_DATE
   case SS_TYPE_DATE:
      lpCellType = SS_SetTypeDate(lpSS, &CellTypeNew, CellType.Style,
                                  &CellType.Spec.Date.Format,
                                  &CellType.Spec.Date.Min,
                                  &CellType.Spec.Date.Max);

      break;
#endif

   case SS_TYPE_EDIT:
      lpCellType = SS_SetTypeEdit(&CellTypeNew, CellType.Style,
                                  CellType.Spec.Edit.Len,
                                  CellType.Spec.Edit.ChrSet,
                                  CellType.Spec.Edit.ChrCase);

      break;

#ifndef SS_NOCT_PIC
   case SS_TYPE_PIC:
      if (CellType.Spec.Pic.hMask)
         {
         lpText = (HPTSTR)lpBuffer;
         lpBuffer += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
         }

      lpSS->lpBook->CurVisCell.Col = Col;
      lpSS->lpBook->CurVisCell.Row = Row;

      lpCellType = SS_SetTypePic(lpSS, &CellTypeNew, CellType.Style, lpText);
      break;
#endif

   case SS_TYPE_FLOAT:
      lpCellType = SS_SetTypeFloatExt(lpSS, &CellTypeNew, CellType.Style,
                                      CellType.Spec.Float.Left,
                                      CellType.Spec.Float.Right,
                                      CellType.Spec.Float.Min,
                                      CellType.Spec.Float.Max,
                                      CellType.Spec.Float.fSetFormat
                                        ? &CellType.Spec.Float.Format
                                        : NULL);
      break;

#ifndef SS_NOCT_INT
   case SS_TYPE_INTEGER:
      lpCellType = SS_SetTypeIntegerExt(lpSS, &CellTypeNew, CellType.Style,
                                        CellType.Spec.Integer.Min,
                                        CellType.Spec.Integer.Max,
                                        CellType.Spec.Integer.fSpinWrap,
                                        CellType.Spec.Integer.lSpinInc);
      break;
#endif

   case SS_TYPE_STATICTEXT:
      lpCellType = SS_SetTypeStaticText(lpSS, &CellTypeNew, CellType.Style);
      break;

#ifndef SS_NOCT_TIME
   case SS_TYPE_TIME:
      lpCellType = SS_SetTypeTime(lpSS, &CellTypeNew, CellType.Style,
                                  &CellType.Spec.Time.Format,
                                 &CellType.Spec.Time.Min,
                                 &CellType.Spec.Time.Max);
      break;
#endif

   case SS_TYPE_PICTURE:
      lpText = NULL;
      hBitmap = 0;

      if (CellType.Spec.ViewPict.hPictName &&
          !(CellType.Style & VPS_HANDLE))
         {
         lpText = (LPTSTR)lpBuffer;
         lpBuffer += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
         }

      else if (CellType.Spec.ViewPict.hPictName &&
               (CellType.Style & VPS_HANDLE))
         {
         lpBuffer = SS_LoadBitmap(lpBuffer, lBufferLen, &hBitmap,
                                  lphPalDefault, lpdPaletteSize,
                                  lphGlobalPalette);
         lpText = (LPTSTR)&hBitmap;

         if (CellType.Style & VPS_ICON)
            {
            CellType.Style &= ~VPS_ICON;
            CellType.Style |= VPS_BMP;
            }
         
         // BJO 05May97 SEL5626 - Begin fix
         // Orginal picture should have been a bitmap or an icon.
         // Icon pictures are converted to bitmaps when saved.  Thus,
         // the picture should now be a bitmap and have the VPS_BMP
         // style.
         if (hBitmap && !(CellType.Style & VPS_BMP))
            {
            DeleteObject(hBitmap);
            hBitmap = 0;
            }
         // BJO 05May97 SEL5626 - End fix
         }

      if (hBitmap)
      {
         lpCellType = SS_SetTypePictureHandle(lpSS, &CellTypeNew,
                                              CellType.Style, hBitmap, 0,
                                              CellType.Spec.ViewPict.fDeleteHandle);
         #if defined(SS_VB)
         CellTypeNew.Spec.ViewPict.hPic =
             CreateVbPict((HANDLE)CellTypeNew.Spec.ViewPict.hPictName,
                          (CellTypeNew.Style & VPS_BMP) == VPS_BMP,
                          (CellTypeNew.Style & VPS_ICON) == VPS_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.ViewPict.lpPict =
             CreateOcxPict((HANDLE)CellTypeNew.Spec.ViewPict.hPictName,
                        (CellTypeNew.Style & VPS_BMP) == VPS_BMP,
                        (CellTypeNew.Style & VPS_ICON) == VPS_ICON);
         #endif 
      }
      else
         lpCellType = SS_SetTypePicture(lpSS, &CellTypeNew, CellType.Style,
                                        lpText);
      break;

#ifndef SS_NOCT_BUTTON
   case SS_TYPE_BUTTON:
      CellType.Spec.Button.fDelHandle = FALSE;
      CellType.Spec.Button.fDelDownHandle = FALSE;

      if (CellType.Spec.Button.hText)
         {
         lpText = (HPTSTR)lpBuffer;
         lpBuffer += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
         }

      lpBuffer = SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                                   lphPalDefault, lpdPaletteSize,
                                   lphGlobalPalette,
                                   CellType.Spec.Button.hPictName,
                                   &CellType.Spec.Button.nPictureType,
                                   &lpPict, &hBitmap);

      lpBuffer = SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                                   lphPalDefault, lpdPaletteSize,
                                   lphGlobalPalette,
                                   CellType.Spec.Button.hPictDownName,
                                   &CellType.Spec.Button.nPictureDownType,
                                   &lpPictDown, &hBitmapDown);

      lpCellType = SS_SetTypeButton(lpSS, &CellTypeNew, CellType.Style,
                                    lpText, lpPict,
                                    CellType.Spec.Button.nPictureType,
                                    lpPictDown,
                                    CellType.Spec.Button.nPictureDownType,
                                    CellType.Spec.Button.ButtonType,
                                    CellType.Spec.Button.ShadowSize,
                                    &CellType.Spec.Button.Color);

      if (CellType.Spec.Button.hPictName &&
          (CellType.Spec.Button.nPictureType & SUPERBTN_PICT_HANDLE))
         {
         #if defined(SS_VB)
         CellTypeNew.Spec.Button.hPic =
            CreateVbPict(CellTypeNew.Spec.Button.hPictName,
                         (CellTypeNew.Spec.Button.nPictureType
                          & SUPERBTN_PICT_BITMAP) == SUPERBTN_PICT_BITMAP,
                         (CellTypeNew.Spec.Button.nPictureType
                          & SUPERBTN_PICT_ICON) == SUPERBTN_PICT_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.Button.lpPict =
            CreateOcxPict(CellTypeNew.Spec.Button.hPictName,
                          (CellTypeNew.Spec.Button.nPictureType
                           & SUPERBTN_PICT_BITMAP) == SUPERBTN_PICT_BITMAP,
                          (CellTypeNew.Spec.Button.nPictureType
                           & SUPERBTN_PICT_ICON) == SUPERBTN_PICT_ICON);
         #else
         SS_PicRef(lpSS->lpBook, (HANDLE)CellTypeNew.Spec.Button.hPictName);
         CellTypeNew.Spec.Button.fDelHandle = TRUE;
         #endif
         }

      if (CellType.Spec.Button.hPictDownName &&
          (CellType.Spec.Button.nPictureDownType & SUPERBTN_PICT_HANDLE))
         {
         #if defined(SS_VB)
         CellTypeNew.Spec.Button.hPicDown =
            CreateVbPict(CellTypeNew.Spec.Button.hPictDownName,
                         (CellTypeNew.Spec.Button.nPictureDownType
                          & SUPERBTN_PICT_BITMAP) == SUPERBTN_PICT_BITMAP,
                         (CellTypeNew.Spec.Button.nPictureDownType
                          & SUPERBTN_PICT_ICON) == SUPERBTN_PICT_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.Button.lpPictDown =
            CreateOcxPict(CellTypeNew.Spec.Button.hPictDownName,
                          (CellTypeNew.Spec.Button.nPictureDownType
                           & SUPERBTN_PICT_BITMAP) == SUPERBTN_PICT_BITMAP,
                          (CellTypeNew.Spec.Button.nPictureDownType
                           & SUPERBTN_PICT_ICON) == SUPERBTN_PICT_ICON);
         #else
         SS_PicRef(lpSS->lpBook, (HANDLE)CellTypeNew.Spec.Button.hPictDownName);
         CellTypeNew.Spec.Button.fDelDownHandle = TRUE;
         #endif
         }

      break;
#endif

#ifndef SS_NOCT_COMBO
   case SS_TYPE_COMBOBOX:
      if (CellType.Spec.ComboBox.hItems && CellType.Spec.ComboBox.dItemCnt)
         {
         lpText = (HPTSTR)lpBuffer;

         for (i = 0; i < CellType.Spec.ComboBox.dItemCnt; i++)
            {
            if (i > 0)
               *((HPTSTR)lpBuffer - 1) = '\t';

            lpBuffer += (StrHugeLen((HPTSTR)lpBuffer) + 1) * sizeof(TCHAR);
            }
         }

      lpCellType = SS_SetTypeComboBox(lpSS, &CellTypeNew, CellType.Style,
                                      lpText, -1, -1, 0, 0, SS_COMBOBOX_AUTOSEARCH_SINGLECHAR);

      break;
#endif

#ifndef SS_NOCT_CHECK
   case SS_TYPE_CHECKBOX:
      {
      LPTSTR  lpPictUp = NULL;
      HBITMAP hBitmapUp;
      LPTSTR  lpPictDown = NULL;
      HBITMAP hBitmapDown;
      LPTSTR  lpPictFocusUp = NULL;
      HBITMAP hBitmapFocusUp;
      LPTSTR  lpPictFocusDown = NULL;
      HBITMAP hBitmapFocusDown;
      LPTSTR  lpPictGray = NULL;
      HBITMAP hBitmapGray;
      LPTSTR  lpPictFocusGray = NULL;
      HBITMAP hBitmapFocusGray;

      CellType.Spec.CheckBox.fDelUpHandle = FALSE;
      CellType.Spec.CheckBox.fDelDownHandle = FALSE;
      CellType.Spec.CheckBox.fDelFocusUpHandle = FALSE;
      CellType.Spec.CheckBox.fDelFocusDownHandle = FALSE;
      CellType.Spec.CheckBox.fDelGrayHandle = FALSE;
      CellType.Spec.CheckBox.fDelFocusGrayHandle = FALSE;

      if (CellType.Spec.CheckBox.hText)
         {
         lpText = (HPTSTR)lpBuffer;
         lpBuffer += (StrHugeLen(lpText) + 1) * sizeof(TCHAR);
         }

      lpBuffer = SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                 lphPalDefault, lpdPaletteSize,
                 lphGlobalPalette,
                 (HANDLE)CellType.Spec.CheckBox.hPictUpName,
                 (LPSHORT)&CellType.Spec.CheckBox.bPictUpType,
                 &lpPictUp, &hBitmapUp);

      lpBuffer = SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                 lphPalDefault, lpdPaletteSize,
                 lphGlobalPalette,
                 (HANDLE)CellType.Spec.CheckBox.hPictDownName,
                 (LPSHORT)&CellType.Spec.CheckBox.bPictDownType,
                 &lpPictDown, &hBitmapDown);

      lpBuffer = SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                 lphPalDefault, lpdPaletteSize,
                 lphGlobalPalette,
                 (HANDLE)CellType.Spec.CheckBox.hPictFocusUpName,
                 (LPSHORT)&CellType.Spec.CheckBox.bPictFocusUpType,
                 &lpPictFocusUp, &hBitmapFocusUp);

      lpBuffer = SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                 lphPalDefault, lpdPaletteSize,
                 lphGlobalPalette,
                 (HANDLE)CellType.Spec.CheckBox.hPictFocusDownName,
                 (LPSHORT)&CellType.Spec.CheckBox.bPictFocusDownType,
                 &lpPictFocusDown, &hBitmapFocusDown);

      lpBuffer = SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                 lphPalDefault, lpdPaletteSize,
                 lphGlobalPalette,
                 (HANDLE)CellType.Spec.CheckBox.hPictGrayName,
                 (LPSHORT)&CellType.Spec.CheckBox.bPictGrayType,
                 &lpPictGray, &hBitmapGray);

      lpBuffer = SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                 lphPalDefault, lpdPaletteSize,
                 lphGlobalPalette,
                 (HANDLE)CellType.Spec.CheckBox.hPictFocusGrayName,
                 (LPSHORT)&CellType.Spec.CheckBox.bPictFocusGrayType,
                 &lpPictFocusGray, &hBitmapFocusGray);

      lpCellType = SS_SetTypeCheckBox(lpSS, &CellTypeNew, CellType.Style,
                                 lpText,
                                 lpPictUp,
                                 CellType.Spec.CheckBox.bPictUpType,
                                 lpPictDown,
                                 CellType.Spec.CheckBox.bPictDownType,
                                 lpPictFocusUp,
                                 CellType.Spec.CheckBox.bPictFocusUpType,
                                 lpPictFocusDown,
                                 CellType.Spec.CheckBox.bPictFocusDownType,
                                 lpPictGray,
                                 CellType.Spec.CheckBox.bPictGrayType,
                                 lpPictFocusGray,
                                 CellType.Spec.CheckBox.bPictFocusGrayType);

      if (CellType.Spec.CheckBox.hPictUpName &&
          (CellType.Spec.CheckBox.bPictUpType & SUPERBTN_PICT_HANDLE))
         {
         #if defined(SS_VB)
         CellTypeNew.Spec.CheckBox.hPicUp =
            CreateVbPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictUpName,
                         (CellTypeNew.Spec.CheckBox.bPictUpType
                          & BT_BITMAP) == BT_BITMAP,
                         (CellTypeNew.Spec.CheckBox.bPictUpType
                          & BT_ICON) == BT_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.CheckBox.lpPictUp =
            CreateOcxPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictUpName,
                          (CellTypeNew.Spec.CheckBox.bPictUpType
                           & BT_BITMAP) == BT_BITMAP,
                          (CellTypeNew.Spec.CheckBox.bPictUpType
                           & BT_ICON) == BT_ICON);
         #else
         SS_PicRef(lpSS->lpBook, (HANDLE)CellTypeNew.Spec.CheckBox.hPictUpName);
         CellTypeNew.Spec.CheckBox.fDelUpHandle = TRUE;
         #endif
         }

      if (CellType.Spec.CheckBox.hPictDownName &&
          (CellType.Spec.CheckBox.bPictDownType & SUPERBTN_PICT_HANDLE))
         {
         #if defined(SS_VB)
         CellTypeNew.Spec.CheckBox.hPicDown =
            CreateVbPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictDownName,
                         (CellTypeNew.Spec.CheckBox.bPictDownType
                          & BT_BITMAP) == BT_BITMAP,
                         (CellTypeNew.Spec.CheckBox.bPictDownType
                          & BT_ICON) == BT_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.CheckBox.lpPictDown =
            CreateOcxPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictDownName,
                          (CellTypeNew.Spec.CheckBox.bPictDownType
                           & BT_BITMAP) == BT_BITMAP,
                          (CellTypeNew.Spec.CheckBox.bPictDownType
                           & BT_ICON) == BT_ICON);
         #else
         SS_PicRef(lpSS->lpBook, (HANDLE)CellTypeNew.Spec.CheckBox.hPictDownName);
         CellTypeNew.Spec.CheckBox.fDelDownHandle = TRUE;
         #endif
         }

      if (CellType.Spec.CheckBox.hPictFocusUpName &&
          (CellType.Spec.CheckBox.bPictFocusUpType & SUPERBTN_PICT_HANDLE))
         {
         #if defined(SS_VB)
         CellTypeNew.Spec.CheckBox.hPicFocusUp =
            CreateVbPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusUpName,
                         (CellTypeNew.Spec.CheckBox.bPictFocusUpType
                          & BT_BITMAP) == BT_BITMAP,
                         (CellTypeNew.Spec.CheckBox.bPictFocusUpType
                          & BT_ICON) == BT_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.CheckBox.lpPictFocusUp =
            CreateOcxPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusUpName,
                          (CellTypeNew.Spec.CheckBox.bPictFocusUpType
                           & BT_BITMAP) == BT_BITMAP,
                          (CellTypeNew.Spec.CheckBox.bPictFocusUpType
                           & BT_ICON) == BT_ICON);
         #else
         SS_PicRef(lpSS->lpBook, (HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusUpName);
         CellTypeNew.Spec.CheckBox.fDelFocusUpHandle = TRUE;
         #endif
         }

      if (CellType.Spec.CheckBox.hPictFocusDownName &&
          (CellType.Spec.CheckBox.bPictFocusDownType & SUPERBTN_PICT_HANDLE))
         {
         #if defined(SS_VB)
         CellTypeNew.Spec.CheckBox.hPicFocusDown =
            CreateVbPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusDownName,
                         (CellTypeNew.Spec.CheckBox.bPictFocusDownType
                          & BT_BITMAP) == BT_BITMAP,
                         (CellTypeNew.Spec.CheckBox.bPictFocusDownType
                          & BT_ICON) == BT_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.CheckBox.lpPictFocusDown =
            CreateOcxPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusDownName,
                          (CellTypeNew.Spec.CheckBox.bPictFocusDownType
                           & BT_BITMAP) == BT_BITMAP,
                          (CellTypeNew.Spec.CheckBox.bPictFocusDownType
                           & BT_ICON) == BT_ICON);
         #else
         SS_PicRef(lpSS->lpBook, (HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusDownName);
         CellTypeNew.Spec.CheckBox.fDelFocusDownHandle = TRUE;
         #endif
         }

      if (CellType.Spec.CheckBox.hPictGrayName &&
          (CellType.Spec.CheckBox.bPictGrayType & SUPERBTN_PICT_HANDLE))
         {
         #if defined(SS_VB)
         CellTypeNew.Spec.CheckBox.hPicGray =
            CreateVbPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictGrayName,
                         (CellTypeNew.Spec.CheckBox.bPictGrayType
                          & BT_BITMAP) == BT_BITMAP,
                         (CellTypeNew.Spec.CheckBox.bPictGrayType
                          & BT_ICON) == BT_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.CheckBox.lpPictGray =
            CreateOcxPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictGrayName,
                          (CellTypeNew.Spec.CheckBox.bPictGrayType
                           & BT_BITMAP) == BT_BITMAP,
                          (CellTypeNew.Spec.CheckBox.bPictGrayType
                           & BT_ICON) == BT_ICON);
         #else
         SS_PicRef(lpSS->lpBook,
                    (HANDLE)CellTypeNew.Spec.CheckBox.hPictGrayName);
         CellTypeNew.Spec.CheckBox.fDelGrayHandle = TRUE;
         #endif
         }

      if (CellType.Spec.CheckBox.hPictFocusGrayName &&
         (CellType.Spec.CheckBox.bPictFocusGrayType & SUPERBTN_PICT_HANDLE))
         {
         #if defined(SS_VB)
         CellTypeNew.Spec.CheckBox.hPicFocusGray =
            CreateVbPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusGrayName,
                         (CellTypeNew.Spec.CheckBox.bPictFocusGrayType
                          & BT_BITMAP) == BT_BITMAP,
                         (CellTypeNew.Spec.CheckBox.bPictFocusGrayType
                          & BT_ICON) == BT_ICON);
         #elif defined(SS_OCX)
         CellTypeNew.Spec.CheckBox.lpPictFocusGray =
            CreateOcxPict((HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusGrayName,
                          (CellTypeNew.Spec.CheckBox.bPictFocusGrayType
                           & BT_BITMAP) == BT_BITMAP,
                          (CellTypeNew.Spec.CheckBox.bPictFocusGrayType
                           & BT_ICON) == BT_ICON);
         #else
         SS_PicRef(lpSS->lpBook,
                    (HANDLE)CellTypeNew.Spec.CheckBox.hPictFocusGrayName);
         CellTypeNew.Spec.CheckBox.fDelFocusGrayHandle = TRUE;
         #endif
         }
      }

      break;
#endif

#ifndef SS_NOCT_OWNERDRAW
   case SS_TYPE_OWNERDRAW:
      lpCellType = SS_SetTypeOwnerDraw(lpSS, &CellTypeNew, CellType.Style);
      break;
#endif
   }

if (lpCellType)
   SS_SetCellTypeRange(lpSS, Col, Row, Col, Row, lpCellType);

return (lpBuffer);
}


HPBYTE SS_LoadButtonPict(lpSS, lpBuffer, lBufferLen,
                         lphPalDefault, lpdPaletteSize, lphGlobalPalette,
                         hPictName, lpnPictType, lplpPict, lphBitmap)

LPSPREADSHEET  lpSS;
HPBYTE         lpBuffer;
long           lBufferLen;
LPHPALETTE     lphPalDefault;
LPSHORT        lpdPaletteSize;
LPGLOBALHANDLE lphGlobalPalette;
HANDLE         hPictName;
LPSHORT        lpnPictType;
LPTSTR FAR    *lplpPict;
HBITMAP FAR   *lphBitmap;
{
if (hPictName && !(*lpnPictType & SUPERBTN_PICT_HANDLE))
   {
   *lplpPict = (LPTSTR)lpBuffer;
   lpBuffer += (StrHugeLen((HPTSTR)lpBuffer) + 1) * sizeof(TCHAR);
   }
else if (hPictName && (*lpnPictType & SUPERBTN_PICT_HANDLE))
   {
   lpBuffer = SS_LoadBitmap(lpBuffer, lBufferLen, lphBitmap,
                            lphPalDefault, lpdPaletteSize,
                            lphGlobalPalette);
   *lplpPict = (LPTSTR)lphBitmap;

   if (*lpnPictType & SUPERBTN_PICT_ICON)
      {
      *lpnPictType &= ~SUPERBTN_PICT_ICON;
      *lpnPictType |= SUPERBTN_PICT_BITMAP;
      }
   }

return (lpBuffer);
}


HPBYTE SS_SaveAlloc(Buffer, BufferLen, Value, ValueLen, AllocLen, hBuffer)

HPBYTE            Buffer;
LPLONG            BufferLen;
LPVOID            Value;
long              ValueLen;
LPLONG            AllocLen;
GLOBALHANDLE FAR *hBuffer;
{
if (*BufferLen + ValueLen > *AllocLen)
   {
   *AllocLen += max(ValueLen, BUFFER_ALLOC_CNT);

   if (!(*hBuffer))
      {
      if (!(*hBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, *AllocLen)))
         {
         *BufferLen = 0;
         return (FALSE);
         }
      }

   else
      {
      GlobalUnlock(*hBuffer);
      if (!(*hBuffer = GlobalReAlloc(*hBuffer, *AllocLen,
                                     GMEM_MOVEABLE | GMEM_ZEROINIT)))
         {
         *BufferLen = 0;
         return (FALSE);
         }
      }

   Buffer = (HPBYTE)GlobalLock(*hBuffer);
   }

if (Value)
   MemHugeCpy(&Buffer[*BufferLen], Value, ValueLen);
else
   MemHugeSet(&Buffer[*BufferLen], '\0', (short)ValueLen);

*BufferLen += ValueLen;

return (Buffer);
}


HPBYTE SS_SavePicture(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                      Color, hPicture, lStyle, lpfPaletteSaved)

LPSPREADSHEET lpSS;
HPBYTE        Buffer;
LPLONG        BufferLen;
LPLONG        AllocLen;
LPHANDLE      hBuffer;
COLORREF      Color;
HANDLE        hPicture;
long          lStyle;
LPBOOL        lpfPaletteSaved;
{
if (lStyle & VPS_BMP)
   return (SS_SaveBitmap(Buffer, BufferLen, AllocLen, hBuffer,
                         (HBITMAP)hPicture, lpfPaletteSaved));
else
   return (SS_SaveIcon(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                       (HICON)hPicture, Color, lpfPaletteSaved));
}


HPBYTE SS_SaveButtonPicture(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                            Color, hPicture, nPictureType, lpfPaletteSaved)

LPSPREADSHEET lpSS;
HPBYTE        Buffer;
LPLONG        BufferLen;
LPLONG        AllocLen;
LPHANDLE      hBuffer;
COLORREF      Color;
HANDLE        hPicture;
short         nPictureType;
LPBOOL        lpfPaletteSaved;
{
if (nPictureType & SUPERBTN_PICT_BITMAP)
   return (SS_SaveBitmap(Buffer, BufferLen, AllocLen, hBuffer,
                         (HBITMAP)hPicture, lpfPaletteSaved));
else
   return (SS_SaveIcon(lpSS, Buffer, BufferLen, AllocLen, hBuffer,
                       (HICON)hPicture, Color, lpfPaletteSaved));
}


HPBYTE SS_SaveIcon(lpSS, Buffer, BufferLen, AllocLen, hBuffer, hIcon,
                   Color, lpfPaletteSaved)

LPSPREADSHEET lpSS;
HPBYTE        Buffer;
LPLONG        BufferLen;
LPLONG        AllocLen;
LPHANDLE      hBuffer;
HICON         hIcon;
COLORREF      Color;
LPBOOL        lpfPaletteSaved;
{
HBITMAP       hBitmap;
HWND          hWnd;

if (lpSS->lpBook->hWnd)  // handle an OCX control without a HWND
   hWnd = lpSS->lpBook->hWnd;
else
   hWnd = GetDesktopWindow();

hBitmap = tbIconToBitmap(hWnd, hIcon, Color);

Buffer = SS_SaveBitmap(Buffer, BufferLen, AllocLen, hBuffer, hBitmap,
                       lpfPaletteSaved);

DeleteObject(hBitmap);

return (Buffer);
}


HPBYTE SS_SaveBitmap(Buffer, BufferLen, AllocLen, hBuffer, hBitmap,
                         lpfPaletteSaved)

HPBYTE       Buffer;
LPLONG       BufferLen;
LPLONG       AllocLen;
LPHANDLE     hBuffer;
HBITMAP      hBitmap;
LPBOOL       lpfPaletteSaved;
{
GLOBALHANDLE hDib;
LPBYTE       lpDib;
DWORD        dwLen;
DWORD        dwSaveLen;
BYTE         bPaletteType;
short        dPaletteSize;

if (!(hDib = tbDibFromBitmap(hBitmap, BI_RGB, 0, 0, &dwLen, &dPaletteSize)))
   dwLen = 0;

if (*lpfPaletteSaved)
   {
   bPaletteType = SS_SAVEPALETTETYPE_USEDEF;

   dwSaveLen = dwLen - dPaletteSize;

   if (Buffer = SS_SaveAlloc(Buffer, BufferLen, &bPaletteType, sizeof(BYTE),
                             AllocLen, hBuffer))
      Buffer = SS_SaveAlloc(Buffer, BufferLen, &dwSaveLen, sizeof(DWORD),
                            AllocLen, hBuffer);

   if (!Buffer)
      {
      if (hDib)
         GlobalFree(hDib);

      return (NULL);
      }

   if (hDib)
      {
      lpDib = (LPBYTE)GlobalLock(hDib);

      if (Buffer = SS_SaveAlloc(Buffer, BufferLen, lpDib,
                                sizeof(BITMAPINFOHEADER), AllocLen, hBuffer))
         Buffer = SS_SaveAlloc(Buffer, BufferLen,
                               &lpDib[sizeof(BITMAPINFOHEADER) + dPaletteSize],
                               dwLen - (sizeof(BITMAPINFOHEADER) +
                               dPaletteSize), AllocLen, hBuffer);

      GlobalUnlock(hDib);
      GlobalFree(hDib);
      }
   }

else
   {
   bPaletteType = SS_SAVEPALETTETYPE_LOADDEF;

   if (Buffer = SS_SaveAlloc(Buffer, BufferLen, &bPaletteType, sizeof(BYTE),
                             AllocLen, hBuffer))
      Buffer = SS_SaveAlloc(Buffer, BufferLen, &dwLen, sizeof(DWORD),
                            AllocLen, hBuffer);

   if (!Buffer)
      {
      if (hDib)
         GlobalFree(hDib);

      return (NULL);
      }

   if (hDib)
      {
      lpDib = (LPBYTE)GlobalLock(hDib);

      Buffer = SS_SaveAlloc(Buffer, BufferLen, lpDib, dwLen, AllocLen,
                            hBuffer);

      GlobalUnlock(hDib);
      GlobalFree(hDib);
      }

   *lpfPaletteSaved = TRUE;
   }

return (Buffer);
}


HPBYTE SS_LoadBitmap(lpBuffer, lBufferLen, lphBitmap, lphPalDefault,
                     lpdPaletteSize, lphGlobalPalette)

HPBYTE         lpBuffer;
long           lBufferLen;
LPHBITMAP      lphBitmap;
LPHPALETTE     lphPalDefault;
LPSHORT        lpdPaletteSize;
LPGLOBALHANDLE lphGlobalPalette;
{
GLOBALHANDLE   hDib;
HPALETTE       hPal;
LPBYTE         lpDib;
LPBYTE         lpTemp;
DWORD          dwLen;
BYTE           bPaletteType;

*lphBitmap = 0;

MemHugeCpy(&bPaletteType, lpBuffer, sizeof(BYTE));
lpBuffer += sizeof(BYTE);
MemHugeCpy(&dwLen, lpBuffer, sizeof(DWORD));
lpBuffer += sizeof(DWORD);

if (dwLen > 0)
   {
   if (bPaletteType == SS_SAVEPALETTETYPE_LOADDEF)
      {
      if (!(hDib = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwLen)))
         return (lpBuffer + dwLen);

      lpDib = (LPBYTE)GlobalLock(hDib);
      MemHugeCpy(lpDib, lpBuffer, dwLen);
      lpBuffer += dwLen;
      GlobalUnlock(hDib);

      hPal = tbCreateDibPalette(hDib);
      *lphPalDefault = hPal;

      lpDib = (LPBYTE)GlobalLock(hDib);
      *lpdPaletteSize = tbPaletteSize(lpDib);
      GlobalUnlock(hDib);

      if (*lphGlobalPalette = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
          *lpdPaletteSize))
         {
         lpDib = (LPBYTE)GlobalLock(hDib);
         lpTemp = (LPBYTE)GlobalLock(*lphGlobalPalette);
         _fmemcpy(lpTemp, &lpDib[sizeof(BITMAPINFOHEADER)], *lpdPaletteSize);
         GlobalUnlock(*lphGlobalPalette);
         GlobalUnlock(hDib);
         }

      *lphBitmap = tbBitmapFromDib(hDib, hPal, TRUE);
      }
   else
      {
      if (!(hDib = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwLen +
                               *lpdPaletteSize)))
         return (lpBuffer + dwLen);

      lpDib = (LPBYTE)GlobalLock(hDib);
      MemHugeCpy(lpDib, lpBuffer, sizeof(BITMAPINFOHEADER));
      lpBuffer += sizeof(BITMAPINFOHEADER);

      lpTemp = (LPBYTE)GlobalLock(*lphGlobalPalette);
      MemHugeCpy(&lpDib[sizeof(BITMAPINFOHEADER)], lpTemp, *lpdPaletteSize);
      GlobalUnlock(*lphGlobalPalette);

      MemHugeCpy(&lpDib[sizeof(BITMAPINFOHEADER) + *lpdPaletteSize],
                 lpBuffer, dwLen - sizeof(BITMAPINFOHEADER));
      lpBuffer += dwLen - sizeof(BITMAPINFOHEADER);
      GlobalUnlock(hDib);

      *lphBitmap = tbBitmapFromDib(hDib, *lphPalDefault, TRUE);
      }

   GlobalFree(hDib);
   }

return (lpBuffer);
}


void SS_SaveAddColor(lpSS, lpSaveColorTable, lpColorNew, lpColorOld)

LPSPREADSHEET  lpSS;
LPCOLORREF     lpSaveColorTable;
LPSS_COLORITEM lpColorNew;
LPSS_SAVECOLORITEM lpColorOld;
{
if (lpSaveColorTable)
   {
   if (lpColorOld->ForegroundId == 0)
      lpColorNew->ForegroundId = 0;
   else
      lpColorNew->ForegroundId = SS_AddColor(lpSaveColorTable[lpColorOld->ForegroundId - 1]);

   if (lpColorOld->BackgroundId == 0)
      lpColorNew->BackgroundId = 0;
   else
      lpColorNew->BackgroundId = SS_AddColor(lpSaveColorTable[lpColorOld->BackgroundId - 1]);
   }

else
   {
   lpColorNew->ForegroundId = 0;
   lpColorNew->BackgroundId = 0;
   }
}


#ifndef SS_NOBORDERS

void SS_LoadBorder(LPSPREADSHEET lpSS, LPCOLORREF lpSaveColorTable,
                   short dColorTableCnt, LPTBGLOBALHANDLE lphBorderNew,
                   LPSS_SAVEBORDER lpBorderOld)
{
COLORREF clr = RGBCOLOR_DEFAULT;

if (lpSaveColorTable && lpBorderOld->ColorId > 0)
   clr = lpSaveColorTable[lpBorderOld->ColorId - 1];

SS_SetBorderItem(lpSS, lphBorderNew, lpBorderOld->bType, lpBorderOld->bStyle,
                 clr);
}

#endif


SS_COLORID SS_LoadColor(LPSPREADSHEET lpSS, LPCOLORREF lpSaveColorTable,
                        SS_COLORID ColorIdOld)
{
SS_COLORID ColorId = 0;

if (lpSaveColorTable && ColorIdOld > 0)
   ColorId = SS_AddColor(lpSaveColorTable[ColorIdOld - 1]);

return (ColorId);
}


LPTSTR SS_LoadData(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                   HPTSTR hpText, LPTSTR lpBuffer,
                   LPGLOBALHANDLE lphBuffer, LPSHORT lpdAllocLen)
{
short dValueLen = (short)(StrHugeLen(hpText) + 1) * sizeof(TCHAR);

if (dValueLen > *lpdAllocLen)
   {
   *lpdAllocLen += max(dValueLen, 50);

   if (!(*lphBuffer))
      {
      if (!(*lphBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                     *lpdAllocLen)))
         return (FALSE);
      }

   else
      {
      GlobalUnlock(*lphBuffer);
      if (!(*lphBuffer = GlobalReAlloc(*lphBuffer, *lpdAllocLen,
                                       GMEM_MOVEABLE | GMEM_ZEROINIT)))
         return (FALSE);
      }

   lpBuffer = (LPTSTR)GlobalLock(*lphBuffer);
   }

MemHugeCpy(lpBuffer, hpText, dValueLen);

SS_SetDataRange(lpSS, Col, Row, Col, Row, lpBuffer, FALSE, FALSE, FALSE);
return (lpBuffer);
}


LPTSTR SS_LoadCellData(HWND hWnd, SS_COORD Col, SS_COORD Row, HPTSTR hpText,
                      LPTSTR lpBuffer, LPGLOBALHANDLE lphBuffer,
                      LPSHORT lpdAllocLen)
{
LPSPREADSHEET lpSS;
short dValueLen = (short)(StrHugeLen(hpText) + 1) * sizeof(TCHAR);

lpSS = SS_SheetLock(hWnd);

if (dValueLen > *lpdAllocLen)
   {
   *lpdAllocLen += max(dValueLen, 50);

   if (!(*lphBuffer))
      {
      if (!(*lphBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                     *lpdAllocLen)))
         return (FALSE);
      }

   else
      {
      GlobalUnlock(*lphBuffer);
      if (!(*lphBuffer = GlobalReAlloc(*lphBuffer, *lpdAllocLen,
                                       GMEM_MOVEABLE | GMEM_ZEROINIT)))
         return (FALSE);
      }

   lpBuffer = (LPTSTR)GlobalLock(*lphBuffer);
   }

MemHugeCpy(lpBuffer, hpText, dValueLen);

SS_SetCellData(lpSS, Col, Row, lpBuffer, (short)StrHugeLen(hpText));

SS_SheetUnlock(hWnd);

return (lpBuffer);
}


short SS_SaveInitFont(LPSPREADSHEET lpSS, LPSS_SAVEFONTTBL lpSaveFont)
{
short dID;
BOOL  fDel;

dID = SS_InitFont(lpSS->lpBook, lpSaveFont->hFont, -1, FALSE, &fDel);

if (!fDel)
   lpSaveFont->fNoDelete = TRUE;

return (dID);
}


static long StrHugeLen(HPTSTR hpStr)
{
long lLen = 0L;

while (*hpStr++)
   lLen++;

return (lLen);
}


#if ((defined(SS_BOUNDCONTROL) || defined(SS_QE)) && !defined(SS_OCX))

HPBYTE SS_SaveDataAware(HWND hWnd, HPBYTE Buffer, LPLONG BufferLen,
                        LPLONG AllocLen, LPHANDLE hBuffer)
{
LPVBSPREAD lpSpread;
HCTL       hCtl = VBGetHwndControl(hWnd);
char       szUnused[20];

lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);

#ifdef SS_QE

if (lpSpread->DBInfo.hDataConnect)
   {
   LPTSTR lpszText;

   lpszText = (LPTSTR)tbGlobalLock(lpSpread->DBInfo.hDataConnect);
   if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpszText,
                               (lstrlen(lpszText) + 1) * sizeof(TCHAR),
                               AllocLen, hBuffer)))
      return (NULL);
   tbGlobalUnlock(lpSpread->DBInfo.hDataConnect);
   }
else
   if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, _T(""), sizeof(TCHAR),
                               AllocLen, hBuffer)))
      return (NULL);

if (lpSpread->DBInfo.hDataSelect)
   {
   LPTSTR lpszText;

   lpszText = (LPTSTR)tbGlobalLock(lpSpread->DBInfo.hDataSelect);
   if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpszText,
                               (lstrlen(lpszText) + 1) * sizeof(TCHAR),
                               AllocLen, hBuffer)))
   tbGlobalUnlock(lpSpread->DBInfo.hDataSelect);
   }
else
   if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, _T(""), sizeof(TCHAR),
                               AllocLen, hBuffer)))
      return (NULL);

#else

   if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, _T("\0"), 2 * sizeof(TCHAR),
                               AllocLen, hBuffer)))
      return (NULL);

#endif

#if 0
   {
   HSZ hszText;

   if (!VBGetControlProperty(hCtl, IPROP_SPREAD_DATASOURCE, &hszText))
      {
      lpszText = VBDerefHsz(hszText);

      if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, lpszText,
                                  lstrlen(lpszText) + 1, AllocLen, hBuffer)))
         return (NULL);

      VBDestroyHsz(hszText);
      }
   else
      if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, "", 1, AllocLen, hBuffer)))
         return (NULL);
   }
#else
   if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, "", 1, AllocLen, hBuffer)))
      return (NULL);
#endif

if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, &lpSpread->DataOpts,
                            sizeof(DATAOPTS), AllocLen, hBuffer)))
   return (NULL);

_fmemset(szUnused, '\0', sizeof(szUnused));

if (!(Buffer = SS_SaveAlloc(Buffer, BufferLen, szUnused, sizeof(szUnused),
                            AllocLen, hBuffer)))
   return (NULL);

return (Buffer);
}
#endif // (SS_BOUNDCONTROL || SS_QE) && !SS_OCX 

#if (defined(SS_BOUNDCONTROL) || defined(SS_QE))
HPBYTE SS_LoadDataAware(HWND hWnd, HPBYTE lpBuffer, LONG lBufferLen)
{
LPVBSPREAD lpSpread;
long       lLen;
short      nDataOpts;

#ifdef SS_OCX  
// Lock structures
LPSPREADSHEET lpSS = SS_SheetLock(hWnd);
lpSpread = (LPVBSPREAD)SSOcxLockVBSpread((LONG)lpSS->lpBook->lpOleControl);

#else //not SS_OCX
HCTL       hCtl = VBGetHwndControl(hWnd);
lpSpread = (LPVBSPREAD)VBDerefControl(hCtl);
#endif

lLen = StrHugeLen((HPTSTR)lpBuffer);

#ifdef SS_QE

if (lLen)
   {
   if (lpSpread->DBInfo.hDataConnect = tbGlobalAlloc(GMEM_MOVEABLE |
                                                     GMEM_ZEROINIT,
                                                     (lLen + 1)
                                                     * sizeof(TCHAR)))
      {
      LPTSTR lpText;

      lpText = (LPTSTR)tbGlobalLock(lpSpread->DBInfo.hDataConnect);
      MemHugeCpy(lpText, lpBuffer, (lLen + 1) * sizeof(TCHAR));
      tbGlobalUnlock(lpSpread->DBInfo.hDataConnect);
      }
   }

#endif

lpBuffer += lLen + 1;

lLen = StrHugeLen((HPTSTR)lpBuffer);

#ifdef SS_QE

if (lLen)
   {
   if (lpSpread->DBInfo.hDataSelect = tbGlobalAlloc(GMEM_MOVEABLE |
                                                    GMEM_ZEROINIT,
                                                    (lLen + 1)
                                                    * sizeof(TCHAR)))
      {
      LPTSTR lpText;

      lpText = (LPTSTR)tbGlobalLock(lpSpread->DBInfo.hDataSelect);
      MemHugeCpy(lpText, lpBuffer, (lLen + 1) * sizeof(TCHAR));
      tbGlobalUnlock(lpSpread->DBInfo.hDataSelect);
      }
   }

#endif

lpBuffer += lLen + 1;

lLen = StrHugeLen((HPTSTR)lpBuffer);

#if (defined(SS_BOUNDCONTROL) && !defined(SS_OCX))
if (lLen)
   VBSetControlProperty(hCtl, IPROP_SPREAD_DATASOURCE, (long)lpBuffer);
#endif

lpBuffer += lLen + 1;

// Note: The DATAOPTS type is a structure of bitfields.  The size of
//       a DATAOPTS stucture is compiler dependent.
MemHugeCpy(&nDataOpts, lpBuffer, sizeof(nDataOpts));
lpBuffer += sizeof(nDataOpts);
lpSpread->DataOpts.fAutoSave = (nDataOpts >> 0) & 1;
lpSpread->DataOpts.fHeadings = (nDataOpts >> 1) & 1;
lpSpread->DataOpts.fCellTypes = (nDataOpts >> 2) & 1;
lpSpread->DataOpts.fAutoFill = (nDataOpts >> 3) & 1;
lpSpread->DataOpts.fSizeCols = (nDataOpts >> 4) & 2;
lpSpread->DataOpts.fAutoDataInform = (nDataOpts >> 6) & 1;

lpBuffer += 20;                        // Unused

#ifdef SS_OCX
// Unlock structures
SSOcxUnlockVBSpread((LONG)lpSS->lpBook->lpOleControl);
SS_SheetUnlock(hWnd);
#endif
return (lpBuffer);
}

#endif // SS_BOUNDCONTROL || SS_QE


#else  //ifndef SS_NOSAVE

BOOL DLLENTRY SSSaveToFile(hWnd, lpFileName, fDataOnly)

HWND   hWnd;
LPTSTR lpFileName;
BOOL   fDataOnly;
{
return (1);
}


GLOBALHANDLE DLLENTRY SSSaveToBuffer(hWnd, lpBufferLen, fDataOnly)

HWND      hWnd;
long far *lpBufferLen;
BOOL      fDataOnly;
{
return (1);
}


BOOL DLLENTRY SSLoadFromFile(hWnd, lpFileName)

HWND   hWnd;
LPTSTR lpFileName;
{
return (1);
}


BOOL DLLENTRY SSLoadFromBuffer(hWnd, hBuffer, lBufferLen)

HWND         hWnd;
GLOBALHANDLE hBuffer;
long         lBufferLen;
{
return (1);
}


BOOL DLLENTRY SSSaveTabFile(HWND hWnd, LPTSTR lpFileName)
{
return (1);
}

#endif  //ifndef-else SS_NOSAVE
