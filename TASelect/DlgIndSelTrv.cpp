#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"
#include "Select.h"

#include "DlgNoDevTrvFound.h"
#include "DlgConfSel.h"
#include "DlgTechParam.h"
#include "DlgSizeRad.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelTrv.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelTrv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelTrv::CDlgIndSelTrv( CWnd *pParent )
	: CDlgIndSelBase( m_clIndSelTRVParams, CDlgIndSelTrv::IDD, pParent )
{
	m_clIndSelTRVParams.m_eProductSubCategory = ProductSubCategory::PSC_TC_ThermostaticValve;
	m_clIndSelTRVParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	pRViewSSelTrv = NULL;
	m_bDiffDpSV = false;
	m_bDiffDpRV = false;
	m_dMaxDpSV = 0.0;
	m_dMaxDpRV = 0.0;
	m_strKvsOrCv = _T("");
	m_iRadioThermoOrElectroActuator = Radio_ThermostaticHead;
}

CDlgIndSelTrv::~CDlgIndSelTrv()
{
}

void CDlgIndSelTrv::SaveSelectionParameters()
{
	CDlgIndSelBase::SaveSelectionParameters();

	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelTRVParams.m_pTADS->GetpIndSelParameter();
	
	if( NULL == pclIndSelParameter )
	{
		return;
	}
	// HYS-1305 : Add Power / DT field for dialog trv
	pclIndSelParameter->SetTrvRadioFlowPowerDT( (int)m_clIndSelTRVParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetTrvDpCheckBox( (int)m_clIndSelTRVParams.m_bDpEnabled );
	pclIndSelParameter->SetTrvActuatorType( (int)m_clIndSelTRVParams.m_bIsThermostaticHead );
	pclIndSelParameter->SetTrvRadiatorValveType( (int)m_clIndSelTRVParams.m_eValveType );
	pclIndSelParameter->SetTrvSVInsertInKv( ( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType ) ? true : false );

	// If we are not working with insert...
	if( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType )
	{
		pclIndSelParameter->SetTrvSVFamilyID( m_clIndSelTRVParams.m_strComboSVFamilyID );
		pclIndSelParameter->SetTrvSVConnectID( m_clIndSelTRVParams.m_strComboSVConnectID );
		pclIndSelParameter->SetTrvSVVersionID( m_clIndSelTRVParams.m_strComboSVVersionID );

		pclIndSelParameter->SetTrvRVFamilyID( m_clIndSelTRVParams.m_strComboRVFamilyID );
		pclIndSelParameter->SetTrvRVConnectID( ( RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode ) ? m_clIndSelTRVParams.m_strComboRVConnectID : _T("") );
		pclIndSelParameter->SetTrvRVVersionID( ( RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode ) ? m_clIndSelTRVParams.m_strComboRVVersionID : _T("") );
		pclIndSelParameter->SetTrvRVMode( m_clIndSelTRVParams.m_eReturnValveMode );

		pclIndSelParameter->SetTrvSVInsertName( _T("") );
		pclIndSelParameter->SetTrvSVInsertFamilyID( _T("") );
		pclIndSelParameter->SetTrvSVInsertTypeID( _T("") );
		pclIndSelParameter->SetTrvSVInsertKvValue( 0.0 );
		pclIndSelParameter->SetTrvRVInsertFamilyID( _T("") );

		pclIndSelParameter->SetTrvActuatorTypeID( m_clIndSelTRVParams.m_strComboActuatorTypeID );
		pclIndSelParameter->SetTrvActuatorFamilyID( m_clIndSelTRVParams.m_strComboActuatorFamilyID );
		pclIndSelParameter->SetTrvActuatorVersionID( m_clIndSelTRVParams.m_strComboActuatorVersionID );
	}
	else
	{
		pclIndSelParameter->SetTrvSVFamilyID( _T("") );
		pclIndSelParameter->SetTrvSVConnectID( _T("") );
		pclIndSelParameter->SetTrvSVVersionID( _T("") );

		pclIndSelParameter->SetTrvRVFamilyID( _T("") );
		pclIndSelParameter->SetTrvRVConnectID( _T("") );
		pclIndSelParameter->SetTrvRVVersionID( _T("") );
		if( false == pclIndSelParameter->GetTrvSVInsertInKv() )
		{
			pclIndSelParameter->SetTrvRVMode( RadiatorReturnValveMode::RRVM_IMI );
		}
		else
		{
			pclIndSelParameter->SetTrvRVMode( RadiatorReturnValveMode::RRVM_Other );
		}

		pclIndSelParameter->SetTrvSVInsertName( m_clIndSelTRVParams.m_strComboSVInsertName );
		pclIndSelParameter->SetTrvSVInsertFamilyID( m_clIndSelTRVParams.m_strComboSVInsertFamilyID );
		pclIndSelParameter->SetTrvSVInsertTypeID( m_clIndSelTRVParams.m_strComboSVInsertTypeID );
		pclIndSelParameter->SetTrvSVInsertKvValue( m_clIndSelTRVParams.m_dInsertKvValue );
		pclIndSelParameter->SetTrvRVInsertFamilyID( m_clIndSelTRVParams.m_strComboRVInsertFamilyID );

		pclIndSelParameter->SetTrvActuatorTypeID( _T("") );
		pclIndSelParameter->SetTrvActuatorFamilyID( _T("") );
		pclIndSelParameter->SetTrvActuatorVersionID( _T("") );
	}
	
	pclIndSelParameter->SetTrvRadInfos( &m_clIndSelTRVParams.m_RadInfos );

	if( NULL != pRViewSSelTrv )
	{
		pRViewSSelTrv->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelTrv::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelTrv )
	{
		pRViewSSelTrv->Reset();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelTrv, CDlgIndSelBase )
	ON_COMMAND( IDC_GROUPDP, OnBnClickedCheckGroupDp )
	ON_BN_CLICKED( IDC_BUTTONCOMPRAD, OnButtonComputeRadiatorData )
	ON_CBN_SELCHANGE( IDC_COMBOVALVETYPE, OnCbnSelChangeRadiatorValveType )
	ON_CBN_SELCHANGE( IDC_COMBOSUPVALVE, OnCbnSelChangeSVFamily )
	ON_CBN_SELCHANGE( IDC_COMBOSUPCONNECT, OnCbnSelChangeSVConnect )
	ON_CBN_SELCHANGE( IDC_COMBOSUPVER, OnCbnSelChangeSVVersion )
	ON_CBN_SELCHANGE( IDC_COMBORETVALVE, OnCbnSelChangeRVFamily )
	ON_CBN_SELCHANGE( IDC_COMBORETCONNECT, OnCbnSelChangeRVConnect )
	ON_CBN_SELCHANGE( IDC_COMBORETVER, OnCbnSelChangeRVVersion )
	ON_CBN_SELCHANGE( IDC_COMBOSVINSERTNAME, OnCbnSelChangeSVInsertName )
	ON_CBN_SELCHANGE( IDC_COMBOSVINSERTFAMILY, OnCbnSelChangeSVInsertFamily )
	ON_CBN_SELCHANGE( IDC_COMBOSVINSERTTYPE, OnCbnSelChangeSVInsertType )
	ON_CBN_SELCHANGE( IDC_COMBORVINSERTFAMILY, OnCbnSelChangeRVInsertFamily )
	ON_CBN_SELCHANGE( IDC_COMBOACTRTYPE, OnCbnSelChangeActuatorType )
	ON_CBN_SELCHANGE( IDC_COMBOACTRFAM, OnCbnSelChangeActuatorFamily )
	ON_CBN_SELCHANGE( IDC_COMBOACTRVER, OnCbnSelChangeActuatorVersion )
	ON_EN_CHANGE( IDC_EDITKV, OnChangeEditInsertKv )
	ON_EN_KILLFOCUS( IDC_EDITKV, OnKillFocusEditInsertKv )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITKV, OnEditEnterChar )
	ON_BN_CLICKED( IDC_RADIOTHRMHEAD, OnBnClickedRadioActuatorType )
	ON_BN_CLICKED( IDC_RADIOELECACTR, OnBnClickedRadioActuatorType )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDTRV, OnModifySelectedTrv )
END_MESSAGE_MAP()

void CDlgIndSelTrv::DoDataExchange( CDataExchange *pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTONCOMPRAD, m_ButtonCompRad );
	DDX_Control( pDX, IDC_COMBOVALVETYPE, m_ComboRadiatorValveType );
	DDX_Control( pDX, IDC_COMBOSUPVER, m_ComboSVVersion );
	DDX_Control( pDX, IDC_COMBOSUPVALVE, m_ComboSVFamily );
	DDX_Control( pDX, IDC_COMBOSUPCONNECT, m_ComboSVConnect );
	DDX_Control( pDX, IDC_COMBORETVALVE, m_ComboRVFamily );
	DDX_Control( pDX, IDC_COMBORETCONNECT, m_ComboRVConnect );
	DDX_Control( pDX, IDC_COMBORETVER, m_ComboRVVersion );
	DDX_Control( pDX, IDC_COMBOSVINSERTNAME, m_ComboSVInsertName );
	DDX_Control( pDX, IDC_COMBOSVINSERTFAMILY, m_ComboSVInsertFamily );
	DDX_Control( pDX, IDC_COMBOSVINSERTTYPE, m_ComboSVInsertType );
	DDX_Control( pDX, IDC_EDITKV, m_EditSVInsertKv );
	DDX_Control( pDX, IDC_COMBORVINSERTFAMILY, m_ComboRVInsertFamily );
	DDX_Control( pDX, IDC_COMBOACTRTYPE, m_ComboActuatorType );
	DDX_Control( pDX, IDC_COMBOACTRFAM, m_ComboActuatorFamily );
	DDX_Control( pDX, IDC_COMBOACTRVER, m_ComboActuatorVersion );
	DDX_Control( pDX, IDC_GROUPVALVE, m_GroupValve );
	DDX_Control( pDX, IDC_GROUPTHRMH, m_GroupThermoHead );
	DDX_Radio( pDX, IDC_RADIOTHRMHEAD, m_iRadioThermoOrElectroActuator );

	// 'CDlgIndSelBase' variables.
	DDX_Control( pDX, IDC_EDITDP, m_clExtEditDp );
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_GROUPDP, m_clGroupDp );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );
}

BOOL CDlgIndSelTrv::OnInitDialog()
{
	CDlgIndSelBase::OnInitDialog();

	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str;
	
	// HYS-1305 : Add Power / DT field for dialog trv
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELBV_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str ); 
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICWORKWITH );
	GetDlgItem( IDC_STATICWORKWITH )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_RADIOTHRMHEAD );
	GetDlgItem( IDC_RADIOTHRMHEAD )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_RADIOELECACTR );
	GetDlgItem( IDC_RADIOELECACTR )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICVALVETYPE );
	GetDlgItem( IDC_STATICVALVETYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICSSVNAME );
	GetDlgItem( IDC_STATICSUPNAME )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICRVNAME );
	GetDlgItem( IDC_STATICRETNAME )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICCONNECT );
	GetDlgItem( IDC_STATICSUPCONNECT )->SetWindowText( str );
	GetDlgItem( IDC_STATICRETCONNECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICVERSION );
	GetDlgItem( IDC_STATICSUPVER )->SetWindowText( str );
	GetDlgItem( IDC_STATICRETVER )->SetWindowText( str );

	// Insert
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICINSERTNAME );
	GetDlgItem( IDC_STATICINSERTNAME )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICINSERTFAMILY );
	GetDlgItem( IDC_STATICINSERTFAMILY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICINSERTTYPE );
	GetDlgItem( IDC_STATICINSERTTYPE )->SetWindowText( str );

	// Actuators.
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICACTRTYPE );
	GetDlgItem( IDC_STATICACTRTYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICACTRFAM );
	GetDlgItem( IDC_STATICACTRFAM )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICACTRVER );
	GetDlgItem( IDC_STATICACTRVER )->SetWindowText( str );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	
	if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		m_strKvsOrCv = TASApp.LoadLocalizedString( IDS_KVS );
	}
	else
	{
		m_strKvsOrCv = TASApp.LoadLocalizedString( IDS_CV );
	}

	GetDlgItem( IDC_STATICKVS )->SetWindowText( m_strKvsOrCv );

	// Set proper style and add icons to Qdp and valve groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupQ.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	m_clGroupQ.SetInOffice2007Mainframe( true );
	
	m_clGroupDp.SetCheckBoxStyle( BS_AUTOCHECKBOX );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupDp.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_DP );
	}

	m_clGroupDp.SetInOffice2007Mainframe( true );
	
	if( NULL != pclImgListGroupBox )
	{
		m_GroupValve.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
	}

	m_GroupValve.SetInOffice2007Mainframe( true );
	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupValve.SetExpandCollapseMode( true );
// 	m_GroupValve.SetNotificationHandler( this );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupThermoHead.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_ThermoH );
	}

	m_GroupThermoHead.SetInOffice2007Mainframe( true );
	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupThermoHead.SetExpandCollapseMode( true );
// 	m_GroupThermoHead.SetNotificationHandler( this );

	// Add bitmap and tool tip to CompRad button.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT ( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButtonCompRad.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_CompRad ) );
	}

	m_EditSVInsertKv.SetPhysicalType( _C_KVCVCOEFF );
	m_EditSVInsertKv.SetEditType( CNumString::eDouble, CNumString::ePositive );

	if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString TTstr;
		TTstr = TASApp.LoadLocalizedString( IDS_LVTOOLTIP_BUTCOMPRAD );
		m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTTONCOMPRAD ), TTstr );
	}

	return TRUE;  
}

void CDlgIndSelTrv::OnBnClickedCheckGroupDp()
{
	_UpdateGroupDp();
	ClearAll();
}

void CDlgIndSelTrv::OnButtonComputeRadiatorData() 
{
	CDlgSizeRad clCDlgSizeRad( &m_clIndSelTRVParams, this );
	
	if( IDOK == clCDlgSizeRad.DoModal() )
	{
		// Keep a copy before clearing all.
		RadInfo_struct rRadInfos = m_clIndSelTRVParams.m_RadInfos;

		ClearAll();

		m_clIndSelTRVParams.m_RadInfos = rRadInfos;
		m_clIndSelTRVParams.m_dFlow = m_clIndSelTRVParams.m_RadInfos.dFlow;
		UpdateFlowFieldValue();

		// HYS-1305 : Add insert with automatic flow limiter
		if( ( RadiatorValveType::RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
			&& ( false == m_clIndSelTRVParams.m_bIsFLCVInsert ) )
		{
			m_clIndSelTRVParams.m_bDpEnabled = m_clIndSelTRVParams.m_RadInfos.bAvailableDpChecked;
			m_clGroupDp.SetCheck( ( true == m_clIndSelTRVParams.m_bDpEnabled ) ? TRUE : FALSE );
			
			m_clIndSelTRVParams.m_dDp = 0.0;

			if( true == m_clIndSelTRVParams.m_bDpEnabled )
			{
				m_clIndSelTRVParams.m_dDp = m_clIndSelTRVParams.m_RadInfos.dAvailableDp;
			}

			_UpdateGroupDp();
		}

		PostWMCommandToControl( GetpBtnSuggest() );
	}
}

void CDlgIndSelTrv::OnCbnSelChangeRadiatorValveType()
{
	m_clIndSelTRVParams.m_eValveType = (RadiatorValveType)m_ComboRadiatorValveType.GetCurSel();
	_UpdateLayout();

	// Update combos.
	if( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType )
	{
		// HYS-1305 : Add insert with automatic flow limiter
		m_clIndSelTRVParams.m_bIsFLCVInsert = false;
		_FillComboSVFamily();
		OnCbnSelChangeSVFamily();
		// HYS-1002: We arre not in RVT_Inserts mode m_eInsertType must have the init value 
		m_clIndSelTRVParams.m_eInsertType = RIT_Undefined;
	}
	else
	{
		// HYS-1002: Don't need to use the last Insert name when we was in RVT_Inserts mode
		_FillComboSVInsertName( );
		OnCbnSelChangeSVInsertName();
	}
}

void CDlgIndSelTrv::OnCbnSelChangeSVFamily()
{
	m_clIndSelTRVParams.m_strComboSVFamilyID = m_ComboSVFamily.GetCBCurSelIDPtr().ID;

	if( _T('\0') != *m_clIndSelTRVParams.m_strComboSVFamilyID )
	{
		// There is no combo for the type but we use type to extract valves from the database.
		m_clIndSelTRVParams.m_strSVTypeID = ( (CDB_StringID*)m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboSVFamilyID ).MP )->GetIDstr2();
		
		m_clIndSelTRVParams.m_bIsTrvTypePreset = ( _T("TRVTYPE_PRESET") == m_clIndSelTRVParams.m_strSVTypeID ) ? true : false;
	}
	// HYS-1776: Now we have the possibility to choose *** All families ***. So when the valve type
	// is Presettable Trv we have to set m_strSVTypeID and m_bIsTrvTypePreset.
	else if( m_clIndSelTRVParams.m_eValveType == RadiatorValveType::RVT_Presettable )
	{
		m_clIndSelTRVParams.m_strSVTypeID = _T( "TRVTYPE_PRESET" );
		m_clIndSelTRVParams.m_bIsTrvTypePreset = true;
	}
	else
	{
		m_clIndSelTRVParams.m_strSVTypeID = _T("");
		m_clIndSelTRVParams.m_bIsTrvTypePreset = false;
	}

	_UpdateLayout();
	_FillComboSVConnect();
	OnCbnSelChangeSVConnect();
}

void CDlgIndSelTrv::OnCbnSelChangeSVConnect()
{
	m_clIndSelTRVParams.m_strComboSVConnectID = m_ComboSVConnect.GetCBCurSelIDPtr().ID;
	_FillComboSVVersion();
	OnCbnSelChangeSVVersion();
}

void CDlgIndSelTrv::OnCbnSelChangeSVVersion() 
{
	m_clIndSelTRVParams.m_strComboSVVersionID = m_ComboSVVersion.GetCBCurSelIDPtr().ID;

	// Actuator combos are linked to current supply valve choice.
	_FillComboActuatorType();
	OnCbnSelChangeActuatorType();
}

void CDlgIndSelTrv::OnCbnSelChangeRVFamily() 
{
	m_clIndSelTRVParams.m_strComboRVFamilyID = m_ComboRVFamily.GetCBCurSelIDPtr().ID;
	_UpdateLayout();

	if( RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode )
	{
		_FillComboRVConnect();
		OnCbnSelChangeRVConnect();
	}
	else
	{
		ClearAll();
	}
}

void CDlgIndSelTrv::OnCbnSelChangeRVConnect() 
{
	m_clIndSelTRVParams.m_strComboRVConnectID = m_ComboRVConnect.GetCBCurSelIDPtr().ID;
	_FillComboRVVersion();
	OnCbnSelChangeRVVersion();
}

void CDlgIndSelTrv::OnCbnSelChangeRVVersion()
{
	m_clIndSelTRVParams.m_strComboRVVersionID = m_ComboRVVersion.GetCBCurSelIDPtr().ID;
	ClearAll();
}

void CDlgIndSelTrv::OnCbnSelChangeSVInsertName() 
{
	if( CB_ERR == m_ComboSVInsertName.GetCurSel() )
	{
		return;
	}
	
	CString str( _T("") );
	int n = m_ComboSVInsertName.GetLBTextLen( m_ComboSVInsertName.GetCurSel() );
	m_ComboSVInsertName.GetLBText( m_ComboSVInsertName.GetCurSel(), str.GetBuffer( n ) );
	
	m_clIndSelTRVParams.m_strComboSVInsertName = str;
	m_clIndSelTRVParams.m_eInsertType = ( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_OTHERINSERT ) == str ) ? RadiatorInsertType::RIT_ByKv : RadiatorInsertType::RIT_Heimeier;

	_UpdateLayout();

	if( RadiatorInsertType::RIT_Heimeier == m_clIndSelTRVParams.m_eInsertType )
	{
		_FillComboSVInsertFamily();
		OnCbnSelChangeSVInsertFamily();
	}
}

void CDlgIndSelTrv::OnCbnSelChangeSVInsertFamily()
{
	m_clIndSelTRVParams.m_strComboSVInsertFamilyID = m_ComboSVInsertFamily.GetCBCurSelIDPtr().ID;
	_FillComboSVInsertType();
	OnCbnSelChangeSVInsertType();
}

void CDlgIndSelTrv::OnCbnSelChangeSVInsertType()
{
	m_clIndSelTRVParams.m_strComboSVInsertTypeID = m_ComboSVInsertType.GetCBCurSelIDPtr().ID;
	ClearAll();	
}

void CDlgIndSelTrv::OnCbnSelChangeRVInsertFamily()
{
	m_clIndSelTRVParams.m_strComboRVInsertFamilyID = m_ComboRVInsertFamily.GetCBCurSelIDPtr().ID;
	ClearAll();
}

void CDlgIndSelTrv::OnCbnSelChangeActuatorType()
{
	m_clIndSelTRVParams.m_strComboActuatorTypeID = m_ComboActuatorType.GetCBCurSelIDPtr().ID;
	_FillComboActuatorFamily();
	OnCbnSelChangeActuatorFamily();
}

void CDlgIndSelTrv::OnCbnSelChangeActuatorFamily()
{
	m_clIndSelTRVParams.m_strComboActuatorFamilyID = m_ComboActuatorFamily.GetCBCurSelIDPtr().ID;
	_FillComboActuatorVersion();
	OnCbnSelChangeActuatorVersion();
}

void CDlgIndSelTrv::OnCbnSelChangeActuatorVersion()
{
	m_clIndSelTRVParams.m_strComboActuatorVersionID = m_ComboActuatorVersion.GetCBCurSelIDPtr().ID;
	ClearAll();
}

void CDlgIndSelTrv::OnChangeEditInsertKv()
{
	ReadCUDouble( _C_KVCVCOEFF, m_EditSVInsertKv, &m_clIndSelTRVParams.m_dInsertKvValue );
	ClearAll();
}

void CDlgIndSelTrv::OnKillFocusEditInsertKv() 
{
	double dVal;

	if( RD_NOT_NUMBER == ReadCUDouble( _C_KVCVCOEFF, m_EditSVInsertKv, &dVal ) )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
		m_EditSVInsertKv.SetSel( 0, -1 );
		m_EditSVInsertKv.SetFocus();
	}
}

void CDlgIndSelTrv::OnBnClickedRadioActuatorType()
{
	_UpdateRadioThermoOrElectroActuator();
	_UpdateLayout();

	// Update actuator combos.
	_FillComboActuatorType();
	OnCbnSelChangeActuatorType();
}

LRESULT CDlgIndSelTrv::OnModifySelectedTrv(WPARAM wParam, LPARAM lParam)
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelRadSet *pSelTrv = dynamic_cast<CDS_SSelRadSet *>( (CData *)lParam );
	ASSERT( NULL != pSelTrv );

	if( NULL == pSelTrv || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelTRVParams.m_SelIDPtr = pSelTrv->GetIDPtr();

	m_bInitialised = false;

	// Send message to notify the display of the SSel tab.
	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// HYS-1242: SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );
	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_TC_ThermostaticValve );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelTRVParams.m_eApplicationType = pSelTrv->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelTRVParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelTRVParams.m_WC = *( pSelTrv->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pSelTrv->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Remark: The 'm_clIndSelTRVParams.m_strPipeSeriesID' and 'm_clIndSelTRVParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pSelTrv->GetPipeSeriesID(), pSelTrv->GetPipeID() );

	// Set flow.
	m_clIndSelTRVParams.m_dFlow = pSelTrv->GetQ();
	UpdateFlowFieldValue();

	// Set Dp.
	m_clIndSelTRVParams.m_dDp = pSelTrv->GetDp();
	m_clIndSelTRVParams.m_bDpEnabled = ( m_clIndSelTRVParams.m_dDp > 0.0 ) ? true : false;
	m_clGroupDp.SetCheck( ( true == m_clIndSelTRVParams.m_bDpEnabled ) ? BST_CHECKED : BST_UNCHECKED );
	_UpdateGroupDp();

	// Get return valve mode.
	m_clIndSelTRVParams.m_eReturnValveMode = pSelTrv->GetReturnValveMode();

	// Get the thermostatic radiator valve type.
	m_clIndSelTRVParams.m_eValveType = (RadiatorValveType)pSelTrv->GetRadiatorValveType();

	// Get thermostatic head check box state.
	// Remark: 'm_iRadioThermoOrElectroActuator' must be set before any call to '_UpdateLayout'.
	m_iRadioThermoOrElectroActuator = pSelTrv->GetActuatorType();
	
	// HYS-1305 : Add Power / DT field for dialog trv
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pSelTrv->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelBVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelTRVParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelTRVParams.m_dFlow = pSelTrv->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelTRVParams.m_dPower = pSelTrv->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelTRVParams.m_dDT = pSelTrv->GetDT();
		UpdateDTFieldValue();
	}


	// Remark: 'm_clIndSelTRVParams.m_bIsThermostaticHead' is updated in the following method in regards to the 'm_iRadioThermoOrElectroActuator' variable.
	_UpdateRadioThermoOrElectroActuator();

	// Fill combos.
	if( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType )
	{
		// HYS-1305 : Add insert with automatic flow limiter
		m_clIndSelTRVParams.m_bIsFLCVInsert = false;

		// Remark: Corresponding combo variables in 'm_clIndSelTRVParams' are updated in each of this following methods.
		_FillComboRadiatorValveType( m_clIndSelTRVParams.m_eValveType );
		_FillComboSVFamily( pSelTrv->GetSupplyValveFamilyID() );
		_FillComboSVConnect( pSelTrv->GetSupplyValveConnectID() );
		_FillComboSVVersion( pSelTrv->GetSupplyValveVersionID() );

		_FillComboRVFamily( pSelTrv->GetReturnValveFamilyID() );
		OnCbnSelChangeRVFamily();
		
		_FillComboRVConnect( pSelTrv->GetReturnValveConnectID() );
		_FillComboRVVersion( pSelTrv->GetReturnValveVersionID() );

		_FillComboActuatorType( pSelTrv->GetActuatorTypeID() );
		_FillComboActuatorFamily( pSelTrv->GetActuatorFamilyID() );
		_FillComboActuatorVersion( pSelTrv->GetActuatorVersionID() );

		_FillComboSVInsertName();
		_FillComboSVInsertFamily();
		_FillComboSVInsertType();
		_FillComboRVInsertFamily();
		_UpdateLayout();
	}
	else
	{
		_FillComboRadiatorValveType( m_clIndSelTRVParams.m_eValveType );
		_FillComboSVFamily();
		_FillComboSVConnect();
		_FillComboSVVersion();
		_FillComboRVFamily();
		_FillComboRVConnect();
		_FillComboRVVersion();

		// Remark: Corresponding combo variables in 'm_clIndSelTRVParams' are updated in each of this following methods.
		_FillComboSVInsertName( pSelTrv->GetSVInsertName() );
		_FillComboSVInsertFamily( pSelTrv->GetSupplyValveFamilyID() );
		_FillComboSVInsertType( pSelTrv->GetSupplyValveTypeID() );
		_FillComboRVInsertFamily( pSelTrv->GetReturnValveFamilyID() );

		m_clIndSelTRVParams.m_eInsertType = ( true == pSelTrv->IsInsertInKv() ) ? RadiatorInsertType::RIT_ByKv : RadiatorInsertType::RIT_Heimeier;
		m_clIndSelTRVParams.m_dInsertKvValue = pSelTrv->GetInsertKvValue();

		_UpdateLayout();
		
		if( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType )
		{
			m_EditSVInsertKv.SetSel( 0, -1 );
			m_EditSVInsertKv.ReplaceSel( WriteDouble( m_clIndSelTRVParams.m_dInsertKvValue, 3, 2 ) );
		}
		else
		{		
			// Remark: Corresponding combo variables in 'm_clIndSelTRVParams' are updated in each of this following methods.
			_FillComboActuatorType( pSelTrv->GetActuatorTypeID() );
			_FillComboActuatorFamily( pSelTrv->GetActuatorFamilyID() );
			_FillComboActuatorVersion( pSelTrv->GetActuatorVersionID() );
		}
	}
	
	// Transfer radiator infos.
	pSelTrv->GetRadInfos( &m_clIndSelTRVParams.m_RadInfos );

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );
	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

void CDlgIndSelTrv::OnBnClickedSuggest() 
{
	if( NULL == pRViewSSelTrv )
	{
		return;
	}

	ClearAll();

	// Disable the Selection button.
	EnableSelectButton(false);

	// Build up suggestion (pre-selection) list.
	bool bNoDevFound = false;

	if( true == _VerifySubmittedValues() )
	{
		double dDp = 0.0;
		m_dMaxDpSV = 0.0;		// Will be assigned in SelectTrv().
		m_dMaxDpRV = -DBL_MAX;	// Will be assigned in this function.
		m_bDiffDpSV = false;
		m_bDiffDpRV = false;
		int iSupplyValveFound = 0;
		int iReturnValveFound = 0;

		// Prepare selection list.
		if( NULL != m_clIndSelTRVParams.m_pclSelectSupplyValveList )
		{
			delete m_clIndSelTRVParams.m_pclSelectSupplyValveList;
		}

		m_clIndSelTRVParams.m_pclSelectSupplyValveList = new CSelectTrvList();

		if( NULL == m_clIndSelTRVParams.m_pclSelectSupplyValveList || NULL == m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetSelectPipeList() )
		{
			return;
		}

		m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetSelectPipeList()->SelectPipes( &m_clIndSelTRVParams, m_clIndSelTRVParams.m_dFlow );
		
		// Do a first suggestion with all parameters chosen by user.
		bool bSizeShiftProblem = false;
		_DoSelection( Selection_NoOptimization, iSupplyValveFound, iReturnValveFound, &bSizeShiftProblem );

		if( iSupplyValveFound > 0 || ( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType && iReturnValveFound > 0 ) )
		{
			VerifyModificationMode();

			if( RadiatorInsertType::RIT_ByKv != m_clIndSelTRVParams.m_eInsertType )
			{
				m_bDiffDpSV = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetDiffDpSV();
			}

			CRViewSSelTrv::SuggestParams rSuggesParams;
			rSuggesParams.m_dMaxDpSV = m_dMaxDpSV;
			rSuggesParams.m_dMaxDpRV = m_dMaxDpRV;
			rSuggesParams.m_bIsDifferentDpOnSVExist = m_bDiffDpSV;
			rSuggesParams.m_bIsDifferentDpOnRVExist = m_bDiffDpRV;

			pRViewSSelTrv->Suggest( &m_clIndSelTRVParams, (LPARAM)&rSuggesParams );
		}
		else if( 0 == iSupplyValveFound && true == bSizeShiftProblem )
		{
			// In that case valves exist but not with the parameters specified.
			// Problem of SizeShift.
			if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_SIZE_SHIFT_PROB, MB_YESNO | MB_ICONQUESTION, 0 ) )
			{
				CDlgTechParam *pdlg = new CDlgTechParam( CDlgTechParam::PageGeneralDeviceSizes );

				if( NULL == pdlg )
				{
					return;
				}

				if( IDOK != pdlg->DoModal() )
				{
					// Set the focus on the first available edit control.
					SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
				}

				delete pdlg;
			}
		}
		else /*if( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType )*/
		{
			// NOT FOUND.
			// TODO: propose alternative solution for insert!

			// Try alternative suggestion with less parameters that user has chosen.
			_DoSelection( Selection_Optimization1, iSupplyValveFound, iReturnValveFound, &bSizeShiftProblem );
			
			if( iSupplyValveFound > 0 )
			{
				// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevTrvFound' dialog.
				m_clIndSelTRVParams.m_SupplyValveList.PurgeAll();
				double dKey = 0.0;

				for( CSelectedValve *pclSelectedValve = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
					pclSelectedValve = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetNext<CSelectedValve>() )
				{
					CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

					if( NULL == pTAP )
					{
						continue;
					}

					m_clIndSelTRVParams.m_SupplyValveList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
				}

				// Launch the no trv found dialog box if something is found.
				// Remark: Combos in this dialog will be filled thanks to the new alternative balancing and control valves found and saved in the 'BCVList' variable.
				CDlgNoDevTrvFoundParams clNoDevTrvFoundParams = CDlgNoDevTrvFoundParams( &m_clIndSelTRVParams.m_SupplyValveList, m_clIndSelTRVParams.m_strComboSVFamilyID, 
						m_clIndSelTRVParams.m_strComboSVConnectID, m_clIndSelTRVParams.m_strComboSVVersionID );

				CDlgNoDevTrvFound dlg( &m_clIndSelTRVParams, &clNoDevTrvFoundParams );
				
				if( IDOK == dlg.DoModal() )
				{
					clNoDevTrvFoundParams = *dlg.GetNoDevTrvFoundParams();

					_LaunchSuggestion( clNoDevTrvFoundParams.m_strFamilyID, clNoDevTrvFoundParams.m_strConnectID, clNoDevTrvFoundParams.m_strVersionID );
				}
			}
			else
			{
				// No device found message.
				TASApp.AfxLocalizeMessageBox( AFXMSG_NO_DEV_FOUND );
			}
		}
	}
}

void CDlgIndSelTrv::OnBnClickedSelect() 
{
	try
	{
		if( NULL == m_clIndSelTRVParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelTRVParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelTRVParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelTRVParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelTrv )
		{
			HYSELECT_THROW( _T("Internal error: The thermostatic valve right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The thermostatic valve right view is not visible.") );
		}
		else if( NULL == m_clIndSelTRVParams.m_pclSelectSupplyValveList )
		{
			HYSELECT_THROW( _T("Internal error: Can't retrieve the thermostatic valve selected.") );
		}

		// Verify pointer validity; user can delete object after entering in Edition.
		if( _T('\0') != *m_clIndSelTRVParams.m_SelIDPtr.ID )
		{
			m_clIndSelTRVParams.m_SelIDPtr = m_clIndSelTRVParams.m_pTADS->Get( m_clIndSelTRVParams.m_SelIDPtr.ID );
		}
	
		// Create selected object and initialize it.
		CDS_SSelRadSet *pSelRadSet = dynamic_cast<CDS_SSelRadSet *>( m_clIndSelTRVParams.m_SelIDPtr.MP );

		if( NULL == pSelRadSet )
		{
			IDPTR IDPtr;
			m_clIndSelTRVParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelRadSet ) );
			pSelRadSet = (CDS_SSelRadSet *)IDPtr.MP;
		}
	
		if( m_clIndSelTRVParams.m_dFlow <= 0.0 )
		{
			HYSELECT_THROW( _T("Internal error: Flow '%f' can't be lower or equal to 0."), m_clIndSelTRVParams.m_dFlow );
		}

		pSelRadSet->SetSelectedAsAPackage( m_clIndSelTRVParams.m_bOnlyForSet );

		// Save flow.
		pSelRadSet->SetQ( m_clIndSelTRVParams.m_dFlow );

		// HYS-1305 : Add Power / DT field for dialog trv
		if( CDS_SelProd::efdFlow == m_clIndSelTRVParams.m_eFlowOrPowerDTMode )
		{
			pSelRadSet->SetFlowDef( CDS_SelProd::efdFlow );
			pSelRadSet->SetPower( 0.0 );
			pSelRadSet->SetDT( 0.0 );
		}
		else
		{
			pSelRadSet->SetFlowDef( CDS_SelProd::efdPower );
			pSelRadSet->SetPower( m_clIndSelTRVParams.m_dPower );
			pSelRadSet->SetDT( m_clIndSelTRVParams.m_dDT );
		}

		// Save Dp entered by user (it's here the total Dp!!).
		pSelRadSet->SetDp( m_clIndSelTRVParams.m_dDp );

		if( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType )
		{
			// Supply valve combos info.
			pSelRadSet->SetSupplyValveFamilyID( m_clIndSelTRVParams.m_strComboSVFamilyID );
			pSelRadSet->SetSupplyValveTypeID( _T("") );
			pSelRadSet->SetSupplyValveConnectID( m_clIndSelTRVParams.m_strComboSVConnectID );
			pSelRadSet->SetSupplyValveVersionID( m_clIndSelTRVParams.m_strComboSVVersionID );

			// Return valve combos info.
			pSelRadSet->SetReturnValveFamilyID( m_clIndSelTRVParams.m_strComboRVFamilyID );
			pSelRadSet->SetReturnValveConnectID( ( RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode ) ? m_clIndSelTRVParams.m_strComboRVConnectID : _T("") );
			pSelRadSet->SetReturnValveVersionID( ( RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode ) ? m_clIndSelTRVParams.m_strComboRVVersionID : _T("") );
			pSelRadSet->SetReturnValveMode( m_clIndSelTRVParams.m_eReturnValveMode );

			pSelRadSet->SetSVInsertName( _T("") );

			// Set that we are not working with insert Kv.
			pSelRadSet->SetInsertInKv( false );
		}
		else
		{
			// Supply valve combos info.
			pSelRadSet->SetSupplyValveFamilyID( m_clIndSelTRVParams.m_strComboSVInsertFamilyID );
			pSelRadSet->SetSupplyValveTypeID( m_clIndSelTRVParams.m_strComboSVInsertTypeID );
			pSelRadSet->SetSupplyValveConnectID( _T("") );
			pSelRadSet->SetSupplyValveVersionID( _T("") );

			// Return valve combos info.
			pSelRadSet->SetReturnValveFamilyID( m_clIndSelTRVParams.m_strComboSVInsertFamilyID );
			pSelRadSet->SetReturnValveConnectID( _T("") );
			pSelRadSet->SetReturnValveVersionID( _T("") );

			pSelRadSet->SetSVInsertName( m_clIndSelTRVParams.m_strComboSVInsertName );
		
			// Set if we are working with insert Kv.
			pSelRadSet->SetInsertInKv( ( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType ) ? true : false );

			// Set insert kv value.
			pSelRadSet->SetInsertKvValue( ( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType ) ? m_clIndSelTRVParams.m_dInsertKvValue : 0.0 );

			if( 0 == pSelRadSet->GetInsertKvValue() )
			{
				pSelRadSet->SetReturnValveMode( RadiatorReturnValveMode::RRVM_IMI );
			}
			else
			{
				pSelRadSet->SetReturnValveMode( RadiatorReturnValveMode::RRVM_Other );
			}
		}

		// Set the radiator valve type.
		pSelRadSet->SetRadiatorValveType( (int)m_clIndSelTRVParams.m_eValveType );

		// Copy rad infos.
		pSelRadSet->SetRadInfos( &m_clIndSelTRVParams.m_RadInfos );

		// Set the trv actuator type.
		// HYS-1068: Set the right actuator type 
		if( m_clIndSelTRVParams.m_bIsThermostaticHead )
		{
			pSelRadSet->SetActuatorType( Radio_ThermostaticHead );
		}
		else
		{
			pSelRadSet->SetActuatorType( Radio_ElectroActuator );
		}

		// Set the actuators info.
		pSelRadSet->SetActuatorTypeID( m_clIndSelTRVParams.m_strComboActuatorTypeID );
		pSelRadSet->SetActuatorFamilyID( m_clIndSelTRVParams.m_strComboActuatorFamilyID );
		pSelRadSet->SetActuatorVersionID( m_clIndSelTRVParams.m_strComboActuatorVersionID );

		pSelRadSet->SetPipeSeriesID( m_clIndSelTRVParams.m_strPipeSeriesID );
		pSelRadSet->SetPipeID( m_clIndSelTRVParams.m_strPipeID );

		*pSelRadSet->GetpSelectedInfos()->GetpWCData() = m_clIndSelTRVParams.m_WC;

		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelRadSet->GetpSelectedInfos()->SetDT( m_clIndSelTRVParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelRadSet->GetpSelectedInfos()->SetApplicationType( m_clIndSelTRVParams.m_eApplicationType );

		pRViewSSelTrv->FillInSelected( pSelRadSet );

		CDlgConfSel dlg( &m_clIndSelTRVParams );
		IDPTR IDPtr = pSelRadSet->GetIDPtr();

		dlg.Display( pSelRadSet );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelTRVParams.m_pTADS->Get( _T("RADSET_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'RADSET_TAB' table from the datastruct.") );
			}
		
			if( _T('\0') == *m_clIndSelTRVParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelTRVParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelRadSet ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelTRVParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable Select Button
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelTRVParams.m_SelIDPtr.ID )
			{
				m_clIndSelTRVParams.m_pTADS->DeleteObject( IDPtr );
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelTrv::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelTrv::OnEditEnterChar( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITKV == pNMHDR->idFrom )
	{
		GetpBtnSuggest()->SetFocus();

		// Reset Focus on the modified Edit.
		m_EditSVInsertKv.SetFocus();

		PostWMCommandToControl( GetpBtnSuggest() );
	}
	else
	{
		CDlgIndSelBase::OnEditEnterChar( pNMHDR, pResult );
	}
}

void CDlgIndSelTrv::OnEnChangeFlow() 
{
	if( GetFocus() == GetpEditFlow() )
	{
		CDlgIndSelBase::OnEnChangeFlow();
		ClearAll();

		// Special case for insert used with Kv instead of IMI product.
		_UpdateKvMin();
	}
}

void CDlgIndSelTrv::OnEnChangeDp() 
{
	if( GetFocus() == GetpEditDp() )
	{
		ClearAll();

		// Special case for insert used with Kv instead of IMI product.
		_UpdateKvMin();
	}
}

void CDlgIndSelTrv::ActivateLeftTabDialog()
{
	CDlgIndSelBase::ActivateLeftTabDialog();
	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clIndSelTRVParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clIndSelTRVParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}
}

LRESULT CDlgIndSelTrv::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );
	
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Get last selected parameters.
	CString str1, str2;
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelTRVParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}
	
	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == ( CDS_SelProd::eFlowDef )pclIndSelParameter->GetTrvRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );

	// Remark: 'm_clIndSelBVParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	// Set the text for static text 
	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		m_strKvsOrCv = TASApp.LoadLocalizedString( IDS_KVS );
	}
	else
	{
		m_strKvsOrCv = TASApp.LoadLocalizedString( IDS_CV );
	}

	m_clIndSelTRVParams.m_eInsertType = ( true == pclIndSelParameter->GetTrvSVInsertInKv() ) ? RadiatorInsertType::RIT_ByKv : RadiatorInsertType::RIT_Heimeier;

	// Remark: Corresponding combo variables in 'm_clIndSelTRVParams' are updated in each of this following methods.
	_FillComboRadiatorValveType( (RadiatorValveType)pclIndSelParameter->GetTrvRadiatorValveType() );
	_FillComboSVFamily( pclIndSelParameter->GetTrvSVFamilyID() );
	_FillComboSVConnect( pclIndSelParameter->GetTrvSVConnectID() );
	_FillComboSVVersion( pclIndSelParameter->GetTrvSVVersionID() );

	_FillComboRVFamily( pclIndSelParameter->GetTrvRVFamilyID() );

	m_clIndSelTRVParams.m_eReturnValveMode = pclIndSelParameter->GetTrvRVMode();

	// HYS-1305 : Fill combo RV connect and version
	_FillComboRVConnect( pclIndSelParameter->GetTrvRVConnectID() );
	_FillComboRVVersion( pclIndSelParameter->GetTrvRVVersionID() );

	// Remark: Corresponding combo variables in 'm_clIndSelTRVParams' are updated in each of this following methods.
	_FillComboSVInsertName( pclIndSelParameter->GetTrvSVInsertName() );
	_FillComboSVInsertFamily( pclIndSelParameter->GetTrvSVInsertFamilyID() );
	_FillComboSVInsertType( pclIndSelParameter->GetTrvSVInsertTypeID() );
	_FillComboRVInsertFamily( pclIndSelParameter->GetTrvRVInsertFamilyID() );

	m_iRadioThermoOrElectroActuator = ( 0 == pclIndSelParameter->GetTrvActuatorType() ) ? Radio_ElectroActuator : Radio_ThermostaticHead;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioThermoOrElectroActuator' variable.
	UpdateData( FALSE );

	// Remark: The 'm_clIndSelTRVParams.m_bIsThermostaticHead' is updated when calling the '_UpdateRadioThermoOrElectroActuator' method.
	_UpdateRadioThermoOrElectroActuator();

	_FillComboActuatorType( pclIndSelParameter->GetTrvActuatorTypeID() );
	_FillComboActuatorFamily( pclIndSelParameter->GetTrvActuatorFamilyID() );
	_FillComboActuatorVersion( pclIndSelParameter->GetTrvActuatorVersionID() );

	_UpdateLayout();

	m_clIndSelTRVParams.m_eInsertType = ( true == pclIndSelParameter->GetTrvSVInsertInKv() ) ? RadiatorInsertType::RIT_ByKv : RadiatorInsertType::RIT_Heimeier;
	m_clIndSelTRVParams.m_dInsertKvValue = pclIndSelParameter->GetTrvSVInsertKvValue();

	if( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType )
	{
		m_EditSVInsertKv.SetSel( 0, -1 );
		str1.Format( _T("%.1f"), m_clIndSelTRVParams.m_dInsertKvValue );
		m_EditSVInsertKv.ReplaceSel( str1 );
	}
	
	// Disable the Selection button.
	EnableSelectButton( false );
	
	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();

	// Set the Dp check box state.
	// Remark: The 'm_clIndSelTRVParams.m_bDpEnabled' variable will be updated by the '_UpdateGroupDp' method.
	m_clGroupDp.SetCheck( ( 0 == pclIndSelParameter->GetTrvDpCheckbox() ) ? BST_UNCHECKED : BST_CHECKED );
	_UpdateGroupDp();

	// Fill 'm_RadInfos'.
	pclIndSelParameter->GetTrvRadInfos( &m_clIndSelTRVParams.m_RadInfos );
	m_bInitialised = true;

	if( NULL != pRViewSSelTrv )
	{
		pRViewSSelTrv->OnNewDocument( pclIndSelParameter );
	}

	return 0;
}

LRESULT CDlgIndSelTrv::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnPipeChange( wParam, lParam );
	
	if( NULL != pRViewSSelTrv )
	{
		pRViewSSelTrv->Reset();
	}
	
	return 0;
}

LRESULT CDlgIndSelTrv::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelTrv )
	{
		pRViewSSelTrv->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelTrv::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnUnitChange( wParam, lParam );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	
	// Set the static text for edit Kvs (or Cv).
	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		m_strKvsOrCv = TASApp.LoadLocalizedString( IDS_KVS );
	}
	else
	{
		m_strKvsOrCv = TASApp.LoadLocalizedString( IDS_CV );
	}

	GetDlgItem( IDC_STATICKVS )->SetWindowText( m_strKvsOrCv );

	// Convert Kv value in CV.
	m_EditSVInsertKv.SetSel( 0, -1 );
	m_EditSVInsertKv.ReplaceSel( WriteCUDouble( _C_KVCVCOEFF, m_clIndSelTRVParams.m_dInsertKvValue, false, 3, 2 ) );
	_UpdateKvMin();
	
	// Relaunch the suggestion procedure
	if( NULL != pRViewSSelTrv && false == pRViewSSelTrv->IsEmpty() )
	{
		_LaunchSuggestion();
	}
	else
	{
		ClearAll();
	}

	return 0;
}

LRESULT CDlgIndSelTrv::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel )
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnWaterChange( wParam, lParam );

	if( NULL != pRViewSSelTrv )
	{
		pRViewSSelTrv->Reset();
	}

	return 0;
}

CRViewSSelSS *CDlgIndSelTrv::GetLinkedRightViewSSel( void )
{ 
	return pRViewSSelTrv;
}

void CDlgIndSelTrv::ClearAll()
{
	CDlgIndSelBase::ClearAll();
	memset( &m_clIndSelTRVParams.m_RadInfos, 0, sizeof( m_clIndSelTRVParams.m_RadInfos ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgIndSelTrv::_FillComboRadiatorValveType( RadiatorValveType eRadiatorValveType )
{
	m_ComboRadiatorValveType.ResetContent();
	m_ComboRadiatorValveType.AddString( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_VTSTANDARD ) );
	m_ComboRadiatorValveType.AddString( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_VTPRESET ) );
	m_ComboRadiatorValveType.AddString( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_VTFL ) );
	m_ComboRadiatorValveType.AddString( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_VTINSERT ) );
	m_ComboRadiatorValveType.SetCurSel( (int)eRadiatorValveType );
	m_clIndSelTRVParams.m_eValveType = eRadiatorValveType;
}

void CDlgIndSelTrv::_FillComboSVFamily( CString strSelItem )
{
	CRankEx FamList;

	// For insert, it's the '_FillComboSVInsertFamily' that is called.
	switch( m_clIndSelTRVParams.m_eValveType )
	{
		case RadiatorValveType::RVT_Standard:
			m_clIndSelTRVParams.m_pTADB->GetTrvFamilyList( &FamList, _T("TRVTYPE_NOPRSET"), m_clIndSelTRVParams.m_eFilterSelection );
			break;

		case RadiatorValveType::RVT_Presettable:
			m_clIndSelTRVParams.m_pTADB->GetTrvFamilyList( &FamList, _T("TRVTYPE_PRESET"), m_clIndSelTRVParams.m_eFilterSelection );
			break;

		case RadiatorValveType::RVT_WithFlowLimitation:
			m_clIndSelTRVParams.m_pTADB->GetFLCVFamilyList( &FamList, _T(""), m_clIndSelTRVParams.m_eFilterSelection );
			break;
	}

	// Remark: 0 to not have "All families..."
	// HYS-1776: We replace the 0 by GetComboFamilyAllID() to have All families.
	m_ComboSVFamily.FillInCombo( &FamList, strSelItem, m_clIndSelTRVParams.GetComboFamilyAllID() );
	m_clIndSelTRVParams.m_strComboSVFamilyID = m_ComboSVFamily.GetCBCurSelIDPtr().ID;

	if( _T('\0') != *m_clIndSelTRVParams.m_strComboSVFamilyID )
	{
		// There is no combo for the type but we use type to extract valves from the database.
		m_clIndSelTRVParams.m_strSVTypeID = ( (CDB_StringID*)m_clIndSelTRVParams.m_pTADB->Get( m_clIndSelTRVParams.m_strComboSVFamilyID ).MP )->GetIDstr2();
		m_clIndSelTRVParams.m_bIsTrvTypePreset = ( _T("TRVTYPE_PRESET") == m_clIndSelTRVParams.m_strSVTypeID ) ? true : false;
	}
	// HYS-1776: Now we have the possibility to choose *** All family ***. So when the valve type
    // is Presettable Trv we have to set m_strSVTypeID and m_bIsTrvTypePreset.
	else if( m_clIndSelTRVParams.m_eValveType == RadiatorValveType::RVT_Presettable )
	{
		m_clIndSelTRVParams.m_strSVTypeID = _T( "TRVTYPE_PRESET" );
		m_clIndSelTRVParams.m_bIsTrvTypePreset = true;
	}
	else
	{
		m_clIndSelTRVParams.m_strSVTypeID = _T("");
		m_clIndSelTRVParams.m_bIsTrvTypePreset = false;
	}
}

void CDlgIndSelTrv::_FillComboSVConnect( CString strSelItem )
{
	CRankEx ConnList;

	if( RadiatorValveType::RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
	{
		m_clIndSelTRVParams.m_pTADB->GetTrvConnectList( &ConnList, _T(""), m_ComboSVFamily.GetCBCurSelIDPtr().ID, m_clIndSelTRVParams.m_eFilterSelection );
	}
	else
	{
		m_clIndSelTRVParams.m_pTADB->GetFLCVConnectList( &ConnList, _T(""), m_ComboSVFamily.GetCBCurSelIDPtr().ID, m_clIndSelTRVParams.m_eFilterSelection );
	}
	
	m_ComboSVConnect.FillInCombo( &ConnList, strSelItem, m_clIndSelTRVParams.GetComboConnectAllID() );
	m_clIndSelTRVParams.m_strComboSVConnectID = m_ComboSVConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboSVVersion( CString strSelItem )
{
	CRankEx VerList;

	if( RadiatorValveType::RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
	{
		m_clIndSelTRVParams.m_pTADB->GetTrvVersList( &VerList, _T(""), m_ComboSVFamily.GetCBCurSelIDPtr().ID, m_ComboSVConnect.GetCBCurSelIDPtr().ID, 
				m_clIndSelTRVParams.m_eFilterSelection );
	}
	else
	{
		m_clIndSelTRVParams.m_pTADB->GetFLCVVersList( &VerList, _T(""), m_ComboSVFamily.GetCBCurSelIDPtr().ID, m_ComboSVConnect.GetCBCurSelIDPtr().ID, 
				m_clIndSelTRVParams.m_eFilterSelection );
	}

	m_ComboSVVersion.FillInCombo( &VerList, strSelItem, m_clIndSelTRVParams.GetComboVersionAllID() );
	m_clIndSelTRVParams.m_strComboSVVersionID = m_ComboSVVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboRVFamily( CString strSelItem )
{
	CRankEx FamList;
	m_clIndSelTRVParams.m_pTADB->GetReturnValveFamilyList( &FamList, m_clIndSelTRVParams.m_eFilterSelection );
	
	// Remark: 0 to not have "All families..."
	// HYS-1776: We replace the 0 by GetComboFamilyAllID() to have All families.
	m_ComboRVFamily.FillInCombo( &FamList, strSelItem, m_clIndSelTRVParams.GetComboFamilyAllID() );

	CTable *pTable = (CTable*)( m_clIndSelTRVParams.m_pTADB->Get( _T("CSTDATA_TAB") ).MP );

	if( NULL == pTable )
	{
		return;
	}
	
	// Adding "Nothing"
	CString str;
	CData *pData = (CData*)( pTable->Get( _T("NOTHING_ID") ).MP );
	
	if( NULL != pData )
	{
		str = TASApp.LoadLocalizedString( IDS_NOTHING );
		int iIndex = m_ComboRVFamily.AddString( str );
        m_ComboRVFamily.SetItemData( iIndex, (LPARAM)pData );
	}

	// We have to recall 'FindCBIDptr' to select corresponding item in combo because it can be 'Other kv' or 'Nothing'.
	if( false == strSelItem.IsEmpty() )
	{
		int iSelPos = max( m_ComboRVFamily.FindCBIDPtr( strSelItem ), 0 );
		m_ComboRVFamily.SetCurSel( iSelPos );
	}

	m_clIndSelTRVParams.m_strComboRVFamilyID = m_ComboRVFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboRVConnect( CString strSelItem )
{
	CRankEx ConnList;
	m_clIndSelTRVParams.m_pTADB->GetReturnValveConnectList( &ConnList, m_ComboRVFamily.GetCBCurSelIDPtr().ID, m_clIndSelTRVParams.m_eFilterSelection );
	m_ComboRVConnect.FillInCombo( &ConnList, strSelItem, m_clIndSelTRVParams.GetComboConnectAllID() );
	m_clIndSelTRVParams.m_strComboRVConnectID = m_ComboRVConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboRVVersion( CString strSelItem )
{
	CRankEx VerList;

	m_clIndSelTRVParams.m_pTADB->GetReturnValveVersList( &VerList, m_ComboRVFamily.GetCBCurSelIDPtr().ID, m_ComboRVConnect.GetCBCurSelIDPtr().ID, 
			m_clIndSelTRVParams.m_eFilterSelection );

	m_ComboRVVersion.FillInCombo( &VerList, strSelItem, m_clIndSelTRVParams.GetComboVersionAllID() );
	m_clIndSelTRVParams.m_strComboRVVersionID = m_ComboRVVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboSVInsertName( CString strSelItem )
{
	int nSelect = 0;

	m_ComboSVInsertName.ResetContent();

	// Adding "Heimeier insert".
	m_ComboSVInsertName.AddString( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_INSERTHEIMEIER ) );
	
	// Adding "Other insert".
	CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_OTHERINSERT );
	m_ComboSVInsertName.AddString( str );

	if( false == strSelItem.IsEmpty() && str == strSelItem )
	{
		nSelect = 1;
	}

	m_ComboSVInsertName.SetCurSel( nSelect );

	if( CB_ERR != m_ComboSVInsertName.GetCurSel() )
	{
		m_ComboSVInsertName.GetLBText( m_ComboSVInsertName.GetCurSel(), m_clIndSelTRVParams.m_strComboSVInsertName );
	}
}

void CDlgIndSelTrv::_FillComboSVInsertFamily( CString strSelItem )
{
	CRankEx FamList;
	m_clIndSelTRVParams.m_pTADB->GetTrvInsertFamilyList( &FamList, m_clIndSelTRVParams.m_eFilterSelection );
	// HYS-1305 : Add insert with automatic flow limiter
	m_clIndSelTRVParams.m_pTADB->GetFLCVInsertFamilyList( &FamList, m_clIndSelTRVParams.m_eFilterSelection );
	m_ComboSVInsertFamily.FillInCombo( &FamList, strSelItem );
	m_clIndSelTRVParams.m_strComboSVInsertFamilyID = m_ComboSVInsertFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboSVInsertType( CString strSelItem )
{
	CRankEx TypeList;
	// HYS-1467 : Add CVFunction to differentiate presettable and control only
	m_clIndSelTRVParams.m_pTADB->GetTrvInsertTypeList( &TypeList, m_ComboSVInsertFamily.GetCBCurSelIDPtr().ID, CDB_ControlProperties::LastCVFUNC, 
		m_clIndSelTRVParams.m_eFilterSelection );
	// HYS-1305 : Add insert with automatic flow limiter
	m_clIndSelTRVParams.m_pTADB->GetFLCVInsertTypeList( &TypeList, m_ComboSVInsertFamily.GetCBCurSelIDPtr().ID, m_clIndSelTRVParams.m_eFilterSelection );
	m_ComboSVInsertType.FillInCombo( &TypeList, strSelItem );
	m_clIndSelTRVParams.m_strComboSVInsertTypeID = m_ComboSVInsertType.GetCBCurSelIDPtr().ID;

	// HYS-1220: When we proceed OnNewDocument this function is called even if the vavle type is not insert.
	// Setting m_bIsTrvTypePreset when valve type is not insert is in _FillComboSVFamilly
	if( RVT_Inserts == m_clIndSelTRVParams.m_eValveType )
	{
		// HYS-1115: Presetable Trv insert 
		m_clIndSelTRVParams.m_bIsTrvTypePreset = ( _T("TRVTYPE_INSERT_PRESET") == m_clIndSelTRVParams.m_strComboSVInsertTypeID ) ? true : false;
	}
	// HYS-1305 : Add insert with automatic flow limiter. Set m_bIsFLCVInsert and GroupDp for insert.
	bool bShowDp = true;
	if( _T( "TRVTYPE_INSERT_FL" ) == m_clIndSelTRVParams.m_strComboSVInsertTypeID )
	{
		bShowDp = false;
		m_clIndSelTRVParams.m_bIsFLCVInsert = true;
	}
	else
	{
		m_clIndSelTRVParams.m_bIsFLCVInsert = false;
	}

	// Group Dp.
	_ShowControl( GetDlgItem( IDC_GROUPDP ), bShowDp );
	_ShowControl( GetDlgItem( IDC_EDITDP ), bShowDp );
	_ShowControl( GetDlgItem( IDC_STATICDPUNIT ), bShowDp );

	// HYS-1305 : With FLC type the checkbox must be updated
	if( false == bShowDp )
	{
		m_clGroupDp.SetCheck( BST_UNCHECKED );
		_UpdateGroupDp();
	}
}

void CDlgIndSelTrv::_FillComboRVInsertFamily( CString strSelItem )
{
	CRankEx FamList;
	m_clIndSelTRVParams.m_pTADB->GetRVInsertFamilyList( &FamList, m_clIndSelTRVParams.m_eFilterSelection );

	m_ComboRVInsertFamily.FillInCombo( &FamList, strSelItem, m_clIndSelTRVParams.GetComboFamilyAllID() );
	m_clIndSelTRVParams.m_strComboRVInsertFamilyID = m_ComboRVInsertFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboActuatorType( CString strSelItem )
{
	CRankEx TypeList;
	CTable *pTab = (CTable*)( m_clIndSelTRVParams.m_pTADB->Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Retrieve list of supply valve corresponding to current combo selection.
	CRankEx SVList;
	int iCount = 0;

	if( RadiatorValveType::RVT_Inserts == m_clIndSelTRVParams.m_eValveType && RadiatorInsertType::RIT_ByKv != m_clIndSelTRVParams.m_eInsertType )
	{
		// HYS-1305 : Add insert with automatic flow limiter
		if( false == m_clIndSelTRVParams.m_bIsFLCVInsert )
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetTrvInsertList( &SVList, _T( "" ), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVInsertFamilyID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID,
				(LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T( "" ), m_clIndSelTRVParams.m_eFilterSelection );
		}

		else
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetFLCVInsertList( &SVList, _T( "" ), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVInsertFamilyID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID,
				(LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T( "" ), m_clIndSelTRVParams.m_eFilterSelection );
		}
	}
	else
	{
		if( RadiatorValveType::RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetTrvList( &SVList, _T(""), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVFamilyID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, 
					(LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T(""), m_clIndSelTRVParams.m_eFilterSelection );
		}
		else
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetFLCVList( &SVList, _T(""), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVFamilyID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, 
					(LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T(""), m_clIndSelTRVParams.m_eFilterSelection );
		}
	}

	if( 0 == iCount )
	{
		return;
	}

	// First pass: run all supply valves and retrieve actuator groups.
	_string str;
	LPARAM lParam;
	std::map<int, int> mapActrKeyList;
	std::map<CTable*, int> mapActrGpList;

	for( BOOL bContinue = SVList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = SVList.GetNext( str, lParam ) )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( (CData*)lParam );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Retrieve actuator group.
		IDPTR ActrGpIDPtr = pclControlValve->GetActuatorGroupIDPtr();

		if( NULL == ActrGpIDPtr.MP || _T('\0') == *ActrGpIDPtr.ID )
		{
			continue;
		}

		CTable *pclActrTab = dynamic_cast<CTable*>( m_clIndSelTRVParams.m_pTADB->Get( ActrGpIDPtr.ID ).MP );

		if( NULL == pclActrTab )
		{
			continue;
		}

		if( mapActrGpList.count( pclActrTab ) > 0 )
		{
			continue;
		}

		mapActrGpList[pclActrTab] = 0;
	}

	// Second pass: run all actuator groups.
	for( std::map<CTable*, int>::iterator iter = mapActrGpList.begin(); iter != mapActrGpList.end(); iter++ )
	{
		CTable *pclActrTab = iter->first;

		// Run all actuators in this group.
		for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); _T('\0') != *ActrIDPtr.ID; ActrIDPtr = pclActrTab->GetNext() )
		{
			// Check in regards to current radio choice, if actuator is matching.
			CDB_Actuator *pActuator = NULL;

			if( Radio_ThermostaticHead == m_iRadioThermoOrElectroActuator )
			{
				CDB_ThermostaticActuator *pThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator*>( ActrIDPtr.MP );

				if( NULL != pThermostaticActuator )
				{
					pActuator = (CDB_Actuator *)pThermostaticActuator;
				}
			}
			else if( Radio_ElectroActuator == m_iRadioThermoOrElectroActuator )
			{
				CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator*>( ActrIDPtr.MP );

				if( NULL != pElectroActuator )
				{
					pActuator = (CDB_Actuator *)pElectroActuator;
				}
			}

			if( NULL == pActuator )
			{
				continue;
			}
		
			if( false == pActuator->IsSelectable( true ) )
			{
				continue;
			}

			CTable *pActTypeTab = (CTable*)( m_clIndSelTRVParams.m_pTADB->Get( _T("ACTTYPE_TAB") ).MP );

			if( NULL == pActTypeTab )
			{
				continue;
			}

			CDB_StringID *pActrType = dynamic_cast<CDB_StringID*>( pActTypeTab->Get( pActuator->GetTypeIDPtr().ID ).MP );

			if( NULL == pActrType )
			{
				continue;
			}

			int iKey = _ttoi( pActrType->GetIDstr() );

			if( 0 == mapActrKeyList.count(iKey) )
			{
				TypeList.Add( ( (CDB_StringID *)pActuator->GetTypeIDPtr().MP )->GetString(), iKey, (LPARAM)(void *)pActuator->GetTypeIDPtr().MP );
				mapActrKeyList[iKey] = iKey;
			}
		}
	}

	m_ComboActuatorType.FillInCombo( &TypeList, strSelItem, m_clIndSelTRVParams.GetComboTypeAllID() );
	m_clIndSelTRVParams.m_strComboActuatorTypeID = m_ComboActuatorType.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboActuatorFamily( CString strSelItem )
{
	CRankEx FamilyList;
	CTable *pTab = (CTable*)( m_clIndSelTRVParams.m_pTADB->Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Retrieve list of supply valve corresponding to current combo selection.
	CRankEx SVList;
	int iCount = 0;

	if( RadiatorValveType::RVT_Inserts == m_clIndSelTRVParams.m_eValveType && RadiatorInsertType::RIT_ByKv != m_clIndSelTRVParams.m_eInsertType )
	{
		// HYS-1305 : Add insert with automatic flow limiter
		if( false == m_clIndSelTRVParams.m_bIsFLCVInsert )
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetTrvInsertList( &SVList, _T( "" ), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVInsertFamilyID,
				(LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T( "" ),
				m_clIndSelTRVParams.m_eFilterSelection );
		}
		else
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetFLCVInsertList( &SVList, _T( "" ), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVInsertFamilyID,
				(LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T( "" ),
				m_clIndSelTRVParams.m_eFilterSelection );
		}
	}
	else
	{
		if( RadiatorValveType::RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetTrvList( &SVList, _T(""), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVFamilyID, 
					(LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T(""), 
					m_clIndSelTRVParams.m_eFilterSelection );
		}
		else
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetFLCVList( &SVList, _T(""), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVFamilyID, 
					(LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T(""), 
					m_clIndSelTRVParams.m_eFilterSelection );
		}
	}

	if( 0 == iCount )
	{
		return;
	}

	// First pass: run all supply valves and retrieve actuator groups.
	_string str;
	LPARAM lParam;
	std::map<int, int> mapActrKeyList;
	std::map<CTable*, int> mapActrGpList;
	
	for( BOOL bContinue = SVList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = SVList.GetNext( str, lParam ) )
	{
		// Check if it's well a thermostatic valve.
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( (CData*)lParam );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Retrieve actuator group.
		IDPTR ActrGpIDPtr = pclControlValve->GetActuatorGroupIDPtr();

		if( NULL == ActrGpIDPtr.MP || _T('\0') == *ActrGpIDPtr.ID )
		{
			continue;
		}

		CTable *pclActrTab = dynamic_cast<CTable*>( m_clIndSelTRVParams.m_pTADB->Get( ActrGpIDPtr.ID ).MP );

		if( NULL == pclActrTab )
		{
			continue;
		}

		if( mapActrGpList.count( pclActrTab ) > 0 )
		{
			continue;
		}

		mapActrGpList[pclActrTab] = 0;
	}

	// Second pass: run all actuator groups.
	_string SelActuatorTypeID = m_ComboActuatorType.GetCBCurSelIDPtr().ID;
	
	for( std::map<CTable*, int>::iterator iter = mapActrGpList.begin(); iter != mapActrGpList.end(); iter++ )
	{
		CTable *pclActrTab = iter->first;

		// Run all actuators in this group.
		for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); _T('\0') != *ActrIDPtr.ID; ActrIDPtr = pclActrTab->GetNext() )
		{
			// Check in regards to current radio choice, if actuator is matching.
			CDB_Actuator *pActuator = NULL;

			if( Radio_ThermostaticHead == m_iRadioThermoOrElectroActuator )
			{
				CDB_ThermostaticActuator *pThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator*>( ActrIDPtr.MP );

				if( NULL != pThermostaticActuator )
				{
					pActuator = (CDB_Actuator *)pThermostaticActuator;
				}
			}
			else if( Radio_ElectroActuator == m_iRadioThermoOrElectroActuator )
			{
				CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator*>( ActrIDPtr.MP );

				if( NULL != pElectroActuator )
				{
					pActuator = (CDB_Actuator *)pElectroActuator;
				}
			}

			if( NULL == pActuator )
			{
				continue;
			}
		
			if( false == pActuator->IsSelectable( true ) )
			{
				continue;
			}

			// Exclude actuators that have not the same type.
			if( false == SelActuatorTypeID.empty() && SelActuatorTypeID != pActuator->GetTypeIDPtr().ID )
			{
				continue;
			}

			CTable *pActFamilyTab = (CTable*)( m_clIndSelTRVParams.m_pTADB->Get( _T("ACTFAM_TAB") ).MP );

			if( NULL == pActFamilyTab )
			{
				continue;
			}

			CDB_StringID *pActrFamily = dynamic_cast<CDB_StringID*>( pActFamilyTab->Get( pActuator->GetFamilyIDPtr().ID ).MP );

			if( NULL == pActrFamily )
			{
				continue;
			}

			int iKey = _ttoi( pActrFamily->GetIDstr() );
			
			if( 0 == mapActrKeyList.count(iKey) )
			{
				FamilyList.Add( ( (CDB_StringID *)pActuator->GetFamilyIDPtr().MP )->GetString(), iKey, (LPARAM)(void *)pActuator->GetFamilyIDPtr().MP );
				mapActrKeyList[iKey] = iKey;
			}
		}
	}

	m_ComboActuatorFamily.FillInCombo( &FamilyList, strSelItem, m_clIndSelTRVParams.GetComboFamilyAllID() );
	m_clIndSelTRVParams.m_strComboActuatorFamilyID = m_ComboActuatorFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelTrv::_FillComboActuatorVersion( CString strSelItem )
{
	CRankEx VersionList;
	CTable *pTab = (CTable*)( m_clIndSelTRVParams.m_pTADB->Get( _T("ACTUATOR_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Retrieve list of supply valve corresponding to current combo selection.
	CRankEx SVList;
	int iCount = 0;

	if( RadiatorValveType::RVT_Inserts == m_clIndSelTRVParams.m_eValveType && RadiatorInsertType::RIT_ByKv != m_clIndSelTRVParams.m_eInsertType )
	{
		// HYS-1305 : Add insert with automatic flow limiter
		if( false == m_clIndSelTRVParams.m_bIsFLCVInsert )
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetTrvInsertList( &SVList, _T( "" ), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVInsertFamilyID,
				(LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T( "" ),
				m_clIndSelTRVParams.m_eFilterSelection );
		}
		else
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetFLCVInsertList( &SVList, _T( "" ), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVInsertFamilyID,
				(LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T( "" ),
				m_clIndSelTRVParams.m_eFilterSelection );
		}
	}
	else
	{
		if( RadiatorValveType::RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetTrvList( &SVList, _T(""), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVFamilyID, 
					(LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T(""), 
					m_clIndSelTRVParams.m_eFilterSelection );
		}
		else
		{
			iCount = m_clIndSelTRVParams.m_pTADB->GetFLCVList( &SVList, _T(""), (LPCTSTR)m_clIndSelTRVParams.m_strComboSVFamilyID, 
					(LPCTSTR)m_clIndSelTRVParams.m_strComboSVConnectID, (LPCTSTR)m_clIndSelTRVParams.m_strComboSVVersionID, _T(""), 
					m_clIndSelTRVParams.m_eFilterSelection );
		}
	}

	if( 0 == iCount )
	{
		return;
	}

	// First pass: run all supply valves and retrieve actuator groups.
	_string str;
	LPARAM lParam;
	std::map<int, int> mapActrKeyList;
	std::map<CTable*, int> mapActrGpList;
	
	for( BOOL bContinue = SVList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = SVList.GetNext( str, lParam ) )
	{
		// Check if it's well a thermostatic valve.
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( (CData*)lParam );

		if( NULL == pclControlValve )
		{
			continue;
		}

		// Retrieve actuator group.
		IDPTR ActrGpIDPtr = pclControlValve->GetActuatorGroupIDPtr();

		if( NULL == ActrGpIDPtr.MP || _T('\0') == *ActrGpIDPtr.ID )
		{
			continue;
		}

		CTable *pclActrTab = dynamic_cast<CTable*>( m_clIndSelTRVParams.m_pTADB->Get( ActrGpIDPtr.ID ).MP );

		if( NULL == pclActrTab )
		{
			continue;
		}

		if( mapActrGpList.count( pclActrTab ) > 0 )
		{
			continue;
		}

		mapActrGpList[pclActrTab] = 0;
	}

	// Second pass: run all actuator groups.
	_string SelActuatorTypeID = m_ComboActuatorType.GetCBCurSelIDPtr().ID;
	_string SelActuatorFamilyID = m_ComboActuatorFamily.GetCBCurSelIDPtr().ID;

	for( std::map<CTable*, int>::iterator iter = mapActrGpList.begin(); iter != mapActrGpList.end(); iter++ )
	{
		CTable *pclActrTab = iter->first;

		// Run all actuators in this group.
		for( IDPTR ActrIDPtr = pclActrTab->GetFirst(); _T('\0') != *ActrIDPtr.ID; ActrIDPtr = pclActrTab->GetNext() )
		{
			// Check in regards to current radio choice, if actuator is matching.
			CDB_Actuator *pActuator = NULL;

			if( Radio_ThermostaticHead == m_iRadioThermoOrElectroActuator )
			{
				CDB_ThermostaticActuator *pThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator*>( ActrIDPtr.MP );

				if( NULL != pThermostaticActuator )
				{
					pActuator = (CDB_Actuator *)pThermostaticActuator;
				}
			}
			else if( Radio_ElectroActuator == m_iRadioThermoOrElectroActuator )
			{
				CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator*>( ActrIDPtr.MP );

				if( NULL != pElectroActuator )
				{
					pActuator = (CDB_Actuator *)pElectroActuator;
				}
			}

			if( NULL == pActuator )
			{
				continue;
			}
		
			if( false == pActuator->IsSelectable( true ) )
			{
				continue;
			}

			// Exclude actuators that have not the same type or family
			if( false == SelActuatorTypeID.empty() && SelActuatorTypeID != pActuator->GetTypeIDPtr().ID )
			{
				continue;
			}

			if( false == SelActuatorFamilyID.empty() && SelActuatorFamilyID != pActuator->GetFamilyIDPtr().ID )
			{
				continue;
			}

			CTable *pActVersionTab = (CTable*)( m_clIndSelTRVParams.m_pTADB->Get( _T("ACTVERSION_TAB") ).MP );
			
			if( NULL == pActVersionTab )
			{
				continue;
			}

			CDB_StringID *pActrVersion = dynamic_cast<CDB_StringID*>( pActVersionTab->Get( pActuator->GetVersionIDPtr().ID ).MP );

			if( NULL == pActrVersion )
			{
				continue;
			}

			int iKey = _ttoi( pActrVersion->GetIDstr() );
			
			if( 0 == mapActrKeyList.count(iKey) )
			{
				VersionList.Add( ( (CDB_StringID *)pActuator->GetVersionIDPtr().MP )->GetString(), iKey, (LPARAM)(void *)pActuator->GetVersionIDPtr().MP );
				mapActrKeyList[iKey] = iKey;
			}
		}
	}

	m_ComboActuatorVersion.FillInCombo( &VersionList, strSelItem, m_clIndSelTRVParams.GetComboVersionAllID() );
	m_clIndSelTRVParams.m_strComboActuatorVersionID = m_ComboActuatorVersion.GetCBCurSelIDPtr().ID;
}

bool CDlgIndSelTrv::_VerifySubmittedValues()
{
	bool bValid = false;

	if( m_clIndSelTRVParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetpEditFlow()->SetFocus();
	}
	else
	{
		// Check if Dp is valid or not (or check box is checked but there is no value).
		if( CDlgIndSelBase::VDPFlag_DpError != VerifyDpValue() )
		{
			bValid = true;
		}
	}

	return bValid;
}

void CDlgIndSelTrv::_LaunchSuggestion()
{
	ClearAll();
	PostWMCommandToControl( GetpBtnSuggest() );
}

void CDlgIndSelTrv::_LaunchSuggestion( CString strFamilyID, CString strConnectID, CString strVersionID )
{
	// For the moment, the trying of alternative solution is done only for normal supply valve but not insert.
	_FillComboSVFamily( strFamilyID );
	_FillComboSVConnect( strConnectID );
	_FillComboSVVersion( strVersionID );

	PostWMCommandToControl( GetpBtnSuggest() );
}

void CDlgIndSelTrv::_GetSelSizeShift( int &iAbove, int &iBelow )
{
	iAbove = m_clIndSelTRVParams.m_pTADS->GetpTechParams()->GetSizeShiftAbove();
	iBelow = m_clIndSelTRVParams.m_pTADS->GetpTechParams()->GetSizeShiftBelow();
}

void CDlgIndSelTrv::_UpdateLayout()
{
	// Insert controls.
	bool bShowInsert = ( RadiatorValveType::RVT_Inserts == m_clIndSelTRVParams.m_eValveType ) ? true : false;
	// HYS-1305 : Add insert with automatic flow limiter
	bool bShowDp = true;
	if( ( RadiatorValveType::RVT_WithFlowLimitation == m_clIndSelTRVParams.m_eValveType )
		|| ( true == m_clIndSelTRVParams.m_bIsFLCVInsert ) )
	{
		bShowDp = false;
	}
	
	// Group Dp.
	_ShowControl( GetDlgItem( IDC_GROUPDP ), bShowDp );
	_ShowControl( GetDlgItem( IDC_EDITDP ), bShowDp );
	_ShowControl( GetDlgItem( IDC_STATICDPUNIT ), bShowDp );

	// HYS-1305 : With FLC type the checkbox must be updated
	if( false == bShowDp )
	{
		m_clGroupDp.SetCheck( BST_UNCHECKED );
		_UpdateGroupDp();
	}

	_ShowControl( GetDlgItem( IDC_STATICINSERTNAME ), bShowInsert );
	_ShowControl( GetDlgItem( IDC_COMBOSVINSERTNAME ), bShowInsert );

	_ShowControl( GetDlgItem( IDC_STATICINSERTFAMILY ), bShowInsert );
	_ShowControl( GetDlgItem( IDC_COMBOSVINSERTFAMILY ), bShowInsert );

	_ShowControl( GetDlgItem( IDC_STATICINSERTTYPE ), bShowInsert );
	_ShowControl( GetDlgItem( IDC_COMBOSVINSERTTYPE ), bShowInsert );

	_ShowControl( GetDlgItem( IDC_COMBORVINSERTFAMILY ), bShowInsert );

	// By default we hide insert Kvs.
	_ShowControl( GetDlgItem( IDC_STATICKVS ), false );
	_ShowControl( GetDlgItem( IDC_EDITKV ), false );

	// By default we show also actuators combos.
	_ShowControl( GetDlgItem( IDC_STATICWORKWITH ), true );
	_ShowControl( GetDlgItem( IDC_RADIOTHRMHEAD ), true );
	_ShowControl( GetDlgItem( IDC_RADIOELECACTR ), true );
	_ShowControl( GetDlgItem( IDC_GROUPTHRMH ), true );
	_ShowControl( GetDlgItem( IDC_STATICACTRTYPE ), true );
	_ShowControl( GetDlgItem( IDC_COMBOACTRTYPE ), true );
	_ShowControl( GetDlgItem( IDC_STATICACTRFAM ), true );
	_ShowControl( GetDlgItem( IDC_COMBOACTRFAM ), true );
	_ShowControl( GetDlgItem( IDC_STATICACTRVER ), true );
	_ShowControl( GetDlgItem( IDC_COMBOACTRVER ), true );

	if( true == bShowInsert )
	{
		if( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType )
		{
			_ShowControl( GetDlgItem( IDC_STATICINSERTFAMILY ), false );
			_ShowControl( GetDlgItem( IDC_COMBOSVINSERTFAMILY ), false );
			_ShowControl( GetDlgItem( IDC_STATICINSERTTYPE ), false );
			_ShowControl( GetDlgItem( IDC_COMBOSVINSERTTYPE ), false );
			_ShowControl( GetDlgItem( IDC_STATICKVS ), true );
			_ShowControl( GetDlgItem( IDC_EDITKV ), true );

			// It's the only case where you don't show actuator combos.
			_ShowControl( GetDlgItem( IDC_STATICWORKWITH ), false );
			_ShowControl( GetDlgItem( IDC_RADIOTHRMHEAD ), false );
			_ShowControl( GetDlgItem( IDC_RADIOELECACTR ), false );
			_ShowControl( GetDlgItem( IDC_GROUPTHRMH ), false );
			_ShowControl( GetDlgItem( IDC_STATICACTRTYPE ), false );
			_ShowControl( GetDlgItem( IDC_COMBOACTRTYPE ), false );
			_ShowControl( GetDlgItem( IDC_STATICACTRFAM ), false );
			_ShowControl( GetDlgItem( IDC_COMBOACTRFAM ), false );
			_ShowControl( GetDlgItem( IDC_STATICACTRVER ), false );
			_ShowControl( GetDlgItem( IDC_COMBOACTRVER ), false );
		}
		else
		{
			_ShowControl( GetDlgItem( IDC_STATICINSERTFAMILY ), true );
			_ShowControl( GetDlgItem( IDC_COMBOSVINSERTFAMILY ), true );
			_ShowControl( GetDlgItem( IDC_STATICINSERTTYPE ), true );
			_ShowControl( GetDlgItem( IDC_COMBOSVINSERTTYPE ), true );
			_ShowControl( GetDlgItem( IDC_STATICKVS ), false );
			_ShowControl( GetDlgItem( IDC_EDITKV ), false );
		}
	}

	// Other controls.
	int iShowOther = ( true == bShowInsert ) ? SW_HIDE : SW_SHOW;

	_ShowControl( GetDlgItem( IDC_STATICSUPNAME ), !bShowInsert );
	_ShowControl( GetDlgItem( IDC_COMBOSUPVALVE ), !bShowInsert );
	
	_ShowControl( GetDlgItem( IDC_STATICSUPCONNECT ), !bShowInsert );
	_ShowControl( GetDlgItem( IDC_COMBOSUPCONNECT ), !bShowInsert );
	
	_ShowControl( GetDlgItem( IDC_STATICSUPVER ), !bShowInsert );
	_ShowControl( GetDlgItem( IDC_COMBOSUPVER ), !bShowInsert );
	
	// Keep 'IDC_STATICRETNAME'.
	_ShowControl( GetDlgItem( IDC_COMBORETVALVE ), !bShowInsert );
	
	_ShowControl( GetDlgItem( IDC_STATICRETVER ), !bShowInsert );
	_ShowControl( GetDlgItem( IDC_COMBORETVER ), !bShowInsert );

	_ShowControl( GetDlgItem( IDC_STATICRETCONNECT ), !bShowInsert );
	_ShowControl( GetDlgItem( IDC_COMBORETCONNECT ), !bShowInsert );

	if( false == bShowInsert )
	{
		CString str;
	
		if( _T("") == m_clIndSelTRVParams.m_strComboRVFamilyID )
		{
			m_ComboRVVersion.ShowWindow( SW_HIDE );
			m_ComboRVConnect.ShowWindow( SW_HIDE );
		}
		else if( _T("NOTHING_ID") == m_clIndSelTRVParams.m_strComboRVFamilyID )
		{
			m_ComboRVVersion.ShowWindow( SW_HIDE );
			m_ComboRVConnect.ShowWindow( SW_HIDE );

			m_clIndSelTRVParams.m_eReturnValveMode = RadiatorReturnValveMode::RRVM_Nothing;
			GetDlgItem( IDC_STATICRETVER )->ShowWindow( SW_HIDE );
			GetDlgItem( IDC_STATICRETCONNECT )->ShowWindow( SW_HIDE );
		}
		else
		{
			m_clIndSelTRVParams.m_eReturnValveMode = RadiatorReturnValveMode::RRVM_IMI;
			
			str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICVERSION );
			GetDlgItem( IDC_STATICRETVER )->SetWindowText( str );
			GetDlgItem( IDC_STATICRETVER )->ShowWindow( SW_SHOW );
			
			str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_STATICCONNECT );
			GetDlgItem( IDC_STATICRETCONNECT )->SetWindowText( str );
			GetDlgItem( IDC_STATICRETCONNECT )->ShowWindow( SW_SHOW );
			
			m_ComboRVVersion.ShowWindow( SW_SHOW );	
			m_ComboRVConnect.ShowWindow( SW_SHOW );
		}
	}
	else
	{
		// HYS-1305 : Set m_clIndSelTRVParams.m_eReturnValveMode if with thermostatic insert. 
		// Before it takes init value or last defined value
		m_ComboRVVersion.ShowWindow( SW_HIDE );
		m_ComboRVConnect.ShowWindow( SW_HIDE );
		if( m_clIndSelTRVParams.m_eInsertType != RIT_ByKv )
		{
			m_clIndSelTRVParams.m_eReturnValveMode = RadiatorReturnValveMode::RRVM_IMI;
		}
		else
		{
			m_clIndSelTRVParams.m_eReturnValveMode = RadiatorReturnValveMode::RRVM_Other;
		}
	}
}

void CDlgIndSelTrv::_ShowControl( CWnd *pControl, bool bShow )
{
	if( NULL == pControl )
	{
		return;
	}
	
	if( true == bShow )
	{
		BOOL bEnable = !( WS_DISABLED == ( pControl->GetStyle() & WS_DISABLED ) );
		pControl->ShowWindow( SW_SHOW );
		pControl->EnableWindow( bEnable );
	}
	else
		pControl->ShowWindow( SW_HIDE );
}

void CDlgIndSelTrv::_UpdateKvMin( void )
{
	CString strKvTitle = m_strKvsOrCv;
	
	if( true == m_clIndSelTRVParams.m_bDpEnabled && m_clIndSelTRVParams.m_dFlow > 0.0 && m_clIndSelTRVParams.m_dDp > 0.0 )
	{
		strKvTitle += CString( _T(" ( ") );
		double dKvMin = CalcKv( m_clIndSelTRVParams.m_dFlow, m_clIndSelTRVParams.m_dDp, m_clIndSelTRVParams.m_WC.GetDens() );
		
		CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

		if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			strKvTitle += TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_KVMIN );
		}
		else
		{
			strKvTitle += TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELTRV_CVMIN );
		}

		strKvTitle += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, dKvMin, false, 3, 2 ) + CString( _T(" )") );
	}

	GetDlgItem( IDC_STATICKVS )->SetWindowText( strKvTitle );
}

void CDlgIndSelTrv::_DoSelection( SelectionOptimization eSelectOpt, int &iSupplyValveFound, int &iReturnValveFound, bool *pbSizeShiftProblem )
{
	if( NULL == pbSizeShiftProblem )
	{
		return;
	}

	iSupplyValveFound = 0;
	iReturnValveFound = 0;
	double dDpOnSupplyValve = 0.0;

	// Prepare some variables to facilitate code after.
	CString strSVFamily = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboSVFamilyID : m_clIndSelTRVParams.m_strComboSVInsertFamilyID;
	CString strSVType = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strSVTypeID : m_clIndSelTRVParams.m_strComboSVInsertTypeID;
	CString strSVConnect = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboSVConnectID : _T("");
	CString strSVVersion = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboSVVersionID : _T("");
	CString strSVPN = _T("");
	CString strRVFamily = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboRVFamilyID : m_clIndSelTRVParams.m_strComboRVInsertFamilyID;
	CString strRVType = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? _T("RVTYPE_RV") : _T("RVTYPE_RV_INSERT");
	CString strRVConnect = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboRVConnectID : _T("");
	CString strRVVersion = ( RadiatorValveType::RVT_Inserts != m_clIndSelTRVParams.m_eValveType ) ? m_clIndSelTRVParams.m_strComboRVVersionID : _T("");

	switch( eSelectOpt )
	{
		case Selection_NoOptimization:
			break;

		case Selection_Optimization1:
			strSVType = _T("");
			strSVConnect = _T("");
			strSVVersion = _T("");
			strRVConnect = _T("");
			strRVVersion = _T("");
			break;
	}

	bool bInsert = ( RadiatorValveType::RVT_Inserts == m_clIndSelTRVParams.m_eValveType ) ? true : false;
	// HYS-1002: The bInsertInKv has to be tested when we are in RVT_Inserts mode
	bool bInsertInKv = false;
	if( bInsert )
	{
		bInsertInKv = ( RadiatorInsertType::RIT_ByKv == m_clIndSelTRVParams.m_eInsertType ) ? true : false;
	}
	
	// Pre-select valves with SizeShift = 99, i.e. pre-select all diameters.
	if( true == m_clIndSelTRVParams.m_bIsTrvTypePreset )
	{
		// TRVTYPE_PRESET or TRVTYPE_INSERT_PRESET.

		// Determine the right Dp.
		if( true == bInsert || RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode )
		{
			iReturnValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectReturnValve( &m_clIndSelTRVParams, (LPCTSTR)strRVType, 
					(LPCTSTR)strRVFamily, (LPCTSTR)strRVConnect, (LPCTSTR)strRVVersion, -1.0, true );
			
			if( 0 != iReturnValveFound )
			{
				// Determine MaxDpRv and if there are different DpRv's.
				// Remark: the bigger DpRv leads to the lowest Dp for the Trv.
				CSelectList *pReturnValveList = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetReturnValveList();
				
				for( CSelectedBase *pclSelectedProduct = pReturnValveList->GetFirst<CSelectedBase>(); NULL != pclSelectedProduct; pclSelectedProduct = pReturnValveList->GetNext<CSelectedBase>() )
				{
					if( -DBL_MAX != m_dMaxDpRV )
					{
						if( m_dMaxDpRV != pclSelectedProduct->GetDp() )
						{
							m_bDiffDpRV = true;
							m_dMaxDpRV = max( m_dMaxDpRV, pclSelectedProduct->GetDp() );
						}
					}
					else
					{
						m_dMaxDpRV = pclSelectedProduct->GetDp();
					}
				}
			}

			dDpOnSupplyValve = ( true == m_clIndSelTRVParams.m_bDpEnabled ) ? m_clIndSelTRVParams.m_dDp - m_dMaxDpRV : 0.0;
		}
		else if( RadiatorReturnValveMode::RRVM_Nothing == m_clIndSelTRVParams.m_eReturnValveMode )
		{
			dDpOnSupplyValve = ( true == m_clIndSelTRVParams.m_bDpEnabled ) ? m_clIndSelTRVParams.m_dDp : 0.0;
		}

		m_clIndSelTRVParams.m_SupplyValveList.PurgeAll();
		
		// Pre-select thermostatic valve ONLY if we are not in 'Other insert' mode.
		
		if( !( true == bInsert && true == bInsertInKv ) )
		{
			m_clIndSelTRVParams.m_pTADB->GetTrvList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect, 
					(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

			bool bValidFound = false;

			if( ( true == m_bDiffDpRV && RadiatorReturnValveMode::RRVM_Nothing != m_clIndSelTRVParams.m_eReturnValveMode ) || false == m_clIndSelTRVParams.m_bDpEnabled )
			{
				// With 'fForceFullOpen' set to 'true'.
				iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectSupplyValve( &m_clIndSelTRVParams, dDpOnSupplyValve,
						bValidFound, pbSizeShiftProblem, true );
			}
			else
			{
				// With Dp to be adjusted as determined above from Rv pre-selection.
				iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectSupplyValve( &m_clIndSelTRVParams, dDpOnSupplyValve,
						bValidFound, pbSizeShiftProblem, false );
			}
		}
	}
	else
	{
		// TRVTYPE_NOPRSET, TRVTYPE_INSERT_NOPRESET, TRVTYPE_INSERT_FL or TRVTYPE_FLOWLIMITED.
			
		if( false == bInsertInKv )
		{
			// Pre-select thermostatic valves.
			m_clIndSelTRVParams.m_SupplyValveList.PurgeAll();

			// Select supply valve with 'fForceFullOpen' set to 'true'.
			bool bValidFound = false;
			dDpOnSupplyValve = ( true == m_clIndSelTRVParams.m_bDpEnabled ) ? m_clIndSelTRVParams.m_dDp : 0.0;
			// HYS-1305 : Add insert with automatic flow limiter and No preset trv insert
			if( false == bInsert )
			{
				if( RadiatorValveType::RVT_WithFlowLimitation != m_clIndSelTRVParams.m_eValveType )
				{
					m_clIndSelTRVParams.m_pTADB->GetTrvList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
						(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

					iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectSupplyValve( &m_clIndSelTRVParams, dDpOnSupplyValve,
						bValidFound, pbSizeShiftProblem, true );
				}
				else
				{
					m_clIndSelTRVParams.m_pTADB->GetFLCVList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
						(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

					iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectFlowLimitedControlValve( &m_clIndSelTRVParams,
						bValidFound, pbSizeShiftProblem, false );
				}
			}
			else
			{
				if( false == m_clIndSelTRVParams.m_bIsFLCVInsert )
				{
					m_clIndSelTRVParams.m_pTADB->GetTrvInsertList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
						(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

					iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectSupplyValve( &m_clIndSelTRVParams, dDpOnSupplyValve,
						bValidFound, pbSizeShiftProblem, true );
				}
				else
				{
					m_clIndSelTRVParams.m_pTADB->GetFLCVInsertList( &m_clIndSelTRVParams.m_SupplyValveList, (LPCTSTR)strSVType, (LPCTSTR)strSVFamily, (LPCTSTR)strSVConnect,
						(LPCTSTR)strSVVersion, (LPCTSTR)strSVPN, m_clIndSelTRVParams.m_eFilterSelection, 0, INT_MAX, NULL );

					iSupplyValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectFlowLimitedControlValve( &m_clIndSelTRVParams,
						bValidFound, pbSizeShiftProblem, false );
				}
			}
			m_dMaxDpSV = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetMaxDpSupplyValve();
			m_bDiffDpSV = m_clIndSelTRVParams.m_pclSelectSupplyValveList->GetDiffDpSV();

			if( iSupplyValveFound > 0 )
			{
				// Handle return valve according to selected mode.
				if( true == bInsert || RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode )
				{
					// If user wants a pressure drop that is below pressure drop on supply valve, we force full opening on return valve.
					if( true == m_bDiffDpSV || false == m_clIndSelTRVParams.m_bDpEnabled || m_clIndSelTRVParams.m_dDp <= m_dMaxDpSV )
					{
						// With 'fForceFullOpen' set to 'true'.
						iReturnValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectReturnValve( &m_clIndSelTRVParams, (LPCTSTR)strRVType,
								(LPCTSTR)strRVFamily,  (LPCTSTR)strRVConnect, (LPCTSTR)strRVVersion, -1.0, true );
					}
					else
					{
						// With Dp to be adjusted as determined above from Trv pre-selection
						double dDpOnReturnValve = m_clIndSelTRVParams.m_dDp - m_dMaxDpSV;

						iReturnValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectReturnValve( &m_clIndSelTRVParams, (LPCTSTR)strRVType,
								(LPCTSTR)strRVFamily, (LPCTSTR)strRVConnect, (LPCTSTR)strRVVersion, dDpOnReturnValve, false );
					}
				}
			}
		}
		else
		{
			// Here user inputs Kv for the supply insert. Thus, thanks to flow we can deduce Dp on this insert. The Dp here is only on the insert and is not
			// the same as 'm_SelDp' (that is the Dp on both supply and return valves that users input).
			m_bDiffDpSV = false;
			m_dMaxDpSV = CalcDp( m_clIndSelTRVParams.m_dFlow, m_clIndSelTRVParams.m_dInsertKvValue, m_clIndSelTRVParams.m_WC.GetDens() );

			// Handle return valve according to selected mode.
			if( true == bInsert || RadiatorReturnValveMode::RRVM_IMI == m_clIndSelTRVParams.m_eReturnValveMode )
			{
				// With 'fForceFullOpen' set to 'true'.
				iReturnValveFound = m_clIndSelTRVParams.m_pclSelectSupplyValveList->SelectReturnValve( &m_clIndSelTRVParams, (LPCTSTR)strRVType, 
						(LPCTSTR)strRVFamily, (LPCTSTR)strRVConnect, (LPCTSTR)strRVVersion, -1.0, true );
			}
		}
	}
}

void CDlgIndSelTrv::_UpdateGroupDp()
{
	m_clIndSelTRVParams.m_bDpEnabled = ( BST_CHECKED == m_clGroupDp.GetCheck() ) ? true : false;
	CDlgIndSelBase::UpdateDpFieldState();

	// Special case for insert used with Kv instead of IMI product.
	_UpdateKvMin();
}

void CDlgIndSelTrv::_UpdateRadioThermoOrElectroActuator()
{
	// 'TRUE' to fill 'm_iRadioThermoOrElectroActuator' variable with the radio state in the dialog.
	UpdateData( TRUE );

	m_clIndSelTRVParams.m_bIsThermostaticHead = ( Radio_ThermostaticHead == m_iRadioThermoOrElectroActuator ) ? true : false;
}
