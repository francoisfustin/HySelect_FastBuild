/***********************************************************************
* ssocxdb.cpp : Implementation of the DB-Specific CSsocxCtrl methods.
*
* Copyright (C) 1995 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
***********************************************************************/

#define NO_VBX_SUPPORT

#if ((defined(SS_OCX) || defined(FP_DLL)) && defined(WIN32))
#define WIN32_LEAN_AND_MEAN   //exclude conflicts with VBAPI.H in WIN32
#endif

//#include <windows.h>
#include "stdafx.h"

#include "..\sssrc\spread\ss_cell.h"
#include "ssocx.h"
#include "ssocxctl.h"

#if SS_V70 && FP_ADB
#include "..\sssrc\spread\ss_alloc.h"
extern "C" {
// This frees the bookmark if there is one (func is in VBBOUND.C)
void DLLENTRY vbSS_SetBookmark(LPVOID lpDestBookMark, LPVOID srcBookMark);   
}
#endif

#include "..\..\..\fplibs\fptools\src\fpconvrt.h"         // includes fpvbsub.h 
#include "..\..\..\fplibs\fptools\src\fptstr.h"
#include "..\..\..\fplibs\fptools\src\vbinterf.h"
DEFINE_GUID(IID_IVBGetControl, 0x40A050A0L, 0x3C31, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);

#include "..\sssrc\classes\wintools.h"

extern "C" {
#include "..\vbsrc\vbspread.h"
}

#include "..\..\..\fplibs\fpdb\src\fpdbutil.h"

#include "ssocxdb.h"
static void SSOcxInitVBSpread(LPVBSPREAD lpSpread);

//-------------------------------------------------------------------------
// OUTPUT hook from CfpOleBinding --> VBBOUND.C.
//
// This function is called by CfpOleBinding.  A ptr to the function was
// passed to m_fpOleBinding via m_fpOleBinding.SetDataMsgFunc().
//
// The function in VBBOUND.C is called to process the Data (VBM_DATA) message.
//
LRESULT SSOcxBoundProc( LPVOID lpObject, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LRESULT lResult;
  CSsocxCtrl *pThis = (CSsocxCtrl *)lpObject; 
  LPSPREADSHEET lpSS;
#if defined(SS_V70) && defined(FP_ADB)
  CfpOleBinding* lpOleBinding;
  int i, cnt = pThis->GetSheetCount();

  if( uMsg == VBM_DATA_UERROR )
    // RFW - 4/29/04 - 14176 / 14187
    //lpOleBinding = (CfpOleBinding*)((LPDATAACCESS)*((LONG*)lParam + 3))->lpfpOleBinding;
    lpOleBinding = (CfpOleBinding*)((LPDATAACCESS)*((LONG*)lParam + 2))->lpfpOleBinding;
  else if( uMsg == VBM_DATA_POSTMSGTOSELF )
	lpOleBinding = (CfpOleBinding*)lParam;
  else
	lpOleBinding = (CfpOleBinding*)((LPDATAACCESS)lParam)->lpfpOleBinding;

  if( uMsg == VBM_DATA_QUERYUSERMODE )
    lpSS = pThis->LockActiveSheet();
  else
  {
    for( i = 0; i < cnt; i++ )
    {
      lpSS = pThis->LockSheet(i);
      if( lpSS->pfpOleBinding == lpOleBinding )
        break;
      pThis->UnlockSheet(i);
      lpSS = NULL;
    }
    if( !lpSS )
      return -1;
  }
#else
  lpSS = pThis->LockSheet(0);
#endif
  // Lock memory to VBSpread and call function in VBBOUND.C
  pThis->m_lpVBSpread = (LPVOID)tbGlobalLock(pThis->m_hVBSpread);

  lResult = vbSpreadBoundProc(lpSS, (LONG)lpObject, pThis->m_hWnd, (WORD)uMsg,
    (WORD)wParam, lParam);

#if defined(SS_V70) && defined(FP_ADB)
  pThis->UnlockSheet(i);
#else
  pThis->UnlockSheet(0);
#endif
  tbGlobalUnlock(pThis->m_hVBSpread);
  return lResult;
}

//-------------------------------------------------------------------------
// INPUT hook from VBBOUND.C --> CfpOleBinding.
//
// This function is called by VBBOUND.C when it needs a Data call processed.
// It simply calls the function in the CfpOleBinding class.
//
/* extern "C" */ 
SHORT SSOcxDataCall( LONG lObject, LPVOID lpda, USHORT msg, USHORT action,
  SS_COORD row, SS_COORD col, LPSPREADSHEET lpSS)
{
  CSsocxCtrl *pThis = (CSsocxCtrl *)lObject;
//  LPSPREADSHEET lpSS = pThis->LockActiveSheet();
  SS_AdjustCellCoordsOut(lpSS, &col, &row);
//  pThis->UnlockActiveSheet();

  SS_CELLCOORD cell = { col, row };
  
  ((LPDATAACCESS)lpda)->sAction = action;
#if defined(SS_V70) && defined(FP_ADB)
  if( lpSS->pfpOleBinding )
    return ((CfpOleBinding*)lpSS->pfpOleBinding)->DataCall( lpda, msg, action, (LPVOID)&cell);
  return -1;
#else
  return pThis->m_fpOleBinding.DataCall( lpda, msg, action, (LPVOID)&cell);
#endif
}


//-------------------------------------------------------------------------
// This function is called by VBBOUND.C when it needs a ptr to the
// VBSPREAD structure (m_lpvbSpread).  At this point, the memory has
// already been locked.
//
/* extern "C" */ 
LPVOID SSOcxVBDerefControl( LONG lObject)
{
  return ((CSsocxCtrl *)lObject)->m_lpVBSpread;
}

//-------------------------------------------------------------------------
// This function locks the VBSPREAD memory before a call to VBBOUND.C.
// VBBOUND.C calls VBDrefControl to get ptr to VBSPREAD structure.
//
// NOTE:  The member variable "m_lpVBSpread" is set by this function
//
/* extern "C" */ 
LPVOID SSOcxLockVBSpread( LONG lObject)
{
  CSsocxCtrl *pThis = (CSsocxCtrl *)lObject;  

  // Lock memory to VBSpread
  pThis->m_lpVBSpread = (LPVOID)tbGlobalLock(pThis->m_hVBSpread);
  return pThis->m_lpVBSpread;
}

//-------------------------------------------------------------------------
// This function Unlocks the VBSPREAD memory.
//
/* extern "C" */ 
VOID SSOcxUnlockVBSpread( LONG lObject)
{
  CSsocxCtrl *pThis = (CSsocxCtrl *)lObject;  

  // Unlock memory to VBSpread
  tbGlobalUnlock(pThis->m_hVBSpread);
}

/* extern "C" */ 
BOOL SSOcxSetTypePictPicture(LONG lObject, SS_COORD lCol, SS_COORD lRow,
  HBITMAP hBitmap, HPALETTE hPal, HICON hIcon) 
{
  CSsocxCtrl *pThis = (CSsocxCtrl *)lObject;  
  return pThis->PrivateSetTypePictPicture(lCol, lRow, hBitmap, hPal, hIcon);
}

//-------------------------------------------------------------------------
// Allocate memory for VBSPREAD structure
//
BOOL SSOcxCreateVBSpread(LPTBGLOBALHANDLE lphVBSpread)
{
  LPVBSPREAD lpSpread;

#ifndef SS_NO_USE_SH
  // Set pool for LBlock to use.
  LBlockSetStringPool((DWORD)tbStringPool);
#endif

  *lphVBSpread = tbGlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(VBSPREAD));
  
  // Verify that allocation was successful
  ASSERT( *lphVBSpread != (TBGLOBALHANDLE)NULL);

  // Lock memory to VBSpread and set default values
  lpSpread = (LPVBSPREAD)tbGlobalLock(*lphVBSpread);
  if (lpSpread)
  {
     SSOcxInitVBSpread(lpSpread);

     tbGlobalUnlock(*lphVBSpread);
  }

  return (*lphVBSpread != (TBGLOBALHANDLE)NULL);
}

//-------------------------------------------------------------------------
// Free memory allocated for VBSPREAD
//
void SSOcxDestroyVBSpread( TBGLOBALHANDLE hVBSpread)
{
  // "Free" pool used by LBlock
  LBlockSetStringPool((DWORD)0);

  if (hVBSpread)
     tbGlobalFree(hVBSpread);

  return;
}

#if !defined(SS_V70) || !defined(FP_ADB)
//-------------------------------------------------------------------------
// Setup CfpOleBinding to interface with this Ole control
//
void SSOcxInitOleBinding( CSsocxCtrl FAR *lpSSOcx)
{
  lpSSOcx->m_fpOleBinding.SetOwner( (COleControl FAR *)lpSSOcx);
  lpSSOcx->m_fpOleBinding.SetDataSourceDispid( lpSSOcx->dispidDataSource);
  lpSSOcx->m_fpOleBinding.SetDataMsgFunc( (LPVOID)SSOcxBoundProc);
  lpSSOcx->m_fpOleBinding.SetBindToBookmark(TRUE);

  return;
}
#else // SS_V70 && FP_ADB
void SSOcxInitSheetOleBinding(CSsocxCtrl FAR *lpSSOcx, LPSPREADSHEET lpSS)
{
  CfpOleBinding *pBinding;

  if( lpSS->pfpOleBinding )
	  SSOcxFreeSheetOleBinding(lpSS);
  pBinding = new CfpOleBinding();
  pBinding->SetOwner( (COleControl FAR *)lpSSOcx);
  pBinding->SetDataSourceDispid( lpSSOcx->dispidDataSource);
  pBinding->SetDataMsgFunc( (LPVOID)SSOcxBoundProc);
  pBinding->SetBindToBookmark(TRUE);
  lpSS->pfpOleBinding = pBinding;
}
void SSOcxFreeSheetOleBinding(LPSPREADSHEET lpSS)
{
  if( lpSS->pfpOleBinding )
  { // 11457 -> -scl
	LPSS_ROW lpRow;
    for (SS_COORD i = (lpSS->fVirtualMode ? lpSS->Virtual.VTop : lpSS->Row.HeaderCnt);
         i < (lpSS->fVirtualMode ? lpSS->Row.DataCnt : lpSS->Row.AllocCnt); i++)
       if (lpRow = SS_LockRowItem(lpSS, i))
          {
          vbSS_SetBookmark( (LPVOID)&lpRow->hlstrBookMark, NULL);
          SS_UnlockRowItem(lpSS, i);
          }
	lpSS->fBoundActive = FALSE;

   ((CfpOleBinding*)lpSS->pfpOleBinding)->SetDataSource(NULL); // RFW - 4/11/08 - 22132

	// 11457 <- -scl
// ...9181
    delete (CfpOleBinding *)lpSS->pfpOleBinding; // RFW - 8/26/05 - 16366
    lpSS->pfpOleBinding = 0; // RFW - 8/20/04 - 15061
  }
}
#endif
void SSOcxFireDbEditError(LONG lObject, LONG lDbErrCode, 
  LPSS_CELLCOORD lpCell)
{
  CSsocxCtrl *pThis = (CSsocxCtrl *)lObject;  
  SS_COORD lCol = -1;
  SS_COORD lRow = -1;
  short sEditError;

  if (lpCell)
  {
    lCol = SS_HEADER == lpCell->Col ? 0 : lpCell->Col;
    lRow = SS_HEADER == lpCell->Row ? 0 : lpCell->Row;
  }

  // translate from FPDB error code to SPREAD EditError code
  switch (lDbErrCode)
  {
    case FPDB_ERR_BEGINUPDATE_ADD: 
      sEditError = EDITERROR_DB_DAO_BEGINUPDATE_ADD_FAILED; 
      break;
    case FPDB_ERR_BEGINUPDATE_UPDATE: 
      sEditError = EDITERROR_DB_DAO_BEGINUPDATE_UPDATE_FAILED; 
      break;

    case FPDB_ERR_CANTSETAUTOINCR:
    case FPDB_ERR_CONVERTSTRINGTOTYPE:
      sEditError = EDITERROR_DB_SETVALUE_FAILED;
      break;

    case FPDB_ERR_DELETE:
      sEditError = EDITERROR_DB_DAO_DELETE_FAILED;
      break;

    case FPDB_ERR_FETCHCURRENTROW:
    case FPDB_ERR_GETNEXTROW:
      sEditError = EDITERROR_DB_CURRENTPOSITIONINVALID;
      break;

    case FPDB_ERR_LBLOCKALLOC: 
    case FPDB_ERR_MEMLOCK_DATABUFFER: 
      sEditError = EDITERROR_DB_MEMORYALLOCATION; 
      break;

    case FPDB_ERR_NOTIMPLEMENTED:
      sEditError = EDITERROR_DB_NOTSUPPORTED;
      break;
    case FPDB_ERR_QINTF_UPDATECURSOR:
      sEditError = EDITERROR_DB_NONUPDATABLE_RECORDSET;
      break;
    case FPDB_ERR_SETCOLUMN:
      sEditError = EDITERROR_DB_DAO_SETCOLUMN_FAILED;
      break;
    case FPDB_ERR_UPDATE:
      sEditError = EDITERROR_DB_DAO_UPDATE_FAILED;
      lCol = (SS_COORD)-1;
      break;
    case FPDB_ERR_UPDATEINPROGRESS:
      sEditError = EDITERROR_DB_UPDATEINPROGRESS;
      break;
    case FPDB_ERR_MOVETO_NEWROW:
      sEditError = EDITERROR_DB_MOVETONEWROW_FAILED;
      break;
    default:
      sEditError = EDITERROR_DB_OPERATION_FAILED;
      break;
  }

  pThis->FireEditError(lCol, lRow, sEditError);
}

//--------------------------------------------------------------------



void SSOcxFireDataFill(LONG lObject, long Col, long Row, short DataType, 
  short fGetData, short FAR* lpCancel, LPSPREADSHEET lpSS)
{
  CSsocxCtrl *pThis = (CSsocxCtrl *)lObject;  
#ifdef SS_V35
  {
  LPSS_BOOK lpBook = pThis->LockBook();
  short nCancel = *lpCancel;

#ifdef SS_V70
  lpBook->nSheetSendingMsg = lpSS->nSheetIndex;
#endif
  if (lpBook->bScriptEnhanced)
     {
     VARIANT arg;
     VARIANT varParam;

     memset(&arg, 0, sizeof(VARIANT));
     memset(&varParam, 0, sizeof(VARIANT));
     varParam.vt = VT_I2; 
     varParam.iVal  = nCancel; 

     pThis->FireScriptDataFill(Col, Row, DataType, fGetData, &varParam);
     VariantChangeType(&arg, &varParam, 0, VT_I2); 

     nCancel = arg.iVal;
     }
  else
     pThis->FireDataFill(Col, Row, DataType, fGetData, &nCancel);

  *lpCancel = nCancel;

  pThis->UnlockBook();
  }
#else
  pThis->FireDataFill(Col, Row, DataType, fGetData, lpCancel);
#endif
}

void SSOcxFireDataColConfig(LONG lObject, long Col, LPCTSTR DataField, short DataType, LPSPREADSHEET lpSS)
{
  CSsocxCtrl *pThis = (CSsocxCtrl *)lObject;  
  
  // BJO 21Mar96 SCS2475 - removed TSTR to BSTR conversion
  //
  //BOOL   fFreeParam = FALSE;
  //if (sizeof(OLECHAR) != sizeof(TCHAR))
  //{
  //  DataField = (LPCTSTR)fpSysAllocBSTRFromTString((LPTSTR)DataField);
  //  fFreeParam = TRUE;
  //}  

#ifdef SS_V70
  lpSS->lpBook->nSheetSendingMsg = lpSS->nSheetIndex;
#endif

  pThis->FireDataColConfig(Col, DataField, DataType);

  //if (fFreeParam && DataField)
  //  fpSysFreeTString((LPTSTR)DataField);
}

BOOL CSsocxCtrl::PrivateSetTypePictPicture(SS_COORD lCol, SS_COORD lRow, 
  HBITMAP hBitmap, HPALETTE hPal, HICON hIcon) 
{
  PICTDESC pictDesc;                      // OLE structure
  LPPICTURE lpPict = NULL;                // OLE IPicture interface ptr
  HRESULT h;

  pictDesc.cbSizeofstruct = sizeof(pictDesc);
  if (hIcon == NULL)
  {
    pictDesc.picType = PICTYPE_BITMAP;
    pictDesc.bmp.hbitmap = hBitmap;
    pictDesc.bmp.hpal = hPal;
  }
  else
  {
    pictDesc.picType = PICTYPE_ICON;
    pictDesc.icon.hicon = hIcon;
  }

  h = OleCreatePictureIndirect(&pictDesc, IID_IPicture, TRUE, 
                               (LPVOID FAR *)&lpPict);

  if (m_lppictTypePictPicture)
    m_lppictTypePictPicture->Release();

  m_lppictTypePictPicture = NULL;

  if ((h == S_OK) && lpPict)
  {
    // save member variables for col & row
    LONG lColSave = m_lCol;
    LONG lRowSave = m_lRow;

    m_lCol = lCol;
    m_lRow = lRow;
    m_lppictTypePictPicture = lpPict;
    SetType(dispidTypePictPicture);

    // restore member variables (col & row)
    m_lCol = lColSave;
    m_lRow = lRowSave;
  }

  return TRUE;
}

// SSM_POSTDATACHANGE
LONG CSsocxCtrl::OnSsmPostDataChange(UINT wParam, LONG lParam)
{
   LPVBSPREAD lpSpread;

   lpSpread = (LPVBSPREAD)SSOcxLockVBSpread((LONG)this);
   if (lpSpread)
   {
      if (lpSpread->DataOpts.fAutoSave)
         {
         LPSPREADSHEET lpSS = LockActiveSheet();
         VBSSBoundManualSave(lpSS, (LONG)this, m_hWnd, lpSpread, lpSS->Row.HeaderCnt,
                             max(lpSS->Row.DataCnt, (SS_COORD)lParam + 1), TRUE);
         UnlockActiveSheet();
         }
      lpSpread->fChangeMsgPosted = FALSE;
      SSOcxUnlockVBSpread((LONG)this);
   }
   return (0);
}

// SSM_POSTADDNEW
LONG CSsocxCtrl::OnSsmPostAddNew(UINT wParam, LONG lParam)
{
   LPSPREADSHEET lpSS = LockActiveSheet();
   SSOcxLockVBSpread((LONG)this);
   VBSSBoundProcessPostAdd(lpSS, (LONG)this, m_hWnd, lParam);
   SSOcxUnlockVBSpread((LONG)this);
   UnlockActiveSheet();

   return (0);
}

// SSM_BOUNDCLICK
LONG CSsocxCtrl::OnSsmBoundClick(UINT wParam, LONG lParam)
{
   LPSPREADSHEET lpSS = LockActiveSheet();
   SSOcxLockVBSpread((LONG)this);
   vbSpreadBoundClick(lpSS, (LONG)this, ((LPSS_CELLCOORD)lParam)->Row);
   SSOcxUnlockVBSpread((LONG)this);
   UnlockActiveSheet();
   return (0);
}

// SSM_PROCESSSAVEBOUND
/*
LONG CSsocxCtrl::OnSsmProcessSaveBound(UINT wParam, LONG lParam)
{
   LPSPREADSHEET lpSS = SS_Lock(m_hWnd);
   if (lpSS && lpSS->fSaveBoundData)
   {
      SSOcxLockVBSpread((LONG)this);
      vbSpreadSaveBoundRow((LONG)this, m_hWnd, lpSS->Row.CurAt, lpSS->Row.CurAt,
                           lpSS->Col.CurAt);
      lpSS->fSaveBoundData = FALSE;
      SSOcxUnlockVBSpread((LONG)this);
   }
   if (lpSS)
      SS_Unlock(m_hWnd);

   return (0);
}
*/

static void SSOcxInitVBSpread(LPVBSPREAD lpSpread)
{
lpSpread->Col = ALLCOLS;
lpSpread->Row = ALLROWS;

#ifdef SS_BOUNDCONTROL
lpSpread->DataOpts.fHeadings = TRUE;
lpSpread->DataOpts.fCellTypes = TRUE;
lpSpread->DataOpts.fAutoFill = TRUE;
lpSpread->DataOpts.fSizeCols = 2;
lpSpread->DataOpts.fAutoDataInform = TRUE;
lpSpread->DataOpts.fAutoSave = TRUE;
lpSpread->hlstrLastBookMark = NULL;
#endif

lpSpread->dDataGetFillCol = -1;
lpSpread->dDataSetFillCol = -1;
}

void SSOcxInitDB(TBGLOBALHANDLE hVBSpread)
{
  LPVBSPREAD lpSpread;
  lpSpread = (LPVBSPREAD)tbGlobalLock(hVBSpread);
  if (lpSpread)
  {
     SSOcxInitVBSpread(lpSpread);
     tbGlobalUnlock(hVBSpread);
  }
}

//-----------------------------------------------------------------------
// This is called by vbSpreadBoundProc() if there is not a hWnd at the time
// when the container is initializing the Data Bound notification for the
// control.  To proceed, we must first determine if we are operating in
// an environment OTHER THAN VB, if we are not in VB, try to create the
// hWnd, otherwise, return false.
//
HWND SSOcxDBCreateHWnd(void *pObject)
{
#ifndef WIN32
  return NULL;  // don't create for WIN16
#else // WIN32
  IVBGetControl *pGetControl; 
  LPDISPATCH lpdispExtendedCtrl;
  BOOL fIsVB = FALSE;
  CSsocxCtrl *pThis = (CSsocxCtrl *)pObject;

  // Determine if this is VB.
  if (lpdispExtendedCtrl = pThis->GetExtendedControl())
  {
    if (NOERROR == lpdispExtendedCtrl->QueryInterface(IID_IVBGetControl, (LPVOID FAR *)&pGetControl))
    {
      fIsVB = TRUE;
      pGetControl->Release();
    }
    lpdispExtendedCtrl->Release();
  }

  // If not VB, Try to create the hWnd for the control.
  if (!fIsVB)
    pThis->SSCreateWindowForSubclassedCtrl();

  // If not VB environment and we have a hWnd return hWnd, else NULL.
  return ((!fIsVB && pThis->m_hWnd) ? pThis->m_hWnd : NULL);
#endif // if-else WIN32
}

//-----------------------------------------------------------------------
// This function makes "CreateWindowForSubclassedControl()" public, so
// it can be called by a function whose's not a member function of the
// class.
//
void CSsocxCtrl::SSCreateWindowForSubclassedCtrl(void)
{
  COleControl::CreateWindowForSubclassedControl();
}
