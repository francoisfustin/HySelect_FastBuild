
#pragma once

#include "XGroupBox.h"

// DlgPrjInfo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgDispRef dialog

class CDlgDispRef : public CDialogEx
{
// Construction
public:
	CDlgDispRef(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DLGDISPREF };
		// NOTE: the ClassWizard will add data members here


// Overrides
	// ClassWizard generated virtual function overrides
	protected:

		CXGroupBox	m_GroupProject;
		CXGroupBox	m_GroupRef;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
public:
	int Display();

protected:
	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};