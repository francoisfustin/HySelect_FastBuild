/***********************************************************************
* FPDATOBJ.H - header for CfpDataObjectFiles, CfpDataObject,
*			   and CfpDataObjectFilesEnum classes
*
* Copyright (C) 1995 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#ifndef __FP_DATAOBJECT_H
#define __FP_DATAOBJECT_H

#include "stdafx.h"

#ifndef NO_MFC
// {69310C25-4993-11d1-8905-0020AF131A57}
const GUID CDECL BASED_CODE IID_IfpDataObjectFiles = 
{ 0x69310c25, 0x4993, 0x11d1, { 0x89, 0x5, 0x0, 0x20, 0xaf, 0x13, 0x1a, 0x57 } };

// {69310C27-4993-11d1-8905-0020AF131A57}
const GUID CDECL BASED_CODE IID_IfpDataObject = 
{ 0x69310c27, 0x4993, 0x11d1, { 0x89, 0x5, 0x0, 0x20, 0xaf, 0x13, 0x1a, 0x57 } };

// forward declarations
class IfpDataObject;
class IfpDataObjectFiles;

class IfpDataObject : public IDispatch
{	// same interface as IVBDataObject
public:
	STDMETHOD(Clear)() = 0;
	STDMETHOD(GetData)(short nFormat, VARIANT* pvData) = 0;
	STDMETHOD(GetFormat)(short nFormat, BOOL* pfFormatSupported) = 0;
	STDMETHOD(SetData)(VARIANT vData, VARIANT vFormat) = 0;
	STDMETHOD(Files)(IfpDataObjectFiles** ppFiles) = 0;
};

class IfpDataObjectFiles : public IDispatch
{	// same interface as IVBDataObjectFiles
public:
	STDMETHOD(Item)(long lIndex, BSTR FAR* bstrItem) = 0;
	STDMETHOD(Count)(long FAR* lCount) = 0;
	STDMETHOD(Add)(BSTR bstrFileName, VARIANT vIndex) = 0;
	STDMETHOD(Clear)() = 0;
	STDMETHOD(Remove)(VARIANT vIndex) = 0;
	STDMETHOD(_NewEnum)(IUnknown** ppUnk) = 0;
};

#endif

// forward declarations
class CfpDataObject;
class CfpDataObjectFiles;
class CfpDataObjectFilesEnum;

class CfpDataObject : public IfpDataObject
{
private:
	ITypeInfo*			m_pTypeInfo;	// ptr to typeinfo for this object
	long				m_lRefCount;
	LPDATAOBJECT		m_pData;
	CfpDataObjectFiles*	m_pFiles;

	HRESULT LoadTypeInfo();		// loads the typeinfo for this object
public:
	BOOL  m_fFireOLESetData[8];
	BOOL  m_fFormatSet;

	CfpDataObject();
	CfpDataObject(LPDATAOBJECT pDataObject);
	~CfpDataObject();

	virtual LPDATAOBJECT GetIDataObject(BOOL fAddRef)
	{
		if (m_pData && fAddRef)
			m_pData->AddRef();
		return m_pData; 
	}
	virtual void SetIDataObject(LPDATAOBJECT pDataObject)
	{	
		if (m_pData == pDataObject)
			return;
		if (m_pData) 
			m_pData->Release(); 
		m_pData = pDataObject;
		if( m_pData )
			m_pData->AddRef();
	}
	
    STDMETHOD(QueryInterface)(REFIID iid, void** ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();
	STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId); 
	STDMETHOD(GetTypeInfo)(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo); 
	STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo); 
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr); 
	STDMETHOD(Clear)();
	STDMETHOD(GetData)(short nFormat, VARIANT* pvData);
#ifdef NO_MFC
	STDMETHOD(GetFormat)(short nFormat, VARIANT_BOOL* pfFormatSupported);
#else
	STDMETHOD(GetFormat)(short nFormat, BOOL* pfFormatSupported);
#endif
	STDMETHOD(SetData)(VARIANT vData, VARIANT vFormat);
#ifdef NO_MFC
	STDMETHOD(get_Files)(IfpDataObjectFiles** ppFiles);
#else
	STDMETHOD(Files)(IfpDataObjectFiles** ppFiles);
#endif
};

class CfpDataObjectFiles : public IfpDataObjectFiles
{
private:
	ITypeInfo*		m_pTypeInfo;	// ptr to typeinfo for this object
	long			m_lRefCount;	// ref count
	CfpDataObject*  m_pfpDataObject;// ptr to fpDataObject
	LPDATAOBJECT	m_pData;		// ptr to IDataObject
	STGMEDIUM		medium;			// stgmedium to retrieve HDROP into
	static FORMATETC format;		// format for calls to IDataObject::GetData
									// (doesn't change)
	HRESULT LoadTypeInfo();			// loads the typeinfo for this object
protected:
	HRESULT GetHDrop(HDROP* phDrop);// fctn to retrieve HDROP form m_pData
	HRESULT SetHDrop(HDROP hDrop);	// fctn to put HDROP into m_pData
public:
	CfpDataObjectFiles(CfpDataObject* pfpDataOject, LPDATAOBJECT pDataObject);
	~CfpDataObjectFiles();
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();
	STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId); 
	STDMETHOD(GetTypeInfo)(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo); 
	STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo); 
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr); 
#ifdef NO_MFC
	STDMETHOD(get_Item)(long lIndex, BSTR FAR* pbstrItem);
	STDMETHOD(get_Count)(long* plCount);
#else
	STDMETHOD(Item)(long lIndex, BSTR FAR* pbstrItem);
	STDMETHOD(Count)(long* plCount);
#endif
	STDMETHOD(Add)(BSTR bstrFileName, VARIANT vIndex);
	STDMETHOD(Clear)();
	STDMETHOD(Remove)(VARIANT vIndex);
#ifdef NO_MFC
	STDMETHOD(get__NewEnum)(IUnknown** ppUnk);
#else
	STDMETHOD(_NewEnum)(IUnknown** ppUnk);
#endif
};

class CfpDataObjectFilesEnum : public IEnumVARIANT
{
private:
	long				m_lRefCount;	// ref count
	HDROP				m_hDrop;		// handle to global DROPFILES struct
	long				m_lCount;		// count of filenames
public:
	long	m_lCurrent;		// index of next item to enumerate
	CfpDataObjectFilesEnum(HDROP hDrop);
	~CfpDataObjectFilesEnum();
	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

    STDMETHOD(Next)(ULONG celt, VARIANT *rgVar, ULONG *pCeltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumVARIANT **ppEnum);

//  Old IEnumString interface
//  STDMETHOD(Next)(unsigned long celt, LPOLESTR* rgelt, unsigned long FAR* pceltFetched);
//  STDMETHOD(Skip)(unsigned long celt);
//  STDMETHOD(Reset)();
//  STDMETHOD(Clone)(IEnumString** ppenum);
};

#endif // __FP_DATAOBJECT_H

