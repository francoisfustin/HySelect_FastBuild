//=--------------------------------------------------------------------------=
// ROWSET.H:	Interface for CRowset wrapper class
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _ROWSET_H_
#define _ROWSET_H_

#include <adbhelp.h>
#include <bookmark.h>

class CRowset
{
public:
	CRowset();
	virtual ~CRowset();

// Attributes
public:
	// Allow consumers to get at key Rowset interfaces.
	// These interfaces are returned without AddRef.
	//
	IRowset* GetRowset(void);
	IAccessor* GetAccessor(void);
	IRowPosition* GetRowPosition(void);
	IDataSource* GetDataSource(void);

// Overidables
public:
	// Attach to a DataSource or an IRowset or IRowPosition
	//
	virtual HRESULT Attach(IUnknown *punk, BSTR bstrDM = NULL);
	
	// Support for hierachical Rowsets
	//
	virtual HCHAPTER GetChapter(void) {return m_hChapter;}
	virtual HCHAPTER SetChapter(HCHAPTER hchapter);

	// Allows consumers to review most relevant
	// Rowset properties
	//
	virtual const ROWSETPROPERTIES& GetProperties(void);

// Operations
public:
	// IRowsetScroll
	//
	HRESULT GetCount(long& lCount);
	HRESULT GetApproximatePosition(const CBookmark& bmk, long& lPos);
	HRESULT GetRowsAtRatio(ULONG ulNumerator, ULONG ulDenominator, long cRowsRequested, HROW*& phRows, ULONG& cRows);

	// IAccessor
	//
	HRESULT CreateAccessor(const DBBINDING *pBindings, ULONG cBindings, HACCESSOR& haccessor, DBBINDSTATUS *pStatus = NULL);
	HRESULT CreateAccessor(const CBindingsArray& bindings, HACCESSOR& haccessor, DBBINDSTATUS *pStatus = NULL);
	HRESULT ReleaseAccessor(HACCESSOR haccessor);
	HRESULT AddRefAccessor(HACCESSOR haccessor);
	HRESULT GetBindings(HACCESSOR haccessor, DBBINDING*& pBindings, ULONG& cBindings, DBACCESSORFLAGS& dwFlags);

	// IColumnInfo and IColumnsRowset
	//
	HRESULT GetColumns(COLUMNINFO*& pCols, ULONG& cCols);
	HRESULT GetColumnOrdinals(ULONG *pOrdinals, const DBID* pdbColIDs, ULONG cColIDs);

	// IRowPosition
	//
	HRESULT GetCurrentRow(HROW& hrow, DBPOSITIONFLAGS *pdwPositionFlags = NULL);

	// IRowset
	//
	HRESULT RestartPosition(void);
	HRESULT GetNextRows(long lOffset, long cRowsRequested, HROW*& phRows, ULONG& cRows);
	HRESULT FreeRows(const HROW *pRows, ULONG cRows);
	HRESULT ReleaseRows(const HROW *pRows, ULONG cRows);
	HRESULT AddRefRows(const HROW *pRows, ULONG cRows);
	HRESULT GetData(HROW hrow, HACCESSOR haccessor, void *pBuffer);

	// IRowsetChange
	//
	HRESULT SetData(HROW hrow, HACCESSOR haccessor, void *pBuffer);
	HRESULT DeleteRows(const HROW *phRows, ULONG cRows, DBROWSTATUS *pRowsStatus);
	HRESULT InsertRow(HACCESSOR haccessor, void *pData, HROW& hrow);

	// IRowsetUpdate
	//
	HRESULT GetOriginalData(HROW hrow, HACCESSOR haccessor, void *pData);
	HRESULT GetPendingRows(DBPENDINGSTATUS dwStatus, HROW*& phRows, ULONG& cRows, DBPENDINGSTATUS **ppStatus);
	HRESULT GetRowStatus(const HROW *phRows, ULONG cRows, DBPENDINGSTATUS *pStatus);
	HRESULT Undo(const HROW *phRows, ULONG cRows, ULONG *pcRows, HROW **pphRows, DBROWSTATUS **ppStatus);
	HRESULT Update(const HROW *phRows, ULONG cRows, ULONG *pcRows, HROW **pphRows, DBROWSTATUS **ppStatus);

	// IRowsetLocate
	//
	HRESULT GetRows(const CBookmark& bmk, long lOffset, long cRowsRequested, HROW*& phRows, ULONG& cRows);
	HRESULT GetRows(const CBookmark *pBookmarks, ULONG cBookmarks, HROW* phRows, DBROWSTATUS *pRowStatus = NULL);
	HRESULT CompareBookmarks(const CBookmark& bmk1,const CBookmark& bmk2, DBCOMPARE& dbCmp);

	// IRowsetInfo
	//
	HRESULT GetRowsetProperties(IUnknown *pRowset, const DBPROPID *pPropIDs, VARIANT *pValues, ULONG cPropIDs);

	// IRowsetIdentity
	//
	HRESULT IsSameRow(HROW hrow1, HROW hrow2);

	// IConvertType
	//
	HRESULT CanConvert(DBTYPE dbtFrom, DBTYPE dbtTo);

	// Other Rowset operations
	//
	HRESULT IsSameRow(const CBookmark& bmk1, const CBookmark& bmk2);
	HRESULT GetSelfBookmark(HROW hrow, CBookmark& bmk);
	HRESULT GetExactPosition(const CBookmark& bmk, long& lPos);
	
	DBCOLUMNFLAGS GetColumnFlags(int iOrdinal);
	ULONG GetColumnSize(int iOrdinal);
	DBTYPE GetColumnType(int iOrdinal);

// Implementation
protected:
	// Interfaces cached as needed
	//
	IRowset				*m_pRowset;
	IRowPosition		*m_pRowsetPos;
	IRowsetLocate		*m_pRowsetLocate;
	IRowsetScroll		*m_pRowsetScroll;
	IRowsetFind			*m_pRowsetFind;
	IRowsetInfo			*m_pRowsetInfo;
	IRowsetIdentity		*m_pRowsetIdentity;
	IRowsetChange		*m_pRowsetChange;
	IRowsetUpdate		*m_pRowsetUpdate;
	IAccessor			*m_pAccessor;
	IConvertType		*m_pConvertType;
	IColumnsInfo		*m_pColumnsInfo;
	DataSource			*m_pDataSource;

	// DataMember
	//
	BSTR				m_bstrDM;

	// Used for hierachical Rowsets
	//
	HCHAPTER			m_hChapter;

	// Intialized after every attachment
	//
	ROWSETPROPERTIES	m_properties;

	// Used to cache a subset of DBCOLUMNINFO
	//
	struct COLUMN
	{
		COLUMN();
		COLUMN(const DBCOLUMNINFO& dbColInfo);

		DBCOLUMNFLAGS dwFlags;
		ULONG         ulSize;
		DBTYPE        wType;

		COLUMN& operator=(const COLUMN& other);
		COLUMN& operator=(const DBCOLUMNINFO& dbColInfo);
	};
	CArray<COLUMN, COLUMN&>	m_columns;

	// These are called as needed by methods to
	// do one time intializaion of Rowset interfaces
	//
	HRESULT InitRowset(void);
	HRESULT InitRowsetLocate(void);
	HRESULT InitRowsetScroll(void);
	HRESULT InitRowsetFind(void);
	HRESULT InitRowsetInfo(void);
	HRESULT InitRowsetIdentity(void);
	HRESULT InitRowsetChange(void);
	HRESULT InitRowsetUpdate(void);
	HRESULT InitAccessor(void);
	HRESULT InitConvertType(void);
	HRESULT InitColumnsInfo(void);
	HRESULT InitColumnsMetadata(COLUMNINFO*& pColumns, ULONG cColumns);
	HRESULT InitColumns(void);
};

#endif // _ROWSET_H_
