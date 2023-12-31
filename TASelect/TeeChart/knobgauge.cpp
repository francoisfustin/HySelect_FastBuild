// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "knobgauge.h"

// Dispatch interfaces referenced by this interface
#include "teeshape.h"
#include "teerect.h"
#include "framedborder.h"
#include "gaugepointerrange.h"
#include "gaugeseriespointer.h"
#include "gaugeranges.h"
#include "pointer.h"
#include "lineargauge.h"
#include "numericgauge.h"
#include "axis.h"
#include "gaugehand.h"
#include "gaugehands.h"
#include "teepoint2d.h"
#include "gradient.h"


/////////////////////////////////////////////////////////////////////////////
// CKnobGauge properties

/////////////////////////////////////////////////////////////////////////////
// CKnobGauge operations

CTeeShape CKnobGauge::GetFace()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x60020000, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeShape(pDispatch);
}

double CKnobGauge::GetValue()
{
	double result;
	InvokeHelper(0xcb, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetValue(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0xcb, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

CTeeRect CKnobGauge::GetCustomBounds()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xc9, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeRect(pDispatch);
}

unsigned long CKnobGauge::GetPaletteColor(long Index)
{
	unsigned long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xca, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		Index);
	return result;
}

void CKnobGauge::SetCustomBounds(long ALeft, long ATop, long ARight, long ABottom)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0xcc, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 ALeft, ATop, ARight, ABottom);
}

CFramedBorder CKnobGauge::GetFrame()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CFramedBorder(pDispatch);
}

CGaugePointerRange CKnobGauge::GetGreenLine()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugePointerRange(pDispatch);
}

BOOL CKnobGauge::GetHorizontal()
{
	BOOL result;
	InvokeHelper(0x12f, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetHorizontal(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x12f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

double CKnobGauge::GetMaximum()
{
	double result;
	InvokeHelper(0x130, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetMaximum(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x130, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

double CKnobGauge::GetMinimum()
{
	double result;
	InvokeHelper(0x131, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetMinimum(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x131, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

long CKnobGauge::GetMinorTickDistance()
{
	long result;
	InvokeHelper(0x132, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetMinorTickDistance(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x132, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CGaugeSeriesPointer CKnobGauge::GetMinorTicks()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x133, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugeSeriesPointer(pDispatch);
}

CGaugePointerRange CKnobGauge::GetRedLine()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x134, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugePointerRange(pDispatch);
}

CGaugeSeriesPointer CKnobGauge::GetTicks()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x135, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugeSeriesPointer(pDispatch);
}

CGaugeRanges CKnobGauge::GetAxisRanges()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x12e, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugeRanges(pDispatch);
}

CGaugeRanges CKnobGauge::GetRanges()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x136, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugeRanges(pDispatch);
}

CGaugeSeriesPointer CKnobGauge::GetCenter()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x191, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugeSeriesPointer(pDispatch);
}

BOOL CKnobGauge::GetCircled()
{
	BOOL result;
	InvokeHelper(0x192, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetCircled(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x192, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CPointer CKnobGauge::GetEndPoint()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x193, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPointer(pDispatch);
}

BOOL CKnobGauge::GetLabelsInside()
{
	BOOL result;
	InvokeHelper(0x195, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetLabelsInside(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x195, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CKnobGauge::GetRotateLabels()
{
	BOOL result;
	InvokeHelper(0x196, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetRotateLabels(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x196, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

double CKnobGauge::GetRotationAngle()
{
	double result;
	InvokeHelper(0x197, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetRotationAngle(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x197, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

double CKnobGauge::GetTotalAngle()
{
	double result;
	InvokeHelper(0x198, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetTotalAngle(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x198, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

BOOL CKnobGauge::GetAutoPositionLinearGauge()
{
	BOOL result;
	InvokeHelper(0x199, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetAutoPositionLinearGauge(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x199, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CKnobGauge::GetAutoValueLinearGauge()
{
	BOOL result;
	InvokeHelper(0x19a, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetAutoValueLinearGauge(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x19a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CKnobGauge::GetAutoPositionNumericGauge()
{
	BOOL result;
	InvokeHelper(0x19b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetAutoPositionNumericGauge(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x19b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CKnobGauge::GetAutoValueNumericGauge()
{
	BOOL result;
	InvokeHelper(0x19c, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetAutoValueNumericGauge(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x19c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CKnobGauge::GetDisplayTotalAngle()
{
	long result;
	InvokeHelper(0x19d, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetDisplayTotalAngle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x19d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CKnobGauge::GetDisplayRotationAngle()
{
	long result;
	InvokeHelper(0x19e, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetDisplayRotationAngle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x19e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CKnobGauge::GetFillToSemiCircle()
{
	BOOL result;
	InvokeHelper(0x19f, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetFillToSemiCircle(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x19f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CKnobGauge::GetHandOffset()
{
	long result;
	InvokeHelper(0x1a1, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetHandOffset(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1a1, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CLinearGauge CKnobGauge::GetLinearGauge()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1a2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CLinearGauge(pDispatch);
}

CNumericGauge CKnobGauge::GetNumericGauge()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1a3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CNumericGauge(pDispatch);
}

CAxis CKnobGauge::GetAxis()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1a4, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CAxis(pDispatch);
}

BOOL CKnobGauge::GetUseAxis()
{
	BOOL result;
	InvokeHelper(0x1a5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

long CKnobGauge::GetCircleXCenter()
{
	long result;
	InvokeHelper(0x1a6, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CKnobGauge::GetCircleYCenter()
{
	long result;
	InvokeHelper(0x1a7, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CKnobGauge::GetXRadius()
{
	long result;
	InvokeHelper(0x1a8, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CKnobGauge::GetYRadius()
{
	long result;
	InvokeHelper(0x1a9, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

CFramedBorder CKnobGauge::GetCircularFrame()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1a0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CFramedBorder(pDispatch);
}

CGaugeHand CKnobGauge::GetHand()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x194, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugeHand(pDispatch);
}

CGaugeHands CKnobGauge::GetHands()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1aa, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGaugeHands(pDispatch);
}

CTeePoint2D CKnobGauge::GetCenterPoint()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1f5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeePoint2D(pDispatch);
}

CGradient CKnobGauge::GetHighLight()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1f6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGradient(pDispatch);
}

BOOL CKnobGauge::GetActiveCenter()
{
	BOOL result;
	InvokeHelper(0x1f7, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetActiveCenter(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x1f7, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CKnobGauge::GetAxisInside()
{
	BOOL result;
	InvokeHelper(0x1f8, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CKnobGauge::SetAxisInside(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x1f8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}
