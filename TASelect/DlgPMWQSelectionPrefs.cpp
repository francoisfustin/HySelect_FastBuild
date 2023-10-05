#include "stdafx.h"
#include "TASelect.h"
#include "DlgPMWQSelectionPrefs.h"


IMPLEMENT_DYNAMIC( CDlgPMWQSelectionPrefs, CDialogEx )

CDlgPMWQSelectionPrefs::CDlgPMWQSelectionPrefs( CPMWQPrefs *pclPMWQSelectionPreferences, CTADatabase *pTADB, CWnd *pParent )
		: CDialogEx( CDlgPMWQSelectionPrefs::IDD, pParent )
{
	m_pclPMWQSelectionPreferences = pclPMWQSelectionPreferences;
	m_pTADB = pTADB;

	// Make a copy to check if user has changed on option when clicking on 'OK'.
	m_bChanges = false;
	m_clPMWQSelectPrefCopy.CopyFrom( pclPMWQSelectionPreferences );
}

void CDlgPMWQSelectionPrefs::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKGENONEDEVICE, m_clCheckGenOneDevice );
	DDX_Control( pDX, IDC_COMBOGENREDDCY, m_ComboGenRedundancy );
	DDX_Control( pDX, IDC_CHECKCPINTCOATING, m_clCheckCPInternalCoating );
	DDX_Control( pDX, IDC_CHECKCPEXTAIR, m_clCheckCPExternalAir );
	DDX_Control( pDX, IDC_CHECKPUMPDEGCOOLINGINSULATION, m_clCheckPumpDegCoolingVersion );
	DDX_Control( pDX, IDC_CHECKWMBREAKTANK, m_clCheckWMBreakTank );
	DDX_Control( pDX, IDC_CHECKWMDUTYSTDBY, m_clCheckWMDutyStandBy );
}

BOOL CDlgPMWQSelectionPrefs::OnInitDialog()
{
	if( FALSE == CDialogEx::OnInitDialog() || NULL == m_pclPMWQSelectionPreferences )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_CAPTION );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_GROUPGENERAL );
	GetDlgItem( IDC_GROUPGENERAL )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_GENONEDEVICE );
	m_clCheckGenOneDevice.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_GENREDDCY );
	GetDlgItem( IDC_STATICREDUNDANCY )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_GROUPCOMPRESSOR );
	GetDlgItem( IDC_GROUPCOMPRESSO )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_CPINTCOATING );
	m_clCheckCPInternalCoating.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_CPEXTERNALAIR );
	m_clCheckCPExternalAir.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_GROUPPUMPDEGASSING );
	GetDlgItem( IDC_GROUPPUMPDEGASSING )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_PUMPDEGCOOLING );
	m_clCheckPumpDegCoolingVersion.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_GROUPWATERMAKEUP );
	GetDlgItem( IDC_GROUPWATERMAKEUP )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_WMBREAKTANK );
	m_clCheckWMBreakTank.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPMWQSELPREFS_WMDUTYSTDBY );
	m_clCheckWMDutyStandBy.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );

	m_clCheckGenOneDevice.SetCheck( ( true == m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? BST_CHECKED : BST_UNCHECKED );
	m_clCheckGenOneDevice.EnableWindow( ( true == m_pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? FALSE : TRUE );
	_FillComboGenRedundancy();
	// HYS-1537 : Unchek checkbox when it is disabled and checked
	if( false == m_clCheckGenOneDevice.IsWindowEnabled() && BST_CHECKED == m_clCheckGenOneDevice.GetCheck() )
	{
		m_clCheckGenOneDevice.SetCheck( BST_UNCHECKED );
	}
	
	m_clCheckCPInternalCoating.SetCheck( ( true == m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::CompressoInternalCoating ) ) ? BST_CHECKED : BST_UNCHECKED );
	m_clCheckCPInternalCoating.EnableWindow( ( true == m_pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::CompressoInternalCoating ) ) ? FALSE : TRUE );
	if( false == m_clCheckCPInternalCoating.IsWindowEnabled() && BST_CHECKED == m_clCheckCPInternalCoating.GetCheck() )
	{
		m_clCheckCPInternalCoating.SetCheck( BST_UNCHECKED );
	}

	m_clCheckCPExternalAir.SetCheck( ( true == m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::CompressoExternalAir ) ) ? BST_CHECKED : BST_UNCHECKED );
	m_clCheckCPExternalAir.EnableWindow( ( true == m_pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::CompressoExternalAir ) ) ? FALSE : TRUE );
	if( false == m_clCheckCPExternalAir.IsWindowEnabled() && BST_CHECKED == m_clCheckCPExternalAir.GetCheck() )
	{
		m_clCheckCPExternalAir.SetCheck( BST_UNCHECKED );
	}

	m_clCheckPumpDegCoolingVersion.SetCheck( ( true == m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion ) ) ? BST_CHECKED : BST_UNCHECKED );
	m_clCheckPumpDegCoolingVersion.EnableWindow( ( true == m_pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion ) ) ? FALSE : TRUE );
	if( false == m_clCheckPumpDegCoolingVersion.IsWindowEnabled() && BST_CHECKED == m_clCheckPumpDegCoolingVersion.GetCheck() )
	{
		m_clCheckPumpDegCoolingVersion.SetCheck( BST_UNCHECKED );
	}
	
	m_clCheckWMBreakTank.SetCheck( ( true == m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::WaterMakeupBreakTank ) ) ? BST_CHECKED : BST_UNCHECKED );
	m_clCheckWMBreakTank.EnableWindow( ( true == m_pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::WaterMakeupBreakTank ) ) ? FALSE : TRUE );
	if( false == m_clCheckWMBreakTank.IsWindowEnabled() && BST_CHECKED == m_clCheckWMBreakTank.GetCheck() )
	{
		m_clCheckWMBreakTank.SetCheck( BST_UNCHECKED );
	}
	m_clCheckWMDutyStandBy.SetCheck( ( true == m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy ) ) ? BST_CHECKED : BST_UNCHECKED );
	m_clCheckWMDutyStandBy.EnableWindow( ( true == m_pclPMWQSelectionPreferences->IsDisabled( CPMWQPrefs::WaterMakeupDutyStandBy ) ) ? FALSE : TRUE );
	if( false == m_clCheckWMDutyStandBy.IsWindowEnabled() && BST_CHECKED == m_clCheckWMDutyStandBy.GetCheck() )
	{
		m_clCheckWMDutyStandBy.SetCheck( BST_UNCHECKED );
	}
	return TRUE;
}

void CDlgPMWQSelectionPrefs::OnOK()
{
	m_pclPMWQSelectionPreferences->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, ( BST_CHECKED == m_clCheckGenOneDevice.GetCheck() ) ? true : false );
	
	CData *pclData = ( CData * )( m_ComboGenRedundancy.GetItemData( m_ComboGenRedundancy.GetCurSel() ) );
	m_pclPMWQSelectionPreferences->SetRedundancy( (wchar_t*)&pclData->GetIDPtr().ID[0] );
	m_pclPMWQSelectionPreferences->SetChecked( CPMWQPrefs::CompressoInternalCoating, ( BST_CHECKED == m_clCheckCPInternalCoating.GetCheck() ) ? true : false );
	m_pclPMWQSelectionPreferences->SetChecked( CPMWQPrefs::CompressoExternalAir, ( BST_CHECKED == m_clCheckCPExternalAir.GetCheck() ) ? true : false );
	m_pclPMWQSelectionPreferences->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, ( BST_CHECKED == m_clCheckPumpDegCoolingVersion.GetCheck() ) ? true : false );
	m_pclPMWQSelectionPreferences->SetChecked( CPMWQPrefs::WaterMakeupBreakTank, ( BST_CHECKED == m_clCheckWMBreakTank.GetCheck() ) ? true : false );
	m_pclPMWQSelectionPreferences->SetChecked( CPMWQPrefs::WaterMakeupDutyStandBy, ( BST_CHECKED == m_clCheckWMDutyStandBy.GetCheck() ) ? true : false );

	// HYS-1537 : If the checkbox is disabled, no changes.
	if( m_clPMWQSelectPrefCopy.IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) != m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) 
		&& false == m_clPMWQSelectPrefCopy.IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) )
	{
		m_bChanges = true;
	}
	else if( m_clPMWQSelectPrefCopy.IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) != m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp )
		&& false == m_clPMWQSelectPrefCopy.IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp ) )
	{
		m_bChanges = true;
	}
	else if( m_clPMWQSelectPrefCopy.IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) != m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) 
		&& false == m_clPMWQSelectPrefCopy.IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) )
	{
		m_bChanges = true;
	}
	else if( m_clPMWQSelectPrefCopy.IsChecked( CPMWQPrefs::CompressoInternalCoating ) != m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::CompressoInternalCoating ) 
		&& false == m_clPMWQSelectPrefCopy.IsDisabled( CPMWQPrefs::CompressoInternalCoating ) )
	{
		m_bChanges = true;
	}
	else if( m_clPMWQSelectPrefCopy.IsChecked( CPMWQPrefs::CompressoExternalAir ) != m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::CompressoExternalAir ) 
		&& false == m_clPMWQSelectPrefCopy.IsDisabled( CPMWQPrefs::CompressoExternalAir ) )
	{
		m_bChanges = true;
	}
	else if( m_clPMWQSelectPrefCopy.IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion ) != m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion ) 
		&& false == m_clPMWQSelectPrefCopy.IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion ) )
	{
		m_bChanges = true;
	}
	else if( m_clPMWQSelectPrefCopy.IsChecked( CPMWQPrefs::WaterMakeupBreakTank ) != m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::WaterMakeupBreakTank ) 
        && false == m_clPMWQSelectPrefCopy.IsDisabled( CPMWQPrefs::WaterMakeupBreakTank ) )
	{
		m_bChanges = true;
	}
	else if( m_clPMWQSelectPrefCopy.IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy ) != m_pclPMWQSelectionPreferences->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy ) 
	    && false == m_clPMWQSelectPrefCopy.IsDisabled( CPMWQPrefs::WaterMakeupDutyStandBy ) )
	{
		m_bChanges = true;
	}

	CDialogEx::OnOK();
}

void CDlgPMWQSelectionPrefs::_FillComboGenRedundancy()
{
	if( NULL == m_pTADB )
	{
		ASSERT_RETURN;
	}

	CTable *pTable = dynamic_cast<CTable *>( ( CData * )( m_pTADB->Get( _T("PMREDUNDANCY_TAB") ).MP ) );
	ASSERT( NULL != pTable );

	if( NULL == pTable )
	{
		return;
	}

	m_ComboGenRedundancy.ResetContent();
	int iSel = 0;

	for( IDPTR IDPtr = pTable->GetFirst(); _NULL_IDPTR != IDPtr; IDPtr = pTable->GetNext() )
	{
		CDB_StringID *pclStringID = dynamic_cast<CDB_StringID *>( ( CData * )IDPtr.MP );
		ASSERT( NULL != pclStringID );

		if( NULL == pclStringID )
		{
			continue;
		}

		int iIndex = m_ComboGenRedundancy.AddString( pclStringID->GetString() );
		m_ComboGenRedundancy.SetItemData( iIndex, ( DWORD_PTR )IDPtr.MP );

		if( m_pclPMWQSelectionPreferences->IsRedundancyIDMatch( IDPtr.ID ) )
		{
			iSel = iIndex;
		}
	}

	m_ComboGenRedundancy.SetCurSel( iSel );
}
