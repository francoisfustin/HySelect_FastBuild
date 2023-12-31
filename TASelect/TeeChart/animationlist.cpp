// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "animationlist.h"

// Dispatch interfaces referenced by this interface
#include "chartanimations.h"


/////////////////////////////////////////////////////////////////////////////
// CAnimationList properties

/////////////////////////////////////////////////////////////////////////////
// CAnimationList operations

long CAnimationList::Add(long AnimationClass)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xc9, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		AnimationClass);
	return result;
}

CChartAnimations CAnimationList::GetItems(long Index)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, parms,
		Index);
	return CChartAnimations(pDispatch);
}

VARIANT CAnimationList::GetControlDefault(long Index)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xcb, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, parms,
		Index);
	return result;
}

void CAnimationList::Clear()
{
	InvokeHelper(0xcc, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CAnimationList::Delete(long Index)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xcd, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Index);
}

long CAnimationList::GetCount()
{
	long result;
	InvokeHelper(0xce, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAnimationList::SetActive(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xcf, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

void CAnimationList::Exchange(long Index1, long Index2)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0xd0, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Index1, Index2);
}
