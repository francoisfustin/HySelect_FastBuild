// ssocxevt.cpp : Implementation of the CSsocxCtrl OLE control events.

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

#include "..\sssrc\spread\ss_alloc.h"
#include "..\sssrc\spread\ss_bool.h"
#include "..\sssrc\spread\ss_bord.h"
#include "..\sssrc\spread\ss_calc.h"
#include "..\sssrc\spread\ss_cell.h"
#include "..\sssrc\spread\ss_col.h"
#include "..\sssrc\spread\ss_curs.h"
#include "..\sssrc\spread\ss_data.h"
#include "..\sssrc\spread\ss_dde.h"
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

//DBOCX...
#include "ssocxdb.h"
#include "..\..\..\fplibs\fptools\src\fpconvrt.h"            // includes fpvbsub.h

#include "..\..\..\fplibs\fptools\src\fptstr.h"

extern "C" {
#include "..\vbsrc\vbspread.h"
} //extern "C"
//DBOCX.

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmCellLocked(UINT wParam, LONG lParam)
{
  LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
  SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
  SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;

  FireEditError(lCol, lRow, EDITERROR_CELLLOCKED);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmClick(UINT wParam, LONG lParam)
{
  LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
  SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
  SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;

  FireClick(lCol, lRow);
  if (lCol == -1 || lRow == -1)
	  return (TRUE);

  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmColRestricted(UINT wParam, LONG lParam)
{
  SS_COORD lCol = SS_HEADER == lParam ? 0 : lParam;

  FireEditError(lCol, -1, EDITERROR_COLRESTRICTED);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmColRowRestricted(UINT wParam, LONG lParam)
{
  LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
  SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
  SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;

  FireEditError(lCol, lRow, EDITERROR_COLROWRESTRICTED);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmColWidthChange(UINT wParam, LONG lParam)
{
  LPSS_COORDRANGE lpRange = (LPSS_COORDRANGE)lParam;
  SS_COORD lCoord1 = SS_HEADER == lpRange->Coord1 ? 0 : lpRange->Coord1;
  SS_COORD lCoord2 = SS_HEADER == lpRange->Coord2 ? 0 : lpRange->Coord2;

  #ifndef SSE_NO_COLWIDTHCHANGE
  FireColWidthChange(lCoord1, lCoord2);
  #endif
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmDataChange(UINT wParam, LONG lParam)
{
  LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
  SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
  SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;
  LPSPREADSHEET lpSS = LockSheetSendingMsg();

  FireChange(lCol, lRow);
//DBOCX...
  SSOcxLockVBSpread((LONG)this);
  SS_AdjustCellCoords(lpSS, &lCol, &lRow);
  VBSSBoundChangeMade(lpSS, (LONG)this, m_hWnd, lCol, lRow);
  SSOcxUnlockVBSpread((LONG)this);
//DBOCX.
  UnlockSheetSendingMsg();
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmDblClk(UINT wParam, LONG lParam)
{
  LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
  SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
  SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;

  FireDblClick(lCol, lRow);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmDragDrop(UINT wParam, LONG lParam)
{
  LPSS_DRAGDROP lpDragDrop = (LPSS_DRAGDROP)lParam;
  short nAction = (short)lpDragDrop->fCopy;
  BOOL bDataOnly = lpDragDrop->fDataOnly;
  BOOL bCancel = lpDragDrop->fCancel;
  
#ifdef SS_V35
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg[3];
     VARIANT varParam[3];

     memset(&arg, 0, sizeof(VARIANT) * 3);
     memset(&varParam, 0, sizeof(VARIANT) * 3);

     varParam[0].vt = VT_I2; 
     varParam[0].iVal  = nAction; 

     varParam[1].vt = VT_BOOL; 
     varParam[1].boolVal  = (bDataOnly ? VARIANT_TRUE : VARIANT_FALSE); 

     varParam[2].vt = VT_BOOL; 
     varParam[2].boolVal  = (bCancel ? VARIANT_TRUE : VARIANT_FALSE); 

     FireScriptDragDropBlock(lpDragDrop->BlockPrev.UL.Col, lpDragDrop->BlockPrev.UL.Row,
                       lpDragDrop->BlockPrev.LR.Col, lpDragDrop->BlockPrev.LR.Row,
                       lpDragDrop->BlockNew.UL.Col, lpDragDrop->BlockNew.UL.Row,
                       lpDragDrop->BlockNew.LR.Col, lpDragDrop->BlockNew.LR.Row,
                       lpDragDrop->fDataBeingOverwritten, &varParam[0],
                       &varParam[1], &varParam[2]);
     VariantChangeType(&arg[0], &varParam[0], 0, VT_I2); 
     VariantChangeType(&arg[1], &varParam[1], 0, VT_BOOL); 
     VariantChangeType(&arg[2], &varParam[2], 0, VT_BOOL); 
     nAction = arg[0].iVal;
     bDataOnly = (arg[1].boolVal == VARIANT_TRUE ? TRUE : FALSE);
     bCancel = (arg[2].boolVal == VARIANT_TRUE ? TRUE : FALSE);
     }
  else
     FireDragDropBlock(lpDragDrop->BlockPrev.UL.Col, lpDragDrop->BlockPrev.UL.Row,
                       lpDragDrop->BlockPrev.LR.Col, lpDragDrop->BlockPrev.LR.Row,
                       lpDragDrop->BlockNew.UL.Col, lpDragDrop->BlockNew.UL.Row,
                       lpDragDrop->BlockNew.LR.Col, lpDragDrop->BlockNew.LR.Row,
                       lpDragDrop->fDataBeingOverwritten, &nAction,
                       &bDataOnly, &bCancel);

  UnlockBook();
#else
  #ifndef SSE_NO_DRAGDROPBLOCK
  FireDragDropBlock(lpDragDrop->BlockPrev.UL.Col, lpDragDrop->BlockPrev.UL.Row,
                    lpDragDrop->BlockPrev.LR.Col, lpDragDrop->BlockPrev.LR.Row,
                    lpDragDrop->BlockNew.UL.Col, lpDragDrop->BlockNew.UL.Row,
                    lpDragDrop->BlockNew.LR.Col, lpDragDrop->BlockNew.LR.Row,
                    lpDragDrop->fDataBeingOverwritten, &nAction,
                    &bDataOnly, &bCancel);
  #endif
#endif

  lpDragDrop->fCopy = nAction;
  lpDragDrop->fDataOnly = bDataOnly;
  lpDragDrop->fCancel = bCancel;

//DBOCX...
  #ifndef SSE_NO_DRAGDROPBLOCK
  {
    LPSPREADSHEET lpSS = LockActiveSheet();
    SS_COORD Col = lpDragDrop->BlockNew.UL.Col;
    SS_COORD Row = lpDragDrop->BlockNew.UL.Row;
    SS_AdjustCellCoords(lpSS, &Col, &Row);

    if (!lpDragDrop->fCancel &&
        lpSS->Row.CurAt != Row && Row >= lpSS->Row.HeaderCnt)
    {
       SSOcxLockVBSpread((LONG)this);
       vbSpreadSaveBoundRow(lpSS, (LONG)this, m_hWnd, lpSS->Row.CurAt, Row, Col);
       SSOcxUnlockVBSpread((LONG)this);
    }
    UnlockActiveSheet();
  }
  #endif
//DBOCX.
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmDrawItem(UINT wParam, LONG lParam)
{
  LPSS_DRAWITEMSTRUCT lpDrawItem = (LPSS_DRAWITEMSTRUCT)lParam;
  SS_COORD lCol = SS_HEADER == lpDrawItem->Col ? 0 : lpDrawItem->Col;
  SS_COORD lRow = SS_HEADER == lpDrawItem->Row ? 0 : lpDrawItem->Row;
  long lLeft = XPixelsToTwips(lpDrawItem->Rect.left);
  long lTop = YPixelsToTwips(lpDrawItem->Rect.top);
  long lRight = XPixelsToTwips(lpDrawItem->Rect.right);
  long lBottom = YPixelsToTwips(lpDrawItem->Rect.bottom);

  #ifndef SSE_NO_DRAWITEM
  FireDrawItem(lCol, lRow, (OLE_HANDLE)lpDrawItem->hDC,
               lLeft, lTop, lRight, lBottom, lpDrawItem->lStyle);
  #endif

  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmEditModeOff(UINT wParam, LONG lParam)
{
  LPSS_EDITMODEOFF lpEditModeOff = (LPSS_EDITMODEOFF)lParam;
  SS_COORD lCol = SS_HEADER == lpEditModeOff->Col ? 0 : lpEditModeOff->Col;
  SS_COORD lRow = SS_HEADER == lpEditModeOff->Row ? 0 : lpEditModeOff->Row;

  FireEditMode(lCol, lRow, FALSE, lpEditModeOff->fChangeMade);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmEditModeOn(UINT wParam, LONG lParam)
{
  if (lParam)
  {
    LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
    SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
    SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;
 
    FireEditMode(lCol, lRow, TRUE, FALSE);
  }
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmEnterRow(UINT wParam, LONG lParam)
{
  LPSS_ENTERROW lpEnterRow = (LPSS_ENTERROW)lParam;
  SS_COORD lRow = SS_HEADER == lpEnterRow->Row ? 0 : lpEnterRow->Row;

  #ifndef SSE_NO_ENTERROW
  FireEnterRow(lRow, lpEnterRow->fNewRow);
  #endif
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmFireEventPrintAbort(UINT wParam, LONG lParam)
{
#ifdef SS_V35
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));
     varParam.vt = VT_BOOL; 
     varParam.boolVal  = (*(BOOL FAR *)lParam ? VARIANT_TRUE : VARIANT_FALSE); 

     FireScriptPrintAbort(&varParam);
     VariantChangeType(&arg, &varParam, 0, VT_BOOL); 

     *(BOOL FAR *)lParam = (arg.boolVal == VARIANT_TRUE ? TRUE : FALSE);
     }
  else
     FirePrintAbort((BOOL FAR*)lParam);

  UnlockBook();
#else
  #ifndef SSE_NO_PRINTABORT
  FirePrintAbort((BOOL FAR*)lParam);
  #endif
#endif
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmFireEventPrintMsgClose(UINT wParam, LONG lParam)
{
#ifdef SS_V35
  LPCTSTR lpszText;
  long lPrintID = wParam;
  BOOL bOpenMsg = FALSE;
  BOOL bProcessed = FALSE;
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->hPrintAbortMsg)
    lpszText = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);
  else
    lpszText = _T("");

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));
     varParam.vt = VT_BOOL; 
     varParam.boolVal  = (bProcessed ? VARIANT_TRUE : VARIANT_FALSE); 

     FireScriptPrintMsgBox(lpszText, lPrintID, bOpenMsg, &varParam);
     VariantChangeType(&arg, &varParam, 0, VT_BOOL); 
     bProcessed = (arg.boolVal == VARIANT_TRUE ? TRUE : FALSE);
     }
  else
     FirePrintMsgBox(lpszText, lPrintID, bOpenMsg, &bProcessed);

  if (lpBook->hPrintAbortMsg)
    tbGlobalUnlock(lpBook->hPrintAbortMsg);
  UnlockBook();
  return bProcessed;
#else
  #ifndef SSE_NO_PRINTMSGBOX
  LPSS_BOOK lpBook = LockBook();
  LPCTSTR lpszText;
  long lPrintID = wParam;
  BOOL bOpenMsg = FALSE;
  BOOL bProcessed = FALSE;

  if (lpBook->hPrintAbortMsg)
    lpszText = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);
  else
    lpszText = _T("");

  FirePrintMsgBox(lpszText, lPrintID, bOpenMsg, &bProcessed);
  if (lpBook->hPrintAbortMsg)
    tbGlobalUnlock(lpBook->hPrintAbortMsg);
  UnlockBook();
  return bProcessed;
  #else
  return 0;
  #endif
#endif
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmFireEventPrintMsgOpen(UINT wParam, LONG lParam)
{
#ifdef SS_V35
  LPCTSTR lpszText;
  long lPrintID = wParam;
  BOOL bOpenMsg = TRUE;
  BOOL bProcessed = FALSE;
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->hPrintAbortMsg)
    lpszText = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);
  else
    lpszText = _T("");

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));
     varParam.vt = VT_BOOL; 
     varParam.boolVal  = (bProcessed ? VARIANT_TRUE : VARIANT_FALSE); 

     FireScriptPrintMsgBox(lpszText, lPrintID, bOpenMsg, &varParam);
     VariantChangeType(&arg, &varParam, 0, VT_BOOL); 
     bProcessed = (arg.boolVal == VARIANT_TRUE ? TRUE : FALSE);
     }
  else
     FirePrintMsgBox(lpszText, lPrintID, bOpenMsg, &bProcessed);

  if (lpBook->hPrintAbortMsg)
    tbGlobalUnlock(lpBook->hPrintAbortMsg);

  UnlockBook();
  return bProcessed;
#else
  #ifndef SSE_NO_PRINTMSGBOX
  LPCTSTR lpszText;
  long lPrintID = wParam;
  BOOL bOpenMsg = TRUE;
  BOOL bProcessed = FALSE;
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->hPrintAbortMsg)
    lpszText = (LPTSTR)tbGlobalLock(lpBook->hPrintAbortMsg);
  else
    lpszText = _T("");

  FirePrintMsgBox(lpszText, lPrintID, bOpenMsg, &bProcessed);

  if (lpBook->hPrintAbortMsg)
    tbGlobalUnlock(lpBook->hPrintAbortMsg);

  UnlockBook();
  return bProcessed;
  #else
  return 0;
  #endif
#endif
}

//--------------------------------------------------------------------

#ifdef SS_V35
void CSsocxCtrl::OnKeyDownEvent( USHORT nChar, USHORT nShiftState )
{
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
    {
    VARIANT arg;
    VARIANT varParam;

    memset(&arg, 0, sizeof(VARIANT));
    memset(&varParam, 0, sizeof(VARIANT));
    varParam.vt = VT_I2;
    varParam.iVal  = nChar;

    FireScriptKeyDown(&varParam, nShiftState);
    VariantChangeType(&arg, &varParam, 0, VT_I2);
    nChar = (USHORT)arg.iVal;
    }

  UnlockBook();
}
#endif

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmKeyDown(UINT wParam, LONG lParam)
{
  USHORT nKeyCode = LOWORD(lParam);
  short nShift = HIWORD(lParam);
  LPSS_BOOK lpBook = LockBook();

  if( SS_GetEditMode(lpBook) )
    {
#ifdef SS_V35
#ifdef SS_V40
    if (m_bEventEnabled[EventEnabled_KeyDown])
#endif // SS_V40
      {
      if (lpBook->bScriptEnhanced)
         {
         VARIANT arg;
         VARIANT varParam;

         memset(&arg, 0, sizeof(VARIANT));
         memset(&varParam, 0, sizeof(VARIANT));
         varParam.vt = VT_I2;                        // 9369
         varParam.iVal  = nKeyCode;                  // 9369

         FireScriptKeyDown(&varParam, nShift);
         VariantChangeType(&arg, &varParam, 0, VT_I2);    // 9369
         nKeyCode = (USHORT)arg.iVal;                     // 9369
         }
      else
         FireKeyDown(&nKeyCode, nShift);
      }
#else
    FireKeyDown(&nKeyCode, nShift);
#endif
    }
  UnlockBook();
  return 0;
}

//--------------------------------------------------------------------

#ifdef SS_V35
void CSsocxCtrl::OnKeyPressEvent( USHORT nChar )
{
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
    {
    VARIANT arg;
    VARIANT varParam;

    memset(&arg, 0, sizeof(VARIANT));
    memset(&varParam, 0, sizeof(VARIANT));
    varParam.vt = VT_I2;                     // 9369
    varParam.iVal  = nChar;                  // 9369

    FireScriptKeyPress(&varParam);
    VariantChangeType(&arg, &varParam, 0, VT_I2);  // 9369
    nChar = (USHORT)arg.iVal;                      // 9369
    }

  UnlockBook();
}
#endif

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmKeyPress(UINT wParam, LONG lParam)
{
  if( !lParam )
    return 0;
  LPSS_BOOK lpBook = LockBook();
#ifndef _UNICODE
  BOOL fIsLeadByte = FALSE;
#endif // _UNICODE
// SPRD014	Wei Feng 1997/9/9
//#ifdef  SPREAD_JPN
	USHORT nChar = *(USHORT*)lParam;

#ifndef _UNICODE
	if(IsDBCSLeadByte((BYTE)nChar))
	{
		MSG msg;
		::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR, PM_NOREMOVE);
		nChar = nChar << 8;
		nChar |= (unsigned short)msg.wParam;
      fIsLeadByte = TRUE;
	}
#endif // _UNICODE

//---Changed by dean  1998/7/7------
// -To fix Keypress event bug Sp25_006 
// lParam Is The Return Value Of KeyAscii Parameter Of KeyPress Event 
// Value should be replaced by the KeyAscii Parameter.

	USHORT nOldChar;
	nOldChar = nChar;
// RFW - 8/24/01 #ifndef  SPREAD_JPN
    if( SS_GetEditMode(lpBook) )
// RFW - 8/24/01 #endif
       {
#ifdef SS_V35
#ifdef SS_V40
       if (m_bEventEnabled[EventEnabled_KeyPress])
#endif // SS_V40
          {
			 if (lpBook->bScriptEnhanced)
				 {
				 VARIANT arg;
				 VARIANT varParam;

				 memset(&arg, 0, sizeof(VARIANT));
				 memset(&varParam, 0, sizeof(VARIANT));
				 varParam.vt = VT_I2;                     // 9369
				 varParam.iVal  = nChar;                  // 9369

				 FireScriptKeyPress(&varParam);
				 VariantChangeType(&arg, &varParam, 0, VT_I2);  // 9369
				 nChar = (USHORT)arg.iVal;                      // 9369
				 }
			 else
				 FireKeyPress(&nChar);
          }
#else
       FireKeyPress(&nChar);
#endif
       }

	if (nOldChar != nChar)	
	{
			*(USHORT*)lParam=nChar;
// RFW - 7/25/03
#ifndef _UNICODE
         if (fIsLeadByte)
         {
           MSG msg;
			  ::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR, PM_REMOVE);
         }
#endif // _UNICODE
	}
//-------------------------------
//#else
//  if( SS_GetEditMode(lpSS) )
//    FireKeyPress((USHORT*)lParam);
//#endif
  UnlockBook();
  return 0;
}

//--------------------------------------------------------------------

#ifdef SS_V35
void CSsocxCtrl::OnKeyUpEvent( USHORT nChar, USHORT nShiftState )
{
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
    {
    VARIANT arg;
    VARIANT varParam;

    memset(&arg, 0, sizeof(VARIANT));
    memset(&varParam, 0, sizeof(VARIANT));
    varParam.vt = VT_I2;
    varParam.iVal  = nChar;

    FireScriptKeyUp(&varParam, nShiftState);
    VariantChangeType(&arg, &varParam, 0, VT_I2);
    nChar = (USHORT)arg.iVal;
    }
  UnlockBook();
}
#endif

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmKeyUp(UINT wParam, LONG lParam)
{
  USHORT nKeyCode = LOWORD(lParam);
  short nShift = HIWORD(lParam);
  LPSS_BOOK lpBook = LockBook();

  if( SS_GetEditMode(lpBook) )
    {
#ifdef SS_V35
#ifdef SS_V40
    if (m_bEventEnabled[EventEnabled_KeyUp])
#endif // SS_V40
       {
		 if (lpBook->bScriptEnhanced)
			 {
			 VARIANT arg;
			 VARIANT varParam;

			 memset(&arg, 0, sizeof(VARIANT));
			 memset(&varParam, 0, sizeof(VARIANT));
			 varParam.vt = VT_I2;                    // 9369
			 varParam.iVal  = nKeyCode;              // 9369

			 FireScriptKeyUp(&varParam, nShift);
			 VariantChangeType(&arg, &varParam, 0, VT_I2);      // 9369
			 nKeyCode = (USHORT)arg.iVal;                       // 9369
			 }
		 else
			 FireKeyUp(&nKeyCode, nShift);
       }
#else
    FireKeyUp(&nKeyCode, nShift);
#endif
    }
  UnlockBook();
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmLeaveCell(UINT wParam, LONG lParam)
{
  LPSS_LEAVECELL lpLeaveCell = (LPSS_LEAVECELL)lParam;
  SS_COORD lColCur = SS_HEADER == lpLeaveCell->ColCurrent ? 0 : lpLeaveCell->ColCurrent;
  SS_COORD lRowCur = SS_HEADER == lpLeaveCell->RowCurrent ? 0 : lpLeaveCell->RowCurrent;
  SS_COORD lColNew = SS_HEADER == lpLeaveCell->ColNew ? 0  :lpLeaveCell->ColNew;
  SS_COORD lRowNew = SS_HEADER == lpLeaveCell->RowNew ? 0  :lpLeaveCell->RowNew;
  BOOL bCancel = FALSE;

//- SPRD018 Wei Feng 1997/9/26
#ifdef SPREAD_JPN
  if(!m_bFiring)
  {
	  m_bFiring = TRUE;
#endif  

#ifdef SS_V35
  {
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));
     varParam.vt = VT_BOOL; 
     varParam.boolVal  = (bCancel ? VARIANT_TRUE : VARIANT_FALSE); 

     FireScriptLeaveCell(lColCur, lRowCur, lColNew, lRowNew, &varParam);
     VariantChangeType(&arg, &varParam, 0, VT_BOOL); 
     bCancel = (arg.boolVal == VARIANT_TRUE ? TRUE : FALSE);
     }
  else
     FireLeaveCell(lColCur, lRowCur, lColNew, lRowNew, &bCancel);

  UnlockBook();
  }
#else
  FireLeaveCell(lColCur, lRowCur, lColNew, lRowNew, &bCancel);
#endif

//DBOCX...
  {
    SS_COORD Row = lpLeaveCell->RowCurrent;
    SS_COORD RowNew = lpLeaveCell->RowNew;
    SS_COORD ColNew = lpLeaveCell->ColNew;
	 LPSPREADSHEET lpSS = LockSheetSendingMsg();

	 SS_AdjustCellCoords(lpSS, NULL, &Row);
	 SS_AdjustCellCoords(lpSS, &ColNew, &RowNew);

    if (!bCancel && Row != RowNew)
    {
       SSOcxLockVBSpread((LONG)this);
       bCancel = vbSpreadSaveBoundRow(lpSS, (LONG)this, m_hWnd, Row, RowNew == -1 ? Row :
                   RowNew, ColNew);
       SSOcxUnlockVBSpread((LONG)this);
    }

	 UnlockSheetSendingMsg();
  }
//DBOCX.

//- SPRD018 Wei Feng 1997/9/26
#ifdef SPREAD_JPN
	m_bFiring = FALSE;
  }
#endif

  return bCancel;
}


//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmLeaveRow(UINT wParam, LONG lParam)
{
  LPSS_LEAVEROW lpLeaveRow = (LPSS_LEAVEROW)lParam;
  SS_COORD lRowCur = SS_HEADER == lpLeaveRow->RowCurrent ? 0 : lpLeaveRow->RowCurrent;
  SS_COORD lRowNew = SS_HEADER == lpLeaveRow->RowNew ? 0 : lpLeaveRow->RowNew;
  BOOL bCancel = FALSE;

  #ifndef SSE_NO_LEAVEROW
// Added by BOC FMH for fAllCellsHaveData's value miss 1996.06.28. --------------->>
  /* while EditMode if off, leave a row not causing LeaveCell. so the current
   * fAllCellsHaveData is not checked. The following code is copied from VBSPREAD.C's
   * SS_AllCellsHaveData function.
   */
#if defined(SPREAD_JPN)
  SS_CELLTYPE   CellType;
  LPSS_CELL     lpCell;
  SS_COORD      i;
  LPSPREADSHEET lpSS = LockSheetSendingMsg();

  lpLeaveRow->fAllCellsHaveData = TRUE;
  for (i = lpSS->Col.HeaderCnt; i <= lpSS->Col.Max; i++)
   {
   SS_RetrieveCellType(lpSS, &CellType, NULL, i, lRowCur);

   if (CellType.Type != SS_TYPE_STATICTEXT &&
       CellType.Type != SS_TYPE_OWNERDRAW &&
       CellType.Type != SS_TYPE_PICTURE &&
       CellType.Type != SS_TYPE_BUTTON &&
       CellType.Type != SS_TYPE_CHECKBOX)
      if (lpCell = SS_LockCellItem(lpSS, i, lRowCur))
         {
         if (!lpCell->Data.bDataType)
            lpLeaveRow->fAllCellsHaveData = FALSE;

         SS_UnlockCellItem(lpSS, i, lRowCur);
         }
      else
           lpLeaveRow->fAllCellsHaveData = FALSE;
   }
  UnlockSheetSendingMsg();
#endif
//----------------------------<<

#ifdef SS_V35
  {
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));
     varParam.vt = VT_BOOL; 
     varParam.boolVal  = (bCancel ? VARIANT_TRUE : VARIANT_FALSE); 

     FireScriptLeaveRow(lRowCur, lpLeaveRow->fAddRow,
                        lpLeaveRow->fRowChangeMade, lpLeaveRow->fAllCellsHaveData,
                        lRowNew, lpLeaveRow->fRowNewBeyondLastDataRow,
                        &varParam);
     VariantChangeType(&arg, &varParam, 0, VT_BOOL); 
     bCancel = (arg.boolVal == VARIANT_TRUE ? TRUE : FALSE);
     }
  else
     FireLeaveRow(lRowCur, lpLeaveRow->fAddRow,
                  lpLeaveRow->fRowChangeMade, lpLeaveRow->fAllCellsHaveData,
                  lRowNew, lpLeaveRow->fRowNewBeyondLastDataRow,
                  &bCancel);

  UnlockBook();
  }
#else
  FireLeaveRow(lRowCur, lpLeaveRow->fAddRow,
               lpLeaveRow->fRowChangeMade, lpLeaveRow->fAllCellsHaveData,
               lRowNew, lpLeaveRow->fRowNewBeyondLastDataRow,
               &bCancel);
#endif
  #endif
//DBOCX...
  {
    if (!bCancel && lRowCur != lRowNew && lRowNew != -1)
    {
		 LPSPREADSHEET lpSS = LockSheetSendingMsg();
		 SS_AdjustCellCoords(lpSS, NULL, &lRowCur);
		 SS_AdjustCellCoords(lpSS, NULL, &lRowNew);

       SSOcxLockVBSpread((LONG)this);
       bCancel = vbSpreadSaveBoundRow(lpSS, (LONG)this, m_hWnd, lRowCur, lRowNew, -1);
       SSOcxUnlockVBSpread((LONG)this);

		 UnlockSheetSendingMsg();
    }
  }
//DBOCX.
  return bCancel;
}

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkDispatchNotCreated(UINT wParam, LONG lParam)
{
  LPSS_LINKMSG lpMsg = (LPSS_LINKMSG)lParam;
  FireLinkDispatchNotCreated(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkError(UINT wParam, LONG lParam)
{
  LPSS_LINKERROR lpError = (LPSS_LINKERROR)lParam;
  FireLinkError(lpError->lpszServer, lpError->lpszTopic, lpError->lpszItem,
                lpError->nError, lpError->lpszError);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkItemNotSupported(UINT wParam, LONG lParam)
{
  LPSS_LINKMSG lpMsg = (LPSS_LINKMSG)lParam;
  FireLinkItemNotSupported(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkItemSupported(UINT wParam, LONG lParam)
{
  LPSS_LINKMSG lpMsg = (LPSS_LINKMSG)lParam;
  FireLinkItemSupported(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkNewData(UINT wParam, LONG lParam)
{
  LPSS_LINKMSG lpMsg = (LPSS_LINKMSG)lParam;
  FireLinkNewData(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkNotify(UINT wParam, LONG lParam)
{
  LPSS_LINKMSG lpMsg = (LPSS_LINKMSG)lParam;
  FireLinkNotify(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkOutOfMemory(UINT wParam, LONG lParam)
{
  LPSS_LINKMSG lpMsg = (LPSS_LINKMSG)lParam;
  FireLinkOutOfMemory(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkPokeCompleted(UINT wParam, LONG lParam)
{
  LPSS_LINKPOKECOMPLETED lpMsg = (LPSS_LINKPOKECOMPLETED)lParam;
  FireLinkPokeCompleted(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem, lpMsg->lCode);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkRequestCompleted(UINT wParam, LONG lParam)
{
  LPSS_LINKREQUESTCOMPLETED lpMsg = (LPSS_LINKREQUESTCOMPLETED)lParam;
  FireLinkRequestCompleted(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem, lpMsg->lCode);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkServerDisconnected(UINT wParam, LONG lParam)
{
  LPSS_LINKMSG lpMsg = (LPSS_LINKMSG)lParam;
  FireLinkServerDisconnected(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------
//
//
//

#if defined(SS_DDE)
LONG CSsocxCtrl::OnSsmLinkUnableToConnectToServer(UINT wParam, LONG lParam)
{
  LPSS_LINKMSG lpMsg = (LPSS_LINKMSG)lParam;
  FireLinkUnableToConnectToServer(lpMsg->lpszServer, lpMsg->lpszTopic, lpMsg->lpszItem);
  return 0;
}
#endif // SS_DDE

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmRButton(UINT wParam, LONG lParam)
{
  LPSS_RBUTTON lpRButton = (LPSS_RBUTTON)lParam;
  SS_COORD lCol = SS_HEADER == lpRButton->Col ? 0 : lpRButton->Col;
  SS_COORD lRow = SS_HEADER == lpRButton->Row ? 0 : lpRButton->Row;
  long xPos = XPixelsToTwips(lpRButton->xPos);
  long yPos = YPixelsToTwips(lpRButton->yPos);

  #ifndef SSE_NO_RIGHTCLICK
  FireRightClick(lpRButton->RButtonType, lCol, lRow, xPos, yPos);
  #endif
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmRowHeightChange(UINT wParam, LONG lParam)
{
  LPSS_COORDRANGE lpRange = (LPSS_COORDRANGE)lParam;
  SS_COORD lCoord1 = SS_HEADER == lpRange->Coord1 ? 0 : lpRange->Coord1;
  SS_COORD lCoord2 = SS_HEADER == lpRange->Coord2 ? 0 : lpRange->Coord2;

  #ifndef SSE_NO_ROWHEIGHTCHANGE
  FireRowHeightChange(lCoord1, lCoord2);
  #endif
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmRowRestricted(UINT wParam, LONG lParam)
{
  SS_COORD lRow = SS_HEADER == lParam ? 0 : lParam;

  FireEditError(-1, lRow, EDITERROR_ROWRESTRICTED);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmVClearData(UINT wParam, LONG lParam)
{
  LPSS_VCLEARDATA lpVClearData = (LPSS_VCLEARDATA)lParam;
  LPSPREADSHEET lpSS = LockSheetSendingMsg();
  LONG lRet = 0L;                                             //DBOCX

  FireVirtualClearData(lpVClearData->Row, lpVClearData->RowsBeingCleared);
//DBOCX...
  SSOcxLockVBSpread((LONG)this);
  if (VBSSBoundIsBound((LONG)this, lpSS))
     vbSSBoundVClearData(lpSS, (LONG)this, m_hWnd, lpVClearData);
  SSOcxUnlockVBSpread((LONG)this);
//DBOCX.

  UnlockSheetSendingMsg();
  return lRet;                                                //DBOCX
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmVQueryData(UINT wParam, LONG lParam)
{
  LPSS_VQUERYDATA lpVQueryData = (LPSS_VQUERYDATA)lParam;
  BOOL bAtTop = lpVQueryData->fAtTop;
  BOOL bAtBottom = lpVQueryData->fAtBottom;
  LONG lRet = 0L;
  LPSS_BOOK lpBook = LockBook();

#ifdef SS_V35
  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg[3];
     VARIANT varParam[3];

     memset(&arg, 0, sizeof(VARIANT) * 3);
     memset(&varParam, 0, sizeof(VARIANT) * 3);

     varParam[0].vt = VT_I4; 
     varParam[0].lVal  = lpVQueryData->RowsLoaded; 

     varParam[1].vt = VT_BOOL; 
     varParam[1].boolVal  = (bAtTop ? VARIANT_TRUE : VARIANT_FALSE); 

     varParam[2].vt = VT_BOOL; 
     varParam[2].boolVal  = (bAtBottom ? VARIANT_TRUE : VARIANT_FALSE); 

     FireScriptQueryData(lpVQueryData->Row,  lpVQueryData->RowsNeeded,
                   &varParam[0], lpVQueryData->wDirection,
                   &varParam[1], &varParam[2]);

     VariantChangeType(&arg[0], &varParam[0], 0, VT_I4); 
     VariantChangeType(&arg[1], &varParam[1], 0, VT_BOOL); 
     VariantChangeType(&arg[2], &varParam[2], 0, VT_BOOL); 

     lpVQueryData->RowsLoaded = arg[0].lVal;
     bAtTop = (arg[1].boolVal == VARIANT_TRUE ? TRUE : FALSE);
     bAtBottom = (arg[2].boolVal == VARIANT_TRUE ? TRUE : FALSE);
     }
  else
     FireQueryData(lpVQueryData->Row,  lpVQueryData->RowsNeeded,
                   &lpVQueryData->RowsLoaded, lpVQueryData->wDirection,
                   &bAtTop, &bAtBottom);
#else
  FireQueryData(lpVQueryData->Row,  lpVQueryData->RowsNeeded,
                &lpVQueryData->RowsLoaded, lpVQueryData->wDirection,
                &bAtTop, &bAtBottom);
#endif
  lpVQueryData->fAtTop = bAtTop;
  lpVQueryData->fAtBottom = bAtBottom;
  UnlockBook();

//DBOCX..
  LPSPREADSHEET lpSS = LockSheetSendingMsg();
  SSOcxLockVBSpread((LONG)this);
  if (VBSSBoundIsBound((LONG)this, lpSS))
     lRet = vbSSBoundVQueryData(lpSS, (LONG)this, m_hWnd, lpVQueryData);
  SSOcxUnlockVBSpread((LONG)this);
  UnlockSheetSendingMsg();
//DBOCX

  return lRet;
}

#ifdef SS_V30

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmComboCloseUp(UINT wParam, LONG lParam)
{
  LPSS_COMBOCLOSEUP lpCloseUp = (LPSS_COMBOCLOSEUP)lParam;
  SS_COORD lCol = SS_HEADER == lpCloseUp->Col ? 0 : lpCloseUp->Col;
  SS_COORD lRow = SS_HEADER == lpCloseUp->Row ? 0 : lpCloseUp->Row;

  FireComboCloseUp(lCol, lRow, lpCloseUp->dSelChange);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmComboDropDown(UINT wParam, LONG lParam)
{
  LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
  SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
  SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;

  FireComboDropDown(lCol, lRow);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmComboSelChange(UINT wParam, LONG lParam)
{
  LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
  SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
  SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;

  FireComboSelChange(lCol, lRow);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmTextTipFetch(UINT wParam, LONG lParam)
{
  LPSS_TEXTTIPFETCH lpSSFetch = (LPSS_TEXTTIPFETCH)lParam;
  SS_COORD lCol = SS_HEADER == lpSSFetch->Col ? 0 : lpSSFetch->Col;
  SS_COORD lRow = SS_HEADER == lpSSFetch->Row ? 0 : lpSSFetch->Row;
  long     lWidth = XPixelsToTwips(lpSSFetch->nWidth);
  FPTSTR   tstrText;
  BSTR     bstrText;

#ifdef SS_V70
  if (lpSSFetch->hText)
  {
    LPTSTR lpszText = (LPTSTR)GlobalLock(lpSSFetch->hText);
    bstrText = (BSTR)fpSysAllocBSTRFromTString(lpszText);
    GlobalUnlock(lpSSFetch->hText);
  }
  else
    bstrText = (BSTR)fpSysAllocBSTRFromTString(_T(""));
#else
  bstrText = (BSTR)fpSysAllocBSTRFromTString(lpSSFetch->szText);
#endif // SS_V70

#ifdef SS_V35
  {
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg[4];
     VARIANT varParam[4];

     memset(&arg, 0, sizeof(VARIANT) * 4);
     memset(&varParam, 0, sizeof(VARIANT) * 4);
     varParam[0].vt = VT_I2; 
     varParam[0].iVal  = (short)lpSSFetch->wMultiLine; 

     varParam[1].vt = VT_I4; 
     varParam[1].lVal  = lWidth; 

     varParam[2].vt = VT_BSTR; 
     varParam[2].bstrVal  = bstrText; 

     varParam[3].vt = VT_I2; 
     varParam[3].iVal  = lpSSFetch->fShow; 

     FireScriptTextTipFetch(lCol, lRow, &varParam[0], &varParam[1], &varParam[2], &varParam[3]);

     VariantChangeType(&arg[0], &varParam[0], 0, VT_I2); 
     VariantChangeType(&arg[1], &varParam[1], 0, VT_I4); 
     VariantChangeType(&arg[2], &varParam[2], 0, VT_BSTR); 
     VariantChangeType(&arg[3], &varParam[3], 0, VT_I2); 

     lpSSFetch->wMultiLine = (WORD)arg[0].iVal;
     lWidth = (WORD)arg[1].lVal;
     bstrText = arg[2].bstrVal;
     lpSSFetch->fShow = arg[3].iVal;
     }
  else
     FireTextTipFetch(lCol, lRow, (LPSHORT)&lpSSFetch->wMultiLine, &lWidth, &bstrText, &lpSSFetch->fShow);

  UnlockBook();
  }
#else
  FireTextTipFetch(lCol, lRow, (LPSHORT)&lpSSFetch->wMultiLine, &lWidth, &bstrText, &lpSSFetch->fShow);
#endif

  tstrText = fpSysAllocTStringFromBSTR((LPVOID)bstrText);

#ifdef SS_V70
  if (lpSSFetch->hText)
  {
    GlobalFree(lpSSFetch->hText);
    lpSSFetch->hText = NULL;
  }

  if (tstrText && *tstrText)
  {
    LPTSTR lpszText;

    lpSSFetch->hText = GlobalAlloc(GHND, (lstrlen(tstrText)+1)*sizeof(TCHAR));
    lpszText = (LPTSTR)GlobalLock(lpSSFetch->hText);
    _tcscpy(lpszText, tstrText);
    GlobalUnlock(lpSSFetch->hText);
  }
  else
    bstrText = (BSTR)fpSysAllocBSTRFromTString(_T(""));
#else
  lstrcpyn(lpSSFetch->szText, (LPTSTR)tstrText, SS_TT_TEXTMAX);
#endif // SS_V70

  if (tstrText)
     fpSysFreeTString((LPTSTR)tstrText);

  if (bstrText)
     SysFreeString(bstrText);

  lpSSFetch->nWidth = (short)XTwipsToPixels(lWidth);
  return 0;
}

//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmEditChange(UINT wParam, LONG lParam)
{
  LPSS_CELLCOORD lpCell = (LPSS_CELLCOORD)lParam;
  SS_COORD lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
  SS_COORD lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;

  FireEditChange(lCol, lRow);
  return 0;
}

#endif // SS_V30

#ifdef SS_V35
//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmBeforeUserSort(UINT wParam, LONG lParam)
{
  LPSS_BEFOREUSERSORT lpBeforeSort = (LPSS_BEFOREUSERSORT)lParam;
  long lDefaultAction = lpBeforeSort->lDefaultAction;
  LPSS_BOOK lpBook = LockBook();

  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));

     varParam.vt = VT_I4; 
     varParam.lVal = lDefaultAction; 

     FireScriptBeforeUserSort(lpBeforeSort->lCol, lpBeforeSort->lState, &varParam);

     VariantChangeType(&arg, &varParam, 0, VT_I4); 
     lDefaultAction = arg.lVal;
     }
  else
     FireBeforeUserSort(lpBeforeSort->lCol, lpBeforeSort->lState, &lDefaultAction);

  lpBeforeSort->lDefaultAction = lDefaultAction;
  UnlockBook();
  return 0;
}
//--------------------------------------------------------------------

LONG CSsocxCtrl::OnSsmAfterUserSort(UINT wParam, LONG lParam)
{
  SS_COORD lCol = (SS_HEADER == lParam ? 0 : lParam);

  FireAfterUserSort(lCol);
  return 0;
}
#endif
//--------------------------------------------------------------------

#ifdef SS_V70
LONG CSsocxCtrl::OnSsmSheetChanging(UINT wParam, LONG lParam)
{
  short nOldSheet = LOWORD(lParam);
  short nNewSheet = HIWORD(lParam);
  VARIANT arg;
  VARIANT varParam;
  long lRet;

  memset(&arg, 0, sizeof(VARIANT));
  memset(&varParam, 0, sizeof(VARIANT));

  varParam.vt = VT_BOOL; 
  varParam.boolVal = VARIANT_FALSE; 

  FireSheetChanging(nOldSheet, nNewSheet, &varParam);

  VariantChangeType(&arg, &varParam, 0, VT_BOOL);
  lRet = arg.boolVal;
  return lRet;
}

LONG CSsocxCtrl::OnSsmSheetChanged(UINT wParam, LONG lParam)
{
  short nOldSheet = LOWORD(lParam);
  short nNewSheet = HIWORD(lParam);
  
  FireSheetChanged(nOldSheet, nNewSheet);
  return 0;
}

LONG CSsocxCtrl::OnSsmTabScrolling(UINT wParam, LONG lParam)
{
  short nOldLeftSheet = LOWORD(lParam);
  short nNewLeftSheet = HIWORD(lParam);
  
  FireTabScrolling(nOldLeftSheet, nNewLeftSheet);
  return 0;
}

LONG CSsocxCtrl::OnSsmTabScrolled(UINT wParam, LONG lParam)
{
  short nOldLeftSheet = LOWORD(lParam);
  short nNewLeftSheet = HIWORD(lParam);
  
  FireTabScrolled(nOldLeftSheet, nNewLeftSheet);
  return 0;
}

LONG CSsocxCtrl::OnSsmBeforeEditMode(UINT wParam, LONG lParam)
{
  LPSS_BEFOREEDITMODE lpBeforeEditMode = (LPSS_BEFOREEDITMODE)lParam;
  VARIANT arg[2];
  VARIANT varParam[2];

  memset(&arg, 0, sizeof(VARIANT) * 2);
  memset(&varParam, 0, sizeof(VARIANT) * 2);

  varParam[0].vt = VT_I2; 
  varParam[0].iVal = lpBeforeEditMode->wCursorPos; 

  varParam[1].vt = VT_BOOL;
  varParam[1].boolVal = lpBeforeEditMode->fCancel ? VARIANT_TRUE : VARIANT_FALSE;

  FireBeforeEditMode(lpBeforeEditMode->Col, lpBeforeEditMode->Row, (long)lpBeforeEditMode->wUserAction, &varParam[0], &varParam[1]);

  VariantChangeType(&arg[0], &varParam[0], 0, VT_I2);
  VariantChangeType(&arg[1], &varParam[1], 0, VT_BOOL);
  lpBeforeEditMode->wCursorPos = (WORD)arg[0].iVal;
  lpBeforeEditMode->fCancel = arg[1].boolVal;

  return 0;
}

LONG CSsocxCtrl::OnSsmBeforeScrollTip(UINT wParam, LONG lParam)
{
  LPSS_BEFORESCROLLTIP lpBeforeScrollTip = (LPSS_BEFORESCROLLTIP)lParam;
  VARIANT arg;
  VARIANT varParam;
  FPTSTR   tstrText;
  BSTR     bstrText;

  memset(&arg, 0, sizeof(VARIANT));
  memset(&varParam, 0, sizeof(VARIANT));

  varParam.vt = VT_I2; 
  varParam.iVal = lpBeforeScrollTip->nWidth; 

  bstrText = (BSTR)fpSysAllocBSTRFromTString(lpBeforeScrollTip->szText);

  FireBeforeScrollTip(lpBeforeScrollTip->fIsVertical, lpBeforeScrollTip->Index, &bstrText, &varParam);
  
  VariantChangeType(&arg, &varParam, 0, VT_I2);
  lpBeforeScrollTip->nWidth = arg.iVal;

  tstrText = fpSysAllocTStringFromBSTR((LPVOID)bstrText);
  lstrcpyn(lpBeforeScrollTip->szText, (LPTSTR)tstrText, SS_SCROLLTIP_TEXTMAX);

  return 0;
}

LONG CSsocxCtrl::OnSsmBeforeColMove(UINT wParam, LONG lParam)
{
  LPSS_BEFORECOLMOVE lpBeforeColMove = (LPSS_BEFORECOLMOVE)lParam;
  VARIANT arg;
  VARIANT varParam;

  memset(&arg, 0, sizeof(VARIANT));
  memset(&varParam, 0, sizeof(VARIANT));

  varParam.vt = VT_BOOL;
  varParam.boolVal = lpBeforeColMove->fCancel ? VARIANT_TRUE : VARIANT_FALSE;

  FireBeforeColMove(lpBeforeColMove->Col, lpBeforeColMove->Col2, (long)lpBeforeColMove->ColDest, &varParam);

  VariantChangeType(&arg, &varParam, 0, VT_BOOL);
  lpBeforeColMove->fCancel = arg.boolVal;

  return 0;
}

LONG CSsocxCtrl::OnSsmBeforeRowMove(UINT wParam, LONG lParam)
{
  LPSS_BEFOREROWMOVE lpBeforeRowMove = (LPSS_BEFOREROWMOVE)lParam;
  VARIANT arg;
  VARIANT varParam;

  memset(&arg, 0, sizeof(VARIANT));
  memset(&varParam, 0, sizeof(VARIANT));

  varParam.vt = VT_BOOL;
  varParam.boolVal = lpBeforeRowMove->fCancel ? VARIANT_TRUE : VARIANT_FALSE;

  FireBeforeRowMove(lpBeforeRowMove->Row, lpBeforeRowMove->Row2, (long)lpBeforeRowMove->RowDest, &varParam);

  VariantChangeType(&arg, &varParam, 0, VT_BOOL);
  lpBeforeRowMove->fCancel = arg.boolVal;

  return 0;
}

LONG OnDataLoaded(LONG lObject)
{
  CSsocxCtrl *pThis = (CSsocxCtrl *)lObject;  

  pThis->FireDataLoaded();
  return 0;
}

#endif // SS_V70

//--------------------------------------------------------------------

#ifdef SS_V80
LONG CSsocxCtrl::OnSsmCircularFormula(UINT wParam, LONG lParam)
{
  LPSS_CIRCULARFORMULA lpCircularFormula = (LPSS_CIRCULARFORMULA)lParam;
  short nSheet = lpCircularFormula->Sheet;
  long lCol = lpCircularFormula->Col;
  long lRow = lpCircularFormula->Row;

  FireCircularFormula(nSheet, lCol, lRow);
  return 0;
}
#endif // SS_V80

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnBlockChanged(UINT id)
{
  LPSS_BOOK lpBook = LockBook();
  SS_CELLCOORD cellUL;
  SS_CELLCOORD cellLR;
  SS_COORD lCol;
  SS_COORD lRow;
  short nSheetSave = lpBook->nSheet;

  lpBook->nSheet = lpBook->nSheetSendingMsg;
  SSGetSelectBlock(m_hWnd, &cellUL, &cellLR);
  SSGetCurrSelBlockPos(m_hWnd, &lCol, &lRow);
  lpBook->nSheet = nSheetSave;
  UnlockBook();
  FireSelChange(cellUL.Col, cellUL.Row, cellLR.Col, cellLR.Row, lCol, lRow);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnBlockSelected(UINT id)
{
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = LockSheetSendingMsg();
  SS_CELLCOORD cellUL;
  SS_CELLCOORD cellLR;
  short nSheetSave = lpBook->nSheet;

  lpBook->nSheet = lpBook->nSheetSendingMsg;

  // RFW - 3/11/04 - 13827
  //SSGetSelectBlock(m_hWnd, &cellUL, &cellLR);

  cellUL.Row = lpSS->BlockCellUL.Row;
  cellUL.Col = lpSS->BlockCellUL.Col;

  if (lpSS->Row.HeaderCnt > cellUL.Row )
    cellUL.Row = -1;
  if (lpSS->Col.HeaderCnt > cellUL.Col )
    cellUL.Col = -1;

  cellLR.Row = lpSS->BlockCellLR.Row;
  cellLR.Col = lpSS->BlockCellLR.Col;

  // RFW - 1/18/05 - 15428
  if (cellUL.Row == -1 && cellLR.Row == 0)
    cellLR.Row = -1;

  if (cellUL.Col == -1 && cellLR.Col == 0)
    cellLR.Col = -1;

  SS_AdjustCellCoordsOut(lpSS, &cellUL.Col, &cellUL.Row);
  SS_AdjustCellCoordsOut(lpSS, &cellLR.Col, &cellLR.Row);

  UnlockSheetSendingMsg();
  lpBook->nSheet = nSheetSave;
  UnlockBook();
  FireBlockSelected(cellUL.Col, cellUL.Row, cellLR.Col, cellLR.Row);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnButtonUp(UINT id)
{
  SS_COORD lCol;
  SS_COORD lRow;
  SSGetCellSendingMsg(m_hWnd, &lCol, &lRow);
  FireButtonClicked(lCol, lRow, FALSE);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnButtonDown(UINT id)
{
  SS_COORD lCol;
  SS_COORD lRow;
  SSGetCellSendingMsg(m_hWnd, &lCol, &lRow);
  FireButtonClicked(lCol, lRow, TRUE);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnCheckBoxTyping(UINT id)
{
  SS_COORD lCol;
  SS_COORD lRow;
  SSGetActiveCell(m_hWnd, &lCol, &lRow);
  FireEditError(lCol, lRow, EDITERROR_CHECKBOXTYPING);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnEditModeIns(UINT id)
{
  SS_COORD lCol;
  SS_COORD lRow;
  SSGetActiveCell(m_hWnd, &lCol, &lRow);
  FireEditError(lCol, lRow, EDITERROR_EDITMODEINS);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnExitNext(UINT id)
{
  FireAdvance(TRUE);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnExitPrev(UINT id)
{
  FireAdvance(FALSE);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnInvalidColType(UINT id)
{
  SS_COORD lCol;
  SS_COORD lRow;
  SSGetActiveCell(m_hWnd, &lCol, &lRow);
  FireEditError(lCol, lRow, EDITERROR_INVALIDCOLTYPE);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnInvalidData(UINT id)
{
  LPSS_BOOK lpBook = LockBook();
  SS_COORD lCol = lpBook->CurVisCell.Col;
  SS_COORD lRow = lpBook->CurVisCell.Row;
  UnlockBook();
  FireEditError(lCol, lRow, EDITERROR_INVALIDDATA);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnInvalidPasteData(UINT id)
{
  SS_COORD lCol;
  SS_COORD lRow;
  SSGetActiveCell(m_hWnd, &lCol, &lRow);
  FireEditError(lCol, lRow, EDITERROR_INVALIDPASTEDATA);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnLeftColChange(UINT id)
{
  LPSPREADSHEET lpSS = LockSheetSendingMsg();
  SS_COORD lColPrev;
  SS_COORD lRowPrev;
  SS_COORD lCol = lpSS->Col.UL;
  SS_COORD lRow = lpSS->Row.UL;
  SS_GetTopLeftPrev(lpSS, &lColPrev, &lRowPrev);
  SS_AdjustCellCoordsOut(lpSS, &lColPrev, &lRowPrev);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
  UnlockSheetSendingMsg();
  #ifndef SSE_NO_TOPLEFTCHANGE
  FireTopLeftChange(lColPrev, lRowPrev, lCol, lRow);
  #endif
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnTopLeftChange(UINT id)
{
  LPSPREADSHEET lpSS = LockSheetSendingMsg();
  SS_COORD lColPrev;
  SS_COORD lRowPrev;
  SS_COORD lCol = lpSS->Col.UL;
  SS_COORD lRow = lpSS->Row.UL;
  SS_GetTopLeftPrev(lpSS, &lColPrev, &lRowPrev);
  SS_AdjustCellCoordsOut(lpSS, &lColPrev, &lRowPrev);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
  UnlockSheetSendingMsg();
  #ifndef SSE_NO_TOPLEFTCHANGE
  FireTopLeftChange(lColPrev, lRowPrev, lCol, lRow);
  #endif
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnTopRowChange(UINT id)
{
  LPSPREADSHEET lpSS = LockSheetSendingMsg();
  SS_COORD lColPrev;
  SS_COORD lRowPrev;
  SS_COORD lCol = lpSS->Col.UL;
  SS_COORD lRow = lpSS->Row.UL;
  SS_GetTopLeftPrev(lpSS, &lColPrev, &lRowPrev);
  SS_AdjustCellCoordsOut(lpSS, &lColPrev, &lRowPrev);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);
  UnlockSheetSendingMsg();
  #ifndef SSE_NO_TOPLEFTCHANGE
  FireTopLeftChange(lColPrev, lRowPrev, lCol, lRow);
  #endif
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnTypingProhibited(UINT id)
{
  SS_COORD lCol;
  SS_COORD lRow;
  SSGetActiveCell(m_hWnd, &lCol, &lRow);
  FireEditError(lCol, lRow, EDITERROR_TYPINGPROHIBITED);
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnSsnUserFormula(UINT id)
{
  #ifndef SSE_NO_USERFORMULAENTERED
  LPSPREADSHEET lpSS = LockSheetSendingMsg();
  SS_COORD lCol;
  SS_COORD lRow;
  SS_GetActiveCell(lpSS, &lCol, &lRow);
  // RFW - 6/1/04 - 14236
  SS_GetActualCell(lpSS, lCol, lRow, &lCol, &lRow, NULL);
  SS_AdjustCellCoordsOut(lpSS, &lCol, &lRow);

  FireUserFormulaEntered(lCol, lRow);
  UnlockSheetSendingMsg();
  #endif
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
#if defined(WIN32) || defined(_WIN32)
  UINT nId = LOWORD(wParam);
  HWND hWndCtrl = (HWND)lParam;
  int nNotifyCode = HIWORD(wParam);
#else
  UINT nId = wParam;
  HWND hWndCtrl = (HWND)LOWORD(lParam);
  int nNotifyCode = HIWORD(lParam);
#endif
  BOOL bRet = TRUE;

  switch( nNotifyCode )
  {
    case SSN_BLOCKCHANGED:
      OnSsnBlockChanged(nId);
      break;
    case SSN_BLOCKSELECTED:
      OnSsnBlockSelected(nId);
      break;
    case SSN_BUTTONDOWN:
      OnSsnButtonDown(nId);
      break;
    case SSN_BUTTONUP:
      OnSsnButtonUp(nId);
      break;
    case SSN_CHECKBOXTYPING:
      OnSsnCheckBoxTyping(nId);
      break;
    case SSN_EDITMODEINS:
      OnSsnEditModeIns(nId);
      break;
    case SSN_EXITNEXT:
      OnSsnExitNext(nId);
      break;
    case SSN_EXITPREV:
      OnSsnExitPrev(nId);
      break;
    case SSN_INVALIDCOLTYPE:
      OnSsnInvalidColType(nId);
      break;
    case SSN_INVALIDDATA:
      OnSsnInvalidData(nId);
      break;
    case SSN_INVALIDPASTEDATA:
      OnSsnInvalidPasteData(nId);
      break;
    case SSN_LEFTCOLCHANGE:
      OnSsnLeftColChange(nId);
      break;
    case SSN_TOPLEFTCHANGE:
      OnSsnTopLeftChange(nId);
      break;
    case SSN_TOPROWCHANGE:
      OnSsnTopRowChange(nId);
      break;
    case SSN_TYPINGPROHIBITED:
      OnSsnTypingProhibited(nId);
      break;
    case SSN_USERFORMULA:
      OnSsnUserFormula(nId);
      break;
    default:
      bRet = COleControl::OnCommand(wParam, lParam);
  }
  return bRet;
}

//--------------------------------------------------------------------

#if 0
#ifdef	SPREAD_JPN
// SPRD022 Wei Feng 1997/9/26
void CSsocxCtrl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
// SP25_011 Zhang HaoNan 1998/4/25
// Add the code to fix the SetFocus problem when the EditModePermanent= true &&
// To fix the bug that when EditModePermanent = ture cause the KeyDown event happen twice.
/*
	LPSPREADSHEET lpSS = SS_Lock(m_hWnd);
	BOOL bEditModePermanent = lpSS->fEditModePermanent;
	BOOL bEditModeOn = lpSS->EditModeOn;
	SS_Unlock(hWnd);

	if( bEditModeOn && (nChar == VK_DOWN || nChar == VK_UP || 
	   nChar == VK_NEXT || nChar == VK_PRIOR || nChar == VK_F1))
	{
		HWND hWndSave = m_hWnd;
		USHORT nCharShort = (USHORT)nChar;
		if ((m_hWnd == hWndSave) && (nCharShort != 0))
			DefWindowProc(WM_KEYDOWN, nCharShort, MAKELONG(nRepCnt, nFlags));
	}
	else
		COleControl::OnKeyDown(nChar, nRepCnt, nFlags);*/
	COleControl::OnKeyDown(nChar, nRepCnt, nFlags);
}
#endif
#endif // 0

#ifdef SS_V40

/*
void AFX_CDECL CSsocxCtrl::FireEvent(DISPID dispid, BYTE* pbParams, ...)
{
	COleControl::FireEvent(dispid, pbParams);
}
*/

/*
void CSsocxCtrl::FireMouseMove(short nButton, short nShiftState, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y)
{
	COleControl::FireMouseMove(nButton, nShiftState, x, y);
}
*/

#endif // SS_V40


#if SS_V80
extern "C" 
{
void CT_LockInPlaceActive(LPDISPATCH lpDisp)
{
   LPDISPATCH lpDispObject = NULL;
   IOleObject *pole = NULL;
   IOleClientSite *psite = NULL;
   IOleControlSite *pctlSite = NULL;
   FPCONTROL ctl = {0};
   // first try to get Object property for control's actual dispinterface instead of extender control
   ctl.lpDispatch = (long)lpDisp;
   fpVBGetControlProperty(&ctl, 0, &lpDispObject, _T("Object"), VT_DISPATCH);
   if( lpDispObject )
      lpDisp = lpDispObject;

   if( lpDisp && (S_OK == lpDisp->QueryInterface(IID_IOleObject, (void**)&pole)) )
   {
      if( S_OK == pole->GetClientSite(&psite) && psite )
      {
         if( psite && (S_OK == psite->QueryInterface(IID_IOleControlSite, (void**)&pctlSite)) )
         {
            pctlSite->LockInPlaceActive(TRUE);
            pctlSite->Release();
         }
         psite->Release();
      }
      pole->Release();
   }
   if( lpDispObject )
      lpDispObject->Release();
}

void CT_UnlockInPlaceActive(VARIANT *Ctrl)
{
   LPDISPATCH lpDisp = NULL;
   LPDISPATCH lpDispObject = NULL;
   IOleObject *pole = NULL;
   IOleClientSite *psite = NULL;
   IOleControlSite *pctlSite = NULL;
   FPCONTROL ctl = {0};

   if( Ctrl->vt == VT_DISPATCH )
   {
      // first try to get Object property for control's actual dispinterface instead of extender control
      ctl.lpDispatch = (long)Ctrl->pdispVal;
      fpVBGetControlProperty(&ctl, 0, &lpDispObject, _T("Object"), VT_DISPATCH);
      if( lpDispObject )
         lpDisp = lpDispObject;
      else
         lpDisp = Ctrl->pdispVal;

      if( lpDisp && (S_OK == lpDisp->QueryInterface(IID_IOleObject, (void**)&pole)) )
      {
         if( S_OK == pole->GetClientSite(&psite) && psite )
         {
            if( psite && (S_OK == psite->QueryInterface(IID_IOleControlSite, (void**)&pctlSite)) )
            {
               pctlSite->LockInPlaceActive(FALSE);
               pctlSite->Release();
            }
            psite->Release();
         }
         pole->Release();
      }
      if( lpDispObject )
         lpDispObject->Release();
   }
}

// this helper method gets the dispatch pointer from the window property
// and initializes the VARIANT to be passed to the custom celltype event
// for the control argument.
void CT_InitControlArg(HWND hWndSpread, VARIANT *Ctrl, HWND hWndCtrl)
{
//   LPDISPATCH lpDisp = CT_GetIDispatch(hWnd);
   LPSPREADSHEET lpSS = SS_SheetLock(hWndSpread);
   LPDISPATCH lpDisp = (LPDISPATCH)SS_GetControlDispatchFromHwnd(lpSS, hWndCtrl);

   SS_SheetUnlock(hWndSpread);
   VariantInit(Ctrl);
   if( lpDisp )
   {
      Ctrl->vt = VT_DISPATCH;
      Ctrl->pdispVal = lpDisp;
      fpDispatchAddRef((long)lpDisp);
      CT_LockInPlaceActive(lpDisp);
   }
   else
   {
      Ctrl->vt = VT_HANDLE;
      Ctrl->lVal = (long)hWndCtrl;
   }
}
// gets the OCX class for firing events
CSsocxCtrl *CT_GetSpreadClass(HWND hWndSpread)
{
   LPSS_BOOK lpBook = SS_BookLock(hWndSpread);
   CSsocxCtrl *pCtrl = (CSsocxCtrl*)lpBook->lpOleControl;
   SS_BookUnlock(hWndSpread);
   return pCtrl;
}
// copies a SS_CT_VALUE to a VARIANT (SS_CT_VALUE is not cleared)
void CT_ValueToVariant(VARIANT *Variant, SS_CT_VALUE *Value)
{
   LPTSTR lpsz;
//   int len;

   VariantInit(Variant);
   switch( Value->type )
   {
   case SS_CT_VALUE_TYPE_BUFFER:
      {
         SAFEARRAYBOUND bounds;
         LPVOID lpValue;

         if( lpValue = GlobalLock(Value->u.Buffer.hBuffer) )
         {
            bounds.lLbound = 0;
            bounds.cElements = Value->u.Buffer.lSize;
            Variant->parray = SafeArrayCreate(VT_UI1, 1, &bounds);
            if( S_OK == SafeArrayLock(Variant->parray) )
            {
               Variant->vt = VT_ARRAY | VT_UI1;
               MemHugeCpy(Variant->parray->pvData, lpValue, Value->u.Buffer.lSize);
               SafeArrayUnlock(Variant->parray);
            }
            else
               Variant->parray = NULL;
            GlobalUnlock(Value->u.Buffer.hBuffer);
         }
      }
      break;
   case SS_CT_VALUE_TYPE_TSTR:
      Variant->vt = VT_BSTR;
      lpsz = (LPTSTR)GlobalLock(Value->u.hszValue);
      Variant->bstrVal = (BSTR)fpSysAllocBSTRFromTString(lpsz);
      GlobalUnlock(Value->u.hszValue);
      break;
      //len = lstrlen(lpsz);
      //Variant->bstrVal = SysAllocStringByteLen(NULL, (len + 1) * sizeof(TCHAR));
      //MultiByteToWideChar(CP_ACP, 0, lpsz, len, Variant->bstrVal, len);
      //Variant->bstrVal[len] = 0;
   case SS_CT_VALUE_TYPE_LONG:
      Variant->vt = VT_I4;
      Variant->lVal = Value->u.lValue;
      break;
   case SS_CT_VALUE_TYPE_DOUBLE:
      Variant->vt = VT_R8;
      Variant->dblVal = Value->u.dblValue;
      break;
   }
}
// copies a VARIANT to a SS_CT_VALUE and clears the variant
BOOL CT_VariantToValue(VARIANT *Variant, SS_CT_VALUE *Value)
{
   BSTR bstr;
   int len = 0;

   Value->type = SS_CT_VALUE_TYPE_EMPTY;
   switch( Variant->vt )
   {
   case VT_ARRAY|VT_UI1: // Byte()
      if( Variant->parray->cDims == 1 ) // must be one dimension
      {
         long lSize = Variant->parray->rgsabound[0].cElements;
         LPVOID lpValue;

         Value->u.Buffer.hBuffer = GlobalAlloc(GHND, lSize);
         if( lpValue = GlobalLock(Value->u.Buffer.hBuffer) )
         {
            if( S_OK == SafeArrayLock(Variant->parray) )
            {
               Value->type = SS_CT_VALUE_TYPE_BUFFER;
               Value->u.Buffer.lSize = lSize;
               MemHugeCpy(lpValue, Variant->parray->pvData, lSize);
               SafeArrayUnlock(Variant->parray);
            }
            GlobalUnlock(Value->u.Buffer.hBuffer);
         }
         else
            Value->u.Buffer.hBuffer = NULL;
      }
      break;
   case VT_BSTR:
   case VT_BSTR | VT_BYREF:
      Value->type = SS_CT_VALUE_TYPE_TSTR;
      if( Variant->vt & VT_BYREF )
         bstr = *Variant->pbstrVal;
      else
         bstr = Variant->bstrVal;
	  if (bstr != NULL)
		len = wcslen(bstr);
      Value->u.hszValue = GlobalAlloc(GHND, (len + 1) * sizeof(TCHAR));
      if( Value->u.hszValue )
      {
         LPTSTR lpszTemp = (LPTSTR)GlobalLock(Value->u.hszValue);
#if _UNICODE
         lstrcpy(lpszTemp, (LPTSTR)bstr);
#else
         WideCharToMultiByte(CP_ACP, 0, bstr, -1, lpszTemp, len + 1, NULL, NULL);
#endif
         GlobalUnlock(Value->u.hszValue);
      }
      break;
   case VT_I2:
   case VT_I4:
   case VT_BOOL:
   case VT_I1:
   case VT_UI1:
   case VT_UI2:
   case VT_UI4:
   case VT_INT:
   case VT_UINT:
   case VT_I2 | VT_BYREF:
   case VT_I4 | VT_BYREF:
   case VT_BOOL | VT_BYREF:
   case VT_I1 | VT_BYREF:
   case VT_UI1 | VT_BYREF:
   case VT_UI2 | VT_BYREF:
   case VT_UI4 | VT_BYREF:
   case VT_INT | VT_BYREF:
   case VT_UINT | VT_BYREF:

      Value->type = SS_CT_VALUE_TYPE_LONG;
      if( Variant->vt & VT_BYREF )
         Value->u.lValue = *Variant->plVal;
      else
         Value->u.lValue = Variant->lVal;
      break;
   case VT_R4:
   case VT_R8:
   case VT_R4 | VT_BYREF:
   case VT_R8 | VT_BYREF:
      Value->type = SS_CT_VALUE_TYPE_DOUBLE;
      if( Variant->vt & VT_BYREF )
         Value->u.dblValue = *Variant->pdblVal;
      else
         Value->u.dblValue = Variant->dblVal;
      break;
   }
   return Value->type != SS_CT_VALUE_TYPE_EMPTY;
}

BOOL CALLBACK CT_CancelEditing(LPSS_CT_CANCELEDITING_ARGS pArgs)
{
   VARIANT EditorControl;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &EditorControl, pArgs->hWndEditor);
   if( spread )
      spread->FireCTCancelEditing(pArgs->lpszName, &EditorControl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row);
   CT_UnlockInPlaceActive(&EditorControl);
   VariantClear(&EditorControl);
   return TRUE;
}

BOOL CALLBACK CT_CreateEditorControl(LPSS_CT_CREATEEDITORCONTROL_ARGS pArgs)
{
   BOOL Ret = FALSE;
   VARIANT EditorControl;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   VariantInit(&EditorControl);
   if( spread )
      spread->FireCTCreateEditorControl(pArgs->lpszName, pArgs->lStyle, pArgs->CtrlId, &EditorControl);
   if( EditorControl.vt == VT_HANDLE && IsWindow((HWND)EditorControl.lVal) )
   {
      pArgs->hWndEditor = (HWND)EditorControl.lVal;
      Ret = TRUE;
   }
   else 
   {
      FPCONTROL ctl = {0};

      if( EditorControl.vt == VT_DISPATCH )
      {
         ctl.lpDispatch = (long)EditorControl.pdispVal;
         pArgs->hWndEditor = (HWND)fpVBGetControlHwnd(&ctl);
         if( pArgs->hWndEditor == NULL )
         { // try to force window creation
            fpVBRecreateControlHwnd(&ctl);
            pArgs->hWndEditor = (HWND)fpVBGetControlHwnd(&ctl);
         }
         if( pArgs->hWndEditor )
         {
            long ldisp = (long)GetProp(pArgs->hWndEditor, PDISPATCH);
            if( ldisp == 0 )
            {
               SetProp(pArgs->hWndEditor, PDISPATCH, (HANDLE)ctl.lpDispatch);
               fpDispatchAddRef(ctl.lpDispatch);
            }
//            if( (long)GetProp(pArgs->hWndEditor, PDISPATCH) != ctl.lpDispatch && SetProp(pArgs->hWndEditor, PDISPATCH, (HANDLE)ctl.lpDispatch))
//               fpDispatchAddRef(ctl.lpDispatch);
            Ret = TRUE;
         }
      }
   }
   VariantClear(&EditorControl);
   return Ret;
}

BOOL CALLBACK CT_CreateRendererControl(LPSS_CT_CREATERENDERERCONTROL_ARGS pArgs)
{
   BOOL Ret = FALSE;
   VARIANT RendererControl;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   VariantInit(&RendererControl);
   if( spread )
      spread->FireCTCreateRendererControl(pArgs->lpszName, pArgs->lStyle, pArgs->CtrlId, &RendererControl);
   if( RendererControl.vt == VT_HANDLE && IsWindow((HWND)RendererControl.lVal) )
   {
      pArgs->hWndRenderer = (HWND)RendererControl.lVal;
      Ret = TRUE;
   }
   else 
   {
      FPCONTROL ctl = {0};

      if( RendererControl.vt == VT_DISPATCH )
      {
         ctl.lpDispatch = (long)RendererControl.pdispVal;
         pArgs->hWndRenderer = (HWND)fpVBGetControlHwnd(&ctl);
         if( pArgs->hWndRenderer == NULL )
         { // try to force window creation
            fpVBRecreateControlHwnd(&ctl);
            pArgs->hWndRenderer = (HWND)fpVBGetControlHwnd(&ctl);
         }
         if( pArgs->hWndRenderer )
         {
            long ldisp = (long)GetProp(pArgs->hWndRenderer, PDISPATCH);
            if( ldisp == 0 )
            {
               SetProp(pArgs->hWndRenderer, PDISPATCH, (HANDLE)ctl.lpDispatch);
               fpDispatchAddRef(ctl.lpDispatch);
            }
//            if( (long)GetProp(pArgs->hWndRenderer, PDISPATCH) != ctl.lpDispatch && SetProp(pArgs->hWndRenderer, PDISPATCH, (HANDLE)ctl.lpDispatch))
//               fpDispatchAddRef(ctl.lpDispatch);
            Ret = TRUE;
         }
      }
   }
   VariantClear(&RendererControl);
   return Ret;
}

BOOL CALLBACK CT_GetEditorValue(LPSS_CT_GETEDITORVALUE_ARGS pArgs)
{
   VARIANT EditorControl;
   VARIANT Value;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &EditorControl, pArgs->hWndEditor);
   VariantInit(&Value);
   if( spread )
      spread->FireCTGetEditorValue(pArgs->lpszName, &EditorControl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, &Value);
   CT_VariantToValue(&Value, &pArgs->Value);
   CT_UnlockInPlaceActive(&EditorControl);
   VariantClear(&EditorControl);
   VariantClear(&Value);
   return pArgs->Value.type != SS_CT_VALUE_TYPE_EMPTY;
}

BOOL CALLBACK CT_InitializeControl(LPSS_CT_INITIALIZECONTROL_ARGS pArgs)
{
   BOOL Ret = FALSE;
   VARIANT Ctrl;
   CCTAppearance *Appearance = new CCTAppearance(&pArgs->Appearance);
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &Ctrl, pArgs->hWndCtrl);
   if( spread )
      spread->FireCTInitializeControl(pArgs->lpszName, &Ctrl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, Appearance);
   CT_UnlockInPlaceActive(&Ctrl);
   VariantClear(&Ctrl);
   Appearance->Release();
   return TRUE;
}

BOOL CALLBACK CT_IsReservedKey(LPSS_CT_ISRESERVEDKEY_ARGS pArgs)
{
   VARIANT RendererControl;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);
   short nKeyCode = pArgs->nKeyCode;
   BOOL reserved = TRUE;

   CT_InitControlArg(pArgs->hWndSpread, &RendererControl, pArgs->hWndRenderer);
   if( spread )
      spread->FireCTIsReservedKey(pArgs->lpszName, &RendererControl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, nKeyCode, pArgs->nShift, &reserved);
   CT_UnlockInPlaceActive(&RendererControl);
   VariantClear(&RendererControl);
   return reserved;
}

BOOL CALLBACK CT_IsReservedLocation(LPSS_CT_ISRESERVEDLOCATION_ARGS pArgs)
{
   VARIANT RendererControl;
   CCTAppearance *Appearance = new CCTAppearance(&pArgs->Appearance);
   VARIANT Value;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);


   CT_InitControlArg(pArgs->hWndSpread, &RendererControl, pArgs->hWndRenderer);
   CT_ValueToVariant(&Value, &pArgs->Value);
   if( spread )
      spread->FireCTIsReservedLocation(pArgs->lpszName, &RendererControl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, pArgs->p.x, pArgs->p.y, pArgs->rect.left, pArgs->rect.top, pArgs->rect.right, pArgs->rect.bottom, Appearance, &Value, &pArgs->lReservedCursorId);
   CT_UnlockInPlaceActive(&RendererControl);
   VariantClear(&RendererControl);
   VariantClear(&Value);
   Appearance->Release();
   return pArgs->lReservedCursorId != 0;
}

BOOL CALLBACK CT_IsValid(LPSS_CT_ISVALID_ARGS pArgs)
{
   BOOL IsValid = TRUE;
   VARIANT EditorControl;
   VARIANT Value;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &EditorControl, pArgs->hWndEditor);
   CT_ValueToVariant(&Value, &pArgs->Value);
   if( spread )
      spread->FireCTIsValid(pArgs->lpszName, &EditorControl, &Value, &IsValid);
   CT_UnlockInPlaceActive(&EditorControl);
   VariantClear(&EditorControl);
   VariantClear(&Value);
   return IsValid;
}

BOOL CALLBACK CT_GetPreferredSize(LPSS_CT_GETPREFERREDSIZE_ARGS pArgs)
{
   VARIANT RendererControl;
   VARIANT Value;
   CCTAppearance *Appearance = new CCTAppearance(&pArgs->Appearance);
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &RendererControl, pArgs->hWndRenderer);
   CT_ValueToVariant(&Value, &pArgs->Value);
   if( spread )
      spread->FireCTGetPreferredSize(pArgs->lpszName, &RendererControl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, (OLE_HANDLE)pArgs->hDC, pArgs->rect.left, pArgs->rect.top, pArgs->rect.right, pArgs->rect.bottom, Appearance, &Value, &pArgs->sizePreferred.cx, &pArgs->sizePreferred.cy);
   CT_UnlockInPlaceActive(&RendererControl);
   VariantClear(&RendererControl);
   VariantClear(&Value);
   Appearance->Release();
   return TRUE;
}

BOOL CALLBACK CT_GetReservedCursor(LPSS_CT_GETRESERVEDCURSOR_ARGS pArgs)
{
   BOOL Ret = FALSE;
   VARIANT RendererControl;
   VARIANT ReservedCursor;
   IPicture *ipict;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &RendererControl, pArgs->hWndRenderer);
   VariantInit(&ReservedCursor);
   if( spread )
      spread->FireCTGetReservedCursor(pArgs->lpszName, &RendererControl, pArgs->lReservedCursorId, &ReservedCursor);
   if( ReservedCursor.vt == VT_DISPATCH && ReservedCursor.pdispVal->QueryInterface(IID_IPicture, (void**)&ipict) )
   {
      short type;

      ipict->get_Type(&type);
      if( type == PICTYPE_ICON )
      {
         ipict->get_Handle((UINT*)&pArgs->hCursor);
         Ret = TRUE;
      }
      ipict->Release();
   }
   else if( ReservedCursor.vt = VT_HANDLE )
   {
      HICON h = (HICON)ReservedCursor.lVal;
      ICONINFO io;

      if( GetIconInfo(h, &io) )
      {
         pArgs->hCursor = h;
         Ret = TRUE;
         DeleteObject(io.hbmColor);
         DeleteObject(io.hbmMask);
      }
   }
   CT_UnlockInPlaceActive(&RendererControl);
   VariantClear(&RendererControl);
   VariantClear(&ReservedCursor);
   return Ret;
}

BOOL CALLBACK CT_PaintCell(LPSS_CT_PAINTCELL_ARGS pArgs)
{
   BOOL Handled = FALSE;
   VARIANT RendererControl;
   VARIANT Value;
   CCTAppearance *Appearance = new CCTAppearance(&pArgs->Appearance);
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &RendererControl, pArgs->hWndRenderer);
   VariantInit(&Value);
   CT_ValueToVariant(&Value, &pArgs->Value);
   if( spread )
      spread->FireCTPaintCell(pArgs->lpszName, &RendererControl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, &Value, (OLE_HANDLE)pArgs->hDC, pArgs->rect.left, pArgs->rect.top, pArgs->rect.right, pArgs->rect.bottom, Appearance, pArgs->fIsSelected, pArgs->fIsLocked, &Handled);
/*   if( !Handled && EditorControl.vt == VT_DISPATCH )
   {  // TODO: Add code to draw ActiveX as metafile
      
      fpRenderControl((long)EditorControl.pdispVal, pArgs->hDC, &pArgs->rect);
      Handled = VARIANT_TRUE;
   }
*/
   CT_UnlockInPlaceActive(&RendererControl);
   VariantClear(&RendererControl);
   VariantClear(&Value);
   Appearance->Release();
   return Handled;
}

BOOL CALLBACK CT_SetValue(LPSS_CT_SETVALUE_ARGS pArgs)
{
   BOOL Handled = FALSE;
   VARIANT Ctrl;
   VARIANT Value;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &Ctrl, pArgs->hWndCtrl);
   CT_ValueToVariant(&Value, &pArgs->Value);
   if( spread )
      spread->FireCTSetValue(pArgs->lpszName, &Ctrl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, &Value, &Handled);
   CT_UnlockInPlaceActive(&Ctrl);
   VariantClear(&Ctrl);
   VariantClear(&Value);
   return Handled;
}

BOOL CALLBACK CT_StartEditing(LPSS_CT_STARTEDITING_ARGS pArgs)
{
   BOOL Handled = FALSE;
   VARIANT EditorControl;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &EditorControl, pArgs->hWndEditor);
   if( spread )
      spread->FireCTStartEditing(pArgs->lpszName, &EditorControl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, pArgs->msg.message, pArgs->msg.wParam, pArgs->msg.lParam, pArgs->fSelectAll, pArgs->fAutoClipboard, &Handled);
   CT_UnlockInPlaceActive(&EditorControl);
   VariantClear(&EditorControl);
   return Handled;
}

BOOL CALLBACK CT_StopEditing(LPSS_CT_STOPEDITING_ARGS pArgs)
{
   VARIANT EditorControl;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &EditorControl, pArgs->hWndEditor);
   if( spread )
      spread->FireCTStopEditing(pArgs->lpszName, &EditorControl, pArgs->Sheet, pArgs->Cell.Col, pArgs->Cell.Row, &pArgs->Cancel);
   CT_UnlockInPlaceActive(&EditorControl);
   VariantClear(&EditorControl);
   return !pArgs->Cancel;
}

BOOL CALLBACK CT_StringFormat(LPSS_CT_STRINGFORMAT_ARGS pArgs)
{
   BOOL Ret = FALSE;
   VARIANT RendererControl;
   VARIANT Value;
   BSTR Formatted = NULL;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &RendererControl, pArgs->hWndRenderer);
   CT_ValueToVariant(&Value, &pArgs->Value);
   if( spread )
      spread->FireCTStringFormat(pArgs->lpszName, &RendererControl, &Value, pArgs->lStyle, &Formatted);
   if( Formatted )
   {
      LPTSTR lpszFormatted = fpSysAllocTStringFromBSTR(Formatted);
      LPTSTR lpszTemp;
      int len = lstrlen(lpszFormatted);

      pArgs->hszFormatted = GlobalAlloc(GHND, (len + 1) * sizeof(TCHAR));
      if( lpszTemp = (LPTSTR)GlobalLock(pArgs->hszFormatted) )
      {
        lstrcpy(lpszTemp, lpszFormatted);
        GlobalUnlock(pArgs->hszFormatted);
        Ret = TRUE;
      }
      fpSysFreeTString(lpszFormatted);
      SysFreeString(Formatted);
   }
   CT_UnlockInPlaceActive(&RendererControl);
   VariantClear(&RendererControl);
   VariantClear(&Value);
   return Ret;
}

BOOL CALLBACK CT_StringUnformat(LPSS_CT_STRINGUNFORMAT_ARGS pArgs)
{
   VARIANT RendererControl;
   VARIANT Value;
   CSsocxCtrl *spread = CT_GetSpreadClass(pArgs->hWndSpread);

   CT_InitControlArg(pArgs->hWndSpread, &RendererControl, pArgs->hWndRenderer);
   VariantInit(&Value);
   if( spread )
      spread->FireCTStringUnformat(pArgs->lpszName, &RendererControl, pArgs->lpszFormatted, pArgs->lStyle, &Value);
   CT_VariantToValue(&Value, &pArgs->Value);
   CT_UnlockInPlaceActive(&RendererControl);
   VariantClear(&RendererControl);
   VariantClear(&Value);
   return pArgs->Value.type != SS_CT_VALUE_TYPE_EMPTY;
}

} // extern "C"

#endif // SS_V80
