#if !defined(AFX_SERIESXMLSOURCE_H__F1BE8C37_FE6D_44E7_B761_8E3D2FB2C0E7__INCLUDED_)
#define AFX_SERIESXMLSOURCE_H__F1BE8C37_FE6D_44E7_B761_8E3D2FB2C0E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.


// Dispatch interfaces referenced by this interface
class CStrings;

/////////////////////////////////////////////////////////////////////////////
// CSeriesXMLSource wrapper class

class CSeriesXMLSource : public CWnd
{
protected:
	DECLARE_DYNCREATE(CSeriesXMLSource)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0xce16d8e0, 0x26ef, 0x41a1, { 0xaf, 0x12, 0xe7, 0xf, 0x2b, 0xd3, 0x1f, 0xec } };
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
	void Open();
	void Close();
	BOOL GetActive();
	void SetActive(BOOL bNewValue);
	VARIANT GetSeries();
	void SetSeries(const VARIANT& newValue);
	void Load();
	void LoadFromFile(LPCTSTR FileName);
	void LoadFromURL(LPCTSTR URL);
	CString GetFileName();
	void SetFileName(LPCTSTR lpszNewValue);
	long GetLoadMode();
	void SetLoadMode(long nNewValue);
	CString GetSeriesNode();
	void SetSeriesNode(LPCTSTR lpszNewValue);
	CString GetValueSource();
	void SetValueSource(LPCTSTR lpszNewValue);
	CStrings GetXml();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIESXMLSOURCE_H__F1BE8C37_FE6D_44E7_B761_8E3D2FB2C0E7__INCLUDED_)
