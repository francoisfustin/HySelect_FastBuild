#pragma once


#include "DialogExt.h"
#include "afxwin.h"

class CTUProdSelLauncher;
class CTabDlgSpecActTestProdSelTest : public CDialogExt
{
public:
	enum { IDD = IDD_TABSPECACT_TESTPRODSEL_TEST };

	CTabDlgSpecActTestProdSelTest( CWnd *pParent = NULL );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedOpenFile();
	virtual afx_msg void OnBnClickedStartTest();
	virtual afx_msg void OnBnClickedClear();

// Private variables.
private:
	CEdit m_EditFileName;
	CListBox m_List;
	CTUProdSelLauncher m_clTUProdSelLauncher;
};
