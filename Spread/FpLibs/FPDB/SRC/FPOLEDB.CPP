/*************************************************************************\
* FPOLEDB.CPP                                                             *
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
#define RDCVARIANT  1       // Use DBVariants for Remote Data control (RDC)

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

#define DBINITCONSTANTS		         // These cause variable declarations

#ifndef NO_MFC
#define INITGUID
#include <initguid.h>
#endif

#include "fpdbutil.h"		  
#include "fpdbnote.h"		  

#include "fpdbcol.h"
#include "fpoledb.h"		  

#include <limits.h>
#include <malloc.h>

#define DB_VT_SAFEARRAY   0x2011
#define BMKMAX 256

// This will get translated to VBM_DATA_INITIATE
#define DATA_UINIT_CONNECT       300

// For some reason, this isn't defined for WIN16
#ifndef WIN32
#define METHOD_PROLOGUE_EX(theClass, localClass) \
	METHOD_PROLOGUE(theClass, localClass)

#endif

#ifdef SS40
#include "ss40dao.h"
#endif

/////////////////////////////////////////////////////////////////////////////
typedef LONG DBHRESULT;

typedef struct tagNameCol {
// fields for Info on optional metadata
  DWORD  dwInfoName;
  DWORD  dwInfoAutoIncr;
  DWORD  dwInfoFixedLen;
  DWORD  dwInfoNullable;
  DWORD  dwInfoUpdatable;
// fields for values
  DBCOLUMNID colID;          // COLUMNID
  ULONG  ulType;             // TYPE
  ULONG  ulMaxLen;           // MAXLENGTH
  DWORD  dwBindType;         // BINDTYPE
//  ULONG  ulEntryMaxLen;      // ENTRYIDMAXLENGTH
  DWORD  dwUpdatable;        // UPDATABLE
  BOOL   bAutoIncr;          // AUTOINCREMENT
  BOOL   bFixedLen;          // FIXED (length)
  BOOL   bNullable;          // NULLABLE
// fields for lengths
//DWORD  dwLenName;
// fields for strings (var data)
  LPFPOSTR lpfpoName;
//  FPOCHAR  fluff[256];
} ROWDATA;


//--------------------------------------------------------------------------
// Local forward declarations
//--------------------------------------------------------------------------
static CfpDBColumn *CreateColumnField( ROWDATA FAR *pRowData, int iIndex, BOOL fRemoteDataCtl);
static CfpDBColumn *CreateBMKColumnField( ROWDATA FAR *pRowData);
static int  FillMetaDataBindArray(DBCOLUMNBINDING FAR *rgBindCol, int iSize);
static BOOL BookmarksEqual( HLSTR b1, HLSTR b2);
static BOOL IsBeginningBookmark( HLSTR b1);
static BOOL IsEndBookmark( HLSTR b1);
static BOOL IsInvalidBookmark( HLSTR b1);
//static HSZ  CreateHszFromWCHAR( WCHAR FAR *wsz);



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::CfpOleBinding()
 *
 * DESCRIPTION:
 *
 *   Constructor for CfpOleBinding.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpOleBinding::CfpOleBinding()
{
#ifdef _DEBUG
  m_DBEventList = new CfpNotifyEvents;
#else
  m_DBEventList = NULL;
#endif

  m_fIgnoreSetDataField = FALSE;
  m_strBoundDataField = fpSysAllocTString(_T("*"));   // All columns

  m_lpOwner = NULL;            // Pointer to OCX class (owner)    
  m_lpfuncDataMsg = NULL;
  m_dispid = -1;               // invalid value

  m_ulCtlFlags = (ULONG)0;
  m_iNotifyStatus = 0;

  m_lpCursor = NULL;
  m_lpMoveCursor = NULL;				
  m_lpCloneMoveCursor = NULL;
  m_lpUpdateCursor = NULL;

  m_lpConnPt = NULL;

  m_fRemoteDataCtl = FPDB_NOT_RDC;
  m_dwNotifyDBEventsCookie = 0;

  m_ulBoundInlineLen = 0;
  m_ulBoundOutlineLen = 0;
  m_hBoundData = (DWORD)NULL;

  m_dwFetchStatus = FLAGC_FETCH_NEEDED;
  m_hlstrFetchBmark = NULL;
  m_iUpdateStatus = 0;

  m_iErrorCt;

  m_fUseEntryId = FALSE;

  m_fBindBMKCol = FALSE;   // Whether to TRY to bind to bookmark col.
  m_BmkColInfo = NULL;     // Column info if bound to bookmark column.

  m_fVBRefresh = FPDB_NOT_VBREFRESH; // ctl is being refreshed (reading all rows)
  return;

}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::~CfpOleBinding()
 *
 * DESCRIPTION:
 *
 *   Destructor for CfpOleBinding.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpOleBinding::~CfpOleBinding()
{

#ifdef _DEBUG
  delete m_DBEventList;
#endif

  // delete objects in m_ColInfoList
  FlushColInfoList();

  // deallocate memory
  if (m_hBoundData)
  {
    GlobalFree( (HGLOBAL)m_hBoundData);
    m_hBoundData = NULL;
  }    

  if (m_strBoundDataField)
    fpSysFreeTString(m_strBoundDataField);

  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetOwner()
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
void CfpOleBinding::SetOwner( LPVOID lpOleControl)
{
  *(LPVOID FAR *)&m_lpOwner = lpOleControl;
  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetDataSourceDispid()
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
void CfpOleBinding::SetDataSourceDispid( DISPID dispid)
{
  m_dispid = dispid;
  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetBindWithEntryId()
 *
 * DESCRIPTION:
 *
 *   Set flag indicating to Bind using EntryId's where possible.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::SetBindWithEntryId( BOOL fUseEntryId)
{
  m_fUseEntryId = fUseEntryId;
  return;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetDataMsgFunc()
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
void CfpOleBinding::SetDataMsgFunc( LPVOID lpfuncDataMsg)
{
  m_lpfuncDataMsg = 
    (LRESULT (FAR *)(LPVOID lpObject, 
       UINT uMsg, WPARAM wParam, LPARAM lParam))(lpfuncDataMsg);

}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetBindToBookmark()
 *
 * DESCRIPTION:
 *
 *   Set flag on whether to bind to the column containing the bookmarks.
 *   This allows the special ReadFirst/ReadNext procedure.
 *
 * PARAMETERS:   TRUE / FALSE
 *
 * RETURNS:  Previous value
 *
 **************************************************************************/
BOOL CfpOleBinding::SetBindToBookmark(BOOL fSetting)
{
  BOOL fOldSetting = m_fBindBMKCol;
  m_fBindBMKCol = fSetting;
  return fOldSetting;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::ReleaseDataSource()
 *
 * DESCRIPTION:
 *
 *   Release DataSource (removes all connections to cursor).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::ReleaseDataSource(void)
{
  if (m_iNotifyStatus & NOTIFY_ADVISE_ON)
     UnadviseDBEvents();

  if (m_lpConnPt)
  {
     m_lpConnPt->Release();
     m_lpConnPt = NULL;
  }

  // ICursorUpdateARow
  if (m_lpUpdateCursor)
  {
     // If in the middle of a Update, cancel it.
     if (m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))
        m_lpUpdateCursor->Cancel();

     m_lpUpdateCursor->Release();
     m_lpUpdateCursor = NULL;
     m_iUpdateStatus = 0;
  }

  // ICursorMove (Clone)
	if (m_lpCloneMoveCursor)
  {
     m_lpCloneMoveCursor->Release();
     m_lpCloneMoveCursor = NULL;
  }

  // ICursorMove
	if (m_lpMoveCursor)
  {
     m_lpMoveCursor->Release();
     m_lpMoveCursor = NULL;
  }

  // ICursor
	if (m_lpCursor)
  {
     m_lpCursor->Release();
     m_lpCursor = NULL;
  }

  if (m_hlstrFetchBmark)
  {
    LBlockFree((LBLOCK)m_hlstrFetchBmark);
    m_hlstrFetchBmark = NULL;
  }    

  // If Run-mode, delete objects in m_ColInfoList
  FlushColInfoList();
  
  // deallocate memory
  if (m_hBoundData)
  {
    GlobalFree( (HGLOBAL)m_hBoundData);
    m_hBoundData = NULL;
  }    

  return;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::AdviseDBEvents()
 *
 * DESCRIPTION:
 *
 *   Turn-on advising of NotifyDBEvents messages
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Returns previous state
 *
 **************************************************************************/
BOOL CfpOleBinding::AdviseDBEvents()
{
    // If already ON, ignore call.
    if (m_iNotifyStatus & NOTIFY_ADVISE_ON)
       return TRUE;

    if (!m_lpCursor)
       return FALSE;

    // If m_lpConnPt not initialize, get ptr to ConnectionPoint
    if (!m_lpConnPt)
    {
	   LPCONNECTIONPOINTCONTAINER pCPC;
       DBHRESULT h;

	   h = (DBHRESULT)m_lpCursor->QueryInterface(
             IID_IConnectionPointContainer,
             (LPVOID FAR*)&pCPC);
       if ((h != S_OK) || !pCPC)
       {
          // ConnectionContainer has failed
          ASSERT(0);
          return FALSE;
       }
	   h = (DBHRESULT)pCPC->FindConnectionPoint(IID_INotifyDBEvents, &m_lpConnPt);

       pCPC->Release();

       if ((h != S_OK) || !m_lpConnPt)
       {
          // Search for Connection has failed
          ASSERT(0);
          return FALSE;
       }
    }

    // if successful, set bit
#if defined(SS40) || !defined(NO_MFC)
	if (m_lpConnPt->Advise( (LPUNKNOWN)&m_xNotifyDBEvents, 
	                       &m_dwNotifyDBEventsCookie) == S_OK)
#else
	if (m_lpConnPt->Advise( GetUnknown(), 
	                       &m_dwNotifyDBEventsCookie) == S_OK)
#endif
       m_iNotifyStatus |= NOTIFY_ADVISE_ON;
   
    // Advise was successful
    ASSERT(m_iNotifyStatus & NOTIFY_ADVISE_ON);
    // Flush list if _DEBUG is defined
    ASSERT(m_DBEventList->Flush());

    return FALSE;                            // return previous state
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::UnadviseDBEvents()
 *
 * DESCRIPTION:
 *
 *   Turn-off advising of NotifyDBEvents messages
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Returns previous state
 *
 **************************************************************************/
BOOL CfpOleBinding::UnadviseDBEvents()
{
    // If NOT advising, just return
    if ((m_iNotifyStatus & NOTIFY_ADVISE_ON) == 0)
       return FALSE;

    // Flush the internal flags associated with notification.
    m_NotifyPreprocessor.FlushInternalFlags();

    // Turn-off bit
	m_iNotifyStatus &= ~NOTIFY_ADVISE_ON;
		
    // If we have a connection point ("Advise" conn pt), stop advising
    if (m_lpConnPt)
	   m_lpConnPt->Unadvise( m_dwNotifyDBEventsCookie);

    return TRUE;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::FlushColInfoList()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::FlushColInfoList(void) 
{
  // free any BSTR's that might be in the buffer
  if (m_hBoundData)
    FreePreviousFetchData();

  // Remove all from list and delete objects
  while (!m_ColInfoList.IsEmpty())
    delete ((CfpDBColumn *)m_ColInfoList.RemoveTail());

  // if column for Bookmarks, remove it too.
  if (m_BmkColInfo)
  {
    delete m_BmkColInfo;
    m_BmkColInfo = NULL;
  }
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetMetaData()
 *
 * DESCRIPTION:
 *
 *   Gets all of the column descriptions for this table
 *   NOTE:  Parameters may be NULL.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::GetMetaData(CStringArray *pStringArray, CDWordArray *pCookieArray) 
{
#define BINDCOLSIZE 10
	DBCOLUMNBINDING BindCol[BINDCOLSIZE];
    int iBindColCt;
	ICursor *pColumnsCursor;

	ULONG cRows, iRow;
//	ROWDATA RowData, *pRowData;
	ROWDATA FAR *pRowData;
//$Rdc ROWDATA FAR *RowData = NULL;
//$Rdc ROWDATA FAR *pRowData = NULL;
	
	DBFETCHROWS FetchRows;
	LPSTR sOutMem = NULL;
	LARGE_INTEGER liNext;
	DBHRESULT h;
    BOOL rc = TRUE;
    LPTSTR tstrTmp = NULL;
    CfpDBColumn *pCF;
    POSITION pos;
    int iFieldPos;

    // Clear list of Columns
    FlushColInfoList();

    m_fRemoteDataCtl = FALSE;     // reset Remote Data Control flag

  	// Try to Clone
    if (!m_lpMoveCursor)
      if (!GetMoveCursor())
        return FALSE;

	h = (DBHRESULT)m_lpCloneMoveCursor->GetColumnsCursor(IID_ICursor, (IUnknown **)&pColumnsCursor, &cRows);
	if ((h != S_OK) || (!pColumnsCursor))
    {
      pColumnsCursor = NULL;
	  rc = FALSE;
    }
    if (rc)
    {
       iBindColCt = FillMetaDataBindArray( BindCol, BINDCOLSIZE);

	   h = (DBHRESULT)pColumnsCursor->SetBindings(iBindColCt, BindCol, sizeof(ROWDATA),
	         DBCOLUMNBINDOPTS_REPLACE);
       // If the first try failed, we may be bound to a REMOTE Data control,
       // try without optional data.
       // NOTE: SCP 1/13/97 - Since NULLABLE info is slow, always re-bind WITHOUT
       //                     that column.
       if (h == DB_E_BADBINDINFO || h == S_OK)
       {
         DBHRESULT hFirstBind = h;
         iBindColCt -= 1;           // ignore last column (COL_NULLABLE)
	     h = (DBHRESULT)pColumnsCursor->SetBindings(iBindColCt, BindCol, sizeof(ROWDATA),
	           DBCOLUMNBINDOPTS_REPLACE);
         if ((h == S_OK) && (hFirstBind == DB_E_BADBINDINFO))
           m_fRemoteDataCtl = FPDB_RDC | FPDB_RDC_TEST_VER; // indicate RDC
       }
	   if (h != S_OK)
	   	 rc = FALSE;
	}

    if (rc)
    {
       // Allocate RowData with SysAlloc...
	   FetchRows.cRowsRequested = 1;
       if (m_fRemoteDataCtl)
       {
	     FetchRows.dwFlags = DBROWFETCH_CALLEEALLOCATES;
         FetchRows.pData = NULL;
	     FetchRows.pVarData = NULL; //$Rdc bug: if null, it frees pData
	     FetchRows.cRowsRequested = cRows;
       }
       else
       {
         UINT len = (UINT)cRows * sizeof(ROWDATA);
	     FetchRows.dwFlags = DBROWFETCH_DEFAULT;
  //       FetchRows.pData = &RowData;
	     FetchRows.pVarData = NULL;
         // Allocate a buffer for all the rows, then read them all at once.
         tstrTmp = fpSysAllocTStringByteLen(NULL, len);
         FetchRows.pData = tstrTmp;
	     FetchRows.cRowsRequested = cRows;
       }
	   FetchRows.cbVarData = 0;

       // Read ALL rows
	   LISet32(liNext, 0);
   	   h = (DBHRESULT)pColumnsCursor->GetNextRows(liNext, &FetchRows);
  	   if (h == S_OK || h == DB_S_ENDOFCURSOR || h == DB_S_ENDOFRESULTSET)
       {
         ULONG cRowsReturned = FetchRows.cRowsReturned;
         pRowData = (ROWDATA FAR *)FetchRows.pData;

	     for (iRow = 0; iRow < cRowsReturned; iRow++)
	     {
            // add entry if column has a name
            if (!(pRowData->dwInfoName == DB_NOVALUE) &&
                !(pRowData->dwInfoName == DB_NULL) &&
                !(pRowData->dwInfoName == DB_EMPTY))
            {
              int iColIndex = m_ColInfoList.GetCount();
              CfpDBColumn *pCF = 
                CreateColumnField( pRowData, iColIndex, m_fRemoteDataCtl);
          
              // Verify that we received a valid ptr and add it to the list.
              if (pCF)
	   	        m_ColInfoList.AddTail(pCF);
            }
            // else if bookmark column, bind to it.
            else if (m_fBindBMKCol && !m_fRemoteDataCtl &&
                     (_fmemcmp(&pRowData->colID, &COLUMN_BMKTEMPORARY,
                               sizeof(DBCOLUMNID)) == 0))
            {
              m_BmkColInfo = CreateBMKColumnField(pRowData);
            }

            // Next record.
            ++pRowData;
         } // for
	   } // if

       if (tstrTmp)
         fpSysFreeTString(tstrTmp);

       if (iRow == 0)  // no columns, return false
       {
          rc = FALSE;
       }
       else // columns found...
       {
         // Look for the 3 columns that indicate that this is a "replicated" 
         // table.  If it is, then those 3 columns are removed.
         CheckForReplicatedTable();

         // Loop thru list of columns and build list of Data fields & cookies--
         // used for IPerPropertyBrowsing (GetPredefinedStrings(), etc).
         pos = m_ColInfoList.GetHeadPosition();
         iFieldPos = 0;
         while (pos != NULL)
         {
           pCF = (CfpDBColumn *)m_ColInfoList.GetNext(pos);
	       if (pStringArray)
	          pStringArray->Add( pCF->m_strDataField);
	       if (pCookieArray)
	          pCookieArray->Add( pCF->m_iIndex);
         }
       }

    } // if rc

    if (pColumnsCursor)
	   pColumnsCursor->Release();
//    if (pClone)
//	   pClone->Release();

	return rc;
}

/***************************************************************************
 *
 * FUNCTION:  CreateColumnField()
 *
 * DESCRIPTION:
 *
 *   Creates a CfpDBColumn object from "Meta Data row".
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Ptr to NEW CfpDBColumn object if successful, else NULL
 *
 **************************************************************************/
static CfpDBColumn *CreateColumnField( ROWDATA FAR *pRowData, int iIndex, 
  BOOL fRemoteDataCtl)
{
  CfpDBColumn *pCF = NULL;
  LBLOCK colName = NULL;
  UINT uFlags = (UINT)0;
  BSTR bstrName =  (BSTR)(pRowData->lpfpoName);
  UINT uNameCharSize;

  if (!pRowData)
     return NULL;

  // Currently, RDC does not seem to support BINARY fields, so if this
  // is one, just return NULL.
  if (fRemoteDataCtl && pRowData->dwBindType == DBBINDTYPE_ENTRYID)
    return NULL;

  //$Rdc bug: for the ColName, instead of WCHAR, we get CHAR
  if (fRemoteDataCtl)
    uNameCharSize = sizeof(TCHAR);
  else
    uNameCharSize = sizeof(FPOCHAR);

  colName = LBlockAllocXStrToXStr(bstrName, uNameCharSize, sizeof(TCHAR));
  if (!colName)
     return NULL;

  // set the flags (some are from Optional Meta data columns)
  uFlags |= DB_UPDATABLEFIELD; 
  if ((pRowData->dwInfoUpdatable != DB_EMPTY) && 
      (pRowData->dwInfoUpdatable != DB_NULL))
  {
    if (pRowData->dwUpdatable == (DWORD)DBUPDATEABLE_NOTUPDATEABLE)
      uFlags &= ~DB_UPDATABLEFIELD; 
  }

  //$Rdc this flag is bogus from Remote Data Control
  // if a value was given, and it is true, set flag.
  //
  if (!fRemoteDataCtl &&
      ((pRowData->dwInfoAutoIncr != DB_EMPTY) && 
       (pRowData->dwInfoAutoIncr != DB_NULL)  &&
       (pRowData->bAutoIncr)))
     uFlags |= DB_AUTOINCRFIELD;

/* SCP 01/13/97 - This field (NULLABLE) is no longer retrieved because it can 
                  be very slow if RecordSource is a SQL statement.
  //$Rdc: This Meta data column is not supported for RDC, assume it's true
  // if a value was given, and it is true, set flag.
  if (fRemoteDataCtl ||
      ((pRowData->dwInfoNullable != DB_EMPTY) && 
       (pRowData->dwInfoNullable != DB_NULL)  &&
       (pRowData->bNullable)))
     uFlags |= UDB_NULLABLE;
*/

  //$Rdc this flag is bogus from Remote Data Control
  // if a value was given, and it is true, set flag.
 if (!fRemoteDataCtl &&
     ((pRowData->dwInfoFixedLen != DB_EMPTY) && 
      (pRowData->dwInfoFixedLen != DB_NULL)))
  {
     if (pRowData->bFixedLen)
        uFlags |= DB_FIXEDFIELD;
     else
        uFlags |= DB_VARIABLEFIELD;
  }
  else  // else, no value given, Assume that field is FIXED.
     uFlags |= DB_FIXEDFIELD;

  pCF = new CfpDBColumn( (LPTSTR)colName, pRowData->colID, pRowData->ulType,
              pRowData->ulMaxLen, pRowData->dwBindType, iIndex, uFlags);

  // destory allocated string
  LBlockFree(colName);

  // Free the BSTR if NOT Remote DC
  if (bstrName && !fRemoteDataCtl)
     SysFreeString(bstrName);
  
  return pCF;
}

/***************************************************************************
 *
 * FUNCTION:  CreateBMKColumnField()
 *
 * DESCRIPTION:
 *
 *   Creates a CfpDBColumn object for the Bookmark column from "Meta Data row".
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Ptr to NEW CfpDBColumn object if successful, else NULL
 *
 **************************************************************************/
static CfpDBColumn *CreateBMKColumnField( ROWDATA FAR *pRowData)
{
  CfpDBColumn *pCF = NULL;
  UINT uFlags = (UINT)0;

  if (!pRowData)
     return NULL;

  // if max size is known, use DBTYPE_BYTES, else use default (VT_BLOB)
  if (pRowData->ulMaxLen != DB_NOMAXLENGTH)
    pCF = new CfpDBColumn( NULL, pRowData->colID, DBTYPE_BYTES,
                pRowData->ulMaxLen, DBBINDTYPE_DATA, 0, uFlags);
  else
    pCF = new CfpDBColumn( NULL, pRowData->colID, pRowData->ulType,
                pRowData->ulMaxLen, pRowData->dwBindType, 0, uFlags);

  return pCF;
}


/***************************************************************************
 *
 * FUNCTION:  FillMetaDataBindArray()
 *
 * DESCRIPTION:
 *
 *   Fills the DBCOLUMNBINDING array for retrieving the meta data 
 *   for this DataSource.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
static int FillMetaDataBindArray(DBCOLUMNBINDING FAR *rgBindCol, int iSize)
{
#define SIZEOF_FIELD(s,m)	sizeof(((s *)0)->m)
   int i = 0;

/* TEST BUG with BSTR's
   // NAME (Optional Meta Data)
   rgBindCol[i].columnID   = COLUMN_NAME;
   rgBindCol[i].cbMaxLen   = 4;
   rgBindCol[i].obInfo       = 0;
   rgBindCol[i].obVarDataLen = 4;
   rgBindCol[i].obData       = 8;
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_BSTR; //*** try BSTR ***
   if (i == 0)
     return 1;
*/

   // NAME (Optional Meta Data)
   rgBindCol[i].columnID   = COLUMN_NAME;
   rgBindCol[i].obData     = offsetof(ROWDATA, lpfpoName);
// rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, lpfpoName);
   rgBindCol[i].cbMaxLen   = DB_NOMAXLENGTH;
   rgBindCol[i].obInfo     = offsetof(ROWDATA, dwInfoName);
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
// rgBindCol[i].dwDataType = DBTYPE_WCHARS; //*** try BSTR ***
   rgBindCol[i].dwDataType = VT_BSTR; //*** try BSTR ***
// rgBindCol[i].obVarDataLen = offsetof(ROWDATA, dwLenName);
   rgBindCol[i].obVarDataLen = DB_NOVALUE;
   // COLUMNID
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_COLUMNID;
   rgBindCol[i].obData     = offsetof(ROWDATA, colID);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, colID);
   rgBindCol[i].obInfo     = DB_NOVALUE;
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = DBTYPE_COLUMNID;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;
   // TYPE
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_TYPE;
   rgBindCol[i].obData     = offsetof(ROWDATA, ulType);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, ulType);
   rgBindCol[i].obInfo     = DB_NOVALUE;
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_I4;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;
   // MAXLENGTH
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_MAXLENGTH;
   rgBindCol[i].obData     = offsetof(ROWDATA, ulMaxLen);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, ulMaxLen);
   rgBindCol[i].obInfo     = DB_NOVALUE;
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_I4;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;
   // BINDTYPE
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_BINDTYPE;
   rgBindCol[i].obData     = offsetof(ROWDATA, dwBindType);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, dwBindType);
   rgBindCol[i].obInfo     = DB_NOVALUE;
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_I4;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;

/*   // ENTRYID_MAXLENGTH
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_ENTRYIDMAXLENGTH;
   rgBindCol[i].obData     = offsetof(ROWDATA, ulEntryMaxLen);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, ulEntryMaxLen);
   rgBindCol[i].obInfo     = DB_NOVALUE;
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_I4;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;
*/
   // UPDATABLE
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_UPDATABLE;
   rgBindCol[i].obData     = offsetof(ROWDATA, dwUpdatable);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, dwUpdatable);
   rgBindCol[i].obInfo     = offsetof(ROWDATA, dwInfoUpdatable);
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_I4;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;
   // AUTOINCREMENT (Optional Meta Data)
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_AUTOINCREMENT;
   rgBindCol[i].obData     = offsetof(ROWDATA, bAutoIncr);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, bAutoIncr);
   rgBindCol[i].obInfo     = offsetof(ROWDATA, dwInfoAutoIncr);
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_BOOL;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;
   // FIXED (length) (Optional Meta Data)
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_FIXED;
   rgBindCol[i].obData     = offsetof(ROWDATA, bFixedLen);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, bFixedLen);
   rgBindCol[i].obInfo     = offsetof(ROWDATA, dwInfoFixedLen);
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_BOOL;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;

   // NULLABLE (Optional Meta Data)
   //
   // SCP 01/13/97 - This field (NULLABLE) is no longer retrieved because it can 
   //                be very slow if ".RecordSource" is a SQL statement.
   //                HOWEVER, we use this field to determine if the DataSource
   //                is a RemoteData control.
   ++i;
   ASSERT( i < iSize);   // Increase size of array in "GetMetaData()" !
   rgBindCol[i].columnID   = COLUMN_NULLABLE;
   rgBindCol[i].obData     = offsetof(ROWDATA, bNullable);
   rgBindCol[i].cbMaxLen   = SIZEOF_FIELD(ROWDATA, bNullable);
   rgBindCol[i].obInfo     = offsetof(ROWDATA, dwInfoNullable);
   rgBindCol[i].dwBinding  = DBBINDING_DEFAULT;
   rgBindCol[i].dwDataType = VT_BOOL;
   rgBindCol[i].obVarDataLen = DB_NOVALUE;

   return (i+1);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetColumnRec(BSTR bstrColumnName)
 *
 * DESCRIPTION:
 *
 *   Find ColumnField entry for the ColumnName string.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Ptr to CfpDBColumn object if FOUND, else NULL.
 *
 **************************************************************************/
CfpDBColumn *CfpOleBinding::GetColumnRec(LPCTSTR tstrColumnName)
{
    POSITION pos;
    CfpDBColumn *pDF;
    BOOL bFound;

    if (tstrColumnName[0] == (TCHAR)0) 
       return NULL;

    // Scan thru the list looking for matching string.  If no list
    // or not found, return NULL.
    bFound = FALSE;
    pos = m_ColInfoList.GetHeadPosition();
    while (pos != NULL)
    {
	   pDF = (CfpDBColumn *)m_ColInfoList.GetNext(pos);
       if (lstrcmpi(pDF->m_strDataField, tstrColumnName) == 0)
       {
          bFound = TRUE;
          pos = NULL;                // Entry found, END LOOP!
       }
    }

    if (bFound)
       return pDF;
    else
       return NULL;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetColumnRec(INT iIndex)
 *
 * DESCRIPTION:
 *
 *   Find ColumnField entry for the column based on "FieldPosition" index.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Ptr to CfpDBColumn object if FOUND, else NULL.
 *
 **************************************************************************/
CfpDBColumn *CfpOleBinding::GetColumnRec(int iFieldPos)
{
    POSITION pos;
    CfpDBColumn *pDF;
    BOOL bFound;

    // if index is the special one for bookmark, return member variable.
    if (iFieldPos == FPDB_BOOKMARK_COLINDEX)
      return m_BmkColInfo;

    if ((iFieldPos < 0) || (iFieldPos >= m_ColInfoList.GetCount()))
       return NULL;

    // Scan thru the list looking for matching index.  If no list
    // or not found, return NULL.
    bFound = FALSE;
    pos = m_ColInfoList.GetHeadPosition();
    while (pos != NULL)
    {
	   pDF = (CfpDBColumn *)m_ColInfoList.GetNext(pos);
       if (pDF->m_iIndex == iFieldPos)
       {
          bFound = TRUE;
          pos = NULL;                // Entry found, END LOOP!
       }
    }

    if (bFound)
       return pDF;
    else
       return NULL;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::BindDataFieldColumn()
 *
 * DESCRIPTION:
 *
 *   Bind to the Column(s) specified by the parameter.  This binding
 *   is used when a row is fetched.
 *
 * PARAMETERS:
 *
 *   strDataField ... The parameter can be one of the three:
 *
 *                      ""  ...........  Remove any bindings.
 *                      "*" ...........  Bind ALL columns of the cursor.
 *                      column name ...  Bind to one specific column.
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::BindDataFieldColumn( LPTSTR strDataField)
{
  CfpDBColumn *pCF;
  int          ct = 0;
  int          addBmkCol = (m_BmkColInfo == NULL ? 0 : 1); 

  // Build array of CfpDBColumn entries to bind.

  if (!strDataField || !strDataField[0])
  {
    // Remove all bindings
    m_arrayBoundCols.SetSize(0);
  }
  // else if strDataField == "*", then bind all columns
  else if (strDataField[0] == '*' && strDataField[1] == (TCHAR)0)
  {
    int i;
    int max = m_ColInfoList.GetCount();

    m_arrayBoundCols.SetSize(max + addBmkCol);

    for (i = 0; i < max; ++i)
    {
      pCF = GetColumnRec(i);
      if (pCF)
      {
        m_arrayBoundCols.SetAt(ct, pCF);
        ++ct;
      }
    }

    // add bookmark column if specified
    if (addBmkCol)
    {
      m_arrayBoundCols.SetAt(ct, m_BmkColInfo);
      ++ct;
    }

    // if ct is not max, shrink array
    if (ct < (max + addBmkCol))
      m_arrayBoundCols.SetSize(ct);
  }
  else  // else bind to column name
  {
    pCF = GetColumnRec((LPCTSTR)strDataField);
    if (pCF)
    {
      m_arrayBoundCols.SetSize(1 + addBmkCol);
      m_arrayBoundCols.SetAt(0, pCF);
      if (addBmkCol)
        m_arrayBoundCols.SetAt(1, m_BmkColInfo);
       
    }
    else
    {
      m_arrayBoundCols.SetSize(0);
    }
  }

  return SetBoundColumn();
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetRecordCount()
 *
 * DESCRIPTION:
 *
 *   Get the number of records in current MoveCursor.
 *
 * PARAMETERS:
 *
 *   lpLong ...... ptr to return record count
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::GetRecordCount( LPLONG lpLong)
{
	DBHRESULT h;
    ICursorScroll FAR *lpScrollCursor;
    DWORD dwFlags;
    LARGE_INTEGER li;

    *lpLong = 0;

	// Try 
    if (!m_lpMoveCursor)
       if (!GetMoveCursor())
          return FALSE;

    h = (DBHRESULT)m_lpMoveCursor->QueryInterface( IID_ICursorScroll, 
                                        (LPVOID FAR*)&lpScrollCursor);
    if ((h != S_OK) || !(lpScrollCursor))
       return FALSE;

    h = (DBHRESULT)lpScrollCursor->GetApproximateCount( &li, &dwFlags);
    if (h == S_OK)
    {
       // check for 32-bit overflow
/*       if (li.u.HighPart)
         *lpLong = LONG_MAX;
       else
         *lpLong = (LONG)li.u.LowPart;   */
       if (li.HighPart)
         *lpLong = LONG_MAX;
       else
         *lpLong = (LONG)li.LowPart;
    }

    h = (DBHRESULT)lpScrollCursor->Release();

    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Utilities functions


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetCursor()
 *
 * DESCRIPTION:
 *
 *   Get Cursor from IBoundObjectSite for DataSource property.
 *   If successful, new cursor is put in "m_lpCursor"
 *   NOTE: bForceRefresh's default value is FALSE.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::GetCursor(BOOL bForceRefresh)
{
	IBoundObjectSite FAR *pBoundObjectSite = NULL;
	LPVOID pOut;
	IOleClientSite FAR *pOleClientSite;
	DBHRESULT h;
    BOOL rc = TRUE;
		
    // If we have a cursor .... and bForceRefresh is FALSE, return OK, 
    // else release cursor and get new one.
	if (m_lpCursor)
    {
       if (bForceRefresh == FALSE)
          return TRUE;

       m_lpCursor->Release();
       m_lpCursor = NULL;
    }

#ifdef SS40
  pOleClientSite = DataManager_GetClientSite(m_lpOwner);
#elif !defined(NO_MFC)  // MFC
	pOleClientSite = m_lpOwner->GetClientSite();
#else
	pOleClientSite = m_lpOwner->fpAtlGetClientSite();
#endif
    if (!pOleClientSite)
        return FALSE;

	h = (DBHRESULT)pOleClientSite->QueryInterface(IID_IBoundObjectSite, (void **)&pBoundObjectSite);
	if ((h != S_OK) || !pBoundObjectSite)
    {
	   rc = FALSE;
    }
    else
    {
	   h = (DBHRESULT)pBoundObjectSite->GetCursor(m_dispid, &m_lpCursor, &pOut);

	   if ((h != S_OK) || !m_lpCursor)
          rc = FALSE;

       pBoundObjectSite->Release();
    }

#ifdef NO_MFC  // ATL
    if (pOleClientSite)
      pOleClientSite->Release();
#endif // ATL

	return rc;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetMoveCursor()
 *
 * DESCRIPTION:
 *
 *   Get MoveCursor from DataSource's Cursor.
 *   If successful, new cursor is put in "m_lpMoveCursor"
 *   NOTE: bForceRefresh's default value is FALSE.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::GetMoveCursor(BOOL bForceRefresh)
{
	DBHRESULT h;

    // If we have a Move Cursor .... and bForceRefresh is FALSE, return OK, 
    // else release cursor and get new one.
	if (m_lpMoveCursor)
    {
       if (bForceRefresh == FALSE)
          return TRUE;

       m_lpMoveCursor->Release();
       m_lpMoveCursor = NULL;
    }
											  	
    // if cursor not valid, get valid cursor
    if (!m_lpCursor)
       if (!GetCursor())
          return FALSE;

	h = (DBHRESULT)m_lpCursor->QueryInterface(IID_ICursorMove,
	       (void **) &m_lpMoveCursor);

	if ((h != S_OK) || !m_lpMoveCursor)
	   return FALSE;

    // Clone the moveable cursor.
	h = (DBHRESULT)m_lpMoveCursor->Clone(DBCLONEOPTS_DEFAULT,
		  				 	 IID_ICursorMove,
							 (IUnknown **)&m_lpCloneMoveCursor);
	if ((h != S_OK) || !m_lpCloneMoveCursor)
	   return FALSE;
    else
	   return TRUE;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetUpdateCursor()
 *
 * DESCRIPTION:
 *
 *   Get ICursorUpdateARow cursor from DataSource's Cursor.
 *   If successful, new cursor is put in "m_lpUpdateCursor"
 *   NOTE: bForceRefresh's default value is FALSE.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::GetUpdateCursor(BOOL bForceRefresh)
{
	DBHRESULT h;

    // If we have a Move Cursor .... and bForceRefresh is FALSE, return OK, 
    // else release cursor and get new one.
	if (m_lpUpdateCursor)
    {
       if (bForceRefresh == FALSE)
          return TRUE;

       m_lpUpdateCursor->Release();
       m_lpUpdateCursor = NULL;
    }
											  	
    // if cursor not valid, get valid cursor
    if (!m_lpCursor)
       if (!GetCursor())
          return FALSE;

	h = (DBHRESULT)m_lpCursor->QueryInterface(IID_ICursorUpdateARow,
	       (void **) &m_lpUpdateCursor);
    ASSERT((h == S_OK) && m_lpUpdateCursor); // QueryInterface() failed!

	if ((h != S_OK) || !m_lpUpdateCursor)
    {
       fpSetError( FPDB_ERR_QINTF_UPDATECURSOR, (LONG)h, m_lpUpdateCursor);
	   return FALSE;
    }
    else
	   return TRUE;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::FetchCurrentRow()
 *
 * DESCRIPTION:
 *
 *   Gets one row of DATA from the table.  (To be added: if the bookmark
 *   is null, read at current row.)
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful,  else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::FetchCurrentRow(UINT FAR *lpuFlags)
{
	LPSTR lpszData = (LPSTR)GlobalLock((HGLOBAL)m_hBoundData);
	DBFETCHROWS FetchRow;
	LARGE_INTEGER li;
    BOOL          fReturn = TRUE;
	DBHRESULT h = (DBHRESULT)S_OK;
		
	// Move the cursor to the beginning for the data.
	LISet32(li,0);					  	

    ASSERT(lpszData);                       // Did memory lock prorperly?
    if (!lpszData)
    {
       fpSetError( FPDB_ERR_MEMLOCK_DATABUFFER, (LONG)0, (LPVOID)m_hBoundData);
       return FALSE;
    }

	FetchRow.cRowsRequested = 1;
	FetchRow.dwFlags = DBROWFETCH_DEFAULT;  // we allocate the memory
    // Inline buffer
	FetchRow.pData = lpszData;
    // Out-of-line buffer
	FetchRow.pVarData = lpszData + m_ulBoundInlineLen;
	FetchRow.cbVarData = m_ulBoundOutlineLen;

    h = (DBHRESULT)m_lpCloneMoveCursor->GetNextRows(li, &FetchRow);

	if (h != S_OK)
    {
       fpSetError( FPDB_ERR_GETNEXTROW, (LONG)h, NULL);
       fReturn = FALSE;
    }


    if (fReturn && (FetchRow.cRowsReturned == 0))
    {
       *lpuFlags &= DA_fEOF;
       fReturn = FALSE;
    }

    // Reset these flags (NOTE: "var &= ~bits" resets those bits to 0)
    if (fReturn)
       m_dwFetchStatus &= ~FLAGC_FETCH_NEEDED;

	GlobalUnlock((HGLOBAL)m_hBoundData);

    return fReturn;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetFieldValue()
 *
 * DESCRIPTION:
 *
 *   Gets the data for ONE column based on either the current row OR
 *   the row passed (as indicated by a bookmark).
 *
 *   It determines the status of the data in the current fetch buffer.  
 *   If necessary, the position of the Clone is moved to the row BEFORE
 *   the target row and the fetched (with GetNextRows()).  After
 *   filling fetch buffer, the column of data is then retrieved from
 *   the buffer and returned.
 *
 * PARAMETERS:
 *
 *   fOnlyGetLen ... Boolean flag, if TRUE, only get the field's length,
 *                   if FALSE, get the value of the field (the data).
 *   lpvda ......... Ptr to DATAACCESS structure.
 *
 *   Input fields used from lpvda:
 *
 *     hlstrBookMark ..... Bookmark of row. If NULL, then use current row.
 *     hszDataField  ..... Specifies column name of value.  If NULL, 
 *                         uses "sDataFieldIndex".
 *     sDataFieldIndex ... Specifies index of column of value.
 *     usDataType ........ Format to coerce data into.
 *
 *   Output fields set in lpvda:
 *
 *     lData ............. For returning data. It's either a LONG or a ptr,
 *                         "usDataType" determines which.
 *     fs ................ For returning info about data. "DA_fNull" if 
 *                         field doesn't have a value (null).
 *
 * RETURNS:  TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::GetFieldValue(BOOL fOnlyGetLen, LPVOID lpvda, 
  BOOL fReadCurrClone)
{
    LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
    BOOL fReturn = TRUE;
    CfpDBColumn *pCF;
	LPSTR lpszData;
    ULONG ulTmp;
    LPLONG lplData;

    // If DataField is NULL, use DataFieldIndex as the position
    if (lpda->hszDataField)
       pCF = GetColumnRec((LPCTSTR)lpda->hszDataField);
    else
       pCF = GetColumnRec((int)lpda->sDataFieldIndex);

    // if pCF is null, return error
    if (!pCF)
       return FALSE;

    // Verify that fetch buffer matches row.
    if (fReadCurrClone)  // verify that buffer is from clone (read first/next)
      fReturn = ((m_dwFetchStatus & FLAG_FETCHED_FROM_CLONE) != 0);
    else // Verify that fetch buffer matches row, if not, it fetches the row.
      fReturn = VerifyFetchBuffer( (LPVOID)lpda->hlstrBookMark, (UINT *)&(lpda->fs));

    // Get data from fetch buffer
    // match datafield with ColumnField record and get the data from it.
    if (fReturn)
	{
	   lpszData = (LPSTR)GlobalLock((HGLOBAL)m_hBoundData);
       // If fOnlyGetLen is TRUE, pass NULL instead of &lData, so only
       // the length gets retrieved, not the data
       if (fOnlyGetLen)
          lplData = NULL;
       else
          lplData = (LPLONG)&lpda->lData;

       // Note: GetDataValue() returns data's len OR -1 if error.
#ifndef RDCVARIANT	// NOT!
       if (m_fRemoteDataCtl)
         ulTmp = pCF->GetDataValueFromRDC(lpszData, 
           lpszData + m_ulBoundInlineLen, lpda->usDataType, 
           lplData, (UINT *)&lpda->fs, m_lpCloneMoveCursor);
       else
#endif
		   ulTmp = pCF->GetDataValue(lpszData, lpda->usDataType, 
           lpda->ulChunkOffset, lpda->ulChunkNumBytes,
           lplData, (UINT *)&lpda->fs, m_lpCloneMoveCursor);

       fReturn = (ulTmp != (ULONG)-1);
       // if getting just the len, set lData to value.
       if (fOnlyGetLen && fReturn)
          lpda->lData = (LONG)ulTmp;

       GlobalUnlock((HGLOBAL)m_hBoundData);
    }

    return fReturn;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::VerifyFetchBuffer()
 *
 * DESCRIPTION:
 *
 *   Verifies that the row specified mathces the data in the Fetch buffer.
 *   The row is specified as a Bookmark (HLSTR) or if NULL, then the
 *   current row (of m_lpCursorMove) is used.  If the row does not
 *   match the buffer, then the row is put in the Fetch buffer.
 *
 *   After determining the status of the data in the current fetch buffer,
 *   (if necessary) the position of the Clone is moved to the row BEFORE
 *   the target row and fetched (with GetNextRows()).
 *
 *   NOTE:  The bookmark of the row in the fetch buffer is saved in the
 *          member variable, "m_hlstrFetchBmark", and flags are set in
 *          "m_dwFetchStatus" to indicate "current row" or "bookmark row".
 *
 * PARAMETERS:
 *
 *   hlstrBookmark .... Bookmark (HLSTR) of row to fetch.  If NULL, then
 *                      the current row is assumed.
 *   lpuFlags ......... Returned flags (LPUINT).  Could set DA_fEOF.
 *
 * RETURNS:  
 *
 *   TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::VerifyFetchBuffer( LPVOID hlstrBookmark, 
  UINT FAR *lpuFlags)
{
    BOOL fReturn = TRUE;
    HLSTR hlstrFetchBmark = NULL;
    DWORD dwTmpFetchStatus = (DWORD)0;
    UINT uFlags;

    // If the control is the RDC and we have not determined which version 
    // (because previously there were no rows, which is required to
    // determine the version), then determine it now.
    if (m_fRemoteDataCtl & FPDB_RDC_TEST_VER)
      DetermineRdcVersion();

    // If no bookmark was passed, then we are fetching from 
    // the "Current Row".  Determine if the fetch buffer contains data
    // from the current row.  If not, get current bookmark and use it
    // for the next fetch.
    if (!hlstrBookmark)
    {
       if (m_dwFetchStatus & FLAG_CURR_ROW_INVALID)
       {
          fReturn = FALSE;
       }
       else if ((m_dwFetchStatus & (FLAG_CURR_ROW_MOVED|FLAG_CURR_ROW_DATACHANGED)) ||
                !(m_dwFetchStatus & (FLAG_FETCHED_FROM_CURSOR|FLAG_FETCHED_FROM_CLONE)))
       {
          fReturn = GetBookmark(DATA_BOOKMARKCURRENT, NULL, &hlstrFetchBmark, 
                      &uFlags);  
          ASSERT(fReturn); // GetBookmark FAILED to get current BM or invalid
          if (!fReturn || (uFlags & DA_fEOF))
          {
             return FALSE;
          }
          dwTmpFetchStatus = FLAG_FETCHED_FROM_CURSOR;
       }
    }
    else  // a bookmark was passed, determine if last fetch is OK to use.
    {
       // If we don't have a bmark from the last fetch OR
       // if data changed (it MAY have happen to current fetch buffer) OR
       // if the passed bmark doesn't match the bmark saved from the 
       // last fetch then move the fetch buffer.
       if (!(m_dwFetchStatus & FLAGC_BMARK_FROM_FETCH) ||
            (m_dwFetchStatus & FLAGC_DATACHANGED) ||
           !BookmarksEqual(hlstrBookmark, m_hlstrFetchBmark))
       {
          
          hlstrFetchBmark = (HLSTR)LBlockClone((LBLOCK)hlstrBookmark); 
          dwTmpFetchStatus = FLAG_FETCHED_FROM_BMARK;
       }
    }

    // if a new fetch bmark is set, move Clone to before that record.
    if (hlstrFetchBmark)
    {
      int iNotify;

      // reset these bits to invalidate the current fetch
      m_dwFetchStatus &= ~FLAGC_BMARK_FROM_FETCH;

      // suspend advising
      iNotify = m_iNotifyStatus;
      m_iNotifyStatus &= NOTIFY_SUSPENDED;

      // Get rid of saved bookmark
      if (m_hlstrFetchBmark)
      {
         LBlockFree((LBLOCK)m_hlstrFetchBmark);
         m_hlstrFetchBmark = NULL;
      }

      // Move clone to row BEFORE bookmark
      if (fReturn)
      {
         //$Rdc bug: If RemoteDC, move TO bookmark (not BEFORE it) because
         //          GetNextRow(), actually gets current row (not next). 
         LONG lRowOffset = 
           (m_fRemoteDataCtl & FPDB_RDC_VER1 ? (LONG)0 : (LONG)-1);
         fReturn = MoveCurrentRow( DATA_BOOKMARK, hlstrFetchBmark, &uFlags,
                     lRowOffset, m_lpCloneMoveCursor);
      }
      // if invalid position, set error
      if (fReturn && (uFlags & DA_fEOF))
      {
         *lpuFlags |= DA_fEOF; 
         fReturn = FALSE;
      }
      // Save current fetch bookmark and set flag(s)
      if (fReturn)
      {
         m_hlstrFetchBmark = hlstrFetchBmark;
         m_dwFetchStatus |= dwTmpFetchStatus; // whether curr row or bmark
      }
      else
      {  // something failed--we're not saving the bmark, free it
         LBlockFree((LBLOCK)hlstrFetchBmark);
         hlstrFetchBmark = NULL;
      }

      // Resume notification
      m_iNotifyStatus = iNotify;

    } //if hlstrFetchBmark  

    // if above failed, return error
    if (!fReturn)
       return FALSE;

    // if fetch position moved, fetch current row
    if (hlstrFetchBmark)
    {
      // free any BSTR's from previous fetch
      FreePreviousFetchData();

      // Fetch row from DB
      fReturn = FetchCurrentRow( lpuFlags);
      ASSERT(fReturn);                   // Fetch failed
      if (!fReturn)       // Fetch failed, invalidate the fetch
        m_dwFetchStatus &= ~FLAGC_BMARK_FROM_FETCH;
      else                // Setup performed after fetching row
        PostFetch_Init();
    } // if hlstrFetchBmark

    return fReturn;

} //VerifyFetchBuffer()


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::PostFetch_Init()
 *
 * DESCRIPTION:
 *
 *   Any initialization required after fetching a new row of data.
 *
 * PARAMETERS:
 *
 * RETURNS:  void
 *
 **************************************************************************/
void CfpOleBinding::PostFetch_Init(void)
{
  // Setup performed after fetching row
  CfpDBColumn *pCF;
  LPSTR lpszFetchBuff = (LPSTR)GlobalLock((HGLOBAL)m_hBoundData);
  int  i = 0;

  while (pCF = GetColumnRec(i)) 
  {
    pCF->PostFetchSetup(lpszFetchBuff);
    ++i;
  } // while

  if (m_BmkColInfo)
    m_BmkColInfo->PostFetchSetup(lpszFetchBuff);

  GlobalUnlock((HGLOBAL)m_hBoundData);
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::ReadNextRow()
 *
 * DESCRIPTION:
 *
 *   Used to Read FIRST/NEXT row into the fetch buffer.
 *
 * PARAMETERS:
 *
 *   fReadFirst ....... If reading First row pass TRUE, else pass FALSE.
 *
 *   lpuFlags ......... Returned flags (LPUINT).  Could set DA_fEOF.
 *
 * RETURNS:  
 *
 *   TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::ReadNextRow(BOOL fReadFirst, UINT FAR *lpuFlags)
{
    BOOL fReturn = TRUE;
    UINT uFlags = 0;
    int iNotify;
    LONG lRowOffset;

    // If we are NOT bound to the bookmark column, return FALSE (failed)
    // Since we cannot retrieve the bookmark from the table, "they" must 
    // use the "normal" way of reading: move, read, and repeat.
//  if (!m_BmkColInfo)
//    return FALSE;

    *lpuFlags = 0; 

    // If the control is the RDC and we have not determined which version 
    // (because previously there were no rows, which is required to
    // determine the version), then determine it now.
    // NOTE:  DetermineRdcVersion() will call THIS function (ReadNextRow)
    //        but the following condition is not true because ReadNextRow()
    //        will temporarly change "m_fRemoteDataCtl" so that it does not
    //        contain "FPDB_RDC_TEST_VER".
    if (m_fRemoteDataCtl & FPDB_RDC_TEST_VER)
      DetermineRdcVersion();

    // reset these bits to invalidate the current fetch
    m_dwFetchStatus &= ~FLAGC_BMARK_FROM_FETCH;

    // suspend advising
    iNotify = m_iNotifyStatus;
    m_iNotifyStatus &= NOTIFY_SUSPENDED;

   
    // Get rid of saved bookmark
/*  if (m_hlstrFetchBmark)
    {
       LBlockFree((LBLOCK)m_hlstrFetchBmark);
       m_hlstrFetchBmark = NULL;
    }
*/

    // if FIRST read, move to first row
    if (fReadFirst)
    {
      // Move clone to row BEFORE bookmark
      //$Rdc bug: If RemoteDC, move TO bookmark (not BEFORE it) because
      //          GetNextRow(), actually gets current row (not next). 
      lRowOffset = (m_fRemoteDataCtl & FPDB_RDC_VER1 ? (LONG)0 : (LONG)-1);
      fReturn = MoveCurrentRow( DATA_MOVEFIRST, NULL, &uFlags,
                                lRowOffset, m_lpCloneMoveCursor);
    }

    // if RDC1...
    if (m_fRemoteDataCtl & FPDB_RDC_VER1)
    {
      ULONG ulBmkSize = 0;
      BYTE bBookmark[BMKMAX];
      DBHRESULT h = S_OK;
      LARGE_INTEGER li;

      // if not first read, move current row.
      if (!fReadFirst)
      {
         // NOTE: A bug in RDC1 does not allow us to move DBBMK_CURRENT+1, so
         // instead, we must Get current bmk and move + 1.
         LISet32(li, 1);
  	     h = (DBHRESULT)m_lpCloneMoveCursor->Move( 
           LBlockLen((LBLOCK)m_hlstrFetchBmark), m_hlstrFetchBmark, li, NULL);
      }
      // Get bookmark of "new" current row.
      if (h == S_OK)
      {
        if (m_hlstrFetchBmark)
          LBlockFree((LBLOCK)m_hlstrFetchBmark);
        h = (DBHRESULT)m_lpCloneMoveCursor->GetBookmark(
          (DBCOLUMNID *)&COLUMN_BMKTEMPORARY, BMKMAX, &ulBmkSize, 
            (void *)bBookmark);
        if (h == S_OK)
        {
          m_hlstrFetchBmark = LBlockAllocLen(bBookmark, (LONG)ulBmkSize);
          ASSERT(m_hlstrFetchBmark);
          fReturn = (m_hlstrFetchBmark != NULL);
        }
      }
      fReturn = (fReturn && h == S_OK);
    }

    // if invalid position, set error
    if (fReturn && (uFlags & DA_fEOF))
    {
       *lpuFlags |= DA_fEOF; 
       fReturn = FALSE;
    }
    // Save current fetch bookmark and set flag(s)
    if (fReturn)
       m_dwFetchStatus |= FLAG_FETCHED_FROM_CLONE; // from clone cursor

    // Resume notification
    m_iNotifyStatus = iNotify;

    // if above failed, return error
    if (!fReturn)
       return FALSE;

    // free any BSTR's from previous fetch
    FreePreviousFetchData();

    // Fetch row from DB
    fReturn = FetchCurrentRow( lpuFlags);
    if (!fReturn)       // Fetch failed, invalidate the fetch
    {
      m_dwFetchStatus &= ~FLAGC_BMARK_FROM_FETCH;
      *lpuFlags |= DA_fEOF; 
    }
    else                // Setup performed after fetching row
      PostFetch_Init();

    return fReturn;

} //ReadNextRow()


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::FreePreviousFetchData()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:  void
 *
 **************************************************************************/
void CfpOleBinding::FreePreviousFetchData(void)
{
#if (defined(SPREAD_JPN) || defined(USEVARIANT) || defined(RDCVARIANT))
  // Setup performed after fetching row
  CfpDBColumn *pCF;
  LPSTR lpszFetchBuff = (LPSTR)GlobalLock((HGLOBAL)m_hBoundData);
  int  i = 0;

  if (lpszFetchBuff)
  {
    while (pCF = GetColumnRec(i)) 
    {
      pCF->FreeFetchData(lpszFetchBuff);
      ++i;
    } // while

    if (m_BmkColInfo)
      m_BmkColInfo->FreeFetchData(lpszFetchBuff);

    GlobalUnlock((HGLOBAL)m_hBoundData);
  }
#endif
} // FreePreviousFetchData()


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetFieldValue()
 *
 * DESCRIPTION:
 *
 *   Sets the data for ONE column based on the current row.
 *
 *   It determines the status of the write buffer and if BeginUpdate()
 *   should be called.
 *
 * PARAMETERS:
 *
 *   lpvda ......... Ptr to DATAACCESS structure.
 *
 *   Input fields used from lpvda:
 *
 *     hszDataField  ..... Specifies column name of value.  If NULL, 
 *                         uses "sDataFieldIndex".
 *     sDataFieldIndex ... Specifies index of column of value.
 *     usDataType ........ Format to coerce data into.
 *     lData ............. For returning data. It's either a LONG or a ptr,
 *                         "usDataType" determines which.
 *     fs ................ For returning info about data. "DA_fNull" if 
 *                         field doesn't have a value (null).
 *
 * RETURNS:  TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::SetFieldValue(LPVOID lpvda)
{
    LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
    CfpDBColumn *pCF;
    DBHRESULT h;
    BOOL fCancelUpdate = FALSE;
    DBBINDPARAMS BindParams;
    BOOL fFirstSetColumn = FALSE;
    BOOL fReturn = TRUE;
    UINT uFlags = 0;
	LPSTR lpszFetchBuff;
    BOOL  bMemAllocated;
    BOOL  fSetCol;
	BOOL  fInvokeError = FALSE;

    // indicate that SetColumn was ATTEMPTED (but may have failed)
    m_iUpdateStatus |= UPDSTAT_SETCOLUMN;

    // If DataField is NULL, use DataFieldIndex as the position
    if (lpda->hszDataField)
       pCF = GetColumnRec((LPCTSTR)lpda->hszDataField);
    else
       pCF = GetColumnRec((int)lpda->sDataFieldIndex);

    // if pCF is null, return error
    if (!pCF)
       return FALSE;

    // If BeginUpdate() has not been called, call it.
    if ((m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD)) == 0)
    {
       // Force the Fetch buffer to refresh with the LATEST data,
       // not that we need all of the data, just the EntryId's
       // NOTE: VerifyFetchBuffer() uses only the CloneCursor.
       m_dwFetchStatus |= FLAG_CURR_ROW_DATACHANGED;
       if (!VerifyFetchBuffer( NULL, &uFlags))
       {
          fpSetError( FPDB_ERR_FETCHCURRENTROW, (LONG)0, NULL);
          return FALSE;                 // current row may be invalid
       }

       // Force a "fresh" Update cursor.  NOTE: This function will call
       // fpSetError() if there is a problem.
	   if (!GetUpdateCursor(FORCE_CURSOR_REFRESH))
	      return FALSE;

       h = (DBHRESULT)m_lpUpdateCursor->BeginUpdate(DBROWACTION_UPDATE);
//     ASSERT(h == S_OK);        // BeginUpdate() failed!
       if (h != S_OK)
       {
          fpSetError( FPDB_ERR_BEGINUPDATE_UPDATE, (LONG)h, NULL);
          return FALSE;
       }
       m_iUpdateStatus |= UPDSTAT_BEGIN_EDIT;
       fFirstSetColumn = TRUE;                 // if SetColumn fails, Cancel update
    }

    // Lock the fetch buffer because EntryIds used for reading & writing
    // are stored there.
    lpszFetchBuff = (LPSTR)GlobalLock((HGLOBAL)m_hBoundData);

    // Fill the DBBINDPARAMS structure for this field.  Note, if memory
    // is needed, "pData" will point to memory allocated with LBlockAllocLen()
    memset( &BindParams, 0, sizeof(BindParams));
    if (m_fRemoteDataCtl)
      h = (DBHRESULT)pCF->SetDataValueForRDC( &BindParams, lpda->usDataType, 
                lpda->lData, lpda->fs, m_lpUpdateCursor, &bMemAllocated);
    else
      h = (DBHRESULT)pCF->SetDataValue( &BindParams, lpda->usDataType, 
                lpda->ulChunkOffset, lpda->ulChunkNumBytes,
                lpda->lData, lpda->fs, m_lpUpdateCursor, &bMemAllocated);
    fSetCol = FALSE;
    switch((LONG)h)
    {
      case 0:
        fSetCol = TRUE;
        break;

      case FPDB_ERR_CANTSETAUTOINCR:
	    // If this error occurred, invoke the error at the end of this
		// function, but return that we were successful.
        fpSetError( (LONG)h, (LONG)0, NULL);
		fInvokeError = TRUE;
        break;

      case FPDB_ERR_CONVERTSTRINGTOTYPE:
        break;

      default:
        fpSetError( (LONG)h, (LONG)0, NULL);
        fCancelUpdate = TRUE;
        break;
    }

    // Call SetColumn to move data to cursor's "edit buffer"
    if (fSetCol)
    {
       h = (DBHRESULT)m_lpUpdateCursor->SetColumn( &pCF->m_ColID, &BindParams);
  //   ASSERT(h == S_OK);          // ICursorUpdateARow::SetColumn() failed!
       if (h != S_OK)
       {
          fpSetError( FPDB_ERR_SETCOLUMN, (LONG)h, NULL);
          fCancelUpdate = TRUE;
       }
    }

    // Unlock fetch buffer
    GlobalUnlock((HGLOBAL)m_hBoundData);

    // Free any memory allocated
    if (bMemAllocated && BindParams.pData)
    {
//$JP #if WIN32    
//$JP        LBlockFree((LBLOCK)BindParams.pData);
//$JP #else  
       // if not BSTR, free normally, else free BSTR and ptr to it     
       if (BindParams.dwDataType != VT_BSTR)
          LBlockFree((LBLOCK)BindParams.pData);
       else
       {
       // SCP: It seems that SetColumn() frees the BSTR (in WIN32), I don't 
       // know if this is true in WIN16.
#ifndef WIN32
          // if allocated for VARIANT...
          BSTR *pbstr = (BindParams.dwBinding == DBBINDING_VARIANT ?
            &(((DBVARIANT *)BindParams.pData)->bstrVal) :
            (BSTR *)BindParams.pData);
          if (*pbstr)
             SysFreeString(*pbstr);
#endif  
          LBlockFree((LBLOCK)BindParams.pData);
       }
//$JP #endif          
    }
    // if something failed and first call to "SetColumn()", cancel update
    if (fCancelUpdate && fFirstSetColumn)
    {
       // update cancelled
       h = (DBHRESULT)m_lpUpdateCursor->Cancel();
       m_iUpdateStatus &= ~(UPDSTAT_BEGIN_EDIT | UPDSTAT_SETCOLUMN);
    }

	// If this flag is set, invoke the "external error"
	if (fInvokeError)
      fpInvokeError();

	return !fCancelUpdate;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::AddNewRow()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::AddNewRow(void)
{
    DBHRESULT h;
    int  iSaveStatus;
    ULONG ulBmkSize = 0;
//  LPSTR pBookmark;

    // if in middle of EDIT, return error because ADD is not allowed!
    ASSERT( !(m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))); 
    if (m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))
    {
       fpSetError( FPDB_ERR_UPDATEINPROGRESS, (LONG)0, NULL);
       return FALSE;
    }

    // Force new update cursor.  NOTE: This func calls fpSetError() if
    // there is a problem.
	if (!GetUpdateCursor(FORCE_CURSOR_REFRESH))
	   return FALSE;

    // Turn-off "SyncBefore" Notification
    iSaveStatus = m_iNotifyStatus;
//$Addnew    m_iNotifyStatus |= NOTIFY_SYNCBEFORE_IGNORED;

    // indicate that we are processing an ADD ROW
    m_iUpdateStatus |= UPDSTAT_BEGIN_ADD;

    h = (DBHRESULT)m_lpUpdateCursor->BeginUpdate(DBROWACTION_ADD);
//  ASSERT(h == S_OK);        // BeginUpdate() failed!
    if (h != S_OK)
    {
       fpSetError( FPDB_ERR_BEGINUPDATE_ADD, (LONG)h, NULL);
    }
    else
    {
       h = (DBHRESULT)m_lpUpdateCursor->Update(NULL, NULL, NULL);
       if (h != S_OK)
       {
          (DBHRESULT)m_lpUpdateCursor->Cancel();    //GRB792
          fpSetError( FPDB_ERR_UPDATE, (LONG)h, NULL);
       }
    }

    // Free UpdateCursor                                //GRB792
    m_lpUpdateCursor->Release();                        //GRB792
    m_lpUpdateCursor = NULL;                            //GRB792

    m_iUpdateStatus &= ~UPDSTAT_BEGIN_ADD;

    // Restore notification
    m_iNotifyStatus = iSaveStatus;

    return (h == S_OK);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::AddNewRow()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::Process_AddNew(void)
{
    DBHRESULT h=0; // initialize variables & avoid warning C4700 in VC5 -SCL
    int  iSaveStatus;
    ULONG ulBmkSize = 0;
    int i;
    int ct;
    BOOL fRet = FALSE;
    BOOL fEntryId;

    if (m_fRemoteDataCtl)
    {
       fpSetError( FPDB_ERR_BEGINUPDATE_ADD, (LONG)h, NULL);
       return FALSE;
    }

    // if in middle of EDIT, return error because ADD is not allowed!
    ASSERT( !(m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))); 
    if (m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))
    {
       fpSetError( FPDB_ERR_UPDATEINPROGRESS, (LONG)0, NULL);
       return FALSE;
    }

    // Determine if bound columns include any EntryId's. If there are
    // any, add the new row first then re-read and edit it.
    fEntryId = FALSE;
    i = 0;
    ct = m_arrayBoundCols.GetSize();
    while ((i < ct) && !fEntryId)
    {
      if (m_arrayBoundCols[i])
        if (((CfpDBColumn *)m_arrayBoundCols[i])->m_dwBindTypeUsed 
               == DBBINDING_ENTRYID)
          fEntryId = TRUE;
      ++i;
    }

    // if using EntryId's call other function.
    if (fEntryId || m_fRemoteDataCtl)
    {
      // save notify status, then turn-off SYNC messages
      iSaveStatus = m_iNotifyStatus;
      m_iNotifyStatus |= NOTIFY_SYNCBEFORE_IGNORED | NOTIFY_SYNCAFTER_IGNORED;
      // Add new row
      fRet = AddNewRow();
      // Move to new row
      if (fRet)
      {
        UINT uFlags;
        fRet = MoveCurrentRow( DATA_MOVELAST, NULL, &uFlags);
        if (!fRet)
          fpSetError( FPDB_ERR_MOVETO_NEWROW, (LONG)0, NULL);
        else
          CurrentRowChanged(FLAG_CURR_ROW_INVALID, 0); //clear flag
      }
      // Restore messages
      m_iNotifyStatus = iSaveStatus;
      return fRet;
    }

    // Force new update cursor.  NOTE: This func calls fpSetError() if
    // there is a problem.
	if (!GetUpdateCursor(FORCE_CURSOR_REFRESH))
	   return FALSE;

    // indicate that we are processing an ADD ROW
    m_iUpdateStatus |= UPDSTAT_BEGIN_ADD;

    h = (DBHRESULT)m_lpUpdateCursor->BeginUpdate(DBROWACTION_ADD);
//  ASSERT(h == S_OK);        // BeginUpdate() failed!
    if (h != S_OK)
    {
       // BeginUpdate(ADD) failed, undo ADD
       m_iUpdateStatus &= ~UPDSTAT_BEGIN_ADD;
       // Free UpdateCursor
       m_lpUpdateCursor->Release();
       m_lpUpdateCursor = NULL;
       fpSetError( FPDB_ERR_BEGINUPDATE_ADD, (LONG)h, NULL);
    }

    return (h == S_OK);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::DeleteCurrentRow()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::DeleteCurrentRow(void)
{
    DBHRESULT h;
    int  iSaveStatus;
    ULONG ulSaveCtlFlags;

    // if in middle of EDIT, return error because DELETE is not allowed!
    ASSERT( !(m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))); 
    if (m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))
       return FALSE;

    // Force new update cursor
	if (!GetUpdateCursor(FORCE_CURSOR_REFRESH))
	   return FALSE;

    // Turn-off "SyncBefore" Notification
    iSaveStatus = m_iNotifyStatus;

    // If deleting, save current flags and turn-off DATACHANGED,
    // then if deletion failed, restore flags
    ulSaveCtlFlags = m_ulCtlFlags;
    m_ulCtlFlags &= ~CTLFLG_DATACHANGED;

    h = (DBHRESULT)m_lpUpdateCursor->Delete();
//  ASSERT(h == S_OK);        // Delete() failed!
    if (h != S_OK)
    {
       m_ulCtlFlags = ulSaveCtlFlags;  // delete failed, restore flags
       fpSetError( FPDB_ERR_DELETE, (LONG)h, NULL);
    }
    // Free UpdateCursor
    m_lpUpdateCursor->Release();
    m_lpUpdateCursor = NULL;

    // Restore notification
    m_iNotifyStatus = iSaveStatus;

    return (h == S_OK);
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::FinishUpdatingRow()
 *
 * DESCRIPTION:
 *
 *   This is called to finish the updating of a row.  A BeginUpdate was
 *   started and we need to finish it.  Here's the calling sequence for
 *   ICursorUpdateARow:
 *
 *     BeginUpdate(), SetColumn(), Update()/Cancel()
 *
 *   After calling Update(), the UpdateCursor is released.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::FinishUpdatingRow(void)
{
    DBHRESULT h;

    // If Update was not started, just return
    if (!(m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD)))
       return FALSE;

    // Update the row
    h = (DBHRESULT)m_lpUpdateCursor->Update(NULL, NULL, NULL);
//  ASSERT(h == S_OK);  // ICursorUpdateARow::Update failed!!!
    if (h != S_OK)
    {
       m_lpUpdateCursor->Cancel();       //GRB792
       fpSetError( FPDB_ERR_UPDATE, (LONG)h, NULL);
    }  
    // Free UpdateCursor
    m_lpUpdateCursor->Release();
    m_lpUpdateCursor = NULL;

    // Reset flags
    m_iUpdateStatus &= 
      ~(UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD | UPDSTAT_SETCOLUMN);

    return (h == S_OK);
} //FinishUpdatingRow()




/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetBoundColumn()
 *
 * DESCRIPTION:
 *
 *   Bind to column based on "DataField" ID (column ID).
 *   (uses list of columns in m_arrayBoundCols)
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::SetBoundColumn( void)
{
	DBCOLUMNBINDING *arrayOfColBind = NULL;
    ULONG ulDataLen = 0;
    ULONG obInline  = 0;
    ULONG obOutline = 0;

    int iBoundArrayCt;
	DBHRESULT h;
    int ct = 0;
    BOOL fReturn = TRUE;

    iBoundArrayCt = m_arrayBoundCols.GetSize();

    if (iBoundArrayCt > 0)
    {
      // Build array of DBCOLUMNBINDING's
      int i;

      // Allocate memory for array
      arrayOfColBind = new DBCOLUMNBINDING[iBoundArrayCt];

      // fill array
      ct = 0;
      for (i = 0; i < iBoundArrayCt; ++i)      
        if (m_arrayBoundCols[i])
        {
#ifdef RDCVARIANT
          if (m_fRemoteDataCtl)
            ((CfpDBColumn *)m_arrayBoundCols[i])->FillColBindDbVariant( 
			  &arrayOfColBind[ct], &obInline, &obOutline);
		  else
#endif
            ((CfpDBColumn *)m_arrayBoundCols[i])->FillColBindRec( &arrayOfColBind[ct], 
              &obInline, &obOutline, m_fRemoteDataCtl, m_fUseEntryId);
          ++ct;
        }
    }
       
    // set minimum buffer sizes
    if (obInline == 0)
       obInline = 4;               
    // NOTE:  For WIN16, SetBindings() wants some extra bytes (2 for most
    // cases) but I don't know why!  - SCP 1/12/96
    obInline += 10 + (2 * iBoundArrayCt);  // Make extra bytes proportional 
                                           // to the number of columns.
    if (obOutline == 0)
       obOutline = 4;
    // Allocate/reallocate buffer for data
    ulDataLen = obInline + obOutline;
    if (!m_hBoundData ||
        (ulDataLen > (m_ulBoundInlineLen + m_ulBoundOutlineLen)))
    {
      if (m_hBoundData)
         GlobalReAlloc((HGLOBAL)m_hBoundData, ulDataLen, GMEM_MOVEABLE);
      else
         m_hBoundData = (DWORD)(LPVOID)GlobalAlloc(GHND, ulDataLen);

      ASSERT(m_hBoundData);               // Did memory allocate?
      if (!m_hBoundData)
         fReturn = FALSE;
      else
      {
         m_ulBoundInlineLen = obInline;   
         m_ulBoundOutlineLen = obOutline; 
      }
    }

    if (fReturn)
    {
	   h = (DBHRESULT)m_lpMoveCursor->SetBindings(ct, arrayOfColBind, 
	         m_ulBoundInlineLen, DBCOLUMNBINDOPTS_REPLACE);
	   if (h != S_OK)
	   	  fReturn = FALSE;
    }

    // If not already cloned, clone cursor
    if (!m_lpCloneMoveCursor && fReturn)
    {
      h = (DBHRESULT)m_lpMoveCursor->Clone(DBCLONEOPTS_DEFAULT,
      	  				 	 IID_ICursorMove,
      						 (IUnknown **)&m_lpCloneMoveCursor);
      if ((h != S_OK) || !m_lpCloneMoveCursor)
         fReturn = FALSE;
    }
    // bind clone
    if (fReturn)
    {
	   h = (DBHRESULT)m_lpCloneMoveCursor->SetBindings(ct, arrayOfColBind, 
	         m_ulBoundInlineLen, DBCOLUMNBINDOPTS_REPLACE);
	   if (h != S_OK)
	   	  fReturn = FALSE;
	}

    if (arrayOfColBind)
       delete arrayOfColBind;


    // if RemoteDataCtl and version undetermined, try to determine version
    if (m_fRemoteDataCtl & FPDB_RDC_TEST_VER)
      DetermineRdcVersion();

	return fReturn;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetBookmark()
 *
 * DESCRIPTION:
 *
 *   Get the bookmark at the specified position (first, last, current, next).
 *
 * PARAMETERS:
 *
 *   INPUT Parameters:
 *
 *     lBmarkType ........ Type (position) of Bookmark to get. One of the
 *                         following: DATA_BOOKMARKFIRST, DATA_BOOKMARKLAST,
 *                         ...CURRENT, ...PREV, ...NEXT
 *     hlstrBmark ........ Bookmark to use with PREV/NEXT.  
 *                         Required for PREV/NEXT.
 * 
 *   OUTPUT Parameters:
 * 
 *     lphlstrRetBmark ... Returned Bookmark if no error.
 *     lpuRetFlags ....... Returned flags. Either DA_fBOF or DA_fEOF.
 *
 * RETURNS:  
 *
 *   TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::GetBookmark( LONG lBmarkType, LPVOID hlstrBmark, 
  LPVOID FAR *lphlstrRetBmark, UINT FAR *lpuRetFlags, BOOL fUseClone)
{
	DBHRESULT h;
	LARGE_INTEGER li;
	ULONG ulBmkSize = 0;
	BYTE bBookmark[BMKMAX] = {0};
    LONG lRelOffset = 0;              // Relative offset position for move
    UINT uFlags = 0;                  // Temp flags to return
    HLSTR hlstrTmpBmark = NULL;       // Temp Bmark to return
    ICursorMove FAR *lpRefCursor = 
      (fUseClone ? m_lpCloneMoveCursor : m_lpMoveCursor);

    *lphlstrRetBmark = NULL;
    *lpuRetFlags = 0;
    
    if (!lpRefCursor)
      return FALSE;

    // Act on type of Bookmark to get (from lpda->lData)
    switch (lBmarkType)
    {
	  case DATA_BOOKMARKFIRST:
        _fmemcpy( &bBookmark[0], &DBBMK_BEGINNING, sizeof(DBBMK_BEGINNING));
        ulBmkSize = sizeof(DBBMK_BEGINNING);
        lRelOffset = 1;
        break;

      case DATA_BOOKMARKCURRENT:
 	    // Get the bookmark at the cursor's current position.
        // If using Clone and RDC1 and have current bookmark, just copy it.
        if (fUseClone && (m_fRemoteDataCtl & FPDB_RDC_VER1) && m_hlstrFetchBmark)
        {
          ulBmkSize = LBlockLen((LBLOCK)m_hlstrFetchBmark);
          _fmemcpy( bBookmark, m_hlstrFetchBmark, (size_t)ulBmkSize);
          h = S_OK;
        }
        else // get bookmark from Cursor.
        {
          h = (DBHRESULT)lpRefCursor->GetBookmark((DBCOLUMNID *)&COLUMN_BMKTEMPORARY,
		    BMKMAX, &ulBmkSize, (void *)bBookmark);
          if (h != S_OK)
		    return FALSE;
        }
        // set offset relative to current bookmark
        lRelOffset =  0;
        break;

	  case DATA_BOOKMARKNEXT:
	  case DATA_BOOKMARKPREV:
	    if (!hlstrBmark)              // parameter is required for next/prev
	       return FALSE;
        ulBmkSize = LBlockLen((LBLOCK)hlstrBmark);
        _fmemcpy( &bBookmark[0], hlstrBmark, (size_t)ulBmkSize);
        if (lBmarkType == DATA_BOOKMARKNEXT)
           lRelOffset =  1;           // next (after bookmark passed-in)
        else
           lRelOffset = -1;           // previous (before bookmark passed-in)
        break;

      case DATA_BOOKMARKLAST:
        _fmemcpy( &bBookmark[0], &DBBMK_END, sizeof(DBBMK_END));
        ulBmkSize = sizeof(DBBMK_END);
        lRelOffset = -1;
        break;

      default: 
       // Invalid parameter in "lpvda->lData", must be one of above values.
       ASSERT(0);
       return FALSE;
    } //switch

    // If relative offset is not zero, move Clone cursor there and 
    // get bookmark.
	if (lRelOffset != 0)
	{
        LISet32(li, lRelOffset);
		// Move to the next row following the last bookmark.
		h = (DBHRESULT)m_lpCloneMoveCursor->Move(ulBmkSize,
								   (void FAR *)&bBookmark[0],
								   li,
								   NULL);
        if (h == DB_S_ENDOFCURSOR)
        {
           // ENDOFCURSOR is True. We are either at the END or the BEGINNING,
           // determine which. 
           if (lBmarkType == DATA_BOOKMARKNEXT)
			  uFlags = DA_fEOF;
           else if (lBmarkType == DATA_BOOKMARKPREV)
			  uFlags = DA_fBOF;
        }
		else if (h != S_OK)
		{
			return FALSE;
		}

  	    // Get the bookmark.
	    h = (DBHRESULT)m_lpCloneMoveCursor->GetBookmark((DBCOLUMNID *)&COLUMN_BMKTEMPORARY,
				   					 BMKMAX,
								 	 &ulBmkSize,
								   	 (void *)bBookmark);
	    if (h != S_OK)
		   return FALSE;
	} // if NEXT

    hlstrTmpBmark = LBlockAllocLen(bBookmark, (LONG)ulBmkSize);

    // determine if we are at BOF or EOF
    if ((uFlags == 0) && IsBeginningBookmark(hlstrTmpBmark))
	  uFlags = DA_fBOF;
    else if ((uFlags == 0) && IsEndBookmark(hlstrTmpBmark))
	  uFlags = DA_fEOF;
    
    // Set return parameters
    *lpuRetFlags = uFlags;
    *lphlstrRetBmark = hlstrTmpBmark;

	return TRUE;
}  //GetBookmark()

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::MoveCurrentRow()
 *
 * DESCRIPTION:
 *
 *   Moves to a new position and makes it the current row.
 *
 *   NOTE 1: Bookmarks can be used to set the current record (row), however,
 *           "fetching" the row at the current record actually fetches
 *           the NEXT ROW (the function for fetching is "GetNextRows()").
 *
 *   NOTE 2: If YOU move m_lpMoveCursor, YOU must set the following bit:
             "m_dwFetchStatus |= FLAG_CURR_ROW_MOVED".
 *
 * PARAMETERS:
 *
 *   INPUT Parameters:
 *
 *     usMoveAction .... Type of move action.
 *     hlstrBmark ...... If type is DATA_BOOKMARK, then this is that
 *                       bookmark.  Otherwise NULL.
 *     lRowOffset ...... Offset from the chosen action/bookmark combination.
 *                       May be positive OR negative. Defaults to 0.
 *     lpMoveCursor .... Cursor to move. Can be either m_lpMoveCursor OR
 *                       m_lpCloneMoveCursor, the former being the default.
 *
 *   OUTPUT Parameters:
 *
 *     lpuRetFlags ..... Returned flags, either DA_fBOF or DA_fEOF.
 *
 * RETURNS:  
 *
 *   TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::MoveCurrentRow(USHORT usMoveAction, LPVOID hlstrBmark,
  UINT FAR *lpuRetFlags, LONG lRowOffset, ICursorMove FAR *lpMoveCursor)
{
//	ULONG ulBmkSize = 0;
//	BYTE bBookmark[BMKMAX] = {0};
	DBHRESULT h;
	LARGE_INTEGER li;
    LONG lAdjustOffset;               // Relative offset position for move
    HLSTR hlstrTmp;                   // tmp bookmark for moving to
    BOOL fDestoryTmp = FALSE;         // whether to destroy hlstrTmp
    BOOL fReturn = TRUE;         
    LONG lBmarkType = 0;

    *lpuRetFlags = 0;

    // if cursor was not passed, use default
    if (!lpMoveCursor)
       lpMoveCursor = m_lpMoveCursor;

    // Act on type of move
    // Get the Bookmark for moving (gets put in hlstrTmp)
    // NOTE: If hlstrTmp is still NULL after this switch, then get bookmark.
    lAdjustOffset = 0;                // this may get changed
    hlstrTmp = NULL;
    switch (usMoveAction)             
    {                                 
      case DATA_BOOKMARK:
        // *** SPECIAL VB3 FEATURE: ***
        // Setting the Bookmark to 0, causes the cursor's current position
        // to be invalid (as if you deleted the current row).
        // To simulate this, we set the following flag:
        if (!hlstrBmark)
        {
           //ARR94...
           // If status changed AND we are using the actual MoveCursor AND
           // we are processing DBEvents, simulate a DBREASON_MOVE.
           if (!(m_dwFetchStatus & FLAG_CURR_ROW_INVALID) && 
               (lpMoveCursor == m_lpMoveCursor) && m_iNotifyStatus)
           {
             DBNOTIFYREASON rgReasons[2] = {0} ;
             rgReasons[0].dwReason = DBREASON_MOVE;
             Simulate_NotifyDBEvents( DBEVENT_CURRENT_ROW_CHANGED,
              (ULONG)1, rgReasons);
           }
           //ARR94.
           CurrentRowChanged(FLAG_CURR_ROW_INVALID, FLAG_CURR_ROW_INVALID);
           return TRUE;
        }
        else
        {
           hlstrTmp = hlstrBmark; // Use this bookmark (don't get one)
        }
        break;

	  case DATA_MOVEFIRST:
        lBmarkType = DATA_BOOKMARKFIRST;
        hlstrTmp = LBlockAllocLen((const LPBYTE)&DBBMK_BEGINNING, 
          (long)sizeof(DBBMK_BEGINNING));
        fDestoryTmp = TRUE;            // destroy this bookmark before return
        lAdjustOffset = 1;             // after beginning position
        break;

	  case DATA_MOVELAST:
        lBmarkType = DATA_BOOKMARKLAST;
        hlstrTmp = LBlockAllocLen((const LPBYTE)&DBBMK_END, 
          (long)sizeof(DBBMK_END));
        fDestoryTmp = TRUE;            // destroy this bookmark before return
        lAdjustOffset = -1;            // before last position
        break;

	  case DATA_MOVEPREV:
        lBmarkType = DATA_BOOKMARKCURRENT;
        lAdjustOffset = -1;             // before current position
        break;

	  case DATA_MOVENEXT:
        lBmarkType = DATA_BOOKMARKCURRENT;
        lAdjustOffset = 1;              // after current position
        break;

      default: 
       // Invalid parameter passed, must be one of above values.
       ASSERT(0);
       return FALSE;
    } //switch

    // If we don't have a bookmark, retrieve one
    if (!hlstrTmp)
    {  
       // get Bmark based on settings from above
       if (!GetBookmark( lBmarkType, NULL, &hlstrTmp, lpuRetFlags))
          return FALSE;
       fDestoryTmp = TRUE;            // destroy this bookmark before return
    }

    // Move the current record
    LISet32(li, (lRowOffset + lAdjustOffset));
  	h = (DBHRESULT)lpMoveCursor->Move( LBlockLen((LBLOCK)hlstrTmp), 
  	      hlstrTmp, li, NULL);
    // Is there an error?
	if ((h != S_OK) && (h != DB_S_ENDOFCURSOR))
    {
//     ASSERT(0);          // Move returned error!!!
	   fReturn = FALSE;
    }
    // determine if we are at BOF or EOF
    if (fReturn && (h == DB_S_ENDOFCURSOR))
    {
       switch (usMoveAction)
       {
         case DATA_MOVELAST:
         case DATA_MOVENEXT:
	       *lpuRetFlags |= DA_fEOF;
           break;
         case DATA_MOVEFIRST:
         case DATA_MOVEPREV:
	       *lpuRetFlags |= DA_fBOF;
           break;
                                      // if offset is -1, then BOF
         case DATA_BOOKMARK:          // compare bookmark with ends
           if ((lRowOffset + lAdjustOffset) < 0)
          	  *lpuRetFlags |= DA_fBOF;
           else if ((lRowOffset + lAdjustOffset) > 0)   
          	  *lpuRetFlags |= DA_fEOF;
           else
           {
              if (IsBeginningBookmark(hlstrTmp))
	             *lpuRetFlags |= DA_fBOF;
              else if (IsEndBookmark(hlstrTmp))
	             *lpuRetFlags |= DA_fEOF;
           }
           break;
       }  //switch
    }  // h == DB_S_ENDCURSOR

    // Destroy hlstr if instructed to
    if (hlstrTmp && fDestoryTmp)
       LBlockFree((LBLOCK)hlstrTmp);

	return TRUE;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::CurrentRowChanged()
 *
 * DESCRIPTION:
 *
 *   Called by XNotifyDBEvents to notify us that the current row
 *   or its position has changed.  We then update our flags on the status
 *   of the current row. 
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::CurrentRowChanged(DWORD dwMask, DWORD dwFlags)
{
  // reset flags related to current row
  m_dwFetchStatus &= ~dwMask;
  m_dwFetchStatus |= (dwFlags & dwMask);

  // if flags didn't include CurrRowInvalid and currently it is invalid, 
  // see if something changed to make it valid.
  if (!(dwMask & FLAG_CURR_ROW_INVALID) && 
      (m_dwFetchStatus &  FLAG_CURR_ROW_INVALID))
     if (dwFlags & (FLAG_CURR_ROW_MOVED | FLAG_CURR_ROW_DATACHANGED))
        m_dwFetchStatus &= ~FLAG_CURR_ROW_INVALID; // reset bit
}

/***************************************************************************
 *
 * FUNCTION:  BookmarksEqual()
 *
 * DESCRIPTION:
 *
 *   If either bookmark (or both) is NULL, FALSE is returned.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if they are equal, else FALSE.
 *
 **************************************************************************/
static BOOL BookmarksEqual( HLSTR b1, HLSTR b2)
{
  LONG l1 = 0;
  LONG l2 = 0;
  BOOL fReturn = FALSE;

  if (b1)
     l1 = LBlockLen((LBLOCK)b1);
  if (b2)
     l2 = LBlockLen((LBLOCK)b2);

  // if both lengths are not 0 and are equal, compare bytes 
  if (l1 && l2 && (l1 == l2))
    fReturn = (_fmemcmp( (LPSTR)b1, (LPSTR)b2, (size_t)l1) == 0);

  return fReturn;
}


/***************************************************************************
 *
 * FUNCTION:  IsBeginningBookmark()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if Bookmark matches DBBMK_BEGINNING, else FALSE.
 *
 **************************************************************************/
static BOOL IsBeginningBookmark( HLSTR b1)
{
  if (!b1)
     return FALSE;
    
  if (LBlockLen((LBLOCK)b1) != sizeof(DBBMK_BEGINNING))
     return FALSE;
     
  return (_fmemcmp( (LPSTR)b1, &DBBMK_BEGINNING, sizeof(DBBMK_BEGINNING)) == 0);
}

/***************************************************************************
 *
 * FUNCTION:  IsEndBookmark()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if Bookmark matches DBBMK_END, else FALSE.
 *
 **************************************************************************/
static BOOL IsEndBookmark( HLSTR b1)
{
  if (!b1)
     return FALSE;
    
  if (LBlockLen((LBLOCK)b1) != sizeof(DBBMK_END))
     return FALSE;
     
  return (_fmemcmp( (LPSTR)b1, &DBBMK_END, sizeof(DBBMK_END)) == 0);
}

/***************************************************************************
 *
 * FUNCTION:  IsInvalidBookmark()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if Bookmark matches DBBMK_END, else FALSE.
 *
 **************************************************************************/
static BOOL IsInvalidBookmark( HLSTR b1)
{
#if VC8
  DBBOOKMARK invalid = DBBMK_INVALID;

  if (!b1)
     return FALSE;
    
  if (LBlockLen((LBLOCK)b1) != sizeof(DBBOOKMARK))
     return FALSE;
     
  return (_fmemcmp( (LPSTR)b1, (void*)&invalid, sizeof(DBBOOKMARK)) == 0);
#else
  if (!b1)
     return FALSE;
    
  if (LBlockLen((LBLOCK)b1) != sizeof(DBBMK_INVALID))
     return FALSE;
     
  return (_fmemcmp( (LPSTR)b1, &DBBMK_INVALID, sizeof(DBBMK_INVALID)) == 0);
#endif
}


/***************************************************************************
 *
 * FUNCTION:  CreateHszFromWCHAR()
 *
 * DESCRIPTION:
 *
 *   Creates a BSTR and copies WCHAR (WORD char string) to it.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   Ptr to created HSZ (BSTR - SysAllocString), else NULL
 *
 **************************************************************************/
/*
static HSZ CreateHszFromWCHAR( WCHAR FAR *wsz)
{
  int ct;
  UCHAR FAR *hsz = NULL;
  register int i;
  register WCHAR FAR *wszreg;
  register UCHAR FAR *hszreg;


  if (!wsz)
     return NULL;

  // Note: We loop twice, once to get the char count, and once to 
  //       copy the chars.  It may be more efficient to allocate a 
  //       buffer, loop once to copy the chars, and then resize, but
  //       how big do you make the buffer?
  i = 0;
  wszreg = wsz;
  while (*wszreg)
  {  
    ++i;
    ++wszreg;
  }
  ct = i;
  
  hsz = (UCHAR FAR *)fpDBCreateHszLen( NULL, (UINT)ct);
  if (hsz)
  {
    wszreg = wsz;
    hszreg = hsz;
    ++ct;                     // ct + 1 so that the null gets copied
    for (i = 0; i < ct; ++i)   
      *hszreg++ = (UCHAR)*wszreg++;
  }

  return (HSZ)hsz;
}
*/

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::Simulate_NotifyDBEvents()
 *
 * DESCRIPTION:
 *
 *   Simulate the notification of these messages by calling the
 *   "XNotifyDBEvents" functions.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::Simulate_NotifyDBEvents(
  DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[])
{
  // OKToDo checks with the control to see if the actions is "Ok to do".
  // If not, the "Cancelled()" is then called to others who said "OK",
  // BUT in simulation, WE are the only control who gets the message.
  if (INTERFACE_(NotifyDBEvents)OKToDo(dwEventWhat, cReasons, rgReasons) == S_OK)
  {
    INTERFACE_(NotifyDBEvents)SyncBefore(dwEventWhat, cReasons, rgReasons);
    INTERFACE_(NotifyDBEvents)AboutToDo(dwEventWhat, cReasons, rgReasons);
    INTERFACE_(NotifyDBEvents)SyncAfter( dwEventWhat, cReasons, rgReasons);
    INTERFACE_(NotifyDBEvents)DidEvent(dwEventWhat, cReasons, rgReasons);
  }
}


// Utilities functions
/////////////////////////////////////////////////////////////////////////////

// The following should be moved to Owner's .CPP file
//BEGIN_INTERFACE_MAP(CfpOleBinding, COleControl)
//    INTERFACE_PART(CfpOleBinding, IID_IBoundObject, BoundObject)
//END_INTERFACE_MAP()


HRESULT CfpOleBinding::XBound_OnSourceChanged(DISPID dispid, BOOL fBound, 
  BOOL FAR *lpfOwnXferOut)
{
    DBNOTIFYREASON rgReasons[2] = {0} ;
    int  ct;
    BOOL fInitialBinding;
    BOOL fMetaDataValid;

	if (dispid != m_dispid)
	   return (HRESULT)E_INVALIDARG;

    // I'm told by Microsoft (Rick Nasci) that this should always be set to
    // true. -SCP 4/17/97
    *lpfOwnXferOut = TRUE;

    // if fBound is TRUE, then Binding/Refresh, else closing connection
    if (fBound)
    { 
       fInitialBinding = TRUE;
       fMetaDataValid = FALSE;

       // If Re-binding, free the old binding
       if (m_lpCursor)
       {
          ReleaseDataSource();
          fInitialBinding = FALSE;
       }
       // Get new cursor
	   GetCursor();

#ifdef SS40
     if( DataManager_GetAmbientUserMode(m_lpOwner) )
#else
       if (m_lpOwner->GET_AMBIENT_USERMODE())
#endif
          AdviseDBEvents();                   

	   // First, get new list of column names
       if (m_lpCursor)
          fMetaDataValid = GetMetaData(NULL, NULL);

       // if Runtime-mode, "Bind" to Column Name and Turn-ON INotifyDBEvents
#ifdef SS40
     if( DataManager_GetAmbientUserMode(m_lpOwner) )
#else
       if (m_lpOwner->GET_AMBIENT_USERMODE())
#endif
       {
          BindDataFieldColumn(m_strBoundDataField);
//        AdviseDBEvents();                   
          if (m_iNotifyStatus && fMetaDataValid)
          {
             // Simulate INIT_CONNECT & REFRESH notification msgs
             ct = 0;
             if (fInitialBinding)
                rgReasons[ ct++ ].dwReason = UDBREASON_INIT_CONNECT;
             rgReasons[ ct++ ].dwReason = UDBREASON_SOURCECHANGED;
             INTERFACE_(NotifyDBEvents)SyncAfter( DBEVENT_CURRENT_ROW_CHANGED,
               (ULONG)ct, rgReasons);
          }
       }
    }
    else
    {
       // DB Binding is going away.  Notify OLE control and release
       // ALL connections and cursors for DataSource.
       if (m_iNotifyStatus)
       {
          // Simulate DISCONNECT notification msg
          rgReasons[0].dwReason = UDBREASON_DISCONNECT;
          Simulate_NotifyDBEvents( DBEVENT_CURRENT_ROW_CHANGED,
            (ULONG)1, rgReasons);
       }
       ReleaseDataSource();
    }
	
	return (HRESULT)NOERROR;
}

HRESULT CfpOleBinding::XBound_IsDirty(DISPID dispid)
{
	if (dispid == m_dispid && m_lpCursor)
	{
      // if something changed, return S_OK, else return S_FALSE
// Always return FALSE, we will use the NotifyDBEvents to update
//      if (m_ulCtlFlags & CTLFLG_DATACHANGED)
//	    return S_OK;
//      else
		
        ASSERT(m_DBEventList->Add("DT", (ULONG)1, (LPVOID)NULL));
	    return (HRESULT)S_FALSE;
	}
    
	return (HRESULT)E_INVALIDARG;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetDataField()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BSTR CfpOleBinding::GetDataField() 
{
   return (BSTR)fpSysAllocBSTRFromTString(m_strBoundDataField);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetDataField()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::SetDataField(LPCTSTR lpszNewValue) 
{
/*
  CString tmpCString(lpszNewValue);

  // VB calls this with "" after user selects field from DataField list.
  // ignore this call. (set only in Design-mode)
  if (m_fIgnoreSetDataField)
  {
     m_fIgnoreSetDataField = FALSE;
     if (tmpCString.GetLength() == 0)
        return;
  }
*/
  // Save DataField in member variable
  if (m_strBoundDataField)
    fpSysFreeTString(m_strBoundDataField);

  m_strBoundDataField = lpszNewValue ? fpSysAllocTString(lpszNewValue) : NULL;

  // If Runtime and we have a cursor, bind to DataField column.
#ifdef SS40
  if( DataManager_GetAmbientUserMode(m_lpOwner) && m_lpCursor)
#else
  if (m_lpOwner && m_lpOwner->GET_AMBIENT_USERMODE() && m_lpCursor)
#endif
  {
     if (BindDataFieldColumn(m_strBoundDataField))
     {
        // If Binding was successful, send notification to control that
        // it needs to refresh.
        DBNOTIFYREASON rgReasons[1] = {0} ;

        rgReasons[0].dwReason = UDBREASON_SELF_REFRESH;
        INTERFACE_(NotifyDBEvents)SyncAfter( DBEVENT_CURRENT_ROW_CHANGED,
          (ULONG)1, rgReasons);
     }
  }
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetListOfColumnNames()
 *
 * DESCRIPTION:
 *
 *   Typically, this function is called by the control's function,
 *   OnGetPredefinedStrings(), when VB's Browser asks for a list of columns.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
#ifndef NO_MFC  // MFC

BOOL CfpOleBinding::GetListOfColumnNames(CStringArray *pStringArray,
  CDWordArray *pCookieArray)
{
  return PrivGetListOfColNames(pStringArray, pCookieArray);
}

#else  // NO_MFC

HRESULT CfpOleBinding::GetListOfColumnNames(CALPOLESTR *pCaStringsOut,
      CADWORD *pCaCookiesOut)
{
  CStringArray stringArray;
  CDWordArray  cookieArray;
  HRESULT hResult = S_OK;
  int i=0;

  // Containers will call with NULL arguments
  // to determine if the control supports browsing
  // for this property. Returning S_OK for this 
  // case signifies YES
  if (pCaStringsOut == NULL || pCaCookiesOut == NULL)
    return S_OK;  // E_POINTER

  pCaStringsOut->cElems = 0;
  pCaStringsOut->pElems = NULL;
  pCaCookiesOut->cElems = 0;
  pCaCookiesOut->pElems = NULL;

  // Copy strings & cookies from lists to parameters
  if (PrivGetListOfColNames(&stringArray, &cookieArray))
  {
    int ct = stringArray.GetSize();
    LPOLESTR lpOleStr;

    // alloc memory for lists
    pCaStringsOut->pElems = (LPOLESTR*)CoTaskMemAlloc(
		sizeof(LPOLESTR) * ct);
    if (pCaStringsOut->pElems == NULL)
      hResult = E_OUTOFMEMORY;
    else
    {
	  pCaCookiesOut->pElems = (DWORD*)CoTaskMemAlloc(
		sizeof(DWORD*) * ct);
      if (pCaCookiesOut->pElems == NULL)
        hResult = E_OUTOFMEMORY;
      else
      {
        pCaStringsOut->cElems = ct;
        pCaCookiesOut->cElems = ct;
      }
    }

    // loop thru strings creating copies and putting in list
    while (i < ct && hResult == S_OK)
    {
#ifdef SS40
      lpOleStr = DataManager_CoTaskCopyString(stringArray.ValueAt(i));
#else
      lpOleStr = fpAtlCoTaskCopyString(stringArray.ValueAt(i));
#endif
      if (lpOleStr == NULL)
        hResult = E_OUTOFMEMORY;
      else
      {
        pCaStringsOut->pElems[i] = lpOleStr;
        pCaCookiesOut->pElems[i] = cookieArray.ValueAt(i);
        ++i;
      }
    }
  } // if PrivGetListOfColNames()

  // If any errors, free any strings which were allocated.
  if (hResult == E_OUTOFMEMORY)
  {
    if (pCaStringsOut->pElems)
    {
      while (--i >= 0)
        CoTaskMemFree(pCaStringsOut->pElems[i]);
      CoTaskMemFree(pCaStringsOut->pElems);
    }
    if (pCaCookiesOut->pElems)
      CoTaskMemFree(pCaCookiesOut->pElems);
  }

  return (hResult);

}
#endif // ifn-else NO_MFC

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::PrivGetListOfColNames()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::PrivGetListOfColNames(CStringArray *pStringArray,
      CDWordArray *pCookieArray)
{
  CfpDBColumn *pCF;
  int i;
  BOOL    fRet = FALSE;
  HRESULT h = (HRESULT)NOERROR;
  BOOL    fDummy = TRUE; 


  // if Design mode (not User mode), force latest Cursor (DataSource)
#ifdef SS40
  if( !DataManager_GetAmbientUserMode(m_lpOwner) )
#else
  if (!m_lpOwner->GET_AMBIENT_USERMODE())
#endif
  {
     // NOTE:  In Design-mode, we are NOT notified when the DataSource
     // changes, but the only time WE CARE is when the function,
     // OnGetPredefinedStrings(), is called by VB's browser.  
     // This function should be called from OnGetPredefinedStrings()
     // to get the list of columns for the current DataSource.
     //
     
     h = XBound_OnSourceChanged(m_dispid, TRUE, &fDummy);
  }

  // if we have a cursor, Add Column names & cookies to arrays
  if ((h == (HRESULT)NOERROR) && m_lpCursor)
  {
     fRet = TRUE;
     i = 0; 
     while ((pCF = GetColumnRec(i)) != NULL)
     {
        ++i;                       // record num (starting at 1)
        if (pStringArray)
  	      pStringArray->Add(pCF->m_strDataField);
    	   if (pCookieArray)
    	      pCookieArray->Add(i);   // cookies are index + 1
     } //while
  } //if h

  return fRet;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetColumnNameForCookie()
 *
 * DESCRIPTION:
 *
 *   Typically, this function is called by the control's function,
 *   OnGetPredefinedValue(), when VB's Browser asks for a string value
 *   for the cookie.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::GetColumnNameForCookie( DWORD dwCookie, 
  BSTR FAR *lpRetBSTR)
{
  BOOL fRet = FALSE;
  CfpDBColumn *pCF = NULL;

  // Since cookie is Index + 1, cookie must be greater than 0
  if ((int)dwCookie > 0)
     pCF = GetColumnRec( ((int)dwCookie) - 1 );

  if (pCF != NULL)
  {
     *lpRetBSTR = (BSTR)fpSysAllocBSTRFromTString(pCF->m_strDataField);
     fRet = TRUE;
  }

  return fRet;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::DetermineRdcVersion()
 *
 * DESCRIPTION:
 *
 *   This function determines which Remote data control we are bound to.
 *   It does that by testing for a bug which is in Ver 1 but not Ver 2.
 *   
 *   The bug is that GetNextRows() should read the next row, but in version 1
 *   it reads the current row.  By setting the current row on the bookmark
 *   "BMK_BEGINNING", normally GetNextRows() would read the first row of
 *   the table, but version 1 returns an error.  NOTE: This does not work
 *   if there are no rows to read.
 *
 *   The results affect the member variable "m_fRemoteDataCtl".
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::DetermineRdcVersion(void)
{
  int iSaveRdc;
  int fResult;
  UINT uFlags;
  HLSTR hlstrTmpBmark;
  int iNotify;

  if (!(m_fRemoteDataCtl & FPDB_RDC_TEST_VER))
    return;

  iSaveRdc = m_fRemoteDataCtl;
  m_fRemoteDataCtl = FPDB_RDC | FPDB_RDC_VER2; // Assume version 2 for test.

  // get bookmark for beginning of table.
  hlstrTmpBmark = LBlockAllocLen((const LPBYTE)&DBBMK_BEGINNING, (LONG)sizeof(DBBMK_BEGINNING));
  ASSERT(hlstrTmpBmark);
  if (!hlstrTmpBmark)
  {
    m_fRemoteDataCtl = iSaveRdc;
    return;
  }

  // suspend advising
  iNotify = m_iNotifyStatus;
  m_iNotifyStatus &= NOTIFY_SUSPENDED;

  // read row.
  fResult = MoveCurrentRow( DATA_BOOKMARK, hlstrTmpBmark, &uFlags, 0L,
     m_lpCloneMoveCursor);
  if (fResult && !(uFlags & DA_fEOF))
  {
    fResult = ReadNextRow(FALSE, &uFlags);
    // if successful, then this is version 2.
    if (fResult && uFlags == 0)
    {
      iSaveRdc = FPDB_RDC | FPDB_RDC_VER2;
    }
    // if read failed, read next row, if that is successful, assume ver 1.
    else
    {
      fResult = MoveCurrentRow( DATA_BOOKMARK, hlstrTmpBmark, &uFlags, 1L,
        m_lpCloneMoveCursor);
      if (fResult && !(uFlags & DA_fEOF))
        fResult = ReadNextRow(FALSE, &uFlags);
      // if successful, assume version 1
      if (uFlags == 0)
      {
        iSaveRdc = FPDB_RDC | FPDB_RDC_VER1;
        // PLACE CODE HERE!!! - if (does NOT move)
        //  iSaveRdc |= FPDB_RDC_MOVEGET;
      }
    }
  }

  // Either set new flags (set in iSaveRdc) or restore old settings.
  // If not changed, we will test later when data becomes available 
  // (a row is added).
  m_fRemoteDataCtl = iSaveRdc;
  LBlockFree((LBLOCK)hlstrTmpBmark);

  m_iNotifyStatus = iNotify;

  return;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::CheckForReplicatedTable()
 *
 * DESCRIPTION:
 *
 *   Determines if the metadata indicates that the table is a replicated
 *   table.  If it is a replicated table, remove those "special" columns
 *   from the metadata.
 *
 *   To determine if it is a replicated table, look for the following
 *   column names:
 *
 *     s_Generation  VT_I4
 *     s_GUID        VT_BLOB
 *     s_Lineage     VT_BLOB
 *
 *   If so, remove the 2 blob fields (s_GUID & s_Lineage)
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::CheckForReplicatedTable(void)
{
#ifdef WIN32
#define FPDB_GENERATION_STR   _T("s_Generation")
#define FPDB_GUID_STR         _T("s_GUID")
#define FPDB_LINEAGE_STR      _T("s_Lineage")
  CfpDBColumn  *pCF;
  POSITION      pos, posCurr;
  int           iFieldPos;

  int posGen  = -1;
  int posGuid = -1;
  int posLine = -1;

  // Loop thru list of columns looking for those 3 columns.
  pos = m_ColInfoList.GetHeadPosition();
  while (pos != NULL)
  {
    // NOTE: GetNext() will get the column specified by "pos". "pos" is
    // changed by GetNext() to "point" to the next column. 
    pCF = (CfpDBColumn *)m_ColInfoList.GetNext(pos);

    // Look for the 3 columns (just DataField for 1st, Type & DataField for 2nd & 3rd)
    if (posGen == -1 && lstrcmp(pCF->m_strDataField, FPDB_GENERATION_STR) == 0)
      posGen = pCF->m_iIndex;
    else if (posGuid == -1 && pCF->m_ulType == VT_BLOB &&
             lstrcmp(pCF->m_strDataField, FPDB_GUID_STR) == 0)
      posGuid = pCF->m_iIndex;
    else if (posLine == -1 && pCF->m_ulType == VT_BLOB &&
             lstrcmp(pCF->m_strDataField, FPDB_LINEAGE_STR) == 0)
      posLine = pCF->m_iIndex;
  }

  // if all were NOT found, just return.
  if (posGen == -1 || posGuid == -1 || posLine == -1)
    return;

  // At this point, all were found, now remove them from the list.
  pos = m_ColInfoList.GetHeadPosition();
  iFieldPos = 0;
  while (pos != NULL)
  {
    posCurr = pos;
    pCF = (CfpDBColumn *)m_ColInfoList.GetNext(pos);  // GetNext() changes "pos"
    if (pCF->m_iIndex == posGen || 
        pCF->m_iIndex == posGuid ||
        pCF->m_iIndex == posLine)
    {
      // Remove node from the list and delete the object.
      m_ColInfoList.RemoveAt(posCurr);
      delete (pCF);
    }
    else
    { 
      // This node is staying in the list, adjust its index to compensate 
      // for deleted nodes.  Note: m_iIndex may already equal iFieldPos.
      pCF->m_iIndex = iFieldPos;
      ++iFieldPos;
    }
  }
#endif //WIN32
  return; 
}
