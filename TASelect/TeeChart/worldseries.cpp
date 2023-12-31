// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "worldseries.h"

// Dispatch interfaces referenced by this interface
#include "valuelist.h"
#include "brush.h"
#include "pen.h"
#include "mappolygonlist.h"
#include "teeshadow.h"
#include "teeemboss.h"
#include "mapmarks.h"
#include "pointer.h"
#include "worldflags.h"


/////////////////////////////////////////////////////////////////////////////
// CWorldSeries properties

/////////////////////////////////////////////////////////////////////////////
// CWorldSeries operations

long CWorldSeries::AddXYZ(double AX, double AY, double AZ, LPCTSTR AXLabel, unsigned long Value)
{
	long result;
	static BYTE parms[] =
		VTS_R8 VTS_R8 VTS_R8 VTS_BSTR VTS_I4;
	InvokeHelper(0xe, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		AX, AY, AZ, AXLabel, Value);
	return result;
}

double CWorldSeries::MaxZValue()
{
	double result;
	InvokeHelper(0x1, DISPATCH_METHOD, VT_R8, (void*)&result, NULL);
	return result;
}

double CWorldSeries::MinZValue()
{
	double result;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_R8, (void*)&result, NULL);
	return result;
}

long CWorldSeries::GetTimesZOrder()
{
	long result;
	InvokeHelper(0x9, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetTimesZOrder(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x9, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CValueList CWorldSeries::GetZValues()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CValueList(pDispatch);
}

double CWorldSeries::GetZValue(long Index)
{
	double result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x5, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, parms,
		Index);
	return result;
}

void CWorldSeries::SetZValue(long Index, double newValue)
{
	static BYTE parms[] =
		VTS_I4 VTS_R8;
	InvokeHelper(0x5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 Index, newValue);
}

CBrush1 CWorldSeries::GetBrush()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x83, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

CPen1 CWorldSeries::GetPen()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x86, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

void CWorldSeries::AddArrayXYZ(const VARIANT& XArray, const VARIANT& YArray, const VARIANT& ZArray)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x87, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &XArray, &YArray, &ZArray);
}

void CWorldSeries::AddArrayGrid(long NumX, long NumZ, const VARIANT& XZArray)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_VARIANT;
	InvokeHelper(0xc9, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 NumX, NumZ, &XZArray);
}

long CWorldSeries::CalcZPos(long ValueIndex)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xca, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ValueIndex);
	return result;
}

void CWorldSeries::AddArrayXYZColor(const VARIANT& XArray, const VARIANT& YArray, const VARIANT& ZArray, const VARIANT& Colors)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0xcb, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &XArray, &YArray, &ZArray, &Colors);
}

long CWorldSeries::AddPalette(double Value, unsigned long Color)
{
	long result;
	static BYTE parms[] =
		VTS_R8 VTS_I4;
	InvokeHelper(0x11, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		Value, Color);
	return result;
}

unsigned long CWorldSeries::GetStartColor()
{
	unsigned long result;
	InvokeHelper(0xa, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetStartColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xa, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

unsigned long CWorldSeries::GetEndColor()
{
	unsigned long result;
	InvokeHelper(0x4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetEndColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

long CWorldSeries::GetPaletteSteps()
{
	long result;
	InvokeHelper(0x8, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetPaletteSteps(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CWorldSeries::GetUsePalette()
{
	BOOL result;
	InvokeHelper(0xc, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetUsePalette(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CWorldSeries::GetUseColorRange()
{
	BOOL result;
	InvokeHelper(0xb, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetUseColorRange(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xb, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

void CWorldSeries::ClearPalette()
{
	InvokeHelper(0x12, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CWorldSeries::CreateDefaultPalette(long NumSteps)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x13, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 NumSteps);
}

unsigned long CWorldSeries::GetSurfacePaletteColor(double Y)
{
	unsigned long result;
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x10, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		Y);
	return result;
}

unsigned long CWorldSeries::GetMidColor()
{
	unsigned long result;
	InvokeHelper(0x21, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetMidColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x21, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

void CWorldSeries::CreateRangePalette()
{
	InvokeHelper(0x22, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long CWorldSeries::GetPaletteStyle()
{
	long result;
	InvokeHelper(0x23, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetPaletteStyle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x23, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CWorldSeries::GetUsePaletteMin()
{
	BOOL result;
	InvokeHelper(0x12d, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetUsePaletteMin(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x12d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

double CWorldSeries::GetPaletteMin()
{
	double result;
	InvokeHelper(0x12e, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetPaletteMin(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x12e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

double CWorldSeries::GetPaletteStep()
{
	double result;
	InvokeHelper(0x12f, DISPATCH_PROPERTYGET, VT_R8, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetPaletteStep(double newValue)
{
	static BYTE parms[] =
		VTS_R8;
	InvokeHelper(0x12f, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

void CWorldSeries::InvertPalette()
{
	InvokeHelper(0x130, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CWorldSeries::AddCustomPalette(const VARIANT& colorArray)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x131, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &colorArray);
}

long CWorldSeries::GetLegendEvery()
{
	long result;
	InvokeHelper(0x132, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetLegendEvery(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x132, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CWorldSeries::CountLegendItems()
{
	long result;
	InvokeHelper(0x133, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::LoadPaletteFromFile(LPCTSTR FileName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x134, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName);
}

void CWorldSeries::SavePaletteToFile(LPCTSTR FileName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x135, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName);
}

CMapPolygonList CWorldSeries::GetShapes()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CMapPolygonList(pDispatch);
}

long CWorldSeries::GetPointSize()
{
	long result;
	InvokeHelper(0x191, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetPointSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x191, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CTeeShadow CWorldSeries::GetShadow()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x192, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeShadow(pDispatch);
}

BOOL CWorldSeries::GetTransparent()
{
	BOOL result;
	InvokeHelper(0x193, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetTransparent(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x193, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CWorldSeries::GetTransparency()
{
	long result;
	InvokeHelper(0x194, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetTransparency(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x194, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CTeeEmboss CWorldSeries::GetEmboss()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x195, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeEmboss(pDispatch);
}

CMapMarks CWorldSeries::GetMapMarks()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x196, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CMapMarks(pDispatch);
}

CPointer CWorldSeries::GetPointer()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x197, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPointer(pDispatch);
}

long CWorldSeries::GetRenderMode()
{
	long result;
	InvokeHelper(0x198, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetRenderMode(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x198, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

void CWorldSeries::LoadMapFromSHP(LPCTSTR map)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x199, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 map);
}

long CWorldSeries::GetMap()
{
	long result;
	InvokeHelper(0x1f5, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetMap(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1f5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CWorldFlags CWorldSeries::GetFlags()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1f6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CWorldFlags(pDispatch);
}

BOOL CWorldSeries::GetKeepAspect()
{
	BOOL result;
	InvokeHelper(0x1f7, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CWorldSeries::SetKeepAspect(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x1f7, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}
