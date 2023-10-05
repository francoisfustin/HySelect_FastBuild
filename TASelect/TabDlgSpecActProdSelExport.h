#pragma once


#include "DialogExt.h"
#include "afxwin.h"

class CTUProdSelLauncher;
class CTabDlgSpecActTestProdSelExport : public CDialogExt
{
public:
	enum { IDD = IDD_TABSPECACT_TESTPRODSEL_EXPORT };

	CTabDlgSpecActTestProdSelExport( CWnd *pParent = NULL );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedOpenFile();
	virtual afx_msg void OnBnClickedExportSelection();
	virtual afx_msg void OnBnClickedClear();

// Private variables.
private:
	CEdit m_EditFileName;
	CListBox m_List;
	CTUProdSelLauncher m_clTUProdSelLauncher;
};
