
#pragma once

// DlgUserRef.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgUserRef dialog

class CDlgUserRef : public CDialogEx
{
// Construction
public:
	CDlgUserRef(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DLGUSERREF };
		// NOTE: the ClassWizard will add data members here
	
	CEdit m_EditName;
	CEdit m_EditCompany;
	CEdit m_EditAddress;
	CEdit m_EditZip;
	CEdit m_EditTown;
	CEdit m_EditCountry;
	CEdit m_EditTel;
	CEdit m_EditFax;
	CEdit m_EditEMail;
	CEdit m_EditOther;
	CEdit m_EditCustoPath;
	CButton m_LoadCustoLogo;


// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtClickLoadLogo();
	DECLARE_MESSAGE_MAP()
	CUserDatabase* m_pUserDB;
public:
	int Display();

};