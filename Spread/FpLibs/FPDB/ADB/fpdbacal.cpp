/*************************************************************************\
* FPDBACAL.CPP                                                            *
*                                                                         *
* Copyright (C) 1991-1998 - FarPoint Technologies, Inc.                   *
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
#define FP_EXCLUDE_ATL
#include "stdafx.h"
#ifndef VC8
#include <ipserver.h>
#endif
//ATL #include "fpatl.h"
#endif

#include "vbinterf.h"                                                                                                                          

#include "fpconvrt.h"
#include "fptstr.h"

#include "fpoledb.h"		  
#include "fpdbutil.h"		  

#include <limits.h>
#include <malloc.h>

#include "util.h"


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

  if (!m_pColumns)
  {
    // We don't have a cursor. If it's UserMode (Run mode), but we
    // are not bound, return FALSE.
    if (fpQueryUserMode())
	  return FALSE;	

    if (fpInitDataBinding() != S_OK)
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
      _ASSERT(0);                   // force Assertion error.
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
  int i;

  switch (usAction)
  {
    case DATA_BOOKMARK:
      // if returns FALSE or flag is set (indicating BOF or EOF) return 
      // an error (as VB would in this case).
/* $ADB  if ((m_dwFetchStatus & FLAG_CURR_ROW_INVALID) &&
          (lpda->lData == DATA_BOOKMARKCURRENT))
         sReturn = (short)-1;
      else */ if (!fpGetBookmark(lpda->lData, lpda->hlstrBookMark, 
               (LPVOID FAR *)&lpda->lData, &lpda->fs) || (lpda->fs))
         sReturn = (short)-1;
      break;

    case DATA_FIELDCHUNK:
    case DATA_FIELDVALUE:
      if (!this->fpGetFieldValue(FALSE, lpda)) // read field, if FALSE...
         sReturn = (short)-1;                // return error
	  // fix for 11355 / 11957
	  // force IStream release immediately after reading data -scl
	  fpFreePreviousFetchData(FLAG_FETCHED_STREAMBUFFER);
      break;

    case DATA_FIELDSIZE:
      if (!this->fpGetFieldValue(TRUE, lpda))  // read field's length, if FALSE
         sReturn = (short)-1;                // return error
      else
         lpda->usDataType = DT_LONG;         // NOTE: Len is in lpda->lData
      break;

    case DATA_FIELDSCOUNT:
      lpda->lData = (LONG)(m_cColumns - (ULONG)m_fHasBookmarks);
      lpda->usDataType = DT_SHORT;
      break;

    case DATA_BOOKMARKABLE:
      lpda->lData = m_fHasBookmarks;
      lpda->usDataType = DT_LONG;
      break;

    case DATA_UPDATABLE:          // for now, return TRUE if not RemoteDataCtl
//    if (m_fRemoteDataCtl & FPDB_RDC_VER1)
//      lpda->lData = 0;
//    else
//      lpda->lData = 1;
      lpda->lData = 1;
      lpda->usDataType = DT_LONG;
      break;


    case DATA_FIELDPOSITION:      // Return FieldIndex (0..Count-1)
    case DATA_FIELDTYPE:
    case DATA_FIELDATTRIBUTES:
      // If DataField is NULL, use DataFieldIndex as the position
      if (lpda->hszDataField)
         i = fpGetColumnRec((LPCTSTR)lpda->hszDataField);
      else
         i = fpGetColumnRec((int)lpda->sDataFieldIndex);

      // if record not found, return error, else return index
      if (i < 0)
         sReturn = (SHORT)-1;   //error
      else
      {
         pCF = &m_pColumns[i];
         lpda->usDataType = DT_SHORT;
         switch (usAction)
         {
           case DATA_FIELDPOSITION:      // Return FieldIndex (0..Count-1)
             lpda->lData = (LONG)(i - (int)m_fHasBookmarks);
             break;
           case DATA_FIELDTYPE:
             lpda->lData = (LONG)pCF->fpGetVBDataType();
             break;
           case DATA_FIELDATTRIBUTES:
             lpda->usDataType = DT_LONG;
             lpda->lData = (LONG)pCF->uMetaFlags;
             break;
         }
      }
      break;

    case DATA_FIELDNAME:
      lpda->usDataType = DT_HSZ;
      i = fpGetColumnRec((int)lpda->sDataFieldIndex);
      // if record not found, return error, else return fieldname
      if (i < 0)
         sReturn = (SHORT)-1;   // error
      else
         lpda->lData = 
           (LONG)fpDBCreateHsz(m_pColumns[i].tstrName);
      break;

    case DATA_RECORDCOUNT:
      _ASSERT(0);  // ADD SUPPORT!!!
      sReturn = (SHORT)-1;   // error
//      lpda->usDataType = DT_LONG;
//      if (!GetRecordCount( &lpda->lData))
//         sReturn = (SHORT)-1;   // error
      break;

    case DATA_UCTLFLAGS:
      lpda->lData = (LONG)m_ulCtlFlags;
      break;

	case DATA_UDATAFIELD:             // Get the Bound column
      {
        LPTSTR tstr = (LPTSTR)(LPCTSTR)m_strBoundDataField;
        if (tstr)
          lpda->lData = (LONG)fpDBCreateHsz((LPTSTR)tstr);
        else
        {
          lpda->lData = 0L;
          sReturn = (SHORT)-1;   // error
        }
      }
      break;

    case DATA_UREADENABLED: // if supported, lData = TRUE, else FALSE
      // There is no need to support this feature, since we fetch a
      // group of rows instead of one-at-a-time.
      lpda->lData = (long)FALSE; 
      break;

    case DATA_UREADFIRST:
    case DATA_UREADNEXT:
      _ASSERT(0);  // ADD SUPPORT!!!
//      {
//        BOOL fReadFirst = (usAction == DATA_UREADFIRST);
//
//        if (!ReadNextRow(fReadFirst, (UINT FAR *)&lpda->fs))
//          sReturn = (SHORT)-1;
//      }
      break;

    case DATA_UREADFIELDVALUE:
    case DATA_UREADFIELDSIZE:
      _ASSERT(0);  // ADD SUPPORT!!!
//      {
//        BOOL fOnlyGetLen = (usAction == DATA_UREADFIELDSIZE);
//        if (!this->GetFieldValue(fOnlyGetLen, lpda, TRUE))
//           sReturn = (short)-1;                // return error
//      }
      break;

    case DATA_UREADBOOKMARK:
      _ASSERT(0);  // ADD SUPPORT!!!
//      if (m_BmkColInfo) // if bookmark was read with data.
//      {
//        // save settings
//        HSZ   hszSaveField    = lpda->hszDataField;
//        SHORT sSaveFieldIndex = lpda->sDataFieldIndex;
//        USHORT usSaveType     = lpda->usDataType;
//
//        lpda->hszDataField = NULL;
//        lpda->sDataFieldIndex = (SHORT)FPDB_BOOKMARK_COLINDEX;
//        lpda->usDataType = DT_HLSTR;
//        if (!this->GetFieldValue(FALSE, lpda, TRUE))
//          sReturn = (short)-1;                // return error
//
//        // restore settings
//        lpda->hszDataField    = hszSaveField;
//        lpda->sDataFieldIndex = sSaveFieldIndex;
//        lpda->usDataType      = usSaveType;
//      }
//      else // else, get current bookmark from (position of) Clone.
//      {
//        if (!GetBookmark(DATA_BOOKMARKCURRENT, NULL, 
//             (LPVOID FAR *)&lpda->lData, (UINT FAR *)&lpda->fs, 
//             TRUE /*Use CloneCursor*/ ))
//          sReturn = (short)-1;                // return error
//      }
      break;

    case DATA_UEVENTBMKCOUNT:  // 210 - Count of available bookmarks
      lpda->lData = (LONG)m_cBmkEventList;
      break;

    case DATA_UEVENTBMK:       // 211 - specify index in ".sDataFieldIndex"
      lpda->lData = 0;
      sReturn = (SHORT)-1;     // return error, unless we are successful
      if (m_aBmkEventList)
      {
        ULONG index = lpda->sDataFieldIndex;
        _ASSERT(index < m_cBmkEventList);    // index is invalid
        if (index < m_cBmkEventList && (ULONG)m_aBmkEventList[index])
        { // get HLSTR bookmark from array of CBookmarks.
          lpda->lData = (LONG)LBlockAllocLen(
            (const LPBYTE)(void*)m_aBmkEventList[index],
            (LONG)(ULONG)m_aBmkEventList[index]);
          sReturn = 0;
        }
      }
      break;

    case DATA_UEVENTCOLCOUNT:  // 212 - Count of available column indexes
      // If we have an array of columns, return the count (which is
      // the first element in the array).
      lpda->lData = (LONG)(m_aColIdEventList ? m_aColIdEventList[0] : 0);
      break;

    case DATA_UEVENTCOL:       // 213 - specify index in ".sDataFieldIndex"
      lpda->lData = 0;
      sReturn = (SHORT)-1;     // return error, unless we are successful

      if (m_aColIdEventList)   // If we have a list...
      {
        ULONG index = lpda->sDataFieldIndex; // index specified by user
        int   iExtDataFldIndex;
        _ASSERT(index < m_aColIdEventList[0]); // assert if index is invalid
        if (index < m_aColIdEventList[0])      // if index is valid
        { 
          iExtDataFldIndex =  // Get external datafld index for this index
            fpGetExtDataFldIndexForColId(m_aColIdEventList[index+1]);
          if (iExtDataFldIndex != -1) // if ext datafld is valid return OK.
          {
            lpda->lData = (LONG)iExtDataFldIndex;
            sReturn = 0;
          }
        }
      }
      break;

    default:
      // Action not implemented.  IF YOU NEED IT, ADD IT!!!
      _ASSERT(0);
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
  short sReturn = 0;
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
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
      // If control is invoking a "MoveLast" and an insert is pending,
      // then fake a "row position moved" event to force the control
      // to set the field values for the new row.
      if ((usAction == DATA_MOVELAST) && (m_iUpdateStatus & UPDSTAT_BEGIN_ADD))
        fpQueueEvent(DBEVENTPHASE_ABOUTTODO, DBREASON_ROWPOSITION_CHANGED, 0, FPDB_NOHROWS, FPDB_NOCOLIDS);

      if (!fpMoveCurrRow(usAction, lpda->hlstrBookMark, (UINT *)&lpda->fs))
	  	 sReturn = (short)-1;
      else
         m_dwFetchStatus |= FLAG_CURR_ROW_MOVED;
	  break;

	case DATA_UDATAFIELD:             // Set the Bound column
      {
        LPTSTR tstr = fpSysAllocTStringFromBSTR((LPVOID)lpda->hszDataField);
        if (fpBindDataFieldColumn(tstr))
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
      }
      break;

	case DATA_ADDNEW:
      fpResetError(lpvUserData);
      if (!fpInsertRow())
      {
         sReturn = (short)-1;
         // invoke the Error Event
         fpInvokeError();
      }
      break;

	case DATA_DELETE:
      fpResetError(lpvUserData);
      if (!fpDeleteRow(lpda->hlstrBookMark))
      {
         sReturn = (short)-1;
         fpInvokeError();
      }
      break;

    default:
      // Action not implemented.  IF YOU NEED IT, ADD IT!!!
      _ASSERT(0);
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
  short sReturn = 0;
  LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
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
      if (!fpSetFieldValue(lpvda))
         sReturn = (short)-1;
      m_dwFetchStatus |= FLAG_CURR_ROW_DATACHANGED;
	  break;

	case DATA_UFINISHUPDATE:
	  // NOTE: FinishUpdatingRow() calls fpSetError() if there's a problem
      if (fpFinishUpdatingRow() == FALSE) 
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

  // Restore notify state's bits related to SYNC-IGNORE...
  //
  // Remove those bits from notify state.
  m_iNotifyStatus &= ~(NOTIFY_SYNCBEFORE_IGNORED|NOTIFY_SYNCAFTER_IGNORED);
  // Add those bits back from saved status.
  m_iNotifyStatus |= iSaveStatus & (NOTIFY_SYNCBEFORE_IGNORED | NOTIFY_SYNCAFTER_IGNORED); 

  return sReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpQueryError()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::fpQueryError(void)
{
  // if 0, return FALSE, else return TRUE
  return (m_iErrorCt != 0);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpResetError()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpResetError(LPVOID lpvUserData)
{
  m_iErrorCt = 0;
  m_ErrorInfo.lpvUserData = lpvUserData;
  if( lpvUserData )
    ((LPDATAACCESS)lpvUserData)->lpfpOleBinding = this;
  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpSetError()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpSetError(LONG lErrorCode, LONG lRcFromCursorAPI, 
  LPVOID lpvQueryInterfacePtr)
{
  m_ErrorInfo.lErrorCode = lErrorCode;
  m_ErrorInfo.lExtErrorCode = lRcFromCursorAPI;
  m_iErrorCt = 1;

  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpInvokeError()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
LONG CfpOleBinding::fpInvokeError(void)
{
  if (m_iErrorCt)
  {
    // Send back the error as a VBM_DATA msg (VBM_DATA_UERROR)
    // wParam = ErrorCode, lParam = ptr to m_lpErrorInfo STRUCTURE
    //
    _ASSERT(m_lpfuncDataMsg); //Callback func MUST BE SET BY OWNER!
    if (m_lpfuncDataMsg)
       m_lpfuncDataMsg( (LPVOID)m_lpOwner, VBM_DATA_UERROR, 
         (WPARAM)m_ErrorInfo.lErrorCode, (LPARAM)(LPVOID)&m_ErrorInfo);

    // Reset error for next time
    fpResetError();
  }

  return 0;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpQueryUserMode()
 *
 * DESCRIPTION:
 *
 *   This function queries the control for the user mode status.
 *
 * PARAMETERS:
 *
 * RETURNS:  TRUE or FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::fpQueryUserMode(void)
{
#define FPDB_BOGUS_BOOL_VALUE  (BOOL)0x0BAD
  BOOL fUserMode = FPDB_BOGUS_BOOL_VALUE;   // Default to invalid value

  _ASSERT(m_lpfuncDataMsg); // Callback func MUST BE SET BY OWNER!
  if (m_lpfuncDataMsg)      // Call func to query user mode status
    m_lpfuncDataMsg( (LPVOID)m_lpOwner, VBM_DATA_QUERYUSERMODE, 
      (WPARAM)0, (LPARAM)(LPVOID)&fUserMode);

  // If returned value is unchanged, then ASSERT. This is to alert the
  // programmer to support the "VBM_DATA_QUERYUSERMODE" message. They
  // must set the return value (LPARAM is a LPBOOL) to either TRUE or FALSE.
  _ASSERT(fUserMode != FPDB_BOGUS_BOOL_VALUE);

  return (fUserMode == TRUE);
}


