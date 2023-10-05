#pragma once


#include "MyToolTipCtrl.h"
#include "ExtComboBox.h"

class CDlgPageField : public CDialogEx
{
public:
	enum { IDD = IDD_DLGPAGEFIELD };

	CDlgPageField(CWnd *pParent = NULL);
	int Display();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnClickTree( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnBnClickedAddStyle();
	afx_msg void OnBnClickedDeleteStyle();
	afx_msg void OnCbnSelChangeStyle();
	LRESULT OnCheckStateChange( WPARAM wParam, LPARAM lParam );

// Protected variables.
protected:
	CTADatabase *m_pTADB;
	CTADatastruct *m_pTADS;
	CUserDatabase *m_pUSERDB;
	CDB_PageSetup *m_pTADSPageSetup;
	CDB_PageSetup *m_pTADSPageSetupBak;
	CMyToolTipCtrl m_ToolTip;
	char m_bModified;

	CButton m_ButtonOk;
	CButton m_ButtonDelStyle;
	CButton m_ButtonAddStyle;
	CExtNumEditComboBox m_ComboStyle;
	CTreeCtrl m_Tree;

// Private methods.
private:
	// Initialize tree with the content of current style.
	void _InitPageField();

	// Initialize drop down with PageField DB.
	void _InitPageFieldDD( CString str = _T("") );
	
	// Fill current style with current selected Items in tree.
	void _FindSelectedFields();
	
	// Retrieve in Tree (pTree) the first item close (child or sibling item) to the hTree, works top to bottom 
	// bOneBranch	= true if you want to stop when all hTree children are scanned
	//				= false if you want to continue with the hTree sibling
	HTREEITEM _NextTreeItem( CTreeCtrl *pTree, HTREEITEM hTree, bool bOneBranch = false );

	int _IsChildChecked( CTreeCtrl *pTree, HTREEITEM hTree );
};
