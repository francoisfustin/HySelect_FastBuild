// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


#include "stdafx.h"
#include "subchartchart.h"

// Dispatch interfaces referenced by this interface
#include "aspect.h"
#include "axes.h"
#include "canvas.h"
#include "export.h"
#include "titles.h"
#include "pen.h"
#include "import.h"
#include "legend.h"
#include "page.h"
#include "panel.h"
#include "printer.h"
#include "scroll.h"
#include "series.h"
#include "walls.h"
#include "zoom.h"
#include "environment.h"
#include "teepoint2d.h"
#include "toollist.h"
#include "serieslist.h"
#include "teerect.h"
#include "animationlist.h"
#include "teepanning.h"
#include "themeslist.h"
#include "teeanimate.h"
#include "hover.h"


/////////////////////////////////////////////////////////////////////////////
// CSubChartChart properties

/////////////////////////////////////////////////////////////////////////////
// CSubChartChart operations

CAspect CSubChartChart::GetAspect()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CAspect(pDispatch);
}

CAxes CSubChartChart::GetAxis()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CAxes(pDispatch);
}

CCanvas CSubChartChart::GetCanvas()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x3, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CCanvas(pDispatch);
}

BOOL CSubChartChart::GetClipPoints()
{
	BOOL result;
	InvokeHelper(0x4, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetClipPoints(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x4, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

short CSubChartChart::GetCursor()
{
	short result;
	InvokeHelper(0x5, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetCursor(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x5, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

short CSubChartChart::GetDragCursor()
{
	short result;
	InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetDragCursor(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x6, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CSubChartChart::GetDragMode()
{
	long result;
	InvokeHelper(0x7, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetDragMode(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x7, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CSubChartChart::GetEnabled()
{
	BOOL result;
	InvokeHelper(0x8, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetEnabled(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CExport CSubChartChart::GetExport()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x9, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CExport(pDispatch);
}

CTitles CSubChartChart::GetFooter()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xa, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTitles(pDispatch);
}

CPen1 CSubChartChart::GetFrame()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xb, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPen1(pDispatch);
}

CTitles CSubChartChart::GetHeader()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x16, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTitles(pDispatch);
}

long CSubChartChart::GetHeight()
{
	long result;
	InvokeHelper(0xc, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetHeight(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CImport CSubChartChart::GetImport()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CImport(pDispatch);
}

long CSubChartChart::GetLeft()
{
	long result;
	InvokeHelper(0xe, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetLeft(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CLegend CSubChartChart::GetLegend()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xf, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CLegend(pDispatch);
}

CPage CSubChartChart::GetPage()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x10, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPage(pDispatch);
}

CPanel CSubChartChart::GetPanel()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x11, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPanel(pDispatch);
}

CPrinter CSubChartChart::GetPrinter()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x12, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CPrinter(pDispatch);
}

CScroll CSubChartChart::GetScroll()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x13, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CScroll(pDispatch);
}

long CSubChartChart::GetSeriesCount()
{
	long result;
	InvokeHelper(0x15, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CSubChartChart::GetTop()
{
	long result;
	InvokeHelper(0x17, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetTop(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x17, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CSubChartChart::GetVisible()
{
	BOOL result;
	InvokeHelper(0x18, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetVisible(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x18, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CWalls CSubChartChart::GetWalls()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x19, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CWalls(pDispatch);
}

long CSubChartChart::GetWidth()
{
	long result;
	InvokeHelper(0x1a, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetWidth(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1a, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

CZoom CSubChartChart::GetZoom()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x1b, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CZoom(pDispatch);
}

void CSubChartChart::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long CSubChartChart::AddSeries(long SeriesClass)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1c, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		SeriesClass);
	return result;
}

void CSubChartChart::ChangeSeriesType(long SeriesIndex, long NewSeriesType)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0x1d, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 SeriesIndex, NewSeriesType);
}

long CSubChartChart::CloneSeries(long SourceSeriesIndex)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1e, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		SourceSeriesIndex);
	return result;
}

void CSubChartChart::Draw(const VARIANT& DC, long Left, long Top, long Right, long Bottom)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_I4 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x1f, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &DC, Left, Top, Right, Bottom);
}

void CSubChartChart::ExchangeSeries(long Series1, long Series2)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0x20, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Series1, Series2);
}

double CSubChartChart::GetDateTimeStep(long DateTimeStep)
{
	double result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x21, DISPATCH_METHOD, VT_R8, (void*)&result, parms,
		DateTimeStep);
	return result;
}

unsigned long CSubChartChart::GetFreeSeriesColor(BOOL CheckBackgroundColor)
{
	unsigned long result;
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x22, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		CheckBackgroundColor);
	return result;
}

CSeries CSubChartChart::GetLabelsSeries(long DestAxis)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x23, DISPATCH_METHOD, VT_DISPATCH, (void*)&pDispatch, parms,
		DestAxis);
	return CSeries(pDispatch);
}

BOOL CSubChartChart::IsFreeSeriesColor(unsigned long AColor, BOOL CheckBackgroundColor)
{
	BOOL result;
	static BYTE parms[] =
		VTS_I4 VTS_BOOL;
	InvokeHelper(0x24, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		AColor, CheckBackgroundColor);
	return result;
}

void CSubChartChart::RemoveAllSeries()
{
	InvokeHelper(0x25, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CSubChartChart::RemoveSeries(long SeriesIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x26, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 SeriesIndex);
}

void CSubChartChart::Repaint()
{
	InvokeHelper(0x27, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

CString CSubChartChart::SeriesTitleLegend(long ASeriesIndex)
{
	CString result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x28, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		ASeriesIndex);
	return result;
}

void CSubChartChart::ShowEditor(long SeriesIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x29, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 SeriesIndex);
}

void CSubChartChart::StopMouse()
{
	InvokeHelper(0x2a, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

BOOL CSubChartChart::GetTimerEnabled()
{
	BOOL result;
	InvokeHelper(0x2b, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetTimerEnabled(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x2b, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

long CSubChartChart::GetTimerInterval()
{
	long result;
	InvokeHelper(0x2c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetTimerInterval(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x2c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CSubChartChart::GetAutoRepaint()
{
	BOOL result;
	InvokeHelper(0x2d, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetAutoRepaint(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0x2d, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

void CSubChartChart::EditOneSeries(long SeriesIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x2e, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 SeriesIndex);
}

CEnvironment CSubChartChart::GetEnvironment()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x2f, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CEnvironment(pDispatch);
}

long CSubChartChart::GetChartLink()
{
	long result;
	InvokeHelper(0x30, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

CTeePoint2D CSubChartChart::GetMousePosition()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x31, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeePoint2D(pDispatch);
}

void CSubChartChart::ChartRect(long Left, long Top, long Right, long Bottom)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x32, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Left, Top, Right, Bottom);
}

CToolList CSubChartChart::GetTools()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x33, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CToolList(pDispatch);
}

CTitles CSubChartChart::GetSubHeader()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x34, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTitles(pDispatch);
}

CTitles CSubChartChart::GetSubFooter()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0x35, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTitles(pDispatch);
}

void CSubChartChart::ClearChart()
{
	InvokeHelper(0x37, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CSubChartChart::RefreshData()
{
	InvokeHelper(0x36, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

CSeries CSubChartChart::Series(long SeriesIndex)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x38, DISPATCH_METHOD, VT_DISPATCH, (void*)&pDispatch, parms,
		SeriesIndex);
	return CSeries(pDispatch);
}

BOOL CSubChartChart::ShowThemesEditor()
{
	BOOL result;
	InvokeHelper(0xc9, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetTheme(long aTheme, long aPalette)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0xca, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 aTheme, aPalette);
}

void CSubChartChart::ApplyPalette(long paletteIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xcb, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 paletteIndex);
}

void CSubChartChart::ApplyCustomPalette(const VARIANT& colorArray)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0xcc, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &colorArray);
}

CSeriesList CSubChartChart::GetSeriesList()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xcd, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CSeriesList(pDispatch);
}

CString CSubChartChart::GetHint()
{
	CString result;
	InvokeHelper(0xce, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetHint(LPCTSTR lpszNewValue)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0xce, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 lpszNewValue);
}

BOOL CSubChartChart::GetShowHint()
{
	BOOL result;
	InvokeHelper(0xcf, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetShowHint(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xcf, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

CString CSubChartChart::GetVersion()
{
	CString result;
	InvokeHelper(0xd0, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
	return result;
}

BOOL CSubChartChart::GetCustomChartRect()
{
	BOOL result;
	InvokeHelper(0xd1, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetCustomChartRect(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xd1, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CSubChartChart::GetMonochrome()
{
	BOOL result;
	InvokeHelper(0xd2, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetMonochrome(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xd2, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

short CSubChartChart::GetOriginalCursor()
{
	short result;
	InvokeHelper(0xd3, DISPATCH_PROPERTYGET, VT_I2, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetOriginalCursor(short nNewValue)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0xd3, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

BOOL CSubChartChart::GetPrinting()
{
	BOOL result;
	InvokeHelper(0xd4, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

long CSubChartChart::GetChartHeight()
{
	long result;
	InvokeHelper(0xd7, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

long CSubChartChart::GetChartWidth()
{
	long result;
	InvokeHelper(0xd8, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

CTeeRect CSubChartChart::GetChartBounds()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xdb, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeRect(pDispatch);
}

BOOL CSubChartChart::GetBufferedDisplay()
{
	BOOL result;
	InvokeHelper(0xdc, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetBufferedDisplay(BOOL bNewValue)
{
	static BYTE parms[] =
		VTS_BOOL;
	InvokeHelper(0xdc, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 bNewValue);
}

BOOL CSubChartChart::GetCanClip()
{
	BOOL result;
	InvokeHelper(0xd5, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, NULL);
	return result;
}

CTeeRect CSubChartChart::GetGetChartRect()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xd6, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeRect(pDispatch);
}

BOOL CSubChartChart::EditStrings(BSTR* Strings, BOOL ReadOnly)
{
	BOOL result;
	static BYTE parms[] =
		VTS_PBSTR VTS_BOOL;
	InvokeHelper(0xd9, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		Strings, ReadOnly);
	return result;
}

long CSubChartChart::GetLanguage()
{
	long result;
	InvokeHelper(0xda, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetLanguage(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xda, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CSubChartChart::GetLanguaged()
{
	long result;
	InvokeHelper(0xdd, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetLanguaged(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xdd, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

long CSubChartChart::MultiLineTextWidth(LPCTSTR AString, long NumLines, long TextFormat)
{
	long result;
	static BYTE parms[] =
		VTS_BSTR VTS_I4 VTS_I4;
	InvokeHelper(0xde, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		AString, NumLines, TextFormat);
	return result;
}

long CSubChartChart::MultiLineTextHeight(LPCTSTR AString, long NumLines, long TextFormat)
{
	long result;
	static BYTE parms[] =
		VTS_BSTR VTS_I4 VTS_I4;
	InvokeHelper(0xdf, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		AString, NumLines, TextFormat);
	return result;
}

CAnimationList CSubChartChart::GetAnimations()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xe0, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CAnimationList(pDispatch);
}

CSeries CSubChartChart::CreateNewSeriesGallery(long AGalleryWidth, long AGalleryHeight)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0xe1, DISPATCH_METHOD, VT_DISPATCH, (void*)&pDispatch, parms,
		AGalleryWidth, AGalleryHeight);
	return CSeries(pDispatch);
}

CTeePanning CSubChartChart::GetPanning()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xe2, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeePanning(pDispatch);
}

long CSubChartChart::ActiveSeriesLegend(long ItemIndex)
{
	long result;
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe3, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ItemIndex);
	return result;
}

void CSubChartChart::ShowSeriesEditor(long SeriesIndex)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 SeriesIndex);
}

long CSubChartChart::CloneSeriesFromChart(long ChartLink, long SeriesIndex)
{
	long result;
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0xe5, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ChartLink, SeriesIndex);
	return result;
}

long CSubChartChart::CloneToolFromChart(long ChartLink, long ToolIndex)
{
	long result;
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0xe6, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ChartLink, ToolIndex);
	return result;
}

CSeries CSubChartChart::CreateSeriesChangeGallery(long AGalleryWidth, long AGalleryHeight, long SeriesIndex)
{
	LPDISPATCH pDispatch;
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0xe7, DISPATCH_METHOD, VT_DISPATCH, (void*)&pDispatch, parms,
		AGalleryWidth, AGalleryHeight, SeriesIndex);
	return CSeries(pDispatch);
}

CThemesList CSubChartChart::GetThemes()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xe8, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CThemesList(pDispatch);
}

long CSubChartChart::GetZoomWheel()
{
	long result;
	InvokeHelper(0xe9, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
	return result;
}

void CSubChartChart::SetZoomWheel(long nNewValue)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0xe9, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 nNewValue);
}

void CSubChartChart::SetChartRect(long Left, long Top, long Right, long Bottom)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0xea, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Left, Top, Right, Bottom);
}

CTeeAnimate CSubChartChart::GetAnimate()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xeb, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CTeeAnimate(pDispatch);
}

VARIANT CSubChartChart::GetChartLinkx64()
{
	VARIANT result;
	InvokeHelper(0xec, DISPATCH_PROPERTYGET, VT_VARIANT, (void*)&result, NULL);
	return result;
}

long CSubChartChart::CloneSeriesFromChartx64(const VARIANT& ChartLinkx64, long SeriesIndex)
{
	long result;
	static BYTE parms[] =
		VTS_VARIANT VTS_I4;
	InvokeHelper(0xed, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		&ChartLinkx64, SeriesIndex);
	return result;
}

long CSubChartChart::CloneToolFromChartx64(const VARIANT& ChartLinkx64, long ToolIndex)
{
	long result;
	static BYTE parms[] =
		VTS_VARIANT VTS_I4;
	InvokeHelper(0xee, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		&ChartLinkx64, ToolIndex);
	return result;
}

CHover CSubChartChart::GetHover()
{
	LPDISPATCH pDispatch;
	InvokeHelper(0xef, DISPATCH_PROPERTYGET, VT_DISPATCH, (void*)&pDispatch, NULL);
	return CHover(pDispatch);
}
