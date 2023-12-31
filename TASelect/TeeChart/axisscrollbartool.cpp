// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "axisscrollbartool.h"

// Dispatch interfaces referenced by this interface
#include "brush.h"
#include "gradient.h"
#include "scrollbarthumb.h"


/////////////////////////////////////////////////////////////////////////////
// CAxisScrollBarTool properties

/////////////////////////////////////////////////////////////////////////////
// CAxisScrollBarTool operations

BOOL CAxisScrollBarTool::GetAllowResize()
{
	BOOL result;
	InvokeHelper(0xc9, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetAllowResize(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xc9, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CBrush1 CAxisScrollBarTool::GetArrowBrush()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

BOOL CAxisScrollBarTool::GetArrows()
{
	BOOL result;
	InvokeHelper(0xcb, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetArrows(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xcb, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CAxisScrollBarTool::GetAutoRepeat()
{
	BOOL result;
	InvokeHelper(0xcc, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetAutoRepeat(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xcc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

unsigned long CAxisScrollBarTool::GetBackColor()
{
	unsigned long result;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetBackColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xcd, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

long CAxisScrollBarTool::GetBevel()
{
	long result;
	InvokeHelper(0xce, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetBevel(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xce, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CBrush1 CAxisScrollBarTool::GetDisabledArrow()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcf, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

long CAxisScrollBarTool::GetDrawStyle()
{
	long result;
	InvokeHelper(0xd0, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetDrawStyle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd0, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CGradient CAxisScrollBarTool::GetGradient()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGradient(pDispatch);
}

long CAxisScrollBarTool::GetInitialDelay()
{
	long result;
	InvokeHelper(0xd2, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetInitialDelay(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd2, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CAxisScrollBarTool::GetMinThumbSize()
{
	long result;
	InvokeHelper(0xd3, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetMinThumbSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd3, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CAxisScrollBarTool::GetRoundSize()
{
	long result;
	InvokeHelper(0xd4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetRoundSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CAxisScrollBarTool::GetSize()
{
	long result;
	InvokeHelper(0xd5, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CScrollBarThumb CAxisScrollBarTool::GetThumb()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CScrollBarThumb(pDispatch);
}

CBrush1 CAxisScrollBarTool::GetThumbBrush()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

long CAxisScrollBarTool::GetThumbSize()
{
	long result;
	InvokeHelper(0xd8, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetThumbSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CAxisScrollBarTool::GetMargin()
{
	long result;
	InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CAxisScrollBarTool::SetMargin(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x12d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}
