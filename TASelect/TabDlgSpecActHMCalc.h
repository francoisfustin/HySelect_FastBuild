#pragma once


#include "DialogExt.h"


class CTabDlgSpecActHMCalc : public CDialogExt
{
public:
	enum { IDD = IDD_TABSPECACT_HMCALC };

	CTabDlgSpecActHMCalc( CWnd* pParent = NULL );
	virtual ~CTabDlgSpecActHMCalc();
	
// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange( CDataExchange* pDX );
	afx_msg void OnBnClickedDropHMCalc();
	afx_msg void OnBnClickedDropFile();
	afx_msg void OnEnKillFocusDropFileName();

// Private methods.
private:
	void _SetState( void );

// Private variables.
private:
	CButton m_CheckDropHMCalc;
	CButton m_ButtonDropFile;
	CEdit m_EditDropFileName;
	HICON m_hFolderIcon;
	HBITMAP m_hFolderBitmap;
	CBitmap m_clFolderBitmap;
	CString m_strHMDropFileName;
};
