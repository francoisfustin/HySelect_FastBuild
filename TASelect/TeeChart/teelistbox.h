#if !defined(AFX_TEELISTBOX_H__79270903_7F50_45C4_A3D7_8F71AC8FED94__INCLUDED_)
#define AFX_TEELISTBOX_H__79270903_7F50_45C4_A3D7_8F71AC8FED94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CSeriesGroup;
class CTeeListBoxSection;
class CStrings;

/////////////////////////////////////////////////////////////////////////////
// CTeeListBox wrapper class

class CTeeListBox : public CWnd
{
protected:
	DECLARE_DYNCREATE(CTeeListBox)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0xd578a6a4, 0x2ef2, 0x489d, { 0x87, 0x38, 0xd1, 0xe2, 0xc7, 0x3e, 0xff, 0xf6 } };
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
	unsigned long GetColor();
	void SetColor(unsigned long newValue);
	void UpdateSeries();
	long GetSelectedSeries();
	void SetSelectedSeries(long nNewValue);
	BOOL GetShowActive();
	void SetShowActive(BOOL bNewValue);
	BOOL GetShowColor();
	void SetShowColor(BOOL bNewValue);
	BOOL GetShowTitle();
	void SetShowTitle(BOOL bNewValue);
	BOOL GetShowIcon();
	void SetShowIcon(BOOL bNewValue);
	CSeriesGroup GetSeriesGroup();
	void SetSeriesGroup(LPDISPATCH newValue);
	long GetCheckStyle();
	void SetCheckStyle(long nNewValue);
	void ShowEditor();
	void HideEditor();
	long GetItemHeight();
	void SetItemHeight(long nNewValue);
	void ClearItems();
	void Repaint();
	BOOL GetSorted();
	void SetSorted(BOOL bNewValue);
	BOOL GetAskDelete();
	void SetAskDelete(BOOL bNewValue);
	BOOL GetAllowDeleteSeries();
	void SetAllowDeleteSeries(BOOL bNewValue);
	BOOL GetAllowAddSeries();
	void SetAllowAddSeries(BOOL bNewValue);
	CTeeListBoxSection GetSections(long Index);
	BOOL GetFontSeriesColor();
	void SetFontSeriesColor(BOOL bNewValue);
	CStrings GetItems();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEELISTBOX_H__79270903_7F50_45C4_A3D7_8F71AC8FED94__INCLUDED_)
