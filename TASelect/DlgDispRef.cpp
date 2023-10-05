// DlgPrjInfo.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"

#include "DlgDispRef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDispRef dialog


CDlgDispRef::CDlgDispRef(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgDispRef::IDD, pParent)
{

}


void CDlgDispRef::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GROUPPROJECT, m_GroupProject);
	DDX_Control(pDX, IDC_GROUPREF, m_GroupRef);
}


BEGIN_MESSAGE_MAP(CDlgDispRef, CDialogEx)
END_MESSAGE_MAP()

int CDlgDispRef::Display()
{
	return DoModal();
}
/////////////////////////////////////////////////////////////////////////////
// CDlgDispRef message handlers

BOOL CDlgDispRef::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	// Initialize dialog strings
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGDISPREF_CAPTION );
	SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDISPREF_STATICINSTRUCT );
	GetDlgItem( IDC_STATICINSTRUCT)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDISPREF_CHECKDATE );
	GetDlgItem( IDC_CHECKDATE)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDISPREF_CHECKYOURREF );
	GetDlgItem( IDC_CHECKYOURREF)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDISPREF_CHECKOTHERINFO );
	GetDlgItem( IDC_CHECKOTHERINFO)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDISPREF_CHECKPROJECTNAME );
	GetDlgItem( IDC_CHECKPROJECTNAME)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDISPREF_CHECKCUSTOMERREF );
	GetDlgItem( IDC_CHECKCUSTOMERREF)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGDISPREF_CHECKCOMMENTS );
	GetDlgItem( IDC_CHECKCOMMENTS)->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_OK);
	GetDlgItem( IDOK )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_CANCEL);
	GetDlgItem( IDCANCEL )->SetWindowText( str );

	CDB_PageSetup *m_pTADSPageSetup = TASApp.GetpTADS()->GetpPageSetup();
	ASSERT( NULL != m_pTADSPageSetup) ;

    ( (CButton *)GetDlgItem( IDC_CHECKDATE ) )->SetCheck( m_pTADSPageSetup->GetShowDate() );
    ( (CButton *)GetDlgItem( IDC_CHECKYOURREF ) )->SetCheck( m_pTADSPageSetup->GetShowUserRef() );
    ( (CButton *)GetDlgItem( IDC_CHECKOTHERINFO ) )->SetCheck( m_pTADSPageSetup->GetShowOtherInfo() );
    ( (CButton *)GetDlgItem( IDC_CHECKPROJECTNAME ) )->SetCheck( m_pTADSPageSetup->GetShowPrjName() );
    ( (CButton *)GetDlgItem( IDC_CHECKCUSTOMERREF ) )->SetCheck( m_pTADSPageSetup->GetShowCustRef() );
    ( (CButton *)GetDlgItem( IDC_CHECKCOMMENTS ) )->SetCheck( m_pTADSPageSetup->GetShowComment() );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgDispRef::OnOK() 
{
	CDB_PageSetup* m_pTADSPageSetup = (CDB_PageSetup*)TASApp.GetpTADS()->Get(_T("PARAM_PAGESETUP")).MP;		ASSERT(m_pTADSPageSetup);

    if (( (CButton *)GetDlgItem( IDC_CHECKDATE))->GetCheck()) 
		m_pTADSPageSetup->SetShowDate(true);
	else
		m_pTADSPageSetup->SetShowDate(false);

    if (( (CButton *)GetDlgItem( IDC_CHECKYOURREF))->GetCheck())
		m_pTADSPageSetup->SetShowUserRef(true);
	else
		m_pTADSPageSetup->SetShowUserRef(false);
    
	if (( (CButton *)GetDlgItem( IDC_CHECKOTHERINFO))->GetCheck())
		m_pTADSPageSetup->SetShowOtherInfo(true);
	else
		m_pTADSPageSetup->SetShowOtherInfo(false);
    
	if (( (CButton *)GetDlgItem( IDC_CHECKPROJECTNAME))->GetCheck())
		m_pTADSPageSetup->SetShowPrjName(true);
	else
		m_pTADSPageSetup->SetShowPrjName(false);
    
	if (( (CButton *)GetDlgItem( IDC_CHECKCUSTOMERREF))->GetCheck())
		m_pTADSPageSetup->SetShowCustRef(true);
	else
		m_pTADSPageSetup->SetShowCustRef(false);
    
	if (( (CButton *)GetDlgItem( IDC_CHECKCOMMENTS))->GetCheck())
		m_pTADSPageSetup->SetShowComment(true);
	else
		m_pTADSPageSetup->SetShowComment(false);
	
	CDialogEx::OnOK();
}
