#ifndef _PROPERTYCATEGORIES
#define _PROPERTYCATEGORIES

#if !defined( INITGUID )
#include <olectl.h>
#endif

typedef int PROPCAT;

#define VB_PROPCAT_DATA			-7
#define VB_PROPCAT_BEHAVIOR		-6	
#define VB_PROPCAT_APPEARANCE   -5
#define VB_PROPCAT_POSITION		-4
#define VB_PROPCAT_FONT			-3
#define VB_PROPCAT_MISC			-2	// default category
#define VB_PROPCAT_NIL			-1

const IID IID_ICategorizeProperties=
{0x4D07FC10,0xF931,0x11CE,{0xB0,0x01,0x00,0xAA,0x00,0x68,0x84,0xE5}};

//////////////////////////////////////////////////////////////////////////////
//
//  ICategorizeProperties interface
//
//////////////////////////////////////////////////////////////////////////////

#undef  INTERFACE
#define INTERFACE ICategorizeProperties

DECLARE_INTERFACE_(ICategorizeProperties, IUnknown)
{
	//
	//  IUnknown methods
	//
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	//
	//  ICategorizeProperties methods
	//
	STDMETHOD(PropertyToCategory)(THIS_ DISPID dispid, PROPCAT *ppropcat) PURE;
	STDMETHOD(GetCategoryName)(THIS_ PROPCAT propcat, LCID lcid, BSTR *pbstrName) PURE;
};

#endif // !defined(PROPERTY_CATEGORIES)
