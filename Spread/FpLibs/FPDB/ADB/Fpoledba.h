

/*************************************************************************\
* FPOLEDBA.H  - FarPoint Data Binding include file for ADO Binding.       *
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

//
// fpoledba.h : Declaration of the CfpOleBinding class.
//
//             FarPoint's implementation of DB Binding for OLE controls.
//
//             This class is intented to be a member variable (public) of
//             an OCX control (subclassed from COleControl).
//

#ifndef FPOLEDBA_H
#define FPOLEDBA_H

#define NO_ADBCTL_PROPHELP
#ifndef VC8
#include <adbctl.h>
#endif
#include <msdatsrc.h>	// IDataSource and IDataSourceListener
#include <oledberr.h>

#if (!defined(__OCDB_H_) && (!defined(FP_ADB)))
//#include <initguid.h>
#include <ocdbid.h>
#include <olebind.h>
#include <ocdb.h>
#endif

#include <unknown.h>

#include "fpdblist.h"
#include "fpdbcola.h"
#include "fpdbutil.h"
//#include "fpdbnote.h"
#include "fpdbsupp.h"

// SCP - If this the version of OLEDBVER is less than 250, then
// the following defines don't get defined, so we will define them
// ourself.
#if ((OLEDBVER < 0x0250) && !defined(__IRowsetNextRowset_INTERFACE_DEFINED__))
enum DBREASONENUM25
    {	DBREASON_ROWSET_ROWSADDED	= DBREASON_ROW_ASYNCHINSERT + 1,
	DBREASON_ROWSET_POPULATIONCOMPLETE	= DBREASON_ROWSET_ROWSADDED + 1,
	DBREASON_ROWSET_POPULATIONSTOPPED	= DBREASON_ROWSET_POPULATIONCOMPLETE + 1
    };
#endif // OLEDBVER < 0x0250


#define  FPDB_BOOKMARK_COLINDEX  -2

#define  FPDB_NOCOLIDS           NULL
#define  FPDB_NOHROWS            NULL

#define FPADB_IMPL_GETDATASOURCE(CtlCls, FpOleBindObject) \
STDMETHODIMP CtlCls::get_DataSource(DataSource **ppDataSource) \
  { return FpOleBindObject.GetDataSource(ppDataSource); }

#define FPADB_IMPL_SETDATASOURCE(CtlCls, FpOleBindObject) \
STDMETHODIMP CtlCls::putref_DataSource(DataSource *pDataSource) \
  { return FpOleBindObject.SetDataSource(pDataSource); }

#define FPADB_IMPL_GETDATAMEMBER(CtlCls, FpOleBindObject) \
STDMETHODIMP CtlCls::get_DataMember(DataMember *pdmDataMember) \
  { return FpOleBindObject.GetDataMember(pdmDataMember); }

#define FPADB_IMPL_SETDATAMEMBER(CtlCls, FpOleBindObject) \
STDMETHODIMP CtlCls::put_DataMember(DataMember dmDataMember) \
  { return FpOleBindObject.SetDataMember(dmDataMember); }

// added to support data binding through different property names -scl
#define FPADB_IMPL_GETDATASOURCE_EX(CtlCls, FpOleBindObject, PropName) \
STDMETHODIMP CtlCls::get_##PropName(DataSource **ppDataSource) \
  { return FpOleBindObject.GetDataSource(ppDataSource); }

#define FPADB_IMPL_SETDATASOURCE_EX(CtlCls, FpOleBindObject, PropName) \
STDMETHODIMP CtlCls::putref_##PropName(DataSource *pDataSource) \
  { return FpOleBindObject.SetDataSource(pDataSource); }

#define FPADB_IMPL_GETDATAMEMBER_EX(CtlCls, FpOleBindObject, PropName) \
STDMETHODIMP CtlCls::get_##PropName(DataMember *pdmDataMember) \
  { return FpOleBindObject.GetDataMember(pdmDataMember); }

#define FPADB_IMPL_SETDATAMEMBER_EX(CtlCls, FpOleBindObject, PropName) \
STDMETHODIMP CtlCls::put_##PropName(DataMember dmDataMember) \
  { return FpOleBindObject.SetDataMember(dmDataMember); }

#define FPADB_IMPL_GETDATAFIELD(CtlCls, FpOleBindObject) \
STDMETHODIMP CtlCls::get_DataField(BSTR *pbstrDataField) \
  { return FpOleBindObject.GetDataField(pbstrDataField); }

#define FPADB_IMPL_SETDATAFIELD(CtlCls, FpOleBindObject) \
STDMETHODIMP CtlCls::put_DataField(BSTR bstrDataField) \
  { return FpOleBindObject.SetDataField(bstrDataField); }

              
class CfpDBColumn;              // declared in "fpdbcola.h"
class CfpNotifyEvents;          // declared in "fpdbutil.h"



BOOL fpBookmarksEqual(LPVOID hlstrBmk1, LPVOID hlstrBmk2);
BOOL fpBookmarksEqual(LPVOID hlstrBmk, CBookmark *pCBmk);
BOOL fpBookmarksEqual(CBookmark *pCBmk, LPVOID hlstrBmk);
BOOL fpBookmarksEqual(CBookmark *pCBmk1, CBookmark *pCBmk2);

void fpCopyBookmark(CBookmark *pCBmkDest, LPVOID hlstrSource);

//-------------------------------------------------------------------------
// "Public" Class - CfpOleBinding
//-------------------------------------------------------------------------
class CfpOleBinding : 
  public CUnknownObject,
  public CRowset,
  public IRowPositionChange,   // "CNotifications"
  public IRowsetNotify, public IDataSourceListener    // "CNotifications"
{

//-------------------------------------------------------------------------
// PUBLIC MEMBER FUNCTIONS
//-------------------------------------------------------------------------
public:

    DECLARE_STANDARD_UNKNOWN();

// Constructor
	CfpOleBinding();
// Destructor
	~CfpOleBinding();

const ROWSETPROPERTIES& GetProperties(void);
HRESULT GetRowsetProperties(IUnknown *pRowset, const DBPROPID *pPropIDs, VARIANT *pValues, ULONG cPropIDs);

// Set-up of Environment
    void SetOwner( LPVOID lpOleControl);
    void SetDataSourceDispid( DISPID dispid);
    void SetDataMemberDispid( DISPID dispid);
    void SetBindWithEntryId( BOOL fUseEntryId);

    // NOTE: lpfuncDataMsg is ptr to functions of type:
    //       extern "C" void DataMsg( LPVOID lpObject, UINT uMsg,
    //                                WPARAM wParam, LPARAM lParam);
    //       When the function is invoked, "lpObject" is ptr to "Owner".
    void SetDataMsgFunc( LPVOID lpfuncDataMsg);

    // Use to notify FPDB of the status of the "FreezeEvents".
    void OnFreezeEvents(BOOL fFreeze);

    // Whether to include bookmark column when binding columns.
    BOOL SetBindToBookmark(BOOL fSetting);  // Default is FALSE.

// Use for OnGetPredefinedStrings()

    // NOTE: If using MFC, do not call this function, use the function,
    // fpMFCGetListOfColumnNames() (found in FPDBMFC.H), instead.
    HRESULT GetListOfColumnNames(CALPOLESTR *prgOleStr,
      CADWORD *prgCookie);

    LPCTSTR GetColumnNameAtIndex(UINT index, DWORD *pdwCookie);

// Use for OnGetPredefinedValue()
    BOOL GetColumnNameForCookie(DWORD dwCookie, BSTR FAR *lpRetBSTR);

    int fpGetColumnRec(LPCTSTR tstrColumnName);
    int fpGetColumnRec(int iFieldPos);
    int fpGetExtDataFldIndexForColId(ULONG ulOrdId);

    // Functions to get DataMember names.
    HRESULT GetListOfDataMemberNames(CALPOLESTR *prgOleStr,
      CADWORD *prgCookie);
    BOOL GetDataMemberNameForCookie(DWORD dwCookie, BSTR FAR *lpRetBSTR);


// Interface to Data functions (LPVOID -> LPDATAACCESS)
    short DataCall(LPVOID lpvda, USHORT msg, USHORT action, 
      LPVOID lpvUserData = NULL);

    void ForwardMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

    operator IUnknown *() {return PrivateUnknown();}

//-------------------------------------------------------------------------
// PUBLIC MEMBER VARIABLES
//-------------------------------------------------------------------------
public:
    BOOL  m_fIgnoreSetDataField;    // VB workaround: ignore next call
                                    // to SetDataField()
//-------------------------------------------------------------------------
// INTERFACES
//-------------------------------------------------------------------------
public:
// Following is inplace of XBoundObject interface
	HRESULT XBound_OnSourceChanged(DISPID dispid, BOOL fBound, BOOL FAR *lpfOwnXferOut);
	HRESULT XBound_IsDirty(DISPID dispid);

//-------------------------------------------------------------------------    
// PROTECTED MEMBER FUNCTIONS...
//-------------------------------------------------------------------------    

	// IRowPositionChange methods
	//
	STDMETHOD(OnRowPositionChange)(DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);

	// IRowsetNotify methods
	//
	STDMETHOD(OnFieldChange)(IRowset *prowset, HROW hRow, ULONG cColumns, ULONG rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);
	STDMETHOD(OnRowChange)(IRowset *prowset, ULONG cRows, const HROW rghRows[], DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);
	STDMETHOD(OnRowsetChange)(IRowset *prowset, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);

	// IDataSourceListener methods
	//
	STDMETHOD(dataMemberChanged)(THIS_ DataMember bstrDM);
	STDMETHOD(dataMemberAdded)(THIS_ DataMember bstrDM);
	STDMETHOD(dataMemberRemoved)(THIS_ DataMember bstrDM);


// Functions related to Error codes when Updating the record source.
    BOOL fpQueryError(void);
    void fpResetError(LPVOID lpvUserData = NULL);
    void fpSetError(LONG lErrorCode, LONG lRcFromCursorAPI, 
      LPVOID lpvQueryInterfacePtr);
    LONG fpInvokeError(void);
#define FPDB_ERR_BEGINUPDATE_ADD       5000
#define FPDB_ERR_BEGINUPDATE_UPDATE    5001   
#define FPDB_ERR_CANTSETAUTOINCR       5002   
#define FPDB_ERR_CONVERTSTRINGTOTYPE   5003   
#define FPDB_ERR_DELETE                5004   
#define FPDB_ERR_FETCHCURRENTROW       5005   
#define FPDB_ERR_GETNEXTROW            5006   
#define FPDB_ERR_LBLOCKALLOC           5007   
#define FPDB_ERR_MEMLOCK_DATABUFFER    5008   
#define FPDB_ERR_NOTIMPLEMENTED        5009   
#define FPDB_ERR_QINTF_UPDATECURSOR    5010   
#define FPDB_ERR_SETCOLUMN             5011  
#define FPDB_ERR_UPDATE                5012  
#define FPDB_ERR_UPDATEINPROGRESS      5013  
#define FPDB_ERR_MOVETO_NEWROW         5014

    BOOL fpQueryUserMode(void);

//-------------------------------------------------------------------------    
// PROTECTED MEMBER VARIABLES...
//-------------------------------------------------------------------------    
protected:

    LPVOID     m_lpOwner;            // Pointer to OCX class (owner)    

// Function ptr.  "lpObject" is a ptr to Owner object
    LRESULT (FAR *m_lpfuncDataMsg)( LPVOID lpObject,
                                    UINT uMsg, WPARAM wParam, LPARAM lParam);
    DISPID      m_dispid;
    ULONG       m_ulCtlFlags;       // Used to simulate VBSetControlFlags

    int  m_iNotifyStatus;
#define NOTIFY_ADVISE_ON          0x0001 // NotifyDBEvents notification is ON
#define NOTIFY_SYNCBEFORE_IGNORED 0x0002 // Notification is ON, but we ignore SyncBefore
#define NOTIFY_SYNCAFTER_IGNORED  0x0004 // Notification is ON, but we ignore SyncAfter 
#define NOTIFY_SUSPENDED (NOTIFY_SYNCBEFORE_IGNORED | NOTIFY_SYNCAFTER_IGNORED)
#define NOTIFY_ADVISE_DESIGNMODE  0x0008 // Advise is on but for Design mode
#define NOTIFY_ADVISE_DATASOURCE  0x0010 // NotifyDBEvents notification is ON
#define NOTIFY_ADVISE_ROWSET      0x0020 // NotifyDBEvents notification is ON
#define NOTIFY_ADVISE_ROWPOSITION 0x0040 // NotifyDBEvents notification is ON
//#define NOTIFY_SELF_MOVE          0x0080 // Flag that we are causing the move

    //-------------------------------------------------------------------
    // The following were added for ADO
    //-------------------------------------------------------------------
    DataMember   m_dmDataMember; // DataMember property
    DataSource  *m_pDataSrcProp;
    DISPID       m_dispidDataMember;
    LPTSTR       m_strBoundDataField;

	CConnectionsArray  m_connections;	// Manages notifcation connections
    HACCESSOR		   m_hRowAccessor;	// Accessor for row data
    HACCESSOR		   m_hBmkAccessor;	// Accessor for bookmarks
    BLOB              *m_pBlobBinding;  // Blob used when creating accessors

	CfpDBColumn     *m_pColumns;	  // COLUMNINFO array
	ULONG			 m_cColumns;	  // Number of elements in m_pColumns
    BOOL             m_fHasBookmarks;
    BOOL             m_fSuppMultiStreams;
    CfpDBColList     m_BoundCols;     // Array of bound column indexes

    CfpDBColList    *m_pPrevSetDataCols; // Previous array of bound column indexes
    CfpDBColList    *m_pSetDataCols;     // Array of bound column indexes
    HACCESSOR		 m_hSetDataAccessor; // Accessor for calls to SetData
    UINT             m_uSetStmAccessorCt;// The number of stream accessors

    // Fetch "buffer" is really just an array of HROW (row handles).
#define FPDB_MAX_FETCH   10
    CBookmark        m_aBmkFetch[FPDB_MAX_FETCH];
    HROW             m_ahRowFetch[FPDB_MAX_FETCH];
    int              m_cRowFetch;     // Number of valid entries in array
    int              m_iCurrFetch;    // current index of array
#define FPDB_FETCH_FIRSTROW   1
#define FPDB_FETCH_EOF        2
    UINT             m_uFlagsFetch;   // Flags about fetch array.

    DWORD            m_dwFetchStatus; // Fetch status
#define FLAG_CURR_ROW_MOVED        ((DWORD)0x0001)
#define FLAG_CURR_ROW_DATACHANGED  ((DWORD)0x0002)
#define FLAG_NON_CURR_DATACHANGED  ((DWORD)0x0004)
#define FLAG_CURR_ROW_INVALID      ((DWORD)0x0008)
#define FLAG_FETCHED_FROM_CURR_ROW ((DWORD)0x0010) // Fetch is from current row
#define FLAG_FETCHED_FROM_BMARK    ((DWORD)0x0020) // Fetch is from bookmark

#define FLAG_FETCHED_NONSTREAMBUFFER ((DWORD)0x0040) // Fetch is for non-stream columns
#define FLAG_FETCHED_STREAMBUFFER    ((DWORD)0x0080) // Fetch is for one, stream column
// FLAGC - Combinations of the above flags
#define FLAGC_FETCH_NEEDED       \
  (FLAG_CURR_ROW_MOVED | FLAG_CURR_ROW_DATACHANGED | FLAG_NON_CURR_DATACHANGED)
#define FLAGC_CURRENT_FETCH   \
  (FLAG_FETCHED_FROM_CURR_ROW | FLAG_FETCHED_FROM_BMARK) 
#define FLAGC_DATACHANGED        \
  (FLAG_CURR_ROW_DATACHANGED | FLAG_NON_CURR_DATACHANGED)
#define FLAGC_CURR_ROW_RELATED   \
  (FLAG_CURR_ROW_MOVED | FLAG_CURR_ROW_DATACHANGED |  \
   FLAG_NON_CURR_DATACHANGED | FLAG_CURR_ROW_INVALID)
#define FLAGC_FETCHED_BUFFERTYPE \
  (FLAG_FETCHED_NONSTREAMBUFFER | FLAG_FETCHED_STREAMBUFFER)

    CBookmark        m_bmkFetch;      // Current row read by "GetData()"
    int              m_iFetchColIndex;// index of last stream col fetched

    // Data buffer for fetching rows (we allocate to limit mem segmentation).
    ULONG    m_ulBoundInlineLen;       // Size of buffer for Max Data
    LPSTR    m_lpszFetchBuffer;        // "m_hBoundData" locked
    ULONG    m_ulLockCt;
#ifdef FP_USE_GLOBAL
    DWORD    m_hBoundData;             // Buffer handle
#endif

    LPSTR    m_lpszSetDataBuffer;      // Same size as m_lpszFetchBuffer 
                                       //  (m_ulBoundInlineLen).
    int      m_iUpdateStatus;
#define UPDSTAT_BEGIN_EDIT    0x0001
#define UPDSTAT_BEGIN_ADD     0x0002
#define UPDSTAT_SETCOLUMN     0x0004
#define UPDSTAT_SETCOL_FAILED 0x0008

    // NOTE:  This keeps a list of NotifyDBEvents messages and
    //        is only used in DEBUG version.
    CfpNotifyEvents *m_DBEventList;

    ULONG      m_cBmkDeleteList;
    CBookmark *m_aBmkDeleteList;

    ULONG      m_cBmkEventList;
    CBookmark *m_aBmkEventList;
    ULONG     *m_aColIdEventList;

    CBookmark  m_bmkCurrRow;
    HROW       m_hCurrRow;

    BOOL       m_fFreezeEvents;
    BOOL       m_fDataSourceChanged;

// member vars related to Error codes when Updating the record source.
    int      m_iErrorCt;
    struct 
    {
      LONG lErrorCode;
      LONG lExtErrorCode;
      LPVOID lpvUserData;
    } m_ErrorInfo;

    BOOL m_fUseChapters;
    BOOL m_fBatchUpdate;

    CfpDBEventQueue m_EventQueue;

    CfpDbInsRowCls  m_InsertedRowList;

public:
    HRESULT SetDataMember(DataMember dmDataMember);  // DataMember property
    HRESULT GetDataMember(DataMember *dmDataMember); // DataMember property

	HRESULT GetDataSource(DataSource **ppDataSource);
	HRESULT SetDataSource(DataSource *pDataSource);

    HRESULT GetDataField(BSTR *pbstrDataField);
    HRESULT SetDataField(BSTR bstrDataField);


    HRESULT fpInitDataBinding(void);
    HRESULT fpReleaseDataBinding(void);

protected:
    virtual HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);


    HRESULT fpReInitDataBinding(void);

    HRESULT fpInitRowset(void);
    HRESULT fpDetachRowset(void);
    HRESULT fpAttachRowset(void);

    HRESULT fpConnect(BOOL fUserMode);
    HRESULT fpDisconnect(void);

    HRESULT fpFreeRowAccessor(void);
    HRESULT fpFreeBoundCols(void);

    HRESULT fpCopyColumns(COLUMNINFO *pColumns, ULONG cColumns);
    HRESULT fpFreeColumns(void);
    HRESULT fpFreeFetchArray(void);

#define FPDB_REQUERY_ALL       0x00
#define FPDB_RELEASE_ONLY      0x01  // Use with following func.
#define FPDB_PRESERVE_CHAPTER  0x02  // Use with following func.
    HRESULT fpRequeryCurrHRow(int fReleaseFlags = FPDB_REQUERY_ALL);

    HRESULT fpReleaseChapter(HCHAPTER *phChapter);

    HRESULT fpInitColumns(void);

    HRESULT fpSetBoundColumn(void);
    BOOL    fpBindDataFieldColumn( LPTSTR strDataField);
    HRESULT fpCreateStreamAccessors(DBSTATUS *dbStatus, 
      LPBLOB *plpBlob, ULONG obInline);

    HRESULT fpGetBookmarkAtHRow( HROW hRow, CBookmark *pBmk, 
      BOOL fReleaseRow = FALSE);

    BOOL    fpGetBookmark( LONG lBmarkType, LPVOID hlstrBmark, 
      LPVOID FAR *lphlstrRetBmark, USHORT FAR *lpuRetFlags, 
      BOOL fUseSeqRead = FALSE);

    HRESULT fpGetFetchIndex(CBookmark *pBmk, int *pRetIndex, 
      USHORT *pRetFlags, LONG lRelOffset = 0);

    BOOL fpFindFetchIndex(CBookmark *pBmk, int *pRetIndex,
      int iDirection = 1 );

    void fpFreePreviousFetchData(DWORD flags);
    void fpPostFetch_Init(DWORD flags);

    void fpReleaseStreamAccessors(void);

    HRESULT fpVerifyFetchBuffer( LPVOID hlstrBookmark, 
      USHORT FAR *lpuFlags, int colIndex);

    HRESULT CfpOleBinding::fpFillFetchBuffer(CBookmark *pbmkFetch,
      HROW  hRow, DWORD dwBufferType, BOOL  fFetchRow,
      DWORD dwFetchStatus, USHORT *lpuFlags, int colIndex);

    BOOL fpGetFieldValue(BOOL fOnlyGetLen, LPVOID lpvda, 
      BOOL fUseSeqRead = FALSE);

    BOOL fpMoveCurrRow(USHORT usMoveAction, LPVOID hlstrBmark,
      UINT FAR *lpuRetFlags);
    BOOL fpDeleteRow(LPVOID hlstrBmark);
    BOOL fpInsertRow(void);
    BOOL fpSetFieldValue(LPVOID lpvda);
    BOOL fpVerifySetDataAccessor(void);
    BOOL fpFinishUpdatingRow(void);

    void fpGetWriteAccessorCt(UINT *puRetStreamCt, 
      UINT *puRetSeperateAccCt);
    void    fpReleaseWriteAccessors(void);
    HRESULT fpCreateOtherWriteAccessors(LPBLOB *plpBlob,
      UINT uStreamAccessorCt = 0, UINT uOtherAccessorCt  = 0);

    BOOL fpDBPostMsgToSelf(WPARAM wParam, LPARAM lParam);


    // Functions to allocate and lock fetch buffer.
    BOOL    fpHasFetchBuffer(void);
    HRESULT fpAllocFetchBuffer(ULONG cbSize);
public:
    void    fpFreeFetchBuffer(void);
protected:
    LPSTR   fpLockFetchBuffer(void);
    void    fpUnlockFetchBuffer(void);

    HRESULT fpOKToDo(DBREASON eReason);
    HRESULT fpAboutToDo( DBREASON eReason, ULONG cRows,
      const HROW *rghRows, const ULONG *rgCols);
    HRESULT fpFailedToDo(DBREASON eReason);
    HRESULT fpSynchAfter(DBREASON eReason);
    HRESULT fpDidEvent(DBREASON eReason, ULONG cRows, const HROW *rghRows, 
      const ULONG *rgCols);
    HRESULT fpEachDidEvent(DBREASON eReason, ULONG cRows, 
      const HROW *rghRows, const ULONG *rgCols);
    SHORT fpTranslateAboutToDoReason( DBREASON eReason, ULONG cRows,
      const HROW *rghRows, BOOL fDataChanged);
    SHORT fpTranslateDidEventReason( DBREASON eReason, ULONG cRows,
      const HROW *rghRows, const ULONG *rgCols);
    void  fpCurrentRowChanged(DWORD dwMask, DWORD dwFlags);

    BOOL fpSetupReplacementBmks(HROW hRow);

    BOOL GetDataSourceInterface(DataSource **ppRetDataSource,
      BOOL *pRetfRelease);

// Use these defines for the "pfQueueEvent" parameter in the following funcs:
#define FPDB_EVENT_CANCEL_MSG     0
#define FPDB_EVENT_QUEUE_MSG      1
#define FPDB_EVENT_PROCESS_NOW    2

    HRESULT fpFilter_OKToDo(DBREASON eReason, int *pfQueueEvent);
    HRESULT fpFilter_AboutToDo(DBREASON eReason, int *pfQueueEvent);
    HRESULT fpFilter_FailedToDo(DBREASON eReason, int *pfQueueEvent);
    HRESULT fpFilter_SynchAfter(DBREASON eReason, int *pfQueueEvent);
    HRESULT fpFilter_DidEvent(DBREASON eReason, int *pfQueueEvent);

#define FPDB_USE_DELETE_LIST  TRUE  // use for "fUseDeleteList" param.
    void  fpSaveEventBookmarks(ULONG cRows, const HROW *rghRows, 
      const ULONG *rgColIds, BOOL fUseDeleteList = FALSE);
    void  fpClearEventBookmarks(BOOL fUseDeleteList = FALSE);

    void  fpResetEventBookmarks(LPVOID *pRetList, ULONG *pRetListCt,
      ULONG **pRetColIdList);
    void  fpRestoreEventBookmarks(LPVOID list, ULONG listCt,
      ULONG *aColIds);


    HRESULT fpProcessEvent(DBEVENTPHASE ePhase, DBREASON eReason, 
      ULONG cRows, const HROW *rghRows, const ULONG *rgCols);

    HRESULT fpQueueEvent(DBEVENTPHASE ePhase, DBREASON eReason,
      ULONG cRows, const HROW *rghRows, const ULONG *rgCols);

    HRESULT fpGetMultiStreamProp(BOOL *pfMultiStream);

    HRESULT fpGetBatchUpdateProp(BOOL *pfBatchUpdate);


    short ProcessData_Get( LPVOID lpvda, USHORT usAction, LPVOID lpvUserData);
    short ProcessData_Method( LPVOID lpvda, USHORT usAction, LPVOID lpvUserData);
    short ProcessData_Set( LPVOID lpvda, USHORT usAction, LPVOID lpvUserData);

// User-defined DBREASONs to trigger special VBM_DATA messages 
// not represented in the current list of DBREASONs.
#define UDBREASON_INIT_CONNECT   (DBREASON_ROWSET_POPULATIONSTOPPED + 100)
#define UDBREASON_DISCONNECT     (DBREASON_ROWSET_POPULATIONSTOPPED + 101)
#define UDBREASON_SOURCECHANGED  (DBREASON_ROWSET_POPULATIONSTOPPED + 102)
#define UDBREASON_SELF_REFRESH   (DBREASON_ROWSET_POPULATIONSTOPPED + 103)
#define UDBREASON_UPDATE_ROW     (DBREASON_ROWSET_POPULATIONSTOPPED + 104)
#define UDBREASON_REPLACE_BMK    (DBREASON_ROWSET_POPULATIONSTOPPED + 105)

};

#endif  // ifndef FPOLEDBA_H
