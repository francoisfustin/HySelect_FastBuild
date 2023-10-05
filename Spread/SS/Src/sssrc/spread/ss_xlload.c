/*********************************************************
* SS_XLLOAD.C
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
*	RAP	- 12.10.98
* RAP01-SCS8650-Cells were not being imported with the correct lock setting. 6.21.1999
* RAP02-CHB7093 - Retain Selblock                                            9.12.2000
* RAP03-TIB7316                                                              10.19.00 
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

#if !defined(SS_NOCALC) && !defined(SS_OLDCALC)
#include "..\calc\calc.h"
#include "..\calc\cal_cell.h"
#include "..\calc\cal_col.h"
#include "..\calc\cal_cust.h"
#include "..\calc\cal_dde.h"
#include "..\calc\cal_expr.h"
#include "..\calc\cal_mem.h"
#include "..\calc\cal_name.h"
#include "..\calc\cal_oper.h"
#include "..\calc\cal_row.h"
#endif
#include "..\classes\wintools.h"
#include "..\classes\checkbox.h"

#ifdef SS_V70
#include "ss_book.h"
#endif

#ifdef XL_DLL
extern HANDLE hDynamicInst;
#endif
//--------------------------------------------------------------------

BOOL xl_SetGeneralProps(LPSSXL lpssXL, LPSS_BOOK lpBook);
BOOL xl_SetCalcProps(LPSSXL lpssXL, LPSS_BOOK lpBook);

BOOL SS_XL2Spread(LPCTSTR lpszFileName, LPVOID lp, LPSS_BOOK lpBook, short sSheetNum)
{
  LPSSXL lpssXL = (LPSSXL)lp;
  BOOL   fRedraw = FALSE;    
  BOOL   Ret = FALSE;

  if ((int)sSheetNum > (int)lpssXL->dwSubStreamCount-1)
    return TRUE;

#ifdef SS_V70
  if (sSheetNum == -1)
    SS_Reset(lpBook);
  else
    SS_ResetSheet(lpBook, lpBook->nSheet);
#else
  SSDeSelectBlock(lpBook->hWnd);
  SSReset(lpBook->hWnd);
#endif 

  SSSetBool(lpBook->hWnd, SSB_AUTOCALC, FALSE);

  fRedraw = lpBook->Redraw;
  if (fRedraw)
    lpBook->Redraw = FALSE;
  
#ifdef SS_V70
  {
    LPxBUNDLESHT8 lpSheets = (LPxBUNDLESHT8)tbGlobalLock(lpssXL->ghBundleShts);
    if (sSheetNum == -1)
    {
      int i;

      SS_SetSheetCount(lpBook, (short)lpssXL->dwSubStreamCount);
      for (i=0; i<(int)lpssXL->dwSubStreamCount; i++)
      {
        LPCTSTR sheetName = (LPCTSTR)tbGlobalLock(lpSheets[i].ghSheetName);
        SS_SetSheetName(lpBook, (short)i, sheetName);
        if (lpSheets[i].hsState != 0)
          SS_SetSheetVisible(lpBook, (short)i, FALSE);
        tbGlobalUnlock(lpSheets[i].ghSheetName);
      }
      tbGlobalUnlock(lpssXL->ghBundleShts);
#ifdef SS_V70
		SS_SetActiveSheetIndex(lpBook, lpssXL->wb.nSelTabIndex);
      lpBook->nTabStripLeftSheet = lpssXL->wb.nFirstTabIndex;
#endif
    }
    else
    {
      LPCTSTR sheetName = (LPCTSTR)tbGlobalLock(lpSheets[sSheetNum].ghSheetName);
      SS_SetSheetName(lpBook, (short)lpBook->nSheet, sheetName);
      if (lpSheets[sSheetNum].hsState != 0)
        SS_SetSheetVisible(lpBook, (short)sSheetNum, FALSE);
      tbGlobalUnlock(lpSheets[sSheetNum].ghSheetName);
      SS_SetActiveSheetIndex(lpBook, 0);
    }
  }
#endif

  SS_FreeXLWSMemory(lpssXL);

  //SetProtect for SpreadSheet Workbook
#ifdef SS_V70
  lpBook->DocumentProtected = FALSE;
#else
  SSSetBool(lpBook->hWnd, SSB_PROTECT, FALSE);
#endif

  lpssXL->xlSS_HEADER = SS_HEADER; 
  Ret = xl_LoadFile(lpszFileName, (LPSSXL)lpssXL, lpBook, (int)sSheetNum, 1, FALSE);

  xl_SetGeneralProps(lpssXL, lpBook);

  if (fRedraw)
    lpBook->Redraw = TRUE;

//#ifndef SS_V70
  xl_SetCalcProps(lpssXL, lpBook);
//#endif

  return Ret;
}

BOOL SS_GetXLSheetList(LPVOID lp, GLOBALHANDLE FAR *lpghList, LPSHORT lpsCount)
{
  LPSSXL lpssXL = (LPSSXL)lp;
  LPxBUNDLESHT8 lpbs = (LPxBUNDLESHT8)tbGlobalLock(lpssXL->ghBundleShts);
  LPTSTR lptstr;
  int    i;
  long   lLen = 0;
  long   lPos = 0;
  BOOL   Ret = FALSE;

  // Go thru the list of bundlesheets and sum the length of the sheetnames.
  *lpsCount = (short)lpssXL->dwBundleShtCount; 
  for (i=0; i<*lpsCount; i++)
  {
    lLen += (lpbs[i].wSheetNameLen+1) * sizeof(TCHAR);
  }
  // Allocate the sheetlist buffer to return to the caller.
  *lpghList = GlobalAlloc(GHND, lLen);
  lptstr = (LPTSTR)GlobalLock(*lpghList);

  for (i=0; i<*lpsCount; i++)
  {
    LPSTR lpstrSN = (LPSTR)tbGlobalLock(lpbs[i].ghSheetName);

//#ifdef _UNICODE
//    // Convert the multibyte string to a Unicode string.
//    TBGLOBALHANDLE gh = 0;
//    LPTSTR lptstrSN = NULL;
//
//    gh = tbGlobalAlloc(GHND, (lpbs[i].wSheetNameLen+1)*sizeof(TCHAR));
//    lptstrSN = (LPTSTR)tbGlobalLock(gh);
//
//    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpstrSN, -1,
//                        lptstrSN, (lpbs[i].wSheetNameLen+1)*sizeof(TCHAR));
//
//    memcpy((LPBYTE)lptstr+lPos, lptstrSN, (lpbs[i].wSheetNameLen+1)*sizeof(TCHAR));
//    
//    tbGlobalUnlock(lpbs[i].ghSheetName);
//    tbGlobalFree(lpbs[i].ghSheetName);
//    lpbs[i].ghSheetName = gh;
//#else
    memcpy((LPBYTE)lptstr+lPos, lpstrSN, (lpbs[i].wSheetNameLen+1)*sizeof(TCHAR));
    tbGlobalUnlock(lpbs[i].ghSheetName);
//#endif          

    lPos += (lpbs[i].wSheetNameLen+1)*sizeof(TCHAR);  
  }
  
  GlobalUnlock(*lpghList);
  tbGlobalUnlock(lpssXL->ghBundleShts);

  return Ret;
}

BOOL xl_SetGeneralProps(LPSSXL lpssXL, LPSS_BOOK lpBook)
{
  HKEY        hKey1;
  HKEY        hKey2;
  HKEY        hKey3;
  HKEY        hKey4;
  HKEY        hKey5;
  HKEY        hKey6;
  DWORD       dwMoveEnter=-1;
  DWORD       dwOptions3=-1;
  DWORD       dwOptions5=-1;
  long        lSize = sizeof(DWORD);
  DWORD       dwType;
  HRESULT     hr;
  COLORREF    color = (COLORREF)RGBCOLOR_DEFAULT;
#ifndef SS_V70
  SS_CELLTYPE ct;
#endif
  long        lMask = 0;
  BOOL        Ret = FALSE;

  //SetProtect for SpreadSheet Workbook
#ifdef SS_V70
//14220  lpBook->DocumentProtected = lpssXL->bProtect;
  lpBook->nSheet = 0;
  SSSetTabStripPolicy(lpBook->hWnd, SS_TABSTRIPPOLICY_ALWAYS);
#else
  SSSetBool(lpBook->hWnd, SSB_PROTECT, lpssXL->SubStream.bProtect);
#endif

  //All cells are LOCKED by default in Excel
//RAP01d  SSSetLock(lpBook->hWnd, SS_ALLCOLS, SS_ALLROWS, TRUE);

#ifndef SS_V70
  //SetCurrentSelection
  SSSetActiveCell(lpBook->hWnd, lpssXL->SubStream.sel.wCol+1, lpssXL->SubStream.sel.wRow+1);  
  //Set Multi-Block Selection to ALL TRUE.
  SSSetBool(lpBook->hWnd, SSB_ALLOWMULTIBLOCKS, TRUE);
  SSSetSelBlockOptions(lpBook->hWnd, SS_SELBLOCK_COLS | 
                                   SS_SELBLOCK_ROWS | 
                                   SS_SELBLOCK_BLOCKS | 
                                   SS_SELBLOCK_ALL);
//Modify by BOC 99.8.25 (hyt)------------------------------
//for not set selblock if just select one block in excel file
  //if (lpssXL->SubStream.sel.wRefs > 1)
  if (lpssXL->SubStream.sel.wRefs > 0)
//---------------------------------------------------------
  {
    LPyyREF       lpRef = (LPyyREF)tbGlobalLock(lpssXL->SubStream.sel.ghRefs);
    WORD          w;
    TBGLOBALHANDLE  gh = tbGlobalAlloc(GHND, lpssXL->SubStream.sel.wRefs * sizeof(SS_SELBLOCK));
    LPSS_SELBLOCK lpSel = (LPSS_SELBLOCK)tbGlobalLock(gh);
    for (w=0; w<lpssXL->SubStream.sel.wRefs; w++)
    {
      if (lpRef[w].bCol1 == 0xFF)
        lpSel[w].UL.Col = -1;
      else
        lpSel[w].UL.Col = lpRef[w].bCol1+1;

      if (lpRef[w].bCol2 == 0xFF)
        lpSel[w].LR.Col = -1;
      else
        lpSel[w].LR.Col = lpRef[w].bCol2+1;

      if (lpRef[w].wRow1 == 0x3FFF)
        lpSel[w].UL.Row = -1;
      else
        lpSel[w].UL.Row = lpRef[w].wRow1+1;

      if (lpRef[w].wRow2 == 0x3FFF)
        lpSel[w].LR.Row = -1;
      else
        lpSel[w].LR.Row = lpRef[w].wRow2+1;
    }
    tbGlobalUnlock(lpssXL->SubStream.sel.ghRefs);

    SSSetMultiSelBlocks(lpBook->hWnd, lpSel, lpssXL->SubStream.sel.wRefs);
    tbGlobalUnlock(gh);
    tbGlobalFree(gh);
  }
  else  if (lpssXL->SubStream.sel.wRefs)
  {
    SS_CELLCOORD sscell;
    sscell.Col = lpssXL->SubStream.sel.wCol+1;
    sscell.Row = lpssXL->SubStream.sel.wRow+1;

    SSSetSelectBlock(lpBook->hWnd, (LPSS_CELLCOORD)&sscell, (LPSS_CELLCOORD)&sscell);  
  }

  SSSetBool(lpBook->hWnd, SSB_RETAINSELBLOCK, TRUE);  //RAP02a

  SSSetBool(lpBook->hWnd, SSB_RETAINSELBLOCK, TRUE);  //RAP02a

  //ShowScrollbars
  SSSetBool(lpBook->hWnd, SSB_HORZSCROLLBAR, (BOOL)lpssXL->wb.fDspHScroll);
  SSSetBool(lpBook->hWnd, SSB_VERTSCROLLBAR, (BOOL)lpssXL->wb.fDspVScroll);
#endif

  //Query the Registry to obtain property values.
  //!!!These properties are written to the Registry when Excel exits.
  //!!!Changing these property values and saving the Excel spreadsheet to a
  //file will NOT save off these values to the Registry.
  xl_LogFile(lpBook, LOG_REGISTRYPROPS, 0,0, NULL);
  hr = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software"), 0, KEY_READ, (HKEY FAR *)&hKey1);
  if (S_OK == hr)
  {
    hr = RegOpenKeyEx(hKey1, _T("Microsoft"), 0, KEY_READ, (HKEY FAR *)&hKey2);
    if (S_OK == hr)
    {
      hr = RegOpenKeyEx(hKey2, _T("Office"), 0, KEY_READ, (HKEY FAR *)&hKey3);
      if (S_OK == hr)
      {
		    if (lpssXL->bof.wBuild == 0x0DBB) // Excel 97
        {
 			    hr = RegOpenKeyEx(hKey3, _T("8.0"), 0, KEY_READ, (HKEY FAR *)&hKey4);	//Excel 97
        }
		    else if (lpssXL->bof.wBuild == 0x18AF) // Excel 2000
        {
          hr = RegOpenKeyEx(hKey3, _T("9.0"), 0, KEY_READ, (HKEY FAR *)&hKey4);	//Excel 2000
        }
		    else if (lpssXL->bof.wBuild == 0x1917) // Excel 2000 - Japan
        {
          hr = RegOpenKeyEx(hKey3, _T("9.0"), 0, KEY_READ, (HKEY FAR *)&hKey4);	//Excel 2000 - Japan
        }
		    else if (lpssXL->bof.wBuild == 0x1846) // Excel XP
        {
          hr = RegOpenKeyEx(hKey3, _T("10.0"), 0, KEY_READ, (HKEY FAR *)&hKey4);	//Excel XP
        }
        else
        {
          hr = S_FALSE;
        }

        if(hr != S_OK)
          hr = RegOpenKeyEx(hKey3, _T("10.0"), 0, KEY_READ, (HKEY FAR *)&hKey4);//Excel XP
        if(hr != S_OK)
          hr = RegOpenKeyEx(hKey3, _T("9.0"), 0, KEY_READ, (HKEY FAR *)&hKey4);	//Excel 2000
        if(hr != S_OK)
			    hr = RegOpenKeyEx(hKey3, _T("8.0"), 0, KEY_READ, (HKEY FAR *)&hKey4);	//Excel 97
        if (S_OK == hr)
        {
          hr = RegOpenKeyEx(hKey4, _T("Excel"), 0, KEY_READ, (HKEY FAR *)&hKey5);
          if (S_OK == hr)
          {
      			hr = RegOpenKeyEx(hKey5, _T("Options"), 0, KEY_READ, (HKEY FAR *)&hKey6);	//Excel 2000
	      		if(hr!= S_OK)
		  	    	hr = RegOpenKeyEx(hKey5, _T("Microsoft Excel"), 0, KEY_READ, (HKEY FAR *)&hKey6);	//Excel 97
            if (S_OK == hr)
            {
              hr = RegQueryValueEx(hKey6, _T("MoveEnterDir"), (LPDWORD)NULL, 
                                   (LPDWORD)&dwType, (LPBYTE)&dwMoveEnter,
                                   (LPDWORD)&lSize);
              hr = RegQueryValueEx(hKey6, _T("Options3"), (LPDWORD)NULL, 
                                   (LPDWORD)&dwType, (LPBYTE)&dwOptions3,
                                   (LPDWORD)&lSize);
              hr = RegQueryValueEx(hKey6, _T("Options5"), (LPDWORD)NULL, 
                                   (LPDWORD)&dwType, (LPBYTE)&dwOptions5,
                                   (LPDWORD)&lSize);
              hr = RegCloseKey(hKey6);
            }  
            hr = RegCloseKey(hKey5);
          }
          hr = RegCloseKey(hKey4);
        }
        hr = RegCloseKey(hKey3);
      }
      hr = RegCloseKey(hKey2);
    }
    hr = RegCloseKey(hKey1);
  }

  //SetDragDrop
  if (dwOptions3 == -1L || !(dwOptions3 & 0x00000010))
    SSSetBool(lpBook->hWnd, SSB_ALLOWDRAGDROP, TRUE);
  else
    SSSetBool(lpBook->hWnd, SSB_ALLOWDRAGDROP, FALSE);

  //SetEditEnterAction
  if (dwOptions5 == -1L || dwOptions5 & 0x00000800)
    SSSetEditEnterAction(lpBook->hWnd, SS_ENTERACTION_NONE);
  else if (dwMoveEnter == 0)
    SSSetEditEnterAction(lpBook->hWnd, SS_ENTERACTION_DOWN);
  else if (dwMoveEnter == 1)
    SSSetEditEnterAction(lpBook->hWnd, SS_ENTERACTION_RIGHT);
  else if (dwMoveEnter == 2)
    SSSetEditEnterAction(lpBook->hWnd, SS_ENTERACTION_UP);
  else if (dwMoveEnter == 3)
    SSSetEditEnterAction(lpBook->hWnd, SS_ENTERACTION_LEFT);

  //SetAutoClipboard
  if (dwOptions5 == -1L || dwOptions5 & 0x00000080)
    SSSetBool(lpBook->hWnd, SSB_AUTOCLIPBOARD, TRUE);
  else
    SSSetBool(lpBook->hWnd, SSB_AUTOCLIPBOARD, FALSE);

  //SetAllowCellOverflow
  //Excel is TRUE. I do not believe it can be modified.
  SSSetBool(lpBook->hWnd, SSB_ALLOWCELLOVERFLOW, TRUE);
  SSSetBool(lpBook->hWnd, SSB_ALLOWUSERFORMULAS, TRUE);
  SSSetBool(lpBook->hWnd, SSB_ARROWSEXITEDITMODE, TRUE);

#ifndef SS_V70
  //Set Headers to be Horiz Centered and Vert Bottom
  SSGetCellType(lpBook->hWnd, SS_HEADER, 1, &ct);
  lMask = SS_TEXT_LEFT | SS_TEXT_RIGHT | SS_TEXT_CENTER |
          SS_TEXT_VCENTER | SS_TEXT_TOP | SS_TEXT_BOTTOM;
  ct.Style = ct.Style & ~lMask;
  ct.Style |= (SS_TEXT_CENTER | SS_TEXT_BOTTOM);
  SSSetCellType(lpBook->hWnd, SS_HEADER, SS_ALLROWS, &ct);
  SSSetCellType(lpBook->hWnd, SS_ALLCOLS, SS_HEADER, &ct);
#endif

#ifndef SS_V70
//  lpBook->PrintOptions.pf.fShowGrid = (lpssXL->SubStream).wPrintGrid; 
//  lpBook->PrintOptions.pf.fShowColHeaders = (lpssXL->SubStream).wPrintHeaders;
//  lpBook->PrintOptions.pf.fShowRowHeaders = (lpssXL->SubStream).wPrintHeaders;
//  lpBook->PrintOptions.pf.fDrawBorder = (lpssXL->SubStream).wPrintGrid;
//  lpBook->PrintOptions.pf.wOrientation = ((((*lpssXL).SubStream).setup)).fNoOrient? 0:(((((*lpssXL).SubStream).setup)).fLandscape? 1:2);
//  lpBook->PrintOptions.pf.fDrawColors = ((((*lpssXL).SubStream).setup)).fNoColor? 0:1; 
//#ifdef SS_V70
//  if (lpBook->PrintOptions.pf.fDrawColors = ((((*lpssXL).SubStream).setup)).fNoPls == 0)
//    lpBook->PrintOptions.pf.ZoomFactor =((((*lpssXL).SubStream).setup)).wScale/100;
//  lpBook->PrintOptions.pf.nBestFitPagesWide = ((((*lpssXL).SubStream).setup)).wFitWidth;
//  lpBook->PrintOptions.pf.nBestFitPagesTall = ((((*lpssXL).SubStream).setup)).wFitHeight;
//#endif
#endif

  return Ret;
}

BOOL xl_SetCalcProps(LPSSXL lpssXL, LPSS_BOOK lpBook)
{
  BOOL  Ret = FALSE;

  //SetMaxIterations & MaxDelta & Iteration on/off
  SSSetIteration(lpBook->hWnd, lpssXL->SubStream.wIteration, lpssXL->SubStream.wMaxIterations, lpssXL->SubStream.dblMaxDelta);
  
//18783 >>
  if (!lpssXL->SubStream.wCalcMode)
    SSSetBool(lpBook->hWnd, SSB_AUTOCALC, TRUE);
//<< 18783
  SSSetBool(lpBook->hWnd, SSB_AUTOCALC, lpssXL->SubStream.wCalcMode);


  return Ret;
}

#endif //SS_V30

