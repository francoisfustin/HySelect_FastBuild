#pragma once


class CDlgEditString : public CDialogEx
{
public:
	enum { IDD = IDD_DLGEDITSTRING };

	CDlgEditString( CWnd* pParent = NULL );
	int Display( CString &strTitle, CString &strStatic, CString *pstrName );

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual void OnOK();
	virtual BOOL OnInitDialog();

// Private variables.
private:
	CStatic		m_strStaticName;
	CEdit		m_Edit;
	CString		m_strTitle;
	CString		m_strStatic;
	CString*	m_pstrName;
};