#include "stdafx.h"
#include "afxctl.h"
#include "TASelect.h"
#include "global.h"
#include "utilities.h"
#include "database.h"
#include "datastruct.h"
#include "wizard.h"
#include "DlgWizCircuit.h"
#include ".\panelcirc1.h"

CPanelCirc1::CPanelCirc1( CWnd *pParent )
	: CDlgWizard( CPanelCirc1::IDD, pParent )
{
	m_bEditDoNext = false;
	m_bOnActivateInCourse = false;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pInitialTab = NULL;
	m_pHM = NULL;
	m_pFPSheet = NULL;
	m_nTimer = (UINT_PTR)0;
}

CPanelCirc1::~CPanelCirc1()
{
	if( NULL != m_pFPSheet)
	{
		delete m_pFPSheet;
		m_pFPSheet = NULL;
	}
}

CDB_CircuitScheme::eBALTYPE CPanelCirc1::GetBalancingType()
{
	CDB_CircuitScheme::eBALTYPE eReturn = CDB_CircuitScheme::eBALTYPE::LastBALTYPE;

	// HYS-1674: Add Electronic balancing type
	if( BST_CHECKED == m_RadioBalancingTypeNone.GetCheck() )
	{
		eReturn = CDB_CircuitScheme::eBALTYPE::SP;
	}
	else if( BST_CHECKED == m_RadioBalancingTypeManual.GetCheck() )
	{
		eReturn = CDB_CircuitScheme::eBALTYPE::BV;
	}
	else if( BST_CHECKED == m_RadioBalancingTypeDiffPress.GetCheck() )
	{
		eReturn = CDB_CircuitScheme::eBALTYPE::DPC;
	}
	else if( BST_CHECKED == m_RadioBalancingTypeElectronic.GetCheck() )
	{
		eReturn = CDB_CircuitScheme::eBALTYPE::ELECTRONIC;
	}
	else if( BST_CHECKED == m_RadioBalancingTypeElectronicDpC.GetCheck() )
	{
		eReturn = CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC;
	}

	return eReturn;
}

bool CPanelCirc1::IsDirectReturn()
{
	return ( BST_CHECKED == m_RadioReturnTypeDirect.GetCheck() ) ? true : false;
}

bool CPanelCirc1::IsCvOnOff()
{
	return ( BST_CHECKED == m_RadioControlTypeOnOFF.GetCheck() ) ? true : false;
}

CDB_ControlProperties::CvCtrlType CPanelCirc1::GetControlType()
{
	CDB_ControlProperties::CvCtrlType eReturn = CDB_ControlProperties::eLastCvCtrlType;

	if( BST_CHECKED == m_RadioControlTypeOnOFF.GetCheck() )
	{
		eReturn = CDB_ControlProperties::eCvOnOff;
	}
	else if( BST_CHECKED == m_RadioControlType3Points.GetCheck() )
	{
		eReturn = CDB_ControlProperties::eCv3point;
	}
	else if( BST_CHECKED == m_RadioControlTypeProportional.GetCheck() )
	{
		eReturn = CDB_ControlProperties::eCvProportional;
	}
	else if( BST_CHECKED == m_RadioControlTypeNone.GetCheck() )
	{
		eReturn = CDB_ControlProperties::eCvNU;
	}
	
	return eReturn;
}

bool CPanelCirc1::IsTACv()
{
	return ( BST_CHECKED == m_RadioWorkForControlValvesKvsValues.GetCheck() ) ? false : true;
}

void CPanelCirc1::EnableButtonNext()
{
	( (CDlgWizCircuit *)GetWizMan() )->EnableButtons( CWizardManager::WizButNext );
}

BEGIN_MESSAGE_MAP( CPanelCirc1, CDlgWizard )
	ON_BN_CLICKED( IDC_RADIOBTNONE, OnClickedRadioBalancingTypeNoBalance )
	ON_BN_CLICKED( IDC_RADIOBTMANUAL, OnClickedRadioBalancingTypeManualBalancing )
	ON_BN_CLICKED( IDC_RADIOBTDIFFPRESS, OnClickedRadioBalancingTypeDiffPressController )
	ON_BN_CLICKED( IDC_RADIOBTELEC, OnClickedRadioBalancingTypeElectronicBalancing )
	ON_BN_CLICKED( IDC_RADIOBTELECTRONICDIFFPC, OnClickedRadioBalancingTypeElectronicDpC )
	ON_BN_CLICKED( IDC_RADIOTUNONE, OnClickedRadioTUNone )
	ON_BN_CLICKED( IDC_RADIOTUTERMINALUNIT, OnClickedRadioTUTerminalUnit )
	ON_BN_CLICKED( IDC_RADIOTURADIATOR, OnClickedRadioTURadiator )
	ON_BN_CLICKED( IDC_RADIODIRECTRETURN, OnClickedRadioReturnTypeDirect )
	ON_BN_CLICKED( IDC_RADIOREVERSERETURN, OnClickedRadioReturnTypeReverse )
	ON_BN_CLICKED( IDC_RADIOWFCVKV, OnClickedRadioWorkWithControlValvesKvsValues )
	ON_BN_CLICKED( IDC_RADIOWFCVIMI, OnClickedRadioWorkWithControlValvesIMIValves )
	ON_BN_CLICKED( IDC_RADIOCTNONE, OnClickedRadioControlTypeNone )
	ON_BN_CLICKED( IDC_RADIOCTPROP, OnClickedRadioControlTypeProportional )
	ON_BN_CLICKED( IDC_RADIOCTONOFF, OnClickedRadioControlTypeOnOff )
	ON_BN_CLICKED( IDC_RADIOCT3POINTS, OnClickedRadioControlType3Pts )
	ON_BN_CLICKED( IDC_RADIOCVTSTANDARD, OnClickedRadioControlValveTypeStandard )
	ON_BN_CLICKED( IDC_RADIOCVTADJUSTABLE, OnClickedRadioControlValveTypeAdjustable )
	ON_BN_CLICKED( IDC_RADIOCVTADJUSTABLEMEAS, OnClickedRadioControlValveTypeAdjustableAndMeasurable )
	ON_BN_CLICKED( IDC_RADIOCVTPI, OnClickedRadioControlValveTypePressureIndependent )
	ON_BN_CLICKED( IDC_RADIOCVTSMART, OnClickedRadioControlValveTypeSmart )
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CPanelCirc1::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizard::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_RADIOBTNONE, m_RadioBalancingTypeNone );
	DDX_Control( pDX, IDC_RADIOBTMANUAL, m_RadioBalancingTypeManual );
	DDX_Control( pDX, IDC_RADIOBTDIFFPRESS, m_RadioBalancingTypeDiffPress );
	DDX_Control( pDX, IDC_RADIOBTELEC, m_RadioBalancingTypeElectronic );
	DDX_Control( pDX, IDC_RADIOBTELECTRONICDIFFPC, m_RadioBalancingTypeElectronicDpC );
	DDX_Control( pDX, IDC_RADIOTUNONE, m_RadioTUNone );
	DDX_Control( pDX, IDC_RADIOTUTERMINALUNIT, m_RadioTUTerminalUnit );
	DDX_Control( pDX, IDC_RADIOTURADIATOR, m_RadioTURadiator );
	DDX_Control( pDX, IDC_RADIODIRECTRETURN, m_RadioReturnTypeDirect );
	DDX_Control( pDX, IDC_RADIOREVERSERETURN, m_RadioReturnTypeReverse );
	DDX_Control( pDX, IDC_RADIOWFCVKV, m_RadioWorkForControlValvesKvsValues );
	DDX_Control( pDX, IDC_RADIOWFCVIMI, m_RadioWorkForControlValvesIMIValves );
	DDX_Control( pDX, IDC_RADIOCTNONE, m_RadioControlTypeNone );
	DDX_Control( pDX, IDC_RADIOCTPROP, m_RadioControlTypeProportional );
	DDX_Control( pDX, IDC_RADIOCTONOFF, m_RadioControlTypeOnOFF );
	DDX_Control( pDX, IDC_RADIOCT3POINTS, m_RadioControlType3Points );
	DDX_Control( pDX, IDC_RADIOCVTSTANDARD, m_RadioControlValveTypeStandard );
	DDX_Control( pDX, IDC_RADIOCVTADJUSTABLE, m_RadioControlValveTypeAdjustable );
	DDX_Control( pDX, IDC_RADIOCVTADJUSTABLEMEAS, m_RadioControlValveTypeAdjustableMeasurable );
	DDX_Control( pDX, IDC_RADIOCVTPI, m_RadioControlValveTypePressureIndependent );
	DDX_Control( pDX, IDC_RADIOCVTSMART, m_RadioControlValveTypeSmart );
	DDX_Control( pDX, IDC_STATICRETURNTYPE, m_GroupReturnType );
	DDX_Control( pDX, IDC_STATICWFCV, m_GroupWorkForControlValves );
	DDX_Control( pDX, IDC_STATICCTRLTYPE, m_GroupControlType );
	DDX_Control( pDX, IDC_STATICCVTYPE, m_GroupControlValveType );
	DDX_Control( pDX, IDC_STATICTERMINALUNIT, m_GroupTerminalUnit );
	DDX_Control( pDX, IDC_STATICBLTYPE, m_GroupBalancingType );
}

BOOL CPanelCirc1::OnInitDialog()
{
	CDlgWizard::OnInitDialog();
	m_pFPSheet = new CSSheetPanelCirc1();
	ASSERT( NULL != m_pFPSheet );

	if( NULL != m_pFPSheet )
	{
		CRect rect( 50, 50, 100, 100 );

		if( FALSE == m_pFPSheet->Create( ( GetStyle() | WS_CHILD | WS_VSCROLL ) ^ WS_VISIBLE ^ WS_BORDER, rect, this, IDC_FPSPREAD ) )
		{
			return false;
		}
	}

	// Attach Panel to CSetDPI, in case of large font, prevent growing of dialog box.
	dpi.Attach( AfxFindResourceHandle( MAKEINTRESOURCE( IDD ), RT_DIALOG ), m_hWnd, IDD, 96.0 ); // 96 is the DPI
	
	// Added to force focus on Spread sheet.
	m_nTimer = SetTimer( _TIMERID_PANELCIRC1, 200, 0 );
	return TRUE;
}

void CPanelCirc1::OnClickedRadioBalancingTypeNoBalance()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	m_RadioBalancingTypeNone.SetCheck( BST_CHECKED );
	m_RadioBalancingTypeManual.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioBalancingTypeManualBalancing()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	m_RadioBalancingTypeManual.SetCheck( BST_CHECKED );
	m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioBalancingTypeDiffPressController()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	m_RadioBalancingTypeDiffPress.SetCheck( BST_CHECKED );
	m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeManual.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioBalancingTypeElectronicBalancing()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	m_RadioBalancingTypeElectronic.SetCheck( BST_CHECKED );
	m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeManual.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioBalancingTypeElectronicDpC()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	m_RadioBalancingTypeElectronicDpC.SetCheck( BST_CHECKED );
	m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeManual.SetCheck( BST_UNCHECKED );
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioTUNone()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	m_RadioTUNone.SetCheck( BST_CHECKED );
	m_RadioTUTerminalUnit.SetCheck( BST_UNCHECKED );
	m_RadioTURadiator.SetCheck( BST_UNCHECKED );
	InitSpreadSheet();
	
	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioTUTerminalUnit()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	m_RadioTUTerminalUnit.SetCheck( BST_CHECKED );
	m_RadioTURadiator.SetCheck( BST_UNCHECKED );
	m_RadioTUNone.SetCheck( BST_UNCHECKED );
	InitSpreadSheet();
	
	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioTURadiator()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	m_RadioTURadiator.SetCheck( BST_CHECKED );
	m_RadioTUTerminalUnit.SetCheck( BST_UNCHECKED );
	m_RadioTUNone.SetCheck( BST_UNCHECKED );
	InitSpreadSheet();
	
	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioReturnTypeDirect()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioReturnTypeDirect();
	InitSpreadSheet();
	
	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioReturnTypeReverse()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioReturnTypeReverse();
	InitSpreadSheet();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioWorkWithControlValvesKvsValues()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioWorkWithControlValvesKvsValues();
	
	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioWorkWithControlValvesIMIValves()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioWorkWithControlValvesIMIValves();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlTypeNone()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlTypeNone();
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlTypeProportional()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlTypeProportional();
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlTypeOnOff()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlTypeOnOff();
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlType3Pts()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlType3points();
	UpdateCVTypeGroup();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlValveTypeStandard()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlValveTypeStandard();
	InitSpreadSheet();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlValveTypeAdjustable()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlValveTypeAdjustable();
	InitSpreadSheet();
	
	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlValveTypeAdjustableAndMeasurable()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlValveTypeAdjustableAndMeasurable();
	InitSpreadSheet();
	
	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlValveTypePressureIndependent()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlValveTypePressureIndependent();
	InitSpreadSheet();
	
	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnClickedRadioControlValveTypeSmart()
{
	BeginWaitCursor();
	SetRedraw( FALSE );

	ClickRadioControlValveTypeSmart();
	InitSpreadSheet();

	EndWaitCursor();
	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CPanelCirc1::OnTimer( UINT_PTR nIDEvent )
{
	CDlgWizard::OnTimer( nIDEvent );

	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
	{
		return;
	}

	if( false == ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() )
	{
		m_pFPSheet->SetFocus();
	}

	KillTimer( nIDEvent );
	m_nTimer = (UINT_PTR)0;
}

BOOL CPanelCirc1::OnWizNext()
{
	// Save CV parameters.
	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVMode( ( BST_CHECKED == m_RadioWorkForControlValvesKvsValues.GetCheck() ) ? CPrjParams::_CVMode::KvMode : CPrjParams::_CVMode::TAMode );
	pHMGenParam->SetCVCtrlType( GetControlType() );

	CDB_ControlProperties::eCVFUNC CVFunc = CDB_ControlProperties::eCVFUNC::NoControl;

	// We verify check status the group is enabled.
	if( CDB_ControlProperties::CvCtrlType::eCvNU != GetControlType() )
	{
		if( BST_CHECKED == m_RadioControlValveTypeStandard.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC::ControlOnly;
		}
		
		if( BST_CHECKED == m_RadioControlValveTypeAdjustable.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC::Presettable;
		}
		
		if( BST_CHECKED == m_RadioControlValveTypeAdjustableMeasurable.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC::PresetPT;
		}
		
		if( BST_CHECKED == m_RadioControlValveTypePressureIndependent.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC( CDB_ControlProperties::eCVFUNC::Presettable | CDB_ControlProperties::eCVFUNC::PresetPT );
		}

		if( BST_CHECKED == m_RadioControlValveTypeSmart.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC::Electronic;
		}
	}
	
	pHMGenParam->SetCVType( CVFunc );
	return TRUE;
}

BOOL CPanelCirc1::OnWizFinish()
{
	// In non Edit mode remove created HM.
	if( false == ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() )
	{
		if( ( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc2.IsHMExist() )
		{
			( (CDlgWizCircuit *)GetWizMan() )->m_PanelCirc2.DeleteCircuit();
		}
	}
	
	return TRUE;
}

bool CPanelCirc1::OnInitialActivate()
{
	m_bOnActivateInCourse = true;

	// Retrieve current position of the separator.
	CRect rectSeparator;
	GetDlgItem( IDC_SEPARATORUP )->GetWindowRect( &rectSeparator );
	ScreenToClient( &rectSeparator );

	// Retrieve current dialog client area.
	CRect rectDialog;
	GetClientRect( &rectDialog );

	// Remark: -2 to let a space between top separator and spread.
	m_pFPSheet->SetWindowPos( NULL, rectDialog.left, rectSeparator.bottom, rectDialog.Width(), rectDialog.Height() - rectSeparator.bottom - 2, SWP_NOZORDER );

	// Static string initialization.
	CString str;

	// 'Balancing type' group
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_BALANCINGTYPE );
	m_GroupBalancingType.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_BT_NONE );
	m_RadioBalancingTypeNone.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_BT_MANUAL );
	m_RadioBalancingTypeManual.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_BT_DIFFPRESS );
	m_RadioBalancingTypeDiffPress.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_BT_ELECTRONIC );
	m_RadioBalancingTypeElectronic.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_BT_ELECTRONICDPC );
	m_RadioBalancingTypeElectronicDpC.SetWindowText( str );
	
	// 'Terminal unit' group.
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_TERMUNIT );
	m_GroupTerminalUnit.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_TU_NONE );
	m_RadioTUNone.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_TU_TU );
	m_RadioTUTerminalUnit.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_TU_RAD );
	m_RadioTURadiator.SetWindowText( str );

	// 'Return type' group.
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICDIRECTRETURN );
	m_RadioReturnTypeDirect.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICREVERSERETURN );
	m_RadioReturnTypeReverse.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_GROUPRETURNTYPE );
	m_GroupReturnType.SetWindowText( str );

	// 'Work for control valves' group.
	if ( !(CDimValue::AccessUDB() )->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCVKVVALUES );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCVCVVALUES );
	}

	m_RadioWorkForControlValvesKvsValues.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCVTAVALVES );
	m_RadioWorkForControlValvesIMIValves.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCV );
	m_GroupWorkForControlValves.SetWindowText( str );
	m_GroupWorkForControlValves.SaveChildList();

	// 'Control type' group.
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCVCTNONE );
	m_RadioControlTypeNone.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCVCTONOFF );
	m_RadioControlTypeOnOFF.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCVCTPROP );
	m_RadioControlTypeProportional.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCVCT3PTS );
	m_RadioControlType3Points.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_STATICCVCT );
	m_GroupControlType.SetWindowText( str );

	// 'Control valve type' group.
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_RADIOCTRL );
	m_RadioControlValveTypeStandard.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_RADIOCVTADJUSTABLE );
	m_RadioControlValveTypeAdjustable.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_RADIOCVTADJUSTABLEMEAS );
	m_RadioControlValveTypeAdjustableMeasurable.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_RADIOCVTPI );
	m_RadioControlValveTypePressureIndependent.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_RADIOCVTSMART );
	m_RadioControlValveTypeSmart.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGPANELCIRC1_GROUPCVTYPE );
	m_GroupControlValveType.SetWindowText( str );
	m_GroupControlValveType.SaveChildList();

	// Hide 'Radiator' radio button.
	m_RadioTURadiator.SetCheck( BST_UNCHECKED );
	m_RadioTURadiator.EnableWindow( FALSE );
	m_RadioTURadiator.ShowWindow( SW_HIDE );

	// Members variables.
	m_pInitialTab = ( (CDlgWizCircuit *)GetWizMan() )->GetpInitialTab();
	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();

	CDS_ProjectParams *pProjectParams = m_pTADS->GetpProjectParams();

	if( CPrjParams::_CVMode::KvMode == pProjectParams->GetpHmCalcParams()->GetCVMode() )
	{
		ClickRadioWorkWithControlValvesKvsValues();
	}
	else
	{
		ClickRadioWorkWithControlValvesIMIValves();
	}

	if( CDB_ControlProperties::CvCtrlType::eCvOnOff == pProjectParams->GetpHmCalcParams()->GetCVCtrlType() )
	{
		ClickRadioControlTypeOnOff();
	}
	else if( CDB_ControlProperties::CvCtrlType::eCv3point == pProjectParams->GetpHmCalcParams()->GetCVCtrlType() )
	{
		ClickRadioControlType3points();
	}
	else if( CDB_ControlProperties::CvCtrlType::eCvProportional == pProjectParams->GetpHmCalcParams()->GetCVCtrlType() )
	{
		ClickRadioControlTypeProportional();
	}
	else
	{
		ClickRadioControlTypeNone();
	}

	// Balancing type Group initialization.
	m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeManual.SetCheck( BST_CHECKED );
	m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
	m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );

	// We have to check by default control valve type standard
	ClickRadioControlValveTypeStandard();

	// Disable 'Next' button Until selections has be done.
	( (CDlgWizCircuit *)GetWizMan() )->DisableButtons( CWizardManager::WizButNext );
	
	// Keep a trace of modified flag.
	( (CDlgWizCircuit *)GetWizMan() )->m_bModified = ( 1 == m_pTADS->IsModified() ) ? true : false;
	
	// Used to force a DoNext when the user open wizard in Edit Mode.
	m_bEditDoNext = ( ( (CDlgWizCircuit *)GetWizMan() )->IsModeEdit() ) ? true : false;

	return OnActivate();
}

bool CPanelCirc1::OnActivate()
{
	m_bOnActivateInCourse = true;
	m_pHM = ( (CDlgWizCircuit *)GetWizMan() )->GetpCurrentHM();

	BeginWaitCursor();
	SetRedraw( FALSE );

	// Specific initialization in Edit mode: retrieve selected scheme...
	if( NULL != m_pHM )
	{
		VerifyRadiosReturnType();
		VerifyTerminalUnitGroup();
				
		CDB_CircuitScheme *pSch = (CDB_CircuitScheme *)( m_pHM->GetSchemeIDPtr().MP );
		ASSERT( NULL != pSch );

		if( true == m_pHM->IsCvExist() )
		{
			( true == m_pHM->GetpCV()->IsTaCV() ) ? ClickRadioWorkWithControlValvesIMIValves() : ClickRadioWorkWithControlValvesKvsValues();

			if( CDB_ControlProperties::eCvOnOff == m_pHM->GetpCV()->GetCtrlType() )
			{
				ClickRadioControlTypeOnOff();
			}
			else if( CDB_ControlProperties::eCv3point == m_pHM->GetpCV()->GetCtrlType() )
			{
				ClickRadioControlType3points();
			}
			else
			{
				ClickRadioControlTypeProportional();
			}
		}
		else if( true == m_pHM->IsSmartControlValveExist() )
		{
			ClickRadioControlTypeProportional();
		}
		else if( true == m_pHM->IsSmartDpCExist() )
		{
			ClickRadioControlTypeNone();
		}
		else
		{
			ClickRadioControlTypeNone();
		}
		
		// Select balancing type.
		if( CDB_CircuitScheme::eBALTYPE::SP == pSch->GetBalType() )
		{
			m_RadioBalancingTypeManual.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeNone.SetCheck( BST_CHECKED );
			m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
		}
		else if( CDB_CircuitScheme::eBALTYPE::BV == pSch->GetBalType() )
		{
			m_RadioBalancingTypeManual.SetCheck( BST_CHECKED );
			m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
		}
		else if( CDB_CircuitScheme::eBALTYPE::DPC == pSch->GetBalType() )
		{
			m_RadioBalancingTypeManual.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeDiffPress.SetCheck( BST_CHECKED );
			m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
		}
		else if( CDB_CircuitScheme::eBALTYPE::ELECTRONIC == pSch->GetBalType() )
		{
			m_RadioBalancingTypeManual.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeElectronic.SetCheck( BST_CHECKED );
			m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
		}
		else if( CDB_CircuitScheme::eBALTYPE::ELECTRONICDPC == pSch->GetBalType() )
		{
			m_RadioBalancingTypeManual.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
			m_RadioBalancingTypeElectronicDpC.SetCheck( BST_CHECKED );
		}

		if( CDB_ControlProperties::eCVFUNC::Presettable == pSch->GetCvFunc() 
				|| CDB_ControlProperties::eCVFUNC::PresetPT == pSch->GetCvFunc() )
		{
			if( CDB_CircuitScheme::eDpCTypePICV == pSch->GetDpCType() )
			{
				ClickRadioControlValveTypePressureIndependent();
			}
			else if( CDB_ControlProperties::eCVFUNC::Presettable == pSch->GetCvFunc() )
			{
				ClickRadioControlValveTypeAdjustable();
			}
			else
			{
				ClickRadioControlValveTypeAdjustableAndMeasurable();
			}
		}
		else if( CDB_ControlProperties::eCVFUNC::Electronic == pSch->GetCvFunc() )
		{
			ClickRadioControlValveTypeSmart();
		}
		else
		{
			ClickRadioControlValveTypeStandard();
		}
	}
	else
	{
		VerifyRadiosReturnType();
		VerifyTerminalUnitGroup();

		// 'None' by default.
		ClickRadioControlTypeNone();

		// Manual balancing by default.
		m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );
		m_RadioBalancingTypeManual.SetCheck( BST_CHECKED );
		m_RadioBalancingTypeDiffPress.SetCheck( BST_UNCHECKED );
		m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
		m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
	}
	
	m_bOnActivateInCourse = false;
	UpdateCVTypeGroup();

	m_pFPSheet->SetFocus();
	
	EndWaitCursor();
	SetRedraw( FALSE );
	Invalidate();
	UpdateWindow();

	return true;
}

bool CPanelCirc1::OnAfterActivate()
{
	if( NULL != m_pFPSheet->GetSelectedSch() )
	{
		( (CDlgWizCircuit *)GetWizMan() )->EnableButtons( CWizardManager::WizButNext );

		if( true == m_bEditDoNext )
		{
			m_bEditDoNext = false;
			DoNext();
		}
	}
	else
	{
		( (CDlgWizCircuit *)GetWizMan() )->DisableButtons( CWizardManager::WizButNext );
	}

	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	::SetFocus( m_pFPSheet->GetSafeHwnd() );
	
	return true;
}

void CPanelCirc1::OnSize( CRect rectParentSize )
{
	// Retrieve current position of the separator.
	CRect rectSeparator;
	GetDlgItem( IDC_SEPARATORUP )->GetWindowRect( &rectSeparator );
	ScreenToClient( &rectSeparator );

	// Separator set in same place but take the all width.
	GetDlgItem( IDC_SEPARATORUP )->SetWindowPos( NULL, -1, -1, rectParentSize.Width(), rectSeparator.Height(), SWP_NOZORDER | SWP_NOMOVE );

	CRect rectPanelSize;
	GetClientRect( &rectPanelSize );

	m_pFPSheet->SetWindowPos( NULL, rectParentSize.left, rectSeparator.bottom, rectParentSize.Width(), rectPanelSize.Height() - rectSeparator.bottom, SWP_NOZORDER );
	m_pFPSheet->Invalidate();
	m_pFPSheet->UpdateWindow();
}

void CPanelCirc1::OnGetMinMaxInfo( MINMAXINFO *lpMMI )
{
	if( GetDlgItem( IDC_STATICCTRLTYPE )->GetSafeHwnd() != NULL )
	{
		// Retrieve position of right group 'control type'.
		CRect rectStaticCtrlType;
		GetDlgItem( IDC_STATICCTRLTYPE )->GetWindowRect( &rectStaticCtrlType );

		// Retrieve position of the parent dialog.
		CRect rectDialog;
		GetParent()->GetWindowRect( &rectDialog );

		// Take in count the border size.
		int iBorderSize = ::GetSystemMetrics( SM_CXSIZEFRAME );
		lpMMI->ptMinTrackSize.x = rectStaticCtrlType.right - rectDialog.left + iBorderSize;
	}
}

void CPanelCirc1::UpdateCVTypeGroup()
{
	// Retrieve general parameters for this HM.
	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	if( BST_CHECKED == m_RadioControlTypeNone.GetCheck() )
	{
		m_GroupControlValveType.EnableWindow( false, true );
		m_GroupWorkForControlValves.EnableWindow( false, true );
	}
	else if( BST_CHECKED == m_RadioBalancingTypeElectronic.GetCheck() )
	{
		// We active "Smart" and do not activate "Standard", "Adjustable", "Adjustable and measurable", "Pressure independant".
		m_GroupControlValveType.EnableWindow( false, true );
		CRankEx rkList;
		int iCount = m_pTADB->GetSmartControlValveList( &rkList, L"", L"", L"", L"", L"", L"", CTADatabase::FilterSelection::ForHMCalc );

		if( 0 == iCount )
		{
			iCount = m_pTADB->GetSmartDpCList( &rkList, L"", L"", L"", L"", L"", L"", CTADatabase::FilterSelection::ForHMCalc );
		}
		
		if( iCount > 0 )
		{
			m_GroupControlValveType.EnableWindow( true, false );
			m_RadioControlValveTypeSmart.EnableWindow( TRUE );
			ClickRadioControlValveTypeSmart();
		}
	}
	else
	{
		m_GroupControlValveType.EnableWindow( true, true );

		// HYS-1674: Only with electronic balancing
		m_RadioControlValveTypeSmart.EnableWindow( FALSE );
		
		// Define the CtrlPropType.
		CDB_ControlProperties::CvCtrlType eCvType = GetControlType();
		
		// Verify with existing control valves (CTRLVALV_TAB).
		CRankEx rkList;
		CTADatabase::CvTargetTab eCvTargetTab = ( CDB_CircuitScheme::eBALTYPE::BV == GetBalancingType() ) ? 
				CTADatabase::CvTargetTab::eForHMCvBalTypeMV : CTADatabase::CvTargetTab::eForHMCv;

		int iCount = m_pTADB->GetTaCVList( &rkList, eCvTargetTab, true, CDB_ControlProperties::LastCV2W3W, L"", L"", L"", L"", L"", L"", 
				CDB_ControlProperties::eCVFUNC::ControlOnly, eCvType, CTADatabase::FilterSelection::ForHMCalc );
		
		rkList.PurgeAll();
		m_RadioControlValveTypeStandard.EnableWindow( ( iCount > 0 ) ? TRUE : FALSE );
		
		iCount = m_pTADB->GetTaCVList( &rkList, eCvTargetTab, true, CDB_ControlProperties::LastCV2W3W, L"", L"", L"", L"", L"", L"", 
				CDB_ControlProperties::eCVFUNC::Presettable, eCvType, CTADatabase::FilterSelection::ForHMCalc );

		rkList.PurgeAll();

		if( CTADatabase::CvTargetTab::eForHMCvBalTypeMV == eCvTargetTab )
		{
			m_RadioControlValveTypeAdjustable.EnableWindow( ( iCount > 0 ) ? TRUE : FALSE );
			m_RadioControlValveTypePressureIndependent.EnableWindow( FALSE );
		}
		else
		{
			m_RadioControlValveTypeAdjustable.EnableWindow( ( iCount > 0 ) ? TRUE : FALSE );
			m_RadioControlValveTypePressureIndependent.EnableWindow( ( iCount > 0 ) ? TRUE : FALSE );
		}
		
		iCount = m_pTADB->GetTaCVList( &rkList, eCvTargetTab, true, CDB_ControlProperties::LastCV2W3W, L"", L"", L"", L"", L"", L"", 
				CDB_ControlProperties::eCVFUNC::PresetPT, eCvType, CTADatabase::FilterSelection::ForHMCalc );
		
		rkList.PurgeAll();

		if( CTADatabase::CvTargetTab::eForHMCvBalTypeMV == eCvTargetTab )
		{
			m_RadioControlValveTypeAdjustableMeasurable.EnableWindow( ( iCount > 0 ) ? TRUE : FALSE );
			m_RadioControlValveTypePressureIndependent.EnableWindow( FALSE );
		}
		else
		{
			m_RadioControlValveTypePressureIndependent.EnableWindow( ( iCount > 0 ) ? TRUE : FALSE );
			m_RadioControlValveTypeAdjustableMeasurable.EnableWindow( ( iCount > 0 ) ? TRUE : FALSE );
		}

		verifyRadioControlValveType();
	}

	// Init the SpreadSheet.
	InitSpreadSheet();
}

void CPanelCirc1::UpdateBalancingTypeGroup()
{
	// If control type = Proportional / On-off / 3-points we must use balancing.
	if( BST_UNCHECKED == m_RadioControlTypeNone.GetCheck() )
	{
		m_RadioBalancingTypeNone.EnableWindow( FALSE );
		m_RadioBalancingTypeNone.SetCheck( BST_UNCHECKED );

		if( BST_UNCHECKED == m_RadioBalancingTypeManual.GetCheck() && BST_UNCHECKED == m_RadioBalancingTypeDiffPress.GetCheck() 
				&& BST_UNCHECKED == m_RadioBalancingTypeElectronic.GetCheck() )
		{
			m_RadioBalancingTypeManual.SetCheck( BST_CHECKED );
		}
	}
	else
	{
		m_RadioBalancingTypeNone.EnableWindow( TRUE );
	}

	if( BST_UNCHECKED == m_RadioControlTypeProportional.GetCheck() )
	{
		// HYS-1674: Electronic balancing radio button only in proportional mode.
		m_RadioBalancingTypeElectronic.EnableWindow( FALSE );

		if( BST_CHECKED == m_RadioBalancingTypeElectronic.GetCheck() )
		{
			m_RadioBalancingTypeElectronic.SetCheck( BST_UNCHECKED );
			// Return to manual balancing.
			m_RadioBalancingTypeManual.SetCheck( BST_CHECKED );
		}
	}
	
	if( BST_UNCHECKED == m_RadioControlTypeNone.GetCheck() )
	{
		// HYS-1955: Electronic Dp controller radio can be activate when control type is "None" (For smart differential pressure controller).
		m_RadioBalancingTypeElectronicDpC.EnableWindow( FALSE );

		if( BST_CHECKED == m_RadioBalancingTypeElectronicDpC.GetCheck() )
		{
			m_RadioBalancingTypeElectronicDpC.SetCheck( BST_UNCHECKED );
			// Return to manual balancing.
			m_RadioBalancingTypeManual.SetCheck( BST_CHECKED );
		}
	}
	
	if( BST_CHECKED == m_RadioControlTypeProportional.GetCheck() )
	{
		m_RadioBalancingTypeElectronic.EnableWindow( TRUE );
	}
	else if( BST_CHECKED == m_RadioControlTypeNone.GetCheck() )
	{
		m_RadioBalancingTypeElectronicDpC.EnableWindow( TRUE );
	}
}

void CPanelCirc1::InitSpreadSheet()
{
	if( true == m_bOnActivateInCourse )
	{
		return;
	}

	CSSheetPanelCirc1::ePC1CVType eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_NoControl;
	CDB_ControlProperties::eCVFUNC CVFunc = CDB_ControlProperties::eCVFUNC::NoControl;

	// Update CV func if the group control valve type is enabled.
	if( CDB_ControlProperties::CvCtrlType::eCvNU != GetControlType() )
	{
		if( BST_CHECKED == m_RadioControlValveTypeStandard.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC::ControlOnly;
			eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_Standard;
		}
		else if( BST_CHECKED == m_RadioControlValveTypeAdjustable.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC::Presettable;
			eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_Adjustable;
		}
		else if( BST_CHECKED == m_RadioControlValveTypeAdjustableMeasurable.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC::PresetPT;
			eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_AdjustableMeas;
		}
		else if( BST_CHECKED == m_RadioControlValveTypePressureIndependent.GetCheck() )
		{
			CVFunc = (CDB_ControlProperties::eCVFUNC)( CDB_ControlProperties::eCVFUNC::Presettable | CDB_ControlProperties::eCVFUNC::PresetPT );
			eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_PressInd;
		}
		else if( BST_CHECKED == m_RadioControlValveTypeSmart.GetCheck() )
		{
			CVFunc = CDB_ControlProperties::eCVFUNC::Electronic;
			eCVTypeFamily = CSSheetPanelCirc1::ePC1CVType::ePC1CV_Smart;
		}
	}

	( (CDlgWizCircuit *)GetWizMan() )->DisableButtons( CWizardManager::WizButNext );

	// Can we show the distribution pipe?
	bool bShowDistributionPipe = false;
	
	if( NULL != m_pHM )
	{
		bShowDistributionPipe = ( m_pHM->GetLevel() > 0 ) ? true : false;
	}
	else
	{
		CTable *pInitialTable = ( (CDlgWizCircuit *)GetWizMan() )->GetpInitialTab();

		if( NULL != pInitialTable )
		{
			bShowDistributionPipe = ( true == pInitialTable->IsClass( CLASS( CTableHM ) ) ) ? false : true;
		}
	}

	CSSheetPanelCirc1::ePC1TU eTerminalUnit = CSSheetPanelCirc1::ePC1TU::ePC1TU_None;
	
	if( BST_CHECKED == m_RadioTUTerminalUnit.GetCheck() )
	{
		eTerminalUnit = CSSheetPanelCirc1::ePC1TU::ePC1TU_TermUnit;
	}
	else if( BST_CHECKED == m_RadioTURadiator.GetCheck() )
	{
		eTerminalUnit = CSSheetPanelCirc1::ePC1TU::ePC1TU_Radiator;
	}

	CDS_HydroMod::ReturnType eReturnType = ( BST_CHECKED ==  m_RadioReturnTypeDirect.GetCheck() ) ? CDS_HydroMod::ReturnType::Direct : CDS_HydroMod::ReturnType::Reverse;
	
	m_pFPSheet->Init( eTerminalUnit, bShowDistributionPipe, eReturnType, GetBalancingType(), CVFunc, GetControlType(), (int)eCVTypeFamily, m_pHM );
	m_pFPSheet->SetFocus();
}

void CPanelCirc1::VerifyTerminalUnitGroup()
{
	CDS_HydroMod *pHM = (CDS_HydroMod *)( (CDlgWizCircuit *)GetWizMan() )->GetpCurrentHM(); 

	// If hydroMod already exist...
	if( NULL != pHM )
	{
		if( true == pHM->IsaModule() )
		{
			m_RadioTUTerminalUnit.SetCheck( BST_UNCHECKED );
			m_RadioTUNone.SetCheck( BST_CHECKED );
			
			// Terminal unit can't be added:
			//		- if HM is a RootModule .
			//		- if any children exist.
			if( *pHM->GetFirst().ID || 0 == pHM->GetLevel() )
			{
				m_RadioTUTerminalUnit.EnableWindow( FALSE );
				m_RadioTUNone.EnableWindow( TRUE );
			}
			else
			{
				m_RadioTUTerminalUnit.EnableWindow( TRUE );
				m_RadioTUNone.EnableWindow( TRUE );
			}
		}
		else
		{
			m_RadioTUTerminalUnit.SetCheck( BST_CHECKED );
			m_RadioTUTerminalUnit.EnableWindow( TRUE );
			m_RadioTUNone.SetCheck( BST_UNCHECKED );
			m_RadioTUNone.EnableWindow( TRUE );
		}
	}
	else
	{
		CTable *pTab = (CTable *)( (CDlgWizCircuit *)GetWizMan() )->GetpInitialTab();
		if( true == pTab->IsClass( CLASS( CTableHM ) ) )
		{
			m_RadioTUNone.SetCheck( BST_CHECKED );
			m_RadioTUNone.EnableWindow( TRUE );
			m_RadioTUTerminalUnit.SetCheck( BST_UNCHECKED );
			m_RadioTUTerminalUnit.EnableWindow( FALSE );
		}
		else
		{
			// With terminal unit by default.
			m_RadioTUNone.SetCheck( BST_UNCHECKED );
			m_RadioTUNone.EnableWindow( TRUE );
			m_RadioTUTerminalUnit.SetCheck( BST_CHECKED );
			m_RadioTUTerminalUnit.EnableWindow( TRUE );

			// By default if we check terminal unit we check also proportional control type. Otherwise, we are in 
			// control type 'None' and we have just one or two choice
			ClickRadioControlTypeProportional();
		}
	}
	// Pending is available only for Circuits
}

void CPanelCirc1::VerifyRadiosReturnType()
{
	int iRadioDirectState = BST_CHECKED;
	int iRadioReverseState = BST_UNCHECKED;
	BOOL bRadioDirectEnable = TRUE;
	BOOL bRadioReverseEnable = TRUE;

	// If there is already one circuit...
	if( NULL != m_pHM )
	{
		// (A) If 'm_pHM' is a module and it's the first one (level = 0), we can't choose the direct or reverse return mode.
		// Actually there is no distribution pipe.
		//
		// If 'm_pHM' is a module on a deeper level (level > 0) or if 'm_pHM' is a terminal unit, we have 2 solutions:
		//
		//   (B) - If it's the first module (or terminal unit), we can choose the return mode.
		//   (C) - If it's not the first module, we have no choice and we keep the return mode of the other modules (or terminal
		//         unit) at the same level.

		if( true == m_pHM->IsaModule() && 0 == m_pHM->GetLevel() )
		{
			// (A).
			// Disable radio buttons.
			bRadioDirectEnable = FALSE;
			bRadioReverseEnable = FALSE;
		}
		else
		{
			CDS_HydroMod *pPHM = (CDS_HydroMod *)( m_pHM->GetIDPtr().PP );

			if( 1 == pPHM->GetCount() )
			{
				// (B).
				// Keep default values above.
			}
			else
			{
				// (C).
				
				// Get the sibling.
				CDS_HydroMod *pChildHM = (CDS_HydroMod *)( pPHM->GetFirst().MP );

				// If direct return mode, we keep the default value. In case of reverse, we exchange state of radios.
				if( CDS_HydroMod::ReturnType::Reverse == pChildHM->GetReturnType() )
				{
					iRadioDirectState = BST_UNCHECKED;
					iRadioReverseState = BST_CHECKED;
				}

				// Disable radio buttons.
				bRadioDirectEnable = FALSE;
				bRadioReverseEnable = FALSE;
			}
		}
	}
	else
	{
		// We get this case when the user wants to add circuit when directly clicking on the context menu 'Add circuit' in
		// the project tree.
		CTable *pInitialTab = (CTable *)( (CDlgWizCircuit *)GetWizMan() )->GetpInitialTab();
		CDS_HydroMod *pInitialHM = dynamic_cast<CDS_HydroMod *>( pInitialTab );

		if( pInitialHM != NULL )
		{
			// User has right-clicked on an existent module.

			// If there is at least one child...
			if( pInitialHM->GetCount() > 0 )
			{
				// Get this child and check what is it's return mode.
				CDS_HydroMod *pChildHM = (CDS_HydroMod *)( pInitialHM->GetFirst().MP );

				if( pChildHM != NULL )
				{
					// If direct return mode, we keep the default value. In case of reverse, we exchange state of radios.
					if( CDS_HydroMod::ReturnType::Reverse == pChildHM->GetReturnType() )
					{
						iRadioDirectState = BST_UNCHECKED;
						iRadioReverseState = BST_CHECKED;
					}

					// Disable radio buttons.
					bRadioDirectEnable = FALSE;
					bRadioReverseEnable = FALSE;
				}
			}
			else
			{
				// If it's the first child, the user can choose the return mode.
			}
		}
		else
		{
			// The user wants to add a main root module. In this case, he can't choose the return mode now.
			bRadioDirectEnable = FALSE;
			bRadioReverseEnable = FALSE;
		}
	}

	m_RadioReturnTypeDirect.SetCheck( iRadioDirectState );
	m_RadioReturnTypeReverse.SetCheck( iRadioReverseState );
	m_RadioReturnTypeDirect.EnableWindow( bRadioDirectEnable );
	m_RadioReturnTypeReverse.EnableWindow( bRadioReverseEnable );
}

void CPanelCirc1::verifyRadioControlValveType()
{
	if( BST_CHECKED == m_RadioControlValveTypeStandard.GetCheck() )
	{
		// 'Control only' was previously checked.

		if( FALSE == m_RadioControlValveTypeStandard.IsWindowEnabled() )
		{
			// Now the radio is disabled.

			if( TRUE == m_RadioControlValveTypeAdjustable.IsWindowEnabled() )
			{
				// The 'Presettable' radio is enabled, choose this one.
				ClickRadioControlValveTypeAdjustable();
			}
			else if( TRUE == m_RadioControlValveTypeAdjustableMeasurable.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypeAdjustableAndMeasurable();
			}
			else if( TRUE == m_RadioControlValveTypePressureIndependent.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypePressureIndependent();
			}
			else if( TRUE == m_RadioControlValveTypeSmart.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypeSmart();
			}
			else
			{
				// Otherwise simply uncheck 'Control only'.
				m_RadioControlValveTypeStandard.SetCheck( BST_UNCHECKED );
			}
		}
		else
		{
			ClickRadioControlValveTypeStandard();
		}
	}
	else if( BST_CHECKED == m_RadioControlValveTypeAdjustable.GetCheck() )
	{
		// 'Presettable' was previously checked.

		if( FALSE == m_RadioControlValveTypeAdjustable.IsWindowEnabled() )
		{
			// Now the radio is disabled.

			if( TRUE == m_RadioControlValveTypeStandard.IsWindowEnabled() )
			{
				// The 'Control only' radio is enabled, choose this one.
				ClickRadioControlValveTypeStandard();
			}
			else if( TRUE == m_RadioControlValveTypeAdjustableMeasurable.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypeAdjustableAndMeasurable();
			}
			else if( TRUE == m_RadioControlValveTypePressureIndependent.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypePressureIndependent();
			}
			else if( TRUE == m_RadioControlValveTypeSmart.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypeSmart();
			}
			else
			{
				// Otherwise simply uncheck 'Presettable'.
				m_RadioControlValveTypeAdjustable.SetCheck( BST_UNCHECKED );
			}
		}
		else
		{
			ClickRadioControlValveTypeAdjustable();
		}
	}
	else if( BST_CHECKED == m_RadioControlValveTypeAdjustableMeasurable.GetCheck() )
	{
		// 'Presettable' was previously checked.

		if( FALSE == m_RadioControlValveTypeAdjustableMeasurable.IsWindowEnabled() )
		{
			// Now the radio is disabled.

			if( TRUE == m_RadioControlValveTypeStandard.IsWindowEnabled() )
			{
				// The 'Control only' radio is enabled, choose this one.
				ClickRadioControlValveTypeStandard();
			}
			else if( TRUE == m_RadioControlValveTypeAdjustable.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypeAdjustable();
			}
			else if( TRUE == m_RadioControlValveTypePressureIndependent.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypePressureIndependent();
			}
			else if( TRUE == m_RadioControlValveTypeSmart.IsWindowEnabled() )
			{
				// The 'PresetPT' radio is enabled, choose this one.
				ClickRadioControlValveTypeSmart();
			}
			else
			{
				// Otherwise simply uncheck 'Presettable'.
				m_RadioControlValveTypeAdjustableMeasurable.SetCheck( BST_UNCHECKED );
			}
		}
		else
		{
			ClickRadioControlValveTypeAdjustableAndMeasurable();
		}
	}
	else if( BST_CHECKED == m_RadioControlValveTypePressureIndependent.GetCheck() )
	{
		// 'PresetPT' was previously checked.

		if( FALSE == m_RadioControlValveTypePressureIndependent.IsWindowEnabled() )
		{
			// Now the radio is disabled.

			if( TRUE == m_RadioControlValveTypeStandard.IsWindowEnabled() )
			{
				// The 'Control only' radio is enabled, choose this one.
				ClickRadioControlValveTypeStandard();
			}
			else if( TRUE == m_RadioControlValveTypeAdjustable.IsWindowEnabled() )
			{
				// The 'Presettable' radio is enabled, choose this one.
				ClickRadioControlValveTypeAdjustable();
			}
			else if( TRUE == m_RadioControlValveTypeAdjustableMeasurable.IsWindowEnabled() )
			{
				// The 'Presettable' radio is enabled, choose this one.
				ClickRadioControlValveTypeAdjustableAndMeasurable();
			}
			else if( TRUE == m_RadioControlValveTypeSmart.IsWindowEnabled() )
			{
				// The 'Presettable' radio is enabled, choose this one.
				ClickRadioControlValveTypeSmart();
			}
			else
			{
				// Otherwise simply uncheck 'PresetPT'.
				m_RadioControlValveTypePressureIndependent.SetCheck( BST_UNCHECKED );
			}
		}
		else
		{
			ClickRadioControlValveTypePressureIndependent();
		}
	}
	else if( BST_CHECKED == m_RadioControlValveTypeSmart.GetCheck() )
	{
		// 'PresetPT' was previously checked.

		if( FALSE == m_RadioControlValveTypeSmart.IsWindowEnabled() )
		{
			// Now the radio is disabled.

			if( TRUE == m_RadioControlValveTypeStandard.IsWindowEnabled() )
			{
				// The 'Control only' radio is enabled, choose this one.
				ClickRadioControlValveTypeStandard();
			}
			else if( TRUE == m_RadioControlValveTypeAdjustable.IsWindowEnabled() )
			{
				// The 'Presettable' radio is enabled, choose this one.
				ClickRadioControlValveTypeAdjustable();
			}
			else if( TRUE == m_RadioControlValveTypeAdjustableMeasurable.IsWindowEnabled() )
			{
				// The 'Presettable' radio is enabled, choose this one.
				ClickRadioControlValveTypeAdjustableAndMeasurable();
			}
			else if( TRUE == m_RadioControlValveTypePressureIndependent.IsWindowEnabled() )
			{
				// The 'Presettable' radio is enabled, choose this one.
				ClickRadioControlValveTypePressureIndependent();
			}
			else
			{
				// Otherwise simply uncheck 'PresetPT'.
				m_RadioControlValveTypeSmart.SetCheck( BST_UNCHECKED );
			}
		}
		else
		{
			ClickRadioControlValveTypeSmart();
		}
	}
	else
	{
		// In that case, no radio button is check.
		// Check at least one.
		if( TRUE == m_RadioControlValveTypeStandard.IsWindowEnabled() )
		{
			m_RadioControlValveTypeStandard.SetCheck( BST_CHECKED );
			ClickRadioControlValveTypeStandard();
		}
		else if( TRUE == m_RadioControlValveTypeAdjustable.IsWindowEnabled() )
		{
			m_RadioControlValveTypeAdjustable.SetCheck( BST_CHECKED );
			ClickRadioControlValveTypeAdjustable();
		}
		else if( TRUE == m_RadioControlValveTypeAdjustableMeasurable.IsWindowEnabled() )
		{
			m_RadioControlValveTypeAdjustableMeasurable.SetCheck( BST_CHECKED );
			ClickRadioControlValveTypeAdjustableAndMeasurable();
		}
		else if( TRUE == m_RadioControlValveTypePressureIndependent.IsWindowEnabled() )
		{
			m_RadioControlValveTypePressureIndependent.SetCheck( BST_CHECKED );
			ClickRadioControlValveTypePressureIndependent();
		}
		else if( TRUE == m_RadioControlValveTypeSmart.IsWindowEnabled() )
		{
			m_RadioControlValveTypeSmart.SetCheck( BST_CHECKED );
			ClickRadioControlValveTypeSmart();
		}
		else
		{
			// It can happens.
			// For example Japanese version, there is only the thermostatic control valve for the control valve type.
			// If user chooses 'Proportional' for the control type, any thermostatic valve match. Because their have no
			// characteristic.
		}
	}
}

void CPanelCirc1::ClickRadioReturnTypeDirect()
{
	m_RadioReturnTypeDirect.SetCheck( BST_CHECKED );
	m_RadioReturnTypeReverse.SetCheck( BST_UNCHECKED );

	if( m_pHM != NULL )
	{
		m_pHM->SetReturnType( CDS_HydroMod::ReturnType::Direct );
	}
}

void CPanelCirc1::ClickRadioReturnTypeReverse()
{
	m_RadioReturnTypeDirect.SetCheck( BST_UNCHECKED );
	m_RadioReturnTypeReverse.SetCheck( BST_CHECKED );

	if( m_pHM != NULL )
	{
		m_pHM->SetReturnType( CDS_HydroMod::ReturnType::Reverse );
	}
}

void CPanelCirc1::ClickRadioWorkWithControlValvesKvsValues()
{
	m_RadioWorkForControlValvesKvsValues.SetCheck( BST_CHECKED );
	m_RadioWorkForControlValvesIMIValves.SetCheck( BST_UNCHECKED );
	
	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );
	
	pHMGenParam->SetCVMode( CPrjParams::_CVMode::KvMode );
}

void CPanelCirc1::ClickRadioWorkWithControlValvesIMIValves()
{
	m_RadioWorkForControlValvesKvsValues.SetCheck( BST_UNCHECKED );
	m_RadioWorkForControlValvesIMIValves.SetCheck( BST_CHECKED );
	
	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );
	
	pHMGenParam->SetCVMode(CPrjParams::_CVMode::TAMode);
}

void CPanelCirc1::ClickRadioControlTypeNone()
{
	m_RadioControlTypeOnOFF.SetCheck( BST_UNCHECKED );
	m_RadioControlType3Points.SetCheck( BST_UNCHECKED );
	m_RadioControlTypeProportional.SetCheck( BST_UNCHECKED );
	m_RadioControlTypeNone.SetCheck( BST_CHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVCtrlType( CDB_ControlProperties::CvCtrlType::eCvNU );

	UpdateBalancingTypeGroup();
}

void CPanelCirc1::ClickRadioControlTypeProportional()
{
	m_RadioControlTypeOnOFF.SetCheck( BST_UNCHECKED );
	m_RadioControlType3Points.SetCheck( BST_UNCHECKED );
	m_RadioControlTypeNone.SetCheck( BST_UNCHECKED );
	m_RadioControlTypeProportional.SetCheck( BST_CHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVCtrlType( CDB_ControlProperties::CvCtrlType::eCvProportional );
	UpdateBalancingTypeGroup();
}

void CPanelCirc1::ClickRadioControlTypeOnOff()
{
	m_RadioControlTypeOnOFF.SetCheck( BST_CHECKED );
	m_RadioControlType3Points.SetCheck( BST_UNCHECKED );
	m_RadioControlTypeNone.SetCheck( BST_UNCHECKED );
	m_RadioControlTypeProportional.SetCheck( BST_UNCHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVCtrlType( CDB_ControlProperties::CvCtrlType::eCvOnOff );
	UpdateBalancingTypeGroup();
}

void CPanelCirc1::ClickRadioControlType3points()
{
	m_RadioControlType3Points.SetCheck( BST_CHECKED );
	m_RadioControlTypeNone.SetCheck( BST_UNCHECKED );
	m_RadioControlTypeOnOFF.SetCheck( BST_UNCHECKED );
	m_RadioControlTypeProportional.SetCheck( BST_UNCHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVCtrlType( CDB_ControlProperties::CvCtrlType::eCv3point );
	UpdateBalancingTypeGroup();
}

void CPanelCirc1::ClickRadioControlValveTypeStandard()
{
	// Enable work for control valves Kvs option
	m_GroupWorkForControlValves.EnableWindow( true, true );

	m_RadioControlValveTypeStandard.SetCheck( BST_CHECKED );
	m_RadioControlValveTypeAdjustable.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeAdjustableMeasurable.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypePressureIndependent.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeSmart.SetCheck( BST_UNCHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVType( CDB_ControlProperties::eCVFUNC::ControlOnly );
}

void CPanelCirc1::ClickRadioControlValveTypeAdjustable()
{
	// Disabled work for control valves Kvs option
	ClickRadioWorkWithControlValvesIMIValves();
	m_GroupWorkForControlValves.EnableWindow( false, true );

	m_RadioControlValveTypeStandard.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeAdjustable.SetCheck( BST_CHECKED );
	m_RadioControlValveTypeAdjustableMeasurable.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypePressureIndependent.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeSmart.SetCheck( BST_UNCHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVType( CDB_ControlProperties::eCVFUNC::Presettable );
}

void CPanelCirc1::ClickRadioControlValveTypeAdjustableAndMeasurable()
{
	// Disabled work for control valves Kvs option
	ClickRadioWorkWithControlValvesIMIValves();
	m_GroupWorkForControlValves.EnableWindow( false, true );

	m_RadioControlValveTypeStandard.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeAdjustable.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeAdjustableMeasurable.SetCheck( BST_CHECKED );
	m_RadioControlValveTypePressureIndependent.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeSmart.SetCheck( BST_UNCHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVType( CDB_ControlProperties::eCVFUNC::PresetPT );
}

void CPanelCirc1::ClickRadioControlValveTypePressureIndependent()
{
	// Disabled work for control valves Kvs option
	ClickRadioWorkWithControlValvesIMIValves();
	m_GroupWorkForControlValves.EnableWindow( false, true );

	m_RadioControlValveTypeStandard.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeAdjustable.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeAdjustableMeasurable.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypePressureIndependent.SetCheck( BST_CHECKED );
	m_RadioControlValveTypeSmart.SetCheck( BST_UNCHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVType( CDB_ControlProperties::eCVFUNC( CDB_ControlProperties::eCVFUNC::Presettable | CDB_ControlProperties::eCVFUNC::PresetPT ) );
}

void CPanelCirc1::ClickRadioControlValveTypeSmart()
{
	// Disabled work for control valves Kvs option
	ClickRadioWorkWithControlValvesIMIValves();
	m_GroupWorkForControlValves.EnableWindow( false, true );

	m_RadioControlValveTypeStandard.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeAdjustable.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeAdjustableMeasurable.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypePressureIndependent.SetCheck( BST_UNCHECKED );
	m_RadioControlValveTypeSmart.SetCheck( BST_CHECKED );

	CPrjParams *pHMGenParam = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pHMGenParam );

	pHMGenParam->SetCVType( CDB_ControlProperties::eCVFUNC::Electronic );
}
