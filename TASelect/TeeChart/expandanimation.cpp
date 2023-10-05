// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "expandanimation.h"

// Dispatch interfaces referenced by this interface
#include "animations.h"
#include "teeanimate.h"


/////////////////////////////////////////////////////////////////////////////
// CExpandAnimation properties

/////////////////////////////////////////////////////////////////////////////
// CExpandAnimation operations

long CExpandAnimation::GetSpeed()
{
	long result;
	InvokeHelper(0x258, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CExpandAnimation::SetSpeed(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x258, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CAnimations CExpandAnimation::GetAnimations()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x259, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CAnimations(pDispatch);
}

CTeeAnimate CExpandAnimation::GetAnimate()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x25a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeAnimate(pDispatch);
}

BOOL CExpandAnimation::GetLoop()
{
	BOOL result;
	InvokeHelper(0x25b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CExpandAnimation::SetLoop(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x25b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

void CExpandAnimation::Play()
{
	InvokeHelper(0x25c, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long CExpandAnimation::GetSizeBy()
{
	long result;
	InvokeHelper(0x1a4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CExpandAnimation::SetSizeBy(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1a4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CExpandAnimation::GetTarget()
{
	long result;
	InvokeHelper(0x1a5, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CExpandAnimation::SetTarget(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1a5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CExpandAnimation::GetTrigger()
{
	long result;
	InvokeHelper(0x1a6, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CExpandAnimation::SetTrigger(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1a6, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}
