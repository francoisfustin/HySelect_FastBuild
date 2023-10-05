// DlgUserRef.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "Global.h"

#include "DlgUserRef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgUserRef dialog


CDlgUserRef::CDlgUserRef(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgUserRef::IDD, pParent)
{
	m_pUserDB = NULL;
}


void CDlgUserRef::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITNAME, m_EditName);
	DDX_Control(pDX, IDC_EDITCOMPANY, m_EditCompany);
	DDX_Control(pDX, IDC_EDITADDRESS, m_EditAddress);
	DDX_Control(pDX, IDC_EDITZIP, m_EditZip);
	DDX_Control(pDX, IDC_EDITTOWN, m_EditTown);
	DDX_Control(pDX, IDC_EDITCOUNTRY, m_EditCountry);
	DDX_Control(pDX, IDC_EDITTEL, m_EditTel);
	DDX_Control(pDX, IDC_EDITFAX, m_EditFax);
	DDX_Control(pDX, IDC_EDITEMAIL, m_EditEMail);
	DDX_Control(pDX, IDC_EDITOTHER, m_EditOther);
	DDX_Control(pDX, IDC_EDITPATH, m_EditCustoPath);
	DDX_Control( pDX, IDC_BUTTONCUSTO, m_LoadCustoLogo );
}


BEGIN_MESSAGE_MAP(CDlgUserRef, CDialogEx)
	ON_BN_CLICKED( IDC_BUTTONCUSTO, OnBtClickLoadLogo )
END_MESSAGE_MAP()


int CDlgUserRef::Display()
{
	return DoModal();
}



/////////////////////////////////////////////////////////////////////////////
// CDlgUserRef message handlers

BOOL CDlgUserRef::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_CAPTION);
	SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICNAME);
	GetDlgItem(IDC_STATICNAME)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICCOMPANY);
	GetDlgItem(IDC_STATICCOMPANY)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICADDRESS);
	GetDlgItem(IDC_STATICADDRESS)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICZIP);
	GetDlgItem(IDC_STATICZIP)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICTOWN);
	GetDlgItem(IDC_STATICTOWN)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICCOUNTRY);
	GetDlgItem(IDC_STATICCOUNTRY)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICTEL);
	GetDlgItem(IDC_STATICTEL)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICFAX);
	GetDlgItem(IDC_STATICFAX)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICEMAIL);
	GetDlgItem(IDC_STATICEMAIL)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGUSERREF_STATICOTHER);
	GetDlgItem(IDC_STATICOTHER)->SetWindowText(str);
	// HYS-1090
	str = TASApp.LoadLocalizedString( IDS_DLGUSERREF_CUSTOPICT );
	GetDlgItem( IDC_STATICCUSTO )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGPANELDOWNLOAD_DOWNLOAD );
	GetDlgItem( IDC_BUTTONCUSTO )->SetWindowText( str );

	str=TASApp.LoadLocalizedString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);
	str.Empty();

	// Load USERDB to initialize Edit boxes
	m_pUserDB  = TASApp.GetpUserDB();

	CDS_UserRef *pUserRef = (CDS_UserRef*)m_pUserDB->Get(_T("USER_REF")).MP;
	ASSERT( NULL != pUserRef );
	
	m_EditName.SetWindowText( pUserRef->GetString(CDS_UserRef::Name ) );
    m_EditCompany.SetWindowText( pUserRef->GetString( CDS_UserRef::Company ) );
    m_EditAddress.SetWindowText( pUserRef->GetString( CDS_UserRef::Address ) );
    m_EditZip.SetWindowText( pUserRef->GetString( CDS_UserRef::Zip ) );
    m_EditTown.SetWindowText( pUserRef->GetString( CDS_UserRef::Town ) );
    m_EditCountry.SetWindowText( pUserRef->GetString( CDS_UserRef::Country ) );
    m_EditTel.SetWindowText( pUserRef->GetString( CDS_UserRef::Tel ) );
    m_EditFax.SetWindowText( pUserRef->GetString( CDS_UserRef::Fax ) );
    m_EditEMail.SetWindowText( pUserRef->GetString( CDS_UserRef::Email ) );
    m_EditOther.SetWindowText( pUserRef->GetString( CDS_UserRef::Other ) );
    m_EditCustoPath.SetWindowText( pUserRef->GetString( CDS_UserRef::Path ) );

	// Define the number of characters that the 
	// user can input into the comment edit boxes
	UINT uiMaxChar = 250;

	m_EditName.SetLimitText(uiMaxChar);
	m_EditCompany.SetLimitText(uiMaxChar);
	m_EditAddress.SetLimitText(uiMaxChar);
	m_EditZip.SetLimitText(uiMaxChar);
	m_EditTown.SetLimitText(uiMaxChar);
	m_EditCountry.SetLimitText(uiMaxChar);
	m_EditTel.SetLimitText(uiMaxChar);
	m_EditFax.SetLimitText(uiMaxChar);
	m_EditEMail.SetLimitText(uiMaxChar);
	m_EditOther.SetLimitText(uiMaxChar*10);    
	m_EditCustoPath.SetLimitText(uiMaxChar);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgUserRef::OnOK() 
{
	PREVENT_ENTER_KEY
	CDS_UserRef* pUserRef = (CDS_UserRef*) m_pUserDB->Get(_T("USER_REF")).MP;				ASSERT(pUserRef);
	
	// Save all fields into TADS
	CString str;

	m_EditName.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Name,(LPCTSTR)str);
    m_EditCompany.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Company,(LPCTSTR)str);
    m_EditAddress.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Address,(LPCTSTR)str);
    m_EditZip.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Zip,(LPCTSTR)str);
    m_EditTown.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Town,(LPCTSTR)str);
    m_EditCountry.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Country,(LPCTSTR)str);
    m_EditTel.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Tel,(LPCTSTR)str);
    m_EditFax.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Fax,(LPCTSTR)str);
    m_EditEMail.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Email,(LPCTSTR)str);
    m_EditOther.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Other,(LPCTSTR)str);
	m_EditCustoPath.GetWindowText(str);pUserRef->SetString(CDS_UserRef::Path,(LPCTSTR)str);
 	
	CDialogEx::OnOK();
}

void CDlgUserRef::OnBtClickLoadLogo()
{
	CString strFilter = _T("(*.bmp;*.jpeg;*.jpg;*.ico;*.gif)|*.bmp;*.jpeg;*.jpg;*.ico;*.gif||");
	CFileDialog dlg( false, _T( "" ), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXTENSIONDIFFERENT | OFN_PATHMUSTEXIST, strFilter );
	if( IDOK == dlg.DoModal() )
	{
		// Load Bmp
		CString filePath = (LPCTSTR)dlg.GetPathName();
		m_EditCustoPath.SetWindowTextW( filePath );
	}
}