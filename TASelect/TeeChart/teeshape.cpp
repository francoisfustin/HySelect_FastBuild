// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "teeshape.h"

// Dispatch interfaces referenced by this interface
#include "teefont.h"
#include "gradient.h"
#include "teerect.h"
#include "teeshadow.h"
#include "pen.h"
#include "backimage.h"
#include "teeemboss.h"
#include "brush.h"
#include "shapecallout.h"
#include "teecorners.h"
#include "teeshapes.h"


/////////////////////////////////////////////////////////////////////////////
// CTeeShape properties

/////////////////////////////////////////////////////////////////////////////
// CTeeShape operations

unsigned long CTeeShape::GetColor()
{
	unsigned long result;
	InvokeHelper(0x6c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x6c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

CTeeFont CTeeShape::GetFont()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x68, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeFont(pDispatch);
}

CGradient CTeeShape::GetGradient()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x65, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGradient(pDispatch);
}

long CTeeShape::GetShadowSize()
{
	long result;
	InvokeHelper(0x3f4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetShadowSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x3f4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

unsigned long CTeeShape::GetShadowColor()
{
	unsigned long result;
	InvokeHelper(0x66, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetShadowColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x66, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

BOOL CTeeShape::GetTransparent()
{
	BOOL result;
	InvokeHelper(0x3f5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetTransparent(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x3f5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CTeeShape::GetShapeStyle()
{
	long result;
	InvokeHelper(0x67, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetShapeStyle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x67, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CTeeShape::GetTransparency()
{
	long result;
	InvokeHelper(0x7e, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetTransparency(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x7e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CTeeRect CTeeShape::GetShapeBounds()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x85, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeRect(pDispatch);
}

long CTeeShape::GetBevel()
{
	long result;
	InvokeHelper(0x139, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetBevel(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x139, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CTeeShape::GetBevelWidth()
{
	long result;
	InvokeHelper(0x13a, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetBevelWidth(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x13a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

void CTeeShape::ShowEditorShape(BOOL HideTexts, LPCTSTR Caption)
{
	static BYTE parms[] =
		VTS_BOOL VTS_BSTR;
	InvokeHelper(0xc9, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 HideTexts, Caption);
}

CTeeShadow CTeeShape::GetShadow()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeShadow(pDispatch);
}

long CTeeShape::GetRoundSize()
{
	long result;
	InvokeHelper(0xcb, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetRoundSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xcb, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CPen1 CTeeShape::GetPen()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcc, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

CBackImage CTeeShape::GetPicture()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBackImage(pDispatch);
}

void CTeeShape::Show()
{
	InvokeHelper(0xce, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CTeeShape::Hide()
{
	InvokeHelper(0xcf, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

CTeeEmboss CTeeShape::GetEmboss()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeEmboss(pDispatch);
}

long CTeeShape::GetWidth()
{
	long result;
	InvokeHelper(0xdc, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetWidth(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xdc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CTeeShape::GetHeight()
{
	long result;
	InvokeHelper(0xe6, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetHeight(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe6, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CPen1 CTeeShape::GetFrame()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x122, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

CBrush1 CTeeShape::GetBrush()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x100, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

CShapeCallout CTeeShape::GetShapeCallout()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x101, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CShapeCallout(pDispatch);
}

CTeeCorners CTeeShape::GetCorners()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x102, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeCorners(pDispatch);
}

CTeeShapes CTeeShape::GetChildren()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x103, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeShapes(pDispatch);
}

long CTeeShape::GetChildLayout()
{
	long result;
	InvokeHelper(0x106, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetChildLayout(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x106, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CTeeShape::GetAngle()
{
	long result;
	InvokeHelper(0x107, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetAngle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x107, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CTeeShape::GetShapeVisible()
{
	BOOL result;
	InvokeHelper(0xd0, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CTeeShape::SetShapeVisible(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xd0, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

void CTeeShape::LoadImage(LPCTSTR FileName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x13b, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName);
}

void CTeeShape::AssignImage(long ImageHandle)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x13c, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 ImageHandle);
}

void CTeeShape::ClearImage()
{
	InvokeHelper(0x13d, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}