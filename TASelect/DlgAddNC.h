
#pragma once

// DlgAddNC.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAddNC dialog

class CDlgAddNC : public CDialogEx
{
// Construction
public:
	CDlgAddNC(CWnd* pParent = NULL);   // standard constructor
	int Display();
	CString GetRadNCID(){return m_strRadNCID;};
// Dialog Data
	enum { IDD = IDD_DLGADDNC };
	CEdit	m_EditTs;
	CEdit	m_EditTr;
	CEdit	m_EditTi;


// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditti();
	afx_msg void OnChangeEdittr();
	afx_msg void OnChangeEditts();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()

	double m_dTs;
	double m_dTr;
	double m_dTi;
	CString m_strRadNCID;
	CUserDatabase* m_pUSERDB;

	bool CheckTemp(CEdit *pEd,double &value);

};