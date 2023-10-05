
#pragma once

#include "CbiRestrEdit.h"
#include "afxwin.h"
#include "XGroupBox.h"

// DlgRef.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgRef dialog


class CDlgRef : public CDialogEx
{
// Construction
public:
	CDlgRef(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_DLGREF };
	CCbiRestrEdit	m_EditPProject;
	CEdit			m_EditFName;
	CEdit			m_EditFCompany;
	CEdit			m_EditFAdress;
    CEdit			m_EditFZip;
    CEdit			m_EditFTown;
    CEdit			m_EditFCountry;
    CEdit			m_EditFTel;
    CEdit			m_EditFFax;
    CEdit			m_EditFEMail;
    CEdit			m_EditFOther;    
	CEdit			m_EditPComments;
    CEdit			m_EditPRef;
    CEdit			m_EditPName;
	CEdit			m_EditPCompany;
    CEdit			m_EditPAdress;
    CEdit			m_EditPZip;
    CEdit			m_EditPTown;
    CEdit			m_EditPCountry;
    CEdit			m_EditPEMail;
	CEdit			m_EditPTel;
    CEdit			m_EditPFax;
	CXGroupBox		m_GroupSelBy;
	CXGroupBox		m_GroupPrj;
	CXGroupBox		m_GroupCustommer;

// Overrides
	// ClassWizard generated virtual function overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
public:
	int Display();
protected:

	CTADatastruct* m_pTADS;
	CUserDatabase* m_pUserDB;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonsetuserref();
	DECLARE_MESSAGE_MAP()	
};