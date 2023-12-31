// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "horizareaseries.h"

// Dispatch interfaces referenced by this interface
#include "seriespointeritems.h"
#include "pen.h"
#include "brush.h"
#include "teeshadow.h"
#include "gradient.h"


/////////////////////////////////////////////////////////////////////////////
// CHorizAreaSeries properties

/////////////////////////////////////////////////////////////////////////////
// CHorizAreaSeries operations

CSeriesPointerItems CHorizAreaSeries::GetPointer()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x641, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSeriesPointerItems(pDispatch);
}

BOOL CHorizAreaSeries::GetStairs()
{
	BOOL result;
	InvokeHelper(0x642, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetStairs(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x642, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CHorizAreaSeries::GetInvertedStairs()
{
	BOOL result;
	InvokeHelper(0x643, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetInvertedStairs(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x643, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CPen1 CHorizAreaSeries::GetLinePen()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x644, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

long CHorizAreaSeries::GetLineBrush()
{
	long result;
	InvokeHelper(0x645, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetLineBrush(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x645, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CHorizAreaSeries::GetClickableLine()
{
	BOOL result;
	InvokeHelper(0x646, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetClickableLine(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x646, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CHorizAreaSeries::GetLineHeight()
{
	long result;
	InvokeHelper(0x647, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetLineHeight(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x647, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CHorizAreaSeries::GetDark3D()
{
	BOOL result;
	InvokeHelper(0x648, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetDark3D(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x648, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CBrush1 CHorizAreaSeries::GetBrush()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x649, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

BOOL CHorizAreaSeries::GetColorEachLine()
{
	BOOL result;
	InvokeHelper(0x64a, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetColorEachLine(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x64a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CPen1 CHorizAreaSeries::GetOutline()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x64b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

long CHorizAreaSeries::GetTransparency()
{
	long result;
	InvokeHelper(0x64c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetTransparency(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x64c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CTeeShadow CHorizAreaSeries::GetShadow()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x64d, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeShadow(pDispatch);
}

long CHorizAreaSeries::GetTreatNulls()
{
	long result;
	InvokeHelper(0x64e, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetTreatNulls(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x64e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CHorizAreaSeries::GetSmoothed()
{
	BOOL result;
	InvokeHelper(0x64f, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetSmoothed(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x64f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CHorizAreaSeries::GetClickTolerance()
{
	long result;
	InvokeHelper(0x650, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetClickTolerance(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x650, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CHorizAreaSeries::GetDrawStyle()
{
	long result;
	InvokeHelper(0x651, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetDrawStyle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x651, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CHorizAreaSeries::GetInflateMargins()
{
	BOOL result;
	InvokeHelper(0xc9, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetInflateMargins(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xc9, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CHorizAreaSeries::GetPointerBehind()
{
	BOOL result;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetPointerBehind(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xca, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CHorizAreaSeries::GetAreaBrush()
{
	long result;
	InvokeHelper(0x14, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetAreaBrush(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x14, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CPen1 CHorizAreaSeries::GetAreaPen()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x15, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

long CHorizAreaSeries::GetMultiArea()
{
	long result;
	InvokeHelper(0x16, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetMultiArea(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x16, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CHorizAreaSeries::GetOriginPos(long AValueIndex)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x17, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		AValueIndex);
	return result;
}

BOOL CHorizAreaSeries::GetUseYOrigin()
{
	BOOL result;
	InvokeHelper(0x8, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetUseYOrigin(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

double CHorizAreaSeries::GetYOrigin()
{
	double result;
	InvokeHelper(0xa, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetYOrigin(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0xa, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

CGradient CHorizAreaSeries::GetGradient()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xc, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGradient(pDispatch);
}

unsigned long CHorizAreaSeries::GetAreaColor()
{
	unsigned long result;
	InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetAreaColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x12d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

CBrush1 CHorizAreaSeries::GetAreaChartBrush()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x12e, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

long CHorizAreaSeries::GetStackGroup()
{
	long result;
	InvokeHelper(0x12f, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CHorizAreaSeries::SetStackGroup(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x12f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}
