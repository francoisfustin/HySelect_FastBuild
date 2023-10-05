#pragma once


#include "DialogExt.h"
#include "afxwin.h"

class CDlgSpecActComputeCRC : public CDialogExt
{
public:
	enum { IDD = IDD_DLGSPECACT_COMPUTECRC };

	CDlgSpecActComputeCRC( CWnd *pParent = NULL );
	virtual ~CDlgSpecActComputeCRC() {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedOpenFileToCompute();
	virtual afx_msg void OnBnClickedComputeCRC();
	virtual afx_msg void OnBnClickedClear();

// Private variables.
private:
	CEdit m_EditFileToCompute;
	CListBox m_List;
};
