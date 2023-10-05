#pragma once


#include "afxcmn.h"
#include "afxwin.h"

class CDlgProgressBar : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgProgressBar )

public:
	enum { IDD = IDD_DLG_PROGRESSBAR };

	CDlgProgressBar( CWnd* pParent = NULL, CString Title = _T("") );		// Standard constructor.
	CDlgProgressBar( CWinApp* pWnd, CString Title = _T("") );				// Modeless constructor.
	virtual ~CDlgProgressBar() {}

	virtual int Display( bool fModeless = true );
	void UpdateProgress( int iValue, CString strItemName );
	void SetRange( short nLower, short nUpper );

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();

// Protected variables.
protected:
	CProgressCtrl		m_ProgressBar;
	CString				m_strTitle;
	bool				m_bModeless;
	short				m_nLowerLimitProgBar;
	short				m_nUpperLimitProgBar;
};
