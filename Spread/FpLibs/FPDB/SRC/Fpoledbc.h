/*************************************************************************\
* FPOLEDBC.H  -  FarPoint Data Binding include file for "Cursor" Binding. *
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
// fpoledbc.h : Declaration of the CfpOleBinding class.
//
//             FarPoint's implementation of DB Binding for OLE controls.
//
//             This class is intented to be a member variable (public) of
//             an OCX control (subclassed from COleControl).
//

#ifndef FPOLEDBC_H
#define FPOLEDBC_H

#ifndef __OCDB_H_
//#include <initguid.h>
#include <ocdbid.h>
#include <olebind.h>
#include <ocdb.h>
#endif

#if defined(NO_MFC) && !defined(SS40)
#include "fpatl.h"
#endif

//#include "fpdbutil.h"
#include "fpdbnote.h"

#define  FPDB_BOOKMARK_COLINDEX  -2 

class CfpDBColumn;              // declared in "fpdbcol.h"
class CfpNotifyEvents;          // declared in "fpdbutil.h"

//-------------------------------------------------------------------------
// "Public" Class - CfpOleBinding
//-------------------------------------------------------------------------
#if defined(SS40) || !defined(NO_MFC) // MFC
class CfpOleBinding : public CObject
#else          // NO_MFC
class CfpOleBinding : 
  public CComObjectRoot, public INotifyDBEvents
#endif
{

#if defined(SS40) || !defined(NO_MFC)  // NO_MFC
#define INTERFACE_(cls)   m_x##cls. 
#define FPDB_IMPL_FUNC(basecls, cls)   basecls::cls:: 

#define GET_AMBIENT_USERMODE  AmbientUserMode
friend class XBoundObject;
friend class XNotifyDBEvents;

#else  // NO_MFC
#define INTERFACE_(cls)
#define FPDB_IMPL_FUNC(basecls, cls)   basecls:: 
#define METHOD_PROLOGUE_(basecls, cls) basecls *pThis = this;

#define GET_AMBIENT_USERMODE  fpAtlGetUserMode

	STDMETHOD_(ULONG, AddRef)()  {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)() {return InternalRelease();}
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID*ppvObj)
      { return _InternalQueryInterface(iid, ppvObj); }

BEGIN_COM_MAP(CfpOleBinding)
	COM_INTERFACE_ENTRY(INotifyDBEvents)
FP_END_COM_MAP()

#endif

//	DECLARE_DYNCREATE(CfpOleBinding)

public:
    // NOTE:  This keeps a list of NotifyDBEvents messages and
    //        is only used in DEBUG version.
    CfpNotifyEvents *m_DBEventList;

//-------------------------------------------------------------------------
// PUBLIC MEMBER FUNCTIONS
//-------------------------------------------------------------------------
public:
// Constructor
	CfpOleBinding();
// Destructor
	~CfpOleBinding();

// Set-up of Environment
    void SetOwner( LPVOID lpOleControl);
    void SetDataSourceDispid( DISPID dispid);
    void SetBindWithEntryId( BOOL fUseEntryId);

    // NOTE: lpfuncDataMsg is ptr to functions of type:
    //       extern "C" void DataMsg( LPVOID lpObject, UINT uMsg,
    //                                WPARAM wParam, LPARAM lParam);
    //       When the function is invoked, "lpObject" is ptr to "Owner".
    void SetDataMsgFunc( LPVOID lpfuncDataMsg);

    // Whether to include bookmark column when binding columns.
    BOOL SetBindToBookmark(BOOL fSetting);  // Default is FALSE.

// Use for OnGetPredefinedStrings()
    BOOL PrivGetListOfColNames(CStringArray *pStringArray,
      CDWordArray *pCookieArray);

#ifndef NO_MFC  // MFC
    BOOL GetListOfColumnNames(CStringArray *pStringArray,
      CDWordArray *pCookieArray);
#else           // NO_MFC
    HRESULT GetListOfColumnNames(CALPOLESTR *pCaStringsOut,
      CADWORD *pCaCookiesOut);
#endif

// Use for OnGetPredefinedValue()
    BOOL GetColumnNameForCookie(DWORD dwCookie, BSTR FAR *lpRetBSTR);

// Use to support DataField property
    BSTR GetDataField(void); 
    void SetDataField(LPCTSTR lpszNewValue);

// Release DataSource (removes all connections to cursor)
    void ReleaseDataSource(void);

// Related to start/stop NotifyDBEvent notification messages.
// Returns previous state.
    BOOL AdviseDBEvents(void);   // start receiving msgs
    BOOL UnadviseDBEvents(void); // stop receiving

// Interface to Data functions (LPVOID -> LPDATAACCESS)
    short DataCall(LPVOID lpvda, USHORT msg, USHORT action, 
      LPVOID lpvUserData = NULL);

// 	BOOL OnGetDisplayString(DISPID dispid, CString& strValue);
//	BOOL OnGetPredefinedStrings(DISPID dispid, CStringArray *pStringArray, CDWordArray *pCookieArray);
//	BOOL OnGetPredefinedValue(DISPID dispid, DWORD dwCookie, VARIANT FAR *lpvarOut);
//  BOOL GetPredefinedValueForString(DISPID dispid, CString *pLookupCString,
//	       DWORD FAR *pdwCookie);


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

// Following creates member variable:  XNotifyDBEvents m_xNotifyDBEvents;
BEGIN_INTERFACE_PART(NotifyDBEvents, INotifyDBEvents)
	INIT_INTERFACE_PART(CfpOleBinding, NotifyDBEvents)
	STDMETHOD(OKToDo)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
	STDMETHOD(Cancelled)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
	STDMETHOD(SyncBefore)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
	STDMETHOD(AboutToDo)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
	STDMETHOD(FailedToDo)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
	STDMETHOD(SyncAfter)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
	STDMETHOD(DidEvent)(DWORD dwEventWhat, ULONG cReasons, DBNOTIFYREASON rgReasons[]);
    // Not part of STANDARD interface...
    private:
    SHORT TranslateAboutToDoDBReasons( DWORD dwEventWhat, ULONG cReasons, 
      DBNOTIFYREASON FAR *rgReasons, USHORT uActionSize, 
      SHORT FAR *rgActionArray, BOOL FAR *processList);
    SHORT TranslateSyncAfterDBReasons( DWORD dwEventWhat, ULONG cReasons, 
      DBNOTIFYREASON FAR *rgReasons, USHORT uActionSize, 
      SHORT FAR *rgActionArray, BOOL FAR *processList, 
      LPVOID FAR *lphlstrBmark);
END_INTERFACE_PART(NotifyDBEvents)


//-------------------------------------------------------------------------    
// PROTECTED MEMBER FUNCTIONS...
//-------------------------------------------------------------------------    
protected:
    void FlushColInfoList(void);
	BOOL GetBookmark( LONG lBmarkPos, LPVOID hlstrBmark, 
	       LPVOID FAR *lphlstrRetBmark, UINT FAR *lpuRetFlags, 
           BOOL fUseClone = FALSE);
    BOOL MoveCurrentRow( USHORT usMoveAction, LPVOID hlstrBmark,
           UINT FAR *lpuRetFlags, LONG lRowOffset = 0,
           ICursorMove FAR *lpMoveCursor = NULL);
	BOOL GetFieldValue( BOOL fOnlyGetLen, LPVOID lpvda, /* LPVOID -> LPDATAACCESS */
      BOOL fReadCurrClone = FALSE);
    BOOL SetFieldValue(LPVOID lpvda);            /* LPVOID -> LPDATAACCESS */
    BOOL VerifyFetchBuffer( LPVOID hlstrBookmark, UINT FAR *lpuFlags);
    void PostFetch_Init(void);
    BOOL ReadNextRow(BOOL fReadFirst, UINT FAR *lpuFlags);
    BOOL FetchCurrentRow(UINT FAR *lpuFlags);
    void FreePreviousFetchData(void);
    BOOL BindDataFieldColumn( LPTSTR strDataField);
    BOOL AddNewRow(void);
    BOOL Process_AddNew(void);
    BOOL DeleteCurrentRow(void);
    BOOL FinishUpdatingRow(void);

#define FORCE_CURSOR_REFRESH  TRUE
    BOOL GetCursor(BOOL bForceRefresh = FALSE);
    BOOL GetMoveCursor(BOOL bForceRefresh = FALSE);
    BOOL GetUpdateCursor(BOOL bForceRefresh = FALSE);

    CfpDBColumn * GetColumnRec(LPCTSTR tstrColumnName);
    CfpDBColumn * GetColumnRec(int iFieldPos);

	BOOL GetMetaData(CStringArray *pStringArray, CDWordArray *pCookieArray);
    BOOL SetBoundColumn(void);    // Binds as specified by m_arrayBoundCols

    short ProcessData_Get( LPVOID lpvda, USHORT usAction, LPVOID lpvUserData);
    short ProcessData_Method( LPVOID lpvda, USHORT usAction, LPVOID lpvUserData);
    short ProcessData_Set( LPVOID lpvda, USHORT usAction, LPVOID lpvUserData);
    BOOL  GetRecordCount( LPLONG lpLong);
    void  CurrentRowChanged(DWORD dwMask, DWORD dwFlags);

    void  Simulate_NotifyDBEvents( DWORD dwEventWhat, ULONG cReasons, 
            DBNOTIFYREASON rgReasons[]);

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

    void DetermineRdcVersion(void);
    void CheckForReplicatedTable(void);

    
//-------------------------------------------------------------------------    
// PROTECTED MEMBER VARIABLES...
//-------------------------------------------------------------------------    
protected:
// Dependent on environment (OLE Control)
#ifdef SS40
    VOID FAR *m_lpOwner;
#elif !defined(NO_MFC)
    COleControl FAR *m_lpOwner;            // Pointer to OCX class (owner)    
#else
    CfpAtlBase  FAR *m_lpOwner;            // Pointer to OCX class (owner)    
#endif
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

// Data binding members
    ICursor     FAR *m_lpCursor;
	ICursorMove FAR *m_lpMoveCursor;				
    ICursorMove FAR *m_lpCloneMoveCursor;
    ICursorUpdateARow FAR *m_lpUpdateCursor;

    IConnectionPoint FAR *m_lpConnPt;

	CObList          m_ColInfoList;

#define FPDB_NOT_RDC       0x00
#define FPDB_RDC           0x01  // Data Ctl is identified as a Remote Data ctl.
#define FPDB_RDC_TEST_VER  0x02  // Version of RDC not yet determined.
#define FPDB_RDC_VER1      0x04  // Version 1 of RDC
#define FPDB_RDC_VER2      0x08  // Version 2 of RDC
#define FPDB_RDC_MOVEGET   0x10  // GetNextRows() should move to next row but doesn't!

    int              m_fRemoteDataCtl; // Flag if using Remote Data Control	

    DWORD m_dwNotifyDBEventsCookie;

    LPTSTR m_strBoundDataField;      // current DataField setting
    CObArray m_arrayBoundCols;         // Array of CfpDBColumn ptrs

    // Data buffer for fetching rows (we allocate to limit mem segmentation).
    ULONG    m_ulBoundInlineLen;       // Size of buffer for Max Data
    ULONG    m_ulBoundOutlineLen;      // Size of buffer for Max Data
    DWORD    m_hBoundData;             // Buffer handle

    // Fetch is valid until cursor moves to a different record 
    DWORD    m_dwFetchStatus;
#define FLAG_CURR_ROW_MOVED        ((DWORD)0x0001)
#define FLAG_CURR_ROW_DATACHANGED  ((DWORD)0x0002)
#define FLAG_NON_CURR_DATACHANGED  ((DWORD)0x0004)
#define FLAG_CURR_ROW_INVALID      ((DWORD)0x0008)
#define FLAG_FETCHED_FROM_CURSOR   ((DWORD)0x0010) // Fetch is from cursor 
#define FLAG_FETCHED_FROM_BMARK    ((DWORD)0x0020) // Fetch is from bookmark
#define FLAG_FETCHED_FROM_CLONE    ((DWORD)0x0040) // Fetch is from Clone cursor

// FLAGC - Combinations of the above flags
#define FLAGC_FETCH_NEEDED       \
  (FLAG_CURR_ROW_MOVED | FLAG_CURR_ROW_DATACHANGED | FLAG_NON_CURR_DATACHANGED)
#define FLAGC_BMARK_FROM_FETCH   \
  (FLAG_FETCHED_FROM_CURSOR | FLAG_FETCHED_FROM_BMARK | FLAG_FETCHED_FROM_CLONE) 
#define FLAGC_DATACHANGED        \
  (FLAG_CURR_ROW_DATACHANGED | FLAG_NON_CURR_DATACHANGED)
#define FLAGC_CURR_ROW_RELATED   \
  (FLAG_CURR_ROW_MOVED | FLAG_CURR_ROW_DATACHANGED |  \
   FLAG_NON_CURR_DATACHANGED | FLAG_CURR_ROW_INVALID)

    BOOL     m_fCurrRowInvalid;

    LPVOID   m_hlstrFetchBmark;     // LPVOID same as HLSTR

    int      m_iUpdateStatus;
#define UPDSTAT_BEGIN_EDIT 0x0001
#define UPDSTAT_BEGIN_ADD  0x0002
#define UPDSTAT_SETCOLUMN  0x0004

// Pre-processor for the DB notification messages
    CfpDBNotify m_NotifyPreprocessor;

// member vars related to Error codes when Updating the record source.
    int      m_iErrorCt;
    struct 
    {
      LONG lErrorCode;
      LONG lExtErrorCode;
      LPVOID lpvUserData;
    } m_ErrorInfo;

    BOOL m_fUseEntryId;

    BOOL         m_fBindBMKCol;  // Whether to TRY to bind to bookmark col.
    CfpDBColumn *m_BmkColInfo;   // Column info if bound to bookmark column.


#define FPDB_NOT_VBREFRESH     0
#define FPDB_VBREFRESH_FIRST   1 // on this state, move to beginning
#define FPDB_VBREFRESH_NEXT    2 // no more moves, just "GetNextRows()"
    int          m_fVBRefresh;   // ctl is being refreshed (reading all rows)
};

#endif  // ifndef FPOLEDBC_H
