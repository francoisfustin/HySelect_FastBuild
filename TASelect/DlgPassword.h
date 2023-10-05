#pragma once


class CDlgPassword : public CDialogEx
{
public:
	enum { IDD = IDD_DLGPASSWORD };

	CDlgPassword( CWnd *pParent = NULL );
	
// Public variables.
public:
	CEdit m_EditPassWd;
	
// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};