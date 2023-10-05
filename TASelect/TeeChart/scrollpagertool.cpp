// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "scrollpagertool.h"

// Dispatch interfaces referenced by this interface
#include "colorbandtool.h"
#include "pointer.h"
#include "subchartchart.h"
#include "subcharttool.h"


/////////////////////////////////////////////////////////////////////////////
// CScrollPagerTool properties

/////////////////////////////////////////////////////////////////////////////
// CScrollPagerTool operations

VARIANT CScrollPagerTool::GetSeries()
{
	VARIANT result;
	InvokeHelper(0xe, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, NULL);
	return result;
}

void CScrollPagerTool::SetSeries(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xe, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 &newValue);
}

long CScrollPagerTool::GetAlign()
{
	long result;
	InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CScrollPagerTool::SetAlign(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x12d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CColorBandTool CScrollPagerTool::GetColorBandTool()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x12e, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CColorBandTool(pDispatch);
}

double CScrollPagerTool::GetDivisionRatio()
{
	double result;
	InvokeHelper(0x12f, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CScrollPagerTool::SetDivisionRatio(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x12f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

CPointer CScrollPagerTool::GetEndLinePointer()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x130, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPointer(pDispatch);
}

BOOL CScrollPagerTool::GetNoLimitDrag()
{
	BOOL result;
	InvokeHelper(0x131, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CScrollPagerTool::SetNoLimitDrag(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x131, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

unsigned long CScrollPagerTool::GetPointerHighlightColor()
{
	unsigned long result;
	InvokeHelper(0x132, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CScrollPagerTool::SetPointerHighlightColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x132, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

CPointer CScrollPagerTool::GetStartLinePointer()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x133, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPointer(pDispatch);
}

CSubChartChart CScrollPagerTool::GetSubChartTChart()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x134, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSubChartChart(pDispatch);
}

CSubChartTool CScrollPagerTool::GetSubChartTool()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x135, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSubChartTool(pDispatch);
}

void CScrollPagerTool::AddSeries(long AIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x136, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 AIndex);
}

void CScrollPagerTool::RemoveSeries(long AIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x137, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 AIndex);
}
