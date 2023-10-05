
#pragma once

#include "XGroupBox.h"

// DlgGateway.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgGateway dialog

class CDlgGateway : public CDialogEx
{
// Construction
public:
	CDlgGateway(CWnd* pParent = NULL);   // standard constructor

	enum eGateway
	{
		// line 0 AutoUpdate
		// line 1 Gateway used or not
		// line 2 Gateway FTP Server address
		// line 3 Gateway FTP password
		// line 4 Gateway FTP port number
		// line 5 Gateway HTTP address
		// line 6 Gateway HTTP password
		// line 7 Gateway HTTP port number
		egwFtpAutoUpdate,
		egwUsed,
		egwFtpAdd,
		egwFtpPsw,
		egwFtpPort,
		egwHttpAdd,
		egwHttpPsw,
		egwHttpPort,
		egwLast
	};
// Dialog Data
	enum { IDD = IDD_DLGGATEWAY };
	CButton			m_RadioCheckAuto;
	CButton			m_RadioFixedTime;
	CButton			m_RadioCheckNever;
	CButton			m_CheckUseGateway;
	CButton			m_CheckIpFtp;
	CIPAddressCtrl	m_EditGatewayIpFtp;
	CEdit			m_EditPortFtp;
	CEdit			m_EditGatewayNameFtp;
	CButton			m_CheckIpHttp;
	CIPAddressCtrl	m_EditGatewayIpHttp;
	CEdit			m_EditPortHttp;
	CEdit			m_EditGatewayNameHttp;
	CXGroupBox		m_GroupGateWay;


// Overrides
	// ClassWizard generated virtual function overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation

public:
	int CDlgGateway::Display();
	
protected:
	CBitmap m_Bitmap;

	// Generated message map functions
	afx_msg void OnCheckusegateway();
	afx_msg void OnCheckFtpIP();
	afx_msg void OnCheckHttpIP();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadiocheckauto();
public:
	afx_msg void OnBnClickedRadiofixedtime();
public:
	afx_msg void OnBnClickedRadionever();
};