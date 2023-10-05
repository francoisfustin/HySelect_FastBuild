#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "DlgLTtabctrl.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabInfo.h"

#include "DlgPassword.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgPassword::CDlgPassword( CWnd *pParent )
	: CDialogEx( CDlgPassword::IDD, pParent )
{
}

void CDlgPassword::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITPASSWD, m_EditPassWd );
}

BEGIN_MESSAGE_MAP( CDlgPassword, CDialogEx )
END_MESSAGE_MAP()

BOOL CDlgPassword::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	
	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_DLGPASSWORD_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
	str.Empty();

	// Reset password to default.
	TASApp.SetHidingPsw( false );
	TASApp.GetpTADB()->SetSuperUserPsw( false );
	TASApp.LoadFlags();
	m_EditPassWd.SetWindowText( _T("") );
	return TRUE;
}

void CDlgPassword::OnOK() 
{
	CDataBase *pTADB = TASApp.GetpTADB();
	CDB_MultiString *pPass = (CDB_MultiString *)( pTADB->Get( _T("PASSWORD_PARAM") ).MP );
	ASSERT( NULL != pPass);
	
	CString str; 
	m_EditPassWd.GetWindowText( str );
		
	// Scan all password table.
	if( pPass->GetString( 0 ) == str )
	{
		// Developer's password.
		// Set all password to true.
		TASApp.SetHidingPsw( true );
		TASApp.GetpTADB()->SetSuperUserPsw( true );
	}
	else if( pPass->GetString( 1 ) == str )
	{
		// Hiding password.
		TASApp.SetHidingPsw( true );

		// Allow to show TRV.
		TASApp.SetTrvUsed( true );
	}
	else if( pPass->GetString( 2 ) == str )
	{
		// Get debug functionalities in release mode.
		TASApp.GetpTADB()->SetDebugPsw( true );
	}

	if( NULL != pDlgLeftTabInfo )
	{
		pDlgLeftTabInfo->ResetTree();
	}

	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_NEWDOCUMENT );
	::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_NEWDOCUMENT );
	::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_PSWCHANGED );
	
	CDialogEx::OnOK();
}
