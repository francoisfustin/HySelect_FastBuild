// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "customtextshape.h"

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
// CCustomTextShape properties

/////////////////////////////////////////////////////////////////////////////
// CCustomTextShape operations

unsigned long CCustomTextShape::GetColor()
{
	unsigned long result;
	InvokeHelper(0x6c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x6c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

CTeeFont CCustomTextShape::GetFont()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x68, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeFont(pDispatch);
}

CGradient CCustomTextShape::GetGradient()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x65, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGradient(pDispatch);
}

long CCustomTextShape::GetShadowSize()
{
	long result;
	InvokeHelper(0x3f4, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetShadowSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x3f4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

unsigned long CCustomTextShape::GetShadowColor()
{
	unsigned long result;
	InvokeHelper(0x66, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetShadowColor(unsigned long newValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x66, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 newValue);
}

BOOL CCustomTextShape::GetTransparent()
{
	BOOL result;
	InvokeHelper(0x3f5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetTransparent(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x3f5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CCustomTextShape::GetShapeStyle()
{
	long result;
	InvokeHelper(0x67, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetShapeStyle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x67, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CCustomTextShape::GetTransparency()
{
	long result;
	InvokeHelper(0x7e, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetTransparency(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x7e, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CTeeRect CCustomTextShape::GetShapeBounds()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x85, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeRect(pDispatch);
}

long CCustomTextShape::GetBevel()
{
	long result;
	InvokeHelper(0x139, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetBevel(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x139, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CCustomTextShape::GetBevelWidth()
{
	long result;
	InvokeHelper(0x13a, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetBevelWidth(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x13a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

void CCustomTextShape::ShowEditorShape(BOOL HideTexts, LPCTSTR Caption)
{
	static BYTE parms[] =
		VTS_BOOL VTS_BSTR;
	InvokeHelper(0xc9, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 HideTexts, Caption);
}

CTeeShadow CCustomTextShape::GetShadow()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeShadow(pDispatch);
}

long CCustomTextShape::GetRoundSize()
{
	long result;
	InvokeHelper(0xcb, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetRoundSize(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xcb, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CPen1 CCustomTextShape::GetPen()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcc, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

CBackImage CCustomTextShape::GetPicture()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBackImage(pDispatch);
}

void CCustomTextShape::Show()
{
	InvokeHelper(0xce, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CCustomTextShape::Hide()
{
	InvokeHelper(0xcf, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

CTeeEmboss CCustomTextShape::GetEmboss()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeEmboss(pDispatch);
}

long CCustomTextShape::GetWidth()
{
	long result;
	InvokeHelper(0xdc, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetWidth(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xdc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CCustomTextShape::GetHeight()
{
	long result;
	InvokeHelper(0xe6, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetHeight(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe6, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CPen1 CCustomTextShape::GetFrame()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x122, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

CBrush1 CCustomTextShape::GetBrush()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x100, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrush1(pDispatch);
}

CShapeCallout CCustomTextShape::GetShapeCallout()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x101, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CShapeCallout(pDispatch);
}

CTeeCorners CCustomTextShape::GetCorners()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x102, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeCorners(pDispatch);
}

CTeeShapes CCustomTextShape::GetChildren()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x103, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeShapes(pDispatch);
}

long CCustomTextShape::GetChildLayout()
{
	long result;
	InvokeHelper(0x106, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetChildLayout(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x106, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CCustomTextShape::GetAngle()
{
	long result;
	InvokeHelper(0x107, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetAngle(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x107, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CCustomTextShape::GetShapeVisible()
{
	BOOL result;
	InvokeHelper(0xd0, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetShapeVisible(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xd0, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CCustomTextShape::GetTextFormat()
{
	long result;
	InvokeHelper(0x353, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CCustomTextShape::SetTextFormat(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x353, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}
