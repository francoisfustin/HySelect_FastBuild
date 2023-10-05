//=--------------------------------------------------------------------------=
// ADVCON.H:	Interface for IConnectionPoint and IPropertyNotifySink helpers
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _ADVCON_H_
#define _ADVCON_H_

#include <arrtmpl.h>

//=--------------------------------------------------------------------------=
// Helpers 
//

HRESULT WINAPI AdviseConnect(LPUNKNOWN punkNotifier, REFIID riid, LPUNKNOWN punkNotify, DWORD* pdwCookie, BOOL fAddRef = TRUE);
HRESULT WINAPI AdviseDisconnect(LPUNKNOWN punkNotifier, REFIID riid, LPUNKNOWN punkNotify, DWORD dwCookie, BOOL fAddRef = TRUE);

/////////////////////////////////////////////////////////////////////////////
// CAdviseConnection
//
class CAdviseConnection
{
public:
	CAdviseConnection();
	CAdviseConnection(REFIID riid);
	virtual ~CAdviseConnection();

	// Operations
	//
	virtual HRESULT Advise(IUnknown *punkNotifier, IUnknown *punkNotify, BOOL fAddRef = TRUE);
	virtual HRESULT Unadvise(void);
	HRESULT SetNotifyInterface(REFIID riid);
	
	// Attributes
	//
	BOOL IsAdvising(void) const {return m_fAdvising;}
	REFIID GetNotifyInterface(void) const {return m_guid;}

	// Assignments
	//
	CAdviseConnection& operator=(const CAdviseConnection& other);

protected:
	GUID		m_guid;
	IUnknown	*m_punkNotifier;
	IUnknown	*m_punkNotify;
	DWORD		m_dwCookie;

	UINT		m_fAddRefNotify	:1;
	UINT		m_fAdvising		:1;

	IUnknown *GetNotifier(void) {return m_punkNotifier;}
	IUnknown *GetNotify(void) {return m_punkNotify;}
};

/////////////////////////////////////////////////////////////////////////////
// CConnectionsArray
//
#if VC8
class CConnectionsArray : public CArrayOld<CAdviseConnection, CAdviseConnection&>
#else
class CConnectionsArray : public CArray<CAdviseConnection, CAdviseConnection&>
#endif
{
public:
	CConnectionsArray()	{};
	~CConnectionsArray(){};

	HRESULT Add(REFIID riid, IUnknown *punkNotifier, IUnknown *punkNotify, BOOL fAddRef = TRUE);
	HRESULT Remove(REFIID riid);

	int Find(REFIID riid);
};

//=--------------------------------------------------------------------------=
// CPropNotifySink
//
class CPropNotifySink : public IPropertyNotifySink, protected CAdviseConnection
{
public:
	// Constructor requires Owner object and DISPID of of property object
	//
	CPropNotifySink(IPropertyNotifySink *pPropNotify, DISPID dispidPropNotify, BOOL fAddRefNotify = TRUE, BOOL fAddRefSource = TRUE);
	~CPropNotifySink();

public:
	// IUnknown methods
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHOD_(ULONG,AddRef)(THIS);
	STDMETHOD_(ULONG,Release)(THIS);

	// IPropertyNofitySink
	//
	STDMETHOD(OnChanged)(DISPID dispid);
	STDMETHOD(OnRequestEdit)(DISPID dispid);

public:
	// Attributes
	//
	BOOL IsAdvising(void) {return CAdviseConnection::IsAdvising();}

	// Call in OnChanged or OnRequestEdit to get DISPID of Object's
	// property DISPID that is changing
	//
	inline DISPID GetPropID(void) {return m_dispidProp;}

	// Operation
	//
	virtual HRESULT Advise(IUnknown *punkNotifier);
	virtual HRESULT Unadvise(void);

protected:
	// Implementation
	//
	IPropertyNotifySink	*m_pPropNotify;			// External IPropertyNotifySink object
	DISPID				m_dispidPropNotifier;	// DISPID of notifier object
	DISPID				m_dispidProp;			// DISPID of notifier object property
	BOOL				m_fAddRefNotify;		// TRUE to keep ref on notifyee
	BOOL				m_fAddRefNotifier;		// TRUE to keep ref on notifier
};

#endif // !defined (_ADVCON_H_)
