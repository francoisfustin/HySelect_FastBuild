#if !defined(AFX_CHARTPAGENAVIGATOR_H__96E2F014_4C35_4426_B06B_F9A95FD9FC2B__INCLUDED_)
#define AFX_CHARTPAGENAVIGATOR_H__96E2F014_4C35_4426_B06B_F9A95FD9FC2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CChartPageNavigator wrapper class

class CChartPageNavigator : public CWnd
{
protected:
	DECLARE_DYNCREATE(CChartPageNavigator)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x95384bdf, 0x373d, 0x4fbe, { 0xaf, 0xa3, 0xb4, 0x33, 0x9c, 0xe9, 0xd3, 0x2b } };
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
	void EnableButtons();
	unsigned long GetColor();
	void SetColor(unsigned long newValue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHARTPAGENAVIGATOR_H__96E2F014_4C35_4426_B06B_F9A95FD9FC2B__INCLUDED_)
