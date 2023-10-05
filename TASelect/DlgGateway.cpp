// DlgGateway.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "Global.h"
#include "DlgGateway.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgGateway dialog


CDlgGateway::CDlgGateway(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGateway::IDD, pParent)
{
}


void CDlgGateway::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIOCHECKAUTO, m_RadioCheckAuto);
	DDX_Control(pDX, IDC_RADIOFIXEDTIME, m_RadioFixedTime);
	DDX_Control(pDX, IDC_RADIONEVER, m_RadioCheckNever);
	DDX_Control(pDX, IDC_CHECKUSEGATEWAY, m_CheckUseGateway);
	DDX_Control(pDX, IDC_CHECKIPFTP, m_CheckIpFtp);
	DDX_Control(pDX, IDC_EDITGATEWAYIPFTP, m_EditGatewayIpFtp);
	DDX_Control(pDX, IDC_EDITPORTFTP, m_EditPortFtp);
	DDX_Control(pDX, IDC_EDITGATEWAYNAMEFTP, m_EditGatewayNameFtp);
	DDX_Control(pDX, IDC_CHECKIPHTTP, m_CheckIpHttp);
	DDX_Control(pDX, IDC_EDITGATEWAYIPHTTP, m_EditGatewayIpHttp);
	DDX_Control(pDX, IDC_EDITPORTHTTP, m_EditPortHttp);
	DDX_Control(pDX, IDC_EDITGATEWAYNAMEHTTP, m_EditGatewayNameHttp);
	DDX_Control(pDX, IDC_GROUPGATEWAY, m_GroupGateWay);
}


BEGIN_MESSAGE_MAP(CDlgGateway, CDialogEx)
	ON_BN_CLICKED(IDC_CHECKUSEGATEWAY, OnCheckusegateway)
	ON_BN_CLICKED(IDC_CHECKIPFTP, OnCheckFtpIP)
	ON_BN_CLICKED(IDC_CHECKIPHTTP, OnCheckHttpIP)
	ON_BN_CLICKED(IDC_RADIOCHECKAUTO, &CDlgGateway::OnBnClickedRadiocheckauto)
	ON_BN_CLICKED(IDC_RADIOFIXEDTIME, &CDlgGateway::OnBnClickedRadiofixedtime)
	ON_BN_CLICKED(IDC_RADIONEVER, &CDlgGateway::OnBnClickedRadionever)
END_MESSAGE_MAP()

int CDlgGateway::Display() 
{
	return DoModal();
}
/////////////////////////////////////////////////////////////////////////////
// CDlgGateway message handlers

BOOL CDlgGateway::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_CAPTION);
	SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_RBFULLAUTO);
	m_RadioCheckAuto.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_RBPROPOSEATFIXTIME);
	m_RadioFixedTime.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_RBNEVERPROPOSE);
	m_RadioCheckNever.SetWindowText(str);

	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_GROUPGATEWAY);
	GetDlgItem(IDC_GROUPGATEWAY)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_CHECKUSEGATEWAY);
	GetDlgItem(IDC_CHECKUSEGATEWAY)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_STATICPROXYNAME);
	GetDlgItem(IDC_STATICPROXYNAME)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_STATICPORTNUMBER);
	GetDlgItem(IDC_STATICPORTNUMBER)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGGATEWAY_CHECKIP);
	GetDlgItem(IDC_CHECKIPFTP)->SetWindowText(str);
	GetDlgItem(IDC_CHECKIPHTTP)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);

	//LoadMappedBitmap for IDC_STATICCONNECT
	m_Bitmap.LoadMappedBitmap(IDB_CONNECT);
	HBITMAP hBitmap = (HBITMAP) m_Bitmap.GetSafeHandle();
	((CStatic*)GetDlgItem(IDC_STATICCONNECT))->SetBitmap(hBitmap);

	// Get Ftp connection parameters from USERDB
	CDB_MultiString* pmsFtpGatewayParams = (CDB_MultiString *) TASApp.GetpUserDB()->Get(_T("FTPGATEWAY_PAR")).MP;
	m_CheckUseGateway.SetCheck(_tcsicmp(pmsFtpGatewayParams->GetString(egwUsed),_T("true"))?0:1);

	// Activate or not the AutoUpdate mode
	// Use value stored into the registry, Auto Update flag can be modified by HyUpdate when the application is running
	int k=TASApp.GetProfileInt(HYUPDATE,AUTOUPDATE,1);
	switch (k)
	{
		case 0: OnBnClickedRadionever();		break;
		case 1: OnBnClickedRadiocheckauto();	break;
		case 2: OnBnClickedRadiofixedtime();	break;
	}
	
	// FTP Gateway name
	str = (CString)pmsFtpGatewayParams->GetString(egwFtpAdd);
	// Is it a IP address or a name ?	
	int pos=0;
	CString substr(str);
	int i,val=0;
	for (i=0; i<4; i++)
	{
		pos=substr.Find('.',pos);
		if (pos<0 || pos>3) break;
		val = _ttoi((LPCTSTR)substr.Left(pos));
		if (val<0 || val>255) break;
		substr=str.Mid(pos+1);
	}
	if (i!=4)	// process aborded not an IP address
		m_CheckIpFtp.SetCheck(false);
	else
		m_CheckIpFtp.SetCheck(true);

	// HTTP Gateway name
	str = (CString)pmsFtpGatewayParams->GetString(egwHttpAdd);
	// Is it a IP address or a name ?	
	pos=0;
	substr=str;
	for (i=0; i<4; i++)
	{
		pos=substr.Find('.',pos);
		if (pos<0 || pos>3) break;
		val = _ttoi((LPCTSTR)substr.Left(pos));
		if (val<0 || val>255) break;
		substr=str.Mid(pos+1);
	}
	if (i!=4)	// process aborded not an IP address
		m_CheckIpHttp.SetCheck(false);
	else
		m_CheckIpHttp.SetCheck(true);


	m_EditGatewayNameFtp.SetWindowText(pmsFtpGatewayParams->GetString(egwFtpAdd));
	m_EditGatewayIpFtp.SetWindowText(pmsFtpGatewayParams->GetString(egwFtpAdd));

	m_EditGatewayNameHttp.SetWindowText(pmsFtpGatewayParams->GetString(egwHttpAdd));
	m_EditGatewayIpHttp.SetWindowText(pmsFtpGatewayParams->GetString(egwHttpAdd));

	m_EditPortFtp.SetWindowText(pmsFtpGatewayParams->GetString(egwFtpPort));
	m_EditPortHttp.SetWindowText(pmsFtpGatewayParams->GetString(egwHttpPort));
	
	OnCheckFtpIP();
	OnCheckHttpIP();
	OnCheckusegateway();

	// Disable the Ftp edit box
	m_EditGatewayNameFtp.ShowWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgGateway::OnCheckusegateway() 
{
	if (m_CheckUseGateway.GetCheck())
	{
		m_CheckIpFtp.EnableWindow(true);
		m_EditGatewayIpFtp.EnableWindow(true);
		m_EditGatewayNameFtp.EnableWindow(true);
		m_EditPortFtp.EnableWindow(true);

		m_CheckIpHttp.EnableWindow(true);
		m_EditGatewayIpHttp.EnableWindow(true);
		m_EditGatewayNameHttp.EnableWindow(true);
		m_EditPortHttp.EnableWindow(true);

		GetDlgItem(IDC_STATICPROXYNAME)->EnableWindow(true);
		GetDlgItem(IDC_STATICPORTNUMBER)->EnableWindow(true);
	}
	else
	{
		m_CheckIpFtp.EnableWindow(false);
		m_EditGatewayNameFtp.EnableWindow(false);
		m_EditGatewayIpFtp.EnableWindow(false);
		m_EditPortFtp.EnableWindow(false);

		m_CheckIpHttp.EnableWindow(false);
		m_EditGatewayIpHttp.EnableWindow(false);
		m_EditGatewayNameHttp.EnableWindow(false);
		m_EditPortHttp.EnableWindow(false);

		GetDlgItem(IDC_STATICPROXYNAME)->EnableWindow(false);
		GetDlgItem(IDC_STATICPORTNUMBER)->EnableWindow(false);
	}
}

void CDlgGateway::OnCheckFtpIP() 
{
	if (m_CheckIpFtp.GetCheck())
	{
	m_EditGatewayIpFtp.ShowWindow(true);
	m_EditGatewayNameFtp.ShowWindow(false);
	}
	else
	{
	m_EditGatewayIpFtp.ShowWindow(false);
	m_EditGatewayNameFtp.ShowWindow(true);
	}
}
void CDlgGateway::OnCheckHttpIP() 
{
	if (m_CheckIpHttp.GetCheck())
	{
	m_EditGatewayIpHttp.ShowWindow(true);
	m_EditGatewayNameHttp.ShowWindow(false);
	}
	else
	{
	m_EditGatewayIpHttp.ShowWindow(false);
	m_EditGatewayNameHttp.ShowWindow(true);
	}
}

void CDlgGateway::OnOK() 
{
	PREVENT_ENTER_KEY
	
	CDB_MultiString* pmsFtpGatewayParams = (CDB_MultiString *) TASApp.GetpUserDB()->Get(_T("FTPGATEWAY_PAR")).MP;
	// Save all parameters in TADS
	if (m_CheckUseGateway.GetCheck())
		pmsFtpGatewayParams->SetString(egwUsed,_T("true"));
	else
		pmsFtpGatewayParams->SetString(egwUsed,_T("false"));

	if (m_RadioCheckAuto.GetCheck())
		pmsFtpGatewayParams->SetString(egwFtpAutoUpdate,_T("true"));
	else if (m_RadioFixedTime.GetCheck())
		pmsFtpGatewayParams->SetString(egwFtpAutoUpdate,_T("fixedtime"));
	else
		pmsFtpGatewayParams->SetString(egwFtpAutoUpdate,_T("false"));

	CString str;
	m_EditPortFtp.GetWindowText(str);
	pmsFtpGatewayParams->SetString(egwFtpPort,(LPCTSTR)str);
	if (!m_CheckIpFtp.GetCheck())
	{
		m_EditGatewayNameFtp.GetWindowText(str);
		pmsFtpGatewayParams->SetString(egwFtpAdd,(LPCTSTR)str);
	}
	else
	{
		m_EditGatewayIpFtp.GetWindowText(str);
		pmsFtpGatewayParams->SetString(egwFtpAdd,(LPCTSTR)str);
	}

	m_EditPortHttp.GetWindowText(str);
	pmsFtpGatewayParams->SetString(egwHttpPort,(LPCTSTR)str);
	if (!m_CheckIpHttp.GetCheck())
	{
		m_EditGatewayNameHttp.GetWindowText(str);
		pmsFtpGatewayParams->SetString(egwHttpAdd,(LPCTSTR)str);
	}
	else
	{
		m_EditGatewayIpHttp.GetWindowText(str);
		pmsFtpGatewayParams->SetString(egwHttpAdd,(LPCTSTR)str);
	}

	// Update Registry	
	TASApp.WriteProfileInt(HYUPDATE,GATEWAYUSED,_tcsicmp(pmsFtpGatewayParams->GetString(egwUsed),_T("true"))?0:1);
	TASApp.WriteProfileString(HYUPDATE,FTPGATEWAY,pmsFtpGatewayParams->GetString(egwFtpAdd));
	TASApp.WriteProfileString(HYUPDATE,FTPGATEWAYPSW,pmsFtpGatewayParams->GetString(egwFtpPsw));
	TASApp.WriteProfileInt(HYUPDATE,FTPPORT,*pmsFtpGatewayParams->GetString(egwFtpPort)?_ttoi(pmsFtpGatewayParams->GetString(egwFtpPort)):21);
	if (!_tcsicmp(pmsFtpGatewayParams->GetString(egwFtpAutoUpdate),_T("true")))
		TASApp.WriteProfileInt(HYUPDATE,AUTOUPDATE,1);
	else if(!_tcsicmp(pmsFtpGatewayParams->GetString(egwFtpAutoUpdate),_T("fixedtime")))
		TASApp.WriteProfileInt(HYUPDATE,AUTOUPDATE,2);
	else
		TASApp.WriteProfileInt(HYUPDATE,AUTOUPDATE,0);

	
	TASApp.WriteProfileString(HYUPDATE,HTTPGATEWAY,pmsFtpGatewayParams->GetString(egwHttpAdd));
	TASApp.WriteProfileString(HYUPDATE,HTTPGATEWAYPSW,pmsFtpGatewayParams->GetString(egwHttpPsw));
	TASApp.WriteProfileInt(HYUPDATE,HTTPPORT,*pmsFtpGatewayParams->GetString(egwHttpPort)?_ttoi(pmsFtpGatewayParams->GetString(egwHttpPort)):80);

	CDialogEx::OnOK();
}


void CDlgGateway::OnBnClickedRadiocheckauto()
{
	m_RadioCheckAuto.SetCheck(true);
	m_RadioFixedTime.SetCheck(false);
	m_RadioCheckNever.SetCheck(false);
}

void CDlgGateway::OnBnClickedRadiofixedtime()
{
	m_RadioCheckAuto.SetCheck(false);
	m_RadioFixedTime.SetCheck(true);
	m_RadioCheckNever.SetCheck(false);
}

void CDlgGateway::OnBnClickedRadionever()
{
	m_RadioCheckAuto.SetCheck(false);
	m_RadioFixedTime.SetCheck(false);
	m_RadioCheckNever.SetCheck(true);
}
