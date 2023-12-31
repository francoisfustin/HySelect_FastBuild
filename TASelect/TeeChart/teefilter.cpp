// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "teefilter.h"

// Dispatch interfaces referenced by this interface
#include "filterregion.h"
#include "resizefilter.h"
#include "cropfilter.h"
#include "grayscalefilter.h"
#include "mosaicfilter.h"
#include "colorfilter.h"
#include "huelumsatfilter.h"
#include "rotatefilter.h"
#include "mirrorfilter.h"
#include "blurfilter.h"
#include "brightnessfilter.h"
#include "contrastfilter.h"
#include "gammacorrectionfilter.h"
#include "amountfilter.h"
#include "sharpenfilter.h"
#include "embossfilter.h"
#include "softenfilter.h"
#include "tilefilter.h"
#include "bevelfilter.h"
#include "zoomfilter.h"
#include "lightfilter.h"
#include "sobelfilter.h"
#include "sepiafilter.h"


/////////////////////////////////////////////////////////////////////////////
// CTeeFilter properties

/////////////////////////////////////////////////////////////////////////////
// CTeeFilter operations

BOOL CTeeFilter::GetEnabled()
{
	BOOL result;
	InvokeHelper(0xcb, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CTeeFilter::SetEnabled(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xcb, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CFilterRegion CTeeFilter::GetRegion()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcc, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CFilterRegion(pDispatch);
}

CResizeFilter CTeeFilter::GetAsResize()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CResizeFilter(pDispatch);
}

CCropFilter CTeeFilter::GetAsCrop()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CCropFilter(pDispatch);
}

CGrayScaleFilter CTeeFilter::GetAsGrayScale()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xce, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGrayScaleFilter(pDispatch);
}

CMosaicFilter CTeeFilter::GetAsMosaic()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcf, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CMosaicFilter(pDispatch);
}

CColorFilter CTeeFilter::GetAsColor()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CColorFilter(pDispatch);
}

CHueLumSatFilter CTeeFilter::GetAsHueLumSat()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CHueLumSatFilter(pDispatch);
}

CRotateFilter CTeeFilter::GetAsRotate()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CRotateFilter(pDispatch);
}

CMirrorFilter CTeeFilter::GetAsMirror()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CMirrorFilter(pDispatch);
}

CBlurFilter CTeeFilter::GetAsBlur()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd4, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBlurFilter(pDispatch);
}

CBrightnessFilter CTeeFilter::GetAsBrightness()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd5, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBrightnessFilter(pDispatch);
}

CContrastFilter CTeeFilter::GetAsContrast()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CContrastFilter(pDispatch);
}

CGammaCorrectionFilter CTeeFilter::GetAsGammaCorrection()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd7, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGammaCorrectionFilter(pDispatch);
}

CAmountFilter CTeeFilter::GetAsAmount()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xc9, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CAmountFilter(pDispatch);
}

CSharpenFilter CTeeFilter::GetAsSharpen()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd8, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSharpenFilter(pDispatch);
}

CEmbossFilter CTeeFilter::GetAsEmboss()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd9, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CEmbossFilter(pDispatch);
}

CSoftenFilter CTeeFilter::GetAsSoften()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xda, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSoftenFilter(pDispatch);
}

CTileFilter CTeeFilter::GetAsTile()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xdb, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTileFilter(pDispatch);
}

CBevelFilter CTeeFilter::GetAsBevel()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xdc, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBevelFilter(pDispatch);
}

CZoomFilter CTeeFilter::GetAsZoom()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xdd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CZoomFilter(pDispatch);
}

CLightFilter CTeeFilter::GetAsLight()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xde, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CLightFilter(pDispatch);
}

CSobelFilter CTeeFilter::GetAsSobel()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xdf, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSobelFilter(pDispatch);
}

CSepiaFilter CTeeFilter::GetAsSepia()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xe0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSepiaFilter(pDispatch);
}
