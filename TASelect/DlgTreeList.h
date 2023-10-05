#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "MyToolTipCtrl.h"


// CDlgTreeList dialog

class CDlgTreeList : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTreeList)

public:
	CDlgTreeList(CWnd* pParent = NULL);   
	virtual ~CDlgTreeList();

	class CStrData
	{
		public:
			CStrData(){};
			CStrData( CString str, CString data ){
				m_str = str; m_data = data;
			};
			CString GetStr() const {
				return m_str;
			}
			CString GetData() const {
				return m_data;
			}
		private:
			CString m_str;
			CString m_data;
	};

	virtual BOOL OnInitDialog();
	int Display( int IDTitle );
	int Display( CString Title );
	const CArray<CStrData> *GetpStrDataArray() {
		return &m_StrDataAr;
	}
	void Add( CString str, CString data  );
// Dialog Data
	enum { IDD = IDD_DLGCHECKLIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	int _TreeSelect( HTREEITEM hItem, BOOL bCheck, bool fSibling, bool fChildren );

	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedButselect();
	afx_msg void OnBnClickedButunselect();
	afx_msg void OnNMClickTree( NMHDR *pNMHDR, LRESULT *pResult );
	// User click on the tree.
	// Display information according to the tree state.
	LRESULT OnCheckStateChange( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()
private:
	CButton m_ButSelectAll;
	CTreeCtrl m_TreeList;
	CButton m_ButUnSelectAll;
	CArray<CStrData> m_StrDataAr;
	CString m_Title;
	CMyToolTipCtrl  m_ToolTip;

};
