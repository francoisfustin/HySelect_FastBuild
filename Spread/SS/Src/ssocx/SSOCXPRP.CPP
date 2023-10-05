// ssocxprp.cpp : Implementation of the CSsocxCtrl OLE control properties.
//
// UPDATE LOG:	
//		RAP01	05.11.99 Removed bug fix and changed back to original due to
//                   customer complaints as requested by Greg B.    

#define NO_VBX_SUPPORT

#include "stdafx.h"
#include "ssocx.h"
#include "ssocxctl.h"
#include "ssocxppg.h"
#include "opmodepr.h"
#include "virtualp.h"
#include "displayp.h"
#include "editmode.h"
#include "headersp.h"
#include <math.h>

#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_bool.h"
#include "..\sssrc\spread\ss_bord.h"
#include "..\sssrc\spread\ss_calc.h"
#include "..\sssrc\spread\ss_cell.h"
#include "..\sssrc\spread\ss_col.h"
#include "..\sssrc\spread\ss_curs.h"
#include "..\sssrc\spread\ss_data.h"
#include "..\sssrc\spread\ss_doc.h"
#include "..\sssrc\spread\ss_draw.h"
#include "..\sssrc\spread\ss_font.h"
#include "..\sssrc\spread\ss_formu.h"
#include "..\sssrc\spread\ss_main.h"
#include "..\sssrc\spread\ss_multi.h"
#include "..\sssrc\spread\ss_row.h"
#include "..\sssrc\spread\ss_save.h"
#include "..\sssrc\spread\ss_scrol.h"
#include "..\sssrc\spread\ss_type.h"
#include "..\sssrc\spread\ss_user.h"
#include "..\sssrc\spread\ss_virt.h"
#include "..\sssrc\classes\wintools.h"
#include "..\vbsrc\vbmsg.h"
#include "..\vbsrc\vbprint2.h"

//DBOCX...
#include "ssocxdb.h"
#include "..\..\..\fplibs\fptools\src\fpconvrt.h"   // includes fpvbsub.h 

extern "C" {
#include "..\vbsrc\vbspread.h"
#include "..\vbsrc\vbsscnvt.h"
} //extern "C"
//DBOCX.

#include "..\..\..\fplibs\fptools\src\fptstr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//--------------------------------------------------------------------

#define GET_NODESIGN() \
  if( AmbientUserMode() == FALSE ) \
    GetNotSupported();

//--------------------------------------------------------------------

/*
#define SET_NODESIGN() \
  if( AmbientUserMode() == FALSE ) \
    SetNotSupported();
*/

//--------------------------------------------------------------------

#define INVALIDATE_CONTROL() \
  if( !AmbientUserMode() ) \
    InvalidateControl();

//--------------------------------------------------------------------

GLOBALHANDLE CStringToHandle(CString s)
{
  GLOBALHANDLE hRet = 0;
  LPTSTR lpszTemp;

  if( !s.IsEmpty() )
  {
    if( hRet = GlobalAlloc(GHND, (s.GetLength() + 1) * sizeof(TCHAR)) )
    {
      lpszTemp = (LPTSTR)GlobalLock(hRet);
      lstrcpy(lpszTemp, s);
      GlobalUnlock(hRet);
    }
  }
  return hRet;
}

//--------------------------------------------------------------------
//
//  The StrHugeLen() function finds the number of characters in
//  a text string.
//

#ifndef WIN32
static long StrHugeLen(const char __huge* hpStr)
{
long lLen = 0L;
if( hpStr )
{
  while (*hpStr++)
     lLen++;
}
return lLen;
}
#endif

//--------------------------------------------------------------------

#ifndef SS_NOPRINT
#ifdef SS_V30

short FAR PASCAL _export vbSpreadPrintProc(HDC hDC, short nCommand,
                                           short nPageNum, long lAppData,
                                           LPSS_PRINTINFO lpPrintInfo)
#else
short FAR PASCAL _export vbSpreadPrintProc(HDC hDC, short nCommand,
                                           short nPageNum, long lAppData)
#endif
{
  CSsocxCtrl FAR* lpSpread = (CSsocxCtrl FAR*)lAppData;
  LPSPREADSHEET lpSS = SS_SheetLock(lpSpread->m_hWnd);
  GLOBALHANDLE hText = 0;
  short nRet = 0;
#ifdef SS_V30
  //nPageNum += (short)(lpSS->PrintOptions.pf.lFirstPageNumber - 1);
#else
  LPSS_PRINTINFO lpPrintInfo = NULL;
#endif

  switch( nCommand )
  {
    case SS_PRINTCMD_HEADERLEN:
      if( lpSS->lpBook->hPrintAbortMsg )
        vbSSAddPrintAbortItem(lpSpread->m_hWndPrintDlgProc, hDC);
      if( lpSS->PrintOptions.pf.hHeader )
        nRet = vbSpreadPrintHeaderFooter(hDC, lpSS->PrintOptions.pf.hHeader, nPageNum,
                                         lpSS->PrintOptions.pf.fMarginRight,
                                         TRUE, lpPrintInfo);
      break;
    case SS_PRINTCMD_FOOTERLEN:
      if( lpSS->PrintOptions.pf.hFooter )
        nRet = vbSpreadPrintHeaderFooter(hDC, lpSS->PrintOptions.pf.hFooter, nPageNum,
                                         lpSS->PrintOptions.pf.fMarginRight,
                                         TRUE, lpPrintInfo);
      break;
    case SS_PRINTCMD_PRINTHEADER:
      if( lpSS->PrintOptions.pf.hHeader )
        vbSpreadPrintHeaderFooter(hDC, lpSS->PrintOptions.pf.hHeader, nPageNum,
                                  lpSS->PrintOptions.pf.fMarginRight, FALSE,
                                  lpPrintInfo);
      break;
    case SS_PRINTCMD_PRINTFOOTER:
      if( lpSS->PrintOptions.pf.hFooter )
        vbSpreadPrintHeaderFooter(hDC, lpSS->PrintOptions.pf.hFooter, nPageNum,
                                  lpSS->PrintOptions.pf.fMarginRight, FALSE,
                                  lpPrintInfo);
      break;
  }

  SS_SheetUnlock(lpSpread->m_hWnd);
  return nRet;
}
#endif

//--------------------------------------------------------------------

#if !defined(SSA_NO_Print) && !defined(SSA_NO_SmartPrint)
void CSsocxCtrl::SetActionPrint()
{
  PrintSpread(SS_PRINT_ACTION_PRINT, 0, NULL, 0, NULL);
}


void CSsocxCtrl::PrintSpread(WORD wAction, HDC hDCPreview, LPRECT lpRectPreview,
                             long lXtra, LPSS_PRINTPAGECALC lpPrintPageCalc)
{
  PRINTABORTTABLE printAbortItem;
  PRINTABORTPARAM printAbortParam;
  SS_PRINTFUNC lpfnPrintProcInst = 0;
  FARPROC lpfnPrintDlgProcInst = 0;
  ABORTPROC lpfnPrintAbortProcInst = 0;
  SS_PAGEFORMAT PageFormat;
  LPCTSTR lpszPrintJobName = _T("Spread");
  HCURSOR hCursor;
  HWND hWndPrintDlgProc = 0;
  LPSPREADSHEET lpSS = LockSheet();

  if( m_hWnd )
  {
    _fmemcpy(&PageFormat, &lpSS->PrintOptions.pf, sizeof(PageFormat));
    PageFormat.x1CellRange = m_lCol;
    PageFormat.y1CellRange = m_lRow;
    PageFormat.x2CellRange = m_lCol2;
    PageFormat.y2CellRange = m_lRow2;

    if (wAction == SS_PRINT_ACTION_PREVIEW || wAction == SS_PRINT_ACTION_OWNERPRINT)
    {
      PageFormat.dPrintType |= SS_PRINTTYPE_PAGERANGE;
      PageFormat.nPageStart = (short)lXtra;
      PageFormat.nPageEnd = (short)lXtra;
    }
    else if (wAction == SS_PRINT_ACTION_NEXTCOL)
    {
      PageFormat.dPrintType |= SS_PRINTTYPE_CELLRANGE;
      PageFormat.x1CellRange = lXtra;
      PageFormat.x2CellRange = -1;
      PageFormat.y1CellRange = -1;
      PageFormat.y2CellRange = -1;
    }
    else if (wAction == SS_PRINT_ACTION_NEXTROW)
    {
      PageFormat.dPrintType |= SS_PRINTTYPE_CELLRANGE;
      PageFormat.y1CellRange = lXtra;
      PageFormat.y2CellRange = -1;
      PageFormat.x1CellRange = -1;
      PageFormat.x2CellRange = -1;
    }
    PageFormat.hDCPrinter = m_hDCPrinter;

    lpfnPrintProcInst = (SS_PRINTFUNC)MakeProcInstance((FARPROC)vbSpreadPrintProc,
                                                       AfxGetInstanceHandle());

    if (wAction == SS_PRINT_ACTION_PRINT)
    {
      if( lpSS->lpBook->hPrintJobName )
        lpszPrintJobName = (LPTSTR)tbGlobalLock(lpSS->lpBook->hPrintJobName);

      if( lpSS->lpBook->hPrintAbortMsg )
      {
        lpfnPrintAbortProcInst = (ABORTPROC)MakeProcInstance((FARPROC)vbSSPrintAbortProc,
                                                             AfxGetInstanceHandle());
        lpfnPrintDlgProcInst = MakeProcInstance((FARPROC)vbSSPrintDlgProc,
                                                AfxGetInstanceHandle());
        printAbortParam.hAbortMsg = lpSS->lpBook->hPrintAbortMsg;
        printAbortParam.hWndSpread = m_hWnd;
        hWndPrintDlgProc = CreateDialogParam(AfxGetInstanceHandle(),
#ifdef SPREAD_JPN
                                             _T("vbSSPrintDlgBox_Jpn"),
#else
											            _T("vbSSPrintDlgBox"),
#endif
                                             m_hWnd, (DLGPROC)lpfnPrintDlgProcInst,
                                             (long)(LPPRINTABORTPARAM)&printAbortParam);
        m_hWndPrintDlgProc = hWndPrintDlgProc;
        vbSSAddPrintAbortItem(hWndPrintDlgProc, 0);
      }
    }

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    ShowCursor(TRUE);

//- Doesn't allow two Spread controls to print at same time.
//  Otherwise it will hang up.
//-Fengwei
#ifdef	SPREAD_JPN
    static BOOL	bIsPrinting = FALSE;
    if(!bIsPrinting || wAction != SS_PRINT_ACTION_PRINT)
    {
    bIsPrinting = TRUE;
#endif

    LPSPREADSHEET lpSS = LockSheet();
	 SS_AdjustCellCoordsOut(lpSS, &PageFormat.x1CellRange, &PageFormat.y1CellRange);
	 SS_AdjustCellCoordsOut(lpSS, &PageFormat.x2CellRange, &PageFormat.y2CellRange);

    if (*(lpSS->lpBook->m_szPrinterDevice))
      SS_Print(lpSS, lpszPrintJobName, &PageFormat, lpfnPrintProcInst,
               (long)(CSsocxCtrl FAR*)this, lpfnPrintAbortProcInst, wAction,
               hDCPreview, lpRectPreview, lpPrintPageCalc, &(lpSS->lpBook->m_PrinterDevMode),
               lpSS->lpBook->m_szPrinterDriver, lpSS->lpBook->m_szPrinterDevice, lpSS->lpBook->m_szPrinterOutput);
    else
      SS_Print(lpSS, lpszPrintJobName, &PageFormat, lpfnPrintProcInst,
               (long)(CSsocxCtrl FAR*)this, lpfnPrintAbortProcInst, wAction,
               hDCPreview, lpRectPreview, lpPrintPageCalc, NULL, NULL, NULL, NULL);

    UnlockSheet();

#ifdef	SPREAD_JPN
    bIsPrinting = FALSE;
    }
#endif

    ShowCursor(FALSE);
    SetCursor(hCursor);

    if (wAction == SS_PRINT_ACTION_PRINT)
    {
      if( lpSS->lpBook->hPrintJobName )
        tbGlobalUnlock(lpSS->lpBook->hPrintJobName);

      if( lpSS->lpBook->hPrintAbortMsg )
      {
        if( vbSSGetPrintAbortItem(&printAbortItem, 0, hWndPrintDlgProc))
        {
          ::DestroyWindow(printAbortItem.hWndDlg);
          vbSSDeletePrintAbortItem(printAbortItem.hWndDlg);
          m_hWndPrintDlgProc = 0;
        }
        if( lpfnPrintAbortProcInst)
          FreeProcInstance((FARPROC)lpfnPrintAbortProcInst);
        if( lpfnPrintDlgProcInst)
          FreeProcInstance(lpfnPrintDlgProcInst);
      }
    }
    if( lpfnPrintProcInst )
      FreeProcInstance((FARPROC)lpfnPrintProcInst);
  }

  UnlockSheet();

}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ACTION
void CSsocxCtrl::SetAction(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLCOORD cellUL, cellLR;

  switch( nNewValue )
  {
    #ifndef SSA_NO_ActivateCell
    case 0:  // SS_ACTION_ACTIVE_CELL
      if( m_hWnd )
      {
        SS_COORD lRowOld = lpSS->Row.CurAt;
        SS_SetActiveCell(lpSS, m_lCol, m_lRow);
        #ifdef SS_BOUNDCONTROL
        vbSpreadSaveBoundRow(lpSS, (long)this, m_hWnd, lRowOld,
                             lpSS->Row.CurAt, lpSS->Col.CurAt);
        #endif
      }
      break;
    #endif

    #ifndef SSA_NO_Goto
    case 1:  // SS_ACTION_GOTO
      SS_ShowCell(lpSS, m_lCol, m_lRow, m_nPosition);
      break;
    #endif

    #ifndef SSA_NO_SelBlock
    case 2:  // SS_ACTION_SELECT_BLOCK
      cellUL.Col = m_lCol;
      cellUL.Row = m_lRow;
      cellLR.Col = m_lCol2;
      cellLR.Row = m_lRow2;
      SS_SetSelectBlock(lpSS, &cellUL, &cellLR);
      break;
    #endif

    #ifndef SSA_NO_Clear
    case 3:  // SS_ACTION_CLEAR
      if( m_bBlockMode )
        SS_ClearRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2, FALSE);
      else
        SS_ClearRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow, FALSE);
      if( 0 == m_lCol || 0 == m_lRow )
        ControlInfoChanged();
      break;
    #endif

    #ifndef SSA_NO_DeleteCol
    case 4:  // SS_ACTION_DELETE_COL
      {
        SS_COORD lCol = m_lCol;
        SS_COORD lCol2 = m_lCol2;
	     SS_AdjustCellCoordsOut(lpSS, &lCol, NULL);
	     SS_AdjustCellCoordsOut(lpSS, &lCol2, NULL);

        if( m_bBlockMode )
          SSxDelColRange(lpSS, lCol, lCol2);
        else
          SSxDelColRange(lpSS, lCol, lCol);
      }
      break;
    #endif

    #ifndef SSA_NO_DeleteRow
    case 5:  // SS_ACTION_DELETE_ROW
      {
//DBOCX...
        SS_COORD Row2;
        BOOL fDel = TRUE;

        if( m_bBlockMode )
          Row2 = m_lRow2;
        else 
          Row2 = m_lRow;

        if (VBSSBoundIsBound((LONG)this, lpSS))
           if (lpSS, VBSSBoundDel(lpSS, (LONG)this, m_hWnd, m_lRow, Row2))
              fDel = FALSE;

        if (fDel)
           {
           SS_COORD lRow = m_lRow;
           SS_COORD lRow2 = Row2;
	        SS_AdjustCellCoordsOut(lpSS, NULL, &lRow);
	        SS_AdjustCellCoordsOut(lpSS, NULL, &lRow2);

           SSxDelRowRange(lpSS, lRow, lRow2);
           }
//DBOCX
      }
      break;
    #endif

    #ifndef SSA_NO_InsertCol
    case 6:  // SS_ACTION_INSERT_COL
      /* RFW - 9/7/05 - 16692
      if( m_hWnd )
      {
        SS_COORD lCol = m_lCol;
        SS_COORD lCol2 = m_lCol2;
	     SS_AdjustCellCoordsOut(lpSS, &lCol, NULL);
	     SS_AdjustCellCoordsOut(lpSS, &lCol2, NULL);

        if( m_bBlockMode )
          SSInsColRange(m_hWnd, lCol, lCol2);
        else
          SSInsColRange(m_hWnd, lCol, lCol);
      }
      */

      if( m_bBlockMode )
        SS_InsColRange(lpSS, m_lCol, m_lCol2);
      else
        SS_InsColRange(lpSS, m_lCol, m_lCol);

      break;
    #endif

    #ifndef SSA_NO_InsertRow
    case 7:  // SS_ACTION_INSERT_ROW
      /* RFW - 9/7/05 - 16692
      if( m_hWnd )
      {
        SS_COORD lRow = m_lRow;
        SS_COORD lRow2 = m_lRow2;
	     SS_AdjustCellCoordsOut(lpSS, NULL, &lRow);
	     SS_AdjustCellCoordsOut(lpSS, NULL, &lRow2);

        if( m_bBlockMode )
          SSInsRowRange(m_hWnd, lRow, lRow2);
        else
          SSInsRowRange(m_hWnd, lRow, lRow);
      }
      */

      if( m_bBlockMode )
        SS_InsRowRange(lpSS, m_lRow, m_lRow2);
      else
        SS_InsRowRange(lpSS, m_lRow, m_lRow);

      break;
    #endif

    #ifndef SSA_NO_Load
    case 8:  // SS_ACTION_LOAD_SPREAD_SHEET
      break;
    #endif

    #ifndef SSA_NO_SaveAll
    case 9:  // SS_ACTION_SAVE_ALL
      break;
    #endif

    #ifndef SSA_NO_SaveValues
    case 10:  // SS_ACTION_SAVE_VALUES
      break;
    #endif

    #ifndef SSA_NO_Recalc
    case 11:  // SS_ACTION_RECALC
      SS_ReCalcAll(lpSS);
      break;
    #endif

    #ifndef SSA_NO_ClearText
    case 12:  // SS_ACTION_CLEAR_TEXT
      if( m_bBlockMode )
        SS_ClearDataRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2, FALSE, FALSE);
      else
        SS_ClearDataRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow, FALSE, FALSE);
      if( 0 == m_lCol || 0 == m_lRow )
        ControlInfoChanged();
      break;
    #endif

    #ifndef SSA_NO_Print
    case 13:  // SS_ACTION_PRINT
      SetActionPrint();
      break;
    #endif

    #ifndef SSA_NO_DeselectBlock
    case 14:  // SS_ACTION_DESELECT_BLOCK
      SS_DeSelectBlock(lpSS);
      break;
    #endif

    #ifndef SSA_NO_DSave
    case 15:  // SS_ACTION_DSAVE
//DBOCX..
      if (VBSSBoundIsBound((LONG)this, lpSS))
      {
        LPVBSPREAD lpSpread;
        lpSpread = (LPVBSPREAD)SSOcxLockVBSpread((LONG)this);
        if (lpSpread)
        {
           if (!lpSpread->DataOpts.fAutoSave)  // is this "if" needed?
              VBSSBoundManualSave(lpSS, (LONG)this, m_hWnd, lpSpread, lpSS->Row.HeaderCnt, -1, TRUE);
           SSOcxUnlockVBSpread((LONG)this);
        }
      }
//DBOCX
      break;
    #endif

    #ifndef SSA_NO_SetBorder
    case 16:  // SS_ACTION_SET_CELL_BORDER
      if( m_bBlockMode )
        SS_SetBorderRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                          m_nCellBorderType, m_nCellBorderStyle,
                          m_crCellBorderColor);
      else
        SS_SetBorderRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow,
                          m_nCellBorderType, m_nCellBorderStyle,
                          m_crCellBorderColor);
      break;
    #endif

    #ifndef SSA_NO_AddMultiSelBlock
    case 17:  // SS_ACTION_ADD_MULTISELBLOCK
      if( m_hWnd )
      {
        SS_SELBLOCK block;
        block.UL.Col = m_lCol;
        block.UL.Row = m_lRow;
        if( m_bBlockMode )
        {
          block.LR.Col = m_lCol2;
          block.LR.Row = m_lRow2;
        }
        else
        {
          block.LR.Col = m_lCol;
          block.LR.Row = m_lRow;
        }
        SS_AdjustCellCoordsOut(lpSS, &block.UL.Col, &block.UL.Row);
        SS_AdjustCellCoordsOut(lpSS, &block.LR.Col, &block.LR.Row);
        SSAddMultiSelBlocks(lpSS->lpBook->hWnd, &block);
      }
      break;
    #endif

    #ifndef SSA_NO_GetMultiSelBlocks
    case 18:  // SS_ACTION_GET_MULTI_SELECTION
      if( m_hWnd )
      {
        if( m_hMultiSelBlocks )
          GlobalFree(m_hMultiSelBlocks);
        m_hMultiSelBlocks = SS_GetMultiSelBlocks(lpSS, &m_nMultiSelCount);
      }
      break;
    #endif

    #ifndef SSA_NO_CopyRange
    case 19:  // SS_ACTION_COPY_RANGE
#ifdef	SPREAD_JPN
	  //- Wei Feng for Incident 381
	  {
      long lCol  = min(m_lCol, m_lCol2);
	  long lCol2 = max(m_lCol, m_lCol2);
	  long lRow	 = min(m_lRow, m_lRow2);
	  long lRow2 = max(m_lRow, m_lRow2);
      SS_CopySwapRange(lpSS, lCol, lRow, lCol2, lRow2,
                       m_lDestCol, m_lDestRow, SS_CMD_COPY, FALSE, TRUE);
	  }
#else
      SS_CopySwapRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                       m_lDestCol, m_lDestRow, SS_CMD_COPY, FALSE, TRUE);
#endif
      if( 0 == m_lDestCol || 0 == m_lDestRow )
        ControlInfoChanged();
      break;
    #endif

    #ifndef SSA_NO_MoveRange
    case 20:  // SS_ACTION_MOVE_RANGE
#ifdef	SPREAD_JPN
	  //- Wei Feng for Incident 381
	  {
      long lCol  = min(m_lCol, m_lCol2);
	  long lCol2 = max(m_lCol, m_lCol2);
	  long lRow	 = min(m_lRow, m_lRow2);
	  long lRow2 = max(m_lRow, m_lRow2);
      SS_CopySwapRange(lpSS, lCol, lRow, lCol2, lRow2,
                       m_lDestCol, m_lDestRow, SS_CMD_MOVE, FALSE, TRUE);
	  }
#else
      SS_CopySwapRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                       m_lDestCol, m_lDestRow, SS_CMD_MOVE, FALSE, TRUE);
#endif
      if( 0 == m_lCol || 0 == m_lRow || 0 == m_lDestCol || 0 == m_lDestRow )
        ControlInfoChanged();
      break;
    #endif

    #ifndef SSA_NO_SwapRange
    case 21:  // SS_ACTION_SWAP_RANGE
#ifdef	SPREAD_JPN
	  //- Wei Feng for Incident 381
	  {
      long lCol  = min(m_lCol, m_lCol2);
	  long lCol2 = max(m_lCol, m_lCol2);
	  long lRow	 = min(m_lRow, m_lRow2);
	  long lRow2 = max(m_lRow, m_lRow2);
      SS_CopySwapRange(lpSS, lCol, lRow, lCol2, lRow2,
                       m_lDestCol, m_lDestRow, SS_CMD_SWAP, FALSE, TRUE);
	  }
#else
      SS_CopySwapRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                       m_lDestCol, m_lDestRow, SS_CMD_SWAP, FALSE, TRUE);
#endif
      if( 0 == m_lCol || 0 == m_lRow || 0 == m_lDestCol || 0 == m_lDestRow )
        ControlInfoChanged();
      break;
    #endif

    #ifndef SSA_NO_ClipboardCopy
    case 22:  // SS_ACTION_CLIPBOARD_COPY
      if( m_hWnd )
        SSClipboardCopy(m_hWnd);
      break;
    #endif

    #ifndef SSA_NO_ClipboardCut
    case 23:  // SS_ACTION_CLIPBOARD_CUT
      if( m_hWnd )
        SSClipboardCut(m_hWnd);
      ControlInfoChanged();
      break;
    #endif

    #ifndef SSA_NO_ClipboardPaste
    case 24:  // SS_ACTION_CLIPBOARD_PASTE
      if( m_hWnd )
        SSClipboardPaste(m_hWnd);
      ControlInfoChanged();
      break;
    #endif

    #ifndef SSA_NO_Sort
    case 25:  // SS_ACTION_SORT
      if (m_hWnd && m_nSortKeyCnt && m_hSortKeys)
         {
			SS_COORD Col = m_lCol;
			SS_COORD Row = m_lRow;
			SS_COORD Col2 = m_lCol2;
			SS_COORD Row2 = m_lRow2;

         LPSS_SORTKEY lpSortKeys = (LPSS_SORTKEY)GlobalLock(m_hSortKeys);

			SS_AdjustCellCoordsOut(lpSS, &Col, &Row);
			SS_AdjustCellCoordsOut(lpSS, &Col2, &Row2);

         SSSortEx(m_hWnd, Col, Row, Col2, Row2, m_nSortBy, lpSortKeys, m_nSortKeyCnt);
         if( VBSSBoundIsBound((LONG)this, lpSS) )
         {
           if( SSOcxLockVBSpread((LONG)this) )
           {
             LPSPREADSHEET lpSS = LockSheet();
             SS_COORD Row;
             SS_GetActiveCell(lpSS, NULL, &Row);
             vbSpreadBoundClick(lpSS, (LONG)this, Row);
             UnlockSheet();
             SSOcxUnlockVBSpread((LONG)this);
           }
         }
         GlobalUnlock(m_hSortKeys);
         }

      break;
    #endif

    #ifndef SSA_NO_ComboClear
    case 26:  // SS_ACTION_COMBO_CLEAR
      if( m_hWnd )
        SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow,
                              SS_CBM_RESETCONTENT, 0, 0L);
      break;
    #endif

    #ifndef SSA_NO_ComboRemoveString
    case 27:  // SS_ACTION_COMBO_REMOVE
      if( m_hWnd )
	  {
		//Modify by BOC 99.7.5 (hyt)-------------------
		//for remove current select or up item not change 
		//current selected index
		//Modify by BOC 99.7.14 (hyt)-------------------
		//for when combobox is editable remove current select item
		//it will set text "-1"
		SS_CELLTYPE cellType;
		BOOL bEditable = FALSE;
		if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
			bEditable = cellType.Style & SS_CB_DROPDOWN ? TRUE : FALSE;

		short nCount = (short)SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow,
                                         SS_CBM_GETCOUNT, 0, 0);
		short nCurSel = (short)SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow,
                                         SS_CBM_GETCURSEL, 0, 0);
		short nNewCount = (short)SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow, SS_CBM_DELETESTRING,
                              m_nTypeComboBoxIndex, 0L);
		if(nNewCount< nCount && m_nTypeComboBoxIndex<=nCurSel && nCurSel>=0)
		{
			int nNewSel = m_nTypeComboBoxIndex<nCurSel?nCurSel-1:-1;
			if(!bEditable || (bEditable && nNewSel>=0))
				SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow,
					SS_CBM_SETCURSEL, m_nTypeComboBoxIndex<nCurSel?nCurSel-1:-1, 0);
			else
				SS_SetValue(lpSS, m_lCol, m_lRow, _T(""));
		}
		//--------------------------------------------------------------------------
	  }
      break;
    #endif

    #ifndef SSA_NO_Reset
    case 28:  // SS_ACTION_RESET
      if( m_hWnd )
      {
        SSReset(m_hWnd);
        SSSetOwner(m_hWnd, 0); // ???? is needed to prevent VBX bug ????
        DestroyPicts();
        InitOcx();
        ControlInfoChanged();
      }
      break;
    #endif

    #ifndef SSA_NO_SelModeClear
    case 29:  // SS_ACTION_SEL_MODE_CLEAR
      if( m_hWnd )
        SS_SelModeSendMessage(lpSS, SS_SELMODE_CLEARSEL, 0L, 0L, 0L);
      break;
    #endif

    #ifndef SSA_NO_VRefreshBuffer
    case 30:  // SS_ACTION_VMODE_REFRESH
      if( m_hWnd )
        SSVRefreshBuffer(m_hWnd);
      break;
    #endif

    #ifndef SSA_NO_DataRefresh
    case 31:  // SS_ACTION_REFRESH_BOUND
      break;
    #endif

    #ifndef SSA_NO_SmartPrint
    case 32:  // SS_ACTION_SMARTPRINT
      LPSPREADSHEET lpSS = LockSheet();
      BOOL bSmartPrintOld = lpSS->PrintOptions.pf.fSmartPrint;
      lpSS->PrintOptions.pf.fSmartPrint = TRUE;
      SetActionPrint();
      lpSS->PrintOptions.pf.fSmartPrint = bSmartPrintOld;
      UnlockSheet();
      break;
    #endif
  }
  if (IsWindow(m_hWnd))
  {
    INVALIDATE_CONTROL();
  }
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ACTIVECOL
long CSsocxCtrl::GetActiveCol() 
{
  GET_NODESIGN();
  if( m_hWnd )
  {
    SS_COORD lCol, lRow;
    SSGetActiveCell(m_hWnd, &lCol, &lRow);
    if (lCol == SS_HEADER) lCol = 0;
    return lCol;
  }

    return 0;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ACTIVEROW
long CSsocxCtrl::GetActiveRow() 
{
  GET_NODESIGN();
  if( m_hWnd )
  {
    SS_COORD lCol, lRow;
    SSGetActiveCell(m_hWnd, &lCol, &lRow);
    if (lRow == SS_HEADER) lRow = 0;
    return lRow;
  }

  return 0;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ALLOWCELLOVERFLOW
BOOL CSsocxCtrl::GetAllowCellOverflow() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWCELLOVERFLOW);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ALLOWCELLOVERFLOW
void CSsocxCtrl::SetAllowCellOverflow(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWCELLOVERFLOW, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAllowCellOverflow);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ALLOWDRAGDROP
BOOL CSsocxCtrl::GetAllowDragDrop() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWDRAGDROP);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ALLOWDRAGDROP
void CSsocxCtrl::SetAllowDragDrop(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWDRAGDROP, bNewValue);

  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAllowDragDrop);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ALLOWMULTIBLOCKS
BOOL CSsocxCtrl::GetAllowMultiBlocks() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWMULTIBLOCKS);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ALLOWMULTIBLOCKS
void CSsocxCtrl::SetAllowMultiBlocks(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWMULTIBLOCKS, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAllowMultiBlocks);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ALLOWUSERFORMULAS
BOOL CSsocxCtrl::GetAllowUserFormulas() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWUSERFORMULAS);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ALLOWUSERFORMULAS
void CSsocxCtrl::SetAllowUserFormulas(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWUSERFORMULAS, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAllowUserFormulas);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ARROWSEXITEDITMODE
BOOL CSsocxCtrl::GetArrowsExitEditMode() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_ARROWSEXITEDITMODE);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ARROWSEXITEDITMODE
void CSsocxCtrl::SetArrowsExitEditMode(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_ARROWSEXITEDITMODE, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidArrowsExitEditMode);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_AUTOCALC
BOOL CSsocxCtrl::GetAutoCalc() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_AUTOCALC);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_AUTOCALC
void CSsocxCtrl::SetAutoCalc(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOCALC, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAutoCalc);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_AUTOCLIPBOARD
BOOL CSsocxCtrl::GetAutoClipboard() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_AUTOCLIPBOARD);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_AUTOCLIPBOARD
void CSsocxCtrl::SetAutoClipboard(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOCLIPBOARD, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAutoClipboard);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_AUTOSIZE
BOOL CSsocxCtrl::GetAutoSize() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_AUTOSIZE);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_AUTOSIZE
void CSsocxCtrl::SetAutoSize(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_AUTOSIZE, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAutoSize);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BACKCOLOR
OLE_COLOR CSsocxCtrl::GetBackColor() 
{
  /* RFW - 8/2/05 - 16376
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crBackColor, crForeColor;
  if( AmbientUserMode() == FALSE )

  SS_GetColor(lpSS, m_lCol, m_lRow, &crBackColor, &crForeColor);
  UnlockSheet();
  return crBackColor;
  */
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crBackColor, crForeColor;
  if( AmbientUserMode() == FALSE )
    SS_GetColor(lpSS, -1, -1, &crBackColor, &crForeColor);
  else
    SS_GetColor(lpSS, m_lCol, m_lRow, &crBackColor, &crForeColor);
  UnlockSheet();
  return crBackColor;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BACKCOLOR
void CSsocxCtrl::SetBackColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  if( m_bBlockMode )
    SS_SetColorRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                     newValue, SPREAD_COLOR_IGNORE);
  else
    SS_SetColorRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow,
                     newValue, SPREAD_COLOR_IGNORE);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BLOCKMODE
BOOL CSsocxCtrl::GetBlockMode() 
{
  GET_NODESIGN();
  return m_bBlockMode;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BLOCKMODE
void CSsocxCtrl::SetBlockMode(BOOL bNewValue) 
{
  m_bBlockMode = bNewValue;
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BORDERSTYLE
short CSsocxCtrl::GetBorderStyle() 
{
    return m_sBorderStyle;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BORDERSTYLE
void CSsocxCtrl::SetBorderStyle(short nNewValue) 
{
  if( m_sBorderStyle != nNewValue && 0 <= nNewValue && nNewValue <= 1 )
  {
    DWORD dwStyle = m_hWnd ? GetStyle() : 0;
    m_sBorderStyle = nNewValue;
    if( m_hWnd != NULL )
      RecreateControlWindow();
    InvalidateControl();

    // This code invalidates the scroll bars and nonclient area.
    // Its not the best solution, but it works. RFW
    ::ShowWindow(m_hWnd, SW_HIDE);
    if( dwStyle & WS_VISIBLE )  // BJO 22Jun98 GIC5925 - Added visible check
      ::ShowWindow(m_hWnd, SW_SHOW);

    SetModifiedFlag();
    if( AmbientUserMode() == FALSE )
      BoundPropertyChanged(DISPID_BORDERSTYLE);
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BUTTONDRAWMODE
#ifdef SS_V70
long CSsocxCtrl::GetButtonDrawMode() 
{
  LPSS_BOOK lpBook = LockBook();
  long lValue = (long)SS_GetButtonDrawMode(lpBook);
  UnlockBook();
  return lValue;
}
#else
short CSsocxCtrl::GetButtonDrawMode() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetButtonDrawMode(lpBook);
  UnlockBook();
  return nValue;
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BUTTONDRAWMODE
#ifdef SS_V70
void CSsocxCtrl::SetButtonDrawMode(long lNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetButtonDrawMode(lpBook, (short)lNewValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidButtonDrawMode);
}
#else
void CSsocxCtrl::SetButtonDrawMode(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetButtonDrawMode(lpBook, nNewValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidButtonDrawMode);
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CALCDEPENDENCIES
BOOL CSsocxCtrl::GetCalcDependencies() 
{
  GET_NODESIGN();
  return m_bCalcDependencies;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CALCDEPENDENCIES
void CSsocxCtrl::SetCalcDependencies(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  m_bCalcDependencies = bNewValue;
  if( m_bCalcDependencies )
    SS_BuildDependencies(lpSS);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidCalcDependencies);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLBORDERCOLOR
OLE_COLOR CSsocxCtrl::GetCellBorderColor() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crColor = RGBCOLOR_DEFAULT;

#ifndef SS_V30
  WORD wType;
  WORD wStyle;
  if( !SS_GetBorder(lpSS, m_lCol, m_lRow, &wType, &wStyle, &crColor) )
     crColor = 0;
#else
  if ((m_nCellBorderType & SS_BORDERTYPE_LEFT) ||
	   (m_nCellBorderType & SS_BORDERTYPE_OUTLINE))
     SS_GetBorderEx(lpSS, m_lCol, m_lRow,
                   NULL, &crColor, NULL, NULL, NULL, NULL,
                   NULL, NULL);
  else if (m_nCellBorderType & SS_BORDERTYPE_TOP)
     SS_GetBorderEx(lpSS, m_lCol, m_lRow,
                   NULL, NULL, NULL, &crColor, NULL, NULL,
                   NULL, NULL);
  else if (m_nCellBorderType & SS_BORDERTYPE_RIGHT)
     SS_GetBorderEx(lpSS, m_lCol, m_lRow,
                   NULL, NULL, NULL, NULL, NULL, &crColor,
                   NULL, NULL);
  else if (m_nCellBorderType & SS_BORDERTYPE_BOTTOM)
     SS_GetBorderEx(lpSS, m_lCol, m_lRow,
                   NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                   &crColor);
#endif
  UnlockSheet();
  return crColor;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLBORDERCOLOR
void CSsocxCtrl::SetCellBorderColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  m_crCellBorderColor = newValue;
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLBORDERSTYLE
short CSsocxCtrl::GetCellBorderStyle() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  WORD wStyle = 0;
#ifndef SS_V30
  WORD wType;
  COLORREF crColor;
  if( !SS_GetBorder(lpSS, m_lCol, m_lRow, &wType, &wStyle, &crColor) )
    wStyle = 0;
#else
  if (m_nCellBorderType & SS_BORDERTYPE_LEFT)
     SS_GetBorderEx(lpSS, m_lCol, m_lRow,
                    &wStyle, NULL, NULL, NULL, NULL, NULL,
                    NULL, NULL);
  else if (m_nCellBorderType & SS_BORDERTYPE_TOP)
     SS_GetBorderEx(lpSS, m_lCol, m_lRow,
                    NULL, NULL, &wStyle, NULL, NULL, NULL,
                    NULL, NULL);
  else if (m_nCellBorderType & SS_BORDERTYPE_RIGHT)
     SS_GetBorderEx(lpSS, m_lCol, m_lRow,
                    NULL, NULL, NULL, NULL, &wStyle, NULL,
                    NULL, NULL);
  else if (m_nCellBorderType & SS_BORDERTYPE_BOTTOM)
     SS_GetBorderEx(lpSS, m_lCol, m_lRow,
                    NULL, NULL, NULL, NULL, NULL, NULL,
                    &wStyle, NULL);
  else
     SS_GetBorder(lpSS, m_lCol, m_lRow, NULL, &wStyle, NULL);
#endif

  UnlockSheet();
  return wStyle;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLBORDERSTYLE
void CSsocxCtrl::SetCellBorderStyle(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  m_nCellBorderStyle = nNewValue;
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLBORDERTYPE
short CSsocxCtrl::GetCellBorderType() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  WORD wType = 0;
//#ifndef SS_V30
  WORD wStyle;
  COLORREF crColor;
  if( !SS_GetBorder(lpSS, m_lCol, m_lRow, &wType, &wStyle, &crColor) )
    wType = 0;
//#else
//  wType = m_nCellBorderType;
//#endif
  UnlockSheet();
  return wType;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLBORDERTYPE
void CSsocxCtrl::SetCellBorderType(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  m_nCellBorderType = nNewValue;
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLTYPE
short CSsocxCtrl::GetCellType() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLTYPE cellType;
  short nValue = 0;
  SS_RetrieveCellType(lpSS, &cellType, NULL, m_lCol, m_lRow);
  switch( cellType.Type)
  {
    case SS_TYPE_DATE: nValue = 0; break;
    case SS_TYPE_EDIT: nValue = 1; break;
    case SS_TYPE_FLOAT: nValue = 2; break;
    #ifndef SS_NOCT_INT
    case SS_TYPE_INTEGER: nValue = 3; break;
    #endif
    case SS_TYPE_PIC: nValue = 4; break;
    case SS_TYPE_STATICTEXT: nValue = 5; break;
    case SS_TYPE_TIME: nValue = 6; break;
    case SS_TYPE_BUTTON: nValue = 7; break;
    case SS_TYPE_COMBOBOX: nValue = 8; break;
    case SS_TYPE_PICTURE: nValue = 9; break;
    case SS_TYPE_CHECKBOX: nValue = 10; break;
    #ifndef SS_NOCT_OWNERDRAW
    case SS_TYPE_OWNERDRAW: nValue = 11; break;
    #endif
#ifdef SS_V40
    case SS_TYPE_CURRENCY: nValue = 12; break;
    case SS_TYPE_NUMBER: nValue = 13; break;
    case SS_TYPE_PERCENT: nValue = 14; break;
#endif // SS_V40
#ifdef SS_V70
    case SS_TYPE_SCIENTIFIC: nValue = 15; break;
#endif // SS_V70
#ifdef SS_V80
    case SS_TYPE_CUSTOM: nValue = 16; break;
#endif
  }
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLTYPE
void CSsocxCtrl::SetCellType(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLTYPE cellType;
  DATEFORMAT dateFormat;
  TIMEFORMAT timeFormat;
  SUPERBTNCOLOR color;
  long lStyle;
  TCHAR szBuffer[330];
  short nLeft;
  CString sMask;
#if SS_V80
  SS_CT_VALUE ItemData;
#endif

#ifdef SS_V80
  if( 0 <= nNewValue && nNewValue <= 16 )
#elif SS_V70
  if( 0 <= nNewValue && nNewValue <= 15 )
#elif defined(SS_V40)
  if( 0 <= nNewValue && nNewValue <= 14 )
#else
  if( 0 <= nNewValue && nNewValue <= 11 )
#endif // SS_V40
  {
    switch( nNewValue )
    {
      case 0:  // Date
        lStyle = ES_LEFT;
        dateFormat.bCentury = m_bTypeDateCentury;
        dateFormat.cSeparator = (TCHAR)m_nTypeDateSeparator;
        dateFormat.nFormat = IDF_DDMONYY + m_nTypeDateFormat;
        dateFormat.bSpin = m_bTypeSpin;
        SS_SetTypeDate(
          lpSS, &cellType, lStyle, &dateFormat, &m_TypeDateMin, &m_TypeDateMax);
        break;
      case 1:  // Edit
        lStyle = ES_LEFT | ES_AUTOHSCROLL;
        if( m_bTypeEditPassword )
          lStyle |= ES_PASSWORD;
        if( m_bTypeEditMultiLine )
        {
          lStyle |= ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN;
          lStyle &= ~ES_AUTOHSCROLL;
        }
        SS_SetTypeEdit(
          &cellType, lStyle, (short)m_lTypeEditLen,
          m_nTypeEditCharSet, m_nTypeEditCharCase);
        break;
      case 2:  // Float
        lStyle = ES_RIGHT;
        if( m_bTypeFloatSeparator )
          lStyle |= FS_SEPARATOR;
        if( m_bTypeFloatMoney )
          lStyle |= FS_MONEY;
#ifdef SS_V40
        if (m_fTypeNegRed)
          lStyle |= SSS_NEGRED;
#endif // SS_V40

        StrPrintf(
          szBuffer, _T("%.*f"), 2,
          max(fabs(m_dfTypeFloatMax), fabs(m_dfTypeFloatMin)));
        nLeft = (short)(StrChr(szBuffer,'.') - (LPTSTR)szBuffer);
        SS_SetTypeFloatExt(
          lpSS, &cellType, lStyle, nLeft, m_nTypeFloatDecimalPlaces,
          m_dfTypeFloatMin, m_dfTypeFloatMax, NULL);
        break;
      #ifndef SS_NOCT_INT
      case 3:  // Integer
        lStyle = ES_RIGHT;
        if( m_bTypeSpin )
          lStyle |= IS_SPIN;
#ifdef SS_V40
        if (m_fTypeNegRed)
          lStyle |= SSS_NEGRED;
#endif // SS_V40
        SS_SetTypeIntegerExt(
          lpSS, &cellType, lStyle, m_lTypeIntegerMin, m_lTypeIntegerMax,
          m_bTypeIntegerSpinWrap, m_lTypeIntegerSpinInc);
        break;
      #endif
      case 4:  // PIC
        lStyle = ES_LEFT;
        sMask = m_sTypePicMask;
        if( !m_sTypePicDefaultText.IsEmpty() )
// 96' 5/17 BOC Gao. for an error after adding /J option to compiler. why?
//          sMask += '\n' + m_sTypePicDefaultText;
          {
          sMask += '\n'; 
      sMask += m_sTypePicDefaultText;
          }
// ---------------------------------------------------------------<<
        SS_SetTypePic(lpSS, &cellType, lStyle, sMask);
        break;
      case 5:  // StaticText
        lStyle = SS_TEXT_LEFT;
        if( 0 == m_nTypeTextAlignVert )
          lStyle |= SS_TEXT_BOTTOM;
        else if( 1 == m_nTypeTextAlignVert )
          lStyle |= SS_TEXT_VCENTER;
        else if( 2 == m_nTypeTextAlignVert )
          lStyle |= SS_TEXT_TOP;
        if( m_bTypeTextPrefix )
          lStyle |= SS_TEXT_PREFIX;
        if( m_bTypeTextShadow )
          lStyle |= SS_TEXT_SHADOW;
        if( m_bTypeTextShadowIn )
          lStyle |= SS_TEXT_SHADOWIN;
        if( m_bTypeTextWordWrap )
          lStyle |= SS_TEXT_WORDWRAP;
        SS_SetTypeStaticText(lpSS, &cellType, lStyle);
        break;
      case 6:  // Time
        lStyle = ES_LEFT;
        timeFormat.b24Hour = m_nTypeTime24Hour ? 1 : 0;
        timeFormat.bSeconds = m_bTypeTimeSeconds;
        timeFormat.cSeparator = (TCHAR)m_nTypeTimeSeparator;
        timeFormat.bSpin = m_bTypeSpin;
        SS_SetTypeTime(
          lpSS, &cellType, lStyle, &timeFormat,
          &m_TypeTimeMin, &m_TypeTimeMax);
        break;
      case 7:  // Button
        lStyle = 0;
        if( 0 == m_nTypeButtonAlign )
          lStyle |= SBS_TEXTBOT_PICTTOP;
        else if( 1 == m_nTypeButtonAlign )
          lStyle |= SBS_TEXTTOP_PICTBOT;
        else if( 2 == m_nTypeButtonAlign )
          lStyle |= SBS_TEXTLEFT_PICTRIGHT;
        else if( 3 == m_nTypeButtonAlign )
          lStyle |= SBS_TEXTRIGHT_PICTLEFT;
        color.Color = m_crTypeButtonColor;
        color.ColorBorder = m_crTypeButtonBorderColor;
        color.ColorShadow = m_crTypeButtonDarkColor;
        color.ColorHighlight = m_crTypeButtonLightColor;
        color.ColorText = m_crTypeButtonTextColor;
        SS_SetTypeButton(
          lpSS, &cellType, lStyle, m_sTypeButtonText,
          NULL, SUPERBTN_PICT_NONE, NULL, SUPERBTN_PICT_NONE,
          m_nTypeButtonType, m_nTypeButtonShadowSize, &color);
        break;
      case 8:  // ComboBox
        lStyle = 0;
        if( m_bTypeComboBoxEditable )
          lStyle |= SS_CB_DROPDOWN;
		//Modify by BOC 99.5.5 (hyt)--------------------------------
		//for TypeComboBoxMaxDrop and TypeMaxEditLen property for 
		//combo box type cell is 6 and 150. 
        //SS_SetTypeComboBox(lpSS, &cellType, lStyle, 0, -1, -1, 0, 0);
		SS_SetTypeComboBox(lpSS, &cellType, lStyle, 0, 6, 150, 0, 0, SS_COMBOBOX_AUTOSEARCH_SINGLECHAR);
		//-------------------------------------------------------------
        break;
      case 9:  // Picture ????
        SS_SetTypePicture(lpSS, &cellType, 0, NULL);
        break;
      case 10:  // CheckBox ????
        lStyle = 0;
        if( m_bTypeCheckCenter )
          lStyle |= BS_CENTER;
        SS_SetTypeCheckBox(
          lpSS, &cellType, lStyle, m_sTypeCheckText,
          NULL, BT_NONE, NULL, BT_NONE,
          NULL, BT_NONE, NULL, BT_NONE,
          NULL, BT_NONE, NULL, BT_NONE);
        break;
      #ifndef SS_NOCT_OWNERDRAW
      case 11:  // OwnerDraw
        SS_SetTypeOwnerDraw(lpSS, &cellType, m_lTypeOwnerDrawStyle);
        break;
      #endif
#ifdef SS_V40
      case 12:  // Currency
        cellType = m_TypeCurrency;
        break;
      case 13:  // Number
        cellType = m_TypeNumber;
        break;
      case 14:  // Percent
        cellType = m_TypePercent;
        break;
#endif // SS_V40
#ifdef SS_V70
      case 15:  // Scientific
        cellType = m_TypeScientific;
        break;
#endif // SS_V70
#if SS_V80
      case 16: // Custom
         CT_VariantToValue(&m_TypeCustomItemData, &ItemData);
         SS_SetTypeCustom(lpSS, &cellType, m_sTypeCustomName, m_lTypeCustomStyle, &ItemData);
         break;
#endif // SS_V80
      default:
        // ????????
        break;
    }
    if( m_bBlockMode )
      SS_SetCellTypeRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2, &cellType);
    else
      SS_SetCellTypeRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow, &cellType);

    if( 0 == m_lCol || 0 == m_lRow )
      ControlInfoChanged();

    INVALIDATE_CONTROL();
    SetModifiedFlag();
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);

  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CHANGEMADE
BOOL CSsocxCtrl::GetChangeMade() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_CHANGEMADE);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CHANGEMADE
void CSsocxCtrl::SetChangeMade(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_CHANGEMADE, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CLIP
BSTR CSsocxCtrl::GetClip() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  CString s;
  GLOBALHANDLE hText;
  LPTSTR lpszText;
  hText = SS_ClipOut(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2, FALSE, FALSE);
  if( hText && (lpszText = (LPTSTR)GlobalLock(hText)) )
  {
    s = lpszText;
    GlobalUnlock(hText);
    GlobalFree(hText);
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CLIP
void CSsocxCtrl::SetClip(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  LPSPREADSHEET lpSS = LockSheet();
  SS_ClipIn(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
            lpszNewValue, lstrlen(lpszNewValue),
            FALSE, FALSE, FALSE, NULL, NULL);
  if( 0 == m_lCol || 0 == m_lRow )
    ControlInfoChanged();
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CLIPVALUE
BSTR CSsocxCtrl::GetClipValue() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  CString s;
  GLOBALHANDLE hText;
  LPTSTR lpszText;
  hText = SS_ClipOut(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2, TRUE, FALSE);
  if( hText && (lpszText = (LPTSTR)GlobalLock(hText)) )
  {
    s = lpszText;
    GlobalUnlock(hText);
    GlobalFree(hText);
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CLIPVALUE
void CSsocxCtrl::SetClipValue(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  LPSPREADSHEET lpSS = LockSheet();
  SS_ClipIn(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
            lpszNewValue, lstrlen(lpszNewValue),
            TRUE, FALSE, FALSE, NULL, NULL);
  if( 0 == m_lCol || 0 == m_lRow )
    ControlInfoChanged();
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COL
long CSsocxCtrl::GetCol() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol = m_lCol;
  GET_NODESIGN();
  SS_AdjustCellCoordsOut(lpSS, &lCol, NULL);
  UnlockSheet();
  return lCol;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COL
void CSsocxCtrl::SetCol(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  if( (-1 <= nNewValue && nNewValue <= SS_MAXCOORD) ||
      (SS_HEADER <= nNewValue && nNewValue < SS_HEADER + lpSS->Col.HeaderCnt))
  {
/* RAP01d
    LPSPREADSHEET lpSS = LockSheet();
// GIC8667 RFW 3/9/99    m_lCol = nNewValue;
    m_lCol = min(nNewValue, SS_GetMaxCols(lpSS));
    SetModifiedFlag();
    UnlockSheet();
*/
// RAP01a>>
    m_lCol = nNewValue;
	 SS_AdjustCellCoords(lpSS, &m_lCol, NULL);
    SetModifiedFlag();
//<<RAP01a  
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);

  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COL2
long CSsocxCtrl::GetCol2() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol = m_lCol2;
  GET_NODESIGN();
  SS_AdjustCellCoordsOut(lpSS, &lCol, NULL);
  UnlockSheet();
  return lCol;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COL2
void CSsocxCtrl::SetCol2(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  if( (-1 <= nNewValue && nNewValue <= SS_MAXCOORD) ||
      (SS_HEADER <= nNewValue && nNewValue < SS_HEADER + lpSS->Col.HeaderCnt))
  {
/* RAP01d
    LPSPREADSHEET lpSS = LockSheet();
// GIC8667 RFW 3/9/99    m_lCol2 = nNewValue;
    m_lCol2 = min(nNewValue, SS_GetMaxCols(lpSS));
    SetModifiedFlag();
    UnlockSheet();
*/
// RAP01a>>
    m_lCol2 = nNewValue;
	 SS_AdjustCellCoords(lpSS, &m_lCol2, NULL);
    SetModifiedFlag();
// <<RAP01a
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);

  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLHEADERDISPLAY
short CSsocxCtrl::GetColHeaderDisplay() 
{
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = SS_GetColHeaderDisplay(lpSS);
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLHEADERDISPLAY
void CSsocxCtrl::SetColHeaderDisplay(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetColHeaderDisplay(lpSS, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidColHeaderDisplay);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLHIDDEN
BOOL CSsocxCtrl::GetColHidden() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL          fRet = FALSE;

  fRet = SS_IsColHidden(lpSS, m_lCol) ? -1 : 0;
  UnlockSheet();
  return fRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLHIDDEN
void CSsocxCtrl::SetColHidden(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD c, c1, c2;

  // RFW - 4/28/99 - This was changed to allow BlockMode to work
  // with ColHidden

  if( m_bBlockMode )
  {
    c1 = m_lCol;
    c2 = m_lCol2;
  }
  else
    c1 = c2 = m_lCol;

  if( SS_ALLCOLS == c1 || SS_ALLCOLS == c2 )
    c1 = c2 = SS_ALLCOLS;

  for( c = c1; c <= c2; c++ )
    SS_ShowCol(lpSS, c, !bNewValue);

  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLPAGEBREAK
BOOL CSsocxCtrl::GetColPageBreak() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetColPageBreak(lpSS, m_lCol);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLPAGEBREAK
void CSsocxCtrl::SetColPageBreak(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetColPageBreak(lpSS, m_lCol, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLSFROZEN
long CSsocxCtrl::GetColsFrozen() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCols;
  SS_COORD lRows;
  SS_GetFreeze(lpSS, &lCols, &lRows);
  UnlockSheet();
  return lCols;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLSFROZEN
void CSsocxCtrl::SetColsFrozen(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCols;
  SS_COORD lRows;
  SS_GetFreeze(lpSS, &lCols, &lRows);
  SS_SetFreeze(lpSS, nNewValue, lRows);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidColsFrozen);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLWIDTH
double CSsocxCtrl::GetColWidth(long lCol) 
{
  GET_NODESIGN();
  double dfWidth = 0.0;
  LPSPREADSHEET lpSS = LockSheet();

  if ( -2 <= lCol && lCol <= SS_MAXCOORD ||
      (SS_HEADER <= lCol && lCol < SS_HEADER + lpSS->Col.HeaderCnt))
  {
    if( lCol == -2 )
      lCol = m_lCol;
    else
      SS_AdjustCellCoords(lpSS, &lCol, NULL);
    SS_GetColWidth(lpSS, lCol, &dfWidth);
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYARRAYINDEX);

  UnlockSheet();
  return dfWidth;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_COLWIDTH
void CSsocxCtrl::SetColWidth(long lCol, double newValue) 
{
  LPSPREADSHEET lpSS = LockSheet();

  if ( -2 <= lCol && lCol <= SS_MAXCOORD ||
      (SS_HEADER <= lCol && lCol < SS_HEADER + lpSS->Col.HeaderCnt))
  {
    if (lCol == -2)
       lCol = m_lCol;
    else
      SS_AdjustCellCoords(lpSS, &lCol, NULL);
    SS_SetColWidth(lpSS, lCol, newValue);
    INVALIDATE_CONTROL();
    SetModifiedFlag();
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYARRAYINDEX);

  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CURSORICON
LPPICTUREDISP CSsocxCtrl::GetCursorIcon() 
{
  GET_NODESIGN();
  LPPICTUREDISP lpdispValue = NULL;

  if( m_lppictCursorIcon )
    m_lppictCursorIcon->QueryInterface(IID_IPictureDisp, (LPVOID FAR*)&lpdispValue);
// fix for 8049 -scl
  else
  {
    CPictureHolder pict;
    pict.CreateEmpty();
    lpdispValue = pict.GetPictureDispatch();
  }
  return lpdispValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CURSORICON
void CSsocxCtrl::SetCursorIcon(LPPICTUREDISP newValue) 
{
  short nType;

  if( m_lppictCursorIcon )
    m_lppictCursorIcon->Release();
  if( newValue )
    newValue->QueryInterface(IID_IPicture, (LPVOID FAR*)&m_lppictCursorIcon);
  else
    m_lppictCursorIcon = NULL;
  if( m_lppictCursorIcon )
  {
    m_lppictCursorIcon->get_Type(&nType);
    if( PICTYPE_ICON != nType )
    {
      m_lppictCursorIcon->Release();
      m_lppictCursorIcon = NULL;
    }
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CURSORSTYLE
short CSsocxCtrl::GetCursorStyle() 
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  WORD wType = SS_CURSORTYPE_DEFAULT;
  HCURSOR hCursor;
  short nValue = 0;  // user defined cursor
  switch( m_nCursorType )
  {
    case 0: wType = SS_CURSORTYPE_DEFAULT; break;
    case 1: wType = SS_CURSORTYPE_COLRESIZE; break;
    case 2: wType = SS_CURSORTYPE_ROWRESIZE; break;
    case 3: wType = SS_CURSORTYPE_BUTTON; break;
    case 4: wType = SS_CURSORTYPE_GRAYAREA; break;
    case 5: wType = SS_CURSORTYPE_LOCKEDCELL; break;
    case 6: wType = SS_CURSORTYPE_COLHEADER; break;
    case 7: wType = SS_CURSORTYPE_ROWHEADER; break;
    case 8: wType = SS_CURSORTYPE_DRAGDROPAREA; break;
    case 9: wType = SS_CURSORTYPE_DRAGDROP; break;
  }
  hCursor = SS_GetCursor(lpBook, wType);
  switch( (long)(LPVOID)hCursor )
  {
    case (long)SS_CURSOR_DEFAULT: nValue = 1; break;
    case (long)SS_CURSOR_ARROW: nValue = 2; break;
    case (long)SS_CURSOR_DEFCOLRESIZE: nValue = 3; break;
    case (long)SS_CURSOR_DEFROWRESIZE: nValue = 4; break;
  }
  if( m_lppictCursorIcon )
    m_lppictCursorIcon->Release();
  m_lppictCursorIcon = m_lppictCursors[m_nCursorType];
  if( m_lppictCursorIcon )
    m_lppictCursorIcon->AddRef();
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CURSORSTYLE
void CSsocxCtrl::SetCursorStyle(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  WORD wType = SS_CURSORTYPE_DEFAULT;
  HCURSOR hCursor = SS_CURSOR_DEFAULT;
  BOOL bCreatedCursor = FALSE;
  if( 0 <= nNewValue && nNewValue <= 4 )
  {
    switch( m_nCursorType )
    {
      case 0: wType = SS_CURSORTYPE_DEFAULT; break;
      case 1: wType = SS_CURSORTYPE_COLRESIZE; break;
      case 2: wType = SS_CURSORTYPE_ROWRESIZE; break;
      case 3: wType = SS_CURSORTYPE_BUTTON; break;
      case 4: wType = SS_CURSORTYPE_GRAYAREA; break;
      case 5: wType = SS_CURSORTYPE_LOCKEDCELL; break;
      case 6: wType = SS_CURSORTYPE_COLHEADER; break;
      case 7: wType = SS_CURSORTYPE_ROWHEADER; break;
      case 8: wType = SS_CURSORTYPE_DRAGDROPAREA; break;
      case 9: wType = SS_CURSORTYPE_DRAGDROP; break;
    }
    switch( nNewValue )
    {
      case 1: hCursor = SS_CURSOR_DEFAULT; break;
      case 2: hCursor = SS_CURSOR_ARROW; break;
      case 3: hCursor = SS_CURSOR_DEFCOLRESIZE; break;
      case 4: hCursor = SS_CURSOR_DEFROWRESIZE; break;
      default:
        if( m_lppictCursorIcon )
          m_lppictCursorIcon->get_Handle((OLE_HANDLE FAR*)&hCursor);
        if( hCursor )
        {
          bCreatedCursor = TRUE;
          hCursor = OleIconToCursor(AfxGetInstanceHandle(), hCursor);
        }
        break;
    }
    SS_SetCursor(lpBook, wType, hCursor, FALSE);
    if( m_lppictCursors[m_nCursorType] )
      m_lppictCursors[m_nCursorType]->Release();
    if( nNewValue )
      m_lppictCursors[m_nCursorType] = NULL;
    else
      m_lppictCursors[m_nCursorType] = m_lppictCursorIcon;
    if( m_lppictCursors[m_nCursorType] )
      m_lppictCursors[m_nCursorType]->AddRef();
    if( m_hcurCursors[m_nCursorType] )
      DestroyCursor(m_hcurCursors[m_nCursorType]);
    if( bCreatedCursor )
      m_hcurCursors[m_nCursorType] = hCursor;
    else
      m_hcurCursors[m_nCursorType] = NULL;
  }
  UnlockBook();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CURSORTYPE
short CSsocxCtrl::GetCursorType() 
{
  GET_NODESIGN();
  return m_nCursorType;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CURSORTYPE
void CSsocxCtrl::SetCursorType(short nNewValue) 
{
#ifdef SS_V30
  if( 0 <= nNewValue && nNewValue <= 9 )
#else
  if( 0 <= nNewValue && nNewValue <= 7 )
#endif
    m_nCursorType = nNewValue;
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DATACOLCNT
long CSsocxCtrl::GetDataColCnt() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lColCnt, lRowCnt;
  /* RFW - 6/29/04 - Countrywide
  SSGetDataCnt(lpSS->lpBook->hWnd, &lColCnt, &lRowCnt);
  */
  SS_GetDataCnt(lpSS, &lColCnt, &lRowCnt);
  SS_AdjustCellCoordsOut(lpSS, &lColCnt, NULL);
  lColCnt = max(0, lColCnt - 1);

  UnlockSheet();
  return lColCnt;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DATAFIELD
BSTR CSsocxCtrl::GetDataField() 
{
  GET_NODESIGN();

//DBOCX...
  BSTR bstrReturn = NULL;
  LPSPREADSHEET lpSS = LockSheet();

  if (m_lCol == SS_HEADER || (m_lCol >= -1L && m_lCol <= SS_GetMaxCols(lpSS)))
  {
     TBGLOBALHANDLE  hFieldName;
     if (hFieldName = SS_GetColFieldName(lpSS, m_lCol))
     {
        bstrReturn = 
          (BSTR)fpSysAllocBSTRFromTString((LPTSTR)tbGlobalLock(hFieldName));
        tbGlobalUnlock(hFieldName);
     }
     else
     {
        HSZ hszFieldName = VBSSBoundGetFieldName(lpSS, (LONG)this, m_hWnd, m_lCol);
        bstrReturn = 
          (BSTR)fpSysAllocBSTRFromTString((LPTSTR)hszFieldName);
        ssVBDestroyHsz(hszFieldName);
     }
  }

  UnlockSheet();
  return bstrReturn;
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DATAFIELD
void CSsocxCtrl::SetDataField(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
//DBOCX...
  LPSPREADSHEET lpSS = LockSheet();
  if (SS_SetColFieldName(lpSS, m_lCol, (LPTSTR)lpszNewValue))
  {
     INVALIDATE_CONTROL();
     SetModifiedFlag();
#ifdef FP_ADB
#ifdef SS_V70
     if( lpSS->pfpOleBinding )
     {
	     ((CfpOleBinding*)lpSS->pfpOleBinding)->fpReleaseDataBinding();
	     ((CfpOleBinding*)lpSS->pfpOleBinding)->fpInitDataBinding();
     }
#else
	 m_fpOleBinding.fpReleaseDataBinding();
	 m_fpOleBinding.fpInitDataBinding();
#endif
#endif
  }
  UnlockSheet();
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DATAFILLEVENT
BOOL CSsocxCtrl::GetDataFillEvent() 
{
  GET_NODESIGN();
//DBOCX...
  LPSS_COL      lpCol;
  BOOL fReturn = FALSE;
  LPSPREADSHEET lpSS = LockSheet();

  fReturn = (lpSS->fDataFillEvent ? -1 : 0);

  if (m_lCol != -1 && (lpCol = SS_LockColItem(lpSS, m_lCol)))
  {
     if (lpCol->bDataFillEvent == SS_DATAFILLEVENT_YES)
        fReturn = TRUE;
     else if (lpCol->bDataFillEvent == SS_DATAFILLEVENT_NO)
        fReturn = FALSE;
     else
        fReturn = lpSS->fDataFillEvent;

     SS_UnlockColItem(lpSS, m_lCol);
  }

  UnlockSheet();

  return fReturn;
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DATAFILLEVENT
void CSsocxCtrl::SetDataFillEvent(BOOL bNewValue) 
{
//DBOCX...
  LPSS_COL      lpCol;
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fModified = TRUE;

  if (m_lCol == -1)
  {
     SS_COORD c;

     lpSS->fDataFillEvent = (bNewValue ? TRUE : FALSE);

     // RFW - 4/29/99 - SPR_DAO_001_001

     for( c = lpSS->Col.HeaderCnt; c < lpSS->Col.AllocCnt; c++ )
        if (lpCol = SS_AllocLockCol(lpSS, c))
        {
           lpCol->bDataFillEvent = SS_DATAFILLEVENT_DEF;
           SS_UnlockColItem(lpSS, c);
        }
  }
  else if (lpCol = SS_AllocLockCol(lpSS, m_lCol))
  {
     lpCol->bDataFillEvent = bNewValue ? SS_DATAFILLEVENT_YES :
                                         SS_DATAFILLEVENT_NO;
     SS_UnlockColItem(lpSS, m_lCol);
  }
  else 
     fModified = FALSE;

  UnlockSheet();
  if (fModified)
     SetModifiedFlag();
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DATAROWCNT
long CSsocxCtrl::GetDataRowCnt() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lColCnt, lRowCnt;
  /* RFW - 6/29/04 - Countrywide
  SSGetDataCnt(lpSS->lpBook->hWnd, &lColCnt, &lRowCnt);
  */
  SS_GetDataCnt(lpSS, &lColCnt, &lRowCnt);
  SS_AdjustCellCoordsOut(lpSS, NULL, &lRowCnt);
  lRowCnt = max(0, lRowCnt - 1);

  UnlockSheet();
  return lRowCnt;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOCELLTYPES
BOOL CSsocxCtrl::GetDAutoCellTypes() 
{
//DBOCX...

  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);
  BOOL fReturn = FALSE;

  if (lpSpread)
  {
    fReturn = lpSpread->DataOpts.fCellTypes;
    tbGlobalUnlock(m_hVBSpread);
  }

  return fReturn;
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOCELLTYPES
void CSsocxCtrl::SetDAutoCellTypes(BOOL bNewValue) 
{
//DBOCX...
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);

  if (lpSpread)
  {
    LPSPREADSHEET lpSS = LockSheet();

    lpSpread->DataOpts.fCellTypes = (bNewValue ? 1 : 0);
    INVALIDATE_CONTROL();
    UnlockSheet();
    tbGlobalUnlock(m_hVBSpread);
    SetModifiedFlag();
  }
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOFILL
BOOL CSsocxCtrl::GetDAutoFill()
{
//DBOCX...
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);
  BOOL fReturn = FALSE;

  if (lpSpread)
  {
    fReturn = lpSpread->DataOpts.fAutoFill;
    tbGlobalUnlock(m_hVBSpread);
  }

  return fReturn;
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOFILL
void CSsocxCtrl::SetDAutoFill(BOOL bNewValue) 
{
//DBOCX...
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);

  if (lpSpread)
  {
    LPSPREADSHEET lpSS = LockSheet();

    lpSpread->DataOpts.fAutoFill = (bNewValue ? 1 : 0);
    INVALIDATE_CONTROL();
    UnlockSheet();
    tbGlobalUnlock(m_hVBSpread);
    SetModifiedFlag();
    if( AmbientUserMode() == FALSE )
      BoundPropertyChanged(dispidDAutoFill);
  }
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOHEADINGS
BOOL CSsocxCtrl::GetDAutoHeadings() 
{
//DBOCX...
  // Lock memory to VBSpread and call function in VBBOUND.C
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);
  BOOL fReturn = FALSE;

  if (lpSpread)
  {
    fReturn = lpSpread->DataOpts.fHeadings;
    tbGlobalUnlock(m_hVBSpread);
  }
  return fReturn;
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOHEADINGS
void CSsocxCtrl::SetDAutoHeadings(BOOL bNewValue) 
{
//DBOCX...
  // Lock memory to VBSpread and call function in VBBOUND.C
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);

  if (lpSpread)
  {
    LPSPREADSHEET lpSS = LockSheet();

    lpSpread->DataOpts.fHeadings = (bNewValue ? 1 : 0);
    INVALIDATE_CONTROL();
    UnlockSheet();
    tbGlobalUnlock(m_hVBSpread);
    SetModifiedFlag();
    if( AmbientUserMode() == FALSE )
      BoundPropertyChanged(dispidDAutoHeadings);
  }
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOSAVE
BOOL CSsocxCtrl::GetDAutoSave() 
{
//DBOCX...
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);
  BOOL fReturn = FALSE;

  if (lpSpread)
  {
    fReturn = lpSpread->DataOpts.fAutoSave;
    tbGlobalUnlock(m_hVBSpread);
  }

  return fReturn;
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOSAVE
void CSsocxCtrl::SetDAutoSave(BOOL bNewValue) 
{
//DBOCX.
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);

  if (lpSpread)
  {
    LPSPREADSHEET lpSS = LockSheet();

    lpSpread->DataOpts.fAutoSave = (bNewValue ? 1 : 0);
//  INVALIDATE_CONTROL();      // not needed for DAutoSave
    UnlockSheet();
    tbGlobalUnlock(m_hVBSpread);
    SetModifiedFlag();
  }
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOSIZECOLS
//DBOCX...
short CSsocxCtrl::GetDAutoSizeCols() 
{
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);
  short sReturn = 0;

  if (lpSpread)
  {
    sReturn = (short)lpSpread->DataOpts.fSizeCols;
    tbGlobalUnlock(m_hVBSpread);
  }

  return sReturn;
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DAUTOSIZECOLS
//DBOCX.
void CSsocxCtrl::SetDAutoSizeCols(short sNewValue) 
{
  if ((sNewValue < 0) || (sNewValue > 2))
     return;

  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);

  if (lpSpread)
  {
    LPSPREADSHEET lpSS = LockSheet();

    lpSpread->DataOpts.fSizeCols = sNewValue;
    INVALIDATE_CONTROL();
    UnlockSheet();
    tbGlobalUnlock(m_hVBSpread);
    SetModifiedFlag();
  }
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DESTCOL
long CSsocxCtrl::GetDestCol() 
{
  GET_NODESIGN();
  SS_COORD lCol = m_lDestCol;
  LPSPREADSHEET lpSS = LockSheet();
  SS_AdjustCellCoordsOut(lpSS, &lCol, NULL);
  UnlockSheet();
  return lCol;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DESTCOL
void CSsocxCtrl::SetDestCol(long nNewValue) 
{
  if( -1 <= nNewValue && nNewValue <= SS_MAXCOORD )
  {
    LPSPREADSHEET lpSS = LockSheet();
    m_lDestCol = nNewValue;
    SS_AdjustCellCoords(lpSS, &m_lDestCol, NULL);
    SetModifiedFlag();
    UnlockSheet();
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DESTROW
long CSsocxCtrl::GetDestRow() 
{
  GET_NODESIGN();
  SS_COORD lRow = m_lDestRow;
  LPSPREADSHEET lpSS = LockSheet();
  SS_AdjustCellCoordsOut(lpSS, &lRow, NULL);
  UnlockSheet();
  return lRow;
  GET_NODESIGN();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DESTROW
void CSsocxCtrl::SetDestRow(long nNewValue) 
{
  if( -1 <= nNewValue && nNewValue <= SS_MAXCOORD )
  {
    LPSPREADSHEET lpSS = LockSheet();
    m_lDestRow = nNewValue;
    SS_AdjustCellCoords(lpSS, &m_lDestRow, NULL);
    SetModifiedFlag();
    UnlockSheet();
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DINFORMACTIVEROWCHANGE
BOOL CSsocxCtrl::GetDInformActiveRowChange() 
{
//DBOCX...
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);
  BOOL fReturn = FALSE;

  if (lpSpread)
  {
    fReturn = lpSpread->DataOpts.fAutoDataInform;
    tbGlobalUnlock(m_hVBSpread);
  }

  return fReturn;
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DINFORMACTIVEROWCHANGE
void CSsocxCtrl::SetDInformActiveRowChange(BOOL bNewValue) 
{
//DBOCX...
  LPVBSPREAD lpSpread = (LPVBSPREAD)tbGlobalLock(m_hVBSpread);

  if (lpSpread)
  {
    LPSPREADSHEET lpSS = LockSheet();

    lpSpread->DataOpts.fAutoDataInform = (bNewValue ? 1 : 0);
//  INVALIDATE_CONTROL();    // not needed for this property
    UnlockSheet();
    tbGlobalUnlock(m_hVBSpread);
    SetModifiedFlag();
  }
//DBOCX.
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DISPLAYCOLHEADERS
BOOL CSsocxCtrl::GetDisplayColHeaders() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_SHOWCOLHEADERS);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DISPLAYCOLHEADERS
void CSsocxCtrl::SetDisplayColHeaders(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_SHOWCOLHEADERS, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidDisplayColHeaders);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DISPLAYROWHEADERS
BOOL CSsocxCtrl::GetDisplayRowHeaders() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_SHOWROWHEADERS);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_DISPLAYROWHEADERS
void CSsocxCtrl::SetDisplayRowHeaders(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_SHOWROWHEADERS, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidDisplayRowHeaders);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_EDITENTERACTION
short CSsocxCtrl::GetEditEnterAction() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetEditEnterAction(lpBook);
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_EDITENTERACTION
void CSsocxCtrl::SetEditEnterAction(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetEditEnterAction(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidEditEnterAction);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_EDITMODE
BOOL CSsocxCtrl::GetEditMode() 
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  BOOL bValue = SS_GetEditMode(lpBook);
  UnlockBook();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_EDITMODE
void CSsocxCtrl::SetEditMode(BOOL bNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetEditMode(lpBook, bNewValue);
  UnlockBook();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_EDITMODEPERMANENT
BOOL CSsocxCtrl::GetEditModePermanent() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_EDITMODEPERMANENT);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_EDITMODEPERMANENT
void CSsocxCtrl::SetEditModePermanent(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_EDITMODEPERMANENT, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidEditModePermanent);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_EDITMODEREPLACE
BOOL CSsocxCtrl::GetEditModeReplace() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_EDITMODEREPLACE);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_EDITMODEREPLACE
void CSsocxCtrl::SetEditModeReplace(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_EDITMODEREPLACE, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidEditModeReplace);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FILENUM
short CSsocxCtrl::GetFileNum() 
{
  GET_NODESIGN();
  return m_nFileNum;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FILENUM
void CSsocxCtrl::SetFileNum(short nNewValue) 
{
  m_nFileNum = nNewValue;
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FLOATDEFCURRENCYCHAR
short CSsocxCtrl::GetFloatDefCurrencyChar() 
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  FLOATFORMAT format;
  SS_GetDefFloatFormat(lpBook, &format);
  UnlockBook();
  return (_TUCHAR)format.cCurrencySign;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FLOATDEFCURRENCYCHAR
void CSsocxCtrl::SetFloatDefCurrencyChar(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  FLOATFORMAT format;
  SSx_GetDefFloatFormat(lpBook, &format);
  format.cCurrencySign = (TCHAR)nNewValue;
  SS_SetDefFloatFormat(lpBook, &format);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FLOATDEFDECIMALCHAR
short CSsocxCtrl::GetFloatDefDecimalChar() 
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  FLOATFORMAT format;
  SS_GetDefFloatFormat(lpBook, &format);
  UnlockBook();
  return (_TUCHAR)format.cDecimalSign;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FLOATDEFDECIMALCHAR
void CSsocxCtrl::SetFloatDefDecimalChar(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  FLOATFORMAT format;
  SSx_GetDefFloatFormat(lpBook, &format);
  format.cDecimalSign = (TCHAR)nNewValue;
  SS_SetDefFloatFormat(lpBook, &format);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FLOATDEFSEPCHAR
short CSsocxCtrl::GetFloatDefSepChar() 
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  FLOATFORMAT format;
  SS_GetDefFloatFormat(lpBook, &format);
  UnlockBook();
  return (_TUCHAR)format.cSeparator;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FLOATDEFSEPCHAR
void CSsocxCtrl::SetFloatDefSepChar(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  FLOATFORMAT format;
  SSx_GetDefFloatFormat(lpBook, &format);
  format.cSeparator = (TCHAR)nNewValue;
  SS_SetDefFloatFormat(lpBook, &format);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FONT
LPFONTDISP CSsocxCtrl::GetFont() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_FONT font;
  SS_FONTID fontId;

  if( fontId = SS_GetFontId(lpSS, m_lCol, m_lRow) )
  {
    SS_GetFont(lpSS->lpBook, &font, fontId);
    CreateFont(&m_fontSS, &font.LogFont);
  }
  UnlockSheet();

  return m_fontSS.GetFontDispatch();
}
#endif


BOOL CSsocxCtrl::CreateFont(CFontHolder *font, LPLOGFONT lpLogFont)
{

  FONTDESC fontDesc;
  long lHeight;
  CDC dc;

  dc.CreateCompatibleDC(NULL);
  lHeight = lpLogFont->lfHeight;
  lHeight *= lHeight > 0 ? 72 : -72;
  lHeight *= 10000;
  lHeight /= dc.GetDeviceCaps(LOGPIXELSY);
  fontDesc.cbSizeofstruct = sizeof(fontDesc);
  // if necessary, convert from CHAR to WCHAR
  fontDesc.lpstrName = 
    (BSTR)fpSysAllocBSTRFromTString(lpLogFont->lfFaceName);
  fontDesc.cySize.Lo = lHeight;
  fontDesc.cySize.Hi = 0;
  fontDesc.sWeight = (short)lpLogFont->lfWeight;
  fontDesc.sCharset = lpLogFont->lfCharSet;
  fontDesc.fItalic = lpLogFont->lfItalic;
  fontDesc.fUnderline = lpLogFont->lfUnderline;
  fontDesc.fStrikethrough = lpLogFont->lfStrikeOut;
  font->InitializeFont(&fontDesc, NULL);

  if (fontDesc.lpstrName)
    fpSysFreeTString((LPTSTR)fontDesc.lpstrName);

  return TRUE;
}

//--------------------------------------------------------------------

//static const FONTDESC _fontdescSS =
//{
//  sizeof(FONTDESC), "MS Sans Serif", FONTSIZE(12), FW_BOLD,
//  ANSI_CHARSET, FALSE, FALSE, FALSE
//};

#ifndef SSP_NO_FONT
void CSsocxCtrl::SetFont(LPFONTDISP newValue) 
{
  ASSERT(newValue == NULL ||
         AfxIsValidAddress(newValue, sizeof(IDispatch), FALSE));
  //m_fontSS.InitializeFont(&_fontdescSS, newValue);
  m_fontSS.InitializeFont(NULL, newValue);
  OnFontChanged();

  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

void CSsocxCtrl::OnFontChanged()
{
  if (!m_fInsideCreateControlWindow)
  {
	 LPSPREADSHEET lpSS = LockSheet();
    if( m_bBlockMode )
      SS_SetFontRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                      m_fontSS.GetFontHandle(), FALSE);
    else
      SS_SetFontRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow,
                      m_fontSS.GetFontHandle(), FALSE);
    INVALIDATE_CONTROL();
    UnlockSheet();
  }
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetFontBold()
{
  GET_NODESIGN();
  BOOL bFontBold = FALSE;
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->get_Bold(&bFontBold);
    lpFontDisp->Release();
  }
  return bFontBold;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetFontBold(BOOL bFontBold)
{
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->put_Bold(bFontBold);
    OnFontChanged();
    SetModifiedFlag();
    lpFontDisp->Release();
  }
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetFontItalic()
{
  GET_NODESIGN();
  BOOL bFontItalic = FALSE;
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->get_Italic(&bFontItalic);
    lpFontDisp->Release();
  }
  return bFontItalic;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetFontItalic(BOOL bFontItalic)
{
  LPFONTDISP lpFontDisp = GetFont();

  if( lpFontDisp )
  {
    m_fontSS.m_pFont->put_Italic(bFontItalic);
    OnFontChanged();
    SetModifiedFlag();
    lpFontDisp->Release();
  }
}

//--------------------------------------------------------------------

BSTR CSsocxCtrl::GetFontName()
{
  GET_NODESIGN();
  BSTR strFontName = NULL;
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->get_Name(&strFontName);
    lpFontDisp->Release();
  }
  return strFontName;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetFontName(LPCTSTR strFontName)
{
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    BSTR bstr = (BSTR)fpSysAllocBSTRFromTString((LPTSTR)strFontName);
    m_fontSS.m_pFont->put_Name(bstr);
    SysFreeString(bstr);
    OnFontChanged();
    SetModifiedFlag();
    lpFontDisp->Release();
  }
}

//--------------------------------------------------------------------

float CSsocxCtrl::GetFontSize()
{
  GET_NODESIGN();
  VARIANTARG varCy;
  V_VT(&varCy) = VT_CY;
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->get_Size(&V_CY(&varCy));
    lpFontDisp->Release();
  }
  VARIANTARG varR4;
  VariantInit(&varR4);
  VariantChangeType(&varR4, &varCy, 0, VT_R4);
  return V_R4(&varR4);
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetFontSize(float fpFontSize)
{
  VARIANTARG varR4;
  V_VT(&varR4) = VT_R4;
  V_R4(&varR4) = fpFontSize;
  VARIANTARG varCy;
  VariantInit(&varCy);
  VariantChangeType(&varCy, &varR4, 0, VT_CY);
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->put_Size(V_CY(&varCy));
    OnFontChanged();
    SetModifiedFlag();
    lpFontDisp->Release();
  }
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetFontStrikethru()
{
  GET_NODESIGN();
  BOOL bFontStrikethru = FALSE;
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->get_Strikethrough(&bFontStrikethru);
    lpFontDisp->Release();
  }
  return bFontStrikethru;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetFontStrikethru(BOOL bFontStrikethru)
{
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->put_Strikethrough(bFontStrikethru);
    OnFontChanged();
    SetModifiedFlag();
    lpFontDisp->Release();
  }
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetFontUnderline()
{
  GET_NODESIGN();
  BOOL bFontUnderline = FALSE;
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->get_Underline(&bFontUnderline);
    lpFontDisp->Release();
  }
  return bFontUnderline;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetFontUnderline(BOOL bFontUnderline)
{
  LPFONTDISP lpFontDisp = GetFont();
  if( lpFontDisp )
  {
    m_fontSS.m_pFont->put_Underline(bFontUnderline);
    OnFontChanged();
    SetModifiedFlag();
    lpFontDisp->Release();
  }
}

//--------------------------------------------------------------------

#ifndef SSP_NO_FORECOLOR
OLE_COLOR CSsocxCtrl::GetForeColor() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crBackColor, crForeColor;
  SS_GetColor(lpSS, m_lCol, m_lRow, &crBackColor, &crForeColor);
  UnlockSheet();
  return crForeColor;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FORECOLOR
void CSsocxCtrl::SetForeColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  if( m_bBlockMode )
    SS_SetColorRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                     SPREAD_COLOR_IGNORE, newValue);
  else
    SS_SetColorRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow,
                     SPREAD_COLOR_IGNORE, newValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FORMULA
BSTR CSsocxCtrl::GetFormula() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  CString s;
  #ifdef SS_OLDCALC
  TBGLOBALHANDLE hFormula;
  LPTSTR lpszFormula;
  if( hFormula = SS_GetFormula(lpSS, m_lCol, m_lRow) )
  {
    if( lpszFormula = (LPTSTR)tbGlobalLock(hFormula) )
    {
      s = lpszFormula;
      tbGlobalUnlock(hFormula);
    }
  }
  #else
  TCHAR szFormula[CALC_MAX_EXPR_LEN];
  SS_COORD lCol = m_lCol;
  SS_COORD lRow = m_lRow;
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
  if( CalcGetExpr(&lpSS->CalcInfo, lCol, lRow, 
#if SS_V80 // 24919 -scl
     FALSE,
#endif
     szFormula, CALC_MAX_EXPR_LEN) )
    s = szFormula;
  #endif
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FORMULA
void CSsocxCtrl::SetFormula(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  LPSPREADSHEET lpSS = LockSheet();
  if( m_bBlockMode )
    SS_SetFormulaRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                       lpszNewValue, m_bCalcDependencies);
  else
    SS_SetFormulaRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow,
                       lpszNewValue, m_bCalcDependencies);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FORMULASYNC
BOOL CSsocxCtrl::GetFormulaSync()
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_FORMULASYNC);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_FORMULASYNC
void CSsocxCtrl::SetFormulaSync(BOOL bNewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_FORMULASYNC, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidFormulaSync);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRAYAREABACKCOLOR
OLE_COLOR CSsocxCtrl::GetGrayAreaBackColor() 
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF crBackColor;
  SS_GetGrayAreaColor(lpBook, &crBackColor, NULL);
  UnlockBook();
  return crBackColor;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRAYAREABACKCOLOR
void CSsocxCtrl::SetGrayAreaBackColor(OLE_COLOR newValue) 
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF crBackColor, crForeColor;
  SS_GetGrayAreaColor(lpBook, &crBackColor, &crForeColor);
  SS_SetGrayAreaColor(lpBook, newValue, crForeColor);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE && crBackColor != newValue )
    BoundPropertyChanged(dispidGrayAreaBackColor);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRIDCOLOR
OLE_COLOR CSsocxCtrl::GetGridColor() 
{
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crRet = SS_GetGridColor(lpSS);
  UnlockSheet();
  return crRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRIDCOLOR
void CSsocxCtrl::SetGridColor(OLE_COLOR newValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crOldValue = SS_SetGridColor(lpSS, newValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE && crOldValue != newValue )
    BoundPropertyChanged(dispidGridColor);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRIDSHOWHORIZ
BOOL CSsocxCtrl::GetGridShowHoriz() 
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wType = SS_GetGridType(lpSS);
  UnlockSheet();
  return (wType & SS_GRID_HORIZONTAL) != 0;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRIDSHOWHORIZ
void CSsocxCtrl::SetGridShowHoriz(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wType = SS_GetGridType(lpSS);
  if( bNewValue )
    wType |= SS_GRID_HORIZONTAL;
  else
    wType &= ~SS_GRID_HORIZONTAL;
  SS_SetGridType(lpSS, wType);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidGridShowHoriz);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRIDSHOWVERT
BOOL CSsocxCtrl::GetGridShowVert() 
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wType = SS_GetGridType(lpSS);
  UnlockSheet();
  return (wType & SS_GRID_VERTICAL) != 0;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRIDSHOWVERT
void CSsocxCtrl::SetGridShowVert(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wType = SS_GetGridType(lpSS);
  if( bNewValue )
    wType |= SS_GRID_VERTICAL;
  else
    wType &= ~SS_GRID_VERTICAL;
  SS_SetGridType(lpSS, wType);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidGridShowVert);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRIDSOLID
BOOL CSsocxCtrl::GetGridSolid() 
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wType = SS_GetGridType(lpSS);
  UnlockSheet();
  return (wType & SS_GRID_SOLID) != 0;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_GRIDSOLID
void CSsocxCtrl::SetGridSolid(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wType = SS_GetGridType(lpSS);
  if( bNewValue )
    wType |= SS_GRID_SOLID;
  else
    wType &= ~SS_GRID_SOLID;
  SS_SetGridType(lpSS, wType);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidGridSolid);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_HDCPRINTER
OLE_HANDLE CSsocxCtrl::GetHDCPrinter() 
{
  GET_NODESIGN();
  return (OLE_HANDLE)m_hDCPrinter;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_HDCPRINTER
void CSsocxCtrl::SetHDCPrinter(OLE_HANDLE newValue) 
{
  m_hDCPrinter = (HDC)newValue;
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_HWND
OLE_HANDLE CSsocxCtrl::GetHWnd() 
{
  GET_NODESIGN();
  return (OLE_HANDLE)m_hWnd;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ISBLOCKSELECTED
BOOL CSsocxCtrl::GetIsBlockSelected() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetSelectBlock(lpSS, NULL, NULL);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_LEFTCOL
long CSsocxCtrl::GetLeftCol() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol, lRow;
  SS_GetTopLeftCell(lpSS, &lCol, &lRow);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
  UnlockSheet();
  return lCol;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_LEFTCOL
void CSsocxCtrl::SetLeftCol(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol, lRow;
  SS_GetTopLeftCell(lpSS, &lCol, &lRow);
  lpSS->fShowCellCalled = TRUE;
  SS_AdjustCellCoords(lpSS, &nNewValue, NULL);
  SS_ShowCell(lpSS, nNewValue, lRow, SS_SHOW_TOPLEFT);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_LOCK
BOOL CSsocxCtrl::GetLock() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  LPSS_COL lpCol = SS_LockColItem(lpSS, m_lCol);
  LPSS_ROW lpRow = SS_LockRowItem(lpSS, m_lRow);
  BOOL bValue = SSxx_GetLock(lpSS, lpCol, lpRow, NULL, m_lCol, m_lRow, TRUE);
  SS_UnlockColItem(lpSS, m_lCol);
  SS_UnlockRowItem(lpSS, m_lRow);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_LOCK
void CSsocxCtrl::SetLock(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  if( m_bBlockMode )
    SS_SetLockRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2, bNewValue);
  else
    SS_SetLockRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_LOCKBACKCOLOR
OLE_COLOR CSsocxCtrl::GetLockBackColor() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crBackColor, crForeColor;
  SS_GetLockColor(lpSS, &crBackColor, &crForeColor);
  UnlockSheet();
  return crBackColor;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_LOCKBACKCOLOR
void CSsocxCtrl::SetLockBackColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crBackColor, crForeColor;
  SS_GetLockColor(lpSS, &crBackColor, &crForeColor);
  SS_SetLockColor(lpSS, newValue, crForeColor);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_LOCKFORECOLOR
OLE_COLOR CSsocxCtrl::GetLockForeColor() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crBackColor, crForeColor;
  SS_GetLockColor(lpSS, &crBackColor, &crForeColor);
  UnlockSheet();
  return crForeColor;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_LOCKFORECOLOR
void CSsocxCtrl::SetLockForeColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crBackColor, crForeColor;
  SS_GetLockColor(lpSS, &crBackColor, &crForeColor);
  SS_SetLockColor(lpSS, crBackColor, newValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MAXCOLS
long CSsocxCtrl::GetMaxCols() 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = SS_GetMaxCols(lpSS);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MAXCOLS
void CSsocxCtrl::SetMaxCols(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
//  nNewValue += lpSS->Col.HeaderCnt - 1;
  SS_SetMaxCols(lpSS, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidMaxCols);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MAXROWS
long CSsocxCtrl::GetMaxRows() 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = SS_GetMaxRows(lpSS);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MAXROWS
void CSsocxCtrl::SetMaxRows(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
//  nNewValue += lpSS->Row.HeaderCnt - 1;
  SS_SetMaxRows(lpSS, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidMaxRows);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MAXTEXTCELLHEIGHT
double CSsocxCtrl::GetMaxTextCellHeight() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  double dfWidth, dfHeight;
  SS_GetMaxTextCellSize(lpSS, m_lCol, m_lRow, &dfWidth, &dfHeight);
  UnlockSheet();
  return dfHeight;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MAXTEXTCELLWIDTH
double CSsocxCtrl::GetMaxTextCellWidth() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  double dfWidth, dfHeight;
  SS_GetMaxTextCellSize(lpSS, m_lCol, m_lRow, &dfWidth, &dfHeight);
  UnlockSheet();
  return dfWidth;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MAXTEXTCOLWIDTH
double CSsocxCtrl::GetMaxTextColWidth(long lCol) 
{
  LPSPREADSHEET lpSS = LockSheet();
  double dfWidth;
  SS_AdjustCellCoords(lpSS, &lCol, NULL);
  SS_GetMaxTextColWidth(lpSS, lCol, &dfWidth);
  UnlockSheet();
  return dfWidth;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MAXTEXTROWHEIGHT
double CSsocxCtrl::GetMaxTextRowHeight(long lRow) 
{
  LPSPREADSHEET lpSS = LockSheet();
  double dfHeight;
  SS_AdjustCellCoords(lpSS, NULL, &lRow);
  SS_GetMaxTextRowHeight(lpSS, lRow, &dfHeight);
  UnlockSheet();
  return dfHeight;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MOVEACTIVEONFOCUS
BOOL CSsocxCtrl::GetMoveActiveOnFocus() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_MOVEACTIVEONFOCUS);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MOVEACTIVEONFOCUS
void CSsocxCtrl::SetMoveActiveOnFocus(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_MOVEACTIVEONFOCUS, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidMoveActiveOnFocus);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MULTISELCOUNT
long CSsocxCtrl::GetMultiSelCount() 
{
  GET_NODESIGN();
  return m_nMultiSelCount;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MULTISELINDEX
long CSsocxCtrl::GetMultiSelIndex() 
{
  GET_NODESIGN();
  return m_nMultiSelIndex;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_MULTISELINDEX
void CSsocxCtrl::SetMultiSelIndex(long nNewValue) 
{
  if( m_hMultiSelBlocks && 0 <= nNewValue && nNewValue < m_nMultiSelCount )
  {
    LPSS_SELBLOCK lpMultiSelBlocks;
    if( lpMultiSelBlocks = (LPSS_SELBLOCK)GlobalLock(m_hMultiSelBlocks) )
    {
      m_nMultiSelIndex = (short)nNewValue;
      m_lCol = lpMultiSelBlocks[m_nMultiSelIndex].UL.Col;
      m_lRow = lpMultiSelBlocks[m_nMultiSelIndex].UL.Row;
      m_lCol2 = lpMultiSelBlocks[m_nMultiSelIndex].LR.Col;
      m_lRow2 = lpMultiSelBlocks[m_nMultiSelIndex].LR.Row;
      GlobalUnlock(m_hMultiSelBlocks);
    }
  }
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_NOBEEP
BOOL CSsocxCtrl::GetNoBeep() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_NOBEEP);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_NOBEEP
void CSsocxCtrl::SetNoBeep(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_NOBEEP, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidNoBeep);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_NOBORDER
BOOL CSsocxCtrl::GetNoBorder() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_NOBORDER);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_NOBORDER
void CSsocxCtrl::SetNoBorder(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_NOBORDER, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidNoBorder);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_OPERATIONMODE
short CSsocxCtrl::GetOperationMode() 
{
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = SS_GetOperationMode(lpSS);
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_OPERATIONMODE
void CSsocxCtrl::SetOperationMode(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetOperationMode(lpSS, nNewValue );
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidOperationMode);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_POSITION
short CSsocxCtrl::GetPosition() 
{
  return m_nPosition;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_POSITION
void CSsocxCtrl::SetPosition(short nNewValue) 
{
  if( 0 <= nNewValue && nNewValue <= 8 )
    m_nPosition = nNewValue;
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidPosition);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTABORTMSG
BSTR CSsocxCtrl::GetPrintAbortMsg() 
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  CString s("");
  if (lpBook->hPrintAbortMsg)
  {
    LPTSTR lpszText = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);
    s = lpszText;
    tbGlobalUnlock(lpBook->hPrintAbortMsg);
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTABORTMSG
void CSsocxCtrl::SetPrintAbortMsg(LPCTSTR lpszNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetPrintAbortMsg(lpBook, (LPTSTR)lpszNewValue);
  SetModifiedFlag();
  UnlockBook();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTBORDER
BOOL CSsocxCtrl::GetPrintBorder() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fDrawBorder;
  UnlockSheet();
  return fValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTBORDER
void CSsocxCtrl::SetPrintBorder(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fDrawBorder = bNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTCOLHEADERS
BOOL CSsocxCtrl::GetPrintColHeaders() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fShowColHeaders;
  UnlockSheet();
  return fValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTCOLHEADERS
void CSsocxCtrl::SetPrintColHeaders(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fShowColHeaders = bNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTCOLOR
BOOL CSsocxCtrl::GetPrintColor() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fDrawColors;
  UnlockSheet();
  return fValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTCOLOR
void CSsocxCtrl::SetPrintColor(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fDrawColors = bNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTFOOTER
BSTR CSsocxCtrl::GetPrintFooter() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  CString s("");
  if (lpSS->PrintOptions.pf.hFooter)
  {
    LPTSTR lpszText = (LPTSTR)GlobalLock(lpSS->PrintOptions.pf.hFooter);
    s = lpszText;
    GlobalUnlock(lpSS->PrintOptions.pf.hFooter);
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTFOOTER
void CSsocxCtrl::SetPrintFooter(LPCTSTR lpszNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetPrintFooter(lpSS, (LPTSTR)lpszNewValue);
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTGRID
BOOL CSsocxCtrl::GetPrintGrid() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fShowGrid;
  UnlockSheet();
  return fValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTGRID
void CSsocxCtrl::SetPrintGrid(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fShowGrid = bNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTHEADER
BSTR CSsocxCtrl::GetPrintHeader() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  CString s("");
  if (lpSS->PrintOptions.pf.hHeader)
  {
    LPTSTR lpszText = (LPTSTR)GlobalLock(lpSS->PrintOptions.pf.hHeader);
    s = lpszText;
    GlobalUnlock(lpSS->PrintOptions.pf.hHeader);
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTHEADER
void CSsocxCtrl::SetPrintHeader(LPCTSTR lpszNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetPrintHeader(lpSS, (LPTSTR)lpszNewValue);
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTJOBNAME
BSTR CSsocxCtrl::GetPrintJobName() 
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  CString s("");
  if (lpBook->hPrintJobName)
  {
    LPTSTR lpszText = (LPTSTR)tbGlobalLock(lpBook->hPrintJobName);
    s = lpszText;
    tbGlobalUnlock(lpBook->hPrintJobName);
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTJOBNAME
void CSsocxCtrl::SetPrintJobName(LPCTSTR lpszNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetPrintJobName(lpBook, (LPTSTR)lpszNewValue);
  SetModifiedFlag();
  UnlockBook();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTMARGINBOTTOM
long CSsocxCtrl::GetPrintMarginBottom() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = (long)(lpSS->PrintOptions.pf.fMarginBottom * (float)1440.0);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTMARGINBOTTOM
void CSsocxCtrl::SetPrintMarginBottom(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fMarginBottom = (float)nNewValue / (float)1440.0;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTMARGINLEFT
long CSsocxCtrl::GetPrintMarginLeft() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = (long)(lpSS->PrintOptions.pf.fMarginLeft * (float)1440.0);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTMARGINLEFT
void CSsocxCtrl::SetPrintMarginLeft(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fMarginLeft = (float)nNewValue / (float)1440.0;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTMARGINRIGHT
long CSsocxCtrl::GetPrintMarginRight() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = (long)(lpSS->PrintOptions.pf.fMarginRight * (float)1440.0);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTMARGINRIGHT
void CSsocxCtrl::SetPrintMarginRight(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fMarginRight = (float)nNewValue / (float)1440.0;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTMARGINTOP
long CSsocxCtrl::GetPrintMarginTop() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = (long)(lpSS->PrintOptions.pf.fMarginTop * (float)1440.0);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTMARGINTOP
void CSsocxCtrl::SetPrintMarginTop(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fMarginTop = (float)nNewValue / (float)1440.0;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTORIENTATION
short CSsocxCtrl::GetPrintOrientation() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = lpSS->PrintOptions.pf.wOrientation;
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTORIENTATION
void CSsocxCtrl::SetPrintOrientation(short nNewValue) 
{
  if( 0 <= nNewValue && nNewValue <= 2 )
  {
    LPSPREADSHEET lpSS = LockSheet();
    lpSS->PrintOptions.pf.wOrientation = nNewValue;
    SetModifiedFlag();
    UnlockSheet();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTPAGEEND
short CSsocxCtrl::GetPrintPageEnd() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = lpSS->PrintOptions.pf.nPageEnd;
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTPAGEEND
void CSsocxCtrl::SetPrintPageEnd(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.nPageEnd = nNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTPAGESTART
short CSsocxCtrl::GetPrintPageStart() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = lpSS->PrintOptions.pf.nPageStart;
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTPAGESTART
void CSsocxCtrl::SetPrintPageStart(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.nPageStart = nNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTROWHEADERS
BOOL CSsocxCtrl::GetPrintRowHeaders() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fShowRowHeaders;
  UnlockSheet();
  return fValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTROWHEADERS
void CSsocxCtrl::SetPrintRowHeaders(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fShowRowHeaders = bNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTSHADOWS
BOOL CSsocxCtrl::GetPrintShadows() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fDrawShadows;
  UnlockSheet();
  return fValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTSHADOWS
void CSsocxCtrl::SetPrintShadows(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fDrawShadows = bNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTTYPE
short CSsocxCtrl::GetPrintType() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = 0;
  if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_CELLRANGE)
    nValue = 1;
  else if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_CURRENTPAGE)
    nValue = 2;
  else if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_PAGERANGE)
    nValue = 3;

  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTTYPE
void CSsocxCtrl::SetPrintType(short nNewValue) 
{
  //Add by BOC 99.5.13 (hyt)--------------------------
  //For ensure invalidate value can't input
  if (nNewValue <0 || nNewValue > 3 )
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    LPSPREADSHEET lpSS = LockSheet();
    short dPrintType = lpSS->PrintOptions.pf.dPrintType & ~(SS_PRINTTYPE_CELLRANGE | SS_PRINTTYPE_CURRENTPAGE | SS_PRINTTYPE_PAGERANGE);

    switch (nNewValue)
    {
      case 1: dPrintType |= SS_PRINTTYPE_CELLRANGE; break;
      case 2: dPrintType |= SS_PRINTTYPE_CURRENTPAGE; break;
      case 3: dPrintType |= SS_PRINTTYPE_PAGERANGE; break;
    }
    lpSS->PrintOptions.pf.dPrintType = dPrintType;
    SetModifiedFlag();
    UnlockSheet();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTUSEDATAMAX
BOOL CSsocxCtrl::GetPrintUseDataMax()
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fUseDataMax;
  UnlockSheet();
  return fValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PRINTUSEDATAMAX
void CSsocxCtrl::SetPrintUseDataMax(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fUseDataMax = bNewValue;
  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifdef SS_V30
BOOL CSsocxCtrl::GetPrintSmartPrint()
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fSmartPrint;
  UnlockSheet();
  return fValue;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetPrintSmartPrint(BOOL bNewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fSmartPrint = bNewValue;
  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetPrintPageOrder()
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = 0;
  if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_DOWNTHENOVER)
    nValue = 1;
  else if (lpSS->PrintOptions.pf.dPrintType & SS_PRINTTYPE_OVERTHENDOWN)
    nValue = 2;

  UnlockSheet();
  return nValue;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetPrintPageOrder(short nNewValue)
{
  if (nNewValue <0 || nNewValue > 2 )
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    LPSPREADSHEET lpSS = LockSheet();
    short dPrintType = lpSS->PrintOptions.pf.dPrintType & ~(SS_PRINTTYPE_DOWNTHENOVER | SS_PRINTTYPE_OVERTHENDOWN);

    switch (nNewValue)
    {
      case 1: dPrintType |= SS_PRINTTYPE_DOWNTHENOVER; break;
      case 2: dPrintType |= SS_PRINTTYPE_OVERTHENDOWN; break;
    }
    lpSS->PrintOptions.pf.dPrintType = dPrintType;
    SetModifiedFlag();
    UnlockSheet();
  }
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetPrintPageCount()
{
  PRVW_INFO PrvwInfo;
  GET_NODESIGN();

  OnPrvwmGetInfo(0, (LONG)(LPVOID)&PrvwInfo);
  return PrvwInfo.lPageEnd - PrvwInfo.lPageBeg + 1;
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetPrintFirstPageNumber()
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = lpSS->PrintOptions.pf.lFirstPageNumber;
  UnlockSheet();
  return lValue;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetPrintFirstPageNumber(long lNewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.lFirstPageNumber = lNewValue;
  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetPrintNextPageBreakCol()
{
  SS_PRINTPAGECALC PrintPageCalc;

  GET_NODESIGN();

  LPSPREADSHEET lpSS = LockSheet();
  PrintPageCalc.lPageCnt = -1;
  PrintSpread(SS_PRINT_ACTION_NEXTCOL, 0, NULL, m_lPrintNextPageBreakCol,
              &PrintPageCalc);
  m_lPrintNextPageBreakCol = PrintPageCalc.lPageCnt;
  SS_AdjustCellCoordsOut(lpSS, &PrintPageCalc.lPageCnt, NULL);
  UnlockSheet();

  return (PrintPageCalc.lPageCnt);
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetPrintNextPageBreakCol(long lNewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_AdjustCellCoords(lpSS, &lNewValue, NULL);
  m_lPrintNextPageBreakCol = lNewValue;
  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetPrintNextPageBreakRow()
{
  SS_PRINTPAGECALC PrintPageCalc;

  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();

  PrintPageCalc.lPageCnt = -1;
  PrintSpread(SS_PRINT_ACTION_NEXTROW, 0, NULL, m_lPrintNextPageBreakRow,
              &PrintPageCalc);
  m_lPrintNextPageBreakRow = PrintPageCalc.lPageCnt;
  SS_AdjustCellCoordsOut(lpSS, NULL, &PrintPageCalc.lPageCnt);
  UnlockSheet();

  return (PrintPageCalc.lPageCnt);
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetPrintNextPageBreakRow(long lNewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_AdjustCellCoords(lpSS, NULL, &lNewValue);
  m_lPrintNextPageBreakRow = lNewValue;
  SetModifiedFlag();
  UnlockSheet();
}

#endif // SS_V30

//--------------------------------------------------------------------

#ifndef SSP_NO_PROCESSTAB
BOOL CSsocxCtrl::GetProcessTab() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_PROCESSTAB);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PROCESSTAB
void CSsocxCtrl::SetProcessTab(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_PROCESSTAB, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidProcessTab);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PROTECT
BOOL CSsocxCtrl::GetProtect() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_PROTECT);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_PROTECT
void CSsocxCtrl::SetProtect(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_PROTECT, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidProtect);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_REDRAW
BOOL CSsocxCtrl::GetReDraw() 
{
  LPSPREADSHEET lpSS = LockSheet();
  GET_NODESIGN();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_REDRAW);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_REDRAW
void CSsocxCtrl::SetReDraw(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_REDRAW, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidReDraw);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_RESTRICTCOLS
BOOL CSsocxCtrl::GetRestrictCols() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_RESTRICTCOLS);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_RESTRICTCOLS
void CSsocxCtrl::SetRestrictCols(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_RESTRICTCOLS, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidRestrictCols);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_RESTRICTROWS
BOOL CSsocxCtrl::GetRestrictRows() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_RESTRICTROWS);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_RESTRICTROWS
void CSsocxCtrl::SetRestrictRows(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_RESTRICTROWS, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidRestrictRows);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_RETAINSELBLOCK
BOOL CSsocxCtrl::GetRetainSelBlock() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_RETAINSELBLOCK);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_RETAINSELBLOCK
void CSsocxCtrl::SetRetainSelBlock(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_RETAINSELBLOCK, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidRetainSelBlock);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROW
long CSsocxCtrl::GetRow() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lRow = m_lRow;
  GET_NODESIGN();
  SS_AdjustCellCoordsOut(lpSS, NULL, &lRow);
  UnlockSheet();
  return lRow;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROW
void CSsocxCtrl::SetRow(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  if( (-1 <= nNewValue && nNewValue <= SS_MAXCOORD) ||
      (SS_HEADER <= nNewValue && nNewValue < SS_HEADER + lpSS->Row.HeaderCnt))
  {
/* RAP01d
    LPSPREADSHEET lpSS = LockSheet();
// GIC8667 RFW 3/9/99    m_lRow = nNewValue;
    m_lRow = min(nNewValue, SS_GetMaxRows(lpSS));
    SetModifiedFlag();
    UnlockSheet();
*/
// RAP01a>>
    m_lRow = nNewValue;
	 SS_AdjustCellCoords(lpSS, NULL, &m_lRow);
    SetModifiedFlag();
//<<RAP01a
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);

  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROW2
long CSsocxCtrl::GetRow2() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lRow = m_lRow2;
  GET_NODESIGN();
  SS_AdjustCellCoordsOut(lpSS, NULL, &lRow);
  UnlockSheet();
  return lRow;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROW2
void CSsocxCtrl::SetRow2(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  if( (-1 <= nNewValue && nNewValue <= SS_MAXCOORD) ||
      (SS_HEADER <= nNewValue && nNewValue < SS_HEADER + lpSS->Row.HeaderCnt))
  {
/* RAP01d
    LPSPREADSHEET lpSS = LockSheet();
// GIC8667 RFW 3/9/99    m_lRow2 = nNewValue;
    m_lRow2 = min(nNewValue, SS_GetMaxRows(lpSS));
    SetModifiedFlag();
    UnlockSheet();
*/
// RAP01a>>    
    m_lRow2 = nNewValue;
	 SS_AdjustCellCoords(lpSS, NULL, &m_lRow2);
    SetModifiedFlag();
//<<RAP01a
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);

  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWSFROZEN
long CSsocxCtrl::GetRowsFrozen() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCols, lRows;
  SS_GetFreeze(lpSS, &lCols, &lRows);
  UnlockSheet();
  return lRows;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWSFROZEN
void CSsocxCtrl::SetRowsFrozen(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCols, lRows;
  SS_GetFreeze(lpSS, &lCols, &lRows);
  SS_SetFreeze(lpSS, lCols, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidRowsFrozen);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWHEADERDISPLAY
short CSsocxCtrl::GetRowHeaderDisplay() 
{
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = SS_GetRowHeaderDisplay(lpSS);
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWHEADERDISPLAY
void CSsocxCtrl::SetRowHeaderDisplay(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetRowHeaderDisplay(lpSS, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidRowHeaderDisplay);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWHEIGHT
double CSsocxCtrl::GetRowHeight(long lRow) 
{
  GET_NODESIGN();
  double dfHeight = 0.0;
  LPSPREADSHEET lpSS = LockSheet();

  if ( -2 <= lRow && lRow <= SS_MAXCOORD ||
      (SS_HEADER <= lRow && lRow < SS_HEADER + lpSS->Row.HeaderCnt))
  {
    if (lRow == -2)
      lRow = m_lRow;
    else
      SS_AdjustCellCoords(lpSS, NULL, &lRow);
    SS_GetRowHeight(lpSS, lRow, &dfHeight);
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYARRAYINDEX);

  UnlockSheet();
  return dfHeight;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWHEIGHT
void CSsocxCtrl::SetRowHeight(long lRow, double newValue) 
{
  LPSPREADSHEET lpSS = LockSheet();

  if ( -2 <= lRow && lRow <= SS_MAXCOORD ||
      (SS_HEADER <= lRow && lRow < SS_HEADER + lpSS->Row.HeaderCnt))
  {
    if (lRow == -2)
       lRow = m_lRow;
    else
      SS_AdjustCellCoords(lpSS, NULL, &lRow);
    SS_SetRowHeight(lpSS, lRow, newValue);
    INVALIDATE_CONTROL();
    SetModifiedFlag();
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYARRAYINDEX);

  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWHIDDEN
BOOL CSsocxCtrl::GetRowHidden() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL          fRet = FALSE;

  fRet = SS_IsRowHidden(lpSS, m_lRow) ? -1 : 0;

  UnlockSheet();
  return fRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWHIDDEN
void CSsocxCtrl::SetRowHidden(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_COORD r, r1, r2;

  // RFW - 4/28/99 - This was changed to allow BlockMode to work
  // with RowHidden

  if( m_bBlockMode )
  {
    r1 = m_lRow;
    r2 = m_lRow2;
  }
  else
    r1 = r2 = m_lRow;

  if( SS_ALLROWS == r1 || SS_ALLROWS == r2 )
    r1 = r2 = SS_ALLROWS;

  for( r = r1; r <= r2; r++ )
    SS_ShowRow(lpSS, r, !bNewValue);

  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWPAGEBREAK
BOOL CSsocxCtrl::GetRowPageBreak() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetRowPageBreak(lpSS, m_lRow);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ROWPAGEBREAK
void CSsocxCtrl::SetRowPageBreak(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetRowPageBreak(lpSS, m_lRow, bNewValue);
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBAREXTMODE
BOOL CSsocxCtrl::GetScrollBarExtMode() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_SCROLLBAREXTMODE);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBAREXTMODE
void CSsocxCtrl::SetScrollBarExtMode(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_SCROLLBAREXTMODE, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidScrollBarExtMode);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBARMAXALIGN
BOOL CSsocxCtrl::GetScrollBarMaxAlign() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_SCROLLBARMAXALIGN);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBARMAXALIGN
void CSsocxCtrl::SetScrollBarMaxAlign(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_SCROLLBARMAXALIGN, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidScrollBarMaxAlign);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBARS
short CSsocxCtrl::GetScrollBars() 
{
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = 0;
  if( SS_GetBool(lpSS->lpBook, lpSS, SSB_HORZSCROLLBAR) )
    nValue |= 1;
  if( SS_GetBool(lpSS->lpBook, lpSS, SSB_VERTSCROLLBAR) )
    nValue |= 2;
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBARS
void CSsocxCtrl::SetScrollBars(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bOldRedraw = SS_SetBool(lpSS->lpBook, lpSS, SSB_REDRAW, FALSE);
  SS_SetBool(lpSS->lpBook, lpSS, SSB_HORZSCROLLBAR, nNewValue & 1 ? TRUE : FALSE);
  SS_SetBool(lpSS->lpBook, lpSS, SSB_VERTSCROLLBAR, nNewValue & 2 ? TRUE : FALSE);
  SS_SetBool(lpSS->lpBook, lpSS, SSB_REDRAW, bOldRedraw);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidScrollBars);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBARSHOWMAX
BOOL CSsocxCtrl::GetScrollBarShowMax() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_SCROLLBARSHOWMAX);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBARSHOWMAX
void CSsocxCtrl::SetScrollBarShowMax(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_SCROLLBARSHOWMAX, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidScrollBarShowMax);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELBLOCKCOL
long CSsocxCtrl::GetSelBlockCol() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLCOORD cellUL, cellLR;
  SS_COORD lCol, lRow;
  if( SS_GetSelectBlock(lpSS, &cellUL, &cellLR) )
    lCol = cellUL.Col;
  else
    SS_GetActiveCell(lpSS, &lCol, &lRow);

  SS_AdjustCellCoordsOut(lpSS, &lCol, NULL);
  UnlockSheet();
  return lCol;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELBLOCKCOL2
long CSsocxCtrl::GetSelBlockCol2() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLCOORD cellUL, cellLR;
  SS_COORD lCol, lRow;
  if( SS_GetSelectBlock(lpSS, &cellUL, &cellLR) )
    lCol = cellLR.Col;
  else
    SS_GetActiveCell(lpSS, &lCol, &lRow);

  SS_AdjustCellCoordsOut(lpSS, &lCol, NULL);
  UnlockSheet();
  return lCol;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELBLOCKROW
long CSsocxCtrl::GetSelBlockRow() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLCOORD cellUL, cellLR;
  SS_COORD lCol, lRow;
  if( SS_GetSelectBlock(lpSS, &cellUL, &cellLR) )
    lRow = cellUL.Row;
  else
    SS_GetActiveCell(lpSS, &lCol, &lRow);

  SS_AdjustCellCoordsOut(lpSS, NULL, &lRow);
  UnlockSheet();
  return lRow;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELBLOCKROW2
long CSsocxCtrl::GetSelBlockRow2() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLCOORD cellUL, cellLR;
  SS_COORD lCol, lRow;
  if( SS_GetSelectBlock(lpSS, &cellUL, &cellLR) )
    lRow = cellLR.Row;
  else
    SS_GetActiveCell(lpSS, &lCol, &lRow);

  SS_AdjustCellCoordsOut(lpSS, NULL, &lRow);
  UnlockSheet();
  return lRow;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELECTBLOCKOPTIONS
#ifdef SS_V70
long CSsocxCtrl::GetSelectBlockOptions() 
{
  LPSS_BOOK lpBook = LockBook();
  long lValue = (long)SS_GetSelBlockOptions(lpBook);
  UnlockBook();
  return lValue;
}
#else
short CSsocxCtrl::GetSelectBlockOptions() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetSelBlockOptions(lpBook);
  UnlockBook();
  return nValue;
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELECTBLOCKOPTIONS
#ifdef SS_V70
void CSsocxCtrl::SetSelectBlockOptions(long lNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetSelBlockOptions(lpBook, (short)lNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidSelectBlockOptions);
}
#else
void CSsocxCtrl::SetSelectBlockOptions(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetSelBlockOptions(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidSelectBlockOptions);
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELLENGTH
long CSsocxCtrl::GetSelLength() 
{
  GET_NODESIGN();
  if( m_hWnd )
  {
    int nStart, nEnd;
    SSGetSel(m_hWnd, &nStart, &nEnd);
    return nEnd - nStart;
  }
  return 0;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELLENGTH
void CSsocxCtrl::SetSelLength(long nNewValue) 
{
  if( m_hWnd )
  {
    int nStart, nEnd;
    SSGetSel(m_hWnd, &nStart, &nEnd);
    SSSetSel(m_hWnd, nStart, nStart + (short)nNewValue);
  }
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELMODEINDEX
long CSsocxCtrl::GetSelModeIndex() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = SS_SelModeSendMessage(lpSS, SS_SELMODE_GETCURSEL, 0, 0, 0);
  SS_AdjustCellCoordsOut(lpSS, NULL, &lValue);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELMODEINDEX
void CSsocxCtrl::SetSelModeIndex(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_AdjustCellCoords(lpSS, NULL, &nNewValue);
  SS_SelModeSendMessage(lpSS, SS_SELMODE_SETCURSEL, nNewValue, 0, 0);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELMODESELCOUNT
long CSsocxCtrl::GetSelModeSelCount() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = SS_SelModeSendMessage(lpSS, SS_SELMODE_GETSELCOUNT, 0, 0, 0);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELMODESELECTED
BOOL CSsocxCtrl::GetSelModeSelected() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = (BOOL)SS_SelModeSendMessage(lpSS, SS_SELMODE_GETSEL,
                                            m_lRow, 0, 0);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELMODESELECTED
void CSsocxCtrl::SetSelModeSelected(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SelModeSendMessage(lpSS, SS_SELMODE_SETSEL, bNewValue, m_lRow, 0);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELSTART
long CSsocxCtrl::GetSelStart() 
{
  GET_NODESIGN();
  if( m_hWnd )
  {
    int nStart, nEnd;
    SSGetSel(m_hWnd, &nStart, &nEnd);
    return nStart;
  }
  return 0;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELSTART
void CSsocxCtrl::SetSelStart(long nNewValue) 
{
  if( m_hWnd )
  {
    int nStart, nEnd;
    SSGetSel(m_hWnd, &nStart, &nEnd);
    SSSetSel(m_hWnd, (short)nNewValue, (short)max(nNewValue,nEnd));
  }
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELTEXT
BSTR CSsocxCtrl::GetSelText() 
{
  GET_NODESIGN();
  CString s;
  GLOBALHANDLE hText;
  LPTSTR lpszText;
  if( m_hWnd )
  {
    if( hText = SSGetSelText(m_hWnd) )
    {
      if( lpszText = (LPTSTR)GlobalLock(hText) )
      {
        s = lpszText;
        GlobalUnlock(hText);
      }
      GlobalFree(hText);
    }
  }
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SELTEXT
void CSsocxCtrl::SetSelText(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  if( m_hWnd )
    SSReplaceSel(m_hWnd, lpszNewValue);
  SetModifiedFlag();
}
#endif

#ifdef SS_V80
#ifndef SSP_NO_CELLNOTEINDICATORSHAPE
short CSsocxCtrl::GetCellNoteIndicatorShape() 
{
  LPSS_BOOK lpBook = LockBook();
  short crRet = SS_GetCellNoteIndicatorShape(lpBook);
  UnlockBook();
  return crRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLNOTEINDICATORSHAPE
void CSsocxCtrl::SetCellNoteIndicatorShape(short newValue) 
{
  LPSS_BOOK lpBook = LockBook();
  short nOldValue = SS_SetCellNoteIndicatorShape(lpBook, newValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE && nOldValue != newValue )
    BoundPropertyChanged(dispidCellNoteIndicatorShape);
}
#endif

#ifndef SSP_NO_CELLNOTEINDICATORCOLOR
OLE_COLOR CSsocxCtrl::GetCellNoteIndicatorColor() 
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF crRet = SS_GetCellNoteIndicatorColor(lpBook);
  UnlockBook();
  return crRet;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_CELLNOTEINDICATORCOLOR
void CSsocxCtrl::SetCellNoteIndicatorColor(OLE_COLOR newValue) 
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF crOldValue = SS_SetCellNoteIndicatorColor(lpBook, newValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE && crOldValue != newValue )
    BoundPropertyChanged(dispidCellNoteIndicatorColor);
}
#endif

//--------------------------------------------------------------------
#ifndef SSP_NO_APPEARANCESTYLE
short CSsocxCtrl::GetAppearanceStyle() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetAppearanceStyle(lpBook);
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_APPEARANCESTYLE
void CSsocxCtrl::SetAppearanceStyle(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetAppearanceStyle(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAppearanceStyle);
}
#endif

#ifndef SSP_NO_SCROLLBARSTYLE
short CSsocxCtrl::GetScrollBarStyle() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetScrollBarStyle(lpBook);
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SCROLLBARSTYLE
void CSsocxCtrl::SetScrollBarStyle(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetScrollBarStyle(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidScrollBarStyle);
}
#endif


#ifndef SSP_NO_ENHANCESTATICCELLS
BOOL CSsocxCtrl::GetEnhanceStaticCells() 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL nValue = SS_GetEnhanceStaticCells(lpBook);
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_ENHANCESTATICCELLS
void CSsocxCtrl::SetEnhanceStaticCells(BOOL nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetEnhanceStaticCells(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidEnhanceStaticCells);
}
#endif


#ifndef SSP_NO_USEVISUALSTYLES
short CSsocxCtrl::GetUseVisualStyles() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetUseVisualStyles(lpBook);
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_USEVISUALSTYLES
void CSsocxCtrl::SetUseVisualStyles(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetUseVisualStyles(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidUseVisualStyles);
}
#endif


#ifndef SSP_NO_TABENHANCEDSHAPE
short CSsocxCtrl::GetTabEnhancedShape() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetTabEnhancedShape(lpBook);
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TABENHANCEDSHAPE
void CSsocxCtrl::SetTabEnhancedShape(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetTabEnhancedShape(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidTabEnhancedShape);
}
#endif

#ifndef SSP_NO_HIGHLIGHTHEADERS
short CSsocxCtrl::GetHighlightHeaders() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetHighlightHeaders(lpBook);
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_HIGHLIGHTHEADERS
void CSsocxCtrl::SetHighlightHeaders(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetHighlightHeaders(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidHighlightHeaders);
}
#endif

#ifndef SSP_NO_HIGHLIGHTSTYLE
short CSsocxCtrl::GetHighlightStyle() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetHighlightStyle(lpBook);
  UnlockBook();
  return nValue;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetHighlightStyle(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetHighlightStyle(lpBook, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidHighlightStyle);
}
#endif

#ifndef SSP_NO_HIGHLIGHTALPHABLENDCOLOR
OLE_COLOR CSsocxCtrl::GetHighlightAlphaBlendColor() 
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF clrValue;
  SS_GetHighlightAlphaBlend(lpBook, &clrValue, NULL);
  UnlockBook();
  return clrValue;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetHighlightAlphaBlendColor(OLE_COLOR newValue) 
{
  LPSS_BOOK lpBook = LockBook();
  short nAlphaBlend;

  SS_GetHighlightAlphaBlend(lpBook, NULL, &nAlphaBlend);
  SS_SetHighlightAlphaBlend(lpBook, newValue, nAlphaBlend);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidHighlightAlphaBlendColor);
}
#endif

#ifndef SSP_NO_HIGHLIGHTALPHABLEND
short CSsocxCtrl::GetHighlightAlphaBlend() 
{
  LPSS_BOOK lpBook = LockBook();
  short nValue;
  SS_GetHighlightAlphaBlend(lpBook, NULL, &nValue);
  UnlockBook();
  return nValue;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetHighlightAlphaBlend(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF clrHighlightAlphaBlend;

  SS_GetHighlightAlphaBlend(lpBook, &clrHighlightAlphaBlend, NULL);
  SS_SetHighlightAlphaBlend(lpBook, clrHighlightAlphaBlend, nNewValue);
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidHighlightAlphaBlend);
}
#endif
#endif // SS_V80