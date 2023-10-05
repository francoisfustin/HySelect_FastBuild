#include "stdafx.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "utilities.h"
#include "DlgWaterChar.h"
#include "DlgKvSeries.h"
#include "DlgKvSeriesTabNomCond.h"

CDlgKvSeriesTabNomCond::CDlgKvSeriesTabNomCond( CWnd *pParent )
	: CDialogExt( CDlgKvSeriesTabNomCond::IDD, pParent )
{
	SetBackgroundColor( _WHITE_DLGBOX );
	m_dDp = 0.0;
	m_dFlow = 0.0;
}

void CDlgKvSeriesTabNomCond::SetWaterChar() 
{
	CString str1;
	CString str2;
	CTADatastruct *pTADS = TASApp.GetpTADS();
	m_clWaterChar.BuildWaterStrings( str1, str2 );
	
	// Set the text for the first static.
	SetDlgItemText( IDC_STATICFLUID1, str1 );
	
	// Set the text for the second static.
	SetDlgItemText( IDC_STATICFLUID2,str2 );
}

void CDlgKvSeriesTabNomCond::ResetAll()
{
	m_EditDp.SetWindowText( _T("") );
	m_EditFlow.SetWindowText( _T("") );
	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabNomCond::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFLOW, m_EditFlow );
	DDX_Control( pDX, IDC_EDITDP, m_EditDp );
	DDX_Control( pDX, IDC_GROUPWATER, m_GroupWater );
	DDX_Control( pDX, IDC_BUTTONMODWATER, m_ButtonModWater );
	DDX_Control( pDX, IDC_STATICQUNIT, m_StaticQUnit );
	DDX_Control( pDX, IDC_STATICPDUNIT, m_StaticPdUnit );
}

BEGIN_MESSAGE_MAP( CDlgKvSeriesTabNomCond, CDialogExt )
	ON_EN_CHANGE( IDC_EDITFLOW, OnEnChangeFlow )
	ON_EN_KILLFOCUS( IDC_EDITFLOW, OnEnKillFocusFlow )
	ON_EN_CHANGE( IDC_EDITDP, OnEnChangeDp )
	ON_EN_KILLFOCUS( IDC_EDITDP, OnEnKillFocusDp )
	ON_BN_CLICKED( IDC_BUTTONMODWATER, OnButtonModWater )
END_MESSAGE_MAP()

BOOL CDlgKvSeriesTabNomCond::OnInitDialog() 
{
	CDialogExt::OnInitDialog();
	
	// Initialize dialog strings
	CString str = TASApp.LoadLocalizedString( IDS_TABDLGNMCOND_STATICFLOW );
	GetDlgItem( IDC_STATICFLOW )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGNMCOND_STATICDP );
	GetDlgItem( IDC_STATICDP )->SetWindowText( str );

	// Initialize units for each edit box.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	GetNameOf( pUnitDB->GetUnit( _U_DIFFPRESS, pUnitDB->GetDefaultUnitIndex( _U_DIFFPRESS ) ), tcName );
	m_StaticPdUnit.SetWindowText( tcName );
	
	GetNameOf( pUnitDB->GetUnit( _U_FLOW, pUnitDB->GetDefaultUnitIndex( _U_FLOW ) ), tcName );
	m_StaticQUnit.SetWindowText( tcName );	
	
	// Set Water Group Icon.
	m_GroupWater.SetBckgndColor( _WHITE_DLGBOX );
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupWater.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Water );
	}
	
	// Add bitmap to "Water" button.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT ( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonModWater.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_ModifyWater ) );
	}

	// Save what is the current project type.
	// Can not calculate for Change-over
	if( ChangeOver == TASApp.GetpTADS()->GetpTechParams()->GetProductSelectionApplicationType() )
	{
		TASApp.GetpTADS()->GetpTechParams()->SetProductSelectionApplicationType( Heating );
		CWaterChar *pWC = TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData();
		*pWC = *( TASApp.GetpTADS()->GetpTechParams()->GetDefaultISHeatingWC() );
	}

	m_eCurrentProjectType = TASApp.GetpTADS()->GetpTechParams()->GetProductSelectionApplicationType();

	m_clWaterChar = *( TASApp.GetpTADS()->GetpWCForProductSelection()->GetpWCData() );

	// Init water static.
	SetWaterChar();

	return TRUE;
}

// Used to capture Enter and Esc Key .....
void CDlgKvSeriesTabNomCond::OnOK() 
{
}

void CDlgKvSeriesTabNomCond::OnCancel() 
{
}

void CDlgKvSeriesTabNomCond::OnEnChangeFlow() 
{
	if( GetFocus() != &m_EditFlow )
	{
		return; 
	}

	CString str;

	switch( ReadDouble( m_EditFlow, &m_dFlow ) )
	{
		case RD_EMPTY:
			m_dFlow = 0.0;
			break;

		case RD_NOT_NUMBER:
			m_EditFlow.GetWindowText(str);
			
			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) ) 
			{
				return;
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}

			m_EditFlow.SetWindowText( _T("") );
			break;

		case RD_OK:
			m_dFlow = CDimValue::CUtoSI( _U_FLOW, m_dFlow );
			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent)->UpdateCurrentKv();
}

void CDlgKvSeriesTabNomCond::OnEnKillFocusFlow() 
{
	double dValue;

	switch( ReadDouble( m_EditFlow, &dValue ) )
	{
		case RD_EMPTY:
			break;

		case RD_OK:

			if( dValue < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditFlow.SetFocus();
			}
			else
			{
				m_EditFlow.SetWindowText( WriteDouble( dValue, 4, 1 ) );
			}

			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabNomCond::OnEnChangeDp() 
{
	if( GetFocus() != &m_EditDp )
	{
		return; 
	}

	CString str;

	switch( ReadDouble( m_EditDp, &m_dDp ) )
	{
		case RD_EMPTY:
			m_dDp = 0.0;
			break;

		case RD_NOT_NUMBER:
			m_EditDp.GetWindowText( str );

			if( _T(".") == str || _T(",") == str || _T("e") == str.Right( 1 ) || _T("e+") == str.Right( 2 ) || _T("e-") == str.Right( 2 ) ) 
			{
				return;
			}
			else
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			}

			m_EditDp.SetWindowText( _T("") );
			break;

		case RD_OK:
			m_dDp = CDimValue::CUtoSI( _U_DIFFPRESS, m_dDp );
			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabNomCond::OnEnKillFocusDp() 
{
	double dValue;

	switch( ReadDouble( m_EditDp, &dValue ) )
	{
		case RD_EMPTY:
			break;

		case RD_OK:

			if( dValue < 0.0 )
			{
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				m_EditDp.SetFocus();
			}
			else
			{
				m_EditDp.SetWindowText( WriteDouble( dValue, 4, 1 ) );
			}

			break;
	};

	CWnd *pParent = GetParent();
	pParent = pParent->GetParent();
	( (CDlgKvSeries *)pParent )->UpdateCurrentKv();
}

void CDlgKvSeriesTabNomCond::OnButtonModWater() 
{
	// Display the dialog.
	CDlgWaterChar dlg;
	dlg.Display( NULL, CDlgWaterChar::DlgWaterChar_ForTools, true, m_eCurrentProjectType );

	// If the user has simply changed the application type, or has changed something in the current application type.
	if( m_eCurrentProjectType != dlg.GetCurrentApplicationType() || true == dlg.IsSomethingChanged( dlg.GetCurrentApplicationType() ) )
	{
		// Save the new user choice to force it if we call again the 'CDlgWaterChar' dialog.
		m_eCurrentProjectType = dlg.GetCurrentApplicationType();

		// Update the 2 Water statics because
		// WM_USER_WATERCHANGE message does not reach the dialog
		double dDT = 0.0;
		dlg.GetWaterChar( dlg.GetCurrentApplicationType(), m_clWaterChar, dDT );

		SetWaterChar();

		CWnd *pParent = GetParent();
		pParent = pParent->GetParent();
		( (CDlgKvSeries *)pParent )->UpdateLstCtrl();
	}
}
