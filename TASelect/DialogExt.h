#pragma once


#define AFX_MAX_CLASS_NAME 255
#define AFX_STATIC_CLASS _T("Static")
#define AFX_EDIT_CLASS _T("Edit")
#define AFX_BUTTON_CLASS _T("Button")
#define	AFX_SLIDER_CLASS _T("msctls_trackbar32")
#define	AFX_COMBOBOX_CLASS _T("ComboBox")

class CDialogExt : public CDialogEx
{
	DECLARE_DYNCREATE( CDialogExt )

public:
	CDialogExt( UINT nID = 0, CWnd *pParent = NULL, bool bUseThemeColor = true );
	virtual ~CDialogExt() {}

	virtual void SetApplicationBackground( COLORREF cBackColor ) { CDialogEx::SetBackgroundColor( cBackColor ); }
	virtual void OnApplicationLook( COLORREF cBackColor ) {}

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor( CDC *pDC, CWnd *pWnd, UINT nCtlColor );
	afx_msg BOOL OnEraseBkgnd( CDC *pDC );
	afx_msg void OnMove( int x, int y );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	void SetLocalizedText( int nControlID, UINT uiStringID );
	void SetLocalizedText( CButton &button, UINT uiStringID );
	void SetLocalizedTitle( UINT uiStringID );

// Protected variables.
protected:
	bool m_bInitialized;
	CString m_strSectionName;
};

