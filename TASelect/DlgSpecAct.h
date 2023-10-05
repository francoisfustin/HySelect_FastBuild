#pragma once


class CTabDlgSpecActUser;
class CTabDlgSpecActDev;
class CTabDlgSpecActTesting;
class CTabDlgSpecActTesting2;
class CTabDlgSpecActHMCalc;
class CTabDlgSpecActTestProdSel;
class CDlgSpecAct : public CDialogEx
{
public:
	enum { IDD = IDD_DLGSPECACT };

	CDlgSpecAct( CWnd *pParent = NULL );
	virtual ~CDlgSpecAct();

	int Display();
	int CheckFile();

	CTabDlgSpecActUser *GetpTabUser( void ) { return m_pTabSpecActUser; }
	CTabDlgSpecActDev *GetpTabDev( void ) { return m_pTabSpecActDev; }
	CTabDlgSpecActTesting *GetpTabTesting( void ) { return m_pTabSpecActTesting; }
	CTabDlgSpecActTesting2 *GetpTabTesting2( void ) { return m_pTabSpecActTesting2; }
	CTabDlgSpecActHMCalc *GetpTabHMCalc( void ) { return m_pTabSpecActHMCalc; }
	CTabDlgSpecActTestProdSel *GetpTabProdSel( void ) { return m_pTabSpecActTestProdSel; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnSelchangeTabctrl( NMHDR *pNMHDR, LRESULT *pResult );
	
// Private methods.
private:
	void _Clean( void );
	void _DisplayTabCDlg( CDialogEx *pTabCDlg );

// Private variables.
private:
	CDialogEx *m_pCurCDlg;	
	CTabDlgSpecActUser *m_pTabSpecActUser;
	CTabDlgSpecActDev *m_pTabSpecActDev;
	CTabDlgSpecActTesting *m_pTabSpecActTesting;
	CTabDlgSpecActTesting2 *m_pTabSpecActTesting2;
	CTabDlgSpecActHMCalc *m_pTabSpecActHMCalc;
	CTabDlgSpecActTestProdSel *m_pTabSpecActTestProdSel;
	CTabCtrl m_TabCtrl;
};
