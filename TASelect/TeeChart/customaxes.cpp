// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "customaxes.h"

// Dispatch interfaces referenced by this interface
#include "axis.h"


/////////////////////////////////////////////////////////////////////////////
// CCustomAxes properties

/////////////////////////////////////////////////////////////////////////////
// CCustomAxes operations

long CCustomAxes::Add()
{
	long result;
	InvokeHelper(0xc9, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

CAxis CCustomAxes::GetItems(long AIndex)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, parms,
		AIndex);
	return CAxis(pDispatch);
}

void CCustomAxes::ResetScales(long AIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xcb, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 AIndex);
}
