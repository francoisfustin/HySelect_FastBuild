/***********************************************************************
* FPDBSIMP.CPP
*
* Copyright (C) 1991-1997 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
*
************************************************************************
*
* Implementation of the CfpOleSimpleBound class.
*
* FarPoint's implementation of OLE's "Simple" DB Binding
* for OLE controls.
*
* This class is intented to be a member variable (public) of
* an OCX control (subclassed from COleControl).
***********************************************************************/
#ifndef NO_MFC
#include <afxctl.h>         // MFC support for OLE Custom Controls
#else  //NO_MFC
#include "stdafx.h"
#include "fpatl.h"
#endif

//#include "stdafx.h"
#include "vbinterf.h"                                                                                                                          

#include "fpconvrt.h"
#include "fptstr.h"

#include "fpdbutil.h"		  
#include "fpdbsimp.h"

#include <limits.h>
#include <malloc.h>

#define DB_VT_SAFEARRAY   0x2011

// This will get translated to VBM_DATA_INITIATE
#define DATA_UINIT_CONNECT       300

#ifndef NO_MFC  // NO_MFC
#define BOUNDPROP_REQUESTEDIT  BoundPropertyRequestEdit
#define BOUNDPROP_CHANGED      BoundPropertyChanged
#else
#define BOUNDPROP_REQUESTEDIT  fpAtlBoundPropertyRequestEdit
#define BOUNDPROP_CHANGED      fpAtlBoundPropertyChanged
#endif

static BOOL fpCopyVariant( LPVARIANT lpvarDest, LPVARIANT lpvarSrc,
  BOOL fFreeOldDest, BOOL fDontReallocBSTR)
{
  if (!lpvarDest || !lpvarSrc)
    return FALSE;

  // If we currently have a value, and it's a BSTR, free it.
  if (fFreeOldDest && (lpvarDest->vt == VT_BSTR) && (lpvarDest->bstrVal))
    SysFreeString(lpvarDest->bstrVal);

  // copy variant
  _fmemcpy(lpvarDest, lpvarSrc, sizeof(VARIANT));

  // Allocate new BSTR if it's of that type.
  if ((lpvarSrc->vt == VT_BSTR) && (lpvarSrc->bstrVal) && !fDontReallocBSTR)
    lpvarDest->bstrVal = SysAllocString(lpvarSrc->bstrVal);

  return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::CfpOleSimpleBound()
 *
 * DESCRIPTION:
 *
 *   Constructor for CfpOleSimpleBound.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpOleSimpleBound::CfpOleSimpleBound()
{

  m_lpOwner = NULL;            // Pointer to OCX class (owner)    
  m_lpfuncDataMsg = NULL;
  m_dispid = -1;               // invalid value

  m_ulCtlFlags = (ULONG)0;

  m_fValSet = FALSE;          // whether the value was set
  _fmemset(&m_varValSet, 0, sizeof(m_varValSet)); // the last value from fpSetBoundVal().

  m_fCurrGet = FALSE;
  m_lpvarCurrGet = NULL;

  m_fOnlyBSTR = FALSE;

  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::~CfpOleSimpleBound()
 *
 * DESCRIPTION:
 *
 *   Destructor for CfpOleSimpleBound.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpOleSimpleBound::~CfpOleSimpleBound()
{
  if (m_fValSet && m_varValSet.vt == VT_BSTR && m_varValSet.bstrVal != NULL)
    SysFreeString(m_varValSet.bstrVal);

  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::SetOwner()
 *
 * DESCRIPTION:
 *
 *   Save ptr back to the Owner object (subclassed from COleControl).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleSimpleBound::SetOwner( LPVOID lpOleControl)
{
  *(LPVOID FAR *)&m_lpOwner = lpOleControl;
  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::SetDataSourceDispid()
 *
 * DESCRIPTION:
 *
 *   Save the dispid associated with the DataSource property.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleSimpleBound::SetBoundValDispid( DISPID dispid)
{
  m_dispid = dispid;
  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::SetDataMsgFunc()
 *
 * DESCRIPTION:
 *
 *   Save the ptr to the function called to invoke a VBM_DATA msg.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleSimpleBound::SetDataMsgFunc( LPVOID lpfuncDataMsg)
{
  m_lpfuncDataMsg = 
    (LRESULT (FAR *)(LPVOID lpObject, 
       UINT uMsg, WPARAM wParam, LPARAM lParam))(lpfuncDataMsg);

}



/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::fpGetBoundVal()
 *
 * DESCRIPTION:
 *
 *   
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
FP_PROPRETTYPE CfpOleSimpleBound::fpGetBoundVal(LPVARIANT lpVar)
{
  // If we have a Callback function, call it with VBM_DATA_REQUEST.
  if (m_lpfuncDataMsg)
  {
    long lResult;
    DATAACCESS da = {0};

    m_fCurrGet = FALSE;       // set to TRUE if control changed the value.
    m_lpvarCurrGet = lpVar;   // save temporarly

    da.sAction = DATA_UPDATE;
    da.lpfpOleBinding = this;
    lResult = m_lpfuncDataMsg( (LPVOID)m_lpOwner, VBM_DATA_REQUEST,
      (WPARAM)0, (LPARAM)(LPVOID)&da);

    // if control did not change the value, use value from last SET.
    if (!m_fCurrGet)           
      fpCopyVariant(lpVar, &m_varValSet, FALSE, FALSE);

    m_lpvarCurrGet = NULL;    // reset.
  }

  return FP_PROPRETVAL(TRUE, S_OK);
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::fpGetBoundVal()
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
FP_PROPRETTYPE CfpOleSimpleBound::fpGetBoundVal(BSTR FAR *lpbstr)
{
  VARIANTARG varData1, varData2;
  HRESULT h;

  *lpbstr = NULL;
  VariantInit(&varData1);

  // Call same function but with a VARIANT parameter.
  fpGetBoundVal(&varData1);
  
  // if returned a VT_BSTR, use it.
  if (varData1.vt == VT_BSTR)
  {
    *lpbstr = varData1.bstrVal;
  }
  else // else, convert it to a BSTR.
  {
    VariantInit(&varData2);
    h = VariantChangeType(&varData2, &varData1, (USHORT)0, VT_BSTR);
    if (h == S_OK && varData2.vt == VT_BSTR)
      *lpbstr = varData2.bstrVal;
  }

  return FP_PROPRETVAL(TRUE, S_OK);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::fpSetBoundVal()
 *
 * DESCRIPTION:
 *
 *   
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
FP_PROPRETTYPE CfpOleSimpleBound::fpSetBoundVal(LPVARIANT lpVar)
{
  BOOL fOldValSet = m_fValSet;

  // copy variant
  fpCopyVariant(&m_varValSet, lpVar, m_fValSet, m_fOnlyBSTR);

  m_fValSet = TRUE;

  // If we have a Callback function, call it with VBM_DATA_AVAILABLE.
  if (m_lpfuncDataMsg)
  {
    long lResult;
    DATAACCESS da = {0};

    // If fOldValSet is False, then this is the first time thru, set sAction
    // to REFRESH, else use MOVENEXT.
    da.sAction = (!fOldValSet ? DATA_REFRESH : DATA_MOVENEXT);
    da.lpfpOleBinding = this;
    lResult = m_lpfuncDataMsg( (LPVOID)m_lpOwner, VBM_DATA_AVAILABLE,
      (WPARAM)0, (LPARAM)(LPVOID)&da);
  }

  return FP_PROPRETVAL(TRUE, S_OK);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::fpSetBoundVal()
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
FP_PROPRETTYPE CfpOleSimpleBound::fpSetBoundVal(LPCTSTR lpctstr)
{
  VARIANT va;

  m_fOnlyBSTR = TRUE;

  VariantInit(&va);
  va.vt = VT_BSTR;

  if (lpctstr == NULL)
    va.bstrVal = NULL;
  else
  {
    va.bstrVal = (BSTR)fpSysAllocBSTRFromTString((LPTSTR)lpctstr);
    if (va.bstrVal == NULL)
    {
#if !defined(NO_MFC) && !defined(SS40) // MFC
      m_lpOwner->ThrowError(CTL_E_OUTOFMEMORY);
      return FALSE;
#else  // NO_MFC
      return CTL_E_OUTOFMEMORY;
#endif
    }
  }

  // call function with VARIANT
  return fpSetBoundVal(&va);
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::DataCall()
 *
 * DESCRIPTION:
 *
 *   Interface to Data function.  Simulates VB3's VBM_DATA msgs.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
short CfpOleSimpleBound::DataCall( LPVOID lpvda, USHORT msg, 
                               USHORT action, LPVOID lpvUserData)
{
  short sReturn = 0;

  // If NOT getting/setting ControlFlag, and VB never called our SET function,
  // then we are not bound, return error.
  if (action != DATA_UCTLFLAGS && !m_fValSet)
    return (short)-1;
    
  switch (msg)
  {
    case VBM_DATA_GET:
      sReturn = ProcessData_Get( lpvda, action, lpvUserData);
      break;

    case VBM_DATA_SET:
      sReturn = ProcessData_Set( lpvda, action, lpvUserData);
      break;

    // Any other types are not supported!!!
    case VBM_DATA_METHOD:   // Simple Binding does not support METHOD.
    default:
      ASSERT(0);                   // force Assertion error.
      sReturn = (short)-1;
  }

  return sReturn;
}


/***************************************************************************
 *
 * FUNCTION:     CfpOleSimpleBound::ProcessData_Get()
 *
 * DESCRIPTION:
 *
 *   Process VB's VBM_DATA_GET messages.  Get bookmark, field info,
 *   field value, or record count.
 *
 *   Normally, this function is only called by CfpOleSimpleBound::DataCall().
 *
 * PARAMETERS:
 *
 *   lpvda ... Far ptr to DATAACCESS structure.
 *
 *   usAction	     Description
 *   --------        -----------
 *   DATA_FIELDATTRIBUTES
 *                   Get attribute for the field. The returned lData value
 *                   can be 0 or any combination of the following values:
 *    > DB_UPDATABLEFIELD - value in the field can be changed.
 *  
 *   DATA_FIELDCHUNK   Get a chunk of data as specified by the ulChunkOffset
 *                     and ulChunkNumBytes. Returns an HLSTR.
 *   DATA_FIELDNAME    Get the field name. Returns an HSZ.
 *   DATA_FIELDPOSITION	 Returns the ordinal position of the of the field in the
 *                     Fields collection. Returns a DT_SHORT.
 *   DATA_FIELDSIZE	   Get the field size in bytes. Returns a DT_LONG.
 *   DATA_FIELDTYPE    Get the field data type. The returned lData value is:
 *    > VT_DATA_BOOL - True/False
 *    > VT_DATA_VAR_BTYE - Byte
 *    > VT_DATA_INTEGER - Integer
 *    > VT_DATA_LONG - Long
 *    > VT_DATA_CURRENCY - Currency
 *    > VT_DATA_SINGLE - Single
 *    > VT_DATA_DOUBLE - Double
 *    > VT_DATA_DATETIME - Date/Time
 *    > VT_DATA_TEXT - Text
 *    > VT_DATA_BINARY - Long Binary
 *    > VT_DATA_MEMO - Memo
 *                     Returns a DT_SHORT.
 *  
 *   DATA_FIELDVALUE   Get the field value. Returns a value that is coerced, 
 *                     if possible, into the data type specified by usDataType.
 *   DATA_UCTLFLAGS    Equivalent to VBSetControlFlags(). Return flags in lData.
 *
 * RETURNS:  0 if successful,  non-0 if error
 *
 **************************************************************************/
short CfpOleSimpleBound::ProcessData_Get( LPVOID lpvda, USHORT usAction,
  LPVOID lpvUserData)
{
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
  short sReturn = 0;

  switch (usAction)
  {
    case DATA_FIELDCHUNK:
    case DATA_FIELDVALUE:
      if (!GetDataValue(FALSE, lpda)) // read field, if FALSE...
         sReturn = (short)-1;                // return error
      break;

    case DATA_FIELDSIZE:
      if (!GetDataValue(TRUE, lpda))  // read field's length, if FALSE
         sReturn = (short)-1;                // return error
      else
         lpda->usDataType = DT_LONG;         // NOTE: Len is in lpda->lData
      break;

    case DATA_FIELDPOSITION:      // Return FieldIndex (0..Count-1)
    case DATA_FIELDTYPE:
      lpda->usDataType = DT_SHORT;
      if (usAction == DATA_FIELDTYPE)
        lpda->lData = (LONG)GetVBDataType();
      else
        lpda->lData = 0L;
      break;
#ifndef SS40 // excluded from Spread 4:
    case DATA_FIELDATTRIBUTES:
      lpda->usDataType = DT_LONG;
      lpda->lData = (LONG)0x20; // DB_UPDATABLEFIELD
      if (!m_lpOwner->BOUNDPROP_REQUESTEDIT(m_dispid))
        lpda->lData = 0L;
      break;
#endif

	case DATA_UDATAFIELD:             // Get the Bound column
    case DATA_FIELDNAME:
      lpda->usDataType = DT_HSZ;
      lpda->lData = (LONG)fpDBCreateHsz((LPTSTR)_T("DataField"));
      break;

    case DATA_UCTLFLAGS:
      lpda->lData = (LONG)m_ulCtlFlags;
      break;

    default:
      // Action not implemented.  IF YOU NEED IT, ADD IT!!!
      ASSERT(0);
      break;
  } //switch (action)

  return sReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::ProcessData_Set()
 *
 * DESCRIPTION:
 *
 *   Process VB's VBM_DATA_SET messages.  Sets field value.
 *
 *   Normally, this function is only called by CfpOleSimpleBound::DataCall().
 *
 * PARAMETERS:
 *
 *   lpvda ... Far ptr to DATAACCESS structure.
 *
 *   usAction	      Description
 *   --------         -----------
 *   DATA_FIELDVALUE  Set the field value. The usDataType member of the
 *                    DATAACCESS structure specifies the data type of lData. 
 *                    Equivalent to Data1.recordset.Fields(fieldname).Value.
 *   DATA_FIELDCHUNK  Set the field value using a chunk of data. This
 *                    requires creating an HLSTR that contains the data
 *                    and passing it as the lData value. Equivalent to 
 *                    Data1.recordset.Fields(fieldname).AppendChunk.
 *   DATA_UCTLFLAGS   Equivalent to VBSetControlFlags(), sets flags.
 *
 * RETURNS:  0 if successful,  non-0 if error
 *
 **************************************************************************/
short CfpOleSimpleBound::ProcessData_Set( LPVOID lpvda, USHORT usAction,
  LPVOID lpvUserData)
{
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
  short sReturn = 0;
  ULONG ulTmpFlags;

  // reset any errors
  fpResetError(lpvUserData);

  switch (usAction)
  {
	case DATA_FIELDCHUNK:
	case DATA_FIELDVALUE:
      if (!SetDataValue(lpvda))
         sReturn = (short)-1;
	  break;

    case DATA_UCTLFLAGS:
      // if setting DATACHANGED but BOUNDDATASET is currently set, ignore
      // the bit.
      if ((m_ulCtlFlags & CTLFLG_BOUNDDATASET) &&
          ((ULONG)lpda->lData & CTLFLG_DATACHANGED) &&
          !(m_ulCtlFlags & CTLFLG_DATACHANGED))
         ulTmpFlags = (ULONG)(lpda->lData & ~CTLFLG_DATACHANGED);
      else
         ulTmpFlags = (ULONG)lpda->lData;
      // if flag includes setting "DATACHANGED" and not already set, 
      // call function to notify VB that property value changed.
// LYW320...
//      if ((ulTmpFlags & CTLFLG_DATACHANGED) && 
//          !(m_ulCtlFlags & CTLFLG_DATACHANGED) && m_fValSet)
// force DataChanged every time, due to fact that we are not synched 
//   with VB
      if ((ulTmpFlags & CTLFLG_DATACHANGED) && m_fValSet)
// LYW320
      {
#ifndef SS40 // excluded from Spread 4:
        if (m_lpOwner->BOUNDPROP_REQUESTEDIT(m_dispid))
          m_lpOwner->BOUNDPROP_CHANGED(m_dispid);
        else  // ERROR requesting to change data
        { 
          fpSetError(FPDB_ERR_UPDATE, -1L, NULL);
          sReturn = (SHORT)-1;
        }
#endif
      }
      // Set flag (ignoring any errors).
      m_ulCtlFlags = ulTmpFlags;
      break;

      default:
        ASSERT(0);  // not supported
  } // switch

  // if an error occurred, invoke the EditError event
  if (sReturn != (SHORT)0)
     fpInvokeError();

  fpResetError(NULL);

  return sReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::GetVBDataType()
 *
 * DESCRIPTION:
 *
 *   VB Data types:
 *   --------------
 *   DATA_VT_BOOL
 *   DATA_VT_VAR_BYTE
 *   DATA_VT_INTEGER
 *   DATA_VT_LONG
 *   DATA_VT_CURRENCY
 *   DATA_VT_SINGLE
 *   DATA_VT_DOUBLE
 *   DATA_VT_DATETIME
 *   DATA_VT_TEXT
 *   DATA_VT_BINARY
 *   DATA_VT_MEMO
 *
 * PARAMETERS:
 *
 * RETURNS:  VB Data type (USHORT)
 *
 **************************************************************************/
USHORT CfpOleSimpleBound::GetVBDataType(void)  // used member var: m_varValSet
{
  USHORT retType = (WORD)DATA_VT_TEXT;

  if (!m_fValSet)
    return retType;

  switch (m_varValSet.vt)
  {

    case VT_I2:     /* 2   */
    case VT_UI2:    /* 504 */  
      retType = DATA_VT_INTEGER;
      break; 

    case VT_I4:     /* 3   */
    case VT_UI4:    /* 505 */  
      retType = DATA_VT_LONG;
      break; 

    case VT_R4:     /* 4   */  retType = DATA_VT_SINGLE;   break; 
    case VT_R8:     /* 5   */  retType = DATA_VT_DOUBLE;   break; 
    case VT_CY:     /* 6   */  retType = DATA_VT_CURRENCY; break; 
    case VT_DATE:   /* 7   */  retType = DATA_VT_DATETIME; break; 
    case VT_BOOL:   /* 11  */  retType = DATA_VT_BOOL;     break; 

    case VT_BSTR:       /* 8   */ 
      // if size of greater than 255, assume MEMO.
      if (m_varValSet.bstrVal && SysStringLen(m_varValSet.bstrVal) > (UINT)255)
        retType = DATA_VT_MEMO;
      else
        retType = DATA_VT_TEXT;
      break;

    case VT_BLOB:   /* 65 */
      retType = DATA_VT_BINARY;   break; 

	case VT_EMPTY:       // 0  
	case VT_NULL:        // 1
      retType = DATA_VT_TEXT;     break;  // assume Text field

    // Types not yet translated or not supported:
//  case VT_I8:          // 20
//	case VT_HRESULT:     // 25
//	case VT_FILETIME:    // 64
//	case VT_UUID:        // 72
//	case VT_UI8:         // 506
//	case VT_COLUMNID:    // 507
//	case VT_ANYVARIANT:  // 51
    default:
      ASSERT(0);  // Unknown type.
  }

  return retType;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleSimpleBound::GetDataValue()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: (ULONG)
 *
 *   0 ............. if no data (or null)
 *   Len of data ... If successful (does not include null terminator)
 *   FFFFFFFF ...... If error (-1) 
 *
 **************************************************************************/
BOOL CfpOleSimpleBound::GetDataValue( BOOL fGetLenOnly, LPVOID lpvda)
{
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
  LONG FAR *lplData = &(lpda->lData);
  USHORT usVBDataType = lpda->usDataType;
  ULONG ulChunkOffset = lpda->ulChunkOffset;
  ULONG ulChunkSize = lpda->ulChunkNumBytes;
  FSHORT FAR *lpuFlags = &(lpda->fs);

  DWORD dwVarLen = (DWORD)-1;
  ULONG ulReturn = (ULONG)-1;
  HRESULT h;
  UINT  uSrcCharSize;
  UINT  uDestCharSize;
  ULONG ulAjustedOffset;
  ULONG ulAjustedSize;
  LBLOCK tmpLBlock = NULL;
  VARTYPE vtDataType;
  LPSTR  lpszData ;

  *lplData = 0L;

  vtDataType = m_varValSet.vt;

  // if the field value is null, set flag and return
  if ((m_varValSet.vt == VT_EMPTY) || (m_varValSet.vt == VT_NULL))
  {
    *lpuFlags = DA_fNull;
    return TRUE;
  }

  // if stuck with only BSTR support...
  // if the field value is null, set flag and return
  if (m_fOnlyBSTR && (m_varValSet.vt == VT_BSTR) &&
      (m_varValSet.bstrVal == NULL || SysStringLen(m_varValSet.bstrVal) == 0))
  {
    *lpuFlags = DA_fNull;
    return TRUE;
  }

  uSrcCharSize = (ULONG)sizeof(BYTE);

  switch (vtDataType)
  {
    case VT_BSTR:
      if (!m_varValSet.bstrVal)
        return FALSE;

      lpszData = (LPSTR)m_varValSet.bstrVal;
      dwVarLen = SysStringByteLen(m_varValSet.bstrVal);
      uSrcCharSize = (ULONG)sizeof(OLECHAR);

      ulAjustedOffset = ulChunkOffset * uSrcCharSize;
      ulAjustedSize = ulChunkSize * uSrcCharSize;
      // adjust for offset & size
      if (ulAjustedOffset)
      {
        if (ulAjustedOffset >= (ULONG)dwVarLen)
          return FALSE;
        lpszData += ulAjustedOffset;     // move ptr
        dwVarLen -= ulAjustedOffset;     // decrease size
      }

      if (ulAjustedSize && (dwVarLen > ulAjustedSize))
        dwVarLen = ulAjustedSize;

      // Set the destination char size (assuming TCHAR)
      uDestCharSize = sizeof(TCHAR);

      // switch on VB data type
      switch (usVBDataType)
      {
        case DT_HLSTR:
        case DT_HSZ:
          if (!fGetLenOnly)
          {
             // copy from buffer TO LBlock, then, if HSZ, call fpDBCreateHsz()
             // so that the "types" match.  
             tmpLBlock = LBlockAllocXLenToXLen(lpszData,
               (UINT)(dwVarLen / uSrcCharSize), uSrcCharSize, uDestCharSize);
             if ((usVBDataType == DT_HSZ) && tmpLBlock)
             {
                *lplData = (LONG)fpDBCreateHsz((LPTSTR)tmpLBlock);
                LBlockFree(tmpLBlock);
             }
             else   
                *lplData = (LONG)tmpLBlock;
          }
          ulReturn = (ULONG)dwVarLen / uSrcCharSize;
          break;

        default:
          // Add support for this VB type!!!
          ASSERT(0);
          ulReturn = (ULONG)-1;
          break;
      } //switch (usVBDataType)
      break;

    case VT_R8:
    case VT_I8:
    case VT_UI8:
    case VT_DATE:                  // convert 8-byte type to user's choice
        ulReturn = (ULONG)sizeof(double); // should be 8 bytes
        if (!fGetLenOnly)               // if value is wanted
        {
           if ((usVBDataType == DT_HSZ) || (usVBDataType == DT_HLSTR))
           {
             VARIANTARG varData1, varData2;
             VariantInit(&varData1);
             varData1.vt = vtDataType;
             varData1.date = m_varValSet.date;
             VariantInit(&varData2);
             h = VariantChangeType(&varData2, &varData1, (USHORT)0, VT_BSTR);
             ASSERT(h == S_OK);         // Could not convert DATE to BSTR!
             if ((h == S_OK) && varData2.bstrVal)
             {
               if (usVBDataType == DT_HSZ)
               {
                  LPTSTR tstr = fpSysAllocTStringFromBSTR(varData2.bstrVal);
                  if (tstr)
                  {
                    *lplData = (LONG)fpDBCreateHszLen((LPTSTR)tstr, 
                                       fpSysTStringLen(tstr));
                    fpSysFreeTString(tstr);
                  }
               }
               else
                  *lplData = (LONG)LBlockAllocXLenToXLen( varData2.bstrVal,
                    SysStringLen(varData2.bstrVal), sizeof(OLECHAR), sizeof(TCHAR));
               // free variant's BSTR
               SysFreeString(varData2.bstrVal);
             }
             else
                ulReturn = (ULONG)-1;
           }
           else if (usVBDataType == VT_DATE)
           {
             // Add support for converting DATE to YOUR type!
             ASSERT(0);
             ulReturn = (ULONG)-1;
           }
           else
           {
             // Add support for converting DATE to YOUR type!
             ASSERT(0);
             ulReturn = (ULONG)-1;
           }
        }
      break;  // VT_DATE

    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_CY:
    case VT_BOOL:
    case VT_I1:
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_INT:
    case VT_UINT:
      ulReturn = 2;
      if (vtDataType == VT_I4 || vtDataType == VT_R4 || vtDataType == VT_CY ||
          vtDataType == VT_UI4 || 
          ((vtDataType == VT_INT || vtDataType == VT_UINT ||
            vtDataType == VT_BOOL) && (sizeof(int) == 4)))
        ulReturn = 4;
      else if (vtDataType == VT_I1 || vtDataType == VT_UI1)
        ulReturn = 1;
      if (!fGetLenOnly)
      {
         if ((usVBDataType == DT_HSZ) || (usVBDataType == DT_HLSTR))
         {
           FPTSTR tstr;
           tstr = (FPTSTR)fpConvertTypeToTSTR((LPVOID)&m_varValSet.lVal, 
                            (USHORT)vtDataType);
           if (tstr)
           {
             if (usVBDataType == DT_HSZ)
               *lplData = (LONG)fpDBCreateHsz( (LPTSTR)tstr);
             else
               *lplData = (LONG)LBlockAllocXStrToXStr(tstr,
                            sizeof(TCHAR), sizeof(TCHAR));
             fpSysFreeTString(tstr);
           } //if tstr
         }
         else
         {
           if (ulReturn == 1)
             *(ULONG FAR *)lplData = (ULONG)m_varValSet.bVal;
           else if (ulReturn == 2)
             *lplData = (LONG)m_varValSet.iVal;
           else
             *lplData = m_varValSet.lVal;
         }
      }
      break;

    default:
      ASSERT(0);
  } //switch (m_ulType)

  if (fGetLenOnly)
    *lplData = (LONG)ulReturn;

// fix for BOM1493 -scl
//  return (ulReturn > 0 ? TRUE : FALSE);
  return (ulReturn >= 0 ? TRUE : FALSE);
}

/***************************************************************************
 *
 * FUNCTION:  CfpDBColumn::SetDataValue()
 *
 * DESCRIPTION:
 *
 *   This function fills the DBBINDPARAMS structure used by 
 *   ICursorUpdateARow::SetColumn().  
 *
 * PARAMETERS:
 *
 *   lpBindParams ... Ptr to DBBINDPARAMS structure.
 *   usVBDataType ... Format for the returned data (DT_LONG, DT_HSZ, etc).
 *   ulChunkOffset .. For types DBTYPE_CHAR and VT_BLOB, gets chunk at offset.
 *   ulChunkSize .... Sizeof chunk to get.
 *   lData  ......... Either the value or ptr to string. 
 *   uFlags ......... Flags (indicates NULL value)
 *   lpCursor ....... Ptr to Cursor.  Used when referencing BLOB values.
 *
 * RETURNS:
 *
 *   0 if successful, else ERROR
 *
 **************************************************************************/
BOOL CfpOleSimpleBound::SetDataValue(LPVOID lpvda)
{
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
  LONG lData = lpda->lData;
  USHORT usVBDataType = lpda->usDataType;
  ULONG ulChunkOffset = lpda->ulChunkOffset;
  ULONG ulChunkSize = lpda->ulChunkNumBytes;
  UINT uFlags = lpda->fs;
  BSTR bstr;

  // if stuck with only BSTR support...
  // Are we setting it to NULL?
  if (m_fOnlyBSTR && (uFlags & DA_fNull))
  {
     m_fCurrGet = TRUE;
     m_lpvarCurrGet->vt = VT_BSTR;
     m_lpvarCurrGet->bstrVal = SysAllocStringLen(NULL, 0);
     return TRUE;
  }

  // Are we setting it to NULL?
  if (uFlags & DA_fNull)
  {
     m_fCurrGet = TRUE;
     m_lpvarCurrGet->vt = VT_NULL;
     return TRUE;
  }
                             
  // if we are given a string, set BSTR in VARIANT 
  if ((usVBDataType == DT_HLSTR) || (usVBDataType == DT_HSZ))
  {
    // If size is different from entire string...
    if (ulChunkSize)
    {
      TCHAR tcSave = ((LPTSTR)lData)[ulChunkSize]; // save last+1
      ((LPTSTR)lData)[ulChunkSize] = (TCHAR)0;     // replace last+1 with NULL
      bstr = (BSTR)fpSysAllocBSTRFromTString((LPTSTR)lData);  // copy string
      ((LPTSTR)lData)[ulChunkSize] = tcSave;       // restore last+1
    }
    else
      bstr = (BSTR)fpSysAllocBSTRFromTString((LPTSTR)lData);

    if (!bstr)
    {
#if !defined(NO_MFC) && !defined(SS40) // MFC
      m_lpOwner->ThrowError(CTL_E_OUTOFMEMORY);
#endif
      return FALSE;
    }

    // if appending data to previous BSTR...
    if (m_lpvarCurrGet->vt == VT_BSTR && m_lpvarCurrGet->bstrVal)
    {
      UINT oldSize = SysStringLen(m_lpvarCurrGet->bstrVal);
      UINT addSize = SysStringLen(bstr);
      if (!SysReAllocStringLen(&m_lpvarCurrGet->bstrVal, NULL, oldSize+addSize))
      {
#if !defined(NO_MFC) && !defined(SS40) // MFC
        m_lpOwner->ThrowError(CTL_E_OUTOFMEMORY);
#endif
        return FALSE;
      }  
      _fmemcpy(&m_lpvarCurrGet->bstrVal[oldSize], bstr, addSize * sizeof(OLECHAR));
      SysFreeString(bstr);
    }
    else
    {
      m_fCurrGet = TRUE;
      m_lpvarCurrGet->vt = VT_BSTR;
      m_lpvarCurrGet->bstrVal = bstr;
    }
  } 
  // else some binary type (not given a string)
  else
  {
    VARTYPE vt = 0;
    m_fCurrGet = TRUE;

    switch (usVBDataType)
    {
      case DT_SHORT:  vt = VT_I2; m_lpvarCurrGet->iVal = (SHORT)lData;     break; // 0x02
      case DT_BOOL :  vt = VT_BOOL; m_lpvarCurrGet->boolVal = (BOOL)lData; break; // 0x04
      case DT_LONG :  vt = VT_I4; m_lpvarCurrGet->lVal = lData;            break; // 0x03
      case DT_REAL :  vt = VT_R4; m_lpvarCurrGet->fltVal = *(float FAR *)&lData;  break; // 0x07
//    case DT_DATETIME: vt = VT_DATE; m_lpvarCurrGet->date = *(DATE FAR *)lData;  break; // 0x07
      default:
        ASSERT(0);    //Add support!!!
        m_fCurrGet = FALSE;
    }
    if (m_fCurrGet)
      m_lpvarCurrGet->vt = vt;
  }

  return TRUE;
}



BOOL CfpOleSimpleBound::fpQueryError(void)
{
  // if 0, return FALSE, else return TRUE
  return (m_iErrorCt != 0);
}

void CfpOleSimpleBound::fpResetError(LPVOID lpvUserData)
{
  m_iErrorCt = 0;
  m_ErrorInfo.lpvUserData = lpvUserData;
  if( lpvUserData )
    ((LPDATAACCESS)lpvUserData)->lpfpOleBinding = this;
  return;
}

void CfpOleSimpleBound::fpSetError(LONG lErrorCode, LONG lRcFromCursorAPI, 
  LPVOID lpvQueryInterfacePtr)
{
  m_ErrorInfo.lErrorCode = lErrorCode;
  m_ErrorInfo.lExtErrorCode = lRcFromCursorAPI;
  m_iErrorCt = 1;

  return;
}

LONG CfpOleSimpleBound::fpInvokeError(void)
{
  if (m_iErrorCt)
  {
    // Send back the error as a VBM_DATA msg (VBM_DATA_UERROR)
    // wParam = ErrorCode, lParam = ptr to m_lpErrorInfo STRUCTURE
    //
    ASSERT(m_lpfuncDataMsg); //Callback func MUST BE SET BY OWNER!
    if (m_lpfuncDataMsg)
       m_lpfuncDataMsg( (LPVOID)m_lpOwner, VBM_DATA_UERROR, 
         (WPARAM)m_ErrorInfo.lErrorCode, (LPARAM)(LPVOID)&m_ErrorInfo);

    // Reset error for next time
    fpResetError();
  }

  return 0;
}
