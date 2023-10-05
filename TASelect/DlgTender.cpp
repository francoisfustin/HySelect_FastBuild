// boîte de dialogue CAboutDlg utilisée pour la boîte de dialogue 'À propos de' pour votre application

#include "stdafx.h"
// #include "TASelect.h"
//
// #include "FileTableMng.h"
// #include "TAScopeUtil.h"

#include "DlgTender.h"

CTenderDlg::CTenderDlg( CWnd *pParent ) : CDialogEx( CTenderDlg::IDD, pParent )
{
}

void CTenderDlg::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CTenderDlg, CDialogEx )
	ON_BN_CLICKED( IDC_COPYCLIPBOARD, &CTenderDlg::OnBnClickedCopyclipboard )
END_MESSAGE_MAP()

BOOL CTenderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	HICON hIcon = LoadIcon( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_TENDER ) );
	SetIcon( hIcon, FALSE );

	// Initialize dialog strings
	CString str = _T( "" );
	str.Format( _T("%s - %s"), m_artName.c_str(), m_artNum.c_str() );

	if( m_artName.empty() )
	{
		SetWindowText( m_artNum.c_str() );
	}
	else
	{
		SetWindowText( str );
	}

	GetDlgItem( IDC_RICHEDIT_TENDER )->SetWindowText( m_tender.c_str() );
	str = TASApp.LoadLocalizedString( IDS_CLOSE );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TENDER_COPYCLIPBOARD );
	GetDlgItem( IDC_COPYCLIPBOARD )->SetWindowText( str );
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CTenderDlg::SetTender( const _string &artName, const _string &artNum, const _string &tenderText )
{
	m_artName = artName;
	m_artNum = artNum;
	m_tender = tenderText;
}

void CTenderDlg::OnBnClickedCopyclipboard()
{
	OpenClipboard();
	EmptyClipboard();
	size_t buffLen = ( m_tender.size() + 1 ) * sizeof( TCHAR );
	HGLOBAL hg = GlobalAlloc( GMEM_MOVEABLE,  buffLen );

	if( !hg )
	{
		CloseClipboard();
		return;
	}

	memcpy( GlobalLock( hg ), m_tender.c_str(), buffLen );
	GlobalUnlock( hg );
	SetClipboardData( CF_UNICODETEXT, hg );
	CloseClipboard();
	GlobalFree( hg );
}
