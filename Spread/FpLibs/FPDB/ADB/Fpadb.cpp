/*************************************************************************\
* FPADB.CPP - FarPoint Data Binding source file for ADO Binding.                                                            *
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

// Define the GUIDs for the ADO interfaces.
#undef DEFINE_GUID
// Re-define "DEFINE_GUID" so that the Ado GUIDs get implemented.
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#include <adoid.h>
// Re-define "DEFINE_GUID" so that the GUIDs have forward declarations.
#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID FAR name

#include <adoint.h>

#include <limits.h>
#include <malloc.h>

#include "util.h"

#define DATA_UINIT_CONNECT       300
#define DATA_UPOSTMSG            301


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::OnFreezeEvents()
 *
 * DESCRIPTION:
 *
 *   If the fFreeze status is different than our current status, change
 *   our status and if un-freezing the event and the DataSource has
 *   changed, then init/reinit the "Data Bindings".
 *
 * PARAMETERS:
 *
 * RETURNS:   HRESULT
 *
 **************************************************************************/
void CfpOleBinding::OnFreezeEvents(BOOL fFreeze)
{
  // If the fFreeze status is different than our current status, change
  // our status and if un-freezing the event and the DataSource has
  // changed, then init/reinit the "Data Bindings".
  if (fFreeze != m_fFreezeEvents)
  {
    m_fFreezeEvents = fFreeze;
    if (!fFreeze && m_fDataSourceChanged) // turning-off freeze AND data changed
    {
      fpReInitDataBinding();              // Re-init data bindings
      m_fDataSourceChanged = FALSE;       // reset flag
    }
  }
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpInitDataBinding()
 *
 * DESCRIPTION:
 *
 *   Public function called by control to initialize the data binding
 *   (connect to rowset).  This is usually called by the control's
 *   FreezeEvents handler (when actually unfreezing events).
 *
 * PARAMETERS:
 *
 * RETURNS:   HRESULT
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpInitDataBinding(void)
{
  HRESULT hr;

  BOOL fRunMode = fpQueryUserMode(); // User mode (not design mode)

  // If Events are frozen AND Runtime, then simply set the flag that indicates
  // that the Data Bindings need to be re-initialized.
  if (m_fFreezeEvents && fRunMode)
  {
    m_fDataSourceChanged = TRUE;
    return S_OK;
  }

  if (m_pColumns)            // if we already have the Metadata, return
    return S_OK;

  hr = fpInitRowset();       // Initialize ADB interface ptrs.

  if (hr == S_OK)            // Get Metadata info
  {
  	COLUMNINFO	*pColumns;   // COLUMNINFO array
	ULONG       cColumns;	 // Number of elements in pColumns

    // Get Metadata
	hr = CRowset::GetColumns(pColumns, cColumns);

    // If successful, copy column list to our own list (m_pColumns).
	if (SUCCEEDED(hr) && cColumns > 0 && pColumns)
    {
      hr = fpCopyColumns(pColumns, cColumns);  // copy to our own list
      delete [] pColumns;       // free allocated list
    }
    m_fHasBookmarks = (GetProperties().flag.HasBookmarks ? 1 : 0);
  }

  // if Runtime-mode, "Bind" to Column Name and get data
  if (m_pColumns && fRunMode)
  {
    fpGetMultiStreamProp(&m_fSuppMultiStreams);
    // Determine if Batch Updating (LockType = adLockBatchOptimistic).
    fpGetBatchUpdateProp(&m_fBatchUpdate);

    hr = fpBindDataFieldColumn(m_strBoundDataField);  // bind
    if (hr == S_OK)
    {
      DATAACCESS da = {0};
      UINT  uDataMsg;
      UINT  uFlags;

      _ASSERT(m_lpfuncDataMsg); //Callback func MUST BE SET BY OWNER!
      if (!m_lpfuncDataMsg)
        return hr;

      da.sAction = DATA_UINIT_CONNECT;
      da.lpfpOleBinding = this;
      uDataMsg = VBM_DATA_INITIATE;
      // Call the owner's function to process the VBM_DATA_INITIATE message
      m_lpfuncDataMsg( (LPVOID)m_lpOwner, uDataMsg, (WPARAM)0, (LPARAM)(LPVOID)&da);

      ZeroMemory(&da, sizeof(da));
      da.sAction = DATA_REFRESH;
      da.lpfpOleBinding = this;
      uDataMsg = VBM_DATA_AVAILABLE;
      // Call the owner's function to process the VBM_DATA_AVAILABLE message
      m_lpfuncDataMsg( (LPVOID)m_lpOwner, uDataMsg, (WPARAM)0, (LPARAM)(LPVOID)&da);

      // If no current row, then set current row to the first row.
      fpRequeryCurrHRow();
      if (!m_hCurrRow)
      {
        int iSaveStatus = m_iNotifyStatus; 
        m_iNotifyStatus |= NOTIFY_SUSPENDED; // temporarly, suspend notification.
        // Make first row the current row.
        fpMoveCurrRow(DATA_MOVEFIRST, NULL, &uFlags);
        m_iNotifyStatus = iSaveStatus;       // restore notification
      }

      // Tell owner to get the current row.
      ZeroMemory(&da, sizeof(da));
      da.sAction = DATA_BOOKMARK;
      da.lpfpOleBinding = this;
      m_lpfuncDataMsg( (LPVOID)m_lpOwner, uDataMsg, (WPARAM)0, (LPARAM)(LPVOID)&da);
    }
  }

  return hr;
}

HRESULT CfpOleBinding::GetRowsetProperties(IUnknown *pRowset, const DBPROPID *pPropIDs, VARIANT *pValues, ULONG cPropIDs)
{
	// Validate arguments
	//
	if (!(pPropIDs && pValues && cPropIDs))
		return E_INVALIDARG;

	HRESULT hr;
	IRowsetInfo * pRowsetInfo = NULL;

	if (pRowset && pRowset != m_pRowset)
		// Traverse to desired interface
		//
		hr = pRowset->QueryInterface(IID_IRowsetInfo, (void**)&pRowsetInfo);
	else
	{
		// Use attached Rowset
		//
		hr = InitRowsetInfo();
		pRowsetInfo = m_pRowsetInfo;
	}
	RETURN_ON_FAILURE(hr);

	// Initialize DBPROPSET parameter
	//
	DBPROPIDSET propidSet;
	propidSet.rgPropertyIDs = (DBPROPID*)pPropIDs;
	propidSet.cPropertyIDs = cPropIDs;
	propidSet.guidPropertySet = DBPROPSET_ROWSET;

	// Retrieve property values
	//
	DBPROPSET *ppropSet = NULL;	// Array DBPROPSET
	ULONG 	cpropSets = 0;		// Number DBPROPSET in array
	
	hr = pRowsetInfo->GetProperties(1, &propidSet, &cpropSets, &ppropSet);

	if (SUCCEEDED(hr) && NULL != ppropSet)
	{
		// Do a byte copy of the variant property 
		// value and leave it up to the caller to call
		// VariantClear when it is done with the value 
		// to do any necessary clean up
		//
		for (ULONG i = 0; i < ppropSet[0].cProperties; i++)
			pValues[i] = ppropSet[0].rgProperties[i].vValue;

		// Free callee allocated buffer
		//
		CoTaskMemFree((void*)ppropSet[0].rgProperties);
		CoTaskMemFree((void*)ppropSet);
	}
	// If pRowsetInfo is not ours then release it
	//
	if (NULL != pRowsetInfo && pRowsetInfo != m_pRowsetInfo)
		pRowsetInfo->Release();

	return hr;
}

#define IS_TRUE(pv) (VT_BOOL == V_VT(pv) && VARIANT_FALSE != V_BOOL(pv))

const ROWSETPROPERTIES& CfpOleBinding::GetProperties(void)
{
	// List of DBPROPSET_ROWSET Properties we 
	// are interested in
	//
	static const DBPROPID rgpropid[] =
	{
		// Value props
		//
		DBPROP_MAXOPENROWS,	
		DBPROP_UPDATABILITY,	

		// BOOL props
		//
		DBPROP_STRONGIDENTITY,
		DBPROP_LITERALIDENTITY,
		DBPROP_BOOKMARKS,
		DBPROP_CANHOLDROWS,
		DBPROP_CANSCROLLBACKWARDS,
		DBPROP_CANFETCHBACKWARDS,
		DBPROP_IRowsetChange,
		DBPROP_IRowsetUpdate,
		DBPROP_IRowsetScroll,
	};
	static const ULONG cPropIDs = sizeof rgpropid/sizeof rgpropid[0];

	// Check to see if we need to initialize
	// the properties structure
	//
	if (NULL == m_pRowsetInfo)
	{
		HRESULT hr = InitRowsetInfo();
		
		if (SUCCEEDED(hr))
		{
			// Allocate VARIANT values array
			//
			VARIANT *pValues = new VARIANT[cPropIDs];
			
			if (NULL == pValues)
				hr = E_OUTOFMEMORY;
			else
			{
				// Zero out VARIANTs and get property values
				//
				ZeroMemory((void*)pValues, cPropIDs * sizeof(VARIANT));
				hr = GetRowsetProperties(NULL, rgpropid, pValues, cPropIDs);

				// Save property values in structure
				//
				if (SUCCEEDED(hr))
				{
					m_properties.value.MaxOpenRows	= VT_I4 == V_VT(pValues) ? V_I4(pValues) : 0;
					m_properties.value.Updatability	= VT_I4 == V_VT(pValues+1) ? V_I4(pValues+1) : 0;

					m_properties.flag.StrongId			= IS_TRUE(pValues+2);
					m_properties.flag.LiteralId			= IS_TRUE(pValues+3);
					m_properties.flag.HasBookmarks		= IS_TRUE(pValues+4);
					m_properties.flag.CanHoldRows		= IS_TRUE(pValues+5);
					m_properties.flag.CanScrollBack		= IS_TRUE(pValues+6);
					m_properties.flag.CanFetchBack		= IS_TRUE(pValues+7);
					m_properties.flag.CanChange			= IS_TRUE(pValues+8);
					m_properties.flag.CanUpdate			= IS_TRUE(pValues+9);
					m_properties.flag.IRowsetScroll		= IS_TRUE(pValues+10);
				}
        
        // fix for bug 9062 -scl
        {
          ULONG i;
          for( i = 0; i < cPropIDs; i++ )
            VariantClear(&pValues[i]);
        }
				delete [] pValues;
			}
		}
	}
	return m_properties;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpReInitDataBinding()
 *
 * DESCRIPTION:
 *
 *   This is called to release the current data binding and re-initialize
 *   the data binding.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpReInitDataBinding(void)
{
  BOOL fRunMode = fpQueryUserMode(); // User mode (not design mode)

  // If Events are frozen, then simply set the flag that indicates
  // that the Data Bindings need to be re-initialized.
  if (m_fFreezeEvents && fRunMode)
  {
    m_fDataSourceChanged = TRUE;
    return S_OK;
  }

  // Disconnect from DataBinding
  fpReleaseDataBinding();

  // Connect to DataBinding
  return fpInitDataBinding();
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpReleaseDataBinding()
 *
 * DESCRIPTION:
 *
 *   This is called to release the current data binding.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpReleaseDataBinding(void)
{
  // Clean current rowset attachment, if any
  //
  if (CRowset::GetRowset())
  {
    // DB Binding is going away.  Notify OLE control.
    if (m_iNotifyStatus & NOTIFY_ADVISE_ON)
    {
      // Simulate DISCONNECT notification msg
      fpProcessEvent(DBEVENTPHASE_ABOUTTODO, UDBREASON_DISCONNECT, 0, FPDB_NOHROWS, FPDB_NOCOLIDS);
      fpProcessEvent(DBEVENTPHASE_DIDEVENT, UDBREASON_DISCONNECT, 0, FPDB_NOHROWS, FPDB_NOCOLIDS);
    }

    // Release the HROW of the current row
    if (m_hCurrRow)
      fpRequeryCurrHRow(FPDB_RELEASE_ONLY);  

    // Clear list of inserted rows (if not already empty).
    m_InsertedRowList.ClearList(this);

    // Free anything related to dbbinding
    fpFreeBoundCols();

    // Free ColumnInfo array
    fpFreeColumns();

    // Notify Control that we are no longer bound.
    // IMPLEMENT!!! ==> ClearContent();

    // Detach from the rowset
    fpDetachRowset();

    m_iUpdateStatus = 0;
  }

  // Ensure that we are disconnected from the notifications (we may already
  // be disconnected, but lets make sure).
  fpDisconnect();

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpInitRowset()
 *
 * DESCRIPTION:
 *
 *   If rowset has not been attached, this function calls the function 
 *   which attaches to the rowset of the current data source and also 
 *   gets the HROW of the current row.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpInitRowset(void)
{
  // If GetRowset returns a value then
  // the rowset is already attached, so
  // return success
  if (CRowset::GetRowset())
    return S_OK;

  if (m_pDataSrcProp)
  {
    BOOL fUserMode;
    HRESULT hr = fpAttachRowset();
    RETURN_ON_FAILURE(hr);  // if an error, return it.
 
    fUserMode = fpQueryUserMode();

    if (fUserMode)
    {
      // IDataSource::getDataMember succeeds, but if it did not
      // get an interface for IRowPosition then return S_FALSE.
      if (!CRowset::GetRowPosition())
        return S_FALSE;

      // Save the current row's handle for later use.
      fpRequeryCurrHRow();  
    }

    // Rowset is OK, return S_OK.
    return S_OK;
  }

  // No data source, so return failure
  return E_FAIL;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpAttachRowset()
 *
 * DESCRIPTION:
 *
 *   This function attaches to the rowset of the current data source.
 *   This is usually called by fpInitDataBinding().
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpAttachRowset(void)
{
    HRESULT hr;

    // If DEBUG only...
    _ASSERT(m_DBEventList->Flush());

	// Clean current rowset attachment, if any
	//
	if (CRowset::GetRowset())
      fpDetachRowset();

	// First let base class attach to the
	// new interface
	//
	hr = CRowset::Attach(m_pDataSrcProp, m_dmDataMember);
	RETURN_ON_FAILURE(hr);

	InitColumnsInfo(); // ignore result

	// Connect notifications
	//
    hr = fpConnect(fpQueryUserMode());

	return hr;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpDetachRowset()
 *
 * DESCRIPTION:
 *
 *   Detaches from the current rowset.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpDetachRowset(void)
{
  // Clean current rowset attachment, if any
  //
  if (CRowset::GetRowset())
  {
    // Reset (Flush) the Event queue.
    m_EventQueue.ResetQueue(this);

    // Disconnect notifications
    fpDisconnect();

    // Let base class do clean up
    CRowset::Attach(NULL);
  }

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFreeColumns()
 *
 * DESCRIPTION:
 *
 *   This function frees the list of columns (metadata) for the rowset.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFreeColumns(void)
{
	if (m_pColumns)
	{
      // Free the objects
      delete [] m_pColumns;
	
      m_pColumns = NULL;
      m_cColumns = 0;
      m_fHasBookmarks = 0;
      m_fSuppMultiStreams = FALSE;
      m_fBatchUpdate = TRUE;
	}
	return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpCopyColumns()
 *
 * DESCRIPTION:
 *
 *   Given the array of COLUMNINFO items, copy them to our own list
 *   (member var: m_pColumns).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpCopyColumns(COLUMNINFO *pColumns, ULONG cColumns)
{
  ULONG i;
  CfpDBColumn  *pfpCol;          // tmp ptr
  COLUMNINFO   *pCol;            // tmp ptr

  // Free current set of columns
  if (m_pColumns)
    fpFreeColumns();

  // if no columns, just return
  if (cColumns == 0)
    return S_OK;

  // Allocate a new set of columns, if it fails, return error
  m_pColumns = new CfpDBColumn[cColumns];
  if (m_pColumns == NULL)
    return E_OUTOFMEMORY;

  m_cColumns = 0;         // reset member count

  pfpCol = m_pColumns;           // tmp col ptr
  pCol   = pColumns;             // tmp col ptr
  for (i = 0; i < cColumns; i++)
  {
    // Copy the column only if type is not "HCHAPTER".
    if (pCol->wType != DBTYPE_HCHAPTER)
    {
      *pfpCol = *pCol;             // copy column to our list
      ++pfpCol;                    // next dest column
      ++m_cColumns;                // increment column count
    }
    ++pCol;                        // next source column
  }  

  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpConnect()
 *
 * DESCRIPTION:
 *
 *  This function is used to connect to the connection points of the
 *  ADO control so that we can receive notification when something changes.
 *  Note, if this is Design time, then the connections are limited to the
 *  DataSource itself (m_pDataSource->addDataSourceListener()).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpConnect(BOOL fUserMode)
{
  m_iNotifyStatus = 0; // reset status, then add flags as we go.

  // If UserMode (Run-time) and have Rowset ptr, then add connection
  if (fUserMode && GetRowset())
  {
    if (m_connections.Add(IID_IRowsetNotify, (IUnknown*)GetRowset(), 
         (IUnknown*)(IRowsetNotify*)this) == S_OK)
      m_iNotifyStatus |= NOTIFY_ADVISE_ROWSET;
  }

  // If UserMode (Run-time) and have RowPosition ptr, then add connection
  if (fUserMode && GetRowPosition())
  {
    if (m_connections.Add(IID_IRowPositionChange, (IUnknown*)GetRowPosition(), 
         (IUnknown*)(IRowPositionChange*)this) == S_OK)
      m_iNotifyStatus |= NOTIFY_ADVISE_ROWPOSITION;
  }

  if (m_pDataSource)
  {
    // If add was successful, then if UserMode (Run-time), set advise ON,
    // else set design flag.
    if (m_pDataSource->addDataSourceListener((IDataSourceListener*)this) == S_OK)
      m_iNotifyStatus |= NOTIFY_ADVISE_DATASOURCE;
  }

  // If any "advise connection" were set, then set advise ON.
  if (m_iNotifyStatus)
    m_iNotifyStatus |= 
      (fUserMode ? NOTIFY_ADVISE_ON : NOTIFY_ADVISE_DESIGNMODE);

  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpDisconnect()
 *
 * DESCRIPTION:
 *
 *   This function disconnects from the connection points of the 
 *   ADO control.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpDisconnect(void)
{
  // Remove the connections

  // If either of the two Advise modes, then remove the connections
  if (m_iNotifyStatus & NOTIFY_ADVISE_DATASOURCE)
  {
	if (m_pDataSource)
		m_pDataSource->removeDataSourceListener((IDataSourceListener*)this);
  }

  if (m_iNotifyStatus & NOTIFY_ADVISE_ROWPOSITION)
    m_connections.Remove(IID_IRowPositionChange);

  if (m_iNotifyStatus & NOTIFY_ADVISE_ROWSET)
	  m_connections.Remove(IID_IRowsetNotify);

  m_iNotifyStatus = 0;  // reset notify status

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFreeBoundCols()
 *
 * DESCRIPTION:
 *
 *   This function frees all of the row accessors.  Before it
 *   frees the accessors, it releases any data "held" by the individual
 *   columns.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFreeBoundCols(void)
{
  if (!fpHasFetchBuffer())
    return S_OK;

  // Let each column free its portion of the fetch buffer.
  fpFreePreviousFetchData(0);

  // Fetch array is no longer needed
  fpFreeFetchArray();

  // Free any RowAccessors
  fpFreeRowAccessor();

  // Free allocate buffer
  fpFreeFetchBuffer();

  // reset this index
  m_iFetchColIndex = -1;

  // Free array of bound columns
  if ((int)m_BoundCols)
    m_BoundCols.Clear();

  // Free the blob which is used when creating accessors
  if (m_pBlobBinding)
  {
    free(m_pBlobBinding);
    m_pBlobBinding = NULL;
  }

  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpBindDataFieldColumn()
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
BOOL CfpOleBinding::fpBindDataFieldColumn( LPTSTR strDataField)
{
  // Reset variables related to binding.
  fpFreeBoundCols();

  // Build array of CfpDBColumn entries to bind.

  if (!strDataField || !strDataField[0])
  {
    // Remove all bindings
    ; // do nothing since array was reset earlier.
  }
  // else if strDataField == "*", then bind all columns
  else if (strDataField[0] == '*' && strDataField[1] == (TCHAR)0)
  {
    ULONG i;
    ULONG cCols = m_cColumns;
    ULONG max = cCols - (ULONG)m_fHasBookmarks;

    // Allocate to calculated size.
    m_BoundCols.SetSize((UINT)max, FPDB_TRIM_EXTRA);
    if ((int)m_BoundCols == 0)
      return FALSE;

    // Set the size to 0 but don't trim. This will set the size to 0
    // but it will leave the allocated size to "max.
    m_BoundCols.SetSize(0);

    // Note: "m_fHasBookmarks" is 1 if there are bookmarks, else 0.
    for (i = (ULONG)m_fHasBookmarks; i < cCols; ++i)
      m_BoundCols.Add(i);
  }
  else  // else bind to column name
  {
    int i = fpGetColumnRec((LPCTSTR)strDataField);
    if (i > -1)
    {
      m_BoundCols.SetSize(1, FPDB_TRIM_EXTRA);  // set allocated size to 1
      if ((int)m_BoundCols == 0)
        return FALSE;
      m_BoundCols.SetSize(0);                   // set list size back to 0
      m_BoundCols.Add(i);                       // add item to list
    }
    else
    {
      ; // else do nothing (array was reset earlier).
    }
  }

  return fpSetBoundColumn();
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpSetBoundColumn()
 *
 * DESCRIPTION:
 *
 *   Binds as specified by m_arrayBoundCols.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpSetBoundColumn(void) 
{
	DBBINDING *colBindList;
    UINT      cColListCt = 0;
	DBBINDING *colBindBookmark;

    BLOB      *lpBlob = NULL;  // Additional memory needed for bindings.
    ULONG obInline  = 0;
    ULONG cBoundCols;

	HRESULT hr = S_OK;
    ULONG index;
    ULONG i;
    ULONG cTotalBoundCols = 0; // Number of bound columns including bookmark
    DBSTATUS *dbStatus = NULL;


    // If free failed OR binding to 0 columns, return.
    if (hr != S_OK)
      return hr;

    // Total number of bound columns (including any bookmark columns).
    cBoundCols = (ULONG)(UINT)m_BoundCols;
    cTotalBoundCols = cBoundCols + (m_fHasBookmarks ? 1 : 0);

    if (cTotalBoundCols == 0)  // if no columns at all, just return.
      return S_OK;

    // Allocate memory for list of columns + 1 for bookmarks (if
    // available).
    colBindList = new DBBINDING[cTotalBoundCols];
    RETURN_ON_NULLALLOC(colBindList);
    ZeroMemory((void*)colBindList, sizeof(DBBINDING)*cTotalBoundCols);
    if (m_fHasBookmarks)
      colBindBookmark = &colBindList[cTotalBoundCols-1];

    // Fill array elements with each non-bookmark column.
    for (i = 0; i < cBoundCols; ++i)
    {
      // NOTE: "m_aBoundCols[i].colIndex" is the index of m_pColumns 
      // for this column.
      index = m_BoundCols[i].colIndex;

      // Determine if this column goes in the "normal" list (columns
      // which don't use stream if no multi-stream support).
      if (m_fSuppMultiStreams || !m_pColumns[index].fpIsStreamCol())
      {
        // Fill array element with the binding info for this column
        m_pColumns[index].fpFillColBindRec(
          &colBindList[cColListCt], &obInline, &lpBlob);
        ++cColListCt;

        // Save ordinal value in bound column array.
        m_BoundCols[i].ordinalId = m_pColumns[index].iOrdinal;
      }
    }

    // Fill last element with data for bookmark column.
    if (m_fHasBookmarks)
    {
      m_pColumns[0].fpFillColBindRec( colBindBookmark,
        &obInline, &lpBlob, FPDB_BOOKMARK_COL);
    }

    // set minimum buffer sizes
    if (obInline == 0)
       obInline = 4;               

    // Allocate/reallocate buffer for data
    hr = fpAllocFetchBuffer(obInline);
    if (hr == S_OK)
    {
      // Create an accessor with this column binding.
      // DBSTATUS is an optional argument. If this
      // fails you can check the status in the 
      // debugger
      dbStatus = (DBSTATUS *)malloc(sizeof(DBSTATUS) * max(cColListCt, 1));
      _ASSERT(dbStatus);
      if (dbStatus == NULL)
        hr = E_OUTOFMEMORY;
    }

    if (hr == S_OK)
    {
      // Create the "normal" accessor (for non-stream columns)
      if (cColListCt > 0)
        hr = CRowset::CreateAccessor(colBindList, cColListCt, 
          m_hRowAccessor, dbStatus);

      // if table has bookmarks, bind to that column seperately.
      if (hr == S_OK && m_fHasBookmarks)
        hr = CRowset::CreateAccessor(colBindBookmark, 1, 
          m_hBmkAccessor, dbStatus);

      // If any elements needed streams, then create accessors for
      // each column using streams.
      if (hr == S_OK && cBoundCols != cColListCt)
        hr = fpCreateStreamAccessors(dbStatus, &lpBlob, obInline);

      free(dbStatus);
    } 

    if (colBindList)
       delete [] colBindList;

    if (lpBlob)
      free(lpBlob);

	return hr;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpCreateStreamAccessors()
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
HRESULT CfpOleBinding::fpCreateStreamAccessors(
  DBSTATUS *dbStatus, LPBLOB *plpBlob, ULONG obInline)
{
  // Col index (decremented to 0), skip bookmark col (last col in list)
  UINT i;                         // list index
  int  index;                     // index in m_pColumns
  DBBINDING tmpBindings;
  HACCESSOR hAccessor = 0;
  CfpDBColumn *pDBCol;
  HRESULT hr = S_OK;
  UINT cBoundCols = (UINT)m_BoundCols;

  // For each column...
  for (i = 0; i < (int)cBoundCols; ++i)
  {
    // NOTE: "m_aBoundCols[i].colIndex" is the index of m_pColumns 
    // for this column.
    index = m_BoundCols[i].colIndex;
    pDBCol = &m_pColumns[index];

    // If this column uses streams...
    if (pDBCol->fpIsStreamCol())
    {
      ZeroMemory(&tmpBindings, sizeof(tmpBindings));

      // Fill array element with the binding info for this column
      pDBCol->fpFillColBindRec(&tmpBindings, &obInline, plpBlob);

      // Allocate/reallocate buffer for data
      hr = fpAllocFetchBuffer(obInline);
      if (hr != S_OK)
        return hr;

      // Create the accessor stream column
      hr = CRowset::CreateAccessor(&tmpBindings, 1, hAccessor, dbStatus);
      _ASSERT(hr == S_OK);
      if (hr != S_OK)
        return hr;

      // if successful, let the column "maintain" the accessor.
      pDBCol->m_hReadAccessor = hAccessor;

      // Save ordinal value in bound column array.
      m_BoundCols[i].ordinalId = pDBCol->iOrdinal;
    } // if IUNKNOWN

  } // for

  return hr;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFreeRowAccessor()
 *
 * DESCRIPTION:
 *
 *   Releases the row accessors (both for reading and writing).
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpFreeRowAccessor(void)
{
  HRESULT hr = S_OK;

  // Release all accessors used for reading data from the rowset.
  m_BoundCols.ReleaseAllAccessors(this);

  if (m_hRowAccessor)
  {
    hr = CRowset::ReleaseAccessor(m_hRowAccessor);
    m_hRowAccessor = NULL;
  }

  if (m_hBmkAccessor)
  {
    hr = CRowset::ReleaseAccessor(m_hBmkAccessor);
    m_hBmkAccessor = NULL;
  }

  // Release the row accessors used to write data to the rowset.
  if (m_pSetDataCols)
  {
    m_pSetDataCols->ReleaseAllAccessors(this);
    delete m_pSetDataCols;
    m_pSetDataCols = NULL;
  }

  // Release the previous row accessors used to write data to the rowset.
  if (m_pPrevSetDataCols)
  {
    m_pPrevSetDataCols->ReleaseAllAccessors(this);
    delete m_pPrevSetDataCols;
    m_pPrevSetDataCols = NULL;
  }

  // Release all of the stream accessors
  // RFW - 12/20/02 - 11355

  UINT i;                         // list index
  int  index;                     // index in m_pColumns
  CfpDBColumn *pDBCol;
  UINT cBoundCols = (UINT)m_BoundCols;

  // For each column...
  for (i = 0; i < (int)cBoundCols; ++i)
  {
    // NOTE: "m_aBoundCols[i].colIndex" is the index of m_pColumns 
    // for this column.
    index = m_BoundCols[i].colIndex;
    pDBCol = &m_pColumns[index];

    if (pDBCol->m_hReadAccessor)
    {
      hr = CRowset::ReleaseAccessor(pDBCol->m_hReadAccessor);
      pDBCol->m_hReadAccessor = NULL;
    }

  } // for

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpHasFetchBuffer()
 *
 * DESCRIPTION:
 *
 *   Returns TRUE if a buffer was allocated for fetching data.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::fpHasFetchBuffer(void)
{
#ifndef FP_USE_GLOBAL
  return (m_lpszFetchBuffer != NULL);
#else
  return (m_hBoundData != 0);
#endif
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpAllocFetchBuffer()
 *
 * DESCRIPTION:
 *
 *   Allocates/reallocates the buffer used for fetching data.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpAllocFetchBuffer(ULONG cbSize)
{
  HRESULT hr = S_OK;

  _ASSERT(m_ulLockCt == 0);

#ifndef FP_USE_GLOBAL
  // Allocate/reallocate buffer for data
  if (!m_lpszFetchBuffer || (cbSize != m_ulBoundInlineLen))
  {
    if (m_lpszFetchBuffer)
      m_lpszFetchBuffer = (LPSTR)CoTaskMemRealloc(m_lpszFetchBuffer, cbSize);
    else
      m_lpszFetchBuffer = (LPSTR)CoTaskMemAlloc(cbSize);

    _ASSERT(m_lpszFetchBuffer);       // Did memory allocate?
    if (!m_lpszFetchBuffer)
      hr = E_OUTOFMEMORY;
    else
      m_ulBoundInlineLen = cbSize;
  }
#else  
  // Allocate/reallocate buffer for data
  if (!m_hBoundData || (cbSize != m_ulBoundInlineLen))
  {
    if (m_hBoundData)
      m_hBoundData = (DWORD)GlobalReAlloc((HGLOBAL)m_hBoundData, cbSize, GMEM_MOVEABLE);
    else
      m_hBoundData = (DWORD)(LPVOID)GlobalAlloc(GHND, cbSize);

    _ASSERT(m_hBoundData);               // Did memory allocate?
    if (!m_hBoundData)
      hr = E_OUTOFMEMORY;
    else
      m_ulBoundInlineLen = cbSize;
  }
#endif

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpFreeFetchBuffer()
 *
 * DESCRIPTION:
 *
 *   Frees the buffer used for fetching data.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpFreeFetchBuffer(void)
{
  // Free allocate buffer
  _ASSERT(m_ulLockCt == 0);

#ifndef FP_USE_GLOBAL
  if (m_lpszFetchBuffer)
  {
    CoTaskMemFree( m_lpszFetchBuffer);
    m_lpszFetchBuffer = NULL;
    m_ulLockCt = 0;
  }
#else
  if (m_hBoundData && m_ulLockCt == 0)
  {
    GlobalFree((HGLOBAL)m_hBoundData);
    m_hBoundData = NULL;
  }
#endif
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpLockFetchBuffer()
 *
 * DESCRIPTION:
 *
 *   "Locks" the buffer used for fetching data.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
LPSTR CfpOleBinding::fpLockFetchBuffer(void)
{
#ifndef FP_USE_GLOBAL
#ifdef _DEBUG
  _ASSERT(m_lpszFetchBuffer);
  if (m_lpszFetchBuffer)
    ++m_ulLockCt;
#endif
  return m_lpszFetchBuffer;
#else
  if (!m_ulLockCt)
    if (m_hBoundData)
      m_lpszFetchBuffer = (LPSTR)GlobalLock((HGLOBAL)m_hBoundData);

  if (m_lpszFetchBuffer)
    ++m_ulLockCt;

  return m_lpszFetchBuffer;
#endif
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpUnlockFetchBuffer()
 *
 * DESCRIPTION:
 *
 *   "Unlocks" the buffer used for fetching data.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::fpUnlockFetchBuffer(void)
{
#ifndef FP_USE_GLOBAL
#ifdef _DEBUG
  _ASSERT(m_ulLockCt);
  _ASSERT(m_lpszFetchBuffer);
  if (m_ulLockCt)
    if (m_lpszFetchBuffer)
      --m_ulLockCt;
#endif
  return; 
#else
  if (m_ulLockCt)
  {
    --m_ulLockCt;
    if (!m_ulLockCt)
    {
      if (m_hBoundData)
        GlobalUnlock((HGLOBAL)m_hBoundData);
      m_lpszFetchBuffer = NULL;
    }
  }
#endif
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpRequeryCurrHRow()
 *
 * DESCRIPTION:
 *
 *   Requeries the ADO control for the current row.  Stores the current
 *   row's HROW in the member variable, "m_hCurrRow".
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpRequeryCurrHRow(int fReleaseFlags /* = FPDB_REQUERY_ALL */ )
{
  HRESULT hr = S_OK;
  HCHAPTER hTmpChapter = DB_NULL_HCHAPTER;

  // Release the HROW of the current row.
  if (m_hCurrRow)
  {
    CRowset::ReleaseRows(&m_hCurrRow, 1);
    m_hCurrRow = 0;
  }

  // If we are to re-query the current row...
  if (!(fReleaseFlags & FPDB_RELEASE_ONLY))
  {
    DBPOSITIONFLAGS dwPosFlags;

    // IDataSource::getDataMember succeeds, but if it did not
    // get an interface for IRowPosition then return S_FALSE.
    if (!CRowset::GetRowPosition())
      return S_FALSE;

    // Save the current row's handle for later use.
    hr = m_pRowsetPos->GetRowPosition(&hTmpChapter, &m_hCurrRow, &dwPosFlags);
  }

  // If NOT preserving m_hChapter, either release m_hChapter or
  // replace it with the temporary chapter.
  if (!(fReleaseFlags & FPDB_PRESERVE_CHAPTER))
  {
    // If only releasing the chapter, then release it.
    if (fReleaseFlags & FPDB_RELEASE_ONLY)
    {
      if (m_hChapter != DB_NULL_HCHAPTER)
        fpReleaseChapter(&m_hChapter);
    }
    // Else requerying chapter, release previous chapter and assign new one.
    else
    {
      if (m_hChapter != DB_NULL_HCHAPTER)
        fpReleaseChapter(&m_hChapter);
      m_hChapter = hTmpChapter;   // save new chapter
      hTmpChapter = DB_NULL_HCHAPTER; // prevent new chapter from being released.
    }
  }

  // Release tmp chapter if it is not null
  if (hTmpChapter != DB_NULL_HCHAPTER)
    fpReleaseChapter(&hTmpChapter);

  return hr;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpReleaseChapter()
 *
 * DESCRIPTION:
 *
 *   Releases the given chapter.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpReleaseChapter(HCHAPTER *phChapter)
{
  HRESULT hr = S_OK;

  // If chapter is valid and so is Rowset ptr...
  if (*phChapter != DB_NULL_HCHAPTER && m_pRowset)
  {
    IChapteredRowset *pChapRowset = NULL;

    // Get IChapteredRowset interface, release the chapter and release
    // interface.
    hr = m_pRowset->QueryInterface(IID_IChapteredRowset, (void**)&pChapRowset);
    if (hr == S_OK && pChapRowset)
    {
      pChapRowset->ReleaseChapter(*phChapter, NULL);
      pChapRowset->Release();
      *phChapter = DB_NULL_HCHAPTER; 
    }
  }

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::ForwardMsg()
 *
 * DESCRIPTION:
 *
 *   Processes a message received on behalf of the Data Binding.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpOleBinding::ForwardMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  FP_DBEVENT *pEvent;
  UINT i;

  switch (uMsg)
  {
    case VBM_DATA_POSTMSGTOSELF:  // received a posted update msg
      // Process the DB Notify events in the queue. Limit ourself to
      // 400 messages at a time to prevent hogging the message processing.
      i = m_EventQueue.GetEventCount();
      if (i > 400)            // Limit processing to maximum of 400 events.
        i = 400;
      while (i > 0 && (pEvent = m_EventQueue.GetNextEvent()))
      {
        // Process the event.
        fpProcessEvent(pEvent->ePhase, pEvent->eReason, pEvent->cRows,
          pEvent->rghRows, pEvent->rgCols);
        m_EventQueue.FreeEvent(pEvent, this); // Free the event.
        --i;                  // Decrement our loop counter.
      }

      // If there are more events to process, re-post a msg.
      if (m_EventQueue.PeekNextEvent())
        m_EventQueue.m_fMsgPosted = fpDBPostMsgToSelf(0, (LPARAM)this);
      else // else, clear flag
        m_EventQueue.m_fMsgPosted = FALSE;

      break; // VBM_DATA_POSTMSGTOSELF
  }
  
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpDBPostMsgToSelf()
 *
 * DESCRIPTION:
 *
 *   Tell owner to post a message on our behalf.  
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::fpDBPostMsgToSelf(WPARAM wParam, LPARAM lParam)
{
  BOOL fReturn = FALSE;

  // To post a msg, call the owner's function to process the
  // VBM_DATA_POSTMSGTOSELF message. The caller will post the msg for us.
  _ASSERT(m_lpfuncDataMsg); //Callback func MUST BE SET BY OWNER!
  if (m_lpfuncDataMsg)
    fReturn = 
      (BOOL)m_lpfuncDataMsg((LPVOID)m_lpOwner, VBM_DATA_POSTMSGTOSELF,
        wParam, (LPARAM)this);

  return fReturn;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpProcessEvent()
 *
 * DESCRIPTION:
 *
 *   Processes the notification "messages" received from the rowset.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpProcessEvent(DBEVENTPHASE ePhase,
  DBREASON eReason, ULONG cRows, const HROW *rghRows, const ULONG *rgCols)
{
  HRESULT hr;

  switch(ePhase)
  {
    case DBEVENTPHASE_OKTODO:
      hr = fpOKToDo(eReason);
      break;
    case DBEVENTPHASE_ABOUTTODO:
      hr = fpAboutToDo(eReason, cRows, rghRows, rgCols);
      break;
    case DBEVENTPHASE_SYNCHAFTER:
      hr = fpSynchAfter(eReason);
      break;
    case DBEVENTPHASE_FAILEDTODO:
      hr = fpFailedToDo(eReason);
      break;
    case DBEVENTPHASE_DIDEVENT:
      hr = fpDidEvent(eReason, cRows, rghRows, rgCols);
      break;
    default:
      _ASSERT(0);  // Unknown phase!!!
      hr = S_OK;
      break;
  }

  return hr;  
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpQueueEvent()
 *
 * DESCRIPTION:
 *
 *   Processes the notification "messages" received from the rowset by
 *   adding it to the DB Events queue.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpQueueEvent(DBEVENTPHASE ePhase,
  DBREASON eReason, ULONG cRows, const HROW *rghRows, const ULONG *rgCols)
{
  HRESULT hr;
  int     fQueueEvent = FPDB_EVENT_QUEUE_MSG;

  switch(ePhase)
  {
    case DBEVENTPHASE_OKTODO:
      hr = fpFilter_OKToDo(eReason, &fQueueEvent);
      break;
    case DBEVENTPHASE_ABOUTTODO:
      hr = fpFilter_AboutToDo(eReason, &fQueueEvent);
      break;
    case DBEVENTPHASE_SYNCHAFTER:
      hr = fpFilter_SynchAfter(eReason, &fQueueEvent);
      break;
    case DBEVENTPHASE_FAILEDTODO:
      hr = fpFilter_FailedToDo(eReason, &fQueueEvent);
      break;
    case DBEVENTPHASE_DIDEVENT:
      hr = fpFilter_DidEvent(eReason, &fQueueEvent);
      break;
    default:
      _ASSERT(0);  // Unknown phase!!!
      fQueueEvent = FALSE;    // don't queue this unknown message.
      hr = S_OK;
      break;
  }

  // If Filter func has instructed us to process the message now, do it.
  if (fQueueEvent == FPDB_EVENT_PROCESS_NOW)
  {
    hr = fpProcessEvent(ePhase, eReason, cRows, rghRows, rgCols);
  }
  // If msg is still to be processed, add it to the queue.
  else if (fQueueEvent == FPDB_EVENT_QUEUE_MSG)
  {
    // Try to add event to queue.
    HRESULT hr2 = m_EventQueue.AddEvent(ePhase, eReason, cRows, rghRows, rgCols, this);
    
    // If memory error, return that code, else Post the message.
    if (hr2 == E_OUTOFMEMORY)
      return E_OUTOFMEMORY;
    // Post message if no messages are pending.
    else if (hr2 == S_OK && !m_EventQueue.m_fMsgPosted) 
      m_EventQueue.m_fMsgPosted = fpDBPostMsgToSelf(0, (LPARAM)this);
  }

  return hr;  
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::OnRowPositionChange()
 *
 * DESCRIPTION:
 *
 *   This function is called (by the rowset) to notify that the current
 *   row position has changed.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP CfpOleBinding::OnRowPositionChange(DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny)
{
  return fpQueueEvent(ePhase, eReason, 0, FPDB_NOHROWS, FPDB_NOCOLIDS);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::OnFieldChange()
 *
 * DESCRIPTION:
 *
 *   This function is called (by the rowset) to notify that a field
 *   (column) value has changed.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP CfpOleBinding::OnFieldChange(IRowset *prowset, HROW hRow, 
  ULONG cColumns, ULONG rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase, 
  BOOL fCantDeny)
{
#define FPDB_DEFCOLCT 20
  HRESULT hr;
  ULONG rgDefColArray[FPDB_DEFCOLCT+1]; // If 20 or less, use this array
  ULONG *pulTmpCols = NULL;   // If more than 20, then allocate a buffer.

  // If any columns, then put them in an array.
  if (cColumns)
  {
    if (cColumns <= FPDB_DEFCOLCT)
      pulTmpCols = rgDefColArray;
    else
    {
      pulTmpCols = (ULONG *)malloc((cColumns  + 1) * sizeof(ULONG));
      if (!pulTmpCols)
        return E_OUTOFMEMORY;
    }
    // copy columns to array.
    pulTmpCols[0] = cColumns;
    memcpy(&pulTmpCols[1], rgColumns, cColumns * sizeof(ULONG));
  }

  // Process (Queue) the event.
  hr = fpQueueEvent(ePhase, eReason, 1, &hRow, pulTmpCols);

  // If buffer was allocated, then free it.
  if (pulTmpCols && (pulTmpCols != rgDefColArray))
    free(pulTmpCols);

  return hr;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::OnRowChange()
 *
 * DESCRIPTION:
 *
 *   This function is called (by the rowset) to notify that a row
 *   has changed.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP CfpOleBinding::OnRowChange(IRowset *prowset, ULONG cRows, const HROW rghRows[], DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny)
{
  return fpQueueEvent(ePhase, eReason, cRows, rghRows, FPDB_NOCOLIDS);
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::OnRowsetChange()
 *
 * DESCRIPTION:
 *
 *   This function is called (by the rowset) to notify that there's 
 *   a change to the rowset.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP CfpOleBinding::OnRowsetChange(IRowset *prowset, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny)
{
  return fpQueueEvent(ePhase, eReason, 0, FPDB_NOHROWS, FPDB_NOCOLIDS);
}


/***************************************************************************
 *
 * FUNCTION:  fpCompareDataMembers()
 *
 * DESCRIPTION:
 *
 *   This function compares the two strings. It allows either string ptr
 *   to be null.  If a string ptr is NULL, then "" is substituted.
 *
 * PARAMETERS:
 *
 * RETURNS: TRUE if the strings match
 *
 **************************************************************************/
BOOL fpCompareDataMembers(DataMember bstrDM1, DataMember bstrDM2)
{
  BOOL bMatch = FALSE;

  // If both variables are NULL or "", then they match.
  if ( (bstrDM1 == NULL || bstrDM1[0] == '\0') && 
	   (bstrDM2 == NULL || bstrDM2[0] == '\0') )
    bMatch = TRUE;

  // else, if neither string is null, then compare the strings.
  else if (bstrDM1 != NULL && bstrDM2 != NULL)
    bMatch = (bstrcmpi(bstrDM1, bstrDM2) == 0);

  return bMatch;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::dataMemberChanged()
 *
 * DESCRIPTION:
 *
 *   This function is called (by the rowset) to notify that a DataMember
 *   of the DataSource has changed.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP CfpOleBinding::dataMemberChanged(DataMember bstrDM)
{
  // If the same, then reset the attached rowset because it may 
  // now be invalid or may have changed.
  if (fpCompareDataMembers(m_dmDataMember, bstrDM))
    fpReInitDataBinding();

  return S_OK;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::dataMemberAdded()
 *
 * DESCRIPTION:
 *
 *   This function is called (by the rowset) to notify that a DataMember
 *   of the DataSource has been added. We don't care about this.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP CfpOleBinding::dataMemberAdded(DataMember bstrDM)
{
	OutputDebugString(_T("CfpOleBinding::dataMemberAdded\n"));
	return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::dataMemberRemoved()
 *
 * DESCRIPTION:
 *
 *   This function is called (by the rowset) to notify that a DataMember
 *   of the DataSource has been removed.  We only care if the data member
 *   is the one we are currently using.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
STDMETHODIMP CfpOleBinding::dataMemberRemoved(DataMember bstrDM)
{
  // If the same, then reset the attached rowset because it may 
  // now be invalid or may have changed.
  if (fpCompareDataMembers(m_dmDataMember, bstrDM))
    fpReleaseDataBinding();

  return S_OK;
}


HRESULT CfpOleBinding::InternalQueryInterface(REFIID riid, void **ppvObjOut)
{
	if (DO_GUIDS_MATCH(riid, IID_IRowsetNotify))
		*ppvObjOut = (void*)(IUnknown*)(IRowsetNotify*)this;
	else if (DO_GUIDS_MATCH(riid, IID_IRowPositionChange))
		*ppvObjOut = (void*)(IUnknown*)(IRowPositionChange*)this;
	else
		return CUnknownObject::InternalQueryInterface(riid, ppvObjOut);

	// Interface supported, so addref and return
	//
	((IUnknown *)(*ppvObjOut))->AddRef();
	return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetMultiStreamProp()
 *
 * DESCRIPTION:
 *
 *   This function gets the DB property for the flag, 
 *   MULTIPLE_STORAGE_OBJECTS.  The rowset is queried for this setting.
 *
 * PARAMETERS:
 *
 * RETURNS:   HRESULT
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpGetMultiStreamProp(BOOL *pfMultiStream)
{
  static const DBPROPID propid =  DBPROP_MULTIPLESTORAGEOBJECTS;

  // Check to see if we need to initialize
  // the properties structure
  HRESULT hr = InitRowsetInfo();

  *pfMultiStream = FALSE;

  if (SUCCEEDED(hr))
  {
    // Allocate VARIANT values array
    //
    VARIANT value;

    VariantInit(&value);
    hr = GetRowsetProperties(NULL, &propid, &value, 1);

    // Save property values in structure
    if (SUCCEEDED(hr))
      *pfMultiStream = value.boolVal;
  }

  return hr;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::fpGetBatchUpdateProp()
 *
 * DESCRIPTION:
 *
 *   This function gets the value of the LockType property on the
 *   ADO DataSource.  If the LockType property is "adLockBatchOptimistic"
 *   then TRUE is returned, else FALSE is returned.
 *
 * PARAMETERS:
 *
 * RETURNS:   HRESULT
 *
 **************************************************************************/
HRESULT CfpOleBinding::fpGetBatchUpdateProp(BOOL *pfBatchUpdate)
{
  HRESULT        hr = S_OK;
  IUnknown      *pUnk = NULL;
  _ADORecordset *pAdoRec = NULL;
  LockTypeEnum   lockType;

  *pfBatchUpdate = FALSE;  // Assume false, prove otherwise.

  // See if we can get the interface ptr for the ADORecordset.
  //
  // NOTE: pUnk is not the ADORecordSet, even though, that is what is
  //       returned for "IID_IADORecordset".  To get the ADORecordset
  //       interface ptr, we must query "IID_IADORecordset" off pUnk.
  if (m_pDataSource)
  {
    hr = m_pDataSource->getDataMember(m_bstrDM, IID_IADORecordset, &pUnk);
    if (hr == S_OK && pUnk)
    {
      // Get ADORecordset interface ptr from pUnk.
      hr = pUnk->QueryInterface(IID_IADORecordset, (LPVOID*)&pAdoRec);
      
      // Release pUnk, it is no longer needed.
      pUnk->Release();

      // If we have a ADORecordset ptr, get the LockType property.
      if (hr == S_OK && pAdoRec)
      {
        hr = pAdoRec->get_LockType(&lockType);

        // If BatchOptimistic, set the return flag TRUE.
        if (hr == S_OK && lockType == adLockBatchOptimistic)
          *pfBatchUpdate = TRUE;

        // Release the ADORecordset ptr
        pAdoRec->Release();
      }
    }
  }

  return hr;
}


/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetDataSourceInterface()
 *
 * DESCRIPTION:
 *
 *   Used by the next two functions to get an interface pointer for
 *   the DataSource.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpOleBinding::GetDataSourceInterface(DataSource **ppRetDataSource,
  BOOL *pRetfRelease)
{
  BOOL        fRelease = FALSE;
  DataSource *pDataSource;
  HRESULT     hr;

  // Get ptr to DataSource interface.  First try member variable.
  pDataSource = m_pDataSource;
  if (!pDataSource)
  {
    // Now try from member variable, m_pDataSrcProp.
    if (!m_pDataSrcProp)
      return FALSE;            // No DataSource, just return.
    
    // Else, query interface from m_pDataSrcProp.
	hr = m_pDataSrcProp->QueryInterface(IID_DataSource, (void**)&pDataSource);
    if (hr != S_OK || !pDataSource)
      return FALSE;
      
    fRelease = TRUE;       // later, release pDataSource.
  }

  // Set return parameters and return success.
  *ppRetDataSource = pDataSource;
  *pRetfRelease = fRelease;
  return TRUE;
}



/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetListOfDataMemberNames()
 *
 * DESCRIPTION:
 *
 *   Typically, this function is called by the control's function,
 *   OnGetPredefinedStrings(), when VB's Browser asks for a list of
 *   DataMembers.
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpOleBinding::GetListOfDataMemberNames(CALPOLESTR *prgOleStr,
      CADWORD *prgCookie)
{
  DataSource *pDataSource;
  BOOL       fRelDataSrc = FALSE;
  HRESULT    hr = S_OK;
  LONG       lCt;
  LONG       i;  
  BSTR       bstr;

  // Containers will call with NULL arguments
  // to determine if the control supports browsing
  // for this property. Returning S_OK for this 
  // case signifies YES
  if (!prgOleStr || !prgCookie)
    return S_OK;

  // Start off with no entries
  prgOleStr->cElems = prgCookie->cElems = 0;
  prgOleStr->pElems = NULL;
  prgCookie->pElems = NULL;

  // Get ptr to DataSource interface.  First try member variable.
  if (!GetDataSourceInterface(&pDataSource, &fRelDataSrc))
    return S_OK;              // no interface, return.

  // Get the number of DataMembers.
  hr = pDataSource->getDataMemberCount(&lCt);

  // Nothing to do if there are no data members.
  if (hr != S_OK || lCt == 0)
  {
    if (fRelDataSrc)
      pDataSource->Release();
    return S_OK;
  }

  // Allocate return buffers
  prgOleStr->pElems = (LPOLESTR*)CoTaskMemAlloc(sizeof(LPOLESTR) * lCt);
  if (NULL == prgOleStr->pElems) 
    goto DataMemberOutOfMemory;

  prgCookie->pElems = (DWORD*)CoTaskMemAlloc(sizeof(DWORD) * lCt);
  if (NULL == prgCookie->pElems)
    goto DataMemberOutOfMemory;

  for (i = 0; i < lCt; i++)
  {
    // Get DataMember name
//  must use CoTaskMemAlloc to allocate the strings, not SysAllocString -scl
    if (SUCCEEDED(hr = pDataSource->getDataMemberName(i, &bstr)) && bstr)
	  {
      // Put name in array (or null if failed)
      prgOleStr->pElems[i] = wstralloc(bstr);
      SysFreeString(bstr);
    }
    else
      prgOleStr->pElems[i] = NULL;

    // Cookie is just the index.
    prgCookie->pElems[i] = i;
  }

  // Set counts
  prgCookie->cElems = prgOleStr->cElems = lCt;

  // if necessary, release ptr.
  if (fRelDataSrc)
    pDataSource->Release();

  // return OK.
  return S_OK;


DataMemberOutOfMemory:   // if memory failure
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
  if (fRelDataSrc)
    pDataSource->Release();

  return E_OUTOFMEMORY;
}

/***************************************************************************
 *
 * FUNCTION:  CfpOleBinding::GetDataMemberNameForCookie()
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
BOOL CfpOleBinding::GetDataMemberNameForCookie( DWORD dwCookie, 
  BSTR FAR *lpRetBSTR)
{
  DataSource *pDataSource;
  BOOL       fRelDataSrc = FALSE;
  HRESULT    hr = S_OK;
  LONG       lCt;

  *lpRetBSTR = NULL;          // init return parameter.

  // Get ptr to DataSource interface.  First try member variable.
  if (!GetDataSourceInterface(&pDataSource, &fRelDataSrc))
    return S_OK;              // no interface, return.

  // Get the number of DataMembers.
  hr = pDataSource->getDataMemberCount(&lCt);

  // Nothing to do if there are no DataMembers.
  if (hr != S_OK || lCt == 0)
  {
    if (fRelDataSrc)
      pDataSource->Release();
    return FALSE;
  }

  // Get DataMember name at index (cookie).
  if ((long)dwCookie < lCt)
    hr = pDataSource->getDataMemberName((long)dwCookie, lpRetBSTR);

  // Return TRUE if we have a BSTR ptr.
  return (*lpRetBSTR != NULL);
}
