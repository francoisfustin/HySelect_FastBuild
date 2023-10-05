#pragma once


#include "AfxDialogEx.h"
#include "ExtStatic.h"
#include "afxwin.h"

class CDlgDpSVisioDFUInstallation : public CDialogEx
{
public:
	enum { IDD = IDD_DLGDPSVISIO_DFUINSTALLATION };

	CDlgDpSVisioDFUInstallation( CWnd *pParent = NULL );
	virtual ~CDlgDpSVisioDFUInstallation() {}

// Public methods.
public:
	void SetText( CString strText, COLORREF TextColor, int iSize, bool bBold );
	bool OKPressed( void ) { return m_bOKPressed; }

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();

// Private variables.
private:
	CExtStatic m_StaticInfo;
	CButton m_ButtonOK;
	bool m_bOKPressed;
};
