/*********************************************************
* SS_EXCEL.C
* 
* Copyright (C) 1999 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
* UPDATE LOG:
* -----------

* RAP   - 12.10.98
* RAP01 - 03.04.99
*********************************************************/
#if defined(SS_V30) && defined(WIN32)

#if ((defined(SS_OCX) || defined(FP_DLL)) && defined(WIN32))
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#endif
#endif

#include <windows.h>
#include <math.h>
#include <tchar.h>
#if defined(SS_DDE)
#include <oaidl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ole2.h>
#include <memory.h>
#include "..\..\..\..\fplibs\fptools\src\fpconvrt.h"
#include "spread.h"
#include "ss_alloc.h"
#include "ss_bool.h"
#include "ss_calc.h"
#include "ss_col.h"
#include "ss_data.h"
#include "ss_dde.h"
#include "ss_doc.h"
#include "ss_draw.h"
#include "ss_formu.h"
#include "ss_main.h"
#include "ss_row.h"
#include "ss_save.h"
#include "ss_save4.h"
#include "ss_type.h"
#include "ss_user.h"
#include "ss_virt.h"
#include "ss_win.h"
#include "ss_excel.h"
#if defined(SS_V80) && defined(XL12)
#include "ss_xl12.h"
#endif

#define DEF_SHEETNAME _T("Sheet1")
#define EXCEL_EXTENSION _T(".xls")
#define EXCEL_EXTENSIONNODOT _T("xls")

//--------------------------------------------------------------------
//
//  The SSIsExcelFile() function is used determine if a file is a
//  Structured Storage Model (SSM) file, and has a BIFF formatted section
//  that is in BIFF8 format.

int DLLENTRY SSIsExcelFile(LPCTSTR lpszFileName)
{
int nRet = FALSE;

nRet = xl_LoadFile(lpszFileName, NULL, NULL, 0, 0, TRUE);

if (nRet == 0)
  return 1;
else if (nRet == 2)
  return 2;
else
  return 0;
}

//--------------------------------------------------------------------
//
//  The SS_FreeXLWSMemory() function is used to free the memory allocated
//  for an Excel sheet.

BOOL SS_FreeXLWSMemory(LPVOID lpXL)
{
LPSSXL lp = (LPSSXL)lpXL;

if (lp->SubStream.pls.ghPrintStruct)  
{
  tbGlobalFree(lp->SubStream.pls.ghPrintStruct);      
  lp->SubStream.pls.ghPrintStruct = 0;
}
if (lp->SubStream.sel.ghRefs)  
{
  tbGlobalFree(lp->SubStream.sel.ghRefs);      
  lp->SubStream.sel.ghRefs = 0;
}
if (lp->SubStream.ghHeader)  
{
  tbGlobalFree(lp->SubStream.ghHeader);      
  lp->SubStream.ghHeader = 0;
}
if (lp->SubStream.ghFooter)  
{
  tbGlobalFree(lp->SubStream.ghFooter);      
  lp->SubStream.ghFooter = 0;
}
if (lp->SubStream.ghSorts)
{  
  LPySORT lpSort = (LPySORT)tbGlobalLock(lp->SubStream.ghSorts);
  int i;
  for (i=0; i<(int)lp->SubStream.dwSortCount; i++)
  {
    if (lpSort[i].ghKey1)
      tbGlobalFree(lpSort[i].ghKey1);
    if (lpSort[i].ghKey2)
      tbGlobalFree(lpSort[i].ghKey2);
    if (lpSort[i].ghKey3)
      tbGlobalFree(lpSort[i].ghKey3);
  }  
  tbGlobalUnlock(lp->SubStream.ghSorts);
  tbGlobalFree(lp->SubStream.ghSorts);
  lp->SubStream.ghSorts = 0;
  lp->SubStream.dwSortCount = 0;
} 
if (lp->SubStream.ghRows)
{
  tbGlobalFree(lp->SubStream.ghRows);      
  lp->SubStream.ghRows = 0;
}
if (lp->SubStream.ghCols)
{
  tbGlobalFree(lp->SubStream.ghCols);      
  lp->SubStream.ghCols = 0;
}
if (lp->SubStream.ghDBCells)
{
  LPyDBCELL lpDBCell = (LPyDBCELL)tbGlobalLock(lp->SubStream.ghDBCells);
  int i;

  for(i=0; i<(int)lp->SubStream.dwDBCellCount; i++)
  {
    if (lpDBCell->ghStreamOffsets)
      tbGlobalFree(lpDBCell->ghStreamOffsets);
  }  
  tbGlobalUnlock(lp->SubStream.ghDBCells);
  tbGlobalFree(lp->SubStream.ghDBCells);      
  lp->SubStream.ghDBCells = 0;
  lp->SubStream.dwDBCellCount = 0;
}
if (lp->SubStream.ghCells)
{
  LPyCELLVAL lpCell = (LPyCELLVAL)tbGlobalLock(lp->SubStream.ghCells);
  int i;

  for (i=0; i<(int)lp->SubStream.dwCellCount; i++)
  { 
    switch(lpCell->wCellType)
    {
      case xlFORMULA:
      {
        LPyyFORMULA lp = (LPyyFORMULA)tbGlobalLock(lpCell->ghCell);
        if (lp->ghFormula)
          tbGlobalFree(lp->ghFormula);
        tbGlobalUnlock(lpCell->ghCell);
        tbGlobalFree(lpCell->ghCell);
      }
      break;
      case xlFORMULA2:
      break;
      case xlLABEL:
      {
        LPyyLABEL lp = (LPyyLABEL)tbGlobalLock(lpCell->ghCell);
        if (lp->ghString)
          tbGlobalFree(lp->ghString);
        tbGlobalUnlock(lpCell->ghCell);
        tbGlobalFree(lpCell->ghCell);
      }
      break;
    }
  }
  tbGlobalUnlock(lp->SubStream.ghCells);
  tbGlobalFree(lp->SubStream.ghCells);
  lp->SubStream.ghCells = 0;
  lp->SubStream.dwCellCount = 0;
} 
if (lp->SubStream.ghMulCells)
{
  tbGlobalFree(lp->SubStream.ghMulCells);      
  lp->SubStream.ghMulCells = 0;
  lp->SubStream.dwMulCellCount = 0;
}
if (lp->SubStream.ghTXOs)
{
  tbGlobalFree(lp->SubStream.ghTXOs);
  lp->SubStream.ghTXOs = 0;
  lp->SubStream.dwTXOLen = 0;
}
if (lp->SubStream.ghOBJs)
{
  tbGlobalFree(lp->SubStream.ghOBJs);
  lp->SubStream.ghOBJs = 0;
  lp->SubStream.dwOBJCount = 0;
}
memset(&lp->SubStream, 0, sizeof(SUBSTREAM));

//Shared formula stuff is treated as sheet data because of the way it is
//stored in the BIFF file.  Its position in the stream is important to how
//it is processed.  However, this needs to be revisited to see if it can be
//added to the sheet section.
if (lp->ShrFmlaCell.ghCell)
{
  tbGlobalFree(lp->ShrFmlaCell.ghCell);
  lp->ShrFmlaCell.ghCell = 0;
}

if (lp->ghShrFmla)
{
  LPyySHRFMLA lpf = (LPyySHRFMLA)tbGlobalLock(lp->ghShrFmla);
  int i;
  for (i=0; i<(int)lp->dwShrFmlaCount; i++)
  {
    if (lpf[i].ghFmla)
      tbGlobalFree(lpf[i].ghFmla);
  }
  tbGlobalUnlock(lp->ghShrFmla);
  tbGlobalFree(lp->ghShrFmla);
  lp->ghShrFmla = 0;
  lp->dwShrFmlaCount = 0;
}

return FALSE;
}

//--------------------------------------------------------------------
//
//  The SS_FreeXLWBMemory() function is used to free the memory allocated
//  for an Excel workbook. The memory for the loaded sheet is freed first.

BOOL SS_FreeXLWBMemory(LPVOID lpXL)
{
LPSSXL lp = lpXL;
DWORD  dw = 0;

if (lp == NULL)
  return FALSE;

// Free the memory for the sheet.
SS_FreeXLWSMemory(lp);

if (lp->ghXLFileName)
{
  tbGlobalFree(lp->ghXLFileName);
  lp->ghXLFileName = 0;
}
if (lp->ghXLLogFileName)
{
  tbGlobalFree(lp->ghXLLogFileName);
  lp->ghXLLogFileName = 0;
}
if (lp->sst.ghStrings)
{
  tbGlobalFree(lp->sst.ghStrings);
  lp->sst.ghStrings = 0;
}
if (lp->extsst.ghSSTinf)
{
  tbGlobalFree(lp->extsst.ghSSTinf);
  lp->extsst.ghSSTinf = 0;
}
if (lp->ghCodeName)
{
  tbGlobalFree(lp->ghCodeName);
  lp->ghCodeName = 0;
}
if (lp->ghWriteAccess)
{
  tbGlobalFree(lp->ghWriteAccess);
  lp->ghWriteAccess = 0;
}
if (lp->ghFonts)
{
  LPxFONT lpFont = (LPxFONT)tbGlobalLock(lp->ghFonts);
  for (dw = 0; dw<lp->dwFontCount; dw++)
  {
    if (lpFont[dw].ghFontName)
      tbGlobalFree(lpFont[dw].ghFontName);
  }
  tbGlobalFree(lp->ghFonts);
  lp->ghFonts = 0;
}
if (lp->ghFormats)
{
  LPxFORMAT lpFormat = (LPxFORMAT)tbGlobalLock(lp->ghFormats);
  for (dw = 0; dw<lp->dwFormatCount; dw++)
  {
    if (lpFormat[dw].ghFormat)
      tbGlobalFree(lpFormat[dw].ghFormat);
  }
  tbGlobalFree(lp->ghFormats);
  lp->ghFormats = 0;
}

if (lp->ghXFs)
{
  tbGlobalFree(lp->ghXFs);
  lp->ghXFs = 0;
}
if (lp->ghStyles)
{
  tbGlobalFree(lp->ghStyles);
  lp->ghStyles = 0;
}
if (lp->ghBundleShts)
{
  int i;
  LPxBUNDLESHT8 lpBS = (LPxBUNDLESHT8)tbGlobalLock(lp->ghBundleShts);
  for (i=0; i<(int)lp->dwBundleShtCount; i++)
  {
    if (lpBS[i].ghSheetName)
      tbGlobalFree(lpBS[i].ghSheetName);
  }
  tbGlobalUnlock(lp->ghBundleShts);
  tbGlobalFree(lp->ghBundleShts);
  lp->ghBundleShts = 0;
}

#if SS_V80ROBBY
if (lp->ghExternNames)
{
  LPxEXTERNNAME lpN = (LPxEXTERNNAME)tbGlobalLock(lp->ghExternNames);
  int i;
  for (i=0; i<(int)lp->dwExternNameCount; i++)
  {
    if (lpN[i].ghName)
      tbGlobalFree(lpN[i].ghName);
    if (lpN[i].ghDefinition)
      tbGlobalFree(lpN[i].ghDefinition);
  }
  tbGlobalUnlock(lp->ghExternNames);
  tbGlobalFree(lp->ghExternNames);
  lp->ghExternNames = 0;
}
#endif

if (lp->ghNames)
{
  LPxNAME lpN = (LPxNAME)tbGlobalLock(lp->ghNames);
  int i;
  for (i=0; i<(int)lp->dwNameCount; i++)
  {
    if (lpN[i].ghName)
      tbGlobalFree(lpN[i].ghName);
    if (lpN[i].ghNameDef)
      tbGlobalFree(lpN[i].ghNameDef);
    if (lpN[i].ghMenuText)
      tbGlobalFree(lpN[i].ghMenuText);
    if (lpN[i].ghDescription)
      tbGlobalFree(lpN[i].ghDescription);
    if (lpN[i].ghHelptopic)
      tbGlobalFree(lpN[i].ghHelptopic);
    if (lpN[i].ghStatusBarText)
      tbGlobalFree(lpN[i].ghStatusBarText);
  }
  tbGlobalUnlock(lp->ghNames);
  tbGlobalFree(lp->ghNames);
  lp->ghNames = 0;
}

if (lp->ghDefaultLogFont)
{
  tbGlobalFree(lp->ghDefaultLogFont);
  lp->ghDefaultLogFont = 0;
}

#ifdef SS_V70
if (lp->ghSUPBOOKData)
{
  tbGlobalFree(lp->ghSUPBOOKData);
  lp->ghSUPBOOKData = 0;
  lp->dwSUPBOOKCount = 0;
}

if (lp->ghEXTERNSHEETs)
{
  tbGlobalFree(lp->ghEXTERNSHEETs);
  lp->ghEXTERNSHEETs = 0;
  lp->dwEXTERNSHEETCount = 0;
}
#endif

return FALSE;
}

//--------------------------------------------------------------------
//
//  The SSGetExcelSheetList() function calls SS_GetExcelSheetList to 
//  retrieve a list of sheets in an Excel workbook.

BOOL DLLENTRY SSGetExcelSheetList(HWND hWnd, LPCTSTR lpszFileName, GLOBALHANDLE FAR *lpghList, LPSHORT lpnListCount, LPCTSTR lpszLogFileName, LPSHORT lpnWorkbookId, BOOL fReplace)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      fRet = FALSE;

fRet = SS_GetExcelSheetList(lpBook, lpszFileName, lpghList, lpnListCount, lpszLogFileName, lpnWorkbookId, fReplace);
SS_SheetUnlock(hWnd);

return fRet;
}

//--------------------------------------------------------------------
//
//  The SS_GetExcelSheetList() function is used to load an Excel File,
//  extract the "generic" workbook info, and build & return a list of sheet names
//  that exist in the workbook.

BOOL DLLENTRY SS_GetExcelSheetList(LPSS_BOOK lpBook, LPCTSTR lpszFileName, GLOBALHANDLE *lpghList, LPSHORT lpnListCount, LPCTSTR lpszLogFileName, LPSHORT lpnWorkbookId, BOOL fReplace)
{
TCHAR  acFileName[256];
LPTSTR lpszXLFileName = NULL;
LPSSXL lpExcelTable = NULL;
int    i;
BOOL   fFound = FALSE;
BOOL   fRet = FALSE;
short  ret = SSIsExcelFile(lpszFileName);

memset(acFileName, 0, 256*sizeof(TCHAR));

if (!ret || ret == 2)
  return FALSE;

if (!SS_ExcelTableAlloc())
  return FALSE;

lpExcelTable = SS_ExcelTableLock();

for (i=0; i<ExcelTable.dTableCnt; i++)
{
  if (lpExcelTable[i].ghXLFileName)
  {
    LPTSTR lp = (LPTSTR)tbGlobalLock(lpExcelTable[i].ghXLFileName);
    if (!_tcscmp(lpszFileName, lp))
    {
      if (lpszLogFileName && lstrlen(lpszLogFileName) && 
          lpExcelTable[i].ghXLLogFileName)
      {
        lp = (LPTSTR)tbGlobalLock(lpExcelTable[i].ghXLLogFileName);
        if (!_tcscmp(lpszLogFileName, lp))
        {
          tbGlobalUnlock(lpExcelTable[i].ghXLFileName);
          tbGlobalUnlock(lpExcelTable[i].ghXLLogFileName);
          *lpnWorkbookId = i;
          fFound = TRUE;
          break;
        }
      }
      else if ((!lpszLogFileName || !lstrlen(lpszLogFileName)) && !lpExcelTable[i].ghXLLogFileName)
      {
        tbGlobalUnlock(lpExcelTable[i].ghXLFileName);
        tbGlobalUnlock(lpExcelTable[i].ghXLLogFileName);
        *lpnWorkbookId = i;
        fFound = TRUE;
        break;
      }
    }
    tbGlobalUnlock(lpExcelTable[i].ghXLFileName);
  }
}

if (fFound && !fReplace)
{
  fRet = SS_GetXLSheetList(&(lpExcelTable[*lpnWorkbookId]), lpghList, lpnListCount);
  SS_ExcelTableUnlock();
  return TRUE;
}
else if (fFound && fReplace)
{
  SS_FreeXLWBMemory((LPSSXL)(&lpExcelTable[*lpnWorkbookId]));
}
else
  *lpnWorkbookId = (short)ExcelTable.dTableCnt;

/*******************************
* Perform the load & conversion
*******************************/
if (!lpszLogFileName)
  lpBook->lXLLogFile = -1;
else
  lpBook->lXLLogFile = 0;

#ifdef _DEBUG
lpBook->lXLLogFile = 0;
if (lpszLogFileName && lstrlen(lpszLogFileName))
  lstrcpy(acFileName, lpszLogFileName);
else
{
//  lstrcpy(acFileName, lpszFileName);
//  _tcscat(acFileName, _T(".log"));
}
#else
if (lpszLogFileName && lstrlen(lpszLogFileName))
  lstrcpy(acFileName, lpszLogFileName);
else
  acFileName[0] = 0;
#endif

xl_LogFile(lpBook, LOG_OPEN, 0, 0, (LPVOID)acFileName);
xl_LogFile(lpBook, LOG_LOADFILE, 0, 0, (LPVOID)lpszFileName);

fRet = xl_LoadFile(lpszFileName, &(lpExcelTable[*lpnWorkbookId]), lpBook, 0, 0, FALSE);
if (fRet != 0)
  return FALSE;
lpExcelTable[*lpnWorkbookId].ghXLFileName = tbGlobalAlloc(GHND, (lstrlen(lpszFileName)+1)*sizeof(TCHAR));
lpszXLFileName = (LPTSTR)tbGlobalLock(lpExcelTable[*lpnWorkbookId].ghXLFileName);
lstrcpy(lpszXLFileName, lpszFileName);
tbGlobalUnlock(lpExcelTable[*lpnWorkbookId].ghXLFileName);

if (acFileName[0] != 0)
	{
	lpExcelTable[*lpnWorkbookId].ghXLLogFileName = tbGlobalAlloc(GHND, (lstrlen(acFileName)+1)*sizeof(TCHAR));
	lpszXLFileName = (LPTSTR)tbGlobalLock(lpExcelTable[*lpnWorkbookId].ghXLLogFileName);
	if (lstrlen(acFileName))
	  lstrcpy(lpszXLFileName, acFileName);
	tbGlobalUnlock(lpExcelTable[*lpnWorkbookId].ghXLLogFileName);
	}
else
	lpExcelTable[*lpnWorkbookId].ghXLLogFileName = 0;

fRet = SS_GetXLSheetList(&(lpExcelTable[*lpnWorkbookId]), lpghList, lpnListCount);

xl_LogFile(lpBook, LOG_CLOSE, 0, 0, NULL);

//*******************************

if (fRet)
{
  // free the Excel conversion memory.
  SS_FreeXLWBMemory((LPSSXL)(&lpExcelTable[*lpnWorkbookId]));
  SS_ExcelTableUnlock();
  *lpnWorkbookId = 0;
  return FALSE;
}
if (!fFound || !fReplace)
  ExcelTable.dTableCnt++;

SS_ExcelTableUnlock();

return TRUE;
}

//--------------------------------------------------------------------
//
//  The SSImportExcelSheet() function calls SS_LoadXLSheet to load a
//  sheet from an Excel workbook.

BOOL DLLENTRY SSImportExcelSheet(HWND hWnd, short nWorkbookHandle, short nSheetIndex, LPCTSTR lpszSheetName)
{
//LPSPREADSHEET lpSS;
BOOL          fRet = FALSE;

LPSS_BOOK lpBook = SS_BookLock(hWnd);

SSReset(lpBook->hWnd);

fRet = SS_LoadXLSheet(lpBook, nWorkbookHandle, nSheetIndex, lpszSheetName);

SS_SheetUnlock(hWnd);

return !fRet;
}

//--------------------------------------------------------------------
//
//  The SS_LoadXLSheet() function is used to load a sheet from an Excel
//  workbook.
BOOL DLLENTRY SS_LoadXLSheet(LPSS_BOOK lpBook, short sWBHandle, short sSheetIndex, LPCTSTR lpszSheetName)
{
LPSSXL lpXL = NULL;
LPTSTR lpszFileName = NULL;
LPTSTR lpszLogFileName = NULL;
BOOL   fRet = FALSE;

#ifdef SS_V70
SS_ResetSheet(lpBook, lpBook->nSheet);
#endif

if (sSheetIndex == -1)
{
  LPSSXL       lpXL = NULL;
  GLOBALHANDLE ghList = 0;
  short        sListCount = 0;
  LPBYTE       lpb = NULL;
  short        i;
  
  if (!lpszSheetName || 0 == lstrlen(lpszSheetName))
    return TRUE;

  sSheetIndex = 0;

  lpXL = (LPSSXL)SS_ExcelTableLock();

  SS_GetXLSheetList(&(lpXL[sWBHandle]), (GLOBALHANDLE FAR *)&ghList, (LPSHORT)&sListCount);
  if (!ghList)
    return TRUE;

  lpb = (LPBYTE)GlobalLock(ghList);

  for (i=0; i<sListCount; i++)
  {
    if (!_tcscmp((LPTSTR)lpb, lpszSheetName))
    {
      break;
    }
    lpb = lpb + (lstrlen((LPTSTR)lpb) + 1)*sizeof(TCHAR);
    sSheetIndex++;    
  }
  SS_ExcelTableUnlock();
}

if (sWBHandle > ExcelTable.dTableCnt)
{
  return TRUE;
}

lpXL = (LPSSXL)SS_ExcelTableLock();

if (lpXL)
{
  lpszFileName = (LPTSTR)tbGlobalLock(lpXL[sWBHandle].ghXLFileName);
  if (!lpszFileName || 0 == lstrlen(lpszFileName))
  {
    SS_ExcelTableUnlock();
    return TRUE;
  }
}
else
  return TRUE;
  
lpszLogFileName = (LPTSTR)tbGlobalLock(lpXL[sWBHandle].ghXLLogFileName);

xl_LogFile(lpBook, LOG_OPEN, 1, sSheetIndex, (LPVOID)lpszLogFileName);
fRet = SS_XL2Spread(lpszFileName, &(lpXL[sWBHandle]), lpBook, sSheetIndex);
xl_LogFile(lpBook, LOG_CLOSE, 1, 0, NULL);

tbGlobalUnlock(lpXL[sWBHandle].ghXLFileName);
tbGlobalUnlock(lpXL[sWBHandle].ghXLLogFileName);

SS_ExcelTableUnlock();
return fRet;
}

#ifdef SS_V70
BOOL DLLENTRY SSImportExcelBook(HWND hWnd, LPCTSTR fileName, LPCTSTR logFileName)
{
  LPSS_BOOK lpBook = SS_BookLock(hWnd);
  BOOL      fRet = FALSE;
  
  // 26640 -scl
  //SS_LoadXLBook(lpBook, fileName, logFileName);
  fRet = SS_LoadXLBook(lpBook, fileName, logFileName);

  SS_BookUnlock(hWnd);

  return !fRet;
}

BOOL DLLENTRY SSExportExcelBookEx(HWND hWnd, LPCTSTR fileName, LPCTSTR logFileName, SHORT flags)
{
  LPSS_BOOK lpBook = SS_BookLock(hWnd);
  BOOL      fRet = FALSE;

  // 26640 -scl
  //SS_SaveXLFile(lpBook, fileName, NULL, logFileName, NULL, NULL, TRUE, flags);
  fRet = SS_SaveXLFile(lpBook, fileName, NULL, logFileName, NULL, NULL, TRUE, flags);

  SS_BookUnlock(hWnd);

  return !fRet;
}

BOOL DLLENTRY SSExportExcelBook(HWND hWnd, LPCTSTR fileName, LPCTSTR logFileName)
{
  LPSS_BOOK lpBook = SS_BookLock(hWnd);
  BOOL      fRet = FALSE;

  // 26640 -scl
  //SS_SaveXLFile(lpBook, fileName, NULL, logFileName, NULL, NULL, TRUE, 0);
  fRet = SS_SaveXLFile(lpBook, fileName, NULL, logFileName, NULL, NULL, TRUE, 0);

  SS_BookUnlock(hWnd);

  return !fRet;
}

//--------------------------------------------------------------------
//
//  The SS_LoadXLBook() function is used to load a sheet from an Excel
//  workbook.
BOOL DLLENTRY SS_LoadXLBook(LPSS_BOOK lpBook, LPCTSTR fileName, LPCTSTR logFileName)
{
  LPSSXL       lpXL = NULL;
  LPTSTR       lpszFileName = NULL;
  LPTSTR       lpszLogFileName = NULL;
  GLOBALHANDLE ghList = (GLOBALHANDLE)0;
  short        listCount = 0;
  short        nWorkbookId = 0;
  BOOL         fRet = FALSE;


//  SSReset(lpBook->hWnd); 15568

  fRet = SS_GetExcelSheetList(lpBook, fileName, &ghList, &listCount, logFileName, &nWorkbookId, TRUE);  
  if (!fRet)
  {
#if defined(SS_V80) && defined(XL12)
  FPCONTROL spread = {lpBook->hWnd, 0, 0, 0};
  if( SS_IsExcel2007File(&spread, fileName) )
     return SS_OpenExcel2007File(&spread, fileName, NULL, -1, -1, logFileName);
  else
#endif
    return (TRUE);
  }

  lpXL = (LPSSXL)SS_ExcelTableLock();

  if (lpXL)
  {
    lpszFileName = (LPTSTR)tbGlobalLock(lpXL[nWorkbookId].ghXLFileName);
    if (lpszFileName && *lpszFileName)
    {
      lpszLogFileName = (LPTSTR)tbGlobalLock(lpXL[nWorkbookId].ghXLLogFileName);

      xl_LogFile(lpBook, LOG_OPEN, 1, -1, (LPVOID)lpszLogFileName);
      fRet = SS_XL2Spread(lpszFileName, &(lpXL[nWorkbookId]), lpBook, -1);
      xl_LogFile(lpBook, LOG_CLOSE, 1, 0, NULL);

      tbGlobalUnlock(lpXL[nWorkbookId].ghXLFileName);
      tbGlobalUnlock(lpXL[nWorkbookId].ghXLLogFileName);
    }

    SS_ExcelTableUnlock();
  }

  // RFW - 10/24/05 - 17372
  if (ghList)
    GlobalFree(ghList);

  return fRet;
}
#endif

#ifdef SS_V80
#ifdef XL12
/*
BOOL SS_OpenExcelFile(LPFPCONTROL spread, LPCTSTR fileName)
{
  return xl_LoadExcel12File(spread, fileName);
}

BOOL SS_SaveExcelFile(LPFPCONTROL spread, LPCTSTR fileName)
{
  return xl_SaveExcel12File(spread, fileName);
}
*/
BOOL SS_OpenExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short sheet, short excelSheet, LPCTSTR logFileName)
{
  return xl_OpenExcel2007File(spread, fileName, password, sheet, excelSheet, logFileName);
}

BOOL SS_SaveExcel2007File(LPFPCONTROL spread, LPCTSTR fileName, LPCTSTR password, short flags, LPCTSTR logFileName)
{
  return xl_SaveExcel2007File(spread, fileName, password, flags, logFileName);
}

short SS_IsExcel2007File(LPFPCONTROL spread, LPCTSTR fileName)
{
  return xl_IsExcel2007File(spread, fileName);
}

/*
BOOL DLLENTRY SSSaveExcelFile(HWND hWnd, LPCTSTR lpszFileName)
{
  BOOL fRet = FALSE;
  LPSS_BOOK lpBook = SS_BookLock(hWnd);
  FPCONTROL fpControl = { lpBook->hWnd, 0, 0L, 0L };
  fRet = SS_SaveXL12File(&fpControl, lpszFileName);
  SS_BookUnlock(hWnd);
  return fRet;
}

BOOL DLLENTRY SSOpenExcelFile(HWND hWnd, LPCTSTR lpszFileName)
{
  BOOL fRet = FALSE;
  LPSS_BOOK lpBook = SS_BookLock(hWnd);
  FPCONTROL fpControl = { lpBook->hWnd, 0, 0L, 0L };
  fRet = SS_LoadXL12File(&fpControl, lpszFileName);
  SS_BookUnlock(hWnd);
  return fRet;
}
*/
BOOL DLLENTRY SSSaveExcel2007File(HWND hWnd, LPCTSTR lpszFileName, LPCTSTR lpszPassword, short nFlags, LPCTSTR lpszLogFileName)
{
  BOOL fRet = FALSE;
  LPSS_BOOK lpBook = SS_BookLock(hWnd);
  FPCONTROL fpControl = { lpBook->hWnd, 0, 0L, 0L };
  fRet = SS_SaveExcel2007File(&fpControl, lpszFileName, lpszPassword, nFlags, lpszLogFileName);
  SS_BookUnlock(hWnd);
  return fRet;
}

BOOL DLLENTRY SSOpenExcel2007File(HWND hWnd, LPCTSTR lpszFileName, LPCTSTR lpszPassword, short nSheet, short nExcelSheet, LPCTSTR lpszLogFileName)
{
  BOOL fRet;
  LPSS_BOOK lpBook;
  FPCONTROL fpControl = {0};
  if( SSIsExcelFile(lpszFileName) )
    return SSImportExcelBook(hWnd, lpszFileName, lpszLogFileName);
  fRet = FALSE;
  lpBook = SS_BookLock(hWnd);
  fpControl.hWnd = lpBook->hWnd;
  fRet = SS_OpenExcel2007File(&fpControl, lpszFileName, lpszPassword, nSheet, nExcelSheet, lpszLogFileName);
  SS_BookUnlock(hWnd);
  return fRet;
}

short DLLENTRY SSIsExcel2007File(HWND hWnd, LPCTSTR lpszFileName)
{
  short fRet;
  LPSS_BOOK lpBook = SS_BookLock(hWnd);
  FPCONTROL fpControl = { lpBook->hWnd, 0, 0L, 0L };
  fRet = SS_IsExcel2007File(&fpControl, lpszFileName);
  SS_BookUnlock(hWnd);
  return fRet;
}

BOOL DLLENTRY SSIsExcel2007Supported()
{
   return xl_IsExcel2007Supported();
}
#endif // XL12
#endif // SS_V80

#ifdef SS_V70
BOOL DLLENTRY SSExportToExcelEx(HWND hWnd, LPCTSTR lpszFileName, LPCTSTR lpszSheetName, LPCTSTR lpszLogFileName, SHORT flags)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      Ret = FALSE;

if (lpszSheetName && 0 != lstrlen(lpszSheetName))
  Ret = SS_SaveXLFile(lpBook, lpszFileName, lpszSheetName, lpszLogFileName, NULL, NULL, TRUE, flags);
else
#ifdef SS_V70
  {
    TCHAR buffer[100];
    _stprintf(buffer, _T("Sheet%d"), lpBook->nSheet+1);
    Ret = SS_SaveXLFile(lpBook, lpszFileName, buffer, lpszLogFileName, NULL, NULL, TRUE, flags);
  }
#else
  Ret = SS_SaveXLFile(lpBook, lpszFileName, DEF_SHEETNAME, lpszLogFileName, NULL, NULL, TRUE, flags);
#endif

SS_BookUnlock(hWnd);


return !Ret;
}
#endif
//--------------------------------------------------------------------
//
//  The SSExportToExcel() function calls SS_SaveXLFile to write the Excel
//  data to a file.

BOOL DLLENTRY SSExportToExcel(HWND hWnd, LPCTSTR lpszFileName, LPCTSTR lpszSheetName, LPCTSTR lpszLogFileName)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      Ret = FALSE;

if (lpszSheetName && 0 != lstrlen(lpszSheetName))
  Ret = SS_SaveXLFile(lpBook, lpszFileName, lpszSheetName, lpszLogFileName, NULL, NULL, TRUE, 0);
else
  Ret = SS_SaveXLFile(lpBook, lpszFileName, DEF_SHEETNAME, lpszLogFileName, NULL, NULL, TRUE, 0);

SS_BookUnlock(hWnd);

return !Ret;
}

//--------------------------------------------------------------------
//
//  The SS_SaveXLFile() function is used to create a Structured Storage
//  Model (SSM) file, create an Excel workbook stream, build an Excel sheet
//  and add it to the file.

BOOL SS_SaveXLFile(LPSS_BOOK lpBook, LPCTSTR lpszFileName, LPCTSTR lpszSheetName, LPCTSTR lpszLogFileName, LPTBGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen, BOOL fToFile, SHORT sFlags)
{
BOOL fRet = FALSE;
TCHAR acFileName[500]; 
TCHAR acLogFileName[500];

memset(acFileName, 0, 500*sizeof(TCHAR));
memset(acLogFileName, 0, 500*sizeof(TCHAR));

if (lpszFileName && 0 != lstrlen(lpszFileName))
{
  long lFileNameLen = lstrlen(lpszFileName);
  lstrcpy(acFileName, lpszFileName);      
}
else if (fToFile)
  return TRUE;

#ifdef _DEBUG
if (lpszLogFileName && lstrlen(lpszLogFileName))
  lstrcpy(acLogFileName, lpszLogFileName);
else if (lstrlen(lpszFileName))
{
  lstrcpy(acLogFileName, lpszFileName);
  _tcscat(acLogFileName, _T(".log"));
}
else
  acLogFileName[0] = 0;
#else
if (lpszLogFileName && lstrlen(lpszLogFileName))
  lstrcpy(acLogFileName, lpszLogFileName);
else
  acLogFileName[0] = 0;
#endif

lpBook->lXLLogFile = -1;
// xl_LogFile(lpBook, LOG_OPEN, 0, 1, (LPVOID)acLogFileName);
// xl_LogFile(lpBook, LOG_SAVEFILE, 0, 0, (LPVOID)acFileName);
//Modify By BOC 99.4.27  (hyt)---------------------------
//for always return true
//xl_SaveFile(acFileName, lpszSheetName, lpSS, lpghBuffer, lplBufferLen, fToFile);

fRet = xl_SaveFile(acFileName, lpszSheetName, lpBook, lpghBuffer, lplBufferLen, fToFile, sFlags);
//----------------------------------------------------------
//xl_LogFile(lpBook, LOG_CLOSE, 2, 0, NULL);

return fRet;
}

#ifdef SS_V70
extern short SSTab_GetSheetDisplayName(LPSS_BOOK lpBook, short nSheet, LPTSTR lpszName, int nLen);

BOOL SSGetSheetDisplayName(HWND hWnd, short nSheet, LPTBGLOBALHANDLE lpgh)
{
  LPSS_BOOK      lpBook = SS_BookLock(hWnd);
  BOOL           bRet = FALSE;
  LPTSTR         name = NULL;
  
  *lpgh = tbGlobalAlloc(GHND, 100);
  name = (LPTSTR)tbGlobalLock(*lpgh);
  
  SSTab_GetSheetDisplayName(lpBook, nSheet, name, -1);

  tbGlobalUnlock(*lpgh);
  
  return bRet;
}
#endif // SS_V70

BOOL DLLENTRY SSGetBiffBuffer(HWND hWnd, LPGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen)
{
LPSS_BOOK lpBook = SS_BookLock(hWnd);
BOOL      bRet = FALSE;

bRet = SS_RetrieveBiffBuffer(lpBook, NULL, NULL, lpghBuffer, lplBufferLen);

SS_BookUnlock(hWnd);
return bRet;
}

//--------------------------------------------------------------------
//
//  The SSRetrieveBiffBuffer() function calls SS_SaveXLFile to write the Excel
//  data to a file. This function is used from the Excel File Conversion DLL. It 
//  is doubtful that it would need to be called from anywhere else.

BOOL SS_RetrieveBiffBuffer(LPSS_BOOK lpBook, LPCTSTR lpszSheetName, LPCTSTR lpszLogFileName, LPGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen)
{
BOOL           Ret = FALSE;
TBGLOBALHANDLE tbghBuffer = 0;
long           ltbBufferLen = 0;
LPVOID         lp = NULL;
LPVOID         lptb = NULL;

if (lpszSheetName && lstrlen(lpszSheetName))
  Ret = SS_SaveXLFile(lpBook, NULL, lpszSheetName, lpszLogFileName, (LPTBGLOBALHANDLE)&tbghBuffer, (LPLONG)&ltbBufferLen, FALSE, 0);
else
  Ret = SS_SaveXLFile(lpBook, NULL, DEF_SHEETNAME, lpszLogFileName, (LPTBGLOBALHANDLE)&tbghBuffer, (LPLONG)&ltbBufferLen, FALSE, 0);

if (ltbBufferLen)
{
  *lpghBuffer = GlobalAlloc(GHND, ltbBufferLen);
  lp = (LPVOID)GlobalLock(*lpghBuffer);
  lptb = (LPVOID)tbGlobalLock(tbghBuffer);

  _fmemcpy(lp, lptb, ltbBufferLen);
  *lplBufferLen = ltbBufferLen;
  
  tbGlobalUnlock(tbghBuffer);
  tbGlobalFree(tbghBuffer);
  GlobalUnlock(*lpghBuffer);
}

return !Ret;
}


BOOL SS_SetSpreadFile2BiffBuffer(LPTSTR lpszFileName, LPGLOBALHANDLE lpghBuffer, LPLONG lplBufferLen)
{
  // Create the Spread window
  HWND hWndSS = CreateWindow(SS_CLASSNAME,
                             NULL,
                             0, //WS_VISIBLE,
                             0, 
                             0, 
                             300, 
                             200, 
                             HWND_DESKTOP,
                             NULL, 
                             fpInstance, 
                             NULL);

  SSLoadFromFile(hWndSS, lpszFileName);

  SSGetBiffBuffer(hWndSS, lpghBuffer, lplBufferLen);

  SendMessage(hWndSS, WM_CLOSE, 0, 0);

  return TRUE;
}

//--------------------------------------------------------------------
//
//  Allocate a Global Handle for an Excel file.  Create the Global Table 
//  if necessary.

BOOL SS_ExcelTableAlloc(void)
{
if (ExcelTable.dTableCnt >= ExcelTable.dTableAllocCnt)
   {
   TBGLOBALHANDLE hTableOrig = ExcelTable.hTable;

   ExcelTable.hTable = SSx_AllocList(ExcelTable.hTable, &ExcelTable.dTableAllocCnt,
                                     sizeof(SSXL));

#ifdef WIN32
   if (!hTableOrig && ExcelTable.hTable)
      InitializeCriticalSection(&ExcelTable.cs);
   else if (hTableOrig && !ExcelTable.hTable)
      DeleteCriticalSection(&ExcelTable.cs);
#endif

   if (!ExcelTable.hTable)
      {
      ExcelTable.dTableCnt = 0;
      ExcelTable.dTableAllocCnt = 0;
      }

   return (ExcelTable.hTable != 0);
   }

return (TRUE);
}

//--------------------------------------------------------------------
//
//  Delete the Global Excel File Handle Table.

void SS_ExcelTableDelete(void)
{
LPSSXL lpExcelTable;
short  i;

if (ExcelTable.hTable && ExcelTable.dTableCnt)
   {
   lpExcelTable = (LPSSXL)tbGlobalLock(ExcelTable.hTable);

   for (i = 0; i < ExcelTable.dTableCnt; i++)
     SS_FreeXLWBMemory(&(lpExcelTable[i]));
   tbGlobalUnlock(ExcelTable.hTable);
   tbGlobalFree(ExcelTable.hTable);
#ifdef WIN32
   DeleteCriticalSection(&ExcelTable.cs);
#endif
   }

ExcelTable.hTable = 0;
ExcelTable.dTableCnt = 0;
ExcelTable.dTableAllocCnt = 0;
}


//--------------------------------------------------------------------
//
//  Lock the Global Excel File Handle Table.  Enter the Critical Section
//  so that any other app attempting to acces this Global Table will have
//  to wait until we leave the Critical Section.

LPSSXL SS_ExcelTableLock(void)
{
if (!ExcelTable.hTable)
   return (NULL);

#ifdef WIN32
EnterCriticalSection(&ExcelTable.cs);
#endif

return ((LPSSXL)tbGlobalLock(ExcelTable.hTable));
}


//--------------------------------------------------------------------
//
//  Unlock the Global Excel File Handle Table.  Leave the Critical Section
//  so any app can acces this Global Table.

void SS_ExcelTableUnlock(void)
{
if (ExcelTable.hTable)
   {
#ifdef WIN32
   LeaveCriticalSection(&ExcelTable.cs);
#endif

   tbGlobalUnlock(ExcelTable.hTable);
   }
}

#endif
