// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "legenditem.h"

// Dispatch interfaces referenced by this interface
#include "teefont.h"


/////////////////////////////////////////////////////////////////////////////
// CLegendItem properties

/////////////////////////////////////////////////////////////////////////////
// CLegendItem operations

long CLegendItem::GetAlign()
{
	long result;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CLegendItem::GetLeft()
{
	long result;
	InvokeHelper(0xcb, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

CString CLegendItem::GetText()
{
	CString result;
	InvokeHelper(0xcc, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

void CLegendItem::SetText(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0xcc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 lpszNewValue);
}

long CLegendItem::GetTop()
{
	long result;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

CTeeFont CLegendItem::GetFont()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xce, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeFont(pDispatch);
}
