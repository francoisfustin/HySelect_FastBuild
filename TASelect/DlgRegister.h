#pragma once
#include "afxwin.h"


// CDlgRegister dialog

class CDlgRegister : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgRegister)

public:
	CDlgRegister(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgRegister();

// Dialog Data
	enum { IDD = IDD_DLGREGISTER };

protected:
	union _uDate
	{
		struct {
			unsigned nDay	:	5;	//0..31	(used 1 to 31)
			unsigned nMonth	:	4;	//0..15	(used 1 to 12)
			unsigned nYear	:	7;	//0..127(used 0 to 99 start at 2000)
		}bf;
		USHORT usDate;
	};

	unsigned char m_BFKey[16];
	unsigned char m_encryptedDC[8];
	unsigned char m_encryptedAC[8];
	unsigned char m_encryptedUAC[8];
	unsigned char m_DC[8];
	unsigned char m_AC[8];

	bool		  m_bMachineDependant;
	CTADatabase		*m_pTADB;

	// Initialise all variable
	// return true if already registered

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	CEdit m_EditAC1;
	CEdit m_EditAC2;
	CEdit m_EditAC3;
	CEdit m_EditAC4;
	CEdit m_EditDC;
	CString m_Regstr;
	CEnBitmap m_BitmapTaLogo;
	afx_msg void OnEnChangeEditac1();
	afx_msg void OnEnChangeEditac2();
	afx_msg void OnEnChangeEditac3();
	afx_msg void OnEnChangeEditac4();
	virtual BOOL OnInitDialog();
	void SetDC();
	void SetAC(USHORT nYear =1, BYTE nMonth=3);
	bool VerifyInputChar(CEdit *pe);
	bool VerifyUserAC();
	void CreateRegKeyIfNeeded();
	void UpdateRegistry();

public:
	long Init();
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
