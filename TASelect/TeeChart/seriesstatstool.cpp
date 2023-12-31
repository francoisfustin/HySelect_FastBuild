// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "seriesstatstool.h"

// Dispatch interfaces referenced by this interface
#include "strings.h"


/////////////////////////////////////////////////////////////////////////////
// CSeriesStatsTool properties

/////////////////////////////////////////////////////////////////////////////
// CSeriesStatsTool operations

VARIANT CSeriesStatsTool::GetSeries()
{
	VARIANT result;
	InvokeHelper(0xe, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, NULL);
	return result;
}

void CSeriesStatsTool::SetSeries(const VARIANT& newValue)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xe, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 &newValue);
}

CStrings CSeriesStatsTool::GetStatistics()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CStrings(pDispatch);
}

void CSeriesStatsTool::FreeStatsSeries()
{
	InvokeHelper(0x12e, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

double CSeriesStatsTool::Kurtosis()
{
	double result;
	InvokeHelper(0x12f, DISPATCH_METHOD, VT_R8, (void*)&result, NULL);
	return result;
}

double CSeriesStatsTool::Mean()
{
	double result;
	InvokeHelper(0x130, DISPATCH_METHOD, VT_R8, (void*)&result, NULL);
	return result;
}

double CSeriesStatsTool::Skewness()
{
	double result;
	InvokeHelper(0x131, DISPATCH_METHOD, VT_R8, (void*)&result, NULL);
	return result;
}

double CSeriesStatsTool::StdDeviation()
{
	double result;
	InvokeHelper(0x132, DISPATCH_METHOD, VT_R8, (void*)&result, NULL);
	return result;
}
