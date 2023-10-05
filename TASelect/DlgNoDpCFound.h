//
// DlgNoDevFound.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CDlgNoDpCFound dialog

class CDlgNoDpCFound : public CDialogEx
{
// Construction
public:
	CDlgNoDpCFound(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DLGNODPCFOUND };

// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

public:
	// Opens the dialog window.
	// Returns the same value as domodal().
	int Display(double DpToStab,int DpStab,int MvLoc);

protected:
	double m_dDpToStab;
	int m_iDpStab;
	int m_iMvLoc;
};