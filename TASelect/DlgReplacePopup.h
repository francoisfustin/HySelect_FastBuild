#pragma once


#include "XGroupBox.h"

class CDlgReplacePopup : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgReplacePopup)

public:
	CDlgReplacePopup( CWnd* pParent = NULL );   // standard constructor
	virtual ~CDlgReplacePopup();

// DialogEx Data
	enum { IDD = IDD_DLGREPLACEPOPUP };

	// Call this function from a DlgCtrlPropPage to update the text
	void UpdateData();
	// Call this function from a DlgCtrlPropPage to disable the "Find Next" button
	void DisableFindNextBtn();
	bool IsFindNextBtnDisable(){return m_BtnFindNext.IsWindowEnabled()?false:true;};


	void SetGroupFind(CString str) { m_strGroupFind = str; };
	void SetGroupReplace(CString str) { m_strGroupReplace = str; };
	void SetStaticFind1(CString str) { m_strStaticFind1 = str; };
	void SetStaticFind2(CString str) { m_strStaticFind2 = str; };
	void SetStaticReplace(CString str) { m_strStaticReplace = str; };


	afx_msg void OnBnClickedBtnClose();

protected:
	CWnd *m_pParent;
	CString m_strGroupFind;
	CString m_strGroupReplace;
	CString m_strStaticFind1;
	CString m_strStaticFind2;
	CString m_strStaticReplace;

	CStatic		m_StaticFind1;
	CStatic		m_StaticFind2;
	CStatic		m_StaticReplace;
	CXGroupBox	m_GroupFind;
	CXGroupBox	m_GroupReplace;
	CButton		m_BtnFindNext;
	CButton		m_BtnReplace;
	CButton		m_BtnReplaceAll;
	CButton		m_BtnClose;


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnFindnext();
	afx_msg void OnBnClickedBtnReplace();
	afx_msg void OnBnClickedBtnReplaceall();
};
