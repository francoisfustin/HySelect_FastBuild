// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "seriesgroup.h"

// Dispatch interfaces referenced by this interface
#include "customserieslist.h"


/////////////////////////////////////////////////////////////////////////////
// CSeriesGroup properties

/////////////////////////////////////////////////////////////////////////////
// CSeriesGroup operations

void CSeriesGroup::Add(long SeriesIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xc9, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 SeriesIndex);
}

void CSeriesGroup::Hide()
{
	InvokeHelper(0xca, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CSeriesGroup::Show()
{
	InvokeHelper(0xcb, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long CSeriesGroup::GetActive()
{
	long result;
	InvokeHelper(0xcc, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSeriesGroup::SetActive(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xcc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CString CSeriesGroup::GetName()
{
	CString result;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

void CSeriesGroup::SetName(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0xcd, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 lpszNewValue);
}

VARIANT CSeriesGroup::GetSeriesCount()
{
	VARIANT result;
	InvokeHelper(0xce, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, NULL);
	return result;
}

long CSeriesGroup::GetGroupLink()
{
	long result;
	InvokeHelper(0xcf, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSeriesGroup::Delete(long Index)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd0, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Index);
}

void CSeriesGroup::Insert(long IndexInGroup, long SeriesIndex)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0xd1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 IndexInGroup, SeriesIndex);
}

CCustomSeriesList CSeriesGroup::GetSeries()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CCustomSeriesList(pDispatch);
}
