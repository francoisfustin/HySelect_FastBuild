#pragma once


// CDlgRibbonDateTime dialog

class CDlgRibbonDateTime : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgRibbonDateTime)

public:
	CDlgRibbonDateTime(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgRibbonDateTime();

// Dialog Data
	enum { IDD = IDD_DLGRIBBONDATETIME };

protected:

// Variables
	CDateTimeCtrl m_DatePickStart;
	CDateTimeCtrl m_DatePickEnd;
	CDateTimeCtrl m_TimePickStart;
	CDateTimeCtrl m_TimePickEnd;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOK();
	afx_msg void OnBnClickedReset();
};
