// ssocxpr2.cpp : Implementation of the CSsocxCtrl OLE control properties.

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
#include "..\sssrc\spread\ss_emode.h"
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
#include "..\vbsrc\vbdattim.h"
#ifdef SS_V70
#include "..\sssrc\spread\ss_book.h"
#endif // SS_V70

//DBOCX...
#include "ssocxdb.h"
#include "..\..\..\fplibs\fptools\src\fpconvrt.h"   // includes fpvbsub.h 
// this define has been added to the Spread 3.5 projects
//#ifdef SS_V35
//#define TT_V2
//#endif
#include "texttip.h"

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

#define SS_VALIGNTRANSLATE(lParam) (lParam == 1 ? SSS_ALIGN_BOTTOM : (lParam == 2 ? SSS_ALIGN_VCENTER : SSS_ALIGN_TOP))
#define SS_HALIGNTRANSLATE(lParam) (lParam == 1 ? ES_RIGHT : (lParam == 2 ? ES_CENTER : ES_LEFT))

//--------------------------------------------------------------------

// function found in ssocxprp.cpp
extern GLOBALHANDLE CStringToHandle(CString s);

LONG FP_Sort(BOOL fSetProp, short nIndex, long lValue, BOOL fOrder);
DWORD SS_TextOrientTranslate(short nTypeTextOrient);

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

#ifndef SSP_NO_SHADOWCOLOR
OLE_COLOR CSsocxCtrl::GetShadowColor() 
{
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crBackgrnd;
  SS_GetShadowColor(lpSS, &crBackgrnd, NULL, NULL, NULL);
  UnlockSheet();
  return crBackgrnd;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SHADOWCOLOR
void CSsocxCtrl::SetShadowColor(OLE_COLOR newValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crOldValue;
  SS_GetShadowColor(lpSS, &crOldValue, NULL, NULL, NULL);
  SS_SetShadowColor(lpSS, newValue, RGBCOLOR_DEFAULT,
                    RGBCOLOR_DEFAULT, RGBCOLOR_DEFAULT);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE && crOldValue != newValue )
    BoundPropertyChanged(dispidShadowColor);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SHADOWDARK
OLE_COLOR CSsocxCtrl::GetShadowDark() 
{
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crDark;
  SS_GetShadowColor(lpSS, NULL, NULL, &crDark, NULL);
  UnlockSheet();
  return crDark;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SHADOWDARK
void CSsocxCtrl::SetShadowDark(OLE_COLOR newValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crOldValue;
  SS_GetShadowColor(lpSS, NULL, NULL, &crOldValue, NULL);
  SS_SetShadowColor(lpSS, RGBCOLOR_DEFAULT, RGBCOLOR_DEFAULT,
                    newValue, RGBCOLOR_DEFAULT);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE && crOldValue != newValue)
    BoundPropertyChanged(dispidShadowDark);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SHADOWTEXT
OLE_COLOR CSsocxCtrl::GetShadowText() 
{
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crText;
  SS_GetShadowColor(lpSS, NULL, &crText, NULL, NULL);
  UnlockSheet();
  return crText;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SHADOWTEXT
void CSsocxCtrl::SetShadowText(OLE_COLOR newValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  COLORREF crOldValue;
  SS_GetShadowColor(lpSS, NULL, &crOldValue, NULL, NULL);
  SS_SetShadowColor(lpSS, RGBCOLOR_DEFAULT, newValue,
                    RGBCOLOR_DEFAULT, RGBCOLOR_DEFAULT);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE && crOldValue != newValue)
    BoundPropertyChanged(dispidShadowText);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SORTBY
short CSsocxCtrl::GetSortBy() 
{
  GET_NODESIGN();
  return m_nSortBy;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SORTBY
void CSsocxCtrl::SetSortBy(short nNewValue) 
{
  if( 0 <= nNewValue && nNewValue <= 1 )
    m_nSortBy = nNewValue;
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SORTKEY
long CSsocxCtrl::GetSortKey(short nIndex) 
{
  GET_NODESIGN();

  return (FP_Sort(FALSE, nIndex, 0, FALSE));
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SORTKEY
void CSsocxCtrl::SetSortKey(short nIndex, long nNewValue) 
{
  /* RFW - 5/10/05 - 16218
  if( 0 <= nNewValue && nNewValue <= SS_MAXCOORD )
  */
  LPSPREADSHEET lpSS = LockSheet();
  if( (0 <= nNewValue && nNewValue <= SS_MAXCOORD) ||
      (SS_HEADER <= nNewValue && nNewValue < SS_HEADER + lpSS->Col.HeaderCnt))
    FP_Sort(TRUE, nIndex, nNewValue, FALSE);

  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SORTKEYORDER
short CSsocxCtrl::GetSortKeyOrder(short nIndex) 
{
  GET_NODESIGN();

  return ((short)FP_Sort(FALSE, nIndex, 0, TRUE));
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_SORTKEYORDER
void CSsocxCtrl::SetSortKeyOrder(short nIndex, short nNewValue)
{
  if( 0 <= nNewValue && nNewValue <= 2 )
     FP_Sort(TRUE, nIndex, (long)nNewValue, TRUE);

  SetModifiedFlag();
}
#endif


LONG CSsocxCtrl::FP_Sort(BOOL fSetProp, short nIndex, long lValue, BOOL fOrder)
{
LONG lRet = 0;

if (nIndex <= 0 || nIndex > SS_SORTKEY_MAX)
   ThrowError(CTL_E_INVALIDPROPERTYARRAYINDEX);

else if (!fSetProp)
   {
   if (nIndex > m_nSortKeyCnt)
      {
      if (fOrder)
         lRet = 0;
      else
         lRet = -1;
      }

   else
      {
      LPSS_SORTKEY lpSortKeys = (LPSS_SORTKEY)GlobalLock(m_hSortKeys);

      if (fOrder)
         lRet = (long)lpSortKeys[nIndex - 1].wOrder;
      else
         lRet = (long)lpSortKeys[nIndex - 1].Reference;

      GlobalUnlock(m_hSortKeys);
      }
   }

else
   {
   LPSS_SORTKEY lpSortKeys;
   short     i;

   if (nIndex > m_nSortKeyCnt)
      {
      if (m_nSortKeyCnt == 0)
         m_hSortKeys = GlobalAlloc(GHND, sizeof(SS_SORTKEY) * nIndex);
      else
         m_hSortKeys = GlobalReAlloc(m_hSortKeys,
                                             sizeof(SS_SORTKEY) * nIndex, GHND);

      lpSortKeys = (LPSS_SORTKEY)GlobalLock(m_hSortKeys);

      for (i = m_nSortKeyCnt; i < nIndex; i++)
         {
         lpSortKeys[i].wOrder = 0;
         lpSortKeys[i].Reference = -1;
         }

      GlobalUnlock(m_hSortKeys);
      m_nSortKeyCnt = nIndex;
      }

   lpSortKeys = (LPSS_SORTKEY)GlobalLock(m_hSortKeys);

   if (fOrder)
      lpSortKeys[nIndex - 1].wOrder = (WORD)lValue;
   else
      lpSortKeys[nIndex - 1].Reference = (SS_COORD)lValue;

   if ((lpSortKeys[nIndex - 1].wOrder == 0 &&
       lpSortKeys[nIndex - 1].Reference == -1) &&
       nIndex == m_nSortKeyCnt)
      {
      for ( ; m_nSortKeyCnt > 0 &&
           (lpSortKeys[m_nSortKeyCnt - 1].wOrder == 0 &&
            lpSortKeys[m_nSortKeyCnt - 1].Reference == -1);
           m_nSortKeyCnt--)
         ;

      if (m_nSortKeyCnt == 0)
         {
         GlobalUnlock(m_hSortKeys);
         GlobalFree(m_hSortKeys);
         m_hSortKeys = 0;
         return (0);
         }
      }

   GlobalUnlock(m_hSortKeys);
   }

return (lRet);
}

//--------------------------------------------------------------------

#ifndef SSP_NO_STARTINGCOLNUMBER
long CSsocxCtrl::GetStartingColNumber() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol, lRow;
  SS_GetStartingNumbers(lpSS, &lCol, &lRow);
  UnlockSheet();
  return lCol;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_STARTINGCOLNUMBER
void CSsocxCtrl::SetStartingColNumber(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol, lRow;
  SS_GetStartingNumbers(lpSS, &lCol, &lRow);
  SS_SetStartingNumbers(lpSS, nNewValue, lRow);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidStartingColNumber);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_STARTINGROWNUMBER
long CSsocxCtrl::GetStartingRowNumber() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol, lRow;
  SS_GetStartingNumbers(lpSS, &lCol, &lRow);
  UnlockSheet();
  return lRow;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_STARTINGROWNUMBER
void CSsocxCtrl::SetStartingRowNumber(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol, lRow;
  SS_GetStartingNumbers(lpSS, &lCol, &lRow);
  SS_SetStartingNumbers(lpSS, lCol, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidStartingRowNumber);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TEXT
BSTR CSsocxCtrl::GetText() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  CString s;
  TBGLOBALHANDLE hText;
  LPTSTR lpszText;
  SS_COORD ColEdit, RowEdit;
  SS_CELLTYPE cellType;
  SS_RetrieveCellType(lpSS, &cellType, NULL, m_lCol, m_lRow);

  SS_GetCellBeingEdited(lpSS, &ColEdit, &RowEdit);  // RFW - 4/18/05 - 16009

  if( lpSS->lpBook->EditModeOn && lpSS->nSheetIndex == lpSS->lpBook->nActiveSheet && ColEdit == m_lCol && RowEdit == m_lRow )
    hText = SS_GetEditModeOnData(lpSS->lpBook->hWnd, lpSS, FALSE);
  else
    hText = SS_GetData(lpSS, &cellType, m_lCol, m_lRow, FALSE);
  if( hText )
  {
    lpszText = (LPTSTR)tbGlobalLock(hText);
    s = lpszText;
    tbGlobalUnlock(hText);
    if( lpSS->lpBook->EditModeOn && lpSS->nSheetIndex == lpSS->lpBook->nActiveSheet && lpSS->Col.CurAt == m_lCol && lpSS->Row.CurAt == m_lRow )
      tbGlobalFree(hText);
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TEXT
void CSsocxCtrl::SetText(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  LPSPREADSHEET lpSS = LockSheet();
  if( m_bBlockMode )
    SS_SetDataRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                    lpszNewValue, FALSE, FALSE, TRUE);
  else
    SS_SetDataRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow,
                    lpszNewValue, FALSE, FALSE, TRUE);
  if( 0 == m_lCol || 0 == m_lRow )
    ControlInfoChanged();
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TOPROW
long CSsocxCtrl::GetTopRow() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol, lRow;
  SS_GetTopLeftCell(lpSS, &lCol, &lRow);
  SS_AdjustCellCoordsOut(lpSS, NULL, &lRow);
  UnlockSheet();
  return lRow;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TOPROW
void CSsocxCtrl::SetTopRow(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCol, lRow;
  SS_GetTopLeftCell(lpSS, &lCol, &lRow);
  lpSS->fShowCellCalled = TRUE;
  SS_AdjustCellCoords(lpSS, NULL, &nNewValue);
  SS_ShowCell(lpSS, lCol, nNewValue, SS_SHOW_TOPLEFT);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONALIGN
short CSsocxCtrl::GetTypeButtonAlign() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
  {
    cellType.Style &= SBS_TEXTTOP_PICTBOT | SBS_TEXTBOT_PICTTOP |
                      SBS_TEXTLEFT_PICTRIGHT | SBS_TEXTRIGHT_PICTLEFT;
    switch( cellType.Style )
    {
      case SBS_TEXTBOT_PICTTOP: nValue = 0; break;
      case SBS_TEXTTOP_PICTBOT: nValue = 1; break;
      case SBS_TEXTLEFT_PICTRIGHT: nValue = 2; break;
      case SBS_TEXTRIGHT_PICTLEFT: nValue = 3; break;
    }
  }
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONALIGN
void CSsocxCtrl::SetTypeButtonAlign(short nNewValue) 
{
  m_nTypeButtonAlign = nNewValue;
  SetType(dispidTypeButtonAlign);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONBORDERCOLOR
OLE_COLOR CSsocxCtrl::GetTypeButtonBorderColor() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  OLE_COLOR crValue = RGB(0,0,0);
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    crValue = cellType.Spec.Button.Color.ColorBorder;
  return crValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONBORDERCOLOR
void CSsocxCtrl::SetTypeButtonBorderColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  m_crTypeButtonBorderColor = newValue;
  SetType(dispidTypeButtonBorderColor);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONCOLOR
OLE_COLOR CSsocxCtrl::GetTypeButtonColor() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  OLE_COLOR crValue = RGB(0,0,0);
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    crValue = cellType.Spec.Button.Color.Color;
  return crValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONCOLOR
void CSsocxCtrl::SetTypeButtonColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  m_crTypeButtonColor = newValue;
  SetType(dispidTypeButtonColor);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONDARKCOLOR
OLE_COLOR CSsocxCtrl::GetTypeButtonDarkColor() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  OLE_COLOR crValue = RGB(0,0,0);
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    crValue = cellType.Spec.Button.Color.ColorShadow;
  return crValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONDARKCOLOR
void CSsocxCtrl::SetTypeButtonDarkColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  m_crTypeButtonDarkColor = newValue;
  SetType(dispidTypeButtonDarkColor);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONLIGHTCOLOR
OLE_COLOR CSsocxCtrl::GetTypeButtonLightColor() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  OLE_COLOR crValue = RGB(0,0,0);
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    crValue = cellType.Spec.Button.Color.ColorHighlight;
  return crValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONLIGHTCOLOR
void CSsocxCtrl::SetTypeButtonLightColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  m_crTypeButtonLightColor = newValue;
  SetType(dispidTypeButtonLightColor);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONPICTURE
LPPICTUREDISP CSsocxCtrl::GetTypeButtonPicture() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  LPPICTURE lppict;
  LPPICTUREDISP lpdispValue = NULL;
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    if( lppict = (LPPICTURE)cellType.Spec.Button.lpPict )
      lppict->QueryInterface(IID_IPictureDisp, (LPVOID FAR*)&lpdispValue);
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

#ifndef SSP_NO_TYPEBUTTONPICTURE
void CSsocxCtrl::SetTypeButtonPicture(LPPICTUREDISP newValue) 
{
// fix for SEL5004 -scl
	if (IsPictMetaFile(newValue))
		return;
// fix for SEL5004 -scl
  if( m_lppictTypeButtonPicture )
    m_lppictTypeButtonPicture->Release();
  m_lppictTypeButtonPicture = NULL;
  if( newValue )
    newValue->QueryInterface(IID_IPicture, (LPVOID FAR*)&m_lppictTypeButtonPicture);
  SetType(dispidTypeButtonPicture);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONPICTUREDOWN
LPPICTUREDISP CSsocxCtrl::GetTypeButtonPictureDown() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  LPPICTURE lppict;
  LPPICTUREDISP lpdispValue = NULL;
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    if( lppict = (LPPICTURE)cellType.Spec.Button.lpPictDown )
      lppict->QueryInterface(IID_IPictureDisp, (LPVOID FAR*)&lpdispValue);
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

#ifndef SSP_NO_TYPEBUTTONPICTUREDOWN
void CSsocxCtrl::SetTypeButtonPictureDown(LPPICTUREDISP newValue) 
{
// fix for SEL5004 -scl
	if (IsPictMetaFile(newValue))
		return;
// fix for SEL5004 -scl
  if( m_lppictTypeButtonPictureDown )
    m_lppictTypeButtonPictureDown->Release();
  m_lppictTypeButtonPictureDown = NULL;
  if( newValue )
    newValue->QueryInterface(IID_IPicture, (LPVOID FAR*)&m_lppictTypeButtonPictureDown);
  SetType(dispidTypeButtonPictureDown);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONSHADOWSIZE
short CSsocxCtrl::GetTypeButtonShadowSize() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    nValue = cellType.Spec.Button.ShadowSize;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONSHADOWSIZE
void CSsocxCtrl::SetTypeButtonShadowSize(short nNewValue) 
{
  m_nTypeButtonShadowSize = nNewValue;
  SetType(dispidTypeButtonShadowSize);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONTEXT
BSTR CSsocxCtrl::GetTypeButtonText() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
  {
    if( cellType.Spec.Button.hText )
    {
      sValue = (LPCTSTR)GlobalLock(cellType.Spec.Button.hText);
      GlobalUnlock(cellType.Spec.Button.hText);
    }
  }
  return sValue.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONTEXT
void CSsocxCtrl::SetTypeButtonText(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  m_sTypeButtonText = lpszNewValue;
  SetType(dispidTypeButtonText);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONTEXTCOLOR
OLE_COLOR CSsocxCtrl::GetTypeButtonTextColor() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  OLE_COLOR crValue = RGB(0,0,0);
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    crValue = cellType.Spec.Button.Color.ColorText;
  return crValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONTEXTCOLOR
void CSsocxCtrl::SetTypeButtonTextColor(OLE_COLOR newValue) 
{
//  SET_NODESIGN();
  m_crTypeButtonTextColor = newValue;
  SetType(dispidTypeButtonTextColor);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONTYPE
short CSsocxCtrl::GetTypeButtonType() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_BUTTON) )
    nValue = cellType.Spec.Button.ButtonType;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEBUTTONTYPE
void CSsocxCtrl::SetTypeButtonType(short nNewValue) 
{
  m_nTypeButtonType = nNewValue;
  SetType(dispidTypeButtonType);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKCENTER
BOOL CSsocxCtrl::GetTypeCheckCenter() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_CHECKBOX) )
    bValue = cellType.Style & BS_CENTER ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKCENTER
void CSsocxCtrl::SetTypeCheckCenter(BOOL bNewValue) 
{
  m_bTypeCheckCenter = bNewValue;
  SetType(dispidTypeCheckCenter);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKPICTURE
LPPICTUREDISP CSsocxCtrl::GetTypeCheckPicture(short Index) 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  LPPICTURE lppict = NULL;
  LPPICTUREDISP lpdispValue = NULL;
  if( GetTypeOf(&cellType, SS_TYPE_CHECKBOX) )
  {
    switch( Index )
    {
      case 0: lppict = (LPPICTURE)cellType.Spec.CheckBox.lpPictUp; break;
      case 1: lppict = (LPPICTURE)cellType.Spec.CheckBox.lpPictDown; break;
      case 2: lppict = (LPPICTURE)cellType.Spec.CheckBox.lpPictFocusUp; break;
      case 3: lppict = (LPPICTURE)cellType.Spec.CheckBox.lpPictFocusDown; break;
      case 4: lppict = (LPPICTURE)cellType.Spec.CheckBox.lpPictGray; break;
      case 5: lppict = (LPPICTURE)cellType.Spec.CheckBox.lpPictFocusGray; break;
    }
    if( lppict )
      lppict->QueryInterface(IID_IPictureDisp, (LPVOID FAR*)&lpdispValue);
    else
    {
      CPictureHolder value;
      value.CreateEmpty();
      lpdispValue = value.GetPictureDispatch();
    }
  }
  return lpdispValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKPICTURE
void CSsocxCtrl::SetTypeCheckPicture(short Index, LPPICTUREDISP newValue) 
{
// fix for SEL5004 -scl
	if (IsPictMetaFile(newValue))
		return;
// fix for SEL5004 -scl
  if( 0 <= Index && Index <= 5 )
  {
    m_nTypeCheckPictureIndex = Index;
    if( m_lppictTypeCheckPicture[Index] )
      m_lppictTypeCheckPicture[Index]->Release();
    m_lppictTypeCheckPicture[Index] = NULL;
    if( newValue )
      newValue->QueryInterface(IID_IPicture,
                               (LPVOID FAR*)&m_lppictTypeCheckPicture[Index]);
    SetType(dispidTypeCheckPicture);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKTEXT
BSTR CSsocxCtrl::GetTypeCheckText() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if( GetTypeOf(&cellType, SS_TYPE_CHECKBOX) )
  {
    if( cellType.Spec.CheckBox.hText )
    {
      sValue = (LPCTSTR)GlobalLock(cellType.Spec.CheckBox.hText);
      GlobalUnlock(cellType.Spec.CheckBox.hText);
    }
  }
  return sValue.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKTEXT
void CSsocxCtrl::SetTypeCheckText(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  m_sTypeCheckText = lpszNewValue;
  SetType(dispidTypeCheckText);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKTEXTALIGN
short CSsocxCtrl::GetTypeCheckTextAlign() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_CHECKBOX) )
    nValue = cellType.Style & BS_LEFTTEXT ? 0 : 1;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKTEXTALIGN
void CSsocxCtrl::SetTypeCheckTextAlign(short nNewValue) 
{
  m_nTypeCheckTextAlign = nNewValue;
  SetType(dispidTypeCheckTextAlign);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKTYPE
short CSsocxCtrl::GetTypeCheckType() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_CHECKBOX) )
  {
    if( (cellType.Style & 0x0F) == BS_AUTO3STATE )
      nValue = 1;
    else
      nValue = 0;
  }
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECHECKTYPE
void CSsocxCtrl::SetTypeCheckType(short nNewValue) 
{
  if( 0 <= nNewValue && nNewValue <= 1 )
  {
    m_nTypeCheckType = nNewValue;
    SetType(dispidTypeCheckType);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXCOUNT
short CSsocxCtrl::GetTypeComboBoxCount(void) 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = 0;

  nValue = (short)SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow,
                                        SS_CBM_GETCOUNT, 0, 0);
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXCURSEL
short CSsocxCtrl::GetTypeComboBoxCurSel() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = 0;

  nValue = (short)SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow,
                                         SS_CBM_GETCURSEL, 0, 0);
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXCURSEL
void CSsocxCtrl::SetTypeComboBoxCurSel(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD c, c1, c2;
  SS_COORD r, r1, r2;

  if( m_bBlockMode )
  {
    c1 = m_lCol;
    c2 = m_lCol2;
    r1 = m_lRow;
    r2 = m_lRow2;
  }
  else
  {
    c1 = c2 = m_lCol;
    r1 = r2 = m_lRow;
  }

  if( SS_ALLCOLS == c1 || SS_ALLCOLS == c2 )
    c1 = c2 = SS_ALLCOLS;
  if( SS_ALLROWS == r1 || SS_ALLROWS == r2 )
    r1 = r2 = SS_ALLROWS;
  for( c = c1; c <= c2; c++ )
    for( r = r1; r <= r2; r++ )
      SS_ComboBoxSendMessage(lpSS, c, r, SS_CBM_SETCURSEL, nNewValue, 0);

  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXEDITABLE
BOOL CSsocxCtrl::GetTypeComboBoxEditable() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
    bValue = cellType.Style & SS_CB_DROPDOWN ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXEDITABLE
void CSsocxCtrl::SetTypeComboBoxEditable(BOOL bNewValue) 
{
  m_bTypeComboBoxEditable = bNewValue;
  SetType(dispidTypeComboBoxEditable);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXINDEX
short CSsocxCtrl::GetTypeComboBoxIndex() 
{
  GET_NODESIGN();
  return m_nTypeComboBoxIndex;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXINDEX
void CSsocxCtrl::SetTypeComboBoxIndex(short nNewValue) 
{
  m_nTypeComboBoxIndex = nNewValue;
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXLIST
BSTR CSsocxCtrl::GetTypeComboBoxList() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  LPCTSTR lpItem;
  CString sValue;
  int i;
  if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
  {
    if( cellType.Spec.ComboBox.hItems )
    {
      lpItem = (LPCTSTR)tbGlobalLock(cellType.Spec.ComboBox.hItems);
      for( i = 0; i < cellType.Spec.ComboBox.dItemCnt; i++ )
      {
        sValue += lpItem;
        sValue += _T("\t");
        lpItem += lstrlen(lpItem) + 1;
      }
      tbGlobalUnlock(cellType.Spec.ComboBox.hItems);
    }
  }
  return sValue.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXLIST
void CSsocxCtrl::SetTypeComboBoxList(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
/* RFW - 8/17/99
  m_sTypeComboBoxList = lpszNewValue;
  SetType(dispidTypeComboBoxList);
  SetModifiedFlag();
*/

  SS_COORD c, c1, c2;
  SS_COORD r, r1, r2;
  LPSPREADSHEET lpSS = LockSheet();

  if( m_bBlockMode )
  {
    c1 = m_lCol;
    c2 = m_lCol2;
    r1 = m_lRow;
    r2 = m_lRow2;
  }
  else
  {
    c1 = c2 = m_lCol;
    r1 = r2 = m_lRow;
  }
  if( SS_ALLCOLS == c1 || SS_ALLCOLS == c2 )
    c1 = c2 = SS_ALLCOLS;
  if( SS_ALLROWS == r1 || SS_ALLROWS == r2 )
    r1 = r2 = SS_ALLROWS;
  for( c = c1; c <= c2; c++ )
    for( r = r1; r <= r2; r++ )
      SS_ComboBoxSendMessage(lpSS, c, r, SS_CBM_SETLIST, 0, (long)lpszNewValue);

  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXSTRING
BSTR CSsocxCtrl::GetTypeComboBoxString() 
{
  GET_NODESIGN();
    // TODO: Add your property handler here
  LPSPREADSHEET lpSS = LockSheet();

  CString s;
  LPTSTR lpszBuffer;
  int nLen;

  nLen = (int)SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow, SS_CBM_GETLBTEXTLEN,
                                     m_nTypeComboBoxIndex, 0);
  if( nLen )
  {
    lpszBuffer = s.GetBufferSetLength(nLen+1);
    SS_ComboBoxSendMessage(lpSS, m_lCol, m_lRow, SS_CBM_GETLBTEXT,
                           m_nTypeComboBoxIndex, (long)lpszBuffer);
    s.ReleaseBuffer();
  }

  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPECOMBOBOXSTRING
void CSsocxCtrl::SetTypeComboBoxString(LPCTSTR lpszNewValue) 
{
  SS_COORD c, c1, c2;
  SS_COORD r, r1, r2;
  LPSPREADSHEET lpSS = LockSheet();

  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif

  if( m_bBlockMode )
  {
    c1 = m_lCol;
    c2 = m_lCol2;
    r1 = m_lRow;
    r2 = m_lRow2;
  }
  else
  {
    c1 = c2 = m_lCol;
    r1 = r2 = m_lRow;
  }
  if( SS_ALLCOLS == c1 || SS_ALLCOLS == c2 )
    c1 = c2 = SS_ALLCOLS;
  if( SS_ALLROWS == r1 || SS_ALLROWS == r2 )
    r1 = r2 = SS_ALLROWS;
  for( c = c1; c <= c2; c++ )
  {
    for( r = r1; r <= r2; r++ )
    {
      SS_ComboBoxSendMessage(lpSS, c, r, SS_CBM_INSERTSTRING,
                             m_nTypeComboBoxIndex, (long)lpszNewValue);
    }
  }

  SetModifiedFlag();
  UnlockSheet();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATECENTURY
BOOL CSsocxCtrl::GetTypeDateCentury() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_DATE) )
    bValue = cellType.Spec.Date.Format.bCentury;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATECENTURY
void CSsocxCtrl::SetTypeDateCentury(BOOL bNewValue) 
{
  m_bTypeDateCentury = bNewValue;
  SetType(dispidTypeDateCentury);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATEFORMAT
short CSsocxCtrl::GetTypeDateFormat() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_DATE) )
    nValue = cellType.Spec.Date.Format.nFormat - IDF_DDMONYY;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATEFORMAT
void CSsocxCtrl::SetTypeDateFormat(short nNewValue) 
{
  //Add by BOC 99.4.23 (hyt)--------------------------
  //For ensure invalidate value can't input
#ifdef  SPREAD_JPN
#ifdef SS_V35
  if ( nNewValue <0 || (nNewValue > 8) )
#else
  if (nNewValue <0 || nNewValue > 8)
#endif
#else // SPREAD_JPN
#ifdef SS_V35
  if ( nNewValue <0 || (nNewValue > 5 && 99 != nNewValue) ) // 99 means use the default
#else
  if (nNewValue <0 || nNewValue > 5)
#endif
#endif // SPREAD_JPN
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_nTypeDateFormat = nNewValue;
    SetType(dispidTypeDateFormat);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATEMAX
BSTR CSsocxCtrl::GetTypeDateMax() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  TCHAR szBuffer[9];
  if( GetTypeOf(&cellType, SS_TYPE_DATE) )
  {
    SS_DateToString(szBuffer, &cellType.Spec.Date.Max, cellType.Spec.Date.Format);
    sValue = szBuffer;
  }
  return sValue.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATEMAX
void CSsocxCtrl::SetTypeDateMax(LPCTSTR lpszNewValue) 
{
  SS_CELLTYPE   cellType;
  TB_DATE       newDate;
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  if( GetTypeOf(&cellType, SS_TYPE_DATE) )
  {  
    if( SS_DateValidate(lpszNewValue, &newDate, cellType.Spec.Date.Format) )
    {
	  //Add by BOC 99.4.28 (hyt)--------------------------
	  //For ensure invalidate value can't input
     long lMinDays = DateDMYToJulian (&cellType.Spec.Date.Min);
     long lMaxDays = DateDMYToJulian (&newDate);

	  if (lMinDays > lMaxDays)
		 ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
      //---------------------------------------------------
      else
      {
        _fmemcpy(&m_TypeDateMax, &newDate, sizeof(TB_DATE));
        SetType(dispidTypeDateMax);
      }
    }
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATEMIN
BSTR CSsocxCtrl::GetTypeDateMin() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  TCHAR szBuffer[9];
  if( GetTypeOf(&cellType, SS_TYPE_DATE) )
  {
    SS_DateToString(szBuffer, &cellType.Spec.Date.Min, cellType.Spec.Date.Format);
    sValue = szBuffer;
  }
  return sValue.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATEMIN
void CSsocxCtrl::SetTypeDateMin(LPCTSTR lpszNewValue) 
{
  SS_CELLTYPE   cellType;
  TB_DATE       newDate;
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  if( GetTypeOf(&cellType, SS_TYPE_DATE) )
  {
    if( SS_DateValidate(lpszNewValue, &newDate, cellType.Spec.Date.Format) )
    {
	  //Add by BOC 99.4.28 (hyt)--------------------------
	  //For ensure invalidate value can't input
     long lMinDays = DateDMYToJulian (&newDate);
     long lMaxDays = DateDMYToJulian (&cellType.Spec.Date.Max);

	  if (lMinDays > lMaxDays)
		 ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
      //---------------------------------------------------
      else
      {
        _fmemcpy(&m_TypeDateMin, &newDate, sizeof(TB_DATE));
        SetType(dispidTypeDateMin);
      }
    }
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATESEPARATOR
short CSsocxCtrl::GetTypeDateSeparator() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_DATE) )
    nValue = (_TUCHAR)cellType.Spec.Date.Format.cSeparator;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEDATESEPARATOR
void CSsocxCtrl::SetTypeDateSeparator(short nNewValue) 
{
//#ifdef  SPREAD_JPN
  // Wei Feng for Incident 181
  if(IsCharAlphaNumeric((TCHAR)nNewValue) ||
     (_TIsDBCSLeadByte(HIBYTE(nNewValue)) && LOBYTE(nNewValue) != 0) )
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);
//#endif
  else
  {
/*
    if (nNewValue == 0)
    {
      DATEFORMAT defDateFormat;
      DateDefault(&defDateFormat);
      m_nTypeDateSeparator = defDateFormat.cSeparator;
    }
    else
*/
      m_nTypeDateSeparator = nNewValue;

    SetType(dispidTypeDateSeparator);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITCHARCASE
short CSsocxCtrl::GetTypeEditCharCase() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_EDIT) )
    nValue = cellType.Spec.Edit.ChrCase;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITCHARCASE
void CSsocxCtrl::SetTypeEditCharCase(short nNewValue) 
{
  //Add by BOC 99.4.23 (hyt)--------------------------
  //For ensure invalidate value can't input
  if (nNewValue <0 || nNewValue > 2)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_nTypeEditCharCase = nNewValue;
    SetType(dispidTypeEditCharCase);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITCHARSET
short CSsocxCtrl::GetTypeEditCharSet() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_EDIT) )
    nValue = cellType.Spec.Edit.ChrSet;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITCHARSET
void CSsocxCtrl::SetTypeEditCharSet(short nNewValue) 
{
  //Add by BOC 99.5.12 (hyt)--------------------------
  //For ensure invalidate value can't input
#ifdef SPREAD_JPN	
  if (nNewValue <0 || nNewValue > 6)
#else
  if (nNewValue <0 || nNewValue > 3)
#endif
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_nTypeEditCharSet = nNewValue;
    SetType(dispidTypeEditCharSet);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITLEN
long CSsocxCtrl::GetTypeEditLen() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  long lValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_EDIT) )
    lValue = cellType.Spec.Edit.Len;
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITLEN
void CSsocxCtrl::SetTypeEditLen(long nNewValue) 
{
  if( nNewValue < 0 )
    nNewValue = 0;
  else if( nNewValue > SHRT_MAX )
    nNewValue = SHRT_MAX;
  m_lTypeEditLen = nNewValue;
  SetType(dispidTypeEditLen);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITMULTILINE
BOOL CSsocxCtrl::GetTypeEditMultiLine() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_EDIT) )
    bValue = cellType.Style & ES_MULTILINE ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITMULTILINE
void CSsocxCtrl::SetTypeEditMultiLine(BOOL bNewValue) 
{
  m_bTypeEditMultiLine = bNewValue;
  SetType(dispidTypeEditMultiLine);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITPASSWORD
BOOL CSsocxCtrl::GetTypeEditPassword() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_EDIT) )
    bValue = cellType.Style & ES_PASSWORD ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEEDITPASSWORD
void CSsocxCtrl::SetTypeEditPassword(BOOL bNewValue) 
{
  m_bTypeEditPassword = bNewValue;
  SetType(dispidTypeEditPassword);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATCURRENCYCHAR
short CSsocxCtrl::GetTypeFloatCurrencyChar() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_FLOAT) )
  {
// Modify by BOC 99.6.11 (hyt)---------------------------
//for if not set format return incorrect value
	  if(cellType.Spec.Float.fSetFormat)
		 nValue = (_TUCHAR)cellType.Spec.Float.Format.cCurrencySign;
	  else
	  {
		SS_FLOATFORMAT ff;
		SSGetDefFloatFormat(m_hWnd, (LPSS_FLOATFORMAT)&ff);

		nValue = ff.cCurrencySign;
	  }
//-------------------------------------------------------------------
  }
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATCURRENCYCHAR
void CSsocxCtrl::SetTypeFloatCurrencyChar(short nNewValue) 
{
  m_nTypeFloatCurrencyChar = nNewValue;
  SetType(dispidTypeFloatCurrencyChar);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATDECIMALCHAR
short CSsocxCtrl::GetTypeFloatDecimalChar() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_FLOAT) )
// Modify by BOC 99.6.11 (hyt)---------------------------
//for if not set format return incorrect value
	  if(cellType.Spec.Float.fSetFormat)
		 nValue = (_TUCHAR)cellType.Spec.Float.Format.cDecimalSign;
	  else
	  {
		SS_FLOATFORMAT ff;
		SSGetDefFloatFormat(m_hWnd, (LPSS_FLOATFORMAT)&ff);

		nValue = ff.cDecimalSign;
	  }
//-------------------------------------------------------------------

  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATDECIMALCHAR
void CSsocxCtrl::SetTypeFloatDecimalChar(short nNewValue) 
{
  m_nTypeFloatDecimalChar = nNewValue;
  SetType(dispidTypeFloatDecimalChar);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATDECIMALPLACES
short CSsocxCtrl::GetTypeFloatDecimalPlaces() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_FLOAT) )
    nValue = cellType.Spec.Float.Right;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATDECIMALPLACES
void CSsocxCtrl::SetTypeFloatDecimalPlaces(short nNewValue) 
{
  //Add by BOC 99.4.23 (hyt)--------------------------
  //For ensure invalidate value can't input
  if (nNewValue <0 || nNewValue > 14)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_nTypeFloatDecimalPlaces = nNewValue;
    SetType(dispidTypeFloatDecimalPlaces);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATMAX
double CSsocxCtrl::GetTypeFloatMax() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double dfValue = 0.0;
  if( GetTypeOf(&cellType, SS_TYPE_FLOAT) )
    dfValue = cellType.Spec.Float.Max;
  return dfValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATMAX
void CSsocxCtrl::SetTypeFloatMax(double newValue) 
{
  m_dfTypeFloatMax = newValue;
  SetType(dispidTypeFloatMax);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATMIN
double CSsocxCtrl::GetTypeFloatMin() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double dfValue = 0.0;
  if( GetTypeOf(&cellType, SS_TYPE_FLOAT) )
    dfValue = cellType.Spec.Float.Min;
  return dfValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATMIN
void CSsocxCtrl::SetTypeFloatMin(double newValue) 
{
  m_dfTypeFloatMin = newValue;
  SetType(dispidTypeFloatMin);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATMONEY
BOOL CSsocxCtrl::GetTypeFloatMoney() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_FLOAT) )
    bValue = cellType.Style & FS_MONEY ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATMONEY
void CSsocxCtrl::SetTypeFloatMoney(BOOL bNewValue) 
{
  m_bTypeFloatMoney = bNewValue;
  SetType(dispidTypeFloatMoney);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATSEPARATOR
BOOL CSsocxCtrl::GetTypeFloatSeparator() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_FLOAT) )
    bValue = cellType.Style & FS_SEPARATOR ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATSEPARATOR
void CSsocxCtrl::SetTypeFloatSeparator(BOOL bNewValue) 
{
  m_bTypeFloatSeparator = bNewValue;
  SetType(dispidTypeFloatSeparator);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATSEPCHAR
short CSsocxCtrl::GetTypeFloatSepChar() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_FLOAT) )
// Modify by BOC 99.6.11 (hyt)---------------------------
//for if not set format return incorrect value
	  if(cellType.Spec.Float.fSetFormat)
		     nValue = (_TUCHAR)cellType.Spec.Float.Format.cSeparator;
	  else
	  {
		SS_FLOATFORMAT ff;
		SSGetDefFloatFormat(m_hWnd, (LPSS_FLOATFORMAT)&ff);

		nValue = ff.cSeparator;
	  }
//-------------------------------------------------------------------
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEFLOATSEPCHAR
void CSsocxCtrl::SetTypeFloatSepChar(short nNewValue) 
{
  m_nTypeFloatSepChar = nNewValue;
  SetType(dispidTypeFloatSepChar);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEHALIGN
short CSsocxCtrl::GetTypeHAlign() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetType(&cellType) )
  {
    switch( cellType.Type )
    {
      case SS_TYPE_DATE:
      case SS_TYPE_EDIT:
      case SS_TYPE_FLOAT:
#ifdef SS_V40
      case SS_TYPE_CURRENCY:
      case SS_TYPE_NUMBER:
      case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
      case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      case SS_TYPE_INTEGER:
      case SS_TYPE_PIC:
      case SS_TYPE_TIME:
        cellType.Style &= (ES_LEFT | ES_RIGHT | ES_CENTER);
        if( ES_LEFT == cellType.Style )
          nValue = 0;
        else if( ES_RIGHT == cellType.Style )
          nValue = 1;
        else if( ES_CENTER == cellType.Style )
          nValue = 2;
        break;
      case SS_TYPE_STATICTEXT:
        cellType.Style &= (SS_TEXT_LEFT | SS_TEXT_RIGHT | SS_TEXT_CENTER);
        if( SS_TEXT_LEFT == cellType.Style )
          nValue = 0;
        else if( SS_TEXT_RIGHT == cellType.Style )
          nValue = 1;
        else if( SS_TEXT_CENTER == cellType.Style )
          nValue = 2;
        break;

      case SS_TYPE_PICTURE:
      case SS_TYPE_COMBOBOX:
      case SS_TYPE_CHECKBOX:
         if (cellType.Style & SSS_ALIGN_CENTER)
            nValue = 2;
         else if (cellType.Style & SSS_ALIGN_RIGHT)
            nValue = 1;
         else
            nValue = 0;
        break;

      case SS_TYPE_BUTTON:
        if (cellType.Style & SSS_ALIGN_LEFT)
           nValue = 0;
        else if (cellType.Style & SSS_ALIGN_RIGHT)
           nValue = 1;
        else
           nValue = 2;

        break;
    }    
  }
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEHALIGN
void CSsocxCtrl::SetTypeHAlign(short nNewValue) 
{
  m_nTypeHAlign = nNewValue;
  SetType(dispidTypeHAlign);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEINTEGERMAX
long CSsocxCtrl::GetTypeIntegerMax() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  long lValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_INTEGER) )
    lValue = cellType.Spec.Integer.Max;
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEINTEGERMAX
void CSsocxCtrl::SetTypeIntegerMax(long nNewValue) 
{
  //Add by BOC 99.4.23 (hyt)--------------------------
  //For ensure invalidate value can't input
  if (nNewValue < GetTypeIntegerMin())
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_lTypeIntegerMax = nNewValue;
    SetType(dispidTypeIntegerMax);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEINTEGERMIN
long CSsocxCtrl::GetTypeIntegerMin() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  long lValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_INTEGER) )
    lValue = cellType.Spec.Integer.Min;
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEINTEGERMIN
void CSsocxCtrl::SetTypeIntegerMin(long nNewValue) 
{
  //Add by BOC 99.4.23 (hyt)--------------------------
  //For ensure invalidate value can't input
#ifdef SPREAD_JPN
  if (nNewValue > GetTypeIntegerMax())
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
#endif
  //---------------------------------------------------
  m_lTypeIntegerMin = nNewValue;
  SetType(dispidTypeIntegerMin);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEINTEGERSPININC
long CSsocxCtrl::GetTypeIntegerSpinInc() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  long lValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_INTEGER) )
    lValue = cellType.Spec.Integer.lSpinInc;
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEINTEGERSPININC
void CSsocxCtrl::SetTypeIntegerSpinInc(long nNewValue) 
{
  //Add by BOC 99.5.13 (hyt)--------------------------
  //For ensure invalidate value can't input
  if (nNewValue <0)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_lTypeIntegerSpinInc = nNewValue;
    SetType(dispidTypeIntegerSpinInc);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEINTEGERSPINWRAP
BOOL CSsocxCtrl::GetTypeIntegerSpinWrap() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_INTEGER) )
    bValue = cellType.Spec.Integer.fSpinWrap;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEINTEGERSPINWRAP
void CSsocxCtrl::SetTypeIntegerSpinWrap(BOOL bNewValue) 
{
  m_bTypeIntegerSpinWrap = bNewValue;
  SetType(dispidTypeIntegerSpinWrap);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEOWNERDRAWSTYLE
long CSsocxCtrl::GetTypeOwnerDrawStyle() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  long lValue;
  if( GetTypeOf(&cellType, SS_TYPE_OWNERDRAW) )
    lValue = cellType.Style;
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEOWNERDRAWSTYLE
void CSsocxCtrl::SetTypeOwnerDrawStyle(long nNewValue) 
{
  m_lTypeOwnerDrawStyle = nNewValue;
  SetType(dispidTypeOwnerDrawStyle);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICDEFAULTTEXT
BSTR CSsocxCtrl::GetTypePicDefaultText() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  LPTSTR lpszMask;
  LPTSTR lpszDefText;
  CString sValue;
  if( GetTypeOf(&cellType, SS_TYPE_PIC) )
  {
    if( cellType.Spec.Pic.hMask )
    {
      lpszMask = (LPTSTR)tbGlobalLock(cellType.Spec.Pic.hMask);
      if( lpszDefText = StrpBrk(lpszMask, _T("\t\n")) )
        sValue = lpszDefText + 1;
      tbGlobalUnlock(cellType.Spec.Pic.hMask);
    }
  }
  return sValue.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICDEFAULTTEXT
void CSsocxCtrl::SetTypePicDefaultText(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  m_sTypePicDefaultText = lpszNewValue;
  SetType(dispidTypePicDefaultText);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICMASK
BSTR CSsocxCtrl::GetTypePicMask() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  LPTSTR lpszMask;
  LPTSTR lpszDefText;
  CString sValue;
  if( GetTypeOf(&cellType, SS_TYPE_PIC) )
  {
    if( cellType.Spec.Pic.hMask )
    {
      lpszMask = (LPTSTR)tbGlobalLock(cellType.Spec.Pic.hMask);
      if( lpszDefText = StrpBrk(lpszMask, _T("\t\n")) )
        sValue = CString(lpszMask, (int)(lpszDefText - lpszMask));
      else
        sValue = lpszMask;
      tbGlobalUnlock(cellType.Spec.Pic.hMask);
    }
  }
  return sValue.AllocSysString();
}
#endif
                     
//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICMASK
void CSsocxCtrl::SetTypePicMask(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  m_sTypePicMask = lpszNewValue;
  SetType(dispidTypePicMask);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICTCENTER
BOOL CSsocxCtrl::GetTypePictCenter() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_PICTURE) )
    bValue = cellType.Style & VPS_CENTER ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICTCENTER
void CSsocxCtrl::SetTypePictCenter(BOOL bNewValue) 
{
  m_bTypePictCenter = bNewValue;
  SetType(dispidTypePictCenter);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICTMAINTAINSCALE
BOOL CSsocxCtrl::GetTypePictMaintainScale() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_PICTURE) )
    bValue = cellType.Style & VPS_MAINTAINSCALE ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICTMAINTAINSCALE
void CSsocxCtrl::SetTypePictMaintainScale(BOOL bNewValue) 
{
  m_bTypePictMaintainScale = bNewValue;
  SetType(dispidTypePictMaintainScale);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICTPICTURE
LPPICTUREDISP CSsocxCtrl::GetTypePictPicture() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  LPPICTURE lppict;
  LPPICTUREDISP lpdispValue = NULL;
  if( GetTypeOf(&cellType, SS_TYPE_PICTURE) )
    if( lppict = (LPPICTURE)cellType.Spec.ViewPict.lpPict )
      lppict->QueryInterface(IID_IPictureDisp, (LPVOID FAR*)&lpdispValue);
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

#ifndef SSP_NO_TYPEPICTPICTURE
void CSsocxCtrl::SetTypePictPicture(LPPICTUREDISP newValue) 
{
// fix for SEL5004 -scl
	if (IsPictMetaFile(newValue))
		return;
// fix for SEL5004 -scl
  if( (LPPICTUREDISP)m_lppictTypePictPicture != newValue )
  {
    if( m_lppictTypePictPicture )
    {
       m_lppictTypePictPicture->Release();
       m_lppictTypePictPicture = NULL;
    }
    if( newValue )
       newValue->QueryInterface(IID_IPicture, (LPVOID FAR*)&m_lppictTypePictPicture);
  }
  SetType(dispidTypePictPicture);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICTSTRETCH
BOOL CSsocxCtrl::GetTypePictStretch() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_PICTURE) )
    bValue = cellType.Style & VPS_STRETCH ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPEPICTSTRETCH
void CSsocxCtrl::SetTypePictStretch(BOOL bNewValue) 
{
  m_bTypePictStretch = bNewValue;
  SetType(dispidTypePictStretch);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPESPIN
BOOL CSsocxCtrl::GetTypeSpin() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetType(&cellType) )
  {
    if( SS_TYPE_DATE == cellType.Type )
      bValue = cellType.Spec.Date.Format.bSpin;
    else if( SS_TYPE_INTEGER == cellType.Type )
      bValue = cellType.Style & IS_SPIN ? TRUE : FALSE;
    else if( SS_TYPE_TIME == cellType.Type )
      bValue = cellType.Spec.Time.Format.bSpin;
#ifdef SS_V40
    else if( SS_TYPE_CURRENCY == cellType.Type )
      bValue = cellType.Spec.Currency.fSpin;
    else if( SS_TYPE_NUMBER == cellType.Type )
      bValue = cellType.Spec.Number.fSpin;
    else if( SS_TYPE_PERCENT == cellType.Type )
      bValue = cellType.Spec.Percent.fSpin;
#endif // SS_V40
  }
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPESPIN
void CSsocxCtrl::SetTypeSpin(BOOL bNewValue) 
{
#ifdef SS_V40
  SS_CELLTYPE cellType;
  if( GetType(&cellType) )
  {
    if( SS_TYPE_CURRENCY == cellType.Type )
      m_TypeCurrency.Spec.Currency.fSpin = bNewValue;
    else if( SS_TYPE_NUMBER == cellType.Type )
      m_TypeNumber.Spec.Number.fSpin = bNewValue;
    else if( SS_TYPE_PERCENT == cellType.Type )
      m_TypePercent.Spec.Percent.fSpin = bNewValue;
  }
#endif // SS_V40

  m_bTypeSpin = bNewValue;
  SetType(dispidTypeSpin);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTALIGNVERT
short CSsocxCtrl::GetTypeTextAlignVert() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_STATICTEXT) )
  {
    cellType.Style &= SS_TEXT_BOTTOM | SS_TEXT_VCENTER | SS_TEXT_TOP;
    switch( cellType.Style )
    {
      case SS_TEXT_BOTTOM: nValue = 0; break;
      case SS_TEXT_VCENTER: nValue = 1; break;
      case SS_TEXT_TOP: nValue = 2; break;
    }
  }
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTALIGNVERT
void CSsocxCtrl::SetTypeTextAlignVert(short nNewValue) 
{
  m_nTypeTextAlignVert = nNewValue;
  SetType(dispidTypeTextAlignVert);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTPREFIX
BOOL CSsocxCtrl::GetTypeTextPrefix() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_STATICTEXT) )
    bValue = cellType.Style & SS_TEXT_PREFIX ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTPREFIX
void CSsocxCtrl::SetTypeTextPrefix(BOOL bNewValue) 
{
  m_bTypeTextPrefix = bNewValue;
  SetType(dispidTypeTextPrefix);
  SetModifiedFlag();
  if( 0 == m_lCol || 0 == m_lRow )
    ControlInfoChanged();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTSHADOW
BOOL CSsocxCtrl::GetTypeTextShadow() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_STATICTEXT) )
    bValue = cellType.Style & SS_TEXT_SHADOW ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTSHADOW
void CSsocxCtrl::SetTypeTextShadow(BOOL bNewValue) 
{
  m_bTypeTextShadow = bNewValue;
  SetType(dispidTypeTextShadow);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTSHADOWIN
BOOL CSsocxCtrl::GetTypeTextShadowIn() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_STATICTEXT) )
    bValue = cellType.Style & SS_TEXT_SHADOWIN ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTSHADOWIN
void CSsocxCtrl::SetTypeTextShadowIn(BOOL bNewValue) 
{
  m_bTypeTextShadowIn = bNewValue;
  SetType(dispidTypeTextShadowIn);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTWORDWRAP
BOOL CSsocxCtrl::GetTypeTextWordWrap() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_STATICTEXT) )
    bValue = cellType.Style & SS_TEXT_WORDWRAP ? TRUE : FALSE;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETEXTWORDWRAP
void CSsocxCtrl::SetTypeTextWordWrap(BOOL bNewValue) 
{
  m_bTypeTextWordWrap = bNewValue;
  SetType(dispidTypeTextWordWrap);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIME24HOUR
short CSsocxCtrl::GetTypeTime24Hour() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_TIME) )
    nValue = cellType.Spec.Time.Format.b24Hour;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIME24HOUR
void CSsocxCtrl::SetTypeTime24Hour(short nNewValue) 
{
  m_nTypeTime24Hour = nNewValue;
  SetType(dispidTypeTime24Hour);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIMEMAX
BSTR CSsocxCtrl::GetTypeTimeMax() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  TCHAR szBuffer[7];
  CString sValue;
  if( GetTypeOf(&cellType, SS_TYPE_TIME) )
  {
    SS_TimeToString(szBuffer, &cellType.Spec.Time.Max);
    sValue = szBuffer;
  }
  return sValue.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIMEMAX
void CSsocxCtrl::SetTypeTimeMax(LPCTSTR lpszNewValue) 
{
  SS_CELLTYPE cellType;
  TIME newTime;
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  if( GetTypeOf(&cellType, SS_TYPE_TIME) )
  {
    if( SS_TimeValidate(lpszNewValue, &newTime) )
    {
      TIME TimeMin = cellType.Spec.Time.Min;

      /* RFW - 1/23/05 - 15473
      if (cellType.Spec.Time.SetRange &&
          newTime.nHour*(24*60) + newTime.nMinute*60 + newTime.nSecond <
          TimeMin.nHour*(24*60) + TimeMin.nMinute*60 + TimeMin.nSecond)
      */
      if (cellType.Spec.Time.SetRange &&
          newTime.nHour*(60*60) + newTime.nMinute*60 + newTime.nSecond <
          TimeMin.nHour*(60*60) + TimeMin.nMinute*60 + TimeMin.nSecond)
        ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
      else
      {
        _fmemcpy(&m_TypeTimeMax, &newTime, sizeof(TIME));
        SetType(dispidTypeTimeMax);
      }
    }
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIMEMIN
BSTR CSsocxCtrl::GetTypeTimeMin() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  TCHAR szBuffer[7];
  CString sValue;
  if( GetTypeOf(&cellType, SS_TYPE_TIME) )
  {
    SS_TimeToString(szBuffer, &cellType.Spec.Time.Min);
    sValue = szBuffer;
  }
  return sValue.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIMEMIN
void CSsocxCtrl::SetTypeTimeMin(LPCTSTR lpszNewValue) 
{
  SS_CELLTYPE cellType;
  TIME newTime;
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  if( GetTypeOf(&cellType, SS_TYPE_TIME) )
  {
    if( SS_TimeValidate(lpszNewValue, &newTime) )
    {
      TIME TimeMax = cellType.Spec.Time.Max;

      /* RFW - 1/23/05 - 15473
      if (cellType.Spec.Time.SetRange &&
          newTime.nHour*(24*60) + newTime.nMinute*60 + newTime.nSecond >
          TimeMax.nHour*(24*60) + TimeMax.nMinute*60 + TimeMax.nSecond)
      */
      if (cellType.Spec.Time.SetRange &&
          newTime.nHour*(60*60) + newTime.nMinute*60 + newTime.nSecond >
          TimeMax.nHour*(60*60) + TimeMax.nMinute*60 + TimeMax.nSecond)
        ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
      else
      {
        _fmemcpy(&m_TypeTimeMin, &newTime, sizeof(TIME));
        SetType(dispidTypeTimeMin);
      }
    }
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIMESECONDS
BOOL CSsocxCtrl::GetTypeTimeSeconds() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetTypeOf(&cellType, SS_TYPE_TIME) )
    bValue = cellType.Spec.Time.Format.bSeconds;
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIMESECONDS
void CSsocxCtrl::SetTypeTimeSeconds(BOOL bNewValue) 
{
  m_bTypeTimeSeconds = bNewValue;
  SetType(dispidTypeTimeSeconds);
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIMESEPARATOR
short CSsocxCtrl::GetTypeTimeSeparator() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_TIME) )
    nValue = (_TUCHAR)cellType.Spec.Time.Format.cSeparator;
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_TYPETIMESEPARATOR
void CSsocxCtrl::SetTypeTimeSeparator(short nNewValue) 
{
  //Add by BOC 99.7.1 (hyt)------------------------------
  //process same with date separator
  // Wei Feng for Incident 181
  if(IsCharAlphaNumeric((TCHAR)nNewValue) || nNewValue == 0 ||
     (_TIsDBCSLeadByte(HIBYTE(nNewValue)) && LOBYTE(nNewValue) != 0) )
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);
  //-------------------------------------------------------------
  else
  {
    m_nTypeTimeSeparator = nNewValue;
    SetType(dispidTypeTimeSeparator);
    SetModifiedFlag();
  }
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_UNITTYPE
short CSsocxCtrl::GetUnitType() 
{
  LPSS_BOOK lpBook = LockBook();
  short bValue = SS_GetUnitType(lpBook);
  UnlockBook();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_UNITTYPE
void CSsocxCtrl::SetUnitType(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetUnitType(lpBook, nNewValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidUnitType);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_USERRESIZE
short CSsocxCtrl::GetUserResize() 
{
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = SS_GetUserResize(lpSS);
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_USERRESIZE
void CSsocxCtrl::SetUserResize(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetUserResize(lpSS, nNewValue);
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidUserResize);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_USERRESIZECOL
short CSsocxCtrl::GetUserResizeCol() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = SS_GetUserResizeCol(lpSS, m_lCol);
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_USERRESIZECOL
void CSsocxCtrl::SetUserResizeCol(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetUserResizeCol(lpSS, m_lCol, nNewValue);
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_USERRESIZEROW
short CSsocxCtrl::GetUserResizeRow() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = SS_GetUserResizeRow(lpSS, m_lRow);
  UnlockSheet();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_USERRESIZEROW
void CSsocxCtrl::SetUserResizeRow(short nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetUserResizeRow(lpSS, m_lRow, nNewValue);
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VALUE
BSTR CSsocxCtrl::GetValue() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  CString s;
  TBGLOBALHANDLE hValue;
  LPTSTR lpszValue;
  if( hValue = SS_GetValue(lpSS, m_lCol, m_lRow) )
  {
    if( lpszValue = (LPTSTR)tbGlobalLock(hValue) )
    {
      s = lpszValue;
      tbGlobalUnlock(hValue);
    }
    tbGlobalFree(hValue);
  }
  UnlockSheet();
  return s.AllocSysString();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VALUE
void CSsocxCtrl::SetValue(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  LPSPREADSHEET lpSS = LockSheet();
  if( m_bBlockMode )
    SS_SetDataRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2,
                    lpszNewValue, TRUE, FALSE, TRUE);
  else
    SS_SetDataRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow,
                    lpszNewValue, TRUE, FALSE, TRUE);
  if( 0 == m_lCol || 0 == m_lRow )
    ControlInfoChanged();
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALCURROWCOUNT
long CSsocxCtrl::GetVirtualCurRowCount() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = SS_VGetPhysBufferSize(lpSS);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALCURTOP
long CSsocxCtrl::GetVirtualCurTop() 
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = SS_VGetPhysBufferTop(lpSS);
  SS_AdjustCellCoordsOut(lpSS, NULL, &lValue);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALMAXROWS
long CSsocxCtrl::GetVirtualMaxRows() 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lValue = SS_VGetMax(lpSS);
  UnlockSheet();
  return lValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALMAXROWS
void CSsocxCtrl::SetVirtualMaxRows(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_VSetMax(lpSS, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVirtualMaxRows);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALMODE
BOOL CSsocxCtrl::GetVirtualMode() 
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_VIRTUALMODE);
  UnlockSheet();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALMODE
void CSsocxCtrl::SetVirtualMode(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_VIRTUALMODE, bNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVirtualMode);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALOVERLAP
long CSsocxCtrl::GetVirtualOverlap() 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lBufferSize, lOverlapSize;
  SS_VGetBufferSize(lpSS, &lBufferSize, &lOverlapSize);
  UnlockSheet();
  return lOverlapSize;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALOVERLAP
void CSsocxCtrl::SetVirtualOverlap(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lBufferSize, lOverlapSize;
  SS_VGetBufferSize(lpSS, &lBufferSize, &lOverlapSize);
  SS_VSetBufferSize(lpSS, lBufferSize, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVirtualOverlap);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALROWS
long CSsocxCtrl::GetVirtualRows() 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lBufferSize, lOverlapSize;
  SS_VGetBufferSize(lpSS, &lBufferSize, &lOverlapSize);
  UnlockSheet();
  return lBufferSize;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALROWS
void CSsocxCtrl::SetVirtualRows(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lBufferSize, lOverlapSize;
  SS_VGetBufferSize(lpSS, &lBufferSize, &lOverlapSize);
  SS_VSetBufferSize(lpSS, nNewValue, lOverlapSize);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVirtualRows);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALSCROLLBUFFER
BOOL CSsocxCtrl::GetVirtualScrollBuffer() 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lStyle = SS_VGetStyle(lpSS);
  UnlockSheet();
  return lStyle & SSV_SCROLLBARSHOWBUFFER ? TRUE : FALSE;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VIRTUALSCROLLBUFFER
void CSsocxCtrl::SetVirtualScrollBuffer(BOOL bNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  long lStyle = SS_VGetStyle(lpSS);
  if( bNewValue )
    lStyle |= SSV_SCROLLBARSHOWBUFFER;
  else
    lStyle &= ~SSV_SCROLLBARSHOWBUFFER;
  SS_VSetStyle(lpSS, lStyle);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVirtualScrollBuffer);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VISIBLECOLS
long CSsocxCtrl::GetVisibleCols() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCols, lRows;
  SS_GetAutoSizeVisible(lpSS, &lCols, &lRows);
  UnlockSheet();
  return lCols;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VISIBLECOLS
void CSsocxCtrl::SetVisibleCols(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCols, lRows;
  SS_GetAutoSizeVisible(lpSS, &lCols, &lRows);
  SS_SetAutoSizeVisible(lpSS, nNewValue, lRows);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVisibleCols);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VISIBLEROWS
long CSsocxCtrl::GetVisibleRows() 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCols, lRows;
  SS_GetAutoSizeVisible(lpSS, &lCols, &lRows);
  UnlockSheet();
  return lRows;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VISIBLEROWS
void CSsocxCtrl::SetVisibleRows(long nNewValue) 
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lCols, lRows;
  SS_GetAutoSizeVisible(lpSS, &lCols, &lRows);
  SS_SetAutoSizeVisible(lpSS, lCols, nNewValue);
  INVALIDATE_CONTROL();
  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVisibleRows);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VSCROLLSPECIAL
BOOL CSsocxCtrl::GetVScrollSpecial() 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bValue = SS_VScrollGetSpecial(lpBook, NULL);
  UnlockBook();
  return bValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VSCROLLSPECIAL
void CSsocxCtrl::SetVScrollSpecial(BOOL bNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  WORD wOptions;
  SS_VScrollGetSpecial(lpBook, &wOptions);
  SS_VScrollSetSpecial(lpBook, bNewValue, wOptions);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVScrollSpecial);
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VSCROLLSPECIALTYPE
#ifdef SS_V70
long CSsocxCtrl::GetVScrollSpecialType() 
{
  LPSS_BOOK lpBook = LockBook();
  WORD wOptions;
  SS_VScrollGetSpecial(lpBook, &wOptions);
  UnlockBook();
  return (long)wOptions;
}
#else
short CSsocxCtrl::GetVScrollSpecialType() 
{
  LPSS_BOOK lpBook = LockBook();
  WORD wOptions;
  SS_VScrollGetSpecial(lpBook, &wOptions);
  UnlockBook();
  return wOptions;
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_VSCROLLSPECIALTYPE
#ifdef SS_V70
void CSsocxCtrl::SetVScrollSpecialType(long lNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bUseSpecialVScroll = SS_VScrollGetSpecial(lpBook, NULL);
  SS_VScrollSetSpecial(lpBook, bUseSpecialVScroll, (short)lNewValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVScrollSpecialType);
}
#else
void CSsocxCtrl::SetVScrollSpecialType(short nNewValue) 
{
  LPSS_BOOK lpBook = LockBook();
  BOOL bUseSpecialVScroll = SS_VScrollGetSpecial(lpBook, NULL);
  SS_VScrollSetSpecial(lpBook, bUseSpecialVScroll, nNewValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidVScrollSpecialType);
}
#endif
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BACKCOLORSTYLE
short CSsocxCtrl::GetBackColorStyle()
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetBackColorStyle(lpBook);
  UnlockBook();
  return nValue;
}
#endif

//--------------------------------------------------------------------

#ifndef SSP_NO_BACKCOLORSTYLE
void CSsocxCtrl::SetBackColorStyle(short nNewValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetBackColorStyle(lpBook, nNewValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidBackColorStyle);
}
#endif

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypeButton(LPSPREADSHEET lpSS, DISPID dispid,
                               LPSS_CELLTYPE lpCellTypeOld,
                               LPSS_CELLTYPE lpCellTypeNew)
{
  CString sText;
  OLE_HANDLE hPict;
  short nType;

  if( lpCellTypeOld->Spec.Button.hText )
  {
    sText = (LPCTSTR)GlobalLock(lpCellTypeOld->Spec.Button.hText);
    GlobalUnlock(lpCellTypeOld->Spec.Button.hText);
  }
  switch( dispid )
  {
    #ifndef SSP_NO_TYPEBUTTONALIGN
    case dispidTypeButtonAlign:
      lpCellTypeOld->Style &= ~(SBS_TEXTBOT_PICTTOP | SBS_TEXTTOP_PICTBOT |
                                SBS_TEXTLEFT_PICTRIGHT | SBS_TEXTRIGHT_PICTLEFT);
      if( 0 == m_nTypeButtonAlign )
        lpCellTypeOld->Style |= SBS_TEXTBOT_PICTTOP;
      else if( 1 == m_nTypeButtonAlign )
        lpCellTypeOld->Style |= SBS_TEXTTOP_PICTBOT;
      else if( 2 == m_nTypeButtonAlign )
        lpCellTypeOld->Style |= SBS_TEXTLEFT_PICTRIGHT;
      else if( 3 == m_nTypeButtonAlign )
        lpCellTypeOld->Style |= SBS_TEXTRIGHT_PICTLEFT;
      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONBORDERCOLOR
    case dispidTypeButtonBorderColor:
      lpCellTypeOld->Spec.Button.Color.ColorBorder = m_crTypeButtonBorderColor;
      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONCOLOR
    case dispidTypeButtonColor:
      lpCellTypeOld->Spec.Button.Color.Color = m_crTypeButtonColor;
      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONDARKCOLOR
    case dispidTypeButtonDarkColor:
      lpCellTypeOld->Spec.Button.Color.ColorShadow = m_crTypeButtonDarkColor;
      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONLIGHTCOLOR
    case dispidTypeButtonLightColor:
      lpCellTypeOld->Spec.Button.Color.ColorHighlight = m_crTypeButtonLightColor;
      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONPICTURE
    case dispidTypeButtonPicture:
      if( m_lppictTypeButtonPicture )
      {
        m_lppictTypeButtonPicture->get_Handle(&hPict);
        m_lppictTypeButtonPicture->get_Type(&nType);
        lpCellTypeOld->Spec.Button.lpPict = m_lppictTypeButtonPicture;
        lpCellTypeOld->Spec.Button.hPictName = (GLOBALHANDLE)hPict;
        lpCellTypeOld->Spec.Button.nPictureType = SUPERBTN_PICT_HANDLE;
        if( PICTYPE_BITMAP == nType )
          lpCellTypeOld->Spec.Button.nPictureType |= SUPERBTN_PICT_BITMAP;
        else if( PICTYPE_ICON == nType )
          lpCellTypeOld->Spec.Button.nPictureType |= SUPERBTN_PICT_ICON;
        else if( 0 == nType )
          ;
        else
          return FALSE;
      }
		else
			{
		   lpCellTypeOld->Spec.Button.lpPict = 0;
         lpCellTypeOld->Spec.Button.hPictName = 0;
         lpCellTypeOld->Spec.Button.nPictureType = SUPERBTN_PICT_NONE;
			}

      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONPICTUREDOWN
    case dispidTypeButtonPictureDown:
      if( m_lppictTypeButtonPictureDown )
      {
        m_lppictTypeButtonPictureDown->get_Handle(&hPict);
        m_lppictTypeButtonPictureDown->get_Type(&nType);
        lpCellTypeOld->Spec.Button.lpPictDown = m_lppictTypeButtonPictureDown;
        lpCellTypeOld->Spec.Button.hPictDownName = (GLOBALHANDLE)hPict;
        lpCellTypeOld->Spec.Button.nPictureDownType = SUPERBTN_PICT_HANDLE;
        if( PICTYPE_BITMAP == nType )
          lpCellTypeOld->Spec.Button.nPictureDownType |= SUPERBTN_PICT_BITMAP;
        else if( PICTYPE_ICON == nType )
          lpCellTypeOld->Spec.Button.nPictureDownType |= SUPERBTN_PICT_ICON;
        else if( 0 == nType )
          ;
        else
          return FALSE;
      }
		else
			{
		   lpCellTypeOld->Spec.Button.lpPictDown = 0;
         lpCellTypeOld->Spec.Button.hPictDownName = 0;
         lpCellTypeOld->Spec.Button.nPictureDownType = SUPERBTN_PICT_NONE;
			}

      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONSHADOWSIZE
    case dispidTypeButtonShadowSize:
      lpCellTypeOld->Spec.Button.ShadowSize = (BYTE)m_nTypeButtonShadowSize;
      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONTEXT
    case dispidTypeButtonText:
      sText = m_sTypeButtonText;
      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONTEXTCOLOR
    case dispidTypeButtonTextColor:
      lpCellTypeOld->Spec.Button.Color.ColorText = m_crTypeButtonTextColor;
      break;
    #endif
    #ifndef SSP_NO_TYPEBUTTONTYPE
    case dispidTypeButtonType:
      lpCellTypeOld->Spec.Button.ButtonType = (BYTE)m_nTypeButtonType;
      break;
    #endif

    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);

      if (m_nTypeHAlign == 0)
         lpCellTypeOld->Style |= SSS_ALIGN_LEFT;
      else if (m_nTypeHAlign == 1)
         lpCellTypeOld->Style |= SSS_ALIGN_RIGHT;
      else if (m_nTypeHAlign == 2)
         lpCellTypeOld->Style |= SSS_ALIGN_CENTER;

      break;

#ifdef SS_V30
   case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif

    default:
      return FALSE;
  }
  SS_SetTypeButton(
    lpSS, lpCellTypeNew, lpCellTypeOld->Style, sText,
    (LPCTSTR)&lpCellTypeOld->Spec.Button.hPictName,
    lpCellTypeOld->Spec.Button.nPictureType,
    (LPCTSTR)&lpCellTypeOld->Spec.Button.hPictDownName,
    lpCellTypeOld->Spec.Button.nPictureDownType,
    lpCellTypeOld->Spec.Button.ButtonType,
    lpCellTypeOld->Spec.Button.ShadowSize,
    &lpCellTypeOld->Spec.Button.Color
  );
  lpCellTypeNew->Spec.Button.lpPict = lpCellTypeOld->Spec.Button.lpPict;
  lpCellTypeNew->Spec.Button.lpPictDown = lpCellTypeOld->Spec.Button.lpPictDown;
  if( lpCellTypeNew->Spec.Button.lpPict )
    ((LPPICTURE)lpCellTypeNew->Spec.Button.lpPict)->AddRef();
  if( lpCellTypeNew->Spec.Button.lpPictDown )
    ((LPPICTURE)lpCellTypeNew->Spec.Button.lpPictDown)->AddRef();
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypeCheck(LPSPREADSHEET lpSS, DISPID dispid,
                              LPSS_CELLTYPE lpCellTypeOld,
                              LPSS_CELLTYPE lpCellTypeNew)
{
  CString sText;
  OLE_HANDLE hPict;
  short nType;
  BYTE bPictType;
  LPCTSTR lpszPictUp = 0;
  LPCTSTR lpszPictDown = 0;
  LPCTSTR lpszPictFocUp = 0;
  LPCTSTR lpszPictFocDown = 0;
  LPCTSTR lpszPictDisUp = 0;
  LPCTSTR lpszPictDisDown = 0;
  LPPICTURE lppict = m_lppictTypeCheckPicture[m_nTypeCheckPictureIndex];
  

  if( lpCellTypeOld->Spec.CheckBox.hText )
  {
    sText = (LPCTSTR)GlobalLock(lpCellTypeOld->Spec.CheckBox.hText);
    GlobalUnlock(lpCellTypeOld->Spec.CheckBox.hText);
  }
  switch( dispid )
  {
    #ifndef SSP_NO_TYPECHECKCENTER
    case dispidTypeCheckCenter:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);
      if( m_bTypeCheckCenter )
        lpCellTypeOld->Style |= BS_CENTER;
      else
        lpCellTypeOld->Style &= ~BS_CENTER;
      break;
    #endif
    #ifndef SSP_NO_TYPECHECKPICTURE
    case dispidTypeCheckPicture:
      if( lppict )
      {
        lppict->get_Handle(&hPict);
        lppict->get_Type(&nType);
		}
		else
		{
		  hPict = 0;
		  nType = 0;
		}

     if( PICTYPE_BITMAP == nType )
       bPictType = BT_HANDLE | BT_BITMAP;
     else if( PICTYPE_ICON == nType )
       bPictType = BT_HANDLE | BT_ICON;
     else if( 0 == nType )
       //bPictType = BT_HANDLE;
       bPictType = BT_NONE; // BJO 14Mar96 SEL3426
     else
       return FALSE;
     switch( m_nTypeCheckPictureIndex )
     {
       case 0:
         lpCellTypeOld->Spec.CheckBox.lpPictUp = lppict;
         lpCellTypeOld->Spec.CheckBox.hPictUpName = (TBGLOBALHANDLE)hPict;
         lpCellTypeOld->Spec.CheckBox.bPictUpType = bPictType;
         break;
       case 1:
         lpCellTypeOld->Spec.CheckBox.lpPictDown = lppict;
         lpCellTypeOld->Spec.CheckBox.hPictDownName = (TBGLOBALHANDLE)hPict;
         lpCellTypeOld->Spec.CheckBox.bPictDownType = bPictType;
         break;
       case 2:
         lpCellTypeOld->Spec.CheckBox.lpPictFocusUp = lppict;
         lpCellTypeOld->Spec.CheckBox.hPictFocusUpName = (TBGLOBALHANDLE)hPict;
         lpCellTypeOld->Spec.CheckBox.bPictFocusUpType = bPictType;
         break;
       case 3:
         lpCellTypeOld->Spec.CheckBox.lpPictFocusDown = lppict;
         lpCellTypeOld->Spec.CheckBox.hPictFocusDownName = (TBGLOBALHANDLE)hPict;
         lpCellTypeOld->Spec.CheckBox.bPictFocusDownType = bPictType;
         break;
       case 4:
         lpCellTypeOld->Spec.CheckBox.lpPictGray = lppict;
         lpCellTypeOld->Spec.CheckBox.hPictGrayName = (TBGLOBALHANDLE)hPict;
         lpCellTypeOld->Spec.CheckBox.bPictGrayType = bPictType;
         break;
       case 5:
         lpCellTypeOld->Spec.CheckBox.lpPictFocusGray = lppict;
         lpCellTypeOld->Spec.CheckBox.hPictFocusGrayName = (TBGLOBALHANDLE)hPict;
         lpCellTypeOld->Spec.CheckBox.bPictFocusGrayType = bPictType;
         break;
     }

      break;
    #endif
    #ifndef SSP_NO_TYPECHECKTEXT
    case dispidTypeCheckText:
      sText = m_sTypeCheckText;
      break;
    #endif
    #ifndef SSP_NO_TYPECHECKTEXTALIGN
    case dispidTypeCheckTextAlign:
      if( 0 == m_nTypeCheckTextAlign )
        lpCellTypeOld->Style |= BS_LEFTTEXT;
      else
        lpCellTypeOld->Style &= ~BS_LEFTTEXT;
      break;
    #endif
    #ifndef SSP_NO_TYPECHECKTYPE
    case dispidTypeCheckType:
      lpCellTypeOld->Style &= 0xFFFFFFF0;
      if( 0 == m_nTypeCheckType )
        lpCellTypeOld->Style |= BS_AUTOCHECKBOX;
      else
        lpCellTypeOld->Style |= BS_AUTO3STATE;
      break;
    #endif
   case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);
      lpCellTypeOld->Style &= ~BS_CENTER;

      if (m_nTypeHAlign == 0)
         lpCellTypeOld->Style |= SSS_ALIGN_LEFT;
      else if (m_nTypeHAlign == 1)
         lpCellTypeOld->Style |= SSS_ALIGN_RIGHT;
      else if (m_nTypeHAlign == 2)
         {
         lpCellTypeOld->Style |= SSS_ALIGN_CENTER;
         lpCellTypeOld->Style |= BS_CENTER;
         }

      break;

   case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;

    default:
      return FALSE;
  }
  if( lpCellTypeOld->Spec.CheckBox.hPictUpName )
    lpszPictUp = (LPCTSTR)&lpCellTypeOld->Spec.CheckBox.hPictUpName;
  if( lpCellTypeOld->Spec.CheckBox.hPictDownName )
    lpszPictDown = (LPCTSTR)&lpCellTypeOld->Spec.CheckBox.hPictDownName;
  if( lpCellTypeOld->Spec.CheckBox.hPictFocusUpName )
    lpszPictFocUp = (LPCTSTR)&lpCellTypeOld->Spec.CheckBox.hPictFocusUpName;
  if( lpCellTypeOld->Spec.CheckBox.hPictFocusDownName )
    lpszPictFocDown = (LPCTSTR)&lpCellTypeOld->Spec.CheckBox.hPictFocusDownName;
  if( lpCellTypeOld->Spec.CheckBox.hPictGrayName )
    lpszPictDisUp = (LPCTSTR)&lpCellTypeOld->Spec.CheckBox.hPictGrayName;
  if( lpCellTypeOld->Spec.CheckBox.hPictFocusGrayName )
    lpszPictDisDown = (LPCTSTR)&lpCellTypeOld->Spec.CheckBox.hPictFocusGrayName;
  SS_SetTypeCheckBox(
    lpSS, lpCellTypeNew, lpCellTypeOld->Style, sText,
    lpszPictUp,lpCellTypeOld->Spec.CheckBox.bPictUpType,
    lpszPictDown,lpCellTypeOld->Spec.CheckBox.bPictDownType,
    lpszPictFocUp,lpCellTypeOld->Spec.CheckBox.bPictFocusUpType,
    lpszPictFocDown,lpCellTypeOld->Spec.CheckBox.bPictFocusDownType,
    lpszPictDisUp,lpCellTypeOld->Spec.CheckBox.bPictGrayType,
    lpszPictDisDown, lpCellTypeOld->Spec.CheckBox.bPictFocusGrayType
  );
  lpCellTypeNew->Spec.CheckBox.lpPictUp = lpCellTypeOld->Spec.CheckBox.lpPictUp;
  lpCellTypeNew->Spec.CheckBox.lpPictDown = lpCellTypeOld->Spec.CheckBox.lpPictDown;
  lpCellTypeNew->Spec.CheckBox.lpPictFocusUp = lpCellTypeOld->Spec.CheckBox.lpPictFocusUp;
  lpCellTypeNew->Spec.CheckBox.lpPictFocusDown = lpCellTypeOld->Spec.CheckBox.lpPictFocusDown;
  lpCellTypeNew->Spec.CheckBox.lpPictGray = lpCellTypeOld->Spec.CheckBox.lpPictGray;
  lpCellTypeNew->Spec.CheckBox.lpPictFocusGray = lpCellTypeOld->Spec.CheckBox.lpPictFocusGray;
  if( lpCellTypeNew->Spec.CheckBox.lpPictUp )
    ((LPPICTURE)lpCellTypeNew->Spec.CheckBox.lpPictUp)->AddRef();
  if( lpCellTypeNew->Spec.CheckBox.lpPictDown )
    ((LPPICTURE)lpCellTypeNew->Spec.CheckBox.lpPictDown)->AddRef();
  if( lpCellTypeNew->Spec.CheckBox.lpPictFocusUp )
    ((LPPICTURE)lpCellTypeNew->Spec.CheckBox.lpPictFocusUp)->AddRef();
  if( lpCellTypeNew->Spec.CheckBox.lpPictFocusDown )
    ((LPPICTURE)lpCellTypeNew->Spec.CheckBox.lpPictFocusDown)->AddRef();
  if( lpCellTypeNew->Spec.CheckBox.lpPictGray )
    ((LPPICTURE)lpCellTypeNew->Spec.CheckBox.lpPictGray)->AddRef();
  if( lpCellTypeNew->Spec.CheckBox.lpPictFocusGray )
    ((LPPICTURE)lpCellTypeNew->Spec.CheckBox.lpPictFocusGray)->AddRef();
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypeCombo(LPSPREADSHEET lpSS, DISPID dispid,
                              LPSS_CELLTYPE lpCellTypeOld,
                              LPSS_CELLTYPE lpCellTypeNew)
{
  CString sList;
  LPCTSTR lpItem;
  BOOL bUseList = FALSE;
  int i;

  if( lpCellTypeOld->Spec.ComboBox.hItems )
  {
    lpItem = (LPCTSTR)tbGlobalLock(lpCellTypeOld->Spec.ComboBox.hItems);
    for( i = 0; i < lpCellTypeOld->Spec.ComboBox.dItemCnt; i++ )
    {
      bUseList = TRUE;
      sList += lpItem;
      if( i < lpCellTypeOld->Spec.ComboBox.dItemCnt - 1 )
        sList += _T("\t");
      lpItem += lstrlen(lpItem) + 1;
    }
    tbGlobalUnlock(lpCellTypeOld->Spec.ComboBox.hItems);
  }
  switch( dispid )
  {
    #ifndef SSP_NO_TYPECOMBOBOXEDITABLE
    case dispidTypeComboBoxEditable:
      if( m_bTypeComboBoxEditable )
        lpCellTypeOld->Style |= SS_CB_DROPDOWN;
      else
        lpCellTypeOld->Style &= ~SS_CB_DROPDOWN;
      break;
    #endif
    #ifndef SSP_NO_TYPECOMBOBOXLIST
    case dispidTypeComboBoxList:
      bUseList = TRUE;
      sList = m_sTypeComboBoxList;
      break;
    #endif

#ifdef SS_V30
    case dispidTypeComboBoxMaxDrop:
      lpCellTypeOld->Spec.ComboBox.dMaxRows = (short)m_nTypeComboMaxDrop;
      break;

   case dispidTypeComboBoxWidth:
      lpCellTypeOld->Spec.ComboBox.dComboWidth = m_nTypeComboWidth;
      break;

   case dispidTypeMaxEditLen:
      lpCellTypeOld->Spec.ComboBox.dMaxEditLen = (short)m_nTypeComboMaxEditLen;
      break;

   case dispidTypeComboBoxhWnd:
      lpCellTypeOld->Spec.ComboBox.hWndDropDown = (HWND)m_nTypeCombohWnd;
      break;
#endif

#ifdef SS_V40
   case dispidTypeComboBoxAutoSearch:
      lpCellTypeOld->Spec.ComboBox.AutoSearch = (BYTE)m_bTypeComboAutoSearch;
      break;
#endif // SS_V40

   case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);

      if (m_nTypeHAlign == 0)
         lpCellTypeOld->Style |= SSS_ALIGN_LEFT;
      else if (m_nTypeHAlign == 1)
         lpCellTypeOld->Style |= SSS_ALIGN_RIGHT;
      else if (m_nTypeHAlign == 2)
         lpCellTypeOld->Style |= SSS_ALIGN_CENTER;
      break;

#ifdef SS_V30
   case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif
  }

#ifdef SS_V40
  SS_SetTypeComboBox(lpSS, lpCellTypeNew, lpCellTypeOld->Style,
                     bUseList ? (LPCTSTR)sList : NULL,
                     lpCellTypeOld->Spec.ComboBox.dMaxRows,
                     lpCellTypeOld->Spec.ComboBox.dMaxEditLen,
                     lpCellTypeOld->Spec.ComboBox.dComboWidth,
                     lpCellTypeOld->Spec.ComboBox.hWndDropDown,
                     lpCellTypeOld->Spec.ComboBox.AutoSearch);
#elif defined(SS_V30)
  SS_SetTypeComboBox(lpSS, lpCellTypeNew, lpCellTypeOld->Style,
                     bUseList ? (LPCTSTR)sList : NULL,
                     lpCellTypeOld->Spec.ComboBox.dMaxRows,
                     lpCellTypeOld->Spec.ComboBox.dMaxEditLen,
                     lpCellTypeOld->Spec.ComboBox.dComboWidth,
                     lpCellTypeOld->Spec.ComboBox.hWndDropDown,
                     SS_COMBOBOX_AUTOSEARCH_SINGLECHAR);
#else
  SS_SetTypeComboBox(lpSS, lpCellTypeNew, lpCellTypeOld->Style,
                     bUseList ? (LPCTSTR)sList : NULL, -1, -1, 0, 0, SS_COMBOBOX_AUTOSEARCH_SINGLECHAR);
#endif
  lpCellTypeNew->ControlID = lpCellTypeOld->ControlID;

  return TRUE;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypeDate(LPSPREADSHEET lpSS, DISPID dispid,
                             LPSS_CELLTYPE lpCellTypeOld,
                             LPSS_CELLTYPE lpCellTypeNew)
{
  LPTB_DATE lpDateMax = NULL;
  LPTB_DATE lpDateMin = NULL;

  if( !lpCellTypeOld->Spec.Date.SetFormat )
    DateGetFormat(0, &lpCellTypeOld->Spec.Date.Format);
  if( lpCellTypeOld->Spec.Date.SetRange )
  {
    lpDateMax = &lpCellTypeOld->Spec.Date.Max;
    lpDateMin = &lpCellTypeOld->Spec.Date.Min;
  }
// 9026
  lpCellTypeOld->Style &= (ES_LEFT | ES_CENTER | ES_RIGHT |
                           SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM |
#ifdef SS_V40
						   SSS_TEXTORIENT_MASK |
#endif
#ifdef SS_V35
                           SSS_ALIGN_VCENTER | SSS_ELLIPSES);
#else
                           SSS_ALIGN_VCENTER);
#endif

  switch( dispid )
  {
#ifdef SS_V30
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif

#ifdef SS_V35
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
#endif

    #ifndef SSP_NO_TYPEDATECENTURY
    case dispidTypeDateCentury:
      lpCellTypeOld->Spec.Date.Format.bCentury = m_bTypeDateCentury;
      break;
    #endif
    #ifndef SSP_NO_TYPEDATEFORMAT
    case dispidTypeDateFormat:
#ifdef SS_V35
      if (99 == m_nTypeDateFormat) // 99 is the value in SSOCX.ODL
      {
        SS_DATEFORMAT df;
        DateDefault(&df);
        m_nTypeDateFormat = df.nFormat - IDF_DDMONYY;
      }
#endif
      lpCellTypeOld->Spec.Date.Format.nFormat = IDF_DDMONYY + m_nTypeDateFormat;
      break;
    #endif
    #ifndef SSP_NO_TYPEDATEMAX
    case dispidTypeDateMax:
      lpDateMax = &m_TypeDateMax;
      break;
    #endif
    #ifndef SSP_NO_TYPEDATEMIN
    case dispidTypeDateMin:
      lpDateMin = &m_TypeDateMin;
      break;
    #endif
    #ifndef SSP_NO_TYPEDATESEPARATOR
    case dispidTypeDateSeparator:
      if (m_nTypeDateSeparator == 0)
      {
        SS_DATEFORMAT df;
        DateDefault(&df);
        lpCellTypeOld->Spec.Date.Format.cSeparator = df.cSeparator;
      }
      else
        lpCellTypeOld->Spec.Date.Format.cSeparator = (TCHAR)m_nTypeDateSeparator;
      break;
    #endif
    #ifndef SSP_NO_TYPEHALIGN
    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFT | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFT;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    #endif
    #ifndef SSP_NO_TYPESPIN
    case dispidTypeSpin:
      lpCellTypeOld->Spec.Date.Format.bSpin = m_bTypeSpin;
      break;
    #endif
#ifdef SS_V40
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
#endif // SS_V40
    default:
      return FALSE;
  }
  SS_SetTypeDate(lpSS, lpCellTypeNew, lpCellTypeOld->Style,
                 &lpCellTypeOld->Spec.Date.Format, lpDateMin, lpDateMax);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypeEdit(LPSPREADSHEET lpSS, DISPID dispid,
                             LPSS_CELLTYPE lpCellTypeOld,
                             LPSS_CELLTYPE lpCellTypeNew)
{
  switch( dispid )
  {
    #ifndef SSP_NO_TYPEEDITCHARCASE
    case dispidTypeEditCharCase:
      lpCellTypeOld->Spec.Edit.ChrCase = m_nTypeEditCharCase;
      break;
    #endif
    #ifndef SSP_NO_TYPEEDITCHARSET
    case dispidTypeEditCharSet:
      lpCellTypeOld->Spec.Edit.ChrSet = m_nTypeEditCharSet;
      break;
    #endif
    #ifndef SSP_NO_TYPEEDITLEN
    case dispidTypeEditLen:
#ifdef SS_V30
    case dispidTypeMaxEditLen:
#endif
      lpCellTypeOld->Spec.Edit.Len = (short)m_lTypeEditLen;
      break;
    #endif
    #ifndef SSP_NO_TYPEEDITMULTILINE
    case dispidTypeEditMultiLine:
      lpCellTypeOld->Style &= ~(ES_AUTOHSCROLL | ES_AUTOVSCROLL |
                                ES_MULTILINE | ES_WANTRETURN);
      if( m_bTypeEditMultiLine )
        lpCellTypeOld->Style |= ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN;
      else
        lpCellTypeOld->Style |= ES_AUTOHSCROLL;
      break;
    #endif
    #ifndef SSP_NO_TYPEEDITPASSWORD
    case dispidTypeEditPassword:
      if( m_bTypeEditPassword )
        lpCellTypeOld->Style |= ES_PASSWORD;
      else
        lpCellTypeOld->Style &= ~ES_PASSWORD;
      break;
    #endif
    #ifndef SSP_NO_TYPEHALIGN
    case dispidTypeHAlign:
		// RFW - 1/4/05 - 15434
      lpCellTypeOld->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);
		/*
		// RFW - 6/29/04 - 14702
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER |
                                SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);
		*/
      lpCellTypeOld->Style &= ~(ES_LEFT | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFT;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    #endif

#ifdef SS_V30
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif

#ifdef SS_V35
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
#endif
#ifdef SS_V40
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
#endif // SS_V40

    default:
      return FALSE;
  }
  SS_SetTypeEdit(lpCellTypeNew, lpCellTypeOld->Style,
                 lpCellTypeOld->Spec.Edit.Len,
                 lpCellTypeOld->Spec.Edit.ChrSet,
                 lpCellTypeOld->Spec.Edit.ChrCase);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypeFloat(LPSPREADSHEET lpSS, DISPID dispid,
                              LPSS_CELLTYPE lpCellTypeOld,
                              LPSS_CELLTYPE lpCellTypeNew)
{
  TCHAR szBuffer[330];
  short nLeft;

  switch( dispid )
  {
    #ifndef SSP_NO_TYPEFLOATCURRENCYCHAR
    case dispidTypeFloatCurrencyChar:
      lpCellTypeOld->Spec.Float.Format.cCurrencySign = (TCHAR)m_nTypeFloatCurrencyChar;
      break;
    #endif
    #ifndef SSP_NO_TYPEFLOATDECIMALCHAR
    case dispidTypeFloatDecimalChar:
      lpCellTypeOld->Spec.Float.Format.cDecimalSign = (TCHAR)m_nTypeFloatDecimalChar;
      break;
    #endif
    #ifndef SSP_NO_TYPEFLOATDECIMALPLACES
    case dispidTypeFloatDecimalPlaces:
      lpCellTypeOld->Spec.Float.Right = m_nTypeFloatDecimalPlaces;
      break;
    #endif
    #ifndef SSP_NO_TYPEFLOATMAX
    case dispidTypeFloatMax:
      lpCellTypeOld->Spec.Float.Max = m_dfTypeFloatMax;
      break;
    #endif
    #ifndef SSP_NO_TYPEFLOATMIN
    case dispidTypeFloatMin:
      lpCellTypeOld->Spec.Float.Min = m_dfTypeFloatMin;
      break;
    #endif
    #ifndef SSP_NO_TYPEFLOATMONEY
    case dispidTypeFloatMoney:
      if( m_bTypeFloatMoney )
        lpCellTypeOld->Style |= FS_MONEY;
      else
        lpCellTypeOld->Style &= ~FS_MONEY;
      break;
    #endif
    #ifndef SSP_NO_TYPEFLOATSEPARATOR
    case dispidTypeFloatSeparator:
      if( m_bTypeFloatSeparator )
        lpCellTypeOld->Style |= FS_SEPARATOR;
      else
        lpCellTypeOld->Style &= ~FS_SEPARATOR;
      break;
    #endif
    #ifndef SSP_NO_TYPEFLOATSEPCHAR
    case dispidTypeFloatSepChar:
      lpCellTypeOld->Spec.Float.Format.cSeparator = (TCHAR)m_nTypeFloatSepChar;
      break;
    #endif
    #ifndef SSP_NO_TYPEHALIGN
    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFTALIGN | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFTALIGN;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    #endif
#ifdef SS_V30
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif

#ifdef SS_V35
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
#endif
#ifdef SS_V40
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
    case dispidTypeNegRed:
      lpCellTypeOld->Style &= ~(SSS_NEGRED);
      if (m_fTypeNegRed)
         lpCellTypeOld->Style |= SSS_NEGRED;
      break;
#endif // SS_V40

    default:
      return FALSE;
  }
  StrPrintf(szBuffer, _T("%.*f"), 2, max(fabs(lpCellTypeOld->Spec.Float.Min),
                                     fabs(lpCellTypeOld->Spec.Float.Max)));
  nLeft = (short)(StrChr(szBuffer, '.') - (LPTSTR)szBuffer);
  SS_SetTypeFloatExt(lpSS, lpCellTypeNew, lpCellTypeOld->Style,
                     nLeft, lpCellTypeOld->Spec.Float.Right,
                     lpCellTypeOld->Spec.Float.Min,
                     lpCellTypeOld->Spec.Float.Max,
                     &lpCellTypeOld->Spec.Float.Format); 
  return TRUE;
}

//--------------------------------------------------------------------

#ifdef SS_V40

BOOL CSsocxCtrl::SetTypeCurrency(LPSPREADSHEET lpSS, DISPID dispid,
                                 LPSS_CELLTYPE lpCellTypeOld,
                                 LPSS_CELLTYPE lpCellTypeNew)
{
  switch( dispid )
  {
    case dispidTypeSpin:
      lpCellTypeOld->Spec.Currency.fSpin = m_TypeCurrency.Spec.Currency.fSpin;
      break;
    case dispidTypeSpinWrap:
      lpCellTypeOld->Spec.Currency.fSpinWrap = m_TypeCurrency.Spec.Currency.fSpinWrap;
      break;
    case dispidTypeSpinInc:
      lpCellTypeOld->Spec.Currency.SpinInc = m_TypeCurrency.Spec.Currency.SpinInc;
      break;
    case dispidTypeCurrencyMin:
      lpCellTypeOld->Spec.Currency.Min = m_TypeCurrency.Spec.Currency.Min;
      break;
    case dispidTypeCurrencyMax:
      lpCellTypeOld->Spec.Currency.Max = m_TypeCurrency.Spec.Currency.Max;
      break;
    case dispidTypeCurrencyDecimal:
      lstrcpy(lpCellTypeOld->Spec.Currency.szDecimal, m_TypeCurrency.Spec.Currency.szDecimal);
      break;
    case dispidTypeCurrencySeparator:
      lstrcpy(lpCellTypeOld->Spec.Currency.szSeparator, m_TypeCurrency.Spec.Currency.szSeparator);
      break;
    case dispidTypeCurrencySymbol:
      lstrcpy(lpCellTypeOld->Spec.Currency.szCurrency, m_TypeCurrency.Spec.Currency.szCurrency);
      break;
    case dispidTypeCurrencyLeadingZero:
      lpCellTypeOld->Spec.Currency.bLeadingZero = m_TypeCurrency.Spec.Currency.bLeadingZero;
      break;
    case dispidTypeCurrencyDecPlaces:
      lpCellTypeOld->Spec.Currency.Right = m_TypeCurrency.Spec.Currency.Right;
      break;
    case dispidTypeCurrencyNegStyle:
      lpCellTypeOld->Spec.Currency.bNegStyle = m_TypeCurrency.Spec.Currency.bNegStyle;
      break;
    case dispidTypeCurrencyPosStyle:
      lpCellTypeOld->Spec.Currency.bPosStyle = m_TypeCurrency.Spec.Currency.bPosStyle;
      break;
    case dispidTypeCurrencyShowSep:
      lpCellTypeOld->Spec.Currency.fShowSeparator = m_TypeCurrency.Spec.Currency.fShowSeparator;
      break;
    case dispidTypeCurrencyShowSymbol:
      lpCellTypeOld->Spec.Currency.fShowCurrencySymbol = m_TypeCurrency.Spec.Currency.fShowCurrencySymbol;
      break;
    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFTALIGN | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFTALIGN;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
    case dispidTypeNegRed:
      lpCellTypeOld->Style &= ~(SSS_NEGRED);
      if (m_fTypeNegRed)
         lpCellTypeOld->Style |= SSS_NEGRED;
      break;

    default:
      return FALSE;
  }

  *lpCellTypeNew = *lpCellTypeOld;
  return TRUE;
}

BOOL CSsocxCtrl::SetTypeNumber(LPSPREADSHEET lpSS, DISPID dispid,
                               LPSS_CELLTYPE lpCellTypeOld,
                               LPSS_CELLTYPE lpCellTypeNew)
{
  switch( dispid )
  {
    case dispidTypeSpin:
      lpCellTypeOld->Spec.Number.fSpin = m_TypeNumber.Spec.Number.fSpin;
      break;
    case dispidTypeSpinWrap:
      lpCellTypeOld->Spec.Number.fSpinWrap = m_TypeNumber.Spec.Number.fSpinWrap;
      break;
    case dispidTypeSpinInc:
      lpCellTypeOld->Spec.Number.SpinInc = m_TypeNumber.Spec.Number.SpinInc;
      break;
    case dispidTypeNumberMin:
      lpCellTypeOld->Spec.Number.Min = m_TypeNumber.Spec.Number.Min;
      break;
    case dispidTypeNumberMax:
      lpCellTypeOld->Spec.Number.Max = m_TypeNumber.Spec.Number.Max;
      break;
    case dispidTypeNumberDecimal:
      lstrcpy(lpCellTypeOld->Spec.Number.szDecimal, m_TypeNumber.Spec.Number.szDecimal);
      break;
    case dispidTypeNumberSeparator:
      lstrcpy(lpCellTypeOld->Spec.Number.szSeparator, m_TypeNumber.Spec.Number.szSeparator);
      break;
    case dispidTypeNumberLeadingZero:
      lpCellTypeOld->Spec.Number.bLeadingZero = m_TypeNumber.Spec.Number.bLeadingZero;
      break;
    case dispidTypeNumberDecPlaces:
      lpCellTypeOld->Spec.Number.Right = m_TypeNumber.Spec.Number.Right;
      break;
    case dispidTypeNumberNegStyle:
      lpCellTypeOld->Spec.Number.bNegStyle = m_TypeNumber.Spec.Number.bNegStyle;
      break;
    case dispidTypeNumberShowSep:
      lpCellTypeOld->Spec.Number.fShowSeparator = m_TypeNumber.Spec.Number.fShowSeparator;
      break;
     case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFTALIGN | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFTALIGN;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
    case dispidTypeNegRed:
      lpCellTypeOld->Style &= ~(SSS_NEGRED);
      if (m_fTypeNegRed)
         lpCellTypeOld->Style |= SSS_NEGRED;
      break;

    default:
      return FALSE;
  }

  *lpCellTypeNew = *lpCellTypeOld;
  return TRUE;
}

BOOL CSsocxCtrl::SetTypePercent(LPSPREADSHEET lpSS, DISPID dispid,
                               LPSS_CELLTYPE lpCellTypeOld,
                               LPSS_CELLTYPE lpCellTypeNew)
{
  switch( dispid )
  {
    case dispidTypeSpin:
      lpCellTypeOld->Spec.Percent.fSpin = m_TypePercent.Spec.Percent.fSpin;
      break;
    case dispidTypeSpinWrap:
      lpCellTypeOld->Spec.Percent.fSpinWrap = m_TypePercent.Spec.Percent.fSpinWrap;
      break;
    case dispidTypeSpinInc:
      lpCellTypeOld->Spec.Percent.SpinInc = m_TypePercent.Spec.Percent.SpinInc;
      break;
    case dispidTypePercentMin:
      lpCellTypeOld->Spec.Percent.Min = m_TypePercent.Spec.Percent.Min;
      break;
    case dispidTypePercentMax:
      lpCellTypeOld->Spec.Percent.Max = m_TypePercent.Spec.Percent.Max;
      break;
    case dispidTypePercentDecimal:
      lstrcpy(lpCellTypeOld->Spec.Percent.szDecimal, m_TypePercent.Spec.Percent.szDecimal);
      break;
    case dispidTypePercentLeadingZero:
      lpCellTypeOld->Spec.Percent.bLeadingZero = m_TypePercent.Spec.Percent.bLeadingZero;
      break;
    case dispidTypePercentDecPlaces:
      lpCellTypeOld->Spec.Percent.Right = m_TypePercent.Spec.Percent.Right;
      break;
    case dispidTypePercentNegStyle:
      lpCellTypeOld->Spec.Percent.bNegStyle = m_TypePercent.Spec.Percent.bNegStyle;
      break;
     case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFTALIGN | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFTALIGN;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
    case dispidTypeNegRed:
      lpCellTypeOld->Style &= ~(SSS_NEGRED);
      if (m_fTypeNegRed)
         lpCellTypeOld->Style |= SSS_NEGRED;
      break;

    default:
      return FALSE;
  }

  *lpCellTypeNew = *lpCellTypeOld;
  return TRUE;
}

#endif // SS_V40

#ifdef SS_V70
BOOL CSsocxCtrl::SetTypeScientific(LPSPREADSHEET lpSS, DISPID dispid,
                                   LPSS_CELLTYPE lpCellTypeOld,
                                   LPSS_CELLTYPE lpCellTypeNew)
{
  switch( dispid )
  {
    case dispidTypeScientificMin:
      lpCellTypeOld->Spec.Scientific.Min = m_TypeScientific.Spec.Scientific.Min;
      break;
    case dispidTypeScientificMax:
      lpCellTypeOld->Spec.Scientific.Max = m_TypeScientific.Spec.Scientific.Max;
      break;
    case dispidTypeScientificDecimal:
      lstrcpy(lpCellTypeOld->Spec.Scientific.szDecimal, m_TypeScientific.Spec.Scientific.szDecimal);
      break;
    case dispidTypeScientificDecPlaces:
      lpCellTypeOld->Spec.Scientific.Right = m_TypeScientific.Spec.Scientific.Right;
      break;
     case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFTALIGN | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFTALIGN;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
    case dispidTypeNegRed:
      lpCellTypeOld->Style &= ~(SSS_NEGRED);
      if (m_fTypeNegRed)
         lpCellTypeOld->Style |= SSS_NEGRED;
      break;

    default:
      return FALSE;
  }

  *lpCellTypeNew = *lpCellTypeOld;
  return TRUE;
}
#endif // SS_V70

//--------------------------------------------------------------------

#ifndef SS_NOCT_INT
BOOL CSsocxCtrl::SetTypeInteger(LPSPREADSHEET lpSS, DISPID dispid,
                                LPSS_CELLTYPE lpCellTypeOld,
                                LPSS_CELLTYPE lpCellTypeNew)
{
  switch( dispid )
  {
    #ifndef SSP_NO_TYPEHALIGN
    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFTALIGN | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFTALIGN;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    #endif
#ifdef SS_V30
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif

#ifdef SS_V35
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
#endif

    #ifndef SSP_NO_TYPEINTEGERMAX
    case dispidTypeIntegerMax:
      lpCellTypeOld->Spec.Integer.Max = m_lTypeIntegerMax;
      break;
    #endif
    #ifndef SSP_NO_TYPEINTEGERMIN
    case dispidTypeIntegerMin:
      lpCellTypeOld->Spec.Integer.Min = m_lTypeIntegerMin;
      break;
    #endif
    #ifndef SSP_NO_TYPEINTEGERSPININC
    case dispidTypeIntegerSpinInc:
      lpCellTypeOld->Spec.Integer.lSpinInc = m_lTypeIntegerSpinInc;
      break;
    #endif
    #ifndef SSP_NO_TYPEINTEGERSPINWRAP
    case dispidTypeIntegerSpinWrap:
      lpCellTypeOld->Spec.Integer.fSpinWrap = m_bTypeIntegerSpinWrap;
      break;
    #endif
    #ifndef SSP_NO_TYPESPIN
    case dispidTypeSpin:
      if( m_bTypeSpin )
        lpCellTypeOld->Style |= IS_SPIN;
      else
        lpCellTypeOld->Style &= ~IS_SPIN;
      break;
    #endif
#ifdef SS_V40
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
    case dispidTypeNegRed:
      lpCellTypeOld->Style &= ~(SSS_NEGRED);
      if (m_fTypeNegRed)
         lpCellTypeOld->Style |= SSS_NEGRED;
      break;
#endif // SS_V40
    default:
      return FALSE;
  }
  SS_SetTypeIntegerExt(lpSS, lpCellTypeNew,
                       lpCellTypeOld->Style,
                       lpCellTypeOld->Spec.Integer.Min,
                       lpCellTypeOld->Spec.Integer.Max,
                       lpCellTypeOld->Spec.Integer.fSpinWrap,
                       lpCellTypeOld->Spec.Integer.lSpinInc);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

#ifndef SS_NOCT_OWNERDRAW
BOOL CSsocxCtrl::SetTypeOwner(LPSPREADSHEET lpSS, DISPID dispid,
                              LPSS_CELLTYPE lpCellTypeOld,
                              LPSS_CELLTYPE lpCellTypeNew)
{
  switch( dispid )
  {
    #ifndef SSP_NO_TYPEOWNERDRAWSTYLE
    case dispidTypeOwnerDrawStyle:
      lpCellTypeOld->Style = m_lTypeOwnerDrawStyle;
      break;
    #endif
    default:
      return FALSE;
  }
  SS_SetTypeOwnerDraw(lpSS, lpCellTypeNew, lpCellTypeOld->Style);
  return TRUE;
}
#endif

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypePic(LPSPREADSHEET lpSS, DISPID dispid,
                            LPSS_CELLTYPE lpCellTypeOld,
                            LPSS_CELLTYPE lpCellTypeNew)
{
  LPTSTR lpszMask;
  LPTSTR lpszDefText;
  CString sMask;
  CString sDefText;

  if( lpCellTypeOld->Spec.Pic.hMask )
  {
    lpszMask = (LPTSTR)tbGlobalLock(lpCellTypeOld->Spec.Pic.hMask);
    if( lpszDefText = StrpBrk(lpszMask, _T("\t\n")) )
    {
      sMask = CString(lpszMask, (int)(lpszDefText - lpszMask));
      sDefText = lpszDefText + 1;
    }
    else
      sMask = lpszMask;
    tbGlobalUnlock(lpCellTypeOld->Spec.Pic.hMask);
  }
  switch( dispid )
  {
    #ifndef SSP_NO_TYPEHALIGN
    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFT | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFT;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    #endif
#ifdef SS_V30
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif

#ifdef SS_V35
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
#endif

    #ifndef SSP_NO_TYPEPICDEFAULTTEXT
    case dispidTypePicDefaultText:
      sDefText = m_sTypePicDefaultText;
      break;
    #endif
    #ifndef SSP_NO_TYPEPICMASK
    case dispidTypePicMask:
      sMask = m_sTypePicMask;
      break;
    #endif
#ifdef SS_V40
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
#endif // SS_V40

    default:
      return FALSE;
  }
  if( !sDefText.IsEmpty() )
    sMask += _T("\n") + sDefText;
  SS_SetTypePic(lpSS, lpCellTypeNew, lpCellTypeOld->Style, sMask);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypePict(LPSPREADSHEET lpSS, DISPID dispid,
                             LPSS_CELLTYPE lpCellTypeOld,
                             LPSS_CELLTYPE lpCellTypeNew)
{
  OLE_HANDLE hPict = NULL;
  OLE_HANDLE hPal = NULL;
  short nType = 0;
  switch( dispid )
  {
    #ifndef SSP_NO_TYPEPICTCENTER
    case dispidTypePictCenter:
      /*
      if( m_bTypePictCenter )
        lpCellTypeOld->Style |= VPS_CENTER;
      else
        lpCellTypeOld->Style &= ~VPS_CENTER;
      */

      lpCellTypeOld->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);

      if( m_bTypePictCenter )
         lpCellTypeOld->Style |= SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER;

      break;
    #endif
    #ifndef SSP_NO_TYPEPICTMAINTAINSCALE
    case dispidTypePictMaintainScale:
      if( m_bTypePictMaintainScale )
        lpCellTypeOld->Style |= VPS_MAINTAINSCALE;
      else
        lpCellTypeOld->Style &= ~VPS_MAINTAINSCALE;
      break;
    #endif
    #ifndef SSP_NO_TYPEPICTPICTURE
    case dispidTypePictPicture:
      if( m_lppictTypePictPicture )
      {
        m_lppictTypePictPicture->get_Handle(&hPict);
        m_lppictTypePictPicture->get_hPal(&hPal);
        m_lppictTypePictPicture->get_Type(&nType);
        lpCellTypeOld->Spec.ViewPict.lpPict = m_lppictTypePictPicture;
        lpCellTypeOld->Spec.ViewPict.hPictName = (TBGLOBALHANDLE)hPict;
        lpCellTypeOld->Spec.ViewPict.hPal = (HPALETTE)hPal;
        lpCellTypeOld->Style &= ~(VPS_BMP | VPS_ICON);
        if( PICTYPE_BITMAP == nType )
          lpCellTypeOld->Style |= VPS_HANDLE | VPS_BMP;
        else if( PICTYPE_ICON == nType )
          lpCellTypeOld->Style |= VPS_HANDLE | VPS_ICON;
        else if( 0 == nType )
          lpCellTypeOld->Style |= VPS_HANDLE;
        else
        {
          m_lppictTypePictPicture->Release();
          return FALSE;
        }
      }
		else
		{
		  lpCellTypeOld->Spec.ViewPict.lpPict = 0;
        lpCellTypeOld->Spec.ViewPict.hPictName = 0;
        lpCellTypeOld->Spec.ViewPict.hPal = 0;
		  lpCellTypeOld->Style &= ~(VPS_BMP | VPS_ICON);
		}
      break;
    #endif
    #ifndef SSP_NO_TYPEPICTSTRETCH
    case dispidTypePictStretch:
      if( m_bTypePictStretch )
        lpCellTypeOld->Style |= VPS_STRETCH;
      else
        lpCellTypeOld->Style &= ~VPS_STRETCH;
      break;
    #endif
#ifdef SS_V30
    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_LEFT | SSS_ALIGN_RIGHT | SSS_ALIGN_CENTER);

      if (m_nTypeHAlign == 0)
         lpCellTypeOld->Style |= SSS_ALIGN_LEFT;
      else if (m_nTypeHAlign == 1)
         lpCellTypeOld->Style |= SSS_ALIGN_RIGHT;
      else if (m_nTypeHAlign == 2)
         lpCellTypeOld->Style |= SSS_ALIGN_CENTER;

      break;
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif
    default:
      return FALSE;
  }
  if( lpCellTypeOld->Spec.ViewPict.hPal )
    SS_SetTypePictureHandle(lpSS, lpCellTypeNew, lpCellTypeOld->Style,
                            (HANDLE)lpCellTypeOld->Spec.ViewPict.hPictName,
                            lpCellTypeOld->Spec.ViewPict.hPal, FALSE);
  else
    SS_SetTypePicture(lpSS, lpCellTypeNew, lpCellTypeOld->Style,
                      (LPTSTR)&lpCellTypeOld->Spec.ViewPict.hPictName);
  lpCellTypeNew->Spec.ViewPict.lpPict = lpCellTypeOld->Spec.ViewPict.lpPict;
  if( lpCellTypeNew->Spec.ViewPict.lpPict )
    ((LPPICTURE)lpCellTypeNew->Spec.ViewPict.lpPict)->AddRef();
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypeStatic(LPSPREADSHEET lpSS, DISPID dispid,
                               LPSS_CELLTYPE lpCellTypeOld,
                               LPSS_CELLTYPE lpCellTypeNew)
{
  switch( dispid )
  {
    #ifndef SSP_NO_TYPEHALIGN
    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(SS_TEXT_LEFT | SS_TEXT_RIGHT |
                                SS_TEXT_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= SS_TEXT_LEFT;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= SS_TEXT_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= SS_TEXT_CENTER;
      break;
    #endif
    #ifndef SSP_NO_TYPETEXTALIGNVERT
    case dispidTypeTextAlignVert:
      lpCellTypeOld->Style &= ~(SS_TEXT_BOTTOM | SS_TEXT_VCENTER |
                                SS_TEXT_TOP);
      if( 0 == m_nTypeTextAlignVert )
        lpCellTypeOld->Style |= SS_TEXT_BOTTOM;
      else if( 1 == m_nTypeTextAlignVert )
        lpCellTypeOld->Style |= SS_TEXT_VCENTER;
      else if( 2 == m_nTypeTextAlignVert )
        lpCellTypeOld->Style |= SS_TEXT_TOP;
      break;
    #endif
    #ifdef SS_V30
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SS_TEXT_BOTTOM | SS_TEXT_VCENTER |
                                SS_TEXT_TOP);
      if( 0 == m_nTypeVAlign )
        lpCellTypeOld->Style |= SS_TEXT_TOP;
      else if( 1 == m_nTypeVAlign )
        lpCellTypeOld->Style |= SS_TEXT_BOTTOM;
      else if( 2 == m_nTypeVAlign )
        lpCellTypeOld->Style |= SS_TEXT_VCENTER;
      break;
    #endif

#ifdef SS_V35
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
#endif

    #ifndef SSP_NO_TYPETEXTPREFIX
    case dispidTypeTextPrefix:
      if( m_bTypeTextPrefix )
        lpCellTypeOld->Style |= SS_TEXT_PREFIX;
      else
        lpCellTypeOld->Style &= ~SS_TEXT_PREFIX;
      break;
    #endif
    #ifndef SSP_NO_TYPETEXTSHADOW
    case dispidTypeTextShadow:
      if( m_bTypeTextShadow )
        lpCellTypeOld->Style |= SS_TEXT_SHADOW;
      else
        lpCellTypeOld->Style &= ~SS_TEXT_SHADOW;
      break;
    #endif
    #ifndef SSP_NO_TYPETEXTSHADOWIN
    case dispidTypeTextShadowIn:
      if( m_bTypeTextShadowIn )
        lpCellTypeOld->Style |= SS_TEXT_SHADOWIN;
      else
        lpCellTypeOld->Style &= ~SS_TEXT_SHADOWIN;
      break;
    #endif
    #ifndef SSP_NO_TYPETEXTWORDWRAP
    case dispidTypeTextWordWrap:
      if( m_bTypeTextWordWrap )
        lpCellTypeOld->Style |= SS_TEXT_WORDWRAP;
      else
        lpCellTypeOld->Style &= ~SS_TEXT_WORDWRAP;
      break;
    #endif
#ifdef SS_V40
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
#endif // SS_V40

    default:
      return FALSE;
  }
  SS_SetTypeStaticText(lpSS, lpCellTypeNew, lpCellTypeOld->Style);
  return TRUE;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetTypeTime(LPSPREADSHEET lpSS, DISPID dispid,
                             LPSS_CELLTYPE lpCellTypeOld,
                             LPSS_CELLTYPE lpCellTypeNew)
{
  LPTIME lpTimeMin = NULL;
  LPTIME lpTimeMax = NULL;

  if( lpCellTypeOld->Spec.Time.SetRange )
  {
    lpTimeMin = &lpCellTypeOld->Spec.Time.Min;
    lpTimeMax = &lpCellTypeOld->Spec.Time.Max;
  }
  switch( dispid )
  {
    #ifndef SSP_NO_TYPEHALIGN
    case dispidTypeHAlign:
      lpCellTypeOld->Style &= ~(ES_LEFT | ES_RIGHT | ES_CENTER);
      if( 0 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_LEFT;
      else if( 1 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_RIGHT;
      else if( 2 == m_nTypeHAlign )
        lpCellTypeOld->Style |= ES_CENTER;
      break;
    #endif
#ifdef SS_V30
    case dispidTypeVAlign:
      lpCellTypeOld->Style &= ~(SSS_ALIGN_TOP | SSS_ALIGN_BOTTOM | SSS_ALIGN_VCENTER);
      lpCellTypeOld->Style |= SS_VALIGNTRANSLATE(m_nTypeVAlign);
      break;
#endif

#ifdef SS_V35
    case dispidTypeEllipses:
      lpCellTypeOld->Style &= ~(SSS_ELLIPSES);
      if (m_bTypeEllipses)
         lpCellTypeOld->Style |= SSS_ELLIPSES;
      break;
#endif

    #ifndef SSP_NO_TYPESPIN
    case dispidTypeSpin:
      lpCellTypeOld->Spec.Time.Format.bSpin = m_bTypeSpin;
      break;
    #endif
    #ifndef SSP_NO_TYPETIME24HOUR
    case dispidTypeTime24Hour:
#if defined(SS_V35) && !defined(SPREAD_JPN)
      if (2 == m_nTypeTime24Hour) // 2 == Default as defined in SSOCX.ODL
      {
        SS_TIMEFORMAT tf;
        TimeDefault(&tf);
        m_nTypeTime24Hour = tf.b24Hour;
      }
#endif
      lpCellTypeOld->Spec.Time.Format.b24Hour = m_nTypeTime24Hour;
      break;
    #endif
    #ifndef SSP_NO_TYPETIMEMAX
    case dispidTypeTimeMax:
      lpTimeMax = &m_TypeTimeMax;
      break;
    #endif
    #ifndef SSP_NO_TYPETIMEMIN
    case dispidTypeTimeMin:
      lpTimeMin = &m_TypeTimeMin;
      break;
    #endif
    #ifndef SSP_NO_TYPETIMESECONDS
    case dispidTypeTimeSeconds:
      lpCellTypeOld->Spec.Time.Format.bSeconds = m_bTypeTimeSeconds;
      break;
    #endif
    #ifndef SSP_NO_TYPETIMESEPARATOR
    case dispidTypeTimeSeparator:
      lpCellTypeOld->Spec.Time.Format.cSeparator = (TCHAR)m_nTypeTimeSeparator;
      break;
    #endif
#ifdef SS_V40
    case dispidTypeTextOrient:
		lpCellTypeOld->Style &= ~(SSS_TEXTORIENT_MASK);
		lpCellTypeOld->Style |= SS_TextOrientTranslate(m_nTypeTextOrient);
      break;
#endif // SS_V40

    default:
      return FALSE;
  }
  SS_SetTypeTime(lpSS, lpCellTypeNew, lpCellTypeOld->Style,
                 &lpCellTypeOld->Spec.Time.Format, lpTimeMin, lpTimeMax);
  return TRUE;
}

#if SS_V80
BOOL CSsocxCtrl::SetTypeCustom(LPSPREADSHEET lpSS, DISPID dispid,
                              LPSS_CELLTYPE lpCellTypeOld,
                              LPSS_CELLTYPE lpCellTypeNew)
{
  CString sText;
  if( lpCellTypeOld->Spec.Custom.hName )
  {
    sText = (LPCTSTR)GlobalLock(lpCellTypeOld->Spec.Custom.hName);
    GlobalUnlock(lpCellTypeOld->Spec.Custom.hName);
  }
  switch( dispid )
  {
    case dispidTypeCustomName:
       sText = m_sTypeCustomName;
       break;
    case dispidTypeCustomStyle:
      lpCellTypeOld->Style = m_lTypeCustomStyle;
      break;
    case dispidTypeCustomItemData:
      CT_VariantToValue(&m_TypeCustomItemData, &lpCellTypeOld->Spec.Custom.ItemData);
      break;
    default:
      return FALSE;
  }
  SS_SetTypeCustom(lpSS, lpCellTypeNew, sText, lpCellTypeOld->Style, &lpCellTypeOld->Spec.Custom.ItemData);
  return TRUE;
}
#endif // SS_V80
//--------------------------------------------------------------------

BOOL CSsocxCtrl::SetType(DISPID dispid)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_CELLTYPE cellTypeOld;
  SS_CELLTYPE cellTypeNew;
  SS_COORD x, x1, x2;
  SS_COORD y, y1, y2;
  BOOL bRet = TRUE;
  if( m_bBlockMode )
  {
    x1 = m_lRow;
    x2 = m_lRow2;
    y1 = m_lCol;
    y2 = m_lCol2;
  }
  else
  {
    x1 = x2 = m_lRow;
    y1 = y2 = m_lCol;
  }
  if( SS_ALLROWS == x1 || SS_ALLROWS == x2 )
    x1 = x2 = SS_ALLROWS;
  if( SS_ALLCOLS == y1 || SS_ALLCOLS == y2 )
    y1 = y2 = SS_ALLCOLS;
  for( x = x1; bRet && x <= x2; x++)
  {
    for( y = y1; bRet && y <= y2; y++ )
    {
      if( SS_RetrieveCellType(lpSS, &cellTypeOld, NULL, y, x) )
      {
        switch( cellTypeOld.Type )
        {
          case SS_TYPE_BUTTON:
            bRet = SetTypeButton(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_CHECKBOX:
            bRet = SetTypeCheck(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_COMBOBOX:
            bRet = SetTypeCombo(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_DATE:
            bRet = SetTypeDate(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_EDIT:
            bRet = SetTypeEdit(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_FLOAT:
            bRet = SetTypeFloat(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
#ifdef SS_V40
          case SS_TYPE_CURRENCY:
            bRet = SetTypeCurrency(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_NUMBER:
            bRet = SetTypeNumber(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_PERCENT:
            bRet = SetTypePercent(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
#endif // SS_V40
#ifdef SS_V70
          case SS_TYPE_SCIENTIFIC:
            bRet = SetTypeScientific(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
#endif // SS_V70
          #ifndef SS_NOCT_INT
          case SS_TYPE_INTEGER:
            bRet = SetTypeInteger(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          #endif
          #ifndef SS_NOCT_OWNERDRAW
          case SS_TYPE_OWNERDRAW:
            bRet = SetTypeOwner(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          #endif
          case SS_TYPE_PIC:
            bRet = SetTypePic(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_PICTURE:
            bRet = SetTypePict(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_STATICTEXT:
            bRet = SetTypeStatic(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
          case SS_TYPE_TIME:
            bRet = SetTypeTime(lpSS, dispid, &cellTypeOld, &cellTypeNew);
            break;
#ifdef SS_V80
          case SS_TYPE_CUSTOM:
             bRet = SetTypeCustom(lpSS, dispid, &cellTypeOld, &cellTypeNew);
             break;
#endif // SS_V80
          default:
            bRet = FALSE;
            break;
        }
      }
      else
        bRet = FALSE;
      if( bRet )
        SS_SetCellTypeRange(lpSS, y, x, y, x, &cellTypeNew);
    }
  }
  UnlockSheet();
  return bRet;
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetType(LPSS_CELLTYPE lpCellType)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  if( SS_RetrieveCellType(lpSS, lpCellType, NULL, m_lCol, m_lRow) )
    bRet = TRUE;
  UnlockSheet();
  return bRet;
}


//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetTypeOf(LPSS_CELLTYPE lpCellType, BYTE nType)
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bRet = FALSE;
  if( SS_RetrieveCellType(lpSS, lpCellType, NULL, m_lCol, m_lRow) )
    bRet = nType == lpCellType->Type;
  UnlockSheet();
  return bRet;
}  

#ifdef  BUGS
// Bug-017
BOOL GetSpreadAutoFill( HWND hwnd )
{
return TRUE; //CSsocxCtrl::GetDAutoFill();
}
#endif

//--------------------------------------------------------------------

//DBOCX...
#ifndef SSP_NO_DATASOURCE
#ifdef FP_ADB
DataSource * CSsocxCtrl::GetDataSource() 
{
  DataSource *pDataSource = NULL;
#ifdef SS_V70
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = LockSheet(AmbientUserMode() ? lpBook->nSheet : 0);
  if (lpSS) 
  {
    if( lpSS->pfpOleBinding )
      ((CfpOleBinding*)lpSS->pfpOleBinding)->GetDataSource(&pDataSource);
    UnlockSheet(lpBook->nSheet);
  }
  UnlockBook();
#else
  m_fpOleBinding.GetDataSource(&pDataSource);
#endif
  return pDataSource;
}
#else // not FP_ADB
LPUNKNOWN CSsocxCtrl::GetDataSource() 
{
  return NULL;
}
#endif  // if-else FP_ADB
#endif

//DBOCX.

//DBOCX...
#ifndef SSP_NO_DATASOURCE
#ifdef FP_ADB
void CSsocxCtrl::SetDataSource(DataSource *newValue) 
{
#ifdef SS_V70
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = LockSheet(AmbientUserMode() ? lpBook->nSheet : 0);
  if( lpSS ) // RFW - 5/15/07 - 20341
  {  
     if( newValue ) // 18029 -scl
     {
       if( !lpSS->pfpOleBinding )
         SSOcxInitSheetOleBinding(this, lpSS);
       ((CfpOleBinding*)lpSS->pfpOleBinding)->SetDataSource(newValue);
     }
     else
   	   SSOcxFreeSheetOleBinding(lpSS);
  }
  UnlockSheet(lpBook->nSheet);
  UnlockBook();
#else
  m_fpOleBinding.SetDataSource(newValue);
#endif
}
#else  // not FP_ADB
void CSsocxCtrl::SetDataSource(LPUNKNOWN newValue) 
{
  return;
}
#endif  // if-else FP_ADB
#endif

#if (!defined(SSP_NO_DATASOURCE) && defined(FP_ADB))
BSTR CSsocxCtrl::GetDataMember() 
{
  BSTR bstrDataMember = NULL;
#ifdef SS_V70
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = LockSheet(AmbientUserMode() ? lpBook->nSheet : 0);
  if (lpSS)
  {
    if( lpSS->pfpOleBinding )
      ((CfpOleBinding*)lpSS->pfpOleBinding)->GetDataMember(&bstrDataMember);
    UnlockSheet(lpBook->nSheet);
  }
  UnlockBook();
#else
  m_fpOleBinding.GetDataMember(&bstrDataMember);
#endif
  return bstrDataMember;
}
#endif

#if (!defined(SSP_NO_DATASOURCE) && defined(FP_ADB))
void CSsocxCtrl::SetDataMember(LPCTSTR lpszNewValue) 
{
  BSTR bstr = (lpszNewValue ? 
    (BSTR)fpSysAllocBSTRFromTString((LPTSTR)lpszNewValue) : NULL);
#ifdef SS_V70
  LPSS_BOOK lpBook = LockBook();
  LPSPREADSHEET lpSS = LockSheet(AmbientUserMode() ? lpBook->nSheet : 0);
  if (lpSS)
  {
    if( !lpSS->pfpOleBinding )
      SSOcxInitSheetOleBinding(this, lpSS);
    ((CfpOleBinding*)lpSS->pfpOleBinding)->SetDataMember(bstr);
    UnlockSheet(lpBook->nSheet);
  }
  UnlockBook();
#else
  m_fpOleBinding.SetDataMember(bstr);
#endif
  if (bstr)
    SysFreeString(bstr);
  return;
}
#endif

//DBOCX.

//--------------------------------------------------------------------

//DBOCX...
#ifndef SSP_NO_DATACHANGED
BOOL CSsocxCtrl::GetDataChanged()
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  // 2nd parameter is a MASK, if 0, the flags are only returned, not set.
  ULONG ulFlags = ssVBSetControlFlags((LONG)this, 0, 0, lpSS);
  UnlockSheet();
  // if bit is set return TRUE, else return FALSE
  return ((ulFlags & CTLFLG_DATACHANGED) != 0);
}
#endif
//DBOCX.

//DBOCX...
#ifndef SSP_NO_DATACHANGED
void CSsocxCtrl::SetDataChanged(BOOL bNewValue)
{
  ULONG ulFlags = (bNewValue ? CTLFLG_DATACHANGED : (ULONG)0);
  LPSPREADSHEET lpSS = LockSheet();

  // 2nd parameter is a MASK, 3rd is the value to set/reset.
  ssVBSetControlFlags((LONG)this, CTLFLG_DATACHANGED, ulFlags, lpSS);
  UnlockSheet();
}
#endif
//DBOCX.

#ifdef SS_V30
short CSsocxCtrl::GetAppearance()
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = (short)SS_GetAppearance(lpBook);
  UnlockBook();
  return nValue;
}

void CSsocxCtrl::SetAppearance(short nNewValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetAppearance(lpBook, nNewValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidAppearance);
}

short CSsocxCtrl::GetTypeComboBoxMaxDrop()
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
    nValue = cellType.Spec.ComboBox.dMaxRows;
  return nValue;
}

void CSsocxCtrl::SetTypeComboBoxMaxDrop(short newValue)
{
  //Add by BOC 99.5.5 (hyt)--------------------------
  //For ensure invalidate value can't input
  if (newValue <0)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    LPSS_BOOK lpBook = LockBook();
//    SET_NODESIGN();
    m_nTypeComboMaxDrop = newValue;
    lpBook->fLeaveEditModeOn = TRUE;
    SetType(dispidTypeComboBoxMaxDrop);
    lpBook->fLeaveEditModeOn = FALSE;
    SetModifiedFlag();
    UnlockBook();
  }
}

short CSsocxCtrl::GetTypeMaxEditLen()
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
    nValue = cellType.Spec.ComboBox.dMaxEditLen;
  else if( GetTypeOf(&cellType, SS_TYPE_EDIT) )
    nValue = cellType.Spec.Edit.Len;
  return nValue;
}

void CSsocxCtrl::SetTypeMaxEditLen(short nNewValue)
{
  SS_CELLTYPE cellType;

  if( nNewValue < 0 )
    nNewValue = 0;
  else if( nNewValue > SHRT_MAX )
    nNewValue = SHRT_MAX;

  if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
    m_nTypeComboMaxEditLen = nNewValue;
  else
    m_lTypeEditLen = nNewValue;

  SetType(dispidTypeMaxEditLen);
  SetModifiedFlag();
}

short CSsocxCtrl::GetTypeComboBoxWidth()
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
    nValue = cellType.Spec.ComboBox.dComboWidth;
  return nValue;
}

void CSsocxCtrl::SetTypeComboBoxWidth(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
//  SET_NODESIGN();
  m_nTypeComboWidth = newValue;
  lpBook->fLeaveEditModeOn = TRUE;
  SetType(dispidTypeComboBoxWidth);
  lpBook->fLeaveEditModeOn = FALSE;
  SetModifiedFlag();
  UnlockBook();
}

short CSsocxCtrl::GetTextTip()
{
  short nValue;

  if (m_hWnd)
     {
     FP_TT_INFO Info;
     fpTextTipGetInfo(m_hWnd, &Info);
     nValue = (short)Info.wStatus;
     }
  else
     nValue = m_nTextTip;

  return nValue;
}

void CSsocxCtrl::SetTextTip(short nNewValue)
{
  //Add by BOC 99.4.23 (hyt)--------------------------
  //For ensure invalidate value can't input
  if (nNewValue <0 || nNewValue > 4)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    if (m_hWnd)
       {
       FP_TT_INFO Info;

       WORD wItem = FP_TT_ITEM_STATUS;

       if (AmbientUserMode() == FALSE)
          {
          Info.fActive = FALSE;
          wItem |= FP_TT_ITEM_ACTIVE;
          }

       Info.wStatus = (WORD)nNewValue;
       fpTextTipSetInfo(m_hWnd, wItem, &Info);
       }
    else
       {
       m_nTextTip = nNewValue;
       fTextTipSet = TRUE;
       }

    SetModifiedFlag();
    if( AmbientUserMode() == FALSE )
      BoundPropertyChanged(dispidTextTip);
  }
}

long CSsocxCtrl::GetTextTipDelay()
{
  long lValue;

  if (m_hWnd)
     {
     FP_TT_INFO Info;
     fpTextTipGetInfo(m_hWnd, &Info);
     lValue = Info.lDelay;
     }
  else
     lValue = m_lTextTipDelay;

  return lValue;
}

void CSsocxCtrl::SetTextTipDelay(long lNewValue)
{
  if (m_hWnd)
     {
     FP_TT_INFO Info;

     Info.lDelay = lNewValue;
     fpTextTipSetInfo(m_hWnd, FP_TT_ITEM_DELAY, &Info);
     }
  else
     {
     m_lTextTipDelay = lNewValue;
     fTextTipDelaySet = TRUE;
     }

  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidTextTipDelay);
}

short CSsocxCtrl::GetScrollBarTrack()
{
  LPSPREADSHEET lpSS = LockSheet();
  WORD wScrollBarTrack = 0;
  BOOL fHorz = SS_GetBool(lpSS->lpBook, lpSS, SSB_HSCROLLBARTRACK);
  BOOL fVert = SS_GetBool(lpSS->lpBook, lpSS, SSB_VSCROLLBARTRACK);

  if (fHorz && fVert)
     wScrollBarTrack = 3;
  else if (fHorz)
     wScrollBarTrack = 2;
  else if (fVert)
     wScrollBarTrack = 1;

  UnlockSheet();
  return (short)wScrollBarTrack;
}

void CSsocxCtrl::SetScrollBarTrack(short nNewValue)
{
  LPSPREADSHEET lpSS = LockSheet();

   if (nNewValue == 0)
      {
      SS_SetBool(lpSS->lpBook, lpSS, SSB_VSCROLLBARTRACK, FALSE);
      SS_SetBool(lpSS->lpBook, lpSS, SSB_HSCROLLBARTRACK, FALSE);
      }
   else if (nNewValue == 1)
      {
      SS_SetBool(lpSS->lpBook, lpSS, SSB_VSCROLLBARTRACK, TRUE);
      SS_SetBool(lpSS->lpBook, lpSS, SSB_HSCROLLBARTRACK, FALSE);
      }
   else if (nNewValue == 2)
      {
      SS_SetBool(lpSS->lpBook, lpSS, SSB_VSCROLLBARTRACK, FALSE);
      SS_SetBool(lpSS->lpBook, lpSS, SSB_HSCROLLBARTRACK, TRUE);
      }
   else if (nNewValue == 3)
      {
      SS_SetBool(lpSS->lpBook, lpSS, SSB_VSCROLLBARTRACK, TRUE);
      SS_SetBool(lpSS->lpBook, lpSS, SSB_HSCROLLBARTRACK, TRUE);
      }

  UnlockSheet();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidScrollBarTrack);
}

short CSsocxCtrl::GetTypeVAlign()
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;

  if( GetType(&cellType) )
  {
    switch( cellType.Type )
    {
      case SS_TYPE_DATE:
      case SS_TYPE_EDIT:
      case SS_TYPE_FLOAT:
#ifdef SS_V40
      case SS_TYPE_CURRENCY:
      case SS_TYPE_NUMBER:
      case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
      case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      case SS_TYPE_INTEGER:
      case SS_TYPE_PIC:
      case SS_TYPE_TIME:
      case SS_TYPE_PICTURE:
      case SS_TYPE_COMBOBOX:
        if (cellType.Style & SSS_ALIGN_VCENTER)
          nValue = 2;
        else if (cellType.Style & SSS_ALIGN_BOTTOM)
          nValue = 1;
        else
          nValue = 0;

        break;

      case SS_TYPE_STATICTEXT:
        if (cellType.Style & SS_TEXT_VCENTER)
          nValue = 2;
        else if (cellType.Style & SS_TEXT_BOTTOM)
          nValue = 1;
        else
          nValue = 0;

        break;

      case SS_TYPE_CHECKBOX:
      case SS_TYPE_BUTTON:
        if (cellType.Style & SSS_ALIGN_TOP)
           nValue = 0;
        else if (cellType.Style & SSS_ALIGN_BOTTOM)
           nValue = 1;
        else
           nValue = 2;

        break;
    }    
  }

  return nValue;
}

void CSsocxCtrl::SetTypeVAlign(short nNewValue)
{
  m_nTypeVAlign = nNewValue;
  SetType(dispidTypeVAlign);
  SetModifiedFlag();
}

#ifdef SS_V70
long CSsocxCtrl::GetClipboardOptions()
{
  LPSS_BOOK lpBook = LockBook();
  long lValue = (long)lpBook->bClipboardOptions;
  UnlockBook();
  return lValue;
}

void CSsocxCtrl::SetClipboardOptions(long lNewValue)
{
  LPSS_BOOK lpBook = LockBook();
  WORD      wOptionsOld = (WORD)lpBook->bClipboardOptions;

  if (lNewValue >= 0 && lNewValue <= 15)
     lpBook->bClipboardOptions = (BYTE)lNewValue;

  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidClipboardOptions);
}
#else
short CSsocxCtrl::GetClipboardOptions()
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = (short)lpBook->bClipboardOptions;
  UnlockBook();
  return nValue;
}

void CSsocxCtrl::SetClipboardOptions(short nNewValue)
{
  LPSS_BOOK lpBook = LockBook();
  WORD      wOptionsOld = (WORD)lpBook->bClipboardOptions;

  if (nNewValue >= 0 && nNewValue <= 15)
     lpBook->bClipboardOptions = (BYTE)nNewValue;

  UnlockBook();
  SetModifiedFlag();
  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidClipboardOptions);
}
#endif

long CSsocxCtrl::GetTypeComboBoxhWnd()
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  long lValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
    lValue = (long)cellType.Spec.ComboBox.hWndDropDown;
  return lValue;
}

void CSsocxCtrl::SetTypeComboBoxhWnd(long newValue)
{
//  SET_NODESIGN();
  m_nTypeCombohWnd = (HWND)newValue;
  SetType(dispidTypeComboBoxhWnd);
  SetModifiedFlag();
}

OLE_COLOR CSsocxCtrl::GetSelBackColor()
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF crBackColor, crForeColor;
  SS_GetSelColor(lpBook, &crBackColor, &crForeColor);
  UnlockBook();
  return crBackColor;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetSelBackColor(OLE_COLOR newValue)
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF crBackColor, crForeColor;
  SS_GetSelColor(lpBook, &crBackColor, &crForeColor);
  SS_SetSelColor(lpBook, newValue, crForeColor);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
}

OLE_COLOR CSsocxCtrl::GetSelForeColor()
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF crBackColor, crForeColor;
  SS_GetSelColor(lpBook, &crBackColor, &crForeColor);
  UnlockBook();
  return crForeColor;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetSelForeColor(OLE_COLOR newValue)
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF crBackColor, crForeColor;
  SS_GetSelColor(lpBook, &crBackColor, &crForeColor);
  SS_SetSelColor(lpBook, crBackColor, newValue);
  INVALIDATE_CONTROL();
  UnlockBook();
  SetModifiedFlag();
}

short CSsocxCtrl::GetTwoDigitYearMax()
{
  LPSS_BOOK lpBook = LockBook();
  short     nTwoDigitYearMax = SS_GetTwoDigitYearMax(lpBook);
  UnlockBook();
  return nTwoDigitYearMax;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetTwoDigitYearMax(short nTwoDigitYearMax)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetTwoDigitYearMax(lpBook, nTwoDigitYearMax);
  UnlockBook();
  SetModifiedFlag();
}

#endif // SS_V30

#ifdef SS_V35
BSTR CSsocxCtrl::GetCellNote()
{
  GET_NODESIGN();
  LPSPREADSHEET  lpSS = LockSheet();
  CString        s = "";
  short          nLen = SS_GetCellNote(lpSS, m_lCol, m_lRow, NULL);
  
  if (nLen)
  {
    TBGLOBALHANDLE ghNote = tbGlobalAlloc(GHND, (nLen+1)*sizeof(TCHAR));
    LPTSTR         lpszNote = (LPTSTR)tbGlobalLock(ghNote);

    SS_GetCellNote(lpSS, m_lCol, m_lRow, lpszNote);
    s = lpszNote;  
    tbGlobalUnlock(ghNote);
    tbGlobalFree(ghNote);

    UnlockSheet();

    return s.AllocSysString();
  }
  else
    return NULL;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetCellNote(LPCTSTR Note)
{
  LPSPREADSHEET  lpSS = LockSheet();
  
//  SS_SetCellCellNote(lpSS, m_lCol, m_lRow, Note);

  if( m_bBlockMode )
    SS_SetCellNoteRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2, (LPTSTR)Note);
  else
    SS_SetCellNoteRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow, (LPTSTR)Note);

  INVALIDATE_CONTROL();
  SetModifiedFlag();
  UnlockSheet();

  if( 0 == m_lCol || 0 == m_lRow )
    ControlInfoChanged();
}

BOOL CSsocxCtrl::GetScriptEnhanced()
{
  LPSS_BOOK lpBook = LockBook();

  BOOL  bScriptEnhanced = SS_GetScriptEnhanced(lpBook);
  
  UnlockBook();

  return bScriptEnhanced;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetScriptEnhanced(BOOL NewValue)
{
  LPSS_BOOK lpBook = LockBook();

  SS_SetScriptEnhanced(lpBook, NewValue);
  
  INVALIDATE_CONTROL();

  SetModifiedFlag();

  UnlockBook();
}

long CSsocxCtrl::GetSelectionCount()
{
  GET_NODESIGN();
  LPSPREADSHEET  lpSS = LockSheet();
  short nOpMode = SS_GetOperationMode(lpSS);
  long lSelCnt = 0;

  switch( nOpMode )
  {
  case SS_OPMODE_NORMAL:
  case SS_OPMODE_READONLY:
    if( lpSS->lpBook->fAllowMultipleSelBlocks )
    {
      short nCnt;
      HGLOBAL hMultiSelBlocks = SSGetMultiSelBlocks(lpSS->lpBook->hWnd, &nCnt);
		// RFW - 1/2/02 - 9606
		lSelCnt = nCnt;
      // lSelCnt = max(1, nCnt);
      if (hMultiSelBlocks)
        GlobalFree(hMultiSelBlocks);
    }
    else if( SS_GetSelectBlock(lpSS, NULL, NULL) )
      lSelCnt = 1;
    break;
  case SS_OPMODE_SINGLESEL:
    if( SS_SelModeSendMessage(lpSS, SS_SELMODE_GETCURSEL, 0, 0, 0) < 0 )
      lSelCnt = 0;
    else
      lSelCnt = 1;
    break;
  case SS_OPMODE_MULTISEL:
  case SS_OPMODE_EXTSEL:
    lSelCnt = SS_SelModeSendMessage(lpSS, SS_SELMODE_GETSELCOUNT, 0, 0, 0);
    break;
  // selections are not allowed in read-only and row modes. -scl
  }
  UnlockSheet();
  return lSelCnt;
}

void CSsocxCtrl::SetSelectionCount()
{
  SetNotSupported();
}

short CSsocxCtrl::GetUserColAction()
{
  LPSPREADSHEET  lpSS = LockSheet();

  short  nUserColAction = SS_GetUserColAction(lpSS);
  
  UnlockSheet();

  return nUserColAction;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetUserColAction(short NewValue)
{
  LPSPREADSHEET  lpSS = LockSheet();

  SS_SetUserColAction(lpSS, NewValue);
  
  INVALIDATE_CONTROL();

  SetModifiedFlag();

  UnlockSheet();
}

short CSsocxCtrl::GetColUserSortIndicator(long lCol)
{
  LPSPREADSHEET  lpSS = LockSheet();
  short          nSortIndicator;

  SS_AdjustCellCoords(lpSS, &lCol, NULL);
  nSortIndicator = SS_GetColUserSortIndicator(lpSS, lCol);

  UnlockSheet();

  return nSortIndicator;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetColUserSortIndicator(long lCol, short NewValue)
{
  LPSPREADSHEET  lpSS = LockSheet();

  SS_AdjustCellCoords(lpSS, &lCol, NULL);
  SS_SetColUserSortIndicator(lpSS, lCol, NewValue);
  
  INVALIDATE_CONTROL();

  SetModifiedFlag();

  UnlockSheet();
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetShowScrollTips()
{
  LPSS_BOOK lpBook = LockBook();
  short     nShowScrollTips = SS_GetShowScrollTips(lpBook);
  UnlockBook();
  
  return nShowScrollTips;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetShowScrollTips(short NewValue)
{
  LPSS_BOOK lpBook = LockBook();
  
  SS_SetShowScrollTips(lpBook, NewValue);
  INVALIDATE_CONTROL();
  SetModifiedFlag();
  UnlockBook();

  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidShowScrollTips);
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetCellNoteIndicator()
{
  LPSS_BOOK lpBook = LockBook();
  short     nCellNoteIndicator = SS_GetCellNoteIndicator(lpBook);
  UnlockBook();
 
  return nCellNoteIndicator;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetCellNoteIndicator(short NewValue)
{
  LPSS_BOOK lpBook = LockBook();
  
  SS_SetCellNoteIndicator(lpBook, NewValue);
  INVALIDATE_CONTROL();
  SetModifiedFlag();
  UnlockBook();

  if( AmbientUserMode() == FALSE )
    BoundPropertyChanged(dispidCellNoteIndicator);
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetTypeEllipses()
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL bValue = FALSE;
  if( GetType(&cellType) )
  {
    switch( cellType.Type )
    {
      case SS_TYPE_STATICTEXT:
      case SS_TYPE_DATE:
      case SS_TYPE_EDIT:
      case SS_TYPE_FLOAT:
#ifdef SS_V40
      case SS_TYPE_CURRENCY:
      case SS_TYPE_NUMBER:
      case SS_TYPE_PERCENT:
#endif // SS_V40
#ifdef SS_V70
      case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
      case SS_TYPE_INTEGER:
      case SS_TYPE_PIC:
      case SS_TYPE_TIME:
        if( SSS_ELLIPSES & cellType.Style )
          bValue = TRUE;
        break;
    }    
  }
  return bValue;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetTypeEllipses(BOOL bNewValue)
{
  m_bTypeEllipses = bNewValue;
  SetType(dispidTypeEllipses);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

#endif // SS_V35
//--------------------------------------------------------------------

//--------------------------------------------------------------------

void CSsocxCtrl::SetEventEnabled(short nEvent, BOOL fIsEnabled)
{
  if (0 == nEvent)
  {
    short i;
    for (i = 1; i < EVENTENABLED_CNT; i++)
      m_bEventEnabled[i] = fIsEnabled;
  }
  else if( nEvent < EVENTENABLED_CNT )
    m_bEventEnabled[nEvent] = fIsEnabled;

  SetModifiedFlag();
}

#ifdef SS_V40

short CSsocxCtrl::GetColMerge()
{
  GET_NODESIGN();
  LPSPREADSHEET  lpSS = LockSheet();

  short  nColMerge = SS_GetColMerge(lpSS, m_lCol);
  
  UnlockSheet();

  return nColMerge;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetColMerge(short NewValue)
{
  LPSPREADSHEET  lpSS = LockSheet();

  if (m_lCol >= 0)
  {
    SS_SetColMerge(lpSS, m_lCol, NewValue);
    INVALIDATE_CONTROL();
    SetModifiedFlag();
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

  UnlockSheet();
}

short CSsocxCtrl::GetRowMerge()
{
  GET_NODESIGN();
  LPSPREADSHEET  lpSS = LockSheet();

  short  nRowMerge = SS_GetRowMerge(lpSS, m_lRow);
  
  UnlockSheet();

  return nRowMerge;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetRowMerge(short NewValue)
{
  LPSPREADSHEET  lpSS = LockSheet();

  if (m_lRow >= 0)
  {
    SS_SetRowMerge(lpSS, m_lRow, NewValue);
    INVALIDATE_CONTROL();
    SetModifiedFlag();
  }
  else
    ThrowError(CTL_E_INVALIDPROPERTYVALUE);

  UnlockSheet();
}

BSTR CSsocxCtrl::GetColID()
{
  GET_NODESIGN();
  LPSPREADSHEET  lpSS = LockSheet();
  CString        s = "";
  short          nLen = SS_GetColID(lpSS, m_lCol, NULL);
  #define COLID_BUFFER_LEN 10
  TCHAR          Buffer[COLID_BUFFER_LEN + 1];

  if (nLen)
  {
    TBGLOBALHANDLE ghColID;
    LPTSTR         lpszColID;

    if (nLen > COLID_BUFFER_LEN)
    {
      ghColID = tbGlobalAlloc(GHND, (nLen+1)*sizeof(TCHAR));
      lpszColID = (LPTSTR)tbGlobalLock(ghColID);
    }
    else
      lpszColID = Buffer;

    SS_GetColID(lpSS, m_lCol, lpszColID);
    s = lpszColID;
    if (nLen > COLID_BUFFER_LEN)
    {
      tbGlobalUnlock(ghColID);
      tbGlobalFree(ghColID);
    }

    UnlockSheet();

    return s.AllocSysString();
  }

  UnlockSheet();
  return NULL;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetColID(LPCTSTR ColID)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetColID(lpSS, m_lCol, ColID);
  UnlockSheet();

  SetModifiedFlag();
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetRowHeaderCols()
{
  LPSPREADSHEET lpSS = LockSheet();
  long lRowHeaderCols = SS_GetRowHeaderCols(lpSS);
  
  UnlockSheet();

  return lRowHeaderCols;
}

void CSsocxCtrl::SetRowHeaderCols(long NewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lHeaderCntOld = lpSS->Col.HeaderCnt;

  SS_SetRowHeaderCols(lpSS, NewValue);
  // RFW - 5/24/04 - 14327
  if (m_lCol >= lHeaderCntOld)
    m_lCol += lpSS->Col.HeaderCnt - lHeaderCntOld;

  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetRowHeadersAutoTextIndex()
{
  LPSPREADSHEET lpSS = LockSheet();
  long lRowHeadersAutoTextIndex = SS_GetRowHeadersAutoTextIndex(lpSS);
  
  UnlockSheet();

  return lRowHeadersAutoTextIndex;
}

void CSsocxCtrl::SetRowHeadersAutoTextIndex(long NewValue)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_SetRowHeadersAutoTextIndex(lpSS, NewValue);

  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetColHeaderRows()
{
  LPSPREADSHEET lpSS = LockSheet();
  long lColHeaderRows = SS_GetColHeaderRows(lpSS);
  
  UnlockSheet();

  return lColHeaderRows;
}

void CSsocxCtrl::SetColHeaderRows(long NewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_COORD lHeaderCntOld = lpSS->Row.HeaderCnt;

  SS_SetColHeaderRows(lpSS, NewValue);
  // RFW - 5/24/04 - 14327
  if (m_lRow >= lHeaderCntOld)
    m_lRow += lpSS->Row.HeaderCnt - lHeaderCntOld;

  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetColHeadersAutoTextIndex()
{
  LPSPREADSHEET lpSS = LockSheet();
  long lColHeadersAutoTextIndex = SS_GetColHeadersAutoTextIndex(lpSS);
  
  UnlockSheet();

  return lColHeadersAutoTextIndex;
}

void CSsocxCtrl::SetColHeadersAutoTextIndex(long NewValue)
{
  LPSPREADSHEET lpSS = LockSheet();

  SS_SetColHeadersAutoTextIndex(lpSS, NewValue);

  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

long CSsocxCtrl::GetColHeadersUserSortIndex()
{
  LPSPREADSHEET lpSS = LockSheet();
  long lColHeadersUserSortIndex = SS_GetColHeadersUserSortIndex(lpSS);
  UnlockSheet();
  return lColHeadersUserSortIndex;
}

void CSsocxCtrl::SetColHeadersUserSortIndex(long NewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetColHeadersUserSortIndex(lpSS, NewValue);
  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeComboBoxAutoSearch()
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short nValue = 0;
  if( GetTypeOf(&cellType, SS_TYPE_COMBOBOX) )
    nValue = (short)cellType.Spec.ComboBox.AutoSearch;
  return nValue;
}

void CSsocxCtrl::SetTypeComboBoxAutoSearch(short newValue)
{
//  SET_NODESIGN();
  m_bTypeComboAutoSearch = (BYTE)newValue;
  SetType(dispidTypeComboBoxAutoSearch);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypeSpinInc(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetType(&cellType))
  {
    switch( cellType.Type )
    {
      case SS_TYPE_INTEGER:
        Value = cellType.Spec.Integer.lSpinInc;
        break;
      case SS_TYPE_CURRENCY:
        Value = cellType.Spec.Currency.SpinInc;
        break;
      case SS_TYPE_NUMBER:
        Value = cellType.Spec.Number.SpinInc;
        break;
      case SS_TYPE_PERCENT:
        Value = cellType.Spec.Percent.SpinInc;
        break;
      default:
        break;
    }
  }
  return Value;
}

void CSsocxCtrl::SetTypeSpinInc(double newValue)
{
  SS_CELLTYPE cellType;

  if (GetType(&cellType))
  {
    switch( cellType.Type )
    {
      case SS_TYPE_INTEGER:
        m_lTypeIntegerSpinInc = (long)newValue;
        break;
      case SS_TYPE_CURRENCY:
        m_TypeCurrency.Spec.Currency.SpinInc = newValue;
        break;
      case SS_TYPE_NUMBER:
        m_TypeNumber.Spec.Number.SpinInc = newValue;
        break;
      case SS_TYPE_PERCENT:
        m_TypePercent.Spec.Percent.SpinInc = newValue;
        break;
      default:
        break;
    }

    SetType(dispidTypeSpinInc);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetTypeSpinWrap(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL Value = 0;
  if (GetType(&cellType))
  {
    switch( cellType.Type )
    {
      case SS_TYPE_INTEGER:
        Value = cellType.Spec.Integer.fSpinWrap;
        break;
      case SS_TYPE_CURRENCY:
        Value = cellType.Spec.Currency.fSpinWrap;
        break;
      case SS_TYPE_NUMBER:
        Value = cellType.Spec.Number.fSpinWrap;
        break;
      case SS_TYPE_PERCENT:
        Value = cellType.Spec.Percent.fSpinWrap;
        break;
      default:
        break;
    }
  }

  return Value;
}

void CSsocxCtrl::SetTypeSpinWrap(BOOL newValue)
{
  SS_CELLTYPE cellType;

  if (GetType(&cellType))
  {
    switch( cellType.Type )
    {
      case SS_TYPE_INTEGER:
        m_bTypeIntegerSpinWrap = newValue;
        break;
      case SS_TYPE_CURRENCY:
        m_TypeCurrency.Spec.Currency.fSpinWrap = newValue;
        break;
      case SS_TYPE_NUMBER:
        m_TypeNumber.Spec.Number.fSpinWrap = newValue;
        break;
      case SS_TYPE_PERCENT:
        m_TypePercent.Spec.Percent.fSpinWrap = newValue;
        break;
      default:
        break;
    }
    SetType(dispidTypeSpinWrap);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetTypeNegRed(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL Value = 0;
  if (GetType(&cellType))
  {
    switch( cellType.Type )
    {
      case SS_TYPE_CURRENCY:
      case SS_TYPE_NUMBER:
      case SS_TYPE_PERCENT:
      case SS_TYPE_FLOAT:
      case SS_TYPE_INTEGER:
#ifdef SS_V70
      case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
        Value = cellType.Style & SSS_NEGRED ? TRUE : FALSE;
        break;
      default:
        break;
    }
  }

  return Value;
}

void CSsocxCtrl::SetTypeNegRed(BOOL newValue)
{
  SS_CELLTYPE cellType;

  if (GetType(&cellType))
  {
    switch( cellType.Type )
    {
      case SS_TYPE_CURRENCY:
      case SS_TYPE_NUMBER:
      case SS_TYPE_PERCENT:
      case SS_TYPE_FLOAT:
      case SS_TYPE_INTEGER:
#ifdef SS_V70
      case SS_TYPE_SCIENTIFIC:
#endif // SS_V70
        m_fTypeNegRed = newValue;
        break;
      default:
        break;
    }
    SetType(dispidTypeNegRed);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypeCurrencyMin(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    Value = (double)cellType.Spec.Currency.Min;
  return Value;
}

void CSsocxCtrl::SetTypeCurrencyMin(double newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_CURRENCY) )
  {
    if (newValue > cellType.Spec.Currency.Max)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      m_TypeCurrency.Spec.Currency.Min = newValue;
      SetType(dispidTypeCurrencyMin);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypeCurrencyMax(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    Value = (double)cellType.Spec.Currency.Max;
  return Value;
}

void CSsocxCtrl::SetTypeCurrencyMax(double newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_CURRENCY) )
  {
    if (newValue < cellType.Spec.Currency.Min)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      m_TypeCurrency.Spec.Currency.Max = newValue;
      SetType(dispidTypeCurrencyMax);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

BSTR CSsocxCtrl::GetTypeCurrencyDecimal(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    sValue = cellType.Spec.Currency.szDecimal;
  return sValue.AllocSysString();
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetTypeCurrencyDecimal(LPCTSTR newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_CURRENCY) )
  {  
    if (newValue && _tcsclen(newValue) > 1)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      lstrcpyn(m_TypeCurrency.Spec.Currency.szDecimal, newValue ? newValue : _T(""), 4);
      SetType(dispidTypeCurrencyDecimal);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

BSTR CSsocxCtrl::GetTypeCurrencySeparator(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    sValue = cellType.Spec.Currency.szSeparator;
  return sValue.AllocSysString();
}

void CSsocxCtrl::SetTypeCurrencySeparator(LPCTSTR newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_CURRENCY) )
  {  
    lstrcpyn(m_TypeCurrency.Spec.Currency.szSeparator, newValue, 4);
    SetType(dispidTypeCurrencySeparator);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

BSTR CSsocxCtrl::GetTypeCurrencySymbol(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    sValue = cellType.Spec.Currency.szCurrency;
  return sValue.AllocSysString();
}

void CSsocxCtrl::SetTypeCurrencySymbol(LPCTSTR newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_CURRENCY) )
  {  
    lstrcpyn(m_TypeCurrency.Spec.Currency.szCurrency, newValue, 6);
    SetType(dispidTypeCurrencySymbol);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeCurrencyLeadingZero(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    Value = cellType.Spec.Currency.bLeadingZero;
  return Value;
}

void CSsocxCtrl::SetTypeCurrencyLeadingZero(short newValue)
{
  if (newValue < 0 || newValue > 2)
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

  m_TypeCurrency.Spec.Currency.bLeadingZero = (BYTE)newValue;
  SetType(dispidTypeCurrencyLeadingZero);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeCurrencyDecPlaces(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    Value = (short)cellType.Spec.Currency.Right;
  return Value;
}

void CSsocxCtrl::SetTypeCurrencyDecPlaces(short newValue)
{
  if (newValue <0 || newValue > 14)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_TypeCurrency.Spec.Currency.Right = newValue;
    SetType(dispidTypeCurrencyDecPlaces);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeCurrencyNegStyle(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    Value = (short)cellType.Spec.Currency.bNegStyle;
  return Value;
}

void CSsocxCtrl::SetTypeCurrencyNegStyle(short newValue)
{
  if (newValue < 0 || newValue > 16)
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

  m_TypeCurrency.Spec.Currency.bNegStyle = (BYTE)newValue;
  SetType(dispidTypeCurrencyNegStyle);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeCurrencyPosStyle(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    Value = (short)cellType.Spec.Currency.bPosStyle;
  return Value;
}

void CSsocxCtrl::SetTypeCurrencyPosStyle(short newValue)
{
  if (newValue < 0 || newValue > 4)
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

  m_TypeCurrency.Spec.Currency.bPosStyle = (BYTE)newValue;
  SetType(dispidTypeCurrencyPosStyle);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetTypeCurrencyShowSep(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    Value = (BOOL)cellType.Spec.Currency.fShowSeparator;
  return Value;
}

void CSsocxCtrl::SetTypeCurrencyShowSep(BOOL newValue)
{
  m_TypeCurrency.Spec.Currency.fShowSeparator = (BYTE)newValue;
  SetType(dispidTypeCurrencyShowSep);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetTypeCurrencyShowSymbol(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_CURRENCY))
    Value = (BOOL)cellType.Spec.Currency.fShowCurrencySymbol;
  return Value;
}

void CSsocxCtrl::SetTypeCurrencyShowSymbol(BOOL newValue)
{
  m_TypeCurrency.Spec.Currency.fShowCurrencySymbol = (BYTE)newValue;
  SetType(dispidTypeCurrencyShowSymbol);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypeNumberMin(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_NUMBER))
    Value = cellType.Spec.Number.Min;
  return Value;
}

void CSsocxCtrl::SetTypeNumberMin(double newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_NUMBER) )
  {
    if (newValue > cellType.Spec.Number.Max)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      m_TypeNumber.Spec.Number.Min = newValue;
      SetType(dispidTypeNumberMin);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypeNumberMax(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_NUMBER))
    Value = (double)cellType.Spec.Number.Max;
  return Value;
}

void CSsocxCtrl::SetTypeNumberMax(double newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_NUMBER) )
  {
    if (newValue < cellType.Spec.Number.Min)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      m_TypeNumber.Spec.Number.Max = newValue;
      SetType(dispidTypeNumberMax);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

BSTR CSsocxCtrl::GetTypeNumberDecimal(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if (GetTypeOf(&cellType, SS_TYPE_NUMBER))
    sValue = cellType.Spec.Number.szDecimal;
  return sValue.AllocSysString();
}

void CSsocxCtrl::SetTypeNumberDecimal(LPCTSTR newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_NUMBER) )
  {
    if (newValue && _tcsclen(newValue) > 1)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      lstrcpyn(m_TypeNumber.Spec.Number.szDecimal, newValue ? newValue : _T(""), 4);
      SetType(dispidTypeNumberDecimal);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

BSTR CSsocxCtrl::GetTypeNumberSeparator(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if (GetTypeOf(&cellType, SS_TYPE_NUMBER))
    sValue = cellType.Spec.Number.szSeparator;
  return sValue.AllocSysString();
}

void CSsocxCtrl::SetTypeNumberSeparator(LPCTSTR newValue)
{
  SS_CELLTYPE cellType;

  if( GetTypeOf(&cellType, SS_TYPE_NUMBER) )
  {  
    lstrcpyn(m_TypeNumber.Spec.Number.szSeparator, newValue, 4);
    SetType(dispidTypeNumberSeparator);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeNumberLeadingZero(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_NUMBER))
    Value = (short)cellType.Spec.Number.bLeadingZero;
  return Value;
}

void CSsocxCtrl::SetTypeNumberLeadingZero(short newValue)
{
  if (newValue < 0 || newValue > 2)
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

  m_TypeNumber.Spec.Number.bLeadingZero = (BYTE)newValue;
  SetType(dispidTypeNumberLeadingZero);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeNumberDecPlaces(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_NUMBER))
    Value = (short)cellType.Spec.Number.Right;
  return Value;
}

void CSsocxCtrl::SetTypeNumberDecPlaces(short newValue)
{
  if (newValue <0 || newValue > 14)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_TypeNumber.Spec.Number.Right = newValue;
    SetType(dispidTypeNumberDecPlaces);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeNumberNegStyle(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_NUMBER))
    Value = (short)cellType.Spec.Number.bNegStyle;
  return Value;
}

void CSsocxCtrl::SetTypeNumberNegStyle(short newValue)
{
  if (newValue < 0 || newValue > 5)
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

  m_TypeNumber.Spec.Number.bNegStyle = (BYTE)newValue;
  SetType(dispidTypeNumberNegStyle);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetTypeNumberShowSep(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  BOOL Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_NUMBER))
    Value = (BOOL)cellType.Spec.Number.fShowSeparator;
  return Value;
}

void CSsocxCtrl::SetTypeNumberShowSep(BOOL newValue)
{
  m_TypeNumber.Spec.Number.fShowSeparator = (BYTE)newValue;
  SetType(dispidTypeNumberShowSep);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypePercentMin(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_PERCENT))
    Value = (double)cellType.Spec.Percent.Min;
  return Value;
}

void CSsocxCtrl::SetTypePercentMin(double newValue)
{
  SS_CELLTYPE   cellType;
  if( GetTypeOf(&cellType, SS_TYPE_PERCENT) )
  {
    if (newValue > cellType.Spec.Percent.Max)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      m_TypePercent.Spec.Percent.Min = newValue;
      SetType(dispidTypePercentMin);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypePercentMax(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_PERCENT))
    Value = (double)cellType.Spec.Percent.Max;
  return Value;
}

void CSsocxCtrl::SetTypePercentMax(double newValue)
{
  SS_CELLTYPE   cellType;
  if( GetTypeOf(&cellType, SS_TYPE_PERCENT) )
  {
    if (newValue < cellType.Spec.Percent.Min)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      m_TypePercent.Spec.Percent.Max = newValue;
      SetType(dispidTypePercentMax);
      SetModifiedFlag();
    }
  }
  SetModifiedFlag();
}

//--------------------------------------------------------------------

BSTR CSsocxCtrl::GetTypePercentDecimal(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if (GetTypeOf(&cellType, SS_TYPE_PERCENT))
    sValue = cellType.Spec.Percent.szDecimal;
  return sValue.AllocSysString();
}

void CSsocxCtrl::SetTypePercentDecimal(LPCTSTR newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_PERCENT) )
  {  
    if (newValue && _tcsclen(newValue) > 1)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      lstrcpyn(m_TypePercent.Spec.Percent.szDecimal, newValue ? newValue : _T(""), 4);
      SetType(dispidTypePercentDecimal);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypePercentDecPlaces(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_PERCENT))
    Value = (short)cellType.Spec.Percent.Right;
  return Value;
}

void CSsocxCtrl::SetTypePercentDecPlaces(short newValue)
{
  if (newValue <0 || newValue > 14)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_TypePercent.Spec.Percent.Right = newValue;
    SetType(dispidTypePercentDecPlaces);
    SetModifiedFlag();
  }
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypePercentNegStyle(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_PERCENT))
    Value = (short)cellType.Spec.Percent.bNegStyle;
  return Value;
}

void CSsocxCtrl::SetTypePercentNegStyle(short newValue)
{
  if (newValue < 0 || newValue > 8)
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

  m_TypePercent.Spec.Percent.bNegStyle = (BYTE)newValue;
  SetType(dispidTypePercentNegStyle);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypePercentLeadingZero(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_PERCENT))
    Value = (short)cellType.Spec.Percent.bLeadingZero;
  return Value;
}

void CSsocxCtrl::SetTypePercentLeadingZero(short newValue)
{
  if (newValue < 0 || newValue > 2)
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

  m_TypePercent.Spec.Percent.bLeadingZero = (BYTE)newValue;
  SetType(dispidTypePercentLeadingZero);
  SetModifiedFlag();
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeTextOrient(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetType(&cellType))
  {
    if ((cellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_VERT_LTR)
      Value = 1;
    else if ((cellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_DOWN)
      Value = 2;
    else if ((cellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_UP)
      Value = 3;
    else if ((cellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_INVERT)
      Value = 4;
    else if ((cellType.Style & SSS_TEXTORIENT_MASK) == SSS_TEXTORIENT_VERT_RTL)
      Value = 5;
  }

  return Value;
}

void CSsocxCtrl::SetTypeTextOrient(short newValue)
{
  if (newValue < 0 || newValue > 5)
    ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);

    m_nTypeTextOrient = newValue;
    SetType(dispidTypeTextOrient);
    SetModifiedFlag();
}


DWORD SS_TextOrientTranslate(short nTypeTextOrient)
{
  DWORD dwStyle = 0;

  switch (nTypeTextOrient)
  {
    case 1:
      dwStyle = SSS_TEXTORIENT_VERT_LTR; break;
    case 2:
      dwStyle = SSS_TEXTORIENT_DOWN; break;
    case 3:
      dwStyle = SSS_TEXTORIENT_UP; break;
    case 4:
      dwStyle = SSS_TEXTORIENT_INVERT; break;
    case 5:
      dwStyle = SSS_TEXTORIENT_VERT_RTL; break;
  }

  return (dwStyle);
}

BSTR CSsocxCtrl::GetCellTag()
{
  GET_NODESIGN();
  LPSPREADSHEET  lpSS = LockSheet();
  CString        s = "";
  short          nLen = SS_GetCellTag(lpSS, m_lCol, m_lRow, NULL);

  if (nLen)
  {
    TBGLOBALHANDLE ghCellTag = tbGlobalAlloc(GHND, (nLen+1)*sizeof(TCHAR));
    LPTSTR         lpszCellTag = (LPTSTR)tbGlobalLock(ghCellTag);

    SS_GetCellTag(lpSS, m_lCol, m_lRow, lpszCellTag);
    s = lpszCellTag;
    tbGlobalUnlock(ghCellTag);
    tbGlobalFree(ghCellTag);

    UnlockSheet();

    return s.AllocSysString();
  }
  else
    return NULL;
}

//--------------------------------------------------------------------

void CSsocxCtrl::SetCellTag(LPCTSTR CellTag)
{
  LPSPREADSHEET lpSS = LockSheet();

  if( m_bBlockMode )
    SS_SetCellTagRange(lpSS, m_lCol, m_lRow, m_lCol2, m_lRow2, CellTag);
  else
    SS_SetCellTagRange(lpSS, m_lCol, m_lRow, m_lCol, m_lRow, CellTag);

  UnlockSheet();

  SetModifiedFlag();
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetEventEnabled(short nEvent) 
{
  GET_NODESIGN();
  BOOL fIsEnabled = FALSE;

  if (0 == nEvent)
  {
    short i;
    for (i = 1, fIsEnabled = TRUE; fIsEnabled && i < EVENTENABLED_CNT; i++)
      fIsEnabled = m_bEventEnabled[i];
  }
  else if( nEvent < EVENTENABLED_CNT )
    fIsEnabled = m_bEventEnabled[nEvent];

  return (fIsEnabled);
}

//--------------------------------------------------------------------

BOOL CSsocxCtrl::GetAllowEditOverflow()
{
  LPSPREADSHEET lpSS = LockSheet();
  BOOL bValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWEDITOVERFLOW);
  UnlockSheet();
  return bValue;
}

void CSsocxCtrl::SetAllowEditOverflow(BOOL bNewValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWEDITOVERFLOW, bNewValue);
  SetModifiedFlag();
  UnlockSheet();
}

#endif // SS_V40

#ifdef SS_V70
short CSsocxCtrl::GetSheetCount(void)
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetSheetCount(lpBook);
  UnlockBook();
  return nValue;
}


void CSsocxCtrl::SetSheetCount(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetSheetCount(lpBook, newValue);

  if (newValue == 0)
    SS_SetSheetCount(lpBook, 1);

  SetModifiedFlag();
  UnlockBook();
}


short CSsocxCtrl::GetActiveSheet(void)
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetActiveSheet(lpBook);
  nValue = nValue == -1 ? -1 : nValue + 1;

  UnlockBook();
  return nValue;
}


void CSsocxCtrl::SetActiveSheet(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
  newValue = newValue == -1 ? -1 : newValue - 1;
  SS_SetActiveSheet(lpBook, newValue);
  SetModifiedFlag();
  UnlockBook();
}


short CSsocxCtrl::GetTabStripLeftSheet(void)
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetTabStripLeftSheet(lpBook) + 1;
  UnlockBook();
  return nValue;
}


void CSsocxCtrl::SetTabStripLeftSheet(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetTabStripLeftSheet(lpBook, newValue - 1);
  SetModifiedFlag();
  UnlockBook();
}


LPFONTDISP CSsocxCtrl::GetTabStripFont(void)
{
  LPSS_BOOK lpBook = LockBook();
  SS_FONTID idFont;
  SS_FONT   font;

  idFont = lpBook->TabStripFontId ? lpBook->TabStripFontId : lpBook->DefaultFontId;

  SS_GetFont(lpBook, &font, idFont);
  CreateFont(&m_fontTab, &font.LogFont);
  UnlockBook();

  return m_fontTab.GetFontDispatch();
}

void CSsocxCtrl::SetTabStripFont(LPFONTDISP newValue)
{
  LPSS_BOOK lpBook = LockBook();
  ASSERT(newValue == NULL ||
         AfxIsValidAddress(newValue, sizeof(IDispatch), FALSE));
  m_fontTab.InitializeFont(NULL, newValue);
  SS_SetTabStripFont(lpBook, m_fontTab.GetFontHandle());

  SetModifiedFlag();
  UnlockBook();
}

//--------------------------------------------------------------------

void CSsocxCtrl::OnTabFontChanged()
{
  if (!m_fInsideCreateControlWindow)
  {
    LPSS_BOOK lpBook = LockBook();
    SS_SetTabStripFont(lpBook, m_fontTab.GetFontHandle());
    UnlockBook();
    SetModifiedFlag();
  }
}


BSTR CSsocxCtrl::GetSheetName(void)
{
  LPSS_BOOK lpBook = LockBook();
  CString sValue = "";
  TBGLOBALHANDLE hSheetName;

  hSheetName = SS_GetSheetName(lpBook, lpBook->nSheet);

  if (hSheetName)  
  {
    sValue = (LPCTSTR)tbGlobalLock(hSheetName);
    tbGlobalUnlock(hSheetName);
  }

  UnlockBook();
  return sValue.AllocSysString();
}


void CSsocxCtrl::SetSheetName(LPCTSTR newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetSheetName(lpBook, lpBook->nSheet, newValue);
  SetModifiedFlag();
  UnlockBook();
}


short CSsocxCtrl::GetTabStripPolicy(void)
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetTabStripPolicy(lpBook);
  UnlockBook();
  return nValue;
}


void CSsocxCtrl::SetTabStripPolicy(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetTabStripPolicy(lpBook, newValue);
  SetModifiedFlag();
  UnlockBook();
}

#ifdef SS_V80
short CSsocxCtrl::GetTabStripButtonPolicy(void)
{
  LPSS_BOOK lpBook = LockBook();
  short nValue = SS_GetTabStripButtonPolicy(lpBook);
  UnlockBook();
  return nValue;
}


void CSsocxCtrl::SetTabStripButtonPolicy(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetTabStripButtonPolicy(lpBook, newValue);
  SetModifiedFlag();
  UnlockBook();
}
#endif

double CSsocxCtrl::GetTabStripRatio(void)
{
  LPSS_BOOK lpBook = LockBook();
  double dfValue = SS_GetTabStripRatio(lpBook);
  UnlockBook();
  return dfValue;
}


void CSsocxCtrl::SetTabStripRatio(double newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetTabStripRatio(lpBook, newValue);
  SetModifiedFlag();
  UnlockBook();
}


double CSsocxCtrl::GetPrintZoomFactor(void)
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  double dfValue = lpSS->PrintOptions.pf.ZoomFactor;
  UnlockSheet();
  return dfValue;
}


void CSsocxCtrl::SetPrintZoomFactor(double newValue)
{
  if (newValue > 0 && newValue <= 5)
  {
    LPSPREADSHEET lpSS = LockSheet();
    lpSS->PrintOptions.pf.ZoomFactor = newValue;
    SetModifiedFlag();
    UnlockSheet();
  }
}


short CSsocxCtrl::GetScrollBarWidth(void)
{
  LPSS_BOOK lpBook = LockBook();
  short nValue;
  SS_GetScrollBarSize(lpBook, &nValue, NULL);
  UnlockBook();
  return nValue;
}


void CSsocxCtrl::SetScrollBarWidth(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetScrollBarSize(lpBook, newValue, 0);
  SetModifiedFlag();
  UnlockBook();
}


short CSsocxCtrl::GetScrollBarHeight(void)
{
  LPSS_BOOK lpBook = LockBook();
  short nValue;
  SS_GetScrollBarSize(lpBook, NULL, &nValue);
  UnlockBook();
  return nValue;
}


void CSsocxCtrl::SetScrollBarHeight(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetScrollBarSize(lpBook, 0, newValue);
  SetModifiedFlag();
  UnlockBook();
}


short CSsocxCtrl::GetActiveCellHighlightStyle(void)
{
  LPSPREADSHEET lpSS = LockSheet();
  short nValue;
  nValue = SS_GetActiveCellHighlightStyle(lpSS);
  UnlockSheet();
  return nValue;
}


void CSsocxCtrl::SetActiveCellHighlightStyle(short newValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetActiveCellHighlightStyle(lpSS, newValue);
  SetModifiedFlag();
  UnlockSheet();
}


short CSsocxCtrl::GetPrintScalingMethod(void)
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = lpSS->PrintOptions.pf.wScalingMethod;
  UnlockSheet();
  return nValue;
}


void CSsocxCtrl::SetPrintScalingMethod(short newValue)
{
  if (newValue >= 0 && newValue <= 3)
  {
    LPSPREADSHEET lpSS = LockSheet();
    lpSS->PrintOptions.pf.wScalingMethod = newValue;
    SetModifiedFlag();
    UnlockSheet();
  }
}


short CSsocxCtrl::GetPrintBestFitPagesWide(void)
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = lpSS->PrintOptions.pf.nBestFitPagesWide;
  UnlockSheet();
  return nValue;
}


void CSsocxCtrl::SetPrintBestFitPagesWide(short newValue)
{
  /* RFW - 6/20/06 - 19014
  if (newValue > 0 && newValue <= 100)
  */
  if (newValue > 0 && newValue <= 999)
  {
    LPSPREADSHEET lpSS = LockSheet();
    lpSS->PrintOptions.pf.nBestFitPagesWide = newValue;
    SetModifiedFlag();
    UnlockSheet();
  }
}


short CSsocxCtrl::GetPrintBestFitPagesTall(void)
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  short nValue = lpSS->PrintOptions.pf.nBestFitPagesTall;
  UnlockSheet();
  return nValue;
}


void CSsocxCtrl::SetPrintBestFitPagesTall(short newValue)
{
  /* RFW - 6/20/06 - 19014
  if (newValue > 0 && newValue <= 100)
  */
  /* RFW - 2/28/07 - 19951
  if (newValue > 0 && newValue <= 999)
  */
  if (newValue > 0 && newValue <= 10000)
  {
    LPSPREADSHEET lpSS = LockSheet();
    lpSS->PrintOptions.pf.nBestFitPagesTall = newValue;
    SetModifiedFlag();
    UnlockSheet();
  }
}


BOOL CSsocxCtrl::GetPrintCenterOnPageH(void)
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fCenterOnPageH;
  UnlockSheet();
  return fValue;
}


void CSsocxCtrl::SetPrintCenterOnPageH(BOOL newValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fCenterOnPageH = newValue;
  SetModifiedFlag();
  UnlockSheet();
}


BOOL CSsocxCtrl::GetPrintCenterOnPageV(void)
{
  GET_NODESIGN();
  LPSPREADSHEET lpSS = LockSheet();
  BOOL fValue = lpSS->PrintOptions.pf.fCenterOnPageV;
  UnlockSheet();
  return fValue;
}


void CSsocxCtrl::SetPrintCenterOnPageV(BOOL newValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  lpSS->PrintOptions.pf.fCenterOnPageV = newValue;
  SetModifiedFlag();
  UnlockSheet();
}


BOOL CSsocxCtrl::GetAllowColMove(void)
{
  LPSPREADSHEET lpSS = LockSheet();
  short nValue;
  nValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWCOLMOVE);
  UnlockSheet();
  return nValue;
}


void CSsocxCtrl::SetAllowColMove(BOOL newValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWCOLMOVE, newValue);
  SetModifiedFlag();
  UnlockSheet();
}


BOOL CSsocxCtrl::GetAllowRowMove(void)
{
  LPSPREADSHEET lpSS = LockSheet();
  short nValue;
  nValue = SS_GetBool(lpSS->lpBook, lpSS, SSB_ALLOWROWMOVE);
  UnlockSheet();
  return nValue;
}


void CSsocxCtrl::SetAllowRowMove(BOOL newValue)
{
  LPSPREADSHEET lpSS = LockSheet();
  SS_SetBool(lpSS->lpBook, lpSS, SSB_ALLOWROWMOVE, newValue);
  SetModifiedFlag();
  UnlockSheet();
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypeScientificMin(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_SCIENTIFIC))
    Value = cellType.Spec.Scientific.Min;
  return Value;
}

void CSsocxCtrl::SetTypeScientificMin(double newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_SCIENTIFIC) )
  {
    if (newValue > cellType.Spec.Scientific.Max)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      m_TypeScientific.Spec.Scientific.Min = newValue;
      SetType(dispidTypeScientificMin);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

double CSsocxCtrl::GetTypeScientificMax(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  double Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_SCIENTIFIC))
    Value = (double)cellType.Spec.Scientific.Max;
  return Value;
}

void CSsocxCtrl::SetTypeScientificMax(double newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_SCIENTIFIC) )
  {
    if (newValue < cellType.Spec.Scientific.Min)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      m_TypeScientific.Spec.Scientific.Max = newValue;
      SetType(dispidTypeScientificMax);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

BSTR CSsocxCtrl::GetTypeScientificDecimal(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if (GetTypeOf(&cellType, SS_TYPE_SCIENTIFIC))
    sValue = cellType.Spec.Scientific.szDecimal;
  return sValue.AllocSysString();
}

void CSsocxCtrl::SetTypeScientificDecimal(LPCTSTR newValue)
{
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_SCIENTIFIC) )
  {
    if (newValue && _tcsclen(newValue) > 1)
      ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
    else
    {
      lstrcpyn(m_TypeScientific.Spec.Scientific.szDecimal, newValue ? newValue : _T(""), 4);
      SetType(dispidTypeScientificDecimal);
      SetModifiedFlag();
    }
  }
}

//--------------------------------------------------------------------

short CSsocxCtrl::GetTypeScientificDecPlaces(void)
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  short Value = 0;
  if (GetTypeOf(&cellType, SS_TYPE_SCIENTIFIC))
    Value = (short)cellType.Spec.Scientific.Right;
  return Value;
}

void CSsocxCtrl::SetTypeScientificDecPlaces(short newValue)
{
  if (newValue <0 || newValue > 14)
	ThrowError(CTL_E_INVALIDPROPERTYVALUE, AFX_IDP_E_INVALIDPROPERTYVALUE);
  //---------------------------------------------------
  else
  {
    m_TypeScientific.Spec.Scientific.Right = newValue;
    SetType(dispidTypeScientificDecPlaces);
    SetModifiedFlag();
  }
}


OLE_COLOR CSsocxCtrl::GetScrollBarHColor(void)
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF clrValue;
  SS_GetScrollBarColor(lpBook, NULL, &clrValue);
  UnlockBook();
  return clrValue;
}


void CSsocxCtrl::SetScrollBarHColor(OLE_COLOR newValue)
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF clrV;
  SS_GetScrollBarColor(lpBook, &clrV, NULL);
  SS_SetScrollBarColor(lpBook, clrV, newValue);
  SetModifiedFlag();
  UnlockBook();
}


OLE_COLOR CSsocxCtrl::GetScrollBarVColor(void)
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF clrValue;
  SS_GetScrollBarColor(lpBook, &clrValue, NULL);
  UnlockBook();
  return clrValue;
}


void CSsocxCtrl::SetScrollBarVColor(OLE_COLOR newValue)
{
  LPSS_BOOK lpBook = LockBook();
  COLORREF clrH;
  SS_GetScrollBarColor(lpBook, NULL, &clrH);
  SS_SetScrollBarColor(lpBook, newValue, clrH);
  SetModifiedFlag();
  UnlockBook();
}


short CSsocxCtrl::GetSheet(void)
{
  LPSS_BOOK lpBook = LockBook();
  short nSheet;
  nSheet = SS_GetSheet(lpBook);

  UnlockBook();
  return nSheet;
}


void CSsocxCtrl::SetSheet(short newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetSheet(lpBook, newValue);

  if( AmbientUserMode() == FALSE )
  {
    newValue = newValue == -1 ? -1 : newValue - 1;
    SS_SetActiveSheet(lpBook, newValue);
  }

  SetModifiedFlag();
  UnlockBook();
}


short CSsocxCtrl::GetSheetSendingEvent(void)
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  short nSheetSendingEvent;
  nSheetSendingEvent = SS_GetSheetSendingMsg(lpBook);

  UnlockBook();
  return nSheetSendingEvent;
}


BOOL CSsocxCtrl::GetSheetVisible(void)
{
  GET_NODESIGN();
  LPSS_BOOK lpBook = LockBook();
  BOOL fValue = SS_GetSheetVisible(lpBook, lpBook->nSheet);
  UnlockBook();
  return fValue;
}


void CSsocxCtrl::SetSheetVisible(BOOL newValue)
{
  LPSS_BOOK lpBook = LockBook();
  SS_SetSheetVisible(lpBook, lpBook->nSheet, newValue);
  SetModifiedFlag();
  UnlockBook();
}

#endif // SS_V70

#if SS_V80

BSTR CSsocxCtrl::GetTypeCustomName() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  CString sValue;
  if( GetTypeOf(&cellType, SS_TYPE_CUSTOM) )
  {
    if( cellType.Spec.Custom.hName )
    {
      sValue = (LPTSTR)GlobalLock(cellType.Spec.Custom.hName);
      GlobalUnlock(cellType.Spec.Custom.hName);
    }
  }
  return sValue.AllocSysString();
}
                     
//--------------------------------------------------------------------

void CSsocxCtrl::SetTypeCustomName(LPCTSTR lpszNewValue) 
{
  #ifndef WIN32
  if( StrHugeLen(lpszNewValue) > INT_MAX )
    return;
  #endif
  m_sTypeCustomName = lpszNewValue;
  SetType(dispidTypeCustomName);
  SetModifiedFlag();
}

long CSsocxCtrl::GetTypeCustomStyle() 
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  if( GetTypeOf(&cellType, SS_TYPE_CUSTOM) )
     return cellType.Style;
  return 0;
}
                     
//--------------------------------------------------------------------

void CSsocxCtrl::SetTypeCustomStyle(long newValue) 
{
  m_lTypeCustomStyle = newValue;
  SetType(dispidTypeCustomStyle);
  SetModifiedFlag();
}

VARIANT CSsocxCtrl::GetTypeCustomItemData()
{
  GET_NODESIGN();
  SS_CELLTYPE cellType;
  VARIANT Ret;
  VariantInit(&Ret);
  if( GetTypeOf(&cellType, SS_TYPE_CUSTOM) )
     CT_ValueToVariant(&Ret, &cellType.Spec.Custom.ItemData);
  return Ret;
}

void CSsocxCtrl::SetTypeCustomItemData(VARIANT *newValue)
{
  MemHugeCpy(&m_TypeCustomItemData, newValue, sizeof(VARIANT));
  SetType(dispidTypeCustomItemData);
  SetModifiedFlag();
}

#endif // SS_V80