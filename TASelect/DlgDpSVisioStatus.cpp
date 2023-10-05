#include "stdafx.h"
#include "afxdialogex.h"
#include "DpSVisioIncludes.h"
#include "DlgDpSVisioStatus.h"

IMPLEMENT_DYNAMIC( CDlgDpSVisioStatus, CDialogEx )

CDlgDpSVisioStatus::CDlgDpSVisioStatus( CDlgDpSVisio *pParent )
	: CDlgDpSVisioBase( CDlgDpSVisioStatus::IDD, pParent )
{
	m_hIconStatusGreen = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_GREENCIRCLE ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	m_hIconStatusRed = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_REDCIRCLE ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
}

CDlgDpSVisioStatus::~CDlgDpSVisioStatus()
{
	if( INVALID_HANDLE_VALUE != m_hIconStatusGreen )
	{
		DeleteObject( m_hIconStatusGreen );
	}

	if( INVALID_HANDLE_VALUE != m_hIconStatusRed )
	{
		DeleteObject( m_hIconStatusRed );
	}
}

void CDlgDpSVisioStatus::OnDpSVisioConnected( CDlgDpSVisio::WhichDevice eWhichDevice )
{
	CString strMsg;
	CString strTitle;

	switch( eWhichDevice )
	{
		case CDlgDpSVisio::Device_HID:
			m_IconStatus.SetIcon( m_hIconStatusGreen );
			strMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_CONNECTED );
			GetDlgItem( IDC_STATICCONNECTIONSTATUS )->SetWindowText( strMsg );
			_FillDpSVisioInformation();
			break;

		case CDlgDpSVisio::Device_DFUSe:
		case CDlgDpSVisio::Device_NewDFUSe:

			m_IconStatus.SetIcon( m_hIconStatusGreen );
			strMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_CONNECTEDDFU );
			GetDlgItem( IDC_STATICCONNECTIONSTATUS )->SetWindowText( strMsg );
			_FillDpSVisioInformation();

			strMsg = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_UPDATEDFUMSG );
			strTitle = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_UPDATEDFUTITLE );
			
			if( IDYES == MessageBox( strMsg, strTitle, MB_YESNO | MB_ICONWARNING ) )
			{
				m_pParent->PostMessage( WM_USER_DPSVISIO_SHOWDIALOG, (WPARAM)CDlgDpSVisio::Dialog_DFUUpgradeRunning, (LPARAM)CDlgDpSVisio::UpdateParam_DFU );
			}

			break;
	}
}

void CDlgDpSVisioStatus::OnDpSVisioDisconnected( CDlgDpSVisio::WhichDevice eWhichDevice )
{
	m_IconStatus.SetIcon( m_hIconStatusRed );
	CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_NOTCONNECTED );
	GetDlgItem( IDC_STATICCONNECTIONSTATUS )->SetWindowText( str );
	_ResetDpSVisioInformation();
}

void CDlgDpSVisioStatus::DoDataExchange( CDataExchange *pDX )
{
	CDlgDpSVisioBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_ICONSTATUS, m_IconStatus );
}

BOOL CDlgDpSVisioStatus::OnInitDialog()
{
	CDlgDpSVisioBase::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_STATUS );
	GetDlgItem( IDC_STATICSTATUS )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_NOTCONNECTED );
	GetDlgItem( IDC_STATICCONNECTIONSTATUS )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_IDENTIFIER );
	GetDlgItem( IDC_STATIC_IDENTTEXT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_SERIALNUMBER );
	GetDlgItem( IDC_STATIC_SNTEXT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_FIRMWAREREVISION );
	GetDlgItem( IDC_STATIC_FIRMWARETEXT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_AVAILABLEVERSION );
	GetDlgItem( IDC_STATIC_AVAILABLEVERSIONTEXT )->SetWindowText( str );
	GetDlgItem( IDC_STATIC_AVAILABLEVERSIONVALUE )->SetWindowText( m_pParent->GetAvailableFirmwareRevisionString( CDlgDpSVisio::Device_HID ) );

	_FillDpSVisioInformation();

	return TRUE;
}

void CDlgDpSVisioStatus::_FillDpSVisioInformation( void )
{
	_ResetDpSVisioInformation();

	CDpSVisioBaseCommunication *pclDpSVisioCommunication = m_pParent->GetDpSVisioCommunication();

	if( NULL == pclDpSVisioCommunication )
	{
		return;
	}

	CDpSVisioBaseInformation *pclDpSVisioInformation = pclDpSVisioCommunication->GetDpSVisioInformation();

	if( NULL == pclDpSVisioInformation )
	{
		return;
	}

	GetDlgItem( IDC_STATIC_IDENTVALUE )->SetWindowTextW( pclDpSVisioInformation->m_strIdentifier );
	GetDlgItem( IDC_STATIC_SNVALUE )->SetWindowTextW( pclDpSVisioInformation->m_strSerialNumber );
	GetDlgItem( IDC_STATIC_FIRMWAREVALUE )->SetWindowTextW( pclDpSVisioInformation->m_strFirmwareRevision );
	
	CString strAvailableVersion = m_pParent->GetAvailableFirmwareRevisionString( CDlgDpSVisio::Device_HID );
	TCHAR *pStopString;
	double dAvailableVersion = _tcstod( (LPCTSTR)strAvailableVersion, &pStopString );
	
	if( true == m_pParent->IsDpSVisioInNormalModeConnected() )
	{
		if( dAvailableVersion > pclDpSVisioInformation->m_dFirmwareVersion )
		{
			GetDlgItem( IDC_STATIC_CURRENTVERSIONSTATUS )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_NEWVERSION ) );
			GetDlgItem( IDC_STATIC_APPLYUPDATE )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_APPLYUPDATE ) );
			m_pParent->EnableUpdateButtons( CDlgDpSVisio::UBT_Update, TRUE, TRUE );
			m_pParent->EnableUpdateButtons( CDlgDpSVisio::UBT_ForceUpdate, FALSE, FALSE );
		}
		else if( dAvailableVersion == pclDpSVisioInformation->m_dFirmwareVersion )
		{
			GetDlgItem( IDC_STATIC_CURRENTVERSIONSTATUS )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGDPSVISIOSTATUS_SAMEVERSION ) );
			GetDlgItem( IDC_STATIC_APPLYUPDATE )->SetWindowText( _T("") );
			m_pParent->EnableUpdateButtons( CDlgDpSVisio::UBT_Update, FALSE, FALSE );
			m_pParent->EnableUpdateButtons( CDlgDpSVisio::UBT_ForceUpdate, TRUE, TRUE );
		}
	}
	else if( true == m_pParent->IsDpSVisioInDFUModeConnected() )
	{
		m_pParent->EnableUpdateButtons( CDlgDpSVisio::UBT_Update, FALSE, FALSE );
		m_pParent->EnableUpdateButtons( CDlgDpSVisio::UBT_ForceUpdate, TRUE, TRUE );
	}
}

void CDlgDpSVisioStatus::_ResetDpSVisioInformation( void )
{
	GetDlgItem( IDC_STATIC_IDENTVALUE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_STATIC_SNVALUE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_STATIC_FIRMWAREVALUE )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_STATIC_CURRENTVERSIONSTATUS )->SetWindowText( _T( "" ) );
	GetDlgItem( IDC_STATIC_APPLYUPDATE )->SetWindowText( _T( "" ) );
}
