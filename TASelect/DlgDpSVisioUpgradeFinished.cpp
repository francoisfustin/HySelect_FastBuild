#include "stdafx.h"
#include "afxdialogex.h"
#include "DpSVisioIncludes.h"
#include "DlgDpSVisioUpgradeFinished.h"

IMPLEMENT_DYNAMIC( CDlgDpSVisioUpgradeFinished, CDialogEx )

CDlgDpSVisioUpgradeFinished::CDlgDpSVisioUpgradeFinished( CDlgDpSVisio *pParent )
	: CDlgDpSVisioBase( CDlgDpSVisioUpgradeFinished::IDD, pParent )
{
	m_hIconUpgradeStatusOK = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_GREENOK ), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR );
	m_hIconUpgradeStatusFailed = (HICON)LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE( IDI_REDERROR ), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR );
}

CDlgDpSVisioUpgradeFinished::~CDlgDpSVisioUpgradeFinished()
{
	if( INVALID_HANDLE_VALUE != m_hIconUpgradeStatusOK )
	{
		DeleteObject( m_hIconUpgradeStatusOK );
	}

	if( INVALID_HANDLE_VALUE != m_hIconUpgradeStatusFailed )
	{
		DeleteObject( m_hIconUpgradeStatusFailed );
	}
}

void CDlgDpSVisioUpgradeFinished::SetParam( LPARAM lpParam )
{
	m_rUpgradeStatus.m_fUpgradeStatus = ( (CDlgDpSVisio::UpgradeStatus*)lpParam )->m_fUpgradeStatus;
	m_rUpgradeStatus.m_eUpgradeError = ( (CDlgDpSVisio::UpgradeStatus*)lpParam )->m_eUpgradeError;
	m_rUpgradeStatus.m_dwDFUInternalErrorCode = ( (CDlgDpSVisio::UpgradeStatus*)lpParam )->m_dwDFUInternalErrorCode;
}

void CDlgDpSVisioUpgradeFinished::DoDataExchange( CDataExchange* pDX )
{
	CDlgDpSVisioBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICRESULT, m_BitmapUpgradeStatus );
}

BOOL CDlgDpSVisioUpgradeFinished::OnInitDialog()
{
	CDlgDpSVisioBase::OnInitDialog();

	CDpSVisioBaseInformation *pclDpSVisioInformation = NULL;
	CDpSVisioBaseCommunication *pclDpSVisioCommunication = m_pParent->GetDpSVisioCommunication();

	if( NULL != pclDpSVisioCommunication )
	{
		pclDpSVisioInformation = pclDpSVisioCommunication->GetDpSVisioInformation();
	}

	UINT iID = ( true == m_rUpgradeStatus.m_fUpgradeStatus ) ? IDS_DLGDPSVISIOUPGRADEFINISHED_TITLE : IDS_DLGDPSVISIOUPGRADEFINISHEDERROR_TITLE;
	m_pParent->SetWindowText( TASApp.LoadLocalizedString( iID ) );

	iID = ( true == m_rUpgradeStatus.m_fUpgradeStatus ) ? IDS_DLGDPSVISIOUPGRADEFINISHED_STATUSOK : IDS_DLGDPSVISIOUPGRADEFINISHED_STATUSFAILED;
	GetDlgItem( IDC_STATIC_UPGRADERESULT )-> SetWindowText( TASApp.LoadLocalizedString( iID ) );

	CString str;

	if( NULL != pclDpSVisioInformation )
	{
		FormatString( str, IDS_DLGDPSVISIOUPGRADEFINISHED_CURRENTVERSION, pclDpSVisioInformation->m_strShortFirmwareRevision );
	}
	else
	{
		FormatString( str, IDS_DLGDPSVISIOUPGRADEFINISHED_CURRENTVERSION, TASApp.LoadLocalizedString( IDS_DLGDPSVISIOUPGRADEFINISHED_UNKNOWN ) );
	}

	GetDlgItem( IDC_STATIC_CURRENTVERSIONTEXT )->SetWindowText( str );

	// By default do not show this button.
	GetDlgItem( IDC_BUTSHOWMOREINFO )->ShowWindow( SW_HIDE );
	GetDlgItem( IDC_BUTSHOWMOREINFO )->EnableWindow( FALSE );

	m_BitmapUpgradeStatus.SetWindowPos( NULL, -1, -1, 48, 48, SWP_NOMOVE | SWP_NOZORDER );
	
	if( true == m_rUpgradeStatus.m_fUpgradeStatus )
	{
		GetDlgItem( IDC_STATICERROR )->ShowWindow( FALSE );
		GetDlgItem( IDC_STATICERROR2 )->ShowWindow( FALSE );

		CRect RectStaticCVT;
		GetDlgItem( IDC_STATIC_CURRENTVERSIONTEXT )->GetWindowRect( &RectStaticCVT );
		ScreenToClient( &RectStaticCVT );

		CRect RectStaticBUS;
		m_BitmapUpgradeStatus.GetWindowRect( &RectStaticBUS );
		ScreenToClient( &RectStaticBUS );

		m_BitmapUpgradeStatus.SetIcon( m_hIconUpgradeStatusOK );

		CRect ClientRect;
		GetClientRect( &ClientRect );

		CPoint newPos;
		newPos.x = ( ClientRect.Width() - RectStaticBUS.Width() ) >> 1;
		newPos.y = ( ClientRect.bottom - RectStaticCVT.bottom - RectStaticBUS.Height() ) >> 1;
		newPos.y += RectStaticCVT.bottom;
		m_BitmapUpgradeStatus.SetWindowPos( NULL, newPos.x, newPos.y, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	}
	else
	{
		m_BitmapUpgradeStatus.SetIcon( m_hIconUpgradeStatusFailed );
		
		CString str = _T("");

		if( CDlgDpSVisio::UE_DeviceDisconnected == m_rUpgradeStatus.m_eUpgradeError || 
			CDlgDpSVisio::UE_DeviceChanged == m_rUpgradeStatus.m_eUpgradeError )
		{
			// There was a problem when upgrading the DpS-Visio firmware.\r\nError: device disconnected or no more responding.
			FormatString( str, IDS_DLGDPSVISIOUPGRADEFINISHED_ERRORMSG1, TASApp.LoadLocalizedString( IDS_DLGDPSVISIOUPGRADEFINISHED_ERRORDEVDISCONNECTED ) );
		}
		else if( CDlgDpSVisio::UE_DFUNotDetected == m_rUpgradeStatus.m_eUpgradeError )
		{
			// There was a problem when upgrading the DpS-Visio firmware.\r\nError: DFU device not detected.
			FormatString( str, IDS_DLGDPSVISIOUPGRADEFINISHED_ERRORMSG1, TASApp.LoadLocalizedString( IDS_DLGDPSVISIOUPGRADEFINISHED_ERRORDFUNOTDETECTEDSHORT ) );

			// GetDlgItem( IDC_BUTSHOWMOREINFO )->ShowWindow( SW_SHOW );
			// GetDlgItem( IDC_BUTSHOWMOREINFO )->EnableWindow( TRUE );
		}
		else if( CDlgDpSVisio::UE_InternalError == m_rUpgradeStatus.m_eUpgradeError )
		{
			// There was a problem when upgrading the DpS-Visio firmware.\r\nError: internal error (Code: 0xFFFFFFFF).
			CString str2ErrorCode;
			str2ErrorCode.Format( _T("0x%08X"), m_rUpgradeStatus.m_dwDFUInternalErrorCode );
			CString strInternalError;
			FormatString( strInternalError, IDS_DLGDPSVISIOUPGRADEFINISHED_ERRORINTERNAL, str2ErrorCode );
			FormatString( str, IDS_DLGDPSVISIOUPGRADEFINISHED_ERRORMSG1, strInternalError );
		}

		GetDlgItem( IDC_STATICERROR )->SetWindowText( str );

		// Please close this dialog, unplug the DpS-Visio USB cable from your computer, switch-off and on again the DpS-Visio 
		// and plug the USB cable in your computer. Then retry the upgrade operation.
		str = TASApp.LoadLocalizedString( IDS_DLGDPSVISIOUPGRADEFINISHED_ERRORMSG2 );
		GetDlgItem( IDC_STATICERROR2 )->SetWindowText( str );
	}

	m_pParent->ChangeCancelButtonName( TASApp.LoadLocalizedString( IDS_CLOSE ) );
	m_pParent->EnableCancelButton( TRUE );

	return TRUE;
}
