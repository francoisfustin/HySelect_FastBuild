#pragma once
#include "afxwin.h"
// DlgQuickMeasNote dialog

class DlgQuickMeasNote : public CDialogEx
{
	DECLARE_DYNAMIC(DlgQuickMeasNote)

public:
	enum { IDD = IDD_DLGQUICK_MEAS_NOTE };
	DlgQuickMeasNote(CWnd* pParent = NULL);  
	DlgQuickMeasNote(CString strOldNote, CWnd* pParent = NULL);
	virtual ~DlgQuickMeasNote();

	/**
	 * return the last string in the CEdit validated by Ok button
	 * @author awa
	 * @remarks : Created by HYS658
	*/
	CString GetTextNote();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);  
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCancelbuttonnote();
	afx_msg void OnBnClickedOkbuttonnote();
	afx_msg void OnEnChangeEditnote();

private:
	CEdit m_EditNoteOnMeas;
	CString m_OldEditNoteValue;
	CString m_NewEditNoteValue;
	CButton m_ButtonOkNote;
};
