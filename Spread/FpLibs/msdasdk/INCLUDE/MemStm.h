//=--------------------------------------------------------------------------=
// MemStm.h:	Memory Stream Object
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _MEMSTM_H_
#define _MEMSTM_H_

#include <Unknown.h>
#include <arrtmpl.h>
#if VC8
class CMemStream : public CUnknownObject, public IStream, protected CArrayOld<BYTE, BYTE*>
#else
class CMemStream : public CUnknownObject, public IStream, protected CArray<BYTE, BYTE*>
#endif
{
public:
	CMemStream();
	virtual ~CMemStream();

public:
	// IUnknown methods
	//
	DECLARE_STANDARD_UNKNOWN();

	// ISequentialStream methods
	//
	STDMETHOD(Read)(void __RPC_FAR *pv, ULONG cb, ULONG __RPC_FAR *pcbRead);
	STDMETHOD(Write)(const void __RPC_FAR *pv, ULONG cb, ULONG __RPC_FAR *pcbWritten);

	// IStream methods
	//
	STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER __RPC_FAR *plibNewPosition);
	STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
	STDMETHOD(CopyTo)(IStream __RPC_FAR *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER __RPC_FAR *pcbRead, ULARGE_INTEGER __RPC_FAR *pcbWritten);
	STDMETHOD(Commit)(DWORD grfCommitFlags);
	STDMETHOD(Revert)(void);
	STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
	STDMETHOD(Stat)(STATSTG __RPC_FAR *pstatstg, DWORD grfStatFlag);
	STDMETHOD(Clone)(IStream __RPC_FAR *__RPC_FAR *ppstm);

protected:
	virtual HRESULT InternalQueryInterface(REFIID, void **);

protected:
	int		m_nSeek;

#if VC8
	BOOL SetSize(int nSize) {return CArrayOld<BYTE, BYTE*>::SetSize(nSize);}
#else
	BOOL SetSize(int nSize) {return CArray<BYTE, BYTE*>::SetSize(nSize);}
#endif
};

#endif // #define _MEMSTM_H_
