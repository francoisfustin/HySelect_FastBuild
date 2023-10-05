#include "stdafx.h"


#include "TASelect.h"
#include "Global.h"
#include "HydroMod.h"
#include "DlgInjectionError.h"

IMPLEMENT_DYNAMIC( CDlgInjectionError, CDialogEx )

CDlgInjectionError::CDlgInjectionError( std::vector<CDS_HydroMod *> *pvecAllInjectionCircuitsWithTemperatureError, bool bDisplayListErrors, CWnd *pParent )
	: CDialogEx( IDD_DLGINJECTIONERROR, pParent )
{
	m_bDisplayListErrors = bDisplayListErrors;
	m_pvecAllInjectionCircuitsWithTemperatureError = pvecAllInjectionCircuitsWithTemperatureError;
	m_eReturnCode = ReturnCode::Undefined;
}

void CDlgInjectionError::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_LISTERRORS, m_clListBoxErrors );
	DDX_Control( pDX, ID_BTNAPPLYWITHOUTCORRECTION, m_ButtonApplyWithoutCorrection );
	DDX_Control( pDX, ID_BTNAPPLYWITHCORRECTION, m_ButtonApplyWithCorrection );
	DDX_Control( pDX, IDCANCEL, m_ButtonCancel );
}

BOOL CDlgInjectionError::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( ( true == m_bDisplayListErrors ) ? IDS_DLGINJECTIONCIRCUIT_ERRORMSG : IDS_DLGINJECTIONCIRCUIT_ERRORMSG_NOLIST );
	GetDlgItem( IDC_STATICERRORMSG )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_LISTACTIONS );
	GetDlgItem( IDC_STATICLISTACTIONS )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_APPLYACTIONTITLE );
	GetDlgItem( IDC_STATICAPPLYACTIONTITLE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_APPLYCORRECTACTIONTITLE );
	GetDlgItem( IDC_STATICAPPLYCORRECTACTIONTITLE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_CANCELACTIONTITLE );
	GetDlgItem( IDC_STATICCANCELACTIONTITLE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_APPLYACTIONDESCRIPTION );
	GetDlgItem( IDC_STATICAPPLYACTIONDESC )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_APPLYCORRECTACTIONDESCRIPTION );
	GetDlgItem( IDC_STATICAPPLYCORRECTACTIONDESC )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_CANCELACTIONDESCRIPTION );
	GetDlgItem( IDC_STATICCANCELACTIONDESC )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_APPLYACTIONNOTE );
	GetDlgItem( IDC_STATICAPPLYACTIONNOTE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_ERRORLIST );
	GetDlgItem( IDC_STATICERRORLIST )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_BUTTONAPPLY );
	m_ButtonApplyWithoutCorrection.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGINJECTIONCIRCUIT_BUTTONAPPLYCORRECT );
	m_ButtonApplyWithCorrection.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	m_ButtonCancel.SetWindowText( str );

	m_clListBoxErrors.ModifyStyle( LBS_SORT, 0 );

	if( true == m_bDisplayListErrors )
	{
		_FillErrorList();
	}
	else
	{
		m_clListBoxErrors.EnableWindow( FALSE );
		m_clListBoxErrors.ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATICERRORLIST )->ShowWindow( SW_HIDE );

		CRect rectStatic;
		GetDlgItem( IDC_STATIC )->GetWindowRect( &rectStatic );
		ScreenToClient( &rectStatic );

		CRect rectButtonApply;
		m_ButtonApplyWithoutCorrection.GetWindowRect( &rectButtonApply );
		ScreenToClient( &rectButtonApply );
		m_ButtonApplyWithoutCorrection.SetWindowPos( NULL, rectButtonApply.left, rectStatic.bottom + 7, -1, -1, SWP_NOSIZE );

		CRect rectButtonApplyCorrect;
		m_ButtonApplyWithCorrection.GetWindowRect( &rectButtonApplyCorrect );
		ScreenToClient( &rectButtonApplyCorrect );
		m_ButtonApplyWithCorrection.SetWindowPos( NULL, rectButtonApplyCorrect.left, rectStatic.bottom + 7, -1, -1, SWP_NOSIZE );

		CRect rectButtonCancel;
		m_ButtonCancel.GetWindowRect( &rectButtonCancel );
		ScreenToClient( &rectButtonCancel );
		m_ButtonCancel.SetWindowPos( NULL, rectButtonCancel.left, rectStatic.bottom + 7, -1, -1, SWP_NOSIZE );

		CRect rectWindow;
		GetWindowRect( &rectWindow );
		SetWindowPos( NULL, -1, -1, rectWindow.Width(), rectWindow.Height() - ( rectButtonApply.top - rectStatic.bottom - 7 ), SWP_NOMOVE );
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP( CDlgInjectionError, CDialogEx )
	ON_BN_CLICKED( ID_BTNAPPLYWITHOUTCORRECTION, OnBnClickedApplyWithoutCorrection )
	ON_BN_CLICKED( ID_BTNAPPLYWITHCORRECTION, OnBnClickedApplyWithCorrection )
	ON_BN_CLICKED( IDCANCEL, OnBnClickedCancel )
END_MESSAGE_MAP()

void CDlgInjectionError::OnBnClickedApplyWithoutCorrection()
{
	m_eReturnCode = ReturnCode::ApplyWithoutCorrection;
	EndDialog( (int)m_eReturnCode );
}

void CDlgInjectionError::OnBnClickedApplyWithCorrection()
{
	m_eReturnCode = ReturnCode::ApplyWithCorrection;
	EndDialog( (int)m_eReturnCode );
}

void CDlgInjectionError::OnBnClickedCancel()
{
	m_eReturnCode = ReturnCode::Cancel;
	EndDialog( (int)m_eReturnCode );
}

void CDlgInjectionError::_FillErrorList()
{
	if( NULL == m_pvecAllInjectionCircuitsWithTemperatureError )
	{
		return;
	}

	for( auto &iterParent : *m_pvecAllInjectionCircuitsWithTemperatureError )
	{
		INSERT_STRING_IN_LIST_AND_SCROLL( m_clListBoxErrors, iterParent->GetHMName() );
	}
}
