#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralPressureMaintenance.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneralPressureMaintenance, CDlgCtrlPropPage )

CDlgTPGeneralPressureMaintenance::CDlgTPGeneralPressureMaintenance( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHPM_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHPM_PAGENAME );
}

void CDlgTPGeneralPressureMaintenance::Init( bool fResetToDefault )
{
	CDS_TechnicalParameter *pTechP;

	if( true == fResetToDefault )
	{
		pTechP = (CDS_TechnicalParameter*)( TASApp.GetpTADB()->Get( _T("PARAM_TECH") ).MP );
	}
	else
	{
		pTechP = TASApp.GetpTADS()->GetpTechParams();
	}

	// Set the text for the 'pressure.volume' unit.
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	CString strUnit = pUnitDB->GetNameOfDefaultUnit( _U_PRESSURE ).c_str();
	strUnit += CString( _T(".") ) + pUnitDB->GetNameOfDefaultUnit( _U_VOLUME ).c_str();
	SetDlgItemText( IDC_STATICPRESSVOLLIMITUNIT, strUnit );

	m_clCheckPressureVolumeLimit.SetCheck( ( true == pTechP->GetUseVesselPressureVolumeLimit() ) ? BST_CHECKED : BST_UNCHECKED );

	// Initialize the edit boxes.
	m_clEditPressureVolumeLimit.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	
	// 'Pressure.volume' is a multiplication between a pressure and a volume, we thus don't set a physical unit.
	m_clEditPressureVolumeLimit.SetPhysicalType( _U_NODIM );
	
	// Value in 'localdb.txt' is set in SI units (i.e. Pascal.m3). To display value with the current unit, we must convert value SI to
	// CU in 2 phases. First one for the pressure, and second one for the volume.
	double dPressVolLimitCUP = CDimValue::SItoCU( _U_PRESSURE, pTechP->GetPressureVolumeLimit() );
	double dPressVolLimitCUPV = CDimValue::SItoCU( _U_VOLUME, dPressVolLimitCUP );
	m_clEditPressureVolumeLimit.SetCurrentValSI( dPressVolLimitCUPV );
	m_clEditPressureVolumeLimit.Update();
	
	if( BST_CHECKED == pTechP->GetUseVesselPressureVolumeLimit() && true == pTechP->GetCanChangePressureVolumeLimit() )
		m_clEditPressureVolumeLimit.EnableWindow( TRUE );
	else
		m_clEditPressureVolumeLimit.EnableWindow( FALSE );

	// HYS-1126 : Simplify pst calculation checkbox 
	m_clCheckSimplifyPst.SetCheck( ( true == pTechP->GetUseSimplyPst() ) ? BST_CHECKED : BST_UNCHECKED );

	m_clEditMaxAdditionalVesselsInParallel.SetEditType( CNumString::eEditType::eINT, CNumString::eEditSign::ePositive );
	m_clEditMaxAdditionalVesselsInParallel.SetPhysicalType( _U_NODIM );
	m_clEditMaxAdditionalVesselsInParallel.SetCurrentValSI( pTechP->GetMaxNumberOfAdditionalVesselsInParallel() );
	m_clEditMaxAdditionalVesselsInParallel.SetMinIntValue( 1 );
	m_clEditMaxAdditionalVesselsInParallel.Update();

	m_clEditMaxVentoInParallel.SetEditType( CNumString::eEditType::eINT, CNumString::eEditSign::ePositive );
	m_clEditMaxVentoInParallel.SetPhysicalType( _U_NODIM );
	m_clEditMaxVentoInParallel.SetCurrentValSI( pTechP->GetMaxNumberOfVentoInParallel() );
	m_clEditMaxVentoInParallel.SetMinIntValue( 1 );
	m_clEditMaxVentoInParallel.Update();

	m_clEditVesselSelectionRangePercentage.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_clEditVesselSelectionRangePercentage.SetPhysicalType( _U_NODIM );
	m_clEditVesselSelectionRangePercentage.SetCurrentValSI( pTechP->GetVesselSelectionRangePercentage() );
	m_clEditVesselSelectionRangePercentage.SetMinDblValue( 0.0 );
	m_clEditVesselSelectionRangePercentage.SetMaxDblValue( 5.0 );
	m_clEditVesselSelectionRangePercentage.Update();
}

void CDlgTPGeneralPressureMaintenance::Save( CString strSectionName )
{
	CDS_TechnicalParameter* pTech = TASApp.GetpTADS()->GetpTechParams();
	if( NULL != GetSafeHwnd() )
	{
		pTech->SetUseVesselPressureVolumeLimit( ( BST_CHECKED == m_clCheckPressureVolumeLimit.GetCheck() ) ? true : false );
		
		// 'm_clEditPressureVolumeLimit' has not physical type. 'GetCurrentValSI' returns thus in fact the current value in the current unit.
		// To save in technical parameter, we must convert this CU value in SI.
		double dPressVolLimitCUPV = m_clEditPressureVolumeLimit.GetCurrentValSI();
		double dPressVolLimitCUP = CDimValue::CUtoSI( _U_VOLUME, dPressVolLimitCUPV );
		double dPressVolLimitSI = CDimValue::CUtoSI( _U_PRESSURE, dPressVolLimitCUP );
		pTech->SetPressureVolumeLimit( dPressVolLimitSI );
		
		// HYS-1126: Simplify calculation of static pressure
		pTech->SetUseSimplyPst( ( BST_CHECKED == m_clCheckSimplifyPst.GetCheck() ) ? true : false );

		CString str;
		m_clEditMaxAdditionalVesselsInParallel.GetWindowText( str );
		int iMaxVessel = _ttoi( str );
		pTech->SetMaxNumberOfAdditionalVesselsInParallel( iMaxVessel );
		m_clEditMaxVentoInParallel.GetWindowText( str );
		int iMaxVento = _ttoi( str );
		pTech->SetMaxNumberOfVentoInParallel( iMaxVento );
		
		m_clEditVesselSelectionRangePercentage.GetWindowText( str );
		double dVesselSelectionRangePercentage = 0.0;
		ReadDouble( str, &dVesselSelectionRangePercentage );
		pTech->SetVesselSelectionRangePercentage( dVesselSelectionRangePercentage );
	}
}

bool CDlgTPGeneralPressureMaintenance::VerifyInfo()
{
	bool fResult = true;

	// Verify the edit 'pressure.volume'.
	double dValue;
	switch( ReadDouble( m_clEditPressureVolumeLimit, &dValue ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			break;
	}

	// Verify the edit 'max additional vessels in parallel'.
	if( true == fResult )
	{
		switch( ReadDouble( m_clEditMaxAdditionalVesselsInParallel, &dValue ) )
		{
			case RD_EMPTY:
				fResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				break;
		
			case RD_OK:
				break;
		}
	}

	// Verify the edit 'max Vento in parallel'.
	if( true == fResult )
	{
		switch( ReadDouble( m_clEditMaxVentoInParallel, &dValue ) )
		{
			case RD_EMPTY:
				fResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				break;
		
			case RD_OK:
				break;
		}
	}

	// Verify the edit 'Propose vessels below nominal volume'.
	if (true == fResult)
	{
		switch (ReadDouble(m_clEditVesselSelectionRangePercentage, &dValue))
		{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
			break;

		case RD_OK:
			break;
		}
	}

	return fResult;
}

BEGIN_MESSAGE_MAP( CDlgTPGeneralPressureMaintenance, CDlgCtrlPropPage )
	ON_BN_CLICKED( IDC_CHECKPRESSVOLLIMIT, OnBnClickedPressureVolumeLimit )
END_MESSAGE_MAP()

void CDlgTPGeneralPressureMaintenance::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKPRESSVOLLIMIT, m_clCheckPressureVolumeLimit );
	DDX_Control( pDX, IDC_CHECK_PST, m_clCheckSimplifyPst );
	DDX_Control( pDX, IDC_EDITPRESSVOLLIMIT, m_clEditPressureVolumeLimit );
	DDX_Control( pDX, IDC_EDITMAXADDITIONALVESSELS, m_clEditMaxAdditionalVesselsInParallel );
	DDX_Control( pDX, IDC_EDITMAXVENTO, m_clEditMaxVentoInParallel );
	DDX_Control( pDX, IDC_EDITVESSELSELECTIONRANGEPERCENTAGE, m_clEditVesselSelectionRangePercentage );
}

BOOL CDlgTPGeneralPressureMaintenance::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPM_STATICPRESSVOLLIMIT );
	GetDlgItem( IDC_STATICPRESSVOLLIMIT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHVALV_STATICMAXADDITIONALVESSELS );
	GetDlgItem( IDC_STATICMAXADDITIONALVESSELS )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHVALV_STATICMAXVENTO );
	GetDlgItem( IDC_STATICMAXVENTO )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHVALV_STATICVESSELSELECTION );
	GetDlgItem( IDC_STATICVESSELSELECTIONRANGEPERCENTAGE )->SetWindowText(str);

	// HYS-1126 : Simplify pst calculation checkbox 
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPM_STATICSIMPLEPST );
	GetDlgItem( IDC_STATICSIMPLEPST )->SetWindowText( str );

	Init();

	return TRUE;
}

void CDlgTPGeneralPressureMaintenance::OnBnClickedPressureVolumeLimit()
{
	CDS_TechnicalParameter* pTechP = TASApp.GetpTADS()->GetpTechParams();
	if( BST_CHECKED == m_clCheckPressureVolumeLimit.GetCheck() && true == pTechP->GetCanChangePressureVolumeLimit() )
		m_clEditPressureVolumeLimit.EnableWindow( TRUE );
	else
		m_clEditPressureVolumeLimit.EnableWindow( FALSE );
}

