#pragma once


#include "DialogExt.h"

/////////////////////////////////////////////////////////////////////////////
// CTabDlgSpecActUser dialog

class CTabDlgSpecActUser : public CDialogExt
{
// Construction
public:
	CTabDlgSpecActUser(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_TABSPECACT_USER };
	CButton	m_CheckDropSerial;


// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

	// Generated message map functions
	afx_msg void OnCheckdropserial();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
