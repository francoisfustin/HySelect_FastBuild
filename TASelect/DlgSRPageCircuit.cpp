#include "stdafx.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "HMTreeDroptarget.h"
#include "HMTreeListCtrl.h"
#include "DlgSearchReplace.h"
#include "DlgSRPageCircuit.h"


IMPLEMENT_DYNAMIC( CDlgSRPageCircuit, CDlgCtrlPropPage )

CDlgSRPageCircuit::CDlgSRPageCircuit( CWnd *pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_bApplyToAll = false;
	m_pDlgSearchAndReplaceCircuits = NULL;
	m_mapSelectedCicuits.clear();
	m_mapSelCirIt = m_mapSelectedCicuits.end();
	m_mapModifiedHM.clear();
	m_mapModifiedHMIt = m_mapModifiedHM.end();
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGSRCIRCUIT_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGSRCIRCUIT_HEADERTITLE );
	
	m_bAtLeastOneCircuitTypeWithCVSrc = false;

	// Init radio buttons.
	m_eRadioBalancingTypeDest = CDB_CircuitScheme::eBALTYPE::SP;
	m_iRadioBalancingTypeDest = 0;

	m_eRadioValveTypeDest = _eWorkForCV::ewcvTA;
	m_iRadioValveTypeDest = 0;

	m_eRadioControlTypeDest = CDB_ControlProperties::CvCtrlType::eCvProportional;
	m_iRadioControlTypeDest = 0;

	m_eRadioControlValveTypeDest = CDB_ControlProperties::eCVFUNC::ControlOnly;
	m_iRadioControlValveTypeDest = 0;

	m_bOnSelchangeListCircuitTypeEnabled = true;
}

bool CDlgSRPageCircuit::OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams )
{
	m_mapModifiedHM.clear();
	
	// Remove all circuits from selected list that doesn't match source criteria.
	for( m_mapSelCirIt = m_mapSelectedCicuits.begin(); m_mapSelCirIt != m_mapSelectedCicuits.end(); )
	{
		CDS_HydroMod *pHM = m_mapSelCirIt->second;

		if( NULL != pHM )
		{
			if( false == _CheckCompatibilityHMvsCB( pHM ) )
			{
				// Remove this circuit.
				std::map<int, CDS_HydroMod *>::iterator it = m_mapSelCirIt;
				++m_mapSelCirIt;
				m_mapSelectedCicuits.erase( it );
				continue;
			}
		}
		
		++m_mapSelCirIt; 
	}

	bool bBalType = false;
	bool bWorkForCtrlValv = false;
	bool bCtrlType = false;
	bool bCtrlValvType = false;

	m_mapSelCirIt = m_mapSelectedCicuits.begin();

	if( m_mapSelCirIt != m_mapSelectedCicuits.end() )
	{
		CDS_HydroMod *pHM = m_mapSelCirIt->second;

		if( NULL != pHM )
		{
			if( TRUE == m_GroupBalTypeDest.IsWindowEnabled() )
			{
				bBalType = true;
			}

			CDB_CircSchemeCateg *pclCircSchemeCategory = NULL;
			POSITION pos = m_ListBoxCircuitTypeDest.GetFirstSelectedItemPosition();

			if( NULL != pos )
			{
				int nItem = m_ListBoxCircuitTypeDest.GetNextSelectedItem( pos );
				pclCircSchemeCategory = (CDB_CircSchemeCateg *)m_ListBoxCircuitTypeDest.GetItemData( nItem );
			}
			
			if( TRUE == m_GroupValveTypeDest.IsWindowEnabled() )
			{
				bWorkForCtrlValv = true;
			}

			if( TRUE == m_GroupControlTypeDest.IsWindowEnabled() )
			{
				bCtrlType = true;
			}

			if( TRUE == m_GroupControlValveTypeDest.IsWindowEnabled() )
			{
				bCtrlValvType = true;
			}

			// Display the 'CDlgReplacePopup' dialog.
			m_bApplyToAll = false;
			CDlgSearchAndReplaceCircuits Dlg( this );
			m_pDlgSearchAndReplaceCircuits = &Dlg;

			Dlg.SetFlagTarget( bBalType, bWorkForCtrlValv, bCtrlType, bCtrlValvType );
			Dlg.SetTarget( m_eRadioBalancingTypeDest, pclCircSchemeCategory, m_eRadioValveTypeDest, m_eRadioControlTypeDest, m_eRadioControlValveTypeDest );	
			Dlg.SetHMLeftTreeWidth( pDlgSearchReplace->GetLeftTreeWidth() );
			Dlg.UpdateData( pHM );
			Dlg.DoModal();
		}
	}

	return true;
}

void CDlgSRPageCircuit::Init( bool bRstToDefault )
{
	m_mapSelectedCicuits.clear();

	if( NULL != pDlgSearchReplace )
	{
		pDlgSearchReplace->EnableButtonStart( FALSE );
	}

	m_CheckWhereModules.SetCheck( BST_CHECKED );
	m_CheckWhereCircuits.SetCheck( BST_CHECKED );
	m_CheckWhereApplyToLockedCircuit.SetCheck( BST_CHECKED );

	m_CheckBTNoneSrc.SetCheck( BST_UNCHECKED );
	m_CheckBTManualBalancingSrc.SetCheck( BST_UNCHECKED );
	m_CheckBTDiffPressureControlSrc.SetCheck( BST_UNCHECKED );
	m_CheckBTElectronicSrc.SetCheck( BST_UNCHECKED );
	m_CheckBTElectronicDpCSrc.SetCheck( BST_UNCHECKED );

	m_iRadioBalancingTypeDest = 0;

	m_CheckValveTypeIMISrc.SetCheck( BST_UNCHECKED );
	m_CheckValveTypeKvsSrc.SetCheck( BST_UNCHECKED );

	m_iRadioValveTypeDest = 0;

	m_CheckControlTypeProportionalSrc.SetCheck( BST_UNCHECKED );
	m_CheckControlType3PointsSrc.SetCheck( BST_UNCHECKED );
	m_CheckControlTypeOnOffSrc.SetCheck( BST_UNCHECKED );
	
	m_iRadioControlTypeDest = 0;

	m_CheckControlValveTypeCombBalCVSrc.SetCheck( BST_UNCHECKED );
	m_CheckControlValveTypeStandardCVSrc.SetCheck( BST_UNCHECKED );
	m_CheckControlValveTypePresetCvSrc.SetCheck( BST_UNCHECKED );
	m_CheckControlValveTypePIBCVSrc.SetCheck( BST_UNCHECKED );
	m_CheckControlValveTypeSmartSrc.SetCheck( BST_UNCHECKED );

	m_iRadioControlValveTypeDest = 0;
	UpdateData( FALSE );

	_InitListsControlCircuit();

	// Force a refresh of tree status
	OnTreeItemCheckStatusChanged();
}

BEGIN_MESSAGE_MAP( CDlgSRPageCircuit, CDlgCtrlPropPage )
	ON_BN_CLICKED( IDC_CHECK_MODULES, &OnClickedCheckModules )
	ON_BN_CLICKED( IDC_CHECK_CIRCUITS, &OnClickedCheckCircuits )
	ON_BN_CLICKED( IDC_CHECK_APPLYLOCKED, &OnBnClickedCheckApplylocked )

	ON_BN_CLICKED( IDC_CHECKMB_F, &OnBnClickedCheckBalancingType )
	ON_BN_CLICKED( IDC_CHECKNONE_F, &OnBnClickedCheckBalancingType )
	ON_BN_CLICKED( IDC_CHECKDPC_F, &OnBnClickedCheckBalancingType )
	ON_BN_CLICKED( IDC_CHECKELEC_F, &OnBnClickedCheckBalancingType )
	ON_BN_CLICKED( IDC_CHECKELECDPC_F, &OnBnClickedCheckBalancingType )

	ON_BN_CLICKED( IDC_RADIONONE, &OnBnClickedRadioManBalancingType )
	ON_BN_CLICKED( IDC_RADIOMB, &OnBnClickedRadioManBalancingType )
	ON_BN_CLICKED( IDC_RADIODPC, &OnBnClickedRadioManBalancingType )
	ON_BN_CLICKED( IDC_RADIOELEC, &OnBnClickedRadioManBalancingType )
	ON_BN_CLICKED( IDC_RADIOELECDPC, &OnBnClickedRadioManBalancingType )

	ON_NOTIFY( LVN_ITEMCHANGED, IDC_LISTCIRCUITTYPE, &OnSelchangeListCircuitTypeSrc )
	ON_NOTIFY( LVN_ITEMCHANGED, IDC_LISTCIRCUITTYPE_R, &OnSelchangeListCircuitTypeDest )

	ON_BN_CLICKED( IDC_CHECKIMI_F, &OnBnClickedCheckIMIValveType )
	ON_BN_CLICKED( IDC_RADIOIMI, &OnBnClickedRadioValveTypeDest )

	ON_BN_CLICKED( IDC_CHECKKVS_F, &OnBnClickedCheckKvsValveType )
	ON_BN_CLICKED( IDC_RADIOKVS, &OnBnClickedRadioValveTypeDest )

	ON_BN_CLICKED( IDC_CHECKPROP_F, &OnBnClickedCheckControlType )
	ON_BN_CLICKED( IDC_CHECK3PT_F, &OnBnClickedCheckControlType )
	ON_BN_CLICKED( IDC_CHECKONOFF_F, &OnBnClickedCheckControlType )
	
	ON_BN_CLICKED( IDC_RADIOPROP, &OnBnClickedRadioControlType )
	ON_BN_CLICKED( IDC_RADIO3PT, &OnBnClickedRadioControlType )
	ON_BN_CLICKED( IDC_RADIOONOFF, &OnBnClickedRadioControlType)
	
	ON_BN_CLICKED( IDC_CHECKBCV_F, &OnBnClickedCheckControlValveType )
	ON_BN_CLICKED( IDC_CHECKCV_F, &OnBnClickedCheckControlValveType )
	ON_BN_CLICKED( IDC_CHECKSETCV_F, &OnBnClickedCheckControlValveType )
	ON_BN_CLICKED( IDC_CHECKPIBCV_F, &OnBnClickedCheckControlValveType )
	ON_BN_CLICKED( IDC_CHECKSMART_F, &OnBnClickedCheckControlValveType )
	
	ON_BN_CLICKED( IDC_RADIOCV, &OnBnClickedRadioControlValveType )
	ON_BN_CLICKED( IDC_RADIOCBCV, &OnBnClickedRadioControlValveType )
	ON_BN_CLICKED( IDC_RADIOSETCV, &OnBnClickedRadioControlValveType )
	ON_BN_CLICKED( IDC_RADIOPIBCV, &OnBnClickedRadioControlValveType )
	ON_BN_CLICKED( IDC_RADIOSMART, &OnBnClickedRadioControlValveType )

	ON_MESSAGE( WM_USER_HMTREEITEMCHECKCHANGED, &OnTreeItemCheckStatusChanged )
	ON_MESSAGE( WM_USER_REPLACEPOPUPFINDNEXT, &OnFindNextCircuit )
	ON_MESSAGE( WM_USER_REPLACEPOPUPREPLACE, &OnReplaceCircuit )
	ON_MESSAGE( WM_USER_REPLACEPOPUPREPLACEALL, &OnReplaceAllCircuit )
	ON_MESSAGE( WM_USER_CLOSEPOPUPWND, &OnClosePopup )
END_MESSAGE_MAP()

void CDlgSRPageCircuit::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_CHECKNONE_F, m_CheckBTNoneSrc );
	DDX_Control( pDX, IDC_CHECKMB_F, m_CheckBTManualBalancingSrc );
	DDX_Control( pDX, IDC_CHECKDPC_F, m_CheckBTDiffPressureControlSrc );
	DDX_Control( pDX, IDC_CHECKELEC_F, m_CheckBTElectronicSrc );
	DDX_Control( pDX, IDC_CHECKELECDPC_F, m_CheckBTElectronicDpCSrc );
	
	DDX_Radio( pDX, IDC_RADIONONE, m_iRadioBalancingTypeDest );

	DDX_Control( pDX, IDC_LISTCIRCUITTYPE, m_ListBoxCircuitTypeSrc );
	DDX_Control( pDX, IDC_LISTCIRCUITTYPE_R, m_ListBoxCircuitTypeDest );

	DDX_Control( pDX, IDC_CHECKIMI_F, m_CheckValveTypeIMISrc );
	DDX_Control( pDX, IDC_CHECKKVS_F, m_CheckValveTypeKvsSrc );
	
	DDX_Radio( pDX, IDC_RADIOIMI, m_iRadioValveTypeDest );
	
	DDX_Control( pDX, IDC_CHECKPROP_F, m_CheckControlTypeProportionalSrc );
	DDX_Control( pDX, IDC_CHECK3PT_F, m_CheckControlType3PointsSrc );
	DDX_Control( pDX, IDC_CHECKONOFF_F, m_CheckControlTypeOnOffSrc );

	DDX_Radio( pDX, IDC_RADIOPROP, m_iRadioControlTypeDest );

	DDX_Control( pDX, IDC_CHECKBCV_F, m_CheckControlValveTypeCombBalCVSrc );
	DDX_Control( pDX, IDC_CHECKCV_F, m_CheckControlValveTypeStandardCVSrc );
	DDX_Control( pDX, IDC_CHECKSETCV_F, m_CheckControlValveTypePresetCvSrc );
	DDX_Control( pDX, IDC_CHECKPIBCV_F, m_CheckControlValveTypePIBCVSrc );
	DDX_Control( pDX, IDC_CHECKSMART_F, m_CheckControlValveTypeSmartSrc );

	DDX_Radio( pDX, IDC_RADIOCV, m_iRadioControlValveTypeDest );

	DDX_Control( pDX, IDC_CHECK_APPLYLOCKED, m_CheckWhereApplyToLockedCircuit );
	DDX_Control( pDX, IDC_CHECK_MODULES, m_CheckWhereModules );
	DDX_Control( pDX, IDC_CHECK_CIRCUITS, m_CheckWhereCircuits );

	DDX_Control( pDX, IDC_GROUP_FIND, m_GroupWhereSrc );
	DDX_Control( pDX, IDC_GROUP_FINDBV, m_GroupBalancingTypeSrc );
	DDX_Control( pDX, IDC_GROUP_REPBV, m_GroupBalTypeDest );
	DDX_Control( pDX, IDC_GROUP_FINDVT, m_GroupValveTypeSrc );
	DDX_Control( pDX, IDC_GROUP_REPVT, m_GroupValveTypeDest );
	DDX_Control( pDX, IDC_GROUP_FINDCT, m_GroupControlTypeSrc );
	DDX_Control( pDX, IDC_GROUP_REPCT, m_GroupControlTypeDest );
	DDX_Control( pDX, IDC_GROUP_FINDCVT, m_GroupControlValveTypeSrc );
	DDX_Control( pDX, IDC_GROUP_REPCVT, m_GroupControlValveTypeDest );
	DDX_Control( pDX, IDC_GPFIND, m_GroupFind );
	DDX_Control( pDX, IDC_GPREPLACE, m_GroupReplace );
}

BOOL CDlgSRPageCircuit::OnInitDialog()
{
	CDlgCtrlPropPage::OnInitDialog();


	GetDlgItem( IDC_STATIC_CIRCTYPE )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLMODE ) );
	GetDlgItem( IDC_STATIC_CIRCTYPE_R )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLMODE ) );

	m_GroupFind.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_FINDWHAT ) );
	m_GroupReplace.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_REPLACEWITH ) );
	m_GroupWhereSrc.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_WHERE ) );

	m_GroupFind.SetBckgndColor( _WHITE_DLGBOX );
	m_GroupWhereSrc.SetBckgndColor( _WHITE_DLGBOX );
	m_GroupReplace.SetBckgndColor( _WHITE_DLGBOX );

	m_CheckWhereModules.SetWindowText(TASApp.LoadLocalizedString(IDS_DLGSEARCHREPLACECIRCUIT_CHECKMODULES));
	m_CheckWhereCircuits.SetWindowText(TASApp.LoadLocalizedString(IDS_DLGSEARCHREPLACECIRCUIT_CHECKCIRCUITS));
	m_CheckWhereApplyToLockedCircuit.SetWindowText(TASApp.LoadLocalizedString(IDS_DLGSEARCHREPLACECIRCUIT_APPLYTOLOCKCIRCUITS));

	m_GroupBalancingTypeSrc.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_BALTYPE ) );
	m_GroupBalancingTypeSrc.SaveChildList();
	m_GroupBalancingTypeSrc.EnableWindow( false, true );
	m_GroupBalTypeDest.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_BALTYPE ) );
	m_GroupBalTypeDest.SaveChildList();
	m_GroupBalTypeDest.EnableWindow( false, true );
	m_GroupBalancingTypeSrc.SetBckgndColor( _WHITE_DLGBOX );
	m_GroupBalTypeDest.SetBckgndColor( _WHITE_DLGBOX );

	m_CheckBTNoneSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_NONE ) );
	m_CheckBTManualBalancingSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_MANBAL ) );
	m_CheckBTDiffPressureControlSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_DPCBAL ) );
	m_CheckBTElectronicSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_ELECBAL ) );
	m_CheckBTElectronicDpCSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_ELECDPC ) );
	GetDlgItem( IDC_RADIONONE )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_NONE ) );
	GetDlgItem( IDC_RADIOMB )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_MANBAL ) );
	GetDlgItem( IDC_RADIODPC )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_DPCBAL ) );
	GetDlgItem( IDC_RADIOELEC )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_ELECBAL ) );
	GetDlgItem( IDC_RADIOELECDPC )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_ELECDPC ) );

	m_GroupValveTypeSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_VALVETYPE ) );
	m_GroupValveTypeSrc.SaveChildList();
	m_GroupValveTypeSrc.EnableWindow( false, true );
	m_GroupValveTypeDest.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_VALVETYPE ) );
	m_GroupValveTypeDest.SaveChildList();
	m_GroupValveTypeDest.EnableWindow( false, true );
	m_GroupValveTypeSrc.SetBckgndColor( _WHITE_DLGBOX );
	m_GroupValveTypeDest.SetBckgndColor( _WHITE_DLGBOX );

	m_CheckValveTypeIMISrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CVTAVALVES ) );
	m_CheckValveTypeKvsSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CVKVVALUES ) );
	GetDlgItem( IDC_RADIOIMI )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CVTAVALVES ) );
	GetDlgItem( IDC_RADIOKVS )->SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CVKVVALUES ) );

	m_GroupControlTypeSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLTYPE ) );
	m_GroupControlTypeSrc.SaveChildList();
	m_GroupControlTypeSrc.EnableWindow( false, true );
	m_GroupControlTypeDest.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLTYPE ) );
	m_GroupControlTypeDest.SaveChildList();
	m_GroupControlTypeDest.EnableWindow( false, true );
	m_GroupControlTypeSrc.SetBckgndColor( _WHITE_DLGBOX );
	m_GroupControlTypeDest.SetBckgndColor( _WHITE_DLGBOX );

	m_CheckControlTypeProportionalSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLPROP ) );
	m_CheckControlType3PointsSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRL3PTS ) );
	m_CheckControlTypeOnOffSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLONOFF ) );
	GetDlgItem( IDC_RADIOPROP )->SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLPROP ) );
	GetDlgItem( IDC_RADIO3PT )->SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRL3PTS ) );
	GetDlgItem( IDC_RADIOONOFF )->SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLONOFF ) );

	m_GroupControlValveTypeSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLVALVTYPE ) );
	m_GroupControlValveTypeSrc.SaveChildList();
	m_GroupControlValveTypeSrc.EnableWindow( false, true );
	m_GroupControlValveTypeDest.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_CTRLVALVTYPE ) );
	m_GroupControlValveTypeDest.SaveChildList();
	m_GroupControlValveTypeDest.EnableWindow( false, true );
	m_GroupControlValveTypeDest.SetBckgndColor( _WHITE_DLGBOX );
	m_GroupControlValveTypeSrc.SetBckgndColor( _WHITE_DLGBOX );

	m_CheckControlValveTypeCombBalCVSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_COMBBCV ) );
	m_CheckControlValveTypeStandardCVSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_STDCTRL ) );
	m_CheckControlValveTypePresetCvSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_PRESET ) );
	m_CheckControlValveTypePIBCVSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_PIBCV ) );
	m_CheckControlValveTypeSmartSrc.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_SMART ) );
	GetDlgItem( IDC_RADIOCV )->SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_STDCTRL ) );
	GetDlgItem( IDC_RADIOCBCV )->SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_COMBBCV ) );
	GetDlgItem( IDC_RADIOSETCV )->SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_PRESET ) );
	GetDlgItem( IDC_RADIOPIBCV )->SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_PIBCV ) );
	GetDlgItem( IDC_RADIOSMART )->SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUIT_SMART ) );

	return TRUE;
}

void CDlgSRPageCircuit::OnBnClickedCheckBalancingType()
{
	HYSELECT_SETREDRAW_FALSE;

	// When no checkbox is checked the group in the replace part is disabled.
	if( BST_CHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() || BST_CHECKED == m_CheckBTManualBalancingSrc.GetCheck() 
			|| BST_CHECKED == m_CheckBTNoneSrc.GetCheck() || BST_CHECKED == m_CheckBTElectronicSrc.GetCheck()
			|| BST_CHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
	{
		m_GroupBalTypeDest.EnableWindow( true, true );
	}
	else
	{
		m_GroupBalTypeDest.EnableWindow( false, true );
	}

	// Update groups, radio buttons and checkboxes.
	_FillInBalancingTypeSrc();
	
	// Called with false value because of the impact on replace group.
	_FillInBalancingTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnBnClickedRadioManBalancingType()
{
	HYSELECT_SETREDRAW_FALSE;

	UpdateData( TRUE );

	// Update the 'm_eRadioBalancingTypeDest' enum.
	_UpdateBalancingTypeValueDest();
	
	// Update availability of components below.
	_FillInBalancingTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnBnClickedCheckIMIValveType()
{
	HYSELECT_SETREDRAW_FALSE;

	if( BST_CHECKED == m_CheckValveTypeIMISrc.GetCheck() )
	{
		m_GroupControlTypeSrc.EnableWindow( true, true );
		m_GroupControlValveTypeSrc.EnableWindow( true, true );
	}
	else
	{
		m_GroupControlTypeSrc.EnableWindow( false, true );
		m_GroupControlValveTypeSrc.EnableWindow( false, true );
	}
	
	// Update availability of components below.
	_FillInValveTypeSrc();
	_FillInValveTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnBnClickedCheckKvsValveType()
{
	HYSELECT_SETREDRAW_FALSE;

	if( BST_CHECKED == m_CheckValveTypeKvsSrc.GetCheck() && BST_UNCHECKED == m_CheckValveTypeIMISrc.GetCheck() )
	{
		m_GroupControlTypeSrc.EnableWindow( false, true );
		m_GroupControlValveTypeSrc.EnableWindow( false, true );
	}
	else
	{
		if( BST_CHECKED == m_CheckValveTypeIMISrc.GetCheck() )
		{
			m_GroupControlTypeSrc.EnableWindow( true, true );
			m_GroupControlValveTypeSrc.EnableWindow( true, true );
		}
	}
	
	// Update availability of components below.
	_FillInValveTypeSrc();
	_FillInValveTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnBnClickedRadioValveTypeDest()
{
	HYSELECT_SETREDRAW_FALSE;

	UpdateData( TRUE );

	// Update the 'm_eRadioValveTypeDest' enum.
	_UpdateValveTypeValueDest();

	if( 0 == m_iRadioValveTypeDest )
	{
		m_GroupControlTypeDest.EnableWindow( true, true );
		m_GroupControlValveTypeDest.EnableWindow( true, true );
	}
	else
	{
		m_GroupControlTypeDest.EnableWindow( false, true );
		m_GroupControlValveTypeDest.EnableWindow( false, true );
	}
	
	// Update availability of components below.
	_FillInValveTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnBnClickedCheckControlType()
{
	HYSELECT_SETREDRAW_FALSE;

	if( BST_CHECKED == m_CheckControlTypeProportionalSrc.GetCheck() || BST_CHECKED == m_CheckControlType3PointsSrc.GetCheck() 
		|| BST_CHECKED == m_CheckControlTypeOnOffSrc.GetCheck() )
	{
		m_GroupControlValveTypeSrc.EnableWindow( true, true );
	}
	else
	{
		m_GroupControlValveTypeSrc.EnableWindow( false, true );
	}
	
	// Update availability of components below.
	_FillInCtrlTypeSrc();
	_FillInCtrlTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnBnClickedRadioControlType()
{
	HYSELECT_SETREDRAW_FALSE;

	UpdateData( TRUE );

	// Update the 'm_eRadioControlTypeDest' enum.
	_UpdateControlTypeValueDest();

	// Update availability of components below.
	_FillInCtrlValveTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnBnClickedCheckControlValveType()
{
	// Update availability of components below.
	_FillInCtrlValveTypeSrc();
	_FillInCtrlValveTypeDest();

	_UpdateStartButton();
}

void CDlgSRPageCircuit::OnBnClickedRadioControlValveType()
{
	HYSELECT_SETREDRAW_FALSE;

	// TRUE to accept the modification.
	UpdateData( TRUE );

	// Update the 'm_eRadioControlValveTypeDest' enum.
	_UpdateControlValveTypeValueDest();
		
	// Update availability of components below.
	_UpdateStartButton();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnSelchangeListCircuitTypeSrc( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( false == m_bOnSelchangeListCircuitTypeEnabled )
	{
		return;
	}

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if( ( pNMListView->uChanged & LVIF_STATE ) && ( pNMListView->uNewState & LVIS_SELECTED ) )
	{
		HYSELECT_SETREDRAW_FALSE;
		_FillInListCircuitTypeSrc();
		_FillInListCircuitTypeDest();
		HYSELECT_SETREDRAW_TRUE;
	}
}

void CDlgSRPageCircuit::OnSelchangeListCircuitTypeDest( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( false == m_bOnSelchangeListCircuitTypeEnabled )
	{
		return;
	}

	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
	
	if( ( pNMListView->uChanged & LVIF_STATE ) && ( pNMListView->uNewState & LVIS_SELECTED ) )
	{
		HYSELECT_SETREDRAW_FALSE;
		_FillInListCircuitTypeDest();
		HYSELECT_SETREDRAW_TRUE;
	}
}

void CDlgSRPageCircuit::OnClickedCheckCircuits()
{
	HYSELECT_SETREDRAW_FALSE;

	OnTreeItemCheckStatusChanged();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnClickedCheckModules()
{
	HYSELECT_SETREDRAW_FALSE;
	
	OnTreeItemCheckStatusChanged();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::OnBnClickedCheckApplylocked()
{
	HYSELECT_SETREDRAW_FALSE;

	OnTreeItemCheckStatusChanged();

	HYSELECT_SETREDRAW_TRUE;
}

LRESULT CDlgSRPageCircuit::OnFindNextCircuit( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pDlgSearchAndReplaceCircuits || NULL == m_pDlgSearchAndReplaceCircuits->GetSafeHwnd() || NULL == pDlgSearchReplace )
	{ 
		return 0;
	}

	BeginWaitCursor();
	
	// Refresh color.
	// Previous Item.
	HTREEITEM hit = pDlgSearchReplace->GetHItem((CDS_HydroMod *)wParam);

	if( NULL != hit )
	{
		pDlgSearchReplace->GetpTree()->SetItemTextColor( hit, 0, _TAH_BLACK );
		pDlgSearchReplace->GetpTree()->EnsureVisible( hit );
	}
	
	if( m_mapSelCirIt != m_mapSelectedCicuits.end() )
	{
		CDS_HydroMod *pHM = m_mapSelCirIt->second;

		if( NULL != pHM )
		{
			m_pDlgSearchAndReplaceCircuits->UpdateData( pHM );
		}
		
		// Next item.
		hit = pDlgSearchReplace->GetHItem( pHM );

		if( NULL != hit )
		{
			pDlgSearchReplace->GetpTree()->SetItemTextColor( hit, 0, _TAH_ORANGE );
			pDlgSearchReplace->GetpTree()->EnsureVisible( hit );
		}

		++m_mapSelCirIt;

		if( m_mapSelCirIt == m_mapSelectedCicuits.end() )
		{
			m_pDlgSearchAndReplaceCircuits->DisableFindNextBtn();
		}
		
		if( true == m_bApplyToAll )
		{
			m_pDlgSearchAndReplaceCircuits->ApplyAll();
		}
	}
	else
	{
		m_pDlgSearchAndReplaceCircuits->SearchAndReplaceFinished( &m_mapModifiedHM );
	}
	
	EndWaitCursor();
	return TRUE;
}

LRESULT CDlgSRPageCircuit::OnReplaceCircuit( WPARAM wParam, LPARAM lParam )
{
	// Save modification; needed to refresh tree after modification process
	// All modified HM has been replaced by a new ones
	m_bApplyToAll = false;
	
	CDS_HydroMod *pNewHM = (CDS_HydroMod *)lParam;

	m_mapModifiedHM[wParam] = pNewHM;			
	OnFindNextCircuit( wParam, lParam );
	return TRUE;
}

LRESULT CDlgSRPageCircuit::OnReplaceAllCircuit( WPARAM wParam, LPARAM lParam )
{
	m_bApplyToAll = true;
	CDS_HydroMod *pNewHM = (CDS_HydroMod *) lParam;
	m_mapModifiedHM[wParam] = pNewHM;			
	OnFindNextCircuit( wParam, lParam );
	return TRUE;
}

LRESULT CDlgSRPageCircuit::OnClosePopup( WPARAM wParam, LPARAM lParam )
{
	m_pDlgSearchAndReplaceCircuits = NULL;
	
	if( NULL != pDlgSearchReplace )
	{
		pDlgSearchReplace->ResetItemsColor(_TAH_BLACK);
	}

	// Refresh Tree with modified pointers.
	// For each replace on a circuit, it's a new 'CDS_HydroMod' that is created and that replace the old one.
	// Tree contains references to old HM pointers, it should be updated.
	// All modifications are stored into the 'm_mapModifiedHM' map.
	if( m_mapModifiedHM.size() > 0 )
	{
		if( NULL != pDlgSearchReplace )
		{
			pDlgSearchReplace->UpdateTreeHMpointers( &m_mapModifiedHM );
		}

		// Force a refresh of HM pointer.
		OnTreeItemCheckStatusChanged();
	}
	else
	{
		m_mapSelectedCicuits.clear();
		int iPos = 0;

		// Repopulate the 'm_mapSelectedCircuits' with all the circuits that are selected in the left tree.
		// Remark: It's an operation that is normally done in start of the 'OnTreeItemCheckStatusChanged()' method.
		// But in the case where user has made no change, we don't want to reset the check-boxes and radios after.
		if( NULL != pDlgSearchReplace )
		{
			pDlgSearchReplace->GetSelectedModules( &m_mapSelectedCicuits, iPos, ( TRUE == m_CheckWhereApplyToLockedCircuit.GetCheck() ) ? CDS_HydroMod::eHMObj::eNone : CDS_HydroMod::eHMObj::eALL,
					m_CheckWhereModules.GetCheck(), m_CheckWhereCircuits.GetCheck() );
		}
	}
	
	return TRUE;
}

LRESULT CDlgSRPageCircuit::OnTreeItemCheckStatusChanged( WPARAM wParam, LPARAM lParam )
{
	HYSELECT_SETREDRAW_FALSE;

	m_mapSelectedCicuits.clear();
	int iPos = 0;

	// Remove pipe from eALL because pipe length with.
	if( NULL != pDlgSearchReplace )
	{
		pDlgSearchReplace->GetSelectedModules( &m_mapSelectedCicuits, iPos, ( TRUE == m_CheckWhereApplyToLockedCircuit.GetCheck() ) ? CDS_HydroMod::eHMObj::eNone : CDS_HydroMod::eHMObj::eALL,
				m_CheckWhereModules.GetCheck(), m_CheckWhereCircuits.GetCheck() );
	}

	// Update data in find panel.
	_FillInBalancingTypeSrc();
	_FillInBalancingTypeDest();

	HYSELECT_SETREDRAW_TRUE;
	return TRUE;
}

bool CDlgSRPageCircuit::_CheckCompatibilityHMvsCB( CDS_HydroMod *pHM, CString strCurrentGroup )
{
	if( NULL == pHM )
	{
		return false;
	}

	CDB_CircuitScheme *pSch = pHM->GetpSch(); 

	if( NULL == pSch )
	{
		return false;
	}

	CString strGroup = CteEMPTY_STRING;

	// Balancing type.
	if( ( CDB_CircuitScheme::eBALTYPE::BV == pSch->GetBalType() ) && ( BST_UNCHECKED == m_CheckBTManualBalancingSrc.GetCheck() ) )
	{
		if( BST_CHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() || BST_CHECKED == m_CheckBTNoneSrc.GetCheck() 
				|| BST_CHECKED == m_CheckBTElectronicSrc.GetCheck() || BST_CHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
		{
			return false;
		}
	}
	else if( ( CDB_CircuitScheme::eBALTYPE::DPC == pSch->GetBalType() ) && ( BST_UNCHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() ) )
	{
		if( BST_CHECKED == m_CheckBTNoneSrc.GetCheck() || BST_CHECKED == m_CheckBTManualBalancingSrc.GetCheck() 
				|| BST_CHECKED == m_CheckBTElectronicSrc.GetCheck() || BST_CHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
		{
			return false;
		}
	}
	else if( ( CDB_CircuitScheme::eBALTYPE::SP == pSch->GetBalType() ) && ( BST_UNCHECKED == m_CheckBTNoneSrc.GetCheck() ) )
	{
		if( BST_CHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() || BST_CHECKED == m_CheckBTManualBalancingSrc.GetCheck() 
				|| BST_CHECKED == m_CheckBTElectronicSrc.GetCheck() || BST_CHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
		{
			return false;
		}
	}
	else if( ( CDB_CircuitScheme::eBALTYPE::ELECTRONIC == pSch->GetBalType() ) && ( BST_UNCHECKED == m_CheckBTElectronicSrc.GetCheck() ) )
	{
		if( BST_CHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() || BST_CHECKED == m_CheckBTManualBalancingSrc.GetCheck() 
				|| BST_CHECKED == m_CheckBTNoneSrc.GetCheck() || BST_CHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
		{
			return false;
		}
	}
	else if( ( CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC == pSch->GetBalType() ) && ( BST_UNCHECKED == m_CheckBTElectronicDpCSrc.GetCheck() ) )
	{
		if( BST_CHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() || BST_CHECKED == m_CheckBTManualBalancingSrc.GetCheck() 
				|| BST_CHECKED == m_CheckBTNoneSrc.GetCheck() || BST_CHECKED == m_CheckBTElectronicSrc.GetCheck() )
		{
			return false;
		}
	}
	
	m_GroupBalancingTypeSrc.GetWindowText( strGroup );

	if( strGroup == strCurrentGroup )
	{
		return true;
	}

	// Circuit type.
	if( 0 < m_ListBoxCircuitTypeSrc.GetSelectedCount() )
	{
		POSITION pos = m_ListBoxCircuitTypeSrc.GetFirstSelectedItemPosition();
		
		if( NULL != pos )
		{
			bool bFound = false;

			while( pos && ( false == bFound ) )
			{
				int nItem = m_ListBoxCircuitTypeSrc.GetNextSelectedItem( pos );
				CString strItemText = m_ListBoxCircuitTypeSrc.GetItemText( nItem, 0 );
				if( 0 == strItemText.Compare( pSch->GetSchemeCategName().c_str() ) )
				{
					bFound = true;
				}
			}

			if( false == bFound )
			{
				return false;
			}
		}

		GetDlgItem( IDC_STATIC_CIRCTYPE )->GetWindowText( strGroup );

		if( strGroup == strCurrentGroup )
		{
			return true;
		}

		// Work with CV?
		if( true == pHM->IsCvExist( true ) )
		{
			if( ( true == pHM->GetpCV()->IsTaCV() ) && ( BST_UNCHECKED == m_CheckValveTypeIMISrc.GetCheck() ) )
			{
				if( BST_CHECKED == m_CheckValveTypeKvsSrc.GetCheck() )
				{
					return false;
				}
			}
			else if( ( false == pHM->GetpCV()->IsTaCV() ) && ( BST_UNCHECKED == m_CheckValveTypeKvsSrc.GetCheck() ) )
			{
				if( BST_CHECKED == m_CheckValveTypeIMISrc.GetCheck() )
				{
					return false;
				}
			}
			
			m_GroupValveTypeSrc.GetWindowText( strGroup );
			
			if( strGroup == strCurrentGroup )
			{
				return true;
			}
			
			// Control type.
			if( true == pHM->GetpCV()->IsTaCV() )
			{
				if( ( BST_UNCHECKED == m_CheckControlTypeProportionalSrc.GetCheck() )
						&& ( CDB_ControlProperties::CvCtrlType::eCvProportional == pHM->GetpCV()->GetCtrlType() ) )
				{
					if( BST_CHECKED == m_CheckControlType3PointsSrc.GetCheck() || BST_CHECKED == m_CheckControlTypeOnOffSrc.GetCheck() )
					{
						return false;
					}
				}

				if( ( BST_UNCHECKED == m_CheckControlType3PointsSrc.GetCheck() )
						&& ( CDB_ControlProperties::CvCtrlType::eCv3point == pHM->GetpCV()->GetCtrlType() ) )
				{
					if( BST_CHECKED == m_CheckControlTypeProportionalSrc.GetCheck() || BST_CHECKED == m_CheckControlTypeOnOffSrc.GetCheck() )
					{
						return false;
					}
				}
				
				if( ( BST_UNCHECKED == m_CheckControlTypeOnOffSrc.GetCheck() )
						&& ( CDB_ControlProperties::CvCtrlType::eCvOnOff == pHM->GetpCV()->GetCtrlType() ) )
				{
					if( BST_CHECKED == m_CheckControlType3PointsSrc.GetCheck() || BST_CHECKED == m_CheckControlTypeProportionalSrc.GetCheck() )
					{
						return false;
					}
				}
				
				m_GroupControlTypeSrc.GetWindowText( strGroup );
				
				if( strGroup == strCurrentGroup )
				{
					return true;
				}

				// Control valve type.
				if( CDB_ControlProperties::eCVFUNC::ControlOnly == pSch->GetCvFunc()
						&& BST_UNCHECKED == m_CheckControlValveTypeStandardCVSrc.GetCheck() )
				{
					if( BST_CHECKED == m_CheckControlValveTypeCombBalCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypePresetCvSrc.GetCheck() 
							|| BST_CHECKED == m_CheckControlValveTypePIBCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypeSmartSrc.GetCheck() )
					{
						return false;
					}
				}
				
				if( CDB_ControlProperties::eCVFUNC::PresetPT == pSch->GetCvFunc() && pSch->GetDpCType() != CDB_CircuitScheme::eDpCType::eDpCTypePICV
						&& BST_UNCHECKED == m_CheckControlValveTypeCombBalCVSrc.GetCheck() )
				{
					if( BST_CHECKED == m_CheckControlValveTypeStandardCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypePresetCvSrc.GetCheck()
							|| BST_CHECKED == m_CheckControlValveTypePIBCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypeSmartSrc.GetCheck() )
					{
						return false;
					}
				}
				
				if( CDB_ControlProperties::eCVFUNC::Presettable == pSch->GetCvFunc() && pSch->GetDpCType() != CDB_CircuitScheme::eDpCType::eDpCTypePICV
						&& BST_UNCHECKED == m_CheckControlValveTypePresetCvSrc.GetCheck() )
				{
					if( BST_CHECKED == m_CheckControlValveTypeStandardCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypeCombBalCVSrc.GetCheck() 
							|| BST_CHECKED == m_CheckControlValveTypePIBCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypeSmartSrc.GetCheck() )
					{
						return false;
					}
				}

				if( ( CDB_ControlProperties::eCVFUNC::Presettable == ( pSch->GetCvFunc() & CDB_ControlProperties::eCVFUNC::Presettable )
						|| CDB_ControlProperties::eCVFUNC::PresetPT == ( pSch->GetCvFunc() & CDB_ControlProperties::eCVFUNC::PresetPT ) ) 
						  && pSch->GetDpCType() == CDB_CircuitScheme::eDpCType::eDpCTypePICV && BST_UNCHECKED == m_CheckControlValveTypePIBCVSrc.GetCheck() )
				{
					if( BST_CHECKED == m_CheckControlValveTypeStandardCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypeCombBalCVSrc.GetCheck() 
							|| BST_CHECKED == m_CheckControlValveTypePresetCvSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypeSmartSrc.GetCheck() )
					{
						return false;
					}
				}
			}
		}
		else if( true == pHM->IsSmartControlValveExist( true ) && NULL != pHM->GetpSmartControlValve() )
		{
			if( BST_UNCHECKED == m_CheckValveTypeIMISrc.GetCheck() && BST_CHECKED == m_CheckValveTypeKvsSrc.GetCheck() )
			{
				return false;
			}

			m_GroupValveTypeSrc.GetWindowText( strGroup );

			if( strGroup == strCurrentGroup )
			{
				return true;
			}

			if( BST_UNCHECKED == m_CheckControlTypeProportionalSrc.GetCheck() 
					&& ( BST_CHECKED == m_CheckControlType3PointsSrc.GetCheck() || BST_CHECKED == m_CheckControlTypeOnOffSrc.GetCheck() ) )
			{
				return false;
			}

			m_GroupControlTypeSrc.GetWindowText( strGroup );

			if( strGroup == strCurrentGroup )
			{
				return true;
			}

			if( BST_UNCHECKED == m_CheckControlValveTypeSmartSrc.GetCheck() )
			{
				if( BST_CHECKED == m_CheckControlValveTypeStandardCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypePresetCvSrc.GetCheck()
						|| BST_CHECKED == m_CheckControlValveTypePIBCVSrc.GetCheck() || BST_CHECKED == m_CheckControlValveTypeCombBalCVSrc.GetCheck() )
				{
					return false;
				}
			}
		}
    }

	return true;
}

void CDlgSRPageCircuit::_UpdateGroupMBType()
{
	HYSELECT_SETREDRAW_FALSE;

	if( FALSE == m_CheckBTNoneSrc.IsWindowEnabled() && FALSE == m_CheckBTManualBalancingSrc.IsWindowEnabled() 
			&& FALSE == m_CheckBTDiffPressureControlSrc.IsWindowEnabled() && FALSE == m_CheckBTElectronicSrc.IsWindowEnabled() 
			&& FALSE == m_CheckBTElectronicDpCSrc.IsWindowEnabled() )
	{
		m_GroupBalancingTypeSrc.EnableWindow( false, true );
		m_GroupBalTypeDest.EnableWindow( false, true );
	}
	else
	{
		m_GroupBalancingTypeSrc.EnableWindow( true );
	}

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_InitListsControlCircuit()
{
	HYSELECT_SETREDRAW_FALSE;

	CRect rect;
	m_ListBoxCircuitTypeSrc.GetWindowRect( rect );
	m_ListBoxCircuitTypeSrc.InsertColumn( 0, _T( "" ), LVCFMT_LEFT, rect.Width() - 1 );
	m_ListBoxCircuitTypeDest.InsertColumn( 0, _T( "" ), LVCFMT_LEFT, rect.Width() - 1 );

	m_ListBoxCircuitTypeSrc.DeleteAllItems();
	m_ListBoxCircuitTypeDest.DeleteAllItems();
	m_ListBoxCircuitTypeSrc.SetClickListCtrlDisable( true );	
	m_ListBoxCircuitTypeDest.SetClickListCtrlDisable( true );
	
	CTable *pclCircuitSchematicCategoryTable = (CTable *)( TASApp.GetpTADB()->Get( _T("CIRCSCHCAT_TAB") ).MP );
	ASSERT( NULL != pclCircuitSchematicCategoryTable );
	
	std::map<int, CDB_CircSchemeCateg *> mapCircSchemeCategList;

	for( IDPTR IDPtr = pclCircuitSchematicCategoryTable->GetFirst(); _NULL_IDPTR != IDPtr; IDPtr = pclCircuitSchematicCategoryTable->GetNext() )
	{
		CDB_CircSchemeCateg *pclCircuitSchematicCategory = dynamic_cast<CDB_CircSchemeCateg *>( IDPtr.MP );
		ASSERT( NULL != pclCircuitSchematicCategory );
		
		int iItem = pclCircuitSchematicCategory->GetOrderNr() - 1;
		mapCircSchemeCategList.insert( std::pair<int, CDB_CircSchemeCateg*>( iItem, pclCircuitSchematicCategory ) );
	}

	for( auto &iter : mapCircSchemeCategList )
	{
		CString strName =  CString( iter.second->GetSchName().c_str() );
		
		m_ListBoxCircuitTypeSrc.InsertItem( LVIF_TEXT | LVIF_STATE | LVIF_PARAM, iter.first, strName, 0, LVIS_SELECTED, 0, (DWORD_PTR)iter.second );
		m_ListBoxCircuitTypeSrc.SetItemTextColor( iter.first, 0, _GRAY, _GRAY );

		m_ListBoxCircuitTypeDest.InsertItem( LVIF_TEXT | LVIF_STATE | LVIF_PARAM, iter.first, strName, 0, LVIS_SELECTED, 0, (DWORD_PTR)iter.second );
		m_ListBoxCircuitTypeDest.SetItemTextColor( iter.first, 0, _GRAY, _GRAY );
	}

	m_ListBoxCircuitTypeSrc.SetClickListCtrlDisable( true );
	m_ListBoxCircuitTypeDest.SetClickListCtrlDisable( true );

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInBalancingTypeSrc()
{
	HYSELECT_SETREDRAW_FALSE;

	CButton *pBnNoBal = NULL;
	CButton *pBnManBal = NULL;
	CButton *pBnBalDpC = NULL;
	CButton *pBnElecBal = NULL;
	CButton *pBnElecDpC = NULL;

	int iNewStateNone = -1;
	BOOL bIsEnabledNone = FALSE;
	int iNewStateMan = -1;
	BOOL bIsEnabledMan = FALSE;
	int iNewStateDpC = -1;
	BOOL bIsEnabledDpC = FALSE;
	int iNewStateElec = -1;
	BOOL bIsEnabledElec = FALSE;
	int iNewStateElecDpC = -1;
	BOOL bIsEnabledElecDpC = FALSE;

	pBnNoBal = &m_CheckBTNoneSrc;
	pBnManBal = &m_CheckBTManualBalancingSrc;
	pBnBalDpC = &m_CheckBTDiffPressureControlSrc;
	pBnElecBal = &m_CheckBTElectronicSrc;
	pBnElecDpC = &m_CheckBTElectronicDpCSrc;
	
	// Keep old information.
	iNewStateNone = pBnNoBal->GetCheck();
	bIsEnabledNone = pBnNoBal->IsWindowEnabled();
	iNewStateMan = pBnManBal->GetCheck();
	bIsEnabledMan = pBnManBal->IsWindowEnabled();
	iNewStateDpC = pBnBalDpC->GetCheck();
	bIsEnabledDpC = pBnBalDpC->IsWindowEnabled();
	iNewStateElec = pBnElecBal->GetCheck();
	bIsEnabledElec = pBnElecBal->IsWindowEnabled();
	iNewStateElecDpC = pBnElecDpC->GetCheck();
	bIsEnabledElecDpC = pBnElecDpC->IsWindowEnabled();
	
	// Init.
	pBnNoBal->EnableWindow( FALSE );
	pBnNoBal->SetCheck( BST_UNCHECKED );
	pBnManBal->EnableWindow( FALSE );
	pBnManBal->SetCheck( BST_UNCHECKED );
	pBnBalDpC->EnableWindow( FALSE );
	pBnBalDpC->SetCheck( BST_UNCHECKED );
	pBnElecBal->EnableWindow( FALSE );
	pBnElecBal->SetCheck( BST_UNCHECKED );
	pBnElecDpC->EnableWindow( FALSE );
	pBnElecDpC->SetCheck( BST_UNCHECKED );

	bool bManualBalancingExist = false;
	bool bDiffPressExist = false;
	bool bNoBalance = false;
	bool bElecBalancingExists = false;
	bool bElecronicDpCExists = false;

	// Verify that selected circuits contains BalType::BV/DpC/SP/ELECTRONIC/ELECTRONICDPC
	for( m_mapSelCirIt = m_mapSelectedCicuits.begin(); m_mapSelCirIt != m_mapSelectedCicuits.end(); ++m_mapSelCirIt )
	{
		CDS_HydroMod *pHM = m_mapSelCirIt->second;

		if( NULL != pHM )
		{
			CDB_CircuitScheme *pSch = pHM->GetpSch(); 

			if( NULL != pSch )
			{
				if( CDB_CircuitScheme::eBALTYPE::BV == pSch->GetBalType() )
				{
					bManualBalancingExist = true;
				}
				else if( CDB_CircuitScheme::eBALTYPE::DPC == pSch->GetBalType() )
				{
					bDiffPressExist = true;
				}
				else if( CDB_CircuitScheme::eBALTYPE::ELECTRONIC == pSch->GetBalType() )
				{
					bElecBalancingExists = true;
				}
				else if( CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC == pSch->GetBalType() )
				{
					bElecronicDpCExists = true;
				}
				else
				{
					bNoBalance = true;
				}
			}
		}
		
		// Not needed to continue if all circuits are already used.
		if( true == bManualBalancingExist && true == bDiffPressExist && true == bNoBalance && true == bElecBalancingExists
				&& true == bElecronicDpCExists )
		{
			break;
		}
	}

	if( true == bManualBalancingExist )
	{
		pBnManBal->EnableWindow( TRUE );
		pBnManBal->SetCheck( BST_UNCHECKED );

		if( TRUE == bIsEnabledMan )
		{
			pBnManBal->SetCheck( iNewStateMan );
		}
	}

	if( true == bDiffPressExist )
	{
		pBnBalDpC->EnableWindow( TRUE );
		pBnBalDpC->SetCheck( BST_UNCHECKED );

		if( TRUE == bIsEnabledDpC )
		{
			pBnBalDpC->SetCheck( iNewStateDpC );
		}
	}
		
	if( true == bNoBalance )
	{
		pBnNoBal->EnableWindow( TRUE );
		pBnNoBal->SetCheck( BST_UNCHECKED );

		if( TRUE == bIsEnabledNone)
		{
			pBnNoBal->SetCheck( iNewStateNone );
		}
	}

	if( true == bElecBalancingExists )
	{
		pBnElecBal->EnableWindow( TRUE );
		pBnElecBal->SetCheck( BST_UNCHECKED );

		if( TRUE == bIsEnabledElec )
		{
			pBnElecBal->SetCheck( iNewStateElec );
		}
	}

	if( true == bElecronicDpCExists )
	{
		pBnElecDpC->EnableWindow( TRUE );
		pBnElecDpC->SetCheck( BST_UNCHECKED );

		if( TRUE == bIsEnabledElecDpC )
		{
			pBnElecDpC->SetCheck( iNewStateElecDpC );
		}
	}

	if( true == bNoBalance && false == bDiffPressExist && false == bManualBalancingExist && false == bElecBalancingExists
			&& false == bElecronicDpCExists )
	{
		// Straight pipe.
		pBnManBal->EnableWindow( FALSE );
		pBnManBal->SetCheck( BST_UNCHECKED );
		pBnBalDpC->EnableWindow( FALSE );
		pBnBalDpC->SetCheck( BST_UNCHECKED );
		pBnElecBal->EnableWindow( FALSE );
		pBnElecBal->SetCheck( BST_UNCHECKED );
		pBnElecDpC->EnableWindow( FALSE );
		pBnElecDpC->SetCheck( BST_UNCHECKED );
	}

	// Update group.
	_UpdateGroupMBType();

	_FillInListCircuitTypeSrc();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInBalancingTypeDest()
{
	HYSELECT_SETREDRAW_FALSE;

	/*
	bool bManualBalancingExist = false;
	bool bDiffPressExist = false;
	bool bNoBalance = false;

	if( (int)m_mapSelectedCicuits.size() > 0 )
	{
		bManualBalancingExist = true;
		bNoBalance = true;
		
		// Add Dpc type if at least one DpController is selectable.
		CRankEx ListEx;

		if( TASApp.GetpTADB()->GetDpCList( &ListEx, eDpCLoc::DpCLocDownStream, L"", L"", L"", L"", L"", L"", CTADatabase::FilterSelection::ForHMCalc ) )
		{
			bDiffPressExist = true;
		}
	}
	*/

	_FillInListCircuitTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInListCircuitTypeSrc()
{
	HYSELECT_SETREDRAW_FALSE;

	CExtListCtrl *pListCtrl = &m_ListBoxCircuitTypeSrc;
	
	// Verify previous : the initial state of the list depend on balancing type group.
	bool bSkip = false;

	if( BST_UNCHECKED == m_CheckBTNoneSrc.GetCheck() && BST_UNCHECKED == m_CheckBTManualBalancingSrc.GetCheck()
			&& BST_UNCHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() && BST_UNCHECKED == m_CheckBTElectronicSrc.GetCheck()
			&& BST_UNCHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
	{
		bSkip = true;
	}
	
	int iSelectedItem = pListCtrl->GetSelectedCount();
	std::vector<int> vecSelectedIndex;

	if( iSelectedItem > 0 )
	{
		POSITION pos = pListCtrl->GetFirstSelectedItemPosition();

		if( pos != NULL )
		{
			while( pos )
			{
				int nItem = pListCtrl->GetNextSelectedItem( pos );
				vecSelectedIndex.push_back( nItem );
			}
		}
	}

	bool bNoCtrl = false;
	bool bPending = false;
	bool bPump = false;
	bool bStraightPipe = false;
	bool bCtrlMode2Winj = false;
	bool bCtrlMode2W = false;
	bool bCtrlMode3Wdiv = false;
	bool bCtrlMode3Wmix = false;
	bool bCtrlMode3Wdecoup = false;
	// HYS-2084: Add auto-adaptive circuit.
	bool bDiffCtrlAutoadaptive = false;

	bool bWithCV = false;
	bool bWithoutCV = false;
	bool bFound = false;

	std::map<int, bool> mapCircuitFound;

	// Verify with existing control valves (CTRLVALV_TAB)		.
	if ( false == bSkip )
	{
		for( m_mapSelCirIt = m_mapSelectedCicuits.begin();m_mapSelCirIt != m_mapSelectedCicuits.end(); ++m_mapSelCirIt )
		{
			CDS_HydroMod *pHM = m_mapSelCirIt->second;

			if( NULL != pHM )
			{
				CDB_CircuitScheme* pSch = pHM->GetpSch();

				if( NULL != pSch )
				{
					CString str = pSch->GetSchemeCategName().c_str();

					// Verify balancing type.
					if( FALSE == m_CheckBTManualBalancingSrc.IsWindowEnabled() && FALSE == m_CheckBTDiffPressureControlSrc.IsWindowEnabled()
							&& FALSE == m_CheckBTElectronicSrc.IsWindowEnabled() && FALSE == m_CheckBTElectronicDpCSrc.IsWindowEnabled() )
					{
						// No balance.
						if( CDB_CircuitScheme::eBALTYPE::BV == pSch->GetBalType() || CDB_CircuitScheme::eBALTYPE::DPC == pSch->GetBalType()
								|| CDB_CircuitScheme::eBALTYPE::ELECTRONIC == pSch->GetBalType() 
								|| CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC == pSch->GetBalType() )
						{
							continue;
						}
					}
					else if( BST_UNCHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() && BST_CHECKED == m_CheckBTManualBalancingSrc.GetCheck()
							&& BST_UNCHECKED == m_CheckBTNoneSrc.GetCheck() && BST_UNCHECKED == m_CheckBTElectronicSrc.GetCheck() 
							&& BST_UNCHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
					{
						if( CDB_CircuitScheme::eBALTYPE::BV != pSch->GetBalType() )
						{
							continue;
						}
					}
					else if( BST_UNCHECKED == m_CheckBTManualBalancingSrc.GetCheck() && BST_CHECKED == m_CheckBTDiffPressureControlSrc.GetCheck()
							&& BST_UNCHECKED == m_CheckBTNoneSrc.GetCheck() && BST_UNCHECKED == m_CheckBTElectronicSrc.GetCheck()
							&& BST_UNCHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
					{
						if( CDB_CircuitScheme::eBALTYPE::DPC != pSch->GetBalType() )
						{
							continue;
						}
					}
					else if( BST_UNCHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() && BST_CHECKED == m_CheckBTNoneSrc.GetCheck()
							&& BST_UNCHECKED == m_CheckBTManualBalancingSrc.GetCheck() && BST_UNCHECKED == m_CheckBTElectronicSrc.GetCheck() 
							&& BST_UNCHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
					{
						if( CDB_CircuitScheme::eBALTYPE::SP != pSch->GetBalType() )
						{
							continue;
						}
					}
					else if( BST_UNCHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() && BST_CHECKED == m_CheckBTElectronicSrc.GetCheck()
							&& BST_UNCHECKED == m_CheckBTManualBalancingSrc.GetCheck() && BST_UNCHECKED == m_CheckBTNoneSrc.GetCheck() 
							&& BST_UNCHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
					{
						if( CDB_CircuitScheme::eBALTYPE::ELECTRONIC != pSch->GetBalType() )
						{
							continue;
						}
					}
					else if( BST_UNCHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() && BST_CHECKED == m_CheckBTElectronicDpCSrc.GetCheck()
							&& BST_UNCHECKED == m_CheckBTManualBalancingSrc.GetCheck() && BST_UNCHECKED == m_CheckBTNoneSrc.GetCheck() 
							&& BST_UNCHECKED == m_CheckBTElectronicSrc.GetCheck() )
					{
						if( CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC != pSch->GetBalType() )
						{
							continue;
						}
					}

					if( true == pHM->IsCvExist( true ) || BST_CHECKED == m_CheckBTElectronicSrc.GetCheck() )
					{

						if( CDB_ControlProperties::eCVFUNC::NoControl == pSch->GetCvFunc() )
						{
							bNoCtrl = true;
							bFound = true;
						}
						else
						{
							CDB_CircSchemeCateg *pSchCateg = dynamic_cast<CDB_CircSchemeCateg *>( pSch->GetSchemeCategIDPtr().MP );

							if( NULL != pSchCateg )
							{
								if( CDB_ControlProperties::CV2W3W::CV2W == pSchCateg->Get2W3W() )
								{
									if( true == pSchCateg->IsInjection() )
									{
										bCtrlMode2Winj = true;
									}
									else
									{
										bCtrlMode2W = true;
									}
									bFound = true;
								}

								if( CDB_ControlProperties::CV2W3W::CV3W == pSchCateg->Get2W3W() )
								{
									if( CDB_CircSchemeCateg::e3wTypeDividing == pSchCateg->Get3WType() )
									{
										bCtrlMode3Wdiv = true;
										bFound = true;
									}
									else if( CDB_CircSchemeCateg::e3wTypeMixing == pSchCateg->Get3WType() )
									{
										bCtrlMode3Wmix = true;
										bFound = true;
									}
									else if( CDB_CircSchemeCateg::e3wTypeMixDecByp == pSchCateg->Get3WType() )
									{
										bCtrlMode3Wdecoup = true;
										bFound = true;
									}
								}
							}
						}

						if( true == bCtrlMode2Winj && true == bCtrlMode2W && true == bCtrlMode3Wdiv && true == bCtrlMode3Wmix && true == bCtrlMode3Wdecoup )
						{
							bWithCV = true;
						}
					}
					else
					{
						// We find the exact string in the list control and get the index. 0-pending 1-Pump 2-Straight pipe 3-Distribution.
						LVFINDINFO info;
						info.psz = str;
						info.flags = LVFI_STRING;
						int iIndex = pListCtrl->FindItem( &info );

						if( -1 != iIndex )
						{
							switch( iIndex )
							{
								case 0:
									bPending = true;
									bFound = true;
									break;

								case 1:
									bPump = true;
									bFound = true;
									break;

								case 2:
									bStraightPipe = true;
									bFound = true;
									break;

								case 3:
									bNoCtrl = true;
									bFound = true;
									break;

								// HYS-2084: Add auto-adaptive circuit.
								case 9:
									bDiffCtrlAutoadaptive = true;
									bFound = true;
									break;

								default:
									break;
							}
						}

						if( true == bPending && true == bPump && true == bStraightPipe && true == bNoCtrl && true == bDiffCtrlAutoadaptive )
						{
							bWithoutCV = true;
						}
					}

					// Not need to continue if all circuits are already used.
					if( true == bWithCV && true == bWithoutCV )
					{
						break;
					}
				}
			}
		}
	}

	// Construct the map.
	mapCircuitFound.insert( pair<int, bool>( 0, bPending ) );
	mapCircuitFound.insert( pair<int, bool>( 1, bPump ) );
	mapCircuitFound.insert( pair<int, bool>( 2, bStraightPipe ) );
	mapCircuitFound.insert( pair<int, bool>( 3, bNoCtrl ) );
	mapCircuitFound.insert( pair<int, bool>( 4, bCtrlMode2W ) );
	mapCircuitFound.insert( pair<int, bool>( 5, bCtrlMode2Winj ) );
	mapCircuitFound.insert( pair<int, bool>( 6, bCtrlMode3Wdiv ) );
	mapCircuitFound.insert( pair<int, bool>( 7, bCtrlMode3Wmix ) );
	mapCircuitFound.insert( pair<int, bool>( 8, bCtrlMode3Wdecoup ) );
	// HYS-2084: Add auto-adaptive circuit.
	mapCircuitFound.insert( pair<int, bool>( 9, bDiffCtrlAutoadaptive ) );

	// Update list appearance.
	_UpdateListCtrl( pListCtrl, vecSelectedIndex, mapCircuitFound, bFound );

	_FillInValveTypeSrc();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInListCircuitTypeDest()
{
	HYSELECT_SETREDRAW_FALSE;

	CExtListCtrl *pListCtrl = &m_ListBoxCircuitTypeDest;
	
	// Verify previous : the initial state of the list depend on balancing type group.
	bool bSkip = false;

	if( FALSE == m_GroupBalTypeDest.IsWindowEnabled() )
	{
		bSkip = true;
	}
	
	// Even if we can have only one item selected in the destination list, we put the selected index
	// in a vector to be able to use the same '_UpdateListCtrl' for source and destination.
	std::vector<int> vecSelectedIndex;
	
	if( m_ListBoxCircuitTypeDest.GetSelectedCount() > 0 )
	{
		POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
		
		if( pos != NULL )
		{
			vecSelectedIndex.push_back( m_ListBoxCircuitTypeDest.GetNextSelectedItem( pos ) );
		}
	}

	bool bPending = false;
	bool bPump = false;
	bool bStraightPipe = false;
	bool bDistributionCircuit = false;
	bool bCtrlMode2W = false;
	bool bCtrlMode2Winj = false;
	bool bCtrlMode3Wdiv = false;
	bool bCtrlMode3Wmix = false;
	bool bCtrlMode3Wdecoup = false;
	// HYS-2084: Add auto-adaptive circuit.
	bool bDiffCtrlAutoadaptive = false;

	bool bFound = false;
	std::map<int, bool> mapCircuitFound;
	
	// Verify with existing control valves (CTRLVALV_TAB).	
	if( (int)m_mapSelectedCicuits.size() > 0 && m_ListBoxCircuitTypeSrc.GetSelectedCount() > 0 && false == bSkip )
	{
		// Straight pipe and pending circuit are available only if None radio is selected.
		POSITION pos = m_ListBoxCircuitTypeSrc.GetFirstSelectedItemPosition();
		bool bAtLeastOneOtherCircuit = false;

		if( pos != NULL )
		{
			// Straight pipe.
			if( CDB_CircuitScheme::eBALTYPE::SP == m_eRadioBalancingTypeDest )
			{
				bStraightPipe = true;
				bFound = true;
			}
			else
			{
				bDistributionCircuit = true;
				// HYS-2084: Add auto-adaptive circuit.
				bDiffCtrlAutoadaptive = true;
				bCtrlMode2W = true;
				bCtrlMode2Winj = true;
				bCtrlMode3Wmix = true;
				bCtrlMode3Wdecoup = true;
				bFound = true;
			}
			
			// Availability of circuit with pump.
			for( m_mapSelCirIt = m_mapSelectedCicuits.begin(); m_mapSelCirIt != m_mapSelectedCicuits.end(); ++m_mapSelCirIt )
			{
				CDS_HydroMod *pHM = m_mapSelCirIt->second;
				
				// HYS-2084: We have to continue to check the other circuits selected.
				// We already found at least one circuit so bFound is true. Now check for circuit pump, pending and 3way dividing
				// The root module is selected.
				if( NULL != pHM && 0 == pHM->GetLevel() )
				{
					bPump = true;
				}
				// Pending
				else if( false == pHM->IsaModule() && CDB_CircuitScheme::eBALTYPE::SP == m_eRadioBalancingTypeDest )
				{
					bPending = true;
				}
				// 3-way dividing
				else if( false == pHM->IsaModule() )
				{
					bCtrlMode3Wdiv = true;
				}

				if( true == bCtrlMode3Wdiv && true == bPending && true == bPump )
				{
					break;
				}
			}

			if( CDB_CircuitScheme::eBALTYPE::ELECTRONIC == m_eRadioBalancingTypeDest )
			{
				bPump = false;
				bPending = false;
				bStraightPipe = false;
				bDistributionCircuit = false;
				bCtrlMode3Wdiv = false;
				bCtrlMode3Wmix = false;
				bCtrlMode3Wdecoup = false;
				// HYS-2084: Add auto-adaptive circuit.
				bDiffCtrlAutoadaptive = false;
			}
			else if( CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC == m_eRadioBalancingTypeDest )
			{
				bPending = false;
				bPump = false;
				bStraightPipe = false;
				bCtrlMode2W = false;
				bCtrlMode2Winj = false;
				bCtrlMode3Wdiv = false;
				bCtrlMode3Wmix = false;
				bCtrlMode3Wdecoup = false;
				// HYS-2084: Add auto-adaptive circuit.
				bDiffCtrlAutoadaptive = false;
			}
		}
	}

	// Construct the map.
	mapCircuitFound.insert( pair<int, bool>( 0, bPending ) );
	mapCircuitFound.insert( pair<int, bool>( 1, bPump ) );
	mapCircuitFound.insert( pair<int, bool>( 2, bStraightPipe ) );
	mapCircuitFound.insert( pair<int, bool>( 3, bDistributionCircuit ) );
	mapCircuitFound.insert( pair<int, bool>( 4, bCtrlMode2W ) );
	mapCircuitFound.insert( pair<int, bool>( 5, bCtrlMode2Winj ) );
	mapCircuitFound.insert( pair<int, bool>( 6, bCtrlMode3Wdiv ) );
	mapCircuitFound.insert( pair<int, bool>( 7, bCtrlMode3Wmix ) );
	mapCircuitFound.insert( pair<int, bool>( 8, bCtrlMode3Wdecoup ) );
	// HYS-2084: Add auto-adaptive circuit.
	mapCircuitFound.insert( pair<int, bool>( 9, bDiffCtrlAutoadaptive ) );

	// Update list appearance.
	_UpdateListCtrl( pListCtrl, vecSelectedIndex, mapCircuitFound, bFound );

	_FillInValveTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInValveTypeSrc()
{
	HYSELECT_SETREDRAW_FALSE;

	CButton *pCheckBoxIMI = &m_CheckValveTypeIMISrc;
	CButton *pCheckBoxKvs = &m_CheckValveTypeKvsSrc;

	int iNewStateIMI = -1;
	BOOL bIsEnabledIMI = FALSE;
	int iNewStateKvs = -1;
	BOOL bIsEnabledKvs = FALSE;

	// Leep old values.
	iNewStateIMI = pCheckBoxIMI->GetCheck();
	bIsEnabledIMI = pCheckBoxIMI->IsWindowEnabled();
	iNewStateKvs = pCheckBoxKvs->GetCheck();
	bIsEnabledKvs = pCheckBoxKvs->IsWindowEnabled();

	// Init.
	pCheckBoxIMI->EnableWindow( FALSE );
	pCheckBoxIMI->SetCheck( BST_UNCHECKED );
	pCheckBoxKvs->EnableWindow( FALSE );
	pCheckBoxKvs->SetCheck( BST_UNCHECKED );

	bool bKvSCVExist = false;
	bool bTACVExist = false;

	m_bAtLeastOneCircuitTypeWithCVSrc = false;

	if( 0 != m_ListBoxCircuitTypeSrc.GetSelectedCount() )
	{
		POSITION pos = m_ListBoxCircuitTypeSrc.GetFirstSelectedItemPosition();
		
		while( NULL != pos )
		{
			int nItem = m_ListBoxCircuitTypeSrc.GetNextSelectedItem( pos );
			
			CDB_CircSchemeCateg *pclCircSchemeCategory = (CDB_CircSchemeCateg *)m_ListBoxCircuitTypeDest.GetItemData( nItem );
			ASSERT( NULL != pclCircSchemeCategory );

			if( pclCircSchemeCategory->Get2W3W() > CDB_ControlProperties::CV2W3W::CVUnknown 
					&& pclCircSchemeCategory->Get2W3W() < CDB_ControlProperties::CV2W3W::LastCV2W3W )
			{
				m_bAtLeastOneCircuitTypeWithCVSrc = true;
				break;
			}
		}
	}

	if ( true == m_bAtLeastOneCircuitTypeWithCVSrc )
	{
		bool bKvSCVExist = false;
		bool bTACVExist = false;

		// verify that selected circuits contains BalType::BV/DpC
		for (m_mapSelCirIt = m_mapSelectedCicuits.begin();m_mapSelCirIt != m_mapSelectedCicuits.end(); ++m_mapSelCirIt)
		{
			CDS_HydroMod *pHM = m_mapSelCirIt->second;

			if( NULL != pHM )
			{
				// If the Balancing type check box is checked, verify that the circuit meet requirement of Balancing type combo
				CString strCurrentGroup = CteEMPTY_STRING;
				m_GroupValveTypeSrc.GetWindowText( strCurrentGroup );

				if (false == _CheckCompatibilityHMvsCB( pHM, strCurrentGroup ))
				{
					continue;
				}

				if( true == pHM->IsCvExist( true ) || true == pHM->IsSmartControlValveExist( true ) )
				{
					if ( ( NULL == pHM->GetpSmartControlValve() && true == pHM->GetpCV()->IsTaCV() ) || NULL != pHM->GetpSmartControlValve() )
					{
						bTACVExist = true;
					}
					else
					{
						bKvSCVExist = true;
					}
				}
			}

			// Not needed to continue if all circuits are already used.
			if ( true == bTACVExist && true == bKvSCVExist )
			{
				break;
			}
		}

		if( true == bKvSCVExist )
		{
			pCheckBoxKvs->EnableWindow( TRUE );
			pCheckBoxKvs->SetCheck( BST_UNCHECKED );

			if( TRUE == bIsEnabledKvs )
			{
				pCheckBoxKvs->SetCheck( iNewStateKvs );
			}
		}
		else
		{
			pCheckBoxKvs->EnableWindow( FALSE );
			pCheckBoxKvs->SetCheck( BST_UNCHECKED );
		}

		if( true == bTACVExist )
		{
			pCheckBoxIMI->EnableWindow( TRUE );
			pCheckBoxIMI->SetCheck( BST_UNCHECKED );

			if( TRUE == bIsEnabledIMI )
			{
				pCheckBoxIMI->SetCheck( iNewStateIMI );
			}
		}
		else
		{
			pCheckBoxIMI->EnableWindow( FALSE );
			pCheckBoxIMI->SetCheck( BST_UNCHECKED );
		}
	}

	// Update group.
	_UpdateGroupValveType();

	_FillInCtrlTypeSrc();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInValveTypeDest()
{
	HYSELECT_SETREDRAW_FALSE;

	bool bControlAllowed = false;

	if( 0 != m_ListBoxCircuitTypeDest.GetSelectedCount() )
	{
		POSITION pos = m_ListBoxCircuitTypeDest.GetFirstSelectedItemPosition();

		if( NULL != pos )
		{
			int nItem = m_ListBoxCircuitTypeDest.GetNextSelectedItem( pos );
			CDB_CircSchemeCateg *pclCircSchemeCategory = (CDB_CircSchemeCateg *)m_ListBoxCircuitTypeDest.GetItemData( nItem );
			ASSERT( NULL != pclCircSchemeCategory );

			if( pclCircSchemeCategory->Get2W3W() > CDB_ControlProperties::CV2W3W::CVUnknown 
					&& pclCircSchemeCategory->Get2W3W() < CDB_ControlProperties::CV2W3W::LastCV2W3W )
			{
				bControlAllowed = true;
			}
		}
	}

	bool bEnable = false;

	if( true == bControlAllowed )
	{
		if( true == m_bAtLeastOneCircuitTypeWithCVSrc && TRUE == m_GroupValveTypeSrc.IsWindowEnabled() )
		{
			// When the circuit type in destination contains a control valve and it is also the case in the source,
			// we enable the group only if user has done a choice in the 'Valve type' check boxes in the left.
			if( true == _IsAtLeastOneValveTypeChecked() )
			{
				bEnable= true;
			}
		}
		else
		{
			// When the circuit type in destination contains a control valve but there is no circuit type with
			// control in the source, we enable by default the radios.
			bEnable= true;
		}
	}

	m_GroupValveTypeDest.EnableWindow( bEnable, true );

	_FillInCtrlTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInCtrlTypeSrc()
{
	HYSELECT_SETREDRAW_FALSE;

	bool bWithCv = true;
	CButton *pCheckBoxProp = &m_CheckControlTypeProportionalSrc;
	CButton *pCheckBox3Pt = &m_CheckControlType3PointsSrc;
	CButton *pCheckBoxOnoff = &m_CheckControlTypeOnOffSrc;

	// Look checkboxes above
	if( BST_UNCHECKED == m_CheckValveTypeIMISrc.GetCheck() )
	{
		pCheckBoxProp->EnableWindow( FALSE );
		pCheckBoxProp->SetCheck( BST_UNCHECKED );
		pCheckBox3Pt->EnableWindow( FALSE );
		pCheckBox3Pt->SetCheck( BST_UNCHECKED );
		pCheckBoxOnoff->EnableWindow( FALSE );
		pCheckBoxOnoff->SetCheck( BST_UNCHECKED );
		bWithCv = false;
	}

	if( true == bWithCv )
	{
		int iNewStateProp = -1;
		BOOL bIsEnabledProp = FALSE;
		int iNewStateOnoff = -1;
		BOOL bIsEnabledOnoff = FALSE;
		int iNewState3Pt = -1;
		BOOL bIsEnabled3Pt = FALSE;
		iNewStateProp = pCheckBoxProp->GetCheck();
		bIsEnabledProp = pCheckBoxProp->IsWindowEnabled();
		iNewState3Pt = pCheckBox3Pt->GetCheck();
		bIsEnabled3Pt = pCheckBox3Pt->IsWindowEnabled();
		iNewStateOnoff = pCheckBoxOnoff->GetCheck();
		bIsEnabledOnoff = pCheckBoxOnoff->IsWindowEnabled();
		pCheckBoxProp->EnableWindow( FALSE );
		pCheckBoxProp->SetCheck( BST_UNCHECKED );
		pCheckBox3Pt->EnableWindow( FALSE );
		pCheckBox3Pt->SetCheck( BST_UNCHECKED );
		pCheckBoxOnoff->EnableWindow( FALSE );
		pCheckBoxOnoff->SetCheck( BST_UNCHECKED );

		bool bCvOnoffExist = false;
		bool bCVPropExist = false;
		bool bCV3PtExist = false;

		for( m_mapSelCirIt = m_mapSelectedCicuits.begin(); m_mapSelCirIt != m_mapSelectedCicuits.end(); ++m_mapSelCirIt )
		{
			CDS_HydroMod *pHM = m_mapSelCirIt->second;

			if( NULL != pHM )
			{
				// If the Balancing type check box is checked, verify that the circuit meet requirement of Balancing type combo and work for ctrl valve.
				CString strCurrentGroup = CteEMPTY_STRING;
				m_GroupControlTypeSrc.GetWindowText( strCurrentGroup );

				if( false == _CheckCompatibilityHMvsCB( pHM, strCurrentGroup ) )
				{
					continue;
				}

				if( true == pHM->IsCvExist( true ) && true == pHM->GetpCV()->IsTaCV() )
				{
					if( CDB_ControlProperties::CvCtrlType::eCvOnOff == pHM->GetpCV()->GetCtrlType() )
					{
						bCvOnoffExist = true;
					}
					else if( CDB_ControlProperties::CvCtrlType::eCvProportional == pHM->GetpCV()->GetCtrlType() )
					{
						bCVPropExist = true;
					}
					else if( CDB_ControlProperties::CvCtrlType::eCv3point == pHM->GetpCV()->GetCtrlType() )
					{
						bCV3PtExist = true;
					}
				}
				else if( true == pHM->IsSmartControlValveExist() && NULL != pHM->GetpSmartControlValve() )
				{
					// HYS-1679: Proportional control for smart control valve.
					bCVPropExist = true;
				}

				// Not need to continue if all circuits are already used.
				if( true == bCvOnoffExist && true == bCVPropExist && true == bCV3PtExist )
				{
					break;
				}
			}
		}

		if( true == bCvOnoffExist )
		{
			pCheckBoxOnoff->EnableWindow( TRUE );
			pCheckBoxOnoff->SetCheck( BST_UNCHECKED );
			
			if( TRUE == bIsEnabledOnoff )
			{
				pCheckBoxOnoff->SetCheck( iNewStateOnoff );
			}
		}

		if( true == bCVPropExist )
		{
			pCheckBoxProp->EnableWindow( TRUE );
			pCheckBoxProp->SetCheck( BST_UNCHECKED );
			
			if( TRUE == bIsEnabledProp )
			{
				pCheckBoxProp->SetCheck( iNewStateProp );
			}
		}

		if( true == bCV3PtExist )
		{
			pCheckBox3Pt->EnableWindow( TRUE );
			pCheckBox3Pt->SetCheck( BST_UNCHECKED );
			
			if( TRUE == bIsEnabled3Pt )
			{
				pCheckBox3Pt->SetCheck( iNewState3Pt );
			}
		}
	}

	_UpdateGroupCtrlType();
	_FillInCtrlValveTypeSrc();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInCtrlTypeDest()
{
	HYSELECT_SETREDRAW_FALSE;

	bool bWithCv = true;

	// Look checkboxes above.
	if( FALSE == m_GroupValveTypeDest.IsWindowEnabled() || _eWorkForCV::ewcvKvs == m_eRadioValveTypeDest )
	{
		bWithCv = false;
		m_GroupControlTypeDest.EnableWindow( false, true );
	}

	if( true == bWithCv )
	{
		bool bCvOnoffExist = false;
		bool bCVPropExist = false;
		bool bCV3PtExist = false;

		// Verify with existing control valves (CTRLVALV_TAB)		
		CRankEx rkList;

		int iCount = TASApp.GetpTADB()->GetTaCVList( &rkList, CTADatabase::CvTargetTab::eForHMCv, true, CDB_ControlProperties::LastCV2W3W,
				L"", L"", L"", L"", L"", L"", CDB_ControlProperties::eCVFUNC::LastCVFUNC, CDB_ControlProperties::CvCtrlType::eCvOnOff, 
				CTADatabase::FilterSelection::ForHMCalc );

		// HYS-1679: No smart control valve with on-off control.
		if( iCount > 0 && CDB_CircuitScheme::eBALTYPE::ELECTRONIC != m_eRadioBalancingTypeDest )
		{
			bCvOnoffExist = true;
		}
		
		rkList.PurgeAll();
		iCount = TASApp.GetpTADB()->GetTaCVList( &rkList, CTADatabase::CvTargetTab::eForHMCv, true, CDB_ControlProperties::LastCV2W3W,
				L"", L"", L"", L"", L"", L"", CDB_ControlProperties::eCVFUNC::LastCVFUNC, CDB_ControlProperties::CvCtrlType::eCvProportional, 
				CTADatabase::FilterSelection::ForHMCalc );

		if( iCount > 0 )
		{
			bCVPropExist = true;
		}
		
		rkList.PurgeAll();
		iCount = TASApp.GetpTADB()->GetTaCVList( &rkList, CTADatabase::CvTargetTab::eForHMCv, true, CDB_ControlProperties::LastCV2W3W,
				L"", L"", L"", L"", L"", L"", CDB_ControlProperties::eCVFUNC::LastCVFUNC, CDB_ControlProperties::CvCtrlType::eCv3point, 
				CTADatabase::FilterSelection::ForHMCalc );

		// No smart control valve with 3-points control.
		if( iCount > 0 && CDB_CircuitScheme::eBALTYPE::ELECTRONIC != m_eRadioBalancingTypeDest )
		{
			bCV3PtExist = true;
		}
		
		if( true == bCVPropExist || true == bCV3PtExist || true == bCvOnoffExist )
		{
			bool bEnable = false;

			if( true == m_bAtLeastOneCircuitTypeWithCVSrc && TRUE == m_GroupControlTypeSrc.IsWindowEnabled() )
			{
				// When the circuit type in destination contains a control valve and it is also the case in the source,
				// we enable the group only if user has done a choice in the 'Control type' check boxes in the left.
				if( true == _IsAtLeastOneControlTypeChecked() )
				{
					bEnable= true;
				}
			}
			else
			{
				// When the circuit type in destination contains a control valve but there is no circuit type with
				// control in the source, we enable by default the radios.
				bEnable= true;
			}

			m_GroupControlTypeDest.EnableWindow( bEnable, true );

			if( true == bEnable )
			{
				( (CButton *)GetDlgItem( IDC_RADIOPROP ) )->EnableWindow( ( false == bCVPropExist ) ? FALSE : TRUE );
				( (CButton *)GetDlgItem( IDC_RADIO3PT ) )->EnableWindow( ( false == bCV3PtExist ) ? FALSE : TRUE );
				( (CButton *)GetDlgItem( IDC_RADIOONOFF ) )->EnableWindow( ( false == bCvOnoffExist ) ? FALSE : TRUE );

				bool bRadioBecomesInactive = false;
				int iFirstActive = -1;

				// Save the first active radio. And check if the previous choice ('m_iRadioControlTypeDest') becomes
				// disabled.
				if( true == bCVPropExist )
				{
					iFirstActive = 0;
				}
				else if( 0 == m_iRadioControlTypeDest )
				{
					bRadioBecomesInactive = true;
				}
				
				if( true == bCV3PtExist )
				{
					if( -1 == iFirstActive )
					{
						iFirstActive = 1;
					}
				}
				else if( 1 == m_iRadioControlTypeDest )
				{
					bRadioBecomesInactive = true;
				}
				
				if( true == bCvOnoffExist )
				{
					if( -1 == iFirstActive )
					{
						iFirstActive = 2;
					}
				}
				else if( 2 == m_iRadioControlTypeDest )
				{
					bRadioBecomesInactive = true;
				}

				// In case of the first active radio is different of the previous choice...
				if( iFirstActive != m_iRadioControlTypeDest )
				{
					// If the previous choice is no more available, we set select the first radio available.
					// Otherwise we keep the previous value.
					m_iRadioControlTypeDest = ( true == bRadioBecomesInactive ) ? iFirstActive : m_iRadioControlTypeDest;
				}
			}
			else
			{
				// When no active, we set the first radio.
				m_iRadioControlTypeDest = 0;
			}

			// Update the 'm_eRadioControlTypeDest' enum.
			_UpdateControlTypeValueDest();

			UpdateData( FALSE );
		}
	}

	_FillInCtrlValveTypeDest();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInCtrlValveTypeSrc()
{
	HYSELECT_SETREDRAW_FALSE;

	bool bWithCv = true;
	CButton *pCheckBoxCombBCV = &m_CheckControlValveTypeCombBalCVSrc;
	CButton *pCheckBoxCV = &m_CheckControlValveTypeStandardCVSrc;
	CButton *pCheckBoxPresetCv = &m_CheckControlValveTypePresetCvSrc;
	CButton *pCheckBoxPibCv = &m_CheckControlValveTypePIBCVSrc;
	CButton *pCheckBoxSmart = &m_CheckControlValveTypeSmartSrc;
	
	// Verify checkbox above.
	if( BST_UNCHECKED == m_CheckControlTypeProportionalSrc.GetCheck() && BST_UNCHECKED == m_CheckControlType3PointsSrc.GetCheck() 
			&& BST_UNCHECKED == m_CheckControlTypeOnOffSrc.GetCheck() )
	{
		pCheckBoxCombBCV->EnableWindow( FALSE );
		pCheckBoxCombBCV->SetCheck( BST_UNCHECKED );
		pCheckBoxCV->EnableWindow( FALSE );
		pCheckBoxCV->SetCheck( BST_UNCHECKED );
		pCheckBoxPresetCv->EnableWindow( FALSE );
		pCheckBoxPresetCv->SetCheck( BST_UNCHECKED );
		pCheckBoxPibCv->EnableWindow( FALSE );
		pCheckBoxPibCv->SetCheck( BST_UNCHECKED );
		pCheckBoxSmart->EnableWindow( FALSE );
		pCheckBoxSmart->SetCheck( BST_UNCHECKED );
		bWithCv = false;
	}

	if( true == bWithCv )
	{
		int iNewStateCombBCV = -1;
		BOOL bIsEnabledCombBCV = FALSE;
		int iNewStateCV = -1;
		BOOL bIsEnabledCV = FALSE;
		int iNewStatePresetCv = -1;
		BOOL bIsEnabledPresetCv = FALSE;
		int iNewStatePibCv = -1;
		BOOL bIsEnabledPibCv = FALSE;
		int iNewStateSmart = -1;
		BOOL bIsEnabledSmart = FALSE;
		iNewStateCombBCV = pCheckBoxCombBCV->GetCheck();
		bIsEnabledCombBCV = pCheckBoxCombBCV->IsWindowEnabled();
		iNewStateCV = pCheckBoxCV->GetCheck();
		bIsEnabledCV = pCheckBoxCV->IsWindowEnabled();
		iNewStatePresetCv = pCheckBoxPresetCv->GetCheck();
		bIsEnabledPresetCv = pCheckBoxPresetCv->IsWindowEnabled();
		iNewStatePibCv = pCheckBoxPibCv->GetCheck();
		bIsEnabledPibCv = pCheckBoxPibCv->IsWindowEnabled();
		iNewStateSmart = pCheckBoxSmart->GetCheck();
		bIsEnabledSmart = pCheckBoxSmart->IsWindowEnabled();
		pCheckBoxCombBCV->EnableWindow( FALSE );
		pCheckBoxCombBCV->SetCheck( BST_UNCHECKED );
		pCheckBoxCV->EnableWindow( FALSE );
		pCheckBoxCV->SetCheck( BST_UNCHECKED );
		pCheckBoxPresetCv->EnableWindow( FALSE );
		pCheckBoxPresetCv->SetCheck( BST_UNCHECKED );
		pCheckBoxPibCv->EnableWindow( FALSE );
		pCheckBoxPibCv->SetCheck( BST_UNCHECKED );
		pCheckBoxSmart->EnableWindow( FALSE );
		pCheckBoxSmart->SetCheck( BST_UNCHECKED );

		bool bCVFuncControlOnly = false;
		bool bCVFuncPresettable = false;
		bool bCVFuncPresetPT = false;
		bool bCVFuncPibcv = false;
		bool bCVFuncSmart = false;

		// Verify with existing control valves (CTRLVALV_TAB).

		for( m_mapSelCirIt = m_mapSelectedCicuits.begin(); m_mapSelCirIt != m_mapSelectedCicuits.end(); ++m_mapSelCirIt )
		{
			CDS_HydroMod *pHM = m_mapSelCirIt->second;

			if( NULL != pHM )
			{
				// If the Balancing type check box is checked, verify that the circuit meet requirement of Balancing type combo and work for ctrl valve.
				if( false == _CheckCompatibilityHMvsCB( pHM ) )
				{
					continue;
				}

				CDB_CircuitScheme *pSch = pHM->GetpSch();

				if( NULL != pSch )
				{
					if( true == pHM->IsCvExist( true ) && true == pHM->GetpCV()->IsTaCV() )
					{
						if( CDB_ControlProperties::eCVFUNC::ControlOnly == pSch->GetCvFunc() )
						{
							bCVFuncControlOnly = true;
						}

						if( CDB_ControlProperties::eCVFUNC::Presettable == pSch->GetCvFunc() && CDB_CircuitScheme::eDpCTypePICV != pSch->GetDpCType() )
						{
							bCVFuncPresettable = true;
						}

						if( CDB_ControlProperties::eCVFUNC::PresetPT == pSch->GetCvFunc() && CDB_CircuitScheme::eDpCTypePICV != pSch->GetDpCType() )
						{
							bCVFuncPresetPT = true;
						}

						if( CDB_CircuitScheme::eDpCTypePICV == pSch->GetDpCType() )
						{
							bCVFuncPibcv = true;
						}

					}
					// HYS-1679: Add smart control valve
					else if( true == pHM->IsSmartControlValveExist() && NULL != pHM->GetpSmartControlValve() )
					{
						if( CDB_ControlProperties::eCVFUNC::Electronic == pSch->GetCvFunc() )
						{
							bCVFuncSmart = true;
						}
					}
				}

				// Not need to continue if all circuits are already used.
				if( true == bCVFuncControlOnly && true == bCVFuncPresettable && true == bCVFuncPresetPT && true == bCVFuncPibcv && true == bCVFuncSmart )
				{
					break;
				}
			}
		}
		
		if( true == bCVFuncControlOnly )
		{
			pCheckBoxCV->EnableWindow( TRUE );
			pCheckBoxCV->SetCheck( BST_UNCHECKED );
			
			if( TRUE == bIsEnabledCV )
			{
				pCheckBoxCV->SetCheck( iNewStateCV );
			}
		}

		if( true == bCVFuncPresettable )
		{
			pCheckBoxPresetCv->EnableWindow( TRUE );
			pCheckBoxPresetCv->SetCheck( BST_UNCHECKED );
			
			if( TRUE == bIsEnabledPresetCv )
			{
				pCheckBoxPresetCv->SetCheck( iNewStatePresetCv );
			}
		}

		if( true == bCVFuncPresetPT )
		{
			pCheckBoxCombBCV->EnableWindow( TRUE );
			pCheckBoxCombBCV->SetCheck( BST_UNCHECKED );
			
			if( TRUE == bIsEnabledCombBCV )
			{
				pCheckBoxCombBCV->SetCheck( iNewStateCombBCV );
			}
		}

		if( true == bCVFuncPibcv )
		{
			pCheckBoxPibCv->EnableWindow( TRUE );
			pCheckBoxPibCv->SetCheck( BST_UNCHECKED );
			
			if( TRUE == bIsEnabledPibCv )
			{
				pCheckBoxPibCv->SetCheck( iNewStatePibCv );
			}
		}

		if( true == bCVFuncSmart )
		{
			pCheckBoxSmart->EnableWindow( TRUE );
			pCheckBoxSmart->SetCheck( BST_UNCHECKED );
			
			if( TRUE == bIsEnabledSmart )
			{
				pCheckBoxSmart->SetCheck( iNewStateSmart );
			}
		}
	}
	
	// Update group.
	_UpdateGroupCVType();
	_UpdateStartButton();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_FillInCtrlValveTypeDest()
{
	HYSELECT_SETREDRAW_FALSE;

	bool bWithCv = true;
	
	// Verify checkbox above.
	if( FALSE == m_GroupControlTypeDest.IsWindowEnabled() || _eWorkForCV::ewcvKvs == m_eRadioValveTypeDest )
	{
		// HYS-1637: If Kv valves update m_iRadioControlValveTypeDest to standard CV
		if( _eWorkForCV::ewcvKvs == m_eRadioValveTypeDest )
		{
			m_iRadioControlValveTypeDest = 0;
			_UpdateControlValveTypeValueDest();
		}
		m_GroupControlValveTypeDest.EnableWindow( false, true );
		bWithCv = false;
	}

	if( true == bWithCv )
	{
		bool bCVFuncControlOnly = false;
		bool bCVFuncPresettable = false;
		bool bCVFuncPresetPT = false;
		bool bCVFuncPibcv = false;
		bool bCVFuncSmart = false;

		CRankEx rkList;
		int iCount = -1;
		
		if( CDB_CircuitScheme::eBALTYPE::ELECTRONIC != m_eRadioBalancingTypeDest )
		{
			iCount = TASApp.GetpTADB()->GetTaCVList( &rkList, CTADatabase::CvTargetTab::eForHMCv, true, CDB_ControlProperties::LastCV2W3W,
					L"", L"", L"", L"", L"", L"", CDB_ControlProperties::eCVFUNC::ControlOnly, 
					m_eRadioControlTypeDest, CTADatabase::FilterSelection::ForHMCalc );

			if( iCount > 0 )
			{
				bCVFuncControlOnly = true;
			}

			rkList.PurgeAll();
			iCount = TASApp.GetpTADB()->GetTaCVList( &rkList, CTADatabase::CvTargetTab::eForHMCv, true, CDB_ControlProperties::LastCV2W3W,
					L"", L"", L"", L"", L"", L"", CDB_ControlProperties::eCVFUNC::Presettable, m_eRadioControlTypeDest, CTADatabase::FilterSelection::ForHMCalc );

			if( iCount > 0 )
			{
				bCVFuncPresettable = true;
				if( CDB_CircuitScheme::eBALTYPE::DPC == m_eRadioBalancingTypeDest )
				{
					// PIBCV without measuring point
					bCVFuncPibcv = true;
				}
			}

			rkList.PurgeAll();
			iCount = TASApp.GetpTADB()->GetTaCVList( &rkList, CTADatabase::CvTargetTab::eForHMCv, true, CDB_ControlProperties::LastCV2W3W,
				L"", L"", L"", L"", L"", L"", CDB_ControlProperties::eCVFUNC::PresetPT, m_eRadioControlTypeDest, CTADatabase::FilterSelection::ForHMCalc );

			if( iCount > 0 )
			{
				bCVFuncPresetPT = true;
				if( CDB_CircuitScheme::eBALTYPE::DPC == m_eRadioBalancingTypeDest )
				{
					// PIBCV with measuring point
					bCVFuncPibcv = true;
				}
			}
		}
		else
		{
			rkList.PurgeAll();
			iCount = TASApp.GetpTADB()->GetSmartControlValveList( &rkList, L"", L"", L"", L"", L"", L"", CTADatabase::FilterSelection::ForHMCalc );

			if( iCount > 0 )
			{
				bCVFuncSmart = true;
			}
		}

		if( true == bCVFuncControlOnly || true == bCVFuncPresettable || true == bCVFuncPresetPT || true == bCVFuncPibcv || true == bCVFuncSmart )
		{
			bool bEnable = false;

			if( true == m_bAtLeastOneCircuitTypeWithCVSrc && TRUE == m_GroupControlValveTypeSrc.IsWindowEnabled() )
			{
				// When the circuit type in destination contains a control valve and it is also the case in the source,
				// we enable the group only if user has done a choice in the 'Control valve type' check boxes in the left.
				if( true == _IsAtLeastOneControlValveTypeChecked() )
				{
					bEnable= true;
				}
			}
			else
			{
				// When the circuit type in destination contains a control valve but there is no circuit type with
				// control in the source, we enable by default the radios.
				bEnable= true;
			}

			m_GroupControlValveTypeDest.EnableWindow( bEnable, true );

			if( true == bEnable )
			{
				( (CButton *)( GetDlgItem( IDC_RADIOCV ) ) )->EnableWindow( ( false == bCVFuncControlOnly ) ? FALSE : TRUE );
				( (CButton *)( GetDlgItem( IDC_RADIOSETCV ) ) )->EnableWindow( ( false == bCVFuncPresettable ) ? FALSE : TRUE );
				( (CButton *)( GetDlgItem( IDC_RADIOCBCV ) ) )->EnableWindow( ( false == bCVFuncPresetPT ) ? FALSE : TRUE );
				( (CButton *)( GetDlgItem( IDC_RADIOPIBCV ) ) )->EnableWindow( ( false == bCVFuncPibcv ) ? FALSE : TRUE );
				( (CButton *)( GetDlgItem( IDC_RADIOSMART ) ) )->EnableWindow( ( false == bCVFuncSmart ) ? FALSE : TRUE );

				bool bRadioBecomesInactive = false;
				int iFirstActive = -1;
				
				// Save the first active radio. And check if the previous choice ('m_iRadioControlTypeDest') becomes
				// disabled.
				if( true == bCVFuncControlOnly )
				{
					iFirstActive = 0;
				}
				else if( true == bCVFuncPresettable )
				{
					iFirstActive = 1;
				}
				else if( true == bCVFuncPresetPT )
				{
					iFirstActive = 2;
				}
				else if( true == bCVFuncPibcv )
				{
					iFirstActive = 3;
				}
				else if( true == bCVFuncSmart )
				{
					iFirstActive = 4;
				}

				if( false == bCVFuncControlOnly && 0 == m_iRadioControlValveTypeDest )
				{
					bRadioBecomesInactive = true;
				}		
				else if( false == bCVFuncPresettable && 1 == m_iRadioControlValveTypeDest )
				{
					bRadioBecomesInactive = true;
				}
				else if( false == bCVFuncPresetPT && 2 == m_iRadioControlValveTypeDest )
				{
					bRadioBecomesInactive = true;
				}

				else if( false == bCVFuncPibcv && 3 == m_iRadioControlValveTypeDest )
				{
					bRadioBecomesInactive = true;
				}
		
				else if( false == bCVFuncSmart && 4 == m_iRadioControlValveTypeDest )
				{
					bRadioBecomesInactive = true;
				}
				
				// In case of the first active radio is different of the previous choice...
				if( iFirstActive != m_iRadioControlValveTypeDest )
				{
					// If the previous choice is no more available, we set select the first radio available.
					// Otherwise we keep the previous value.
					m_iRadioControlValveTypeDest = ( true == bRadioBecomesInactive ) ? iFirstActive : m_iRadioControlValveTypeDest;
				}
			}
			else
			{
				// When no active, we set the first radio.
				m_iRadioControlValveTypeDest = 0;
			}

			// Update the 'm_eRadioControlValveTypeDest' enum.
			_UpdateControlValveTypeValueDest();

			UpdateData( FALSE );
		}
	}

	_UpdateStartButton();

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_UpdateGroupValveType()
{
	HYSELECT_SETREDRAW_FALSE;

	if( FALSE == m_CheckValveTypeIMISrc.IsWindowEnabled() && FALSE == m_CheckValveTypeKvsSrc.IsWindowEnabled() )
	{
		m_GroupValveTypeSrc.EnableWindow( false );
	}
	else
	{
		m_GroupValveTypeSrc.EnableWindow( true );
	}

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_UpdateGroupCtrlType()
{
	HYSELECT_SETREDRAW_FALSE;

	if( FALSE == m_CheckControlType3PointsSrc.IsWindowEnabled() && FALSE == m_CheckControlTypeOnOffSrc.IsWindowEnabled() && FALSE == m_CheckControlTypeProportionalSrc.IsWindowEnabled() )
	{
		m_GroupControlTypeSrc.EnableWindow( false, false );
	}
	else
	{
		m_GroupControlTypeSrc.EnableWindow( true, false );
	}

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_UpdateGroupCVType()
{
	HYSELECT_SETREDRAW_FALSE;

	if( FALSE == m_CheckControlValveTypeStandardCVSrc.IsWindowEnabled() && FALSE == m_CheckControlValveTypeCombBalCVSrc.IsWindowEnabled() 
			&& FALSE == m_CheckControlValveTypePresetCvSrc.IsWindowEnabled() && FALSE == m_CheckControlValveTypePIBCVSrc.IsWindowEnabled() 
			&& FALSE == m_CheckControlValveTypeSmartSrc.IsWindowEnabled() )
	{
		m_GroupControlValveTypeSrc.EnableWindow( false, false );
	}
	else
	{
		m_GroupControlValveTypeSrc.EnableWindow( true, false );
	}

	HYSELECT_SETREDRAW_TRUE;
}

void CDlgSRPageCircuit::_UpdateStartButton()
{
	// HYS-1686: If user select a circuit we can enable the start button.
	pDlgSearchReplace->EnableButtonStart( ( m_ListBoxCircuitTypeDest.GetSelectedCount() > 0 ) ? TRUE : FALSE );
}

void CDlgSRPageCircuit::_UpdateListCtrl( CExtListCtrl *pListCtrl, std::vector<int> &vecSelectedIndex, std::map<int, bool> mapCircuitFound, bool bNoCircuitFound )
{
	if( bNoCircuitFound == false )
	{
		if( 0 != pListCtrl->GetItemCount() )
		{
			int pos = 0;

			while( pos < pListCtrl->GetItemCount() )
			{
				pListCtrl->SetItemTextColor( pos, 0, _GRAY, _GRAY );
				pListCtrl->SetItemState( pos, 0, LVIS_SELECTED );
				pListCtrl->SetItemEnableState( pos, false );
				pListCtrl->SetItemTextBkColor( pos, 0, _WHITE );
				pListCtrl->Update( pos );
				pos++;
			}
		}
		
		pListCtrl->SetClickListCtrlDisable( true );
	}
	else
	{
		pListCtrl->SetClickListCtrlDisable( false );
		std::map<int, CString>::iterator it;

		for( int i = 0; i < m_ListBoxCircuitTypeDest.GetItemCount(); i++ )
		{
			CDB_CircSchemeCateg *pclCircSchemeCateg = (CDB_CircSchemeCateg *)m_ListBoxCircuitTypeDest.GetItemData( i );

			if( NULL == pclCircSchemeCateg )
			{
				ASSERT_CONTINUE;
			}

			if( true ==  mapCircuitFound.at( i ) )
			{
				pListCtrl->SetItemTextColor( i, 0, _BLACK, _WHITE );
				pListCtrl->SetItemEnableState( i, true );

				if( vecSelectedIndex.size() > 0 )
				{
					vector<int>::iterator itvect = find( vecSelectedIndex.begin(), vecSelectedIndex.end(), i );
						
					if( itvect != vecSelectedIndex.end() )
					{
						pListCtrl->SetSelectionMark( i );
						
						// When calling 'SetItemState', the 'ON_NOTIFY' is sent to this dialog with the message "LVN_ITEMCHANGED" and then
						// the 'OnSelchangeListCircuitTypeSrc' method is called. And, in this method, if the current state is 'LVIS_SELECTED',
						// we call the '_FillInListCircuitTypeSrc' that itself will call the '_UpdateListCtrl' and we are in an infinite loop.
						m_bOnSelchangeListCircuitTypeEnabled = false;
						pListCtrl->SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );
						m_bOnSelchangeListCircuitTypeEnabled = true;
						
						pListCtrl->SetItemTextBkColor( i, 0, _SELECTEDBLUE );
					}
					else
					{
						pListCtrl->SetItemState( i, 0, LVIS_SELECTED );
						pListCtrl->SetItemTextBkColor( i, 0, _WHITE );
					}
				}
				else
				{
					pListCtrl->SetItemState( i, 0, LVIS_SELECTED );
					pListCtrl->SetItemTextBkColor( i, 0, _WHITE );
				}
			}
			else
			{
				pListCtrl->SetItemTextColor( i, 0, _GRAY, _GRAY );
				pListCtrl->SetItemEnableState( i, false );
				pListCtrl->SetItemState( i, 0, LVIS_SELECTED );
				pListCtrl->SetItemTextBkColor( i, 0, _WHITE );
			}
				
			pListCtrl->Update( i );
		}
	}
	
	pListCtrl->UpdateWindow();
}

bool CDlgSRPageCircuit::_IsAtLeastOneValveTypeChecked()
{
	bool bChecked = ( TRUE == m_CheckValveTypeIMISrc.IsWindowEnabled() && BST_CHECKED == m_CheckValveTypeIMISrc.GetCheck() );
	bChecked |= ( TRUE == m_CheckValveTypeKvsSrc.IsWindowEnabled() && BST_CHECKED == m_CheckValveTypeKvsSrc.GetCheck() );
	return bChecked;
}

bool CDlgSRPageCircuit::_IsAtLeastOneControlTypeChecked()
{
	bool bChecked = ( TRUE == m_CheckControlTypeProportionalSrc.IsWindowEnabled() && BST_CHECKED == m_CheckControlTypeProportionalSrc.GetCheck() );
	bChecked |= ( TRUE == m_CheckControlType3PointsSrc.IsWindowEnabled() && BST_CHECKED == m_CheckControlType3PointsSrc.GetCheck() );
	bChecked |= ( TRUE == m_CheckControlTypeOnOffSrc.IsWindowEnabled() && BST_CHECKED == m_CheckControlTypeOnOffSrc.GetCheck() );
	return bChecked;
}

bool CDlgSRPageCircuit::_IsAtLeastOneControlValveTypeChecked()
{
	bool bChecked = ( TRUE == m_CheckControlValveTypeCombBalCVSrc.IsWindowEnabled() && BST_CHECKED == m_CheckControlValveTypeCombBalCVSrc.GetCheck() );
	bChecked |= ( TRUE == m_CheckControlValveTypeStandardCVSrc.IsWindowEnabled() && BST_CHECKED == m_CheckControlValveTypeStandardCVSrc.GetCheck() );
	bChecked |= ( TRUE == m_CheckControlValveTypePresetCvSrc.IsWindowEnabled() && BST_CHECKED == m_CheckControlValveTypePresetCvSrc.GetCheck() );
	bChecked |= ( TRUE == m_CheckControlValveTypePIBCVSrc.IsWindowEnabled() && BST_CHECKED == m_CheckControlValveTypePIBCVSrc.GetCheck() );
	bChecked |= ( TRUE == m_CheckControlValveTypeSmartSrc.IsWindowEnabled() && BST_CHECKED == m_CheckControlValveTypeSmartSrc.GetCheck() );
	return bChecked;
}

void CDlgSRPageCircuit::_GetBalancingTypeSelectedSrc( std::vector<CDB_CircuitScheme::eBALTYPE> &vecBalancingTypeSelected )
{
	if( BST_CHECKED == m_CheckBTNoneSrc.GetCheck() )
	{
		vecBalancingTypeSelected.push_back( CDB_CircuitScheme::eBALTYPE::SP );
	}

	if( BST_CHECKED == m_CheckBTManualBalancingSrc.GetCheck() )
	{
		vecBalancingTypeSelected.push_back( CDB_CircuitScheme::eBALTYPE::BV );
	}

	if( BST_CHECKED == m_CheckBTDiffPressureControlSrc.GetCheck() )
	{
		vecBalancingTypeSelected.push_back( CDB_CircuitScheme::eBALTYPE::DPC );
	}

	if( BST_CHECKED == m_CheckBTElectronicSrc.GetCheck() )
	{
		vecBalancingTypeSelected.push_back( CDB_CircuitScheme::eBALTYPE::ELECTRONIC );
	}

	if( BST_CHECKED == m_CheckBTElectronicDpCSrc.GetCheck() )
	{
		vecBalancingTypeSelected.push_back( CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC );
	}
}

void CDlgSRPageCircuit::_GetCircuitTypeSelectedSrc( std::vector<CDB_CircSchemeCateg *> &vecCircuitTypeSelected )
{
	POSITION pos = m_ListBoxCircuitTypeSrc.GetFirstSelectedItemPosition();

	while( NULL != pos )
	{
		int nItem = m_ListBoxCircuitTypeSrc.GetNextSelectedItem( pos );
		vecCircuitTypeSelected.push_back( (CDB_CircSchemeCateg *)m_ListBoxCircuitTypeSrc.GetItemData( nItem ) );
	}
}

void CDlgSRPageCircuit::_GetValveTypeSelectedSrc( std::vector<_eWorkForCV> &vecValveTypeSelected )
{
	if( BST_CHECKED == m_CheckValveTypeIMISrc.GetCheck() )
	{
		vecValveTypeSelected.push_back( _eWorkForCV::ewcvTA );
	}

	if( BST_CHECKED == m_CheckValveTypeKvsSrc.GetCheck() )
	{
		vecValveTypeSelected.push_back( _eWorkForCV::ewcvKvs );
	}
}

void CDlgSRPageCircuit::_GetControlTypeSelectedSrc( std::vector<CDB_ControlProperties::CvCtrlType> &vecControlTypeSelected )
{
	if( BST_CHECKED == m_CheckControlTypeProportionalSrc.GetCheck() )
	{
		vecControlTypeSelected.push_back( CDB_ControlProperties::CvCtrlType::eCvProportional );
	}

	if( BST_CHECKED == m_CheckControlType3PointsSrc.GetCheck() )
	{
		vecControlTypeSelected.push_back( CDB_ControlProperties::CvCtrlType::eCv3point );
	}

	if( BST_CHECKED == m_CheckControlTypeOnOffSrc.GetCheck() )
	{
		vecControlTypeSelected.push_back( CDB_ControlProperties::CvCtrlType::eCvOnOff );
	}
}

void CDlgSRPageCircuit::_GetControlValveTypeSelectedSrc( std::vector<CDB_ControlProperties::eCVFUNC> &vecControlValveTypeSelected )
{
	if( BST_CHECKED == m_CheckControlValveTypeStandardCVSrc.GetCheck() )
	{
		vecControlValveTypeSelected.push_back( CDB_ControlProperties::eCVFUNC::ControlOnly );
	}

	if( BST_CHECKED == m_CheckControlValveTypeCombBalCVSrc.GetCheck() )
	{
		vecControlValveTypeSelected.push_back( CDB_ControlProperties::eCVFUNC::PresetPT );
	}

	if( BST_CHECKED == m_CheckControlValveTypePresetCvSrc.GetCheck() )
	{
		vecControlValveTypeSelected.push_back( CDB_ControlProperties::eCVFUNC::Presettable );
	}

	if( BST_CHECKED == m_CheckControlValveTypePIBCVSrc.GetCheck() )
	{
		CDB_ControlProperties::eCVFUNC eCVFunc = (CDB_ControlProperties::eCVFUNC)( CDB_ControlProperties::eCVFUNC::Presettable | CDB_ControlProperties::eCVFUNC::PresetPT );
		vecControlValveTypeSelected.push_back( eCVFunc );
	}

	if( BST_CHECKED == m_CheckControlValveTypeSmartSrc.GetCheck() )
	{
		vecControlValveTypeSelected.push_back( CDB_ControlProperties::eCVFUNC::Electronic );
	}
}

CDB_CircSchemeCateg *CDlgSRPageCircuit::_GetCircuitTypeSelectedDest()
{
	CDB_CircSchemeCateg *pclCircSchemeCateg = NULL;
	POSITION pos = m_ListBoxCircuitTypeDest.GetFirstSelectedItemPosition();

	if( NULL != pos )
	{
		int nItem = m_ListBoxCircuitTypeDest.GetNextSelectedItem( pos );
		pclCircSchemeCateg = (CDB_CircSchemeCateg *)m_ListBoxCircuitTypeDest.GetItemData( nItem );
	}

	return pclCircSchemeCateg;
}

void CDlgSRPageCircuit::_UpdateBalancingTypeValueDest()
{
	if( 0 == m_iRadioBalancingTypeDest )
	{
		m_eRadioBalancingTypeDest = CDB_CircuitScheme::eBALTYPE::SP;
	}
	else if( 1 == m_iRadioBalancingTypeDest )
	{
		m_eRadioBalancingTypeDest = CDB_CircuitScheme::eBALTYPE::BV;
	}
	else if( 2 == m_iRadioBalancingTypeDest )
	{
		m_eRadioBalancingTypeDest = CDB_CircuitScheme::eBALTYPE::DPC;
	}
	else if( 3 == m_iRadioBalancingTypeDest )
	{
		m_eRadioBalancingTypeDest = CDB_CircuitScheme::eBALTYPE::ELECTRONIC;
	}
	else
	{
		m_eRadioBalancingTypeDest = CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC;
	}
}
	
void CDlgSRPageCircuit::_UpdateValveTypeValueDest()
{
	if( 0 == m_iRadioValveTypeDest )
	{
		m_eRadioValveTypeDest = _eWorkForCV::ewcvTA;
	}
	else
	{
		m_eRadioValveTypeDest = _eWorkForCV::ewcvKvs;
	}
}

void CDlgSRPageCircuit::_UpdateControlTypeValueDest()
{
	if( 0 == m_iRadioControlTypeDest )
	{
		m_eRadioControlTypeDest = CDB_ControlProperties::CvCtrlType::eCvProportional;
	}
	else if( 1 == m_iRadioControlTypeDest )
	{
		m_eRadioControlTypeDest = CDB_ControlProperties::CvCtrlType::eCv3point;
	}
	else
	{
		m_eRadioControlTypeDest = CDB_ControlProperties::CvCtrlType::eCvOnOff;
	}
}

void CDlgSRPageCircuit::_UpdateControlValveTypeValueDest()
{
	if( 0 == m_iRadioControlValveTypeDest )
	{
		m_eRadioControlValveTypeDest = CDB_ControlProperties::eCVFUNC::ControlOnly;
	}
	else if( 1 == m_iRadioControlValveTypeDest )
	{
		m_eRadioControlValveTypeDest = CDB_ControlProperties::eCVFUNC::Presettable;
	}
	else if( 2 == m_iRadioControlValveTypeDest )
	{
		m_eRadioControlValveTypeDest = CDB_ControlProperties::eCVFUNC::PresetPT;
	}
	else if( 3 == m_iRadioControlValveTypeDest )
	{
		m_eRadioControlValveTypeDest = CDB_ControlProperties::eCVFUNC(CDB_ControlProperties::eCVFUNC::Presettable | CDB_ControlProperties::eCVFUNC::PresetPT);
	}
	else if( 4 == m_iRadioControlValveTypeDest )
	{
		m_eRadioControlValveTypeDest = CDB_ControlProperties::eCVFUNC::Electronic;
	}
}