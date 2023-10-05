#if !defined(AFX_CHARTGRID_H__D9C03580_4620_4D24_96E0_31C25A5ACF1A__INCLUDED_)
#define AFX_CHARTGRID_H__D9C03580_4620_4D24_96E0_31C25A5ACF1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CChartGridCols;
class CChartGridRows;
class CGradient;

/////////////////////////////////////////////////////////////////////////////
// CChartGrid wrapper class

class CChartGrid : public CWnd
{
protected:
	DECLARE_DYNCREATE(CChartGrid)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0xd2daddae, 0x4167, 0x499c, { 0x91, 0x53, 0x74, 0x80, 0x98, 0x6e, 0x91, 0xa5 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); }

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID,
		CFile* pPersist = NULL, BOOL bStorage = FALSE,
		BSTR bstrLicKey = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); }

// Attributes
public:

// Operations
public:
	void SetChart(LPDISPATCH newValue);
	void SetChartLink(long nNewValue);
	void AboutBox();
	void SetChartLinkx64(const VARIANT& newValue);
	long GetCol();
	void SetCol(long nNewValue);
	long GetColCount();
	BOOL GetEditorMode();
	void SetEditorMode(BOOL bNewValue);
	long GetGridHeight();
	long GetGridWidth();
	long GetLeftCol();
	void SetLeftCol(long nNewValue);
	long GetGridLink();
	void RecalcDimensions();
	BOOL GetShowLabels();
	void SetShowLabels(BOOL bNewValue);
	BOOL GetShowColors();
	void SetShowColors(BOOL bNewValue);
	BOOL GetShowFields();
	void SetShowFields(BOOL bNewValue);
	CChartGridCols Cols();
	CChartGridRows Rows();
	long GetDefaultColWidth();
	void SetDefaultColWidth(long nNewValue);
	BOOL GetGrid3DMode();
	void SetGrid3DMode(BOOL bNewValue);
	CString GetLabelHeader();
	void SetLabelHeader(LPCTSTR lpszNewValue);
	BOOL GetAllowAppend();
	void SetAllowAppend(BOOL bNewValue);
	BOOL GetReadOnly();
	void SetReadOnly(BOOL bNewValue);
	BOOL GetAllowInsertSeries();
	void SetAllowInsertSeries(BOOL bNewValue);
	long GetColorsColumn();
	long GetFirstRowNum();
	long GetLabelsColumn();
	BOOL GetSeriesSymbolClickable();
	void SetSeriesSymbolClickable(BOOL bNewValue);
	long GetShowXValues();
	void SetShowXValues(long nNewValue);
	long GetTopRow();
	void SetTopRow(long nNewValue);
	long GetRowHeights(long Index);
	void SetRowHeights(long Index, long nNewValue);
	CGradient GetGradientCell();
	VARIANT GetGridLinkx64();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHARTGRID_H__D9C03580_4620_4D24_96E0_31C25A5ACF1A__INCLUDED_)