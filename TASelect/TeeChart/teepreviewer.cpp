// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "teepreviewer.h"

// Dispatch interfaces referenced by this interface
#include "TeePreviewerOptions.h"

/////////////////////////////////////////////////////////////////////////////
// CTeePreviewer

IMPLEMENT_DYNCREATE(CTeePreviewer, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CTeePreviewer properties

/////////////////////////////////////////////////////////////////////////////
// CTeePreviewer operations

void CTeePreviewer::SetChart(LPDISPATCH newValue)
{
	static BYTE parms[] =
		VTS_DISPATCH;
	InvokeHelper(0x2c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

void CTeePreviewer::SetChartLink(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

void CTeePreviewer::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CTeePreviewer::SetChartLinkx64(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xc9, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 &newValue);
}

CString CTeePreviewer::GetPreviewTitle()
{
	CString result;
	InvokeHelper(0x2, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

void CTeePreviewer::SetPreviewTitle(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x2, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 lpszNewValue);
}

void CTeePreviewer::ShowPreview()
{
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

unsigned long CTeePreviewer::GetPaperColor()
{
	unsigned long result;
	InvokeHelper(0x4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeePreviewer::SetPaperColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

long CTeePreviewer::GetPreviewerLink()
{
	long result;
	InvokeHelper(0x5, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

BOOL CTeePreviewer::GetMaximized()
{
	BOOL result;
	InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CTeePreviewer::SetMaximized(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x6, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CTeePreviewerOptions CTeePreviewer::GetOptions()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeePreviewerOptions(pDispatch);
}

VARIANT CTeePreviewer::GetPreviewerLinkx64()
{
	VARIANT result;
	InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, NULL);
	return result;
}
