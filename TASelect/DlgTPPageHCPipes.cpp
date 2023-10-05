#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPPageHCPipes.h"


IMPLEMENT_DYNAMIC( CDlgTPPageHCPipes, CDlgCtrlPropPage )

CDlgTPPageHCPipes::CDlgTPPageHCPipes( CWnd *pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_strCircPipe = _T("");
	m_strDistSupplyPipe = _T("");
	m_strDistReturnPipe = _T("");
	m_dSafetyFactor = 0.0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPIPES_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPIPES_PAGENAME );
}

void CDlgTPPageHCPipes::Init( bool bResetToDefault )
{
	// Fill variables for ComboBox.
	if( true == bResetToDefault )
	{
		m_pTADS->GetpProjectParams()->GetpHmCalcParams()->ResetPrjParams( true );
	}
	
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	CPrjParams *pHMGenParam = pPrjParam->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );
	
	m_strCircPipe = pHMGenParam->GetPrjParamID( CPrjParams::PipeCircSerieID );
	m_strDistSupplyPipe = pHMGenParam->GetPrjParamID( CPrjParams::PipeDistSupplySerieID );
	m_strDistReturnPipe = pHMGenParam->GetPrjParamID( CPrjParams::PipeDistReturnSerieID );

	FillComboBoxCircPipe();
	FillComboBoxDistSupplyPipe();
	FillComboBoxDistReturnPipe();
	
	m_dSafetyFactor = ( pPrjParam->GetSafetyFactor() - 1.0 ) * 100.0;
	m_EditSafetyFactor.SetWindowTextW( WriteDouble( m_dSafetyFactor, 0, 0, TRUE ) );
}

void CDlgTPPageHCPipes::Save( CString strSectionName )
{
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	CPrjParams *pHMGenParam = pPrjParam->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );
	
	if ( NULL != GetSafeHwnd() )
	{
		pHMGenParam->SetPrjParamID( CPrjParams::PipeCircSerieID, m_strCircPipe );
		pHMGenParam->SetPrjParamID( CPrjParams::PipeDistSupplySerieID, m_strDistSupplyPipe );
		pHMGenParam->SetPrjParamID( CPrjParams::PipeDistReturnSerieID, m_strDistReturnPipe );
		pPrjParam->SetSafetyFactor( m_dSafetyFactor / 100.0 + 1.0 );
	}
}

bool CDlgTPPageHCPipes::VerifyInfo()
{
	bool bResult = true;
	
	// Verify safety factor.
	double dSafetyFactor;
	
	switch( ReadDouble( m_EditSafetyFactor, &dSafetyFactor ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dSafetyFactor > 100.0 )
				bResult = false;
			else
			{
				m_dSafetyFactor = dSafetyFactor;
				m_EditSafetyFactor.SetWindowText( WriteDouble( dSafetyFactor, 3 ) );
			}
			break;
	}
	
	return bResult;
}

BEGIN_MESSAGE_MAP( CDlgTPPageHCPipes, CDlgCtrlPropPage )
	ON_CBN_SELCHANGE( IDC_COMBOCIRCPIPE, OnCbnSelChangeCircPipe )
	ON_CBN_SELCHANGE( IDC_COMBODISTSUPPLYPIPE, OnCbnSelChangeDistSupplyPipe )
	ON_CBN_SELCHANGE( IDC_COMBODISTRETURNPIPE, OnCbnSelChangeDistReturnPipe )
	ON_EN_KILLFOCUS( IDC_EDITPIPESAFETYFACTOR, OnEnKillFocusSafetyFactor )
END_MESSAGE_MAP()

void CDlgTPPageHCPipes::DoDataExchange( CDataExchange *pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOCIRCPIPE, m_ComboCircPipe );
	DDX_Control( pDX, IDC_COMBODISTSUPPLYPIPE, m_ComboDistSupplyPipe );
	DDX_Control( pDX, IDC_COMBODISTRETURNPIPE, m_ComboDistReturnPipe );
	DDX_Control( pDX, IDC_EDITPIPESAFETYFACTOR, m_EditSafetyFactor );
}

BOOL CDlgTPPageHCPipes::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPIPES_STATICCIRCPIPE );
	GetDlgItem( IDC_STATICCIRCPIPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPIPES_STATICDISTSUPPLYPIPE );
	GetDlgItem( IDC_STATICDISTSUPPLYPIPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPIPES_STATICDISTRETURNPIPE );
	GetDlgItem( IDC_STATICDISTRETURNPIPE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_RBN_P_PP_SF );
	GetDlgItem( IDC_STATICSAFETYFACTOR )->SetWindowText( str );

	m_EditSafetyFactor.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditSafetyFactor.SetPhysicalType( _U_NODIM );
	m_EditSafetyFactor.SetMaxDblValue( 100.0 );
	m_EditSafetyFactor.SetMinDblValue( 0.0 );

	// Create tooltips.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	CString TTstr;
	TTstr = TASApp.LoadLocalizedString( IDS_RBN_P_PP_SF_TT );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_EDITPIPESAFETYFACTOR ), TTstr );
	
	m_pTADS = TASApp.GetpTADS();
	Init();

	return TRUE;
}

void CDlgTPPageHCPipes::OnCbnSelChangeCircPipe()
{
	CTable *pTabCircPipe = dynamic_cast<CTable*>( (CData *)m_ComboCircPipe.GetItemData( m_ComboCircPipe.GetCurSel() ) );

	if( NULL == pTabCircPipe )
	{
		return;
	}

	m_strCircPipe = pTabCircPipe->GetIDPtr().ID;
}

void CDlgTPPageHCPipes::OnCbnSelChangeDistSupplyPipe()
{
	CTable *pTabDistSupplyPipe = dynamic_cast<CTable*>( (CData *)m_ComboDistSupplyPipe.GetItemData( m_ComboDistSupplyPipe.GetCurSel() ) );

	if( NULL == pTabDistSupplyPipe )
	{
		return;
	}

	m_strDistSupplyPipe = pTabDistSupplyPipe->GetIDPtr().ID;
}

void CDlgTPPageHCPipes::OnCbnSelChangeDistReturnPipe()
{
	CTable *pTabDistReturnPipe = dynamic_cast<CTable*>( (CData *)m_ComboDistReturnPipe.GetItemData( m_ComboDistReturnPipe.GetCurSel() ) );

	if( NULL == pTabDistReturnPipe )
	{
		return;
	}

	m_strDistReturnPipe = pTabDistReturnPipe->GetIDPtr().ID;
}

void CDlgTPPageHCPipes::OnEnKillFocusSafetyFactor()
{
	VerifyInfo();
}

void CDlgTPPageHCPipes::FillComboBoxCircPipe()
{
	CRank List;
	int iCount = 0;
	iCount = TASApp.GetpTADB()->GetPipeSerieList( &List, CTADatabase::FilterSelection::ForHMCalc );
	FillComboBox( &List, &m_ComboCircPipe, iCount, &m_strCircPipe );
}

void CDlgTPPageHCPipes::FillComboBoxDistSupplyPipe()
{
	CRank List;
	int iCount = 0;
	iCount = TASApp.GetpTADB()->GetPipeSerieList( &List, CTADatabase::FilterSelection::ForHMCalc );
	FillComboBox( &List, &m_ComboDistSupplyPipe, iCount, &m_strDistSupplyPipe );
}

void CDlgTPPageHCPipes::FillComboBoxDistReturnPipe()
{
	CRank List;
	int iCount = 0;
	iCount = TASApp.GetpTADB()->GetPipeSerieList( &List, CTADatabase::FilterSelection::ForHMCalc );
	FillComboBox( &List, &m_ComboDistReturnPipe, iCount, &m_strDistReturnPipe );
}

void CDlgTPPageHCPipes::FillComboBox( CRank *pList, CExtNumEditComboBox *pclComboBox, int iCount, CString *pCstr )
{
	if( NULL == pList || NULL == pclComboBox )
	{
		ASSERT_RETURN;
	}

	CString str1;
	
	if( iCount > 0 )
	{
		pList->Transfer( pclComboBox );
		int iNbre = pclComboBox->GetCount();
		int iPos = -1;
		
		if( NULL != pCstr )
		{
			for( int i = 0; i < iNbre && -1 == iPos; i++ )
			{
				CTable *pPipeTab = dynamic_cast<CTable*>( (CData *)pclComboBox->GetItemData( i ) );

				if( NULL != pPipeTab && pPipeTab->GetIDPtr().ID == *pCstr )
				{
					pclComboBox->SetCurSel( i );
					iPos = i;
				}
			}
		}
		
		if( -1 == iPos && iNbre > 0 )
		{
			pclComboBox->SetCurSel( 0 );
			CTable *pPipeTab = dynamic_cast<CTable*>( (CData *)pclComboBox->GetItemData( 0 ) );

			if( NULL != pCstr )
			{
				*pCstr = pPipeTab->GetIDPtr().ID;
			}
		}
	}

	if( iCount < 2 )
	{
		pclComboBox->EnableWindow( FALSE );
	}
	else
	{
		pclComboBox->EnableWindow( TRUE );
	}

	pList->PurgeAll();
}
