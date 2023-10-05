// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "export.h"

// Dispatch interfaces referenced by this interface
#include "jpegexport.h"
#include "pngexport.h"
#include "gifexport.h"
#include "bmpexport.h"
#include "pcxexport.h"
#include "metafileexport.h"
#include "nativeexport.h"
#include "xmlexport.h"
#include "xlsexport.h"
#include "htmlexport.h"
#include "textexport.h"
#include "pdfexport.h"
#include "epsexport.h"
#include "svgexport.h"
#include "xamlexport.h"
#include "flexexport.h"
#include "javascriptexport.h"
#include "jsonexport.h"
#include "odsexport.h"
#include "xlsobjectexport.h"


/////////////////////////////////////////////////////////////////////////////
// CExport properties

/////////////////////////////////////////////////////////////////////////////
// CExport operations

void CExport::CopyToClipboardBitmap()
{
	InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CExport::CopyToClipboardMetafile(BOOL Enhanced)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Enhanced);
}

void CExport::SaveToBitmapFile(LPCTSTR FileName, long DPI)
{
	static BYTE parms[] =
		VTS_BSTR VTS_I4;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName, DPI);
}

void CExport::SaveToFile(LPCTSTR FileName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName);
}

void CExport::SaveToJPEGFile(LPCTSTR FileName, BOOL Gray, long Performance, long Quality, long Width, long Height, long DPI)
{
	static BYTE parms[] =
		VTS_BSTR VTS_BOOL VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName, Gray, Performance, Quality, Width, Height, DPI);
}

void CExport::SaveToMetafile(LPCTSTR FileName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName);
}

void CExport::SaveToMetafileEnh(LPCTSTR FileName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName);
}

void CExport::ShowExport()
{
	InvokeHelper(0x8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

CString CExport::SaveChartDialog()
{
	CString result;
	InvokeHelper(0x9, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

void CExport::ConvertTeeFileToText(LPCTSTR InputFile, LPCTSTR OutputFile)
{
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0xa, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 InputFile, OutputFile);
}

VARIANT CExport::Stream(long StreamType)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xc, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		StreamType);
	return result;
}

CJPEGExport CExport::GetAsJPEG()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xb, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CJPEGExport(pDispatch);
}

CPNGExport CExport::GetAsPNG()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPNGExport(pDispatch);
}

CGIFExport CExport::GetAsGIF()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xe, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CGIFExport(pDispatch);
}

CBMPExport CExport::GetAsBMP()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xf, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CBMPExport(pDispatch);
}

CPCXExport CExport::GetAsPCX()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x10, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPCXExport(pDispatch);
}

CMetafileExport CExport::GetAsMetafile()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x11, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CMetafileExport(pDispatch);
}

CNativeExport CExport::GetAsNative()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x12, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CNativeExport(pDispatch);
}

CXMLExport CExport::GetAsXML()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x13, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CXMLExport(pDispatch);
}

CXLSExport CExport::GetAsXLS()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x14, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CXLSExport(pDispatch);
}

CHTMLExport CExport::GetAsHTML()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x15, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CHTMLExport(pDispatch);
}

CTextExport CExport::GetAsText()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x16, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTextExport(pDispatch);
}

void CExport::EmailFile(LPCTSTR FileName, LPCTSTR Subject)
{
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x17, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName, Subject);
}

CPDFExport CExport::GetAsPDF()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xc9, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPDFExport(pDispatch);
}

CEPSExport CExport::GetAsEPS()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xca, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CEPSExport(pDispatch);
}

CSVGExport CExport::GetAsSVG()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcb, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSVGExport(pDispatch);
}

CString CExport::GetLabelHeader()
{
	CString result;
	InvokeHelper(0xcc, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

void CExport::SetLabelHeader(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0xcc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 lpszNewValue);
}

CXAMLExport CExport::GetAsXAML()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CXAMLExport(pDispatch);
}

void CExport::SaveToXMLFile(LPCTSTR FileName, BOOL IncludeData, BOOL XMLHeader)
{
	static BYTE parms[] =
		VTS_BSTR VTS_BOOL VTS_BOOL;
	InvokeHelper(0xce, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 FileName, IncludeData, XMLHeader);
}

VARIANT CExport::SaveToXMLStream(BOOL IncludeData, BOOL XMLHeader)
{
	VARIANT result;
	static BYTE parms[] =
		VTS_BOOL VTS_BOOL;
	InvokeHelper(0xcf, DISPATCH_METHOD, VT_VARIANT, (void*)&result, parms,
		IncludeData, XMLHeader);
	return result;
}

CFlexExport CExport::GetAsFlex()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CFlexExport(pDispatch);
}

CJavaScriptExport CExport::GetAsJavaScript()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CJavaScriptExport(pDispatch);
}

CJSONExport CExport::GetAsJSON()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CJSONExport(pDispatch);
}

CODSExport CExport::GetAsOpenDocument()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CODSExport(pDispatch);
}

CXLSObjectExport CExport::GetAsXLSX()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd4, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CXLSObjectExport(pDispatch);
}
