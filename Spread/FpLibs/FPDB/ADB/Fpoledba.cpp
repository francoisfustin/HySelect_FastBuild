/*************************************************************************\
* FPOLEDBA.CPP - FarPoint Data Binding source file for ADO Binding.                                                            *
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
//ATL #include "fpatl.h"
#endif


#include "vbinterf.h"                                                                                                                          

#include "fpconvrt.h"
#include "fptstr.h"

#define DBINITCONSTANTS		         // These cause variable declarations

#ifndef NO_MFC
#define INITGUID
#include <initguid.h>
#endif

#include "fpdbutil.h"		  
//#include "fpdbnote.h"		  

#include "fpdbcola.h"
#include "fpoledb.h"		  

#include <limits.h>
#include <malloc.h>

#include "util.h"

#define DB_VT_SAFEARRAY   0x2011
#define BMKMAX 256

// This will get translated to VBM_DATA_INITIATE
#define DATA_UINIT_CONNECT       300

long g_cLocks = 0;

// Declared in fpadb.cpp
extern BOOL fpCompareDataMembers(DataMember bstrDM1, DataMember bstrDM2);

/***************************************************************************
 *
 * FUNCTION:  fpBookmarksEqual( LBLOCK, LBLOCK )
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
BOOL fpBookmarksEqual(LPVOID hlstrBmk1, LPVOID hlstrBmk2)
{
  LONG l1 = 0;
  LONG l2 = 0;
  BOOL fReturn = FALSE;

  if (hlstrBmk1)
     l1 = LBlockLen((LBLOCK)hlstrBmk1);
  if (hlstrBmk2)
     l2 = LBlockLen((LBLOCK)hlstrBmk2);

  // if both lengths are not 0 and are equal, compare bytes 
  if (l1 && l2 && (l1 == l2))
    fReturn = (_fmemcmp( (LPSTR)hlstrBmk1, (LPSTR)hlstrBmk2, (size_t)l1) == 0);

  return fReturn;
}

/***************************************************************************
 *
 * FUNCTION:  fpBookmarksEqual( LBLOCK, CBookmark* )
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
BOOL fpBookmarksEqual(LPVOID hlstrBmk, CBookmark *pCBmk)
{
  LONG l1 = 0;
  LONG l2 = 0;
  BOOL fReturn = FALSE;

  if (hlstrBmk)
     l1 = LBlockLen((LBLOCK)hlstrBmk);
  if (pCBmk)
     l2 = (LONG)(ULONG)*pCBmk;

  // if both lengths are not 0 and are equal, compare bytes 
  if (l1 && l2 && (l1 == l2))
    fReturn = (_fmemcmp( (LPSTR)hlstrBmk, (LPSTR)(void*)*pCBmk, (size_t)l1) == 0);

  return fReturn;
}

BOOL fpBookmarksEqual(CBookmark *pCBmk, LPVOID hlstrBmk)
{ 
  return fpBookmarksEqual(hlstrBmk, pCBmk);
}

BOOL fpBookmarksEqual(CBookmark *pCBmk1, CBookmark *pCBmk2)
{
  return (*pCBmk1 == *pCBmk2);
}


/***************************************************************************
 *
 * FUNCTION:  fpCopyBookmark( CBookmark* pCBmkDest, LBLOCK hlstrSource)
 *
 * DESCRIPTION:
 *
 *   Copy FROM LBLOCK* TO CBookmark*.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if they are equal, else FALSE.
 *
 **************************************************************************/
void fpCopyBookmark(CBookmark *pCBmkDest, LPVOID hlstrSource)
{
  // If not LBLOCK*, clear dest, else copy data.
  if (!hlstrSource)
    pCBmkDest->Clear();
  else
    pCBmkDest->Set(LBlockLen((LBLOCK)hlstrSource), (const BYTE *)hlstrSource);
    
  return;
}


/***************************************************************************
 *
 * FUNCTION:  fpCopyBookmark( LBLOCK *hlstrDest, CBookmark* pCBmkSource)
 *
 * DESCRIPTION:
 *
 *   Copy FROM LBLOCK* TO CBookmark*.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if they are equal, else FALSE.
 *
 **************************************************************************/
void fpCopyBookmark(LBLOCK *hlstrDest, CBookmark* pCBmkSource)
{
  // If not LBLOCK*, clear dest, else copy data.
  if (!(ULONG)*pCBmkSource)
    *hlstrDest = NULL;
  else
    *hlstrDest = LBlockAllocLen((const LPBYTE)(void*)*pCBmkSource, 
                                (LONG)(ULONG)*pCBmkSource);
  return;
}

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
#pragma warning(disable:4355)  // using 'this' in constructor
CfpOleBinding::CfpOleBinding()
  : CUnknownObject(NULL, this)
{
  m_fIgnoreSetDataField = FALSE;

  m_lpOwner = NULL;            // Pointer to OCX class (owner)    
  m_lpfuncDataMsg = NULL;
  m_dispid = -1;               // invalid value

  m_ulCtlFlags = (ULONG)0;
  m_iNotifyStatus = 0;


  // Add for ADB
  m_dmDataMember = NULL;    // DataMember property
  m_pDataSrcProp = NULL;
  m_dispidDataMember = -1;
  m_hRowAccessor = NULL;	// Accessor for row data
  m_hBmkAccessor = NULL;	// Accessor for bookmarks
  m_hSetDataAccessor = NULL;
  m_uSetStmAccessorCt = 0;
  m_pColumns = NULL;        
  m_cColumns = 0;           // Number of elements in m_pColumns
  m_fHasBookmarks = 0;
  m_fSuppMultiStreams = FALSE;
  m_pBlobBinding = NULL;

  m_pPrevSetDataCols = NULL;
  m_pSetDataCols     = NULL;

  ZeroMemory(&m_ahRowFetch, sizeof(m_ahRowFetch));
  m_cRowFetch = 0;
  m_iCurrFetch = -1;
  m_uFlagsFetch = 0;
  m_iFetchColIndex = -1;

  m_dwFetchStatus = FLAGC_FETCH_NEEDED;

  m_ulBoundInlineLen = 0;
  m_ulLockCt = 0;
  m_lpszFetchBuffer = NULL;

#ifdef FP_USE_GLOBAL
  m_hBoundData = 0;
#endif

  m_iUpdateStatus = 0;

  m_lpszSetDataBuffer = NULL;

#ifdef _DEBUG
  m_DBEventList = new CfpNotifyEvents;
#else
  m_DBEventList = NULL;
#endif

  m_aBmkDeleteList = NULL;
  m_cBmkDeleteList = 0;

  m_aBmkEventList = NULL;
  m_cBmkEventList = 0;
  m_aColIdEventList = NULL;
  m_hCurrRow = 0;

  m_strBoundDataField = fpSysAllocTString(_T("*"));   // All columns

// changed default for FreezeEvents flag to make controls bind correctly in VC++ -scl
//  m_fFreezeEvents = TRUE;
  m_fFreezeEvents = FALSE;
  m_fDataSourceChanged = FALSE;

  m_fBatchUpdate = FALSE;

//m_fUseChapters = TRUE;

  return;
}
#pragma warning(default:4355)  // using 'this' in constructor

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

  m_InsertedRowList.ClearList(this);

#ifdef _DEBUG
  if (m_DBEventList)
    delete m_DBEventList;
#endif

  if (m_pPrevSetDataCols)
    delete m_pPrevSetDataCols;

  if (m_pSetDataCols)
    delete m_pSetDataCols;

  if (m_strBoundDataField)
    fpSysFreeTString(m_strBoundDataField);

  fpClearEventBookmarks(FPDB_USE_DELETE_LIST);
  fpClearEventBookmarks();

  // free data member string -scl
  if (m_dmDataMember)
    SysFreeString((BSTR)m_dmDataMember);

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
 * FUNCTION:  CfpOleBinding::SetDataMemberDispid()
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
void CfpOleBinding::SetDataMemberDispid( DISPID dispid)
{
  m_dispidDataMember = dispid;
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
  // This is no longer needed (supported) for ADB
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
  return FALSE;   // No longer supported for ADB
}



HRESULT CfpOleBinding::XBound_OnSourceChanged(DISPID dispid, BOOL fBound, 
  BOOL FAR *lpfOwnXferOut)
{
  return S_OK;    // No longer supported for ADB
}

HRESULT CfpOleBinding::XBound_IsDirty(DISPID dispid)
{
  return S_OK;    // No longer supported for ADB
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetDataSource()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::GetDataSource(DataSource **ppDataSource)
{
	if (m_pDataSrcProp)
		m_pDataSrcProp->AddRef();

	*ppDataSource = m_pDataSrcProp;

	return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetDataSource()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::SetDataSource(DataSource *pDataSource)
{
	HRESULT hr = S_OK;

	if (m_pDataSrcProp)
	{
		// TODO: Add code to do clean up 
		fpReleaseDataBinding();
		
		// Clear previous data source
		//
		m_pDataSrcProp->Release();
	}
	// Cache the new data source	
	//
	m_pDataSrcProp = pDataSource;

	if (m_pDataSrcProp)
	{
		// Take ownership
		//
		m_pDataSrcProp->AddRef();

		// TODO: Add code to process new data source
	}

    // If "Events" are "on", this will re-read the data.
    fpReInitDataBinding();

	return hr;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetDataMember()
 *
 * DESCRIPTION:
 *
 *   Called by the control to get the current DataMember property value.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::GetDataMember(DataMember *pdmDataMember)
{
//	CHECK_POINTER(pdmDataMember);

	if (m_dmDataMember)
		*pdmDataMember = (DataMember)bstralloc((BSTR)m_dmDataMember);
	else
		*pdmDataMember = (DataMember)SysAllocString(L"");

	return NULL == *pdmDataMember ? E_OUTOFMEMORY : S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetDataMember()
 *
 * DESCRIPTION:
 *
 *   Called by the control to set the DataMember property.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::SetDataMember(DataMember dmDataMember)
{
  // If new DataMember string is the same as the current one, just return.
  if (fpCompareDataMembers(dmDataMember, m_dmDataMember) == TRUE)
    return S_OK;

	// Else...Make a copy of the data member
	if (dmDataMember)
	{
		dmDataMember = (DataMember)SysAllocString(dmDataMember);
		RETURN_ON_NULLALLOC(dmDataMember);
	}

	// Free current data member
	if (m_dmDataMember)
		SysFreeString((BSTR)m_dmDataMember);
	
	// Keep new data member
	m_dmDataMember = dmDataMember;

  // If already bound, re-initialize bindings
  if (m_pColumns)
    fpReInitDataBinding();

	return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetDataField()
 *
 * DESCRIPTION:
 *
 *   Called by the control to get the DataField property value (if the
 *   control supports it).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::GetDataField(BSTR *pbstrDataField)
{
	if (m_strBoundDataField)
		*pbstrDataField = (BSTR)fpSysAllocBSTRFromTString(m_strBoundDataField);
	else
		*pbstrDataField = (BSTR)SysAllocString(L"");

	return NULL == *pbstrDataField ? E_OUTOFMEMORY : S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::SetDataField()
 *
 * DESCRIPTION:
 *
 *   Called by the control to set the DataField property (if the
 *   control supports it).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::SetDataField(BSTR bstrDataField)
{
  // Save DataField in member variable
  if (m_strBoundDataField)
    fpSysFreeTString(m_strBoundDataField);

  m_strBoundDataField = fpSysAllocTStringFromBSTR(bstrDataField);

  // If Runtime and we have a cursor, bind to DataField column.
  if (m_pColumns && fpQueryUserMode())
  {
    HRESULT hr = fpBindDataFieldColumn(m_strBoundDataField);  // bind
    if (hr == S_OK)
    {
      DATAACCESS da = {0};
      UINT  uDataMsg;

      _ASSERT(m_lpfuncDataMsg); //Callback func MUST BE SET BY OWNER!
      if (!m_lpfuncDataMsg)
        return hr;

      ZeroMemory(&da, sizeof(da));
      da.sAction = DATA_REFRESH;
      da.lpfpOleBinding = this;
      uDataMsg = VBM_DATA_AVAILABLE;
      // Call the owner's function to process the VBM_DATA_AVAILABLE message
      m_lpfuncDataMsg( (LPVOID)m_lpOwner, uDataMsg, (WPARAM)0, (LPARAM)(LPVOID)&da);
    }
  }

  return (m_strBoundDataField == NULL ? E_OUTOFMEMORY : S_OK);
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
  if ((ULONG)dwCookie < m_cColumns)
    *lpRetBSTR = 
      (BSTR)SysAllocString(m_pColumns[dwCookie].pwszName);

  return (*lpRetBSTR != NULL);
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
HRESULT CfpOleBinding::GetListOfColumnNames(CALPOLESTR *prgOleStr,
      CADWORD *prgCookie)
{
  // Containers will call with NULL arguments
  // to determine if the control supports browsing
  // for this property. Returning S_OK for this 
  // case signifies YES
  if (!prgOleStr || !prgCookie)
    return S_OK;

  // Ensure columns array is intialized
  HRESULT hr = fpReInitDataBinding();
  RETURN_ON_FAILURE(hr);

  // Start off with no entries
  prgOleStr->cElems = prgCookie->cElems = 0;
  prgOleStr->pElems = NULL;
  prgCookie->pElems = NULL;

  // Nothing to do if there are no columns
  if (!m_cColumns)
    return S_OK;

  // Skip the self-bookmark column
  ULONG uSkipSelfBookmarkColumn = (ULONG)m_fHasBookmarks;
  ULONG cColumns = m_cColumns-uSkipSelfBookmarkColumn;

  // Allocate return buffers
  prgOleStr->pElems = (LPOLESTR*) CoTaskMemAlloc(sizeof(LPOLESTR) * cColumns);
  if (NULL == prgOleStr->pElems) 
    goto OutOfMemory;

  prgCookie->pElems = (DWORD*)CoTaskMemAlloc(sizeof(DWORD) * cColumns);
  if (NULL == prgCookie->pElems)
    goto OutOfMemory;

  for (prgOleStr->cElems = 0; prgOleStr->cElems < cColumns; prgOleStr->cElems++)
  {
    // Return copy of column name
    prgOleStr->pElems[prgOleStr->cElems] = wstralloc(m_pColumns[uSkipSelfBookmarkColumn+prgOleStr->cElems].pwszName);

    // Cookie is the index of the column in the m_pColumns array
    prgCookie->pElems[prgOleStr->cElems] = uSkipSelfBookmarkColumn+prgOleStr->cElems;
  }
  prgCookie->cElems = prgOleStr->cElems;
  return S_OK;

OutOfMemory:
  if (NULL != prgOleStr->cElems)
  {
    CoTaskMemFree(prgOleStr->pElems);
    prgOleStr->cElems = NULL;
  }
  if (NULL != prgCookie->pElems)
  {
    CoTaskMemFree(prgCookie->pElems);
    prgCookie->pElems = NULL;
  }
  return E_OUTOFMEMORY;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetColumnNameAtIndex()
 *
 * DESCRIPTION:
 *
 *   This function will return a pointer to a TCHAR string which is
 *   the column name for the specified index.  If the index is invalid,
 *   a NULL ptr will be returned. 
 *
 *   The intended use of this function is to enumerate through the list
 *   of columns names for the DataField property when it is requested
 *   via IPerPropertyBrowsing::GetPredefinedStrings().
 *
 * PARAMETERS:
 *
 *   Index of column.  Start with 0 and keep calling with the next index
 *   until a NULL ptr is returned.
 *
 * RETURNS:  
 *
 *   NULL or LPCTSTR ptr to string (NOTE: DO NOT FREE THE STRING PTR!)
 *
 **************************************************************************/
LPCTSTR CfpOleBinding::GetColumnNameAtIndex(UINT index, DWORD *pdwCookie)
{
  ULONG uSkipSelfBookmarkColumn;

  // Ensure that we have the metadata.
  if (m_cColumns == 0)
  {
    HRESULT hr = fpInitDataBinding();
    if (hr != S_OK)
      return NULL;
  }

  uSkipSelfBookmarkColumn = (ULONG)m_fHasBookmarks;

  // Index is too great, then return NULL.
  if ((ULONG)index >= (m_cColumns - uSkipSelfBookmarkColumn))
    return NULL;

  // Cookie is actual index into m_pColumns list.
  *pdwCookie = (DWORD)(uSkipSelfBookmarkColumn + (ULONG)index);

  // Else, return the string ptr.
  return 
    (LPCTSTR)m_pColumns[uSkipSelfBookmarkColumn + (ULONG)index].tstrName;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetBookmark()
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
BOOL CfpOleBinding::fpGetBookmark( LONG lBmarkType, LPVOID hlstrBmark, 
  LPVOID FAR *lphlstrRetBmark, USHORT FAR *lpuRetFlags, BOOL fUseSeqRead)
{
	HRESULT hr = S_OK;
    LONG lRelOffset = 0;         // Relative offset position for move
    USHORT uFlags = 0;           // Temp flags to return

    HROW hRow = 0;
    BOOL fReleaseRow = FALSE;
    BOOL fCurrRow    = FALSE;
    CBookmark bmkTarget;         // Bookmark of desired row (target)
    CBookmark bmkRefRow;         // Bookmark of reference row
    BOOL fLookInFetchArray = TRUE;

    *lphlstrRetBmark = NULL;
    *lpuRetFlags = 0;

    // If not bound or table does not have bookmarks, return
    if (!m_pRowsetPos || !m_fHasBookmarks)
      return FALSE;

    // Act on type of Bookmark to get (from lpda->lData)
    switch (lBmarkType)
    {
	  case DATA_BOOKMARKFIRST:
      case DATA_BOOKMARKLAST:
      {
        // If looking for the last bookmark and we have items in the
        // "insert row list", then return the last bookmark in that list.
        if (lBmarkType == DATA_BOOKMARKLAST && !m_InsertedRowList.IsEmpty())
        {
          m_InsertedRowList.GetLastBookmark(&bmkTarget);
          break;
        }

        // Copy one of these defines to the CBookmark object "bmk".
        // NOTE: Later in this function, we will fetch the row for this 
        //       bookmark to verify that the row exists.
        BYTE b = (lBmarkType == DATA_BOOKMARKFIRST ? (BYTE)DBBMK_FIRST : (BYTE)DBBMK_LAST);
        bmkRefRow.Set(sizeof(BYTE), &b);

        // If last bookmark, don't look in fetch array (don't load 
        // array starting at the last row).
        if (lBmarkType == DATA_BOOKMARKLAST)
          fLookInFetchArray = FALSE;

        break;
      }

      case DATA_BOOKMARKCURRENT:
 	    // Get the bookmark at the cursor's current position.
        //
        // If using SeqRead get bookmark from current fetch buffer.
        if (fUseSeqRead && m_iCurrFetch != -1)
        {
          hRow = m_ahRowFetch[m_iCurrFetch];
        }
        else // get bookmark from Rowset.
        {
          DBPOSITIONFLAGS dwPosFlags;
          // Get HROW for current row and read bookmark for HROW.
          if (m_pRowsetPos)
          {
            HCHAPTER hOldChapter = m_hChapter; 
            if (m_pRowsetPos->GetRowPosition(&m_hChapter, &hRow, 
                  &dwPosFlags) != S_OK)
		      return FALSE;
            // Release old if different
            if (hOldChapter != m_hChapter)
              fpReleaseChapter(&hOldChapter);
          }
          // If we have a HROW, indicate that we have to release it.
          if (hRow)
          {
            fReleaseRow = TRUE;
            fCurrRow    = TRUE;
            m_bmkCurrRow.Clear();
          }
          // Else if we currently have one stored, use it.
          else if ((ULONG)m_bmkCurrRow)
            bmkTarget = m_bmkCurrRow;
          else
            return FALSE;
        }
        break;

	  case DATA_BOOKMARKNEXT:
	  case DATA_BOOKMARKPREV:
	    if (!hlstrBmark)         // parameter is required for next/prev
          return FALSE;
        bmkRefRow.Set(LBlockLen((LBLOCK)hlstrBmark), (const BYTE *)hlstrBmark);
        lRelOffset = (lBmarkType == DATA_BOOKMARKNEXT ? 1 : -1);
        break;

      default: 
       // Invalid parameter in "lpvda->lData", must be one of above values.
       ASSERT(0);
       return FALSE;
    } //switch


    // If we don't have a HROW and size of bmkRefRow is not 0, then 
    // get one via the bookmark, "bmkRefRow".
	if (!hRow && (ULONG)bmkRefRow)
    {
      // if ok the look in fetch array, look there (if not found, then
      // the array will be loaded starting at the specified bookmark). 
      if (fLookInFetchArray)
      {
        int iFetch;
        HRESULT h = fpGetFetchIndex(&bmkRefRow, &iFetch, &uFlags,
          lRelOffset);
        // If failed or index is -1, return EOF
        if (h != S_OK || iFetch == -1)
          uFlags = DA_fEOF;       // Indicate EOF
        else
          bmkTarget = m_aBmkFetch[iFetch];
      }
      else // don't use fetch array, use GetRows() func.
      {
        ULONG cRows;
        HROW  *phRows = NULL;
        
	    hr = CRowset::GetRows(bmkRefRow, lRelOffset, 1, phRows, cRows);
        if (hr == S_OK && phRows && cRows)
        {
          // Save HROW and free allocated array
          hRow = phRows[0];
          fReleaseRow = TRUE;
        
          CoTaskMemFree((void*)phRows);
        }
        else if (hr == DB_S_ENDOFROWSET || hr == DB_E_BADSTARTPOSITION)
        {
          // ENDOFCURSOR is True. We are either at the END or the BEGINNING,
          // determine which. 
          if (lBmarkType == DATA_BOOKMARKPREV)
	        uFlags = DA_fBOF;
          else // either DATA_BOOKMARKNEXT, DATA_BOOKMARKFIRST or DATA_BOOKMARKLAST
	        uFlags = DA_fEOF;
          hr = S_OK;
        }
	    else if (hr != S_OK)
	    {
	      return FALSE;
	    }
      } // if-else fLookInFetchArray
	} // if !hRow

    // At this point, we should have a hRow handle, fetch the
    // bookmark data for this row.
    if (hr == S_OK && hRow)
    {
      hr = fpGetBookmarkAtHRow(hRow, &bmkTarget, fReleaseRow);
      // If this is the current row, then save a copy of the bookmark
      // for times when GetRowPosition() fails to give us a HROW.
      if (fCurrRow && hr == S_OK)
        m_bmkCurrRow = bmkTarget;
    }

    // if successful AND bookmark is not zero.
    if (hr == S_OK && (ULONG)bmkTarget)
      *lphlstrRetBmark = 
        LBlockAllocLen((const LPBYTE)(void*)bmkTarget, (LONG)(ULONG)bmkTarget);

    // Set return parameters
    *lpuRetFlags = uFlags;

	return (hr == S_OK);
}  //GetBookmark()


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetBookmarkAtHRow()
 *
 * DESCRIPTION:
 *
 *   Get the bookmark at the specified HROW position.
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 *   HRESULT
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpGetBookmarkAtHRow( HROW hRow, CBookmark *pBmk, 
  BOOL fReleaseRow /* = FALSE */)
{
	HRESULT hr = S_OK;

    if (!hRow || !pBmk)
      return E_INVALIDARG;

    pBmk->Clear();

    // If not bound or table does not have bookmarks, return
    if (!m_pRowsetPos || !m_fHasBookmarks)
      return E_FAIL;

    // At this point, we should have a hRow handle, fetch the
    // bookmark data for this row.
    if (hr == S_OK && hRow)
    {
	  LPSTR lpszData = fpLockFetchBuffer();

      // free any data from previous fetch
      m_pColumns[0].fpFreeFetchData(lpszData);

      // GetData
      hr = CRowset::GetData(hRow, m_hBmkAccessor, lpszData);
      if (hr == S_OK)  // Get bookmark from data
      {
        m_pColumns[0].fpPostFetchSetup(lpszData);
        hr = m_pColumns[0].fpGetBookmark(lpszData, pBmk);
      }
      fpUnlockFetchBuffer();
    }

    // if successful AND bookmark is not zero, return S_OK, else S_FALSE
    if (hr == S_OK)
      hr = ((ULONG)*pBmk > 0 ? S_OK : S_FALSE);

    if (fReleaseRow)
      CRowset::ReleaseRows(&hRow, 1);

	return hr;
}  //fpGetBookmarkAtHRow()


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
BOOL CfpOleBinding::fpGetFieldValue(BOOL fOnlyGetLen, LPVOID lpvda, 
  BOOL fUseSeqRead)
{
    LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
    BOOL fReturn = TRUE;
    CfpDBColumn *pCF;
    int colIndex;
	LPSTR lpszData;
    ULONG ulTmp;
    LPLONG lplData;
    HRESULT hr;

    // If DataField is NULL, use DataFieldIndex as the position
    if (lpda->hszDataField)
       colIndex = fpGetColumnRec((LPCTSTR)lpda->hszDataField);
    else
       colIndex = fpGetColumnRec((int)lpda->sDataFieldIndex);

    // if i is invalid, return error
    if (colIndex < 0)
       return FALSE;
    // Verify that this column is bound.
    if (m_BoundCols.FindColIndex(colIndex) == -1)
    {
      _ASSERT(0); // Column is not bound. You must bind to this column! see fpBindDataFieldColumn().
      return FALSE;
    }

    pCF = &m_pColumns[colIndex];  // get ptr to column.

    // Verify that fetch buffer matches row.
    if (fUseSeqRead)  // verify that buffer is from clone (read first/next)
    {
      _ASSERT(0); // FIX CODE to handle this case!!!
//    fReturn = ((m_dwFetchStatus & FLAG_FETCHED_FROM_CLONE) != 0);
    }
    else // Verify that fetch buffer matches row, if not, it fetches the row.
    {
      hr = fpVerifyFetchBuffer( (LPVOID)lpda->hlstrBookMark, (USHORT *)&(lpda->fs), colIndex);
      fReturn = (hr == S_OK);
    }

    // Get data from fetch buffer
    // match datafield with ColumnField record and get the data from it.
    if (fReturn)
    {
      // Loop possibly twice, if first time fails (do to Memo/Binary
      // field requiring a re-load), then re-fetch buffer.
      int loopCt = 0;
      while (loopCt < 2)
      {
	    lpszData = fpLockFetchBuffer();
        // If fOnlyGetLen is TRUE, pass NULL instead of &lData, so only
        // the length gets retrieved, not the data
        if (fOnlyGetLen)
           lplData = NULL;
        else
           lplData = (LPLONG)&lpda->lData;
        
        // Note: fpGetDataValue() returns data's len OR -1 if error.
        ulTmp = pCF->fpGetDataValue(lpszData, lpda->usDataType, 
          lpda->ulChunkOffset, lpda->ulChunkNumBytes,
          lplData, (USHORT *)&lpda->fs);
        
        // If fpGetDataValue() fails with -2, then this indicates that
        // we need to re-fetch the buffer (only do this once).
        if ((ulTmp == (ULONG)-2) && loopCt == 0)
          ++loopCt;
        else // else, force us out of the loop.
          // Setting loopCt = 2 will prevent another iteration and 
          // the loop will end.
          loopCt = 2;  

        fReturn = (ulTmp != (ULONG)-1) && (ulTmp != (ULONG)-2);

        // if getting just the len, set lData to value.
        if (fOnlyGetLen && fReturn)
           lpda->lData = (LONG)ulTmp;
        
        fpUnlockFetchBuffer();

        // Before re-looping, re-load the fetch buffer.
        if (loopCt < 2)
        {
          m_dwFetchStatus |= FLAGC_DATACHANGED;
          if (fpVerifyFetchBuffer((LPVOID)lpda->hlstrBookMark,
                                  (USHORT *)&(lpda->fs), colIndex) != S_OK)
          {
            fReturn = FALSE;
            loopCt = 2;
          }
        }
      }
    }

    return fReturn;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpVerifyFetchBuffer()
 *
 * DESCRIPTION:
 *
 *   Verifies that the row specified mathces the data in the Fetch buffer.
 *   The row is specified as a Bookmark (HLSTR) or if NULL, then the
 *   current row (of m_lpCursorMove) is used.  If the row does not
 *   match the buffer, then the row is put in the Fetch buffer.
 *
 *   NOTE:  The bookmark of the row in the fetch buffer is saved in the
 *          member variable, "m_bmkFetch", and flags are set in
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
HRESULT CfpOleBinding::fpVerifyFetchBuffer( LPVOID hlstrBookmark, 
  USHORT *lpuFlags, int colIndex)
{
  HRESULT hr = S_OK;
  DWORD dwTmpFetchStatus = (DWORD)0;

  CBookmark bmkFetchTmp;
  HROW hRow = 0;
  BOOL fReleaseRow = FALSE;
  DWORD dwBufferType = FLAG_FETCHED_NONSTREAMBUFFER;  // stream or non-stream
  BOOL fGetData = FALSE;  // indicates whether GetData must be called.
  BOOL fFetchRow = FALSE; // indicates whether the fetch must be fetched.

  // If the requested column uses streams and there is no support
  // for multiple streams, fetch buffer for just that column.
  if (!m_fSuppMultiStreams && m_pColumns[colIndex].m_hReadAccessor)
  {
    dwBufferType = FLAG_FETCHED_STREAMBUFFER;
    // Force the data to be fetched if the previously fetched
    // column is not the same as the one specified.
    if (m_iFetchColIndex != colIndex)
      fGetData = TRUE;
  }

  // If current status does not include the same buffer type, then
  // make sure we call the GetData function.
  // NOTE: "dwBufferType" contains one of two bits (either 
  //   FLAG_FETCHED_STREAMBUFFER or FLAG_FETCHED_NONSTREAMBUFFER). 
  //   "m_dwFetchStatus" contains (besides other bits) none of the
  //   these bits, either one of them, or both. Determine if
  //   "m_dwFetchStatus" contains the one bit stored in "dwBufferType".
  if (!fGetData && (dwBufferType & m_dwFetchStatus) == 0)
    fGetData = TRUE;  // then we definitely have to get the data.

  // If no bookmark was passed, then we are fetching from 
  // the "Current Row".  Determine if the fetch buffer contains data
  // from the current row.  If not, get current bookmark and use it
  // for the next fetch.
  if (!hlstrBookmark)
  {
    if (m_dwFetchStatus & FLAG_CURR_ROW_INVALID)
      return FALSE;
    if ((m_dwFetchStatus & (FLAG_CURR_ROW_MOVED|FLAG_CURR_ROW_DATACHANGED)) ||
             !(m_dwFetchStatus & FLAG_FETCHED_FROM_CURR_ROW))
      fFetchRow = TRUE;

    // if fFetchRow is true OR we don't have the right buffer type, then
    // get the parameters needed to read the buffer.
    if (fFetchRow || fGetData)
    {
      DBPOSITIONFLAGS dwPosFlags;
      HCHAPTER hOldChapter = m_hChapter; 
      // Get HROW for current row. 
      if (!m_pRowsetPos || m_pRowsetPos->GetRowPosition(&m_hChapter, 
            &hRow, &dwPosFlags) != S_OK)
        return FALSE;
      // Release old if different
      if (hOldChapter != m_hChapter)
        fpReleaseChapter(&hOldChapter);
      if (hRow)
        fReleaseRow = TRUE;
      // RowsetPos is not giving us a row, use the one in m_bmkCurrRow.
      else if ((ULONG)m_bmkCurrRow)
        bmkFetchTmp = m_bmkCurrRow;

      dwTmpFetchStatus = FLAG_FETCHED_FROM_CURR_ROW;
    }
  }
  else  // a bookmark was passed, determine if last fetch is OK to use.
  {
     // If we don't have a bmark from the last fetch OR
     // if data changed (it MAY have happen to current fetch buffer) OR
     // if the passed bmark doesn't match the bmark saved from the 
     // last fetch then move the fetch buffer.
     if (!(m_dwFetchStatus & FLAG_FETCHED_FROM_BMARK) ||
          (m_dwFetchStatus & FLAGC_DATACHANGED) ||
         !fpBookmarksEqual(hlstrBookmark, &m_bmkFetch))
       fFetchRow = TRUE;

     if (fFetchRow || fGetData)
     {
       fpCopyBookmark(&bmkFetchTmp, hlstrBookmark);
       dwTmpFetchStatus = FLAG_FETCHED_FROM_BMARK;
     }
  }

  // If a new fetch bmark is set or already have a HROW, then 
  // call GetData() for that HROW (if we don't have a HROW, get one
  // from the bookmark).
  if ((ULONG)bmkFetchTmp || hRow)
  {
    hr = fpFillFetchBuffer(&bmkFetchTmp, hRow, dwBufferType, fFetchRow,
                           dwTmpFetchStatus, lpuFlags, colIndex);
  }

  if (fReleaseRow)
    CRowset::ReleaseRows(&hRow, 1);
    
  return hr;

} //fpVerifyFetchBuffer()


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFillFetchBuffer()
 *
 * DESCRIPTION:
 *
 *   This function will fill the fetch buffer by calling GetData()
 *   with the proper Accessor.  Flags and member variables are
 *   set by this function to indicate that the fetch has been
 *   completed.
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 *   S_OK if successful
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFillFetchBuffer(
  CBookmark *pbmkFetch,
  HROW  hRow, 
  DWORD dwBufferType,  // FLAG_FETCHED_STREAMBUFFER or FLAG_FETCHED_NONSTREAMBUFFER
  BOOL  fFetchRow,     // flag which indicates setting certain member variables
  DWORD dwFetchStatus, // setting for member variable if successful
  USHORT *lpuFlags, 
  int colIndex)
{
  HRESULT hr = S_OK;
  int i;
  HACCESSOR hAccessor;
  int iNotify;
  LPSTR lpszData;

  iNotify = m_iNotifyStatus;
  m_iNotifyStatus &= NOTIFY_SUSPENDED;

  // If we are fetching a different row (fFetchRow), then reset 
  // these bits to invalidate the current fetch and clear the bookmark.
  if (fFetchRow)
  {
    m_dwFetchStatus &= ~(FLAGC_CURRENT_FETCH | FLAGC_FETCHED_BUFFERTYPE);
    m_bmkFetch.Clear();
  }

  // If we don't have a HROW, get HROW of specified row.
  if (!hRow)
  {
    hr = fpGetFetchIndex(pbmkFetch, &i, lpuFlags);
    if (hr != S_OK)
    {
      // Restore status flags.
      m_iNotifyStatus = iNotify;
      return hr;
    }
    hRow = m_ahRowFetch[i];
  }

  // lock memory
  lpszData = fpLockFetchBuffer();

  // Free any data from previous fetch
  fpFreePreviousFetchData(dwBufferType);

  // Determine which Accessor to use, whether it's the "normal"
  // row accessor or one of the column accessors used for a stream.
  if (dwBufferType == FLAG_FETCHED_STREAMBUFFER)
  {
    hAccessor = m_pColumns[colIndex].m_hReadAccessor;
    _ASSERT(hAccessor);  // this column should have an accessor!!!
  }
  else
    hAccessor = m_hRowAccessor;

  if (!hAccessor)
    hr = S_FALSE;
  else
  {
    hr = CRowset::GetData(hRow, hAccessor, lpszData);
//    if (hr == S_OK)     // Init any data after GetData().
    if (hr >= S_OK) // 9/3/99 -- if an error status code rather than 
    {	// an error code is returned, continue and set hr to S_OK - scl & scp
	  hr = S_OK;
      // Remember the colIndex if stream column
      if (dwBufferType == FLAG_FETCHED_STREAMBUFFER)
        m_iFetchColIndex = colIndex;

      fpPostFetch_Init(dwBufferType);  // initialize the data 

      // "Remember" this bookmark as the current fetch
      if (fFetchRow)
      {
        m_bmkFetch = *pbmkFetch;
        // Clear the flags that indicate that the fetch is invalid.
        m_dwFetchStatus &= ~FLAGC_FETCH_NEEDED;
        m_dwFetchStatus |= dwFetchStatus; // whether curr row or bmark
      }

      // Indicate which accessor was used (non-stream or stream),
      // though, it doesn't matter if multiple streams are supported.
      m_dwFetchStatus |= dwBufferType;
    }
  }

  // unlock memory
  fpUnlockFetchBuffer();

  // Resume notification
  m_iNotifyStatus = iNotify;

  return hr;
} //fpFillFetchBuffer()


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFreePreviousFetchData()
 *
 * DESCRIPTION:
 *
 *   Releases any data which was loaded by the previous fetch.
 *
 * PARAMETERS:
 *
 *   flags ...... This parameter uses any combination of the following
 *                defines:
 *                   FLAG_FETCHED_NONSTREAMBUFFER ((DWORD)0x0040)
 *                   FLAG_FETCHED_STREAMBUFFER    ((DWORD)0x0080)
 *                If none are specified, then both are assumed.
 *                NOTE: This only makes a difference when multiple
 *                streams are NOT supported.
 *                In the case of FLAG_FETCHED_STREAMBUFFER, then only
 *                the previous column using streams is freed.
 *
 * RETURNS:  void
 *
 **************************************************************************/
void CfpOleBinding::fpFreePreviousFetchData(DWORD flags)
{
  ULONG i;
  ULONG index;
  LPSTR lpszData = fpLockFetchBuffer();
  UINT  ct = m_BoundCols;

  // NOTE: FLAGC_FETCHED_BUFFERTYPE defined as both flags:
  //         (FLAG_FETCHED_NONSTREAMBUFFER | FLAG_FETCHED_STREAMBUFFER)
  // if multiple streams are supported or no flags were specified,
  // assume flags.
  if (m_fSuppMultiStreams || (flags == 0))
    flags = FLAGC_FETCHED_BUFFERTYPE;

  // if just the one flag is set, then just free the one column specified
  // by the member variable, m_iFetchColIndex.
  if (flags == FLAG_FETCHED_STREAMBUFFER)
  {
    // if index is valid, free the data for that column.
    if (m_iFetchColIndex != -1)
      m_pColumns[m_iFetchColIndex].fpFreeFetchData(lpszData);
  }
  else // else, not just the stream column...
  {
    // For each bound column, free the fetch data.
    for (i = 0; i < ct; i++)
    {
      // From the list of bound columns, get the index into "m_pColumns".
      index = m_BoundCols[i].colIndex;
      // if columns with streams are allowed to be freed, OR if this column
      // is not a stream column, then free the data.
      if ((flags & FLAG_FETCHED_STREAMBUFFER) || 
          !m_pColumns[index].m_hReadAccessor)
        m_pColumns[index].fpFreeFetchData(lpszData);
    }
  }

  fpUnlockFetchBuffer();

  // Indicate that the current fetch in no longer valid.
  m_dwFetchStatus &= ~flags;

} // fpFreePreviousFetchData()


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpPostFetch_Init()
 *
 * DESCRIPTION:
 *
 *   This is called after a fetch (GetData()), it initializes the column
 *   after the data is fetched.  It calls "fpPostFetchSetup()" for each
 *   column which is bound.
 *
 * PARAMETERS:
 *
 *   flags ...... This parameter uses any combination of the following
 *                defines:
 *                   FLAG_FETCHED_NONSTREAMBUFFER ((DWORD)0x0040)
 *                   FLAG_FETCHED_STREAMBUFFER    ((DWORD)0x0080)
 *                If none are specified, then both are assumed.
 *                NOTE: This only makes a difference when multiple
 *                streams are NOT supported.
 *                In the case of FLAG_FETCHED_STREAMBUFFER, then only
 *                the previous column using streams is initialized.
 *
 * RETURNS:  void
 *
 **************************************************************************/
void CfpOleBinding::fpPostFetch_Init(DWORD flags)
{
  ULONG i;
  ULONG index;
  LPSTR lpszData = fpLockFetchBuffer();
  UINT  ct = m_BoundCols;
  CfpDBColumn *pDBCol;

  // NOTE: FLAGC_FETCHED_BUFFERTYPE defined as both flags:
  //         (FLAG_FETCHED_NONSTREAMBUFFER | FLAG_FETCHED_STREAMBUFFER)
  // if multiple streams are supported or no flags were specified,
  // assume flags.
  if (m_fSuppMultiStreams || (flags == 0))
    flags = FLAGC_FETCHED_BUFFERTYPE;

  // if just the one flag is set, then just init the one column specified
  // by the member variable, m_iFetchColIndex.
  if (flags == FLAG_FETCHED_STREAMBUFFER)
  {
    // if index is valid, free the data for that column.
    if (m_iFetchColIndex != -1)
      m_pColumns[m_iFetchColIndex].fpPostFetchSetup(lpszData);
  }
  else // else, not just the stream column...
  {
    for (i = 0; i < ct; i++)
    {
      index = m_BoundCols[i].colIndex;
      pDBCol = &m_pColumns[index];
      // if columns with streams are allowed to be processed, OR if this column
      // does not have its own stream column, then process it.
      if ((flags & FLAG_FETCHED_STREAMBUFFER) || !pDBCol->m_hReadAccessor)
        pDBCol->fpPostFetchSetup(lpszData);
    }
  }

  fpUnlockFetchBuffer();
} // fpPostFetch_Init()



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFreeFetchArray()
 *
 * DESCRIPTION:
 *
 *   Free (release) the HROW handles in the "Fetch Array".
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFreeFetchArray(void)
{
  HRESULT hr = S_OK;

  // Release rows
  if (m_cRowFetch)
  {	
    hr = CRowset::ReleaseRows(m_ahRowFetch, m_cRowFetch);
    m_cRowFetch = 0;
  }

  m_iCurrFetch = -1;
  m_uFlagsFetch = 0;

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetFetchIndex()
 *
 * DESCRIPTION:
 *
 *   This will "find" the correct row within the "Fetch Array". If
 *   the row is not already in the fetch array, it will load the
 *   fetch array starting at the bookmark (+ offset).
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpGetFetchIndex(CBookmark *pBmk, 
  int *pRetIndex, USHORT *pRetFlags, LONG lRelOffset /* = 0*/ )
{
  HRESULT hr;
  BOOL    fFound = FALSE;
  int     i = 0;
  ULONG   cRows = 0;
  HROW   *phRows;
  BOOL    fIndexZero;

  *pRetIndex = -1;
  *pRetFlags = 0;

  // Look for bookmark in the fetch array.
  fFound = fpFindFetchIndex(pBmk, &i);

  // If found, return index
  if (fFound)
  {
    // if non-zero lRelOffset was specified, see if that bookmark is
    // also in our fetch array.
    int iTarget = i + (int)lRelOffset;
    if (iTarget < m_cRowFetch && iTarget >= 0)
    {
      *pRetIndex = iTarget;
      return S_OK;
    }
  } 

  // Before fetching a new load, make sure that we are not at the end (or
  // the beginning).
  if (lRelOffset != 0 && m_cRowFetch) 
  {                                   // look for DBBMK_LAST/FIRST
    UINT flag = (lRelOffset > 0 ? FPDB_FETCH_EOF : FPDB_FETCH_FIRSTROW);
    // if status matches flag, then we are at the end/beginning and
    // can go no further.
    if (m_uFlagsFetch & flag)
    {
      *pRetFlags = (lRelOffset > 0 ? DA_fEOF : DA_fBOF);
      return S_FALSE;
    }
  }

  // Release rows and reset flags
  fpFreeFetchArray();

  // Insure that "m_pRowsetLocate" member has been initialized.
  hr = InitRowsetLocate();
  RETURN_ON_FAILURE(hr);

  // This flag is used to indicate that the desired row was read into
  // the fetch array at index 0. If FALSE, then we have to search for
  // the specified bookmark.
  fIndexZero = TRUE;

  phRows = m_ahRowFetch;
  // Load fetch array with the row starting at "pBmk"+offset.
  // NOTE: If looking for "previous row" (lRelOffset < 0), try to
  //       load "target" bookmark into the last row.
  hr = m_pRowsetLocate->GetRowsAt(NULL, m_hChapter, (ULONG)*pBmk, 
    (const BYTE*)*pBmk, 
    lRelOffset + (lRelOffset < 0 ? -FPDB_MAX_FETCH + 1: 0), 
    FPDB_MAX_FETCH, &cRows, &phRows);

  // If bad start position...
  if (hr == DB_E_BADSTARTPOSITION)
  {
    // If invalid start position and looking for "previous row",
    // load fetch array starting at FIRST row.
    if (lRelOffset < 0)
    {
      BYTE b = DBBMK_FIRST;

      // Fill array starting with first row.
      hr = m_pRowsetLocate->GetRowsAt(NULL, m_hChapter, sizeof(b), 
        &b, 0, FPDB_MAX_FETCH, &cRows, &phRows);

      // If successful, indicate that buffer includes the "First row".
      if ((hr == S_OK || hr == DB_S_ENDOFROWSET) && cRows)
        m_uFlagsFetch |= FPDB_FETCH_FIRSTROW;

      // Indicate that the desired row is not at index 0.
      fIndexZero = FALSE;
    }
    else // Bad start position but not looking for prev row, set flags.
    {
      m_uFlagsFetch |= FPDB_FETCH_EOF;
      *pRetFlags = DA_fEOF;
    }
  }

  // Convert this code to S_OK
  if (hr == DB_S_ENDOFROWSET)
  {
    hr = S_OK;
    m_uFlagsFetch |= FPDB_FETCH_EOF;
  }

  // If successful, check if we read the first row.
  if (hr == S_OK && cRows && pBmk->IsFirstRow())
    m_uFlagsFetch |= FPDB_FETCH_FIRSTROW;

  // return if failed or no rows
  RETURN_ON_FAILURE(hr);
  if (!cRows)
    return S_FALSE;

  m_cRowFetch = (int)cRows;      // Remember count.

  // Now get the bookmarks for each HROW that we read.
  for (i = 0; i < m_cRowFetch; i++)
  {
    hr = fpGetBookmarkAtHRow( m_ahRowFetch[i], &m_aBmkFetch[i]); 
    _ASSERT(hr == S_OK);  // Did GetData() fail?
  }  

  // If this flag is still TRUE, then return the index as 0.
  if (fIndexZero)
  {
    // IF lRelOffset is neg, then the "target" bookmark is at
    // the end of the array, ELSE it is at the first index.
    *pRetIndex = (lRelOffset < 0 ? (cRows-1) : 0);
    return S_OK;
  }

  // Desired row is not at index 0, so search freshly loaded array.
  fFound = fpFindFetchIndex(pBmk, &i);

  // If found, return index
  if (fFound)
  {
    // If an offset was specified, adjust the index and check that it's
    // still valid.
    int iTarget = i + (int)lRelOffset;
    if (iTarget < m_cRowFetch && iTarget >= 0)
    {
      *pRetIndex = iTarget;
      return S_OK;
    }
    // At this point, an offset was specified but the row was not found,
    // so specify the EOF/BOF flag.
    *pRetFlags = (lRelOffset > 0 ? DA_fEOF : DA_fBOF);
  } 

  return S_FALSE;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFindFetchIndex()
 *
 * DESCRIPTION:
 *
 *   This will "find" the correct row within the "Fetch Array". If
 *   the row is not already in the fetch array, it will return FALSE.
 *
 * PARAMETERS:
 *
 * RETURNS:  
 *
 **************************************************************************/
BOOL CfpOleBinding::fpFindFetchIndex(CBookmark *pBmk, int *pRetIndex,
  int iDirection /* = 1*/ )
{
  BOOL    fFound;
  int     i;

  *pRetIndex = -1;

  // If empty, return
  if (!m_cRowFetch)
    return FALSE;

  // If first row was specified and flag is set, then index is 0.
  if (pBmk->IsFirstRow() && (m_uFlagsFetch & FPDB_FETCH_FIRSTROW))
  {  
    *pRetIndex = 0;
    return TRUE;
  }
  // If first row was specified and flag is set, then index is 0.
  else if (pBmk->IsLastRow() && (m_uFlagsFetch & FPDB_FETCH_EOF))
  {  
    *pRetIndex = m_cRowFetch - 1;
    return TRUE;
  }

  // If fetch buffer is has data, look for bookmark.
  // first check at current fetch
  if (m_iCurrFetch != -1)
    if (m_aBmkFetch[m_iCurrFetch] == *pBmk)
    {
      *pRetIndex = m_iCurrFetch;
      return TRUE;
    }

  if (iDirection < 0)
  {
    // search backwards
    iDirection = -1;                   // Either -1 or 1
    i = m_cRowFetch - 1;
  }
  else
  {
    // search forewards
    iDirection = 1;                    // Either -1 or 1
    i = 0;
  }
  fFound = FALSE;

  // if not found, check entire array.
  while (!fFound && i < m_cRowFetch && i >= 0)
    // Note: Skip the one we already checked (m_iCurrFetch)
    if (i != m_iCurrFetch && m_aBmkFetch[i] == *pBmk)
      fFound = TRUE;
    else
      i += iDirection;
  
  // If found, return index
  if (fFound)
    *pRetIndex = i;

  return fFound;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetColumnRec()
 *
 * DESCRIPTION:
 *
 *   Find Column index for the ColumnName string.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   index of CfpDBColumn object if FOUND, else -1
 *
 **************************************************************************/
int CfpOleBinding::fpGetColumnRec(LPCTSTR tstrColumnName)
{
  BOOL bFound;
  ULONG i;

  if (tstrColumnName[0] == (TCHAR)0) 
    return NULL;

  // Scan thru the list looking for matching string.  If no list
  // or not found, return NULL.
  bFound = FALSE;
  i = (ULONG)m_fHasBookmarks;
  while (i < m_cColumns && !bFound)
  {
    if (lstrcmpi(m_pColumns[i].tstrName, tstrColumnName) == 0)
      bFound = TRUE;
    else
      ++i;
  }

  // if found return index, else return -1
  return (bFound ? i : -1);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetColumnRec()
 *
 * DESCRIPTION:
 *
 *   Find Column index for the "external" DataFieldIndex.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   index of CfpDBColumn object if FOUND, else -1
 *
 **************************************************************************/
int CfpOleBinding::fpGetColumnRec(int iFieldPos)
{
  int i;

  i = (int)iFieldPos + (int)m_fHasBookmarks;
  if (i >= (int)m_cColumns)
    i = -1;

  return i;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetExtDataFldIndexForColId()
 *
 * DESCRIPTION:
 *
 *   Finds the "external" DataFieldIndex for the passed Column Id.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   DataFieldIndex for ColId passed if FOUND, else -1
 *
 **************************************************************************/
int CfpOleBinding::fpGetExtDataFldIndexForColId(ULONG ulOrdId)
{
  int iColIndex = m_BoundCols.GetColIndexForOrdinalId(ulOrdId);

  // If found and index is > 0, adjust "i" to exclude bookmark column in index.
  if (iColIndex > 0 && m_fHasBookmarks)
    --iColIndex;

  return iColIndex;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpMoveCurrRow()
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
BOOL CfpOleBinding::fpMoveCurrRow(USHORT usMoveAction, LPVOID hlstrBmark,
  UINT FAR *lpuRetFlags)
{
	HRESULT hr = S_OK;
    LONG lRelOffset;               // Relative offset position for move
    CBookmark bmkTarget;

    *lpuRetFlags = 0;

    // Act on type of move
    // Get the Bookmark for moving (gets put in hlstrTmp)
    // NOTE: If hlstrTmp is still NULL after this switch, then get bookmark.
    lRelOffset = 0;                // this may get changed
    switch (usMoveAction)             
    {                                 
      case DATA_BOOKMARK:
        // *** SPECIAL VB3 FEATURE: ***
        // Setting the Bookmark to 0, causes the cursor's current position
        // to be invalid (as if you deleted the current row).
        // To simulate this, we set the following flag:
        if (!hlstrBmark)
        {
           // If status changed, simulate a DBREASON_MOVE.
           if (!(m_dwFetchStatus & FLAG_CURR_ROW_INVALID) && m_iNotifyStatus)
             hr = fpProcessEvent(DBEVENTPHASE_ABOUTTODO, 
               DBREASON_ROWPOSITION_CLEARED, 0, FPDB_NOHROWS, FPDB_NOCOLIDS);
           fpCurrentRowChanged(FLAG_CURR_ROW_INVALID, FLAG_CURR_ROW_INVALID);
           return TRUE;
        }
        else
        {
           bmkTarget.Set(LBlockLen((LBLOCK)hlstrBmark), (const BYTE *)hlstrBmark);
        }
        break;

	  case DATA_MOVEFIRST:
	  case DATA_MOVELAST:
        {
          BYTE b = (usMoveAction == DATA_MOVEFIRST ? DBBMK_FIRST : DBBMK_LAST);
          bmkTarget.Set(sizeof(b), &b);
        }
        break;

	  case DATA_MOVEPREV:
	  case DATA_MOVENEXT:
        {
          DBPOSITIONFLAGS dwPosFlags;
          HROW hRow = 0;
          HCHAPTER hOldChapter = m_hChapter; 
          // Get HROW for current row. 
          if (!m_pRowsetPos || m_pRowsetPos->GetRowPosition(&m_hChapter, 
              &hRow, &dwPosFlags) != S_OK)
            return FALSE;
          // Release old if different
          if (hOldChapter != m_hChapter)
            fpReleaseChapter(&hOldChapter);
          if (hRow)
          {
            hr = fpGetBookmarkAtHRow(hRow, &bmkTarget, TRUE);
            if (hr != S_OK)
              return FALSE;
          }
          // RowsetPos is not giving us a row, use the one in m_bmkCurrRow.
          else if ((ULONG)m_bmkCurrRow)
            bmkTarget = m_bmkCurrRow;
        
          lRelOffset = (usMoveAction == DATA_MOVENEXT ? 1 : -1);
        }
        break;

      default: 
       // Invalid parameter passed, must be one of above values.
       ASSERT(0);
       return FALSE;
    } //switch

    if ((ULONG)bmkTarget)
    {
      ULONG cRows;
      HROW  *phRows = NULL;
      HROW  hRow = 0;
        
	  hr = CRowset::GetRows(bmkTarget, lRelOffset, 1, phRows, cRows);
      if (hr == S_OK && phRows && cRows && m_pRowsetPos)
      {
        // Save HROW and free allocated array
        hRow = phRows[0];
        CoTaskMemFree((void*)phRows);
        hr = m_pRowsetPos->ClearRowPosition();
        if (hr == S_OK)
        {
//        m_iNotifyStatus |= NOTIFY_SELF_MOVE; // set flag to we are causing the move
          hr = m_pRowsetPos->SetRowPosition(m_hChapter, hRow, DBPOSITION_OK);
//        m_iNotifyStatus &= ~NOTIFY_SELF_MOVE; // reset flag
        }
        CRowset::ReleaseRows(&hRow, 1);
      }
      else if (hr == DB_S_ENDOFROWSET || hr == DB_E_BADSTARTPOSITION)
      {
        // ENDOFCURSOR is True. We are either at the END or the BEGINNING,
        // determine which. 
        if (lRelOffset < 0)
	      *lpuRetFlags = DA_fBOF;
        else // either DATA_BOOKMARKNEXT, DATA_BOOKMARKFIRST or DATA_BOOKMARKLAST
	      *lpuRetFlags = DA_fEOF;
        return FALSE;
      }
	  else if (hr != S_OK)
	  {
	    return FALSE;
	  }
    }

	return (hr == S_OK);
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpDeleteRow()
 *
 * DESCRIPTION:
 *
 *   This function deletes the row at the specified bookmark (or at the
 *   current row if none is specified).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::fpDeleteRow(LPVOID hlstrBmark)
{
  HRESULT hr;
  int  iSaveStatus;
  ULONG ulSaveCtlFlags;
  DBPOSITIONFLAGS dwPosFlags;
  HROW hRow = 0;
  CBookmark bmkTarget;
  DBROWSTATUS rowStatus;

    // if in middle of EDIT, return error because DELETE is not allowed!
    ASSERT( !(m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))); 
    if (m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))
       return FALSE;

    // If a bookmark was passed, get a HROW for that row, else
    // get the HROW for the current row.
    if (hlstrBmark)
    {
      bmkTarget.Set(LBlockLen((LBLOCK)hlstrBmark), (const BYTE *)hlstrBmark);
    }
    else // no bookmark was passed, use current row.
    {
      // Get HROW for current row. 
      HCHAPTER hOldChapter = m_hChapter; 
      if (!m_pRowsetPos || m_pRowsetPos->GetRowPosition(&m_hChapter, 
          &hRow, &dwPosFlags) != S_OK)
        return FALSE;
      // Release old if different
      if (hOldChapter != m_hChapter)
        fpReleaseChapter(&hOldChapter);
      // RowsetPos did not give us a row, use the one in m_bmkCurrRow.
      if (!hRow && (ULONG)m_bmkCurrRow)
        bmkTarget = m_bmkCurrRow;
    }

    // If given a bookmark, get the HROW for it.
    if ((ULONG)bmkTarget)
    {
      ULONG cRows;
      HROW  *phRows = NULL;
        
	  hr = CRowset::GetRows(bmkTarget, 0, 1, phRows, cRows);
      if (hr == S_OK && phRows && cRows)
      {
        // Save HROW and free allocated array
        hRow = phRows[0];
        CoTaskMemFree((void*)phRows);
      }
    }

    // If no HROW, then just return.
    if (!hRow)
      return FALSE;

    // Turn-off "SyncBefore" Notification
    iSaveStatus = m_iNotifyStatus;

    // If deleting, save current flags and turn-off DATACHANGED,
    // then if deletion failed, restore flags
    ulSaveCtlFlags = m_ulCtlFlags;
    m_ulCtlFlags &= ~CTLFLG_DATACHANGED;

    // At this point, we have a HROW, now delete the row.
    hr = CRowset::DeleteRows(&hRow, 1, &rowStatus);
    // Update database if OK, not Batch Update and have RowsetUpdate ptr.
    if (hr == S_OK && !m_fBatchUpdate && InitRowsetUpdate() == S_OK)
    {
      DBROWSTATUS *pRowStatus = NULL;
      // Force an update to the rowset
	  hr = CRowset::Update(&hRow, 1, NULL, NULL, &pRowStatus);
      if (pRowStatus)                // check for success
      {
        if (hr == S_OK && *pRowStatus != DBROWSTATUS_S_OK)
          hr = S_FALSE;              // not successful
        CoTaskMemFree(pRowStatus);   // free allocated memory
      }
    }

    CRowset::ReleaseRows(&hRow, 1);

    if (hr != S_OK)
      fpSetError( FPDB_ERR_DELETE, (LONG)hr, NULL);

    // Restore notification
    m_iNotifyStatus = iSaveStatus;

    return (hr == S_OK);
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpInsertRow()
 *
 * DESCRIPTION:
 *
 *   This function inserts (adds) a row.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::fpInsertRow(void)
{
  HRESULT hr;
  int  iSaveStatus;
  BOOL fRet = FALSE;
  HROW hRow = 0;
  HACCESSOR	hEmptyAccessor = NULL;
  CBookmark bmk;

  // if in middle of EDIT, return error because ADD is not allowed!
  ASSERT( !(m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))); 
  if (m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD))
  {
     fpSetError( FPDB_ERR_UPDATEINPROGRESS, (LONG)0, NULL);
     return FALSE;
  }

  iSaveStatus = m_iNotifyStatus;
  m_iNotifyStatus |= NOTIFY_SYNCBEFORE_IGNORED | NOTIFY_SYNCAFTER_IGNORED;

  // Create an "empty" accessor to create the row
  hr = CRowset::CreateAccessor(NULL, 0, hEmptyAccessor, NULL);
  _ASSERT(hr == S_OK && hEmptyAccessor);
  if (hr == S_OK && hEmptyAccessor)
  {
    // NOTE: We cannot call CRowset::Update() to force the rowset
    // to update because the data has to be set first.  When
    // CRowset::SetData() is called, it will be followed by a call to
    // CRowset::Update() which will update the rowset.
    hr = CRowset::InsertRow(hEmptyAccessor, NULL, hRow); // Add new row
    CRowset::ReleaseAccessor(hEmptyAccessor);   // Free the accessor
  }

  // If insert failed at this point, indicate an error and remove flag.
  if (hr != S_OK || hEmptyAccessor == NULL)
    fpSetError( FPDB_ERR_BEGINUPDATE_ADD, (LONG)0, NULL);

  // Move to new row
  if (hr == S_OK && hEmptyAccessor)
  {
    // "Insert" was successful, set flag and return TRUE.
    fpCurrentRowChanged(FLAG_CURR_ROW_INVALID, 0); //clear flag
    fRet = TRUE;
    // set flag to indicate an add in progress
    m_iUpdateStatus |= UPDSTAT_BEGIN_ADD;
    // Save hRow as the CurrRow (but first free the current one).
    if (m_hCurrRow)
      CRowset::ReleaseRows(&m_hCurrRow, 1);
    // Note: this HROW gets released later.
    m_hCurrRow = hRow;
    CRowset::AddRefRows(&hRow, 1);

    // Try to get the bookmark for this hRow. If it fails, then set some
    // flag that indicates that if we get an Update event for this hRow,
    // then refresh the entire data so that the control gets the new
    // bookmark for that row.
    hr = fpGetBookmarkAtHRow(hRow, &bmk);
    if (hr != S_OK || (ULONG)bmk == 0)
      m_InsertedRowList.AddRow(hRow, FPDB_ADDREF, this);
  }

  // Restore messages
  m_iNotifyStatus = iSaveStatus;

  return fRet;
}




/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpSetFieldValue()
 *
 * DESCRIPTION:
 *
 *   Sets the data for ONE column based on the current row.
 *
 *   It determines the status of the write buffer and if SetData().
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
BOOL CfpOleBinding::fpSetFieldValue(LPVOID lpvda)
{
    LPDATAACCESS lpda = (LPDATAACCESS)lpvda;
    HRESULT hr;
    BOOL fReturn = TRUE;
    UINT uFlags = 0;
    int  iBoundCol;
    int  i;
    ULONG ulFlags;
    CfpDBColumn *pDBCol;

    // If DataField is NULL, use DataFieldIndex as the position
    if (lpda->hszDataField)
       i = fpGetColumnRec((LPCTSTR)lpda->hszDataField);
    else
       i = fpGetColumnRec((int)lpda->sDataFieldIndex);

    // if invalid column, return error
    _ASSERT(i >= 0);
    if (i >= 0)
    {
      // Insure that this column is bound.
      iBoundCol = m_BoundCols.FindColIndex(i);
      _ASSERT(iBoundCol >= 0);
    }

    // If either index is invalid, return error
    if (i < 0 || iBoundCol < 0)
    {
      m_iUpdateStatus |= UPDSTAT_SETCOL_FAILED;
      return FALSE;
    }

    // If this is the first call for this row, then verify that we
    // have a current row and create the "SetData" buffer.
    if ((m_iUpdateStatus & UPDSTAT_BEGIN_EDIT) == 0)
    {
      // Verify that we have a current row to change
      if (!m_hCurrRow)
      {
        fpRequeryCurrHRow();
        _ASSERT(m_hCurrRow);   // no current row to modify
      }

      // Re-check current row and that we have a buffer.
      if (!m_hCurrRow || !m_ulBoundInlineLen)
        fReturn = FALSE;

      // If not created yet, create the List used to manage the SetCols.
      if (fReturn)
      {
        if (m_pSetDataCols)
          m_pSetDataCols->Clear();          // reset the list
        else
          m_pSetDataCols = new CfpDBColList;// create list
        if (!m_pSetDataCols)                // if failed, set flag
          fReturn = FALSE;
      }  

      // Create the buffer which will be used with the call to SetData().
      if (fReturn)
      { 
        m_lpszSetDataBuffer = (LPSTR)CoTaskMemAlloc(m_ulBoundInlineLen);
        _ASSERT(m_lpszSetDataBuffer);       // Did memory allocate?
        if (!m_lpszSetDataBuffer)
          fReturn = FALSE;
      }

      // If an error occurred, set flag and return FALSE.
      if (fReturn == FALSE)
      {
        m_iUpdateStatus |= UPDSTAT_SETCOL_FAILED;
        return FALSE;
      } 

      ZeroMemory(m_lpszSetDataBuffer, m_ulBoundInlineLen);

      m_iUpdateStatus |= UPDSTAT_BEGIN_EDIT;
    }

    pDBCol = &m_pColumns[i];

    // Fill the portion of m_lpszSetDataBuffer with the specified value.
    hr = (HRESULT)pDBCol->fpSetDataValue(m_lpszSetDataBuffer, lpda->usDataType, 
                lpda->ulChunkOffset, lpda->ulChunkNumBytes,
                lpda->lData, lpda->fs);
    switch((LONG)hr)
    {
      case 0: // if successful, add column to SetCol list.
        ulFlags = (pDBCol->fpHasStreamSetData() ? // if a stream is required...
          FPDB_HASSTREAMDATA : 0);
        m_pSetDataCols->AddWithFlags(i, ulFlags, FPDB_FIND_INDEX); // "find index" prevents duplicate entries
        // indicate that SetColumn was successful
        m_iUpdateStatus |= UPDSTAT_SETCOLUMN;
        break;

      case FPDB_ERR_CANTSETAUTOINCR:
        // this condition is ok, but no value is saved (defaulted)
        break;

      // case FPDB_ERR_CONVERTSTRINGTOTYPE:
      default:
        m_iUpdateStatus |= UPDSTAT_SETCOL_FAILED;
        fReturn = FALSE;
        break;
    }

    return fReturn;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFinishUpdatingRow()
 *
 * DESCRIPTION:
 *
 *   This is called to finish the updating of a row.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::fpFinishUpdatingRow(void)
{
  HRESULT hr = S_OK;
  int i;
  int cCols;
  int colIndex;
  BOOL fUpdateRow = TRUE;
  CfpDBColList *pTmp;
  BOOL fBatchUpdate;


  // If a SetColumn error occurred, set external error and cancel
  // the update/add.
  if (m_iUpdateStatus & UPDSTAT_SETCOL_FAILED)
  {
    fpSetError( FPDB_ERR_SETCOLUMN, (LONG)0, NULL);
    // If a new row was added, undo it.
    if (m_iUpdateStatus & UPDSTAT_BEGIN_ADD)
    {
      DBROWSTATUS *pRowStatus = NULL;
      // If not batch update, cancel the insert.
      if (!m_fBatchUpdate)
        hr = CRowset::Undo(&m_hCurrRow, 1, NULL, NULL, &pRowStatus);
      if (pRowStatus)
        CoTaskMemFree(pRowStatus);   // free allocated memory
    }
    // Set flag which will prevent the data from being set.
    fUpdateRow = FALSE;
  }

  // If Update was not started, just return
  if (!(m_iUpdateStatus & (UPDSTAT_BEGIN_EDIT | UPDSTAT_BEGIN_ADD)))
  {
     m_iUpdateStatus = 0;  // reset "update status"
     return FALSE;
  }

  // If at least one value was successfully set...
  if ((m_iUpdateStatus & UPDSTAT_SETCOLUMN) && m_pSetDataCols &&
      !m_pSetDataCols->IsEmpty())
  {
    // Verify the accessor used for calls to SetData().
    _ASSERT(m_hCurrRow);       // This should not be 0!!!
    if ((m_hCurrRow == 0) || !fpVerifySetDataAccessor())
    {
      fpSetError( FPDB_ERR_UPDATE, (LONG)0, NULL);
      fUpdateRow = FALSE;
    }

    // if allowed to update the row, do it.
    if (fUpdateRow)
    {
      BOOL fSetUsesStream = m_pSetDataCols->HasStreamData();

      // For each Column with a VALUESET, prepare column for SetData().
      cCols = *m_pSetDataCols;
      for (i = 0; i < cCols; i++)
      {
        colIndex = (*m_pSetDataCols)[i].colIndex;
        m_pColumns[colIndex].fpPrepareSetState();
      }

      // If streams are being used for set data and we don't have support
      // for multiple storage objects (stream), then free the current
      // fetch buffer.
      if (!m_fSuppMultiStreams && fSetUsesStream)
        fpFreePreviousFetchData(FLAG_FETCHED_STREAMBUFFER);

      fBatchUpdate = m_fBatchUpdate;  // use setting, but it may change

      // Verify that the RowsetUpdate (or RowsetChange) interface is OK.
      hr = CRowset::InitRowsetUpdate();
      if (hr != S_OK)
      {
        hr = CRowset::InitRowsetChange();

        // If successful, it means that the interface being used for
        // SetData() is the RowsetChange interface (instead of 
        // RowsetUpdate), so set "batch" flag to TRUE so that the
        // call to m_pSetDataCols->SetData() does not call
        // the Update() function after SetData().
        if (hr == S_OK)
          fBatchUpdate = TRUE;
      }
       
      // Using accessor, call SetData() to write the values to the 
      // recordset.
      if (hr == S_OK)
        hr = m_pSetDataCols->SetData(this, m_hCurrRow, m_lpszSetDataBuffer,
          fBatchUpdate);

      // If unsuccessful, report error
      if (hr != S_OK)
      {
        fpSetError( FPDB_ERR_UPDATE, (LONG)0, NULL);
        fUpdateRow = FALSE;
      }
    }

    // For each Column with a VALUESET, reset the flags and values
    cCols = *m_pSetDataCols;
    for (i = 0; i < cCols; i++)
    {
      colIndex = (*m_pSetDataCols)[i].colIndex;
      m_pColumns[colIndex].fpClearSetState(m_lpszSetDataBuffer);
    }
  } // if UPDSTAT_SETCOLUMN

  // Make the "current" SetCols, the "previous" SetCol.
  pTmp = m_pPrevSetDataCols;
  m_pPrevSetDataCols = m_pSetDataCols; // current becomes prev
  m_pSetDataCols = pTmp;               // prev becomes current

  // if anything in m_pSetDataCols, then clear it.
  if (m_pSetDataCols && !m_pSetDataCols->IsEmpty())
    m_pSetDataCols->Clear();

  // Free the buffer
  CoTaskMemFree(m_lpszSetDataBuffer);
  m_lpszSetDataBuffer = NULL;

  // Reset flags
  m_iUpdateStatus = 0; 

  return fUpdateRow;

} //fpFinishUpdatingRow()


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpVerifySetDataAccessor()
 *
 * DESCRIPTION:
 *
 *   This function will create the accessor(s) used to set the data.
 *
 *   Since each column in an accessor must have a value when setting
 *   data, we must create an accessor to match the columns with values.
 *   The accessor used for the previous "set" is saved and may be used
 *   for the next "set" provided that we have data for each column in
 *   the accessor.
 *
 *   There may actually be more than just one "set" accessor.  If the
 *   DB Provider does not support more than one stream at a time and
 *   we have more than one column using streams, then we must use
 *   multiple accessors to set the data.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *   TRUE if successful, else FALSE
 *
 **************************************************************************/
BOOL CfpOleBinding::fpVerifySetDataAccessor(void)
{
  HRESULT hr = S_OK;
  int cCols;
  int ct;
  int colIndex;
  int i;
  int iStreamAccessorCt;
  int iOtherAccessorCt;
  BOOL fReturn = TRUE;
  CfpDBColumn *pDBCol;

  // If we already have an accessor for SetData, then VERIFY that
  // every column with a set value is in the accessor and that
  // we have the same number of columns with values set as number
  // of columns in the accessor.  If they DON'T match, then free
  // the current accessor (so we can create a new one).
  if (m_pPrevSetDataCols && m_pPrevSetDataCols->HasValidAccessor())
  {
    // If the current list of SetCols matches the previous list, then
    // use the previous list as the current list.
    if (m_pSetDataCols->CompareList(m_pPrevSetDataCols))
    {
      // Switch lists.
      CfpDBColList *pTmp = m_pSetDataCols;
      m_pSetDataCols = m_pPrevSetDataCols;  // prev becomes current
      m_pPrevSetDataCols = pTmp;            // current becomes prev
    }
    // Else, there's a mismatch, free accessors from previous list
    else
    {
      m_pPrevSetDataCols->ReleaseAllAccessors(this);
    }
  }

  // Now, if we don't have an accessor for SetData(), gather the
  // info needed to create one that matches the set columns.
  if (!m_pSetDataCols->HasValidAccessor())
  {
    // Allocate array (max size is the number of bound columns).
    LPBLOB pBlob = NULL;
    DBBINDSTATUS *dbStatus = NULL;
    size_t  uBindingsSize = sizeof(DBBINDING) * (int)*m_pSetDataCols;
    DBBINDING *bindings = (DBBINDING *)malloc(uBindingsSize);
    _ASSERT(bindings);
    if (!bindings)
      return FALSE;
    ZeroMemory(bindings, uBindingsSize);

    // Get the number of special (seperate) accessors needed.
    fpGetWriteAccessorCt((UINT *)&iStreamAccessorCt, 
      (UINT *)&iOtherAccessorCt);

    // If the DB supports multiple streams, then clear the stream ct.
    if (m_fSuppMultiStreams)
      iStreamAccessorCt = 0;
    // Else, if stream accessor count is 1, then let it use the
    // "default" accessor.
    else if (iStreamAccessorCt == 1)
      iStreamAccessorCt = 0;

    cCols = *m_pSetDataCols;  // get the number of columns

    // If total number of accessors needed is the same as the total 
    // number of cols to set, then skip creation of the "default" accessor.
    if ((iStreamAccessorCt + iOtherAccessorCt) == cCols)
      cCols = 0;

    ct = 0;
    // Loop to create the "default" accessor.
    for (i = 0; i < cCols; i++)
    {
      colIndex = (*m_pSetDataCols)[i].colIndex;
      pDBCol = &m_pColumns[colIndex];

      // If we don't need multiple streams OR this column does
      // not use streams, AND no seperate accessor, then include 
      // this column in the default accessor.
      if ((iStreamAccessorCt == 0 || !pDBCol->fpIsStreamCol()) &&
          (iOtherAccessorCt == 0 || !pDBCol->fpNeedsOwnAccessor()))
      {
        // Fill DBBINDING array with bindings stored in each column.
        if (pDBCol->fpFillSetDataBinding(&bindings[ct], &pBlob))
          ++ct;
        else  // an error occurred
        {
          fReturn = FALSE;
          break;
        }
      }
    }

    if (fReturn && ct)    // create array for status
    {
      dbStatus = (DBBINDSTATUS *)malloc(sizeof(DBBINDSTATUS) * ct);
      _ASSERT(dbStatus);
      fReturn = (dbStatus != NULL);
    }

    // Create accessor using array of DBBINDINGs
    if (dbStatus)
    {
      HACCESSOR hAccessor = NULL;
      hr = CRowset::CreateAccessor(bindings, (ULONG)ct, 
        hAccessor, dbStatus);
      if (hr == S_OK && hAccessor) // if accessor is ok, then set accessor
        m_pSetDataCols->SetAccessor(FPDB_DEFAULT_ACCESSOR, hAccessor);
      free(dbStatus);
    } 

    // If we need stream accessors, create them.  If this fails,
    // then we will release all accessors and return an error.
    if (fReturn && (iStreamAccessorCt > 0 || iOtherAccessorCt > 0))
      if (fpCreateOtherWriteAccessors(&pBlob, iStreamAccessorCt,
           iOtherAccessorCt) != S_OK)
      {
        fpReleaseWriteAccessors();    // release the accessors
        fReturn = FALSE;
      }

    if (pBlob)                      // free allocated blob.
      free(pBlob);

	// free bindings allocated -scl
	if (bindings)
	  free(bindings);
  }

  return m_pSetDataCols->HasValidAccessor();
} //fpVerifySetDataAccessor()




/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetWriteAccessorCt()
 *
 * DESCRIPTION:
 *
 *   This function returns the number of streams needed to write
 *   the set data. It gives the count of the Stream accessors and
 *   the count of other seperate accessors needed to write the data.
 *
 * PARAMETERS:
 *
 * RETURNS: count
 *
 **************************************************************************/
void CfpOleBinding::fpGetWriteAccessorCt(UINT *puRetStreamCt, 
  UINT *puRetSeperateAccCt)
{
  int i;
  int ct = 0;
  int cCols = *m_pSetDataCols;  // get num of columns in list
  int colIndex;

  UINT uStreamCt = 0;
  UINT uSeperateAccCt = 0;

  // For each Column with a VALUESET...
  for (i = 0; i < cCols; i++)
  {
    colIndex = (*m_pSetDataCols)[i].colIndex;

    // If this column uses a stream, then increment the count.
    if (m_pColumns[colIndex].fpIsStreamCol())
      ++uStreamCt;
    // If this col needs a seperate accessor, then increment that count.
    else if (m_pColumns[colIndex].fpNeedsOwnAccessor())
      ++uSeperateAccCt;
  }

  *puRetStreamCt = uStreamCt; 
  *puRetSeperateAccCt = uSeperateAccCt;

  return;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpCreateOtherWriteAccessors()
 *
 * DESCRIPTION:
 *
 *   This function creates the accessors required to bind to the columns
 *   when multi-stream accessors are not supported.
 *
 *   This function loops thru the list of bound columns; for each column
 *   using streams, the DBBINDING structure is filled with the 
 *   necessary info and an accessor is created for that column.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpCreateOtherWriteAccessors(LPBLOB *plpBlob,
  UINT uStreamAccessorCt /* = 0 */, 
  UINT uOtherAccessorCt  /* = 0 */ )
{
  // Col index (decremented to 0), skip bookmark col (last col in list)
  UINT i;                         // list index
  int  index;                     // index in m_pColumns
  DBBINDING tmpBindings;
  HACCESSOR hAccessor;
  CfpDBColumn *pDBCol;
  HRESULT hr;
  UINT cCols = *m_pSetDataCols; // get number of columns
  DBBINDSTATUS dbStatus;
  UINT uTmpAccessorCt = 0;
  UINT uTotalCt = uStreamAccessorCt + uOtherAccessorCt;
            
  // For each Column with a VALUESET...
  for (i = 0; i < cCols; i++)
  {
    // NOTE: "(*m_pSetDataCols)[i].colIndex" is the index of m_pColumns 
    // for this column.
    index = (*m_pSetDataCols)[i].colIndex;
    pDBCol = &m_pColumns[index];

    // If this column uses streams...
    if ((uStreamAccessorCt > 0 && pDBCol->fpIsStreamCol()) ||
        (uOtherAccessorCt > 0  && pDBCol->fpNeedsOwnAccessor()))
    {
      ZeroMemory(&tmpBindings, sizeof(tmpBindings));
      hAccessor = NULL;

      // Fill array element with the binding info for this column
      if (pDBCol->fpFillSetDataBinding(&tmpBindings, plpBlob) == FALSE)
        return E_OUTOFMEMORY;

      // Create the accessor stream column
      hr = CRowset::CreateAccessor(&tmpBindings, 1, hAccessor, &dbStatus);
      _ASSERT(hr == S_OK);
      if (hr != S_OK)
        return hr;

      // if successful, let the column "maintain" the accessor.
      m_pSetDataCols->SetAccessor(i, hAccessor);

      // Increment stream count, and if count matches the one passed in,
      // then break.
      ++uTmpAccessorCt;
      if (uTmpAccessorCt == uTotalCt)
        break;                    // we found all of them, exit loop
    } // if StreamCol
  }

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpReleaseWriteAccessors()
 *
 * DESCRIPTION:
 *
 *   This function releases the accessors created for writing data.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpReleaseWriteAccessors(void)
{
  // Release all accessors used for writing to DB.
  if (m_pSetDataCols)
    m_pSetDataCols->ReleaseAllAccessors(this);

  if (m_pPrevSetDataCols)
    m_pPrevSetDataCols->ReleaseAllAccessors(this);
}
