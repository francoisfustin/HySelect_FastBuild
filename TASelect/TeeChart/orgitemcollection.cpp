// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "orgitemcollection.h"

// Dispatch interfaces referenced by this interface
#include "orgitem.h"


/////////////////////////////////////////////////////////////////////////////
// COrgItemCollection properties

/////////////////////////////////////////////////////////////////////////////
// COrgItemCollection operations

COrgItem COrgItemCollection::GetItems(long Index)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xc9, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, parms,
		Index);
	return COrgItem(pDispatch);
}

long COrgItemCollection::GetCount()
{
	long result;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}
