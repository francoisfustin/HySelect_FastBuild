// boîte de dialogue CAboutDlg utilisée pour la boîte de dialogue 'À propos de' pour votre application

#include "stdafx.h"
#include "TASelect.h"

#include "FileTableMng.h"
#include "TAScopeUtil.h"
#include "DlgHtmlView.h"
#include "DlgLicense.h"
#include "DlgAboutBox.h"

CAboutDlg::CAboutDlg() : CDialogEx( CAboutDlg::IDD )
{
}

void CAboutDlg::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP( CAboutDlg, CDialogEx )
	ON_WM_DESTROY()
	ON_BN_CLICKED( IDC_BUTTONLICENSE, OnBnClickedLicence )
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Initialize dialog strings
	CString str;
	str = TASApp.LoadLocalizedString( IDS_ABOUTBOX_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_ABOUTBOX_STATICSOFTNAME );
	GetDlgItem( IDC_STATICSOFTNAME )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_ABOUTBOX_STATICDEVELOPBY );
	GetDlgItem( IDC_STATICDEVELOPBY )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_ABOUTBOX_STATICCOPYRIGHT );
	GetDlgItem( IDC_STATICCOPYRIGHT )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_ABOUTBOX_STATICALLRIGHTS );
	GetDlgItem( IDC_STATICALLRIGHTS )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_ABOUTBOX_STATICADDITINFO );
	GetDlgItem( IDC_STATICADDITINFO )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGLICENSE_TITLE );
	GetDlgItem( IDC_BUTTONLICENSE )->SetWindowText( str );

	m_TASLogo.LoadBitmap( IDB_TAS3LOGO_80x82 );
	( ( CStatic * )GetDlgItem( IDC_STATICLOGOTASELECT ) )->SetBitmap( ( HBITMAP ) m_TASLogo.GetSafeHandle() );

	TASApp.GetCompanyLogo( &m_TALogo, this );
	( ( CStatic * )GetDlgItem( IDC_STATICLOGOTA ) )->SetBitmap( ( HBITMAP ) m_TALogo.GetSafeHandle() );

	// Load the version number of the application into IDC_STATICVERSION
	CModuleVersion ver;
	CString tmp;
	str.Empty();
	char buffer[8];

	if( ver.GetFileVersionInfo( L"HySelect.exe" ) )
	{
		str = TASApp.LoadLocalizedString( IDS_ABOUTBOX_VERSION );
		
		WORD verDigit = HIWORD( ver.dwProductVersionMS );
		_itoa_s( verDigit, buffer, sizeof( buffer ), 10 );
		str += buffer;
		
		verDigit = LOWORD( ver.dwProductVersionMS );
		_itoa_s( verDigit, buffer, sizeof( buffer ), 10 );
		str += CString( _T(".") ) + CString( buffer );
		
		verDigit = HIWORD( ver.dwProductVersionLS );
		_itoa_s( verDigit, buffer, sizeof( buffer ), 10 );
		str += CString( _T(".") ) + CString( buffer );
		
		verDigit = LOWORD( ver.dwProductVersionLS );
		_itoa_s( verDigit, buffer, sizeof( buffer ), 10 );
		str += CString( _T(".") ) + CString( buffer );
	}

	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );

	// Set installation date
	str.Empty();
	CString strTmp;
	strTmp = TASApp.LoadLocalizedString( IDS_INSTALLATIONDATE );
	CString strDate = ::AfxGetApp()->GetProfileString( L"Files", L"Installation Date", L"" );

	COleDateTime InstDate;

	if( !InstDate.ParseDateTime( ( LPCTSTR ) strDate, VAR_DATEVALUEONLY, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ) ) )
	{
		strTmp += CString( L": " ) + L"-";
	}
	else
	{
		strTmp += CString( L": " ) + InstDate.Format( L"%d/%m/%Y" );
	}

	str = strTmp + CString( L"\r\n" );

	// Load the date of the last HyUpdate into IDC_EDITADDITINFO
	strDate = ::AfxGetApp()->GetProfileString( HYUPDATE, LASTUPDATE, _T( "" ) );
	COleDateTime lastupdate;

	// If last update doesn't contain any value, set the date to "-"
	if( !lastupdate.ParseDateTime( ( LPCTSTR ) strDate, VAR_DATEVALUEONLY ) )
	{
		strDate = L"-";
	}

	strTmp = TASApp.LoadLocalizedString( IDS_LASTUPDATE );
	strTmp += CString( L": " ) + strDate;
	str += strTmp + CString( L"\r\n" );

	// Set TADB Version and location.
	CTADatabase *pTADB = TASApp.GetpTADB();
	strTmp = TASApp.LoadLocalizedString( IDS_TADBAREA );
	strTmp += CString( L": " ) + CString( ( ( CDB_MultiString * )pTADB->Get( L"TADB_VERSION" ).MP )->GetString( 1 ) );
	str += strTmp + CString( _T("\r\n") );

	strTmp = TASApp.LoadLocalizedString( IDS_TADBVERSION );
	strTmp += CString( L": " ) + CString( ( ( CDB_MultiString * )pTADB->Get( L"TADB_VERSION" ).MP )->GetString( 0 ) );
	str += strTmp + CString( L"\r\n" );

	// Set TA-SCOPE software version.
	strTmp = TASApp.LoadLocalizedString( IDS_TASCOPEVERSION );
	CString strTAScopeVersion = TASApp.LoadLocalizedString( IDS_UNKNOWN );
	CString strDpSVisioVersion = TASApp.LoadLocalizedString( IDS_UNKNOWN );
	CFileStatus status;
	CFileTable *pSFT = new CFileTable();

	if( TRUE == CFile::GetStatus( CTAScopeUtil::GetTAScopeLocHFTPath(), status ) )
	{
		pSFT->ReadFile( ( wchar_t * )( LPCTSTR )CTAScopeUtil::GetTAScopeLocHFTPath() );

		if( true == pSFT->FindTargetWoPath( L"app.bin" ) )
		{
			strTAScopeVersion = pSFT->VersionDoubleToString( pSFT->GetVersion(), FILETABLE_VERSIONDIGITNBR_HH_APP );
		}

		if( true == pSFT->FindTargetWoPath( L"dps2.dfu" ) )
		{
			strDpSVisioVersion = pSFT->VersionDoubleToString( pSFT->GetVersion(), FILETABLE_VERSIONDIGITNBR_DPS_APP );
		}
	}

	if( NULL != pSFT )
	{
		delete pSFT;
	}

	strTmp += CString( L": " ) + strTAScopeVersion;
	str += strTmp + CString( L"\r\n" );

	// Set DpS-Vision software version.
	strTmp = TASApp.LoadLocalizedString( IDS_DPSVISIOVERSION );
	strTmp += CString( L": " ) + strDpSVisioVersion;
	str += strTmp + CString( L"\r\n" );

	// Set Compatible version of CBI.
	strTmp = TASApp.LoadLocalizedString( IDS_COMPATIBLEVERSIONOFCBI );
	strTmp += CString( L": " ) + CBIVERSIONPREFIX + CString( L"**." )
			  + CString( ( ( CDB_MultiString * )pTADB->Get( L"CBI_COMPATIBIL" ).MP )->GetString( 0 ) )
			  + CString( L" " )
			  + CString( ( ( CDB_MultiString * )pTADB->Get( L"CBI_COMPATIBIL" ).MP )->GetString( 1 ) );
	str += strTmp;

	GetDlgItem( IDC_EDITADDITINFO )->SetWindowText( str );

	return TRUE;
}

void CAboutDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	m_font.DeleteObject();
}

void CAboutDlg::OnBnClickedLicence()
{
	CDlgLicense dlg;
	dlg.DoModal();
}
