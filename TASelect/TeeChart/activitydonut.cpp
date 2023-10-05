// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "activitydonut.h"

// Dispatch interfaces referenced by this interface
#include "teepoint2d.h"
#include "teeshadow.h"
#include "brush.h"
#include "valuelist.h"
#include "pieotherslice.h"
#include "explodedslices.h"
#include "pen.h"
#include "gradient.h"
#include "piemarks.h"
#include "pieangle.h"


/////////////////////////////////////////////////////////////////////////////
// CActivityDonut properties

/////////////////////////////////////////////////////////////////////////////
// CActivityDonut operations

long CActivityDonut::GetXRadius()
{
	long result;
	InvokeHelper(0x1, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetXRadius(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CActivityDonut::GetYRadius()
{
	long result;
	InvokeHelper(0x2, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetYRadius(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x2, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CActivityDonut::GetXCenter()
{
	long result;
	InvokeHelper(0x4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CActivityDonut::GetYCenter()
{
	long result;
	InvokeHelper(0x5, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CActivityDonut::GetCircleWidth()
{
	long result;
	InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CActivityDonut::GetCircleHeight()
{
	long result;
	InvokeHelper(0x7, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

unsigned long CActivityDonut::GetCircleBackColor()
{
	unsigned long result;
	InvokeHelper(0x8, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetCircleBackColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

BOOL CActivityDonut::GetCircled()
{
	BOOL result;
	InvokeHelper(0x9, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetCircled(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x9, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CActivityDonut::GetRotationAngle()
{
	long result;
	InvokeHelper(0xa, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetRotationAngle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xa, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CTeePoint2D CActivityDonut::AngleToPoint(double Angle, double AXRadius, double AYRadius)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_R8 VTS_R8 VTS_R8;
	InvokeHelper(0xb, DISPATCH_METHOD, VT_DISPATCH, (void*)&pDispatch, parms,
		Angle, AXRadius, AYRadius);
	return CTeePoint2D(pDispatch);
}

double CActivityDonut::PointToAngle(long XCoord, long YCoord)
{
	double result;
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0xc, DISPATCH_METHOD, VT_R8, (void*)&result, parms,
		XCoord, YCoord);
	return result;
}

double CActivityDonut::PointToRadius(long XCoord, long YCoord)
{
	double result;
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0x191, DISPATCH_METHOD, VT_R8, (void*)&result, parms,
		XCoord, YCoord);
	return result;
}

double CActivityDonut::GetCustomXRadius()
{
	double result;
	InvokeHelper(0x1f5, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetCustomXRadius(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x1f5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

double CActivityDonut::GetCustomYRadius()
{
	double result;
	InvokeHelper(0x259, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetCustomYRadius(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x259, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

CTeeShadow CActivityDonut::GetShadow()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x2bd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeShadow(pDispatch);
}

CBrush1 CActivityDonut::GetCircleBrush()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x321, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

BOOL CActivityDonut::GetUsePatterns()
{
	BOOL result;
	InvokeHelper(0x16, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetUsePatterns(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x16, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CValueList CActivityDonut::GetPieValues()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x17, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CValueList(pDispatch);
}

BOOL CActivityDonut::GetDark3D()
{
	BOOL result;
	InvokeHelper(0x18, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetDark3D(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x18, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CActivityDonut::GetExplodeBiggest()
{
	long result;
	InvokeHelper(0x19, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetExplodeBiggest(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x19, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CPieOtherSlice CActivityDonut::GetOtherSlice()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1a, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPieOtherSlice(pDispatch);
}

CExplodedSlices CActivityDonut::GetExplodedSlice()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CExplodedSlices(pDispatch);
}

CPen1 CActivityDonut::GetPiePen()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1c, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

long CActivityDonut::GetAngleSize()
{
	long result;
	InvokeHelper(0xd, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetAngleSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xd, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CActivityDonut::GetAutoMarkPosition()
{
	BOOL result;
	InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetAutoMarkPosition(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x12d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CGradient CActivityDonut::GetGradient()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x12e, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGradient(pDispatch);
}

long CActivityDonut::GetSliceHeight(long Index)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x12f, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, parms,
		Index);
	return result;
}

void CActivityDonut::SetSliceHeight(long Index, long nNewValue)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0x12f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 Index, nNewValue);
}

long CActivityDonut::GetDarkPen()
{
	long result;
	InvokeHelper(0x130, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetDarkPen(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x130, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CActivityDonut::GetMultiPie()
{
	long result;
	InvokeHelper(0x131, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetMultiPie(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x131, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CPieMarks CActivityDonut::GetPieMarks()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x132, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPieMarks(pDispatch);
}

long CActivityDonut::GetGradientBright()
{
	long result;
	InvokeHelper(0x133, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetGradientBright(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x133, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CActivityDonut::GetEdgeStyle()
{
	long result;
	InvokeHelper(0x134, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetEdgeStyle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x134, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CActivityDonut::GetBevelPercent()
{
	long result;
	InvokeHelper(0x135, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetBevelPercent(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x135, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CActivityDonut::GetTransparency()
{
	long result;
	InvokeHelper(0x136, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetTransparency(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x136, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CPieAngle CActivityDonut::GetAngles(long Index)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x137, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, parms,
		Index);
	return CPieAngle(pDispatch);
}

BOOL CActivityDonut::GetUniqueCustomRadius()
{
	BOOL result;
	InvokeHelper(0x138, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetUniqueCustomRadius(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x138, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CActivityDonut::GetDonutPercent()
{
	long result;
	InvokeHelper(0x6f, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetDonutPercent(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x6f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

double CActivityDonut::GetValue()
{
	double result;
	InvokeHelper(0x1f6, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetValue(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x1f6, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

unsigned long CActivityDonut::GetColor()
{
	unsigned long result;
	InvokeHelper(0x1f7, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1f7, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

unsigned long CActivityDonut::GetBackColor()
{
	unsigned long result;
	InvokeHelper(0x1f8, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CActivityDonut::SetBackColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1f8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}
