#pragma once
#include "afxwin.h"


#ifdef _DEBUG

class CDlgDebugSearchDBObject : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgDebugSearchDBObject)

public:
	enum { IDD = IDD_DLGDEBUGSEARCHDBOBJECT };
	CDlgDebugSearchDBObject( CWnd *pParent = NULL );
	virtual ~CDlgDebugSearchDBObject() {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	afx_msg void OnChangeEditSearchPattern();
	afx_msg void OnBnClickedButtonselect();
	afx_msg void OnBnClickedButtonsearch();

private:
	CEdit m_EditSearchPattern;
	CComboBox m_ComboResults;
	std::map<CString, bool> m_mapResults;
	CEdit m_EditSearchByAn;
	CEdit m_EditIdListResult;
	CStatic m_StaticNbFound;
};

#endif
