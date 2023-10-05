#pragma once


#include "DialogExt.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "TabDlgSpecActProdSelUpdate.h"
#include "TabDlgSpecActProdSelExport.h"
#include "TabDlgSpecActProdSelTest.h"

class CTUProdSelLauncher;
class CTabDlgSpecActTestProdSel : public CDialogExt
{
public:
	enum { IDD = IDD_TABSPECACT_TESTPRODSEL };

	CTabDlgSpecActTestProdSel( CWnd *pParent = NULL );
	virtual ~CTabDlgSpecActTestProdSel();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnSelChangeTabCtrl( NMHDR *pNMHDR, LRESULT *pResult );

// Private methods.
private:
	void _Clean( void );
	void _DisplayTabCDlg( CDialogEx *pTabCDlg );

// Private variables.
private:
	CTabCtrl m_WndTabCtrl;
	CDialogEx *m_pCurCDlg;	
	CTabDlgSpecActTestProdSelExport *m_pclTabDlgSpecActTestProdSelExport;
	CTabDlgSpecActTestProdSelTest *m_pclTabDlgSpecActTestProdSelTest;
	CTabDlgSpecActTestProdSelUpdate *m_pclTabDlgSpecActTestProdSelUpdate;
};
