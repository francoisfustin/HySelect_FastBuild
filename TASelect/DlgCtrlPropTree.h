#pragma once


#include "DlgCtrlPropPage.h"
#include "DlgCtrlPropPageMng.h"
#include "afxcmn.h"

class CDlgCtrlPropTree : public CDlgCtrlPropPageMng
{
	DECLARE_DYNAMIC( CDlgCtrlPropTree )

public:
	enum { IDD = IDD_DLGCTRLPROPTREE };

	CDlgCtrlPropTree( CWnd* pParent = NULL );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNMClickTreectrlprop( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void OnTvnSelchangedTreectrlprop( NMHDR *pNMHDR, LRESULT *pResult );

	bool AddPageToTree( CDlgCtrlPropPage* pTreePage, HTREEITEM* pItemAdded = NULL );
	bool AddChildPageToTree( CDlgCtrlPropPage* pTreePageParent, CDlgCtrlPropPage* pTreePage, HTREEITEM* pItemAdded = NULL );

// Protected variables.
protected:
	CTreeCtrl	m_Tree;
	bool		m_bAllowNewSelectionTree;
	int			m_iMargin;
};
