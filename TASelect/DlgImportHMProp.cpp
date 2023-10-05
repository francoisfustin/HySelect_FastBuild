// DlgImportHMProp.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"

#include "DlgImportHMProp.h"


// CDlgImportHMProp dialog

IMPLEMENT_DYNAMIC(CDlgImportHMProp, CDialogEx)

CString SectionName = L"DlgImportHMProp";

CDlgImportHMProp::CDlgImportHMProp(sCheckBoxState sCBState, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgImportHMProp::IDD, pParent)
{
	m_sCheckBoxState = sCBState;
}

CDlgImportHMProp::~CDlgImportHMProp()
{
}

BOOL CDlgImportHMProp::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	// Initial strings
	CString str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_CAPTION);
	SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_STTITLE1);
	m_StaticTitle1.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_STTITLE2);
	m_StaticTitle2.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_CBDISTPIPES);
	m_CBDistPipes.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_CBCIRCPIPES);
	m_CBCircPipes.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_CBBV);
	m_CBBv.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_CBBVBYP);
	m_CBBvByp.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_CBDPC);
	m_CBDpC.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_CBCV);
	m_CBCv.SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_LOCKALL);
	GetDlgItem(IDB_TNLOCKALL)->SetWindowText(str);
	str = TASApp.LoadLocalizedString(IDS_DLGIMPORTHMPROP_UNLOCKALL);
	GetDlgItem(IDB_TNUNLOCKALL)->SetWindowText(str);

	// Recuperate the parameters from the current state
	if (m_sCheckBoxState.bUseCheckBoxState)
	{
		m_CBDistPipes.SetCheck(m_sCheckBoxState.bDistPipes);
		m_CBCircPipes.SetCheck(m_sCheckBoxState.bCircPipes);
		m_CBBv.SetCheck(m_sCheckBoxState.bBv);
		m_CBBvByp.SetCheck(m_sCheckBoxState.bBvByp);
		m_CBDpC.SetCheck(m_sCheckBoxState.bDpC);
		m_CBCv.SetCheck(m_sCheckBoxState.bCv);
	}
	// Recuperate the parameters from the registry
	else
	{
		int iValue = ::AfxGetApp()->GetProfileInt(SectionName,L"DistPipes",0);
		m_CBDistPipes.SetCheck(iValue);
		iValue = ::AfxGetApp()->GetProfileInt(SectionName,L"CircPipes",0);
		m_CBCircPipes.SetCheck(iValue);
		iValue = ::AfxGetApp()->GetProfileInt(SectionName,L"Bv",0);
		m_CBBv.SetCheck(iValue);
		iValue = ::AfxGetApp()->GetProfileInt(SectionName,L"BvByp",0);
		m_CBBvByp.SetCheck(iValue);
		iValue = ::AfxGetApp()->GetProfileInt(SectionName,L"DpC",0);
		m_CBDpC.SetCheck(iValue);
		iValue = ::AfxGetApp()->GetProfileInt(SectionName,L"Cv",0);
		m_CBCv.SetCheck(iValue);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgImportHMProp::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CHECKUNCHECK, m_StaticTitle1);
	DDX_Control(pDX, IDC_STATIC2_CHECKUNCHECK, m_StaticTitle2);
	DDX_Control(pDX, IDC_CHECK_DISTPIPES, m_CBDistPipes);
	DDX_Control(pDX, IDC_CHECK_CIRCPIPES, m_CBCircPipes);
	DDX_Control(pDX, IDC_CHECK_BV, m_CBBv);
	DDX_Control(pDX, IDC_CHECK_BVBYP, m_CBBvByp);
	DDX_Control(pDX, IDC_CHECK_DPC, m_CBDpC);
	DDX_Control(pDX, IDC_CHECK_CV, m_CBCv);
}


BEGIN_MESSAGE_MAP(CDlgImportHMProp, CDialogEx)
	ON_BN_CLICKED(IDOK, OnBnClickedOK)
	ON_BN_CLICKED(IDB_TNLOCKALL, OnBnClickedBtnLockAll)
	ON_BN_CLICKED(IDB_TNUNLOCKALL, OnBnClickedBtnUnlockAll)
END_MESSAGE_MAP()


// CDlgImportHMProp message handlers
void CDlgImportHMProp::OnBnClickedOK()
{
	// Save parameters into the current HM
	if (m_sCheckBoxState.bUseCheckBoxState)
	{
		m_sCheckBoxState.bDistPipes = m_CBDistPipes.GetCheck()?1:0;
		m_sCheckBoxState.bCircPipes = m_CBCircPipes.GetCheck()?1:0;
		m_sCheckBoxState.bBv = m_CBBv.GetCheck()?1:0;
		m_sCheckBoxState.bBvByp = m_CBBvByp.GetCheck()?1:0;
		m_sCheckBoxState.bDpC = m_CBDpC.GetCheck()?1:0;
		m_sCheckBoxState.bCv = m_CBCv.GetCheck()?1:0;
	}
	// Save parameters into the registry
	else
	{
		::AfxGetApp()->WriteProfileInt(SectionName,L"DistPipes",m_CBDistPipes.GetCheck());
		::AfxGetApp()->WriteProfileInt(SectionName,L"CircPipes",m_CBCircPipes.GetCheck());
		::AfxGetApp()->WriteProfileInt(SectionName,L"Bv",m_CBBv.GetCheck());
		::AfxGetApp()->WriteProfileInt(SectionName,L"BvByp",m_CBBvByp.GetCheck());
		::AfxGetApp()->WriteProfileInt(SectionName,L"DpC",m_CBDpC.GetCheck());
		::AfxGetApp()->WriteProfileInt(SectionName,L"Cv",m_CBCv.GetCheck());
	}

	// Do the default function
	CDialogEx::OnOK();
}

void CDlgImportHMProp::OnBnClickedBtnLockAll()
{
	m_CBDistPipes.SetCheck(true);
	m_CBCircPipes.SetCheck(true);
	m_CBBv.SetCheck(true);
	m_CBBvByp.SetCheck(true);
	m_CBDpC.SetCheck(true);
	m_CBCv.SetCheck(true);
}

void CDlgImportHMProp::OnBnClickedBtnUnlockAll()
{
	m_CBDistPipes.SetCheck(false);
	m_CBCircPipes.SetCheck(false);
	m_CBBv.SetCheck(false);
	m_CBBvByp.SetCheck(false);
	m_CBDpC.SetCheck(false);
	m_CBCv.SetCheck(false);
}