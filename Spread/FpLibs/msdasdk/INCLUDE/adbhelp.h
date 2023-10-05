//=--------------------------------------------------------------------------=
// ADBHELP.H:	Interface for OLE DB helpers
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _ADBHELP_H_
#define _ADBHELP_H_

// Use OLEDB 2.0 interfaces, but supply our own
// implementations if providers does not
//
#undef OLEDBVER
#define OLEDBVER 0x0200
#include <oledb.h>
#include <msdatsrc.h>
#include <arrtmpl.h>
/////////////////////////////////////////////////////////////////////////////
// Helpers
//

void ClearID(DBID& dbid);
void CopyID(DBID& dbidDst, const DBID& dbidSrc);
BOOL IsSameIDs(const DBID& dbid1, const DBID& dbid2);
BOOL IsBmkColumn(const DBCOLUMNINFO& col);

#define UNUSED_OFFSET	((ULONG)-1L)

void InitBinding(DBBINDING& dbBinding, ULONG iOrdinal, 
 DBTYPE wType, ULONG cbMaxLen, ULONG obValue, 
 ULONG obLength = UNUSED_OFFSET, ULONG obStatus = UNUSED_OFFSET, 
 BYTE bPercision = 0, BYTE bScale = 0);

/////////////////////////////////////////////////////////////////////////////
// Data Types
//

// Columns structure extending DBCOLUMNINFO
// to contain addtional metadata
//
struct COLUMNINFO : public DBCOLUMNINFO
{
public:
	COLUMNINFO();
	~COLUMNINFO();

	void Clear(BOOL fZeroMemory = TRUE);

// Additional metadata
public:
	VARIANT	varDefault;		// Default value for a column

// Operators
public:
	COLUMNINFO& operator=(const COLUMNINFO& other);
	COLUMNINFO& operator=(const DBCOLUMNINFO& other);
};

// This structure contains the most pertinent
// properties for a Rowset that a consumer 
// may be interested in
//
struct ROWSETPROPERTIES
{
	// Value properties
	//
	struct
	{
		long	MaxOpenRows;	// Maximum rows consumers are allowed to hold
		long	Updatability;	// IRowsetChange methods supported
	}	value;

	// Boolean properties
	//
	struct
	{
		UINT	StrongId		:1;	// Newly inserted HROWs can be compared
		UINT	LiteralId		:1;	// HROWs can be literally compared
		UINT	HasBookmarks	:1;	// Rows have self-bookmark columns
									//	IRowsetLocate is implemented
		UINT	CanHoldRows		:1;	// HROWs can be held inbetween GetRows
		UINT	CanScrollBack	:1;	// GetRows can scroll backwards
									//	IRowsetScroll is implemented
		UINT	CanFetchBack	:1;	// GetRows can fetch backwards
		UINT	CanChange		:1;	// IRowsetChange is implemented
		UINT	CanUpdate		:1;	// IRowsetUpdate is implemented

		UINT	IRowsetScroll	:1;		// IRowsetScroll is implemented
	}	flag;
};

/////////////////////////////////////////////////////////////////////////////
// CBindingsArray
//
class CBindingsArray : public CArray<DBBINDING, const DBBINDING&>
{
public:
	CBindingsArray() {};
	~CBindingsArray() {};

// Operations
public:
	HRESULT AddBinding(const DBBINDING& dbBinding) {return Add(dbBinding) ? S_OK : E_OUTOFMEMORY;}
	HRESULT AddBinding(ULONG iOrdinal, DBTYPE wType, 
	 ULONG cbMaxLen, ULONG obValue, 
	 ULONG obLength = UNUSED_OFFSET, ULONG obStatus = UNUSED_OFFSET, 
	 BYTE bPercision = 0, BYTE bScale = 0);
};

#endif // _ADBHELP_H_