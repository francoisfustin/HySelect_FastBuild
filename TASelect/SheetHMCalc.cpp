#include "stdafx.h"
#include "afxctl.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "Utilities.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "HMPump.h"
#include "Hydronic.h"
#include "RViewHMCalc.h"
#include "DlgWizCircuit.h"
#include "DlgComboBoxHM.h"
#include "Picture.h"
#include "SheetHMCalc.h"
#include "DlgInjectionError.h"
#include "DlgLeftTabProject.h"
#include "DlgSelectActuator.h"
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define XMARGIN 2   // Pixels
#define YMARGIN 1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSheetHMCalc::CSheetHMCalc(): CSSheet()
{
	for( int iLoopSheet = SheetDescription::SFirst; iLoopSheet <= SheetDescription::SLast; iLoopSheet++ )
	{
		m_arptLeftTop[iLoopSheet] = CPoint( 0, 0 );
	}

	m_bInitialized = false;
	m_bPrinting = false;
	m_bExporting = false;
	m_bReserverFirstSheet = false;
	m_lMSSelectedRow = 0;
	m_nCurrentSheet = SheetDescription::SDistribution;
	m_lLastEditedRow = 0;
	m_lLastEditedCol = 0;
	m_strCellText = _T( "" );
	m_dCellDouble = 0.0;
	m_bCellModified = false;
	m_lpCellParam = (LPARAM)0;
	m_bComboEditMode = false;
	m_iTabOffSet = 0;
	m_pSCB = NULL;
	m_pDistributionSupplyPipeTable = NULL;
	m_pDistributionReturnPipeTable = NULL;
	m_pPipeCircTab = NULL;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pHM = NULL;
	m_bAtLeastOneReverseMode = false;
	m_pHMSelectActuator = NULL;
	m_vecCutRow.clear();
	m_rFillColors.Reset();
}

CSheetHMCalc::~CSheetHMCalc()
{
	CSSheet::MM_UnregisterAllNotificationHandlers( this );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
	// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).

	// Remark: we set NULL pointer in all 'CSheetDescription' to avoid that destructor attempt to kill 'CSSheetHMCalc' again.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SDistribution );

	if( NULL != pclSheetDescription )
	{
		pclSheetDescription->SetSSheetPointer( NULL );
	}

	pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SCircuit );

	if( NULL != pclSheetDescription )
	{
		pclSheetDescription->SetSSheetPointer( NULL );
	}

	pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SCircuitInj );

	if( NULL != pclSheetDescription )
	{
		pclSheetDescription->SetSSheetPointer( NULL );
	}

	pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SMeasurement );

	if( NULL != pclSheetDescription )
	{
		pclSheetDescription->SetSSheetPointer( NULL );
	}

	pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::STADiagnostic );

	if( NULL != pclSheetDescription )
	{
		pclSheetDescription->SetSSheetPointer( NULL );
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	for( int iLoop = SheetDescription::SFirst; iLoop <= SheetDescription::SLast; iLoop++ )
	{
		m_arSheetGroupExpandedList[iLoop].clear();
	}

	if( NULL != m_pSCB )
	{
		delete m_pSCB;
		m_pSCB = NULL;
	}

	if( NULL != m_pHMSelectActuator )
	{
		delete m_pHMSelectActuator;
		m_pHMSelectActuator = NULL;
	}

	if( NULL != m_DragDropInfo.m_hHandOpen )
	{
		DeleteObject( m_DragDropInfo.m_hHandOpen );
		m_DragDropInfo.m_hHandOpen = NULL;
	}

	if( NULL != m_DragDropInfo.m_hHandClosed )
	{
		DeleteObject( m_DragDropInfo.m_hHandClosed );
		m_DragDropInfo.m_hHandClosed = NULL;
	}

	if( NULL != m_DragDropInfo.m_hHandForbidden )
	{
		DeleteObject( m_DragDropInfo.m_hHandForbidden );
		m_DragDropInfo.m_hHandForbidden = NULL;
	}
}

BOOL CSheetHMCalc::Create( DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID )
{
	if( FALSE == CSSheet::Create( ( dwStyle & ~WS_BORDER ), rect, pParentWnd, nID ) )
	{
		return FALSE;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
	// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).

	CSheetDescription *pclSheetDescription = m_ViewDescription.AddSheetDescription( SheetDescription::SDistribution, -1, this, CPoint( 0, 0 ) );

	if( NULL == pclSheetDescription )
	{
		return FALSE;
	}

	m_mapResizingColumnInfos[SheetDescription::SDistribution] = pclSheetDescription->GetResizingColumnInfo();

	pclSheetDescription = m_ViewDescription.AddSheetDescription( SheetDescription::SCircuit, -1, this, CPoint( 0, 0 ) );

	if( NULL == pclSheetDescription )
	{
		return FALSE;
	}

	m_mapResizingColumnInfos[SheetDescription::SCircuit] = pclSheetDescription->GetResizingColumnInfo();

	pclSheetDescription = m_ViewDescription.AddSheetDescription( SheetDescription::SCircuitInj, -1, this, CPoint( 0, 0 ) );

	if( NULL == pclSheetDescription )
	{
		return FALSE;
	}

	m_mapResizingColumnInfos[SheetDescription::SCircuitInj] = pclSheetDescription->GetResizingColumnInfo();

	pclSheetDescription = m_ViewDescription.AddSheetDescription( SheetDescription::SMeasurement, -1, this, CPoint( 0, 0 ) );

	if( NULL == pclSheetDescription )
	{
		return FALSE;
	}

	m_mapResizingColumnInfos[SheetDescription::SMeasurement] = pclSheetDescription->GetResizingColumnInfo();

	pclSheetDescription = m_ViewDescription.AddSheetDescription( SheetDescription::STADiagnostic, -1, this, CPoint( 0, 0 ) );

	if( NULL == pclSheetDescription )
	{
		return FALSE;
	}

	m_mapResizingColumnInfos[SheetDescription::STADiagnostic] = pclSheetDescription->GetResizingColumnInfo();
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return TRUE;
}

void CSheetHMCalc::Reset( bool bAll )
{
	CSSheet::Reset();

	for( int iLoopSheet = SheetDescription::SFirst; iLoopSheet <= SheetDescription::SLast; iLoopSheet++ )
	{
		m_arptLeftTop[iLoopSheet] = CPoint( 0, 0 );
	}

	m_bInitialized = false;
	m_bPrinting = false;
	m_bExporting = false;
	m_bReserverFirstSheet = false;
	m_lMSSelectedRow = 0;
	m_nCurrentSheet = SheetDescription::SDistribution;
	m_lLastEditedRow = 0;
	m_lLastEditedCol = 0;
	m_strCellText = _T( "" );
	m_dCellDouble = 0.0;
	m_bCellModified = false;
	m_lpCellParam = (LPARAM)0;
	m_bComboEditMode = false;
	m_iTabOffSet = 0;
	m_pDistributionSupplyPipeTable = NULL;
	m_pDistributionReturnPipeTable = NULL;
	m_pPipeCircTab = NULL;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pHM = NULL;

	for( int iLoop = SheetDescription::SFirst; iLoop <= SheetDescription::SLast; iLoop++ )
	{
		m_arSheetGroupExpandedList[iLoop].clear();
	}

	if( true == bAll )
	{
		m_vecCutRow.clear();
	}

	m_DragDropInfo.Reset();
	CSSheet::MM_UnregisterAllNotificationHandlers( this );
}

void CSheetHMCalc::Init( CDS_HydroMod *pHM, bool bPrinting, int iTabOffset, bool bForExport )
{
	m_bInitialized = true;
	m_bPrinting = bPrinting;
	m_iTabOffSet = iTabOffset;
	m_bExporting = bForExport;

	// Save current sheet to reinit it at the end of this method. 'Init' can be internally called. And in this case, it's better to reset
	// the sheet where the user was.
	short nCurrentSheet = m_nCurrentSheet;

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		if( NULL != m_mapResizingColumnInfos[SheetDescription::SDistribution] )
		{
			m_mapResizingColumnInfos[SheetDescription::SDistribution]->Reset();
		}

		if( NULL != m_mapResizingColumnInfos[SheetDescription::SCircuit] )
		{
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->Reset();
		}

		if( NULL != m_mapResizingColumnInfos[SheetDescription::SCircuitInj] )
		{
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->Reset();
		}

		if( NULL != m_mapResizingColumnInfos[SheetDescription::SMeasurement] )
		{
			m_mapResizingColumnInfos[SheetDescription::SMeasurement]->Reset();
		}

		if( NULL != m_mapResizingColumnInfos[SheetDescription::STADiagnostic] )
		{
			m_mapResizingColumnInfos[SheetDescription::STADiagnostic]->Reset();
		}
	}

	if( NULL != pHM )
	{
		if( pHM->IsaModule() )
		{
			m_pHM = pHM;
		}
		else
		{
			// If Terminal Unit Show Page for Parent
			m_pHM = pHM->GetParent();
		}
	}

	SetBool( SSB_REDRAW, FALSE );

	// Variables.
	bool bDisplayMeasTab = false;
	bool bHMCalcMode =  false;
	bool bAtLeastOneInjCir = false;
	bool bDisplayTADiagnostic = false;

	// Define the HMCalc mode.
	bHMCalcMode = pMainFrame->IsHMCalcChecked();

	bAtLeastOneInjCir = IsInjectionCircuitExist( m_pHM );

	// Verify if measurement exist.
	if( true == IsMeasurementExist( m_pHM ) || false == bHMCalcMode )
	{
		bDisplayMeasTab = true;
	}

	// If we must display the measurement tab...
	if( true == bDisplayMeasTab )
	{
		// Remarks: For balancing, all modules must have measurements. Thus, if first module is balanced, it's ok.
		CDS_HydroMod::CMeasData *pMeasData = m_pHM->GetpMeasData( 0 );

		if( NULL != pMeasData )
		{
			bDisplayTADiagnostic = ( pMeasData->GetFlagBalanced() == CDS_HydroModX::eTABalanceMethod::etabmTABalPlus );
		}
		else		//No PV! test first child
		{
			CDS_HydroMod *pChildHM = dynamic_cast<CDS_HydroMod *>( m_pHM->GetFirst().MP );

			if( NULL != pChildHM )
			{
				pMeasData = pChildHM->GetpMeasData( 0 );

				if( NULL != pMeasData )
				{
					bDisplayTADiagnostic = ( pMeasData->GetFlagBalanced() == CDS_HydroModX::eTABalanceMethod::etabmTABalPlus );
				}
			}
		}
	}

	// Due to spread limitation we block the tab's number.
	SetSheetCount( GetSheetNumber( SheetDescription::SLast ) );

	// Create as sheet as necessary.
	for( int iLoopSheet = SheetDescription::SFirst; iLoopSheet <= SheetDescription::SLast; iLoopSheet++ )
	{
		m_arptLeftTop[iLoopSheet] = CPoint( 0, 0 );
	}

	m_pTADS = TASApp.GetpTADS();
	m_pTADB = TASApp.GetpTADB();
	ASSERT( NULL != m_pTADB );

	if( NULL != m_pSCB )
	{
		delete m_pSCB;
	}

	m_pSCB = NULL;
	m_pSCB = new CDlgComboBoxHM( this );

	if( NULL == m_pSCB )
	{
		m_bInitialized = false;
		return;
	}

	if( NULL == m_pHMSelectActuator )
	{
		m_pHMSelectActuator = new CDlgSelectActuator( CTADatabase::FilterSelection::ForHMCalc, true, this );
	}

	m_bCellModified = false;
	m_dCellDouble = 0.0;
	m_lpCellParam = NULL;
	m_strCellText = _T( "" );
	m_bComboEditMode = false;

	SetTabStripRatio( 0.3 );

	// Get the row coordinates of the top cell displayed.
	long lTopRow = GetTopRow();

	if( true == bHMCalcMode )
	{
		// Init the Distribution tab.
		_SetCurrentSheet( GetSheetNumber( SheetDescription::SDistribution ) );
		CSSheet::Init();
		
		SetBool( SSB_AUTOCLIPBOARD, FALSE );
		SetBool( SSB_ALLOWDRAGDROP, FALSE );
		
		CString str = TASApp.LoadLocalizedString( IDS_SHEETDISTRIBUTION );

		if( true == m_bExporting )
		{
			// Remove '*' because excel doesn't support the character for the sheet name.
			CString strName = m_pHM->GetHMName();
			strName.Remove( '*' );
			str = strName + _T(" ") + str;
		}

		SetSheetName( GetSheetNumber( SheetDescription::SDistribution ), ( LPCTSTR )str );
		_InitDistributionSheet( m_pHM );
		_FillDistributionSheet();

		if( true == m_bPrinting || true == m_bExporting )
		{
			_ExpandAll();
		}

		// Init the Circuit tab.
		_SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuit ) );
		CSSheet::Init();

		SetBool( SSB_ALLOWDRAGDROP, FALSE );
		str = TASApp.LoadLocalizedString( IDS_SHEETCIRCUITS );

		if( true == m_bExporting )
		{
			// Remove '*' because excel doesn't support the character for the sheet name.
			CString strName = m_pHM->GetHMName();
			strName.Remove( '*' );
			str = strName + _T(" ") + str;
		}

		SetSheetName( GetSheetNumber( SheetDescription::SCircuit ), ( LPCTSTR )str );
		_InitCircuitsSheet( m_pHM );
		_FillCircuitsSheet();

		if( true == m_bPrinting || true == m_bExporting )
		{
			_ExpandAll();
		}

		if( true == bAtLeastOneInjCir )
		{
			// Init the CircuitInj tab.
			_SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuitInj ) );
			CSSheet::Init();
			
			SetBool( SSB_ALLOWDRAGDROP, FALSE );
			str = TASApp.LoadLocalizedString( IDS_HMCALC_UNITSECSIDE );

			if( true == m_bExporting )
			{
				// Remove '*' because excel doesn't support the character for the sheet name.
				CString strName = m_pHM->GetHMName();
				strName.Remove( '*' );
				str = strName + _T(" ") + str;
			}

			SetSheetName( GetSheetNumber( SheetDescription::SCircuitInj ), ( LPCTSTR )str );
			_InitCircuitInjSheet( m_pHM );
			_FillCircuitInjSheet();

			if( true == m_bPrinting || true == m_bExporting )
			{
				_ExpandAll();
			}
		}
	}

	// Init the SMeasurement.
	if( true == bDisplayMeasTab ) // Verify at least one measurement exist
	{
		_SetCurrentSheet( GetSheetNumber( SheetDescription::SMeasurement ) );
		CSSheet::Init();
		CString str = TASApp.LoadLocalizedString( IDS_SHEETMEAS );

		if( true == m_bExporting )
		{
			// Remove '*' because excel doesn't support the character for the sheet name.
			CString strName = m_pHM->GetHMName();
			strName.Remove( '*' );
			str = strName + _T(" ") + str;
		}

		SetSheetName( GetSheetNumber( SheetDescription::SMeasurement ), ( LPCTSTR )str );
		_InitMeasurementSheet( m_pHM );
		_FillMeasurementSheet();

		if( true == m_bPrinting || true == m_bExporting )
		{
			_ExpandAll();
		}
	}

	// Init the 'TA Diagnostic' tab.
	if( true == bDisplayTADiagnostic )
	{
		_SetCurrentSheet( GetSheetNumber( SheetDescription::STADiagnostic ) );
		CSSheet::Init();
		CString str = TASApp.LoadLocalizedString( IDS_SHEETTABALANCEP );

		if( true == m_bExporting )
		{
			// Remove '*' because excel doesn't support the character for in tabs.
			CString strName = m_pHM->GetHMName();
			strName.Remove( '*' );
			str = strName + _T(" ") + str;
		}

		SetSheetName( GetSheetNumber( SheetDescription::STADiagnostic ), ( LPCTSTR )str );
		_InitTADiagnosticSheet( m_pHM );
		_FillTADiagnosticSheet();

		if( true == m_bPrinting || true == m_bExporting )
		{
			_ExpandAll();
		}
	}

	SetCellNoteIndicator( SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT );
	SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// In case the HMCalc mode is not active, display only the measurement and balancing tabs (if exists).
	// In case the HMCalc mode is active, display the circuit, distribution, measurement (if exists) and balancing (if exists) tabs.
	SetSheetVisible( GetSheetNumber( SheetDescription::SDistribution ), ( true == bHMCalcMode ) ? TRUE : FALSE );
	SetSheetVisible( GetSheetNumber( SheetDescription::SCircuit ), ( true == bHMCalcMode ) ? TRUE : FALSE );
	SetSheetVisible( GetSheetNumber( SheetDescription::SCircuitInj ), ( true == bHMCalcMode && true == bAtLeastOneInjCir ) ? TRUE : FALSE );
	SetSheetVisible( GetSheetNumber( SheetDescription::SMeasurement ), ( true == bDisplayMeasTab ) ? TRUE : FALSE );
	SetSheetVisible( GetSheetNumber( SheetDescription::STADiagnostic ), ( true == bDisplayTADiagnostic ) ? TRUE : FALSE );

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).

		// Add possibility to change column size.
		// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
		if( true == _SetCurrentSheet( GetSheetNumber( SheetDescription::SDistribution ) ) )
		{
			m_mapResizingColumnInfos[SheetDescription::SDistribution]->AddRangeColumn( CD_Distribution_Name, CD_Distribution_SupplyPipeVelocity, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SDistribution]->AddRangeColumn( CD_Distribution_SupplyAccDesc, CD_Distribution_SupplyAccDp, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SDistribution]->AddRangeColumn( CD_Distribution_ReturnPicture, CD_Distribution_ReturnAccDp, RD_Header_ColName, RD_Header_Unit );
		}

		if( true == _SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuit ) ) )
		{
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_Name, CD_Circuit_Desc, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_UnitDesc, CD_Circuit_UnitDpref, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_CVName, CD_Circuit_CVDesignAuth, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_ActuatorName, CD_Circuit_ActuatorDRP, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_SmartValveName, CD_Circuit_SmartValveDpl, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_BvName, CD_Circuit_BvDpMin, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_SvPict, CD_Circuit_SvDp, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_DpCName, CD_Circuit_DpCDpMin, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_BvBypPict, CD_Circuit_BvBypSet, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_PipeSerie, CD_Circuit_PipeV, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuit]->AddRangeColumn( CD_Circuit_PipeAccDesc, CD_Circuit_PipeAccDp, RD_Header_ColName, RD_Header_Unit );
		}

		if( true == _SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuitInj ) ) )
		{
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_Name, CD_CircuitInjSecSide_Desc, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_UnitDesc, CD_CircuitInjSecSide_Cv, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_PumpH, CD_CircuitInjSecSide_PumpHMin, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_TempTsp, CD_CircuitInjSecSide_TempTrs, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_CVName, CD_CircuitInjSecSide_CVDesignAuth, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_ActuatorName, CD_CircuitInjSecSide_ActuatorDRP, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_BvName, CD_CircuitInjSecSide_BvDpMin, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_PipeSerie, CD_CircuitInjSecSide_PipeV, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SCircuitInj]->AddRangeColumn( CD_CircuitInjSecSide_AccDesc, CD_CircuitInjSecSide_AccDp, RD_Header_ColName, RD_Header_Unit );
		}

		if( true == _SetCurrentSheet( GetSheetNumber( SheetDescription::SMeasurement ) ) )
		{
			m_mapResizingColumnInfos[SheetDescription::SMeasurement]->AddRangeColumn( CD_Measurement_DateTime, CD_Measurement_Description, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SMeasurement]->AddRangeColumn( CD_Measurement_WaterChar, CD_Measurement_KvSignal, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SMeasurement]->AddRangeColumn( CD_Measurement_Dp, CD_Measurement_Dpl, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SMeasurement]->AddRangeColumn( CD_Measurement_MeasureDp, CD_Measurement_FlowDeviation, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SMeasurement]->AddRangeColumn( CD_Measurement_TempHH, CD_Measurement_DiffTempDeviation, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::SMeasurement]->AddRangeColumn( CD_Measurement_Power, CD_Measurement_PowerDeviation, RD_Header_ColName, RD_Header_Unit );
		}

		if( true == _SetCurrentSheet( GetSheetNumber( SheetDescription::STADiagnostic ) ) )
		{
			m_mapResizingColumnInfos[SheetDescription::STADiagnostic]->AddRangeColumn( CD_TADiagnostic_DateTime, CD_TADiagnostic_Description, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::STADiagnostic]->AddRangeColumn( CD_TADiagnostic_WaterChar, CD_TADiagnostic_DesignFlow, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::STADiagnostic]->AddRangeColumn( CD_TADiagnostic_Dp1, CD_TADiagnostic_FlowDeviation, RD_Header_ColName, RD_Header_Unit );
			m_mapResizingColumnInfos[SheetDescription::STADiagnostic]->AddRangeColumn( CD_TADiagnostic_CircuitDpMeas, CD_TADiagnostic_DistribDpDesign, RD_Header_ColName, RD_Header_Unit );
		}
	}

	// Define the default sheet to display.
	if( false == bHMCalcMode )
	{
		nCurrentSheet = SheetDescription::SMeasurement;
	}
	else if( SheetDescription::SMeasurement == nCurrentSheet && false == bDisplayMeasTab )
	{
		nCurrentSheet = SheetDescription::SDistribution;
	}
	else if( SheetDescription::SCircuitInj == nCurrentSheet && false == bAtLeastOneInjCir )
	{
		nCurrentSheet = SheetDescription::SDistribution;
	}

	_SetCurrentSheet( nCurrentSheet );

	// Restore the previous row position.
	if( GetMaxRows() < lTopRow )
	{
		lTopRow = 1;
	}

	ShowCell( GetTopCol(), lTopRow, SS_SHOW_TOPLEFT );

	SetActiveCell( 0, 0 );

	// Register notification to receive messages from CSSheet.
	CMessageManager::MM_RegisterNotificationHandler( this, CMessageManager::SSheetNHFlags::SSheetNHF_MouseMove 
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDown
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonUp
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDblClk
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseRButtonDown
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseCaptureChanged
			| CMessageManager::SSheetNHFlags::SSheetNHF_ColWidthChanged
			| CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyDown );

	if( false == m_bPrinting && false == m_bExporting )
	{
		if( NULL == m_DragDropInfo.m_hHandOpen )
		{
			m_DragDropInfo.m_hHandOpen = AfxGetApp()->LoadIcon( IDI_HANDOPEN );
			SetCursor( SS_CURSORTYPE_DRAGDROPAREA, m_DragDropInfo.m_hHandOpen );
		}

		if( NULL == m_DragDropInfo.m_hHandClosed )
		{
			m_DragDropInfo.m_hHandClosed = AfxGetApp()->LoadIcon( IDI_HANDCLOSED );
		}

		if( NULL == m_DragDropInfo.m_hHandForbidden )
		{
			m_DragDropInfo.m_hHandForbidden = AfxGetApp()->LoadIcon( IDI_HANDFORBIDDEN );
		}

		m_DragDropInfo.m_mapFrozenColsWidth.clear();
		m_DragDropInfo.m_mapFrozenRowsWidth.clear();

		for( int iLoopSheet = SDistribution; iLoopSheet <= SCircuitInj; iLoopSheet++ )
		{
			_SetCurrentSheet( iLoopSheet );

			SS_COORD lFreezeCol, lFreezeRow;
			GetFreeze( &lFreezeCol, &lFreezeRow );

			int iSheetNumber = GetSheetNumber( (SheetDescription)iLoopSheet );
			m_DragDropInfo.m_mapFrozenColsWidth.insert( std::pair<int, long>( iSheetNumber, 0 ) );

			for( long lLoopCol = 1; lLoopCol <= lFreezeCol; lLoopCol++ )
			{
				m_DragDropInfo.m_mapFrozenColsWidth.at( iSheetNumber ) += GetColWidthInPixelsW( lLoopCol );
			}

			m_DragDropInfo.m_mapFrozenRowsWidth.insert( std::pair<int, long>( iSheetNumber, 0 ) );

			for( long lLoopRow = 1; lLoopRow <= lFreezeRow; lLoopRow++ )
			{
				m_DragDropInfo.m_mapFrozenRowsWidth.at( iSheetNumber ) += GetRowHeightInPixelsW( lLoopRow );
			}
		}
	}

	_SetCurrentSheet( nCurrentSheet );
	
	SetBool( SSB_REDRAW, TRUE );
}

void CSheetHMCalc::SaveState( void )
{
	// Only in normal mode.
	if( true == m_bPrinting || true == m_bExporting )
	{
		return;
	}

	// Save selected lines.
	CArray<long> arlSelectedRow;
	GetSelectedRows( &arlSelectedRow );
	m_rSavedState.m_vecSelectedHM.clear();

	for( int iLoop = 0; iLoop < arlSelectedRow.GetCount(); iLoop++ )
	{
		// Don't call 'GetpHM' because this method verify 'm_bInitialized' that will be here set to 'false' most of the time.
		if( arlSelectedRow.GetAt( iLoop ) < 1 || arlSelectedRow.GetAt( iLoop ) > GetMaxRows() )
		{
			continue;
		}

		CDS_HydroMod *pHM = (CDS_HydroMod *)GetCellParam( GetMaxCols(), arlSelectedRow.GetAt( iLoop ) );

		if( NULL != pHM )
		{
			m_rSavedState.m_vecSelectedHM.push_back( pHM );
		}
	}

	m_rSavedState.m_nCurrentSheet = m_nCurrentSheet;
	m_rSavedState.m_lLastEditedCol = m_lLastEditedCol;
	m_rSavedState.m_lLastEditedRow = m_lLastEditedRow;
	m_rSavedState.m_strCellText = m_strCellText;
	m_rSavedState.m_dCellDouble = m_dCellDouble;
	m_rSavedState.m_bCellModified = m_bCellModified;
	m_rSavedState.m_lpCellParam = m_lpCellParam;

	for( int iLoop = SheetDescription::SFirst; iLoop <= SheetDescription::SLast; iLoop++ )
	{
		m_rSavedState.m_arSheetGroupExpandedList[iLoop] = m_arSheetGroupExpandedList[iLoop];
		m_rSavedState.m_arptLeftTop[iLoop] = m_arptLeftTop[iLoop];
	}

	m_rSavedState.m_bIsSaved = true;
}

void CSheetHMCalc::RestoreState( void )
{
	// Only in normal mode.
	if( true == m_bPrinting || true == m_bExporting )
	{
		return;
	}

	m_lLastEditedCol = m_rSavedState.m_lLastEditedCol;
	m_lLastEditedRow = m_rSavedState.m_lLastEditedRow;
	m_strCellText = m_rSavedState.m_strCellText;
	m_dCellDouble = m_rSavedState.m_dCellDouble;
	m_bCellModified = m_rSavedState.m_bCellModified;
	m_lpCellParam = m_rSavedState.m_lpCellParam;
	_SetCurrentSheet( m_rSavedState.m_nCurrentSheet );

	// Restore selected line.
	bool fAtLeastOneSelectRowDone = false;

	for( int iLoop = 0; iLoop < ( int )m_rSavedState.m_vecSelectedHM.size(); iLoop++ )
	{
		if( true == SelectRow( m_rSavedState.m_vecSelectedHM[iLoop] ) )
		{
			fAtLeastOneSelectRowDone = true;
		}
	}

	m_rSavedState.m_vecSelectedHM.clear();

	// If no selected line...
	if( false == fAtLeastOneSelectRowDone )
	{
		SelectOneRow( RowDescription_Header::RD_Header_FirstCirc, 2 );
	}

	// Scroll must be done here because 'SelectOneRow' scroll to the first column.
	for( int iLoop = SheetDescription::SFirst; iLoop <= SheetDescription::SLast; iLoop++ )
	{
		m_arSheetGroupExpandedList[iLoop] = m_rSavedState.m_arSheetGroupExpandedList[iLoop];
		m_arptLeftTop[iLoop ] = m_rSavedState.m_arptLeftTop[iLoop];
	}

	if( false == m_bPrinting && false == m_bExporting )
	{
		_RestoreGroupStateAndScrolling();
	}

	m_rSavedState.m_bIsSaved = false;
}

CDS_HydroMod::eHMObj CSheetHMCalc::GetHMObj( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return CDS_HydroMod::eHMObj::eNone;
	}

	CDS_HydroMod *pHM = GetpHM( lRow );

	if( NULL == pHM )
	{
		return CDS_HydroMod::eHMObj::eNone;
	}

	CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eHMObj::eNone;

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		switch( lColumn )
		{
			// Distribution supply pipe.
			case CD_Distribution_SupplyPipeSize:
				eHMObj = CDS_HydroMod::eHMObj::eDistributionSupplyPipe;
				break;

			// Distribution return pipe.
			case CD_Distribution_ReturnPipeSize:
				eHMObj = CDS_HydroMod::eHMObj::eDistributionReturnPipe;
				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		switch( lColumn )
		{
			// CV.
			case CD_Circuit_CVName:
			case CD_Circuit_CVKvs:
			{
				if( true == pHM->IsCvExist( true ) )
				{
					eHMObj = CDS_HydroMod::eHMObj::eCV;
				}

				break;
			}

			// HYS-1676: Add Smart valve to Sheet HM Calc
			// Smart valve.
			case CD_Circuit_SmartValveName:
			case CD_Circuit_SmartValveControlMode:
			case CD_Circuit_SmartValveLocation:
			{
				if( true == pHM->IsSmartControlValveExist( true ) )
				{
					eHMObj = CDS_HydroMod::eHMObj::eSmartControlValve;
				}
				else if( true == pHM->IsSmartDpCExist( true ) )
				{
					eHMObj = CDS_HydroMod::eHMObj::eSmartDpC;
				}

				break;
			}

			// BV.
			case CD_Circuit_BvName:
				if( true == pHM->IsBvExist( true ) )
				{
					eHMObj = CDS_HydroMod::eHMObj::eBVprim;
				}

				break;

			// BV Bypass.
			case CD_Circuit_BvBypName:
				if( true == pHM->IsBvBypExist( true ) )
				{
					eHMObj = CDS_HydroMod::eHMObj::eBVbyp;
				}

				break;

			// DpC.
			case CD_Circuit_DpCName:
				if( true == pHM->IsDpCExist( true ) )
				{
					eHMObj = CDS_HydroMod::eHMObj::eDpC;
				}

				break;

			case CD_Circuit_SvName:
				if( true == pHM->IsShutoffValveExist( true ) && NULL != pHM->GetpSch() )
				{
					if( ShutoffValveLoc::ShutoffValveLocSupply == pHM->GetpSch()->GetShutoffValveLoc() )
					{
						eHMObj = CDS_HydroMod::eShutoffValveSupply;
					}
					else if( ShutoffValveLoc::ShutoffValveLocReturn == pHM->GetpSch()->GetShutoffValveLoc() )
					{
						eHMObj = CDS_HydroMod::eShutoffValveReturn;
					}
				}

				break;

			// Circuit Pipe.
			case CD_Circuit_PipeSize:
				eHMObj = CDS_HydroMod::eHMObj::eCircuitPrimaryPipe;
				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		switch( lColumn )
		{
			// CV.
			case CD_CircuitInjSecSide_CVName:
			case CD_CircuitInjSecSide_CVKvs:
			{
				if( true == pHM->IsCvExist( true ) )
				{
					eHMObj = CDS_HydroMod::eHMObj::eCV;
				}

				break;
			}

			// BV.
			case CD_CircuitInjSecSide_BvName:
				if( true == pHM->IsBvSecExist( true ) )
				{
					eHMObj = CDS_HydroMod::eHMObj::eBVsec;
				}

				break;

			// Circuit Pipe.
			case CD_CircuitInjSecSide_PipeSize:
				eHMObj = CDS_HydroMod::eHMObj::eCircuitSecondaryPipe;
				break;
		}
	}

	return eHMObj;
}

bool CSheetHMCalc::IsHMObjInColumn( long lColumn, CDS_HydroMod::eHMObj *peHMObjInColumn )
{
	if( false == m_bInitialized || lColumn < 1 || lColumn > GetMaxCols() || GetMaxRows() < RowDescription_Header::RD_Header_FirstCirc )
	{
		return false;
	}

	bool bExist = false;
	CDS_HydroMod::eHMObj eHMObject;

	for( long lLoopRow = RowDescription_Header::RD_Header_FirstCirc; lLoopRow <= GetMaxRows() && false == bExist; lLoopRow++ )
	{
		eHMObject = GetHMObj( lColumn, lLoopRow );

		if( CDS_HydroMod::eHMObj::eNone != eHMObject )
		{
			bExist = true;

			if( NULL != peHMObjInColumn )
			{
				*peHMObjInColumn = eHMObject;
			}
		}
	}

	return bExist;
}

CDS_HydroMod *CSheetHMCalc::GetpHM( long lRow )
{
	if( false == m_bInitialized || lRow < 1 || lRow > GetMaxRows() )
	{
		return NULL;
	}

	return ( (CDS_HydroMod *)GetCellParam( GetMaxCols(), lRow ) );
}

CDS_HydroMod *CSheetHMCalc::GetSelectedHM()
{
	if( false == m_bInitialized )
	{
		return NULL;
	}

	CArray <long> aRow;

	if( GetSelectedRows( &aRow ) != 1 )
	{
		return NULL;
	}

	long lRow = aRow.GetAt( 0 );

	if( lRow > GetMaxRows() )
	{
		return NULL;
	}

	CDS_HydroMod *pHM = GetpHM( lRow );

	if( NULL != pHM )		// is a row with HydroMod pointer ?
	{
		return pHM;
	}

	return NULL;
}

void CSheetHMCalc::GetAllHMVisible( std::vector<CDS_HydroMod *> *pvecAllHM )
{
	if( NULL == pvecAllHM )
	{
		return;
	}

	pvecAllHM->clear();

	if( GetMaxRows() < RD_Header_FirstCirc )
	{
		return;
	}

	for( int iLoopRow = RD_Header_FirstCirc; iLoopRow <= GetMaxRows(); iLoopRow++ )
	{
		CDS_HydroMod *pHM = GetpHM( iLoopRow );

		if( NULL != pHM )
		{
			pvecAllHM->push_back( pHM );
		}
	}
}

bool CSheetHMCalc::SelectRow( CDS_HydroMod *pHM )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	bool bSelectRowDone = false;
	long lColumn = GetMaxCols();

	for( long r = 1; r <= GetMaxRows(); r++ )
	{
		if( GetCellParam( lColumn, r ) == ( long )pHM )
		{
			SelectOneRow( r, 2 );
			ShowCell( 1, r, SS_SHOW_CENTERLEFT );
			SetActiveCell( 1, r );
			bSelectRowDone = true;
			break;
		}
	}

	return bSelectRowDone;
}

void CSheetHMCalc::EditHMRow()
{
	if( false == m_bInitialized )
	{
		return;
	}

	short nSheet = GetSheet();
	CDS_HydroMod *pHM = GetSelectedHM();

	if( NULL != pHM && NULL != m_pHM )
	{
		// 		MSG msg;
		// 		while( ::PeekMessage( &msg, GetSafeHwnd(), 0, 0, PM_REMOVE ) );

		// WARNING: Edit parent module, pointer could be modified by dialog circuit.
		CString ModuleName = m_pHM->GetHMName();
		BeginWaitCursor();
		CDlgWizCircuit dlg( this );

		// Prepare some variables to determine on which module to reset when coming back from 'CDlgWizCircuit'.
		bool bEditChild = ( pHM != m_pHM ) ? true : false;
		int iLevel = m_pHM->GetLevel();
		int iPos = m_pHM->GetPos();

		if( pHM != m_pHM )
		{
			if( true == dlg.Init( pHM, true, ( CTable ** )&m_pHM ) )
			{
				dlg.DisplayWizard();
			}
		}
		else
		{
			if( true == dlg.Init( pHM, true, ( CTable ** )&pHM ) )
			{
				dlg.DisplayWizard();
			}
		}

		// To deactivate all interaction coming from TSpread.
		m_bInitialized = false;

		// When returning from editing, pHM is not especially the same for instance if the user change circuit type!
		pHM = dlg.GetpCurrentHM();
		CDS_HydroMod *pHMToReset = NULL;

		if( true == bEditChild )
		{
			pHMToReset = pHM;
		}
		else
		{
			pHMToReset = ( iLevel != pHM->GetLevel() ) ? pHM->GetParent() : pHM;
		}

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)pHMToReset );
		}

		EndWaitCursor();
	}
}

void CSheetHMCalc::DeleteHMRow()
{
	if( false == m_bInitialized )
	{
		return;
	}

	// Multi-selected rows?
	CArray<long> aSelRows;
	int iCount = GetSelectedRows( &aSelRows );

	if( iCount < 1 )
	{
		return;
	}

	// Check first if in the list of selected lines, there is the parent. In this case, we remove the children from the selection.
	bool bParentExist = false;

	for( int i = 0; i < aSelRows.GetCount() && false == bParentExist; i++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)GetCellParam( GetMaxCols(), aSelRows.GetAt( i ) );

		if( NULL != pHM && pHM == m_pHM )
		{
			bParentExist = true;
		}
	}

	// Prepare the AFX message.
	CString str;

	if( true == bParentExist )
	{
		FormatString( str, IDS_DELETEMODULE, m_pHM->GetHMName() );
	}
	else if( iCount > 1 )
	{
		CString str1;
		str1.Format( _T("%d"), iCount );
		FormatString( str, IDS_HMCALC_DELETEALLCIRCUITS, str1 );
	}
	else
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)GetCellParam( GetMaxCols(), aSelRows.GetAt( 0 ) );

		if( NULL == pHM )
		{
			return;
		}

		if( true == pHM->IsaModule() )
		{
			FormatString( str, IDS_DELETEMODULE, pHM->GetHMName() );
		}
		else
		{
			FormatString( str, IDS_DELETEVALVE, pHM->GetHMName() );
		}
	}

	if( ::AfxMessageBox( ( LPCTSTR ) str, MB_YESNO | MB_DEFBUTTON2 | MB_ICONSTOP ) != IDYES )
	{
		return;
	}

	CDS_HydroMod *pHMToReset = NULL;

	if( true == bParentExist )
	{
		pHMToReset = dynamic_cast<CDS_HydroMod *>( m_pHM->GetIDPtr().PP );
	}
	else
	{
		pHMToReset = m_pHM;
	}

	BeginWaitCursor();

	bool bAtLeastOneDeleteDone = false;

	for( int i = 0; i < aSelRows.GetCount(); i++ )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)GetCellParam( GetMaxCols(), aSelRows.GetAt( i ) );

		if( NULL == pHM || ( true == bParentExist && pHM != m_pHM ) )
		{
			continue;
		}

		SetCellParam( GetMaxCols(), aSelRows.GetAt( i ), NULL );
		m_pTADS->DeleteHM( pHM );
		_ClearOneLineCombos( GetSheetDescription( m_nCurrentSheet ), aSelRows.GetAt( i ) );

		bAtLeastOneDeleteDone = true;
	}

	// Reset the HM tree.
	if( true == bAtLeastOneDeleteDone && pHMToReset != NULL )
	{
		UnSelectMultipleRows();
		pHMToReset->ComputeAll();
	}

	// To deactivate all interaction coming from TSpread.
	m_bInitialized = false;

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)pHMToReset );
	}

	EndWaitCursor();
}

bool CSheetHMCalc::IsMeasurementExist( CDS_HydroMod *pHM )
{
	if( false == m_bInitialized || NULL == pHM )
	{
		return false;
	}

	// Do a loop on the children of the module to see if Measurement exist
	for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)IDPtr.MP;

		if( NULL != pHM )
		{
			if( 0 != pHM->GetMeasDataSize() )
			{
				return true;
			}
		}
	}

	return false;
}

bool CSheetHMCalc::IsInjectionCircuitExist( CDS_HydroMod *pHM )
{
	if( false == m_bInitialized || NULL == pHM )
	{
		return false;
	}

	if( NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsSecondarySideExist() && NULL != pHM->GetpCircuitSecondaryPipe() )
	{
		return true;
	}

	// Do a loop on the children of the module to see if 2w injection circuit exist.
	for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext() )
	{
		CDS_HydroMod *pChildHM = (CDS_HydroMod *)IDPtr.MP;

		if( NULL != pChildHM && NULL != pChildHM->GetpSchcat() && true == pChildHM->GetpSchcat()->IsSecondarySideExist() && NULL != pChildHM->GetpCircuitSecondaryPipe() )
		{
			return true;
		}
	}

	return false;
}

bool CSheetHMCalc::IsEditAccAvailable( long lColumn, long lRow, CPipes *&pPipes )
{
	if( false == m_bInitialized )
	{
		return false;
	}

	pPipes = NULL;
	CDS_HydroMod *pHM = GetpHM( lRow );

	if( NULL == pHM )
	{
		return false;
	}

	bool bIsEditAccAvailable = false;

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		if( lColumn >= CD_Distribution_SupplyPipeSeries && lColumn <= CD_Distribution_SupplyAccDp )
		{
			// No distribution pipe accessories for main module.
			if( 0 != pHM->GetLevel() && NULL != pHM->GetpDistrSupplyPipe() )
			{
				bIsEditAccAvailable = true;
				pPipes = pHM->GetpDistrSupplyPipe() ;
			}
		}
		else if( lColumn >= CD_Distribution_ReturnPipeSeries && lColumn <= CD_Distribution_ReturnAccDp )
		{
			// No distribution pipe accessories for main module.
			if( 0 != pHM->GetLevel() && NULL != pHM->GetpDistrReturnPipe() )
			{
				bIsEditAccAvailable = true;
				pPipes = pHM->GetpDistrReturnPipe();
			}
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		if( lColumn >= CD_Circuit_PipeSerie && lColumn <= CD_Circuit_PipeAccDp )
		{
			// No circuit pipe accessories for a module except for injection module.
			if( ( false == pHM->IsaModule() || ( NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsSecondarySideExist() ) ) && NULL != pHM->GetpCircuitPrimaryPipe() )
			{
				bIsEditAccAvailable = true;
				pPipes = pHM->GetpCircuitPrimaryPipe();
			}
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		if( lColumn >= CD_CircuitInjSecSide_PipeSerie && lColumn <= CD_CircuitInjSecSide_AccDp )
		{
			// No circuit pipe accessories for a module.
			// HYS-1735: No accessories to edit for 3-way mixing circuit secondary pipe.
			if( false == pHM->IsaModule() && NULL != pHM->GetpCircuitSecondaryPipe() 
				&& CDB_CircSchemeCateg::e3wTypeMixing != pHM->GetpSchcat()->Get3WType() )
			{
				bIsEditAccAvailable = true;
				pPipes = pHM->GetpCircuitSecondaryPipe();
			}
		}
	}

	return bIsEditAccAvailable;
}

int CSheetHMCalc::GetSheetNumber( SheetDescription eSheet )
{
	if( false == m_bInitialized )
	{
		return -1;
	}

	int iWorkBookSheet = ( int )eSheet + ( m_iTabOffSet * SheetDescription::SLast );

	if( true == m_bReserverFirstSheet )
	{
		iWorkBookSheet++;
	}

	return iWorkBookSheet;
}

CSheetHMCalc::SheetDescription CSheetHMCalc::GetSheetDescription( int iSheetNumber )
{
	if( false == m_bInitialized )
	{
		return SheetDescription::SUndefined;
	}

	if( true == m_bReserverFirstSheet )
	{
		iSheetNumber--;
	}

	int iSheetDescription = ( ( iSheetNumber - 1 ) % ( int )( SheetDescription::SLast ) ) + 1;

	if( iSheetDescription < ( int )SheetDescription::SFirst || iSheetDescription > ( int )SheetDescription::SLast )
	{
		return SheetDescription::SUndefined;
	}

	return ( SheetDescription )iSheetDescription;
}

void CSheetHMCalc::ChangeSheet( bool bShiftRight )
{
	if( false == m_bInitialized || true == m_bPrinting || true == m_bExporting )
	{
		return;
	}

	short nSheetCount = GetSheetCount();

	if( nSheetCount < 2 )
	{
		return;
	}

	short nCurrentSheet = GetActiveSheet();
	short nNewSheet = nCurrentSheet;

	if( true == bShiftRight )
	{
		do
		{
			nNewSheet++;

			if( nNewSheet > nSheetCount )
			{
				nNewSheet = 1;
			}

			if( TRUE == GetSheetVisible( nNewSheet ) )
			{
				break;
			}
		}
		while( nNewSheet != nCurrentSheet );
	}
	else
	{
		do
		{
			nNewSheet--;

			if( nNewSheet < 1 )
			{
				nNewSheet = nSheetCount;
			}

			if( TRUE == GetSheetVisible( nNewSheet ) )
			{
				break;
			}
		}
		while( nNewSheet != nCurrentSheet );
	}

	_SetCurrentSheet( nNewSheet );
}

CDS_HydroMod *CSheetHMCalc::GetHMUnderMouse( CPoint point, long *plColCoord, long *plRowCoord )
{
	if( false == m_bInitialized || true == m_bPrinting || true == m_bExporting )
	{
		return NULL;
	}

	// Transform Screen coordinate in client coordinate.
	ScreenToClient( &point );

	// Retrieve cell from pixel.
	SS_COORD ColCoord, RowCoord;
	GetCellFromPixel( &ColCoord, &RowCoord, point.x, point.y );

	if( NULL != plColCoord )
	{
		*plColCoord = ColCoord;
	}

	if( NULL != plRowCoord )
	{
		*plRowCoord = RowCoord;
	}

	return dynamic_cast<CDS_HydroMod *>( (CData *)GetCellParam( GetMaxCols(), RowCoord ) );
}

CDB_TAProduct *CSheetHMCalc::GetTAProductUnderMouse( CPoint point )
{
	short nActiveSheet = GetActiveSheet();

	if( nActiveSheet < SheetDescription::SDistribution || nActiveSheet > SheetDescription::STADiagnostic )
	{
		return NULL;
	}

	// Transform Screen coordinate in client coordinate.
	ScreenToClient( &point );

	// Retrieve cell from pixel.
	SS_COORD ColCoord, RowCoord;
	GetCellFromPixel( &ColCoord, &RowCoord, point.x, point.y );

	// First of all, is there a product on the line?
	CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)GetCellParam( GetMaxCols(), RowCoord ) );

	if( NULL == pHM )
	{
		return NULL;
	}

	CDB_TAProduct *pTAProduct = NULL;
	IDPTR ValveIDPtr = _NULL_IDPTR;

	switch( nActiveSheet )
	{
		case SheetDescription::SDistribution:
			break;

		case SheetDescription::SCircuit:
			switch( ColCoord )
			{
				case CD_Circuit_CVName:
					if( true == pHM->IsCvExist( true ) )
					{
						ValveIDPtr = pHM->GetpCV()->GetCvIDPtr();
					}

					break;
				
				// HYS-1676: Add Smart valve to Sheet HM Calc.
				case CD_Circuit_SmartValveName:
				case CD_Circuit_SmartValveControlMode:
				case CD_Circuit_SmartValveLocation:
					if( true == pHM->IsSmartControlValveExist( true ) )
					{
						ValveIDPtr = pHM->GetpSmartControlValve()->GetIDPtr();
					}
					else if( true == pHM->IsSmartDpCExist( true ) )
					{
						ValveIDPtr = pHM->GetpSmartDpC()->GetIDPtr();
					}

					break;

				case CD_Circuit_BvName:
					if( true == pHM->IsBvExist( true ) )
					{
						ValveIDPtr = pHM->GetpBv()->GetIDPtr();
					}

					break;

				case CD_Circuit_DpCName:
					if( true == pHM->IsDpCExist( true ) )
					{
						ValveIDPtr = pHM->GetpDpC()->GetIDPtr();
					}

					break;

				case CD_Circuit_BvBypName:
					if( true == pHM->IsBvBypExist( true ) )
					{
						ValveIDPtr = pHM->GetpBypBv()->GetIDPtr();
					}

				case CD_Circuit_SvName:
					if( true == pHM->IsShutoffValveExist( true ) && NULL != pHM->GetpSch() )
					{
						CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

						if( ShutoffValveLoc::ShutoffValveLocSupply == pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveSupply;
						}
						else if( ShutoffValveLoc::ShutoffValveLocReturn == pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveReturn;
						}

						ValveIDPtr = pHM->GetpShutoffValve( eHMObj )->GetIDPtr();
					}

					break;
			}

			break;

		case SheetDescription::SCircuitInj:
		{
			if( CD_CircuitInjSecSide_CVName == ColCoord )
			{
				if( true == pHM->IsCvExist( true ) )
				{
					ValveIDPtr = pHM->GetpCV()->GetCvIDPtr();
				}
			}

			if( CD_CircuitInjSecSide_BvName == ColCoord )
			{
				if( true == pHM->IsBvSecExist( true ) )
				{
					ValveIDPtr = pHM->GetpSecBv()->GetIDPtr();
				}
			}
		}
		break;

		case SheetDescription::SMeasurement:
			if( CD_Measurement_Valve == ColCoord )
			{
				// Same code as in 'FillMeasurementSheet' method when filling each row with valve and its measurements.
				ValveIDPtr = pHM->GetTADBValveIDPtr();

				if( false == pHM->IsBvExist() && true == pHM->IsCvExist( true ) )
				{
					ValveIDPtr = pHM->GetpCV()->GetCvIDPtr();
				}
				else if( true == pHM->IsBvExist( true ) )
				{
					ValveIDPtr = pHM->GetpBv()->GetIDPtr();
				}
			}

			break;

		case SheetDescription::STADiagnostic:
			if( CD_TADiagnostic_Valve == ColCoord )
			{
				// Same code as in 'FillTADiagnosticSheet' method when filling each row with valve and its measurements.
				ValveIDPtr = pHM->GetTADBValveIDPtr();

				if( false == pHM->IsBvExist() && true == pHM->IsCvExist( true ) )
				{
					ValveIDPtr = pHM->GetpCV()->GetCvIDPtr();
				}
				else if( true == pHM->IsBvExist( true ) )
				{
					ValveIDPtr = pHM->GetpBv()->GetIDPtr();
				}
			}

			break;
	}

	pTAProduct = dynamic_cast<CDB_TAProduct *>( ValveIDPtr.MP );
	return pTAProduct;
}

void CSheetHMCalc::GetAllColumnWidth( mapMapShortLong &mapSSheetColumnWidth )
{
	if( false == m_bInitialized || true == m_bPrinting || true == m_bExporting )
	{
		return;
	}

	if( m_mapSSheetColumnWidth.size() > 0 )
	{
		for( mapMapShortLongIter mapSheetIter = m_mapSSheetColumnWidth.begin(); mapSheetIter != m_mapSSheetColumnWidth.end(); ++mapSheetIter )
		{
			for( mapShortLongIter mapColumnIter = mapSheetIter->second.begin(); mapColumnIter != mapSheetIter->second.end(); ++mapColumnIter )
			{
				mapSSheetColumnWidth[mapSheetIter->first][mapColumnIter->first] = mapColumnIter->second;
			}
		}
	}
}

void CSheetHMCalc::SetAllColumnWidth( mapMapShortLong &mapSSheetColumnWidth )
{
	if( false == m_bInitialized || true == m_bPrinting || true == m_bExporting )
	{
		return;
	}

	m_mapSSheetColumnWidth.clear();

	if( mapSSheetColumnWidth.size() > 0 )
	{
		for( mapMapShortLongIter mapSheetIter = mapSSheetColumnWidth.begin(); mapSheetIter != mapSSheetColumnWidth.end(); ++mapSheetIter )
		{
			for( mapShortLongIter mapColumnIter = mapSheetIter->second.begin(); mapColumnIter != mapSheetIter->second.end(); ++mapColumnIter )
			{
				m_mapSSheetColumnWidth[mapSheetIter->first][mapColumnIter->first] = mapColumnIter->second;
			}
		}
	}
}

bool CSheetHMCalc::IsEditModeActive( void )
{
	if( false == m_bInitialized || true == m_bPrinting || true == m_bExporting )
	{
		return false;
	}

	bool bReturn = false;

	if( GetSheetDescription( m_nCurrentSheet ) >= SheetDescription::SDistribution && GetSheetDescription( m_nCurrentSheet ) <= STADiagnostic )
	{
		bReturn = ( TRUE == GetEditMode() ) ? true : false;
	}

	return bReturn;
}

void CSheetHMCalc::CloseDialogSCB( CDialogEx *pDlgSCB, bool bModified )
{
	if( false == m_bInitialized )
	{
		return;
	}

	// HYS-1443 : Don't need to call ComputeAll when the actuator change with DlgSelectActuator
	bool bForDlgActuator = false;

	CDS_HydroMod *pHM = NULL;

	if( NULL != pDlgSCB )
	{
		if( pDlgSCB == m_pSCB )
		{
			pHM = ( ( CDlgComboBoxHM * )pDlgSCB )->GetSelectedHM();
		}
		else if( pDlgSCB == m_pHMSelectActuator )
		{
			pHM = ( ( CDlgSelectActuator * ) m_pHMSelectActuator )->GetSelectedHM();
			bForDlgActuator = true;
		}
	}

	if( NULL == pHM )
	{
		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)m_pHM );
		}

		return;
	}

	if( true == bModified )
	{
		BeginWaitCursor();
		pHM->ComputeHM( CDS_HydroMod::eceResize );
		// HYS-1443 : Don't need to call ComputeAll when the actuator change with DlgSelectActuator
		if( false == bForDlgActuator )
		{
			pHM->ComputeAll();
		}
		else
		{
			// HYS-1684: Actuator name is not updated in the output window when the actuator changes in sheet HMCalc.
			CDS_ProjectParams* pPrjParam = TASApp.GetpTADS()->GetpProjectParams();

			if( NULL != pPrjParam && true == pPrjParam->GetHNAutomaticCheck() )
			{
				// We call verifyAllRootModules() to put output messages of other root modules.
				TASApp.GetpTADS()->VerifyAllRootModules();
			}
		}

		// To deactivate all interaction coming from TSpread.
		m_bInitialized = false;

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)m_pHM );
		}

		EndWaitCursor();
	}
}

void CSheetHMCalc::SetFlagCut( std::vector<CDS_HydroMod *> *pvecHydromod )
{
	m_vecCutRow.clear();

	if( NULL == pvecHydromod || 0 == ( int )pvecHydromod->size() )
	{
		return;
	}

	for( int i = 0; i < ( int )pvecHydromod->size(); i++ )
	{
		if( NULL != pvecHydromod->at( i ) )
		{
			m_vecCutRow.push_back( pvecHydromod->at( i ) );

			if( true == pvecHydromod->at( i )->IsaModule() )
			{
				_AddChildrenInCutList( pvecHydromod->at( i ) );
			}
		}
	}
}

bool CSheetHMCalc::ResetFlagCut( void )
{
	bool bAtLeastOneItemChanged = ( m_vecCutRow.size() > 0 ) ? true : false;
	m_vecCutRow.clear();
	return bAtLeastOneItemChanged;
}

bool CSheetHMCalc::IsFlagCutSet( void )
{
	return ( m_vecCutRow.size() > 0 ) ? true : false;
}

bool CSheetHMCalc::IsObjectInCutMode( CData *pObject )
{
	bool bInCutMode = false;

	if( m_vecCutRow.size() > 0 )
	{
		for( int i = 0; i < ( int )m_vecCutRow.size() && false == bInCutMode; i++ )
		{
			if( NULL != m_vecCutRow[i] && pObject == (CData *)m_vecCutRow[i] )
			{
				bInCutMode = true;
			}
		}
	}

	return bInCutMode;
}

void CSheetHMCalc::ComboDropDown( long lColumn, long lRow )
{
	if( false == m_bInitialized || true == GetpParentHM()->IsForHub() )
	{
		return;
	}

	CDS_HydroMod *pHM = GetpHM( lRow );
	ASSERT( NULL != m_pSCB );		// Combo Box not created.

	if( NULL == pHM || NULL == m_pSCB )
	{
		return;
	}

	BeginWaitCursor();

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		switch( lColumn )
		{
			case CD_Distribution_SupplyPipeSeries:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DistributionSupplyPipeSerie, lColumn, lRow );
				break;

			case CD_Distribution_SupplyPipeSize:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DistributionSupplyPipeSize, lColumn, lRow );
				break;

			case CD_Distribution_ReturnPipeSeries:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DistributionReturnPipeSerie, lColumn, lRow );
				break;

			case CD_Distribution_ReturnPipeSize:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DistributionReturnPipeSize, lColumn, lRow );
				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		switch( lColumn )
		{
			case CD_Circuit_CVKvs:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CvKvs, lColumn, lRow );
				break;

			case CD_Circuit_CVName:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_Cv, lColumn, lRow );
				break;

			case CD_Circuit_ActuatorName:
				if( NULL != m_pHMSelectActuator )
				{
					m_pHMSelectActuator->OpenDlgSelectActr( pHM, lColumn, lRow );
				}

				break;

			// HYS-1676: Add smart valve to Sheet HM Calc
			case CD_Circuit_SmartValveName:
				if( true == pHM->IsSmartControlValveExist() )
				{
					m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_SmartControlValve, lColumn, lRow );
				}
				else if( true == pHM->IsSmartDpCExist() )
				{
					m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_SmartDpC, lColumn, lRow );
				}
				break;

			case CD_Circuit_BvName:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_BvPName, lColumn, lRow );
				break;

			case CD_Circuit_SvName:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_ShutOffValve, lColumn, lRow );
				break;

			case CD_Circuit_DpCName:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DpC, lColumn, lRow );
				break;

			case CD_Circuit_BvBypName:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_BvBName, lColumn, lRow );
				break;

			case CD_Circuit_PipeSerie:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSerie, lColumn, lRow );
				break;

			case CD_Circuit_PipeSize:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSize, lColumn, lRow );
				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		switch( lColumn )
		{
			case CD_CircuitInjSecSide_CVKvs:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CvKvs, lColumn, lRow );
				break;

			case CD_CircuitInjSecSide_CVName:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_Cv, lColumn, lRow );
				break;

			case CD_CircuitInjSecSide_BvName:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_BvSName, lColumn, lRow );
				break;

			case CD_CircuitInjSecSide_PipeSerie:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CircuitSecondaryPipeSerie, lColumn, lRow );
				break;

			case CD_CircuitInjSecSide_PipeSize:
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CircuitSecondaryPipeSize, lColumn, lRow );
				break;
		}
	}

	EndWaitCursor();
}

void CSheetHMCalc::DragDropBlock( long lOldColumnFrom, long lOldRowFrom, long lOldColumnTo, long lOldRowTo, long lNewColumFrom, long lNewRowFrom,
		long lNewColumnTo, long lNewRowTo, BOOL *pbCancel )
{
	// Don't use spread Drag&Drop.
	if( pbCancel )
	{
		*pbCancel = true;
	}

	if( false == m_bInitialized )
	{
		return;
	}

	// Only vertical drag and drop.
	if( lOldColumnFrom != lOldColumnTo || lOldColumnFrom != lNewColumnTo )
	{
		return;
	}

	// One cell at a time.
	if( lOldRowFrom != lOldRowTo )
	{
		return;
	}

	// Update member variable m_CellDouble and m_CellText.
	CDS_HydroMod *pclHMSource = GetSelectedHM();

	if( NULL == pclHMSource )
	{
		return;
	}

	BeginWaitCursor();
	SetREDRAW( FALSE );

	bool bComputeAllNeeded = true;
	bool bResetTreeNeeded = true;
	lOldRowFrom = max( RD_Header_FirstCirc, lOldRowFrom );
	lNewRowFrom = max( RD_Header_FirstCirc, lNewRowFrom );

	if( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) && CD_Distribution_Pos == lOldColumnFrom ) 
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) && CD_Circuit_Pos == lOldColumnFrom ) 
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) && CD_CircuitInjSecSide_Pos == lOldColumnFrom ) )
	{
		// Change circuit position.
		// HYS-2004: Verify if the destination has a valid HM.
		if( NULL != GetpHM( lNewRowFrom ) )
		{
			m_pTADS->ChangePosition( pclHMSource, GetpHM( lNewRowFrom )->GetPos() );
			m_lLastEditedRow = lNewRowFrom;
		}
	}
	else if( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) && CD_Distribution_SupplyAccDesc == lOldColumnFrom ) 
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) && CD_Distribution_ReturnAccDesc == lOldColumnFrom ) 
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) && CD_Circuit_PipeAccDesc == lOldColumnFrom ) 
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) && CD_CircuitInjSecSide_AccDesc == lOldColumnFrom ) )
	{
		// Copy accessories.
		CPipes *pPipe = NULL;

		if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
		{
			if( CD_Distribution_SupplyAccDesc == lOldColumnFrom )
			{
				pPipe = pclHMSource->GetpDistrSupplyPipe();
			}
			else
			{
				pPipe = pclHMSource->GetpDistrReturnPipe();
			}
		}
		else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
		{
			pPipe = pclHMSource->GetpCircuitPrimaryPipe();
		}
		else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
		{
			pPipe = pclHMSource->GetpCircuitSecondaryPipe();
		}

		if( NULL != pPipe )
		{
			for( int i = min( lOldRowFrom, lNewRowFrom ); i <= max( lOldRowFrom, lNewRowFrom ); i++ )
			{
				if( i == lOldRowFrom )
				{
					continue;
				}

				m_lLastEditedRow = i;
				CDS_HydroMod *pTrgHM = GetpHM( i );

				if( NULL != pTrgHM )
				{
					// Drop on the main module is not allowed (there is no pipe).
					if( 0 == pTrgHM->GetLevel() )
					{
						continue;
					}

					CPipes *pTrgPipe = NULL;

					if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == pPipe->GetLocate() )
					{
						pTrgPipe = pTrgHM->GetpDistrSupplyPipe();
					}
					else if( CDS_HydroMod::eHMObj::eDistributionReturnPipe == pPipe->GetLocate() )
					{
						pTrgPipe = pTrgHM->GetpDistrReturnPipe();
					}
					else if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == pPipe->GetLocate() )
					{
						// Drop on a module is not allowed for circuit pipe (there is no pipe).
						if( true == pTrgHM->IsaModule() )
						{
							continue;
						}

						pTrgPipe = pTrgHM->GetpCircuitPrimaryPipe();
					}
					else if( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe == pPipe->GetLocate() )
					{
						// Drop on a module is not allowed for circuit pipe (there is no pipe).
						if( true == pTrgHM->IsaModule() )
						{
							continue;
						}

						pTrgPipe = pTrgHM->GetpCircuitSecondaryPipe();
					}

					if( NULL != pTrgPipe )
					{
						pTrgPipe->RemoveAllSingularities();
						pPipe->CopyAllSingularities( pTrgPipe );
					}
				}
			}
		}
	}
	else
	{
		// User editable drag and drop operation.
		EditChange( lOldColumnFrom, lOldRowFrom );

		if( true == m_bCellModified )
		{
			if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
			{
				if( CD_Distribution_Desc == lOldColumnFrom )
				{
					bComputeAllNeeded = false;
					bResetTreeNeeded = false;
				}
			}
			else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
			{
				if( CD_Circuit_Desc == lOldColumnFrom || CD_Circuit_UnitDesc == lOldColumnFrom )
				{
					bComputeAllNeeded = false;
					bResetTreeNeeded = false;
				}
			}
			else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
			{
				if( CD_CircuitInjSecSide_Desc == lOldColumnFrom )
				{
					bComputeAllNeeded = false;
					bResetTreeNeeded = false;
				}
			}

			// Unselect row before processing.
			UnSelectMultipleRows();

			// Prepare HCO to receive drag and drop warning
			CDlgHMCompilationOutput::CHMInterface clOutputInterface;
			clOutputInterface.ClearOutput();
			clOutputInterface.BlockOutput();

			bool bAtLeastOneChange = false;

			for( int i = min( lOldRowFrom, lNewRowFrom ); i <= max( lOldRowFrom, lNewRowFrom ); i++ )
			{
				if( i == lOldRowFrom )
				{
					continue;
				}

				CDS_HydroMod *pclHMDest = dynamic_cast<CDS_HydroMod *>( (CData *)GetCellParam( GetMaxCols(), i ) );

				if( true == _ApplyDragAndDrop( pclHMSource, pclHMDest, lOldColumnFrom, i ) )
				{
					bAtLeastOneChange = true;
				}	
			}

			// To avoid to compute and reset if no drag & drop has been applied.
			bComputeAllNeeded &= bAtLeastOneChange;
			bResetTreeNeeded &= bAtLeastOneChange;

			clOutputInterface.ReleaseOutput();
		}
	}

	m_bCellModified = false;

	SetREDRAW( TRUE );
	Invalidate();
	_SelectRow( m_lLastEditedRow );

	if( true == bComputeAllNeeded )
	{
		// Temporary disable message produced by ComputeAll()
		CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
		bool SaveAutoCheck = pPrjParam->GetHNAutomaticCheck();
		pPrjParam->SetHNAutomaticCheck( false );
		pclHMSource->ComputeAll();
		pPrjParam->SetHNAutomaticCheck( SaveAutoCheck );
	}

	if( true == bResetTreeNeeded )
	{
		// To deactivate all interaction coming from TSpread.
		m_bInitialized = false;

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)m_pHM );
		}
	}

	EndWaitCursor();
}

void CSheetHMCalc::EditChange( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	m_lLastEditedRow = lRow;
	m_lLastEditedCol = lColumn;
	CString str;
	str = GetCellText( lColumn, lRow );
	m_dCellDouble = 0.0;
	m_strCellText = _T( "" );
	m_lpCellParam = NULL;
	m_bCellModified = false;

	// HYS-1307: Enable to use "-" sign for temperature.
	bool bIsForTemperature = false;
	
	if( lColumn == CD_CircuitInjSecSide_TempTss )
	{
		bIsForTemperature = true;
	}

	if( true == IsCellProperty( lColumn, lRow, CellText ) )
	{
		m_dCellDouble = 0.0;
		m_strCellText = str;
		m_bCellModified = true;
	}
	else if( true == IsCellProperty( lColumn, lRow, CellDouble ) )
	{
		double dValue;

		if( true == str.IsEmpty() )
		{
			str = _T("0.0");
		}

		switch( ReadDouble( str, &dValue ) )
		{
			case RD_NOT_NUMBER:
			{
				// Number is uncompleted ?
				// try to remove partial engineering notation if exist .
				int i = str.FindOneOf( _T("eE") );

				if( i != -1 )
				{
					str.Delete( i );

					if( RD_OK == ReadDouble( str, &dValue ) )
					{
						break;
					}
				}

				i = str.FindOneOf( _T("-+") );

				if( i != -1 )
				{
					str.Delete( i );

					// HYS-1307: If "-" sign is first allow to continue ( Not set the cell with the string without "-" sign ).
					if( true == bIsForTemperature && true == str.IsEmpty() )
					{
						str = _T( "0.0" );
					}

					if( RD_OK == ReadDouble( str, &dValue ) )
					{
						break;
					}
				}

				SetCellText( lColumn, lRow, m_strCellText );
			}
			break;

			case RD_EMPTY:
				break;

			case RD_OK:

				// Check validity field.
				if( true == _CheckValidity( dValue ) )
				{
					m_dCellDouble = dValue;
					m_strCellText = str;
					m_bCellModified = true;
				}

				break;
		}
	}
	else if( SS_TYPE_COMBOBOX == GetCellTypeW( lColumn, lRow ) )
	{
		m_lpCellParam = GetCellParam( m_lLastEditedCol, m_lLastEditedRow );

		if( m_lpCellParam )
		{
			m_bCellModified = true;
		}
	}
}

void CSheetHMCalc::LeaveCell( long lOldColumn, long lOldRow, long lNewColumn, long lNewRow, BOOL *pfCancel )
{
	if( NULL != pfCancel )
	{
		*pfCancel = TRUE;
	}

	if( false == m_bInitialized )
	{
		return;
	}

	if( true == m_bComboEditMode && false == m_bCellModified )
	{
		m_bComboEditMode = false;

		// Nothing change.
		CDS_HydroMod *pHM = GetpHM( lOldRow );

		if( NULL != pHM && true == pHM->IsForHub() )
		{
			return;
		}

		if( NULL != pHM && true == pHM->IsCvExist( true ) && false == pHM->GetpCV()->IsTaCV() &&
			( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) && lOldColumn == CD_Circuit_CVKvs ) ||
			  m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) && lOldColumn == CD_CircuitInjSecSide_CVKvs ) )
		{
			if( GetCellTypeW( lOldColumn, lOldRow ) != SS_TYPE_COMBOBOX )
			{
				SaveTextPattern( lOldColumn, lOldRow );
				FormatEditCombo( lOldColumn, lOldRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, (LPARAM)pHM->GetpCV() );
				RestoreTextPattern( lOldColumn, lOldRow );
			}
		}
	}

	// Leave a modified cell compute depending parameters.
	if( true == m_bCellModified )
	{
		m_bCellModified = false;

		if( true == m_bComboEditMode )
		{
			m_bComboEditMode = false;
		}

		bool bRet = _SaveAndCompute( lOldColumn, lOldRow );

		if( true == bRet )
		{
			if( NULL != pDlgLeftTabProject )
			{
				pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)m_pHM );
			}

			return;
		}
	}

	if( -1 == lNewColumn || -1 == lNewRow )
	{
		return;
	}

	BOOL bCancel = FALSE;

	if( lNewRow != lOldRow )
	{
		if( lNewRow == lOldRow + 1 )
		{
			bCancel = ( true == _ArrowKeyDown( lOldRow ) ) ? FALSE : TRUE;
		}
		else if( lNewRow == lOldRow - 1 )
		{
			bCancel = ( true == _ArrowKeyUp( lOldRow ) ) ? FALSE : TRUE;
		}
		else
		{
			bCancel = ( true == _SelectRow( lNewRow ) ) ? FALSE : TRUE;
		}
	}

	if( NULL != pfCancel )
	{
		*pfCancel = bCancel;
	}
}

void CSheetHMCalc::PrepareforExport()
{
	if( false == m_bInitialized )
	{
		return;
	}

	// Run all sheets.
	for( int iLoopSheet = 0; iLoopSheet < GetSheetCount(); iLoopSheet++ )
	{
		SetSheet( iLoopSheet + 1 );

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// CODE TO CLARIFY -> WHAT IS THE AIM ?

		// Run all columns from the last to the first.
		for( long lLoopCol = GetMaxCols(); lLoopCol > 0; lLoopCol-- )
		{
			// Don't check if column is not hidden.
			if( FALSE == IsColHidden( lLoopCol ) )
			{
				continue;
			}

			// If current column (at row 1) does not belong to a span, we can continue with the next column.
			SS_COORD lColAnchor, lRowAnchor, lNumCols, lNumRows;

			if( SS_SPAN_NO == GetCellSpan( lLoopCol, 1, &lColAnchor, &lRowAnchor, &lNumCols, &lNumRows ) )
			{
				continue;
			}

			// If current column is not the first one, we can continue with the next column.
			if( lLoopCol > lColAnchor )
			{
				continue;
			}

			if( lNumCols > 1 )
			{
				RemoveCellSpan( lColAnchor, lRowAnchor );
				AddCellSpan( lLoopCol + 1, 1, lNumCols - 1, lNumRows );

				// Copy current cell in the next column (to not lost span and content for the group).
				CopyRange( lLoopCol, 1, lLoopCol, 1, lLoopCol + 1, 1 );
			}

			DelCol( lLoopCol );
		}

		//
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// When exporting, the first column just next to the frozen one is automatically set with a predefined width.
		// To avoid that first column in each page be shorter, we insert a fake column.
		InsCol( CD_Distribution_Pos + 1 );
		SetColWidthInPixels( CD_Distribution_Pos + 1, 0 );
	}
}

void CSheetHMCalc::SheetChanged( short nOldSheet, short nNewSheet )
{
	if( false == m_bInitialized )
	{
		return;
	}

	CArray<long> arlSelRows;

	GetSelectedRows( &arlSelRows );
	UnSelectMultipleRows();
	int iOldRow = GetMaxRows();
	_SetCurrentSheet( nNewSheet );
	int iNewRow = GetMaxRows();

	SetREDRAW( FALSE );

	bool bAtLeastOneRowSelected = false;

	for( int i = 0; i < arlSelRows.GetCount(); i++ )
	{
		// Verify first if row can be selected.
		if( false == _IsRowSelectable( arlSelRows.GetAt( i ) ) )
		{
			continue;
		}

		bAtLeastOneRowSelected = true;
		SelectOneRow( arlSelRows.GetAt( i ), 2 );

		if( i == ( arlSelRows.GetCount() - 1 ) )
		{
			SetActiveCell( 0, arlSelRows.GetAt( i ) );
		}
	}

	// If when changing sheet, previous selection can't be done on the similar rows...
	// Remark: can be the case with 'CircuitInj' sheet.
	if( arlSelRows.GetCount() > 0 && false == bAtLeastOneRowSelected )
	{
		// Try to set selection on the first selectable row.
		for( long lLoopRow = RD_Header_FirstCirc; lLoopRow <= GetMaxRows(); lLoopRow++ )
		{
			if( false == _IsRowSelectable( lLoopRow ) )
			{
				continue;
			}

			SelectOneRow( lLoopRow, 2 );
			SetActiveCell( 0, lLoopRow );
			break;
		}
	}

	SetREDRAW( TRUE );
	Invalidate();
}

void CSheetHMCalc::TextTipFetch( long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pbShowTip )
{
	CDS_HydroMod *pHM = NULL;
	CString str;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;
	*pbShowTip = FALSE;

	if( false == m_bInitialized )
	{
		return;
	}

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		pHM = GetpHM( lRow );

		// HYS-1411: Verify that supply or return pipe exist before to get singularity on it.
		if( NULL != pHM )
		{
			if( lColumn > CD_Distribution_Pos && lColumn < CD_Distribution_SupplyAccDesc )
			{
				if( true == CSSheet::IsTextTruncated( lColumn, lRow ) )
				{
					str = GetCellText( lColumn, lRow );
				}
			}
			else if( lColumn >= CD_Distribution_SupplyAccDesc && lColumn <= CD_Distribution_SupplyAccDp && NULL != pHM->GetpDistrSupplyPipe() )
			{
				str = pHM->GetpDistrSupplyPipe()->GetSingularityListFull( true, true, true, true, HMCALC_ACCDISPMAX );
			}
			else if( lColumn >= CD_Distribution_ReturnAccDesc && lColumn <= CD_Distribution_ReturnAccDp && NULL != pHM->GetpDistrReturnPipe() )
			{
				str = pHM->GetpDistrReturnPipe()->GetSingularityListFull( true, true, true, true, HMCALC_ACCDISPMAX );
			}

			// HYS-1878: 1- Display a tooltip message when the mouse is over the red error value.
			if( lColumn == CD_Distribution_SupplyPipeVelocity || lColumn == CD_Distribution_ReturnPipeVelocity )
			{
				CPipes* pPipe = NULL;
				if( lColumn == CD_Distribution_SupplyPipeVelocity )
				{
					pPipe = pHM->GetpDistrSupplyPipe();
				}
				else
				{
					pPipe = pHM->GetpDistrReturnPipe();
				}

				if( NULL != pPipe && eb3False == pPipe->CheckVelocityMinMax() 
					&& NULL != pPipe->GetpParent() && NULL != pPipe->GetpParent()->GetpTechParam() )
				{
					CString str2 = CString( _T("[") ) + WriteCUDouble( _U_VELOCITY, pPipe->GetpParent()->GetpTechParam()->GetPipeMinVel() );
					str2 += CString( _T(" - ") ) + WriteCUDouble( _U_VELOCITY, pPipe->GetpParent()->GetpTechParam()->GetPipeMaxVel() );
					str2 += CString( _T("]") );
					FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );
				}
			}
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		pHM = GetpHM( lRow );

		if( NULL != pHM )
		{
			if( CD_Circuit_Pos == lColumn )
			{
				str = pHM->GetHMName();
			}
			else if( CD_Circuit_UnitQ == lColumn )
			{
				// For the moment, we manage the tooltip only for the smart control valve and the smart differential pressure controller.
				if( true == pHM->IsSmartControlValveExist() )
				{
					str = pHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow );
				}
				else if( true == pHM->IsSmartDpCExist() )
				{
					str = pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
							CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow );
				}
			}
			else if( CD_Circuit_UnitP == lColumn || CD_Circuit_UnitDT == lColumn )
			{
				// For the moment, we manage the tooltip only for the smart control valve.
				if( true == pHM->IsSmartControlValveExist() )
				{
					str = pHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Power );
				}
			}
			else if( CD_Circuit_SmartValveName == lColumn )
			{
				// This column is common to smart control valve and smart differential pressure controller.
				if( true == pHM->IsSmartControlValveExist() )
				{
					str = pHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All );
				}
				else if( true == pHM->IsSmartDpCExist() )
				{
					str = pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
							CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All );
				}
			}
			else if( CD_Circuit_SmartValveDp == lColumn )
			{
				// "Dp" is for smart control valve and smart differential pressure controller.
				if( true == pHM->IsSmartControlValveExist() )
				{
					str = pHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp );
				}
				else if( true == pHM->IsSmartDpCExist() )
				{
					str = pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
							CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp );
				}
			}
			else if( lColumn == CD_Circuit_SmartValveMoreInfo && true == pHM->IsSmartControlValveExist() )
			{
				// Info is only for the smart control valve (We don't show details for smart differential pressure controller).
				CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( pHM->GetpSmartControlValve()->GetIDPtr().MP );
				
				if( NULL != pclSmartControlValve )
				{
					str = TASApp.LoadLocalizedString( IDS_HMCALC_POWERSUPPLY ) + _T(": ") + pclSmartControlValve->GetPowerSupplyStr();
					str += _T("\n") + TASApp.LoadLocalizedString( IDS_HMCALC_INPUTSIGNAL ) + _T(": ") + pclSmartControlValve->GetInOutSignalsStr( true );
					str += _T("\n") + TASApp.LoadLocalizedString( IDS_HMCALC_OUTPUTSIGNAL ) + _T(": ") + pclSmartControlValve->GetInOutSignalsStr( false );
				}
			}
			else if( lColumn == CD_Circuit_SmartValveProductSet )
			{
				// Dp sensor set/connection set is only for smart differential pressure controller.
				if( true == pHM->IsSmartDpCExist() )
				{
					str = pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor, CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All );
				}
			}
			else if( lColumn == CD_Circuit_SmartValveDpl )
			{
				// "Dpl" is only for smart differential pressure controller (We don't show "Dpl" for smart control valve).
				if( true == pHM->IsSmartDpCExist() )
				{
					str = pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor, CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dpl );
				}
			}
			else if( lColumn == CD_Circuit_PipeV )
			{
				// HYS-1878: 1- Display a tooltip message when the mouse is over the red error value.
				CPipes *pPipe = pHM->GetpCircuitPrimaryPipe();

				if( NULL != pPipe && eb3False == pPipe->CheckVelocityMinMax()
						&& NULL != pPipe->GetpParent() && NULL != pPipe->GetpParent()->GetpTechParam() )
				{
					CString str2 = CString( _T("[") ) + WriteCUDouble( _U_VELOCITY, pPipe->GetpParent()->GetpTechParam()->GetPipeMinVel() );
					str2 += CString( _T(" - ") ) + WriteCUDouble( _U_VELOCITY, pPipe->GetpParent()->GetpTechParam()->GetPipeMaxVel() );
					str2 += CString( _T("]") );
					FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );
				}
			}
			else if( lColumn >= CD_Circuit_PipeAccDesc && lColumn <= CD_Circuit_PipeAccDp )
			{
				str = pHM->GetpCircuitPrimaryPipe()->GetSingularityListFull( true, true, true, true, HMCALC_ACCDISPMAX );
			}
			else
			{
				if( true == CSSheet::IsTextTruncated( lColumn, lRow ) )
				{
					str = GetCellText( lColumn, lRow );
				}
			}
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		pHM = GetpHM( lRow );

		if( NULL != pHM )
		{
			if( CD_CircuitInjSecSide_Pos == lColumn )
			{
				str = pHM->GetHMName();
			}
			else if( CD_CircuitInjSecSide_PumpHMin == lColumn )
			{
				if( eDpStab::DpStabOnBVBypass == pHM->GetpSch()->GetDpStab() )
				{
					// Refresh error messages.
					int iValidity = pHM->CheckValidity();
					str = pHM->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_SecondaryPump );
				}
			}
			else if( CD_CircuitInjSecSide_TempTsp == lColumn || CD_CircuitInjSecSide_TempTss == lColumn || CD_CircuitInjSecSide_TempTrs == lColumn )
			{
				int iErrors = 0;
				std::vector<CString> vecStrErrors;

				if( false == pHM->CheckIfTempAreValid( &iErrors, &vecStrErrors ) )
				{
					str = _T("");

					for( int i = 0; i < (int)vecStrErrors.size(); i++ )
					{
						str += vecStrErrors.at( i );

						if( i < (int)vecStrErrors.size() - 1 )
						{
							str += _T("\r\n");
						}
					}
				}
			}
			else if( false == pHM->IsaModule() && lColumn >= CD_CircuitInjSecSide_AccDesc && lColumn <= CD_CircuitInjSecSide_AccDp )
			{
				if( pHM->GetpCircuitSecondaryPipe() != NULL )
				{
					str = pHM->GetpCircuitSecondaryPipe()->GetSingularityListFull( true, true, true, true, HMCALC_ACCDISPMAX );
				}
			}
			else if( lColumn == CD_CircuitInjSecSide_PipeV )
			{
				// HYS-1878: 1- Display a tooltip message when the mouse is over the red error value.
				CPipes* pPipe = pHM->GetpCircuitSecondaryPipe();

				if( NULL != pPipe && eb3False == pPipe->CheckVelocityMinMax()
					&& NULL != pPipe->GetpParent() && NULL != pPipe->GetpParent()->GetpTechParam() )
				{
					CString str2 = CString( _T("[") ) + WriteCUDouble( _U_VELOCITY, pPipe->GetpParent()->GetpTechParam()->GetPipeMinVel() );
					str2 += CString( _T(" - ") ) + WriteCUDouble( _U_VELOCITY, pPipe->GetpParent()->GetpTechParam()->GetPipeMaxVel() );
					str2 += CString( _T("]") );
					FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );
				}
			}
			else
			{
				if( true == CSSheet::IsTextTruncated( lColumn, lRow ) )
				{
					str = GetCellText( lColumn, lRow );
				}
			}
		}
	}

	if( NULL != pHM )
	{
		*pnTipWidth = (SHORT)GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pbShowTip = TRUE;
	}
}

void CSheetHMCalc::TopLeftChangeFpspread( long lNewLeft, long lNewTop )
{
	if( false == m_bInitialized )
	{
		return;
	}

	SheetDescription eSheetDescription = GetSheetDescription( m_nCurrentSheet );

	if( SheetDescription::SUndefined == eSheetDescription )
	{
		return;
	}

	m_arptLeftTop[eSheetDescription] = CPoint( lNewLeft, lNewTop );
}

BEGIN_MESSAGE_MAP( CSheetHMCalc, CSSheet )
	ON_MESSAGE( WM_USER_MESSAGEDATA, MM_OnMessageReceive )
	ON_MESSAGE( SSM_COMBOSELCHANGE, ComboSelChangeFpspread )
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CSheetHMCalc::OnPaint()
{
	CSSheet::OnPaint();
	_VerifyComboBoxError();

}
LRESULT CSheetHMCalc::MM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
{
	CMessageManager::CMessage *pclMessage = CMessageManager::MM_ReadMessage( wParam );

	if( NULL == pclMessage )
	{
		return 0;
	}

	if( CMessageManager::ClassID::CID_CSSheet != ( CMessageManager::ClassID )pclMessage->GetClassID() )
	{
		delete pclMessage;
		return 0;
	}

	switch( ( CMessageManager::SSheetMessageType )pclMessage->GetMessageType() )
	{
		case CMessageManager::SSheetMessageType::SSheetMST_MouseEvent:
		{
			CMessageManager::CSSheetMEMsg *pclSShetMEMsg = dynamic_cast<CMessageManager::CSSheetMEMsg *>( pclMessage );

			if( NULL == pclSShetMEMsg )
			{
				delete pclMessage;
				return 0;
			}

			SSheetMEParams *prParams = pclSShetMEMsg->GetParams();

			if( NULL == prParams->m_hSheet || prParams->m_hSheet != GetSafeHwnd() )
			{
				delete pclMessage;
				return 0;
			}

			switch( prParams->m_eMouseEventType )
			{
				case CMessageManager::SSheetMET::MET_MouseMove:
					_OnMouseMove( prParams->m_lColumn, prParams->m_lRow );
					break;

				case CMessageManager::SSheetMET::MET_LButtonDown:
					_OnCellClicked( prParams->m_lColumn, prParams->m_lRow );
					break;

				case CMessageManager::SSheetMET::MET_LButtonUp:
					_OnLButtonUp( prParams->m_lColumn, prParams->m_lRow );
					break;

				case CMessageManager::SSheetMET::MET_LButtonDblClk:
					_OnCellDblClick( prParams->m_lColumn, prParams->m_lRow );
					break;

				case CMessageManager::SSheetMET::MET_RButtonDown:
					_OnRightClick( prParams->m_lColumn, prParams->m_lRow );
					break;
			}

			// Send manually to 'CViewDescription' for the class 'CResizingColumnInfo'. Because at now, 'MM_OnMessageReceive' messages are only sent to a 'CWnd'
			// and 'CResizingColumnInfo' is not.
			m_mapResizingColumnInfos[m_nCurrentSheet]->OnMouseEvent( prParams->m_eMouseEventType, prParams->m_lColumn, prParams->m_lRow, prParams->m_ptMouse );
		}
		break;

		case CMessageManager::SSheetMessageType::SSheetMST_KeyboardEvent:
		{
			CMessageManager::CSSheetKEMsg *pclSShetKEMsg = dynamic_cast<CMessageManager::CSSheetKEMsg *>( pclMessage );

			if( NULL == pclSShetKEMsg )
			{
				delete pclMessage;
				return 0;
			}

			SSheetKEParams *prParams = pclSShetKEMsg->GetParams();

			if( NULL == prParams->m_hSheet || prParams->m_hSheet != GetSafeHwnd() )
			{
				delete pclMessage;
				return 0;
			}

			switch( prParams->m_eKeyboardEventType )
			{
				case CMessageManager::SSheetKET::KET_VirtualKeyDown:
					if( VK_RETURN == prParams->m_iKeyCode || VK_TAB == prParams->m_iKeyCode )
					{
						EditChange( m_lLastEditedCol, m_lLastEditedRow );
						SetActiveCell( 0, 0 );
						Invalidate();
						LeaveCell( m_lLastEditedCol, m_lLastEditedRow, -1, -1, NULL );
					}

					break;
			}
		}
		break;

		case CMessageManager::SSheetMessageType::SSheetMST_ColumnWidthChanged:
		{
			if( false == m_bInitialized || true == m_bPrinting || true == m_bExporting || 0 == m_mapSSheetColumnWidth.count( m_nCurrentSheet ) )
			{
				delete pclMessage;
				return 0;
			}

			CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( m_nCurrentSheet );

			if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
			{
				delete pclMessage;
				return 0;
			}

			CMessageManager::CSSheetCWCMsg *pclSShetCWCMsg = dynamic_cast<CMessageManager::CSSheetCWCMsg *>( pclMessage );

			if( NULL == pclSShetCWCMsg )
			{
				delete pclMessage;
				return 0;
			}

			SSheetCWCParams *prParams = pclSShetCWCMsg->GetParams();

			if( NULL == prParams->m_hSheet || prParams->m_hSheet != pclSheetDescription->GetSSheetPointer()->GetSafeHwnd() )
			{
				delete pclMessage;
				return 0;
			}

			for( mapShortLongIter iter = m_mapSSheetColumnWidth[m_nCurrentSheet].begin(); iter != m_mapSSheetColumnWidth[m_nCurrentSheet].end(); ++iter )
			{
				if( FALSE == pclSheetDescription->GetSSheetPointer()->IsColHidden( iter->first ) )
				{
					CRect rectPixel = pclSheetDescription->GetSSheetPointer()->GetCellCoordInPixel( iter->first, pclSheetDescription->GetFirstSelectableRow() );
					iter->second = rectPixel.Width();
				}
			}
		}
		break;
	}

	delete pclMessage;
	return 0;
}

LRESULT CSheetHMCalc::ComboSelChangeFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;

	CDS_HydroMod *pHM = GetpHM( pCellCoord->Row );

	if( NULL == pHM )
	{
		return 0;
	}

	bool bCompute = false;
	bool bRefresh = false;
	
	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		int iCurrentSel = ComboBoxSendMessage( pCellCoord->Col, pCellCoord->Row, SS_CBM_GETCURSEL, 0, 0 );

		switch( pCellCoord->Col )
		{
			// Control mode is only for smart control valve (Not for smart differential pressure controller).
			case CD_Circuit_SmartValveControlMode:
				{
					SmartValveControlMode eSmartControlValveControlMode = SmartValveControlMode::SCVCM_Undefined;

					if( 0 == iCurrentSel )
					{
						eSmartControlValveControlMode = SmartValveControlMode::SCVCM_Flow;
					}
					else if( 1 == iCurrentSel )
					{
						eSmartControlValveControlMode = SmartValveControlMode::SCVCM_Power;
					}

					pHM->GetpSmartControlValve()->SetControlMode( eSmartControlValveControlMode );

					bCompute = true;
					bRefresh = true;
				}
				break;

			case CD_Circuit_SmartValveLocation:
				{
					// Control mode is for smart control valve and smart differential pressure controller.
					SmartValveLocalization eSmartValveLocalization = SmartValveLocalization::SmartValveLocNone;

					if( 0 == iCurrentSel )
					{
						eSmartValveLocalization = SmartValveLocalization::SmartValveLocSupply;
					
					}
					else
					{
						eSmartValveLocalization = SmartValveLocalization::SmartValveLocReturn;
					}

					if( true == pHM->IsSmartControlValveExist() )
					{
							pHM->GetpSmartControlValve()->SetSelLocalizationUserChoice( eSmartValveLocalization );
							pHM->GetpSmartControlValve()->SetLocalization( eSmartValveLocalization );
					}
					else if( true == pHM->IsSmartDpCExist() )
					{
							pHM->GetpSmartDpC()->SetSelLocalizationUserChoice( eSmartValveLocalization );
							pHM->GetpSmartDpC()->SetLocalization( eSmartValveLocalization );
					}

					pHM->ResetSchemeIDPtr();

					bCompute = true;
					bRefresh = true;
				}
				break;

			// Product set is only for smart differential pressure controller (Not for smart control valve).
			case CD_Circuit_SmartValveProductSet:
				{
					int iCurrentSel = ComboBoxSendMessage( pCellCoord->Col, pCellCoord->Row, SS_CBM_GETCURSEL, 0, 0 );
					CDB_Product *pclProductSetSelected = dynamic_cast<CDB_Product *>( (CData *)_GetComboBoxParam( SheetDescription::SCircuit, pCellCoord->Col, pCellCoord->Row, iCurrentSel ) );

					if( NULL != pclProductSetSelected )
					{
						pHM->GetpSmartDpC()->SetProductSetIDPtr( pclProductSetSelected->GetIDPtr() );
						//pHM->GetpSmartDpC()->SetSelectedAsaPackage( eBool3::eb3True );
						bRefresh = true;
					}
				}
				break;

			// Dp sensor is only for smart differential pressure controller (Not for smart control valve).
			case CD_Circuit_SmartValveDpSensor:
				{
					int iCurrentSel = ComboBoxSendMessage( pCellCoord->Col, pCellCoord->Row, SS_CBM_GETCURSEL, 0, 0 );
					CDB_DpSensor *pclDpSensorSelected = dynamic_cast<CDB_DpSensor *>( (CData *)_GetComboBoxParam( SheetDescription::SCircuit, pCellCoord->Col, pCellCoord->Row, iCurrentSel ) );

					if( NULL != pclDpSensorSelected )
					{
						pHM->GetpSmartDpC()->SetDpSensorIDPtr( pclDpSensorSelected->GetIDPtr() );
						pHM->GetpSmartDpC()->SetSelectedAsaPackage( eBool3::eb3False );
						bRefresh = true;
					}
				}
				break;
		}
	}

	if( true == bCompute ) 
	{
		BeginWaitCursor();
		
		pHM->ComputeHM( CDS_HydroMod::eceResize );
		pHM->ComputeAll();

		EndWaitCursor();
	}
	
	// To deactivate all interaction coming from TSpread.
	m_bInitialized = false;

	if( true == bRefresh && NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)m_pHM );
	}

	return 0;
}

void CSheetHMCalc::_OnMouseMove( long lColumn, long lRow )
{
	// This code is only when drag & drop is currently running !!
	if( false == m_bInitialized || true == m_bPrinting || true == m_bExporting || false == m_DragDropInfo.m_bDragDropRunning )
	{
		return;
	}

	if( NULL == m_pHM )
	{
		return;
	}

	CDS_HydroMod *pCurrentHM = GetpHM( lRow );

	if( pCurrentHM == m_DragDropInfo.m_pHMSource )
	{
		if( m_DragDropInfo.m_hHandForbidden == ::GetCursor() )
		{
			::SetCursor( m_DragDropInfo.m_hHandClosed );
		}
	}
	else if( false == _IsDragDropPossible( m_DragDropInfo.m_pHMSource, pCurrentHM, lColumn, lRow ) )
	{
		if( m_DragDropInfo.m_hHandClosed == ::GetCursor() )
		{
			::SetCursor( m_DragDropInfo.m_hHandForbidden );
		}
	}
	else
	{
		if( m_DragDropInfo.m_hHandForbidden == ::GetCursor() )
		{
			::SetCursor( m_DragDropInfo.m_hHandClosed );
		}
	}
}

void CSheetHMCalc::_OnLButtonUp( long lColumn, long lRow )
{
	// If a drag & drop was running, we disable the reception of mouse move events.
	if( true == m_DragDropInfo.m_bDragDropRunning )
	{
		SetBool( SSB_ALLOWDRAGDROP, FALSE );
		m_DragDropInfo.Reset();
	}
}

void CSheetHMCalc::_OnCellClicked( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	if( true == m_bPrinting || true == m_bExporting )
	{
		return;
	}

	SetBool( SSB_REDRAW, FALSE );

	bool bRedraw = false;
	bool bMustReset = false;
	CDS_HydroMod *pHM = GetpHM( lRow );

	// If click is on a main Expand/Collapse button...
	if( true == IsMainExpandCollapseButton( lColumn, lRow ) )
	{
		bool bExecute = true;

		// If user has not clicked on any main Expand/Collapse button...
		if( ( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) ) && lColumn != CD_Distribution_Pos ) 
				|| ( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) ) && lColumn != CD_Circuit_Pos )
				|| ( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) ) && lColumn != CD_CircuitInjSecSide_Pos )
				|| ( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SMeasurement ) ) && lColumn != CD_Measurement_Pos )
				|| ( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::STADiagnostic ) ) && lColumn != CD_TADiagnostic_Pos ) )
		{
			// We do nothing.
			bExecute = false;
		}

		if( true == bExecute )
		{
			bRedraw = true;

			// Change state of all child Expand/Collapse column buttons.
			int iCurrentState = GetMainExpandCollapseColumnState( lColumn, lRow );

			switch( iCurrentState )
			{
				case CSSheet::ExpandCollapseColumnState::ECCS_Collapsed:
					_ExpandAll();
					break;

				case CSSheet::ExpandCollapseColumnState::ECCS_Expanded:
					_CollapseAll();
					break;

				case CSSheet::ExpandCollapseColumnState::ECCS_Invalid:
					bRedraw = false;
					break;
			}

			// Change state of main Expand/Collapse column button.
			if( true == bRedraw )
			{
				ChangeMainExpandCollapseButtonState( lColumn, lRow, CSSheet::ExpandCollapseColumnAction::ECCA_Toggle );
			}
		}
	}
	else if( true == IsExpandCollapseColumnButton( lColumn, lRow ) )
	{
		// Change state of button.
		ChangeExpandCollapseColumnButtonState( lColumn, lRow, RD_Header_FirstCirc, GetMaxRows(), ExpandCollapseColumnAction::ECCA_Toggle );

		// If collapse or extend has been executed on a group ...
		_HideEmptyColumns();

		// Verify main Expand/Collapse button state.
		_RefreshMainCollapseExpandButton();

		// Refill m_arlXXX CArray variable with columns collapsed.
		m_arSheetGroupExpandedList[m_nCurrentSheet].clear();

		for( long lLoopCol = 1; lLoopCol < GetMaxCols(); lLoopCol++ )
		{
			if( CSSheet::ExpandCollapseColumnState::ECCS_Expanded == GetExpandCollapseColumnState( lLoopCol, RD_Header_ButDlg ) )
			{
				m_arSheetGroupExpandedList[m_nCurrentSheet].push_back( lLoopCol );
			}
		}

		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		_RescanResizeColumnInfo( m_nCurrentSheet );

		bRedraw = true;
	}
	else if( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) && CD_Distribution_Sub == lColumn ) 
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) && CD_Circuit_Sub == lColumn )
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) && CD_CircuitInjSecSide_Sub == lColumn )
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SMeasurement ) && CD_Measurement_Sub == lColumn )
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::STADiagnostic ) && CD_TADiagnostic_Sub == lColumn ) )
	{
		// Open a module
		if( NULL != pHM )
		{
			CDS_HydroMod *pclHydroModToDisplay = NULL;

			if( pHM != m_pHM )
			{
				// Go in children.
				if( true == pHM->IsaModule() )
				{
					pclHydroModToDisplay = pHM;
				}
			}
			else
			{
				// Go in parent.
				if( NULL != pHM->GetIDPtr().PP )
				{
					pclHydroModToDisplay = (CDS_HydroMod *)( pHM->GetIDPtr().PP );
				}
			}

			if( NULL != pDlgLeftTabProject )
			{
				pDlgLeftTabProject->ChangeSelection( pclHydroModToDisplay );
			}
		}
	}

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		// Bypass valve button Auto, never or always.
		if( NULL != pHM && NULL != dynamic_cast<CDS_Hm3W *>( pHM ) && false == pHM->GetpPrjParam()->IsFreezed() && CD_Circuit_BvBypPict == lColumn )
		{
			if( ( (CDS_Hm3W *)pHM )->GetUseBypBv() < CDS_HydroMod::eUseProduct::Always )
			{
				( (CDS_Hm3W *)pHM )->SetUseBypBv( ( CDS_HydroMod::eUseProduct )( ( (CDS_Hm3W *)pHM )->GetUseBypBv() + 1 ) );
			}
			else
			{
				( (CDS_Hm3W *)pHM )->SetUseBypBv( CDS_HydroMod::eUseProduct::Auto );
			}

			bMustReset = true;
		}
		else if( NULL != pHM && NULL != dynamic_cast<CDS_Hm2W *>( pHM ) && NULL != pHM->GetpSch() && false == pHM->GetpPrjParam()->IsFreezed() && CD_Circuit_SvPict == lColumn )
		{
			CDS_Hm2W *pHm2W = dynamic_cast<CDS_Hm2W *>( pHM );
			CDS_HydroMod::eHMObj eHMShutoffValveLoc = CDS_HydroMod::eNone;

			if( ShutoffValveLoc::ShutoffValveLocSupply == pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMShutoffValveLoc = CDS_HydroMod::eShutoffValveSupply;
			}
			else if( ShutoffValveLoc::ShutoffValveLocReturn == pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMShutoffValveLoc = CDS_HydroMod::eShutoffValveReturn;
			}

			if( CDS_HydroMod::eNone != eHMShutoffValveLoc )
			{
				if( CDS_HydroMod::eUseProduct::Always == pHm2W->GetUseShutoffValve() )
				{
					pHm2W->SetUseShutoffValve( CDS_HydroMod::eUseProduct::Never, eHMShutoffValveLoc );
				}
				else
				{
					pHm2W->SetUseShutoffValve( CDS_HydroMod::eUseProduct::Always, eHMShutoffValveLoc );
				}

				bMustReset = true;
			}
		}
	}

	if( true == bMustReset )
	{
		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->PostMessage( WM_USER_RESETHMTREE, ( WPARAM )true, (LPARAM)m_pHM );
		}
	}
	else
	{
		if( false == m_DragDropInfo.m_bDragDropModeEnabled )
		{
			// Allow/Disable drag & drop operations.
			if( true == _IsDragDropAvailable( lColumn, lRow ) )
			{
				SetBool( SSB_ALLOWDRAGDROP, TRUE );

				// These variables are used only when receiving mouse move events to eventually change the cursor.
				m_DragDropInfo.m_bDragDropModeEnabled = true;
				m_DragDropInfo.m_lStartCol = lColumn;
				m_DragDropInfo.m_lStartRow = lRow;
				m_DragDropInfo.m_pHMSource = pHM;

				// Retrieve the cell coordinates.
				m_DragDropInfo.m_rectStartCell = GetCellCoordInPixel( lColumn, lRow );
				m_DragDropInfo.m_rectStartCell.top -= 2;
				m_DragDropInfo.m_rectStartCell.bottom += 2;

				SetCursor( SS_CURSORTYPE_DRAGDROP, m_DragDropInfo.m_hHandClosed );
			}
		}
		else if( false == m_DragDropInfo.m_bDragDropRunning )
		{
			CPoint point;
			GetCursorPos( &point );
			ScreenToClient( &point );

			// Retrieve 
			long lTopLeftCellCol;
			long lTopLeftCellRow;
			GetTopLeftCell( &lTopLeftCellCol, &lTopLeftCellRow );
			CRect rectTopLeftCell = GetCellCoordInPixel( lTopLeftCellCol, lTopLeftCellRow );

			rectTopLeftCell.OffsetRect( CPoint( -m_DragDropInfo.m_mapFrozenColsWidth.at( m_nCurrentSheet ), -m_DragDropInfo.m_mapFrozenRowsWidth.at( m_nCurrentSheet ) ) );

			point.x += rectTopLeftCell.left;
			point.y += rectTopLeftCell.top;

			// If user clicks on the same cell...
			if( m_DragDropInfo.m_lStartCol == lColumn && TRUE == m_DragDropInfo.m_rectStartCell.PtInRect( point ) )
			{
				// With the closed hand it means that the drag & drop is starting.
				if( m_DragDropInfo.m_hHandClosed == ::GetCursor() )
				{
					m_DragDropInfo.m_bDragDropRunning = true;
				}
			}
			else
			{
				// Otherwise we need to check if this other cell can be also drag & dropped.
				if( true == _IsDragDropAvailable( lColumn, lRow ) )
				{
					SetBool( SSB_ALLOWDRAGDROP, TRUE );

					// These variables are used only when receiving mouse move events to eventually change the cursor.
					m_DragDropInfo.m_bDragDropModeEnabled = true;
					m_DragDropInfo.m_lStartCol = lColumn;
					m_DragDropInfo.m_lStartRow = lRow;
					m_DragDropInfo.m_pHMSource = pHM;

					// Retrieve the cell coordinates.
					m_DragDropInfo.m_rectStartCell = GetCellCoordInPixel( lColumn, lRow );
					m_DragDropInfo.m_rectStartCell.top--;

					SetCursor( SS_CURSORTYPE_DRAGDROP, m_DragDropInfo.m_hHandClosed );
				}
				else
				{
					SetBool( SSB_ALLOWDRAGDROP, FALSE );
					m_DragDropInfo.Reset();
				}
			}
		}

		SetBool( SSB_REDRAW, TRUE );

		if( true == bRedraw )
		{
			Invalidate();
		}
	}
}

void CSheetHMCalc::_OnCellDblClick( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	// Verify if we are in HMCalc mode.
	if( false == pMainFrame->IsHMCalcChecked() )
	{
		// In that case, we allow to modify only references.
		if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SMeasurement ) && CD_Measurement_Reference != lColumn
				&& CD_Measurement_Description != lColumn )
		{
			return;
		}
	}

	// No dbl-click above the first circuit.
	if( lRow < RD_Header_FirstCirc )
	{
		return;
	}

	CDS_HydroMod *pHM = GetpHM( lRow );

	if( NULL == pHM || true == pHM->IsForHub() )
	{
		return;
	}

	// Eject first column.
	if( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) && CD_Distribution_Sub == lColumn )
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) && CD_Circuit_Sub == lColumn )
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) && CD_CircuitInjSecSide_Sub == lColumn )
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SMeasurement ) && CD_Measurement_Sub == lColumn )
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::STADiagnostic ) && CD_TADiagnostic_Sub == lColumn ) )
	{
		return;
	}

	// Name can be edited if project is frozen or not.
	bool bSetEditMode = false;

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		if( CD_Distribution_Name == lColumn || CD_Distribution_Desc == lColumn )
		{
			bSetEditMode = true;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		if( CD_Circuit_Name == lColumn || CD_Circuit_Desc == lColumn || CD_Circuit_UnitDesc == lColumn || CD_Circuit_CVDesc == lColumn )
		{
			bSetEditMode = true;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		if( CD_CircuitInjSecSide_Name == lColumn || CD_CircuitInjSecSide_Desc == lColumn || CD_CircuitInjSecSide_CVDesc == lColumn )
		{
			bSetEditMode = true;
		}
	}

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SMeasurement ) )
	{
		if( CD_Measurement_Reference == lColumn || CD_Measurement_Description == lColumn )
		{
			bSetEditMode = true;
		}
	}

	if( true == bSetEditMode )
	{
		SetEditMode( true );
		return;
	}

	// Combo Edit mode.
	if( false == m_bComboEditMode && pHM && pHM->IsCvExist( true ) && false == pHM->GetpCV()->IsTaCV()
		&& ( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) && CD_Circuit_CVKvs == lColumn ) ||
			 ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) && CD_CircuitInjSecSide_CVKvs == lColumn ) )
		&& false == m_pHM->GetpPrjParam()->IsFreezed() )
	{
		m_bComboEditMode = true;
		SaveTextPattern( lColumn, lRow );

		if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
		{
			FormatEditDouble( CD_Circuit_CVKvs, lRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
		}
		else
		{
			FormatEditDouble( CD_CircuitInjSecSide_CVKvs, lRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
		}

		RestoreTextPattern( lColumn, lRow );
		SetEditMode( true );
		return;
	}

	if( false == m_pHM->GetpPrjParam()->IsFreezed() && ( true == IsCellProperty( lColumn, lRow, CellText )
			|| true == IsCellProperty( lColumn, lRow, CellDouble ) ) )
	{
		SetEditMode( true );
		return;
	}

	// If it's a combo, ignore the double click.
	if( SS_TYPE_COMBOBOX == GetCellTypeW( lColumn, lRow ) )
	{
		return;
	}

	// Show cursor as an hourglass.
	BeginWaitCursor();

	bool redraw = false;

	// Launch dialog wizard circuit.
	EditHMRow();

	if( true == redraw )
	{
		Invalidate();
	}

	// Reset cursor.
	EndWaitCursor();
}

void CSheetHMCalc::_OnRightClick( long lColumn, long lRow )
{
	if( false == m_bInitialized )
	{
		return;
	}

	// Right click on another row that previous selected row, update selection.
	// Multiple selection ?
	CArray <long> pa;
	bool bFound = false;

	if( GetSelectedRows( &pa ) > 0 )
	{
		// Right click in a selected row?
		for( int i = 0; i < pa.GetCount() && false == bFound; i++ )
		{
			if( pa.GetAt( i ) == lRow )
			{
				bFound = true;
			}
		}
	}

	if( false == bFound )
	{
		if( NULL != GetpHM( lRow ) )
		{
			_SelectRow( lRow );
		}
	}
}

void CSheetHMCalc::_InitDistributionSheet( CDS_HydroMod *pHM )
{
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == m_mapSSheetColumnWidth.size() || 0 == m_mapSSheetColumnWidth.count( SheetDescription::SDistribution ) )
	{
		_ResetColumnWidth( SheetDescription::SDistribution );
	}
	
	_ClearAllCombos( SheetDescription::SDistribution );

	SetMaxRows( RD_Header_FirstCirc - 1 );
	SetMaxCols( CD_Distribution_Pointer );

	// All cells are static by default and filled with _T("").
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Freeze (make non scrollable) line above first circuit and first column.
	SetFreeze( CD_Distribution_Pos, RD_Header_FirstCirc - 1 );

	// Verify the project is frozen.
	bool bFreezed = false;

	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL != pPrjParam )
	{
		bFreezed = pPrjParam->IsFreezed();
	}

	// Increase ROW height.
	double RowHeight = 12.75;
	SetRowHeight( RD_Header_GroupName, RowHeight * 1.5 );
	SetRowHeight( RD_Header_ButDlg, RowHeight * 1.2 );
	SetRowHeight( RD_Header_ColName, RowHeight * 2 );
	SetRowHeight( RD_Header_Unit, RowHeight * 1.2 );
	SetRowHeight( RD_Header_Picture, RowHeight );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetColWidthInPixels( CD_Distribution_Sub, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_Sub] );
	}
	else
	{
		ShowCol( CD_Distribution_Sub, FALSE );
	}

	SetColWidthInPixels( CD_Distribution_Pos, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_Pos] );

	// Information.
	SetColWidthInPixels( CD_Distribution_Name, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_Name] );
	SetColWidthInPixels( CD_Distribution_Desc, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_Desc] );
	SetColWidthInPixels( CD_Distribution_Type, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_Type] );
	SetColWidthInPixels( CD_Distribution_Q, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_Q] );
	SetColWidthInPixels( CD_Distribution_H, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_H] );

	// Supply pipe.
	SetColWidthInPixels( CD_Distribution_SupplyPicture, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPicture] );
	SetColWidthInPixels( CD_Distribution_SupplyPipeSeries, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPipeSeries] );
	SetColWidthInPixels( CD_Distribution_SupplyPipeSize, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPipeSize] );
	SetColWidthInPixels( CD_Distribution_SupplyPipeLength, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPipeLength] );
	SetColWidthInPixels( CD_Distribution_SupplyPipeDp, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPipeDp] );
	SetColWidthInPixels( CD_Distribution_SupplyPipeTotalQ, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPipeTotalQ] );
	SetColWidthInPixels( CD_Distribution_SupplyDiversityFactor, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyDiversityFactor] );
	SetColWidthInPixels( CD_Distribution_SupplyPipeLinDp, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPipeLinDp] );
	SetColWidthInPixels( CD_Distribution_SupplyPipeVelocity, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPipeVelocity] );
	SetColWidthInPixels( CD_Distribution_SupplyPipeSep, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyPipeSep] );

	// Accessories on supply pipe.
	SetColWidthInPixels( CD_Distribution_SupplyAccDesc, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyAccDesc] );
	SetColWidthInPixels( CD_Distribution_SupplyAccData, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyAccData] );
	SetColWidthInPixels( CD_Distribution_SupplyAccDp, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_SupplyAccDp] );

	// To verify if we must show distribution return pipe group, we have to check module and one children. If one of both is in reverse return mode,
	// we can show it.
	// Remark: children must have all the same return type (otherwise it's a bug!). Then we can check only one child of the parent to determine the
	//         return type of all of them.
	m_bAtLeastOneReverseMode = false;

	if( CDS_HydroMod::ReturnType::Reverse == pHM->GetReturnType() )
	{
		m_bAtLeastOneReverseMode = true;
	}
	else if( NULL != pHM->GetFirst().MP )
	{
		if( CDS_HydroMod::ReturnType::Reverse == ( (CDS_HydroMod *)pHM->GetFirst().MP )->GetReturnType() )
		{
			m_bAtLeastOneReverseMode = true;
		}
	}

	if( true == m_bAtLeastOneReverseMode )
	{
		// Return pipe.
		SetColWidthInPixels( CD_Distribution_ReturnPicture, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPicture] );
		SetColWidthInPixels( CD_Distribution_ReturnPipeSeries, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPipeSeries] );
		SetColWidthInPixels( CD_Distribution_ReturnPipeSize, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPipeSize] );
		SetColWidthInPixels( CD_Distribution_ReturnPipeLength, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPipeLength] );
		SetColWidthInPixels( CD_Distribution_ReturnPipeDp, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPipeDp] );
		SetColWidthInPixels( CD_Distribution_ReturnPipeTotalQ, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPipeTotalQ] );
		SetColWidthInPixels( CD_Distribution_ReturnDiversityFactor, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnDiversityFactor] );
		SetColWidthInPixels( CD_Distribution_ReturnPipeLinDp, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPipeLinDp] );
		SetColWidthInPixels( CD_Distribution_ReturnPipeVelocity, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPipeVelocity] );
		SetColWidthInPixels( CD_Distribution_ReturnPipeSep, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnPipeSep] );

		// Accessories on return pipe.
		SetColWidthInPixels( CD_Distribution_ReturnAccDesc, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnAccDesc] );
		SetColWidthInPixels( CD_Distribution_ReturnAccData, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnAccData] );
		SetColWidthInPixels( CD_Distribution_ReturnAccDp, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_ReturnAccDp] );
	}
	else
	{
		// Hide columns (or delete if we are in export mode)
		for( long lLoop = CD_Distribution_ReturnPicture; lLoop <= CD_Distribution_ReturnAccDp; lLoop++ )
		{
			ShowCol( lLoop, FALSE );
		}
	}

	SetColWidthInPixels( CD_Distribution_Pointer, m_mapSSheetColumnWidth[SheetDescription::SDistribution][CD_Distribution_Pointer] );

	// Allow to determine if distribution of all circuits belongs to the same series. If it's the case, we don't show the
	// series column and we put the series name in the main title of the distribution group.
	m_pDistributionSupplyPipeTable = NULL;
	m_pDistributionReturnPipeTable = NULL;

	if( NULL != pHM )
	{
		// If level 0, that means hydromod is the main root (*A, *B, ...). And there is no distribution pipe of the main root.
		if( pHM->GetLevel() > 0 )
		{
			m_pDistributionSupplyPipeTable = pHM->GetpDistrSupplyPipe()->GetIDPtr().PP;

			if( CDS_HydroMod::ReturnType::Reverse == pHM->GetReturnType() )
			{
				m_pDistributionReturnPipeTable = pHM->GetpDistrReturnPipe()->GetIDPtr().PP;
			}
		}

		// Distribution supply pipe.
		bool fPipeSeries = true;

		for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID && true == fPipeSeries; IDPtr = pHM->GetNext() )
		{
			CDS_HydroMod *pChildHM = (CDS_HydroMod *)IDPtr.MP;
			CPipes *pPipe = pChildHM->GetpDistrSupplyPipe();

			if( NULL != pPipe )
			{
				if( NULL == m_pDistributionSupplyPipeTable )
				{
					m_pDistributionSupplyPipeTable = (CTable *)( pPipe->GetIDPtr().PP );
				}
				else if( pPipe->GetIDPtr().PP && m_pDistributionSupplyPipeTable != (CTable *)pPipe->GetIDPtr().PP )
				{
					m_pDistributionSupplyPipeTable = NULL;
					fPipeSeries = false;
				}
			}
		}

		if( true == m_bAtLeastOneReverseMode )
		{
			bool fPipeSeries = true;

			for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID && true == fPipeSeries; IDPtr = pHM->GetNext() )
			{
				CDS_HydroMod *pCHM = (CDS_HydroMod *)IDPtr.MP;
				CPipes *pPipe = pCHM->GetpDistrReturnPipe();

				if( NULL != pPipe )
				{
					if( NULL == m_pDistributionReturnPipeTable )
					{
						m_pDistributionReturnPipeTable = (CTable *)( pPipe->GetIDPtr().PP );
					}
					else if( pPipe->GetIDPtr().PP && m_pDistributionReturnPipeTable != (CTable *)pPipe->GetIDPtr().PP )
					{
						m_pDistributionReturnPipeTable = NULL;
						fPipeSeries = false;
					}
				}
			}
		}
	}

	// All cells are not selectable (in header only).
	SetCellProperty( CD_Distribution_Pos, RD_Header_GroupName, CD_Distribution_Pointer, RD_Header_FirstCirc - 1, CSSheet::_SSCellProperty::CellNoSelection, true );

	// Flag unhidden columns.
	SetFlagShowEvenEmpty( CD_Distribution_Q, CD_Distribution_Q, true );
	SetFlagShowEvenEmpty( CD_Distribution_H, CD_Distribution_H, true );

	SetFlagShowEvenEmpty( CD_Distribution_SupplyPicture, CD_Distribution_SupplyPicture, true );
	SetFlagShowEvenEmpty( CD_Distribution_SupplyPipeSize, CD_Distribution_SupplyPipeSize, true );
	SetFlagShowEvenEmpty( CD_Distribution_SupplyPipeLength, CD_Distribution_SupplyPipeLength, true );
	SetFlagShowEvenEmpty( CD_Distribution_SupplyPipeDp, CD_Distribution_SupplyPipeDp, true );
	SetFlagShowEvenEmpty( CD_Distribution_SupplyPipeSep, CD_Distribution_SupplyPipeSep, true );
	SetFlagShowEvenEmpty( CD_Distribution_SupplyAccDp, CD_Distribution_SupplyAccDp, true );

	SetFlagShowEvenEmpty( CD_Distribution_ReturnPicture, CD_Distribution_ReturnPicture, true );
	SetFlagShowEvenEmpty( CD_Distribution_ReturnPipeSize, CD_Distribution_ReturnPipeSize, true );
	SetFlagShowEvenEmpty( CD_Distribution_ReturnPipeLength, CD_Distribution_ReturnPipeLength, true );
	SetFlagShowEvenEmpty( CD_Distribution_ReturnPipeDp, CD_Distribution_ReturnPipeDp, true );
	SetFlagShowEvenEmpty( CD_Distribution_ReturnPipeSep, CD_Distribution_ReturnPipeSep, true );
	SetFlagShowEvenEmpty( CD_Distribution_ReturnAccDp, CD_Distribution_ReturnAccDp, true );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Circuit' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	AddCellSpanW( CD_Distribution_Name, RD_Header_GroupName, CD_Distribution_H - CD_Distribution_Name + 1, 2 );
	_SetStaticText( CD_Distribution_Name, RD_Header_GroupName, IDS_HMCALC_CIRCUIT );

	// Sub.
	if( NULL != pHM )
	{
		bool bFlag = true;

		for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext() )
		{
			if( true == ( (CDS_HydroMod *)IDPtr.MP )->IsaModule() )
			{
				bFlag = false;
				break;
			}
		}
	}

	// Name.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	_SetStaticText( CD_Distribution_Name, RD_Header_ColName, IDS_HMCALC_NAME );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Distribution_Name, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Description.
	_SetStaticText( CD_Distribution_Desc, RD_Header_ColName, IDS_HMCALC_DESC );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Distribution_Desc, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Type.
	_SetStaticText( CD_Distribution_Type, RD_Header_ColName, IDS_HMCALC_TYPE );

	// Q.
	_SetStaticText( CD_Distribution_Q, RD_Header_ColName, IDS_HMCALC_Q );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Distribution_Q, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// H.
	_SetStaticText( CD_Distribution_H, RD_Header_ColName, IDS_HMCALC_H );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Distribution_H, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Border.
	SetCellBorder( CD_Distribution_Name, RD_Header_Picture - 1, CD_Distribution_H, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
			SPREAD_COLOR_NONE );

	SetCellBorder( CD_Distribution_Name, RD_Header_FirstCirc - 1, CD_Distribution_H, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
			SPREAD_COLOR_NONE );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Distribution supply pipe' group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Determine pipe series.
	long lFirstcol = CD_Distribution_SupplyPipeSeries;

	CString str( TASApp.LoadLocalizedString( ( false == m_bAtLeastOneReverseMode ) ? IDS_HMCALC_DISTRIBUTION : IDS_HMCALC_DISTRIBUTIONSUPPLY ) );
	bool bShowPipeSeries = true;

	if( NULL != m_pDistributionSupplyPipeTable )
	{
		lFirstcol = CD_Distribution_SupplyPipeSize;
		str += CString( _T("\r\n") ) + m_pDistributionSupplyPipeTable->GetName();
		ShowCol( CD_Distribution_SupplyPipeSeries, FALSE );
		bShowPipeSeries = false;
	}
	else
	{
		ShowCol( CD_Distribution_SupplyPipeSeries, TRUE );
	}

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	AddCellSpanW( lFirstcol, RD_Header_GroupName, CD_Distribution_SupplyPipeSep - lFirstcol, 2 );
	_SetStaticText( lFirstcol, RD_Header_GroupName, str );

	// Pipe series.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

	if( true == bShowPipeSeries )
	{
		_SetStaticText( CD_Distribution_SupplyPipeSeries, RD_Header_ColName, IDS_HMCALC_PIPESERIE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Distribution_SupplyPipeSeries, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}

	// Pipe size.
	_SetStaticText( CD_Distribution_SupplyPipeSize, RD_Header_ColName, IDS_HMCALC_PIPESIZE );

	if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_COMBO, CD_Distribution_SupplyPipeSize, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Length.
	_SetStaticText( CD_Distribution_SupplyPipeLength, RD_Header_ColName, IDS_HMCALC_PIPEL );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Distribution_SupplyPipeLength, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Distribution_SupplyPipeLength, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Dp.
	_SetStaticText( CD_Distribution_SupplyPipeDp, RD_Header_ColName, IDS_HMCALC_DP );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Distribution_SupplyPipeDp, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Flow.
	_SetStaticText( CD_Distribution_SupplyPipeTotalQ, RD_Header_ColName, IDS_HMCALC_FLOW );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Distribution_SupplyPipeTotalQ, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Diversity factor.
	_SetStaticText( CD_Distribution_SupplyDiversityFactor, RD_Header_ColName, IDS_HMCALC_DIVERSITYFACTOR );

	// linear Dp.
	_SetStaticText( CD_Distribution_SupplyPipeLinDp, RD_Header_ColName, IDS_HMCALC_LINDP );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Distribution_SupplyPipeLinDp, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Velocity.
	_SetStaticText( CD_Distribution_SupplyPipeVelocity, RD_Header_ColName, IDS_HMCALC_PIPEVA );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Distribution_SupplyPipeVelocity, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Border.
	SetCellBorder( lFirstcol, RD_Header_Picture - 1, CD_Distribution_SupplyPipeSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
			SPREAD_COLOR_NONE );

	SetCellBorder( lFirstcol, RD_Header_FirstCirc - 1, CD_Distribution_SupplyPipeSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
			SPREAD_COLOR_NONE );

	// Collapse button.
	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateExpandCollapseColumnButton( CD_Distribution_SupplyPipeSep, RD_Header_ButDlg, CD_Distribution_SupplyPipeTotalQ, CD_Distribution_SupplyPipeVelocity,
				GetMaxRows() );

		ChangeExpandCollapseColumnButtonState( CD_Distribution_SupplyPipeSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Accessories on distribution supply pipe.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	AddCellSpanW( CD_Distribution_SupplyAccDesc, RD_Header_GroupName, CD_Distribution_SupplyAccDp - CD_Distribution_SupplyAccDesc + 1, 2 );
	_SetStaticText( CD_Distribution_SupplyAccDesc, RD_Header_GroupName, IDS_HMCALC_ACCESSORIES );

	// Description.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	_SetStaticText( CD_Distribution_SupplyAccDesc, RD_Header_ColName, IDS_HMCALC_ACCDESCRIPTION );

	// Data needed for printing.
	_SetStaticText( CD_Distribution_SupplyAccData, RD_Header_ColName, IDS_HMCALCINPUTDATA );

	// Dp.
	_SetStaticText( CD_Distribution_SupplyAccDp, RD_Header_ColName, IDS_HMCALC_DP );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Distribution_SupplyAccDp, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Border.
	SetCellBorder( CD_Distribution_SupplyAccDesc, RD_Header_Picture - 1, CD_Distribution_SupplyAccDp, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	SetCellBorder( CD_Distribution_SupplyAccDesc, RD_Header_FirstCirc - 1, CD_Distribution_SupplyAccDp, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	if( true == m_bAtLeastOneReverseMode )
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Distribution return pipe' group.
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// Determine pipe series.
		long lFirstcol = CD_Distribution_ReturnPipeSeries;

		bool bShowPipeSeries = true;
		CString str( TASApp.LoadLocalizedString( IDS_HMCALC_DISTRIBUTIONRETURN ) );

		if( NULL != m_pDistributionReturnPipeTable )
		{
			lFirstcol = CD_Distribution_ReturnPipeSize;
			str += CString( _T("\r\n") ) + m_pDistributionReturnPipeTable->GetName();
			ShowCol( CD_Distribution_ReturnPipeSeries, FALSE );
			bShowPipeSeries = false;
		}
		else
		{
			ShowCol( CD_Distribution_ReturnPipeSeries, TRUE );
		}

		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		AddCellSpanW( lFirstcol, RD_Header_GroupName, CD_Distribution_ReturnPipeSep - lFirstcol, 2 );
		_SetStaticText( lFirstcol, RD_Header_GroupName, str );

		// Pipe series.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

		if( true == bShowPipeSeries )
		{
			_SetStaticText( CD_Distribution_ReturnPipeSeries, RD_Header_ColName, IDS_HMCALC_PIPESERIE );

			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_COMBO, CD_Distribution_ReturnPipeSeries, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}

		// Pipe size.
		_SetStaticText( CD_Distribution_ReturnPipeSize, RD_Header_ColName, IDS_HMCALC_PIPESIZE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Distribution_ReturnPipeSize, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Length.
		_SetStaticText( CD_Distribution_ReturnPipeLength, RD_Header_ColName, IDS_HMCALC_PIPEL );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Distribution_ReturnPipeLength, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Distribution_ReturnPipeLength, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Dp.
		_SetStaticText( CD_Distribution_ReturnPipeDp, RD_Header_ColName, IDS_HMCALC_DP );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Distribution_ReturnPipeDp, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Flow.
		_SetStaticText( CD_Distribution_ReturnPipeTotalQ, RD_Header_ColName, IDS_HMCALC_FLOW );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Distribution_ReturnPipeTotalQ, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Diversity factor.
		_SetStaticText( CD_Distribution_ReturnDiversityFactor, RD_Header_ColName, IDS_HMCALC_DIVERSITYFACTOR );

		// linear Dp.
		_SetStaticText( CD_Distribution_ReturnPipeLinDp, RD_Header_ColName, IDS_HMCALC_LINDP );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Distribution_ReturnPipeLinDp, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Velocity.
		_SetStaticText( CD_Distribution_ReturnPipeVelocity, RD_Header_ColName, IDS_HMCALC_PIPEVA );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Distribution_ReturnPipeVelocity, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( lFirstcol, RD_Header_Picture - 1, CD_Distribution_ReturnPipeSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				SPREAD_COLOR_NONE );

		SetCellBorder( lFirstcol, RD_Header_FirstCirc - 1, CD_Distribution_ReturnPipeSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				SPREAD_COLOR_NONE );

		// Collapse button
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_Distribution_ReturnPipeSep, RD_Header_ButDlg, CD_Distribution_ReturnPipeTotalQ, CD_Distribution_ReturnPipeVelocity,
					GetMaxRows() );

			ChangeExpandCollapseColumnButtonState( CD_Distribution_ReturnPipeSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Accessories on distribution return pipe.
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		AddCellSpanW( CD_Distribution_ReturnAccDesc, RD_Header_GroupName, CD_Distribution_ReturnAccDp - CD_Distribution_ReturnAccDesc + 1, 2 );
		_SetStaticText( CD_Distribution_ReturnAccDesc, RD_Header_GroupName, IDS_HMCALC_ACCESSORIES );

		// Description.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		_SetStaticText( CD_Distribution_ReturnAccDesc, RD_Header_ColName, IDS_HMCALC_ACCDESCRIPTION );

		// Data needed for printing.
		_SetStaticText( CD_Distribution_ReturnAccData, RD_Header_ColName, IDS_HMCALCINPUTDATA );

		// Dp.
		_SetStaticText( CD_Distribution_ReturnAccDp, RD_Header_ColName, IDS_HMCALC_DP );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Distribution_ReturnAccDp, RD_Header_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( CD_Distribution_ReturnAccDesc, RD_Header_Picture - 1, CD_Distribution_ReturnAccDp, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_Distribution_ReturnAccDesc, RD_Header_FirstCirc - 1, CD_Distribution_ReturnAccDp, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	}

	// Add separator in header for the resizing feature.
	SetBorderRange( CD_Distribution_Name, RD_Header_ColName, CD_Distribution_Pointer, RD_Header_Unit, SS_BORDERTYPE_LEFT, 
			SS_BORDERSTYLE_SOLID, RGB( 192, 192, 192 ) );

	// Because the 'CD_Distribution_Pos' column is frozen, left border of the 'CD_Distribution_Name' column is never shown.
	SetBorderRange( CD_Distribution_Pos, RD_Header_ColName, CD_Distribution_Pos, RD_Header_Unit, SS_BORDERTYPE_RIGHT, 
			SS_BORDERSTYLE_SOLID, RGB( 192, 192, 192 ) );

	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateMainExpandCollapseButton( CD_Distribution_Pos, RD_Header_ButDlg );
	}
}

void CSheetHMCalc::_InitCircuitsSheet( CDS_HydroMod *pHM )
{
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == m_mapSSheetColumnWidth.size() || 0 == m_mapSSheetColumnWidth.count( SheetDescription::SCircuit ) )
	{
		_ResetColumnWidth( SheetDescription::SCircuit );
	}

	_ClearAllCombos( SheetDescription::SCircuit );

	SetMaxRows( RD_Header_FirstCirc - 1 );
	SetMaxCols( CD_Circuit_Pointer );

	// All cells are static by default and filled with _T("").
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	SetFreeze( CD_Circuit_Pos, RD_Header_FirstCirc - 1 );

	// Verify the project is frozen.
	bool bFreezed = false;

	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL != pPrjParam )
	{
		bFreezed = pPrjParam->IsFreezed();
	}

	// Increase ROW height.
	double dRowHeight = 12.75;// Reference
	SetRowHeight( RD_Header_GroupName, dRowHeight * 1.5 );
	SetRowHeight( RD_Header_ButDlg, dRowHeight * 1.2 );
	SetRowHeight( RD_Header_ColName, dRowHeight * 2 );
	SetRowHeight( RD_Header_Unit, dRowHeight * 1.2 );
	SetRowHeight( RD_Header_Picture, dRowHeight );

	// Set columns size.

	// Unit.
	if( false == m_bPrinting )
	{
		SetColWidthInPixels( CD_Circuit_Sub, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_Sub] );
	}
	else
	{
		ShowCol( CD_Circuit_Sub, FALSE );
	}

	SetColWidthInPixels( CD_Circuit_Pos, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_Pos] );
	SetColWidthInPixels( CD_Circuit_Name, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_Name] );
	SetColWidthInPixels( CD_Circuit_Desc, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_Desc] );
	SetColWidthInPixels( CD_Circuit_DescSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_DescSep] );
	SetColWidthInPixels( CD_Circuit_UnitDesc, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitDesc] );
	SetColWidthInPixels( CD_Circuit_UnitQ, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitQ] );
	SetColWidthInPixels( CD_Circuit_UnitP, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitP] );
	SetColWidthInPixels( CD_Circuit_UnitDT, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitDT] );
	SetColWidthInPixels( CD_Circuit_UnitDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitDp] );
	SetColWidthInPixels( CD_Circuit_UnitQref, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitQref] );
	SetColWidthInPixels( CD_Circuit_UnitDpref, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitDpref] );
	SetColWidthInPixels( CD_Circuit_UnitKv, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitKv] );
	SetColWidthInPixels( CD_Circuit_UnitCv, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitCv] );
	SetColWidthInPixels( CD_Circuit_UnitSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_UnitSep] );

	// Pump.
	SetColWidthInPixels( CD_Circuit_PumpHUser, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PumpHUser] );
	SetColWidthInPixels( CD_Circuit_PumpHMin, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PumpHMin] );
	SetColWidthInPixels( CD_Circuit_PumpSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PumpSep] );

	// CV.
	SetColWidthInPixels( CD_Circuit_CVName, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVName] );
	SetColWidthInPixels( CD_Circuit_CVDesc, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVDesc] );
	SetColWidthInPixels( CD_Circuit_CVKvsMax, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVKvsMax] );
	SetColWidthInPixels( CD_Circuit_CVKvs, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVKvs] );
	SetColWidthInPixels( CD_Circuit_CVSet, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVSet] );
	SetColWidthInPixels( CD_Circuit_CVDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVDp] );
	SetColWidthInPixels( CD_Circuit_CVDpMin, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVDpMin] );
	SetColWidthInPixels( CD_Circuit_CVAuth, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVAuth] );
	SetColWidthInPixels( CD_Circuit_CVDesignAuth, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVDesignAuth] );
	SetColWidthInPixels( CD_Circuit_CVSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_CVSep] );

	// Actuators.
	SetColWidthInPixels( CD_Circuit_ActuatorName, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_ActuatorName] );
	SetColWidthInPixels( CD_Circuit_ActuatorPowerSupply, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_ActuatorPowerSupply] );
	SetColWidthInPixels( CD_Circuit_ActuatorInputSignal, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_ActuatorInputSignal] );
	SetColWidthInPixels( CD_Circuit_ActuatorOutputSignal, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_ActuatorOutputSignal] );
	SetColWidthInPixels( CD_Circuit_ActuatorRelayType, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_ActuatorRelayType] );
	SetColWidthInPixels( CD_Circuit_ActuatorFailSafe, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_ActuatorFailSafe] );
	SetColWidthInPixels( CD_Circuit_ActuatorDRP, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_ActuatorDRP] );
	SetColWidthInPixels( CD_Circuit_ActuatorSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_ActuatorSep] );

	// HYS-1676: Add smart valve to Sheet HM Calc
	// Smart valve.
	SetColWidthInPixels( CD_Circuit_SmartValveName, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveName] );
	SetColWidthInPixels( CD_Circuit_SmartValveDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveDp] );
	SetColWidthInPixels( CD_Circuit_SmartValveDpMin, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveDpMin] );
	SetColWidthInPixels( CD_Circuit_SmartValveControlMode, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveControlMode] );
	SetColWidthInPixels( CD_Circuit_SmartValveLocation, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveLocation] );
	SetColWidthInPixels( CD_Circuit_SmartValveKvs, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveKvs] );
	SetColWidthInPixels( CD_Circuit_SmartValveMoreInfo, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveMoreInfo] );
	SetColWidthInPixels( CD_Circuit_SmartValveProductSet, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveProductSet] );
	SetColWidthInPixels( CD_Circuit_SmartValveDpSensor, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveDpSensor] );
	SetColWidthInPixels( CD_Circuit_SmartValveDpl, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveDpl] );
	SetColWidthInPixels( CD_Circuit_SmartValveSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SmartValveSep] );

	// BV.
	SetColWidthInPixels( CD_Circuit_BvName, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvName] );
	SetColWidthInPixels( CD_Circuit_BvSet, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvSet] );
	SetColWidthInPixels( CD_Circuit_BvDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvDp] );
	SetColWidthInPixels( CD_Circuit_BvDpSig, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvDpSig] );
	SetColWidthInPixels( CD_Circuit_BvDpMin, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvDpMin] );
	SetColWidthInPixels( CD_Circuit_BvSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvSep] );

	// Shut-off valve.
	SetColWidthInPixels( CD_Circuit_SvPict, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SvPict] );
	SetColWidthInPixels( CD_Circuit_SvName, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SvName] );
	SetColWidthInPixels( CD_Circuit_SvDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SvDp] );
	SetColWidthInPixels( CD_Circuit_SvSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_SvSep] );

	// DpC.
	SetColWidthInPixels( CD_Circuit_DpCName, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_DpCName] );
	SetColWidthInPixels( CD_Circuit_DpCDpLr, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_DpCDpLr] );
	SetColWidthInPixels( CD_Circuit_DpCSet, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_DpCSet] );
	SetColWidthInPixels( CD_Circuit_DpCDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_DpCDp] );
	SetColWidthInPixels( CD_Circuit_DpCDpl, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_DpCDpl] );
	SetColWidthInPixels( CD_Circuit_DpCDpMin, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_DpCDpMin] );
	SetColWidthInPixels( CD_Circuit_DpCSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_DpCSep] );

	// BV Bypass.
	SetColWidthInPixels( CD_Circuit_BvBypPict, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvBypPict] );
	SetColWidthInPixels( CD_Circuit_BvBypName, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvBypName] );
	SetColWidthInPixels( CD_Circuit_BvBypSet, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvBypSet] );
	SetColWidthInPixels( CD_Circuit_BvBypSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_BvBypSep] );

	// Circuit pipe.
	SetColWidthInPixels( CD_Circuit_PipeSerie, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeSerie] );
	SetColWidthInPixels( CD_Circuit_PipeSize, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeSize] );
	SetColWidthInPixels( CD_Circuit_PipeL, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeL] );
	SetColWidthInPixels( CD_Circuit_PipeDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeDp] );
	SetColWidthInPixels( CD_Circuit_PipeLinDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeLinDp] );
	SetColWidthInPixels( CD_Circuit_PipeV, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeV] );
	SetColWidthInPixels( CD_Circuit_PipeSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeSep] );

	// Circuit pipe accessories.
	SetColWidthInPixels( CD_Circuit_PipeAccDesc, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeAccDesc] );
	SetColWidthInPixels( CD_Circuit_PipeAccData, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeAccData] );
	SetColWidthInPixels( CD_Circuit_PipeAccDp, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeAccDp] );
	SetColWidthInPixels( CD_Circuit_PipeAccSep, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_PipeAccSep] );

	SetColWidthInPixels( CD_Circuit_Pointer, m_mapSSheetColumnWidth[SheetDescription::SCircuit][CD_Circuit_Pointer] );

	// All cells are not selectable.
	SetCellProperty( CD_Circuit_Sub, RD_Header_GroupName, CD_Circuit_Pointer, RD_Header_FirstCirc - 1, CSSheet::_SSCellProperty::CellNoSelection, true );

	// Flag unhidden columns.
	SetFlagShowEvenEmpty( CD_Circuit_DescSep, CD_Circuit_DescSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_UnitSep, CD_Circuit_UnitSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_PumpSep, CD_Circuit_PumpSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_CVSep, CD_Circuit_CVSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_ActuatorSep, CD_Circuit_ActuatorSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_SmartValveSep, CD_Circuit_SmartValveSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_BvSep, CD_Circuit_BvSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_SvSep, CD_Circuit_SvSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_DpCSep, CD_Circuit_DpCSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_BvBypSep, CD_Circuit_BvBypSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_PipeSep, CD_Circuit_PipeSep, true );
	SetFlagShowEvenEmpty( CD_Circuit_PipeSize, CD_Circuit_PipeSize, true );
	SetFlagShowEvenEmpty( CD_Circuit_PipeDp, CD_Circuit_PipeDp, true );
	SetFlagShowEvenEmpty( CD_Circuit_PipeL, CD_Circuit_PipeL, true );
	SetFlagShowEvenEmpty( CD_Circuit_PipeAccDp, CD_Circuit_PipeAccDp, true );
	SetFlagShowEvenEmpty( CD_Circuit_PipeAccSep, CD_Circuit_PipeAccSep, true );

	// Used to determine background color for group name.
	// 0 for default TitleGroup default color, 1 for _TAH_TITLE_GROUP2 color.
	bool bUnitQ = false;
	bool bUnitDp = false;
	bool bUnitDT = false;
	bool bColorGroup = false;
	bool bPump = false;			// Display or not the pump group.
	bool bBV = false;			// Display or not group BV.
	bool bBypBV = false;		// Display or not group Bypass BV.
	bool bCV = false;			// Display or not group CV.
	// HYS-1676
	bool bSmartValve = false;	// Display or not group Smart valve.
	bool bSV = false;			// Display or not group Shut-off valve.
	bool bActuator = false;		// Display or not group Actuator.
	bool bCVPreset = false;		// Presetting is displayed only for presetable TA valve.
	bool bCVKv = false;			// Kvs and Kvmax are displayed only for not presetable valve.
	bool bCVTA = false;			// Hidde column Name if there are no TA Valve.
	bool bCVNOTA = false;
	bool bAuthority = false;
	bool bDpC = false;
	bool bHidePipeGroup = true;
	bool bPipeSerie = true;				// Same series for all circuit pipes by default.
	bool bSub = false;
	std::map<int, bool> mapTUQVariables;
	std::map<int, bool> mapTUDpVariables;
	m_pPipeCircTab = NULL;

	if( NULL != pHM )
	{
		// Scan parent module and children.
		IDPTR IDPtr = pHM->GetIDPtr();
		bool bFirst = true;

		while( _T('\0') != *IDPtr.ID )
		{
			CDS_HydroMod *pCHM = (CDS_HydroMod *)IDPtr.MP;

			// Save QType and DpType to know what are the columns to show.
			if( 0 == mapTUQVariables.count( pCHM->GetpTermUnit()->GetQType() ) )
			{
				mapTUQVariables[( int )pCHM->GetpTermUnit()->GetQType()] = true;
			}

			if( 0 == mapTUDpVariables.count( pCHM->GetpTermUnit()->GetDpType() ) )
			{
				mapTUDpVariables[( int )pCHM->GetpTermUnit()->GetDpType()] = true;
			}

			// HYS-1202: if secondary side exists the Dp and Flow columns are not editable
			if( NULL != pCHM->GetpSchcat() && false == pCHM->GetpSchcat()->IsSecondarySideExist() )
			{
				if( false == pCHM->GetpTermUnit()->IsVirtual()
					&& CDS_HydroMod::eDpType::Dp == pCHM->GetpTermUnit()->GetDpType() )
				{
					bUnitDp = true;
				}

				if( false == pCHM->GetpTermUnit()->IsVirtual()
					&& CTermUnit::_QType::Q == pCHM->GetpTermUnit()->GetQType() )
				{
					bUnitQ = true;
					// HYS-1882
					if( true == pCHM->IsDTFieldDisplayedInFlowMode() )
					{
						bUnitDT = true;
					}
				}
			}

			// We take only the case for root module (level = 0) with pump that are not a circuit in injection.
			if( 0 == pCHM->GetLevel() && pCHM->GetpPump() != NULL
					&& ( NULL != pHM->GetpSchcat() && false == pHM->GetpSchcat()->IsSecondarySideExist() ) )
			{
				bPump = true;
			}

			if( true == pCHM->IsBvExist( true ) )
			{
				bBV = true;
			}

			if( true == pCHM->IsCvExist( true ) && eb3True == pCHM->GetpCV()->IsCVLocInPrimary() )
			{
				bCV = true;

				if( true == pCHM->GetpCV()->IsTaCV() )
				{
					bCVTA = true;
				}

				if( false == pCHM->GetpCV()->IsTaCV() )
				{
					bCVNOTA = true;
				}

				// Not TA Valve with kvs value show Kvs and Kvmax
				if (_T('\0') == *(pCHM->GetpCV()->GetCvIDPtr().ID) )
				{
					bCVKv = true;
				}
				// Not presettable valve, show Kvs and Kvmax.
				else if( false == pCHM->GetpCV()->IsPresettable() && _T('\0') != *(pCHM->GetpCV()->GetCvIDPtr().ID ) )
				{
					bCVKv = true;
				}

				// Presetting is displayed for presettable valve.
				if( true == pCHM->GetpCV()->IsPresettable() && _T('\0') != *( pCHM->GetpCV()->GetCvIDPtr().ID ) )
				{
					bCVPreset = true ;
				}

				if( false == pCHM->GetpCV()->IsOn_Off() && eb3False == pCHM->GetpCV()->IsPICV()
						&& eb3False == pCHM->GetpCV()->IsDpCBCV() )
				{
					bAuthority = true;
				}

				// Check if actuator exist.
				// Remark: Either user has selected an actuator, and in this case we can show the group, or no actuator selected but
				//         the control valve has actuator group. In that case, user must be able to choose from a combo.
				if( NULL != pCHM->GetpCV()->GetActrIDPtr().MP
						|| ( NULL != pCHM->GetpCV()->GetpCV() && NULL != pCHM->GetpCV()->GetpCV()->GetActuatorGroupIDPtr().MP ) )
				{
					bActuator = true;
				}
			}

			// HYS-1676: Add smart valve to Sheet HM Calc
			if( true == pCHM->IsSmartControlValveExist( true ) || true == pCHM->IsSmartDpCExist( true ) )
			{
				bSmartValve = true;
			}

			if( true == pCHM->IsDpCExist( true ) )
			{
				bDpC = true;
			}

			if( true == pCHM->IsBvBypExist( true ) )
			{
				bBypBV = true;
			}

			if( true == pCHM->IsShutoffValveExist( true ) )
			{
				bSV = true;
			}

			// Test circuit pipe series only for children or parent if in injection.
			if( true == bPipeSerie && ( pHM != pCHM || ( NULL != pCHM->GetpSchcat() && true == pCHM->GetpSchcat()->IsSecondarySideExist() ) ) )
			{
				CPipes *pPipe = pCHM->GetpCircuitPrimaryPipe();

				if( NULL != pPipe )
				{
					if( NULL == m_pPipeCircTab )
					{
						m_pPipeCircTab = pPipe->GetPipeSeries();
					}
					else if( NULL != pPipe->GetPipeSeries() && m_pPipeCircTab != pPipe->GetPipeSeries() )
					{
						m_pPipeCircTab = NULL;
						bPipeSerie = false;
					}
				}
			}

			// Hide Pipe Group when we have only module.
			if( false == pCHM->IsaModule() || ( NULL != pCHM->GetpSchcat() && true == pCHM->GetpSchcat()->IsSecondarySideExist() ) )
			{
				bHidePipeGroup &= false;
			}

			if( true == bFirst )
			{
				IDPtr = pHM->GetFirst();
				bFirst = false;
			}
			else
			{
				IDPtr = pHM->GetNext();
			}

			if( true == pCHM->IsaModule() && pCHM != pHM )
			{
				bSub = true;
			}
		};
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Circuit' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	AddCellSpanW( CD_Circuit_Name, RD_Header_GroupName, CD_Circuit_Desc - CD_Circuit_Name + 1, 2 );
	_SetStaticText( CD_Circuit_Name, RD_Header_GroupName, IDS_HMCALC_CIRCUIT );
	bColorGroup ^= 1;

	// Name.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	_SetStaticText( CD_Circuit_Name, RD_Header_ColName, IDS_HMCALC_NAME );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_Name, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Description.
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	_SetStaticText( CD_Circuit_Desc, RD_Header_ColName, IDS_HMCALC_DESC );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_Desc, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Border.
	SetCellBorder( CD_Circuit_Name, RD_Header_Picture - 1, CD_Circuit_Desc, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
			SPREAD_COLOR_NONE );

	SetCellBorder( CD_Circuit_Name, RD_Header_FirstCirc - 1, CD_Circuit_Desc, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
			SPREAD_COLOR_NONE );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Unit' group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main Title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	if( true == bColorGroup )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	}

	bColorGroup ^= 1;
	AddCellSpanW( CD_Circuit_UnitDesc, RD_Header_GroupName, CD_Circuit_UnitSep - CD_Circuit_UnitDesc, 2 );
	_SetStaticText( CD_Circuit_UnitDesc, RD_Header_GroupName, ( false == IsInjectionCircuitExist( pHM ) ) ? IDS_HMCALC_UNIT : IDS_HMCALC_UNITPRIMSIDE );

	// Description.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	_SetStaticText( CD_Circuit_UnitDesc, RD_Header_ColName, IDS_HMCALC_DESC );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitDesc, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Q.
	_SetStaticText( CD_Circuit_UnitQ, RD_Header_ColName, IDS_HMCALC_Q );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Circuit_UnitQ, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( true == bUnitQ && false == bFreezed && false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitQ, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	if( mapTUQVariables.count( CTermUnit::_QType::PdT ) > 0 )
	{
		// Power.
		_SetStaticText( CD_Circuit_UnitP, RD_Header_ColName, IDS_HMCALC_P );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_UnitP, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitP, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_Circuit_UnitP, FALSE );
	}

	// DT.
	// HYS-1882: Show DT
	if( mapTUQVariables.count( CTermUnit::_QType::PdT ) > 0 || true == bUnitDT )
	{
		_SetStaticText( CD_Circuit_UnitDT, RD_Header_ColName, IDS_HMCALC_DT );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_UnitDT, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitDT, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_Circuit_UnitDT, FALSE );
	}
	
	// Dp.
	_SetStaticText( CD_Circuit_UnitDp, RD_Header_ColName, IDS_HMCALC_DP );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Circuit_UnitDp, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( true == bUnitDp && false == bFreezed && false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitDp, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// QRef.
	if( mapTUDpVariables.count( CDS_HydroMod::eDpType::QDpref ) > 0 )
	{
		_SetStaticText( CD_Circuit_UnitQref, RD_Header_ColName, IDS_HMCALC_QREF );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_UnitQref, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitQref, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_Circuit_UnitQref, FALSE );
	}

	// DpRef.
	if( mapTUDpVariables.count( CDS_HydroMod::eDpType::QDpref ) > 0 )
	{
		_SetStaticText( CD_Circuit_UnitDpref, RD_Header_ColName, IDS_HMCALC_DPREF );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_UnitDpref, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitDpref, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_Circuit_UnitDpref, FALSE );
	}

	// Kv.
	if( mapTUDpVariables.count( CDS_HydroMod::eDpType::Kv ) > 0 )
	{
		_SetStaticText( CD_Circuit_UnitKv, RD_Header_ColName, IDS_KV );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitKv, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_Circuit_UnitKv, FALSE );
	}

	// Cv.
	if( mapTUDpVariables.count( CDS_HydroMod::eDpType::Cv ) > 0 )
	{
		_SetStaticText( CD_Circuit_UnitCv, RD_Header_ColName, IDS_CV );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_UnitCv, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_Circuit_UnitCv, FALSE );
	}

	// Border.
	SetCellBorder( CD_Circuit_UnitDesc, RD_Header_Picture - 1, CD_Circuit_UnitSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	SetCellBorder( CD_Circuit_UnitDesc, RD_Header_FirstCirc - 1, CD_Circuit_UnitSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	if( mapTUQVariables.size() > 0 || mapTUDpVariables.size() > 0 )
	{
		if( false == m_bPrinting && false == m_bExporting )
		{
			// Collapse button.
			CreateExpandCollapseColumnButton( CD_Circuit_UnitSep, RD_Header_ButDlg, CD_Circuit_UnitP, CD_Circuit_UnitCv, GetMaxRows() );
			// Exclude col Dp from collapse function.
			SetFlagCanBeCollapsed( CD_Circuit_UnitDp, CD_Circuit_UnitDp, false );
			ChangeExpandCollapseColumnButtonState( CD_Circuit_UnitSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pump.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	if( true == bPump )
	{
		// Main Title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_Circuit_PumpHUser, RD_Header_GroupName, CD_Circuit_PumpSep - CD_Circuit_PumpHUser, 2 );
		_SetStaticText( CD_Circuit_PumpHUser, RD_Header_GroupName, IDS_HMCALC_PUMP );

		// Pump head.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		_SetStaticText( CD_Circuit_PumpHUser, RD_Header_ColName, IDS_HMCALC_PUMPH );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_PumpHUser, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_PumpHUser, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Hmin.
		_SetStaticText( CD_Circuit_PumpHMin, RD_Header_ColName, IDS_HMCALC_HMIN );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_PumpHMin, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( CD_Circuit_PumpHUser, RD_Header_Picture - 1, CD_Circuit_PumpSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
		SetCellBorder( CD_Circuit_PumpHUser, RD_Header_FirstCirc - 1, CD_Circuit_PumpSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	}
	else
	{
		ShowColRange( CD_Circuit_PumpHUser, CD_Circuit_PumpSep, FALSE );
	}

	// Control Valve.
	if( true == bCV )
	{
		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_Circuit_CVName, RD_Header_GroupName, CD_Circuit_CVSep - CD_Circuit_CVName, 2 );
		_SetStaticText( CD_Circuit_CVName, RD_Header_GroupName, IDS_HMCALC_CV );

		// Name.
		if( true == bCVTA )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			_SetStaticText( CD_Circuit_CVName, RD_Header_ColName, IDS_HMCALC_NAME );

			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_COMBO, CD_Circuit_CVName, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}
		else
		{
			ShowCol( CD_Circuit_CVName, FALSE );
		}

		// Description.
		if( true == bCVNOTA )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			_SetStaticText( CD_Circuit_CVDesc, RD_Header_ColName, IDS_HMCALC_DESC );

			if( false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_CVDesc, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}
		else
		{
			ShowCol( CD_Circuit_CVDesc, FALSE );
		}

		// Kvsmax.
		if( true == bCVKv )
		{
			CString str;

			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_KVSMAX );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_CVMAX );
			}

			_SetStaticText( CD_Circuit_CVKvsMax, RD_Header_ColName, str );
		}
		else
		{
			ShowCol( CD_Circuit_CVKvsMax, FALSE );
		}

		// Kvs.
		if( true == bCVKv )
		{
			CString str;

			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_KVS );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_CV );
			}

			_SetStaticText( CD_Circuit_CVKvs, RD_Header_ColName, str );

			if( false == bFreezed && false == m_bPrinting && false == m_bExporting && true == bCVNOTA )
			{
				SetPictureCellWithID( IDI_COMBOEDIT, CD_Circuit_CVKvs, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}
		else
		{
			ShowCol( CD_Circuit_CVKvs, FALSE );
		}

		// Setting.
		if( true == bCVPreset )
		{
			_SetStaticText( CD_Circuit_CVSet, RD_Header_ColName, IDS_HMCALC_SETTING );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			_SetStaticText( CD_Circuit_CVSet, RD_Header_Unit, IDS_TURNS );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		}
		else
		{
			ShowCol( CD_Circuit_CVSet, FALSE );
		}

		// Dp.
		_SetStaticText( CD_Circuit_CVDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_CVDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Dp Min.
		_SetStaticText( CD_Circuit_CVDpMin, RD_Header_ColName, IDS_HMCALC_DPMIN );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_CVDpMin, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Authority.
		if( true == bAuthority )
		{
			_SetStaticText( CD_Circuit_CVAuth, RD_Header_ColName, IDS_HMCALC_MINAUTHOR );
		}
		else
		{
			ShowCol( CD_Circuit_CVAuth, FALSE );
		}

		// Design Authority.
		if( true == bAuthority )
		{
			_SetStaticText( CD_Circuit_CVDesignAuth, RD_Header_ColName, IDS_HMCALC_DESAUTHOR );
		}
		else
		{
			ShowCol( CD_Circuit_CVDesignAuth, FALSE );
		}

		// Border.
		SetCellBorder( CD_Circuit_CVName, RD_Header_Picture - 1, CD_Circuit_CVSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, 
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_Circuit_CVName, RD_Header_FirstCirc - 1, CD_Circuit_CVSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, 
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( true == bCVPreset )
		{
			if( false == m_bPrinting && false == m_bExporting )
			{
				CreateExpandCollapseColumnButton( CD_Circuit_CVSep, RD_Header_ButDlg, CD_Circuit_CVSet, CD_Circuit_CVSet, GetMaxRows() );
				ChangeExpandCollapseColumnButtonState( CD_Circuit_CVSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
			}
		}
	}
	else
	{
		ShowColRange( CD_Circuit_CVName, CD_Circuit_CVSep, FALSE );
	}

	// Actuator.
	if( true == bActuator )
	{
		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_Circuit_ActuatorName, RD_Header_GroupName, CD_Circuit_ActuatorSep - CD_Circuit_ActuatorName, 2 );
		_SetStaticText( CD_Circuit_ActuatorName, RD_Header_GroupName, IDS_HMCALC_ACTUATOR );

		// Name.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		_SetStaticText( CD_Circuit_ActuatorName, RD_Header_ColName, IDS_HMCALC_NAME );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_ActuatorName, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Power supply.
		_SetStaticText( CD_Circuit_ActuatorPowerSupply, RD_Header_ColName, IDS_HMCALC_POWERSUPPLY );

		// Input signal.
		_SetStaticText( CD_Circuit_ActuatorInputSignal, RD_Header_ColName, IDS_HMCALC_INPUTSIGNAL );

		// Output signal.
		_SetStaticText( CD_Circuit_ActuatorOutputSignal, RD_Header_ColName, IDS_HMCALC_OUTPUTSIGNAL );

		// Relay type.
		_SetStaticText( CD_Circuit_ActuatorRelayType, RD_Header_ColName, IDS_HMCALC_RELAYTYPE );

		// Fail-safe tyye.
		_SetStaticText( CD_Circuit_ActuatorFailSafe, RD_Header_ColName, IDS_HMCALC_FAILSAFE );

		// return position.
		_SetStaticText( CD_Circuit_ActuatorDRP, RD_Header_ColName, IDS_SSHEETSSEL_ACTRETURNPOSITION );

		// Border.
		SetCellBorder( CD_Circuit_ActuatorName, RD_Header_Picture - 1, CD_Circuit_ActuatorSep - 1, RD_Header_Picture - 1, true, 
				SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_Circuit_ActuatorName, RD_Header_FirstCirc - 1, CD_Circuit_ActuatorSep - 1, RD_Header_FirstCirc - 1, true, 
				SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_Circuit_ActuatorSep, RD_Header_ButDlg, CD_Circuit_ActuatorPowerSupply, CD_Circuit_ActuatorDRP, GetMaxRows() );
			ChangeExpandCollapseColumnButtonState( CD_Circuit_ActuatorSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}
	else
	{
		ShowColRange( CD_Circuit_ActuatorName, CD_Circuit_ActuatorSep, FALSE );
	}

	// HYS-1676: Add smart valve to Sheet HM Calc
	// Smart Valve.
	if( true == bSmartValve )
	{
		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_Circuit_SmartValveName, RD_Header_GroupName, CD_Circuit_SmartValveSep - CD_Circuit_SmartValveName, 2 );
		_SetStaticText( CD_Circuit_SmartValveName, RD_Header_GroupName, IDS_HMCALC_SMARTVALVE );

		// Name.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		_SetStaticText( CD_Circuit_SmartValveName, RD_Header_ColName, IDS_HMCALC_NAME );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_SmartValveName, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Dp.
		_SetStaticText( CD_Circuit_SmartValveDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_SmartValveDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Dp min.
		_SetStaticText( CD_Circuit_SmartValveDpMin, RD_Header_ColName, IDS_HMCALC_DPMIN );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_SmartValveDpMin, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Control mode (It's only for smart control valve).
		_SetStaticText( CD_Circuit_SmartValveControlMode, RD_Header_ColName, IDS_HMCALC_CONTROLMODE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_SmartValveControlMode, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Location.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		_SetStaticText( CD_Circuit_SmartValveLocation, RD_Header_ColName, IDS_HMCALC_SMARTVLOC );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_SmartValveLocation, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Kvs.
		CString str;

		if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KVS );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CV );
		}

		_SetStaticText( CD_Circuit_SmartValveKvs, RD_Header_ColName, str );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting && true == bCVNOTA )
		{
			SetPictureCellWithID( IDI_COMBOEDIT, CD_Circuit_SmartValveKvs, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// More information column (It's only for smart control valve).
		if( false == m_bPrinting && false == m_bExporting )
		{
			_SetStaticText( CD_Circuit_SmartValveMoreInfo, RD_Header_ColName, IDS_HMCALC_MORE );
		}

		// Product set (It's only for smart differential pressure controller).
		_SetStaticText( CD_Circuit_SmartValveProductSet, RD_Header_ColName, IDS_HMCALC_SETS );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_SmartValveProductSet, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Dp sensor (It's only for smart differential pressure controller).
		_SetStaticText( CD_Circuit_SmartValveDpSensor, RD_Header_ColName, IDS_HMCALC_DPSENSOR );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_SmartValveDpSensor, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Dpl (It's only for smart differential pressure controller).
		_SetStaticText( CD_Circuit_SmartValveDpl, RD_Header_ColName, IDS_HMCALC_DPCDPL );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_SmartValveDpl, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( CD_Circuit_SmartValveName, RD_Header_Picture - 1, CD_Circuit_SmartValveSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_Circuit_SmartValveName, RD_Header_FirstCirc - 1, CD_Circuit_SmartValveSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_Circuit_SmartValveSep, RD_Header_ButDlg, CD_Circuit_SmartValveKvs, CD_Circuit_SmartValveMoreInfo, GetMaxRows() );
			ChangeExpandCollapseColumnButtonState( CD_Circuit_SmartValveSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}

	}
	else
	{
		ShowColRange( CD_Circuit_SmartValveName, CD_Circuit_SmartValveSep, FALSE );
	}

	// Regulating Valve.
	if( true == bBV )
	{
		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_Circuit_BvName, RD_Header_GroupName, CD_Circuit_BvSep - CD_Circuit_BvName, 2 );
		_SetStaticText( CD_Circuit_BvName, RD_Header_GroupName, IDS_HMCALC_BV );

		// Name.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		_SetStaticText( CD_Circuit_BvName, RD_Header_ColName, IDS_HMCALC_BVNAME );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_BvName, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Setting.
		_SetStaticText( CD_Circuit_BvSet, RD_Header_ColName, IDS_HMCALC_SETTING );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_BvSet, RD_Header_Unit, IDS_TURNS );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Dp.
		_SetStaticText( CD_Circuit_BvDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_BvDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// DpSig.
		_SetStaticText( CD_Circuit_BvDpSig, RD_Header_ColName, IDS_HMCALC_DPSIG );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_BvDpSig, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// DpMin.
		_SetStaticText( CD_Circuit_BvDpMin, RD_Header_ColName, IDS_HMCALC_DPFULLOPEN );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_BvDpMin, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( CD_Circuit_BvName, RD_Header_Picture - 1, CD_Circuit_BvSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, 
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_Circuit_BvName, RD_Header_FirstCirc - 1, CD_Circuit_BvSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, 
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_Circuit_BvSep, RD_Header_ButDlg, CD_Circuit_BvSet, CD_Circuit_BvDpMin, GetMaxRows() );
			SetFlagCanBeCollapsed( CD_Circuit_BvDp, CD_Circuit_BvDp, false );
			SetFlagCanBeCollapsed( CD_Circuit_BvDpSig, CD_Circuit_BvDpSig, false );
			ChangeExpandCollapseColumnButtonState( CD_Circuit_BvSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}
	else
	{
		ShowColRange( CD_Circuit_BvName, CD_Circuit_BvSep, FALSE );
	}

	// Shut-off valve.
	if( true == bSV )
	{
		SetFlagShowEvenEmpty( CD_Circuit_SvPict, CD_Circuit_SvPict, true );
		SetFlagShowEvenEmpty( CD_Circuit_SvName, CD_Circuit_SvName, true );

		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_Circuit_SvPict, RD_Header_GroupName, CD_Circuit_SvSep - CD_Circuit_SvPict, 2 );
		_SetStaticText( CD_Circuit_SvPict, RD_Header_GroupName, IDS_HMCALC_SV );

		// Name.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		AddCellSpanW( CD_Circuit_SvPict, RD_Header_ColName, 2, 1 );
		_SetStaticText( CD_Circuit_SvPict, RD_Header_ColName, IDS_HMCALC_SVNAME );

		AddCellSpanW( CD_Circuit_SvPict, RD_Header_Picture, 2, 1 );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_SvPict, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Dp.
		_SetStaticText( CD_Circuit_SvDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_SvDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( CD_Circuit_SvPict, RD_Header_Picture - 1, CD_Circuit_SvSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_Circuit_SvPict, RD_Header_FirstCirc - 1, CD_Circuit_SvSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, 
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	}
	else
	{
		ShowColRange( CD_Circuit_SvPict, CD_Circuit_SvSep, FALSE );
	}

	if( true == bDpC )
	{
		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_Circuit_DpCName, RD_Header_GroupName, CD_Circuit_DpCSep - CD_Circuit_DpCName, 2 );
		_SetStaticText( CD_Circuit_DpCName, RD_Header_GroupName, IDS_HMCALC_DPC );

		// Name.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		_SetStaticText( CD_Circuit_DpCName, RD_Header_ColName, IDS_HMCALC_DPCNAME );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_DpCName, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// DpL range.
		_SetStaticText( CD_Circuit_DpCDpLr, RD_Header_ColName, IDS_HMCALC_DPLRANGE );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_DpCDpLr, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Setting.
		_SetStaticText( CD_Circuit_DpCSet, RD_Header_ColName, IDS_HMCALC_SETTING );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_DpCSet, RD_Header_Unit, IDS_TURNS );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Dp.
		_SetStaticText( CD_Circuit_DpCDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_DpCDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// DpL.
		_SetStaticText( CD_Circuit_DpCDpl, RD_Header_ColName, IDS_HMCALC_DPCDPL );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_DpCDpl, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// DpMin.
		_SetStaticText( CD_Circuit_DpCDpMin, RD_Header_ColName, IDS_HMCALC_DPCDPFULLOPEN );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_DpCDpMin, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( CD_Circuit_DpCName, RD_Header_Picture - 1, CD_Circuit_DpCDpMin, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_Circuit_DpCName, RD_Header_FirstCirc - 1, CD_Circuit_DpCDpMin, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_Circuit_DpCSep, RD_Header_ButDlg, CD_Circuit_DpCSet, CD_Circuit_DpCDpMin, GetMaxRows() );
			SetFlagCanBeCollapsed( CD_Circuit_DpCDp, CD_Circuit_DpCDpl, false );
			ChangeExpandCollapseColumnButtonState( CD_Circuit_DpCSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}
	else
	{
		ShowColRange( CD_Circuit_DpCName, CD_Circuit_DpCSep, FALSE );
	}

	// Bypass Regulating Valve.
	if( true == bBypBV )
	{
		SetFlagShowEvenEmpty( CD_Circuit_BvBypPict, CD_Circuit_BvBypPict, true );
		SetFlagShowEvenEmpty( CD_Circuit_BvBypName, CD_Circuit_BvBypName, true );

		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_Circuit_BvBypPict, RD_Header_GroupName, CD_Circuit_BvBypSep - CD_Circuit_BvBypPict, 2 );
		_SetStaticText( CD_Circuit_BvBypPict, RD_Header_GroupName, IDS_HMCALC_BYPBV );

		// Name.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		AddCellSpanW( CD_Circuit_BvBypPict, RD_Header_ColName, 2, 1 );
		_SetStaticText( CD_Circuit_BvBypPict, RD_Header_ColName, IDS_HMCALC_BVNAME );
		AddCellSpanW( CD_Circuit_BvBypPict, RD_Header_Picture, 2, 1 );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_BvBypPict, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Setting.
		_SetStaticText( CD_Circuit_BvBypSet, RD_Header_ColName, IDS_HMCALC_SETTING );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_BvBypSet, RD_Header_Unit, IDS_TURNS );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( CD_Circuit_BvBypPict, RD_Header_Picture - 1, CD_Circuit_BvBypSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
		SetCellBorder( CD_Circuit_BvBypPict, RD_Header_FirstCirc - 1, CD_Circuit_BvBypSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM,	SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	}
	else
	{
		ShowColRange( CD_Circuit_BvBypPict, CD_Circuit_BvBypSep, FALSE );
	}

	// 'Circuit Pipe' Group.
	if( false == bHidePipeGroup )
	{
		CString str;
		str = TASApp.LoadLocalizedString( IDS_HMCALC_CIRCUITPIPES );
		long lFirstCol = CD_Circuit_PipeSerie;
		bool bShowPipeSeries = true;

		if( NULL != m_pPipeCircTab )
		{
			lFirstCol = CD_Circuit_PipeSize;
			str += CString( _T("\r\n") ) + m_pPipeCircTab->GetName();
			ShowCol( CD_Circuit_PipeSerie, FALSE );
			bShowPipeSeries = false;
		}
		else
		{
			ShowCol( CD_Circuit_PipeSerie, TRUE );
		}

		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( lFirstCol, RD_Header_GroupName, CD_Circuit_PipeSep - lFirstCol, 2 );
		_SetStaticText( lFirstCol, RD_Header_GroupName, str );

		// Pipe series.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

		if( true == bShowPipeSeries )
		{
			_SetStaticText( CD_Circuit_PipeSerie, RD_Header_ColName, IDS_HMCALC_PIPESERIE );

			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_COMBO, CD_Circuit_PipeSerie, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}

		// Pipe size.
		_SetStaticText( CD_Circuit_PipeSize, RD_Header_ColName, IDS_HMCALC_PIPESIZE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_Circuit_PipeSize, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Length.
		_SetStaticText( CD_Circuit_PipeL, RD_Header_ColName, IDS_HMCALC_PIPEL );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ), tcName );
		_SetStaticText( CD_Circuit_PipeL, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Dp.
		_SetStaticText( CD_Circuit_PipeDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_PipeDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// linear Dp.
		_SetStaticText( CD_Circuit_PipeLinDp, RD_Header_ColName, IDS_HMCALC_LINDP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_PipeLinDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Velocity.
		_SetStaticText( CD_Circuit_PipeV, RD_Header_ColName, IDS_HMCALC_PIPEVA );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_Circuit_PipeV, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( lFirstCol, RD_Header_Picture - 1, CD_Circuit_PipeSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
		SetCellBorder( lFirstCol, RD_Header_FirstCirc - 1, CD_Circuit_PipeV, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_Circuit_PipeSep, RD_Header_ButDlg, CD_Circuit_PipeLinDp, CD_Circuit_PipeV, GetMaxRows() );
			ChangeExpandCollapseColumnButtonState( CD_Circuit_PipeSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}
	else
	{
		ShowColRange( CD_Circuit_PipeSerie, CD_Circuit_PipeSep, FALSE );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Circuit pipe accessories' group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	if( true == bColorGroup )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	}

	bColorGroup ^= 1;
	AddCellSpanW( CD_Circuit_PipeAccDesc, RD_Header_GroupName, CD_Circuit_PipeAccDp - CD_Circuit_PipeAccDesc + 1, 2 );
	_SetStaticText( CD_Circuit_PipeAccDesc, RD_Header_GroupName, IDS_HMCALC_CIRCUITPIPEACCESSORIES );

	// Description.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	_SetStaticText( CD_Circuit_PipeAccDesc, RD_Header_ColName, IDS_HMCALC_ACCDESCRIPTION );

	// Data needed for printing.
	_SetStaticText( CD_Circuit_PipeAccData, RD_Header_ColName, IDS_HMCALCINPUTDATA );

	// Dp.
	_SetStaticText( CD_Circuit_PipeAccDp, RD_Header_ColName, IDS_HMCALC_DP );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Circuit_PipeAccDp, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Border.
	SetCellBorder( CD_Circuit_PipeAccDesc, RD_Header_Picture - 1, CD_Circuit_PipeAccSep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	SetCellBorder( CD_Circuit_PipeAccDesc, RD_Header_FirstCirc - 1, CD_Circuit_PipeAccSep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Add separator in header for the resizing feature.
	SetBorderRange( CD_Circuit_Name, RD_Header_ColName, CD_Circuit_Pointer, RD_Header_Unit, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, RGB( 192, 192, 192 ) );

	// Because the 'CD_Circuit_Pos' column is frozen, left border of the 'CD_Circuit_Name' column is never shown.
	SetBorderRange( CD_Circuit_Pos, RD_Header_ColName, CD_Circuit_Pos, RD_Header_Unit, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, RGB( 192, 192, 192 ) );

	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateMainExpandCollapseButton( CD_Circuit_Pos, RD_Header_ButDlg );
	}
}

void CSheetHMCalc::_InitCircuitInjSheet( CDS_HydroMod *pHM )
{
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == m_mapSSheetColumnWidth.size() || 0 == m_mapSSheetColumnWidth.count( SheetDescription::SCircuitInj ) )
	{
		_ResetColumnWidth( SheetDescription::SCircuitInj );
	}

	_ClearAllCombos( SheetDescription::SCircuitInj );

	SetMaxRows( RD_Header_FirstCirc - 1 );
	SetMaxCols( CD_CircuitInjSecSide_Pointer );

	// All cells are static by default and filled with _T("").
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	SetFreeze( CD_CircuitInjSecSide_Pos, RD_Header_FirstCirc - 1 );

	// Verify the project is frozen.
	bool bFreezed = false;

	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL != pPrjParam )
	{
		bFreezed = pPrjParam->IsFreezed();
	}

	// Increase ROW height.
	double dRowHeight = 12.75;// Reference
	SetRowHeight( RD_Header_GroupName, dRowHeight * 1.5 );
	SetRowHeight( RD_Header_ButDlg, dRowHeight * 1.2 );
	SetRowHeight( RD_Header_ColName, dRowHeight * 2 );
	SetRowHeight( RD_Header_Unit, dRowHeight * 1.2 );
	SetRowHeight( RD_Header_Picture, dRowHeight );

	// Set columns size.

	// Unit.
	if( false == m_bPrinting )
	{
		SetColWidthInPixels( CD_CircuitInjSecSide_Sub, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Sub] );
	}
	else
	{
		ShowCol( CD_CircuitInjSecSide_Sub, FALSE );
	}

	SetColWidthInPixels( CD_CircuitInjSecSide_Pos, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Pos] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Name, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Name] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Desc, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Desc] );
	SetColWidthInPixels( CD_CircuitInjSecSide_DescSep, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_DescSep] );

	// First group.
	// HYS-1202: Add UnitDesc, Power, DT, Qref, DpRef, Kv and Cv columns
	SetColWidthInPixels( CD_CircuitInjSecSide_UnitDesc, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_UnitDesc] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Q, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Q] );
	SetColWidthInPixels( CD_CircuitInjSecSide_P, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_P] );
	SetColWidthInPixels( CD_CircuitInjSecSide_DT, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_DT] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Dp, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Dp] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Qref, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Qref] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Dpref, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Dpref] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Kv, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Kv] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Cv, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Cv] );
	SetColWidthInPixels( CD_CircuitInjSecSide_Sep, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Sep] );

	// Pump.
	SetColWidthInPixels( CD_CircuitInjSecSide_PumpH, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PumpH] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PumpHMin, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PumpHMin] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PumpHAvailable, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PumpHAvailable] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PumpSep, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PumpSep] );

	// Temperature.
	SetColWidthInPixels( CD_CircuitInjSecSide_TempTsp, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_TempTsp] );
	SetColWidthInPixels( CD_CircuitInjSecSide_TempTss, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_TempTss] );
	SetColWidthInPixels( CD_CircuitInjSecSide_TempTrs, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_TempTrs] );
	SetColWidthInPixels( CD_CircuitInjSecSide_TempSep, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_TempSep] );

	// CV.
	SetColWidthInPixels( CD_CircuitInjSecSide_CVName, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVName] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVDesc, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVDesc] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVKvsMax, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVKvsMax] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVKvs, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVKvs] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVSet, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVSet] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVDp, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVDp] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVDpMin, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVDpMin] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVAuth, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVAuth] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVDesignAuth, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVDesignAuth] );
	SetColWidthInPixels( CD_CircuitInjSecSide_CVSep, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_CVSep] );

	// Actuators
	SetColWidthInPixels( CD_CircuitInjSecSide_ActuatorName, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_ActuatorName] );
	SetColWidthInPixels( CD_CircuitInjSecSide_ActuatorPowerSupply, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_ActuatorPowerSupply] );
	SetColWidthInPixels( CD_CircuitInjSecSide_ActuatorInputSignal, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_ActuatorInputSignal] );
	SetColWidthInPixels( CD_CircuitInjSecSide_ActuatorOutputSignal, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_ActuatorOutputSignal] );
	SetColWidthInPixels( CD_CircuitInjSecSide_ActuatorRelayType, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_ActuatorRelayType] );
	SetColWidthInPixels( CD_CircuitInjSecSide_ActuatorFailSafe, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_ActuatorFailSafe] );
	SetColWidthInPixels( CD_CircuitInjSecSide_ActuatorDRP, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_ActuatorDRP] );
	SetColWidthInPixels( CD_CircuitInjSecSide_ActuatorSep, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_ActuatorSep] );

	// BV.
	SetColWidthInPixels( CD_CircuitInjSecSide_BvName, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_BvName] );
	SetColWidthInPixels( CD_CircuitInjSecSide_BvSet, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_BvSet] );
	SetColWidthInPixels( CD_CircuitInjSecSide_BvDp, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_BvDp] );
	SetColWidthInPixels( CD_CircuitInjSecSide_BvDpSig, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_BvDpSig] );
	SetColWidthInPixels( CD_CircuitInjSecSide_BvDpMin, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_BvDpMin] );
	SetColWidthInPixels( CD_CircuitInjSecSide_BvSep, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_BvSep] );

	// Pipe.
	SetColWidthInPixels( CD_CircuitInjSecSide_PipeSerie, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PipeSerie] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PipeSize, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PipeSize] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PipeL, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PipeL] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PipeDp, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PipeDp] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PipeLinDp, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PipeLinDp] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PipeV, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PipeV] );
	SetColWidthInPixels( CD_CircuitInjSecSide_PipeSep, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_PipeSep] );

	// Accessories.
	SetColWidthInPixels( CD_CircuitInjSecSide_AccDesc, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_AccDesc] );
	SetColWidthInPixels( CD_CircuitInjSecSide_AccData, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_AccData] );
	SetColWidthInPixels( CD_CircuitInjSecSide_AccDp, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_AccDp] );

	SetColWidthInPixels( CD_CircuitInjSecSide_Pointer, m_mapSSheetColumnWidth[SheetDescription::SCircuitInj][CD_CircuitInjSecSide_Pointer] );

	// All cells are not selectable.
	SetCellProperty( CD_CircuitInjSecSide_Sub, RD_Header_GroupName, CD_CircuitInjSecSide_Pointer, RD_Header_FirstCirc - 1,
			CSSheet::_SSCellProperty::CellNoSelection, true );

	// Flag unhidden columns.
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_DescSep, CD_CircuitInjSecSide_DescSep, true );
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_Sep, CD_CircuitInjSecSide_Sep, true );
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_PumpSep, CD_CircuitInjSecSide_PumpSep, true );
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_TempSep, CD_CircuitInjSecSide_TempSep, true );
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_CVSep, CD_CircuitInjSecSide_CVSep, true );
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_ActuatorSep, CD_CircuitInjSecSide_ActuatorSep, true );
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_BvSep, CD_CircuitInjSecSide_BvSep, true );
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_PipeSep, CD_CircuitInjSecSide_PipeSep, true );

	// Used to determine background color for group name.
	// 0 for default TitleGroup default color, 1 for _TAH_TITLE_GROUP2 color.
	bool bColorGroup = false;
	bool bCV = false;			// Display or not group CV.
	bool bActuator = false;		// Display or not group Actuator.
	bool bCVPreset = false;		// Presetting is displayed only for presetable TA valve.
	bool bCVKv = false;			// Kvs and Kvmax are displayed only for not presetable valve.
	bool bCVTA = false;			// Hidde column Name if there are no TA Valve.
	bool bCVNOTA = false;
	bool bAuthority = false;
	bool bBVSec = false;			// Display or not group BV.
	bool bHidePipeGroup = true;		// Hide Pipe group when all circuit are without terminal unit (only module).
	bool bPipeSerie = true;			// Same series for all circuit by default.
	bool bSub = false;
	m_pPipeCircTab = NULL;
	
	// HYS-1202: Add UnitDesc, Power, DT, Qref, DpRef, Kv and Cv columns
	std::map<int, bool> mapTUQVariables;
	std::map<int, bool> mapTUDpVariables;
	// Allow to add "editable" picture
	bool bUnitQ = false;
	bool bUnitDT = false;
	bool bUnitDp = false;

	bool bAtLeastOneInjectionAutoAdaptive = false;
	bool bAtLeastOneInjetionOther = false;

	if( NULL != pHM )
	{
		// Scan parent module and children.
		IDPTR IDPtr = pHM->GetIDPtr();
		bool bFirst = true;

		while( _T('\0') != *IDPtr.ID )
		{
			CDS_HydroMod *pCHM = (CDS_HydroMod *)IDPtr.MP;

			if ( NULL == pCHM )
			{
				continue;
			}
			
			// Save QType and DpType to know what are the columns to show.
			if( 0 == mapTUQVariables.count( pCHM->GetpTermUnit()->GetQType() ) )
			{
				mapTUQVariables[(int)pCHM->GetpTermUnit()->GetQType()] = true;
			}

			if( 0 == mapTUDpVariables.count( pCHM->GetpTermUnit()->GetDpType() ) )
			{
				mapTUDpVariables[(int)pCHM->GetpTermUnit()->GetDpType()] = true;
			}

			// HYS-1202: if secondary side exists the Dp and Flow columns are not editable
			if( NULL != pCHM->GetpSchcat() && true == pCHM->GetpSchcat()->IsSecondarySideExist() )
			{
				if( false == pCHM->GetpTermUnit()->IsVirtual()
					&& CDS_HydroMod::eDpType::Dp == pCHM->GetpTermUnit()->GetDpType() )
				{
					bUnitDp = true;
				}

				if( false == pCHM->GetpTermUnit()->IsVirtual()
					&& CTermUnit::_QType::Q == pCHM->GetpTermUnit()->GetQType() )
				{
					bUnitQ = true;
					// HYS-1882
					if( true == pCHM->IsDTFieldDisplayedInFlowMode() )
					{
						bUnitDT = true;
					}
				}
			}

			if( true == pCHM->IsBvSecExist( true ) )
			{
				bBVSec = true;
			}

			if( true == pCHM->IsCvExist( true ) && eb3False == pCHM->GetpCV()->IsCVLocInPrimary() )
			{
				bCV = true;

				if( true == pCHM->GetpCV()->IsTaCV() )
				{
					bCVTA = true;
				}

				if( false == pCHM->GetpCV()->IsTaCV() )
				{
					bCVNOTA = true;
				}

				// Not presettable valve, show Kvs and Kvmax.
				if( false == pCHM->GetpCV()->IsPresettable() && _T('\0') != *( pCHM->GetpCV()->GetCvIDPtr().ID ) )
				{
					bCVKv = true;
				}

				// Presetting is displayed for presettable valve.
				if( true == pCHM->GetpCV()->IsPresettable() && _T('\0') != *( pCHM->GetpCV()->GetCvIDPtr().ID ) )
				{
					bCVPreset = true ;
				}

				if( false == pCHM->GetpCV()->IsOn_Off() && eb3False == pCHM->GetpCV()->IsPICV() )
				{
					bAuthority = true;
				}

				// Check if actuator exist.
				// Remark: Either user has selected an actuator, and in this case we can show the group, or no actuator selected but
				//         the control valve has actuator group. In that case, user must be able to choose from a combo.
				if( pCHM->GetpCV()->GetActrIDPtr().MP != NULL 
						|| ( NULL != pCHM->GetpCV()->GetpCV() && NULL != pCHM->GetpCV()->GetpCV()->GetActuatorGroupIDPtr().MP ) )
				{
					bActuator = true;
				}
			}

			// Test pipe serie only for children; modules doesn't have circuit pipe.
			if( true == bPipeSerie && pHM != pCHM )
			{
				CPipes *pPipe = pCHM->GetpCircuitSecondaryPipe();

				if( NULL != pPipe )
				{
					if( NULL == m_pPipeCircTab )
					{
						m_pPipeCircTab = pPipe->GetPipeSeries();
					}
					else if( NULL != pPipe->GetPipeSeries() && m_pPipeCircTab != pPipe->GetPipeSeries() )
					{
						m_pPipeCircTab = NULL;
						bPipeSerie = false;
					}
				}
			}

			// Hide Pipe Group when we have only module.
			if( false == pCHM->IsaModule() )
			{
				bHidePipeGroup = false;
			}

			if( true == bFirst )
			{
				IDPtr = pHM->GetFirst();
				bFirst = false;
			}
			else
			{
				IDPtr = pHM->GetNext();
			}

			if( true == pCHM->IsaModule() && pCHM != pHM )
			{
				bSub = true;
			}

			if( NULL != pCHM->GetpSch() && true == pCHM->IsInjectionCircuit() )
			{
				if( eDpStab::DpStabOnBVBypass == pCHM->GetpSch()->GetDpStab() )
				{
					// HYS-1930: For auto-adaptive with decoupling bypass, we show also the "Havailable" column in the "Pump" group.
					bAtLeastOneInjectionAutoAdaptive = true;
				}
				else
				{
					bAtLeastOneInjetionOther = true;
				}
			}
		};
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Circuit' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	AddCellSpanW( CD_CircuitInjSecSide_Name, RD_Header_GroupName, CD_CircuitInjSecSide_Desc - CD_CircuitInjSecSide_Name + 1, 2 );
	_SetStaticText( CD_CircuitInjSecSide_Name, RD_Header_GroupName, IDS_HMCALC_CIRCUIT );
	bColorGroup ^= 1;

	// Name.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	_SetStaticText( CD_CircuitInjSecSide_Name, RD_Header_ColName, IDS_HMCALC_NAME );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_Name, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Description.
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	_SetStaticText( CD_CircuitInjSecSide_Desc, RD_Header_ColName, IDS_HMCALC_DESC );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_Desc, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Border.
	SetCellBorder( CD_CircuitInjSecSide_Name, RD_Header_Picture - 1, CD_CircuitInjSecSide_Desc, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	SetCellBorder( CD_CircuitInjSecSide_Name, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_Desc, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Secondary side.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main Title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	if( true == bColorGroup )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	}

	bColorGroup ^= 1;
	// HYS-1202: Add UnitDesc, Power, DT, Qref, DpRef, Kv and Cv columns
	// HYS-1276
	AddCellSpanW( CD_CircuitInjSecSide_UnitDesc, RD_Header_GroupName, CD_CircuitInjSecSide_Sep - CD_CircuitInjSecSide_UnitDesc, 2 );
	_SetStaticText( CD_CircuitInjSecSide_UnitDesc, RD_Header_GroupName, IDS_HMCALC_UNITSECSIDE );

	// Description.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	_SetStaticText( CD_CircuitInjSecSide_UnitDesc, RD_Header_ColName, IDS_HMCALC_DESC );

	if( false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_UnitDesc, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Q.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	_SetStaticText( CD_CircuitInjSecSide_Q, RD_Header_ColName, IDS_HMCALC_Q );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_CircuitInjSecSide_Q, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( true == bUnitQ && false == bFreezed && false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_Q, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	if( mapTUQVariables.count( CTermUnit::_QType::PdT ) > 0 )
	{
		// Power.
		_SetStaticText( CD_CircuitInjSecSide_P, RD_Header_ColName, IDS_HMCALC_P );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_P, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_P, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_CircuitInjSecSide_P, FALSE );
	}

	// HYS-1882: Show DT.
	if( mapTUQVariables.count( CTermUnit::_QType::PdT ) > 0 || true == bUnitDT )
	{
		// DT.
		_SetStaticText( CD_CircuitInjSecSide_DT, RD_Header_ColName, IDS_HMCALC_DT );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_DT, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_DT, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_CircuitInjSecSide_DT, FALSE );
	}

	// Dp.
	_SetStaticText( CD_CircuitInjSecSide_Dp, RD_Header_ColName, IDS_HMCALC_DP );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_CircuitInjSecSide_Dp, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( true == bUnitDp && false == bFreezed && false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_Dp, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	if( mapTUDpVariables.count( CDS_HydroMod::eDpType::QDpref ) > 0 )
	{
		// QRef.
		_SetStaticText( CD_CircuitInjSecSide_Qref, RD_Header_ColName, IDS_HMCALC_QREF );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_Qref, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_Qref, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// DpRef.
		_SetStaticText( CD_CircuitInjSecSide_Dpref, RD_Header_ColName, IDS_HMCALC_DPREF );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_Dpref, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_Dpref, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_CircuitInjSecSide_Qref, FALSE );
		ShowCol( CD_CircuitInjSecSide_Dpref, FALSE );
	}

	// Kv.
	if( mapTUDpVariables.count( CDS_HydroMod::eDpType::Kv ) > 0 )
	{
		_SetStaticText( CD_CircuitInjSecSide_Kv, RD_Header_ColName, IDS_KV );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_Kv, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_CircuitInjSecSide_Kv, FALSE );
	}

	// Cv.
	if( mapTUDpVariables.count( CDS_HydroMod::eDpType::Cv ) > 0 )
	{
		_SetStaticText( CD_CircuitInjSecSide_Cv, RD_Header_ColName, IDS_CV );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_Cv, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}
	else
	{
		ShowCol( CD_CircuitInjSecSide_Cv, FALSE );
	}

	// Border.
	SetCellBorder( CD_CircuitInjSecSide_UnitDesc, RD_Header_Picture - 1, CD_CircuitInjSecSide_Sep - 1, RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	SetCellBorder( CD_CircuitInjSecSide_UnitDesc, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_Sep - 1, RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	if( mapTUQVariables.size() > 0 || mapTUDpVariables.size() > 0 )
	{
		if( false == m_bPrinting && false == m_bExporting )
		{
			// Collapse button.
			CreateExpandCollapseColumnButton( CD_CircuitInjSecSide_Sep, RD_Header_ButDlg, CD_CircuitInjSecSide_P, CD_CircuitInjSecSide_Cv, GetMaxRows() );
			// Exclude col Dp from collapse function.
			SetFlagCanBeCollapsed( CD_CircuitInjSecSide_Dp, CD_CircuitInjSecSide_Dp, false );
			ChangeExpandCollapseColumnButtonState( CD_CircuitInjSecSide_Sep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pump.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	if( true == bColorGroup )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	}

	bColorGroup ^= 1;
	AddCellSpanW( CD_CircuitInjSecSide_PumpH, RD_Header_GroupName, CD_CircuitInjSecSide_PumpSep - CD_CircuitInjSecSide_PumpH, 2 );
	_SetStaticText( CD_CircuitInjSecSide_PumpH, RD_Header_GroupName, IDS_HMCALC_PUMP );

	// Pump head.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	_SetStaticText( CD_CircuitInjSecSide_PumpH, RD_Header_ColName, IDS_HMCALC_PUMPH );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_CircuitInjSecSide_PumpH, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_PumpH, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	// Hmin.
	_SetStaticText( CD_CircuitInjSecSide_PumpHMin, RD_Header_ColName, IDS_HMCALC_HMIN );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_CircuitInjSecSide_PumpHMin, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Havailable.
	if( true == bAtLeastOneInjectionAutoAdaptive )
	{
		_SetStaticText( CD_CircuitInjSecSide_PumpHAvailable, RD_Header_ColName, IDS_HMCALC_SECPUMPHAVAILABLE );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_PumpHAvailable, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	}
	else
	{
		ShowCol( CD_CircuitInjSecSide_PumpHAvailable, FALSE );
	}

	// Border.
	SetCellBorder( CD_CircuitInjSecSide_PumpH, RD_Header_Picture - 1, CD_CircuitInjSecSide_PumpSep - 1, RD_Header_Picture - 1, true,
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	SetCellBorder( CD_CircuitInjSecSide_PumpH, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_PumpSep - 1, RD_Header_FirstCirc - 1, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Temperature.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_TempTsp, CD_CircuitInjSecSide_TempTsp, true );

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	if( true == bColorGroup )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	}

	bColorGroup ^= 1;
	AddCellSpanW( CD_CircuitInjSecSide_TempTsp, RD_Header_GroupName, CD_CircuitInjSecSide_TempSep - CD_CircuitInjSecSide_TempTsp, 2 );
	_SetStaticText( CD_CircuitInjSecSide_TempTsp, RD_Header_GroupName, IDS_HMCALC_TEMP );

	// Ts.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	_SetStaticText( CD_CircuitInjSecSide_TempTss, RD_Header_ColName, IDS_HMCALC_TS );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_CircuitInjSecSide_TempTss, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
	{
		if( true == bAtLeastOneInjetionOther )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_TempTss, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}
	}

	// Tr.
	_SetStaticText( CD_CircuitInjSecSide_TempTrs, RD_Header_ColName, IDS_HMCALC_TR );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_CircuitInjSecSide_TempTrs, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	// Tp.
	_SetStaticText( CD_CircuitInjSecSide_TempTsp, RD_Header_ColName, IDS_HMCALC_TP );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_CircuitInjSecSide_TempTsp, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Border.
	SetCellBorder( CD_CircuitInjSecSide_TempTsp, RD_Header_Picture - 1, CD_CircuitInjSecSide_TempSep - 1, RD_Header_Picture - 1, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	SetCellBorder( CD_CircuitInjSecSide_TempTsp, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_TempSep - 1, RD_Header_FirstCirc - 1, true,
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Control Valve.
	if( true == bCV )
	{
		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_CircuitInjSecSide_CVName, RD_Header_GroupName, CD_CircuitInjSecSide_CVSep - CD_CircuitInjSecSide_CVName, 2 );
		_SetStaticText( CD_CircuitInjSecSide_CVName, RD_Header_GroupName, IDS_HMCALC_CV );

		// Name.
		if( true == bCVTA )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			_SetStaticText( CD_CircuitInjSecSide_CVName, RD_Header_ColName, IDS_HMCALC_NAME );

			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_COMBO, CD_CircuitInjSecSide_CVName, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}
		else
		{
			ShowCol( CD_CircuitInjSecSide_CVName, FALSE );
		}

		// Description.
		if( true == bCVNOTA )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			_SetStaticText( CD_CircuitInjSecSide_CVDesc, RD_Header_ColName, IDS_HMCALC_DESC );

			if( false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_CVDesc, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}
		else
		{
			ShowCol( CD_CircuitInjSecSide_CVDesc, FALSE );
		}

		// Kvsmax.
		if( true == bCVKv )
		{
			CString str;

			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_KVSMAX );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_CVMAX );
			}

			_SetStaticText( CD_CircuitInjSecSide_CVKvsMax, RD_Header_ColName, str );
		}
		else
		{
			ShowCol( CD_CircuitInjSecSide_CVKvsMax, FALSE );
		}

		// Kvs.
		if( true == bCVKv )
		{
			CString str;

			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_KVS );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_CV );
			}

			_SetStaticText( CD_CircuitInjSecSide_CVKvs, RD_Header_ColName, str );

			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_COMBOEDIT, CD_CircuitInjSecSide_CVKvs, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}
		else
		{
			ShowCol( CD_CircuitInjSecSide_CVKvs, FALSE );
		}

		// Setting.
		if( true == bCVPreset )
		{
			_SetStaticText( CD_CircuitInjSecSide_CVSet, RD_Header_ColName, IDS_HMCALC_SETTING );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			_SetStaticText( CD_CircuitInjSecSide_CVSet, RD_Header_Unit, IDS_TURNS );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		}
		else
		{
			ShowCol( CD_CircuitInjSecSide_CVSet, FALSE );
		}

		// Dp.
		_SetStaticText( CD_CircuitInjSecSide_CVDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_CVDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Dp Min.
		_SetStaticText( CD_CircuitInjSecSide_CVDpMin, RD_Header_ColName, IDS_HMCALC_DPMIN );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_CVDpMin, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Authority.
		if( true == bAuthority )
		{
			_SetStaticText( CD_CircuitInjSecSide_CVAuth, RD_Header_ColName, IDS_HMCALC_MINAUTHOR );
		}
		else
		{
			ShowCol( CD_CircuitInjSecSide_CVAuth, FALSE );
		}

		// Design Authority.
		if( true == bAuthority )
		{
			_SetStaticText( CD_CircuitInjSecSide_CVDesignAuth, RD_Header_ColName, IDS_HMCALC_DESAUTHOR );
		}
		else
		{
			ShowCol( CD_CircuitInjSecSide_CVDesignAuth, FALSE );
		}

		// Border.
		SetCellBorder( CD_CircuitInjSecSide_CVName, RD_Header_Picture - 1, CD_CircuitInjSecSide_CVSep - 1, RD_Header_Picture - 1, 
				true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_CircuitInjSecSide_CVName, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_CVSep - 1, RD_Header_FirstCirc - 1, 
				true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( true == bCVPreset )
		{
			if( false == m_bPrinting && false == m_bExporting )
			{
				CreateExpandCollapseColumnButton( CD_CircuitInjSecSide_CVSep, RD_Header_ButDlg, CD_CircuitInjSecSide_CVSet, CD_CircuitInjSecSide_CVSet, GetMaxRows() );
				ChangeExpandCollapseColumnButtonState( CD_CircuitInjSecSide_CVSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
			}
		}
	}
	else
	{
		ShowColRange( CD_CircuitInjSecSide_CVName, CD_CircuitInjSecSide_CVSep, FALSE );
	}

	// Actuator.
	if( true == bActuator )
	{
		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_CircuitInjSecSide_ActuatorName, RD_Header_GroupName, CD_CircuitInjSecSide_ActuatorSep - CD_CircuitInjSecSide_ActuatorName, 2 );
		_SetStaticText( CD_CircuitInjSecSide_ActuatorName, RD_Header_GroupName, IDS_HMCALC_ACTUATOR );

		// Name.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		_SetStaticText( CD_CircuitInjSecSide_ActuatorName, RD_Header_ColName, IDS_HMCALC_NAME );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_CircuitInjSecSide_ActuatorName, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Power supply.
		_SetStaticText( CD_CircuitInjSecSide_ActuatorPowerSupply, RD_Header_ColName, IDS_HMCALC_POWERSUPPLY );

		// Input signal.
		_SetStaticText( CD_CircuitInjSecSide_ActuatorInputSignal, RD_Header_ColName, IDS_HMCALC_INPUTSIGNAL );

		// Output signal.
		_SetStaticText( CD_CircuitInjSecSide_ActuatorOutputSignal, RD_Header_ColName, IDS_HMCALC_OUTPUTSIGNAL );

		// Relay type.
		_SetStaticText( CD_CircuitInjSecSide_ActuatorRelayType, RD_Header_ColName, IDS_HMCALC_RELAYTYPE );

		// Fail safe.
		_SetStaticText( CD_CircuitInjSecSide_ActuatorFailSafe, RD_Header_ColName, IDS_HMCALC_FAILSAFE );

		// Default return position.
		_SetStaticText( CD_CircuitInjSecSide_ActuatorDRP, RD_Header_ColName, IDS_SSHEETSSEL_ACTRETURNPOSITION );

		// Border.
		SetCellBorder( CD_CircuitInjSecSide_ActuatorName, RD_Header_Picture - 1, CD_CircuitInjSecSide_ActuatorSep - 1, 
				RD_Header_Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_CircuitInjSecSide_ActuatorName, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_ActuatorSep - 1, 
				RD_Header_FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_CircuitInjSecSide_ActuatorSep, RD_Header_ButDlg, CD_CircuitInjSecSide_ActuatorPowerSupply,
				CD_CircuitInjSecSide_ActuatorDRP, GetMaxRows() );

			ChangeExpandCollapseColumnButtonState( CD_CircuitInjSecSide_ActuatorSep, RD_Header_ButDlg, -1, -1, 
					CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}
	else
	{
		ShowColRange( CD_CircuitInjSecSide_ActuatorName, CD_CircuitInjSecSide_ActuatorSep, FALSE );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Regulating Valve.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	if( true == bBVSec )
	{
		SetFlagShowEvenEmpty( CD_CircuitInjSecSide_BvName, CD_CircuitInjSecSide_BvName, true );
		SetFlagShowEvenEmpty( CD_CircuitInjSecSide_BvDp, CD_CircuitInjSecSide_BvDp, true );

		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( CD_CircuitInjSecSide_BvName, RD_Header_GroupName, CD_CircuitInjSecSide_BvSep - CD_CircuitInjSecSide_BvName, 2 );
		_SetStaticText( CD_CircuitInjSecSide_BvName, RD_Header_GroupName, IDS_HMCALC_BV );

		// Name.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		_SetStaticText( CD_CircuitInjSecSide_BvName, RD_Header_ColName, IDS_HMCALC_BVNAME );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_COMBO, CD_CircuitInjSecSide_BvName, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Setting.
		_SetStaticText( CD_CircuitInjSecSide_BvSet, RD_Header_ColName, IDS_HMCALC_SETTING );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_BvSet, RD_Header_Unit, IDS_TURNS );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Dp.
		_SetStaticText( CD_CircuitInjSecSide_BvDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_BvDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// DpSig.
		_SetStaticText( CD_CircuitInjSecSide_BvDpSig, RD_Header_ColName, IDS_HMCALC_DPSIG );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_BvDpSig, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// DpMin.
		_SetStaticText( CD_CircuitInjSecSide_BvDpMin, RD_Header_ColName, IDS_HMCALC_DPFULLOPEN );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_BvDpMin, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( CD_CircuitInjSecSide_BvName, RD_Header_Picture - 1, CD_CircuitInjSecSide_BvSep - 1, RD_Header_Picture - 1, 
				true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( CD_CircuitInjSecSide_BvName, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_BvSep - 1, RD_Header_FirstCirc - 1, 
				true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_CircuitInjSecSide_BvSep, RD_Header_ButDlg, CD_CircuitInjSecSide_BvName, CD_CircuitInjSecSide_BvDpMin, GetMaxRows() );
			SetFlagCanBeCollapsed( CD_CircuitInjSecSide_BvName, CD_CircuitInjSecSide_BvName, false );
			SetFlagCanBeCollapsed( CD_CircuitInjSecSide_BvDp, CD_CircuitInjSecSide_BvDpSig, false );
			ChangeExpandCollapseColumnButtonState( CD_CircuitInjSecSide_BvSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}
	else
	{
		ShowColRange( CD_CircuitInjSecSide_BvName, CD_CircuitInjSecSide_BvSep, FALSE );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Circuit Pipe' Group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if( false == bHidePipeGroup )
	{
		SetFlagShowEvenEmpty( CD_CircuitInjSecSide_PipeSize, CD_CircuitInjSecSide_PipeSize, true );
		SetFlagShowEvenEmpty( CD_CircuitInjSecSide_PipeL, CD_CircuitInjSecSide_PipeL, true );
		SetFlagShowEvenEmpty( CD_CircuitInjSecSide_PipeDp, CD_CircuitInjSecSide_PipeDp, true );

		CString str;
		str = TASApp.LoadLocalizedString( IDS_HMCALC_SECONDARYPIPES );
		long lFirstCol = CD_CircuitInjSecSide_PipeSerie;
		bool bShowPipeSeries = true;

		if( NULL != m_pPipeCircTab )
		{
			lFirstCol = CD_CircuitInjSecSide_PipeSize;
			str += CString( _T("\r\n") ) + m_pPipeCircTab->GetName();
			ShowCol( CD_CircuitInjSecSide_PipeSerie, FALSE );
			bShowPipeSeries = false;
		}
		else
		{
			ShowCol( CD_CircuitInjSecSide_PipeSerie, TRUE );
		}

		// Main title.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

		if( true == bColorGroup )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
		}

		bColorGroup ^= 1;
		AddCellSpanW( lFirstCol, RD_Header_GroupName, CD_CircuitInjSecSide_PipeSep - lFirstCol, 2 );
		_SetStaticText( lFirstCol, RD_Header_GroupName, str );

		// Pipe series.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

		if( true == bShowPipeSeries )
		{
			_SetStaticText( CD_CircuitInjSecSide_PipeSerie, RD_Header_ColName, IDS_HMCALC_PIPESERIE );

			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_COMBO, CD_CircuitInjSecSide_PipeSerie, RD_Header_Picture, CSSheet::PictureCellType::Icon );
			}
		}

		// Pipe size.
		_SetStaticText( CD_CircuitInjSecSide_PipeSize, RD_Header_ColName, IDS_HMCALC_PIPESIZE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting ) 
		{
			SetPictureCellWithID( IDI_COMBO, CD_CircuitInjSecSide_PipeSize, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Length.
		_SetStaticText( CD_CircuitInjSecSide_PipeL, RD_Header_ColName, IDS_HMCALC_PIPEL );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ), tcName );
		_SetStaticText( CD_CircuitInjSecSide_PipeL, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_CircuitInjSecSide_PipeL, RD_Header_Picture, CSSheet::PictureCellType::Icon );
		}

		// Dp.
		_SetStaticText( CD_CircuitInjSecSide_PipeDp, RD_Header_ColName, IDS_HMCALC_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_PipeDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// linear Dp.
		_SetStaticText( CD_CircuitInjSecSide_PipeLinDp, RD_Header_ColName, IDS_HMCALC_LINDP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_PipeLinDp, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Velocity.
		_SetStaticText( CD_CircuitInjSecSide_PipeV, RD_Header_ColName, IDS_HMCALC_PIPEVA );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ), tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		_SetStaticText( CD_CircuitInjSecSide_PipeV, RD_Header_Unit, tcName );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border.
		SetCellBorder( lFirstCol, RD_Header_Picture - 1, CD_CircuitInjSecSide_PipeV, RD_Header_Picture - 1, true, 
				SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		SetCellBorder( lFirstCol, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_PipeV, RD_Header_FirstCirc - 1, true, 
				SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button.
		if( false == m_bPrinting && false == m_bExporting )
		{
			CreateExpandCollapseColumnButton( CD_CircuitInjSecSide_PipeSep, RD_Header_ButDlg, CD_CircuitInjSecSide_PipeLinDp, CD_CircuitInjSecSide_PipeV, GetMaxRows() );
			ChangeExpandCollapseColumnButtonState( CD_CircuitInjSecSide_PipeSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
		}
	}
	else
	{
		ShowColRange( CD_CircuitInjSecSide_PipeSerie, CD_CircuitInjSecSide_PipeSep, FALSE );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Accessories' group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_AccDesc, CD_CircuitInjSecSide_AccDesc, true );
	SetFlagShowEvenEmpty( CD_CircuitInjSecSide_AccDp, CD_CircuitInjSecSide_AccDp, true );

	// Main title.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	if( true == bColorGroup )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	}

	bColorGroup ^= 1;
	AddCellSpanW( CD_CircuitInjSecSide_AccDesc, RD_Header_GroupName, CD_CircuitInjSecSide_AccDp - CD_CircuitInjSecSide_AccDesc + 1, 2 );
	_SetStaticText( CD_CircuitInjSecSide_AccDesc, RD_Header_GroupName, IDS_HMCALC_ACCESSORIES );

	// Description.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	_SetStaticText( CD_CircuitInjSecSide_AccDesc, RD_Header_ColName, IDS_HMCALC_ACCDESCRIPTION );

	// Data needed for printing.
	_SetStaticText( CD_CircuitInjSecSide_AccData, RD_Header_ColName, IDS_HMCALCINPUTDATA );

	// Dp.
	_SetStaticText( CD_CircuitInjSecSide_AccDp, RD_Header_ColName, IDS_HMCALC_DP );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_CircuitInjSecSide_AccDp, RD_Header_Unit, tcName );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Border.
	SetCellBorder( CD_CircuitInjSecSide_AccDesc, RD_Header_Picture - 1, CD_CircuitInjSecSide_AccDp, RD_Header_Picture - 1, 
			true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	SetCellBorder( CD_CircuitInjSecSide_AccDesc, RD_Header_FirstCirc - 1, CD_CircuitInjSecSide_AccDp, RD_Header_FirstCirc - 1, 
			true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Add separator in header for the resizing feature.
	SetBorderRange( CD_CircuitInjSecSide_Name, RD_Header_ColName, CD_CircuitInjSecSide_Pointer, RD_Header_Unit, SS_BORDERTYPE_LEFT, 
			SS_BORDERSTYLE_SOLID, RGB( 192, 192, 192 ) );

	// Because the 'CD_CircuitInjSecSide_Pos' column is frozen, left border of the 'CD_CircuitInjSecSide_Name' column is never shown.
	SetBorderRange( CD_CircuitInjSecSide_Pos, RD_Header_ColName, CD_CircuitInjSecSide_Pos, RD_Header_Unit, SS_BORDERTYPE_RIGHT, 
			SS_BORDERSTYLE_SOLID, RGB( 192, 192, 192 ) );

	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateMainExpandCollapseButton( CD_CircuitInjSecSide_Pos, RD_Header_ButDlg );
	}
}

void CSheetHMCalc::_InitMeasurementSheet( CDS_HydroMod *pHM )
{
	TCHAR unitname[_MAXCHARS];
	CString str;

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == m_mapSSheetColumnWidth.size() || 0 == m_mapSSheetColumnWidth.count( SheetDescription::SMeasurement ) )
	{
		_ResetColumnWidth( SheetDescription::SMeasurement );
	}

	_ClearAllCombos( SheetDescription::SMeasurement );

	SetMaxRows( RD_Header_FirstCirc - 1 );
	SetMaxCols( CD_Measurement_Pointer );

	// All cells are static by default and filled with _T("").
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Freeze (make non scrollable) line above first circuit and first column.
	SetFreeze( CD_Measurement_Pos, RD_Header_FirstCirc - 1 );

	// Increase rows height.
	double RowHeight = 12.75;// Reference.
	SetRowHeight( RD_Header_GroupName, RowHeight * 1.5 );
	SetRowHeight( RD_Header_ButDlg, RowHeight * 1.2 );
	SetRowHeight( RD_Header_ColName, RowHeight * 2 );
	SetRowHeight( RD_Header_Unit, RowHeight * 1.2 );
	SetRowHeight( RD_Header_Picture, RowHeight );

	// Set columns size.
	double dValueWidth = 8;
	double dTextWidth = 24;
	double dLightTextWidth = 12;

	if( false == m_bPrinting )
	{
		SetColWidthInPixels( CD_Measurement_Sub, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Sub] );
	}
	else
	{
		ShowCol( CD_Measurement_Sub, FALSE );
	}

	SetColWidthInPixels( CD_Measurement_Pos, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Pos] );

	// Information.
	SetColWidthInPixels( CD_Measurement_DateTime, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DateTime] );
	SetColWidthInPixels( CD_Measurement_Reference, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Reference] );
	SetColWidthInPixels( CD_Measurement_Description, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Description] );
	SetColWidthInPixels( CD_Measurement_DescriptionSep, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DescriptionSep] );

	// Circuit.
	SetColWidthInPixels( CD_Measurement_WaterChar, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_WaterChar] );
	SetColWidthInPixels( CD_Measurement_Valve, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Valve] );
	SetColWidthInPixels( CD_Measurement_Presetting, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Presetting] );
	SetColWidthInPixels( CD_Measurement_Setting, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Setting] );
	SetColWidthInPixels( CD_Measurement_Kv, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Kv] );
	SetColWidthInPixels( CD_Measurement_KvSignal, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_KvSignal] );
	SetColWidthInPixels( CD_Measurement_KvSignalSep, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_KvSignalSep] );

	// Dp.
	SetColWidthInPixels( CD_Measurement_Dp, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Dp] );
	SetColWidthInPixels( CD_Measurement_Dpl, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Dpl] );
	SetColWidthInPixels( CD_Measurement_DplSep, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DplSep] );

	// Flow.
	SetColWidthInPixels( CD_Measurement_MeasureDp, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_MeasureDp] );
	SetColWidthInPixels( CD_Measurement_DesignDp, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DesignDp] );
	SetColWidthInPixels( CD_Measurement_MeasureFlow, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_MeasureFlow] );
	SetColWidthInPixels( CD_Measurement_DesignFlow, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DesignFlow] );
	SetColWidthInPixels( CD_Measurement_FlowDeviation, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_FlowDeviation] );
	SetColWidthInPixels( CD_Measurement_FlowDeviationSep, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_FlowDeviationSep] );

	// Temperature.
	SetColWidthInPixels( CD_Measurement_TempHH, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_TempHH] );
	SetColWidthInPixels( CD_Measurement_Temp1DPS, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Temp1DPS] );
	SetColWidthInPixels( CD_Measurement_Temp2DPS, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Temp2DPS] );
	SetColWidthInPixels( CD_Measurement_TempRef, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_TempRef] );
	SetColWidthInPixels( CD_Measurement_DiffTemp, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DiffTemp] );
	SetColWidthInPixels( CD_Measurement_DesignDiffTemp, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DesignDiffTemp] );
	SetColWidthInPixels( CD_Measurement_DiffTempDeviation, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DiffTempDeviation] );
	SetColWidthInPixels( CD_Measurement_DiffTempDeviationSep, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DiffTempDeviationSep] );

	// Power.
	SetColWidthInPixels( CD_Measurement_Power, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Power] );
	SetColWidthInPixels( CD_Measurement_DesignPower, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_DesignPower] );
	SetColWidthInPixels( CD_Measurement_PowerDeviation, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_PowerDeviation] );
	SetColWidthInPixels( CD_Measurement_Pointer, m_mapSSheetColumnWidth[SheetDescription::SMeasurement][CD_Measurement_Pointer] );

	// All cells are not selectable (in header only).
	SetCellProperty( CD_Measurement_Pos, RD_Header_GroupName, CD_Measurement_Pointer, RD_Header_FirstCirc - 1, CSSheet::_SSCellProperty::CellNoSelection, true );

	///////////////////////////////////////////////////////////////////
	// Format the SubTitles.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	// Define variables for the background color.
	COLORREF BckColor = GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor );

	///////////////////////////////////////////////////////////////////
	// Determine the columns that must be shown.
	bool fShowDp = false;
	bool fShowTemp = false;
	bool fShowPower = false;

	for( IDPTR IDPtr = m_pHM->GetFirst(); *IDPtr.ID; IDPtr = m_pHM->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)IDPtr.MP;

		if( NULL == pHM )
		{
			continue;
		}

		// Do a loop on all CMeasData
		unsigned int uiNbrMeasData = pHM->GetMeasDataSize();

		for( unsigned int i = 0; i < uiNbrMeasData; i++ )
		{
			// Get the MeasData
			CDS_HydroMod::CMeasData *pMeasData = pHM->GetpMeasData( i );

			if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMPower ) )
			{
				fShowPower = true;
			}

			if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMPower ) ||
				true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMFlow ) ||
				true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMTemp ) )
			{
				fShowTemp = true;
			}

			if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMDp ) ||
				true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMDpl ) )
			{
				fShowDp = true;
			}
		}
	}

	// Parent module.
	pHM = (CDS_HydroMod *)m_pHM->GetIDPtr().MP;

	// Do a loop on all CMeasData.
	unsigned int uiNbrMeasData = pHM->GetMeasDataSize();

	for( unsigned int i = 0; i < uiNbrMeasData; i++ )
	{
		// Get the MeasData.
		CDS_HydroMod::CMeasData *pMeasData = pHM->GetpMeasData( i );

		if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMPower ) )
		{
			fShowPower = true;
		}

		if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMPower ) ||
			true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMFlow ) ||
			true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMTemp ) )
		{
			fShowTemp = true;
		}

		if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMDp )  ||
			true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMDpl ) )
		{
			fShowDp = true;
		}
	}

	//
	////////////////////////////////////////////////////////////////

	// By default some columns cannot be hide (Reference, Valve and Flow).
	SetFlagShowEvenEmpty( CD_Measurement_DesignFlow, CD_Measurement_DesignFlow, true );

	// Set the text in the SubTitle.
	AddCellSpanW( CD_Measurement_DateTime, RD_Header_GroupName, CD_Measurement_DescriptionSep - CD_Measurement_DateTime, 2 );
	_SetStaticText( CD_Measurement_DateTime, RD_Header_GroupName, IDS_HMCALC_INFO );

	// By default this column cannot be hide.
	SetFlagShowEvenEmpty( CD_Measurement_Reference, CD_Measurement_Reference, true );

	// Collapse button for information.
	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateExpandCollapseColumnButton( CD_Measurement_DescriptionSep, RD_Header_ButDlg, CD_Measurement_DateTime, CD_Measurement_Description, GetMaxRows() );
		SetFlagCanBeCollapsed( CD_Measurement_DateTime, CD_Measurement_Reference, false );
		ChangeExpandCollapseColumnButtonState( CD_Measurement_DescriptionSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse, false );
	}

	// Separator can't be hide (because this column is empty, 'HideEmptyColumns' will hide this column if this flag is not set).
	SetFlagShowEvenEmpty( CD_Measurement_DescriptionSep, CD_Measurement_DescriptionSep, true );

	// Set the color for the Circuit.
	_SetGroupColor( BckColor );
	AddCellSpanW( CD_Measurement_WaterChar, RD_Header_GroupName, CD_Measurement_KvSignalSep - CD_Measurement_WaterChar, 2 );
	_SetStaticText( CD_Measurement_WaterChar, RD_Header_GroupName, IDS_HMCALC_CIRCUIT );

	// By default this column cannot be hide.
	SetFlagShowEvenEmpty( CD_Measurement_Valve, CD_Measurement_Valve, true );

	// Collapse button for Circuit.
	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateExpandCollapseColumnButton( CD_Measurement_KvSignalSep, RD_Header_ButDlg, CD_Measurement_WaterChar, CD_Measurement_KvSignal, GetMaxRows() );
		// These columns can't be collapsed (WaterChar, Valve, Setting & KvSignal).
		SetFlagCanBeCollapsed( CD_Measurement_WaterChar, CD_Measurement_Valve, false );
		SetFlagCanBeCollapsed( CD_Measurement_Presetting, CD_Measurement_Setting, false );
		SetFlagCanBeCollapsed( CD_Measurement_Setting, CD_Measurement_Setting, false );
		SetFlagCanBeCollapsed( CD_Measurement_KvSignal, CD_Measurement_KvSignal, false );
		// Collapse Circuit columns.
		ChangeExpandCollapseColumnButtonState( CD_Measurement_KvSignalSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse, false );
	}

	// Separator can't be hide (because this column is empty, 'HideEmptyColumns' will hide this column if this flag is not set).
	SetFlagShowEvenEmpty( CD_Measurement_KvSignalSep, CD_Measurement_KvSignalSep, true );

	// Set the color for the Dp.
	if( true == fShowDp )
	{
		_SetGroupColor( BckColor );
		AddCellSpanW( CD_Measurement_Dp, RD_Header_GroupName, CD_Measurement_DplSep - CD_Measurement_Dp, 2 );
		_SetStaticText( CD_Measurement_Dp, RD_Header_GroupName, IDS_HMCALC_DIFFPRESS );

		// Flag unhidden columns.
		SetFlagShowEvenEmpty( CD_Measurement_DplSep, CD_Measurement_DplSep, true );
	}
	else
	{
		ShowColRange( CD_Measurement_Dp, CD_Measurement_DplSep, FALSE );
	}

	// Set the color for the group 'Flow'.
	_SetGroupColor( BckColor );
	AddCellSpanW( CD_Measurement_MeasureDp, RD_Header_GroupName, CD_Measurement_FlowDeviationSep - CD_Measurement_MeasureDp, 2 );
	_SetStaticText( CD_Measurement_MeasureDp, RD_Header_GroupName, IDS_HMCALC_FLOW );

	// Collapse button for group 'Flow'.
	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateExpandCollapseColumnButton( CD_Measurement_FlowDeviationSep, RD_Header_ButDlg, CD_Measurement_MeasureDp, CD_Measurement_FlowDeviation, GetMaxRows() );
		SetFlagCanBeCollapsed( CD_Measurement_MeasureFlow, CD_Measurement_DesignFlow, false );
		ChangeExpandCollapseColumnButtonState( CD_Measurement_FlowDeviationSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse, false );
	}

	// Flag unhidden columns.
	SetFlagShowEvenEmpty( CD_Measurement_FlowDeviationSep, CD_Measurement_FlowDeviationSep, true );

	// Set the color for the group 'Temperature'.
	if( true == fShowTemp )
	{
		_SetGroupColor( BckColor );
		AddCellSpanW( CD_Measurement_TempHH, RD_Header_GroupName, CD_Measurement_DiffTempDeviationSep - CD_Measurement_TempHH, 2 );
		_SetStaticText( CD_Measurement_TempHH, RD_Header_GroupName, IDS_HMCALC_TEMP );
	}
	else
	{
		ShowColRange( CD_Measurement_TempHH, CD_Measurement_DiffTempDeviationSep, FALSE );
	}

	// Set the color for the group 'Power'.
	if( true == fShowPower )
	{
		_SetGroupColor( BckColor );
		AddCellSpanW( CD_Measurement_Power, RD_Header_GroupName, CD_Measurement_Pointer - CD_Measurement_Power, 2 );
		_SetStaticText( CD_Measurement_Power, RD_Header_GroupName, IDS_HMCALC_POWER );
	}
	else
	{
		ShowColRange( CD_Measurement_Power, CD_Measurement_Pointer, FALSE );
	}

	///////////////////////////////////////////////////////////////////
	// Format the column headers.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Set the text of column headers.
	_SetStaticText( CD_Measurement_DateTime, RD_Header_ColName, IDS_HMCALC_DATETIME );

	_SetStaticText( CD_Measurement_Reference, RD_Header_ColName, IDS_HMCALC_REFERENCE );

	_SetStaticText( CD_Measurement_Description, RD_Header_ColName, IDS_HMCALC_DESC );

	_SetStaticText( CD_Measurement_WaterChar, RD_Header_ColName, IDS_HMCALC_WATERCHAR );

	_SetStaticText( CD_Measurement_Valve, RD_Header_ColName, IDS_HMCALC_VALVE );

	_SetStaticText( CD_Measurement_Presetting, RD_Header_ColName, IDS_HMCALC_PRESETTING );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Measurement_Presetting, RD_Header_Unit, IDS_SHEETHDR_TURNSPOS );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_Setting, RD_Header_ColName, IDS_HMCALC_MEAS_SETTING );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Measurement_Setting, RD_Header_Unit, IDS_SHEETHDR_TURNSPOS );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		_SetStaticText( CD_Measurement_Kv, RD_Header_ColName, IDS_KV );
	}
	else
	{
		_SetStaticText( CD_Measurement_Kv, RD_Header_ColName, IDS_CV );
	}

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		_SetStaticText( CD_Measurement_KvSignal, RD_Header_ColName, IDS_KVSIGNAL );
	}
	else
	{
		_SetStaticText( CD_Measurement_KvSignal, RD_Header_ColName, IDS_CVSIGNAL );
	}

	_SetStaticText( CD_Measurement_Dp, RD_Header_ColName, IDS_HMCALC_MEASDP );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	_SetStaticText( CD_Measurement_Dp, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_Dpl, RD_Header_ColName, IDS_HMCALC_MEASDPL );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	_SetStaticText( CD_Measurement_Dpl, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_MeasureDp, RD_Header_ColName, IDS_HMCALC_MEASDP );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	_SetStaticText( CD_Measurement_MeasureDp, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_DesignDp, RD_Header_ColName, IDS_HMCALC_DESIGNDP );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	_SetStaticText( CD_Measurement_DesignDp, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_MeasureFlow, RD_Header_ColName, IDS_HMCALC_MEASFLOW );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	_SetStaticText( CD_Measurement_MeasureFlow, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_DesignFlow, RD_Header_ColName, IDS_HMCALC_DESIGNFLOW );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	_SetStaticText( CD_Measurement_DesignFlow, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_FlowDeviation, RD_Header_ColName, IDS_HMCALC_FLOWDEV );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Measurement_FlowDeviation, RD_Header_Unit, _T("%") );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_TempHH, RD_Header_ColName, IDS_HMCALC_MEASTEMP );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	_SetStaticText( CD_Measurement_TempHH, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_Temp1DPS, RD_Header_ColName, IDS_HMCALC_T1FORDT );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	_SetStaticText( CD_Measurement_Temp1DPS, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_Temp2DPS, RD_Header_ColName, IDS_HMCALC_T2FORDT );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	_SetStaticText( CD_Measurement_Temp2DPS, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_TempRef, RD_Header_ColName, IDS_HMCALC_TREF );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	_SetStaticText( CD_Measurement_TempRef, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_DiffTemp, RD_Header_ColName, IDS_HMCALC_MEASDT );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), unitname );
	_SetStaticText( CD_Measurement_DiffTemp, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_DesignDiffTemp, RD_Header_ColName, IDS_HMCALC_DESIGNDT );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), unitname );
	_SetStaticText( CD_Measurement_DesignDiffTemp, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_DiffTempDeviation, RD_Header_ColName, IDS_HMCALC_DTDEV );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Measurement_DiffTempDeviation, RD_Header_Unit, _T("%") );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_Power, RD_Header_ColName, IDS_HMCALC_MEASPOWER );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), unitname );
	_SetStaticText( CD_Measurement_Power, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_DesignPower, RD_Header_ColName, IDS_HMCALC_DESIGNSPOWER );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), unitname );
	_SetStaticText( CD_Measurement_DesignPower, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_Measurement_PowerDeviation, RD_Header_ColName, IDS_HMCALC_POWERDEV );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_Measurement_PowerDeviation, RD_Header_Unit, _T("%") );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Draw border.
	SetCellBorder( CD_Measurement_DateTime, RD_Header_Unit, CD_Measurement_Description, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_WaterChar, RD_Header_Unit, CD_Measurement_KvSignal, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_Dp, RD_Header_Unit, CD_Measurement_Dpl, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_MeasureDp, RD_Header_Unit, CD_Measurement_FlowDeviation, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_TempHH, RD_Header_Unit, CD_Measurement_DiffTempDeviation, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_Power, RD_Header_Unit, CD_Measurement_PowerDeviation, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );

	// Freeze row header.
	SetFreeze( 0, RD_Header_Picture );

	// Add separator in header for the resizing feature.
	SetBorderRange( CD_Measurement_DateTime, RD_Header_ColName, CD_Measurement_Pointer, RD_Header_Unit, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, RGB( 192, 192,
					192 ) );
	// Because the 'CD_Measurement_Pos' column is frozen, left border of the 'CD_Measurement_DateTime' column is never shown.
	SetBorderRange( CD_Measurement_Pos, RD_Header_ColName, CD_Measurement_Pos, RD_Header_Unit, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, RGB( 192, 192, 192 ) );

	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateMainExpandCollapseButton( CD_Measurement_Pos, RD_Header_ButDlg );
	}
}

void CSheetHMCalc::_InitTADiagnosticSheet( CDS_HydroMod *pHM )
{
	TCHAR unitname[_MAXCHARS];
	CString str;

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == m_mapSSheetColumnWidth.size() || 0 == m_mapSSheetColumnWidth.count( SheetDescription::STADiagnostic ) )
	{
		_ResetColumnWidth( SheetDescription::STADiagnostic );
	}

	_ClearAllCombos( SheetDescription::STADiagnostic );

	SetMaxRows( RD_Header_FirstCirc - 1 );
	SetMaxCols( CD_TADiagnostic_Pointer );

	// All cells are static by default and filled with _T("").
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Freeze (make non scrollable) line above first circuit and first column.
	SetFreeze( CD_TADiagnostic_Pos, RD_Header_FirstCirc - 1 );

	// Increase rows height.
	double RowHeight = 12.75;// Reference.
	SetRowHeight( RD_Header_GroupName, RowHeight * 1.5 );
	SetRowHeight( RD_Header_ButDlg, RowHeight * 1.2 );
	SetRowHeight( RD_Header_ColName, RowHeight * 2 );
	SetRowHeight( RD_Header_Unit, RowHeight * 1.2 );
	SetRowHeight( RD_Header_Picture, RowHeight );

	// Set columns size.
	double dValueWidth = 8;
	double dTextWidth = 24;
	double dLightTextWidth = 12;

	if( false == m_bPrinting )
	{
		SetColWidthInPixels( CD_TADiagnostic_Sub, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Sub] );
	}
	else
	{
		ShowCol( CD_TADiagnostic_Sub, FALSE );
	}

	SetColWidthInPixels( CD_TADiagnostic_Pos, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Pos] );

	// Information.
	SetColWidthInPixels( CD_TADiagnostic_DateTime, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_DateTime] );
	SetColWidthInPixels( CD_TADiagnostic_Reference, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Reference] );
	SetColWidthInPixels( CD_TADiagnostic_Description, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Description] );
	SetColWidthInPixels( CD_TADiagnostic_DescriptionSep, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_DescriptionSep] );

	// Circuit.
	SetColWidthInPixels( CD_TADiagnostic_WaterChar, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_WaterChar] );
	SetColWidthInPixels( CD_TADiagnostic_Valve, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Valve] );
	SetColWidthInPixels( CD_TADiagnostic_Presetting, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Presetting] );
	SetColWidthInPixels( CD_TADiagnostic_Setting, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Setting] );
	SetColWidthInPixels( CD_TADiagnostic_ComputedSetting, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_ComputedSetting] );
	SetColWidthInPixels( CD_TADiagnostic_Kv, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Kv] );
	SetColWidthInPixels( CD_TADiagnostic_KvSignal, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_KvSignal] );
	SetColWidthInPixels( CD_TADiagnostic_DesignFlow, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_DesignFlow] );
	SetColWidthInPixels( CD_TADiagnostic_DesignFlowSep, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_DesignFlowSep] );


	// TA Diagnostic Measuring Data.
	SetColWidthInPixels( CD_TADiagnostic_Dp1, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Dp1] );
	SetColWidthInPixels( CD_TADiagnostic_Setting1, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Setting1] );
	SetColWidthInPixels( CD_TADiagnostic_Dp2, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Dp2] );
	SetColWidthInPixels( CD_TADiagnostic_MeasFlow, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_MeasFlow] );
	SetColWidthInPixels( CD_TADiagnostic_FlowDeviation, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_FlowDeviation] );
	SetColWidthInPixels( CD_TADiagnostic_FlowDeviationSep, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_FlowDeviationSep] );

	// TA Diagnostic DP trouble shooting.
	SetColWidthInPixels( CD_TADiagnostic_CircuitDpMeas, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_CircuitDpMeas] );
	SetColWidthInPixels( CD_TADiagnostic_CircuitDpDesign, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_CircuitDpDesign] );
	SetColWidthInPixels( CD_TADiagnostic_Picture, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_Picture] );
	SetColWidthInPixels( CD_TADiagnostic_DistribDpMeas, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_DistribDpMeas] );
	SetColWidthInPixels( CD_TADiagnostic_DistribDpDesign, m_mapSSheetColumnWidth[SheetDescription::STADiagnostic][CD_TADiagnostic_DistribDpDesign] );

	// All cells are not selectable (in header only).
	SetCellProperty( CD_TADiagnostic_Pos, RD_Header_GroupName, CD_TADiagnostic_Pointer, RD_Header_FirstCirc - 1, CSSheet::_SSCellProperty::CellNoSelection, true );

	///////////////////////////////////////////////////////////////////
	// Format the SubTitles.
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	// Define variables for the background color.
	COLORREF BckColor = GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor );

	// Flag unhidden columns.
	SetFlagShowEvenEmpty( CD_TADiagnostic_DescriptionSep, CD_TADiagnostic_DescriptionSep, true );
	SetFlagShowEvenEmpty( CD_TADiagnostic_DesignFlowSep, CD_TADiagnostic_DesignFlowSep, true );
	SetFlagShowEvenEmpty( CD_TADiagnostic_FlowDeviationSep, CD_TADiagnostic_FlowDeviationSep, true );
	SetFlagShowEvenEmpty( CD_TADiagnostic_Picture, CD_TADiagnostic_Picture, true );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Information' group.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	AddCellSpanW( CD_TADiagnostic_DateTime, RD_Header_GroupName, CD_TADiagnostic_DescriptionSep - CD_TADiagnostic_DateTime, 2 );
	_SetStaticText( CD_TADiagnostic_DateTime, RD_Header_GroupName, IDS_HMCALC_INFO );

	// Collapse button for information.
	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateExpandCollapseColumnButton( CD_TADiagnostic_DescriptionSep, RD_Header_ButDlg, CD_TADiagnostic_DateTime, CD_TADiagnostic_Description, GetMaxRows() );
		// Set that DateTime & Reference can't be collapsed.
		SetFlagCanBeCollapsed( CD_TADiagnostic_DateTime, CD_TADiagnostic_Reference, false );
		// Collapse Information columns.
		ChangeExpandCollapseColumnButtonState( CD_TADiagnostic_DescriptionSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse, false );
	}

	// Separator can't be hide (because this column is empty, 'HideEmptyColumns' will hide this column if
	// this flag is not set).
	SetFlagShowEvenEmpty( CD_TADiagnostic_DescriptionSep, CD_TADiagnostic_DescriptionSep, true );

	// 'Circuit' group.
	_SetGroupColor( BckColor );
	AddCellSpanW( CD_TADiagnostic_WaterChar, RD_Header_GroupName, CD_TADiagnostic_DesignFlowSep - CD_TADiagnostic_WaterChar, 2 );
	_SetStaticText( CD_TADiagnostic_WaterChar, RD_Header_GroupName, IDS_HMCALC_CIRCUIT );

	// Collapse button for Circuit.
	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateExpandCollapseColumnButton( CD_TADiagnostic_DesignFlowSep, RD_Header_ButDlg, CD_TADiagnostic_WaterChar, CD_TADiagnostic_DesignFlow, GetMaxRows() );
		// These columns can't be collapsed (Valve, Presetting & Setting).
		SetFlagCanBeCollapsed( CD_TADiagnostic_Valve, CD_TADiagnostic_Setting, false );
		SetFlagCanBeCollapsed( CD_TADiagnostic_Valve, CD_TADiagnostic_ComputedSetting, false );
		// Collapse Circuit columns.
		ChangeExpandCollapseColumnButtonState( CD_TADiagnostic_DesignFlowSep, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse, false );
	}

	// Separator can't be hide (because this column is empty, 'HideEmptyColumns' will hide this column if
	// this flag is not set).
	SetFlagShowEvenEmpty( CD_TADiagnostic_DesignFlowSep, CD_TADiagnostic_DesignFlowSep, true );

	// 'TA Balance+ Measuring Data' group.
	_SetGroupColor( BckColor );
	AddCellSpanW( CD_TADiagnostic_Dp1, RD_Header_GroupName, CD_TADiagnostic_FlowDeviationSep - CD_TADiagnostic_Dp1, 2 );

	_SetStaticText( CD_TADiagnostic_Dp1, RD_Header_GroupName, IDS_HMCALC_TBPMEASDATA );

	// Separator can't be hide (because this column is empty, 'HideEmptyColumns' will hide this column if
	// this flag is not set).
	SetFlagShowEvenEmpty( CD_TADiagnostic_FlowDeviationSep, CD_TADiagnostic_FlowDeviationSep, true );

	// 'TA Balance+ Dp Trouble Shooting' group.
	_SetGroupColor( BckColor );
	AddCellSpanW( CD_TADiagnostic_CircuitDpMeas, RD_Header_GroupName, CD_TADiagnostic_Pointer - CD_TADiagnostic_CircuitDpMeas, 2 );
	_SetStaticText( CD_TADiagnostic_CircuitDpMeas, RD_Header_GroupName, IDS_HMCALC_TBPDPTTROUBLESHOOTING );

	// This last column must be hide.
	ShowColRange( CD_TADiagnostic_Pointer, CD_TADiagnostic_Pointer, FALSE );

	///////////////////////////////////////////////////////////////////
	// Format the column headers.
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

	// Set the text of column headers.
	_SetStaticText( CD_TADiagnostic_DateTime, RD_Header_ColName, IDS_HMCALC_DATETIME );

	_SetStaticText( CD_TADiagnostic_Reference, RD_Header_ColName, IDS_HMCALC_REFERENCE );

	_SetStaticText( CD_TADiagnostic_Description, RD_Header_ColName, IDS_HMCALC_DESC );

	_SetStaticText( CD_TADiagnostic_WaterChar, RD_Header_ColName, IDS_HMCALC_WATERCHAR );

	_SetStaticText( CD_TADiagnostic_Valve, RD_Header_ColName, IDS_HMCALC_VALVE );

	_SetStaticText( CD_TADiagnostic_Presetting, RD_Header_ColName, IDS_HMCALC_PRESETTING );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_TADiagnostic_Presetting, RD_Header_Unit, IDS_SHEETHDR_TURNSPOS );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_TADiagnostic_Setting, RD_Header_ColName, IDS_HMCALC_MEAS_SETTING );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_TADiagnostic_Setting, RD_Header_Unit, IDS_SHEETHDR_TURNSPOS );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_TADiagnostic_ComputedSetting, RD_Header_ColName, IDS_HMCALC_TADIAG_COMPUTEDSETTING );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_TADiagnostic_ComputedSetting, RD_Header_Unit, IDS_SHEETHDR_TURNSPOS );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		_SetStaticText( CD_TADiagnostic_Kv, RD_Header_ColName, IDS_KV );
	}
	else
	{
		_SetStaticText( CD_TADiagnostic_Kv, RD_Header_ColName, IDS_CV );
	}

	if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		_SetStaticText( CD_TADiagnostic_KvSignal, RD_Header_ColName, IDS_KVSIGNAL );
	}
	else
	{
		_SetStaticText( CD_TADiagnostic_KvSignal, RD_Header_ColName, IDS_CVSIGNAL );
	}

	_SetStaticText( CD_TADiagnostic_DesignFlow, RD_Header_ColName, IDS_HMCALC_DESIGNFLOW );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	_SetStaticText( CD_TADiagnostic_DesignFlow, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_TADiagnostic_Dp1, RD_Header_ColName, IDS_HMCALC_TBPDP1 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	_SetStaticText( CD_TADiagnostic_Dp1, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_TADiagnostic_Setting1, RD_Header_ColName, IDS_HMCALC_TBPSETTING1 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_TADiagnostic_Setting1, RD_Header_Unit, IDS_SHEETHDR_TURNSPOS );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_TADiagnostic_Dp2, RD_Header_ColName, IDS_HMCALC_TBPDP2 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	_SetStaticText( CD_TADiagnostic_Dp2, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_TADiagnostic_MeasFlow, RD_Header_ColName, IDS_HMCALC_MEASFLOW );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	_SetStaticText( CD_TADiagnostic_MeasFlow, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	_SetStaticText( CD_TADiagnostic_FlowDeviation, RD_Header_ColName, IDS_HMCALC_FLOWDEV );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	_SetStaticText( CD_TADiagnostic_FlowDeviation, RD_Header_Unit, _T("%") );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Merge Circuit Dp measuring and Circuit Dp design for 'ColName' and 'Unit'.
	AddCellSpanW( CD_TADiagnostic_CircuitDpMeas, RD_Header_ColName, CD_TADiagnostic_CircuitDpDesign - CD_TADiagnostic_CircuitDpMeas + 1, 1 );
	AddCellSpanW( CD_TADiagnostic_CircuitDpMeas, RD_Header_Unit, CD_TADiagnostic_CircuitDpDesign - CD_TADiagnostic_CircuitDpMeas + 1, 1 );
	_SetStaticText( CD_TADiagnostic_CircuitDpMeas, RD_Header_ColName, IDS_HMCALC_CIRCUITDPMEASDESIGN );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	_SetStaticText( CD_TADiagnostic_CircuitDpMeas, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Merge Distribution Dp measuring and Distribution Dp design for 'ColName' and 'Unit'.
	AddCellSpanW( CD_TADiagnostic_DistribDpMeas, RD_Header_ColName, CD_TADiagnostic_DistribDpDesign - CD_TADiagnostic_DistribDpMeas + 1, 1 );
	AddCellSpanW( CD_TADiagnostic_DistribDpMeas, RD_Header_Unit, CD_TADiagnostic_DistribDpDesign - CD_TADiagnostic_DistribDpMeas + 1, 1 );
	_SetStaticText( CD_TADiagnostic_DistribDpMeas, RD_Header_ColName, IDS_HMCALC_DISTRIBDPMEASDESIGN );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	_SetStaticText( CD_TADiagnostic_DistribDpMeas, RD_Header_Unit, unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Draw border.
	SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Unit, CD_TADiagnostic_Description, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_TADiagnostic_WaterChar, RD_Header_Unit, CD_TADiagnostic_DesignFlow, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_TADiagnostic_Dp1, RD_Header_Unit, CD_TADiagnostic_FlowDeviation, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_TADiagnostic_CircuitDpMeas, RD_Header_Unit, CD_TADiagnostic_DistribDpDesign, RD_Header_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );

	// Freeze row header.
	SetFreeze( 0, RD_Header_Picture );

	// Add separator in header for the resizing feature.
	SetBorderRange( CD_TADiagnostic_DateTime, RD_Header_ColName, CD_TADiagnostic_Pointer, RD_Header_Unit, SS_BORDERTYPE_LEFT, SS_BORDERSTYLE_SOLID, RGB( 192, 192,
					192 ) );
	// Because the 'CD_TADiagnostic_Pos' column is frozen, left border of the 'CD_TADiagnostic_DateTime' column is never shown.
	SetBorderRange( CD_TADiagnostic_Pos, RD_Header_ColName, CD_TADiagnostic_Pos, RD_Header_Unit, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, RGB( 192, 192, 192 ) );

	if( false == m_bPrinting && false == m_bExporting )
	{
		CreateMainExpandCollapseButton( CD_TADiagnostic_Pos, RD_Header_ButDlg );
	}
}

void CSheetHMCalc::_FillDistributionSheet()
{
	if( NULL == m_pHM )
	{
		return;
	}

	long lCurRow = RD_Header_FirstCirc;

	CTableHM *pPipingTab = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
	bool bDiversityExist = pPipingTab->IsDiversityExistSomewhere();

	// Sort HM in function of position.
	std::map<int, CDS_HydroMod *> mapHMList;
	std::map<int, CDS_HydroMod *>::iterator mapHMListIter;

	CString str;
	long lHMRows = lCurRow;

	for( IDPTR IDPtr = m_pHM->GetFirst(); *IDPtr.ID; IDPtr = m_pHM->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)IDPtr.MP;
		mapHMList[pHM->GetPos()] = pHM;
		lHMRows += 1;	// 1 row by circuit.

		if( m_bPrinting && pHM->GetpDistrSupplyPipe()->GetSingularityCount() > 0 )
		{
			lHMRows += pHM->GetpDistrSupplyPipe()->GetSingularityCount() - 1;
		}
	}

	// Parent module at the first position.
	mapHMList[0] = m_pHM;

	lHMRows += 1;	// 1 row for parent module.

	if( true == m_bPrinting && m_pHM->GetpDistrSupplyPipe()->GetSingularityCount() > 0 )
	{
		lHMRows += m_pHM->GetpDistrSupplyPipe()->GetSingularityCount() - 1;
	}

	SetMaxRows( lHMRows );

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SDistribution );

		if( NULL != pclSheetDescription )
		{
			pclSheetDescription->Init( 1, GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
			pclSheetDescription->SetSelectableRangeRow( lCurRow, GetMaxRows() );
		}
	}

	// Verify if the project is frozen.
	bool bFreezed = false;

	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL != pPrjParam )
	{
		bFreezed = pPrjParam->IsFreezed();
	}

	CDS_HydroMod *pHM;
	CDS_HydroMod *pHMPrevious = NULL;
	CDS_HydroMod *pHMNext = NULL;

	for( mapHMListIter = mapHMList.begin(); mapHMListIter != mapHMList.end(); ++mapHMListIter )
	{
		TCHAR buf[10];
		pHM = mapHMListIter->second;

		// Prepare color to use in regards to the current row is in cut mode or not.
		bool bIsCutMode = _PrepareColors( pHM );

		// If we are on the root module...
		if( pHM == m_pHM )
		{
			// White line.
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );

			for( int j = CD_Distribution_Name; j < CD_Distribution_Pointer; j++ )
			{
				switch( j )
				{
					case CD_Distribution_SupplyPicture:
					case CD_Distribution_SupplyPipeSep:
					case CD_Distribution_ReturnPicture:
					case CD_Distribution_ReturnPipeSep:
						continue;
						break;

					default:
						_SetStaticText( j, lCurRow, _T( "" ) );
						break;
				}
			}
		}

		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Save pointer on HM.
		if( false == m_bExporting && false == m_bPrinting )
		{
			SetCellParam( CD_Distribution_Pointer, lCurRow, ( long )pHM );
		}

		// Sub ?
		if( pHM != m_pHM )
		{
			if( true == pHM->IsaModule() )
			{
				switch( pHM->GetReturnType() )
				{
					case CDS_HydroMod::ReturnType::Direct:
					default:
						SetPictureCellWithID( IDI_HTREE, CD_Distribution_Sub, lCurRow, CSSheet::PictureCellType::Icon );
						break;

					case CDS_HydroMod::ReturnType::Reverse:
						SetPictureCellWithID( IDI_HTREER, CD_Distribution_Sub, lCurRow, CSSheet::PictureCellType::Icon );
						break;
				}
			}
			else if( true == pHM->IsPending() )
			{
				SetPictureCellWithID( IDB_CIRCUITPENDING, CD_Distribution_Sub, lCurRow, CSSheet::PictureCellType::Bitmap );
			}
		}
		else if( 0 != pHM->GetLevel() && false == m_bPrinting && false == m_bExporting )
		{
			SetPictureCellWithID( IDI_ARROWBACK, CD_Distribution_Sub, lCurRow, CSSheet::PictureCellType::Icon );
		}

		// Circuit position.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module.
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		_stprintf_s( buf, SIZEOFINTCHAR( buf ), _T("%d"), pHM->GetPos() );

		if( pHM->IsCircuitIndex() )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_IndexFore );
		}

		_SetStaticText( CD_Distribution_Pos, lCurRow, buf );
		SetCellProperty( CD_Distribution_Pos, lCurRow, CellText, true );
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module.
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Circuit type.
		if( NULL != pHM->GetSchemeIDPtr().MP )
		{
			_SetStaticText( CD_Distribution_Type, lCurRow, ( ( CDB_CircuitScheme * )pHM->GetSchemeIDPtr().MP )->GetAbbrevSchCategName().c_str() );
		}

		// Flow.
		FormatCUDouble( CD_Distribution_Q, lCurRow, _U_FLOW, pHM->GetQ() );

		// Available H.
		if( true == pHM->IsCircuitIndex() )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_IndexFore );
		}

		double dHAvail = pHM->GetHAvail();
		FormatCUDouble( CD_Distribution_H, lCurRow, _U_DIFFPRESS, dHAvail );
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module.
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}
		else if( false == m_bPrinting && false == m_bExporting )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Name.
		FormatEditText( CD_Distribution_Name, lCurRow, pHM->GetHMName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES );

		// Description.
		FormatEditText( CD_Distribution_Desc, lCurRow, pHM->GetDescription(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES | ES_AUTOHSCROLL, 1000 );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Distribution supply pipe' group.
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// It's for the print mode. To allow to set the correct line after having printing some singularities.
		long lSingularityDirectRow = 0;
		long lSingularityReverseRow = 0;

		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
		bool fPipeVisible = true;

		if( pHM == m_pHM )
		{
			// Change background color for parent module.
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			// Distribution pipe doesn't exist for main partner valve.
			fPipeVisible = ( pHM->GetLevel() != 0 );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		CPipes *pPipe = pHM->GetpDistrSupplyPipe();

		if( NULL != pPipe && true == fPipeVisible )
		{
			// Picture.
			if( pHM != m_pHM )
			{
				AddCellSpanW( CD_Distribution_SupplyPicture, lCurRow - 1, 1, 1 );
				int iPicID = ( CDS_HydroMod::ReturnType::Direct == pHM->GetReturnType() ) ? IDI_HMCALCRARROW : IDI_HMCALCRARROWSUP;
				SetPictureCellWithID( iPicID, CD_Distribution_SupplyPicture, lCurRow, CSSheet::PictureCellType::Icon );
			}

			// Distribution pipe series.
			if( NULL == m_pDistributionSupplyPipeTable )
			{
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatComboList( CD_Distribution_SupplyPipeSeries, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Distribution_SupplyPipeSeries, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					// Pipe length exist but size and series are not yet defined, flow is 0 for instance.
					if( NULL != pPipe->GetPipeSeries() )
					{
						ComboBoxSendMessage( CD_Distribution_SupplyPipeSeries, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( pPipe->GetPipeSeries()->GetName() ) );
						ComboBoxSendMessage( CD_Distribution_SupplyPipeSeries, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					}

					SetCellParam( CD_Distribution_SupplyPipeSeries, lCurRow, (LPARAM)pPipe );
				}
				else
				{
					if( NULL != pPipe->GetPipeSeries() )
					{
						_SetStaticText( CD_Distribution_SupplyPipeSeries, lCurRow, pPipe->GetPipeSeries()->GetName() );
						SetFontBold( CD_Distribution_SupplyPipeSeries, lCurRow, ( true == bFreezed ) ? TRUE : FALSE );
					}
				}
			}

			// Size.
			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				FormatComboList( CD_Distribution_SupplyPipeSize, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				ComboBoxSendMessage( CD_Distribution_SupplyPipeSize, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

				// Pipe length exist but size is not yet defined, flow is 0 for instance.
				if( NULL != pPipe->GetIDPtr().MP )
				{
					ComboBoxSendMessage( CD_Distribution_SupplyPipeSize, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() ) );
					ComboBoxSendMessage( CD_Distribution_SupplyPipeSize, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
				}

				SetCellParam( CD_Distribution_SupplyPipeSize, lCurRow, (LPARAM)pPipe );
			}
			else
			{
				if( NULL != pPipe->GetIDPtr().MP )
				{
					_SetStaticText( CD_Distribution_SupplyPipeSize, lCurRow, ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() );
				}
			}

			if( true == pHM->IsLocked( pPipe->GetLocate() ) )
			{
				SetFontBold( CD_Distribution_SupplyPipeSize, lCurRow, TRUE );
			}
			else
			{
				SetFontBold( CD_Distribution_SupplyPipeSize, lCurRow, FALSE );
			}

			// Length is editable.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}
			else if( false == m_bPrinting && false == m_bExporting )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
			}

			// For the root module no distribution pipe.
			if( 0 == pHM->GetLevel() )
			{
				FormatCUDouble( CD_Distribution_SupplyPipeLength, lCurRow, _U_LENGTH, 0.0 );    // Set to _T("")
			}
			else
			{
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatEditDouble( CD_Distribution_SupplyPipeLength, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, false );
				}
				else
				{
					FormatCUDouble( CD_Distribution_SupplyPipeLength, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, true, false );
				}
			}

			SetFontBold( CD_Distribution_SupplyPipeLength, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
			SetTextPatternProperty( CSSheet::TPP_FontBold, (LPARAM) FALSE );

			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			// Dp.
			double dSI = pPipe->GetPipeDp();
			FormatCUDouble( CD_Distribution_SupplyPipeDp, lCurRow, _U_DIFFPRESS, pPipe->GetPipeDp() );

			// Flow.
			FormatCUDouble( CD_Distribution_SupplyPipeTotalQ, lCurRow, _U_FLOW, pPipe->GetRealQ() );

			// Diversity factor.
			CString str = _T( "" );
			_SetStaticText( CD_Distribution_SupplyDiversityFactor, lCurRow, str );
			SetFlagShowEvenEmpty( CD_Distribution_SupplyDiversityFactor, CD_Distribution_SupplyDiversityFactor, false );

			if( true == bDiversityExist )
			{
				if( true == pPipe->IsDiversityApplied() )
				{
					double dDiversity = pPipe->GetRealQ() / pPipe->GetTotalQ();

					if( dDiversity > 0.1 )
					{
						str = WriteDouble( dDiversity, 2 );
						_SetStaticText( CD_Distribution_SupplyDiversityFactor, lCurRow, str );
						SetBackColor( CD_Distribution_Pos, lCurRow, m_rFillColors.m_DiversityBkg );
						SetBackColor( CD_Distribution_SupplyPipeTotalQ, lCurRow, m_rFillColors.m_DiversityBkg );
						SetBackColor( CD_Distribution_SupplyDiversityFactor, lCurRow, m_rFillColors.m_DiversityBkg );
					}
				}
			}

			// Linear Dp.
			FormatCUDouble( CD_Distribution_SupplyPipeLinDp, lCurRow, _U_LINPRESSDROP, pPipe->GetLinDp() );

			if( eb3False == pPipe->CheckLinDpMinMax() )
			{
				SetForeColor( CD_Distribution_SupplyPipeLinDp, lCurRow, m_rFillColors.m_ErrorFore );
			}
			else
			{
				SetForeColor( CD_Distribution_SupplyPipeLinDp, lCurRow, m_rFillColors.m_NormalFore );
			}

			// Velocity.
			FormatCUDouble( CD_Distribution_SupplyPipeVelocity, lCurRow, _U_VELOCITY, pPipe->GetVelocity() );

			if( eb3False == pPipe->CheckVelocityMinMax() )
			{
				SetForeColor( CD_Distribution_SupplyPipeVelocity, lCurRow, m_rFillColors.m_ErrorFore );
			}
			else
			{
				SetForeColor( CD_Distribution_SupplyPipeVelocity, lCurRow, m_rFillColors.m_NormalFore );
			}
		}

		// Singularities (connections or/and accessories).
		// Remark: only for modules that are not in the root level
		lSingularityDirectRow = lCurRow;

		if( pHM->GetLevel() > 0 && NULL != pPipe )
		{
			// In distribution return pipe, we need the circuit n+1 to compute connection singularity of current circuit (n).
			std::map<int, CDS_HydroMod *>::iterator mapHMListIterNext = mapHMListIter;
			pHMNext = NULL;

			if( mapHMListIter != mapHMList.end() )
			{
				mapHMListIterNext = mapHMListIter;
				++mapHMListIterNext;

				if( mapHMListIterNext != mapHMList.end() )
				{
					pHMNext = mapHMListIterNext->second;
				}
			}

			if( true == m_bPrinting )
			{
				// One line by singularity.
				CPipes::CSingularity Singularity;
				int iSingulPos = pPipe->GetFirstSingularity( &Singularity );
				bool fFirstPass = true;

				while( iSingulPos != -1 )
				{
					CString strDescription, strInputData;

					if( true == pPipe->GetSingularityDetails( &Singularity, &strDescription, &strInputData ) )
					{
						if( false == fFirstPass )
						{
							lSingularityDirectRow++;
						}

						// Description.
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
						SetStaticTextEx( CD_Distribution_SupplyAccDesc, lSingularityDirectRow, strDescription, true );

						// Input data.
						SetStaticTextEx( CD_Distribution_SupplyAccData, lSingularityDirectRow, strInputData, true );

						// Dp.
						CString strDp = WriteCUDouble( _U_DIFFPRESS, pPipe->GetSingularityDp( &Singularity, pHMNext ), false );
						SetStaticTextEx( CD_Distribution_SupplyAccDp, lSingularityDirectRow, strDp, true );

						fFirstPass = false;
					}

					iSingulPos = pPipe->GetNextSingularity( iSingulPos, &Singularity );
				}
			}
			else
			{
				// Description.
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				CString str = pPipe->GetSingularitiesDescriptionShort( true );
				_SetStaticText( CD_Distribution_SupplyAccDesc, lCurRow, str );

				// Input data.
				str = pPipe->GetSingularitiesInputDataShort();
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ELLIPSES );
				_SetStaticText( CD_Distribution_SupplyAccData, lCurRow, str );

				// Dp.
				FormatCUDouble( CD_Distribution_SupplyAccDp, lCurRow, _U_DIFFPRESS, pPipe->GetSingularityTotalDp( true, pHMNext ) );
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Distribution return pipe' group.
		////////////////////////////////////////////////////////////////////////////////////////////////////

		if( true == fPipeVisible && CDS_HydroMod::ReturnType::Reverse == pHM->GetReturnType() )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			if( pHM == m_pHM )
			{
				// Change background color for parent module.
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			CPipes *pPipe = pHM->GetpDistrReturnPipe();

			if( NULL != pPipe )
			{
				// Picture.
				if( pHM != m_pHM )
				{
					AddCellSpanW( CD_Distribution_ReturnPicture, lCurRow - 1, 1, 1 );
					SetPictureCellWithID( IDI_HMCALCRARROWRET, CD_Distribution_ReturnPicture, lCurRow, CSSheet::PictureCellType::Icon );
				}

				// Distribution pipe series.
				if( NULL == m_pDistributionReturnPipeTable )
				{
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
					{
						FormatComboList( CD_Distribution_ReturnPipeSeries, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						ComboBoxSendMessage( CD_Distribution_ReturnPipeSeries, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

						// Pipe length exist but size and series are not yet defined, flow is 0 for instance.
						if( NULL != pPipe->GetPipeSeries() )
						{
							ComboBoxSendMessage( CD_Distribution_ReturnPipeSeries, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( pPipe->GetPipeSeries()->GetName() ) );
							ComboBoxSendMessage( CD_Distribution_ReturnPipeSeries, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
						}

						SetCellParam( CD_Distribution_ReturnPipeSeries, lCurRow, (LPARAM)pPipe );
					}
					else
					{
						if( NULL != pPipe->GetPipeSeries() )
						{
							_SetStaticText( CD_Distribution_ReturnPipeSeries, lCurRow, pPipe->GetPipeSeries()->GetName() );
							SetFontBold( CD_Distribution_ReturnPipeSeries, lCurRow, ( true == bFreezed ) ? TRUE : FALSE );
						}
					}
				}

				// Size.
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatComboList( CD_Distribution_ReturnPipeSize, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Distribution_ReturnPipeSize, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					// Pipe length exist but size is not yet defined, flow is 0 for instance.
					if( NULL != pPipe->GetIDPtr().MP )
					{
						ComboBoxSendMessage( CD_Distribution_ReturnPipeSize, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() ) );
						ComboBoxSendMessage( CD_Distribution_ReturnPipeSize, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					}

					SetCellParam( CD_Distribution_ReturnPipeSize, lCurRow, (LPARAM)pPipe );
				}
				else
				{
					if( NULL != pPipe->GetIDPtr().MP )
					{
						_SetStaticText( CD_Distribution_ReturnPipeSize, lCurRow, ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() );
					}
				}

				if( true == pHM->IsLocked( pPipe->GetLocate() ) )
				{
					SetFontBold( CD_Distribution_ReturnPipeSize, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Distribution_ReturnPipeSize, lCurRow, FALSE );
				}

				// Length.

				// Length is editable.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}
				else if( false == m_bPrinting && false == m_bExporting )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
				}

				// For the root module no distribution pipe.
				if( 0 == pHM->GetLevel() )
				{
					FormatCUDouble( CD_Distribution_ReturnPipeLength, lCurRow, _U_LENGTH, 0.0 );    // Set to _T("")
				}
				else
				{
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
					{
						FormatEditDouble( CD_Distribution_ReturnPipeLength, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, false );
					}
					else
					{
						FormatCUDouble( CD_Distribution_ReturnPipeLength, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, true, false );
					}
				}

				SetFontBold( CD_Distribution_ReturnPipeLength, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
				SetTextPatternProperty( CSSheet::TPP_FontBold, (LPARAM) FALSE );

				// Dp.
				double dSI = pPipe->GetPipeDp();
				FormatCUDouble( CD_Distribution_ReturnPipeDp, lCurRow, _U_DIFFPRESS, pPipe->GetPipeDp() );

				// Flow.
				FormatCUDouble( CD_Distribution_ReturnPipeTotalQ, lCurRow, _U_FLOW, pPipe->GetRealQ() );

				// Diversity factor.
				CString str = _T( "" );
				_SetStaticText( CD_Distribution_ReturnDiversityFactor, lCurRow, str );
				SetFlagShowEvenEmpty( CD_Distribution_ReturnDiversityFactor, CD_Distribution_ReturnDiversityFactor, false );

				if( true == bDiversityExist )
				{
					if( true == pPipe->IsDiversityApplied() )
					{
						double dDiversity = pPipe->GetRealQ() / pPipe->GetTotalQ();

						if( dDiversity > 0.1 )
						{
							str = WriteDouble( dDiversity, 2 );
							_SetStaticText( CD_Distribution_ReturnDiversityFactor, lCurRow, str );
							SetBackColor( CD_Distribution_Pos, lCurRow, m_rFillColors.m_DiversityBkg );
							SetBackColor( CD_Distribution_ReturnPipeTotalQ, lCurRow, m_rFillColors.m_DiversityBkg );
							SetBackColor( CD_Distribution_ReturnDiversityFactor, lCurRow, m_rFillColors.m_DiversityBkg );
						}
					}
				}

				// Linear Dp.
				FormatCUDouble( CD_Distribution_ReturnPipeLinDp, lCurRow, _U_LINPRESSDROP, pPipe->GetLinDp() );

				if( eb3False == pPipe->CheckLinDpMinMax() )
				{
					SetForeColor( CD_Distribution_ReturnPipeLinDp, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_Distribution_ReturnPipeLinDp, lCurRow, m_rFillColors.m_NormalFore );
				}

				// Velocity.
				FormatCUDouble( CD_Distribution_ReturnPipeVelocity, lCurRow, _U_VELOCITY, pPipe->GetVelocity() );

				if( eb3False == pPipe->CheckVelocityMinMax() )
				{
					SetForeColor( CD_Distribution_ReturnPipeVelocity, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_Distribution_ReturnPipeVelocity, lCurRow, m_rFillColors.m_NormalFore );
				}
			}

			// Singularities (connections or/and accessories).
			// Remark: only for modules that are not in the root level
			lSingularityReverseRow = lCurRow;

			if( pHM->GetLevel() > 0 )
			{
				if( true == m_bPrinting )
				{
					// One line by singularity.
					CPipes::CSingularity Singularity;
					int iSingulPos = pPipe->GetFirstSingularity( &Singularity );
					bool fFirstPass = true;

					while( iSingulPos != -1 )
					{
						CString strDescription, strInputData;

						if( true == pPipe->GetSingularityDetails( &Singularity, &strDescription, &strInputData ) )
						{
							if( false == fFirstPass )
							{
								lSingularityReverseRow++;
							}

							// Description.
							SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
							SetStaticTextEx( CD_Distribution_ReturnAccDesc, lSingularityReverseRow, strDescription, true );

							// Input Data.
							SetStaticTextEx( CD_Distribution_ReturnAccData, lSingularityReverseRow, strInputData, true );

							// Dp.
							CString strDp = WriteCUDouble( _U_DIFFPRESS, pPipe->GetSingularityDp( &Singularity, pHMNext ), false );
							SetStaticTextEx( CD_Distribution_ReturnAccDp, lSingularityReverseRow, strDp, true );

							fFirstPass = false;
						}

						iSingulPos = pPipe->GetNextSingularity( iSingulPos, &Singularity );
					}
				}
				else
				{
					// Description.
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
					CString str = pPipe->GetSingularitiesDescriptionShort( true );
					_SetStaticText( CD_Distribution_ReturnAccDesc, lCurRow, str );

					// Input data.
					str = pPipe->GetSingularitiesInputDataShort();
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ELLIPSES );
					_SetStaticText( CD_Distribution_ReturnAccData, lCurRow, str );

					// Dp.
					FormatCUDouble( CD_Distribution_ReturnAccDp, lCurRow, _U_DIFFPRESS, pPipe->GetSingularityTotalDp( true, pHMPrevious ) );
				}
			}

			// Add borders between each product selection.
			SetCellBorder( CD_Distribution_ReturnPipeSeries, lCurRow - 1, CD_Distribution_ReturnPipeSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
						   _LIGHTGRAY );
			SetCellBorder( CD_Distribution_ReturnAccDesc, lCurRow - 1, CD_Distribution_ReturnAccDp, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
						   _LIGHTGRAY );
		}

		if( pHM == m_pHM )
		{
			// If we are in printing mode and there is more than one singularity in the accessory group...
			if( true == m_bPrinting && max( lSingularityDirectRow, lSingularityReverseRow ) > lCurRow )
			{
				for( long lLoopRow = lCurRow + 1; lLoopRow <= max( lSingularityDirectRow, lSingularityReverseRow ); lLoopRow++ )
				{
					// Change background color for parent module in the line added.
					for( int lLoopColumn = CD_Distribution_Name; lLoopColumn <= CD_Distribution_Pointer; lLoopColumn++ )
					{
						switch( lLoopColumn )
						{
							case CD_Distribution_SupplyPicture:
							case CD_Distribution_SupplyPipeSep:
							case CD_Distribution_ReturnPicture:
							case CD_Distribution_ReturnPipeSep:
								continue;
								break;

							default:
								SetBackColor( lLoopColumn, lLoopRow, m_rFillColors.m_ParentModuleBkg );
								break;
						}
					}
				}

				lCurRow = max( lSingularityDirectRow, lSingularityReverseRow );
			}

			// Add a solid border below the root module.
			SetCellBorder( CD_Distribution_Name, lCurRow, CD_Distribution_H, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Distribution_SupplyPipeSeries, lCurRow, CD_Distribution_SupplyPipeSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
						   SPREAD_COLOR_NONE );
			SetCellBorder( CD_Distribution_SupplyAccDesc, lCurRow, CD_Distribution_SupplyAccDp, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
						   SPREAD_COLOR_NONE );
			SetCellBorder( CD_Distribution_ReturnPipeSeries, lCurRow, CD_Distribution_ReturnPipeSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
						   SPREAD_COLOR_NONE );
			SetCellBorder( CD_Distribution_ReturnAccDesc, lCurRow, CD_Distribution_ReturnAccDp, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
						   SPREAD_COLOR_NONE );

			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			for( int c = CD_Distribution_Sub; c < CD_Distribution_Pointer; c++ )
			{
				_SetStaticText( c, lCurRow + 1, _T( "" ) );
			}

			lCurRow += 2;
		}
		else
		{
			lCurRow = max( lSingularityDirectRow, lSingularityReverseRow );
			lCurRow++;
		}

		// Add gray border between each product.
		SetCellBorder( CD_Distribution_Name, lCurRow - 1, CD_Distribution_H, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Distribution_SupplyPipeSeries, lCurRow - 1, CD_Distribution_SupplyPipeSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
					   _LIGHTGRAY );
		SetCellBorder( CD_Distribution_SupplyAccDesc, lCurRow - 1, CD_Distribution_SupplyAccDp, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
					   _LIGHTGRAY );

		pHMPrevious = pHM;
	}

	lCurRow--;
	SetCellBorder( CD_Distribution_Name, lCurRow, CD_Distribution_ReturnAccDp, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	_HideEmptyColumns();

	// Check now if we can hide collapse button or not.
	VerifyExpandCollapseColumnButtons( CRect( CD_Distribution_Name, RD_Header_FirstCirc, CD_Distribution_ReturnAccDp, lCurRow /*- 1*/ ), RD_Header_ButDlg );

	// Check if last column is a separator.
	// Remarks: in this case, we must set 'CellCantBePainted' column property to true to
	// avoid selection to be drawn on this last column.
	CheckLastColumnFlagCantBePainted( RD_Header_ButDlg );

	// Check if there is no successive separators.
	_HideSuccessiveSeparator( CD_Distribution_SupplyPipeSep, CD_Distribution_ReturnAccDp, RD_Header_FirstCirc, SheetDescription::SDistribution );
}

void CSheetHMCalc::_FillCircuitsSheet()
{
	if( NULL == m_pHM )
	{
		return;
	}

	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SCircuit );

	if( NULL == pclSheetDescription )
	{
		return;
	}

	long lCurRow = RD_Header_FirstCirc;

	// Sort HM in function of position
	CRank HMList;
	CString str;
	long lHMRows = lCurRow;

	for( IDPTR IDPtr = m_pHM->GetFirst(); '\0' != *IDPtr.ID; IDPtr = m_pHM->GetNext() )
	{

		CDS_HydroMod *pHM = (CDS_HydroMod *)IDPtr.MP;
		HMList.Add( str, pHM->GetPos(), (LPARAM)IDPtr.MP );
		lHMRows += 1;	// 1 row by circuit

		if( true == m_bPrinting && pHM->GetpCircuitPrimaryPipe()->GetSingularityCount() > 0 )
		{
			lHMRows += pHM->GetpCircuitPrimaryPipe()->GetSingularityCount() - 1;
		}
	}

	// Parent module at the first position
	HMList.Add( str, 0, (LPARAM)m_pHM->GetIDPtr().MP );
	lHMRows += 1;	// 1 row for parent module

	if( true == m_bPrinting && m_pHM->GetpCircuitPrimaryPipe()->GetSingularityCount() > 0 )
	{
		lHMRows += m_pHM->GetpCircuitPrimaryPipe()->GetSingularityCount() - 1;
	}

	SetMaxRows( lHMRows );

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		pclSheetDescription->Init( 1, GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
		pclSheetDescription->SetSelectableRangeRow( lCurRow, GetMaxRows() );
	}

	// Used to hide empty columns.
	bool bCVDescription = true;
	CDS_HydroMod *pHM = NULL;
	LPARAM lparam = (LPARAM)0;
	int iNbrBv = 0;
	int	iNbrBvDpSig = 0;
	bool bAtLeastOneCircuitPipeLength = false;

	// Verify if the project is frozen.
	bool bFreezed = false;
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL != pPrjParam )
	{
		bFreezed = pPrjParam->IsFreezed();
	}

	pclSheetDescription->RestartRemarkGenerator();

	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		TCHAR tcBuffer[10];
		pHM = (CDS_HydroMod *)lparam;
		ASSERT( lCurRow <= lHMRows );

		// Prepare color to use in regards to the current row is in cut mode or not.
		bool bIsCutMode = _PrepareColors( pHM );

		// If we are on the root module...
		if( pHM == m_pHM )
		{
			// White line
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );

			for( int lLoopColumn = CD_Circuit_UnitDesc; lLoopColumn < CD_Circuit_Pointer; lLoopColumn++ )
			{
				switch( lLoopColumn )
				{
					case CD_Circuit_DescSep:
					case CD_Circuit_UnitSep:
					case CD_Circuit_PumpSep:
					case CD_Circuit_CVSep:
					case CD_Circuit_ActuatorSep:
					case CD_Circuit_SmartValveSep:
					case CD_Circuit_BvSep:
					case CD_Circuit_SvSep:
					case CD_Circuit_DpCSep:
					case CD_Circuit_BvBypSep:
					case CD_Circuit_PipeSep:
					case CD_Circuit_PipeAccSep:
						continue;
						break;

					default:
						_SetStaticText( lLoopColumn, lCurRow, _T( "" ) );
						break;
				}
			}
		}

		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Save pointer on HM.
		if( false == m_bExporting && false == m_bPrinting )
		{
			SetCellParam( CD_Circuit_Pointer, lCurRow, ( long )pHM );
		}

		// Sub ?
		if( pHM != m_pHM )
		{
			if( true == pHM->IsaModule() )
			{
				SetPictureCellWithID( IDI_HTREE, CD_Circuit_Sub, lCurRow, CSSheet::PictureCellType::Icon );
			}
			else if( true == pHM->IsPending() )
			{
				SetPictureCellWithID( IDB_CIRCUITPENDING, CD_Circuit_Sub, lCurRow, CSSheet::PictureCellType::Bitmap );
			}
		}
		else
		{
			if( 0 != pHM->GetLevel() && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_ARROWBACK, CD_Circuit_Sub, lCurRow, CSSheet::PictureCellType::Icon );
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Circuit Position
		////////////////////////////////////////////////////////////////////////////////////////////////////

		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module.
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}

		_stprintf_s( tcBuffer, SIZEOFINTCHAR( tcBuffer ), _T("%d"), pHM->GetPos() );

		if( true == pHM->IsCircuitIndex() )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_IndexFore );
		}

		_SetStaticText( CD_Circuit_Pos, lCurRow, tcBuffer );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Description' column
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// HydroMod description.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module.
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}
		else if( false == m_bPrinting && false == m_bExporting )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Name.
		FormatEditText( CD_Circuit_Name, lCurRow, pHM->GetHMName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES );

		// Description.
		FormatEditText( CD_Circuit_Desc, lCurRow, pHM->GetDescription(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES | ES_AUTOHSCROLL, 1000 );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Unit' group
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// Unit description.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module.
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}
		else if( false == m_bPrinting && false == m_bExporting )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		if( NULL != pHM->GetpTermUnit() )
		{
			FormatEditText( CD_Circuit_UnitDesc, lCurRow, pHM->GetpTermUnit()->GetDescription(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES );

			// Unit Flow.
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

			int iValidity = pHM->CheckValidity();

			bool bFlowError = ( ( CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) ) 
					|| ( CDS_HydroMod::eValidityFlags::evfFlowTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooLow ) ) );

			bool bPowerError = ( ( CDS_HydroMod::eValidityFlags::evfPowerTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooHigh ) ) 
					|| ( CDS_HydroMod::eValidityFlags::evfPowerTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooLow ) ) );

			if( NULL != pHM->GetpSchcat() && false == pHM->GetpSchcat()->IsSecondarySideExist() )
			{
				switch( pHM->GetpTermUnit()->GetQType() )
				{
					case CTermUnit::_QType::Q:

						if( true == pHM->IsaModule() )
						{
							// Change background in regards of valve is a partner valve or not.
							if( pHM == m_pHM )
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
							}
							else if( false == m_bPrinting && false == m_bExporting )
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
							}

							_SetStaticText( CD_Circuit_UnitQ, lCurRow, WriteCUDouble( _U_FLOW, pHM->GetpTermUnit()->GetQ() ) );

							if( true == bFlowError )
							{
								SetForeColor( CD_Circuit_UnitQ, lCurRow, m_rFillColors.m_ErrorFore );
							}

							SetFontBold( CD_Circuit_UnitQ, lCurRow, FALSE );
						}
						else
						{
							if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
							{
								FormatEditDouble( CD_Circuit_UnitQ, lCurRow, _U_FLOW, pHM->GetpTermUnit()->GetQ(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							}
							else
							{
								FormatCUDouble( CD_Circuit_UnitQ, lCurRow, _U_FLOW, pHM->GetpTermUnit()->GetQ() );
							}

							SetFontBold( CD_Circuit_UnitQ, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

							if( true == bFlowError )
							{
								SetForeColor( CD_Circuit_UnitQ, lCurRow, m_rFillColors.m_ErrorFore );
							}

							// HYS-1882: DT
							if( true == pHM->IsDTFieldDisplayedInFlowMode() )
							{
								if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
								{
									FormatEditDouble( CD_Circuit_UnitDT, lCurRow, _U_DIFFTEMP, pHM->GetpTermUnit()->GetDTFlowMode(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
								}
								else
								{
									FormatCUDouble( CD_Circuit_UnitDT, lCurRow, _U_DIFFTEMP, pHM->GetpTermUnit()->GetDTFlowMode() );
								}

								SetFontBold( CD_Circuit_UnitDT, lCurRow, (true == bFreezed && false == m_bPrinting && false == m_bExporting) ? TRUE : FALSE );

								if( false == pHM->CheckIfTempAreValid() )
								{
									SetForeColor( CD_Circuit_UnitDT, lCurRow, m_rFillColors.m_ErrorFore );
								}
							}
						}

						break;

					case CTermUnit::_QType::PdT:

						if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
						{
							FormatEditDouble( CD_Circuit_UnitP, lCurRow, _U_TH_POWER, pHM->GetpTermUnit()->GetP(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							FormatEditDouble( CD_Circuit_UnitDT, lCurRow, _U_DIFFTEMP, pHM->GetpTermUnit()->GetDT(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						}
						else
						{
							FormatCUDouble( CD_Circuit_UnitP, lCurRow, _U_TH_POWER, pHM->GetpTermUnit()->GetP() );
							FormatCUDouble( CD_Circuit_UnitDT, lCurRow, _U_DIFFTEMP, pHM->GetpTermUnit()->GetDT() );
						}

						SetFontBold( CD_Circuit_UnitP, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );
						SetFontBold( CD_Circuit_UnitDT, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
						FormatCUDouble( CD_Circuit_UnitQ, lCurRow, _U_FLOW, pHM->GetpTermUnit()->GetQ() );
						SetFontBold( CD_Circuit_UnitQ, lCurRow, FALSE );

						if( true == bFlowError )
						{
							SetForeColor( CD_Circuit_UnitQ, lCurRow, m_rFillColors.m_ErrorFore );
						}

						if( true == bPowerError )
						{
							SetForeColor( CD_Circuit_UnitP, lCurRow, m_rFillColors.m_ErrorFore );
							SetForeColor( CD_Circuit_UnitDT, lCurRow, m_rFillColors.m_ErrorFore );
						}

						break;
				}
			}
			else
			{
				// In case of a 2W injection circuit, Q is not saved in the terminal unit.

				// Change background in regards of valve is a partner valve or not.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}
				else if( false == m_bPrinting && false == m_bExporting )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
				}

				_SetStaticText( CD_Circuit_UnitQ, lCurRow, WriteCUDouble( _U_FLOW, pHM->GetQ() ) );

				if( true == bFlowError )
				{
					SetForeColor( CD_Circuit_UnitQ, lCurRow, m_rFillColors.m_ErrorFore );
				}

				SetFontBold( CD_Circuit_UnitQ, lCurRow, FALSE );
			}

			// Unit Dp.
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}
			else if( false == m_bPrinting && false == m_bExporting )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

			if( NULL != pHM->GetpSchcat() && false == pHM->GetpSchcat()->IsSecondarySideExist() )
			{
				switch( pHM->GetpTermUnit()->GetDpType() )
				{
					case CDS_HydroMod::eDpType::Dp:
						if( true == pHM->IsaModule() )
						{
							// Change background in regards of valve is a partner valve or not.
							if( pHM == m_pHM )
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
							}
							else
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
							}

							_SetStaticText( CD_Circuit_UnitDp, lCurRow, WriteCUDouble( _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp() ) );
							SetFontBold( CD_Circuit_UnitDp, lCurRow, FALSE );
						}
						else
						{
							if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
							{
								FormatEditDouble( CD_Circuit_UnitDp, lCurRow, _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							}
							else
							{
								FormatCUDouble( CD_Circuit_UnitDp, lCurRow, _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp() );
							}

							SetFontBold( CD_Circuit_UnitDp, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );
						}

						break;

					case CDS_HydroMod::eDpType::QDpref:
					{
						double dQRef, dDpRef;
						pHM->GetpTermUnit()->GetDp( NULL, &dDpRef, &dQRef );

						if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
						{
							FormatEditDouble( CD_Circuit_UnitQref, lCurRow, _U_FLOW, dQRef, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							FormatEditDouble( CD_Circuit_UnitDpref, lCurRow, _U_DIFFPRESS, dDpRef, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						}
						else
						{
							FormatCUDouble( CD_Circuit_UnitQref, lCurRow, _U_FLOW, dQRef );
							FormatCUDouble( CD_Circuit_UnitDpref, lCurRow, _U_DIFFPRESS, dDpRef );
						}

						SetFontBold( CD_Circuit_UnitQref, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );
						SetFontBold( CD_Circuit_UnitDpref, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
						FormatCUDouble( CD_Circuit_UnitDp, lCurRow, _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp() );
						SetFontBold( CD_Circuit_UnitDp, lCurRow, FALSE );
					}
					break;

					case CDS_HydroMod::eDpType::Kv:
					case CDS_HydroMod::eDpType::Cv:
					{
						double dValue;
						pHM->GetpTermUnit()->GetDp( NULL, &dValue, NULL );
						long lColumn = ( CDS_HydroMod::eDpType::Kv == pHM->GetpTermUnit()->GetDpType() ) ? CD_Circuit_UnitKv : CD_Circuit_UnitCv;

						if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
						{
							FormatEditDouble( lColumn, lCurRow, _C_KVCVCOEFF, dValue, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );

						}
						else
						{
							FormatCUDouble( lColumn, lCurRow, _C_KVCVCOEFF, dValue );
						}

						SetFontBold( lColumn, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
						FormatCUDouble( CD_Circuit_UnitDp, lCurRow, _U_DIFFPRESS, pHM->GetpTermUnit()->GetDp() );
						SetFontBold( CD_Circuit_UnitDp, lCurRow, FALSE );
					}
					break;
				}
			}
			else
			{
				// In case of a 2W injection circuit, Dp is not saved in the terminal unit.

				// Change background in regards of valve is a partner valve or not.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}
				else
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
				}

				_SetStaticText( CD_Circuit_UnitDp, lCurRow, _T("-") );
				SetFontBold( CD_Circuit_UnitDp, lCurRow, FALSE );
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Pump group.
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// We take only the case for root module (level = 0) with pump that are not a circuit in injection.
		if( 0 == pHM->GetLevel() && NULL != pHM->GetpPump() && ( NULL != pHM->GetpSchcat() && false == pHM->GetpSchcat()->IsSecondarySideExist() ) )
		{
			// Head.
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				FormatEditDouble( CD_Circuit_PumpHUser, lCurRow, _U_DIFFPRESS, pHM->GetpPump()->GetHpump(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
			}
			else
			{
				FormatCUDouble( CD_Circuit_PumpHUser, lCurRow, _U_DIFFPRESS, pHM->GetpPump()->GetHpump() );
			}

			SetFontBold( CD_Circuit_PumpHUser, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

			// Hmin.
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			_SetStaticText( CD_Circuit_PumpHMin, lCurRow, WriteCUDouble( _U_DIFFPRESS, pHM->GetpPump()->GetHmin() ) );
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Control Valve' group
		////////////////////////////////////////////////////////////////////////////////////////////////////

		if( true == pHM->IsCvExist( true ) && eb3True == pHM->GetpCV()->IsCVLocInPrimary() )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			if( true == pHM->GetpCV()->IsTaCV() )
			{
				CDB_RegulatingValve *pclControlValve = (CDB_RegulatingValve *)( pHM->GetpCV()->GetCvIDPtr().MP );
				CString strValveName = pclControlValve->GetName();

				if( eb3True == pHM->GetpCV()->GetCVSelectedAsaPackage() )
				{
					strValveName += TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
				}

				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatComboList( CD_Circuit_CVName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Circuit_CVName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					if( NULL != pHM->GetpCV()->GetCvIDPtr().MP )
					{
						ComboBoxSendMessage( CD_Circuit_CVName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( strValveName.GetBuffer() ) );
						ComboBoxSendMessage( CD_Circuit_CVName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					}

					SetCellParam( CD_Circuit_CVName, lCurRow, (LPARAM)pHM->GetpCV() );
				}
				else
				{
					if( NULL != pHM->GetpCV()->GetCvIDPtr().MP )
					{
						_SetStaticText( CD_Circuit_CVName, lCurRow, strValveName );
					}
				}

				if( true == pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
				{
					SetFontBold( CD_Circuit_CVName, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Circuit_CVName, lCurRow, FALSE );
				}

				// Verify the current flow is not higher than the max flow allowed by the PiCv.
				// If it is the case put the Flow and the PiCv in red.
				if( CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( pHM->GetpCV()->GetValidtyFlags() & CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) )
				{
					SetForeColor( CD_Circuit_CVName, lCurRow, m_rFillColors.m_ErrorFore );
				}

				// HYS-1759: Check de Dpmax of the valve.
				if( CDS_HydroMod::eValidityFlags::evfMaxDpTooHigh == ( pHM->GetpCV()->GetValidtyFlags() & CDS_HydroMod::eValidityFlags::evfMaxDpTooHigh ) )
				{
					SetForeColor( CD_Circuit_CVName, lCurRow, m_rFillColors.m_ErrorFore );
				}

				// Kv and Kvmax are displayed only when CV is not presetable.
				if( false == pHM->GetpCV()->IsPresettable() && NULL != pHM->GetpCV()->GetCvIDPtr().MP )
				{
					// Kvs max.
					FormatCUDouble( CD_Circuit_CVKvsMax, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvsmax() );

					// Kvs.
					FormatCUDouble( CD_Circuit_CVKvs, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs() );
				}
			}
			else
			{
				// Description.
				if( pHM != m_pHM && false == m_bPrinting && false == m_bExporting )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
				}

				bCVDescription &= !pHM->GetpCV()->GetDescription().IsEmpty();
				FormatEditText( CD_Circuit_CVDesc, lCurRow, pHM->GetpCV()->GetDescription() );

				// Kvs max.
				if( pHM != m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
				}

				FormatCUDouble( CD_Circuit_CVKvsMax, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvsmax() );

				// Kvs.
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatEditCombo( CD_Circuit_CVKvs, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, (LPARAM)pHM->GetpCV() );
				}
				else
				{
					FormatCUDouble( CD_Circuit_CVKvs, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, true, false );
				}

				if( true == pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
				{
					SetFontBold( CD_Circuit_CVKvs, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Circuit_CVKvs, lCurRow, FALSE );
				}
			}

			// CV setting.
			if( CDS_HydroMod::eValidityFlags::evfFlowTooHigh != ( pHM->GetpCV()->GetValidtyFlags() & CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) )
			{
				bool fFullSetting = false;
				CString strSetting = pHM->GetpCV()->GetSettingStr( false, &fFullSetting );

				if( true == fFullSetting )
				{
					pclSheetDescription->WriteTextWithFlags( strSetting, CD_Circuit_CVSet, lCurRow, CSheetDescription::RemarkFlags::FullOpening );
				}
				else
				{
					_SetStaticText( CD_Circuit_CVSet, lCurRow, strSetting );
				}

				if( eb3False == pHM->GetpCV()->CheckMinOpen() )
				{
					SetForeColor( CD_Circuit_CVSet, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_Circuit_CVSet, lCurRow, m_rFillColors.m_NormalFore );
				}

				// Dp or Dp min depending the Cv is a PiCv or not.
				if( eb3False == pHM->GetpCV()->IsPICV() && eb3False == pHM->GetpCV()->IsDpCBCV() )
				{
					// Dp.
					FormatCUDouble( CD_Circuit_CVDp, lCurRow, _U_DIFFPRESS, pHM->GetpCV()->GetDp() );

					if( eb3False == pHM->GetpCV()->CheckDpMinMax() )
					{
						SetForeColor( CD_Circuit_CVDp, lCurRow, m_rFillColors.m_ErrorFore );
					}
					else
					{
						SetForeColor( CD_Circuit_CVDp, lCurRow, m_rFillColors.m_NormalFore );
					}

					// Dp Min only for BCV.
					if( true == pHM->GetpCV()->IsPresettable() )
					{
						FormatCUDouble( CD_Circuit_CVDpMin, lCurRow, _U_DIFFPRESS, pHM->GetpCV()->GetDpMin( false ) );
					}
				}
				else
				{
					// Dp Min.
					FormatCUDouble( CD_Circuit_CVDpMin, lCurRow, _U_DIFFPRESS, pHM->GetpCV()->GetDpMin() );
				}
			}
			else
			{
				_SetStaticText( CD_Circuit_CVSet, lCurRow, _T("-") );
				_SetStaticText( CD_Circuit_CVDpMin, lCurRow, _T("-") );
			}

			if( false == pHM->GetpCV()->IsOn_Off() )
			{
				// Authority.
				// Set a "-" for PiCv
				if( eb3True == pHM->GetpCV()->IsPICV() || eb3True == pHM->GetpCV()->IsDpCBCV() )
				{
					_SetStaticText( CD_Circuit_CVAuth, lCurRow, L"-" );
					_SetStaticText( CD_Circuit_CVDesignAuth, lCurRow, L"-" );
				}
				else
				{
					FormatCUDouble( CD_Circuit_CVAuth, lCurRow, _U_NODIM, pHM->GetpCV()->GetAuth() );

					if( eb3False == pHM->GetpCV()->CheckMinAuthor() )
					{
						SetForeColor( CD_Circuit_CVAuth, lCurRow, m_rFillColors.m_ErrorFore );
					}
					else
					{
						SetForeColor( CD_Circuit_CVAuth, lCurRow, m_rFillColors.m_NormalFore );
					}

					// Design authority.
					FormatCUDouble( CD_Circuit_CVDesignAuth, lCurRow, _U_NODIM, pHM->GetpCV()->GetAuth( true ) );

					if( eb3False == pHM->GetpCV()->CheckMinAuthor( true ) )
					{
						SetForeColor( CD_Circuit_CVDesignAuth, lCurRow, m_rFillColors.m_ErrorFore );
					}
					else
					{
						SetForeColor( CD_Circuit_CVDesignAuth, lCurRow, m_rFillColors.m_NormalFore );
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Actuator group
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// 2016-10-28: We want here only IMI valve (not valve defines by their Kvs). This is why the second condition.
		if( true == pHM->IsCvExist( true ) && true == pHM->GetpCV()->IsTaCV() && eb3True == pHM->GetpCV()->IsCVLocInPrimary() )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			// Name.
			CDB_ElectroActuator *pclElecActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)( pHM->GetpCV()->GetActrIDPtr().MP ) );
			CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)( pHM->GetpCV()->GetActrIDPtr().MP ) );
			
			if( NULL != pclActuator )
			{
				// An actuator has been selected by user...
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatComboList( CD_Circuit_ActuatorName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Circuit_ActuatorName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );
					ComboBoxSendMessage( CD_Circuit_ActuatorName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR )( pclActuator->GetName() ) );
					ComboBoxSendMessage( CD_Circuit_ActuatorName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					SetCellParam( CD_Circuit_ActuatorName, lCurRow, (LPARAM)pHM->GetpCV() );
					
					// HYS-1759: Do not check the close-of Dp for PIBCV.
					CDB_CloseOffChar* pCloseOffChar = static_cast<CDB_CloseOffChar*>(pHM->GetpCV()->GetpCV()->GetCloseOffCharIDPtr().MP);
					if( eb3True == pHM->GetpCV()->IsPICV() && NULL != pCloseOffChar && pCloseOffChar->GetLimitType() == CDB_CloseOffChar::CloseOffDp )
					{
						// Do not test closeoffDp because the control part never go above Dp max (see CV_name section).
						SetForeColor( CD_Circuit_ActuatorName, lCurRow, m_rFillColors.m_NormalFore );
					}
					// HYS-485 : We display the actuator with error color because it is not enought strong
					else if( false == pHM->GetpCV()->IsActuatorStrongEnough() )
					{
						// HYS-1685: Case of KTM 512
						if( NULL != pCloseOffChar && ( ( pCloseOffChar->GetLimitType() == CDB_CloseOffChar::CloseOffDp )
							|| ( ( pCloseOffChar->GetLimitType() == CDB_CloseOffChar::InletPressure ) 
							&& ( pCloseOffChar->GetMaxInletPressure( pclActuator->GetMaxForceTorque() ) < 500000 ) ) ) )
						{
							SetForeColor( CD_Circuit_ActuatorName, lCurRow, m_rFillColors.m_ErrorFore );
						}
					}
					else
					{
						SetForeColor( CD_Circuit_ActuatorName, lCurRow, m_rFillColors.m_NormalFore );
					}
				}
				else
				{
					_SetStaticText( CD_Circuit_ActuatorName, lCurRow, pclActuator->GetName() );
				}
			}
			else
			{
				// Remark: No actuator has been selected but the control valve has actuator group. In that case, user must be able to choose
				// from a combo.
				if( NULL != pHM->GetpCV()->GetpCV()->GetActuatorGroupIDPtr().MP )
				{
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
					{
						FormatComboList( CD_Circuit_ActuatorName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						ComboBoxSendMessage( CD_Circuit_ActuatorName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );
						ComboBoxSendMessage( CD_Circuit_ActuatorName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR )TASApp.LoadLocalizedString( IDS_NOACTUATOR ) );
						ComboBoxSendMessage( CD_Circuit_ActuatorName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
						SetCellParam( CD_Circuit_ActuatorName, lCurRow, (LPARAM)pHM->GetpCV() );
					}
					else
					{
						_SetStaticText( CD_Circuit_ActuatorName, lCurRow, TASApp.LoadLocalizedString( IDS_NOACTUATOR ) );
					}
				}
				else
				{
					// No actuator selected and no available actuator for the control valve.
					_SetStaticText( CD_Circuit_ActuatorName, lCurRow, TASApp.LoadLocalizedString( IDS_NOACTUATOR ) );
				}
			}

			// If control valve is locked, we consider that actuator is also automatically locked.
			if( true == pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
			{
				SetFontBold( CD_Circuit_ActuatorName, lCurRow, TRUE );
			}
			else
			{
				SetFontBold( CD_Circuit_ActuatorName, lCurRow, FALSE );
			}

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			// Power supply.
			_SetStaticText( CD_Circuit_ActuatorPowerSupply, lCurRow, ( pclElecActuator != NULL ) ? pclElecActuator->GetPowerSupplyStr() : _T( "" ) );

			// Input signal.
			_SetStaticText( CD_Circuit_ActuatorInputSignal, lCurRow, ( pclElecActuator != NULL ) ? pclElecActuator->GetInOutSignalsStr( true ) : _T( "" ) );

			// Output signal.
			_SetStaticText( CD_Circuit_ActuatorOutputSignal, lCurRow, ( pclElecActuator != NULL ) ? pclElecActuator->GetInOutSignalsStr( false ) : _T( "" ) );

			// Relay type.
			_SetStaticText( CD_Circuit_ActuatorRelayType, lCurRow, ( pclElecActuator != NULL ) ? pclElecActuator->GetRelayStr() : _T( "" ) );

			if( pclElecActuator != NULL )
			{
				// Fail safe.
				if( pclElecActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
				{
					str = TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
				}
				else if( pclElecActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
				{
					str = TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_NO );
				}
				_SetStaticText( CD_Circuit_ActuatorFailSafe, lCurRow, str );
			}
			// Default return position.
			_SetStaticText( CD_Circuit_ActuatorDRP, lCurRow, ( pclElecActuator != NULL ) ? pclElecActuator->GetDefaultReturnPosStr( pclElecActuator->GetDefaultReturnPos() ).c_str() : _T( "" ) );
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Smart valve' group
		// Remark: In this group we can have both smart control valve and smart differential pressure controller.
		//         Common columns: CD_Circuit_SmartValveName, CD_Circuit_SmartValveDp, CD_Circuit_SmartValveLocation and CD_Circuit_SmartValveKvs.
		//         Additional columns for smart control valve only: CD_Circuit_SmartValveControlMode and CD_Circuit_SmartValveMoreInfo.
		//         Additional columns for smart differential pressure controller only: CD_Circuit_SmartValveDpl and CD_Circuit_SmartValveProductSet.
		////////////////////////////////////////////////////////////////////////////////////////////////////
	    // HYS-1676: Add smart valve to Sheet HM Calc

		if( true == pHM->IsSmartControlValveExist( true ) || true == pHM->IsSmartDpCExist() )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			CDB_SmartControlValve *pclSmartValve = NULL;
			CString strValveName = _T("");
			SmartValveType eSmartValveType = SmartValveType::SmartValveTypeUndefined;
			bool bIsLocked = false;
			double dDp = 0.0;
			CDB_DpSensor *pclDpSensor = NULL;
			double dDpMin = 0.0;
			SmartValveLocalization eSmartValveLocalization = SmartValveLocalization::SmartValveLocNone;
			SmartValveLocalization eSelectedSmartValveLocalization = SmartValveLocalization::SmartValveLocNone;
			LPARAM lpParam = NULL;
			int iValidity = 0;
			
			if( true == pHM->IsSmartControlValveExist( true ) )
			{
				pclSmartValve = (CDB_SmartControlValve *)( pHM->GetpSmartControlValve()->GetIDPtr().MP );
				eSmartValveType = pclSmartValve->GetSmartValveType();
				strValveName = pclSmartValve->GetName();
				bIsLocked = pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartControlValve );
				dDp = pHM->GetpSmartControlValve()->GetDp();
				dDpMin = pHM->GetpSmartControlValve()->GetDpMin();
				eSmartValveLocalization = pHM->GetpSmartControlValve()->GetLocalization();
				eSelectedSmartValveLocalization = pHM->GetpSmartControlValve()->GetSelLocalization();
				lpParam = (LPARAM)pHM->GetpSmartControlValve();
				iValidity = pHM->GetpSmartControlValve()->CheckValidity();
			}
			else if( true == pHM->IsSmartDpCExist( true ) )
			{
				pclSmartValve = (CDB_SmartControlValve *)( pHM->GetpSmartDpC()->GetIDPtr().MP );
				eSmartValveType = pclSmartValve->GetSmartValveType();
				strValveName = pclSmartValve->GetName();
				bIsLocked = pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC );
				dDp = pHM->GetpSmartDpC()->GetDp();
				pclDpSensor = pHM->GetpSmartDpC()->GetpDpSensor();
				dDpMin = pHM->GetpSmartDpC()->GetDpMin();
				eSmartValveLocalization = pHM->GetpSmartDpC()->GetLocalization();
				eSelectedSmartValveLocalization = pHM->GetpSmartDpC()->GetSelLocalization();
				lpParam = (LPARAM)pHM->GetpSmartDpC();
				iValidity = pHM->GetpSmartDpC()->CheckValidity();
			}
			else
			{
				// Not yet valve selected.
			}
				
			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				FormatComboList( CD_Circuit_SmartValveName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				ComboBoxSendMessage( CD_Circuit_SmartValveName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

				ComboBoxSendMessage( CD_Circuit_SmartValveName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( strValveName.GetBuffer() ) );
				ComboBoxSendMessage( CD_Circuit_SmartValveName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );

				SetCellParam( CD_Circuit_SmartValveName, lCurRow, lpParam );
			}
			else
			{
				_SetStaticText( CD_Circuit_SmartValveName, lCurRow, strValveName );
			}

			bool bNameAlreadyInRed = false;

			// If the product is deleted, we set the name in red.
			if( NULL != pclSmartValve )
			{
				if( true == pclSmartValve->GetIDPtr().MP->IsDeleted() )
				{
					SetForeColor( CD_Circuit_SmartValveName, lCurRow, m_rFillColors.m_ErrorFore );
					bNameAlreadyInRed = true;
				}
				else
				{
					SetForeColor( CD_Circuit_SmartValveName, lCurRow, _BLACK );
				}
			}

			// Check if there is a least on error.
			if( NULL != pclSmartValve )
			{
				if( false == bNameAlreadyInRed )
				{
					if( SmartValveType::SmartValveTypeControl == eSmartValveType )
					{
						if( CDS_HydroMod::eValidityFlags::evfDpTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooHigh ) 
							|| CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooHigh )
							|| CDS_HydroMod::eValidityFlags::evfFlowTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooLow )
							|| CDS_HydroMod::eValidityFlags::evfPowerTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooHigh )
							|| CDS_HydroMod::eValidityFlags::evfPowerTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooLow )
							|| CDS_HydroMod::eValidityFlags::evfTempTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooHigh ) 
							|| CDS_HydroMod::eValidityFlags::evfTempTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooLow ) )
						{
							SetForeColor( CD_Circuit_SmartValveName, lCurRow, m_rFillColors.m_ErrorFore );
						}
					}
					else
					{
						if( CDS_HydroMod::eValidityFlags::evfDpTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooHigh ) 
							|| CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooHigh )
							|| CDS_HydroMod::eValidityFlags::evfFlowTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooLow )
							|| CDS_HydroMod::eValidityFlags::evfTempTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooHigh ) 
							|| CDS_HydroMod::eValidityFlags::evfTempTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooLow ) )
						{
							SetForeColor( CD_Circuit_SmartValveName, lCurRow, m_rFillColors.m_ErrorFore );
						}
					}
				}
			}

			// If product is locked, we set the name in bold.
			if( NULL != pclSmartValve )
			{
				if( true == bIsLocked )
				{
					SetFontBold( CD_Circuit_SmartValveName, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Circuit_SmartValveName, lCurRow, FALSE );
				}
			}

			// Dp.
			if( NULL != pclSmartValve )
			{
				FormatCUDouble( CD_Circuit_SmartValveDp, lCurRow, _U_DIFFPRESS, dDp );

				// Check if there is error with the pressure drop accross the valve.
				if( CDS_HydroMod::eValidityFlags::evfDpTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooHigh ) )
				{
					SetForeColor( CD_Circuit_SmartValveDp, lCurRow, m_rFillColors.m_ErrorFore );
				}
			}
			else
			{
				_SetStaticText( CD_Circuit_SmartValveDp, lCurRow, GetDashDotDash() );
			}

			// Dp min.
			if( NULL != pclSmartValve )
			{
				FormatCUDouble( CD_Circuit_SmartValveDpMin, lCurRow, _U_DIFFPRESS, dDpMin );
			}
			else
			{
				_SetStaticText( CD_Circuit_SmartValveDpMin, lCurRow, GetDashDotDash() );
			}

			// Control mode (Only for smart control valve).
			if( NULL != pclSmartValve && SmartValveType::SmartValveTypeControl == eSmartValveType )
			{
				// We add the last "bIsLocked" because we don't want change the control mode if the smart differential pressure controller is locked.
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting && false == bIsLocked )
				{
					FormatComboList( CD_Circuit_SmartValveControlMode, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Circuit_SmartValveControlMode, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					str = pclSmartValve->GetControlModeString( SmartValveControlMode::SCVCM_Flow );
					ComboBoxSendMessage( CD_Circuit_SmartValveControlMode, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

					str = pclSmartValve->GetControlModeString( SmartValveControlMode::SCVCM_Power );
					ComboBoxSendMessage( CD_Circuit_SmartValveControlMode, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

					int iCurrentSel = 0;

					if( SmartValveControlMode::SCVCM_Flow == pHM->GetpSmartControlValve()->GetControlMode() )
					{
						iCurrentSel = 0;
					}
					else if( SmartValveControlMode::SCVCM_Power == pHM->GetpSmartControlValve()->GetControlMode() )
					{
						iCurrentSel = 1;
					}

					ComboBoxSendMessage( CD_Circuit_SmartValveControlMode, lCurRow, SS_CBM_SETCURSEL, iCurrentSel, 0 );
					SetCellParam( CD_Circuit_SmartValveControlMode, lCurRow, (LPARAM)pHM->GetpSmartControlValve() );
				}
				else
				{
					_SetStaticText( CD_Circuit_SmartValveControlMode, lCurRow, pclSmartValve->GetControlModeString( pHM->GetpSmartControlValve()->GetControlMode() ) );
				}

				if( true == bIsLocked )
				{
					SetFontBold( CD_Circuit_SmartValveControlMode, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Circuit_SmartValveControlMode, lCurRow, FALSE );
				}
			}
			else
			{
				_SetStaticText( CD_Circuit_SmartValveControlMode, lCurRow, _T("") );
			}

			// Location.
			if( NULL != pclSmartValve )
			{
				// We add the last "bIsLocked" because we don't want change the location if the smart differential pressure controller is locked.
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting && false == bIsLocked )
				{
					FormatComboList( CD_Circuit_SmartValveLocation, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Circuit_SmartValveLocation, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					str = pclSmartValve->GetLocalizationString( SmartValveLocalization::SmartValveLocSupply );
					ComboBoxSendMessage( CD_Circuit_SmartValveLocation, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

					str = pclSmartValve->GetLocalizationString( SmartValveLocalization::SmartValveLocReturn );
					ComboBoxSendMessage( CD_Circuit_SmartValveLocation, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

					int iCurrentSel = ( SmartValveLocalization::SmartValveLocSupply == eSmartValveLocalization ) ? 0 : 1;

					if( SmartValveLocalization::SmartValveLocNone != eSelectedSmartValveLocalization )
					{
						iCurrentSel = ( SmartValveLocalization::SmartValveLocSupply == eSelectedSmartValveLocalization ) ? 0 : 1;
					}

					ComboBoxSendMessage( CD_Circuit_SmartValveLocation, lCurRow, SS_CBM_SETCURSEL, iCurrentSel, 0 );
					SetCellParam( CD_Circuit_SmartValveLocation, lCurRow, (LPARAM)pHM->GetpSmartControlValve() );
				}
				else if( NULL != pclSmartValve )
				{
					SmartValveLocalization eToWrite = ( SmartValveLocalization::SmartValveLocNone != eSelectedSmartValveLocalization ) ? eSelectedSmartValveLocalization : eSmartValveLocalization;
					_SetStaticText( CD_Circuit_SmartValveLocation, lCurRow, pclSmartValve->GetLocalizationString( eToWrite ) );
				}

				if( true == bIsLocked )
				{
					SetFontBold( CD_Circuit_SmartValveLocation, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Circuit_SmartValveLocation, lCurRow, FALSE );
				}
			}
			else
			{
				_SetStaticText( CD_Circuit_SmartValveLocation, lCurRow, _T("") );
			}

			// Kvs.
			if( NULL != pclSmartValve )
			{
				FormatCUDouble( CD_Circuit_SmartValveKvs, lCurRow, _C_KVCVCOEFF, pclSmartValve->GetKvs() );
			}
			else
			{
				_SetStaticText( CD_Circuit_SmartValveKvs, lCurRow, GetDashDotDash() );
			}

			// More information column (Only for smart control valve).
			if( NULL != pclSmartValve && SmartValveType::SmartValveTypeControl == pclSmartValve->GetSmartValveType() ) 
			{
				if( false == m_bPrinting && false == m_bExporting )
				{
					SetPictureCellWithID( IDI_SMV_MOREINFO, CD_Circuit_SmartValveMoreInfo, lCurRow, CSSheet::PictureCellType::Icon );
				}
			}

			if( NULL != pclSmartValve && SmartValveType::SmartValveTypeDpC == eSmartValveType )
			{
				
			
				// Selection by set.

				// We add the last "bIsLocked" because we don't want change the Dp sensor set/connection set if the smart differential pressure controller is locked.
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting && false == bIsLocked )
				{
					CTable *pclTable = dynamic_cast<CTable *>( pclSmartValve->GetDpSensorGroupIDPtr().MP );

					int iCurrentSelected = 0;
					int iLoop = 0;
					IDPTR ProductSetSelectedIDPtr = _NULL_IDPTR;
					IDPTR ProductSetInHMIDPtr = pHM->GetpSmartDpC()->GetProductSetIDPtr();

					_FormatComboList( SheetDescription::SCircuit, CD_Circuit_SmartValveProductSet, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Circuit_SmartValveProductSet, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					std::vector<CDB_Product *> vecAllProductSets;
					CRank clAllSets;

					if( NULL != pclTable )
					{
						for( IDPTR IDPtr = pclTable->GetFirst(); *IDPtr.ID; IDPtr = pclTable->GetNext() )
						{
							CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>(IDPtr.MP );

							double dKey = (double)pclProductSet->GetSortingKey();

							if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
							{
								CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclProductSet );

								ASSERT( ( pclDpSensor->GetMinMeasurableDp() / 1000.0 ) < 100.0 );
								dKey = dKey * 100.0 + ( pclDpSensor->GetMinMeasurableDp() / 1000.0 );
							}
							else
							{
								dKey += 1e9;
							}

							clAllSets.Add( pclProductSet->GetName(), dKey, (LPARAM)(IDPtr.MP) );
						}
					}

					CString str;
					LPARAM lpItemData = 0;

					for( BOOL bContinue = clAllSets.GetFirst( str, lpItemData ); TRUE == bContinue; bContinue = clAllSets.GetNext( str, lpItemData ) ) 
					{
						CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>( (CData *)lpItemData );
						CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclProductSet );

						CString strName = pclProductSet->GetName();

						if( NULL != pclDpSensor )
						{
							strName = pclDpSensor->GetFullName();
						}

						ComboBoxSendMessage( CD_Circuit_SmartValveProductSet, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)strName );

						// Save the pointer of the product set object.
						_AddComboBoxParam( SheetDescription::SCircuit, CD_Circuit_SmartValveProductSet, lCurRow, (LPARAM)( pclProductSet ) );

						if( _NULL_IDPTR == ProductSetSelectedIDPtr )
						{
							// Be default we take the first CD_Circuit_SmartValveProductSet available in the combo.
							ProductSetSelectedIDPtr = pclProductSet->GetIDPtr();
						}

						if( pclProductSet == ProductSetInHMIDPtr.MP )
						{
							// If user has already selected a product set, we take this one.
							ProductSetSelectedIDPtr = ProductSetInHMIDPtr;
							iCurrentSelected = iLoop;
						}

						iLoop++;
					}

					ComboBoxSendMessage( CD_Circuit_SmartValveProductSet, lCurRow, SS_CBM_SETCURSEL, iCurrentSelected, 0 );

					// Remark: even if we don't use cell param to retrieve corresponding product set, we set the current product set to allow 
					//         the drag & drop operation work normally.
					SetCellParam( CD_Circuit_SmartValveProductSet, lCurRow, lpParam );
				}
				else
				{
					if( NULL != pHM->GetpSmartDpC()->GetpProductSet() )
					{
						CString strName = pHM->GetpSmartDpC()->GetpProductSet()->GetName();

						if( NULL != dynamic_cast<CDB_DpSensor *>( pHM->GetpSmartDpC()->GetpProductSet() ) )
						{
							CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pHM->GetpSmartDpC()->GetpProductSet() );
							strName = pclDpSensor->GetFullName();
						}

						_SetStaticText( CD_Circuit_SmartValveProductSet, lCurRow, strName );
					}
					else
					{
						_SetStaticText( CD_Circuit_SmartValveProductSet, lCurRow, _T("") );
					}

					if( CDS_HydroMod::eValidityFlags::evfDplTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfDplTooLow )
							|| CDS_HydroMod::eValidityFlags::evfDplTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDplTooHigh ) )
					{
						SetForeColor( CD_Circuit_SmartValveProductSet, lCurRow, m_rFillColors.m_ErrorFore );
					}
				}

				// With TSpread and for a native combo box it is not possible to put text color of one row in red and the other rows in black.
				// So, we use 'OnPaint' to refresh the text color of the Dp sensor combo (See the "_VerifyComboBoxError" method).
				// For the other combo, we don't really use a combobox from TSpread but instead we use the "DlgComboBoxHM" when opening.

				if( true == bIsLocked )
				{
					SetFontBold( CD_Circuit_SmartValveProductSet, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Circuit_SmartValveProductSet, lCurRow, FALSE );
				}
			}
			else
			{
				_SetStaticText( CD_Circuit_SmartValveProductSet, lCurRow, _T("") );
			}

			// Dpl (Only for smart diffential pressure controller).
			if( NULL != pclSmartValve && SmartValveType::SmartValveTypeDpC == eSmartValveType )
			{
				double dDpToStabilize = pHM->GetpSmartDpC()->GetDpToStabilize();

				if( dDpToStabilize > 0.0 )
				{
					FormatCUDouble( CD_Circuit_SmartValveDpl, lCurRow, _U_DIFFPRESS, dDpToStabilize );

					// Verify if we need to put 'Dpl' column in red.
					if( CDS_HydroMod::eValidityFlags::evfDplTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfDplTooLow )
							|| CDS_HydroMod::eValidityFlags::evfDplTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDplTooHigh ) )
					{
						SetForeColor( CD_Circuit_SmartValveDpl, lCurRow, m_rFillColors.m_ErrorFore );
					}
				}
				else
				{
					_SetStaticText( CD_Circuit_SmartValveDpl, lCurRow, GetDashDotDash() );
				}
			}
			else
			{
				_SetStaticText( CD_Circuit_SmartValveDpl, lCurRow, GetDashDotDash() );
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Balancing valve' group
		////////////////////////////////////////////////////////////////////////////////////////////////////
		if( true == pHM->IsBvExist( true ) )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			// Name.
			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				FormatComboList( CD_Circuit_BvName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				ComboBoxSendMessage( CD_Circuit_BvName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

				if( NULL != pHM->GetpBv()->GetIDPtr().MP )
				{
					ComboBoxSendMessage( CD_Circuit_BvName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( ( (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP ) )->GetName() ) );
					ComboBoxSendMessage( CD_Circuit_BvName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
				}

				SetCellParam( CD_Circuit_BvName, lCurRow, (LPARAM)pHM->GetpBv() );
			}
			else
			{
				if( NULL != pHM->GetpBv()->GetIDPtr().MP )
				{
					_SetStaticText( CD_Circuit_BvName, lCurRow, ( (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP ) )->GetName() );
				}
			}

			if( true == pHM->IsLocked( pHM->GetpBv()->GetHMObjectType() ) )
			{
				SetFontBold( CD_Circuit_BvName, lCurRow, TRUE );
			}
			else
			{
				SetFontBold( CD_Circuit_BvName, lCurRow, FALSE );
			}

			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			// Setting.
			bool bFullSetting = false;
			CString strSetting = pHM->GetpBv()->GetSettingStr( false, &bFullSetting );

			if( true == bFullSetting )
			{
				pclSheetDescription->WriteTextWithFlags( strSetting, CD_Circuit_BvSet, lCurRow, CSheetDescription::RemarkFlags::FullOpening );
			}
			else
			{
				_SetStaticText( CD_Circuit_BvSet, lCurRow, strSetting );
			}

			if( eb3False == pHM->GetpBv()->CheckMinOpen() )
			{
				SetForeColor( CD_Circuit_BvSet, lCurRow, m_rFillColors.m_ErrorFore );
			}
			else
			{
				SetForeColor( CD_Circuit_BvSet, lCurRow, m_rFillColors.m_NormalFore );
			}

			// Dp.
			FormatCUDouble( CD_Circuit_BvDp, lCurRow, _U_DIFFPRESS, pHM->GetpBv()->GetDp() );

			if( eb3False == pHM->GetpBv()->CheckDpMinMax() )
			{
				SetForeColor( CD_Circuit_BvDp, lCurRow, m_rFillColors.m_ErrorFore );
			}
			else
			{
				SetForeColor( CD_Circuit_BvDp, lCurRow, m_rFillColors.m_NormalFore );
			}

			// Dp signal.
			FormatCUDouble( CD_Circuit_BvDpSig, lCurRow, _U_DIFFPRESS, pHM->GetpBv()->GetDpSignal() );

			if( eb3False == pHM->GetpBv()->CheckDpMinMax() )
			{
				SetForeColor( CD_Circuit_BvDpSig, lCurRow, m_rFillColors.m_ErrorFore );
			}
			else
			{
				SetForeColor( CD_Circuit_BvDpSig, lCurRow, m_rFillColors.m_NormalFore );
			}

			// Add a test to know if all BV have a DpSig, do not show the Dp column.
			iNbrBv++;

			if( 0 != pHM->GetpBv()->GetDpSignal() )
			{
				iNbrBvDpSig++;
			}

			// Dp 100%.
			FormatCUDouble( CD_Circuit_BvDpMin, lCurRow, _U_DIFFPRESS, pHM->GetpBv()->GetDpMin( false ) );
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Shut-off valve' group
		////////////////////////////////////////////////////////////////////////////////////////////////////

		CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

		if( NULL != pHM->GetpSch() )
		{
			if( ShutoffValveLoc::ShutoffValveLocSupply == pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveSupply;
			}
			else if( ShutoffValveLoc::ShutoffValveLocReturn == pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveReturn;
			}
		}

		// Shutoff valve checkBox only for 2way.
		if( CDS_HydroMod::eNone != eHMObj && false == bFreezed && false == m_bPrinting && false == m_bExporting && NULL != dynamic_cast<CDS_Hm2W *>( pHM ) )
		{
			_DrawCheckbox( CD_Circuit_SvPict, lCurRow, ( (CDS_Hm2W *)pHM )->GetUseShutoffValve() );
		}

		if( true == pHM->IsShutoffValveExist( eHMObj, true ) )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			// Name.
			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				FormatComboList( CD_Circuit_SvName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				ComboBoxSendMessage( CD_Circuit_SvName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

				if( NULL != pHM->GetpShutoffValve( eHMObj )->GetIDPtr().MP )
				{
					ComboBoxSendMessage( CD_Circuit_SvName, lCurRow, SS_CBM_ADDSTRING, 0,
							(LPARAM)( ( (CDB_TAProduct *)( pHM->GetpShutoffValve( eHMObj )->GetIDPtr().MP ) )->GetName() ) );

					ComboBoxSendMessage( CD_Circuit_SvName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
				}

				SetCellParam( CD_Circuit_SvName, lCurRow, (LPARAM)pHM->GetpShutoffValve( eHMObj ) );
			}
			else
			{
				if( NULL != pHM->GetpShutoffValve( eHMObj )->GetIDPtr().MP )
				{
					_SetStaticText( CD_Circuit_SvName, lCurRow, ( (CDB_TAProduct *)( pHM->GetpShutoffValve( eHMObj )->GetIDPtr().MP ) )->GetName() );
				}
			}

			if( false == pHM->IsLocked( eHMObj ) && false == bFreezed )
			{
				SetFontBold( CD_Circuit_SvName, lCurRow, FALSE );
			}
			else
			{
				SetFontBold( CD_Circuit_SvName, lCurRow, TRUE );
			}

			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			// Dp.
			FormatCUDouble( CD_Circuit_SvDp, lCurRow, _U_DIFFPRESS, pHM->GetpShutoffValve( eHMObj )->GetDp() );

			if( eb3False == pHM->GetpShutoffValve( eHMObj )->CheckDpMinMax() )
			{
				SetForeColor( CD_Circuit_SvDp, lCurRow, m_rFillColors.m_ErrorFore );
			}
			else
			{
				SetForeColor( CD_Circuit_SvDp, lCurRow, m_rFillColors.m_NormalFore );
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'DpC' group
		////////////////////////////////////////////////////////////////////////////////////////////////////

		if( true == pHM->IsDpCExist( true ) )
		{
			CDB_DpController *pDpC = ( CDB_DpController * )pHM->GetpDpC()->GetIDPtr().MP;

			if( NULL != pDpC )
			{
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				// Change background color for parent module.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				// Name.
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatComboList( CD_Circuit_DpCName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Circuit_DpCName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );
					ComboBoxSendMessage( CD_Circuit_DpCName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( pDpC->GetName() ) );
					ComboBoxSendMessage( CD_Circuit_DpCName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					SetCellParam( CD_Circuit_DpCName, lCurRow, (LPARAM)pHM->GetpDpC() );
				}
				else
				{
					_SetStaticText( CD_Circuit_DpCName, lCurRow, pDpC->GetName() );
				}

				if( true == pHM->IsLocked( CDS_HydroMod::eHMObj::eDpC ) )
				{
					SetFontBold( CD_Circuit_DpCName, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Circuit_DpCName, lCurRow, FALSE );
				}

				if( ( CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( pHM->GetpDpC()->GetValidtyFlags() & CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) ) ||
					( CDS_HydroMod::eValidityFlags::evfFlowTooLow == ( pHM->GetpDpC()->GetValidtyFlags() & CDS_HydroMod::eValidityFlags::evfFlowTooLow ) ) )
				{
					SetForeColor( CD_Circuit_DpCName, lCurRow, m_rFillColors.m_ErrorFore );
				}

				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				// Change background color for parent module.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				// Dpl range.
				CString str = pDpC->GetFormatedDplRange( false ).c_str();
				_SetStaticText( CD_Circuit_DpCDpLr, lCurRow, str );

				// Setting.
				bool bFullSetting = false;
				CString strSetting = pHM->GetpDpC()->GetSettingStr( false, &bFullSetting );

				if( true == bFullSetting )
				{
					pclSheetDescription->WriteTextWithFlags( strSetting, CD_Circuit_DpCSet, lCurRow, CSheetDescription::RemarkFlags::FullOpening );
				}
				else
				{
					FormatStaticText( CD_Circuit_DpCSet, lCurRow, CD_Circuit_DpCSet, lCurRow, strSetting, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				}

				// Dp: here we call 'FormatCUDouble' (remark or not) to have the cell well formated.
				FormatCUDouble( CD_Circuit_DpCDp, lCurRow, _U_DIFFPRESS, pHM->GetpDpC()->GetDp() );

				if( eb3False == pHM->GetpDpC()->CheckDpMinMax() )
				{
					SetForeColor( CD_Circuit_DpCDp, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_Circuit_DpCDp, lCurRow, m_rFillColors.m_NormalFore );
				}

				// DpL.
				FormatCUDouble( CD_Circuit_DpCDpl, lCurRow, _U_DIFFPRESS, pHM->GetpDpC()->GetDpL() );

				if( eb3False == pHM->GetpDpC()->CheckDpLRange() )
				{
					SetForeColor( CD_Circuit_DpCDpl, lCurRow,  m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_Circuit_DpCDpl, lCurRow, m_rFillColors.m_NormalFore );
				}

				// DpMin.
				FormatCUDouble( CD_Circuit_DpCDpMin, lCurRow, _U_DIFFPRESS, pHM->GetpDpC()->GetDpmin( false ) );
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Bypass balancing valve' group
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// Bypass BV checkBox only for 3way.
		if( false == bFreezed && false == m_bPrinting && false == m_bExporting && NULL != dynamic_cast<CDS_Hm3W *>( pHM ) )
		{
			_DrawCheckbox( CD_Circuit_BvBypPict, lCurRow, ( (CDS_Hm3W *)pHM )->GetUseBypBv() );
		}

		// Bypass BV.
		if( true == pHM->IsBvBypExist( true ) )
		{
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			// Name.
			if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
			{
				FormatComboList( CD_Circuit_BvBypName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				ComboBoxSendMessage( CD_Circuit_BvBypName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

				ComboBoxSendMessage( CD_Circuit_BvBypName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( ( (CDB_TAProduct *)( pHM->GetpBypBv()->GetIDPtr().MP ) )->GetName() ) );
				ComboBoxSendMessage( CD_Circuit_BvBypName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );

				SetCellParam( CD_Circuit_BvBypName, lCurRow, (LPARAM)pHM->GetpBypBv() );
			}
			else
			{
				_SetStaticText( CD_Circuit_BvBypName, lCurRow, ( (CDB_TAProduct *)( pHM->GetpBypBv()->GetIDPtr().MP ) )->GetName() );
			}

			if( false == pHM->IsLocked( pHM->GetpBypBv()->GetHMObjectType() ) && false == bFreezed )
			{
				SetFontBold( CD_Circuit_BvBypName, lCurRow, FALSE );
			}
			else
			{
				SetFontBold( CD_Circuit_BvBypName, lCurRow, TRUE );
			}

			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			// Setting.
			bool bFullSetting = false;
			CString strSetting = pHM->GetpBypBv()->GetSettingStr( false, &bFullSetting );

			if( true == bFullSetting )
			{
				pclSheetDescription->WriteTextWithFlags( strSetting, CD_Circuit_BvBypSet, lCurRow, CSheetDescription::RemarkFlags::FullOpening );
			}
			else
			{
				_SetStaticText( CD_Circuit_BvBypSet, lCurRow, strSetting );
			}

			if( eb3False == pHM->GetpBypBv()->CheckMinOpen() )
			{
				SetForeColor( CD_Circuit_BvBypSet, lCurRow, m_rFillColors.m_ErrorFore );
			}
			else
			{
				SetForeColor( CD_Circuit_BvBypSet, lCurRow, m_rFillColors.m_NormalFore );
			}
		}		
		
		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Pipe' group
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// Circuit pipe.
		bool bPipeVisible = true;

		// Circuit pipe doesn't exist for a module.
		if( true == pHM->IsaModule() && NULL != pHM->GetpSchcat() && false == pHM->GetpSchcat()->IsSecondarySideExist() )
		{
			bPipeVisible = false;
		}

		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module.
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		CPipes *pPipe = pHM->GetpCircuitPrimaryPipe();

		if( NULL != pPipe && true == bPipeVisible )
		{
			// Series.
			if( false == m_pPipeCircTab )
			{
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatComboList( CD_Circuit_PipeSerie, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Circuit_PipeSerie, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					// Pipe length exist but size and series are not yet defined, flow is 0 for instance.
					if( NULL != pPipe->GetPipeSeries() )
					{
						ComboBoxSendMessage( CD_Circuit_PipeSerie, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( pPipe->GetPipeSeries()->GetName() ) );
						ComboBoxSendMessage( CD_Circuit_PipeSerie, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					}

					SetCellParam( CD_Circuit_PipeSerie, lCurRow, (LPARAM)pPipe );
				}
				else
				{
					if( NULL != pPipe->GetPipeSeries() )
					{
						_SetStaticText( CD_Circuit_PipeSerie, lCurRow, pPipe->GetPipeSeries()->GetName() );
						SetFontBold( CD_Circuit_PipeSerie, lCurRow, ( true == bFreezed ) ? TRUE : FALSE );
					}
				}
			}

			if( false == pHM->IsaModule() || ( true == pHM->IsaModule() && NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsSecondarySideExist() && 0 != pHM->GetLevel() ) )
			{
				// Size.
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatComboList( CD_Circuit_PipeSize, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_Circuit_PipeSize, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					// Pipe length exist but size is not yet defined, flow is 0 for instance.
					if( NULL != pPipe->GetIDPtr().MP )
					{
						ComboBoxSendMessage( CD_Circuit_PipeSize, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() ) );
						ComboBoxSendMessage( CD_Circuit_PipeSize, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					}

					SetCellParam( CD_Circuit_PipeSize, lCurRow, (LPARAM)pPipe );
				}
				else
				{
					if( NULL != pPipe->GetIDPtr().MP )
					{
						_SetStaticText( CD_Circuit_PipeSize, lCurRow, ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() );
					}
				}

				if( true == pHM->IsLocked( pPipe->GetLocate() ) )
				{
					SetFontBold( CD_Circuit_PipeSize, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_Circuit_PipeSize, lCurRow, FALSE );
				}

				// Length.
				if( pHM != m_pHM && false == m_bPrinting && false == m_bExporting )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatEditDouble( CD_Circuit_PipeL, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, false );
					bAtLeastOneCircuitPipeLength = true;
				}
				else
				{
					FormatCUDouble( CD_Circuit_PipeL, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, true, false );
				}

				SetFontBold( CD_Circuit_PipeL, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

				if( pHM != m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
				}

				// Dp.
				double dSI = pPipe->GetPipeDp();
				FormatCUDouble( CD_Circuit_PipeDp, lCurRow, _U_DIFFPRESS, pPipe->GetPipeDp() );

				// Linear Dp.
				FormatCUDouble( CD_Circuit_PipeLinDp, lCurRow, _U_LINPRESSDROP, pPipe->GetLinDp() );

				if( eb3False == pPipe->CheckLinDpMinMax() )
				{
					SetForeColor( CD_Circuit_PipeLinDp, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_Circuit_PipeLinDp, lCurRow, m_rFillColors.m_NormalFore );
				}

				// Velocity.
				FormatCUDouble( CD_Circuit_PipeV, lCurRow, _U_VELOCITY, pPipe->GetVelocity() );

				if( eb3False == pPipe->CheckVelocityMinMax() )
				{
					SetForeColor( CD_Circuit_PipeV, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_Circuit_PipeV, lCurRow, m_rFillColors.m_NormalFore );
				}
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Circuit pipe accessories' group
		////////////////////////////////////////////////////////////////////////////////////////////////////
		long lLastCircuitPipeSingularityRow = lCurRow;

		// First, test if we are on a root module.
		// Remark: no singularity (connection or/and accessory) on a root module.
		if( NULL != pPipe && ( false == pHM->IsaModule() || pHM->GetLevel() > 0 ) )
		{
			if( true == m_bPrinting )
			{
				// One line by singularity.
				CPipes::CSingularity Singularity;
				int iSingulPos = pPipe->GetFirstSingularity( &Singularity );
				bool bFirstPass = true;

				while( iSingulPos != -1 )
				{
					CString strDescription, strInputData;

					if( true == pPipe->GetSingularityDetails( &Singularity, &strDescription, &strInputData ) )
					{
						if( false == bFirstPass )
						{
							lLastCircuitPipeSingularityRow++;
						}

						// Description.
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
						SetStaticTextEx( CD_Circuit_PipeAccDesc, lLastCircuitPipeSingularityRow, strDescription, true );

						// Input data.
						SetStaticTextEx( CD_Circuit_PipeAccData, lLastCircuitPipeSingularityRow, strInputData, true );

						// Dp.
						CString strDp = WriteCUDouble( _U_DIFFPRESS, pPipe->GetSingularityDp( &Singularity ), false );
						SetStaticTextEx( CD_Circuit_PipeAccDp, lLastCircuitPipeSingularityRow, strDp, true );

						bFirstPass = false;
					}

					iSingulPos = pPipe->GetNextSingularity( iSingulPos, &Singularity );
				}
			}
			else
			{
				// Short description.
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				CString str = pPipe->GetSingularitiesDescriptionShort( true );
				_SetStaticText( CD_Circuit_PipeAccDesc, lCurRow, str );

				// Input data.
				str = pPipe->GetSingularitiesInputDataShort();
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ELLIPSES );
				_SetStaticText( CD_Circuit_PipeAccData, lCurRow, str );

				// Dp.
				FormatCUDouble( CD_Circuit_PipeAccDp, lCurRow, _U_DIFFPRESS, pPipe->GetSingularityTotalDp( true ) );
			}
		}

		// If we are on a module...
		if( pHM == m_pHM )
		{
			// If we are in printing mode and there is more than one singularity in the accessory group...
			if( true == m_bPrinting && lLastCircuitPipeSingularityRow > lCurRow )
			{
				for( long lLoopRow = lCurRow + 1; lLoopRow <= lLastCircuitPipeSingularityRow; lLoopRow++ )
				{
					// Change background color for parent module in the line added.
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );

					for( long lLoopColumn = CD_Circuit_UnitDesc; lLoopColumn <= CD_Circuit_Pointer; lLoopColumn++ )
					{
						switch( lLoopColumn )
						{
							case CD_Circuit_DescSep:
							case CD_Circuit_UnitSep:
							case CD_Circuit_PumpSep:
							case CD_Circuit_CVSep:
							case CD_Circuit_ActuatorSep:
							case CD_Circuit_SmartValveSep:
							case CD_Circuit_BvSep:
							case CD_Circuit_SvSep:
							case CD_Circuit_DpCSep:
							case CD_Circuit_BvBypSep:
							case CD_Circuit_PipeSep:
							case CD_Circuit_PipeAccSep:
								continue;
								break;

							default:
								_SetStaticText( lLoopColumn, lLoopRow, _T( "" ) );
								break;
						}
					}
				}

				lCurRow = lLastCircuitPipeSingularityRow;
			}

			// Add a solid border below the root module.
			SetCellBorder( CD_Circuit_Name, lCurRow, CD_Circuit_DescSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_UnitDesc, lCurRow, CD_Circuit_UnitSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_PumpHUser, lCurRow, CD_Circuit_PumpSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_CVName, lCurRow, CD_Circuit_CVSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_ActuatorName, lCurRow, CD_Circuit_ActuatorSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_SmartValveName, lCurRow, CD_Circuit_SmartValveSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_BvName, lCurRow, CD_Circuit_BvSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_SvPict, lCurRow, CD_Circuit_SvSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_DpCName, lCurRow, CD_Circuit_DpCSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_BvBypPict, lCurRow, CD_Circuit_BvBypSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_PipeSerie, lCurRow, CD_Circuit_PipeSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( CD_Circuit_PipeAccDesc, lCurRow, CD_Circuit_PipeAccSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			for( int c = CD_Circuit_Sub; c < CD_Circuit_Pointer; c++ )
			{
				_SetStaticText( c, lCurRow + 1, _T( "" ) );
			}

			lCurRow += 2;
		}
		else
		{
			lCurRow = lLastCircuitPipeSingularityRow;
			lCurRow++;
		}

		// Add gray border between each product.
		SetCellBorder( CD_Circuit_Name, lCurRow - 1, CD_Circuit_DescSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_UnitDesc, lCurRow - 1, CD_Circuit_UnitSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_PumpHUser, lCurRow, CD_Circuit_PumpSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_CVName, lCurRow - 1, CD_Circuit_CVSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_ActuatorName, lCurRow - 1, CD_Circuit_ActuatorSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_SmartValveName, lCurRow - 1, CD_Circuit_SmartValveSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_BvName, lCurRow - 1, CD_Circuit_BvSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_SvPict, lCurRow - 1, CD_Circuit_SvSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_DpCName, lCurRow - 1, CD_Circuit_DpCSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_BvBypPict, lCurRow - 1, CD_Circuit_BvBypSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_PipeSerie, lCurRow - 1, CD_Circuit_PipeSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_Circuit_PipeAccDesc, lCurRow - 1, CD_Circuit_PipeAccSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
	}

	// Check to show pencil for the pipe length columns.
	if( true == bAtLeastOneCircuitPipeLength )
	{
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_Circuit_PipeL, RD_Header_Picture, CSSheet::PictureCellType::Icon );
	}

	lCurRow--;
	SetCellBorder( CD_Circuit_Name, lCurRow, CD_Circuit_Pointer - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	_HideEmptyColumns();

	// Hide the Dp Column if there is a DpSig for all Bv
	if( iNbrBv > 0 )
	{
		bool fOnlyDpSig = false;

		if( iNbrBv == iNbrBvDpSig )
		{
			fOnlyDpSig = true;
		}

		ShowCol( CD_Circuit_BvDp, ( true == fOnlyDpSig ) ? FALSE : TRUE );
		ShowCol( CD_Circuit_BvDpSig, ( iNbrBvDpSig > 0 ) ? TRUE : FALSE );
	}

	// Check now if we can hide collapse button or not
	VerifyExpandCollapseColumnButtons( CRect( CD_Circuit_Name, RD_Header_FirstCirc, CD_Circuit_PipeAccDp, lCurRow ), RD_Header_ButDlg );

	// Check if last column is a separator
	// Remarks: in this case, we must set 'CellCantBePainted' column property to true to
	// avoid selection to be drawn on this last column.
	CheckLastColumnFlagCantBePainted( RD_Header_ButDlg );

	// Check if there is no successive separators.
	_HideSuccessiveSeparator( CD_Circuit_DescSep, CD_Circuit_PipeAccDp, RD_Header_FirstCirc, SheetDescription::SCircuit );

	// Add Remarks.
	pclSheetDescription->WriteRemarks( ++lCurRow, CD_Circuit_Name, CD_Circuit_Pointer );
}

void CSheetHMCalc::_FillCircuitInjSheet()
{
	if( NULL == m_pHM )
	{
		return;
	}

	long lCurRow = RD_Header_FirstCirc;

	// Sort HM in function of position.
	CRank HMList;
	CString str;
	long lHMRows = lCurRow;

	for( IDPTR IDPtr = m_pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pHM->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)( IDPtr.MP );
		HMList.Add( str, pHM->GetPos(), (LPARAM)IDPtr.MP );
		lHMRows += 1;	// 1 row by circuit

		if( true == m_bPrinting && NULL != pHM->GetpCircuitSecondaryPipe() && pHM->GetpCircuitSecondaryPipe()->GetSingularityCount() > 0 )
		{
			lHMRows += pHM->GetpCircuitSecondaryPipe()->GetSingularityCount() - 1;
		}
	}

	// Parent module at the first position
	HMList.Add( str, 0, (LPARAM)m_pHM->GetIDPtr().MP );
	lHMRows += 1;	// 1 row for parent module

	if( true == m_bPrinting && NULL != m_pHM->GetpCircuitSecondaryPipe() && m_pHM->GetpCircuitSecondaryPipe()->GetSingularityCount() > 0 )
	{
		lHMRows += m_pHM->GetpCircuitSecondaryPipe()->GetSingularityCount() - 1;
	}

	SetMaxRows( lHMRows );

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SCircuitInj );

		if( NULL != pclSheetDescription )
		{
			pclSheetDescription->Init( 1, GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
			pclSheetDescription->SetSelectableRangeRow( lCurRow, GetMaxRows() );
		}
	}

	// Used to hide empty column.
	bool bCVDescription = true;
	CDS_HydroMod *pHM = NULL;
	LPARAM lparam = (LPARAM)0;
	int iNbrBvSec = 0;
	int	iNbrBvSecDpSig = 0;

	// Verify if the project is frozen.
	bool bFreezed = false;
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL != pPrjParam )
	{
		bFreezed = pPrjParam->IsFreezed();
	}

	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		TCHAR tcBuffer[10];
		pHM = (CDS_HydroMod *)lparam;
		ASSERT( lCurRow <= lHMRows );

		// Prepare color to use in regards to the current row is in cut mode or not.
		bool bIsCutMode = _PrepareColors( pHM );

		// If we are on a root module...
		if( pHM == m_pHM )
		{
			// White line.
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );

			for( int lLoopColumn = CD_CircuitInjSecSide_Name; lLoopColumn < CD_CircuitInjSecSide_Pointer; lLoopColumn++ )
			{
				switch( lLoopColumn )
				{
					case CD_CircuitInjSecSide_DescSep:
					case CD_CircuitInjSecSide_Sep:
					case CD_CircuitInjSecSide_PumpSep:
					case CD_CircuitInjSecSide_TempSep:
					case CD_CircuitInjSecSide_CVSep:
					case CD_CircuitInjSecSide_ActuatorSep:
					case CD_CircuitInjSecSide_BvSep:
					case CD_CircuitInjSecSide_PipeSep:
						continue;
						break;

					default:
						_SetStaticText( lLoopColumn, lCurRow, _T( "" ) );
						break;
				}
			}
		}

		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Save pointer on HM.
		if( false == m_bExporting && false == m_bPrinting )
		{
			SetCellParam( CD_CircuitInjSecSide_Pointer, lCurRow, ( long )pHM );
		}

		// Sub ?
		if( pHM != m_pHM )
		{
			if( true == pHM->IsaModule() )
			{
				SetPictureCellWithID( IDI_HTREE, CD_CircuitInjSecSide_Sub, lCurRow, CSSheet::PictureCellType::Icon );
			}
		}
		else
		{
			if( 0 != pHM->GetLevel() && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_ARROWBACK, CD_CircuitInjSecSide_Sub, lCurRow, CSSheet::PictureCellType::Icon );
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// Circuit position.
		////////////////////////////////////////////////////////////////////////////////////////////////////

		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module.
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}

		_stprintf_s( tcBuffer, SIZEOFINTCHAR( tcBuffer ), _T("%d"), pHM->GetPos() );

		if( true == pHM->IsCircuitIndex() )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_IndexFore );
		}

		_SetStaticText( CD_CircuitInjSecSide_Pos, lCurRow, tcBuffer );

		////////////////////////////////////////////////////////////////////////////////////////////////////
		// 'Description' column.
		////////////////////////////////////////////////////////////////////////////////////////////////////

		// HydroMod Description
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

		// Change background color for parent module
		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}
		else if( false == m_bPrinting && false == m_bExporting )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Name.
		FormatEditText( CD_CircuitInjSecSide_Name, lCurRow, pHM->GetHMName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES );

		// Description.
		FormatEditText( CD_CircuitInjSecSide_Desc, lCurRow, pHM->GetDescription(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES | ES_AUTOHSCROLL, 1000 );

		CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( pHM );
		long lLastSingularityRow = lCurRow;

		if( NULL != pHMInj )
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////
			// Secondary side group.
			////////////////////////////////////////////////////////////////////////////////////////////////////

			// HYS-1202: Add UnitDesc, P, DT, Qref, Dpref, Kv and Cv columns
			// Unit description.
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}
			else if( false == m_bPrinting && false == m_bExporting )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

			if( NULL != pHMInj->GetpTermUnit() )
			{
				FormatEditText( CD_CircuitInjSecSide_UnitDesc, lCurRow, pHM->GetpTermUnit()->GetDescription(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES );

				// Flow.
				int iValidity = pHMInj->CheckValidity();

				bool bFlowError = ( ( CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) )
						|| ( CDS_HydroMod::eValidityFlags::evfFlowTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooLow ) ) );

				switch( pHM->GetpTermUnit()->GetQType() )
				{
					case CTermUnit::_QType::Q:

						if( true == pHMInj->IsaModule() )
						{
							// Change background in regards of valve is a partner valve or not
							if( pHM == m_pHM )
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
							}
							else
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
							}

							_SetStaticText( CD_CircuitInjSecSide_Q, lCurRow, WriteCUDouble( _U_FLOW, pHMInj->GetpTermUnit()->GetQ() ) );

							if( true == bFlowError )
							{
								SetForeColor( CD_CircuitInjSecSide_Q, lCurRow, m_rFillColors.m_ErrorFore );
							}

							SetFontBold( CD_CircuitInjSecSide_Q, lCurRow, FALSE );
						}
						else
						{
							if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
							{
								FormatEditDouble( CD_CircuitInjSecSide_Q, lCurRow, _U_FLOW, pHMInj->GetpTermUnit()->GetQ(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							}
							else
							{
								FormatCUDouble( CD_CircuitInjSecSide_Q, lCurRow, _U_FLOW, pHMInj->GetpTermUnit()->GetQ() );
							}

							SetFontBold( CD_CircuitInjSecSide_Q, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

							if( true == bFlowError )
							{
								SetForeColor( CD_CircuitInjSecSide_Q, lCurRow, m_rFillColors.m_ErrorFore );
							}

							// HYS-1882: DT
							if( true == pHMInj->IsDTFieldDisplayedInFlowMode() )
							{
								if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
								{
									FormatEditDouble( CD_CircuitInjSecSide_DT, lCurRow, _U_DIFFTEMP, pHMInj->GetpTermUnit()->GetDTFlowMode(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
								}
								else
								{
									FormatCUDouble( CD_CircuitInjSecSide_DT, lCurRow, _U_DIFFTEMP, pHMInj->GetpTermUnit()->GetDTFlowMode() );
								}

								SetFontBold( CD_CircuitInjSecSide_DT, lCurRow, (true == bFreezed && false == m_bPrinting && false == m_bExporting) ? TRUE : FALSE );

								if( false == pHMInj->CheckIfTempAreValid() )
								{
									SetForeColor( CD_CircuitInjSecSide_DT, lCurRow, m_rFillColors.m_ErrorFore );
								}
							}
							else
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
							}
						}

						break;

					case CTermUnit::_QType::PdT:

						// HYS-1882: DT is editable.
						if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
						{
							FormatEditDouble( CD_CircuitInjSecSide_P, lCurRow, _U_TH_POWER, pHMInj->GetpTermUnit()->GetP(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							FormatEditDouble( CD_CircuitInjSecSide_DT, lCurRow, _U_DIFFTEMP, pHMInj->GetpTermUnit()->GetDT(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						}
						else
						{
							FormatCUDouble( CD_CircuitInjSecSide_P, lCurRow, _U_TH_POWER, pHMInj->GetpTermUnit()->GetP() );
							FormatCUDouble( CD_CircuitInjSecSide_DT, lCurRow, _U_DIFFTEMP, pHMInj->GetpTermUnit()->GetDT() );
						}

						SetFontBold( CD_CircuitInjSecSide_P, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );
						SetFontBold( CD_CircuitInjSecSide_DT, lCurRow, (true == bFreezed && false == m_bPrinting && false == m_bExporting) ? TRUE : FALSE );

						if( false == pHMInj->CheckIfTempAreValid() )
						{
							SetForeColor( CD_CircuitInjSecSide_DT, lCurRow, m_rFillColors.m_ErrorFore );
						}

						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
						
						FormatCUDouble( CD_CircuitInjSecSide_Q, lCurRow, _U_FLOW, pHMInj->GetpTermUnit()->GetQ() );
						SetFontBold( CD_CircuitInjSecSide_Q, lCurRow, FALSE );

						if( true == bFlowError )
						{
							SetForeColor( CD_CircuitInjSecSide_Q, lCurRow, m_rFillColors.m_ErrorFore );
						}

						break;
				}

				// Dp.
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				// Change background color for parent module.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}
				else if( false == m_bPrinting && false == m_bExporting )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

				switch( pHMInj->GetpTermUnit()->GetDpType() )
				{
					case CDS_HydroMod::eDpType::Dp:
					{
						if( true == pHMInj->IsaModule() )
						{
							// Change background in regards of valve is a partner valve or not
							if( pHM == m_pHM )
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
							}
							else
							{
								SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
							}

							_SetStaticText( CD_CircuitInjSecSide_Dp, lCurRow, WriteCUDouble( _U_DIFFPRESS, pHMInj->GetpTermUnit()->GetDp() ) );
							SetFontBold( CD_CircuitInjSecSide_Dp, lCurRow, FALSE );
						}
						else
						{
							if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
							{
								FormatEditDouble( CD_CircuitInjSecSide_Dp, lCurRow, _U_DIFFPRESS, pHMInj->GetpTermUnit()->GetDp(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							}
							else
							{
								FormatCUDouble( CD_CircuitInjSecSide_Dp, lCurRow, _U_DIFFPRESS, pHMInj->GetpTermUnit()->GetDp() );
							}

							SetFontBold( CD_CircuitInjSecSide_Dp, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );
						}

						break;
					}
					case CDS_HydroMod::eDpType::QDpref:
					{
						double dQRef, dDpRef;
						pHMInj->GetpTermUnit()->GetDp( NULL, &dDpRef, &dQRef );

						if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
						{
							FormatEditDouble( CD_CircuitInjSecSide_Qref, lCurRow, _U_FLOW, dQRef, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							FormatEditDouble( CD_CircuitInjSecSide_Dpref, lCurRow, _U_DIFFPRESS, dDpRef, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						}
						else
						{
							FormatCUDouble( CD_CircuitInjSecSide_Qref, lCurRow, _U_FLOW, dQRef );
							FormatCUDouble( CD_CircuitInjSecSide_Dpref, lCurRow, _U_DIFFPRESS, dDpRef );
						}

						SetFontBold( CD_CircuitInjSecSide_Qref, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );
						SetFontBold( CD_CircuitInjSecSide_Dpref, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
						FormatCUDouble( CD_CircuitInjSecSide_Dp, lCurRow, _U_DIFFPRESS, pHMInj->GetpTermUnit()->GetDp() );
						SetFontBold( CD_CircuitInjSecSide_Dp, lCurRow, FALSE );

						break;
					}

					case CDS_HydroMod::eDpType::Kv:
					case CDS_HydroMod::eDpType::Cv:
					{
						double dValue;
						pHMInj->GetpTermUnit()->GetDp( NULL, &dValue, NULL );
						long lColumn = ( CDS_HydroMod::eDpType::Kv == pHMInj->GetpTermUnit()->GetDpType() ) ? CD_CircuitInjSecSide_Kv : CD_CircuitInjSecSide_Cv;

						if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
						{
							FormatEditDouble( lColumn, lCurRow, _C_KVCVCOEFF, dValue, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );

						}
						else
						{
							FormatCUDouble( lColumn, lCurRow, _C_KVCVCOEFF, dValue );
						}

						SetFontBold( lColumn, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
						FormatCUDouble( CD_CircuitInjSecSide_Dp, lCurRow, _U_DIFFPRESS, pHMInj->GetpTermUnit()->GetDp() );
						SetFontBold( CD_CircuitInjSecSide_Dp, lCurRow, FALSE );

						break;
					}
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			// Pump group.
			////////////////////////////////////////////////////////////////////////////////////////////////////

			if( NULL != pHMInj->GetpPump() )
			{
				// Head.
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}
				else if( false == m_bPrinting && false == m_bExporting )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatEditDouble( CD_CircuitInjSecSide_PumpH, lCurRow, _U_DIFFPRESS, pHMInj->GetpPump()->GetHpump(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				}
				else
				{
					FormatCUDouble( CD_CircuitInjSecSide_PumpH, lCurRow, _U_DIFFPRESS, pHMInj->GetpPump()->GetHpump() );
				}

				SetFontBold( CD_CircuitInjSecSide_PumpH, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

				// Hmin.
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}
				else
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
				}

				_SetStaticText( CD_CircuitInjSecSide_PumpHMin, lCurRow, WriteCUDouble( _U_DIFFPRESS, pHMInj->GetpPump()->GetHmin() ) );

				// HYS-1930: we show the column Havailable for the auto-adaptive with decoupling bypass hydraulic circuit.
				// Havailable is in fact the H available at the secondary pump + H available in the bypass.
				if( eDpStab::DpStabOnBVBypass == pHMInj->GetpSch()->GetDpStab() )
				{
					// Verify first if there is a warning message for the Hmin.
					int iValidity = pHMInj->CheckValidity();

					if( ( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam == ( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam & iValidity ) )
							|| ( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded == ( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded & iValidity ) ) )
					{
						SetForeColor( CD_CircuitInjSecSide_PumpHMin, lCurRow, _TAH_ORANGE );
					}

					// Now fill the Havailable column.
					double dHAvailable = 0.0;

					if( NULL != pHMInj->GetpBypBv() && pHMInj->GetpBypBv()->GetDp() > 0.0 )
					{
						dHAvailable = pHMInj->GetpPump()->GetHAvail() + pHMInj->GetpBypBv()->GetDp();
					}

					_SetStaticText( CD_CircuitInjSecSide_PumpHAvailable, lCurRow, WriteCUDouble( _U_DIFFPRESS, dHAvailable ) );
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			// Temperature group.
			////////////////////////////////////////////////////////////////////////////////////////////////////
			if( NULL != pHMInj->GetpSchcat() )
			{
				int iErrors = 0;
				bool bInjectionAutoAdaptive = ( eDpStab::DpStabOnBVBypass == pHM->GetpSch()->GetDpStab() ) ? true : false;
				bool bTempError = !pHM->CheckIfTempAreValid( &iErrors );

				// Circuit supply temperature at the secondary side.
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				// Change background color for parent module.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}
				else if( false == m_bPrinting && false == m_bExporting && false == bInjectionAutoAdaptive )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				double dCircuitSupplySecondaryTemperature = pHMInj->GetDesignCircuitSupplySecondaryTemperature();

				// HYS-1735: Don't show secondary temperatures for 3-way mixing circuit.
				if( CDB_CircSchemeCateg::e3wTypeMixing != pHMInj->GetpSchcat()->Get3WType() )
				{
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting && false == bInjectionAutoAdaptive )
					{
						FormatEditDouble( CD_CircuitInjSecSide_TempTss, lCurRow, _U_TEMPERATURE, dCircuitSupplySecondaryTemperature, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					}
					else
					{
						FormatCUDouble( CD_CircuitInjSecSide_TempTss, lCurRow, _U_TEMPERATURE, dCircuitSupplySecondaryTemperature );
					}

					SetFontBold( CD_CircuitInjSecSide_TempTss, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting && false == bInjectionAutoAdaptive ) ? TRUE : FALSE );

					if( true == bTempError )
					{
						SetForeColor( CD_CircuitInjSecSide_TempTss, lCurRow, m_rFillColors.m_ErrorFore );
					}

					// Circuit return temperature at the secondary side.
					// HYS-1882: return temp is no more editable
					if( pHM == m_pHM )
					{
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
					}
					else if( false == m_bPrinting && false == m_bExporting )
					{
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
					}
					
					// Computed return temp.
					double dCircuitReturnSecondaryTemperature = pHMInj->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return,
							CAnchorPt::CircuitSide::CircuitSide_Secondary );

					FormatCUDouble( CD_CircuitInjSecSide_TempTrs, lCurRow, _U_TEMPERATURE, dCircuitReturnSecondaryTemperature );

					SetFontBold( CD_CircuitInjSecSide_TempTrs, lCurRow, FALSE );

					if( true == bTempError )
					{
						SetForeColor( CD_CircuitInjSecSide_TempTrs, lCurRow, m_rFillColors.m_ErrorFore );
					}
				}
			
				// Circuit supply temperature at the primary side.
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}
				else
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
				}


				double dCircuitSupplyPrimaryTemperature = pHMInj->GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
				FormatCUDouble( CD_CircuitInjSecSide_TempTsp, lCurRow, _U_TEMPERATURE, dCircuitSupplyPrimaryTemperature );

				if( true == bTempError )
				{
					SetForeColor( CD_CircuitInjSecSide_TempTsp, lCurRow, m_rFillColors.m_ErrorFore );
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Control Valve' group
			////////////////////////////////////////////////////////////////////////////////////////////////////

			if( true == pHM->IsCvExist( true ) && eb3False == pHM->GetpCV()->IsCVLocInPrimary() )
			{
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				// Change background color for parent module
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				if( true == pHM->GetpCV()->IsTaCV() )
				{
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
					{
						FormatComboList( CD_CircuitInjSecSide_CVName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						ComboBoxSendMessage( CD_CircuitInjSecSide_CVName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

						if( NULL != pHM->GetpCV()->GetCvIDPtr().MP )
						{
							ComboBoxSendMessage( CD_CircuitInjSecSide_CVName, lCurRow, SS_CBM_ADDSTRING, 0,
									(LPARAM)( ( (CDB_RegulatingValve *)pHM->GetpCV()->GetCvIDPtr().MP )->GetName() ) );

							ComboBoxSendMessage( CD_CircuitInjSecSide_CVName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
						}

						SetCellParam( CD_CircuitInjSecSide_CVName, lCurRow, (LPARAM)pHM->GetpCV() );
					}
					else
					{
						if( NULL != pHM->GetpCV()->GetCvIDPtr().MP )
						{
							_SetStaticText( CD_CircuitInjSecSide_CVName, lCurRow, ( (CDB_RegulatingValve *)pHM->GetpCV()->GetCvIDPtr().MP )->GetName() );
						}
					}

					// Verify the current flow is not higher than the max flow allowed by the PiCv
					// If it is the case put the Flow and the PiCv in red
					if( CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( pHM->GetpCV()->GetValidtyFlags() & CDS_HydroMod::eValidityFlags::evfFlowTooHigh ) )
					{
						SetForeColor( CD_CircuitInjSecSide_CVName, lCurRow, m_rFillColors.m_ErrorFore );
					}

					// HYS-1759: Check the Dpmax.
					if( CDS_HydroMod::eValidityFlags::evfMaxDpTooHigh == ( pHM->GetpCV()->GetValidtyFlags() & CDS_HydroMod::eValidityFlags::evfMaxDpTooHigh ) )
					{
						SetForeColor( CD_CircuitInjSecSide_CVName, lCurRow, m_rFillColors.m_ErrorFore );
					}

					if( true == pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
					{
						SetFontBold( CD_CircuitInjSecSide_CVName, lCurRow, TRUE );
					}
					else
					{
						SetFontBold( CD_CircuitInjSecSide_CVName, lCurRow, FALSE );
					}

					// Kv and Kvmax are displayed only  when CV is not Presetable
					if( false == pHM->GetpCV()->IsPresettable() && NULL != pHM->GetpCV()->GetCvIDPtr().MP )
					{
						//					if (false == pHM->IsLocked(CDS_HydroMod::eHMObj::eCV))
						{
							// Kvs max
							FormatCUDouble( CD_CircuitInjSecSide_CVKvsMax, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvsmax() );
						}
						// 					else
						// 					{
						// 						FormatEditText( CD_CircuitInjSecSide_CVKvsMax, lCurRow, _T("") );
						// 					}
						// Kvs
						FormatCUDouble( CD_CircuitInjSecSide_CVKvs, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs() );
					}
				}
				else
				{
					// Description
					if( pHM != m_pHM && false == m_bPrinting && false == m_bExporting )
					{
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
					}

					bCVDescription &= !pHM->GetpCV()->GetDescription().IsEmpty();
					FormatEditText( CD_CircuitInjSecSide_CVDesc, lCurRow, pHM->GetpCV()->GetDescription() );

					// Kvs max.
					if( pHM != m_pHM )
					{
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
					}

					FormatCUDouble( CD_CircuitInjSecSide_CVKvsMax, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvsmax() );

					// Kvs.
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
					{
						FormatEditCombo( CD_CircuitInjSecSide_CVKvs, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, (LPARAM)pHM->GetpCV() );
					}
					else
					{
						FormatCUDouble( CD_CircuitInjSecSide_CVKvs, lCurRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, true, false );
					}

					if( true == pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
					{
						SetFontBold( CD_CircuitInjSecSide_CVKvs, lCurRow, TRUE );
					}
					else
					{
						SetFontBold( CD_CircuitInjSecSide_CVKvs, lCurRow, FALSE );
					}
				}

				// CV Setting
				_SetStaticText( CD_CircuitInjSecSide_CVSet, lCurRow, pHM->GetpCV()->GetSettingStr() );

				if( eb3False == pHM->GetpCV()->CheckMinOpen() )
				{
					SetForeColor( CD_CircuitInjSecSide_CVSet, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_CircuitInjSecSide_CVSet, lCurRow, m_rFillColors.m_NormalFore );
				}

				// Dp or Dp Min depending the Cv is a PiCv or not
				if( eb3False == pHM->GetpCV()->IsPICV() )
				{
					// Dp
					FormatCUDouble( CD_CircuitInjSecSide_CVDp, lCurRow, _U_DIFFPRESS, pHM->GetpCV()->GetDp() );

					if( eb3False == pHM->GetpCV()->CheckDpMinMax() )
					{
						SetForeColor( CD_CircuitInjSecSide_CVDp, lCurRow, m_rFillColors.m_ErrorFore );
					}
					else
					{
						SetForeColor( CD_CircuitInjSecSide_CVDp, lCurRow, m_rFillColors.m_NormalFore );
					}

					//Dp Min only for BCV
					if( true == pHM->GetpCV()->IsPresettable() )
					{
						FormatCUDouble( CD_CircuitInjSecSide_CVDpMin, lCurRow, _U_DIFFPRESS, pHM->GetpCV()->GetDpMin( false ) );
					}
				}
				else
				{
					// Dp Min
					FormatCUDouble( CD_CircuitInjSecSide_CVDpMin, lCurRow, _U_DIFFPRESS, pHM->GetpCV()->GetDpMin() );
				}

				if( false == pHM->GetpCV()->IsOn_Off() )
				{
					// Authority
					// Set a "-" for PiCv
					if( eb3True == pHM->GetpCV()->IsPICV() )
					{
						_SetStaticText( CD_CircuitInjSecSide_CVAuth, lCurRow, L"-" );
						_SetStaticText( CD_CircuitInjSecSide_CVDesignAuth, lCurRow, L"-" );
					}
					else
					{
						FormatCUDouble( CD_CircuitInjSecSide_CVAuth, lCurRow, _U_NODIM, pHM->GetpCV()->GetAuth() );

						if( eb3False == pHM->GetpCV()->CheckMinAuthor() )
						{
							SetForeColor( CD_CircuitInjSecSide_CVAuth, lCurRow, m_rFillColors.m_ErrorFore );
						}
						else
						{
							SetForeColor( CD_CircuitInjSecSide_CVAuth, lCurRow, m_rFillColors.m_NormalFore );
						}

						// Design Authority
						FormatCUDouble( CD_CircuitInjSecSide_CVDesignAuth, lCurRow, _U_NODIM, pHM->GetpCV()->GetAuth( true ) );

						if( eb3False == pHM->GetpCV()->CheckMinAuthor( true ) )
						{
							SetForeColor( CD_CircuitInjSecSide_CVDesignAuth, lCurRow, m_rFillColors.m_ErrorFore );
						}
						else
						{
							SetForeColor( CD_CircuitInjSecSide_CVDesignAuth, lCurRow, m_rFillColors.m_NormalFore );
						}
					}
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			// Actuator group
			////////////////////////////////////////////////////////////////////////////////////////////////////

			// 2016-10-28: We want here only IMI valve (not valve defines by their Kvs). This is why the second condition.
			if( true == pHM->IsCvExist( true ) && true == pHM->GetpCV()->IsTaCV() && eb3False == pHM->GetpCV()->IsCVLocInPrimary() )
			{
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				// Change background color for parent module.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				// Name.
				CDB_ElectroActuator *pclActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)( pHM->GetpCV()->GetActrIDPtr().MP ) );

				if( NULL != pclActuator )
				{
					// An actuator has been selected by user...
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
					{
						FormatComboList( CD_CircuitInjSecSide_ActuatorName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						ComboBoxSendMessage( CD_CircuitInjSecSide_ActuatorName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );
						ComboBoxSendMessage( CD_CircuitInjSecSide_ActuatorName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR )( pclActuator->GetName() ) );
						ComboBoxSendMessage( CD_CircuitInjSecSide_ActuatorName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
						SetCellParam( CD_CircuitInjSecSide_ActuatorName, lCurRow, (LPARAM)pHM->GetpCV() );

						CDB_CloseOffChar* pCloseOffChar = static_cast<CDB_CloseOffChar*>(pHM->GetpCV()->GetpCV()->GetCloseOffCharIDPtr().MP);
						// HYS-1685: We display the actuator (CDB_CloseOffChar::CloseOffDp) with error color because it is not enought strong
						SetForeColor( CD_CircuitInjSecSide_ActuatorName, lCurRow, m_rFillColors.m_NormalFore );
						
						// HYS-1759: Do not check the close-of Dp for PIBCV.
						if( eb3True == pHM->GetpCV()->IsPICV() && NULL != pCloseOffChar && pCloseOffChar->GetLimitType() == CDB_CloseOffChar::CloseOffDp )
						{
							// Do not test closeoffDp because the control part never go above Dp max (see CV_name section).
						}
						else if( false == pHM->GetpCV()->IsActuatorStrongEnough() )
						{
							CDB_CloseOffChar* pCloseOffChar = static_cast<CDB_CloseOffChar*>( pHM->GetpCV()->GetpCV()->GetCloseOffCharIDPtr().MP );
							// HYS-1685: Case of KTM 512
							if( NULL != pCloseOffChar && pCloseOffChar->GetLimitType() == CDB_CloseOffChar::CloseOffDp )
							{
								SetForeColor( CD_CircuitInjSecSide_ActuatorName, lCurRow, m_rFillColors.m_ErrorFore );
							}
						}
					}
					else
					{
						_SetStaticText( CD_CircuitInjSecSide_ActuatorName, lCurRow, pclActuator->GetName() );
					}
				}
				else
				{
					// Remark: No actuator has been selected but the control valve has actuator group. In that case, user must be able to choose
					// from a combo.
					if( pHM->GetpCV()->GetpCV()->GetActuatorGroupIDPtr().MP != NULL )
					{
						if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
						{
							FormatComboList( CD_CircuitInjSecSide_ActuatorName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							ComboBoxSendMessage( CD_CircuitInjSecSide_ActuatorName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );
							ComboBoxSendMessage( CD_CircuitInjSecSide_ActuatorName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR )TASApp.LoadLocalizedString( IDS_NOACTUATOR ) );
							ComboBoxSendMessage( CD_CircuitInjSecSide_ActuatorName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
							SetCellParam( CD_CircuitInjSecSide_ActuatorName, lCurRow, (LPARAM)pHM->GetpCV() );
						}
						else
						{
							_SetStaticText( CD_CircuitInjSecSide_ActuatorName, lCurRow, TASApp.LoadLocalizedString( IDS_NOACTUATOR ) );
						}
					}
					else
					{
						// No actuator selected and no available actuator for the control valve.
						_SetStaticText( CD_CircuitInjSecSide_ActuatorName, lCurRow, TASApp.LoadLocalizedString( IDS_NOACTUATOR ) );
					}
				}

				// If control valve is locked, we consider that actuator is also automatically locked.
				if( true == pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
				{
					SetFontBold( CD_CircuitInjSecSide_ActuatorName, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_CircuitInjSecSide_ActuatorName, lCurRow, FALSE );
				}

				// Change background color for parent module.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				// Power supply.
				_SetStaticText( CD_CircuitInjSecSide_ActuatorPowerSupply, lCurRow, ( pclActuator != NULL ) ? pclActuator->GetPowerSupplyStr() : _T( "" ) );

				// Input signal.
				_SetStaticText( CD_CircuitInjSecSide_ActuatorInputSignal, lCurRow, ( pclActuator != NULL ) ? pclActuator->GetInOutSignalsStr( true ) : _T( "" ) );

				// Output signal.
				_SetStaticText( CD_CircuitInjSecSide_ActuatorOutputSignal, lCurRow, ( pclActuator != NULL ) ? pclActuator->GetInOutSignalsStr( false ) : _T( "" ) );

				// Relay type.
				_SetStaticText( CD_CircuitInjSecSide_ActuatorRelayType, lCurRow, ( pclActuator != NULL ) ? pclActuator->GetRelayStr() : _T( "" ) );

				if( pclActuator != NULL )
				{
					// Fail safe.
					if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
					{
						str = TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
					}
					else if( pclActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
					{
						str = TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
					}
					else
					{
						str = TASApp.LoadLocalizedString( IDS_NO );
					}
					_SetStaticText( CD_CircuitInjSecSide_ActuatorFailSafe, lCurRow, str );
				}
				
				// Default return position.
				_SetStaticText( CD_CircuitInjSecSide_ActuatorDRP, lCurRow, ( pclActuator != NULL ) ? pclActuator->GetDefaultReturnPosStr( pclActuator->GetDefaultReturnPos() ).c_str() : _T( "" ) );
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			// Regulating valve group.
			////////////////////////////////////////////////////////////////////////////////////////////////////

			if( true == pHMInj->IsBvSecExist( true ) )
			{
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				// Change background color for parent module.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

				// Name.
				if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
				{
					FormatComboList( CD_CircuitInjSecSide_BvName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( CD_CircuitInjSecSide_BvName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

					if( NULL != pHMInj->GetpSecBv()->GetIDPtr().MP )
					{
						ComboBoxSendMessage( CD_CircuitInjSecSide_BvName, lCurRow, SS_CBM_ADDSTRING, 0,
								(LPARAM)( ( (CDB_TAProduct *)( pHMInj->GetpSecBv()->GetIDPtr().MP ) )->GetName() ) );

						ComboBoxSendMessage( CD_CircuitInjSecSide_BvName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					}

					SetCellParam( CD_CircuitInjSecSide_BvName, lCurRow, (LPARAM)pHMInj->GetpSecBv() );
				}
				else
				{
					if( NULL != pHMInj->GetpSecBv()->GetIDPtr().MP )
					{
						_SetStaticText( CD_CircuitInjSecSide_BvName, lCurRow, ( (CDB_TAProduct *)( pHMInj->GetpSecBv()->GetIDPtr().MP ) )->GetName() );
					}
				}

				if( true == pHMInj->IsLocked( pHMInj->GetpSecBv()->GetHMObjectType() ) )
				{
					SetFontBold( CD_CircuitInjSecSide_BvName, lCurRow, TRUE );
				}
				else
				{
					SetFontBold( CD_CircuitInjSecSide_BvName, lCurRow, FALSE );
				}

				// Setting.
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

				// Change background color for parent module.
				if( pHM == m_pHM )
				{
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
				}

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
				_SetStaticText( CD_CircuitInjSecSide_BvSet, lCurRow, pHMInj->GetpSecBv()->GetSettingStr() );

				if( eb3False == pHMInj->GetpSecBv()->CheckMinOpen() )
				{
					SetForeColor( CD_CircuitInjSecSide_BvSet, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_CircuitInjSecSide_BvSet, lCurRow, m_rFillColors.m_NormalFore );
				}

				// Dp.
				FormatCUDouble( CD_CircuitInjSecSide_BvDp, lCurRow, _U_DIFFPRESS, pHMInj->GetpSecBv()->GetDp() );

				if( eb3False == pHMInj->GetpSecBv()->CheckDpMinMax() )
				{
					SetForeColor( CD_CircuitInjSecSide_BvDp, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_CircuitInjSecSide_BvDp, lCurRow, m_rFillColors.m_NormalFore );
				}

				// DpSig.
				FormatCUDouble( CD_CircuitInjSecSide_BvDpSig, lCurRow, _U_DIFFPRESS, pHMInj->GetpSecBv()->GetDpSignal() );

				if( eb3False == pHMInj->GetpSecBv()->CheckDpMinMax() )
				{
					SetForeColor( CD_CircuitInjSecSide_BvDpSig, lCurRow, m_rFillColors.m_ErrorFore );
				}
				else
				{
					SetForeColor( CD_CircuitInjSecSide_BvDpSig, lCurRow, m_rFillColors.m_NormalFore );
				}

				// Add a test to know if all BV have a DpSig, do not show the Dp column.
				iNbrBvSec++;

				if( 0 != pHMInj->GetpSecBv()->GetDpSignal() )
				{
					iNbrBvSecDpSig++;
				}

				// Dp 100%.
				FormatCUDouble( CD_CircuitInjSecSide_BvDpMin, lCurRow, _U_DIFFPRESS, pHMInj->GetpSecBv()->GetDpMin( false ) );
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			// Pipe group.
			////////////////////////////////////////////////////////////////////////////////////////////////////

			// Circuit Pipe
			bool bPipeVisible = true;

			// Circuit pipe doesn't exist for a module
			if( true == pHMInj->IsaModule() )
			{
				bPipeVisible = false;
			}

			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			// Change background color for parent module.
			if( pHM == m_pHM )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
			}

			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			CPipes *pPipe = pHMInj->GetpCircuitSecondaryPipe();

			if( NULL != pPipe && true == bPipeVisible )
			{
				// Series.
				if( NULL == m_pPipeCircTab )
				{
					// HYS-1735: Do not authorize combonox with possibility to make choice for 3-way mixing circuit.
					// In this case the pipe series in primary should be used.
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting 
						&& ( CDB_CircSchemeCateg::e3wTypeMixing != pHM->GetpSchcat()->Get3WType() ) )
					{
						FormatComboList( CD_CircuitInjSecSide_PipeSerie, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						ComboBoxSendMessage( CD_CircuitInjSecSide_PipeSerie, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

						// Pipe length exist but size and series are not yet defined, flow is 0 for instance.
						if( NULL != pPipe->GetPipeSeries() )
						{
							ComboBoxSendMessage( CD_CircuitInjSecSide_PipeSerie, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( pPipe->GetPipeSeries()->GetName() ) );
							ComboBoxSendMessage( CD_CircuitInjSecSide_PipeSerie, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
						}

						SetCellParam( CD_CircuitInjSecSide_PipeSerie, lCurRow, (LPARAM)pPipe );
					}
					else
					{
						if( NULL != pPipe->GetPipeSeries() )
						{
							_SetStaticText( CD_CircuitInjSecSide_PipeSerie, lCurRow, pPipe->GetPipeSeries()->GetName() );
							SetFontBold( CD_CircuitInjSecSide_PipeSerie, lCurRow, ( true == bFreezed ) ? TRUE : FALSE );
						}
					}
				}

				if( false == pHMInj->IsaModule() )
				{
					// Size.
					// HYS-1735: Do not authorize combonox with possibility to make choice for 3-way mixing circuit.
					// In this case the pipe size in primary should be used.
					if( false == bFreezed && false == m_bPrinting && false == m_bExporting 
						&& ( CDB_CircSchemeCateg::e3wTypeMixing != pHM->GetpSchcat()->Get3WType() ) )
					{
						FormatComboList( CD_CircuitInjSecSide_PipeSize, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						ComboBoxSendMessage( CD_CircuitInjSecSide_PipeSize, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

						// Pipe length exist but size is not yet defined, flow is 0 for instance.
						if( NULL != pPipe->GetIDPtr().MP )
						{
							ComboBoxSendMessage( CD_CircuitInjSecSide_PipeSize, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() ) );
							ComboBoxSendMessage( CD_CircuitInjSecSide_PipeSize, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
						}

						SetCellParam( CD_CircuitInjSecSide_PipeSize, lCurRow, (LPARAM)pPipe );
					}
					else
					{
						if( NULL != pPipe->GetIDPtr().MP )
						{
							_SetStaticText( CD_CircuitInjSecSide_PipeSize, lCurRow, ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() );
						}
					}

					if( true == pHMInj->IsLocked( pPipe->GetLocate() ) )
					{
						SetFontBold( CD_CircuitInjSecSide_PipeSize, lCurRow, TRUE );
					}
					else
					{
						SetFontBold( CD_CircuitInjSecSide_PipeSize, lCurRow, FALSE );
					}

					// HYS-1735: Information not needed
					if( CDB_CircSchemeCateg::e3wTypeMixing == pHMInj->GetpSchcat()->Get3WType() )
					{
						_SetStaticText( CD_CircuitInjSecSide_PipeL, lCurRow, _T( "-" ) );
						_SetStaticText( CD_CircuitInjSecSide_PipeDp, lCurRow, _T( "-" ) );
						_SetStaticText( CD_CircuitInjSecSide_PipeLinDp, lCurRow, _T( "-" ) );
						_SetStaticText( CD_CircuitInjSecSide_PipeV, lCurRow, _T( "-" ) );
					}
					else
					{
						// Length.
						if( pHMInj != m_pHM && false == m_bPrinting && false == m_bExporting )
						{
							SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_EditBkg );
						}

						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

						if( false == bFreezed && false == m_bPrinting && false == m_bExporting )
						{
							FormatEditDouble( CD_CircuitInjSecSide_PipeL, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, false );
						}
						else
						{
							FormatCUDouble( CD_CircuitInjSecSide_PipeL, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, true, false );
						}

						SetFontBold( CD_CircuitInjSecSide_PipeL, lCurRow, ( true == bFreezed && false == m_bPrinting && false == m_bExporting ) ? TRUE : FALSE );

						if( pHMInj != m_pHM )
						{
							SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
						}

						// Dp.
						double dSI = pPipe->GetPipeDp();
						FormatCUDouble( CD_CircuitInjSecSide_PipeDp, lCurRow, _U_DIFFPRESS, pPipe->GetPipeDp() );

						// Linear Dp.
						FormatCUDouble( CD_CircuitInjSecSide_PipeLinDp, lCurRow, _U_LINPRESSDROP, pPipe->GetLinDp() );

						if( eb3False == pPipe->CheckLinDpMinMax() )
						{
							SetForeColor( CD_CircuitInjSecSide_PipeLinDp, lCurRow, m_rFillColors.m_ErrorFore );
						}
						else
						{
							SetForeColor( CD_CircuitInjSecSide_PipeLinDp, lCurRow, m_rFillColors.m_NormalFore );
						}

						// Velocity.
						FormatCUDouble( CD_CircuitInjSecSide_PipeV, lCurRow, _U_VELOCITY, pPipe->GetVelocity() );

						if( eb3False == pPipe->CheckVelocityMinMax() )
						{
							SetForeColor( CD_CircuitInjSecSide_PipeV, lCurRow, m_rFillColors.m_ErrorFore );
						}
						else
						{
							SetForeColor( CD_CircuitInjSecSide_PipeV, lCurRow, m_rFillColors.m_NormalFore );
						}
					}
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Accessories' group
			////////////////////////////////////////////////////////////////////////////////////////////////////
			
			// HYS-1735: Information not needed.
			if( CDB_CircSchemeCateg::e3wTypeMixing == pHMInj->GetpSchcat()->Get3WType() )
			{
				_SetStaticText( CD_CircuitInjSecSide_AccDesc, lCurRow, _T( "-" ) );
				_SetStaticText( CD_CircuitInjSecSide_AccData, lCurRow, _T( "-" ) );
				_SetStaticText( CD_CircuitInjSecSide_AccDp, lCurRow, _T( "-" ) );
			}
			else if( true == m_bPrinting && NULL != pPipe )
			{
				// One line by singularity.
				CPipes::CSingularity Singularity;
				int iSingulPos = pPipe->GetFirstSingularity( &Singularity );
				bool bFirstPass = true;

				while( iSingulPos != -1 )
				{
					CString strDescription, strInputData;

					if( true == pPipe->GetSingularityDetails( &Singularity, &strDescription, &strInputData ) )
					{
						if( false == bFirstPass )
						{
							lLastSingularityRow++;
						}

						// Description.
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
						SetStaticTextEx( CD_CircuitInjSecSide_AccDesc, lLastSingularityRow, strDescription, true );

						// Input data.
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
						SetStaticTextEx( CD_CircuitInjSecSide_AccData, lLastSingularityRow, strInputData, true );

						// Dp.
						CString strDp = WriteCUDouble( _U_DIFFPRESS, pPipe->GetSingularityDp( &Singularity ), false );
						SetStaticTextEx( CD_CircuitInjSecSide_AccDp, lLastSingularityRow, strDp, true );

						bFirstPass = false;
					}

					iSingulPos = pPipe->GetNextSingularity( iSingulPos, &Singularity );
				}
			}
			else
			{
				// Description.
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				CString str = pPipe->GetSingularitiesDescriptionShort( true );
				_SetStaticText( CD_CircuitInjSecSide_AccDesc, lCurRow, str );

				// Input data.
				str = pPipe->GetSingularitiesInputDataShort();
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ELLIPSES );
				_SetStaticText( CD_CircuitInjSecSide_AccData, lCurRow, str );

				// Dp.
				FormatCUDouble( CD_CircuitInjSecSide_AccDp, lCurRow, _U_DIFFPRESS, pPipe->GetSingularityTotalDp( true ) );
			}
		}

		if( pHM == m_pHM )
		{
			// If we are in printing mode and there is more than one singularity in the accessory group...
			if( true == m_bPrinting && lLastSingularityRow > lCurRow )
			{
				for( long lLoopRow = lCurRow + 1; lLoopRow <= lLastSingularityRow; lLoopRow++ )
				{
					// Change background color for parent module in the line added.
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );

					for( int lLoopColumn = CD_CircuitInjSecSide_Name; lLoopColumn <= CD_CircuitInjSecSide_Pointer; lLoopColumn++ )
					{
						switch( lLoopColumn )
						{
							case CD_CircuitInjSecSide_DescSep:
							case CD_CircuitInjSecSide_Sep:
							case CD_CircuitInjSecSide_PumpSep:
							case CD_CircuitInjSecSide_TempSep:
							case CD_CircuitInjSecSide_BvSep:
							case CD_CircuitInjSecSide_PipeSep:
								continue;
								break;

							default:
								_SetStaticText( lLoopColumn, lLoopRow, _T( "" ) );
								break;
						}
					}
				}

				lCurRow = lLastSingularityRow;
			}

			// Add a solid border below the root module.
			SetCellBorder( CD_CircuitInjSecSide_Name, lCurRow, CD_CircuitInjSecSide_DescSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetCellBorder( CD_CircuitInjSecSide_UnitDesc, lCurRow, CD_CircuitInjSecSide_Sep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetCellBorder( CD_CircuitInjSecSide_PumpH, lCurRow, CD_CircuitInjSecSide_PumpSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetCellBorder( CD_CircuitInjSecSide_TempTsp, lCurRow, CD_CircuitInjSecSide_TempSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetCellBorder( CD_CircuitInjSecSide_CVName, lCurRow, CD_CircuitInjSecSide_CVSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetCellBorder( CD_CircuitInjSecSide_ActuatorName, lCurRow, CD_CircuitInjSecSide_ActuatorSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetCellBorder( CD_CircuitInjSecSide_BvName, lCurRow, CD_CircuitInjSecSide_BvSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetCellBorder( CD_CircuitInjSecSide_PipeSerie, lCurRow, CD_CircuitInjSecSide_PipeSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetCellBorder( CD_CircuitInjSecSide_AccDesc, lCurRow, CD_CircuitInjSecSide_AccDp, lCurRow, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );

			for( int c = CD_CircuitInjSecSide_Sub; c < CD_CircuitInjSecSide_Pointer; c++ )
			{
				_SetStaticText( c, lCurRow + 1, _T( "" ) );
			}

			lCurRow += 2;
		}
		else
		{
			lCurRow = lLastSingularityRow;
			lCurRow++;
		}

		// Add gray border between each product.
		SetCellBorder( CD_CircuitInjSecSide_Name, lCurRow, CD_CircuitInjSecSide_DescSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_CircuitInjSecSide_UnitDesc, lCurRow, CD_CircuitInjSecSide_Sep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_CircuitInjSecSide_PumpH, lCurRow, CD_CircuitInjSecSide_PumpSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_CircuitInjSecSide_TempTsp, lCurRow, CD_CircuitInjSecSide_TempSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_CircuitInjSecSide_CVName, lCurRow - 1, CD_CircuitInjSecSide_CVSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_CircuitInjSecSide_ActuatorName, lCurRow - 1, CD_CircuitInjSecSide_ActuatorSep - 1, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_CircuitInjSecSide_BvName, lCurRow, CD_CircuitInjSecSide_BvSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_CircuitInjSecSide_PipeSerie, lCurRow, CD_CircuitInjSecSide_PipeSep - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
		SetCellBorder( CD_CircuitInjSecSide_AccDesc, lCurRow, CD_CircuitInjSecSide_AccDp, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
	}

	lCurRow--;
	SetCellBorder( CD_CircuitInjSecSide_Name, lCurRow, CD_CircuitInjSecSide_Pointer - 1, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
			SPREAD_COLOR_NONE );

	_HideEmptyColumns();

	// Hide the Dp Column if there is a DpSig for all Bv.
	if( iNbrBvSec > 0 )
	{
		bool fOnlyDpSig = false;

		if( iNbrBvSec == iNbrBvSecDpSig )
		{
			fOnlyDpSig = true;
		}

		ShowCol( CD_CircuitInjSecSide_BvDp, ( true == fOnlyDpSig ) ? FALSE : TRUE );
		ShowCol( CD_CircuitInjSecSide_BvDpSig, ( iNbrBvSecDpSig > 0 ) ? TRUE : FALSE );
	}

	// Check now if we can hide collapse button or not.
	VerifyExpandCollapseColumnButtons( CRect( CD_CircuitInjSecSide_Name, RD_Header_FirstCirc, CD_CircuitInjSecSide_AccDp, lCurRow /*- 1*/ ), RD_Header_ButDlg );

	// Check if last column is a separator.
	// Remarks: in this case, we must set 'CellCantBePainted' column property to true to
	// avoid selection to be drawn on this last column.
	CheckLastColumnFlagCantBePainted( RD_Header_ButDlg );

	// Check if there is no successive separators.
	_HideSuccessiveSeparator( CD_CircuitInjSecSide_DescSep, CD_CircuitInjSecSide_AccDp, RD_Header_FirstCirc, SheetDescription::SCircuitInj );
}

void CSheetHMCalc::_FillMeasurementSheet()
{
	if( NULL == m_pHM )
	{
		return;
	}

	// Draw border for the Picture row
	SetCellBorder( CD_Measurement_DateTime, RD_Header_Picture, CD_Measurement_Description, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_WaterChar, RD_Header_Picture, CD_Measurement_KvSignal, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_Dp, RD_Header_Picture, CD_Measurement_Dpl, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_MeasureDp, RD_Header_Picture, CD_Measurement_FlowDeviation, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_TempHH, RD_Header_Picture, CD_Measurement_DiffTempDeviation, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_Measurement_Power, RD_Header_Picture, CD_Measurement_PowerDeviation, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );

	// Variables.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	ASSERT( pUnitDB );
	long lCurRow = RD_Header_FirstCirc;
	long lHMRows = lCurRow;
	bool bKvSignalExist = false;
	bool bPresettingExist = false;		// Settings computed by TASelect.
	bool bSettingExist = false;			// Settings set by user.

	CString str, str1, str2;
	CDS_HydroMod *pHM = NULL;
	LPARAM lparam = (LPARAM)0;
	CRank HMList( false );
	struct sMod
	{
		CDS_HydroMod *pHM;
		bool fSecondarySide;
	};

	int iCount = m_pHM->GetItemCount();

	// Create a list with all circuits and count the number of lines.
	for( IDPTR IDPtr = m_pHM->GetFirst(); '\0' != *IDPtr.ID; IDPtr = m_pHM->GetNext( IDPtr.MP ) )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)IDPtr.MP;

		for( int i = 0; i < 2; i++ )
		{
			sMod *pMod = new sMod;
			pMod->pHM = pHM;
			pMod->fSecondarySide = ( NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsSecondarySideExist() && i == 1 );
			HMList.Add( str, pHM->GetPos() + ( ( true == pMod->fSecondarySide ) ? 0.1 : 0.0 ), (LPARAM)pMod );
			lHMRows += 1;	// 1 row by circuit

			// Abort loop if secondary side doesn't exist.
			if( NULL == pHM->GetpSchcat() || false == pHM->GetpSchcat()->IsSecondarySideExist() )
			{
				break;
			}
		}
	}

	// Parent module at the first position.
	sMod *pMod = new sMod;
	pMod->pHM = m_pHM;
	pMod->fSecondarySide = false;
	HMList.Add( str, 0, (LPARAM)pMod );

	if( NULL != m_pHM->GetpSchcat() && true == m_pHM->GetpSchcat()->IsSecondarySideExist() )
	{
		sMod *pMod = new sMod;
		pMod->pHM = m_pHM;
		pMod->fSecondarySide = true;
		HMList.Add( str, 0.1, (LPARAM)pMod );
		lHMRows += 1;
	}

	// +1 row for parent module
	lHMRows += 1;

	// Redefine the max rows.
	SetMaxRows( lHMRows );

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::SMeasurement );

		if( NULL != pclSheetDescription )
		{
			pclSheetDescription->Init( 1, GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
			pclSheetDescription->SetSelectableRangeRow( RD_Header_FirstCirc, GetMaxRows() );
		}
	}

	// Do a loop on the list and write the info.
	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		ASSERT( lCurRow <= lHMRows );
		sMod *pMod = (sMod *)lparam;

		if( NULL == pMod )
		{
			continue;
		}

		pHM = dynamic_cast<CDS_HydroMod *>( pMod->pHM );

		if( NULL == pHM )
		{
			continue;
		}

		CDB_TAProduct *pclValve = (CDB_TAProduct *)( pHM->GetTADBValveIDPtr().MP );

		if( false == pMod->fSecondarySide )
		{
			if( NULL == pHM->GetpBv() && NULL != pHM->GetpCV() )
			{
				if( NULL != (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP ) )
				{
					pclValve = (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP );
				}
			}
			else if( NULL != pHM->GetpBv() )
			{
				if( NULL != (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP ) )
				{
					pclValve = (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP );
				}
			}
		}
		else
		{
			// Secondary side.

			// Do not display CBI valve in case of secondary exist. Because if secondary size exists but there is no
			// balancing valve, we don't want to display the primary valve here.
			pclValve = NULL;

			if( NULL != pHM->GetpSecBv() )
			{
				if( NULL != (CDB_TAProduct *)( pHM->GetpSecBv()->GetIDPtr().MP ) )
				{
					pclValve = (CDB_TAProduct *)( pHM->GetpSecBv()->GetIDPtr().MP );
				}
			}
		}

		// Prepare color to use in regards to the current row is in cut mode or not.
		bool bIsCutMode = _PrepareColors( pHM );

		// Redefine the text pattern.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		// Allow cells to be split in two parts.
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

		// If it's the partner valve...
		if( pHM == m_pHM )
		{
			bool bDrawBorder = false;

			if( NULL != m_pHM->GetpSchcat() && true == m_pHM->GetpSchcat()->IsSecondarySideExist() )
			{
				if( true == pMod->fSecondarySide )
				{
					bDrawBorder = true;
				}
			}
			else
			{
				bDrawBorder = true;
			}

			COLORREF linecolor = ( true == bDrawBorder ) ? SPREAD_COLOR_NONE : _LIGHTGRAY;

			// Draw border for the partner valve.
			SetCellBorder( CD_Measurement_DateTime, lCurRow, CD_Measurement_Description, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );
			SetCellBorder( CD_Measurement_WaterChar, lCurRow, CD_Measurement_KvSignal, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );
			SetCellBorder( CD_Measurement_Dp, lCurRow, CD_Measurement_Dpl, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );
			SetCellBorder( CD_Measurement_MeasureDp, lCurRow, CD_Measurement_FlowDeviation, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );
			SetCellBorder( CD_Measurement_TempHH, lCurRow, CD_Measurement_DiffTempDeviation, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );
			SetCellBorder( CD_Measurement_Power, lCurRow, CD_Measurement_PowerDeviation, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );


			// Is this a sub module...
			if( m_pHM->GetLevel() && false == pMod->fSecondarySide && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_ARROWBACK, CD_Measurement_Sub, lCurRow, CSSheet::PictureCellType::Icon );
			}


			// Change background color for the parent module.
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );

			for( int lLoopColumn = CD_Measurement_DateTime; lLoopColumn < CD_Measurement_Pointer; lLoopColumn++ )
			{
				switch( lLoopColumn )
				{
					case CD_Measurement_DescriptionSep:
					case CD_Measurement_KvSignalSep:
					case CD_Measurement_DplSep:
					case CD_Measurement_FlowDeviationSep:
					case CD_Measurement_DiffTempDeviationSep:
						continue;
						break;

					default:
						_SetStaticText( lLoopColumn, lCurRow, _T( "" ) );
						break;
				}
			}
		}
		else
		{
			// Draw border.
			SetCellBorder( CD_Measurement_DateTime, lCurRow, CD_Measurement_Description, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( CD_Measurement_WaterChar, lCurRow, CD_Measurement_KvSignal, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( CD_Measurement_Dp, lCurRow, CD_Measurement_Dpl, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( CD_Measurement_MeasureDp, lCurRow, CD_Measurement_FlowDeviation, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( CD_Measurement_TempHH, lCurRow, CD_Measurement_DiffTempDeviation, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( CD_Measurement_Power, lCurRow, CD_Measurement_PowerDeviation, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );

			// Change background color for module valves.
			if( true == pMod->fSecondarySide )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg2nd );
			}
			else
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
			}

			// Draw Icon module
			if( true == pHM->IsaModule() && false == pMod->fSecondarySide )
			{
				SetPictureCellWithID( IDI_HTREE, CD_Measurement_Sub, lCurRow, CSSheet::PictureCellType::Icon );
			}
		}

		// Save pointer on HM (last column).
		if( false == m_bExporting && false == m_bPrinting )
		{
			SetCellParam( CD_Measurement_Pointer, lCurRow, ( long )pHM );
		}

		// Write position.
		TCHAR buf[10];

		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}

		if( true == pMod->fSecondarySide )
		{
			_stprintf_s( buf, SIZEOFINTCHAR( buf ), _T("%d\""), pHM->GetPos() );
		}
		else
		{
			_stprintf_s( buf, SIZEOFINTCHAR( buf ), _T("%d"), pHM->GetPos() );
		}

		if( true == pHM->IsCircuitIndex() )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_IndexFore );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		_SetStaticText( CD_Measurement_Pos, lCurRow, buf );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_NormalFore );

		// Write the reference.
		_SetStaticText( CD_Measurement_Reference, lCurRow, pHM->GetHMName() );

		// Write the description.
		_SetStaticText( CD_Measurement_Description, lCurRow, pHM->GetDescription() );

		// Write the valve.
		if( edt_TADBValve == pHM->GetVDescrType() )
		{
			str = _T( "" );

			if( NULL != pclValve )
			{
				str = pclValve->GetName();
			}
		}
		else if( edt_KvCv == pHM->GetVDescrType() && NULL != pUnitDB )
		{
			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
			{
				str = TASApp.LoadLocalizedString( IDS_KV );
			}
			else
			{
				str = TASApp.LoadLocalizedString( IDS_CV );
			}

			str += ( CString )_T(" = ") + WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, pHM->GetKvCv() ), 3, 0 );

			if( false == str.IsEmpty() )
			{
				str += ( CString )_T("*");
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_NotTAFore );
			}
		}
		else
		{
			str = pHM->GetCBIType() + ( CString )_T(" ") + pHM->GetCBISize();
			str.TrimLeft();

			if( !str.IsEmpty() )
			{
				str += ( CString )_T("*");
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_NotTAFore );
			}
		}

		_SetStaticText( CD_Measurement_Valve, lCurRow, str );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_NormalFore );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Write the presetting (if a presetting exist in the CMeasData it will be overwritten).
		if( false == pMod->fSecondarySide )
		{
			// HYS-1892: we verify pclValve and GetValveCharacteristic() to avoid HySelect crash
			if( pHM->GetPresetting() > 0.0 && NULL != pclValve && NULL != pclValve->GetValveCharacteristic() )
			{
				str = pclValve->GetValveCharacteristic()->GetSettingString( pHM->GetPresetting() );
				_SetStaticText( CD_Measurement_Presetting, lCurRow, str );
			}

			if( NULL != pHM->GetpBv() && L"" != pHM->GetpBv()->GetSettingStr() )
			{
				_SetStaticText( CD_Measurement_Presetting, lCurRow, pHM->GetpBv()->GetSettingStr() );
			}
			else if( NULL != pHM->GetpCV() && L"" != pHM->GetpCV()->GetSettingStr() )
			{
				_SetStaticText( CD_Measurement_Presetting, lCurRow, pHM->GetpCV()->GetSettingStr() );
			}
		}
		else if( NULL != pHM->GetpSecBv() && L"" != pHM->GetpSecBv()->GetSettingStr() )
		{
			_SetStaticText( CD_Measurement_Presetting, lCurRow, pHM->GetpSecBv()->GetSettingStr() );
		}

		// Write the Kv Signal (if a meas exist, re-enter the Kv signal value).
		if( NULL != pclValve && true == pclValve->IsKvSignalEquipped() && -1.0 != pclValve->GetKvSignal() )
		{
			_SetStaticText( CD_Measurement_KvSignal, lCurRow, WriteCUDouble( _C_KVCVCOEFF, pclValve->GetKvSignal() ) );
			bKvSignalExist = true;
		}
		else if( NULL != pclValve )
		{
			bSettingExist = true;
		}

		// Write the design flow.
		CString strFlow = GetDashDotDash();

		if( false == pMod->fSecondarySide )
		{
			if( pHM->GetQDesign() > 0.0 )
			{
				strFlow = WriteCUDouble( _U_FLOW, pHM->GetQDesign() );
			}
		}
		else
		{
			if( NULL != pHM->GetpSecBv() && pHM->GetpSecBv()->GetQ() > 0.0 )
			{
				strFlow = WriteCUDouble( _U_FLOW, pHM->GetpSecBv()->GetQ() );
			}
			else if( NULL != pHM->GetpCircuitSecondaryPipe() && pHM->GetpCircuitSecondaryPipe()->GetTotalQ() > 0.0 )
			{
				strFlow = WriteCUDouble( _U_FLOW, pHM->GetpCircuitSecondaryPipe()->GetTotalQ() );
			}
		}

		_SetStaticText( CD_Measurement_DesignFlow, lCurRow, strFlow );

		// Define the CMeasData to show (this will be the last that fit the boundaries).
		bool bAtLeastOneMeasFit = false;
		unsigned int iMeasDataToShow = 0;
		COleDateTime timLast;
		unsigned int uiNbrMeasData = pHM->GetMeasDataSize();

		for( unsigned int i = 0; i < uiNbrMeasData; i++ )
		{
			// Get the MeasData.
			CDS_HydroMod::CMeasData *pMeasData = pHM->GetpMeasData( i );

			if( NULL == pMeasData )
			{
				continue;
			}

			// HYS-1734: CDS_HydroModX::CMeasData::GetLocate renamed to GetPipeLocation.
			if( true == pMod->fSecondarySide && CDS_HydroModX::eLocate::InSecondary != pMeasData->GetPipeLocation() )
			{
				continue;
			}

			if( CDS_HydroModX::eLocate::undef == pMeasData->GetPipeLocation() )
			{
				pMeasData->SetLocate( CDS_HydroModX::InPrimary );
			}

			if( false == pMod->fSecondarySide
				&& ( CDS_HydroModX::eLocate::InPrimary != pMeasData->GetPipeLocation() /*&& CDS_HydroModX::eLocate::undef != pMeasData->GetPipeLocation() */ ) )
			{
				continue;
			}

			// In case we have measurements from the CBI, no date exist, it will always be shown.
			if( pMeasData->GetInstrument() == CDS_HydroMod/*::eInstrumentUsed*/::TACBI )
			{
				iMeasDataToShow = i;
				bAtLeastOneMeasFit = true;
				continue;
			}

			// Write DateTime.
			__time32_t time32 = pMeasData->GetMeasDateTime();
			tm dt;

			if( 0 != _gmtime32_s( &dt, &time32 ) )
			{
				ZeroMemory( &dt, sizeof( dt ) );
				dt.tm_year = 70;
				dt.tm_mday = 1;
			}

			COleDateTime tim( dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec );

			// In case the measure is outside the limits.
			if( tim < pMainFrame->GetLowerDateTime() || tim > pMainFrame->GetUpperDateTime() )
			{
				continue;
			}

			bAtLeastOneMeasFit = true;

			if( tim > timLast )
			{
				timLast = tim;
				iMeasDataToShow = i;
			}
		}

		// Show the last CMeasData.
		if( uiNbrMeasData > 0  && true == bAtLeastOneMeasFit )
		{
			// Get the MeasData.
			CDS_HydroMod::CMeasData *pMeasData = pHM->GetpMeasData( iMeasDataToShow );

			// Can show measurement information only if Date time is set OR if date time is NULL (it's the case for CBI) we must
			// be sure that data don't come from TAScope.
			if( NULL != pMeasData && 0 != pMeasData->GetMeasDateTime() || ( 0 == pMeasData->GetMeasDateTime()
					&& pMeasData->GetInstrument() != CDS_HydroMod::eInstrumentUsed::TASCOPE ) )
			{
				// Write DateTime.
				if( pMeasData->GetMeasDateTime() != 0 )
				{
					__time32_t t = pMeasData->GetMeasDateTime();
					str = _T( "" );

					if( t >= 0 )
					{
						CTimeUnic dtu( ( __time64_t )t );
						str = dtu.FormatGmt( IDS_DATE_FORMAT ) + CString( _T(" ") ) + dtu.FormatGmt( IDS_TIME_FORMAT );
					}

					_SetStaticText( CD_Measurement_DateTime, lCurRow, str );
				}

				// Write the water characteristic.
				if( CDS_HydroModX::eQMtype::QMTemp == pMeasData->GetQMType() || CDS_HydroModX::eQMtype::QMundef == pMeasData->GetQMType() )
				{
					_SetStaticText( CD_Measurement_WaterChar, lCurRow, L"" );
				}
				else
				{
					CWaterChar *pWC = pMeasData->GetpWC();
					ASSERT( NULL != pWC );

					if( NULL != pWC )
					{
						_SetStaticText( CD_Measurement_WaterChar, lCurRow, WriteCUDouble( _U_TEMPERATURE, pWC->GetTemp() ) );
					}
				}

				// Write Valve.
				CDB_TAProduct *pTAP = pMeasData->GetpTAP();

				if( NULL != pTAP )
				{
					_SetStaticText( CD_Measurement_Valve, lCurRow, pTAP->GetName() );
				}

				// Write Presetting.
				if( CDS_HydroMod::eInstrumentUsed::TACBI == pMeasData->GetInstrument() ) // If it is from a CBI
				{
					if( pHM->GetPresetting() > 0.0 )
					{
						str = pTAP->GetValveCharacteristic()->GetSettingString( pHM->GetPresetting() );
						_SetStaticText( CD_Measurement_Presetting, lCurRow, str );
					}
				}
				else if( pMeasData->GetDesignOpening() > 0 )
				{
					_SetStaticText( CD_Measurement_Presetting, lCurRow, pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetDesignOpening() ) );
				}

				// Write Setting.
				if( pMeasData->GetCurOpening() > 0.0 )
				{
					_SetStaticText( CD_Measurement_Setting, lCurRow, pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetCurOpening() ) );
				}

				// Write Kv.
				_SetStaticText( CD_Measurement_Kv, lCurRow, pMeasData->GetstrKv().c_str() );

				// Write Kv Signal.
				CDB_TAProduct *pTAProd = pMeasData->GetpTAP();

				if( NULL != pTAProd && pTAProd->IsKvSignalEquipped() && -1.0 != pTAProd->GetKvSignal() )
				{
					_SetStaticText( CD_Measurement_KvSignal, lCurRow, WriteCUDouble( _C_KVCVCOEFF, pTAProd->GetKvSignal() ) );
				}

				// Write information depending on the QMtype.
				if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMPower ) )
				{
					// Write Meas. power.
					_SetStaticText( CD_Measurement_Power, lCurRow, WriteCUDouble( _U_TH_POWER, pMeasData->GetPower() ) );
				}

				if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMPower ) ||
					true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMFlow ) )
				{
					// Write Meas. Dp.
					_SetStaticText( CD_Measurement_MeasureDp, lCurRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetMeasDp() ) );

					// Write Design Dp.
					if( CDS_HydroMod::eInstrumentUsed::TACBI == pMeasData->GetInstrument() ) // If it is from a CBI
					{
						if( 0.0 != pHM->GetDpDesign() )
						{
							str = WriteCUDouble( _U_DIFFPRESS, pHM->GetDpDesign() );
							_SetStaticText( CD_Measurement_DesignDp, lCurRow, str );
						}
					}
					else
					{
						double dDp = pMeasData->GetDesignDp();

						if( dDp > 0.0 )
						{
							_SetStaticText( CD_Measurement_DesignDp, lCurRow, WriteCUDouble( _U_DIFFPRESS, dDp ) );
						}
					}

					// Write Meas. Flow.
					if( pMeasData->GetMeasFlow() != 0.0 )
					{
						_SetStaticText( CD_Measurement_MeasureFlow, lCurRow, WriteCUDouble( _U_FLOW, pMeasData->GetMeasFlow() ) );
					}

					// Write Flow Deviation.
					if( pMeasData->GetMeasFlow() != 0.0 && pMeasData->GetDesignQ() != 0.0 )
					{
						str = WriteDouble( pMeasData->GetFlowRatio() - 100, 3, 0 );

						// Special treatment for "0.0".
						if( 0 == IDcmp( str, L"-0.0" ) || 0 == IDcmp( str, L"0.0" ) )
						{
							str = L"0";
						}

						_SetStaticText( CD_Measurement_FlowDeviation, lCurRow, str );
					}
				}

				for( int j = CDS_HydroModX::DTSFIRST; j < CDS_HydroModX::LastDTS; j++ )
				{
					double dT = pMeasData->GetT( ( CDS_HydroModX::eDTS )j );

					if( dT > -273.15 )
					{
						str = WriteCUDouble( _U_TEMPERATURE, dT );
					}
					else
					{
						str = _T( "" );
					}

					long lColumnID = -1;

					switch( j )
					{
						case CDS_HydroModX::DTS2onDPS:
							lColumnID = CD_Measurement_Temp2DPS;
							break;

						case CDS_HydroModX::DTS1onDPS:
							lColumnID = CD_Measurement_Temp1DPS;
							break;

						case CDS_HydroModX::DTSonHH:
							lColumnID = CD_Measurement_TempHH;
							break;

						case CDS_HydroModX::DTSRef:
							lColumnID = CD_Measurement_TempRef;
							break;

						default:
							lColumnID = -1;
							break;
					}

					if( -1 != lColumnID )
					{
						_SetStaticText( lColumnID, lCurRow, str );
					}
				}

				bool fDisplayDT = ( pMeasData->GetUsedDTSensor( 0 ) > CDS_HydroModX::DTSUndef && pMeasData->GetT( pMeasData->GetUsedDTSensor( 0 ) ) > -273.15 &&
									pMeasData->GetUsedDTSensor( 1 ) > CDS_HydroModX::DTSUndef && pMeasData->GetT( pMeasData->GetUsedDTSensor( 1 ) ) > -273.15 );

				if( true == fDisplayDT )
				{
					// Write Meas. DT.
					double dDT = abs( pMeasData->GetT( pMeasData->GetUsedDTSensor( 0 ) ) - pMeasData->GetT( pMeasData->GetUsedDTSensor( 1 ) ) );
					_SetStaticText( CD_Measurement_DiffTemp, lCurRow, WriteCUDouble( _U_DIFFTEMP, dDT ) );
				}

				if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMDp ) )
				{
					// Write Meas. Dp.
					_SetStaticText( CD_Measurement_Dp, lCurRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetMeasDp() ) );
				}

				if( true == pMeasData->IsQMType( CDS_HydroModX::eQMtype::QMDpl ) )
				{
					// Write Meas. Dpl.
					_SetStaticText( CD_Measurement_Dpl, lCurRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetMeasDpL() ) );
				}

				// Write Design Flow.
				_SetStaticText( CD_Measurement_DesignFlow, lCurRow, WriteCUDouble( _U_FLOW, pMeasData->GetDesignQ() ) );
			}
		}

		// Increment the row to create a blank line between PV and children
		if( pHM == m_pHM )
		{
			if( NULL != m_pHM->GetpSchcat() && true == m_pHM->GetpSchcat()->IsSecondarySideExist() )
			{
				if( true == pMod->fSecondarySide )
				{
					lCurRow++;
				}
			}
			else
			{
				lCurRow++;
			}
		}

		lCurRow++;
	}

	SetCellBorder( CD_Measurement_DateTime, lCurRow - 1, CD_Measurement_PowerDeviation, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );

	// Set the water char in the subtitle if possible
	_SetWaterCharSubTitle( CD_Measurement_WaterChar );

	// Set Min, Max flow deviation in the SubTitle if possible
	_SetMinMaxFlowDevInSubTitle( CD_Measurement_FlowDeviation );

	if( true == bKvSignalExist && false == bSettingExist )
	{
		// If at least one valve with KvSignal exist but there isn't any valve with setting (or/and presetting) we hide the column.
		ShowCol( CD_Measurement_Presetting, FALSE );
		ShowCol( CD_Measurement_Setting, FALSE );
	}
	else if( true == bSettingExist && false == bKvSignalExist )
	{
		// If at least one valve with setting exist (or/and presetting) but there isn't any valve with KvSignal we hide the column.
		ShowCol( CD_Measurement_KvSignal, FALSE );
	}
	else if( false == bKvSignalExist && false == bSettingExist )
	{
		// If there is no valve we let the preferences on the 'CD_Measurement_Presetting' and 'CD_Measurement_Setting' columns.
		ShowCol( CD_Measurement_KvSignal, FALSE );
	}

	// Hide empty columns
	_HideEmptyColumns();

	// Check now if we can hide collapse button or not
	VerifyExpandCollapseColumnButtons( CRect( CD_Measurement_DateTime, RD_Header_FirstCirc, CD_Measurement_PowerDeviation, lCurRow - 1 ), RD_Header_ButDlg );

	// Check if last column is a separator
	// Remarks: in this case, we must set 'CellCantBePainted' column property to true to
	// avoid selection to be drawn on this last column.
	CheckLastColumnFlagCantBePainted( RD_Header_ButDlg );

	// Check if there is no successive separators.
	_HideSuccessiveSeparator( CD_Measurement_DescriptionSep, CD_Measurement_PowerDeviation, RD_Header_FirstCirc, SheetDescription::SMeasurement );

	// Free memory
	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		sMod *pMod = ( sMod * )lparam;
		delete pMod;
	}

	HMList.PurgeAll();
}

void CSheetHMCalc::_FillTADiagnosticSheet()
{
	if( NULL == m_pHM )
	{
		return;
	}

	// Draw border for the Picture row
	SetCellBorder( CD_TADiagnostic_DateTime, RD_Header_Picture, CD_TADiagnostic_Description, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_TADiagnostic_WaterChar, RD_Header_Picture, CD_TADiagnostic_DesignFlow, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_TADiagnostic_Dp1, RD_Header_Picture, CD_TADiagnostic_FlowDeviation, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_TADiagnostic_CircuitDpMeas, RD_Header_Picture, CD_TADiagnostic_DistribDpDesign, RD_Header_Picture, true, SS_BORDERTYPE_BOTTOM,
				   SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Variables
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	ASSERT( pUnitDB );
	long lCurRow = RD_Header_FirstCirc;
	long lHMRows = lCurRow;
	CString str, str1, str2;
	CDS_HydroMod *pHM = NULL;
	LPARAM lparam = (LPARAM)0;
	CRank HMList( false );
	int iCountHM = 0;
	struct sMod
	{
		CDS_HydroMod *pHM;
		bool bSecondarySide;
	};

	// Create a list with all circuits included in m_pHM and count the number of lines
	for( IDPTR IDPtr = m_pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = m_pHM->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)IDPtr.MP;

		for( int i = 0; i < 1; i++ )
		{
			sMod *pMod = new sMod;
			pMod->pHM = pHM;
			pMod->bSecondarySide = ( NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsSecondarySideExist() && i == 1 );
			HMList.Add( str, pHM->GetPos() + ( ( true == pMod->bSecondarySide ) ? 0.1 : 0.0 ), (LPARAM)pMod );
			lHMRows += 1;	// 1 row by circuit
			++iCountHM;

			// Abort loop if secondary side doesn't exist.
			if( NULL == pHM->GetpSchcat() || false == pHM->GetpSchcat()->IsSecondarySideExist() )
			{
				break;
			}
		}
	}

	// Parent module at the first position.
	sMod *pMod = new sMod;

	pMod->pHM = m_pHM;
	pMod->bSecondarySide = false;
	HMList.Add( str, 0, (LPARAM)pMod );

	if( NULL != m_pHM->GetpSchcat() && true == m_pHM->GetpSchcat()->IsSecondarySideExist() )
	{
		sMod *pMod = new sMod;
		pMod->pHM = m_pHM;
		pMod->bSecondarySide = true;
		HMList.Add( str, 0.1, (LPARAM)pMod );
		lHMRows += 1;
	}

	// +1 row for parent module.
	lHMRows += 1;

	// Redefine the max rows.
	SetMaxRows( lHMRows );

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SheetDescription::STADiagnostic );

		if( NULL != pclSheetDescription )
		{
			pclSheetDescription->Init( 1, GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
			pclSheetDescription->SetSelectableRangeRow( RD_Header_FirstCirc, GetMaxRows() );
		}
	}

	// Do a loop on the list and write the info.
	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		ASSERT( lCurRow <= lHMRows );
		sMod *pMod = (sMod *)lparam;

		if( NULL == pMod )
		{
			continue;
		}

		pHM = dynamic_cast<CDS_HydroMod *>( pMod->pHM );

		if( NULL == pHM )
		{
			continue;
		}

		CWaterChar *pclWaterChar = NULL;
		CDB_TAProduct *pTAProd = (CDB_TAProduct *)( pHM->GetTADBValveIDPtr().MP );

		if( false == pMod->bSecondarySide )
		{
			if( NULL == pHM->GetpBv() && NULL != pHM->GetpCV() )
			{
				if( NULL != (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP ) )
				{
					pTAProd	= (CDB_TAProduct *)( pHM->GetpCV()->GetCvIDPtr().MP );
					pclWaterChar = pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eCV );
				}
			}
			else if( NULL != pHM->GetpBv() )
			{
				if( NULL != (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP ) )
				{
					pTAProd	= (CDB_TAProduct *)( pHM->GetpBv()->GetIDPtr().MP );
					pclWaterChar = pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eBVprim );
				}
			}
			else if( NULL != pHM->GetpSmartControlValve() )
			{
				// Smart valve must be added when available in the TA-Scope.
				ASSERT_RETURN;
			}
		}
		else// Secondary side
		{
			if( NULL != pHM->GetpSecBv() )
			{
				if( NULL != (CDB_TAProduct *)( pHM->GetpSecBv()->GetIDPtr().MP ) )
				{
					pTAProd	 = (CDB_TAProduct *)( pHM->GetpSecBv()->GetIDPtr().MP );
					pclWaterChar = pHM->GetpWaterChar( CDS_HydroMod::eHMObj::eBVsec );
				}
			}
		}

		// Prepare color to use in regards to the current row is in cut mode or not.
		bool bIsCutMode = _PrepareColors( pHM );

		// Redefine the TextPattern.
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn, bIsCutMode );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Allow cells to be split in two parts.
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

		// If it's the partner valve...
		if( pHM == m_pHM )
		{
			bool bDrawBorder = false;

			if( NULL != m_pHM->GetpSchcat() && true == m_pHM->GetpSchcat()->IsSecondarySideExist() )
			{
				if( true == pMod->bSecondarySide )
				{
					bDrawBorder = true;
				}
			}
			else
			{
				bDrawBorder = true;
			}

			COLORREF linecolor = ( true == bDrawBorder ) ? SPREAD_COLOR_NONE : _LIGHTGRAY;

			// Draw border for the partner valve.
			SetCellBorder( CD_TADiagnostic_DateTime, lCurRow, CD_TADiagnostic_Description, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );
			SetCellBorder( CD_TADiagnostic_WaterChar, lCurRow, CD_TADiagnostic_DesignFlow, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );
			SetCellBorder( CD_TADiagnostic_Dp1, lCurRow, CD_TADiagnostic_FlowDeviation, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );
			SetCellBorder( CD_TADiagnostic_CircuitDpMeas, lCurRow, CD_TADiagnostic_DistribDpDesign, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, linecolor );

			// Is this a sub module...
			if( 0 != pHM->GetLevel() && false == pMod->bSecondarySide && false == m_bPrinting && false == m_bExporting )
			{
				SetPictureCellWithID( IDI_ARROWBACK, CD_TADiagnostic_Sub, lCurRow, CSSheet::PictureCellType::Icon );
			}

			// Change background color for parent module
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );

			for( int lLoopColumn = CD_TADiagnostic_DateTime; lLoopColumn < CD_TADiagnostic_Pointer; lLoopColumn++ )
			{
				switch( lLoopColumn )
				{
					case CD_TADiagnostic_DescriptionSep:
					case CD_TADiagnostic_DesignFlowSep:
					case CD_TADiagnostic_FlowDeviationSep:
						continue;
						break;

					default:
						_SetStaticText( lLoopColumn, lCurRow, _T( "" ) );
						break;
				}
			}
		}
		else
		{
			// Draw border.
			SetCellBorder( CD_TADiagnostic_DateTime, lCurRow, CD_TADiagnostic_Description, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( CD_TADiagnostic_WaterChar, lCurRow, CD_TADiagnostic_DesignFlow, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( CD_TADiagnostic_Dp1, lCurRow, CD_TADiagnostic_FlowDeviation, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( CD_TADiagnostic_CircuitDpMeas, lCurRow, CD_TADiagnostic_DistribDpDesign, lCurRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );

			// Change background color for module valves.
			if( true == pMod->bSecondarySide )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg2nd );
			}
			else
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ModuleCircuitBkg );
			}

			// Draw Icon module.
			if( true == pHM->IsaModule() && false == pMod->bSecondarySide )
			{
				SetPictureCellWithID( IDI_HTREE, CD_TADiagnostic_Sub, lCurRow, CSSheet::PictureCellType::Icon );
			}

		}

		// Save pointer on HM (last column).
		if( false == m_bExporting && false == m_bPrinting )
		{
			SetCellParam( CD_TADiagnostic_Pointer, lCurRow, ( long )pHM );
		}

		// Write position.
		TCHAR buf[10];

		if( pHM == m_pHM )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
		}

		if( true == pMod->bSecondarySide )
		{
			_stprintf_s( buf, SIZEOFINTCHAR( buf ), _T( "" ) );
		}
		else
		{
			_stprintf_s( buf, SIZEOFINTCHAR( buf ), _T("%d"), pHM->GetPos() );
		}

		if( true == pHM->IsCircuitIndex() )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_IndexFore );
		}

		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		_SetStaticText( CD_TADiagnostic_Pos, lCurRow, buf );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_NormalFore );

		// Write reference.
		if( false == pMod->bSecondarySide )
		{
			_SetStaticText( CD_TADiagnostic_Reference, lCurRow, pHM->GetHMName() );
		}

		// Write description.
		_SetStaticText( CD_TADiagnostic_Description, lCurRow, pHM->GetDescription() );

		// Write water characteristics.
		if( NULL != pclWaterChar )
		{
			_SetStaticText( CD_TADiagnostic_WaterChar, lCurRow, WriteCUDouble( _U_TEMPERATURE, pclWaterChar->GetTemp() ) );
		}

		// Write Valve.
		if( edt_TADBValve == pHM->GetVDescrType() )
		{
			str = _T( "" );

			if( NULL != pTAProd )
			{
				str = pTAProd->GetName();
			}
		}
		else if( edt_KvCv == pHM->GetVDescrType() )
		{
			if( NULL != pUnitDB )
			{
				if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				{
					str = TASApp.LoadLocalizedString( IDS_KV );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_CV );
				}

				str += ( CString )_T(" = ") + WriteDouble( CDimValue::SItoCU( _C_KVCVCOEFF, pHM->GetKvCv() ), 3, 0 );

				if( false == str.IsEmpty() )
				{
					str += ( CString )_T("*");
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_NotTAFore );
				}
			}
		}
		else
		{
			str = pHM->GetCBIType() + ( CString )_T(" ") + pHM->GetCBISize();
			str.TrimLeft();

			if( false == str.IsEmpty() )
			{
				str += ( CString )_T("*");
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_NotTAFore );
			}
		}

		_SetStaticText( CD_TADiagnostic_Valve, lCurRow, str );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)m_rFillColors.m_NormalFore );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

		// Write Presetting (if a presetting exist in the CMeasData it will be overwritten).
		if( false == pMod->bSecondarySide )
		{
			if( pHM->GetPresetting() > 0.0 )
			{
				str = pTAProd->GetValveCharacteristic()->GetSettingString( pHM->GetPresetting() );
				_SetStaticText( CD_TADiagnostic_Presetting, lCurRow, str );
			}

			if( NULL != pHM->GetpBv() && L"" != pHM->GetpBv()->GetSettingStr() )
			{
				_SetStaticText( CD_TADiagnostic_Presetting, lCurRow, pHM->GetpBv()->GetSettingStr() );
			}
			else if( NULL != pHM->GetpCV() && L"" != pHM->GetpCV()->GetSettingStr() )
			{
				_SetStaticText( CD_TADiagnostic_Presetting, lCurRow, pHM->GetpCV()->GetSettingStr() );
			}
		}
		else if( NULL != pHM->GetpSecBv() && L"" != pHM->GetpSecBv()->GetSettingStr() )
		{
			_SetStaticText( CD_TADiagnostic_Presetting, lCurRow, pHM->GetpSecBv()->GetSettingStr() );
		}

		// Write Kv (if a meas exist, re-enter the Kv value).
		if( NULL != pTAProd && NULL != pTAProd->GetValveCharacteristic() && 0.0 != pHM->GetPresetting() && -1.0 != pHM->GetPresetting() )
		{
			double dKv = pTAProd->GetValveCharacteristic()->GetKv( pHM->GetPresetting() );
			CString str = ( -1.0 != dKv ) ? WriteCUDouble( _C_KVCVCOEFF, dKv ) : TASApp.LoadLocalizedString( IDS_ERROR );
			_SetStaticText( CD_TADiagnostic_Kv, lCurRow, str );
		}

		// Write Kv Signal (if a meas exist, re-enter the Kv signal value)
		if( NULL != pTAProd && true == pTAProd->IsKvSignalEquipped() && -1.0 != pTAProd->GetKvSignal() )
		{
			_SetStaticText( CD_TADiagnostic_KvSignal, lCurRow, WriteCUDouble( _C_KVCVCOEFF, pTAProd->GetKvSignal() ) );
		}

		// Define the CMeasData to show (this will be the last that fit the boundaries)
		bool bAtLeastOneMeasFit = false;
		unsigned int iMeasDataToShow = 0;
		COleDateTime timLast;
		unsigned int uiNbrMeasData = pHM->GetMeasDataSize();

		for( unsigned int i = 0; i < uiNbrMeasData; i++ )
		{
			// Get the MeasData
			CDS_HydroMod::CMeasData *pMeasData = pHM->GetpMeasData( i );

			if( NULL == pMeasData )
			{
				continue;
			}

			// HYS-1734: CDS_HydroModX::CMeasData::GetLocate renamed to GetPipeLocation.
			if( true == pMod->bSecondarySide && CDS_HydroModX::eLocate::InSecondary != pMeasData->GetPipeLocation() )
			{
				continue;
			}

			if( false == pMod->bSecondarySide && CDS_HydroModX::eLocate::InPrimary != pMeasData->GetPipeLocation() )
			{
				continue;
			}

			// In case we have measurements from the CBI, no date exist, it will always be shown
			if( pMeasData->GetInstrument() == CDS_HydroMod::TACBI )
			{
				iMeasDataToShow = i;
				bAtLeastOneMeasFit = true;
				continue;
			}

			__time32_t time32 = pMeasData->GetTABalDateTime();

			if( 0 >= time32 )
			{
				time32 = pMeasData->GetMeasDateTime();    // For backward compatibility
			}

			if( 0 >= time32 && true == pMeasData->GetFlagTaBalMeasCompleted() )
			{
				iMeasDataToShow = i;
				bAtLeastOneMeasFit = true;
				continue;
			}

			if( 0 >= time32 )
			{
				continue;
			}

			tm dt;
			_gmtime32_s( &dt, &time32 );
			COleDateTime tim( dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec );

			// In case the Measure is outside the limits
			if( tim < pMainFrame->GetLowerDateTime() || tim > pMainFrame->GetUpperDateTime() )
			{
				continue;
			}

			bAtLeastOneMeasFit = true;

			if( tim > timLast )
			{
				timLast = tim;
				iMeasDataToShow = i;
			}
		}

		// Show the last CMeasData.
		if( uiNbrMeasData > 0 && true == bAtLeastOneMeasFit )
		{
			// Get the MeasData
			CDS_HydroMod::CMeasData *pMeasData = pHM->GetpMeasData( iMeasDataToShow );

			if( NULL != pMeasData )
			{
				__time32_t t = pMeasData->GetTABalDateTime();

				if( 0 == t )
				{
					t = pMeasData->GetMeasDateTime();    // For backward compatibility
				}

				// Write DateTime
				if( t != 0 )
				{
					CTimeUnic dtu( ( __time64_t )t );
					str = dtu.FormatGmt( IDS_DATE_FORMAT ) + CString( _T(" ") ) + dtu.FormatGmt( IDS_TIME_FORMAT );
					_SetStaticText( CD_TADiagnostic_DateTime, lCurRow, str );
				}

				// Write Water Char.
				CWaterChar *pWC = pMeasData->GetpWC();
				ASSERT( NULL != pWC );

				if( NULL != pWC )
				{
					_SetStaticText( CD_TADiagnostic_WaterChar, lCurRow, WriteCUDouble( _U_TEMPERATURE, pWC->GetTemp() ) );
				}

				if( edt_TADBValve == pHM->GetVDescrType() )
				{
					// Write Valve
					CDB_TAProduct *pTAP = pMeasData->GetpTAP();

					if( NULL != pTAP )
					{
						_SetStaticText( CD_TADiagnostic_Valve, lCurRow, pTAP->GetName() );
					}

					// Write Presetting.
					if( CDS_HydroMod::eInstrumentUsed::TACBI == pMeasData->GetInstrument() ) // If it is from a CBI
					{
						if( pHM->GetPresetting() != 0.0 && pHM->GetPresetting() != -1.0 )
						{
							str = pTAP->GetValveCharacteristic()->GetSettingString( pHM->GetPresetting() );
							_SetStaticText( CD_TADiagnostic_Presetting, lCurRow, str );
						}
					}
					else
					{
						_SetStaticText( CD_TADiagnostic_Presetting, lCurRow, pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetDesignOpening() ) );
					}

					// Write Setting.
					if( pMeasData->GetCurOpening() > 0.0 )
					{
						_SetStaticText( CD_TADiagnostic_Setting, lCurRow, pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetCurOpening() ) );
					}

					// Write computed setting.
					if( pMeasData->GetTaBalOpeningResult() > 0.0 )
					{
						_SetStaticText( CD_TADiagnostic_ComputedSetting, lCurRow, pTAP->GetValveCharacteristic()->GetSettingString( pMeasData->GetTaBalOpeningResult() ) );
					}
				}

				// Write Kv.
				_SetStaticText( CD_TADiagnostic_Kv, lCurRow, pMeasData->GetstrKv().c_str() );

				// Write Kv Signal.
				CDB_TAProduct *pTAProd = pMeasData->GetpTAP();

				if( NULL != pTAProd && true == pTAProd->IsKvSignalEquipped() && -1.0 != pTAProd->GetKvSignal() )
				{
					_SetStaticText( CD_TADiagnostic_KvSignal, lCurRow, WriteCUDouble( _C_KVCVCOEFF, pTAProd->GetKvSignal() ) );
				}

				// Write Design Flow.
				_SetStaticText( CD_TADiagnostic_DesignFlow, lCurRow, WriteCUDouble( _U_FLOW, pMeasData->GetDesignQ() ) );

				if( pHM != m_pHM )
				{
					// Write Dp1.
					_SetStaticText( CD_TADiagnostic_Dp1, lCurRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetTaBalMeasDp_1() ) );

					// Write setting.
					_SetStaticText( CD_TADiagnostic_Setting1, lCurRow, pTAProd->GetValveCharacteristic()->GetSettingString( pMeasData->GetTaBalOpening_1() ) );
				}
				else
				{
					// Write Dp1.
					_SetStaticText( CD_TADiagnostic_Dp1, lCurRow, GetDashDotDash() );

					// Write Setting 1.
					_SetStaticText( CD_TADiagnostic_Setting1, lCurRow, GetDashDotDash() );
				}

				// Write Dp2.
				_SetStaticText( CD_TADiagnostic_Dp2, lCurRow, WriteCUDouble( _U_DIFFPRESS, pMeasData->GetTaBalMeasDp_2() ) );

				// Write Measuring flow.
				if( pMeasData->GetTaBalOpening_1() != 0 && pMeasData->GetTaBalMeasDp_1() != 0 )
				{
					double dFlow =  pMeasData->GetFlow( pMeasData->GetTaBalOpening_1(), pMeasData->GetTaBalMeasDp_1() );

					_SetStaticText( CD_TADiagnostic_MeasFlow, lCurRow, WriteCUDouble( _U_FLOW, dFlow ) );

					// Write flow deviation.
					if( dFlow != 0 && pMeasData->GetDesignQ() != 0.0 )
					{
						double dFlowRatio = ( dFlow / pMeasData->GetDesignQ() * 100 );
						str = WriteDouble( dFlowRatio - 100, 3, 0 );

						// Special treatment for "0.0".
						if( NULL == IDcmp( str, L"-0.0" ) || NULL == IDcmp( str, L"0.0" ) )
						{
							str = L"0";
						}

						_SetStaticText( CD_TADiagnostic_FlowDeviation, lCurRow, str );
					}
				}

				// Write Circuit DP & Measured.
				// Remarks: do nothing with partner valve.
				if( pHM != m_pHM )
				{
					// Measuring.
					CString cstrMeas = ( pMeasData->GetCircDpComputed() > 0 ) ? WriteCUDouble( _U_DIFFPRESS, pMeasData->GetCircDpComputed() ) : _T("0.0");

					// Design.
					CString cstrDesign;
					CHMXPipe *pPipeCirc = pMeasData->GetpCircuitPipe();
					CHMXTU *pTermUnit = pMeasData->GetpTerminalUnit();

					//if( NULL == pPipeCirc || NULL == pTermUnit)
					//	cstrDesign = GetDashDotDash();
					if( NULL != pPipeCirc && pPipeCirc->GetFlow() > 0 )
					{
						double dTotalDp = 0.0;

						if( NULL != pPipeCirc )
						{
							dTotalDp = pPipeCirc->GetTotalDp();
						}

						if( NULL != pTermUnit )
						{
							dTotalDp += pTermUnit->GetDp();
						}

						cstrDesign = WriteCUDouble( _U_DIFFPRESS, dTotalDp );
					}
					else
					{
						cstrDesign = GetDashDotDash();
					}


					_SetStaticText( CD_TADiagnostic_CircuitDpMeas, lCurRow, cstrMeas );
					_SetStaticText( CD_TADiagnostic_CircuitDpDesign, lCurRow, cstrDesign );
				}

				// Picture (arrow).
				if( pHM != m_pHM )
				{
					AddCellSpanW( CD_TADiagnostic_Picture, lCurRow - 1, 1, 1 );

					if( false == m_bPrinting && false == m_bExporting  )
					{
						SetPictureCellWithID( IDI_HMCALCRARROW, CD_TADiagnostic_Picture, lCurRow, CSSheet::PictureCellType::Icon );
					}
				}

				// Write Distribution DP Computed & Measured.
				if( pHM != m_pHM )
				{
					if( 1 == pHM->GetPos() || iCountHM == pHM->GetPos() )
					{
						// Measuring.
						_SetStaticText( CD_TADiagnostic_DistribDpMeas, lCurRow, GetDashDotDash() );

						// Design.
						CString cstrDesign;
						CHMXPipe *pPipeDistr = pMeasData->GetpDistributionPipe();

						if( NULL != pPipeDistr && pPipeDistr->GetFlow() != 0 )
						{
							cstrDesign = WriteCUDouble( _U_DIFFPRESS, pPipeDistr->GetTotalDp() );
						}
						else
						{
							cstrDesign = GetDashDotDash();
						}

						_SetStaticText( CD_TADiagnostic_DistribDpDesign, lCurRow, cstrDesign );
					}
					else
					{
						CString cstrMeas = ( pMeasData->GetDistribDpComputed() > 0 ) ? WriteCUDouble( _U_DIFFPRESS, pMeasData->GetDistribDpComputed() ) : _T("0.0");

						// Design.
						CString cstrDesign;
						CHMXPipe *pPipeDistr = pMeasData->GetpDistributionPipe();

						if( NULL != pPipeDistr && pPipeDistr->GetFlow() != 0 )
						{
							cstrDesign = WriteCUDouble( _U_DIFFPRESS, pPipeDistr->GetTotalDp() );
						}
						else
						{
							cstrDesign = GetDashDotDash();
						}

						_SetStaticText( CD_TADiagnostic_DistribDpMeas, lCurRow, cstrMeas );
						_SetStaticText( CD_TADiagnostic_DistribDpDesign, lCurRow, cstrDesign );
					}
				}
			}
		}

		// Increment the row to create a blank line between PV and children
		if( pHM == m_pHM )
		{
			if( NULL != m_pHM->GetpSchcat() && true == m_pHM->GetpSchcat()->IsSecondarySideExist() )
			{
				if( true == pMod->bSecondarySide )
				{
					lCurRow++;
				}
			}
			else
			{
				lCurRow++;
			}
		}

		lCurRow++;
	}

	SetCellBorder( CD_TADiagnostic_DateTime, lCurRow - 1, CD_TADiagnostic_DistribDpDesign, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );
	SetCellBorder( CD_TADiagnostic_DateTime, lCurRow - 1, CD_TADiagnostic_FlowDeviation, lCurRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID,
				   SPREAD_COLOR_NONE );

	// Set the water char in the subtitle if possible.
	_SetWaterCharSubTitle( CD_TADiagnostic_WaterChar );

	// Set Min, Max flow deviation in the SubTitle if possible.
	_SetMinMaxFlowDevInSubTitle( CD_TADiagnostic_FlowDeviation );

	// Hide empty columns.
	_HideEmptyColumns();

	// Check now if we can hide collapse button or not.
	VerifyExpandCollapseColumnButtons( CRect( CD_TADiagnostic_DateTime, RD_Header_FirstCirc, CD_TADiagnostic_DistribDpDesign, lCurRow - 1 ), RD_Header_ButDlg );

	// Check if last column is a separator.
	// Remarks: in this case, we must set 'CellCantBePainted' column property to true to
	// avoid selection to be drawn on this last column.
	CheckLastColumnFlagCantBePainted( RD_Header_ButDlg );

	// Check if there is no successive separators.
	_HideSuccessiveSeparator( CD_TADiagnostic_DescriptionSep, CD_TADiagnostic_DistribDpDesign, RD_Header_FirstCirc, SheetDescription::STADiagnostic );

	// Free memory.
	for( BOOL bContinue = HMList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = HMList.GetNext( str, lparam ) )
	{
		sMod *pMod = ( sMod * )lparam;
		delete pMod;
	}

	HMList.PurgeAll();
}

void CSheetHMCalc::_ChangeBv( CDS_HydroMod *pHM, CDS_HydroMod::CBV *pBV, CDS_HydroMod::CBV *pSrcBv )
{
	CDB_TAProduct *pTAP = pBV->GetpTAP();
	unsigned short usLocker = pHM->GetLock();

	if( false == pBV->ForceBVSelection( pSrcBv->GetIDPtr() ) )
	{
		CDlgHMCompilationOutput::CHMInterface clOutputInterface;

		// Force selection failed, drop a message in HCO and return to previous choice
		if( NULL == pTAP )
		{
			return;
		}

		// Unlock the valve to force return to default selection
		pBV->ForceBVSelection( pTAP->GetIDPtr() );
		pHM->SetLock( usLocker );
		CString str = TASApp.LoadLocalizedString( IDS_SOLMSG_HMDD_CHECKDEVSIZE );
		clOutputInterface.FillAndSendMessage( IDS_WARNMSG_HMDD_DEVNOTFIT, CDlgHMCompilationOutput::Warning, pHM, str, pSrcBv->GetpTAP()->GetName() );
	}
}

bool CSheetHMCalc::_SaveAndCompute( long lColumn, long lRow, bool bComputeAll )
{
	// TODO verify bounds for new values and for DD.
	BeginWaitCursor();
	double dSI = 0.0;
	CDS_HydroMod *pHMDest = GetpHM( lRow );

	if( NULL == pHMDest )
	{
		EndWaitCursor();
		return false;
	}

	if( true == pHMDest->IsForHub() )
	{
		EndWaitCursor();
		return false;
	}

	// Access to HCO if needed
	CDlgHMCompilationOutput::CHMInterface clOutputInterface;

	// Disable chain computing if needed.
	bool bHMTreeResetNeeded = false;

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		switch( m_lLastEditedCol )
		{
			// Circuit.
			case CD_Distribution_Name:
				bHMTreeResetNeeded = _SetHMName( lRow );
				bComputeAll = false;
				break;

			case CD_Distribution_Desc:
			{
				pHMDest->SetDescription( m_strCellText );
				bComputeAll = false;
				// Update description on the three sheets.
				// Unselect row before processing.
				bool bRowSelected = GetSelectedRows() ? 1 : 0;
				UnSelectMultipleRows();
				_SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuit ) );
				SetCellText( CD_Circuit_Desc, lRow, m_strCellText );
				_SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuitInj ) );
				SetCellText( CD_CircuitInjSecSide_Desc, lRow, m_strCellText );

				// Return to the original sheet.
				_SetCurrentSheet( GetSheetNumber( SheetDescription::SDistribution ) );

				if( true == bRowSelected )
				{
					_SelectRow( lRow );
				}
			}
			break;

			case CD_Distribution_SupplyPipeSeries:
				{
					// Drag and drop of a combo; m_CellLParam contains original pipe * to be copied.
					CPipes *pSrcPipe = (CPipes *)m_lpCellParam;
					ASSERT( NULL != pSrcPipe );

					pHMDest->GetpDistrSupplyPipe()->SetPipeSeries( pSrcPipe->GetPipeSeries() );
					pHMDest->GetpDistrSupplyPipe()->SetLock( false );
				}
				break;

			case CD_Distribution_ReturnPipeSeries:
				{
					// Drag and drop of a combo; m_CellLParam contains original pipe * to be copied.
					CPipes *pSrcPipe = (CPipes *)m_lpCellParam;
					ASSERT( NULL != pSrcPipe );

					pHMDest->GetpDistrReturnPipe()->SetPipeSeries( pSrcPipe->GetPipeSeries() );
					pHMDest->GetpDistrReturnPipe()->SetLock( false );
				}
				break;

			case CD_Distribution_SupplyPipeSize:
				{
					// Drag and drop of a combo; m_CellLParam contains original pipe * to be copied.
					CPipes *pSrcPipe = (CPipes *)m_lpCellParam;
					ASSERT( NULL != pSrcPipe );

					pHMDest->GetpDistrSupplyPipe()->SetLock( !pHMDest->GetpDistrSupplyPipe()->IsBestPipe( pSrcPipe->GetIDPtr().MP ) );
					pHMDest->GetpDistrSupplyPipe()->SetIDPtr( pSrcPipe->GetIDPtr() );
				}
				break;

			case CD_Distribution_ReturnPipeSize:
				{
					// Drag and drop of a combo; m_CellLParam contains original pipe * to be copied.
					CPipes *pSrcPipe = (CPipes *)m_lpCellParam;
					ASSERT( NULL != pSrcPipe );

					pHMDest->GetpDistrReturnPipe()->SetLock( !pHMDest->GetpDistrReturnPipe()->IsBestPipe( pSrcPipe->GetIDPtr().MP ) );
					pHMDest->GetpDistrReturnPipe()->SetIDPtr( pSrcPipe->GetIDPtr() );
				}
				break;

			case CD_Distribution_SupplyPipeLength:

				dSI = CDimValue::CUtoSI( _U_LENGTH, m_dCellDouble );
				pHMDest->GetpDistrSupplyPipe()->SetLength( dSI );
				break;

			case CD_Distribution_ReturnPipeLength:

				dSI = CDimValue::CUtoSI( _U_LENGTH, m_dCellDouble );
				pHMDest->GetpDistrReturnPipe()->SetLength( dSI );

				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		CTermUnit *pTermUnit = pHMDest->GetpTermUnit();
		ASSERT( NULL != pTermUnit );

		if( NULL == pTermUnit )
		{
			EndWaitCursor();
			return false;
		}

		switch( m_lLastEditedCol )
		{
			// Circuit name.
			case CD_Circuit_Name:
				bHMTreeResetNeeded = _SetHMName( lRow );
				bComputeAll = false;
				break;

			case CD_Circuit_Desc:
			{
				pHMDest->SetDescription( m_strCellText );
				bComputeAll = false;
				bool bRowSelected = GetSelectedRows() ? 1 : 0;

				// Unselect row before processing.
				UnSelectMultipleRows();

				_SetCurrentSheet( GetSheetNumber( SheetDescription::SDistribution ) );
				SetCellText( CD_Distribution_Desc, lRow, m_strCellText );
				_SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuitInj ) );
				SetCellText( CD_CircuitInjSecSide_Desc, lRow, m_strCellText );

				// Return to the original sheet.
				_SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuit ) );

				if( true == bRowSelected )
				{
					_SelectRow( lRow );
				}
			}
			break;

			///////////////////////////////////////////////////////////////////////////
			// Terminal Unit.
			case CD_Circuit_UnitDesc:
				pTermUnit->SetDescription( m_strCellText );
				bComputeAll = false;
				break;

			case CD_Circuit_UnitQ:
				dSI = CDimValue::CUtoSI( _U_FLOW, m_dCellDouble );
				pTermUnit->SetQ( dSI );
				break;

			case CD_Circuit_UnitP:
				dSI = CDimValue::CUtoSI( _U_TH_POWER, m_dCellDouble );
				pTermUnit->SetPdT( dSI, DBL_MAX );
				break;

			case CD_Circuit_UnitDT:
			{
				bool bSaveValue = true;
				dSI = CDimValue::CUtoSI( _U_DIFFTEMP, m_dCellDouble );
				
				// HYS-1882: SetPDT does not set the return temp anymore. We have to update it before the SetPdt function.
				if( dSI > 0 )
				{
					// HYS-1882: Set return temperature with the new DT. It is considered in SetPdt when we call GetQ() to compute flow.
					double dSupplyTemp = pHMDest->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply, CAnchorPt::CircuitSide::CircuitSide_Primary );

					ProjectType eProjectType = TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType();
					double dReturnTemp = (ProjectType::Cooling == eProjectType) ? dSupplyTemp + dSI : dSupplyTemp - dSI;

					// Verify that the new primary return temperature becomes not lower or equal to the freezing point.
					if( ProjectType::Heating == eProjectType )
					{
						double dFreezingPoint = -273.15;
						CWaterChar *pclWaterChar = m_pTADS->GetpWCForProject()->GetpWCData();

						if( NULL != pclWaterChar )
						{
							dFreezingPoint = pclWaterChar->GetTfreez();
						}

						if( dFreezingPoint != -273.15 )
						{
							if( dReturnTemp <= dFreezingPoint )
							{
								// By changing the DT to %1, the return primary temperature becomes lower or equal to the freezing point temperature (%2).

								CString str;
								CString strDT = WriteCUDouble( _U_TEMPERATURE, dSI, true );
								CString strFreezingPoint = WriteCUDouble( _U_TEMPERATURE, dFreezingPoint, true );

								FormatString( str, IDS_CIRC_TEMPERROR_DTGENERATEBADTRP, strDT, strFreezingPoint );
								AfxMessageBox( str, MB_OK | MB_ICONERROR, 0 );
								bSaveValue = false;
							}
						}
					}

					if( true == bSaveValue )
					{
						// Set primary return temperature.
						pHMDest->SetTemperature( dReturnTemp, CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return, CAnchorPt::CircuitSide::CircuitSide_Primary );
					}
				}

				if( true == bSaveValue )
				{
					if( CTermUnit::_QType::PdT == pHMDest->GetpTermUnit()->GetQType() )
					{
						pTermUnit->SetPdT( DBL_MAX, dSI );
					}
					else if( CTermUnit::_QType::Q == pHMDest->GetpTermUnit()->GetQType() )
					{
						if( 0 < dSI )
						{
							pHMDest->GetpTermUnit()->SetDTFlowMode( dSI );
						}
						else
						{
							pHMDest->GetpTermUnit()->GetDTFlowMode( true );
						}
					}
				}

				break;
			}

			case CD_Circuit_UnitDp:
				dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::Dp, dSI );
				break;

			case CD_Circuit_UnitQref:
				dSI = CDimValue::CUtoSI( _U_FLOW, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::QDpref, DBL_MAX, dSI );
				break;

			case CD_Circuit_UnitDpref:
				dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::QDpref, dSI, DBL_MAX );
				break;

			case CD_Circuit_UnitKv:
				dSI = CDimValue::CUtoSI( _C_KVCVCOEFF, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::Kv, dSI );
				break;

			case CD_Circuit_UnitCv:
				dSI = CDimValue::CUtoSI( _C_KVCVCOEFF, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::Cv, dSI );
				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Pump group.
			case CD_Circuit_PumpHUser:
				dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
				pHMDest->GetpPump()->SetHpump( dSI );
				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// CV.
			case CD_Circuit_CVDesc:
				bComputeAll = false;
				pHMDest->GetpCV()->SetDescription( m_strCellText );
				break;

			case CD_Circuit_CVKvs:

				// Cell KVs can be a combo or a editable cell.
				if( true == IsCellProperty( lColumn, lRow, CellDouble ) )
				{
					dSI = CDimValue::CUtoSI( _C_KVCVCOEFF, m_dCellDouble );
					pHMDest->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMDest->GetpCV()->IsBestCV( dSI ) );
					pHMDest->GetpCV()->SetKvs( dSI );
				}
				else if( NULL != m_lpCellParam )
				{
					CDS_HydroMod::CCv *pSrcCv = (CDS_HydroMod::CCv *)m_lpCellParam;
					ASSERT( NULL != pSrcCv );

					pHMDest->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMDest->GetpCV()->IsBestCV( pSrcCv->GetKvs() ) );
					pHMDest->GetpCV()->SetKvs( pSrcCv->GetKvs() );
				}
				else if( m_dCellDouble )	// Drag&Drop source cell is in edition mode.
				{
					if( true == pHMDest->GetpCV()->IsPresettable() )
					{
						bComputeAll = false;
						break;
					}

					pHMDest->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMDest->GetpCV()->IsBestCV( m_dCellDouble ) );
					pHMDest->GetpCV()->SetKvs( m_dCellDouble );
				}

				break;

			case CD_Circuit_CVName:
			{
				// Drag and Drop of combo; m_CellLParam contains original CV * to be copied.
				CDS_HydroMod::CCv *pSrcCv = (CDS_HydroMod::CCv *)m_lpCellParam;
				ASSERT( NULL != pSrcCv );

				pHMDest->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMDest->GetpCV()->IsBestCV( (CData *)pSrcCv->GetCvIDPtr().MP ) );

				pHMDest->GetpCV()->SetCtrlType( pSrcCv->GetCtrlType() );
				pHMDest->GetpCV()->SetCVSelectedAsaPackage( pSrcCv->GetCVSelectedAsaPackage() );
				pHMDest->GetpCV()->ForceCVSelection( pSrcCv->GetCvIDPtr() );

				// We force CV but we need also to force actuator (Or remove if not!).
				pHMDest->GetpCV()->ForceActuatorSelection( pSrcCv );
			}
			break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Actuator.
			case CD_Circuit_ActuatorName:
			{
				// Drag and Drop of combo; m_CellLParam contains original Actuator * to be copied.
				CDS_HydroMod::CCv *pSrcCv = (CDS_HydroMod::CCv *)m_lpCellParam;
				ASSERT( pSrcCv != NULL );

				CDB_Actuator *pclSrcActuator = dynamic_cast<CDB_Actuator *>( (CData *)pSrcCv->GetActrIDPtr().MP );
				pHMDest->GetpCV()->ForceActuatorSelection( pSrcCv );
			}
			break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Smart valve.
			// HYS-1676: Add smart valve to Sheet HM Calc
			// This columun is both for smart control valve and smart differential pressure controller.
			case CD_Circuit_SmartValveName:
				{
					// Drag and Drop of combo; m_CellLParam contains original smart valve * to be copied.
					CDS_HydroMod *pHM = GetpHM( lRow );
					
					if( true == pHM->IsSmartControlValveExist() )
					{
						CDS_HydroMod::CSmartControlValve *pSrcSmartControlValve = (CDS_HydroMod::CSmartControlValve *)m_lpCellParam;
						ASSERT( NULL != pSrcSmartControlValve );

						pHMDest->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, 
								!pHMDest->GetpSmartControlValve()->IsBestSmartControlValve( (CData *)pSrcSmartControlValve->GetIDPtr().MP ) );

						pHMDest->GetpSmartControlValve()->ForceSelection( pSrcSmartControlValve->GetIDPtr() );
					}
					else if( true == pHM->IsSmartDpCExist() )
					{
						CDS_HydroMod::CSmartDpC *pSrcSmartDpC = (CDS_HydroMod::CSmartDpC *)m_lpCellParam;
						ASSERT( NULL != pSrcSmartDpC );

						pHMDest->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, 
								!pHMDest->GetpSmartDpC()->IsBestValve( (CData *)pSrcSmartDpC->GetIDPtr().MP ) );

						pHMDest->GetpSmartDpC()->ForceSelection( pSrcSmartDpC->GetIDPtr() );
					}
				}
				break;

			// This columun is for smart control valve only (Not for the smart differential pressure controller).
			case CD_Circuit_SmartValveControlMode:
				{
					CDS_HydroMod::CSmartControlValve *pSrcSmartCV = (CDS_HydroMod::CSmartControlValve *)m_lpCellParam;
					ASSERT( NULL != pSrcSmartCV );

					pHMDest->GetpSmartControlValve()->SetControlMode( pSrcSmartCV->GetControlMode() );
				}
				break;

			// This columun is both for smart control valve and smart differential pressure controller.
			case CD_Circuit_SmartValveLocation:
				{
					CDS_HydroMod *pHM = GetpHM( lRow );

					if( true == pHM->IsSmartControlValveExist() )
					{
						CDS_HydroMod::CSmartControlValve *pSrcSmartControlValve = (CDS_HydroMod::CSmartControlValve *)m_lpCellParam;
						ASSERT( NULL != pSrcSmartControlValve );

						pHMDest->GetpSmartControlValve()->SetLocalization( pSrcSmartControlValve->GetLocalization() );
						pHMDest->GetpSmartControlValve()->SetSelLocalizationUserChoice( pSrcSmartControlValve->GetLocalization() );
					}
					else if( true == pHM->IsSmartDpCExist() )
					{
						CDS_HydroMod::CSmartDpC *pSrcSmartDpC = (CDS_HydroMod::CSmartDpC *)m_lpCellParam;
						ASSERT( NULL != pSrcSmartDpC );

						pHMDest->GetpSmartDpC()->SetLocalization( pSrcSmartDpC->GetLocalization() );
						pHMDest->GetpSmartDpC()->SetSelLocalizationUserChoice( pSrcSmartDpC->GetLocalization() );
					}

					pHMDest->ResetSchemeIDPtr();
				}
				break;

			// This columun is for smart differential pressure controller only (Not for the smart control valve).
			case CD_Circuit_SmartValveProductSet:
				{
					CDS_HydroMod::CSmartDpC *pSrcSmartDpC = (CDS_HydroMod::CSmartDpC *)m_lpCellParam;
					ASSERT( NULL != pSrcSmartDpC );

					pHMDest->GetpSmartDpC()->SetProductSetIDPtr( pSrcSmartDpC->GetProductSetIDPtr() );
				}
				break;

			// This columun is for smart differential pressure controller only (Not for the smart control valve).
			case CD_Circuit_SmartValveDpSensor:
				{
					CDS_HydroMod::CSmartDpC *pSrcSmartDpC = (CDS_HydroMod::CSmartDpC *)m_lpCellParam;
					ASSERT( NULL != pSrcSmartDpC );

					pHMDest->GetpSmartDpC()->SetDpSensorIDPtr( pSrcSmartDpC->GetDpSensorIDPtr() );
				}
				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// BV.
			case CD_Circuit_BvName:
			{
				// Drag and Drop of combo; m_CellLParam contains original BV * to be copied.
				CDS_HydroMod::CBV *pSrcBv = (CDS_HydroMod::CBV *)m_lpCellParam;
				ASSERT( NULL != pSrcBv );

				_ChangeBv( pHMDest, pHMDest->GetpBv(), pSrcBv );
			}
			break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Shut-off valve.
			case CD_Circuit_SvName:
			{
				// Drag and Drop of combo; m_CellLParam contains original DpC * to be copied.
				CDS_HydroMod::CShutoffValve *pSrcShutoffValve = (CDS_HydroMod::CShutoffValve *)m_lpCellParam;
				ASSERT( NULL != pSrcShutoffValve );

				CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

				if( ShutoffValveLoc::ShutoffValveLocSupply == pHMDest->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveSupply;
				}
				else if( ShutoffValveLoc::ShutoffValveLocReturn == pHMDest->GetpSch()->GetShutoffValveLoc() )
				{
					eHMObj = CDS_HydroMod::eShutoffValveReturn;
				}

				CDB_TAProduct *pTAP = pHMDest->GetpShutoffValve( eHMObj )->GetpTAP();
				unsigned short usLocker = pHMDest->GetLock();

				if( false == pHMDest->GetpShutoffValve( eHMObj )->ForceShutoffValveSelection( pSrcShutoffValve->GetIDPtr() ) )
				{
					// Force selection failed, drop a message in HCO and return to previous choice
					if( NULL == pTAP )
					{
						bComputeAll = false;
						break;
					}

					// Unlock the valve to force return to default selection.
					pHMDest->GetpShutoffValve( eHMObj )->ForceShutoffValveSelection( pTAP->GetIDPtr() );
					pHMDest->SetLock( usLocker );
					CString str = TASApp.LoadLocalizedString( IDS_SOLMSG_HMDD_CHECKDEVSIZE );
					clOutputInterface.FillAndSendMessage( IDS_WARNMSG_HMDD_DEVNOTFIT, CDlgHMCompilationOutput::Warning, pHMDest, str, pSrcShutoffValve->GetpTAP()->GetName() );
				}
			}
			break;

			//
			///////////////////////////////////////////////////////////////////////////


			///////////////////////////////////////////////////////////////////////////
			// DPC.
			case CD_Circuit_DpCName:
			{
				// Drag and Drop of combo; m_CellLParam contains original DpC * to be copied.
				CDS_HydroMod::CDpC *pSrcDpC = (CDS_HydroMod::CDpC *)m_lpCellParam;
				ASSERT( NULL != pSrcDpC );

				unsigned short usLocker = pHMDest->GetLock();

				if( false == pHMDest->GetpDpC()->ForceDpCSelection( pSrcDpC->GetIDPtr() ) )
				{
					// Force selection failed, drop a message in HCO and return to previous choice
					if( NULL == pHMDest->GetpDpC()->GetpTAP() )
					{
						bComputeAll = false;
						break;
					}

					// Unlock the valve to force return to default selection
					pHMDest->GetpDpC()->ForceDpCSelection( pHMDest->GetpDpC()->GetpTAP()->GetIDPtr() );
					pHMDest->SetLock( usLocker );
					CString str = TASApp.LoadLocalizedString( IDS_SOLMSG_HMDD_CHECKDEVSIZE );
					clOutputInterface.FillAndSendMessage( IDS_WARNMSG_HMDD_DEVNOTFIT, CDlgHMCompilationOutput::Warning, pHMDest, str, pSrcDpC->GetpTAP()->GetName() );
				}
			}
			break;
			//
			///////////////////////////////////////////////////////////////////////////


			///////////////////////////////////////////////////////////////////////////
			// Bypass BV.
			case CD_Circuit_BvBypName:
			{
				// Drag and Drop of combo; m_CellLParam contains original BV * to be copied.
				CDS_HydroMod::CBV *pSrcBv = (CDS_HydroMod::CBV *)m_lpCellParam;
				ASSERT( NULL != pSrcBv );

				_ChangeBv( pHMDest, pHMDest->GetpBypBv(), pSrcBv );
			}
			break;

			//
			///////////////////////////////////////////////////////////////////////////


			///////////////////////////////////////////////////////////////////////////
			// Circuit Pipe.
			case CD_Circuit_PipeSerie:
				{
					// Drag and drop of a combo; m_CellLParam contains original pipe * to be copied.
					CPipes *pSrcPipe = (CPipes *)m_lpCellParam;
					ASSERT( NULL != pSrcPipe );
				

					pHMDest->GetpCircuitPrimaryPipe()->SetLock( false );
					pHMDest->GetpCircuitPrimaryPipe()->SetPipeSeries( pSrcPipe->GetPipeSeries() );
				}
				break;

			case CD_Circuit_PipeSize:
				{
					// Drag and drop of a combo; m_CellLParam contains original pipe * to be copied.
					CPipes *pSrcPipe = (CPipes *)m_lpCellParam;
					ASSERT( NULL != pSrcPipe );

					pHMDest->GetpCircuitPrimaryPipe()->SetLock( !pHMDest->GetpCircuitPrimaryPipe()->IsBestPipe( pSrcPipe->GetIDPtr().MP ) );
					pHMDest->GetpCircuitPrimaryPipe()->SetIDPtr( pSrcPipe->GetIDPtr() );
				}
				break;

			case CD_Circuit_PipeL:
				{
					CPipes *pPipe = pHMDest->GetpCircuitPrimaryPipe();
					ASSERT( NULL != pPipe );

					dSI = CDimValue::CUtoSI( _U_LENGTH, m_dCellDouble );
					pHMDest->GetpCircuitPrimaryPipe()->SetLength( dSI );
				}
				break;

		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		CTermUnit *pTermUnit = pHMDest->GetpTermUnit();
		ASSERT( pTermUnit );

		if( NULL == pTermUnit )
		{
			EndWaitCursor();
			return false;
		}

		switch( m_lLastEditedCol )
		{
			// Circuit name.
			case CD_CircuitInjSecSide_Name:
				bHMTreeResetNeeded = _SetHMName( lRow );
				bComputeAll = false;
				break;

			case CD_CircuitInjSecSide_Desc:
			{
				pHMDest->SetDescription( m_strCellText );
				bComputeAll = false;
				bool bRowSelected = GetSelectedRows() ? 1 : 0;

				// Unselect row before processing.
				UnSelectMultipleRows();
				_SetCurrentSheet( GetSheetNumber( SheetDescription::SDistribution ) );
				SetCellText( CD_Distribution_Desc, lRow, m_strCellText );
				_SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuit ) );
				SetCellText( CD_Circuit_Desc, lRow, m_strCellText );

				// Return to the original sheet.
				_SetCurrentSheet( GetSheetNumber( SheetDescription::SCircuitInj ) );

				if( true == bRowSelected )
				{
					_SelectRow( lRow );
				}
			}
			break;

			///////////////////////////////////////////////////////////////////////////
			// First group.
			// HYS-1202: Add UnitDesc, P, DT, Qref, Dpref, Kv and Cv columns
			case CD_CircuitInjSecSide_UnitDesc:
				pTermUnit->SetDescription( m_strCellText );
				bComputeAll = false;
				break;

			case CD_CircuitInjSecSide_Q:
				dSI = CDimValue::CUtoSI( _U_FLOW, m_dCellDouble );
				pTermUnit->SetQ( dSI );
				break;

			case CD_CircuitInjSecSide_P:
				dSI = CDimValue::CUtoSI( _U_TH_POWER, m_dCellDouble );
				pTermUnit->SetPdT( dSI, DBL_MAX );
				break;

			case CD_CircuitInjSecSide_DT:
				{
					CDS_HmInj *pHMInj = (CDS_HmInj *)pHMDest;
					bool bSaveValue = true;
					dSI = CDimValue::CUtoSI( _U_DIFFTEMP, m_dCellDouble );
				
					// HYS-1882: SetPDT does not set the return temp anymore. We have to update it before the SetPdt function.
					if( dSI > 0 )
					{
						// HYS-1882: Set return temperature with the new DT. It is considered in SetPdt when we call GetQ() to compute flow.
						double dSupplyTemp = pHMInj->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply, CAnchorPt::CircuitSide::CircuitSide_Secondary );
						
						ProjectType eProjectType = TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType();
						double dReturnTemp = (ProjectType::Cooling == eProjectType ) ? dSupplyTemp + dSI : dSupplyTemp - dSI;
						
						// Verify that the new secondary return temperature becomes not lower or equal to the freezing point.
						if( ProjectType::Heating == eProjectType )
						{
							double dFreezingPoint = -273.15;
							CWaterChar *pclWaterChar = m_pTADS->GetpWCForProject()->GetpWCData();

							if( NULL != pclWaterChar )
							{
								dFreezingPoint = pclWaterChar->GetTfreez();
							}

							if( dFreezingPoint != -273.15 )
							{
								if( dReturnTemp <= dFreezingPoint )
								{
									// By changing the DT to %1, the return secondary temperature becomes lower or equal to the freezing point temperature (%2).

									CString str;
									CString strDT = WriteCUDouble( _U_TEMPERATURE, dSI, true );
									CString strFreezingPoint = WriteCUDouble( _U_TEMPERATURE, dFreezingPoint, true );

									FormatString( str, IDS_CIRC_TEMPERROR_DTGENERATEBADTRS, strDT, strFreezingPoint );
									AfxMessageBox( str, MB_OK | MB_ICONERROR, 0 );
									bSaveValue = false;
								}
							}
						}

						if( true == bSaveValue )
						{
							pHMInj->SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );
						
							// Set primary return temperature.
							// Remark: if for the secondary side the return temperature is OK, there is no need to verify the primary side.
							dSupplyTemp = pHMInj->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply, CAnchorPt::CircuitSide::CircuitSide_Primary );
							dReturnTemp = (ProjectType::Cooling == eProjectType) ? dSupplyTemp + dSI : dSupplyTemp - dSI;
							pHMInj->SetTemperature( dReturnTemp, CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return, CAnchorPt::CircuitSide::CircuitSide_Primary );
						}
					}

					if( true == bSaveValue )
					{
						if( CTermUnit::_QType::PdT == pHMDest->GetpTermUnit()->GetQType() )
						{
							pTermUnit->SetPdT( DBL_MAX, dSI );
						}
						else if( CTermUnit::_QType::Q == pHMDest->GetpTermUnit()->GetQType() )
						{
							if( dSI > 0.0 )
							{
								pHMDest->GetpTermUnit()->SetDTFlowMode( dSI );
							}
							else
							{
								pHMDest->GetpTermUnit()->GetDTFlowMode( true );
							}
						}
					}

					break;
				}

			case CD_CircuitInjSecSide_Dp:
				dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::Dp, dSI );
				break;

			case CD_CircuitInjSecSide_Qref:
				dSI = CDimValue::CUtoSI( _U_FLOW, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::QDpref, DBL_MAX, dSI );
				break;

			case CD_CircuitInjSecSide_Dpref:
				dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::QDpref, dSI, DBL_MAX );
				break;

			case CD_CircuitInjSecSide_Kv:
				dSI = CDimValue::CUtoSI( _C_KVCVCOEFF, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::Kv, dSI );
				break;

			case CD_CircuitInjSecSide_Cv:
				dSI = CDimValue::CUtoSI( _C_KVCVCOEFF, m_dCellDouble );
				pTermUnit->SetDp( CDS_HydroMod::eDpType::Cv, dSI );
				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Pump group.
			case CD_CircuitInjSecSide_PumpH:
			{
				CDS_HmInj *pHMInj = (CDS_HmInj *)pHMDest;
				dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
				pHMInj->GetpPump()->SetHpump( dSI );
			}
			break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Temperature.
			case CD_CircuitInjSecSide_TempTss:
			{
				CDS_HmInj *pHMInj = (CDS_HmInj *)pHMDest;
				double dNewDesignCircuitSupplySecondaryTemperature = CDimValue::CUtoSI( _U_TEMPERATURE, m_dCellDouble );

				// HYS-1716: we must verify if there is at least one injection hydraulic circuit that have its design temperatures in error
				// with this user change.
				std::multimap<CDS_HydroMod *, CTable *> mmapInjectionHydraulicCircuitWithTemperatureError;
				std::vector<CDS_HydroMod *> vecAllInjectionCircuitsWithTemperatureError;
				CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
				ASSERT( NULL != pclHydraulicNetwork );

				CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode = CDlgInjectionError::ReturnCode::Undefined;
				bool bSaveValue = true;

				bool bIsInjectionCircuitTemperatureError = pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForEditOperation( pHMInj, dNewDesignCircuitSupplySecondaryTemperature, 
						&mmapInjectionHydraulicCircuitWithTemperatureError, &vecAllInjectionCircuitsWithTemperatureError );

				if( true == bIsInjectionCircuitTemperatureError )
				{
					// Show the dialog to ask user if he wants to apply this new temperature without changing all the children in errors, or if wants to apply and 
					// automatically correct the errors or if he wants to cancel.
					CDlgInjectionError DlgInjectionError( &vecAllInjectionCircuitsWithTemperatureError );
					eDlgInjectionErrorReturnCode = (CDlgInjectionError::ReturnCode)DlgInjectionError.DoModal();

					if( CDlgInjectionError::ReturnCode::Cancel == eDlgInjectionErrorReturnCode )
					{
						bSaveValue = false;
					}
				}

				// If there is no error, or error user has choosen 'Apply' or 'Apply/Correct'.
				if( true == bSaveValue )
				{
					double dPreviousDesignCircuitSupplySecondaryTemperature = m_pHM->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply, CAnchorPt::CircuitSide::CircuitSide_Secondary );
					
					// Now we have to check if modification of the supply secondary temperature does not impact the return temperature.
					if( NULL != m_pTADS && NULL != m_pTADS->GetpTechParams() && ProjectType::Heating == m_pTADS->GetpTechParams()->GetProjectApplicationType()
							&& dPreviousDesignCircuitSupplySecondaryTemperature != dNewDesignCircuitSupplySecondaryTemperature )
					{
						double dReturnTemp = 0.0;
						bool bContinue = true;

						if( true == m_pHM->IsaModule() && NULL != pHMInj->GetFirstHMChild() )
						{
							// DT is defined with children and the computed return temperature.
							double dPreviousReturnTemp = m_pHM->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return, CAnchorPt::CircuitSide::CircuitSide_Secondary );
							dReturnTemp = dNewDesignCircuitSupplySecondaryTemperature - ( dPreviousDesignCircuitSupplySecondaryTemperature - dPreviousReturnTemp );
						}
						else
						{
							// We are not with a module or no DT is defined because we do not have children yet.
							// User can change secondary supply temperature for an injection module. In this case, we will adapt the computed return temperature
							// with the DT that has been used when creating the module.

							double dReturnTempAtTempInterface = 0.0;

							if( false == m_pHM->GetDesignTemperature( dReturnTempAtTempInterface, NULL, CAnchorPt::PipeLocation::PipeLocation_Return ) )
							{
								bContinue = false;
							}

							if( true == bContinue )
							{
								dReturnTemp = dNewDesignCircuitSupplySecondaryTemperature - ( dPreviousDesignCircuitSupplySecondaryTemperature - dReturnTempAtTempInterface );
							}
						}

						if( true == bContinue )
						{
							// Check now this new return temperature to be sure that it is not under the freezing point.
							double dFreezingPoint = -273.15;
							CWaterChar *pclWaterChar = m_pTADS->GetpWCForProject()->GetpWCData();

							if( NULL != pclWaterChar )
							{
								dFreezingPoint = pclWaterChar->GetTfreez();
							}

							if( dFreezingPoint != -273.15 )
							{
								if( dReturnTemp <= dFreezingPoint )
								{
									// By changing the supply secondary temperature 'tss' to %1, the return secondary temperature becomes lower or equal to the freezing point temperature (%2).

									CString str;
									CString strTss = WriteCUDouble( _U_TEMPERATURE, dNewDesignCircuitSupplySecondaryTemperature, true );
									CString strFreezingPoint = WriteCUDouble( _U_TEMPERATURE, dFreezingPoint, true );

									FormatString( str, IDS_INJCIRC_TEMPERROR_TSSGENERATEBADTRS, strTss, strFreezingPoint );
									AfxMessageBox( str, MB_OK | MB_ICONERROR, 0 );
									bSaveValue = false;
								}
							}

							if( true == bSaveValue )
							{
								pHMInj->SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );
								m_pHM->SetTemperature( dReturnTemp, CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return, CAnchorPt::CircuitSide::CircuitSide_Primary );
							}
						}
					}

					if( true == bSaveValue )
					{
						// The correction will be applied after the change of the temperature.
						pHMInj->SetDesignCircuitSupplySecondaryTemperature( dNewDesignCircuitSupplySecondaryTemperature );

						// HYS-1716: if there are errors and these ones must be corrected (Choice validated by user in the 'CDlgInjectionError' dialog above).
						if( CDlgInjectionError::ReturnCode::ApplyWithCorrection == eDlgInjectionErrorReturnCode )
						{
							pclHydraulicNetwork->CorrectAllInjectionCircuits( &mmapInjectionHydraulicCircuitWithTemperatureError );
						}

						// HYS-1716: Now that the supply temperature has been changed, we need to run all chidren injection circuits to update their
						// primary flows.
						// Remark: here we pass as argument the current hydraulic circuit because we do not need to check all the network but only
						//         the children of the current injection circuit.
						pclHydraulicNetwork->CorrectAllPrimaryFlow( pHMInj );
					}
				}
			}
			
			break;

			case CD_CircuitInjSecSide_TempTrs:
			{
				CDS_HmInj *pHMInj = (CDS_HmInj *)pHMDest;
				dSI = CDimValue::CUtoSI( _U_TEMPERATURE, m_dCellDouble );
				pHMInj->SetDesignCircuitReturnSecondaryTemperature( dSI );

				// HYS-1716: Now that the supply temperature has been changed, we need to run all chidren injection circuit to update their
				// primary flows.
				// Remark: here we pass as argument the current hydraulic circuit because we do not need to check all the network but only
				//         the module and its children of the current injection circuit (Or the circuit itself if it's not a module).
				CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
				ASSERT( NULL != pclHydraulicNetwork );

				pclHydraulicNetwork->CorrectAllPrimaryFlow( pHMInj );
			}
			break;

			//
			///////////////////////////////////////////////////////////////////////////
			//
			///////////////////////////////////////////////////////////////////////////
			// CV
			///////////////////////////////////////////////////////////////////////////
			// CV.
			case CD_CircuitInjSecSide_CVDesc:
	
				bComputeAll = false;
				pHMDest->GetpCV()->SetDescription( m_strCellText );
				
				break;

			case CD_CircuitInjSecSide_CVKvs:
				
				// Cell KVs can be a combo or a editable cell.
				if( true == IsCellProperty( lColumn, lRow, CellDouble ) )
				{
					dSI = CDimValue::CUtoSI( _C_KVCVCOEFF, m_dCellDouble );
					pHMDest->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMDest->GetpCV()->IsBestCV( dSI ) );
					pHMDest->GetpCV()->SetKvs( dSI );
				}
				else if( m_lpCellParam )
				{
					CDS_HydroMod::CCv *pSrcCv = (CDS_HydroMod::CCv *)m_lpCellParam;
					ASSERT( NULL != pSrcCv );

					pHMDest->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMDest->GetpCV()->IsBestCV( pSrcCv->GetKvs() ) );
					pHMDest->GetpCV()->SetKvs( pSrcCv->GetKvs() );
				}
				else if( m_dCellDouble )	// Drag&Drop source cell is in edition mode.
				{
					if( true == pHMDest->GetpCV()->IsPresettable() )
					{
						bComputeAll = false;
						break;
					}

					pHMDest->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMDest->GetpCV()->IsBestCV( m_dCellDouble ) );
					pHMDest->GetpCV()->SetKvs( m_dCellDouble );
				}
				
				break;

			case CD_CircuitInjSecSide_CVName:
				{
					// Drag and Drop of combo; m_CellLParam contains original CV * to be copied.
					CDS_HydroMod::CCv *pSrcCv = (CDS_HydroMod::CCv *)m_lpCellParam;
					ASSERT( NULL != pSrcCv );

					pHMDest->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMDest->GetpCV()->IsBestCV( (CData *)pSrcCv->GetCvIDPtr().MP ) );
					pHMDest->GetpCV()->SetCtrlType( pSrcCv->GetCtrlType() );
					pHMDest->GetpCV()->SetCVSelectedAsaPackage( pSrcCv->GetCVSelectedAsaPackage() );
					pHMDest->GetpCV()->ForceCVSelection( pSrcCv->GetCvIDPtr() );

					// We force CV but we need also to force actuator (Or remove if not!).
					pHMDest->GetpCV()->ForceActuatorSelection( pSrcCv );
				}
				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Actuator.
			case CD_CircuitInjSecSide_ActuatorName:
				{
					// Drag and Drop of combo; m_CellLParam contains original Actuator * to be copied.
					CDS_HydroMod::CCv *pSrcCv = (CDS_HydroMod::CCv *)m_lpCellParam;
					ASSERT( pSrcCv != NULL );

					pHMDest->GetpCV()->ForceActuatorSelection( pSrcCv );
				}
				break;

			///////////////////////////////////////////////////////////////////////////
			// BV.
			case CD_CircuitInjSecSide_BvName:
				{
					// Drag and Drop of combo; m_CellLParam contains original BV * to be copied.
					CDS_HydroMod::CBV *pSrcBv = ( CDS_HydroMod::CBV * )m_lpCellParam;
					ASSERT( NULL != pSrcBv );

					CDS_HydroMod::CBV *pBVSec = pHMDest->GetpSecBv();
					_ChangeBv( pHMDest, pBVSec, pSrcBv );
				}
				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Circuit Pipe.
			case CD_CircuitInjSecSide_PipeSerie:
				{
					// Drag and drop of a combo; m_CellLParam contains original pipe * to be copied.
					CPipes *pSrcPipe = (CPipes *)m_lpCellParam;
					ASSERT( NULL != pSrcPipe );

					pHMDest->GetpCircuitSecondaryPipe()->SetLock( false );
					pHMDest->GetpCircuitSecondaryPipe()->SetPipeSeries( pSrcPipe->GetPipeSeries() );
				}
				break;

			case CD_CircuitInjSecSide_PipeSize:
				{
					// Copy circuit pipe only when target contains a Terminal Unit.
					if( true == pHMDest->IsaModule() )
					{
						bComputeAll = false;
						break;
					}

					// Drag and drop of a combo; m_CellLParam contains original pipe * to be copied.
					CPipes *pSrcPipe = (CPipes *)m_lpCellParam;
					ASSERT( NULL != pSrcPipe );

					pHMDest->GetpCircuitSecondaryPipe()->SetLock( !pHMDest->GetpCircuitSecondaryPipe()->IsBestPipe( pSrcPipe->GetIDPtr().MP ) );
					pHMDest->GetpCircuitSecondaryPipe()->SetIDPtr( pSrcPipe->GetIDPtr() );
				}
				break;

			case CD_CircuitInjSecSide_PipeL:

				dSI = CDimValue::CUtoSI( _U_LENGTH, m_dCellDouble );
				pHMDest->GetpCircuitSecondaryPipe()->SetLength( dSI );
				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SMeasurement ) )
	{
		switch( m_lLastEditedCol )
		{
			// Circuit name.
			case CD_Measurement_Reference:
				bHMTreeResetNeeded = _SetHMName( lRow );
				bComputeAll = false;
				break;
		}
	}

	if( true == bComputeAll )
	{
		pHMDest->ComputeAll();
	}

	EndWaitCursor();
	return ( bComputeAll || bHMTreeResetNeeded );
}

bool CSheetHMCalc::_SetCurrentSheet( int iSheet )
{
	// When we are not in the export mode we have a max of 'SheetDescription::SLast' by module.
	int iLastSheet = ( false == m_bExporting ) ? SheetDescription::SLast : GetSheetCount();

	if( iSheet < SheetDescription::SDistribution || iSheet > iLastSheet )
	{
		return false;
	}

	// Test first if sheet is visible or not.
	if( FALSE == GetSheetVisible( iSheet ) )
	{
		return false;
	}

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		if( m_nCurrentSheet >= SheetDescription::SDistribution && m_nCurrentSheet <= SheetDescription::SLast && NULL != m_mapResizingColumnInfos[m_nCurrentSheet]
				&& m_nCurrentSheet != iSheet )
		{
			m_mapResizingColumnInfos[m_nCurrentSheet]->ActivateFeature( false, false, 0, 0, false );
		}
	}

	SetSheet( iSheet );
	SetActiveSheet( iSheet );

	if( false == m_bPrinting && false == m_bExporting )
	{
		// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
		// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
		if( NULL != m_mapResizingColumnInfos[iSheet] )
		{
			m_mapResizingColumnInfos[iSheet]->ActivateFeature( true, true, 0, 0, false );
			m_mapResizingColumnInfos[iSheet]->Rescan();
		}
	}

	m_nCurrentSheet = iSheet;
	return true;
}

bool CSheetHMCalc::_IsDragDropAvailable( long lColumn, long lRow )
{
	// Is drag a drop available ?

	CDS_HydroMod *pHM = GetpHM( lRow );

	if( NULL == pHM )
	{
		return false;
	}

	if( true == pHM->GetpPrjParam()->IsFreezed() )
	{
		return false;
	}

	bool bIsDDAvailable = false;

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		switch( lColumn )
		{
			// Circuit.
			case CD_Distribution_Desc:
			case CD_Distribution_Pos:
				bIsDDAvailable = true;
				break;

			// Distribution supply and return pipe.
			case CD_Distribution_SupplyPipeSeries:
			case CD_Distribution_SupplyPipeSize:
			case CD_Distribution_SupplyPipeLength:
			case CD_Distribution_ReturnPipeSeries:
			case CD_Distribution_ReturnPipeSize:
			case CD_Distribution_ReturnPipeLength:
				bIsDDAvailable = true;
				break;

			// Accessories.
			case CD_Distribution_SupplyAccDesc:
			case CD_Distribution_ReturnAccDesc:
				bIsDDAvailable = true;
				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		switch( lColumn )
		{
			case CD_Circuit_Desc:
				bIsDDAvailable = true;
				break;

			// Circuit.
			case CD_Circuit_Pos:
				bIsDDAvailable = true;
				break;

			///////////////////////////////////////////////////////////////////////////
			// Terminal Unit.
			case CD_Circuit_UnitDesc:
				bIsDDAvailable = true;
				break;

			case CD_Circuit_UnitQ:

				if( false == pHM->IsaModule() && CTermUnit::_QType::Q == pHM->GetpTermUnit()->GetQType() )
				{
					bIsDDAvailable = true;
				}

				break;

			case CD_Circuit_UnitP:
			case CD_Circuit_UnitDT:
				// HYS-1882: Flow mode with DT
				if( false == pHM->IsaModule() && ( CTermUnit::_QType::PdT == pHM->GetpTermUnit()->GetQType() || true == pHM->IsDTFieldDisplayedInFlowMode() ) )
				{
					bIsDDAvailable = true;
				}

				break;

			case CD_Circuit_UnitDp:

				if( false == pHM->IsaModule() && CDS_HydroMod::eDpType::Dp == pHM->GetpTermUnit()->GetDpType() )
				{
					bIsDDAvailable = true;
				}

				break;

			case CD_Circuit_UnitKv:

				if( false == pHM->IsaModule() && CDS_HydroMod::eDpType::Kv == pHM->GetpTermUnit()->GetDpType() )
				{
					bIsDDAvailable = true;
				}

				break;

			case CD_Circuit_UnitCv:

				if( false == pHM->IsaModule() && CDS_HydroMod::eDpType::Cv == pHM->GetpTermUnit()->GetDpType() )
				{
					bIsDDAvailable = true;
				}

				break;

			case CD_Circuit_UnitQref:
			case CD_Circuit_UnitDpref:

				if( false == pHM->IsaModule() && CDS_HydroMod::eDpType::QDpref == pHM->GetpTermUnit()->GetDpType() )
				{
					bIsDDAvailable = true;
				}

				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// CV.
			case CD_Circuit_CVDesc:
				if( true == pHM->IsCvExist( true ) && false == pHM->GetpCV()->IsTaCV() && eb3True == pHM->GetpCV()->IsCVLocInPrimary() )
				{
					bIsDDAvailable = true;
				}

				break;

			case CD_Circuit_CVName:
				if( true == pHM->IsCvExist( true ) && true == pHM->GetpCV()->IsTaCV() && eb3True == pHM->GetpCV()->IsCVLocInPrimary() )
				{
					bIsDDAvailable = true;
				}

				break;

			case CD_Circuit_CVKvs:
				if( true == pHM->IsCvExist( true ) && false == pHM->GetpCV()->IsTaCV() && eb3True == pHM->GetpCV()->IsCVLocInPrimary() )
				{
					bIsDDAvailable = true;
				}

				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Actuator.
			case CD_Circuit_ActuatorName:
				if( true == pHM->IsCvExist( true ) && true == pHM->GetpCV()->IsTaCV() && eb3True == pHM->GetpCV()->IsCVLocInPrimary()
					&& dynamic_cast<CDB_Actuator *>( (CData *)pHM->GetpCV()->GetActrIDPtr().MP ) != NULL )
				{
					bIsDDAvailable = true;
				}

				break;

			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Smart valve.
			// HYS-1676: Add smart valve to Sheet HM Calc
			// This column is available for both smart control valve and smart differential pressure controller.
			case CD_Circuit_SmartValveName:
				
				if( true == pHM->IsSmartControlValveExist( true ) || true == pHM->IsSmartDpCExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;

			// This column is available for both smart control valve and smart differential pressure controller.
			case CD_Circuit_SmartValveLocation:

				if( true == pHM->IsSmartControlValveExist( true ) || true == pHM->IsSmartDpCExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;

			// This column is available for smart control valve only.
			case CD_Circuit_SmartValveControlMode:

				if( true == pHM->IsSmartControlValveExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;

			// This column is available for smart differential pressure controller only.
			case CD_Circuit_SmartValveProductSet:

				if( true == pHM->IsSmartDpCExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;

			// This column is available for smart differential pressure controller only.
			case CD_Circuit_SmartValveDpSensor:

				if( true == pHM->IsSmartDpCExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;
			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// BV.
			case CD_Circuit_BvName:
				if( true == pHM->IsBvExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;
			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Shut-off valve.
			case CD_Circuit_SvName:
				if( true == pHM->IsShutoffValveExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;
			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// DpC.
			case CD_Circuit_DpCName:
				if( true == pHM->IsDpCExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;
			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// BV Bypass.
			case CD_Circuit_BvBypName:
				if( true == pHM->IsBvBypExist( true ) )
				{
					bIsDDAvailable = true;
				}

				break;
			//
			///////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Circuit pipe.
			case CD_Circuit_PipeSerie:
			case CD_Circuit_PipeSize:
			case CD_Circuit_PipeL:
				bIsDDAvailable = true;
				break;
			//
			/////////////////////////////////////////////////////////////////////////////

			///////////////////////////////////////////////////////////////////////////
			// Circuit pipe accessories.
			case CD_Circuit_PipeAccDesc:
				bIsDDAvailable = true;
				break;
			//
			///////////////////////////////////////////////////////////////////////////
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		// Only if current row well contains a injection circuit.
		if( NULL != pHM->GetpSchcat() && true == pHM->GetpSchcat()->IsSecondarySideExist() && NULL != pHM->GetpCircuitSecondaryPipe() )
		{
			switch( lColumn )
			{
				// Circuit.
				case CD_CircuitInjSecSide_Pos:
					bIsDDAvailable = true;
					break;

				case CD_CircuitInjSecSide_Desc:
					bIsDDAvailable = true;
					break;

				///////////////////////////////////////////////////////////////////////////
				// First group.
				// HYS-1202: Add UnitDesc, P, DT, Qref, Dpref, Kv and Cv columns
				case CD_CircuitInjSecSide_UnitDesc:
					bIsDDAvailable = true;
					break;

				case CD_CircuitInjSecSide_Q:
					if( CTermUnit::_QType::Q == pHM->GetpTermUnit()->GetQType() )
					{
						bIsDDAvailable = true;
					}

					break;

				case CD_CircuitInjSecSide_P:
				case CD_CircuitInjSecSide_DT:
					// HYS-1882: Flow mode with DT
					if( CTermUnit::_QType::PdT == pHM->GetpTermUnit()->GetQType() || true == pHM->IsDTFieldDisplayedInFlowMode() )
					{
						bIsDDAvailable = true;
					}

					break;

				case CD_CircuitInjSecSide_Dp:
					if( CDS_HydroMod::eDpType::Dp == pHM->GetpTermUnit()->GetDpType() )
					{
						bIsDDAvailable = true;
					}

					break;

				case CD_CircuitInjSecSide_Kv:
					if( CDS_HydroMod::eDpType::Kv == pHM->GetpTermUnit()->GetDpType() )
					{
						bIsDDAvailable = true;
					}

					break;

				case CD_CircuitInjSecSide_Cv:
					if( CDS_HydroMod::eDpType::Cv == pHM->GetpTermUnit()->GetDpType() )
					{
						bIsDDAvailable = true;
					}

					break;

				case CD_CircuitInjSecSide_Qref:
				case CD_CircuitInjSecSide_Dpref:
					if( CDS_HydroMod::eDpType::QDpref == pHM->GetpTermUnit()->GetDpType() )
					{
						bIsDDAvailable = true;
					}

					break;

				//
				///////////////////////////////////////////////////////////////////////////

				///////////////////////////////////////////////////////////////////////////
				// Pump group.
				case CD_CircuitInjSecSide_PumpH:
					bIsDDAvailable = true;
					break;

				//
				///////////////////////////////////////////////////////////////////////////

				///////////////////////////////////////////////////////////////////////////
				// Temperature.
				case CD_CircuitInjSecSide_TempTss:
				case CD_CircuitInjSecSide_TempTrs:
					bIsDDAvailable = true;
					break;

				//
				///////////////////////////////////////////////////////////////////////////

				///////////////////////////////////////////////////////////////////////////
				// CV.
				case CD_CircuitInjSecSide_CVDesc:
					if( true == pHM->IsCvExist( true ) && false == pHM->GetpCV()->IsTaCV() && eb3False == pHM->GetpCV()->IsCVLocInPrimary() )
					{
						bIsDDAvailable = true;
					}

					break;

				case CD_CircuitInjSecSide_CVName:
					if( true == pHM->IsCvExist( true ) && true == pHM->GetpCV()->IsTaCV() && eb3False == pHM->GetpCV()->IsCVLocInPrimary() )
					{
						bIsDDAvailable = true;
					}

					break;

				case CD_CircuitInjSecSide_CVKvs:
					if( true == pHM->IsCvExist( true ) && false == pHM->GetpCV()->IsTaCV() && eb3False == pHM->GetpCV()->IsCVLocInPrimary() )
					{
						bIsDDAvailable = true;
					}

					break;

				//
				///////////////////////////////////////////////////////////////////////////

				///////////////////////////////////////////////////////////////////////////
				// Actuator.
				case CD_CircuitInjSecSide_ActuatorName:
					if( true == pHM->IsCvExist( true ) && true == pHM->GetpCV()->IsTaCV() && eb3False == pHM->GetpCV()->IsCVLocInPrimary()
						&& dynamic_cast<CDB_Actuator *>( (CData *)pHM->GetpCV()->GetActrIDPtr().MP ) != NULL )
					{
						bIsDDAvailable = true;
					}

					break;

				//
				///////////////////////////////////////////////////////////////////////////
				///////////////////////////////////////////////////////////////////////////
				// BV.
				case CD_CircuitInjSecSide_BvName:
					if( true == pHM->IsBvSecExist( true ) )
					{
						bIsDDAvailable = true;
					}

					break;

				//
				///////////////////////////////////////////////////////////////////////////

				///////////////////////////////////////////////////////////////////////////
				// Circuit Pipe.
				case CD_CircuitInjSecSide_PipeSerie:
				case CD_CircuitInjSecSide_PipeSize:
					bIsDDAvailable = true;
					break;
				case CD_CircuitInjSecSide_PipeL:
					// HYS-1735: No length for 3-way mixing circuit secondary pipe.
					if( CDB_CircSchemeCateg::e3wTypeMixing != pHM->GetpSchcat()->Get3WType() )
					{
						bIsDDAvailable = true;
					}
					break;

				//
				/////////////////////////////////////////////////////////////////////////////

				///////////////////////////////////////////////////////////////////////////
				// Accessories.
				case CD_CircuitInjSecSide_AccDesc:
					// HYS-1735: No accessories for 3-way mixing circuit secondary pipe.
					if( CDB_CircSchemeCateg::e3wTypeMixing != pHM->GetpSchcat()->Get3WType() )
					{
						bIsDDAvailable = true;
					}
					break;
					//
					///////////////////////////////////////////////////////////////////////////
			}
		}
	}

	return bIsDDAvailable;
}

bool CSheetHMCalc::_IsDragDropPossible( CDS_HydroMod *pclHMSource, CDS_HydroMod *pclHMDest, long lColumnDest, long lRowDest )
{
	if( NULL == pclHMDest )
	{
		// User is currently drag & dropping but the current line doesn't contain a hydraulic module.
		return false;
	}

	bool bIsPossible = true;

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		switch( lColumnDest )
		{
			case CD_Distribution_SupplyPipeSeries:
			case CD_Distribution_SupplyPipeSize:
			case CD_Distribution_SupplyPipeLength:

				if( 0 == pclHMDest->GetLevel() )
				{
					// No pipe for root module.
					bIsPossible = false;
					break;
				}

				if( NULL == pclHMDest->GetpDistrSupplyPipe() )
				{
					bIsPossible = false;
					break;
				}

				break;

			case CD_Distribution_ReturnPipeSeries:
			case CD_Distribution_ReturnPipeSize:
			case CD_Distribution_ReturnPipeLength:

				if( 0 == pclHMDest->GetLevel() )
				{
					// No pipe for root module.
					bIsPossible = false;
					break;
				}

				if( NULL == pclHMDest->GetpDistrReturnPipe() )
				{
					bIsPossible = false;
					break;
				}

				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		switch( lColumnDest )
		{
			case CD_Circuit_UnitQ:
			case CD_Circuit_UnitP:
			case CD_Circuit_UnitDT:
			case CD_Circuit_UnitDp:
			case CD_Circuit_UnitQref:
			case CD_Circuit_UnitDpref:
			case CD_Circuit_UnitKv:
			case CD_Circuit_UnitCv:

				if( true == pclHMDest->IsaModule() || true == pclHMDest->GetpSchcat()->IsSecondarySideExist() )
				{
					bIsPossible = false;
					break;
				}

				break;

			case CD_Circuit_PumpHUser:
				
				if( NULL == pclHMDest->GetpPump() )
				{
					bIsPossible = false;
					break;
				}
				
				break;

			case CD_Circuit_CVName:

				if( false == pclHMDest->IsCvExist( true ) )
				{
					bIsPossible = false;
					break;
				}
				
				if( eb3False == pclHMDest->GetpCV()->IsCVLocInPrimary() )
				{
					bIsPossible = false;
					break;
				}
				
				if( false == pclHMDest->GetpCV()->IsTaCV() )
				{
					bIsPossible = false;
					break;
				}
				
				if( pclHMDest->GetSchemeIDPtr().MP != pclHMSource->GetSchemeIDPtr().MP )
				{
					bIsPossible = false;
					break;
				}

				if( false == pclHMDest->GetpCV()->IsCvExistInPreselectedList( pclHMSource->GetpCV()->GetpCV() ) )
				{
					// If valve doesn't exit in the preselected list, it means for example that it has the wrong size.
					bIsPossible = false;
					break;
				}

				break;

			case CD_Circuit_CVDesc:

				if( NULL == pclHMDest->GetpCV() )
				{
					bIsPossible = false;
					break;
				}

				if( eb3False == pclHMDest->GetpCV()->IsCVLocInPrimary() )
				{
					bIsPossible = false;
					break;
				}

				if( true == pclHMDest->GetpCV()->IsTaCV() )
				{
					// No description for TA CV.
					bIsPossible = false;
					break;
				}

				break;

			case CD_Circuit_CVKvs:

				if( NULL == pclHMDest->GetpCV() )
				{
					bIsPossible = false;
					break;
				}

				if( eb3False == pclHMDest->GetpCV()->IsCVLocInPrimary() )
				{
					bIsPossible = false;
					break;
				}

				if( true == pclHMDest->GetpCV()->IsTaCV() )
				{
					bIsPossible = false;
					break;
				}

				break;

			case CD_Circuit_ActuatorName:
				{
					if( false == pclHMDest->IsCvExist( true ) )
					{
						bIsPossible = false;
						break;
					}

					if( eb3False == pclHMDest->GetpCV()->IsCVLocInPrimary() )
					{
						bIsPossible = false;
						break;
					}

					if( false == pclHMDest->GetpCV()->IsTaCV() )
					{
						bIsPossible = false;
						break;
					}

					CDB_Actuator *pclSrcActuator = dynamic_cast<CDB_Actuator *>( (CData *)pclHMSource->GetpCV()->GetActrIDPtr().MP );

					if( false == pclHMDest->GetpCV()->GetpCV()->IsActuatorFit( pclSrcActuator ) )
					{
						bIsPossible = false;
						break;
					}
				}
				break;

			// HYS-1676: Add smart valve to Sheet HM Calc
			// This column is available for both smart control valve and smart differential pressure controller.
			case CD_Circuit_SmartValveName:

				if( false == pclHMDest->IsSmartControlValveExist( true ) && false == pclHMDest->IsSmartDpCExist( true ) )
				{
					bIsPossible = false;
					break;
				}

				if( pclHMDest->GetSchemeIDPtr().MP != pclHMSource->GetSchemeIDPtr().MP )
				{
					bIsPossible = false;
					break;
				}

				if( true == pclHMDest->IsSmartControlValveExist( true ) )
				{
					if( false == pclHMDest->GetpSmartControlValve()->IsSmartControlValveExistInPreselectedList( (CDB_SmartControlValve *)pclHMSource->GetpSmartControlValve()->GetIDPtr().MP ) )
					{
						bIsPossible = false;
						break;
					}
				}
				else
				{
					if( false == pclHMDest->GetpSmartDpC()->IsSmartDpCExistInPreselectedList( (CDB_SmartControlValve *)pclHMSource->GetpSmartDpC()->GetIDPtr().MP ) )
					{
						bIsPossible = false;
						break;
					}
				}

				break;

			// This column is available for smart control valve only.
			case CD_Circuit_SmartValveControlMode:
			
				if( false == pclHMDest->IsSmartControlValveExist( true ) )
				{
					bIsPossible = false;
					break;
				}

				break;

			// This column is available for both smart control valve and smart differential pressure controller.
			case CD_Circuit_SmartValveLocation:

				if( false == pclHMDest->IsSmartControlValveExist( true ) && false == pclHMDest->IsSmartDpCExist( true ) )
				{
					bIsPossible = false;
					break;
				}

				break;

			// This column is available for smart differential pressure controller only.
			case CD_Circuit_SmartValveProductSet:

				if( false == pclHMDest->IsSmartDpCExist( true ) )
				{
					bIsPossible = false;
					break;
				}

				if( pclHMDest->GetSchemeIDPtr().MP != pclHMSource->GetSchemeIDPtr().MP )
				{
					bIsPossible = false;
					break;
				}

				break;

			// This column is available for smart differential pressure controller only.
			case CD_Circuit_SmartValveDpSensor:

				if( false == pclHMDest->IsSmartDpCExist( true ) )
				{
					bIsPossible = false;
					break;
				}

				if( pclHMDest->GetSchemeIDPtr().MP != pclHMSource->GetSchemeIDPtr().MP )
				{
					bIsPossible = false;
					break;
				}

				break;

			case CD_Circuit_BvName:
				
				if( false == pclHMDest->IsBvExist( true ) )
				{
					bIsPossible = false;
					break;
				}

				if( false == pclHMDest->GetpBv()->IsBvExistInPreselectedList( pclHMSource->GetpBv()->GetpTAP() ) )
				{
					// If valve doesn't exit in the preselected list, it means for example that it has the wrong size.
					bIsPossible = false;
					break;
				}
				
				break;

			case CD_Circuit_SvName:
				{
					if( false == pclHMDest->IsShutoffValveExist( true ) || NULL == pclHMDest->GetpSch() )
					{
						bIsPossible = false;
						break;
					}

					CDS_HydroMod::eHMObj eSVSourceLocation = CDS_HydroMod::eNone;

					if( ShutoffValveLoc::ShutoffValveLocSupply == pclHMSource->GetpSch()->GetShutoffValveLoc() )
					{
						eSVSourceLocation = CDS_HydroMod::eShutoffValveSupply;
					}
					else if( ShutoffValveLoc::ShutoffValveLocReturn == pclHMSource->GetpSch()->GetShutoffValveLoc() )
					{
						eSVSourceLocation = CDS_HydroMod::eShutoffValveReturn;
					}
				
					if( CDS_HydroMod::eNone == eSVSourceLocation )
					{
						bIsPossible = false;
						break;
					}

					CDS_HydroMod::CShutoffValve *pHMShutoffValve = pclHMDest->GetpShutoffValve( eSVSourceLocation );

					if( NULL == pHMShutoffValve )
					{
						bIsPossible = false;
						break;
					}

					if( false == pclHMDest->GetpShutoffValve( eSVSourceLocation )->IsShutoffValveExistInPreselectedList( pclHMSource->GetpShutoffValve( eSVSourceLocation )->GetpTAP() ) )
					{
						// If valve doesn't exit in the preselected list, it means for example that it has the wrong size.
						bIsPossible = false;
						break;
					}
				}
				break;

			case CD_Circuit_DpCName:

				if( false == pclHMDest->IsDpCExist( true ) )
				{
					bIsPossible = false;
					break;
				}

				if( pclHMSource->GetpSch()->GetDpCLoc() != pclHMDest->GetpSch()->GetDpCLoc() )
				{
					// A DA 516 valve put on the return can't be put on the supply (It's a DAF 512 on the return).
					bIsPossible = false;
					break;
				}

				if( false == pclHMDest->GetpDpC()->IsDpCExistInPreselectedList( pclHMSource->GetpDpC()->GetpTAP() ) )
				{
					// If valve doesn't exit in the preselected list, it means for example that it has the wrong size.
					bIsPossible = false;
					break;
				}

				break;

			case CD_Circuit_BvBypName:

				if( false == pclHMDest->IsBvBypExist( true ) )
				{
					bIsPossible = false;
					break;
				}

				if( 0 == pclHMDest->IsClass( CLASS( CDS_Hm3W ) ) )
				{
					bIsPossible = false;
					break;
				}

				if( CDS_HydroMod::eUseProduct::Never == ( (CDS_Hm3W *)pclHMDest )->GetUseBypBv() )
				{
					bIsPossible = false;
					break;
				}

				if( false == pclHMDest->GetpBypBv()->IsBvExistInPreselectedList( pclHMSource->GetpBypBv()->GetpTAP() ) )
				{
					// If valve doesn't exit in the preselected list, it means for example that it has the wrong size.
					bIsPossible = false;
					break;
				}

				break;

			case CD_Circuit_PipeSerie:
			case CD_Circuit_PipeSize:
			case CD_Circuit_PipeL:

				// If target is a module, we can copy circuit pipe only if this module contains a secondary side.
				if( true == pclHMDest->IsaModule() && NULL != pclHMDest->GetpSchcat() && false == pclHMDest->GetpSchcat()->IsSecondarySideExist() )
				{
					bIsPossible = false;
					break;
				}

				if( NULL == pclHMDest->GetpCircuitPrimaryPipe() )
				{
					bIsPossible = false;
					break;
				}

				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		// Only if current row well contains a injection circuit.
		if( NULL != pclHMSource->GetpSchcat() && true == pclHMSource->GetpSchcat()->IsSecondarySideExist() && NULL != pclHMSource->GetpCircuitSecondaryPipe() )
		{
			if( NULL == pclHMDest->GetpSchcat() || false == pclHMDest->GetpSchcat()->IsSecondarySideExist() || NULL == pclHMDest->GetpCircuitSecondaryPipe() )
			{
				// User is currently drag & dropping a data from the secondary side but the current hydraulic circuit on the current line 
				// doesn't contain secondary side.
				bIsPossible = false;
			}
			else
			{
				switch( lColumnDest )
				{
					case CD_CircuitInjSecSide_Q:
					case CD_CircuitInjSecSide_P:
					case CD_CircuitInjSecSide_DT:
					case CD_CircuitInjSecSide_Dp:
					case CD_CircuitInjSecSide_Qref:
					case CD_CircuitInjSecSide_Dpref:
					case CD_CircuitInjSecSide_Kv:
					case CD_CircuitInjSecSide_Cv:

						if( true == pclHMDest->IsaModule() || false == pclHMDest->GetpSchcat()->IsSecondarySideExist() )
						{
							bIsPossible = false;
							break;
						}

						break;

					case CD_CircuitInjSecSide_CVName:

						if( false == pclHMDest->IsCvExist( true ) )
						{
							bIsPossible = false;
							break;
						}

						if( eb3True == pclHMDest->GetpCV()->IsCVLocInPrimary() )
						{
							bIsPossible = false;
							break;
						}

						if( false == pclHMDest->GetpCV()->IsTaCV() )
						{
							bIsPossible = false;
							break;
						}

						if( pclHMDest->GetSchemeIDPtr().MP != pclHMSource->GetSchemeIDPtr().MP )
						{
							bIsPossible = false;
							break;
						}

						if( false == pclHMDest->GetpCV()->IsCvExistInPreselectedList( pclHMSource->GetpCV()->GetpCV() ) )
						{
							// If valve doesn't exit in the preselected list, it means for example that it has the wrong size.
							bIsPossible = false;
							break;
						}

						break;

					case CD_CircuitInjSecSide_CVDesc:

						if( NULL == pclHMDest->GetpCV() )
						{
							bIsPossible = false;
							break;
						}

						if( eb3True == pclHMDest->GetpCV()->IsCVLocInPrimary() )
						{
							bIsPossible = false;
							break;
						}

						if( true == pclHMDest->GetpCV()->IsTaCV() )
						{
							// No description for TA CV.
							bIsPossible = false;
							break;
						}

						break;

					case CD_CircuitInjSecSide_CVKvs:

						if( NULL == pclHMDest->GetpCV() )
						{
							bIsPossible = false;
							break;
						}

						if( eb3True == pclHMDest->GetpCV()->IsCVLocInPrimary() )
						{
							bIsPossible = false;
							break;
						}

						if( true == pclHMDest->GetpCV()->IsTaCV() )
						{
							bIsPossible = false;
							break;
						}

						break;

					case CD_CircuitInjSecSide_ActuatorName:
						{
							if( false == pclHMDest->IsCvExist( true ) )
							{
								bIsPossible = false;
								break;
							}

							if( eb3True == pclHMDest->GetpCV()->IsCVLocInPrimary() )
							{
								bIsPossible = false;
								break;
							}

							if( false == pclHMDest->GetpCV()->IsTaCV() )
							{
								bIsPossible = false;
								break;
							}

							CDB_Actuator *pclSrcActuator = dynamic_cast<CDB_Actuator *>( (CData *)pclHMSource->GetpCV()->GetActrIDPtr().MP );

							if( false == pclHMDest->GetpCV()->GetpCV()->IsActuatorFit( pclSrcActuator ) )
							{
								bIsPossible = false;
								break;
							}
						}

						break;

					case CD_CircuitInjSecSide_BvName:

						if( false == pclHMDest->IsBvSecExist( true ) )
						{
							bIsPossible = false;
							break;
						}

						if( false == pclHMDest->GetpSecBv()->IsBvExistInPreselectedList( pclHMSource->GetpSecBv()->GetpTAP() ) )
						{
							// If valve doesn't exit in the preselected list, it means for example that it has the wrong size.
							bIsPossible = false;
							break;
						}

						break;

					case CD_CircuitInjSecSide_PipeSerie:
					case CD_CircuitInjSecSide_PipeSize:
					case CD_CircuitInjSecSide_PipeL:

						// Copy circuit pipe only when target contains a terminal unit.
						if( true == pclHMDest->IsaModule() )
						{
							bIsPossible = false;
							break;
						}

						if( NULL == pclHMDest->GetpCircuitSecondaryPipe() )
						{
							bIsPossible = false;
							break;
						}

						// HYS-1735: Value cannot be modifed for 3-way mixing circuit
						if( CDB_CircSchemeCateg::e3wTypeMixing == pclHMDest->GetpSchcat()->Get3WType() )
						{
							bIsPossible = false;
							break;
						}

						break;
				}
			}
		}
	}

	return bIsPossible;
}

bool CSheetHMCalc::_ApplyDragAndDrop( CDS_HydroMod *pclHMSource, CDS_HydroMod *pclHMDest, long lColumnDest, long lRowDest )
{
	if( false == _IsDragDropPossible( pclHMSource, pclHMDest, lColumnDest, lRowDest ) )
	{
		return false;
	}

	// For terminal unit, we need a special treatment that the one in the 'SaveAndCompute' method.
	// Because we allow now to drag & drop for example a 'Power' column on a terminal unit defined with 'Flow'.
	// In this case we replace the 'Flow' by the 'Power' AND 'DT' values.
	if( ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) && lColumnDest >= CD_Circuit_UnitQ && lColumnDest <= CD_Circuit_UnitCv )
			|| ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) && lColumnDest >= CD_CircuitInjSecSide_Q && lColumnDest <= CD_CircuitInjSecSide_Cv ) )
	{
		if( CD_Circuit_UnitQ == lColumnDest || CD_CircuitInjSecSide_Q == lColumnDest )
		{
			if( CTermUnit::_QType::Q != pclHMDest->GetpTermUnit()->GetQType() )
			{
				// If current terminal unit on which we drop the flow was in other mode, we need to delete old values.
				SetCellText( ( CD_Circuit_UnitQ == lColumnDest ) ? CD_Circuit_UnitP : CD_CircuitInjSecSide_P, lRowDest, _T("") );
				// HYS-1882: if src has a DT we keep DT dest
				if( false == pclHMSource->IsDTFieldDisplayedInFlowMode() )
				{
					pclHMDest->GetpTermUnit()->SetPdTOnlyValues( DBL_MAX, 0.0 );
					SetCellText( (CD_Circuit_UnitQ == lColumnDest) ? CD_Circuit_UnitDT : CD_CircuitInjSecSide_DT, lRowDest, _T( "" ) );
				}
			}
				
			pclHMDest->GetpTermUnit()->SetQ( pclHMSource->GetpTermUnit()->GetQ() );
			SetCellText( lColumnDest, lRowDest, pclHMSource->GetpTermUnit()->GetFlowString() );
		}
		else if( CD_Circuit_UnitP == lColumnDest || CD_Circuit_UnitDT == lColumnDest
				|| CD_CircuitInjSecSide_P == lColumnDest || CD_CircuitInjSecSide_DT == lColumnDest )
		{
			CTermUnit::_QType eQType = pclHMDest->GetpTermUnit()->GetQType();

			// HYS-1414 : When the drag and drop process is on 'DT', 'Power' must not change.
			if( CD_Circuit_UnitP == lColumnDest || CD_CircuitInjSecSide_P == lColumnDest )
			{
				// Drag and drop is on 'Power'.
				// HYS-1882: Dest is a flow DT mode we keep the DT
				if( eQType == pclHMSource->GetpTermUnit()->GetQType() || true == pclHMDest->IsDTFieldDisplayedInFlowMode() )
				{
					// Flow type of the destination terminal unit is the same as the source.
					// We copy 'Power' src in 'Power' dest and calculate 'Q' dest with the 'Power' src and 'DT' dest.
					pclHMDest->GetpTermUnit()->SetPdT( pclHMSource->GetpTermUnit()->GetP(), pclHMDest->GetpTermUnit()->GetDT() );
				}
				else
				{
					// Flow type of the destination terminal unit is not the same as the source. Dest. has not got a DT field.
					// We need thus to copy both 'Power' and 'DT' from the source to the destination.
					pclHMDest->GetpTermUnit()->SetPdT( pclHMSource->GetpTermUnit()->GetP(), pclHMSource->GetpTermUnit()->GetDT() );
					SetCellText( ( CD_Circuit_UnitP == lColumnDest ) ? CD_Circuit_UnitDT : CD_CircuitInjSecSide_DT, lRowDest, pclHMSource->GetpTermUnit()->GetDTString() );
				}

				SetCellText( ( CD_Circuit_UnitP == lColumnDest ) ? CD_Circuit_UnitQ : CD_CircuitInjSecSide_Q, lRowDest, pclHMDest->GetpTermUnit()->GetFlowString() );
				SetCellText( ( CD_Circuit_UnitP == lColumnDest ) ? CD_Circuit_UnitP : CD_CircuitInjSecSide_P, lRowDest, pclHMDest->GetpTermUnit()->GetPowerString() );
			}
			else
			{
				// Drag and drop is on 'DT'.
				ProjectType eProjectType = TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType();
				if( eQType == pclHMSource->GetpTermUnit()->GetQType() )
				{
					// Flow type of the destination terminal unit is the same as the source.
					// HYS-1882: Set return temperature with the new DT. (It is considered in SetPdt when we call GetQ() to compute flow).
					double dReturnTemp = 0.0;
					double dSupplyTemp = 0.0;
					double dSI = (true == pclHMSource->IsDTFieldDisplayedInFlowMode()) ? pclHMSource->GetpTermUnit()->GetDTFlowMode() : pclHMSource->GetpTermUnit()->GetDT();
					if( true == pclHMDest->IsInjectionCircuit() )
					{
						dSupplyTemp = pclHMDest->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply,
																 CAnchorPt::CircuitSide::CircuitSide_Secondary );
						dReturnTemp = (ProjectType::Cooling == eProjectType) ? dSupplyTemp + dSI : dSupplyTemp - dSI;
						((CDS_HmInj*)pclHMDest)->SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );
					}

					// Set primary return temperature
					dSupplyTemp = pclHMDest->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply,
															 CAnchorPt::CircuitSide::CircuitSide_Primary );
					dReturnTemp = (ProjectType::Cooling == eProjectType) ? dSupplyTemp + dSI : dSupplyTemp - dSI;
					pclHMDest->SetTemperature( dReturnTemp, CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return,
											   CAnchorPt::CircuitSide::CircuitSide_Primary );

					if( true == pclHMSource->IsDTFieldDisplayedInFlowMode() )
					{
						pclHMDest->GetpTermUnit()->SetDTFlowMode( dSI );
					}
					else
					{
						// We copy 'DT' src in 'DT' dest and calculate 'Q' dest with the 'DT' src and 'Power' dest.
						pclHMDest->GetpTermUnit()->SetPdT( pclHMDest->GetpTermUnit()->GetP(), dSI );
					}
				}
				else
				{
					// Flow type of the destination terminal unit is not the same as the source.
					if( false == pclHMDest->IsDTFieldDisplayedInFlowMode() )
					{
						// Src Flow mode with DT field -> PDT
						if( eQType == CTermUnit::_QType::PdT )
						{
							// HYS-1882: Set return temperature with the new DT. (It is considered in SetPdt when we call GetQ() to compute flow).
							double dReturnTemp = 0.0;
							double dSupplyTemp = 0.0;
							double dSI = pclHMSource->GetpTermUnit()->GetDTFlowMode();
							if( true == pclHMDest->IsInjectionCircuit() )
							{
								dSupplyTemp = pclHMDest->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply,
																		 CAnchorPt::CircuitSide::CircuitSide_Secondary );
								dReturnTemp = (ProjectType::Cooling == eProjectType) ? dSupplyTemp + dSI : dSupplyTemp - dSI;
								((CDS_HmInj*)pclHMDest)->SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );
							}

							// Set primary return temperature
							dSupplyTemp = pclHMDest->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply,
																	 CAnchorPt::CircuitSide::CircuitSide_Primary );
							dReturnTemp = (ProjectType::Cooling == eProjectType) ? dSupplyTemp + dSI : dSupplyTemp - dSI;
							pclHMDest->SetTemperature( dReturnTemp, CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return,
													   CAnchorPt::CircuitSide::CircuitSide_Primary );
							// We copy 'DT' src in 'DT' dest and calculate 'Q' dest with the 'DT' src and 'Power' dest.
							pclHMDest->GetpTermUnit()->SetPdT( pclHMDest->GetpTermUnit()->GetP(), dSI );
						}
						else
						{
							// Src PDT -> Flow mode without DT field
							// We need thus to copy both 'Power' and 'DT' from the source to the destination.
							pclHMDest->GetpTermUnit()->SetPdT( pclHMSource->GetpTermUnit()->GetP(), pclHMSource->GetpTermUnit()->GetDT() );
							SetCellText( (CD_Circuit_UnitP == lColumnDest) ? CD_Circuit_UnitP : CD_CircuitInjSecSide_P, lRowDest, pclHMSource->GetpTermUnit()->GetPowerString() );
						}
					}
					else
					{
						// Src PDT -> Flow mode with DT
						// HYS-1882: Set return temperature with the new DT. (It is considered in SetPdt when we call GetQ() to compute flow).
						double dReturnTemp = 0.0;
						double dSupplyTemp = 0.0;
						double dSI = pclHMSource->GetpTermUnit()->GetDT();
						if( true == pclHMDest->IsInjectionCircuit() )
						{
							dSupplyTemp = pclHMDest->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply,
																	 CAnchorPt::CircuitSide::CircuitSide_Secondary );
							dReturnTemp = (ProjectType::Cooling == eProjectType) ? dSupplyTemp + dSI : dSupplyTemp - dSI;
							((CDS_HmInj*)pclHMDest)->SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );
						}

						// Set primary return temperature
						dSupplyTemp = pclHMDest->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply,
																 CAnchorPt::CircuitSide::CircuitSide_Primary );
						dReturnTemp = (ProjectType::Cooling == eProjectType) ? dSupplyTemp + dSI : dSupplyTemp - dSI;
						pclHMDest->SetTemperature( dReturnTemp, CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return,
												   CAnchorPt::CircuitSide::CircuitSide_Primary );

						pclHMDest->GetpTermUnit()->SetDTFlowMode( dSI );
					}
				}

				SetCellText( ( CD_Circuit_UnitDT == lColumnDest ) ? CD_Circuit_UnitDT : CD_CircuitInjSecSide_DT, lRowDest, pclHMDest->GetpTermUnit()->GetDTString() );
				SetCellText( ( CD_Circuit_UnitDT == lColumnDest ) ? CD_Circuit_UnitQ : CD_CircuitInjSecSide_Q, lRowDest, pclHMDest->GetpTermUnit()->GetFlowString() );
			}
		}
		else
		{
			if( CDS_HydroMod::eDpType::Dp == pclHMSource->GetpTermUnit()->GetDpType()
					&& CDS_HydroMod::eDpType::Dp != pclHMDest->GetpTermUnit()->GetDpType() )
			{
				if( CDS_HydroMod::eDpType::QDpref == pclHMDest->GetpTermUnit()->GetDpType() )
				{
					if( lColumnDest >= CD_Circuit_UnitQ && lColumnDest <= CD_Circuit_UnitCv )
					{
						SetCellText( CD_Circuit_UnitQref, lRowDest, _T("") );
						SetCellText( CD_Circuit_UnitDpref, lRowDest, _T("") );
					}
					else
					{
						SetCellText( CD_CircuitInjSecSide_Qref, lRowDest, _T("") );
						SetCellText( CD_CircuitInjSecSide_Dpref, lRowDest, _T("") );
					}
				}
				else if( CDS_HydroMod::eDpType::Kv == pclHMDest->GetpTermUnit()->GetDpType() )
				{
					if( lColumnDest >= CD_Circuit_UnitQ && lColumnDest <= CD_Circuit_UnitCv )
					{
						SetCellText( CD_Circuit_UnitKv, lRowDest, _T("") );
					}
					else
					{
						SetCellText( CD_CircuitInjSecSide_Kv, lRowDest, _T("") );
					}
				}
				else if( CDS_HydroMod::eDpType::Cv == pclHMDest->GetpTermUnit()->GetDpType() )
				{
					if( lColumnDest >= CD_Circuit_UnitQ && lColumnDest <= CD_Circuit_UnitCv )
					{
						SetCellText( CD_Circuit_UnitCv, lRowDest, _T("") );
					}
					else
					{
						SetCellText( CD_CircuitInjSecSide_Cv, lRowDest, _T("") );
					}
				}
			}

			if( CDS_HydroMod::eDpType::Dp == pclHMSource->GetpTermUnit()->GetDpType() )
			{
				pclHMDest->GetpTermUnit()->SetDp( pclHMSource->GetpTermUnit()->GetDp() );
				
				if( lColumnDest >= CD_Circuit_UnitQ && lColumnDest <= CD_Circuit_UnitCv )
				{
					SetCellText( CD_Circuit_UnitDp, lRowDest, pclHMSource->GetpTermUnit()->GetDpString() );
				}
				else
				{
					SetCellText( CD_CircuitInjSecSide_Dp, lRowDest, pclHMSource->GetpTermUnit()->GetDpString() );
				}
			}
			else if( CDS_HydroMod::eDpType::QDpref == pclHMSource->GetpTermUnit()->GetDpType() )
			{
				CDS_HydroMod::eDpType eDpType = pclHMDest->GetpTermUnit()->GetDpType();

				// HYS-1414 : When the drag and drop process is on 'Dpref', 'Qref' must not change. Vice versa.
				if( CD_Circuit_UnitQref == lColumnDest || CD_CircuitInjSecSide_Qref == lColumnDest )
				{
					// Drag and drop is on 'Qref'.
					
					if( eDpType == pclHMSource->GetpTermUnit()->GetDpType() )
					{
						// Dp type of the destination terminal unit is the same as the source.
						// We copy 'Qref' src in 'Qref' dest and calculate 'Dp' dest with the 'Qref' src and 'DpRef' dest.
						pclHMDest->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, pclHMDest->GetpTermUnit()->GetDpRef(), pclHMSource->GetpTermUnit()->GetQRef() );
					}
					else
					{
						// Dp tpye of the destination terminal unit is not the same as the source.
						// We need thus to copy both 'Qref' and 'Dpref' from the source to the destination.
						pclHMDest->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, pclHMSource->GetpTermUnit()->GetDpRef(), pclHMSource->GetpTermUnit()->GetQRef() );
						SetCellText( ( CD_Circuit_UnitQref == lColumnDest ) ? CD_Circuit_UnitDpref : CD_CircuitInjSecSide_Dpref, lRowDest, pclHMSource->GetpTermUnit()->GetDpRefString() );
					}
					
					SetCellText( ( CD_Circuit_UnitQref == lColumnDest ) ? CD_Circuit_UnitQref : CD_CircuitInjSecSide_Qref, lRowDest, pclHMDest->GetpTermUnit()->GetQRefString() );
				}
				else
				{
					// Drag and drop is on 'Dpref'.
					
					if( eDpType == pclHMSource->GetpTermUnit()->GetDpType() )
					{
						// Dp type of the destination terminal unit is the same as the source.
						// We copy 'Dpref' src in 'Dpref' dest and calculate 'Dp' dest with the 'Dpref' src and 'Qref' dest.
						pclHMDest->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, pclHMSource->GetpTermUnit()->GetDpRef(), pclHMDest->GetpTermUnit()->GetQRef() );
					}
					else
					{
						// Dp tpye of the destination terminal unit is not the same as the source.
						// We need thus to copy both 'Qref' and 'Dpref' from the source to the destination.
						pclHMDest->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, pclHMSource->GetpTermUnit()->GetDpRef(), pclHMSource->GetpTermUnit()->GetQRef() );
						SetCellText( ( CD_Circuit_UnitDpref == lColumnDest ) ? CD_Circuit_UnitQref : CD_CircuitInjSecSide_Qref, lRowDest, pclHMSource->GetpTermUnit()->GetQRefString() );
					}

					SetCellText( ( CD_Circuit_UnitDpref == lColumnDest ) ? CD_Circuit_UnitDpref : CD_CircuitInjSecSide_Dpref, lRowDest, pclHMDest->GetpTermUnit()->GetDpRefString() );
				}
				
				long CD_DP = ( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) ) ? CD_Circuit_UnitDp : CD_CircuitInjSecSide_Dp;
				SetCellText( CD_DP, lRowDest, pclHMDest->GetpTermUnit()->GetDpString() );
			}
			else if( CDS_HydroMod::eDpType::Kv == pclHMSource->GetpTermUnit()->GetDpType() )
			{
				pclHMDest->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Kv, pclHMSource->GetpTermUnit()->GetKv() );

				if( lColumnDest >= CD_Circuit_UnitQ && lColumnDest <= CD_Circuit_UnitCv )
				{
					SetCellText( CD_Circuit_UnitDp, lRowDest, pclHMSource->GetpTermUnit()->GetDpString() );
					SetCellText( CD_Circuit_UnitKv, lRowDest, pclHMSource->GetpTermUnit()->GetKvString() );
				}
				else
				{
					SetCellText( CD_CircuitInjSecSide_Dp, lRowDest, pclHMSource->GetpTermUnit()->GetDpString() );
					SetCellText( CD_CircuitInjSecSide_Kv, lRowDest, pclHMSource->GetpTermUnit()->GetKvString() );
				}
			}
			else if( CDS_HydroMod::eDpType::Cv == pclHMSource->GetpTermUnit()->GetDpType() )
			{
				pclHMDest->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Cv, pclHMSource->GetpTermUnit()->GetCv() );

				if( lColumnDest >= CD_Circuit_UnitQ && lColumnDest <= CD_Circuit_UnitCv )
				{
					SetCellText( CD_Circuit_UnitDp, lRowDest, pclHMSource->GetpTermUnit()->GetDpString() );
					SetCellText( CD_Circuit_UnitCv, lRowDest, pclHMSource->GetpTermUnit()->GetCvString() );
				}
				else
				{
					SetCellText( CD_CircuitInjSecSide_Dp, lRowDest, pclHMSource->GetpTermUnit()->GetDpString() );
					SetCellText( CD_CircuitInjSecSide_Cv, lRowDest, pclHMSource->GetpTermUnit()->GetCvString() );
				}
			}
		}
	}
	else
	{
		_SaveAndCompute( lColumnDest, lRowDest, false );
		SetCellText( lColumnDest, lRowDest, m_strCellText );
	}

	m_lLastEditedRow = lRowDest;
	return true;
}

bool CSheetHMCalc::_SelectRow( long lRow )
{
	bool bSelect = false;
	bool bShiftKey = ( ( ::GetAsyncKeyState( VK_SHIFT ) & 0xFF00 ) ) ? true : false;
	bool bCtrlKey = ( ( ::GetAsyncKeyState( VK_CONTROL ) & 0xFF00 ) ) ? true : false;

	// Verify if row is selectable...
	bSelect = _IsRowSelectable( lRow );

	if( false == bSelect )
	{
		return false;
	}

	CArray<long> arlSelRows;

	////////////////////////////////////////////////////////////////////////////////////////
	// FIRST STEP: check what to do with previous selected rows.
	////////////////////////////////////////////////////////////////////////////////////////

	// If there is some rows selected...
	if( GetSelectedRows( &arlSelRows ) > 0 )
	{
		// If any of both control or shift key was not previously pressed...
		if( !( bCtrlKey || bShiftKey ) )
		{
			// ... unselect all previous rows.
			UnSelectMultipleRows();
		}
		else
		{
			// Run all rows selected.
			for( int i = 0; i < arlSelRows.GetCount(); i++ )
			{
				// If selected row is not the same continue.
				if( arlSelRows.GetAt( i ) != lRow )
				{
					continue;
				}

				// We found the same row that was previously selected.

				// Unselect it.
				UnSelectMultipleRows( lRow );

				// Clean list of selected rows.
				arlSelRows.RemoveAll();

				// If there is yet some selected rows...
				if( GetSelectedRows( &arlSelRows ) >= 1 )
				{
					if( m_lMSSelectedRow == lRow )
					{
						m_lMSSelectedRow = arlSelRows.GetAt( 0 );
					}
				}
				else
				{
					m_lMSSelectedRow = 0;
				}

				return false;
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// SECOND STEP: do the new selection.
	////////////////////////////////////////////////////////////////////////////////////////

	if( true == bSelect )
	{
		// Multi lines.
		if( true == bCtrlKey && true == bShiftKey )
		{
		}
		else if( true == bCtrlKey )
		{
			// If control key was pressed, then we add the selection in the list.
			m_lMSSelectedRow = lRow;
			SelectOneRow( lRow, 2 );
		}
		else if( true == bShiftKey )
		{
			// If shift key was pressed, then we do a selection between previous selected row and
			// current row.

			// First of all we clean all selected rows.
			UnSelectMultipleRows();

			// If no previous selected row...
			if( 0 == m_lMSSelectedRow )
			{
				m_lMSSelectedRow = lRow;
			}

			// Set range between previous and current.
			long lMin = m_lMSSelectedRow;
			long lMax = lRow;

			// Swap if needed.
			if( lRow < m_lMSSelectedRow )
			{
				lMin = lRow;
				lMax = m_lMSSelectedRow;
			}

			// Run all new selected row.
			for( long lLoopRow = lMin; lLoopRow <= lMax; lLoopRow++ )
			{
				// Is row is selectable.
				if( false == _IsRowSelectable( lLoopRow ) )
				{
					continue;
				}

				SelectOneRow( lLoopRow, 2 );
			}
		}
		else
		{
			// If no control or shift key, we select row as usual.
			m_lMSSelectedRow = lRow;
			SelectOneRow( lRow, 2 );
		}
	}

	return bSelect;
}

void CSheetHMCalc::_HideEmptyColumns()
{
	long lColStart, lColEnd, lRowStart, lButDlg;

	// In regards to current sheet, we must set some variables.
	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		lColStart = CD_Distribution_Pos;
		lColEnd = CD_Distribution_Pointer;
		lRowStart = RD_Header_FirstCirc;
		lButDlg = RD_Header_ButDlg;
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		lColStart = CD_Circuit_Pos;
		lColEnd = CD_Circuit_Pointer;
		lRowStart = RD_Header_FirstCirc;
		lButDlg = RD_Header_ButDlg;
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		lColStart = CD_CircuitInjSecSide_Pos;
		lColEnd = CD_CircuitInjSecSide_Pointer;
		lRowStart = RD_Header_FirstCirc;
		lButDlg = RD_Header_ButDlg;
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SMeasurement ) )
	{
		lColStart = CD_Measurement_Pos;
		lColEnd = CD_Measurement_Pointer;
		lRowStart = RD_Header_FirstCirc;
		lButDlg = RD_Header_ButDlg;
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::STADiagnostic ) )
	{
		lColStart = CD_TADiagnostic_Pos;
		lColEnd = CD_TADiagnostic_Pointer;
		lRowStart = RD_Header_FirstCirc;
		lButDlg = RD_Header_ButDlg;
	}
	else
	{
		return;
	}

	// Run all columns
	for( long lLoopCol = lColStart; lLoopCol <= lColEnd; lLoopCol++ )
	{
		// If column can be hidden and can be collapsed...
		if( false == IsFlagShowEvenEmpty( lLoopCol ) && ( false == IsColManagedByButtonCollapse( lLoopCol, lButDlg ) || true == IsFlagCanBeCollapsed( lLoopCol ) ) )
		{
			bool bIsColEmpty = true;

			// If current column is not already hidden...
			if( FALSE == IsColHidden( lLoopCol ) )
			{
				// Run all rows.
				for( long lLoopRow = lRowStart; lLoopRow <= GetMaxRows() && bIsColEmpty; lLoopRow++ )
				{
					bIsColEmpty = GetCellText( lLoopCol, lLoopRow ).IsEmpty();

					if( false == bIsColEmpty )
					{
						// A element exist, no need to run all rows
						break;
					}
				}

				// Hide or not the current column
				ShowCol( lLoopCol, ( true == bIsColEmpty ) ? FALSE : TRUE );
			}
		}
	}

	// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
	// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
	_RescanResizeColumnInfo( m_nCurrentSheet );
}

bool CSheetHMCalc::_ArrowKeyDown( long lFromRow )
{
	if( lFromRow <= 1 || lFromRow > GetMaxRows() )
	{
		return false;
	}

	long lBeginRow = lFromRow + 1;
	long lEndRow = GetMaxRows();

	// Run range.
	for( long lLoopRow = lBeginRow; lLoopRow <= lEndRow; lLoopRow++ )
	{
		// If one row can be selected...
		if( true == _SelectRow( lLoopRow ) )
		{
			return true;
		}
	}

	// If no solution, stay on the same line.
	_SelectRow( lFromRow );
	return false;
}

bool CSheetHMCalc::_ArrowKeyUp( long lFromRow )
{
	if( lFromRow <= 1 || lFromRow > GetMaxRows() )
	{
		return false;
	}

	if( lFromRow - 1 <= RD_Header_FirstCirc )
	{
		return false;
	}
	else
	{
		long lBeginRow = lFromRow - 1;
		long lEndRow = RD_Header_FirstCirc;

		for( long lLoopRow = lBeginRow; lLoopRow >= lEndRow; lLoopRow-- )
		{
			if( true == _SelectRow( lLoopRow ) )
			{
				return true;
			}
		}
	}

	// If no solution, stay on the same line.
	_SelectRow( lFromRow );
	return false;
}

void CSheetHMCalc::_SetGroupColor( COLORREF &Color )
{
	SetTextPattern( TitleGroup );

	if( Color != _IMI_TITLE_GROUP2 )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );
	}

	Color = GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor );
}

void CSheetHMCalc::_RefreshMainCollapseExpandButton()
{
	long lButtonColumn = 0;
	long lFromColumn;
	long lToColumn;

	// If there is some columns in m_xxxSummary CArray variable, it means there is
	// some columns collapsed. But it doesn't means that all buttons are collapsed !!

	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		lButtonColumn = CD_Distribution_Pos;
		lFromColumn = CD_Distribution_Pos;
		lToColumn = CD_Distribution_Pointer;
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		lButtonColumn = CD_Circuit_Pos;
		lFromColumn = CD_Circuit_Pos;
		lToColumn = CD_Circuit_Pointer;
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		lButtonColumn = CD_CircuitInjSecSide_Pos;
		lFromColumn = CD_CircuitInjSecSide_Pos;
		lToColumn = CD_CircuitInjSecSide_Pointer;
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SMeasurement ) )
	{
		lButtonColumn = CD_Measurement_Pos;
		lFromColumn = CD_Measurement_Pos;
		lToColumn = CD_Measurement_Pointer;
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::STADiagnostic ) )
	{
		lButtonColumn = CD_TADiagnostic_Pos;
		lFromColumn = CD_TADiagnostic_Pos;
		lToColumn = CD_TADiagnostic_Pointer;
	}
	else
	{
		return;
	}

	// Run all columns and check state of all Expand/Collapse columns button to eventually change state of main Expand/Collapse column button.
	// See CSSheet.h for more details about parameters.
	VerifyMainExpandCollapseButton( lButtonColumn, RD_Header_ButDlg, RD_Header_ButDlg, lFromColumn, lToColumn );
}

void CSheetHMCalc::_ExpandAll()
{
	SheetDescription eSheetDescription = GetSheetDescription( m_nCurrentSheet );

	if( SheetDescription::SUndefined == eSheetDescription )
	{
		return;
	}

	long lColStart = 0;
	long lColEnd = 0;

	switch( eSheetDescription )
	{
		case SheetDescription::SDistribution:
			lColStart = CD_Distribution_Pos;
			lColEnd = CD_Distribution_Pointer;
			break;

		case SheetDescription::SCircuit:
			lColStart = CD_Circuit_Pos;
			lColEnd = CD_Circuit_Pointer;
			break;

		case SheetDescription::SCircuitInj:
			lColStart = CD_CircuitInjSecSide_Pos;
			lColEnd = CD_CircuitInjSecSide_Pointer;
			break;

		case SheetDescription::SMeasurement:
			lColStart = CD_Measurement_Pos;
			lColEnd = CD_Measurement_Pointer;
			break;

		case SheetDescription::STADiagnostic:
			lColStart = CD_TADiagnostic_Pos;
			lColEnd = CD_TADiagnostic_Pointer;
			break;
	}

	if( lColStart > 0 && lColEnd > 0 )
	{
		if( false == m_bPrinting && false == m_bExporting )
		{
			m_arSheetGroupExpandedList[m_nCurrentSheet].clear();
		}

		for( long lLoopColumn = lColStart; lLoopColumn < lColEnd; lLoopColumn++ )
		{
			if( CSSheet::ExpandCollapseColumnState::ECCS_Collapsed == GetExpandCollapseColumnState( lLoopColumn, RD_Header_ButDlg ) )
			{
				ChangeExpandCollapseColumnButtonState( lLoopColumn, RD_Header_ButDlg, RD_Header_FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_ToExpand );

				if( false == m_bPrinting && false == m_bExporting )
				{
					m_arSheetGroupExpandedList[m_nCurrentSheet].push_back( lLoopColumn );
				}
			}
		}
	}

	_HideEmptyColumns();

	// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
	// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
	_RescanResizeColumnInfo( m_nCurrentSheet );
}

void CSheetHMCalc::_CollapseAll()
{
	SheetDescription eSheetDescription = GetSheetDescription( m_nCurrentSheet );

	if( SheetDescription::SUndefined == eSheetDescription )
	{
		return;
	}

	long lColStart = 0;
	long lColEnd = 0;

	switch( eSheetDescription )
	{
		case SheetDescription::SDistribution:
			lColStart = CD_Distribution_Pos;
			lColEnd = CD_Distribution_Pointer;
			break;

		case SheetDescription::SCircuit:
			lColStart = CD_Circuit_Pos;
			lColEnd = CD_Circuit_Pointer;
			break;

		case SheetDescription::SCircuitInj:
			lColStart = CD_CircuitInjSecSide_Pos;
			lColEnd = CD_CircuitInjSecSide_Pointer;
			break;

		case SheetDescription::SMeasurement:
			lColStart = CD_Measurement_Pos;
			lColEnd = CD_Measurement_Pointer;
			break;

		case SheetDescription::STADiagnostic:
			lColStart = CD_TADiagnostic_Pos;
			lColEnd = CD_TADiagnostic_Pointer;
			break;
	}

	if( lColStart > 0 && lColEnd > 0 )
	{
		if( false == m_bPrinting && false == m_bExporting )
		{
			m_arSheetGroupExpandedList[m_nCurrentSheet].clear();
		}

		for( long lLoopColumn = lColStart; lLoopColumn < lColEnd; lLoopColumn++ )
		{
			if( CSSheet::ExpandCollapseColumnState::ECCS_Expanded == GetExpandCollapseColumnState( lLoopColumn, RD_Header_ButDlg ) )
			{
				ChangeExpandCollapseColumnButtonState( lLoopColumn, RD_Header_ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
			}
		}
	}

	// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
	// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
	_RescanResizeColumnInfo( m_nCurrentSheet );
}

bool CSheetHMCalc::_CheckValidity( double dVal )
{
	if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SDistribution ) )
	{
		switch( m_lLastEditedCol )
		{
			case CD_Distribution_SupplyPipeLength:
				if( dVal >= 0.0 )
				{
					return true;
				}

			case CD_Distribution_ReturnPipeLength:
				if( dVal >= 0.0 )
				{
					return true;
				}

				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuit ) )
	{
		switch( m_lLastEditedCol )
		{
			case CD_Circuit_UnitQ:
			case CD_Circuit_UnitP:
			case CD_Circuit_UnitDT:
			case CD_Circuit_UnitDp:
			case CD_Circuit_UnitQref:
			case CD_Circuit_UnitDpref:
			case CD_Circuit_UnitKv:
			case CD_Circuit_UnitCv:
			case CD_Circuit_PumpHUser:
			case CD_Circuit_CVKvs:
				if( dVal > 0.0 )
				{
					return true;
				}

			case CD_Circuit_PipeL:
				if( dVal >= 0.0 )
				{
					return true;
				}

				break;
		}
	}
	else if( m_nCurrentSheet == GetSheetNumber( SheetDescription::SCircuitInj ) )
	{
		switch( m_lLastEditedCol )
		{
		    // HYS-1202: Add UnitDesc, P, DT, Qref, Dpref, Kv and Cv columns
			case CD_CircuitInjSecSide_Q:
			case CD_CircuitInjSecSide_P:
			case CD_CircuitInjSecSide_DT:
			case CD_CircuitInjSecSide_Dp:
			case CD_CircuitInjSecSide_Qref:
			case CD_CircuitInjSecSide_Dpref:
			case CD_CircuitInjSecSide_Kv:
			case CD_CircuitInjSecSide_Cv:
			case CD_CircuitInjSecSide_PumpH:
			case CD_CircuitInjSecSide_CVKvs:
				if( dVal > 0.0 )
				{
					return true;
				}

			case CD_CircuitInjSecSide_PipeL:
				if( dVal >= 0.0 )
				{
					return true;
				}

				break;

			case CD_CircuitInjSecSide_TempTss:
			case CD_CircuitInjSecSide_TempTrs:
				if( dVal > -273 )
				{
					return true;
				}

				break;
		}
	}

	return false;
}

void CSheetHMCalc::_SetStaticText( long lColumn, long lRow, int iTextID )
{
	CString str( TASApp.LoadLocalizedString( iTextID ) );
	_SetStaticText( lColumn, lRow, str );
}

void CSheetHMCalc::_SetStaticText( long lColumn, long lRow, CString strText )
{
	if( lRow < RD_Header_FirstCirc )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	}
	else
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
	}

	// Calling base class method.
	CSSheet::SetStaticText( lColumn, lRow, strText );
}

void CSheetHMCalc::_SetWaterCharSubTitle( int iColumnPos )
{
	if( NULL == m_pHM )
	{
		return;
	}

	// Variables.
	CString str;
	bool bSameChar = true;

	CDS_HydroMod *pHM = NULL;

	// Recuperate the first measurement.
	str = GetCellText( iColumnPos, RD_Header_FirstCirc );

	for( int i = RD_Header_FirstCirc; i < ( int )GetMaxRows(); i++ )
	{
		// Verify the first row did really had a measurement.
		if( _T("") == str )
		{
			str = GetCellText( iColumnPos, i );
		}

		if( NULL == pHM )
		{
			pHM = dynamic_cast<CDS_HydroMod *>( (CData *)GetCellParam( GetMaxCols(), i ) );

			// Don't take this hydromod if there is no measure data in it.
			if( NULL != pHM && 0 == pHM->GetMeasDataSize() )
			{
				pHM = NULL;
			}
		}

		// Verify it's the same text.
		if( _T("") != GetCellText( iColumnPos, i ) && str != GetCellText( iColumnPos, i ) )
		{
			bSameChar = false;
			break;
		}
	}

	// Verify the boolean to set the WaterCharacteristic
	// in the sub title and hide the water characteristic column.
	if( true == bSameChar )
	{
		CString str1, str2, UnitStr;

		if( NULL != pHM )
		{
			// All the same, 'pHM' is the first one of displayed hydraulic circuits.
			ASSERT( NULL != pHM->GetpMeasData( 0 ) );

			if( NULL != pHM->GetpMeasData( 0 ) && NULL != pHM->GetpMeasData( 0 )->GetpWC() )
			{
				CWaterChar *pclWaterChar = pHM->GetpMeasData( 0 )->GetpWC();

				// HYS-1591 : For non HM Calc project we have no measurement so we must take the project temperature.
				if( str == L"" )
				{
					str = WriteDouble( pclWaterChar->GetTemp(), 2 );
				}
				
				pclWaterChar->BuildWaterStrings( str1, str2 );
				CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
				ASSERT( pUnitDB );
				UnitStr = GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str();
				str2 = TASApp.LoadLocalizedString( IDS_TEMPERATURE ) + CString( ": " ) + str + CString( " " ) ;

				str = str1 + _T(" ") + str2;
				str.Format( L"%s %s %s", str1, str2, UnitStr );
			}
		}

		// Redefine the pattern.
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_IMI_TITLE_GROUP2 );

		// Set the info into the Sub Title.
		str2 = TASApp.LoadLocalizedString( IDS_HMCALC_CIRCUIT );
		str2 += _T("\r\n") + str;
		_SetStaticText( iColumnPos, RD_Header_GroupName, str2 );

		// Resize the Valve column to avoid Sub-Title reading problems.
		SetColWidth( iColumnPos + 1, 24 );

		// Show/Hide the unnecessary column.
		SetColumnAlwaysHidden( iColumnPos, iColumnPos, true );
	}
}

void CSheetHMCalc::_SetMinMaxFlowDevInSubTitle( int iColumnPos )
{
	// Variables.
	CString str, str1, str2;
	double dMinRatio = 9999;
	double dMaxRatio = -9999;
	bool bSetFlowDev = false;

	// Do a loop on the sheet to set the min and max values.
	for( long lRow = RD_Header_FirstCirc; lRow <= GetMaxRows(); lRow++ )
	{
		CString str = GetCellText( iColumnPos, lRow );

		if( str == _T( "" ) )
		{
			continue;
		}

		double dVal = _wtof( str );

		// Set the Min Ratio;
		if( dVal < dMinRatio )
		{
			dMinRatio = dVal;
			str1 = str;
		}

		// Set the Max Ratio.
		if( dVal > dMaxRatio )
		{
			dMaxRatio = dVal;
			str2 = str;
		}

		// The Min Max Text must be displayed.
		bSetFlowDev = true;
	}

	// Set the text.
	if( true == bSetFlowDev )
	{
		// Test if there is one or two values.
		if( dMinRatio != dMaxRatio )
		{
			str1 += _T("% / ");
			str1 += str2;
		}

		str1 += _T("%");

		// Redefine the pattern.
		OLE_COLOR color = GetBackColor( iColumnPos, RD_Header_GroupName );
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)color );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

		str = TASApp.LoadLocalizedString( IDS_HMCALC_FLOW );
		str += _T("\r\n") + str1;
		_SetStaticText( iColumnPos, RD_Header_GroupName, str );
	}
}

bool CSheetHMCalc::_SetHMName( long lRow )
{
	CDS_HydroMod *pHM = GetpHM( lRow );

	if( NULL == pHM )
	{
		return false;
	}

	CString str = m_strCellText;

	// Verify if the name is not already used.
	bool flag = false;

	CTable *pPipTab = m_pTADS->GetpHydroModTable();
	ASSERT( NULL != pPipTab );

	if( m_pTADS->FindHydroMod( str, pPipTab, pHM ) )
	{
		flag = true;
	}

	if( true == flag )
	{
		CString str1;
		::FormatString( str1, IDS_HYDROMODREF_ALREADYUSED, str );
		AfxMessageBox( str1 );
	}
	else
	{
		pHM->SetHMName( str );

		// Is automatic name rename it.
		if( true == pHM->GetHMName().IsEmpty() || '*' == pHM->GetHMName().GetAt( 0 ) )
		{
			pHM->RenameMe();
		}
	}

	// Change background color for parent module.
	if( pHM == m_pHM )
	{
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)m_rFillColors.m_ParentModuleBkg );
	}

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Name.
	SetCellText( CD_Distribution_Name, lRow, pHM->GetHMName() );

	return ( pHM == m_pHM ) ? true : false;
}

void CSheetHMCalc::_DrawCheckbox( long lColumn, long lRow, CDS_HydroMod::eUseProduct eState )
{
	switch( eState )
	{
		case CDS_HydroMod::eUseProduct::Auto:
			SetPictureCellWithID( IDI_SAUTOCHECK, lColumn, lRow, CSSheet::PictureCellType::Icon );
			break;

		case CDS_HydroMod::eUseProduct::Never:
			SetPictureCellWithID( IDI_SUNCHECK, lColumn, lRow, CSSheet::PictureCellType::Icon );
			break;

		case CDS_HydroMod::eUseProduct::Always:
			SetPictureCellWithID( IDI_SCHECK, lColumn, lRow, CSSheet::PictureCellType::Icon );
			break;
	}
}

void CSheetHMCalc::_RescanResizeColumnInfo( int iSSheetNumber )
{
	if( NULL != m_mapResizingColumnInfos[iSSheetNumber] )
	{
		m_mapResizingColumnInfos[iSSheetNumber]->Rescan();
	}
}

bool CSheetHMCalc::_IsRowSelectable( long lRow )
{
	bool fReturn = true;

	// If row is not in a valid range...
	if( lRow < RD_Header_FirstCirc || lRow > GetMaxRows() )
	{
		fReturn = false;
	}

	// Get cell data pointer (last cell contains pointer on a HydroMod).
	if( 0 == GetCellParam( GetMaxCols(), lRow ) )
	{
		fReturn = false;
	}

	return fReturn;
}

void CSheetHMCalc::_HideSuccessiveSeparator( long lFromCol, long lToCol, long lRow, UINT uiSheetDescriptionID )
{
	if( lFromCol < 1 || lFromCol > GetMaxCols() || lToCol < 1 || lToCol > GetMaxCols() ||
		lRow < RD_Header_FirstCirc || lRow > GetMaxRows() )
	{
		return;
	}

	if( lFromCol > lToCol )
	{
		long lTemp = lFromCol;
		lFromCol = lToCol;
		lToCol = lTemp;
	}

	bool fSeparator = false;
	bool fCheckSeparator = false;
	long lSeparatorPos = 0;

	for( long lLoop = lFromCol; lLoop <= lToCol; lLoop++ )
	{
		fCheckSeparator = false;

		switch( uiSheetDescriptionID )
		{
			case SheetDescription::SDistribution:
				if( CD_Distribution_SupplyPipeSep == lLoop || CD_Distribution_ReturnPipeSep == lLoop )
				{
					fCheckSeparator = true;
				}

				break;

			case SheetDescription::SCircuit:
				if( CD_Circuit_DescSep == lLoop || CD_Circuit_UnitSep == lLoop || CD_Circuit_PumpSep == lLoop
						|| CD_Circuit_CVSep == lLoop || CD_Circuit_ActuatorSep == lLoop || CD_Circuit_BvSep == lLoop
						|| CD_Circuit_SvSep == lLoop || CD_Circuit_DpCSep == lLoop || CD_Circuit_BvBypSep == lLoop
						|| CD_Circuit_PipeSep == lLoop || CD_Circuit_PipeAccSep == lLoop )
				{
					fCheckSeparator = true;
				}

				break;

			case SheetDescription::SCircuitInj:
				if( CD_CircuitInjSecSide_DescSep == lLoop || CD_CircuitInjSecSide_Sep == lLoop || CD_CircuitInjSecSide_PumpSep == lLoop
						|| CD_CircuitInjSecSide_TempSep == lLoop || CD_CircuitInjSecSide_CVSep == lLoop || CD_CircuitInjSecSide_ActuatorSep == lLoop
						|| CD_CircuitInjSecSide_BvSep == lLoop || CD_CircuitInjSecSide_PipeSep == lLoop )
				{
					fCheckSeparator = true;
				}

				break;

			case SheetDescription::SMeasurement:
				if( CD_Measurement_DescriptionSep == lLoop || CD_Measurement_KvSignalSep == lLoop || CD_Measurement_DplSep == lLoop
						|| CD_Measurement_FlowDeviationSep == lLoop || CD_Measurement_DiffTempDeviationSep == lLoop )
				{
					fCheckSeparator = true;
				}

				break;

			case SheetDescription::STADiagnostic:
				if( CD_TADiagnostic_DescriptionSep == lLoop || CD_TADiagnostic_DesignFlowSep == lLoop || CD_TADiagnostic_FlowDeviationSep == lLoop )
				{
					fCheckSeparator = true;
				}

				break;
		}

		if( true == fCheckSeparator )
		{
			if( false == fSeparator )
			{
				lSeparatorPos = lLoop;
				fSeparator = true;
			}
			else
			{
				ShowCol( lLoop, FALSE );
			}
		}
		else
		{
			if( true == fSeparator && FALSE == IsColHidden( lLoop ) )
			{
				fSeparator = false;
			}
		}
	}

	// If last column is a separator, hide it...
	// Remark: ONLY if the last separator doesn't contain a button expand/collapse.
	if( true == fSeparator && false == IsExpandCollapseColumnButton( lSeparatorPos, RD_Header_ButDlg ) )
	{
		ShowCol( lSeparatorPos, FALSE );
	}
}

void CSheetHMCalc::_RestoreGroupStateAndScrolling( void )
{
	short nCurrentSheet = m_nCurrentSheet;

	for( int iLoopSheet = SheetDescription::SFirst; iLoopSheet <= SheetDescription::SLast; iLoopSheet++ )
	{
		_SetCurrentSheet( iLoopSheet );

		for( int iLoopGroup = 0; iLoopGroup < ( int )m_arSheetGroupExpandedList[iLoopSheet].size(); iLoopGroup++ )
		{
			long lColumn = m_arSheetGroupExpandedList[iLoopSheet][iLoopGroup];
			ChangeExpandCollapseColumnButtonState( lColumn, RD_Header_ButDlg, RD_Header_FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_ToExpand );
		}

		_RefreshMainCollapseExpandButton();

		if( m_arptLeftTop[iLoopSheet].x >= GetMaxCols() || m_arptLeftTop[iLoopSheet].y >= GetMaxRows() )
		{
			m_arptLeftTop[iLoopSheet] = CPoint( 0, 0 );
		}

		ShowCell( m_arptLeftTop[iLoopSheet].x, m_arptLeftTop[iLoopSheet].y, SS_SHOW_TOPLEFT );
	}

	_SetCurrentSheet( nCurrentSheet );
}

void CSheetHMCalc::_AddChildrenInCutList( CDS_HydroMod *pclParentHM )
{
	IDPTR ChildIDPtr = pclParentHM->GetFirst();

	while( NULL != ChildIDPtr.MP )
	{
		CDS_HydroMod *pChildHM = dynamic_cast<CDS_HydroMod *>( ChildIDPtr.MP );

		if( NULL != pChildHM )
		{
			m_vecCutRow.push_back( pChildHM );

			if( true == pChildHM->IsaModule() )
			{
				_AddChildrenInCutList( pChildHM );
			}
		}

		ChildIDPtr = pclParentHM->GetNext();
	}
}

bool CSheetHMCalc::_PrepareColors( CDS_HydroMod *pHM )
{
	bool fIsCutMode = false;

	if( m_vecCutRow.size() > 0 && m_vecCutRow.end() != std::find( m_vecCutRow.begin(), m_vecCutRow.end(), pHM ) )
	{
		m_rFillColors.m_EditBkg = _EDITCOLOR;
		m_rFillColors.m_DiversityBkg = _TAH_DIVERSITYFACTOR;
		m_rFillColors.m_ParentModuleBkg = _LIGHTLILA;
		m_rFillColors.m_ModuleCircuitBkg = _WHITE;
		m_rFillColors.m_ModuleCircuitBkg2nd = _TAH_ORANGE_XXLIGHT;
		m_rFillColors.m_NormalFore = _TAH_BLACK_LIGHT;
		m_rFillColors.m_IndexFore = _LIGHTINDEXCOLOR;
		m_rFillColors.m_ErrorFore = _LIGHTRED;
		m_rFillColors.m_NotTAFore = _VLIGHTGRAY;
		fIsCutMode = true;
	}
	else
	{
		m_rFillColors.m_EditBkg = _EDITCOLOR;
		m_rFillColors.m_DiversityBkg = _TAH_DIVERSITYFACTOR;
		m_rFillColors.m_ParentModuleBkg = _LILA;
		m_rFillColors.m_ModuleCircuitBkg = _WHITE;
		m_rFillColors.m_ModuleCircuitBkg2nd = _TAH_ORANGE_XXLIGHT;
		m_rFillColors.m_NormalFore = _TAH_BLACK;
		m_rFillColors.m_IndexFore = _INDEXCOLOR;
		m_rFillColors.m_ErrorFore = _RED;
		m_rFillColors.m_NotTAFore = _VDARKGRAY;
	}

	return fIsCutMode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Code to change: it's just for resizing columns. In a near future, SheetHMCalc must be changed to be managed in the same way as the
// individual selection (no more CSheetHMCalc, creation of RViewHMCalc inherited from RViewBase).
// This method is called when user click on a title group (or when column width are not yet initialized).
bool CSheetHMCalc::_ResetColumnWidth( UINT uiSheetDescriptionID )
{
	switch( uiSheetDescriptionID )
	{
		case SheetDescription::SDistribution:
		{
			mapShortLong &mapColumnWidth = m_mapSSheetColumnWidth[SheetDescription::SDistribution];
			mapColumnWidth[CD_Distribution_Sub] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_Distribution_Pos] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_Distribution_Name] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Distribution_Desc] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Distribution_Type] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Distribution_Q] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_H] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_SupplyPicture] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_Distribution_SupplyPipeSeries] = ColWidthToLogUnits( 18 );
			mapColumnWidth[CD_Distribution_SupplyPipeSize] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_SupplyPipeLength] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_SupplyPipeDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_SupplyPipeTotalQ] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_SupplyDiversityFactor] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_SupplyPipeLinDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_SupplyPipeVelocity] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_SupplyPipeSep] = ColWidthToLogUnits( 1.5 );
			mapColumnWidth[CD_Distribution_SupplyAccDesc] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Distribution_SupplyAccData] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_SupplyAccDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_ReturnPicture] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_Distribution_ReturnPipeSeries] = ColWidthToLogUnits( 18 );
			mapColumnWidth[CD_Distribution_ReturnPipeSize] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_ReturnPipeLength] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_ReturnPipeDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_ReturnPipeTotalQ] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_ReturnDiversityFactor] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_ReturnPipeLinDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_ReturnPipeVelocity] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_ReturnPipeSep] = ColWidthToLogUnits( 1.5 );
			mapColumnWidth[CD_Distribution_ReturnAccDesc] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Distribution_ReturnAccData] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Distribution_ReturnAccDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Distribution_Pointer] = ColWidthToLogUnits( 1 );
		}
		break;

		case SheetDescription::SCircuit:
		{
			mapShortLong &mapColumnWidth = m_mapSSheetColumnWidth[SheetDescription::SCircuit];
			mapColumnWidth[CD_Circuit_Sub] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_Circuit_Pos] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_Circuit_Name] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Circuit_Desc] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Circuit_DescSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_UnitDesc] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Circuit_UnitQ] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_UnitP] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_UnitDT] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_UnitDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_UnitQref] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_UnitDpref] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_UnitKv] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_UnitCv] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_UnitSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_PumpHUser] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_PumpHMin] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_PumpSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_CVName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_CVDesc] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Circuit_CVKvsMax] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_CVKvs] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_CVSet] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_CVDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_CVDpMin] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_CVAuth] = ColWidthToLogUnits( 7 );
			mapColumnWidth[CD_Circuit_CVDesignAuth] = ColWidthToLogUnits( 7 );
			mapColumnWidth[CD_Circuit_CVSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_ActuatorName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_ActuatorPowerSupply] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_ActuatorInputSignal] = ColWidthToLogUnits( 20 );
			mapColumnWidth[CD_Circuit_ActuatorFailSafe] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Circuit_ActuatorDRP] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Circuit_ActuatorSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_SmartValveName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_SmartValveDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_SmartValveDpMin] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_SmartValveControlMode] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Circuit_SmartValveLocation] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Circuit_SmartValveKvs] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_SmartValveMoreInfo] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_SmartValveProductSet] = ColWidthToLogUnits( 18 );
			mapColumnWidth[CD_Circuit_SmartValveDpSensor] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_SmartValveDpl] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_SmartValveSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_BvName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_BvSet] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_BvDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_BvDpSig] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_BvDpMin] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_BvSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_SvPict] = ColWidthToLogUnits( 1.2 );
			mapColumnWidth[CD_Circuit_SvName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_SvDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_SvSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_DpCName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_DpCDpLr] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Circuit_DpCSet] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_DpCDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_DpCDpl] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_DpCDpMin] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_DpCSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_BvBypPict] = ColWidthToLogUnits( 1.2 );
			mapColumnWidth[CD_Circuit_BvBypName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_BvBypSet] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_BvBypSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_PipeSerie] = ColWidthToLogUnits( 18 );
			mapColumnWidth[CD_Circuit_PipeSize] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Circuit_PipeL] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Circuit_PipeDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_PipeLinDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_PipeV] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Circuit_PipeSep] = ColWidthToLogUnits( 1.5 );
			mapColumnWidth[CD_Circuit_PipeAccDesc] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_Circuit_PipeAccData] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Circuit_PipeAccDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_Circuit_PipeAccSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Circuit_Pointer] = ColWidthToLogUnits( 0.8 );
			}
		break;

		case SheetDescription::SCircuitInj:
		{
			mapShortLong &mapColumnWidth = m_mapSSheetColumnWidth[SheetDescription::SCircuitInj];
			mapColumnWidth[CD_CircuitInjSecSide_Sub] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_CircuitInjSecSide_Pos] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_CircuitInjSecSide_Name] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_CircuitInjSecSide_Desc] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_CircuitInjSecSide_DescSep] = ColWidthToLogUnits( 0.8 );
			// HYS-1202: Add UnitDesc, P, DT, Qref, Dpref, Kv and Cv columns
			mapColumnWidth[CD_CircuitInjSecSide_UnitDesc] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_CircuitInjSecSide_Q] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_P] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_DT] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_Dp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_Qref] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_Dpref] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_Kv] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_Cv] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_Sep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_CircuitInjSecSide_PumpH] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_PumpHMin] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_PumpHAvailable] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_PumpSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_CircuitInjSecSide_TempTss] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_TempTrs] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_TempTsp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_TempSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_CircuitInjSecSide_CVName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_CircuitInjSecSide_CVDesc] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_CircuitInjSecSide_CVKvsMax] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_CVKvs] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_CVSet] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_CVDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_CVDpMin] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_CVAuth] = ColWidthToLogUnits( 7 );
			mapColumnWidth[CD_CircuitInjSecSide_CVDesignAuth] = ColWidthToLogUnits( 7 );
			mapColumnWidth[CD_CircuitInjSecSide_CVSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_CircuitInjSecSide_ActuatorName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_CircuitInjSecSide_ActuatorPowerSupply] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_CircuitInjSecSide_ActuatorInputSignal] = ColWidthToLogUnits( 20 );
			mapColumnWidth[CD_CircuitInjSecSide_ActuatorFailSafe] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_CircuitInjSecSide_ActuatorDRP] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_CircuitInjSecSide_ActuatorSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_CircuitInjSecSide_BvName] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_CircuitInjSecSide_BvSet] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_BvDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_BvDpSig] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_BvDpMin] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_BvSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_CircuitInjSecSide_PipeSerie] = ColWidthToLogUnits( 18 );
			mapColumnWidth[CD_CircuitInjSecSide_PipeSize] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_CircuitInjSecSide_PipeL] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_CircuitInjSecSide_PipeDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_PipeLinDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_PipeV] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_CircuitInjSecSide_PipeSep] = ColWidthToLogUnits( 1.5 );
			mapColumnWidth[CD_CircuitInjSecSide_AccDesc] = ColWidthToLogUnits( 15 );
			mapColumnWidth[CD_CircuitInjSecSide_AccData] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_CircuitInjSecSide_AccDp] = ColWidthToLogUnits( 6 );
			mapColumnWidth[CD_CircuitInjSecSide_Pointer] = ColWidthToLogUnits( 0.8 );
		}
		break;

		case SheetDescription::SMeasurement:
		{
			mapShortLong &mapColumnWidth = m_mapSSheetColumnWidth[SheetDescription::SMeasurement];
			mapColumnWidth[CD_Measurement_Sub] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_Measurement_Pos] = ColWidthToLogUnits( 5 );
			mapColumnWidth[CD_Measurement_DateTime] = ColWidthToLogUnits( 14 );
			mapColumnWidth[CD_Measurement_Reference] = ColWidthToLogUnits( 12 );
			mapColumnWidth[CD_Measurement_Description] = ColWidthToLogUnits( 10 );
			mapColumnWidth[CD_Measurement_DescriptionSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Measurement_WaterChar] = ColWidthToLogUnits( 19.2 );
			mapColumnWidth[CD_Measurement_Valve] = ColWidthToLogUnits( 12 );
			mapColumnWidth[CD_Measurement_Presetting] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_Setting] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_Kv] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_KvSignal] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_KvSignalSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Measurement_Dp] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_Dpl] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_DplSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Measurement_MeasureDp] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_DesignDp] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_MeasureFlow] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_DesignFlow] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_FlowDeviation] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_FlowDeviationSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Measurement_TempHH] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_Temp1DPS] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_Temp2DPS] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_TempRef] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_DiffTemp] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_DesignDiffTemp] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_DiffTempDeviation] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_DiffTempDeviationSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_Measurement_Power] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_DesignPower] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_PowerDeviation] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_Measurement_Pointer] = ColWidthToLogUnits( 1 );
		}
		break;

		case SheetDescription::STADiagnostic:
		{
			mapShortLong &mapColumnWidth = m_mapSSheetColumnWidth[SheetDescription::STADiagnostic];
			mapColumnWidth[CD_TADiagnostic_Sub] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_TADiagnostic_Pos] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_TADiagnostic_DateTime] = ColWidthToLogUnits( 14 );
			mapColumnWidth[CD_TADiagnostic_Reference] = ColWidthToLogUnits( 12 );
			mapColumnWidth[CD_TADiagnostic_Description] = ColWidthToLogUnits( 24 );
			mapColumnWidth[CD_TADiagnostic_DescriptionSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_TADiagnostic_WaterChar] = ColWidthToLogUnits( 19.2 );
			mapColumnWidth[CD_TADiagnostic_Valve] = ColWidthToLogUnits( 12 );
			mapColumnWidth[CD_TADiagnostic_Presetting] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_Setting] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_ComputedSetting] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_Kv] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_KvSignal] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_DesignFlow] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_DesignFlowSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_TADiagnostic_Dp1] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_Setting1] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_Dp2] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_MeasFlow] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_FlowDeviation] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_FlowDeviationSep] = ColWidthToLogUnits( 0.8 );
			mapColumnWidth[CD_TADiagnostic_CircuitDpMeas] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_CircuitDpDesign] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_Picture] = ColWidthToLogUnits( 3 );
			mapColumnWidth[CD_TADiagnostic_DistribDpMeas] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_DistribDpDesign] = ColWidthToLogUnits( 8 );
			mapColumnWidth[CD_TADiagnostic_Pointer] = ColWidthToLogUnits( 1 );
		}
		break;
	}

	return true;
}

void CSheetHMCalc::_ClearAllCombos( SheetDescription eSheetDescription )
{
	if( m_mapComboBoxList.count( eSheetDescription ) > 0 )
	{
		m_mapComboBoxList.erase( eSheetDescription );
	}
}

void CSheetHMCalc::_ClearOneLineCombos( SheetDescription eSheetDescription, long lRow )
{
	if( 0 == m_mapComboBoxList.count( eSheetDescription ) )
	{
		return;
	}

	for( mappointparam::iterator iterComboBox = m_mapComboBoxList.at( eSheetDescription ).begin(); iterComboBox != m_mapComboBoxList.at( eSheetDescription ).end(); )
	{
		if( lRow == iterComboBox->first.y )
		{
			mappointparam::iterator iterNextComboBox = iterComboBox;
			iterNextComboBox++;
			m_mapComboBoxList.at( eSheetDescription ).erase( iterComboBox );
			iterComboBox = iterNextComboBox;
		}
		else
		{
			iterComboBox++;
		}
	}
}

void CSheetHMCalc::_FormatComboList( SheetDescription eSheetDescription, long lColumn, long lRow, long lStyle )
{
	if( 0 == m_mapComboBoxList.count( eSheetDescription ) )
	{
		mappointparam mapPointParams;
		m_mapComboBoxList.insert( pairsheetdescriptionmapptparam( eSheetDescription, mapPointParams ) );
	}

	vecparam vecParams;
	m_mapComboBoxList.at( eSheetDescription ).insert( pairptvecparam( CPoint( lColumn, lRow ), vecParams ) );

	FormatComboList( lColumn, lRow, lStyle );
}

void CSheetHMCalc::_AddComboBoxParam( SheetDescription eSheetDescription, long lColumn, long lRow, LPARAM lpParam )
{
	if( 0 == m_mapComboBoxList.count( eSheetDescription ) )
	{
		ASSERT_RETURN;
	}

	CPoint ptCombo( lColumn, lRow );

	if( 0 == m_mapComboBoxList.at( eSheetDescription ).count( ptCombo ) )
	{
		ASSERT_RETURN;
	}

	m_mapComboBoxList.at( eSheetDescription ).at( ptCombo ).push_back( lpParam );
}

LPARAM CSheetHMCalc::_GetComboBoxParam( SheetDescription eSheetDescription, long lColumn, long lRow, int iIndex )
{
	if( 0 == m_mapComboBoxList.count( eSheetDescription ) )
	{
		return NULL;
	}

	CPoint ptCombo( lColumn, lRow );

	if( 0 == m_mapComboBoxList.at( eSheetDescription ).count( ptCombo ) )
	{
		return NULL;
	}

	if( iIndex >= (int)m_mapComboBoxList.at( eSheetDescription ).at( ptCombo ).size() )
	{
		ASSERTA_RETURN( NULL );
	}

	return m_mapComboBoxList.at( eSheetDescription ).at( ptCombo ).at( iIndex );
}

void CSheetHMCalc::_VerifyComboBoxError()
{
	for( auto &iterSheetDescription : m_mapComboBoxList )
	{
		for( auto &iterComboBox : iterSheetDescription.second )
		{
			int iCurrentSel = -1;
			bool bSetTextInError = false;

			switch( iterSheetDescription.first )
			{
				case SheetDescription::SDistribution:
					break;

				case SheetDescription::SCircuit:

					if( CD_Circuit_SmartValveProductSet == iterComboBox.first.x )
					{
						iCurrentSel = ComboBoxSendMessage( iterComboBox.first.x, iterComboBox.first.y, SS_CBM_GETCURSEL, 0, 0 );

						if( -1 == iCurrentSel )
						{
							break;
						}

						CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( (CData *)_GetComboBoxParam( iterSheetDescription.first, iterComboBox.first.x, iterComboBox.first.y, iCurrentSel ) );

						if( NULL == pclDpSensor )
						{
							break;
						}

						CDS_HydroMod *pHM = GetpHM( iterComboBox.first.y );

						if( NULL == pHM || NULL == pHM->GetpSmartDpC() || 0.0 == pHM->GetpSmartDpC()->GetDpToStabilize() )
						{
							break;
						}

						double dDpToStabilize = pHM->GetpSmartDpC()->GetDpToStabilize();

						// Check if error.
						if( dDpToStabilize >= pclDpSensor->GetMinMeasurableDp() && dDpToStabilize <= pclDpSensor->GetMaxMeasurableDp() )
						{
							break;
						}

						bSetTextInError = true;
					}

					break;

				case SheetDescription::SCircuitInj:
					break;

				case SheetDescription::SMeasurement:
					break;

				case SheetDescription::STADiagnostic:
					break;
			}

			if( false == bSetTextInError )
			{
				continue;
			}

			// Set the combo box text in red.
			int iTextLength = ComboBoxSendMessage( iterComboBox.first.x, iterComboBox.first.y, SS_CBM_GETLBTEXTLEN, iCurrentSel, 0 );
	
			if( 0 == iTextLength )
			{
				continue;
			}
	
			GLOBALHANDLE hText = GlobalAlloc( GHND, ( iTextLength + 1 ) * sizeof( TCHAR ) );

			if( NULL == hText )
			{
				ASSERT_RETURN;
			}

			LPTSTR lpText = (LPTSTR)GlobalLock( hText );
			ComboBoxSendMessage( iterComboBox.first.x, iterComboBox.first.y, SS_CBM_GETLBTEXT , iCurrentSel, (LPARAM)lpText );
			
			SS_COORD lColAnchor;
			SS_COORD lRowAnchor;
			SS_COORD lNumCols;
			SS_COORD lNumRows;
			CRect clRectCell = GetCellCoordInPixel( iterComboBox.first.x, iterComboBox.first.y );

			if( SS_SPAN_NO != GetCellSpan( iterComboBox.first.x, iterComboBox.first.y, &lColAnchor, &lRowAnchor, &lNumCols, &lNumRows ) )
			{
				CRect clRectSelection( lColAnchor, lRowAnchor, lColAnchor + lNumCols - 1, lRowAnchor );
				CRect clRectSelectionPixels = GetSelectionInPixels( clRectSelection );
				clRectCell.right = clRectCell.left + clRectSelectionPixels.Width() - 1;
			}
	
			short nxMarging = XMARGIN;
			short nyMarging = YMARGIN;
			clRectCell.left += nxMarging;
			clRectCell.top += nyMarging;
			clRectCell.bottom -= nyMarging;

			long lColActive;
			long lRowActive;
			GetActiveCell( &lColActive, &lRowActive );

			if( lColActive == iterComboBox.first.x && lRowActive == iterComboBox.first.y )
			{
				clRectCell.right -= GetSystemMetrics(SM_CXVSCROLL);
			}
			else
			{
				clRectCell.right -= XMARGIN;
			}

			CDC *pDC = GetDC();
			HBRUSH hBrushBackground = CreateSolidBrush( GetBackColor( iterComboBox.first.x, iterComboBox.first.y ) );
			FillRect( pDC->GetSafeHdc(), &clRectCell, hBrushBackground );
			DeleteObject( hBrushBackground );

			HFONT hFont = GetFont( iterComboBox.first.x, iterComboBox.first.y );
			HFONT hFontOld = (HFONT)SelectObject( pDC->GetSafeHdc(), hFont );

			SetBkMode( pDC->GetSafeHdc(), TRANSPARENT );
			SetTextColor( pDC->GetSafeHdc(), RGB( 255, 0, 0 ) );
			
			SS_CELLTYPE rCellType;
			GetCellType( iterComboBox.first.x, iterComboBox.first.y, &rCellType );
			
			UINT uiFormat = 0;

			// Horizontal alignement.
			if( SSS_ALIGN_LEFT == ( rCellType.Style & SSS_ALIGN_LEFT ) )
			{
				uiFormat = DT_LEFT;
			}
			else if( SSS_ALIGN_CENTER == ( rCellType.Style & SSS_ALIGN_CENTER ) )
			{
				uiFormat = DT_CENTER;
			}
			else if( SSS_ALIGN_RIGHT == ( rCellType.Style & SSS_ALIGN_RIGHT ) )
			{
				uiFormat = DT_RIGHT;
			}

			// Vertical alignement.
			if( SSS_ALIGN_TOP == ( rCellType.Style & SSS_ALIGN_TOP ) )
			{
				uiFormat |= DT_TOP;
			}
			else if( SSS_ALIGN_VCENTER == ( rCellType.Style & SSS_ALIGN_VCENTER ) )
			{
				uiFormat |= DT_VCENTER;
			}
			else if( SSS_ALIGN_BOTTOM == ( rCellType.Style & SSS_ALIGN_BOTTOM ) )
			{
				uiFormat |= DT_BOTTOM;
			}
			
			DrawText( pDC->GetSafeHdc(), lpText, -1, &clRectCell, uiFormat | DT_NOPREFIX );
	
			SelectObject( pDC->GetSafeHdc(), hFontOld );
			GlobalUnlock( hText );
			GlobalFree( hText );
			ReleaseDC( pDC );
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
