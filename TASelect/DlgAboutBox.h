#pragma once
#include "ModuleVersion.h"
#include "TASelect.h"
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

// Implémentation
protected:
	CFont m_font;
	CBitmap m_TASLogo;
	CEnBitmap m_TALogo;
	CSetDPI dpi;

	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedLicence();
};

