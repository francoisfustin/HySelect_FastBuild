/*************************************************************************\
* FPDBCALL.CPP                                                            *
*                                                                         *
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.                   *
* All Rights Reserved.                                                    *
*                                                                         *
* No part of this source code may be copied, modified or reproduced       *
* in any form without retaining the above copyright notice.  This         *
* source code, or source code derived from it, may not be redistributed   *
* without express written permission of FarPoint Technologies.            *
*                                                                         *
\*************************************************************************/
#ifndef NO_MFC
#include <afxctl.h>         // MFC support for OLE Custom Controls
#else  //NO_MFC
#include "stdafx.h"
#include "fpatl.h"
#endif

#ifdef SS40
#include "ss40dao.h"
#endif

#include "vbinterf.h"                                                                                                                          

#include "fpconvrt.h"

#include "fpdbutil.h"		  
#include "fptstr.h"

#include "fpdbcol.h"
#include "fpoledb.h"		  


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::DataCall()
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
short CfpOleBinding::DataCall( LPVOID lpvda, USHORT msg, 
                               USHORT action, LPVOID lpvUserData)
{
  short sReturn = 0;

  if (!m_lpMoveCursor)
  {
    // We don't have a cursor. If it's UserMode (Run mode), but we
    // are not bound, return FALSE.
#ifdef SS40
    if (DataManager_GetAmbientUserMode(m_lpOwner) && (m_iNotifyStatus == 0))
#else
    if (m_lpOwner->GET_AMBIENT_USERMODE() && (m_iNotifyStatus == 0))
#endif
	  return FALSE;	

    if (!GetMoveCursor())
      return FALSE;
  }

  switch (msg)
  {
    case VBM_DATA_GET:
      sReturn = ProcessData_Get( lpvda, action, lpvUserData);
      break;

    case VBM_DATA_METHOD:
      sReturn = ProcessData_Method( lpvda, action, lpvUserData);
      break;

    case VBM_DATA_SET:
      sReturn = ProcessData_Set( lpvda, action, lpvUserData);
      break;

    // Any other types are not supported!!!
    default:
      ASSERT(0);                   // force Assertion error.
      sReturn = (short)-1;
  }
  return sReturn;
}


/***************************************************************************
 *
 * FUNCTION:     CfpOleBinding::ProcessData_Get()
 *
 * DESCRIPTION:
 *
 *   Process VB's VBM_DATA_GET messages.  Get bookmark, field info,
 *   field value, or record count.
 *
 *   Normally, this function is only called by CfpOleBinding::DataCall().
 *
 * PARAMETERS:
 *
 *   lpvda ... Far ptr to DATAACCESS structure.
 *
 *   usAction	     Description
 *   --------        -----------
 *   DATA_BOF        Determines whether current record position is before 
 *                   the first record in the recordset. Returns a DT_SHORT.
 *   DATA_BOOKMARK   Get a bookmark for the recordset according to the 
 *                   lData setting:
 *    > DATA_BOOKMARKCURRENT - return bookmark for current record.
 *    > DATA_BOOKMARKFIRST   - return bookmark for first record.
 *    > DATA_BOOKMARKLAST    - return bookmark for last record.
 *    > DATA_BOOKMARKNEXT    - return bookmark for next record relative to 
 *                             bookmark in hlstrBookMark.
 *    > DATA_BOOKMARKPREV    - return bookmark for previous record relative 
 *                             to bookmark in hlstrBookMark.
 *                   Returns an HLSTR.
 *  
 *   DATA_BOOKMARKABLE  Always TRUE.
 *   DATA_EOF	     Determines whether current record position is after 
 *                   the last record in the recordset. Returns a DT_SHORT.
 *   DATA_FIELDATTRIBUTES
 *                   Get attribute for the field. The returned lData value
 *                   can be 0 or any combination of the following values:
 *    > DB_FIXEDFIELD     - value in the field is fixed-length.
 *    > DB_AUTOINCRFIELD  - value for new records is automatically 
 *                          incremented by the database.
 *    > DB_UPDATABLEFIELD - value in the field can be changed.
 *                   Returns DT_LONG.
 *    > UDB_NULLABLE      - Added for OLE, Is null allowed?
 *  
 *   DATA_FIELDCHUNK   Get a chunk of data as specified by the ulChunkOffset
 *                     and ulChunkNumBytes. Returns an HLSTR.
 *   DATA_FIELDNAME    Get the field name. Returns an HSZ.
 *   DATA_FIELDPOSITION	 Returns the ordinal position of the of the field in the
 *                     Fields collection. Returns a DT_SHORT.
 *   DATA_FIELDSCOUNT  Get the number of fields in the current recordset.
 *                     Returns a DT_SHORT.   
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
 *   DATA_LASTMODIFIED Get the bookmark of the last modified record.
 *   DATA_RECORDCOUNT  Determines the number of records in the recordset.
 *                     Returns a DT_LONG.
 *   DATA_UPDATABLE    Whether the recordset can be updated. returns DT_SHORT.
 *   DATA_UCTLFLAGS    Equivalent to VBSetControlFlags(). Return flags in lData.
 *
 * RETURNS:  0 if successful,  non-0 if error
 *
 **************************************************************************/
short CfpOleBinding::ProcessData_Get( LPVOID lpvda, USHORT usAction,
  LPVOID lpvUserData)
{
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
  short sReturn = 0;
  CfpDBColumn *pCF;

  switch (usAction)
  {
    case DATA_BOOKMARK:
      // if returns FALSE or flag is set (indicating BOF or EOF) return 
      // an error (as VB would in this case).
      if ((m_dwFetchStatus & FLAG_CURR_ROW_INVALID) &&
          (lpda->lData == DATA_BOOKMARKCURRENT))
         sReturn = (short)-1;
      else if (!GetBookmark(lpda->lData, lpda->hlstrBookMark, 
               (LPVOID FAR *)&lpda->lData, (UINT FAR *)&lpda->fs) || (lpda->fs))
         sReturn = (short)-1;
      break;

    case DATA_FIELDCHUNK:
    case DATA_FIELDVALUE:
      if (!this->GetFieldValue(FALSE, lpda)) // read field, if FALSE...
         sReturn = (short)-1;                // return error
      break;

    case DATA_FIELDSIZE:
      if (!this->GetFieldValue(TRUE, lpda))  // read field's length, if FALSE
         sReturn = (short)-1;                // return error
      else
         lpda->usDataType = DT_LONG;         // NOTE: Len is in lpda->lData
      break;

    case DATA_FIELDSCOUNT:
      lpda->lData = m_ColInfoList.GetCount();
      lpda->usDataType = DT_SHORT;
      break;

    case DATA_BOOKMARKABLE:       // Always TRUE
      lpda->lData = TRUE;
      lpda->usDataType = DT_LONG;
      break;

    case DATA_UPDATABLE:          // for now, return TRUE if not RemoteDataCtl
//    if (m_fRemoteDataCtl & FPDB_RDC_VER1)
//      lpda->lData = 0;
//    else
        lpda->lData = 1;
      lpda->usDataType = DT_LONG;
      break;


    case DATA_FIELDPOSITION:      // Return FieldIndex (0..Count-1)
    case DATA_FIELDTYPE:
    case DATA_FIELDATTRIBUTES:
      // If DataField is NULL, use DataFieldIndex as the position
      if (lpda->hszDataField)
         pCF = GetColumnRec((LPCTSTR)lpda->hszDataField);
      else
         pCF = GetColumnRec((int)lpda->sDataFieldIndex);

      // if record not found, return error, else return index
      if (pCF == NULL)
         sReturn = (SHORT)-1;   //error
      else
      {
         lpda->usDataType = DT_SHORT;
         switch (usAction)
         {
           case DATA_FIELDPOSITION:      // Return FieldIndex (0..Count-1)
             lpda->lData = (LONG)pCF->m_iIndex;
             break;
           case DATA_FIELDTYPE:
             lpda->lData = (LONG)pCF->GetVBDataType();
             break;
           case DATA_FIELDATTRIBUTES:
             lpda->usDataType = DT_LONG;
             lpda->lData = (LONG)pCF->m_uFlags;
             break;
         }
      }
      break;

    case DATA_FIELDNAME:
      lpda->usDataType = DT_HSZ;
      pCF = GetColumnRec((int)lpda->sDataFieldIndex);
      // if record not found, return error, else return fieldname
      if (pCF == NULL)
         sReturn = (SHORT)-1;   // error
      else
         lpda->lData = 
           (LONG)fpDBCreateHsz((LPTSTR)(LPCTSTR)pCF->m_strDataField);
      break;

    case DATA_RECORDCOUNT:
      lpda->usDataType = DT_LONG;
      if (!GetRecordCount( &lpda->lData))
         sReturn = (SHORT)-1;   // error
      break;

    case DATA_UCTLFLAGS:
      lpda->lData = (LONG)m_ulCtlFlags;
      break;

	case DATA_UDATAFIELD:             // Get the Bound column
      {
      LPTSTR tstr = (LPTSTR)(LPCTSTR)m_strBoundDataField;
      if (tstr)
         lpda->lData = (LONG)fpDBCreateHsz(tstr);
      else
         {
         lpda->lData = 0L;
         sReturn = (SHORT)-1;   // error
         }
      }
      break;

    case DATA_UREADENABLED: // if supported, lData = TRUE, else FALSE
//    lpda->lData = (long)(m_BmkColInfo != NULL);
      lpda->lData = (long)TRUE;  // always return TRUE
      break;

    case DATA_UREADFIRST:
    case DATA_UREADNEXT:
      {
        BOOL fReadFirst = (usAction == DATA_UREADFIRST);

        if (!ReadNextRow(fReadFirst, (UINT FAR *)&lpda->fs))
          sReturn = (SHORT)-1;
      }
      break;

    case DATA_UREADFIELDVALUE:
    case DATA_UREADFIELDSIZE:
      {
        BOOL fOnlyGetLen = (usAction == DATA_UREADFIELDSIZE);
        if (!this->GetFieldValue(fOnlyGetLen, lpda, TRUE))
           sReturn = (short)-1;                // return error
      }
      break;

    case DATA_UREADBOOKMARK:
      if (m_BmkColInfo) // if bookmark was read with data.
      {
        // save settings
        HSZ   hszSaveField    = lpda->hszDataField;
        SHORT sSaveFieldIndex = lpda->sDataFieldIndex;
        USHORT usSaveType     = lpda->usDataType;

        lpda->hszDataField = NULL;
        lpda->sDataFieldIndex = (SHORT)FPDB_BOOKMARK_COLINDEX;
        lpda->usDataType = DT_HLSTR;
        if (!this->GetFieldValue(FALSE, lpda, TRUE))
          sReturn = (short)-1;                // return error

        // restore settings
        lpda->hszDataField    = hszSaveField;
        lpda->sDataFieldIndex = sSaveFieldIndex;
        lpda->usDataType      = usSaveType;
      }
      else // else, get current bookmark from (position of) Clone.
      {
        if (!GetBookmark(DATA_BOOKMARKCURRENT, NULL, 
             (LPVOID FAR *)&lpda->lData, (UINT FAR *)&lpda->fs, 
             TRUE /*Use CloneCursor*/ ))
          sReturn = (short)-1;                // return error
      }
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
 * FUNCTION:  CfpOleBinding::ProcessData_Method()
 *
 * DESCRIPTION:
 *
 *   Process VB's VBM_DATA_METHOD messages.  Move to bookmark, add new
 *   record, or delete record.
 *
 *   Normally, this function is only called by CfpOleBinding::DataCall().
 *
 * PARAMETERS:
 *
 *   lpvda ... Far ptr to DATAACCESS structure.
 *
 *   usAction	     Description
 *   --------        -----------
 *   DATA_ADDNEW	 Prepare a new record to add to the recordset.
 *   DATA_BOOKMARK	 Move to the bookmark as specified by lp->hlstrBookmark. 
 *   DATA_DELETE	 Delete the current record in the recordset.
 *   DATA_MOVEFIRST	 Move to the first record. 
 *   DATA_MOVELAST	 Move to the last record. 
 *   DATA_MOVENEXT	 Move to the next record. 
 *   DATA_MOVEPREV	 Move to the previous record. 
 *	 DATA_UDATAFIELD Set the Bound column
 *
 * RETURNS:  0 if successful,  non-0 if error
 *
 **************************************************************************/
short CfpOleBinding::ProcessData_Method( LPVOID lpvda, USHORT usAction,
  LPVOID lpvUserData)
{
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
  short sReturn = 0;
  int   iSaveStatus;

  iSaveStatus = m_iNotifyStatus;
  // User is causing some action, allow notify messages
  m_iNotifyStatus &= ~(NOTIFY_SYNCBEFORE_IGNORED | NOTIFY_SYNCAFTER_IGNORED);

  switch (usAction)
  {
	case DATA_BOOKMARK:
    case DATA_MOVEFIRST:
    case DATA_MOVELAST:	
    case DATA_MOVENEXT:	
    case DATA_MOVEPREV:	
      if (!MoveCurrentRow(usAction, lpda->hlstrBookMark, (UINT *)&lpda->fs))
	  	 sReturn = (short)-1;
      else
         m_dwFetchStatus |= FLAG_CURR_ROW_MOVED;
	  break;
	
	case DATA_UDATAFIELD:             // Set the Bound column
    {
      LPTSTR tstr = fpSysAllocTStringFromBSTR((LPVOID)lpda->hszDataField);
      if (BindDataFieldColumn(tstr))
      {
         if (m_strBoundDataField)
           fpSysFreeTString(m_strBoundDataField);
         m_strBoundDataField = tstr;
         tstr = NULL;
      }
      else
         sReturn = (short)-1;
      if (tstr)
        fpSysFreeTString(tstr);
	  break;
    }
	case DATA_ADDNEW:
      fpResetError(lpvUserData);
      if (!Process_AddNew())
      {
         sReturn = (short)-1;
         // invoke the Error Event
         fpInvokeError();
      }
      break;

	case DATA_DELETE:
      fpResetError(lpvUserData);
      if (!DeleteCurrentRow())
      {
         sReturn = (short)-1;
         // invoke the Error Event (not currently supported)
         fpInvokeError();
      }
      break;
 
  case DATA_URESETREAD:
      m_dwFetchStatus &= ~FLAGC_BMARK_FROM_FETCH;
      break;

    default:
      // Action not implemented.  IF YOU NEED IT, ADD IT!!!
      ASSERT(0);
      break;

  } //switch (action)

  fpResetError(NULL);

  // restore notify state
  m_iNotifyStatus = iSaveStatus;

  return sReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::ProcessData_Set()
 *
 * DESCRIPTION:
 *
 *   Process VB's VBM_DATA_SET messages.  Sets field value.
 *
 *   Normally, this function is only called by CfpOleBinding::DataCall().
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
short CfpOleBinding::ProcessData_Set( LPVOID lpvda, USHORT usAction,
  LPVOID lpvUserData)
{
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
  short sReturn = 0;
  int   iSaveStatus;

  iSaveStatus = m_iNotifyStatus;
  // Turn-off notify messages
  m_iNotifyStatus |= NOTIFY_SYNCBEFORE_IGNORED | NOTIFY_SYNCAFTER_IGNORED;

  // reset any errors
  fpResetError(lpvUserData);

  switch (usAction)
  {
	case DATA_FIELDCHUNK:
	case DATA_FIELDVALUE:
      if (!SetFieldValue(lpvda))
         sReturn = (short)-1;
      m_dwFetchStatus |= FLAG_CURR_ROW_DATACHANGED;
	  break;

	case DATA_UFINISHUPDATE:
	  // NOTE: FinishUpdatingRow() calls fpSetError() if there's a problem
      if (FinishUpdatingRow() == FALSE) 
         sReturn = (short)-1;
	  break;
	
    case DATA_UCTLFLAGS:
      // if setting DATACHANGED but BOUNDDATASET is currently set, ignore
      // the bit.
      if ((m_ulCtlFlags & CTLFLG_BOUNDDATASET) &&
          ((ULONG)lpda->lData & CTLFLG_DATACHANGED) &&
          !(m_ulCtlFlags & CTLFLG_DATACHANGED))
         m_ulCtlFlags = (ULONG)(lpda->lData & ~CTLFLG_DATACHANGED);
      else
         m_ulCtlFlags = (ULONG)lpda->lData;
      break;
  }

  // if an error occurred, invoke the EditError event
  if (sReturn != (SHORT)0)
     fpInvokeError();

  fpResetError(NULL);

  // restore notify state
  m_iNotifyStatus = iSaveStatus;

  return sReturn;
}




BOOL CfpOleBinding::fpQueryError(void)
{
  // if 0, return FALSE, else return TRUE
  return (m_iErrorCt != 0);
}

void CfpOleBinding::fpResetError(LPVOID lpvUserData)
{
  m_iErrorCt = 0;
  m_ErrorInfo.lpvUserData = lpvUserData;
  return;
}

void CfpOleBinding::fpSetError(LONG lErrorCode, LONG lRcFromCursorAPI, 
  LPVOID lpvQueryInterfacePtr)
{
  m_ErrorInfo.lErrorCode = lErrorCode;
  m_ErrorInfo.lExtErrorCode = lRcFromCursorAPI;
  m_iErrorCt = 1;

  return;
}

LONG CfpOleBinding::fpInvokeError(void)
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
