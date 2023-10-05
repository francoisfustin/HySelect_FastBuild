#pragma once
#include "afxwin.h"


// CDlgImportHMProp dialog

class CDlgImportHMProp : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgImportHMProp)

public:
	
	typedef struct sCheckBoxState
	{
		bool bUseCheckBoxState;
		bool bDistPipes;
		bool bCircPipes;
		bool bBv;
		bool bBvByp;
		bool bDpC;
		bool bCv;
	};

	CDlgImportHMProp(sCheckBoxState sCBState, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgImportHMProp();

	// Opens the dialog window
	// Return the new sCheckBoxState
	sCheckBoxState Display()
	{
		DoModal();
		return m_sCheckBoxState;
	};

// DialogEx Data
	enum { IDD = IDD_DLGIMPORTHMPROP };

protected:

	CStatic			m_StaticTitle1;
	CStatic			m_StaticTitle2;
	CButton			m_CBDistPipes;
	CButton			m_CBCircPipes;
	CButton			m_CBBv;
	CButton			m_CBBvByp;
	CButton			m_CBDpC;
	CButton			m_CBCv;
	sCheckBoxState	m_sCheckBoxState;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOK();
	afx_msg void OnBnClickedBtnLockAll();
	afx_msg void OnBnClickedBtnUnlockAll();
};
