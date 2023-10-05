#include "stdafx.h"
#include "TASelect.h"
#include "global.h"

#include "DlgRef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgRef::CDlgRef( CWnd* pParent )
	: CDialogEx( CDlgRef::IDD, pParent )
{
	m_pTADS = NULL;
	m_pUserDB = NULL;
}

void CDlgRef::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITFNAME, m_EditFName);
	DDX_Control(pDX, IDC_EDITFCOMPANY, m_EditFCompany);
	DDX_Control(pDX, IDC_EDITFADDRESS, m_EditFAdress);
	DDX_Control(pDX, IDC_EDITFZIP, m_EditFZip);
	DDX_Control(pDX, IDC_EDITFTOWN, m_EditFTown);
	DDX_Control(pDX, IDC_EDITFCOUNTRY, m_EditFCountry);
	DDX_Control(pDX, IDC_EDITFTEL, m_EditFTel);
	DDX_Control(pDX, IDC_EDITFFAX, m_EditFFax);
	DDX_Control(pDX, IDC_EDITFEMAIL, m_EditFEMail);
	DDX_Control(pDX, IDC_EDITFOTHER, m_EditFOther);
	DDX_Control(pDX, IDC_EDITPPROJECT, m_EditPProject);
	DDX_Control(pDX, IDC_EDITPCOMMENTS, m_EditPComments);
	DDX_Control(pDX, IDC_EDITPREF, m_EditPRef);
	DDX_Control(pDX, IDC_EDITPNAME, m_EditPName);
	DDX_Control(pDX, IDC_EDITPCOMPANY, m_EditPCompany);
	DDX_Control(pDX, IDC_EDITPADDRESS, m_EditPAdress);
	DDX_Control(pDX, IDC_EDITPZIP, m_EditPZip);
	DDX_Control(pDX, IDC_EDITPTOWN, m_EditPTown);
	DDX_Control(pDX, IDC_EDITPCOUNTRY, m_EditPCountry);
	DDX_Control(pDX, IDC_EDITPEMAIL, m_EditPEMail);
	DDX_Control(pDX, IDC_EDITPTEL, m_EditPTel);
	DDX_Control(pDX, IDC_EDITPFAX, m_EditPFax);
	DDX_Control(pDX, IDC_STATICSELBY, m_GroupSelBy);
	DDX_Control(pDX, IDC_STATICPRJ, m_GroupPrj);
	DDX_Control(pDX, IDC_STATICCUSTOMER, m_GroupCustommer);
}

BEGIN_MESSAGE_MAP(CDlgRef, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTONSETUSERREF, OnButtonsetuserref)
END_MESSAGE_MAP()

int CDlgRef::Display()
{
	return DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRef message handlers

BOOL CDlgRef::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	// Initialize dialog strings
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGREF_CAPTION);
	SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICSELBY);
	m_GroupSelBy.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFNAME);
	GetDlgItem(IDC_STATICFNAME)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFCOMPANY);
	GetDlgItem(IDC_STATICFCOMPANY)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFADDRESS);
	GetDlgItem(IDC_STATICFADDRESS)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFZIP);
	GetDlgItem(IDC_STATICFZIP)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFTOWN);
	GetDlgItem(IDC_STATICFTOWN)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFCOUNTRY);
	GetDlgItem(IDC_STATICFCOUNTRY)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFTEL);
	GetDlgItem(IDC_STATICFTEL)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFFAX);
	GetDlgItem(IDC_STATICFFAX)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFEMAIL);
	GetDlgItem(IDC_STATICFEMAIL)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICFOTHER);
	GetDlgItem(IDC_STATICFOTHER)->SetWindowText(str);

	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPRJ);
	m_GroupPrj.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPPRJNAME);
	GetDlgItem(IDC_STATICPPRJNAME)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPCOMMENTS);
	GetDlgItem(IDC_STATICPCOMMENTS)->SetWindowText(str);

	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICCUSTOMER);
	m_GroupCustommer.SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPREF);
	GetDlgItem(IDC_STATICPREF)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPNAME);
	GetDlgItem(IDC_STATICPNAME)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPCOMPANY);
	GetDlgItem(IDC_STATICPCOMPANY)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPADDRESS);
	GetDlgItem(IDC_STATICPADDRESS)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPZIP);
	GetDlgItem(IDC_STATICPZIP)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPTOWN);
	GetDlgItem(IDC_STATICPTOWN)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPCOUNTRY);
	GetDlgItem(IDC_STATICPCOUNTRY)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPEMAIL);
	GetDlgItem(IDC_STATICPEMAIL)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPTEL);
	GetDlgItem(IDC_STATICPTEL)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREF_STATICPFAX);
	GetDlgItem(IDC_STATICPFAX)->SetWindowText(str);
	
	str=TASApp.LoadLocalizedString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);
	str.Empty();

	m_pTADS = TASApp.GetpTADS();
	m_pUserDB  = TASApp.GetpUserDB();

	CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();
	ASSERT( NULL != pPrjRef );
	
	CDS_UserRef *pUserRef = m_pTADS->GetpUserRef();
	ASSERT( NULL != pUserRef );

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT ( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		( (CButton *)GetDlgItem( IDC_BUTTONSETUSERREF ) )->SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_SetUserRef));
	}

	m_EditFName.SetWindowText(pUserRef->GetString(CDS_UserRef::Name));
	m_EditFCompany.SetWindowText(pUserRef->GetString(CDS_UserRef::Company));
	m_EditFAdress.SetWindowText(pUserRef->GetString(CDS_UserRef::Address));
	m_EditFZip.SetWindowText(pUserRef->GetString(CDS_UserRef::Zip));
	m_EditFTown.SetWindowText(pUserRef->GetString(CDS_UserRef::Town));
	m_EditFCountry.SetWindowText(pUserRef->GetString(CDS_UserRef::Country));
	m_EditFTel.SetWindowText(pUserRef->GetString(CDS_UserRef::Tel));
	m_EditFFax.SetWindowText(pUserRef->GetString(CDS_UserRef::Fax));
	m_EditFEMail.SetWindowText(pUserRef->GetString(CDS_UserRef::Email));
	m_EditFOther.SetWindowText(pUserRef->GetString(CDS_UserRef::Other));
	
	m_EditPProject.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::Name));
	m_EditPComments.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::Comment));
	m_EditPRef.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustRef));
	m_EditPName.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustName));
	m_EditPAdress.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustAddress));
	m_EditPZip.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustZip));
	m_EditPTown.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustTown));
	m_EditPTel.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustTel));
	m_EditPFax.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustFax));
	m_EditPCountry.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustCountry));
	m_EditPEMail.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustEmail));
	m_EditPCompany.SetWindowText(pPrjRef->GetString(CDS_ProjectRef::CustCompany));

	// Define the number of characters that the 
	// user can input into the comment edit boxes
	UINT uiMaxChar = 250;

	m_EditFName.SetLimitText(uiMaxChar);
	m_EditFCompany.SetLimitText(uiMaxChar);
	m_EditFAdress.SetLimitText(uiMaxChar);
	m_EditFZip.SetLimitText(uiMaxChar);
	m_EditFTown.SetLimitText(uiMaxChar);
	m_EditFCountry.SetLimitText(uiMaxChar);
	m_EditFTel.SetLimitText(uiMaxChar);
	m_EditFFax.SetLimitText(uiMaxChar);
	m_EditFEMail.SetLimitText(uiMaxChar);
	m_EditFOther.SetLimitText(uiMaxChar*10);    
	m_EditPProject.SetLimitText(TASApp.GetSiteNameMaxChar());
	m_EditPComments.SetLimitText(uiMaxChar*10);
	m_EditPRef.SetLimitText(uiMaxChar);
	m_EditPName.SetLimitText(uiMaxChar);
	m_EditPCompany.SetLimitText(uiMaxChar);
	m_EditPAdress.SetLimitText(uiMaxChar);
	m_EditPZip.SetLimitText(uiMaxChar);
	m_EditPTown.SetLimitText(uiMaxChar);
	m_EditPCountry.SetLimitText(uiMaxChar);
	m_EditPEMail.SetLimitText(uiMaxChar);
	m_EditPTel.SetLimitText(uiMaxChar);
	m_EditPFax.SetLimitText(uiMaxChar);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgRef::OnOK() 
{
	PREVENT_ENTER_KEY
	CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();
	ASSERT( NULL != pPrjRef );
	
	CDS_UserRef *pUserRef = m_pTADS->GetpUserRef();
	ASSERT( NULL != pUserRef );
	
	// Save all fields into TADS
	CString str;

	m_EditFName.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Name, (LPCTSTR)str );
	
	m_EditFCompany.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Company, (LPCTSTR)str );
	
	m_EditFAdress.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Address, (LPCTSTR)str );
	
	m_EditFZip.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Zip, (LPCTSTR)str );
	
	m_EditFTown.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Town, (LPCTSTR)str );
	
	m_EditFCountry.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Country, (LPCTSTR)str );
	
	m_EditFTel.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Tel, (LPCTSTR)str );
	
	m_EditFFax.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Fax, (LPCTSTR)str );
	
	m_EditFEMail.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Email, (LPCTSTR)str );
	
	m_EditFOther.GetWindowText( str );
	pUserRef->SetString(CDS_UserRef::Other, (LPCTSTR)str );
	

	m_EditPProject.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::Name, (LPCTSTR)str );
	
	m_EditPComments.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::Comment, (LPCTSTR)str );
	
	m_EditPRef.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustRef, (LPCTSTR)str );
	
	m_EditPName.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustName, (LPCTSTR)str );
	
	m_EditPAdress.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustAddress, (LPCTSTR)str );
	
	m_EditPZip.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustZip, (LPCTSTR)str );
	
	m_EditPTown.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustTown, (LPCTSTR)str );
	
	m_EditPTel.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustTel, (LPCTSTR)str );

	m_EditPFax.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustFax, (LPCTSTR)str );
	
	m_EditPCountry.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustCountry, (LPCTSTR)str );
	
	m_EditPEMail.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustEmail, (LPCTSTR)str );
	
	m_EditPCompany.GetWindowText( str );
	pPrjRef->SetString( CDS_ProjectRef::CustCompany, (LPCTSTR)str );

	::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_PROJECTREFCHANGE );
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PROJECTREFCHANGE );

	CDialogEx::OnOK();
}

void CDlgRef::OnButtonsetuserref() 
{
	m_pUserDB  = TASApp.GetpUserDB();

	CDS_UserRef *pUserRef = (CDS_UserRef*) m_pUserDB->Get(_T("USER_REF")).MP;
	ASSERT( NULL != pUserRef );

	m_EditFName.SetWindowText( pUserRef->GetString( CDS_UserRef::Name ) );
	m_EditFCompany.SetWindowText( pUserRef->GetString( CDS_UserRef::Company ) );
	m_EditFAdress.SetWindowText( pUserRef->GetString( CDS_UserRef::Address ) );
	m_EditFZip.SetWindowText( pUserRef->GetString( CDS_UserRef::Zip ) );
	m_EditFTown.SetWindowText( pUserRef->GetString( CDS_UserRef::Town ) );
	m_EditFCountry.SetWindowText( pUserRef->GetString( CDS_UserRef::Country ) );
	m_EditFTel.SetWindowText( pUserRef->GetString( CDS_UserRef::Tel ) );
	m_EditFFax.SetWindowText( pUserRef->GetString( CDS_UserRef::Fax ) );
	m_EditFEMail.SetWindowText( pUserRef->GetString( CDS_UserRef::Email ) );
	m_EditFOther.SetWindowText( pUserRef->GetString( CDS_UserRef::Other ) );
}

