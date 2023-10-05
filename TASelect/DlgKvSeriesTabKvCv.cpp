#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "utilities.h"
#include "DlgKvSeries.h"
#include "DlgKvSeriesTabKvCv.h"

CDlgKvSeriesTabKvCv::CDlgKvSeriesTabKvCv( CWnd *pParent )
	: CDialogExt( CDlgKvSeriesTabKvCv::IDD, pParent )
{
	SetBackgroundColor(_WHITE_DLGBOX);
	m_dKvCv = 0.0;
}

void CDlgKvSeriesTabKvCv::ResetAll( bool bKvCv )
{
	CString str;
	m_EditCoefficient.SetWindowText( _T("") );
	
	if( true == bKvCv )
	{
		str = TASApp.LoadLocalizedString( IDS_KVSERIES_STATICKV );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_KVSERIES_STATICCV );
	}

	m_StaticName.SetWindowText( str );
	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabKvCv::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITKVCV, m_EditCoefficient );
	DDX_Control( pDX, IDC_STATICKVCV, m_StaticName );
}

BEGIN_MESSAGE_MAP( CDlgKvSeriesTabKvCv, CDialogExt )
	ON_EN_CHANGE( IDC_EDITKVCV, OnEnChangeKvCv )
	ON_EN_KILLFOCUS( IDC_EDITKVCV, OnEnKillFocusKvCv )
END_MESSAGE_MAP()

BOOL CDlgKvSeriesTabKvCv::OnInitDialog() 
{
	CDialogExt::OnInitDialog();
	
	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_TABDLGKVCV_STATICKVCV );
	GetDlgItem( IDC_STATICKVCV )->SetWindowText( str );
	
	return TRUE;
}

// Used to capture Enter and Esc Key .....
void CDlgKvSeriesTabKvCv::OnCancel() 
{
}

void CDlgKvSeriesTabKvCv::OnOK() 
{
}

void CDlgKvSeriesTabKvCv::OnEnChangeKvCv() 
{
	if( GetFocus() != &m_EditCoefficient )
	{
		return; 
	}

	CString str;

	switch( ReadDouble( m_EditCoefficient, &m_dKvCv ) ) 
	{
		case RD_EMPTY:
			m_dKvCv = 0.0;
			break;

		case RD_NOT_NUMBER:
			m_EditCoefficient.GetWindowText( str );

			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) ) 
			{
				return;
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}

			m_EditCoefficient.SetWindowText( _T("") );
			break;

		case RD_OK:
			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabKvCv::OnEnKillFocusKvCv() 
{
	double dValue;

	switch( ReadDouble( m_EditCoefficient, &dValue ) )
	{
		case RD_EMPTY:
			break;

		case RD_OK:

			if( dValue < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditCoefficient.SetFocus();
			}
			else
			{
				m_EditCoefficient.SetWindowText( WriteDouble( dValue, 4, 1 ) );
			}

			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}
