#if !defined(AFX_CHARTEDITORPANEL_H__52884DA2_0EEC_4D6A_A861_D0AC4C177436__INCLUDED_)
#define AFX_CHARTEDITORPANEL_H__52884DA2_0EEC_4D6A_A861_D0AC4C177436__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CPageControl;

/////////////////////////////////////////////////////////////////////////////
// CChartEditorPanel wrapper class

class CChartEditorPanel : public CWnd
{
protected:
	DECLARE_DYNCREATE(CChartEditorPanel)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x8239ce9c, 0x91dc, 0x4779, { 0x9f, 0xe6, 0xbd, 0xbf, 0x1a, 0xe3, 0x55, 0x6f } };
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
	void SelectUnderMouse();
	long GetBevelOuter();
	void SetBevelOuter(long nNewValue);
	void SetEditor(LPDISPATCH newValue);
	void SetEditorLink(long nNewValue);
	CPageControl GetMainPage();
	CPageControl GetSubPage();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHARTEDITORPANEL_H__52884DA2_0EEC_4D6A_A861_D0AC4C177436__INCLUDED_)
