#pragma once


#include "DialogExt.h"
#include "afxwin.h"

class CTUProdSelLauncher;
class CTabDlgSpecActTestProdSelUpdate : public CDialogExt
{
public:
	enum { IDD = IDD_TABSPECACT_TESTPRODSEL_UPDATE };

	CTabDlgSpecActTestProdSelUpdate( CWnd *pParent = NULL );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedOpenInputFileName();
	virtual afx_msg void OnBnClickedOpenOutputFileName();
	virtual afx_msg void OnBnClickedStartUpdate();
	virtual afx_msg void OnBnClickedClear();

// Private variables.
private:
	CEdit m_InputFileName;
	CEdit m_OutputFileName;
	CListBox m_List;
	CTUProdSelLauncher m_clTUProdSelLauncher;
};
