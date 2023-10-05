//=--------------------------------------------------------------------------=
// PERSIST.H:	Persistence helpers
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _PERSIST_H_
#define _PERSIST_H_

#include <arrtmpl.h>

#define PERSIST_STREAM_ID	0xABCDEF01

struct PERSISTPROPENTRY
{
public:
	PERSISTPROPENTRY();
	~PERSISTPROPENTRY();

public:
	const WCHAR	*wszName;
	long		lOffset;
	long		cbSize;
	BOOL		fSaveEnabled;
	BOOL		fSaved;
	VARTYPE		vt;
	BYTE		vbDefValue[16];

	// Supports the following types
	//
	// VT_BOOL
	// VT_I2
	// VT_I4
	// VT_LPWSTR
	// VT_LPSTR
	// VT_BSTR
	// VT_R4
	// VT_R8
	// VT_UNKNOWN

public:
	HRESULT SetProp(const WCHAR* wszPropName, VARTYPE vtPropType, long lPropOffset, void *pDefValue);
	HRESULT IsDefault(const void* pBuffer, long cbBuffer);
	HRESULT GetValue(const void* pBuffer, long cbBuffer, VARIANT& var);
	HRESULT SetValue(void* pBuffer, long cbBuffer, const VARIANT& var);
};

class CPersistProperties : public CArray<PERSISTPROPENTRY, PERSISTPROPENTRY&>
{
public:
	CPersistProperties(DWORD dwVersion, DWORD dwStreamID = PERSIST_STREAM_ID);
	~CPersistProperties();

public:
	HRESULT Load(IPropertyBag *pPropertyBag, IErrorLog *pErrorLog, void *pBuffer, long cbBuffer);
	HRESULT Save(IPropertyBag *pPropertyBag, const void *pBuffer, long cbBuffer, BOOL fWriteDefaults = FALSE);
	HRESULT Load(IStream *pStream, void *pBuffer, long cbBuffer);
	HRESULT Save(IStream *pStream, const void *pBuffer, long cbBuffer);

	HRESULT AddProp(const WCHAR* wszName, VARTYPE vtType, long lOffset, void *pDefValue);
	HRESULT EnablePropSave(long lOffset, BOOL fEnable = TRUE);
	BOOL IsPropSaved(long lOffset);
	DWORD	GetVersionPersisted(void) {return m_dwVersionPersisted;}

protected:
	DWORD	m_dwStreamID;
	DWORD	m_dwVersion;
	DWORD	m_dwVersionPersisted;

	long FindProp(long lOffset, BOOL fAddIfNeeded = FALSE);
};

#endif _PERSIST_H_