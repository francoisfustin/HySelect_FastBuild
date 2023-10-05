
//=--------------------------------------------------------------------------=
// BOOKMARK.H:	Interface for CBookmark helper class
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _BOOKMARK_H_
#define _BOOKMARK_H_

// Cursor standard bookmarks
// not to be used with OLE DB
//
#define BMK_SIZE        sizeof(BYTE)
extern const BYTE BMK_INVALID;
extern const BYTE BMK_CURRENT;
extern const BYTE BMK_BEGINNING;
extern const BYTE BMK_END;

// OLE DB standard bookmarks
// not to be used with Cursors
//
extern const BYTE DBBMK_FIRSTROW;
extern const BYTE DBBMK_LASTROW;

/////////////////////////////////////////////////////////////////////////////
// CBookmark
//
class CBookmark : public BLOB
{
public:
	CBookmark();
	CBookmark(const CBookmark& other);
	CBookmark(ULONG ulSize, const BYTE *pBlob);
	CBookmark(const BLOB& blob);
	~CBookmark();

public:
	BOOL IsInvalid(void) const {return 0 == cbSize || (BMK_SIZE == cbSize && BMK_INVALID == BmkByte());}

	// For Cursors
	//
	BOOL IsBeginning(void) const {return BMK_SIZE == cbSize && BMK_BEGINNING == BmkByte();}
	BOOL IsCurrent(void) const {return BMK_SIZE == cbSize && BMK_CURRENT == BmkByte();}
	BOOL IsEnd(void) const {return BMK_SIZE == cbSize && BMK_END == BmkByte();}
	
	// For OLE DB
	//
	BOOL IsFirstRow(void) const {return BMK_SIZE == cbSize && DBBMK_FIRSTROW == BmkByte();}
	BOOL IsLastRow(void) const {return BMK_SIZE == cbSize && DBBMK_LASTROW == BmkByte();}

	void Clear(void);
	HRESULT Set(ULONG ulSize, const BYTE *pBlob);
	HRESULT Set(const VARIANT& var);
	HRESULT Get(VARIANT& var) const;

public:
	operator BLOB&() {return *((BLOB *)this);}
	operator const BLOB&() const {return *((BLOB *)this);}
	operator ULONG() const {return cbSize;}
	operator void*() {return (cbSize > sizeof pBlobData) ? pBlobData : (void*)&pBlobData;}
	operator const void*() const {return (cbSize > sizeof pBlobData) ? pBlobData : (void*)&pBlobData;}
	operator const BYTE*() const {return (const BYTE*)(const void*)*this;}
	operator BYTE*() {return (BYTE*)(void*)*this;}

	BOOL operator==(const CBookmark& other) const;
	BOOL operator==(const VARIANT& var) const;
	CBookmark& operator=(const CBookmark& other);

	static BOOL IsBookmark(const VARIANT& var) {return (VT_ARRAY|VT_UI1) == V_VT(&var) && 1 == SafeArrayGetDim(V_ARRAY(&var));}

protected:
	BYTE BmkByte(void) const {return *((BYTE *)&pBlobData);}
	ULONG BmkULong(void) const {return *((ULONG *)&pBlobData);}
};

#endif // _BOOKMARK_H_
