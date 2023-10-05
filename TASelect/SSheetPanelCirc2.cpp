#include "stdafx.h"


#include <windowsx.h>
#include "afxctl.h"
#include "MainFrm.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "Global.h"
#include "Utilities.h"
#include "Units.h"
#include "Picture.h"
#include "database.h"
#include "datastruct.h"
#include "DataBObj.h"
#include "hminclude.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "wizard.h"
#include "EnBitmapPatchWork.h"
#include "DlgComboBoxHM.h"
#include "PanelCirc2.h"
#include "DlgAccessory.h"
#include "DlgInjectionError.h"
#include "DlgSelectActuator.h"
#include "SSheetPanelCirc2.h"

#define XMARGIN 2   // Pixels
#define YMARGIN 1

CSSheetPanelCirc2::CSSheetPanelCirc2()
{
	m_Box.Reset();
	m_pHMSCB = NULL;
	m_pHMSelectActuator = NULL;
	
	m_BoxPrimaryValues.Reset( _BoxType::ebPrimaryValues );
	m_BoxRem.Reset( _BoxType::ebRem );
	m_BoxPic.Reset( _BoxType::ebNoBox );
	m_BoxDpC.Reset( _BoxType::ebDPC );
	m_BoxTu.Reset( _BoxType::ebTU );
	m_BoxCv.Reset( _BoxType::ebCV );
	m_BoxBvP.Reset( _BoxType::ebBVP );
	m_BoxBvB.Reset( _BoxType::ebBVB );
	m_BoxBvS.Reset( _BoxType::ebBVS );
	m_BoxPump.Reset( _BoxType::ebPump );
	m_BoxSecondaryValues.Reset( _BoxType::ebSecondaryValues );
	m_BoxShutoffValve.Reset( _BoxType::ebShutoffValve );
	m_BoxDpCBCV.Reset( _BoxType::ebDpCBCValve );
	m_BoxSmartControlValve.Reset( _BoxType::ebSmartControlValve );
	m_BoxSmartDpC.Reset( _BoxType::ebSmartDpC );

	m_BoxCircuitPrimaryPipe.Reset( _BoxType::ebCircuitPrimaryPipe );
	m_BoxCircuitSecondaryPipe.Reset( _BoxType::ebCircuitSecondaryPipe );
	m_BoxDistributionSupplyPipe.Reset( _BoxType::ebDistributionPipe );
	m_BoxDistributionReturnPipe.Reset( _BoxType::ebDistributionReturnPipe );

	m_wLastKeyPressed = 0;
	m_wLastShiftCtrlStatus = 0;
	m_lRow = 0;
	m_lCol = 0;
	m_strCellText = _T("");
	m_dCellDouble = 0.0;
	m_bCellModified = false;
	m_bReady = false;
	m_bRedrawAllowed = false;
	m_pPrjParams = NULL;
	m_pPrjParamsSaved = NULL;
	m_dFontFactor = 0.0;
	m_bRefreshIncourse = false;
	m_bPrinting = false;
	m_bRefreshBoxPicture = false;
	m_mapColWidth.clear();
	m_bManageEditOffSpreadNotification = true;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pUnitDB = NULL;
	m_pHM = NULL;
	m_pWC = NULL;
	m_pclTableDN = NULL;
	m_pTechParam = NULL;
	ZeroMemory( &m_rCellCoord, sizeof( m_rCellCoord ) );
}

CSSheetPanelCirc2::~CSSheetPanelCirc2()
{
	if( NULL != m_pHMSCB )
	{
		delete m_pHMSCB;
	}

	if( NULL != m_pHMSelectActuator )
	{
		delete m_pHMSelectActuator;
	}

	if( NULL != m_pPrjParamsSaved )
	{
		delete m_pPrjParamsSaved;
	}
}

void CSSheetPanelCirc2::InitDefaultBox( int iFirstCVRow )
{
	m_Box.nRows = 38;
	m_Box.nCols = 15;

	// Terminal Unit.
	m_Box.TU_Top = 3;
	m_Box.TU_nRows = 8;
	m_Box.TU_Des = 1;				// First position in the box.
	m_Box.TU_Flow = 2;				// Second position in the box.
	m_Box.TU_Power = 1;				// First position after 'Flow' when 'Flow (P/DT)' (This second position in the box).
	m_Box.TU_DeltaT = 2;			// Second position after 'Flow' when 'Flow (P/DT)' (This third position in the box).
	m_Box.TU_Dp = 3;				// Third position in the box.
	m_Box.TU_QRef = 1;				// First position after 'Dp' when 'Dp (QRef/DpRef)'.
	m_Box.TU_DpRef = 2;				// Second position after 'Dp' when 'Dp (QRef/DpRef)'.

	// Pipe.
	m_Box.Pipe_Top = 13;
	m_Box.Pipe_Left = 2;
	m_Box.Pipe_nRows = 6;
	m_Box.Pipe_Serie = 1;
	m_Box.Pipe_Size = 2;
	m_Box.Pipe_Length = 3;
	m_Box.Pipe_Dp = 4;
	m_Box.Pipe_Acc = 5;

	// Dp Controller.
	m_Box.DpC_Top = 20;
	m_Box.DpC_Left = 2;
	m_Box.DpC_nRows = 8;
	m_Box.DpC_Name = 1;				// First position in the box.
	m_Box.DpC_Type = 1;				// First position after 'Name' when deployed (Thus second position in the box).
	m_Box.DpC_Connect = 2;			// Second position after 'Name' when deployed (Thus third position in the box).
	m_Box.DpC_PN = 3;				// Third position after 'Name' when deployed (Thus fourth position in the box).
	m_Box.DpC_Setting = 2;			// Second position in the box when 'Name' is not deployed.
	m_Box.DpC_Dp = 3;				// Third position in the box when 'Name' is not deployed.
	m_Box.DpC_DpL = 4;				// Fourth position in the box when 'Name' is not deployed.

	// Primary values.
	m_Box.PrimaryValues_Top = 27;
	m_Box.PrimaryValues_Left = 7;
	m_Box.PrimaryValues_nRows = 6;
	m_Box.PrimaryValues_Flow = 1;
	m_Box.PrimaryValues_Hmin = 2;
	m_Box.PrimaryValues_AvailH = 3;
	m_Box.PrimaryValues_Tsp = 4;
	m_Box.PrimaryValues_Trp = 5;

	// Control valve KV Mode.
	m_Box.CVKV_Top = 10;
	m_Box.CVKV_Left = 12;
	m_Box.CVKV_nRows = 6;
	m_Box.CV_Loc = 1;

	m_Box.CVKV_Des = iFirstCVRow;
	m_Box.CVKV_Kvsmax = m_Box.CVKV_Des + 1;
	m_Box.CVKV_Kvs = m_Box.CVKV_Kvsmax + 1;
	m_Box.CVKV_Dp = m_Box.CVKV_Kvs + 1;
	m_Box.CVKV_Auth = m_Box.CVKV_Dp + 1;

	// Control valve TA Mode.
	m_Box.CVTA_Top = 10;
	m_Box.CVTA_Left = 12;
	m_Box.CVTA_nRows = 7 + iFirstCVRow;	// 8 or 9
	m_Box.CVTA_Name = iFirstCVRow;						// First position in the box.
	m_Box.CVTA_Conn = 1;								// First position after 'Name' when deployed (Thus second position in the box).
	m_Box.CVTA_Vers = 2;								// Second position after 'Name' when deployed (Thus third position in the box).
	m_Box.CVTA_PN = 3;									// Third position after 'Name' when deployed (Thus fourth position in the box).
	m_Box.CVTA_Preset = 1;
	m_Box.CVTA_Dp = 1;
	m_Box.CVTA_Auth = 2;
	m_Box.CVTA_Actuator = 3;

	// Combined Dp controller, control and balancing valve.
	m_BoxDpCBCV.nRows = DPCBCVNROWS;
	m_BoxDpCBCV.DpCBCV_Name = 1;
	m_BoxDpCBCV.DpCBCV_Connect = 1;
	m_BoxDpCBCV.DpCBCV_Version = 2;
	m_BoxDpCBCV.DpCBCV_PN = 3;
	m_BoxDpCBCV.DpCBCV_Presetting = 2;
	m_BoxDpCBCV.DpCBCV_Dp = 3;
	m_BoxDpCBCV.DpCBCV_Actuator = 4;

	// Shut-off valve.
	m_BoxShutoffValve.nRows = 7;
	m_BoxShutoffValve.SV_Name = 1;
	m_BoxShutoffValve.SV_Type = 1;
	m_BoxShutoffValve.SV_Connect = 2;
	m_BoxShutoffValve.SV_Version = 3;
	m_BoxShutoffValve.SV_PN = 4;
	m_BoxShutoffValve.SV_Dp = 2;

	// Smart control valve.
	m_BoxSmartControlValve.nRows = SMARTCONTROLVALVENROWS;
	m_BoxSmartControlValve.iName = 1;
	m_BoxSmartControlValve.iBodyMaterial = 1;
	m_BoxSmartControlValve.iConnect = 2;
	m_BoxSmartControlValve.iPN = 3;
	m_BoxSmartControlValve.iLocation = 2;
	m_BoxSmartControlValve.iControlMode = 3;
	m_BoxSmartControlValve.iFlowMax = 4;
	m_BoxSmartControlValve.iPowerMax = 5;	// Additional row for power control mode.
	m_BoxSmartControlValve.iDp = 6;
	m_BoxSmartControlValve.iDpMin = 7;

	// Smart differential pressure controller.
	m_BoxSmartDpC.nRows = SMARTDPCNROWS;
	m_BoxSmartDpC.iName = 1;
	m_BoxSmartDpC.iBodyMaterial = 1;
	m_BoxSmartDpC.iConnect = 2;
	m_BoxSmartDpC.iPN = 3;
	m_BoxSmartDpC.iLocation = 2;
	m_BoxSmartDpC.iDp = 3;
	m_BoxSmartDpC.iDpMin = 4;
	m_BoxSmartDpC.iSets = 5;
	m_BoxSmartDpC.iDpl = 6;

	// Remark.
	m_Box.Rem_Top = 30;
	m_Box.Rem_Left = 2;
	m_Box.Rem_nRows = 3;
	m_Box.Rem_l1 = 1;

	// Pictures.
	// HYS-1716: we now set the picture to the 5th row because we need a place for the computed return temperature in 
	// the 'm_BoxSecondaryValues' box for injection circuits.
	m_Box.Pic_Top = 5;
	m_Box.Pic_Left = 7;
	m_Box.Pic_nRows = 28;
	m_Box.Pic_nCols = 4;
}

void CSSheetPanelCirc2::Init( CDS_HydroMod *pHM, CPoint ptInitialSize, bool fPrinting )
{
	SetBool( SSB_REDRAW, FALSE );

	int iFirstCVRow = 1;
	CDS_Hm3WInj *pHM3W = dynamic_cast<CDS_Hm3WInj *>( pHM );
	m_BoxCv.bLocalisationExist = false;

	CDB_CircuitScheme *pCircuitScheme = (CDB_CircuitScheme *)( pHM->GetSchemeIDPtr().MP);

	if( NULL != pHM3W )
	{
		if( NULL != pHM3W->GetpSchcat() && pHM3W->GetpSchcat()->Get3WType() == CDB_CircSchemeCateg::e3wTypeMixDecByp )
		{
			iFirstCVRow = 2;
			m_BoxCv.bLocalisationExist = true;
		}
	}

	InitDefaultBox( iFirstCVRow );

	if( NULL != pCircuitScheme )
	{
		bool bPreset = ( CDB_ControlProperties::eCVFUNC::PresetPT == pCircuitScheme->GetCvFunc() || CDB_ControlProperties::eCVFUNC::Presettable == pCircuitScheme->GetCvFunc() );

		if( true == bPreset )
		{
			m_Box.CVTA_nRows++ ;
		}
	}

	m_bPrinting = fPrinting;
	m_bReady = false;
	m_pHM = pHM;
	m_bRedrawAllowed = false;

	m_pTADS = TASApp.GetpTADS();
	ASSERT( NULL != m_pTADS );

	m_pTADB = TASApp.GetpTADB();
	ASSERT( NULL != m_pTADB );

	m_pUnitDB = CDimValue::AccessUDB();
	ASSERT( NULL != m_pUnitDB );

	m_pWC = m_pTADS->GetpWCForProject()->GetpWCData();
	ASSERT( NULL != m_pWC );

	m_pclTableDN = (CTableDN *)m_pTADB->Get( _T("DN_TAB") ).MP;
	ASSERT( NULL != m_pclTableDN );

	m_pTechParam = m_pTADS->GetpTechParams();
	ASSERT( NULL != m_pTechParam );

	m_dCellDouble = 0.0;
	m_strCellText = _T("");
	m_wLastKeyPressed = 0;
	m_wLastShiftCtrlStatus = 0;
	m_BoxDpC.bExt = false;

	// Terminal unit box.
	m_BoxTu.m_eQType = CTermUnit::_QType::Q;
	m_BoxTu.m_eDPType = CDS_HydroMod::eDpType::Dp;

	m_BoxCv.bExt = false;
	m_BoxCv.bKvEditMode = false;

	// BV primary, bypass and secondary boxes.
	memset( &m_BoxBvP, 0, sizeof( sBoxBv ) );
	memset( &m_BoxBvB, 0, sizeof( sBoxBv ) );
	memset( &m_BoxBvS, 0, sizeof( sBoxBv ) );

	// Circuit and distribution pipe boxes.
	memset( &m_BoxCircuitPrimaryPipe, 0, sizeof( sBoxPipe ) );
	memset( &m_BoxCircuitSecondaryPipe, 0, sizeof( sBoxPipe ) );
	memset( &m_BoxDistributionSupplyPipe, 0, sizeof( sBoxPipe ) );
	memset( &m_BoxDistributionReturnPipe, 0, sizeof( sBoxPipe ) );

	if( NULL != m_pHM->GetpCircuitPrimaryPipe() )
	{
		m_BoxCircuitPrimaryPipe.pPipe = m_pHM->GetpCircuitPrimaryPipe();
	}
	else
	{
		ASSERT( 0 );
	}

	if( NULL != m_pHM->GetpCircuitSecondaryPipe() )
	{
		m_BoxCircuitSecondaryPipe.pPipe = m_pHM->GetpCircuitSecondaryPipe();
	}

	if( NULL !=  m_pHM->GetpDistrSupplyPipe() )
	{
		m_BoxDistributionSupplyPipe.pPipe = m_pHM->GetpDistrSupplyPipe();
	}
	else
	{
		ASSERT( 0 );
	}

	if( NULL != m_pHM->GetpDistrReturnPipe() )
	{
		m_BoxDistributionReturnPipe.pPipe = m_pHM->GetpDistrReturnPipe();
	}

	// Take a copy of user preference.
	if( NULL == m_pPrjParamsSaved )
	{
		m_pPrjParamsSaved = new CPrjParams( m_pTADS->GetpProjectParams() );
	}

	m_pPrjParams = m_pTADS->GetpProjectParams()->GetpHmCalcParams();
	m_pPrjParams->Copy( m_pPrjParamsSaved );

	if( NULL == m_pHMSCB )
	{
		m_pHMSCB = new CDlgComboBoxHM( this );
	}

	if( NULL == m_pHMSelectActuator )
	{
		m_pHMSelectActuator = new CDlgSelectActuator( CTADatabase::FilterSelection::ForHMCalc, true, this );
	}

	ClearRange( -1, -1, -1, -1 );
	CSSheet::Init();
	SetBool( SSB_ALLOWDRAGDROP, FALSE );
	SetBool( SSB_PROCESSTAB, TRUE );
	SetCellNoteIndicator( SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT );
	SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	SetBool( SSB_EDITMODEPERMANENT, FALSE );
	SetButtonDrawMode( SS_BDM_ALWAYSCOMBO );
	SetMaxCols( m_Box.nCols );
	SetMaxRows( m_Box.nRows );

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Preformat row height, prevent flick when user extend a box.
	FormatStaticText( -1, -1, -1, -1, _T("") );

	// All cells are not selectable and by default will not receive mouse event.
	SetCellProperty( 1, 1, GetMaxCols(), GetMaxRows(), CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );

	// Set all cell as static cells.
	SS_CELLTYPE rCellType;
	SetTypeStaticText( &rCellType, SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES );
	SetCellType( -1, -1, &rCellType );

	// Decrease spread font when application run in a configuration where large font are used.
	// Prevent resizing of spread sheet.
	CDC ScreenDC;
	VERIFY( ScreenDC.CreateIC( _T("DISPLAY"), NULL, NULL, NULL ) );

	// Initialize Box position.
	m_BoxTu.ResetPos();
	m_BoxCircuitPrimaryPipe.ResetPos();
	m_BoxCircuitSecondaryPipe.ResetPos();
	m_BoxDistributionSupplyPipe.ResetPos();
	m_BoxDistributionReturnPipe.ResetPos();
	m_BoxPump.ResetPos();
	m_BoxSecondaryValues.ResetPos();
	m_BoxBvP.ResetPos();
	m_BoxBvB.ResetPos();
	m_BoxBvS.ResetPos();
	m_BoxCv.ResetPos();
	m_BoxPrimaryValues.ResetPos();
	m_BoxRem.ResetPos();
	m_BoxDpC.ResetPos();
	m_BoxShutoffValve.ResetPos();
	m_BoxDpCBCV.ResetPos();
	m_BoxSmartControlValve.ResetPos();
	m_BoxSmartDpC.ResetPos();
	m_BoxPic.pos = CPoint( m_Box.Pic_Left, m_Box.Pic_Top );

	m_mapColWidth[eBCol::Butt + 1] = PC2_COLWIDTH_FIXED;
	m_mapColWidth[eBCol::Lock + 1] = PC2_COLWIDTH_LOCK;
	m_mapColWidth[eBCol::Desc + 1] = PC2_COLWIDTH_DESCRIPTION;
	m_mapColWidth[eBCol::Val + 1] = PC2_COLWIDTH_VALUE;
	m_mapColWidth[eBCol::Unit + 1] = PC2_COLWIDTH_UNIT;

	// Fix width for these three columns.
	SetCellParam( 1, 1, PC2_COLUMPROPERTY_FIXED );
	SetCellParam( m_BoxPic.pos.x - 1, 1, PC2_COLUMPROPERTY_FIXED );
	SetCellParam( m_BoxPic.pos.x + m_Box.Pic_nCols, 1, PC2_COLUMPROPERTY_FIXED );

	_ChangeColumnSize();

	m_pHM->ResetSchemeIDPtr();
	m_bRefreshBoxPicture = true;
	FormatBoxPicture();

	m_bRefreshBoxPicture = false;
	m_bRefreshIncourse = true;

	// Primary values.
	m_BoxPrimaryValues.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebPrimaryValues );

	if( m_BoxPrimaryValues.pos != CPoint( 0, 0 ) )
	{
		FormatBoxPrimaryValues();
	}

	m_BoxPump.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebPump );

	if( m_BoxPump.pos != CPoint( 0, 0 ) )
	{
		FormatBoxPump();
		RefreshBoxPump( InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	m_BoxSecondaryValues.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebSecondaryValues );

	if( m_BoxSecondaryValues.pos != CPoint( 0, 0 ) )
	{
		FormatBoxSecondaryValues();
		RefreshBoxSecondaryValues( InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	m_BoxRem.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebRem );

	if( m_BoxRem.pos != CPoint( 0, 0 ) )
	{
		FormatBoxRemark();
		ChangeRemark( InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	m_BoxBvP.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebBVP );

	if( m_BoxBvP.pos != CPoint( 0, 0 ) )
	{
		FormatBoxBV( CDS_HydroMod::eBVprim );
	}

	m_BoxBvS.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebBVS );

	if( m_BoxBvS.pos != CPoint( 0, 0 ) )
	{
		FormatBoxBV( CDS_HydroMod::eBVsec );
	}

	m_BoxBvB.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebBVB );

	if( m_BoxBvB.pos != CPoint( 0, 0 ) )
	{
		FormatBoxBV( CDS_HydroMod::eBVbyp );
	}

	m_BoxCv.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebCV );

	if( ( m_BoxCv.pos != CPoint( 0, 0 ) ) && ( NULL != m_pHM->GetpCV() ) )
	{
		// Switch between FormatBoxCVKV or FormatBowCVTA based on Mode TA or Mode Kv from PrjParam
		if( false == m_pHM->GetpCV()->IsTaCV() )
		{
			FormatBoxCVKV();
		}
		else
		{
			FormatBoxCVTA();
		}
	}

	m_BoxDpC.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebDPC );

	if( m_BoxDpC.pos != CPoint( 0, 0 ) )
	{
		FormatBoxDpC();
	}

	m_BoxShutoffValve.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebShutoffValve );

	if( m_BoxShutoffValve.pos != CPoint( 0, 0 ) )
	{
		FormatBoxSV();
	}

	m_BoxDpCBCV.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebDpCBCValve );

	if( m_BoxDpCBCV.pos != CPoint( 0, 0 ) )
	{
		FormatBoxDpCBCV();
	}

	m_BoxSmartControlValve.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebSmartControlValve );

	if( m_BoxSmartControlValve.pos != CPoint( 0, 0 ) )
	{
		FormatBoxSmartControlValve();
	}

	m_BoxSmartDpC.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebSmartDpC );

	if( m_BoxSmartDpC.pos != CPoint( 0, 0 ) )
	{
		FormatBoxSmartDpC();
	}

	m_BoxCircuitPrimaryPipe.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebCircuitPrimaryPipe );

	// For circuit without terminal unit circuit pipe doesn't exist
	if( true == m_pHM->IsaModule() && NULL != m_pHM->GetpSchcat() && false == m_pHM->GetpSchcat()->IsSecondarySideExist() )
	{
		m_BoxCircuitPrimaryPipe.ResetPos();
	}

	// For circuit/module in the root position, circuit pipe doesn't exist
	if( m_pHM->GetLevel() == 0 )
	{
		m_BoxCircuitPrimaryPipe.ResetPos();
	}

	if( m_BoxCircuitPrimaryPipe.pos != CPoint( 0, 0 ) )
	{
		FormatBoxPipe( &m_BoxCircuitPrimaryPipe );
		ResetPipeSerie( &m_BoxCircuitPrimaryPipe );
		RefreshBoxPipe( &m_BoxCircuitPrimaryPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	m_BoxCircuitSecondaryPipe.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebCircuitSecondaryPipe );

	// For circuit without terminal unit circuit pipe doesn't exist
	if( true == m_pHM->IsaModule() )
	{
		m_BoxCircuitSecondaryPipe.ResetPos();
	}

	if( m_BoxCircuitSecondaryPipe.pos != CPoint( 0, 0 ) )
	{
		FormatBoxPipe( &m_BoxCircuitSecondaryPipe );
		ResetPipeSerie( &m_BoxCircuitSecondaryPipe );
		RefreshBoxPipe( &m_BoxCircuitSecondaryPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	m_BoxDistributionSupplyPipe.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebDistributionPipe );

	// For the root module no distribution pipe.
	if( 0 == m_pHM->GetLevel( ) )
	{
		m_BoxDistributionSupplyPipe.ResetPos();
	}

	if( m_BoxDistributionSupplyPipe.pos != CPoint( 0, 0 ) )
	{
		FormatBoxPipe( &m_BoxDistributionSupplyPipe );
		ResetPipeSerie( &m_BoxDistributionSupplyPipe );
		RefreshBoxPipe( &m_BoxDistributionSupplyPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	if( CDS_HydroMod::ReturnType::Reverse == m_pHM->GetReturnType() )
	{
		m_BoxDistributionReturnPipe.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebDistributionReturnPipe );

		// For the root module no distribution pipe.
		if( 0 == m_pHM->GetLevel() )
		{
			m_BoxDistributionReturnPipe.ResetPos();
		}

		// Pay attention: Reverse distribution pipe is defined by default in the 'CIRCSBX_TAB' but is current hydromod
		//                circuit is not in a reverse return mode, we don't need this box.
		if( m_BoxDistributionReturnPipe.pos != CPoint( 0, 0 ) )
		{
			FormatBoxPipe( &m_BoxDistributionReturnPipe );
			ResetPipeSerie( &m_BoxDistributionReturnPipe );
			RefreshBoxPipe( &m_BoxDistributionReturnPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
		}
	}

	if( false == m_pHM->GetpTermUnit()->IsVirtual() )
	{
		m_BoxTu.pos = pCircuitScheme->GetBox( CDB_CircuitScheme::ebTU );

		if( m_BoxTu.pos != CPoint( 0, 0 ) )
		{
			m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dDT = 0.0;
			// 		m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dTs = 0.0;
			// 		m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dTr = 0.0;
			m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dKv = 0.0;
			m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dCv = 0.0;
			m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dQRef = 0.0;
			m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dDpRef = 0.0;
			m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dDzeta = 0.0;
			m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dDiameter = 0.0;
			
			FormatBoxTU();
			RefreshBoxTU( InputValuesOrigin::InputValuesComeFromHydroMod );
		}
	}

	RefreshBoxPrimaryValues();
	m_bRefreshBoxPicture = true;
	FormatBoxPicture();

	// Is printing in course deploy boxes.
	if( true == fPrinting )
	{
		if( m_BoxBvP.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxBvP.pos.x + eBCol::Butt, m_BoxBvP.pos.y + m_BoxBvP.BV_Name );
		}

		if( m_BoxBvS.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxBvS.pos.x + eBCol::Butt, m_BoxBvS.pos.y + m_BoxBvS.BV_Name );
		}

		if( m_BoxBvB.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxBvB.pos.x + eBCol::Butt, m_BoxBvB.pos.y + m_BoxBvB.BV_Name );
		}

		if( m_BoxDpC.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxDpC.pos.x + eBCol::Butt, m_BoxDpC.pos.y + m_Box.DpC_Name );
		}

		if( m_BoxCv.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxCv.pos.x + eBCol::Butt, m_BoxCv.pos.y + m_Box.CVTA_Name );
		}

		if( m_BoxShutoffValve.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name );
		}

		if( m_BoxDpCBCV.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxDpCBCV.pos.x + eBCol::Butt, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name );
		}

		if( m_BoxSmartControlValve.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxSmartControlValve.pos.x + eBCol::Butt, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName );
		}

		if( m_BoxSmartDpC.pos != CPoint( 0, 0 ) )
		{
			CellClicked( m_BoxSmartDpC.pos.x + eBCol::Butt, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName );
		}
	}

	// Sheet is locked when the project is frozen.
	if( true == m_pHM->GetpPrjParam()->IsFreezed() )
	{
		SSSetLockRange( this->GetSafeHwnd(), 0, 0, GetMaxCols(), GetMaxRows(), true );
		SSSetBool( this->GetSafeHwnd(), SSB_PROTECT, TRUE );
	}

	SetActiveCell( 0, 0 );
	m_bRedrawAllowed = true;
	m_bRefreshIncourse = false;

	// Register notification to receive messages from CSSheet.
	CMessageManager::MM_RegisterNotificationHandler( this,	CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDown );

	ArrowKeyDown( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Des );
	m_bReady = true;

	SetBool( SSB_REDRAW, TRUE );
	Invalidate();
	UpdateWindow();
}

void CSSheetPanelCirc2::RefreshDispatch( _BoxType FromWho )
{
	if( true == m_bRefreshIncourse )
	{
		return;
	}

	m_bRefreshIncourse = true;
	TASApp.OverridedSetRedraw( this, FALSE );

	if( NULL != m_pHM->GetpCircuitPrimaryPipe() && FromWho != _BoxType::ebCircuitPrimaryPipe )
	{
		RefreshBoxPipe( &m_BoxCircuitPrimaryPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	if( NULL != m_pHM->GetpCircuitSecondaryPipe() && FromWho != _BoxType::ebCircuitSecondaryPipe )
	{
		RefreshBoxPipe( &m_BoxCircuitSecondaryPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	if( NULL != m_pHM->GetpDistrSupplyPipe() && FromWho != _BoxType::ebDistributionPipe )
	{
		RefreshBoxPipe( &m_BoxDistributionSupplyPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	if( NULL != m_pHM->GetpDistrReturnPipe() && FromWho != _BoxType::ebDistributionReturnPipe )
	{
		RefreshBoxPipe( &m_BoxDistributionReturnPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	if( true == m_pHM->IsBvBypExist( false ) && FromWho != _BoxType::ebBVB )
	{
		FormatBoxBV( CDS_HydroMod::eBVbyp );
	}

	if( true == m_pHM->IsBvExist( false ) && FromWho != _BoxType::ebBVP )
	{
		ChangeBvType( m_pHM->GetpBv() );
	}

	if( true == m_pHM->IsBvSecExist( false ) && FromWho != _BoxType::ebBVS )
	{
		ChangeBvType( m_pHM->GetpSecBv() );
	}

	if( true == m_pHM->IsDpCExist( false ) && FromWho != _BoxType::ebDPC )
	{
		FormatBoxDpC();
	}

	// Third condition: Combined Dp controller, control and balancing valve are managed just after.
	if( true == m_pHM->IsCvExist( false ) && FromWho != _BoxType::ebCV && eb3False == m_pHM->GetpCV()->IsDpCBCV() )
	{
		RefreshBoxCV( InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	if( true == m_pHM->IsCvExist( false ) && FromWho != _BoxType::ebDpCBCValve && eb3True == m_pHM->GetpCV()->IsDpCBCV() )
	{
		FormatBoxDpCBCV();
	}

	if( true == m_pHM->IsShutoffValveExist( false ) && FromWho != _BoxType::ebShutoffValve )
	{
		FormatBoxSV();
	}

	if( true == m_pHM->IsSmartControlValveExist( false ) && FromWho != _BoxType::ebSmartControlValve )
	{
		FormatBoxSmartControlValve();
	}

	if( true == m_pHM->IsSmartDpCExist( false ) && FromWho != _BoxType::ebSmartDpC )
	{
		FormatBoxSmartDpC();
	}

	if( true == m_pHM->IsPumpExist() )
	{
		// Hmin can be changed due to computation of injection circuit.
		RefreshBoxPump( ( FromWho == _BoxType::ebPump ) ? InputValuesOrigin::InputValuesComeFromHydroMod : InputValuesOrigin::InputValuesComeFromUser );
	}

	// HYS-1882: When we modify supply secondary temp. the computed return is known after the computeAll we should refresh to display it.
	if( true == m_pHM->IsInjectionCircuit() )
	{
		RefreshBoxSecondaryValues( InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	// HYS-1882: Refresh terminal unit to update flow when we are in PDT mode and temperature regime change for example from insert in.
	if( FromWho != _BoxType::ebTU )
	{
		RefreshBoxTU( InputValuesOrigin::InputValuesComeFromHydroMod );
	}

	RefreshBoxPrimaryValues();

	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
	m_bRefreshIncourse = false;
}

void CSSheetPanelCirc2::SaveAndCompute()
{
	// Current Cell is m_Row, m_Col.
	// Current Text is m_CellText.
	// Current double is m_CellDouble.
	_BoxType CurrentBox = WhatBox( m_lCol, m_lRow );

	switch( CurrentBox )
	{
		case _BoxType::ebTU:
			RefreshBoxTU( InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebCV:
			RefreshBoxCV( InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebBVB:
		case _BoxType::ebBVP:
		case _BoxType::ebDPC:
		case _BoxType::ebPrimaryValues:
		case _BoxType::ebShutoffValve:
		case _BoxType::ebDpCBCValve:
		case _BoxType::ebSmartControlValve:
		case _BoxType::ebSmartDpC:
			break;

		case _BoxType::ebCircuitPrimaryPipe:
			RefreshBoxPipe( &m_BoxCircuitPrimaryPipe, InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebCircuitSecondaryPipe:
			RefreshBoxPipe( &m_BoxCircuitSecondaryPipe, InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebDistributionPipe:
			RefreshBoxPipe( &m_BoxDistributionSupplyPipe, InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebDistributionReturnPipe:
			RefreshBoxPipe( &m_BoxDistributionReturnPipe, InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebRem:
			ChangeRemark( InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebPump:
			RefreshBoxPump( InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebSecondaryValues:
			RefreshBoxSecondaryValues( InputValuesOrigin::InputValuesComeFromUser );
			RefreshBoxTU( InputValuesOrigin::InputValuesComeFromUser );
			break;

		case _BoxType::ebNoBox:
			break;

		default:
			break;
	}

	if( CurrentBox != _BoxType::ebNoBox )
	{
		m_pHM->ComputeHM( CDS_HydroMod::eceResize );
		m_pHM->ComputeAll();
		RefreshDispatch( CurrentBox );
	}
}

void CSSheetPanelCirc2::PrintThisSheet()
{
	SS_PRINTFORMAT ssPrintFormat;
	GetPrintOptions( &ssPrintFormat, NULL, 0 );
	ssPrintFormat.fDrawBorder = FALSE;
	ssPrintFormat.fShowColHeaders = FALSE;
	ssPrintFormat.fDrawColors = TRUE;
	ssPrintFormat.fShowGrid = FALSE;
	ssPrintFormat.fMarginTop = 1440;
	ssPrintFormat.fMarginBottom = 1440;
	ssPrintFormat.fMarginLeft = 720;
	ssPrintFormat.fMarginRight = 720;
	ssPrintFormat.dPrintType = 0;					// Print all the page
	ssPrintFormat.fShowRowHeaders = FALSE;
	ssPrintFormat.fDrawShadows = FALSE;
	ssPrintFormat.fUseDataMax = TRUE;
	ssPrintFormat.fCenterOnPageH = TRUE;
	ssPrintFormat.fCenterOnPageV = TRUE;
	ssPrintFormat.nBestFitPagesTall = 1;
	ssPrintFormat.nBestFitPagesWide = 1;
	ssPrintFormat.wScalingMethod = SS_PRINTSCALINGMETHOD_BESTFIT;
	ssPrintFormat.wOrientation = SS_PRINTORIENT_PORTRAIT;
	SetSSPrintFormat( ssPrintFormat );

	TSpread::Print( _T(""), &ssPrintFormat, NULL, 0, NULL );
}

void CSSheetPanelCirc2::CloseDialogSCB( CDialogEx *pDlgSCB, bool bModified )
{
	CString str;
	BeginWaitCursor();

	if( pDlgSCB == m_pHMSCB )
	{
		// Compute all.
		if( true == bModified )
		{
			m_pHM->ComputeAll();
		}

		switch( m_pHMSCB->GetSCBType() )
		{
			case CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSerie:
				
				if( true == bModified )
				{
					ResetPipeSerie( &m_BoxCircuitPrimaryPipe );
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_CircuitSecondaryPipeSerie:
				
				if( true == bModified )
				{
					ResetPipeSerie( &m_BoxCircuitSecondaryPipe );
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_DistributionSupplyPipeSerie:
				
				if( true == bModified )
				{
					ResetPipeSerie( &m_BoxDistributionSupplyPipe );
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_DistributionReturnPipeSerie:
				
				if( true == bModified )
				{
					ResetPipeSerie( &m_BoxDistributionReturnPipe );
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSize:
			case CDlgComboBoxHM::eSCB::SCB_CircuitSecondaryPipeSize:
			case CDlgComboBoxHM::eSCB::SCB_CircuitBypassPipeSize:
			case CDlgComboBoxHM::eSCB::SCB_DistributionSupplyPipeSize:
			case CDlgComboBoxHM::eSCB::SCB_DistributionReturnPipeSize:
				
				if( true == bModified )
				{
					m_pHM->ComputeHM( CDS_HydroMod::eceResize );
					m_pHM->ComputeAll();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_BvLoc:
				{
					CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV * )m_pHMSCB->GetLParam();
					ASSERT( pBv );

					if( true == bModified )
					{
						m_pHM->ComputeHM( CDS_HydroMod::eceResize );
						m_pHM->ComputeAll();
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_BvPName:
			case CDlgComboBoxHM::eSCB::SCB_BvSName:
			case CDlgComboBoxHM::eSCB::SCB_BvBName:
				{
					CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV * )m_pHMSCB->GetLParam();
					ASSERT( pBv );

					if( true == bModified )
					{
						ChangeBvType( pBv );		// Change all in cascade
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_BvPType:
			case CDlgComboBoxHM::eSCB::SCB_BvSType:
			case CDlgComboBoxHM::eSCB::SCB_BvBType:
				{
					CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV * )m_pHMSCB->GetLParam();
					ASSERT( pBv );

					if( NULL == pBv )
					{
						break;
					}

					if( true == bModified )
					{
						ChangeBvType( pBv );
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_BvPConnection:
			case CDlgComboBoxHM::eSCB::SCB_BvSConnection:
			case CDlgComboBoxHM::eSCB::SCB_BvBConnection:
				{
					CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV * )m_pHMSCB->GetLParam();
					ASSERT( pBv );

					if( NULL == pBv )
					{
						break;
					}

					if( true == bModified )
					{
						ChangeBvConnect( pBv );
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_BvPVersion:
			case CDlgComboBoxHM::eSCB::SCB_BvSVersion:
			case CDlgComboBoxHM::eSCB::SCB_BvBVersion:
				{
					CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV * )m_pHMSCB->GetLParam();
					ASSERT( pBv );

					if( NULL == pBv )
					{
						break;
					}

					if( true == bModified )
					{
						ChangeBvVersion( pBv );
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_BvPPN:
			case CDlgComboBoxHM::eSCB::SCB_BvSPN:
			case CDlgComboBoxHM::eSCB::SCB_BvBPN:
				{
					CDS_HydroMod::CBV *pBv = ( CDS_HydroMod::CBV * )m_pHMSCB->GetLParam();
					ASSERT( pBv );

					if( NULL == pBv )
					{
						break;
					}

					if( true == bModified )
					{
						ChangeBvPN( pBv );
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_CvKvs:

				if( true == bModified )
				{
					m_pHM->ComputeHM( CDS_HydroMod::eceResize );
					m_pHM->ComputeAll();
					RefreshBoxCV( InputValuesOrigin::InputValuesComeFromHydroMod );
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_CvLoc:

				if( true == bModified )
				{
					m_pHM->ComputeHM( CDS_HydroMod::eceResize );
					m_pHM->ComputeAll();
					_FillCVLoc();

					// We need to refresh the box to have the right background color for title.
					if( false == m_pHM->GetpCV()->IsTaCV() )
					{
						FormatBoxCVKV();
					}
					else
					{
						FormatBoxCVTA();
					}
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_Cv:
				
				if( true == bModified )
				{
					if( NULL != m_pHM->GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing == m_pHM->GetpSchcat()->Get3WType() )
					{
						// BV-Sec must be recalculated to compensate the overflow
						m_pHM->ComputeHM( CDS_HydroMod::eceResize );
						m_pHM->ComputeAll();
					}
					RefreshBoxCV( InputValuesOrigin::InputValuesComeFromHydroMod );
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_CvConnection:

				if( m_pHM->GetpCV()->GetpSelCVConn() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeCvConnect();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_CvVersion:

				if( m_pHM->GetpCV()->GetpSelCVVers() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeCvVersion();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_CvPN:

				if( m_pHM->GetpCV()->GetpSelCVPN() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeCvPN();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_DpC:
				
				if( true == bModified )
				{
					ChangeDpC();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_DpCType:
				{
					CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
					ASSERT( pDpC );

					if( pDpC->GetpSelDpCType() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )						// nothing change
					{
						break;
					}

					if( true == bModified )
					{
						ChangeDpCType();
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_DpCConnection:
				{
					CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
					ASSERT( pDpC );

					if( pDpC->GetpSelDpCConn() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )						// nothing change
					{
						break;
					}

					if( true == bModified )
					{
						ChangeDpCConnect();
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_DpCPN:
				{
					CDS_HydroMod::CDpC *pDpC = m_pHM->GetpDpC();
					ASSERT( pDpC );

					if( pDpC->GetpSelDpCPN() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )							// nothing change
					{
						break;
					}

					if( true == bModified )
					{
						ChangeDpCPN();
					}
				}
				break;

			case CDlgComboBoxHM::eSCB::SCB_DpCBCValve:
				
				if( true == bModified )
				{
					FormatBoxDpCBCV();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_DpCBCValveConnection:

				if( m_pHM->GetpCV()->GetpSelCVConn() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeDpCBCVConnect();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_DpCBCValveVersion:

				if( m_pHM->GetpCV()->GetpSelCVVers() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeDpCBCVVersion();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_DpCBCValvePN:

				if( m_pHM->GetpCV()->GetpSelCVPN() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeDpCBCVPN();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_ShutOffValveType:
				{
					if( NULL == m_pHM->GetpSch() )
					{
						break;
					}

					CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;
			
					if( NULL != m_pHM->GetpSch() )
					{
						if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveSupply;
						}
						else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveReturn;
						}
					}

					if( CDS_HydroMod::eNone == eHMObj )
					{
						break;
					}

					CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
					ASSERT( NULL != pclHMShutoffValve );

					if( pclHMShutoffValve->GetpSelType() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )						// nothing change
					{
						break;
					}

					if( true == bModified )
					{
						ChangeSVType();
					}
				}
				
				break;

			case CDlgComboBoxHM::eSCB::SCB_ShutOffValveConnection:
				{
					if( NULL == m_pHM->GetpSch() )
					{
						break;
					}

					CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;
			
					if( NULL != m_pHM->GetpSch() )
					{
						if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveSupply;
						}
						else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveReturn;
						}
					}

					if( CDS_HydroMod::eNone == eHMObj )
					{
						break;
					}

					CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
					ASSERT( NULL != pclHMShutoffValve );

					if( pclHMShutoffValve->GetpSelConnection() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )						// nothing change
					{
						break;
					}

					if( true == bModified )
					{
						ChangeSVConnect();
					}
				}
				
				break;

			case CDlgComboBoxHM::eSCB::SCB_ShutOffValveVersion:
				{
					if( NULL == m_pHM->GetpSch() )
					{
						break;
					}

					CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;
			
					if( NULL != m_pHM->GetpSch() )
					{
						if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveSupply;
						}
						else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveReturn;
						}
					}

					if( CDS_HydroMod::eNone == eHMObj )
					{
						break;
					}

					CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
					ASSERT( NULL != pclHMShutoffValve );

					if( pclHMShutoffValve->GetpSelVersion() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )						// nothing change
					{
						break;
					}

					if( true == bModified )
					{
						ChangeSVVersion();
					}
				}
				
				break;

			case CDlgComboBoxHM::eSCB::SCB_ShutOffValvePN:
				{
					if( NULL == m_pHM->GetpSch() )
					{
						break;
					}

					CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;
			
					if( NULL != m_pHM->GetpSch() )
					{
						if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveSupply;
						}
						else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
						{
							eHMObj = CDS_HydroMod::eShutoffValveReturn;
						}
					}

					if( CDS_HydroMod::eNone == eHMObj )
					{
						break;
					}

					CDS_HydroMod::CShutoffValve *pclHMShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
					ASSERT( NULL != pclHMShutoffValve );

					if( pclHMShutoffValve->GetpSelPN() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )						// nothing change
					{
						break;
					}

					if( true == bModified )
					{
						ChangeSVPN();
					}
				}
				
				break;

			case CDlgComboBoxHM::eSCB::SCB_SmartControlValve:
				
				if( true == bModified )
				{
					FormatBoxSmartControlValve();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_SmartControlValveBody:

				if( m_pHM->GetpSmartControlValve()->GetpSelBodyMaterial() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeSmartControlValveBodyMaterial();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_SmartControlValveConnection:

				if( m_pHM->GetpSmartControlValve()->GetpSelConnection() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeSmartControlValveConnect();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_SmartControlValvePN:

				if( m_pHM->GetpSmartControlValve()->GetpSelPN() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeSmartControlValvePN();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_SmartDpC:
				
				if( true == bModified )
				{
					FormatBoxSmartDpC();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_SmartDpCBody:

				if( m_pHM->GetpSmartDpC()->GetpSelBodyMaterial() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeSmartDpCBodyMaterial();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_SmartDpCConnection:

				if( m_pHM->GetpSmartDpC()->GetpSelConnection() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeSmartDpCConnect();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_SmartDpCPN:

				if( m_pHM->GetpSmartDpC()->GetpSelPN() == ( (CDB_StringID *)m_pHMSCB->GetLParam() ) )				// nothing change
				{
					break;
				}

				if( true == bModified )
				{
					ChangeSmartDpCPN();
				}

				break;

			case CDlgComboBoxHM::eSCB::SCB_TU_Flow:
				
				// HYS-1882: use case: We are in PDT mode with a DT1 different than the regime DT. The flow is Q1.
				// Then switch to flow mode: The flow stays Q1 and DT is DT1 (not displayed). But Trp and Tsp are those of the regime.
				// The real DT is DT2 the regime DT but the flow is not updated. Flow should be change because the DT change is not from user.
				// So we update the DT and Flow (TU, Primary, and valves).
				if( true == bModified )
				{
					FormatBoxTU( true );
				}

			case CDlgComboBoxHM::eSCB::SCB_TU_Dp:
				
				if( true == bModified )
				{
					FormatBoxTU( true );
				}

				break;
		}
	}
	else if( pDlgSCB != m_pHMSelectActuator )
	{
		// HYS-1443 : Don't need to call ComputeAll when the actuator change with DlgSelectActuator
		m_pHM->ComputeAll();
		RefreshDispatch();
	}

	if( true == bModified )
	{
		RefreshDispatch( );
	}

	Invalidate();
	EndWaitCursor();
}

#ifdef _DEBUG
bool CSSheetPanelCirc2::TestHydronicSchemesBoxPosition( IDPTR &idptrFail )
{
	CTable *pTab = dynamic_cast<CTable *>( TASApp.GetpTADB()->Get( _T("CIRCSBX_TAB") ).MP );

	if( NULL == pTab )
	{
		return false;
	}

	InitDefaultBox( 2 ); // Worst case

	// nRow = 38 -> y: 0-37
	std::vector< std::vector<bool> > ar( m_Box.nCols, std::vector<bool>( m_Box.nRows ) );
	m_iTestHydronicSchemesBoxPositionStatus = Status_OK;
	m_iTestHydronicSchemesBoxPositionCols = m_Box.nCols;
	m_iTestHydronicSchemesBoxPositionRows = m_Box.nRows;

	for( IDPTR idptr = pTab->GetFirst(); NULL != idptr.MP; idptr = pTab->GetNext( idptr.MP ) )
	{
		for( int x = 0; x < m_Box.nCols; x++ )
		{
			for( int y = 0; y < m_Box.nRows; y++ )
			{
				ar[x][y] = false;
			}
		}

		idptrFail = idptr;
		CDB_MultiPair *pMP = dynamic_cast<CDB_MultiPair *>( idptr.MP );
		CPoint	 pt;

		for( int i = 0; i < pMP->GetSize(); i++ )
		{
			pMP->GetPoint( i, pt );

			if( pt.x == 0 && pt.y == 0 )
			{
				continue;
			}

			int x = pt.x;
			int y = pt.y;
			int ymax = y;

			switch( i )
			{
				// Hp: Primary values
				case Box_PrimaryValues:
					ymax += ( m_Box.PrimaryValues_nRows - 1 );
					break;

				// Pipe-P: Circuit pipe at primary side.
				case Box_PrimaryPipe:	
					ymax += ( m_Box.Pipe_nRows - 1 );
					break;

				// Pipe-D: Distribution pipe.
				case Box_DistributionPipe:
					ymax += ( m_Box.Pipe_nRows - 1 );
					break;

				// TU: Terminal unit.
				case Box_TerminalUnit:
					ymax += ( m_Box.TU_nRows -1 );
					break;

				// CV: Control valve.
				case Box_ControlValve:
					ymax += ( m_Box.CVTA_nRows -1 );
					break;

				// Smart control valve.
				case Box_SmartControlValve:
					ymax += ( SMARTCONTROLVALVENROWS - 1 );
					break;

				// BV-P: Balancing valve on primary.
				case Box_BalancingValveOnPrimary:
					ymax += ( m_BoxBvP.BV_nRows -1 );
					break;

				// BV-B: Balancing valve on bypass.
				case Box_BalancingValveOnBypass:
					ymax += ( m_BoxBvB.BV_nRows -1 );
					break;

				// DPC: Differential pressure controller.
				case Box_DifferentialPressureController:
					ymax += ( m_Box.DpC_nRows - 1 );
					break;

				// Remark box.
				case Box_Remark:
					ymax += ( m_Box.Rem_nRows - 1 );
					break;

				// Pump.
				case Box_Pump:
					ymax += ( PUMPNROWS - 1 );
					break;

				// BV-S: Balancing valve on secondary.
				case Box_BalancingValveOnSecondary:
					ymax += ( m_BoxBvS.BV_nRows - 1 );
					break;

				// Pipe-S: Circuit pipe at secondary side.
				case Box_SecondaryPipe:
					ymax += ( m_Box.Pipe_nRows -1 );
					break;

				// Secondary values.
				case Box_SecondaryValues:
					ymax += ( SECONDARYVALUESNROWS - 1 );
					break;

				// Pipe-DR: Reverse pipe.
				case Box_ReversePipe:
					ymax += ( m_Box.Pipe_nRows - 1 );
					break;

				// Shutoff valves.
				case Box_ShutoffValve:
					ymax += ( SHUTOFFVALVENROWS - 1 );
					break;

				// Combined Dp controller, control and balancing valves.
				case Box_CombinedDpCControlAndBalancingValve:
					ymax += ( DPCBCVNROWS - 1 );
					break;

				// Smart differential pressure controllers.
				case Box_SmartDifferentialPressureController:
					ymax += ( SMARTDPCNROWS - 1 );
					break;

				default:
					break;
			}

			// Coordinates in the circuit shematic boxes are 1-indexed.
			ymax--;

			for( int j = y; j < ymax; j++ )
			{
				if( x > m_Box.nCols || j > m_Box.nRows )
				{
					m_iTestHydronicSchemesBoxPositionErrorBox = i;

					if( x > m_Box.nCols )
					{
						m_iTestHydronicSchemesBoxPositionStatus = Status_XOutOfRange;
						m_iTestHydronicSchemesBoxPositionErrorXValue = x;
					}
					else
					{
						m_iTestHydronicSchemesBoxPositionStatus = Status_YOutOfRange;
						m_iTestHydronicSchemesBoxPositionErrorYValue = y;
					}

					return false;
				}

				if( ar[x][j] != false )
				{
					m_iTestHydronicSchemesBoxPositionStatus = Status_PositionBusy;
					m_iTestHydronicSchemesBoxPositionErrorBox = i;
					m_iTestHydronicSchemesBoxPositionErrorXValue = x;
					m_iTestHydronicSchemesBoxPositionErrorYValue = y;

					return false;
				}

				ar[x][j] = true;
			}
		}
	}

	return true;
}
#endif

BEGIN_MESSAGE_MAP( CSSheetPanelCirc2, CSSheet )
	// Spread DLL messages
	ON_MESSAGE( SSM_COMBODROPDOWN, ComboDropDownFpspread )
	ON_MESSAGE( SSM_COMBOSELCHANGE, ComboSelChangeFpspread )
	ON_MESSAGE( SSM_EDITCHANGE, EditChangeFpspread )
	ON_MESSAGE( SSM_EDITMODEON, EditModeOnFpspread )
	ON_MESSAGE( SSM_EDITMODEOFF, EditModeOffFpspread )
	ON_MESSAGE( SSM_KEYDOWN, KeyDownFpspread )
	ON_MESSAGE( SSM_TEXTTIPFETCH, TextTipFetchFpspread )
	ON_MESSAGE( WM_USER_MESSAGEDATA, MM_OnMessageReceive )
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CSSheetPanelCirc2::OnPaint()
{
	CSSheet::OnPaint();
	_VerifyComboBoxError();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Spread DLL message handlers.
/////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CSSheetPanelCirc2::ComboDropDownFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	ComboDropDown( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CSSheetPanelCirc2::ComboSelChangeFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	ComboSelChange( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CSSheetPanelCirc2::EditChangeFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	EditChange( pCellCoord->Col, pCellCoord->Row );
	return 0;
}

LRESULT CSSheetPanelCirc2::EditModeOnFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pCellCoord = (SS_CELLCOORD *)lParam;
	m_lCol = pCellCoord->Col;
	m_lRow = pCellCoord->Row;
	return 0;
}

LRESULT CSSheetPanelCirc2::EditModeOffFpspread( WPARAM wParam, LPARAM lParam )
{
	if( false == m_bManageEditOffSpreadNotification )
	{
		return 0;
	}

	SS_EDITMODEOFF *pEditModeOff = (SS_EDITMODEOFF *)lParam;

	if( TRUE == pEditModeOff->fChangeMade )
	{
		SaveAndCompute();
	}

	_BoxType NewBox = WhatBox( m_lCol, m_lRow );
	
	if( true == m_BoxCv.bKvEditMode )
	{
		m_BoxCv.bKvEditMode = false;
		FormatBoxCVKV();
		Invalidate();
	}

	// Check if we need to go to one other edit field.
	switch( m_wLastKeyPressed )
	{
		case VK_TAB:
			if( 0x01 == ( m_wLastShiftCtrlStatus & 0x01 ) )
			{
				ArrowKeyUp( m_lCol, m_lRow );
			}
			else
			{
				ArrowKeyDown( m_lCol, m_lRow );
			}

			break;

		case VK_RETURN:
		case VK_DOWN:
			ArrowKeyDown( m_lCol, m_lRow );
			break;

		case VK_UP:
			ArrowKeyUp( m_lCol, m_lRow );
			break;
	}

	m_wLastKeyPressed = 0;

	return 0;
}

LRESULT CSSheetPanelCirc2::KeyDownFpspread( WPARAM wParam, LPARAM lParam )
{
	m_wLastKeyPressed = LOWORD( lParam );
	m_wLastShiftCtrlStatus = HIWORD( lParam );
	return 0;
}

LRESULT CSSheetPanelCirc2::TextTipFetchFpspread( WPARAM wParam, LPARAM lParam )
{
	SS_TEXTTIPFETCH *pTextTipFetch = ( SS_TEXTTIPFETCH * )lParam;
	// If hText is not empty, spread takes hText in place of szText!
	pTextTipFetch->hText = NULL;
	TextTipFetch( pTextTipFetch->Col, pTextTipFetch->Row, &pTextTipFetch->wMultiLine, &pTextTipFetch->nWidth, pTextTipFetch->szText, &pTextTipFetch->fShow );
	return 0;
}

LRESULT CSSheetPanelCirc2::MM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
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
				case CMessageManager::SSheetMET::MET_LButtonDown:
					CellClicked( prParams->m_lColumn, prParams->m_lRow );
					break;
			}
		}
		break;
	}

	delete pclMessage;
	return 0;
}

void CSSheetPanelCirc2::SetRedraw( bool fRedraw )
{
	if( true == fRedraw && true == m_bRedrawAllowed )
	{
		TASApp.OverridedSetRedraw( this, TRUE );
	}
	else
	{
		TASApp.OverridedSetRedraw( this, FALSE );
	}
}

_BoxType CSSheetPanelCirc2::WhatBox( long lColumn, long lRow )
{
	// HYS-1713: For each box verify if it exists in the current SheetPanelCirc2
	if( CPoint( 0, 0 ) != m_BoxTu.pos && lColumn >= m_BoxTu.pos.x + eBCol::Lock && lColumn <= ( m_BoxTu.pos.x + eBCol::Unit ) 
		&& lRow >= m_BoxTu.pos.y && lRow < ( m_BoxTu.pos.y + m_Box.TU_nRows ) )
	{
		return _BoxType::ebTU;
	}

	if( CPoint( 0, 0 ) != m_BoxCircuitPrimaryPipe.pos && lColumn >= m_BoxCircuitPrimaryPipe.pos.x + eBCol::Lock && lColumn <= ( m_BoxCircuitPrimaryPipe.pos.x + eBCol::Unit ) && lRow >= m_BoxCircuitPrimaryPipe.pos.y
			&& lRow < ( m_BoxCircuitPrimaryPipe.pos.y + m_Box.Pipe_nRows ) )
	{
		return _BoxType::ebCircuitPrimaryPipe;
	}

	if( CPoint( 0, 0 ) != m_BoxCircuitSecondaryPipe.pos && lColumn >= m_BoxCircuitSecondaryPipe.pos.x + eBCol::Lock && lColumn <= ( m_BoxCircuitSecondaryPipe.pos.x + eBCol::Unit ) && lRow >= m_BoxCircuitSecondaryPipe.pos.y
			&& lRow < ( m_BoxCircuitSecondaryPipe.pos.y + m_Box.Pipe_nRows ) )
	{
		return _BoxType::ebCircuitSecondaryPipe;
	}

	if( CPoint( 0, 0 ) != m_BoxDistributionSupplyPipe.pos && lColumn >= m_BoxDistributionSupplyPipe.pos.x + eBCol::Lock && lColumn <= ( m_BoxDistributionSupplyPipe.pos.x + eBCol::Unit ) 
		&& lRow >= m_BoxDistributionSupplyPipe.pos.y && lRow < ( m_BoxDistributionSupplyPipe.pos.y + m_Box.Pipe_nRows ) )
	{
		return _BoxType::ebDistributionPipe;
	}

	if( CPoint( 0, 0 ) != m_BoxDistributionReturnPipe.pos && lColumn >= m_BoxDistributionReturnPipe.pos.x + eBCol::Lock && lColumn <= ( m_BoxDistributionReturnPipe.pos.x + eBCol::Unit ) 
		&& lRow >= m_BoxDistributionReturnPipe.pos.y && lRow < ( m_BoxDistributionReturnPipe.pos.y + m_Box.Pipe_nRows ) )
	{
		return _BoxType::ebDistributionReturnPipe;
	}

	if( CPoint( 0, 0 ) != m_BoxBvP.pos && lColumn >= m_BoxBvP.pos.x + eBCol::Lock && lColumn <= ( m_BoxBvP.pos.x + eBCol::Unit ) && lRow >= m_BoxBvP.pos.y
			&& lRow < ( m_BoxBvP.pos.y + m_BoxBvP.BV_nRows ) )
	{
		return _BoxType::ebBVP;
	}

	if( CPoint( 0, 0 ) != m_BoxBvS.pos && lColumn >= m_BoxBvS.pos.x + eBCol::Lock && lColumn <= ( m_BoxBvS.pos.x + eBCol::Unit ) && lRow >= m_BoxBvS.pos.y
			&& lRow < ( m_BoxBvS.pos.y + m_BoxBvS.BV_nRows ) )
	{
		return _BoxType::ebBVS;
	}

	if( CPoint( 0, 0 ) != m_BoxPrimaryValues.pos && lColumn >= m_BoxPrimaryValues.pos.x + eBCol::Lock && lColumn < ( m_BoxPrimaryValues.pos.x + 3 ) 
		&& lRow >= m_BoxPrimaryValues.pos.y && lRow < ( m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_nRows ) )
	{
		return _BoxType::ebPrimaryValues;
	}

	if( CPoint( 0, 0 ) != m_BoxBvB.pos && lColumn >= m_BoxBvB.pos.x + eBCol::Lock && lColumn <= ( m_BoxBvB.pos.x + eBCol::Unit ) && lRow >= m_BoxBvB.pos.y
			&& lRow < ( m_BoxBvB.pos.y + m_BoxBvB.BV_nRows ) )
	{
		return _BoxType::ebBVB;
	}

	// For CV in Kv mode.
	if( CPoint( 0, 0 ) != m_BoxCv.pos && lColumn >= m_BoxCv.pos.x + eBCol::Lock && lColumn <= ( m_BoxCv.pos.x + eBCol::Unit ) && lRow >= m_BoxCv.pos.y
			&& lRow < ( m_BoxCv.pos.y + m_Box.CVKV_nRows ) )
	{
		return _BoxType::ebCV;
	}

	// For CV in TA mode.
	if( CPoint( 0, 0 ) != m_BoxCv.pos && lColumn >= m_BoxCv.pos.x + eBCol::Lock && lColumn <= ( m_BoxCv.pos.x + eBCol::Unit ) && lRow >= m_BoxCv.pos.y
			&& lRow < ( m_BoxCv.pos.y + m_BoxCv.rActuator + 1 ) )
	{
		return _BoxType::ebCV;
	}

	if( CPoint( 0, 0 ) != m_BoxRem.pos && lColumn >= m_BoxRem.pos.x + eBCol::Lock && lColumn <= ( m_BoxRem.pos.x + eBCol::Lock + 3 )
			&& lRow >= m_BoxRem.pos.y + m_Box.Rem_l1 && lRow <= m_BoxRem.pos.y + m_Box.Rem_l1 + 1 )
	{
		return _BoxType::ebRem;
	}

	if( CPoint( 0, 0 ) != m_BoxDpC.pos && lColumn >= m_BoxDpC.pos.x + eBCol::Lock && lColumn <= ( m_BoxDpC.pos.x + eBCol::Unit ) && lRow >= m_BoxDpC.pos.y
			&& lRow < ( m_BoxDpC.pos.y + m_Box.DpC_nRows ) )
	{
		return _BoxType::ebDPC;
	}

	if( CPoint( 0, 0 ) != m_BoxPump.pos && lColumn >= m_BoxPump.pos.x + eBCol::Lock && lColumn <= ( m_BoxPump.pos.x + eBCol::Unit ) && lRow >= m_BoxPump.pos.y
			&& lRow < ( m_BoxPump.pos.y + m_BoxPump.nRows ) )
	{
		return _BoxType::ebPump;
	}

	if( CPoint( 0, 0 ) != m_BoxSecondaryValues.pos && lColumn >= m_BoxSecondaryValues.pos.x + eBCol::Lock && lColumn <= ( m_BoxSecondaryValues.pos.x + eBCol::Unit ) 
		&& lRow >= m_BoxSecondaryValues.pos.y && lRow < ( m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.nRows ) )
	{
		return _BoxType::ebSecondaryValues;
	}

	if( CPoint( 0, 0 ) != m_BoxShutoffValve.pos && lColumn >= m_BoxShutoffValve.pos.x + eBCol::Lock && lColumn <= ( m_BoxShutoffValve.pos.x + eBCol::Unit ) 
		&& lRow >= m_BoxShutoffValve.pos.y && lRow < ( m_BoxShutoffValve.pos.y + m_BoxShutoffValve.nRows ) )
	{
		return _BoxType::ebShutoffValve;
	}

	if( CPoint( 0, 0 ) != m_BoxDpCBCV.pos && lColumn >= m_BoxDpCBCV.pos.x + eBCol::Lock && lColumn <= ( m_BoxDpCBCV.pos.x + eBCol::Unit ) && lRow >= m_BoxDpCBCV.pos.y
			&& lRow < ( m_BoxDpCBCV.pos.y + m_BoxDpCBCV.nRows ) )
	{
		return _BoxType::ebDpCBCValve;
	}

	if( CPoint( 0, 0 ) != m_BoxSmartControlValve.pos && lColumn >= m_BoxSmartControlValve.pos.x + eBCol::Lock && lColumn <= ( m_BoxSmartControlValve.pos.x + eBCol::Unit ) 
			&& lRow >= m_BoxSmartControlValve.pos.y && lRow < ( m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.nRows ) )
	{
		return _BoxType::ebSmartControlValve;
	}

	if( CPoint( 0, 0 ) != m_BoxSmartDpC.pos && lColumn >= m_BoxSmartDpC.pos.x + eBCol::Lock && lColumn <= ( m_BoxSmartDpC.pos.x + eBCol::Unit ) 
			&& lRow >= m_BoxSmartDpC.pos.y && lRow < ( m_BoxSmartDpC.pos.y + m_BoxSmartDpC.nRows ) )
	{
		return _BoxType::ebSmartDpC;
	}

	return _BoxType::ebNoBox;
}

bool CSSheetPanelCirc2::IsCellCanbeEdited( long lColumn, long lRow )
{
	_BoxType CurrentBox = WhatBox( lColumn, lRow );

	switch( CurrentBox )
	{
		case _BoxType::ebTU:
		{
			if( true == m_pHM->GetpTermUnit()->IsVirtual() )
			{
				return false;
			}

			if( lColumn != ( m_BoxTu.pos.x + eBCol::Val ) )
			{
				return false;
			}

			if( lRow == ( m_BoxTu.pos.y + m_Box.TU_Des ) )
			{
				return true;
			}

			int iPosDp = m_BoxTu.pos.y + m_Box.TU_Dp;

			if( m_BoxTu.m_eQType != CTermUnit::_QType::Q )
			{
				int iLower = 1;
				int iUpper = 2;

				switch( m_BoxTu.m_eQType )
				{
					case CTermUnit::_QType::PdT:
						iPosDp += 2;
						break;
				}

				if( lRow >= m_BoxTu.pos.y + m_Box.TU_Flow + iLower && lRow <= m_BoxTu.pos.y + m_Box.TU_Flow + iUpper )
				{
					return true;
				}
			}
			else
			{
				// HYS-1882: Next field after the new DT field.
				if( true == m_pHM->IsDTFieldDisplayedInFlowMode() )
				{
					// If we have a DT field in this case we increment de Dp position to + 1.
					iPosDp += 1;
					if( lRow == (m_BoxTu.pos.y + m_Box.TU_Flow + 1) )
					{
						// If we are on DT field, it can be edited
						return true;
					}
				}

				// If we are in flow case in this case it can be edited.  
				if( lRow == (m_BoxTu.pos.y + m_Box.TU_Flow) )
				{
					return true;
				}
			}

			if( m_BoxTu.m_eDPType != CDS_HydroMod::eDpType::Dp )
			{
				int iLower = 1;
				int iUpper = 1;

				switch( m_BoxTu.m_eDPType )
				{
					case CDS_HydroMod::eDpType::Kv:
					case CDS_HydroMod::eDpType::Cv:
						break;

					case CDS_HydroMod::eDpType::QDpref:
						iUpper = 2;
						break;
				}

				if( lRow >= iPosDp + iLower && lRow <= iPosDp + iUpper )
				{
					return true;
				}
			}
			else if( lRow == iPosDp )
			{
				return true;
			}
		}
		break;

		case _BoxType::ebBVB:
			return false;
			break;

		case _BoxType::ebBVP:
			return false;
			break;

		case _BoxType::ebCV:
			if( true == m_pHM->GetpCV()->IsTaCV() )
			{
				return false;
			}
			else
			{
				if( lColumn != ( m_BoxCv.pos.x + eBCol::Val ) )
				{
					return false;
				}

				if( lRow == ( m_BoxCv.pos.y + m_Box.CVKV_Des ) )
				{
					return true;
				}
			}

			break;

		case _BoxType::ebDPC:
			return false;
			break;

		case _BoxType::ebPrimaryValues:
			return false;
			break;

		case _BoxType::ebCircuitPrimaryPipe:
			if( lColumn != ( m_BoxCircuitPrimaryPipe.pos.x + eBCol::Val ) )
			{
				return false;
			}

			if( lRow == ( m_BoxCircuitPrimaryPipe.pos.y + m_Box.Pipe_Length ) )
			{
				return true;
			}

			break;

		case _BoxType::ebCircuitSecondaryPipe:
			if( m_BoxCircuitSecondaryPipe.pos == CPoint( 0, 0 ) )
			{
				return false;
			}

			if( lColumn != ( m_BoxCircuitSecondaryPipe.pos.x + eBCol::Val ) )
			{
				return false;
			}

			if( lRow == ( m_BoxCircuitSecondaryPipe.pos.y + m_Box.Pipe_Length ) )
			{
				return true;
			}

			break;

		case _BoxType::ebDistributionPipe:
			if( m_BoxDistributionSupplyPipe.pos == CPoint( 0, 0 ) )
			{
				return false;
			}

			if( lColumn != ( m_BoxDistributionSupplyPipe.pos.x + eBCol::Val ) )
			{
				return false;
			}

			if( lRow == ( m_BoxDistributionSupplyPipe.pos.y + m_Box.Pipe_Length ) )
			{
				return true;
			}

			break;

		case _BoxType::ebDistributionReturnPipe:
			if( m_BoxDistributionReturnPipe.pos == CPoint( 0, 0 ) )
			{
				return false;
			}

			if( lColumn != ( m_BoxDistributionReturnPipe.pos.x + eBCol::Val ) )
			{
				return false;
			}

			if( lRow == ( m_BoxDistributionReturnPipe.pos.y + m_Box.Pipe_Length ) )
			{
				return true;
			}

			break;

		case _BoxType::ebPump:
			if( m_BoxPump.pos == CPoint( 0, 0 ) )
			{
				return false;
			}

			if( lColumn == ( m_BoxPump.pos.x + eBCol::Val ) && lRow == ( m_BoxPump.pos.y +  m_BoxPump.iUserHead ) )
			{
				return true;
			}

			break;

		case _BoxType::ebSecondaryValues:
			if( m_BoxSecondaryValues.pos == CPoint( 0, 0 ) )
			{
				return false;
			}

			if( lColumn == ( m_BoxSecondaryValues.pos.x + eBCol::Val ) )
				if( lRow == ( m_BoxSecondaryValues.pos.y +  m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature ) )
				{
					return true;
				}

			break;

		case _BoxType::ebSmartControlValve:
			return false;
			break;

		case _BoxType::ebSmartDpC:
			return false;
			break;

		case _BoxType::ebNoBox:
			break;

		default:
			break;
	}

	return false;
}

void CSSheetPanelCirc2::FormatEditText( long lColumn, long lRow, CString strText, long lStyle, short nMaxEditLen )
{
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
	CSSheet::FormatEditText( lColumn, lRow, strText, lStyle, nMaxEditLen );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
}

void CSSheetPanelCirc2::FormatEditDouble( long lColumn, long lRow, CString strText )
{
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
	CSSheet::FormatEditDouble( lColumn, lRow, strText, SSS_ALIGN_RIGHT );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
}

void CSSheetPanelCirc2::SetPictureLock( long lColumn, long lRow, bool fSet, COLORREF BackgroundColor )
{
	if( 0 == BackgroundColor )
	{
		// Use Title color
		BackgroundColor = GetBackColor( lColumn + 1, lRow );
	}

	SetBackColor( lColumn, lRow, BackgroundColor );

	if( true == fSet )
	{
		SetPictureCellWithID( IDI_LOCKUP, lColumn, lRow, CSSheet::PictureCellType::Icon );
	}
	else
	{
		SetPictureCellWithID( IDI_UNLOCKUP, lColumn, lRow, CSSheet::PictureCellType::Icon );
	}

	// Activate mouse left button event for this picture.
	SetCellProperty( lColumn, lRow, CSSheet::_SSCellProperty::CellCantLeftClick, false );
}

void CSSheetPanelCirc2::FormatBox( long lColumn, long lRow, int iRows, int iTitleID, COLORREF TitleBackgroundColor, bool fShowLockButton, int iCols )
{
	// TODO: Extend columns during printing
	// Fix Col width multiply by .8, it seems that Spread compute size on base of font size = 10.
	// We use a font size of 8.
	double dCellWidthMul = m_bPrinting * 1.5;

	if( dCellWidthMul < 1 )
	{
		dCellWidthMul = 1;
	}

	// 	double dFontSize = GetFontSize( 1, 1 );
	// 	SetColWidthInPixels( lColumn + eBCol::Lock, (int)( dFontSize * PC2_COLWIDTH_LOCK * m_dFontFactor ) );
	// 	SetColWidthInPixels( lColumn + eBCol::Desc, (int)( dFontSize * PC2_COLWIDTH_DESCRIPTION * m_dFontFactor ) );
	// 	SetColWidthInPixels( lColumn + eBCol::Val, (int)( dFontSize * PC2_COLWIDTH_VALUE * m_dFontFactor * dCellWidthMul ) );
	// 	SetColWidthInPixels( lColumn + eBCol::Unit, (int)( dFontSize * PC2_COLWIDTH_UNIT * m_dFontFactor ) );

	// Box Title.
	if( true == fShowLockButton )
	{
		RemoveCellSpan( lColumn, lRow );
		AddCellSpanW( lColumn + 1, lRow, iCols - 1, 1 );
		SetTextPattern( CSSheet::_SSTextPattern::TitleBox );

		if( TitleBackgroundColor != 0 )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)TitleBackgroundColor );
		}

		CString str = TASApp.LoadLocalizedString( iTitleID );
		SetStaticText( lColumn + 1, lRow, iTitleID );
		SetPictureLock( lColumn, lRow, false );
	}
	else
	{
		RemoveCellSpan( lColumn + 1, lRow );
		AddCellSpanW( lColumn, lRow, iCols, 1 );
		SetTextPattern( CSSheet::_SSTextPattern::TitleBox );

		if( TitleBackgroundColor != 0 )
		{
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)TitleBackgroundColor );
		}

		SetStaticText( lColumn, lRow, iTitleID );
	}

	// Border.
	// Col 0 field name.
	int iBorder;

	// Lock and description cell.
	for( int iLoopRow = 1; iLoopRow < iRows; iLoopRow++ )
	{
		iBorder = ( 1 == iLoopRow ) ? 0 : SS_BORDERTYPE_TOP;
		iBorder |= SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;
		SetCellBorder( lColumn + eBCol::Lock, lRow + iLoopRow, lColumn + eBCol::Desc, lRow + iLoopRow, true, iBorder, SS_BORDERSTYLE_FINE_SOLID );
	}

	// Col 1 value.
	for( int iLoopRow = 0; iLoopRow < iRows; iLoopRow++ )
	{
		iBorder = ( 1 == iLoopRow ) ? 0 : SS_BORDERTYPE_TOP;
		iBorder |= SS_BORDERTYPE_LEFT | SS_BORDERTYPE_BOTTOM;
		SetCellBorder( lColumn + eBCol::Val, lRow + iLoopRow, lColumn + eBCol::Val, lRow + iLoopRow, true, iBorder, SS_BORDERSTYLE_FINE_SOLID );
	}

	// Col 2 Unit.
	for( int iLoopRow = 0; iLoopRow < iRows; iLoopRow++ )
	{
		iBorder = ( 1 == iLoopRow ) ? 0 : SS_BORDERTYPE_TOP;
		iBorder |= SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;
		SetCellBorder( lColumn + eBCol::Unit, lRow + iLoopRow, lColumn + eBCol::Unit, lRow + iLoopRow, true, iBorder, SS_BORDERSTYLE_FINE_SOLID );
	}

	SetCellBorder( lColumn + eBCol::Lock, lRow, lColumn + iCols - 1, lRow, true );

	// Box.
	if( true == fShowLockButton )
	{
		SetCellBorder( lColumn + eBCol::Lock, lRow + 1, lColumn + iCols - 1, lRow + iRows - 1, true );
	}
	else
	{
		SetCellBorder( lColumn + eBCol::Lock, lRow, lColumn + iCols - 1, lRow + iRows - 1, true );
	}
}

void CSSheetPanelCirc2::FormatBoxTU( bool bRedraw )
{
	if( m_BoxTu.pos == CPoint( 0, 0 ) || m_pHM->IsaModule() )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	_ClearRange( m_BoxTu.m_eBoxType, m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y, m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y +  m_Box.TU_nRows - 1 );

	// Lock all Terminal Unit Cells.
	SetCellProperty( m_BoxTu.pos.x + eBCol::Butt, m_BoxTu.pos.y, m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_Box.TU_nRows - 1,
					 CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetCellBorder( m_BoxTu.pos.x + eBCol::Butt, m_BoxTu.pos.y, m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_Box.TU_nRows - 1, false );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	FormatStaticText( m_BoxTu.pos.x + eBCol::Butt, m_BoxTu.pos.y, m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_Box.TU_nRows - 1, _T("") );

	// Shape of terminal unit box is depending of flow and Dp description.
	// 5 cases can exist :
	//	 0 - Flow is defined by flow. Power and DT rows are hidden.
	//	 1 - Flow is defined by flow. Power is hidden. DT is displayed.
	//		 Dp is defined by Dp. Qref and Dpref rows are hidden.
	//   2 - Dp is collapsed, Flow is extended Power and DT rows are shown.
	//	 3 - Dp is extended, Flow is collapsed. Qref and Dpref rows are shown.
	//	 4 - Both are extended.
	int iRowQ, iRows;
	iRowQ = m_BoxTu.pos.y + m_Box.TU_Flow;
	m_BoxTu.m_iDpRow = m_Box.TU_Dp;
	iRows = m_Box.TU_Dp + 1;

	// Check if flow is input with another variables...
	int iIncrement = 0;

	// HYS-1882: We look if we have a parent that is a temperature interface when we are in flow mode.
	if( m_pHM->GetpTermUnit()->GetQType() == CTermUnit::_QType::PdT )
	{
		iIncrement = 2;
	}
	// HYS-2025: If parent is a 3way mixing circuit, the temperature regime will not change. DT field should not be displayed for children.
	else if( ( NULL != m_pHM->GetpDesignTemperatureInterface() && m_pHM->GetpRootHM() != m_pHM->GetpDesignTemperatureInterface() )
			 || ( ( true == m_pHM->GetpDesignTemperatureInterface()->GetpSchcat()->IsSecondarySideExist() ) 
			 && ( CDB_CircSchemeCateg::e3WType::e3wTypeMixing != m_pHM->GetpDesignTemperatureInterface()->GetpSchcat()->Get3WType() ) ) )
	{
		// If the root module is not an injection circuit and direct child is not an injection circuit DT should
		// not displayed.
		
		// HYS-1930: we add this additional condition. Because for the auto-adapting variable flow decoupling circuit,
		// the supply temperature at the secondary side is the same as the primary one.
		if( eDpStab::DpStabOnBVBypass != m_pHM->GetpDesignTemperatureInterface()->GetpSch()->GetDpStab() )
		{
			// Show DT with Qtype = flow
			iIncrement = 1;
		}
	}
	else if( m_pHM->GetpSchcat() != NULL && true == m_pHM->GetpSchcat()->IsSecondarySideExist()
			 && CDB_CircSchemeCateg::e3WType::e3wTypeMixing != m_pHM->GetpSchcat()->Get3WType() )
	{
		// HYS-1930: we add this additional condition. Because for the auto-adapting variable flow decoupling circuit,
		// the supply temperature at the secondary side is the same as the primary one.
		if( eDpStab::DpStabOnBVBypass != m_pHM->GetpSch()->GetDpStab() )
		{
			iIncrement = 1;
		}
	}

	m_BoxTu.m_iDpRow += iIncrement;
	iRows += iIncrement;
	m_BoxTu.m_eQType = m_pHM->GetpTermUnit()->GetQType();

	iIncrement = 0;

	switch( m_pHM->GetpTermUnit()->GetDpType() )
	{
		case CDS_HydroMod::eDpType::Kv:
		case CDS_HydroMod::eDpType::Cv:
		case CDS_HydroMod::eDpType::dzeta:
			iIncrement = 1;
			break;

		case CDS_HydroMod::eDpType::QDpref:
			iIncrement = 2;
			break;
	}

	iRows += iIncrement;
	m_BoxTu.m_eDPType = m_pHM->GetpTermUnit()->GetDpType();

	// When we are localized on the secondary side change the box title color
	COLORREF BackGroundColor = _TAH_ORANGE;

	if( NULL != m_pHM->GetpSchcat() && true == m_pHM->GetpSchcat()->IsSecondarySideExist() && CDB_CircSchemeCateg::e3wTypeMixing != m_pHM->GetpSchcat()->Get3WType() )
	{
		BackGroundColor = _TAH_GREEN_MED;
	}

	FormatBox( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y, iRows, IDS_PANELCIRC2_TUTITLE, BackGroundColor );

	// Cell span for description (no unit needed).
	AddCellSpanW( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + 1, 2, 1 );

	// Draw button cells in front of Flow.
	// 	if( false == m_bPrinting )
	// 		SetPictureCellWithID( IDI_SBUTTON, m_BoxTu.pos.x + eBCol::Butt, iRowQ, CSSheet::PictureCellType::Icon );

	// Activate mouse left button event for this button.
	SetCellProperty( m_BoxTu.pos.x + eBCol::Butt, iRowQ, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_Box.TU_Des, 2, 1 );
	SetStaticText( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_Box.TU_Des, IDS_PANELCIRC2_TUDESCRIPTION );

	// Set combo flow.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, iRowQ, 2, 1 );
	_FormatComboList( m_BoxTu.m_eBoxType, m_BoxTu.pos.x + eBCol::Lock, iRowQ );

	ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, iRowQ, SS_CBM_RESETCONTENT, 0, 0 );

	// HYS-1882: Is DT field displayed in flow mode
	bool bDTDisplayedInFlowMode = m_pHM->IsDTFieldDisplayedInFlowMode();

	switch( m_pHM->GetpTermUnit()->GetQType() )
	{
		case CTermUnit::_QType::Q:
			ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, iRowQ, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR )TASApp.LoadLocalizedString( IDS_DLGCBHM_TUFLOW ) );
			
			if( true == bDTDisplayedInFlowMode )
			{
				// HYS-1882: Set DT row.
				AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, iRowQ + 1, 2, 1 );
				SetStaticText( m_BoxTu.pos.x + eBCol::Lock, iRowQ + 1, IDS_PANELCIRC2_TUDELTATEMP );
			}
			
			break;

		case CTermUnit::_QType::PdT:
			ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, iRowQ, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR )TASApp.LoadLocalizedString( IDS_DLGCBHM_TUFLOWPDT ) );

			// Set power row.
			AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, iRowQ + 1, 2, 1 );
			SetStaticText( m_BoxTu.pos.x + eBCol::Lock, iRowQ + 1, IDS_PANELCIRC2_TUPOWER );

			// Set DT row.
			AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, iRowQ + 2, 2, 1 );
			SetStaticText( m_BoxTu.pos.x + eBCol::Lock, iRowQ + 2, IDS_PANELCIRC2_TUDELTATEMP );
			break;
	}

	ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, iRowQ, SS_CBM_SETCURSEL, 0, 0 );
	// Remark: '+1' to avoid null parameter.
	SetCellParam( m_BoxTu.pos.x + eBCol::Lock, iRowQ, (LPARAM)( m_pHM->GetpTermUnit()->GetQType() + 1 ) );

	// Set combo Dp.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, 2, 1 );
	_FormatComboList( m_BoxTu.m_eBoxType, m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow );

	ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, SS_CBM_RESETCONTENT, 0, 0 );

	switch( m_pHM->GetpTermUnit()->GetDpType() )
	{
		case CDS_HydroMod::eDpType::Dp:
			ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( LPCTSTR )TASApp.LoadLocalizedString( IDS_DLGCBHM_TUDP ) );
			break;

		case CDS_HydroMod::eDpType::Kv:
			ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( LPCTSTR )TASApp.LoadLocalizedString( IDS_DLGCBHM_TUDPKV ) );

			// Set Kv.
			AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, 2, 1 );
			SetStaticText( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, IDS_PANELCIRC2_TUKV );
			break;

		case CDS_HydroMod::eDpType::Cv:
			ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( LPCTSTR )TASApp.LoadLocalizedString( IDS_DLGCBHM_TUDPCV ) );

			// Set Cv.
			AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, 2, 1 );
			SetStaticText( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, IDS_PANELCIRC2_TUCV );
			break;

		case CDS_HydroMod::eDpType::QDpref:
			ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( LPCTSTR )TASApp.LoadLocalizedString( IDS_DLGCBHM_TUDPQREFDPREF ) );

			// Set QRef.
			AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, 2, 1 );
			SetStaticText( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, IDS_PANELCIRC2_TUQREF );

			// Set DpRef.
			AddCellSpanW( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 2, 2, 1 );
			SetStaticText( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 2, IDS_PANELCIRC2_TUDPREF );
			break;
	}

	ComboBoxSendMessage( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, SS_CBM_SETCURSEL, 0, 0 );
	// Remark: '+1' to avoid null parameter.
	SetCellParam( m_BoxTu.pos.x + eBCol::Lock, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, (LPARAM)( m_pHM->GetpTermUnit()->GetDpType() + 1 ) );

	// Set value description field.
	FormatEditText( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Des );
	FormatEditText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_Box.TU_Des, _T("") );
	SetCellParam( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + 1, _BoxTU::Des );

	// Note: m_BoxTu.m_eQType = m_pHM->GetpTermUnit()->GetQType() set at the top of this function;
	// Set value flow field.
	switch( m_BoxTu.m_eQType )
	{
		// Remark: 'FormatEditDouble' set the background to '_EDITCOLOR'.
		case CTermUnit::_QType::Q:
			FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, iRowQ );
			SetCellParam( m_BoxTu.pos.x + eBCol::Val, iRowQ, _BoxTU::Flow );
			
			if( true == bDTDisplayedInFlowMode )
			{
				// HYS-1882
				FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, iRowQ + 1 );
				SetCellParam( m_BoxTu.pos.x + eBCol::Val, iRowQ + 1, _BoxTU::DeltaT );
			}

			break;

		case CTermUnit::_QType::PdT:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
			SetStaticText( m_BoxTu.pos.x + eBCol::Val, iRowQ, _T("") );
			FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, iRowQ + 1 );
			SetCellParam( m_BoxTu.pos.x + eBCol::Val, iRowQ + 1, _BoxTU::Power );
			// HYS-1882: Now DT field is always editable in this case.
			FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, iRowQ + 2 );

			SetCellParam( m_BoxTu.pos.x + eBCol::Val, iRowQ + 2, _BoxTU::DeltaT );
			break;
	}

	// Set value Dp field.
	switch( m_BoxTu.m_eDPType )
	{
		case CDS_HydroMod::eDpType::Dp:
			FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow );
			SetCellParam( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, _BoxTU::Dp );
			break;

		case CDS_HydroMod::eDpType::QDpref:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
			SetStaticText( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, _T("") );
			FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1 );
			SetCellParam( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, _BoxTU::Qref );
			FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 2 );
			SetCellParam( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 2, _BoxTU::DpRef );
			break;

		case CDS_HydroMod::eDpType::Kv:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
			SetStaticText( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, _T("") );
			FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1 );
			SetCellParam( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, _BoxTU::Kv );
			break;

		case CDS_HydroMod::eDpType::Cv:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
			SetStaticText( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, _T("") );
			FormatEditDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1 );
			SetCellParam( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, _BoxTU::Cv );
			break;
	}

	// Set unit flow field.
	TCHAR unitname[_MAXCHARS];

	// Note: m_BoxTu.m_eQType = m_pHM->GetpTermUnit()->GetQType();
	switch( m_BoxTu.m_eQType )
	{
		case CTermUnit::_QType::Q:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, iRowQ, unitname );
			// HYS-2025: Don't show the unit field if DT is not displayed.
			if( true == bDTDisplayedInFlowMode )
			{
				// HYS-1882
				GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), unitname );
				SetStaticText( m_BoxTu.pos.x + eBCol::Unit, iRowQ + 1, unitname );
			}
			break;

		case CTermUnit::_QType::PdT:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, iRowQ, unitname );
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_TH_POWER ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, iRowQ + 1, unitname );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, iRowQ + 2, unitname );
			break;
	}

	// Set unit Dp field.
	switch( m_BoxTu.m_eDPType )
	{
		case CDS_HydroMod::eDpType::Dp:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, unitname );
			break;

		case CDS_HydroMod::eDpType::QDpref:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, unitname );
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, unitname );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 2, unitname );
			break;

		case CDS_HydroMod::eDpType::Kv:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, unitname );
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
			GetNameOf( m_pUnitDB->GetUnit( _C_KVCVCOEFF, 0 ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, unitname );
			break;

		case CDS_HydroMod::eDpType::Cv:
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, unitname );
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
			GetNameOf( m_pUnitDB->GetUnit( _C_KVCVCOEFF, 1 ), unitname );
			SetStaticText( m_BoxTu.pos.x + eBCol::Unit, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, unitname );
			break;
	}

	RefreshBoxTU( InputValuesComeFromHydroMod );
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

void CSSheetPanelCirc2::RefreshBoxTU( InputValuesOrigin eInputValuesOrigin )
{
	double dSI = 0.0;

	if( m_BoxTu.pos == CPoint( 0, 0 ) || m_pHM->IsaModule() || NULL == m_pTechParam )
	{
		return;
	}

	// User has input values, we must read them and save them in the current hydraulic circuit 'CDS_HydroMod' object.
	if( InputValuesOrigin::InputValuesComeFromUser == eInputValuesOrigin )
	{
		// First save modified value into the temporary CDS.
		long lValue = ( long )GetCellParam( m_lCol, m_lRow );

		switch( lValue )
		{
			case _BoxTU::Des:
				m_pHM->GetpTermUnit()->SetDescription( m_strCellText );
				break;

			case _BoxTU::Flow:
				dSI = CDimValue::CUtoSI( _U_FLOW, m_dCellDouble );
				m_pHM->GetpTermUnit()->SetQ( dSI );
				break;

			case _BoxTU::Power:
				dSI = CDimValue::CUtoSI( _U_TH_POWER, m_dCellDouble );
				m_pHM->GetpTermUnit()->SetPdT( dSI, DBL_MAX );
				break;

			case _BoxTU::DeltaT:
			{
				bool bSaveValue = true;
				dSI = CDimValue::CUtoSI( _U_DIFFTEMP, m_dCellDouble );

				if( dSI > 0.0 )
				{
					// HYS-1882: Set return temperature with the new DT. It is considered in SetPdt when we call GetQ() to compute flow.
					double dReturnTemp = 0.0;
					double dSupplyTemp = 0.0;

					if( true == m_pHM->IsInjectionCircuit() )
					{
						// Set secondary return temperature.
						dSupplyTemp = m_pHM->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply, CAnchorPt::CircuitSide::CircuitSide_Secondary );
						dReturnTemp = ( ProjectType::Cooling == m_pTechParam->GetProjectApplicationType() ) ? dSupplyTemp + dSI : dSupplyTemp - dSI;

						// Verify that the new secondary return temperature becomes not lower or equal to the freezing point.
						if( ProjectType::Heating == m_pTechParam->GetProjectApplicationType() )
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
							( (CDS_HmInj*)m_pHM )->SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );
						}
					}

					if( true == bSaveValue )
					{
						// We verify the primary return temperature with this new DT only if we do not already verified with the injection above.
						if( false == m_pHM->IsInjectionCircuit() )
						{
							// Set primary return temperature.
							dSupplyTemp = m_pHM->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply, CAnchorPt::CircuitSide::CircuitSide_Primary );
							dReturnTemp = ( ProjectType::Cooling == m_pTechParam->GetProjectApplicationType() ) ? dSupplyTemp + dSI : dSupplyTemp - dSI;

							// Verify that the new return temperature becomes not lower or equal to the freezing point.
							if( ProjectType::Heating == m_pTechParam->GetProjectApplicationType() )
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
						}

						if( true == bSaveValue )
						{
							m_pHM->SetTemperature( dReturnTemp, CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return, CAnchorPt::CircuitSide::CircuitSide_Primary );
						}
					}
				}
				
				if( true == bSaveValue )
				{
					if( CTermUnit::_QType::PdT == m_pHM->GetpTermUnit()->GetQType() )
					{
						m_pHM->GetpTermUnit()->SetPdT( DBL_MAX, dSI );
					}
					else if( CTermUnit::_QType::Q == m_pHM->GetpTermUnit()->GetQType() )
					{
						m_pHM->GetpTermUnit()->SetDTFlowMode( dSI );
					}

					m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dDT = dSI;
				}
			}
			break;

			case _BoxTU::Dp:
				dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );		// Dp entered by user invalidate Qref,DpRef
				m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, dSI );
				break;

			case _BoxTU::DpRef:
				dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
				m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, dSI, DBL_MAX );
				m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dDpRef = dSI;
				break;

			case _BoxTU::Qref:
				dSI = CDimValue::CUtoSI( _U_FLOW, m_dCellDouble );
				m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, DBL_MAX, dSI );
				m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dQRef = dSI;
				break;

			case _BoxTU::Kv:
				m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Kv, m_dCellDouble );
				m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dKv = dSI;
				break;

			case _BoxTU::Cv:
				m_pHM->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Cv, m_dCellDouble );
				m_pHM->GetpTermUnit()->GetpDpWorkStructure()->m_dCv = dSI;
				break;
		}
	}

	bool bDTDisplayedInFlowMode = m_pHM->IsDTFieldDisplayedInFlowMode();

	// Description.
	SetCellText( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Des, m_pHM->GetpTermUnit()->GetDescription() );

	// Flow.
	dSI = m_pHM->GetpTermUnit()->GetQ();
	SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Flow, _U_FLOW, dSI );

	if( CTermUnit::_QType::PdT == m_BoxTu.m_eQType )
	{
		// Power.
		SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Flow + 1, _U_TH_POWER, m_pHM->GetpTermUnit()->GetP() );

		// DeltaT.
		SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Flow + 2, _U_DIFFTEMP, m_pHM->GetpTermUnit()->GetDT() );
	}
	else if( true == bDTDisplayedInFlowMode )
	{
		// DeltaT.
		if( false == m_pHM->CheckIfTempAreValid() )
		{
			SetForeColor( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Flow + 1, _RED );
		}
		else
		{
			SetForeColor( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Flow + 1, _BLACK );
		}
		
		// HYS-1882: We always display a default DT value when we are in init process and if user enter nul value.
		SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_Box.TU_Flow + 1, _U_DIFFTEMP, m_pHM->GetpTermUnit()->GetDTFlowMode( true ) );
	}

	// Dp.
	dSI = m_pHM->GetpTermUnit()->GetDp();
	SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow, _U_DIFFPRESS, dSI );

	if( m_BoxTu.m_eDPType != CDS_HydroMod::eDpType::Dp )
	{
		switch( m_BoxTu.m_eDPType )
		{
			case CDS_HydroMod::eDpType::Kv:
			{
				double dKv;
				m_pHM->GetpTermUnit()->GetDp( NULL, &dKv, NULL );

				// Kv.
				SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, _C_KVCVCOEFF, dKv );
			}
			break;

			case CDS_HydroMod::eDpType::Cv:
			{
				double dCv;
				m_pHM->GetpTermUnit()->GetDp( NULL, &dCv, NULL );

				// Cv.
				SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, _C_KVCVCOEFF, dCv );
			}
			break;

			case CDS_HydroMod::eDpType::QDpref:
			{
				double dQRef, dDpRef;
				m_pHM->GetpTermUnit()->GetDp( NULL, &dDpRef, &dQRef );

				// Qref.
				SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 1, _U_FLOW, dQRef );

				// DpRef.
				SetCUDouble( m_BoxTu.pos.x + eBCol::Val, m_BoxTu.pos.y + m_BoxTu.m_iDpRow + 2, _U_DIFFPRESS, dDpRef );
			}
			break;
		}
	}
}

void CSSheetPanelCirc2::FormatBoxPipe( sBoxPipe *prBoxPipe )
{
	if( prBoxPipe->pos == CPoint( 0, 0 ) )
	{
		return;
	}

	_ClearRange( prBoxPipe->m_eBoxType, prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y, prBoxPipe->pos.x + eBCol::Unit, prBoxPipe->pos.y +  m_Box.Pipe_nRows - 1 );

	switch( prBoxPipe->pPipe->GetLocate() )
	{
		case CDS_HydroMod::eHMObj::eCircuitPrimaryPipe:
			FormatBox( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y, m_Box.Pipe_nRows, IDS_PANELCIRC2_PIPETITLE, 0, true );
			break;

		case CDS_HydroMod::eHMObj::eCircuitSecondaryPipe:
		{
			// When we are localized on the secondary side change the box title color
			COLORREF BackGroundColor = _TAH_ORANGE;

			if( NULL != m_pHM->GetpSchcat() && CDB_CircSchemeCateg::e3wTypeMixing != m_pHM->GetpSchcat()->Get3WType() )
			{
				BackGroundColor = _TAH_GREEN_MED;
			}

			FormatBox( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y, m_Box.Pipe_nRows, IDS_PANELCIRC2_SECPIPETITLE, BackGroundColor, true );
		}
		break;

		case CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe:
		{
			// When we are localized on the bypass change the box title color.
			COLORREF BackGroundColor = _TAH_ORANGE;

			// For the moment we show the bypass pipe box only for the auto-adapting variable flow decoupling circuit.
			// In this case the pipe is orange because at the primary side of the circuit.

			FormatBox( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y, m_Box.Pipe_nRows, IDS_PANELCIRC2_BYPASSPIPETITLE, BackGroundColor, true );
		}
		break;

		case CDS_HydroMod::eHMObj::eDistributionSupplyPipe:
		{
			// Pay attention for the title. In direct return mode we keep the old title "Distribution pipe'. But when we are in the
			// reverse mode, we need to be more explicit: "Distribution supply pipe".
			int iTitleID = ( CDS_HydroMod::ReturnType::Direct == m_pHM->GetReturnType() ) ? IDS_PANELCIRC2_DISTPIPETITLE : IDS_PANELCIRC2_DISTSUPPLYPIPETITLE;
			FormatBox( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y, m_Box.Pipe_nRows, iTitleID, _TAH_BERYLLIUM, true );
		}
		break;

		case CDS_HydroMod::eHMObj::eDistributionReturnPipe:
			FormatBox( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y, m_Box.Pipe_nRows, IDS_PANELCIRC2_DISTRETURNPIPETITLE, _TAH_BERYLLIUM, true );
			break;

		default:
			ASSERT( 0 );
			break;
	}

	// Cell span for pipe series and pipe size (no unit needed).
	AddCellSpanW( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Serie, 2, 1 );
	AddCellSpanW( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Size, 2, 1 );

	// Picture cells.
	if( false == m_bPrinting )
	{
		SetPictureCellWithID( IDI_SBUTTON, prBoxPipe->pos.x + eBCol::Butt, prBoxPipe->pos.y + m_Box.Pipe_Acc, CSSheet::PictureCellType::Icon );
	}

	// Activate mouse left button event for this button.
	SetCellProperty( prBoxPipe->pos.x + eBCol::Butt, prBoxPipe->pos.y + m_Box.Pipe_Acc, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Serie, 2, 1 );
	SetStaticText( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Serie, IDS_PANELCIRC2_PIPESERIE );
	AddCellSpanW( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Size, 2, 1 );
	SetStaticText( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Size, IDS_PANELCIRC2_PIPESIZE );
	AddCellSpanW( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Length, 2, 1 );
	SetStaticText( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Length, IDS_PANELCIRC2_PIPELENGTH );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Dp, 2, 1 );
	SetStaticText( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Dp, IDS_PANELCIRC2_PIPEDP );
	AddCellSpanW( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Acc, 2, 1 );
	SetStaticText( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y + m_Box.Pipe_Acc, IDS_PANELCIRC2_ACCESSORIESDP );

	// Combo fields and value fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( prBoxPipe->m_eBoxType, prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Serie );
	_FormatComboList( prBoxPipe->m_eBoxType, prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Size );
	FormatEditDouble( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Length );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	SetStaticText( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Dp, _T("") );
	SetStaticText( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Acc, _T("") );

	// Unit fields.
	TCHAR unitname[_MAXCHARS];
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_LENGTH ), unitname );
	SetStaticText( prBoxPipe->pos.x + eBCol::Unit, prBoxPipe->pos.y + m_Box.Pipe_Length, unitname );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( prBoxPipe->pos.x + eBCol::Unit, prBoxPipe->pos.y + m_Box.Pipe_Dp, unitname );
	SetStaticText( prBoxPipe->pos.x + eBCol::Unit, prBoxPipe->pos.y + m_Box.Pipe_Acc, unitname );
}

void CSSheetPanelCirc2::RefreshBoxPipe( sBoxPipe *prBoxPipe, InputValuesOrigin eInputValuesOrigin )
{
	if( NULL == prBoxPipe->pPipe )
	{
		return;
	}

	if( prBoxPipe->pos == CPoint( 0, 0 ) )
	{
		return;
	}

	double dSI = 0.0;

	// User has input values, we must read them and save them in the current hydraulic circuit 'CDS_HydroMod' object.
	if( InputValuesOrigin::InputValuesComeFromUser == eInputValuesOrigin )
	{
		// First save modified value into the temporary CDS.
		long lValue = m_lRow - prBoxPipe->pos.y;

		if( lValue == m_Box.Pipe_Length )
		{
			dSI = CDimValue::CUtoSI( _U_LENGTH, m_dCellDouble );
			prBoxPipe->pPipe->SetLength( dSI );
		}
	}

	// Refresh cell Length.
	dSI = prBoxPipe->pPipe->GetLength();
	SetCUDouble( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Length, _U_LENGTH, dSI, false );

	// Refresh Dp.
	dSI = prBoxPipe->pPipe->GetPipeDp();
	SetCUDouble( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Dp, _U_DIFFPRESS, dSI );

	dSI = prBoxPipe->pPipe->GetSingularityTotalDp( true );
	SetCUDouble( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Acc, _U_DIFFPRESS, dSI );

	// Refresh LockPicture.
	// 	COLORREF BackgroundColor = 0;
	// 	BackgroundColor = GetBackColor( prBoxPipe->pos.x + eBCol::Desc, prBoxPipe->pos.y);
	SetPictureLock( prBoxPipe->pos.x + eBCol::Lock, prBoxPipe->pos.y, prBoxPipe->pPipe->IsLocked()/*, BackgroundColor*/ );
	ChangePipeSize( prBoxPipe );
}

void CSSheetPanelCirc2::ResetPipeSerie( sBoxPipe *prBoxPipe )
{
	if( NULL == prBoxPipe )
	{
		return;
	}

	if( NULL == prBoxPipe->pPipe )
	{
		return;
	}

	CTable *pPipeSerie = prBoxPipe->pPipe->GetPipeSeries();

	if( NULL == pPipeSerie )	// PipeSerie doesn't exist yet, take it from PrjParams
	{
		CString strPipeID;

		switch( prBoxPipe->pPipe->GetLocate() )
		{
			case CDS_HydroMod::eHMObj::eCircuitPrimaryPipe:
			case CDS_HydroMod::eHMObj::eCircuitSecondaryPipe:
				strPipeID = m_pPrjParams->GetPrjParamID( CPrjParams::PipeCircSerieID );
				break;

			case CDS_HydroMod::eHMObj::eCircuitBypassPrimaryPipe:
				strPipeID = m_pPrjParams->GetPrjParamID( CPrjParams::PipeByPassSerieID );
				break;

			case CDS_HydroMod::eHMObj::eDistributionSupplyPipe:
				strPipeID = m_pPrjParams->GetPrjParamID( CPrjParams::PipeDistSupplySerieID );
				break;

			case CDS_HydroMod::eHMObj::eDistributionReturnPipe:
				strPipeID = m_pPrjParams->GetPrjParamID( CPrjParams::PipeDistReturnSerieID );
				break;

			default:
				ASSERT( 0 );
				break;
		}

		if( false == strPipeID.IsEmpty() )
		{
			CTable *pTab = TASApp.GetpPipeDB()->GetPipeTab();
			ASSERT( pTab );
			pPipeSerie = ( CTable * )( pTab->Get( ( LPCTSTR )strPipeID ).MP );
			prBoxPipe->pPipe->SetPipeSeries( pPipeSerie );
		}
	}

	if( NULL != pPipeSerie )
	{
		if( pPipeSerie != ( CTable * )GetCellParam( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Serie ) )
		{
			CString str = pPipeSerie->GetName();
			ComboBoxSendMessage( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Serie, SS_CBM_RESETCONTENT, 0, 0 );
			ComboBoxSendMessage( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Serie, SS_CBM_ADDSTRING, 0, (LPARAM)( ( LPCTSTR )str ) );
			ComboBoxSendMessage( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Serie, SS_CBM_SETCURSEL, 0, 0 );

			SetCellParam( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Serie, (LPARAM)pPipeSerie );
		}
	}

	RefreshBoxPipe( prBoxPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
}

void CSSheetPanelCirc2::ChangePipeSize( sBoxPipe *prBoxPipe )
{
	// Current selected pipe.
	CDB_Pipe *pPipe = ( CDB_Pipe * )( prBoxPipe->pPipe->GetIDPtr().MP );

	if( NULL != pPipe )
	{
		// Fill cell pipe size.
		ComboBoxSendMessage( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Size, SS_CBM_RESETCONTENT, 0, 0 );

		if( false == m_pHM->IsLocked( prBoxPipe->pPipe->GetLocate() ) )
		{
			SetFontBold( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Size, false );
		}
		else
		{
			SetFontBold( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Size, true );
		}

		ComboBoxSendMessage( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Size, SS_CBM_ADDSTRING, 0, (LPARAM)( pPipe->GetName() ) );
		ComboBoxSendMessage( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Size, SS_CBM_SETCURSEL, 0, 0 );

		SetCellParam( prBoxPipe->pos.x + eBCol::Val, prBoxPipe->pos.y + m_Box.Pipe_Size, (LPARAM)( pPipe->GetIDPtr().MP ) );
	}
}

void CSSheetPanelCirc2::FormatBoxBV( CDS_HydroMod::eHMObj locate, bool fRedraw )
{
	CDS_HydroMod::CBV *pBV;

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	switch( locate )
	{
		case CDS_HydroMod::eHMObj::eBVprim:
			pBV = m_pHM->GetpBv();

			if( false == m_pHM->GetpSch()->IsAnchorPtExist( CAnchorPt::eFunc::BV_P ) )
			{
				return;
			}

			break;

		case CDS_HydroMod::eHMObj::eBVsec:
			pBV = m_pHM->GetpSecBv();
			break;

		case CDS_HydroMod::eHMObj::eBVbyp:
			pBV = m_pHM->GetpBypBv();
			break;

		default:
			ASSERT( 0 );
			return;
	}


	sBoxBv *prBoxBv = InitpBoxBv( locate, pBV );

	if( prBoxBv->pos == CPoint( 0, 0 ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	_ClearRange( prBoxBv->m_eBoxType, prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y, prBoxBv->pos.x + eBCol::Unit, prBoxBv->pos.y + prBoxBv->BV_nRows - 1 );

	// Lock and erase all BV-P cells.
	SetCellProperty( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, prBoxBv->pos.x + eBCol::Unit, prBoxBv->pos.y + prBoxBv->BV_nRows - 1,
					 CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );
	RemoveCellSpanW( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, prBoxBv->pos.x + eBCol::Unit, prBoxBv->pos.y + prBoxBv->BV_nRows - 1 );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetCellBorder( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, prBoxBv->pos.x + eBCol::Unit, prBoxBv->pos.y + prBoxBv->BV_nRows - 1, false );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	FormatStaticText( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, prBoxBv->pos.x + eBCol::Unit, prBoxBv->pos.y + prBoxBv->BV_nRows - 1, _T("") );

	bool fKvSignalExist = ( true == ( pBV && true == pBV->IsKvSignalEquipped() ) );
	int iRows;

	if( true == prBoxBv->bExt )
	{
		iRows = prBoxBv->BV_Dp + ( ( true == fKvSignalExist ) ? 1 : 0 ) + ( ( true == prBoxBv->bExt ) ? prBoxBv->BV_PN : 0 );
	}
	else
	{
		iRows = prBoxBv->BV_Dp + ( ( true == prBoxBv->bExt ) ? prBoxBv->BV_PN : 0 );
	}

	switch( locate )
	{
		case CDS_HydroMod::eHMObj::eBVprim:
		{
			COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::BV_P );
			FormatBox( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y, ( NULL == pBV ) ? 1 : iRows + 1, IDS_PANELCIRC2_BVPTITLE, BackGroundColor,
					   ( NULL == pBV ) ? false : true );

			if( true == m_pHM->GetpSch()->IsAnchorPtOptional( CAnchorPt::eFunc::BV_P ) )
			{
				_DrawCheckbox( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, ( NULL == pBV ) ? CDS_HydroMod::Never : CDS_HydroMod::Always );
				// Activate mouse left button event for this button.
				SetCellProperty( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, CSSheet::_SSCellProperty::CellCantLeftClick, false );
			}
		}
		break;

		case CDS_HydroMod::eHMObj::eBVsec:
		{
			// When we are localized on the secondary side change the box title color
			COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::BV_S );
			FormatBox( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y, ( NULL == pBV ) ? 1 : iRows + 1, IDS_PANELCIRC2_BVSTITLE, BackGroundColor,
					   ( NULL == pBV ) ? false : true );

			if( true == m_pHM->GetpSch()->IsAnchorPtOptional( CAnchorPt::eFunc::BV_S ) )
			{
				_DrawCheckbox( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, ( NULL == pBV ) ? CDS_HydroMod::Never : CDS_HydroMod::Always );
				// Activate mouse left button event for this button.
				SetCellProperty( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, CSSheet::_SSCellProperty::CellCantLeftClick, false );
			}
		}
		break;

		case CDS_HydroMod::eHMObj::eBVbyp:
		{
			
			if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
			{
				// HYS-1930: auto-adapting variable flow decoupling circuit.
				COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::BV_Byp );
				FormatBox( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y, ( NULL == pBV ) ? 1 : iRows + 1, IDS_PANELCIRC2_BVBTITLE, BackGroundColor,
					   ( NULL == pBV ) ? false : true );
			}
			else
			{
				if( true == m_pHM->IsClass( CLASS( CDS_Hm3W ) ) )
				{
					// Selected product for Bypass valve doesn't exist display only overflow.
					_DrawCheckbox( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, ( ( CDS_Hm3W * )m_pHM )->GetUseBypBv() );

					// Activate mouse left button event for this button.
					SetCellProperty( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y, CSSheet::_SSCellProperty::CellCantLeftClick, false );

					if( NULL != pBV && NULL == *pBV->GetIDPtr().ID )
					{
						COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::BV_Byp );
						FormatBox( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y, 1 + 1, IDS_PANELCIRC2_BVBTITLE, BackGroundColor, false );

						// Description fields.
						SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
						AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_OverFlow, 2, 1 );
						SetStaticText( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_OverFlow, IDS_PANELCIRC2_OVERFLOW );

						// Value field.
						RemoveCellSpan( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_OverFlow );
						SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
						SetStaticText( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_OverFlow, _T("") );
						SetCUDouble( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_OverFlow, _U_NODIM, m_pHM->GetBypOverFlow() );

						// Unit fields.
						SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
						SetStaticText( prBoxBv->pos.x + eBCol::Unit, prBoxBv->pos.y + prBoxBv->BV_OverFlow, _T("%") );

						// Change Picture.
						m_pHM->ResetSchemeIDPtr();
						FormatBoxPicture();
						TASApp.OverridedSetRedraw( this, TRUE );
						Invalidate();

						return;
					}
				}

				COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::BV_Byp );
				FormatBox( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y, iRows + 1, IDS_PANELCIRC2_BVBTITLE, BackGroundColor, true );
			}
		}
		break;

		default:
		{
			ASSERT( 0 );
			Invalidate();
			TASApp.OverridedSetRedraw( this, TRUE );
			return;
		}
	}


	// If the valve container doesn't exist skip fields filling (Secondary Bv for instance)
	if( NULL == pBV )
	{
		m_pHM->ResetSchemeIDPtr();
		FormatBoxPicture();
		TASApp.OverridedSetRedraw( this, TRUE );
		Invalidate();
		return;
	}

	// Draw button cells in front of name.
	if( true == prBoxBv->bExt )
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SMINUS, prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y + prBoxBv->BV_Name, CSSheet::PictureCellType::Icon );
		}

		iRows = prBoxBv->pos.y + prBoxBv->BV_PN;
	}
	else
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SPLUS, prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y + prBoxBv->BV_Name, CSSheet::PictureCellType::Icon );
		}

		iRows = prBoxBv->pos.y;
	}

	// Activate mouse left button event for this minus/plus button.
	SetCellProperty( prBoxBv->pos.x + eBCol::Butt, prBoxBv->pos.y + prBoxBv->BV_Name, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Cell span for BVP Name, Connection, Version and PN.
	AddCellSpanW( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, 2, 1 );
	AddCellSpanW( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Loc, 2, 1 );

	if( true == prBoxBv->bExt )
	{
		AddCellSpanW( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, 2, 1 );
		AddCellSpanW( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect, 2, 1 );
		AddCellSpanW( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version, 2, 1 );
		AddCellSpanW( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN, 2, 1 );
	}
	else
	{
		RemoveCellSpan( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type );
		RemoveCellSpan( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect );
		RemoveCellSpan( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version );
		RemoveCellSpan( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN );
	}

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name, 2, 1 );
	SetStaticText( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name, IDS_PANELCIRC2_BVPNAME );

	if( true == prBoxBv->bMeasuringValve )
	{
		AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Loc, 2, 1 );
		SetStaticText( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Loc, IDS_SSHEETPC2_STATICMVLOC );
	}

	if( true == prBoxBv->bExt )
	{
		AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, 2, 1 );
		SetStaticText( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, IDS_PANELCIRC2_BVPTYPE );
		AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect, 2, 1 );
		SetStaticText( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect, IDS_PANELCIRC2_BVPCONNECT );
		AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version, 2, 1 );
		SetStaticText( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version, IDS_PANELCIRC2_BVPVERSION );
		AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN, 2, 1 );
		SetStaticText( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN, IDS_PANELCIRC2_BVPPN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_Setting, 2, 1 );
	SetStaticText( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_Setting, IDS_PANELCIRC2_BVPSETTING );

	if( true == prBoxBv->bExt )
	{
		AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_Dp, 2, 1 );
		SetStaticText( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_Dp, IDS_PANELCIRC2_BVPDP );
	}
	else
	{
		if( false == fKvSignalExist )
		{
			AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_Dp, 2, 1 );
			SetStaticText( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_Dp, IDS_PANELCIRC2_BVPDP );
		}
	}

	if( true == fKvSignalExist )
	{
		if( true == prBoxBv->bExt )
		{
			AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_DpSignal, 2, 1 );
			SetStaticText( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_DpSignal, IDS_PANELCIRC2_BVPDPSIGNAL );
		}
		else
		{
			AddCellSpanW( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_Dp, 2, 1 );
			SetStaticText( prBoxBv->pos.x + eBCol::Lock, iRows + prBoxBv->BV_Dp, IDS_PANELCIRC2_BVPDPSIGNAL );
		}
	}

	// Combo and value fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( prBoxBv->m_eBoxType, prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name );

	if( true == prBoxBv->bMeasuringValve )
	{
		_FormatComboList( prBoxBv->m_eBoxType, prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Loc );
	}

	if( true == prBoxBv->bExt )
	{
		_FormatComboList( prBoxBv->m_eBoxType, prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type );
		_FormatComboList( prBoxBv->m_eBoxType, prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect );
		_FormatComboList( prBoxBv->m_eBoxType, prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version );
		_FormatComboList( prBoxBv->m_eBoxType, prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	SetStaticText( prBoxBv->pos.x + eBCol::Val, iRows + prBoxBv->BV_Setting, _T("") );

	if( true == prBoxBv->bExt )
	{
		SetStaticText( prBoxBv->pos.x + eBCol::Val, iRows + prBoxBv->BV_Dp, _T("") );
	}
	else
	{
		if( false == fKvSignalExist )
		{
			SetStaticText( prBoxBv->pos.x + eBCol::Val, iRows + prBoxBv->BV_Dp, _T("") );
		}
	}

	if( true == fKvSignalExist )
	{
		if( true == prBoxBv->bExt )
		{
			SetStaticText( prBoxBv->pos.x + eBCol::Val, iRows + prBoxBv->BV_DpSignal, _T("") );
		}
		else
		{
			SetStaticText( prBoxBv->pos.x + eBCol::Val, iRows + prBoxBv->BV_Dp, _T("") );
		}
	}

	// Unit fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( prBoxBv->pos.x + eBCol::Unit, iRows + prBoxBv->BV_Setting, IDS_TURNS );

	if( true == prBoxBv->bExt )
	{
		SetStaticText( prBoxBv->pos.x + eBCol::Unit, iRows + prBoxBv->BV_Dp, unitname );
	}
	else
	{
		if( false == fKvSignalExist )
		{
			SetStaticText( prBoxBv->pos.x + eBCol::Unit, iRows + prBoxBv->BV_Dp, unitname );
		}
	}

	if( true == fKvSignalExist )
	{
		if( true == prBoxBv->bExt )
		{
			SetStaticText( prBoxBv->pos.x + eBCol::Unit, iRows + prBoxBv->BV_DpSignal, unitname );
		}
		else
		{
			SetStaticText( prBoxBv->pos.x + eBCol::Unit, iRows + prBoxBv->BV_Dp, unitname );
		}
	}

	if( true == fRedraw )
	{
		ChangeBvType( pBV );
	}

	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

CSSheetPanelCirc2::sBoxBv *CSSheetPanelCirc2::InitpBoxBv( CDS_HydroMod::eHMObj locate, CDS_HydroMod::CBV *pBV )
{
	CSSheetPanelCirc2::sBoxBv *pBoxBv = NULL;

	switch( locate )
	{
		case CDS_HydroMod::eHMObj::eBVprim:
			pBoxBv = &m_BoxBvP;
			break;

		case CDS_HydroMod::eHMObj::eBVsec:
			pBoxBv = &m_BoxBvS;
			break;

		case CDS_HydroMod::eHMObj::eBVbyp:
			pBoxBv = &m_BoxBvB;
			break;

		default:
			ASSERT( 0 );
			break;
	}

	if( NULL != pBoxBv )
	{
		pBoxBv->BV_nRows = 8;
		pBoxBv->BV_Loc = 1;
		pBoxBv->BV_Name = 1;
		pBoxBv->BV_OverFlow = 1;
		pBoxBv->BV_Type = 1;
		pBoxBv->BV_Connect = 2;
		pBoxBv->BV_Version = 3;
		pBoxBv->BV_PN = 4;
		pBoxBv->BV_Setting = pBoxBv->BV_Name + 1;
		pBoxBv->BV_Dp = pBoxBv->BV_Name + 2;
		pBoxBv->BV_DpSignal = pBoxBv->BV_Name + 3;

		if( CDS_HydroMod::eHMObj::eBVprim == locate )
		{
			if( NULL != pBV )
			{
				CDS_HydroMod *pHM = pBV->GetpParentHM();

				if( true == pHM->IsDpCExist() )
				{
					pBoxBv->bMeasuringValve = true;
					// Add one line for supporting localization of measuring valve
					pBoxBv->BV_nRows = 9;
					pBoxBv->BV_Name = 2;
					pBoxBv->BV_Setting = pBoxBv->BV_Name + 1;
					pBoxBv->BV_Dp = pBoxBv->BV_Name + 2;
					pBoxBv->BV_DpSignal = pBoxBv->BV_Name + 3;
				}
				else
				{
					pBoxBv->bMeasuringValve = false;
				}
			}
			else
			{
				// Take maximum of space for erasing
				pBoxBv->BV_nRows = 9;
			}
		}
	}

	return pBoxBv;
}

void CSSheetPanelCirc2::ChangeBvType( CDS_HydroMod::CBV *pBV )
{
	if( NULL == pBV )
	{
		return;
	}

	sBoxBv *prBoxBv = InitpBoxBv( pBV->GetHMObjectType(), pBV );
	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == prBoxBv->bExt )
	{
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != pBV->GetpSelBVType() )
		{
			// Dp Signal exist for Commissioning set and for RVTYPE_VV.
			if( true == pBV->IsKvSignalEquipped() )
			{
				if( false == prBoxBv->bDpSignalExist )
				{
					prBoxBv->bDpSignalExist = true;
					FormatBoxBV( pBV->GetHMObjectType(), false );
				}
			}
			else
			{
				if( true == prBoxBv->bDpSignalExist )
				{
					prBoxBv->bDpSignalExist = false;
					FormatBoxBV( pBV->GetHMObjectType(), false );
				}
			}

			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, SS_CBM_RESETCONTENT, 0, 0 );
			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( pBV->GetpSelBVType()->GetString() ) );
			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, SS_CBM_SETCURSEL, 0, 0 );

			SetCellParam( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Type, (LPARAM)( pBV->GetpSelBVType()->GetIDPtr().MP ) );
		}
	}

	ChangeBvConnect( pBV );
}

void CSSheetPanelCirc2::ChangeBvConnect( CDS_HydroMod::CBV *pBV )
{
	if( NULL == pBV )
	{
		return;
	}

	sBoxBv *prBoxBv = InitpBoxBv( pBV->GetHMObjectType(), pBV );
	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == prBoxBv->bExt )
	{
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != pBV->GetpSelBVConn() )
		{
			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( pBV->GetpSelBVConn()->GetString() ) );
			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Connect, (LPARAM)( pBV->GetpSelBVConn()->GetIDPtr().MP ) );
		}
	}

	ChangeBvVersion( pBV );
}

void CSSheetPanelCirc2::ChangeBvVersion( CDS_HydroMod::CBV *pBV )
{
	if( NULL == pBV )
	{
		return;
	}

	sBoxBv *prBoxBv = InitpBoxBv( pBV->GetHMObjectType(), pBV );
	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == prBoxBv->bExt )
	{
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != pBV->GetpSelBVVers() )
		{
			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( pBV->GetpSelBVVers()->GetString() ) );
			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_Version, (LPARAM)( pBV->GetpSelBVVers()->GetIDPtr().MP ) );
		}
	}

	ChangeBvPN( pBV );
}

void CSSheetPanelCirc2::ChangeBvPN( CDS_HydroMod::CBV *pBV )
{
	if( NULL == pBV )
	{
		return;
	}

	sBoxBv *prBoxBv = InitpBoxBv( pBV->GetHMObjectType(), pBV );
	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == prBoxBv->bExt )
	{
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != pBV->GetpSelBVPN() )
		{
			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( pBV->GetpSelBVPN()->GetString() ) );
			ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name + prBoxBv->BV_PN, (LPARAM)( pBV->GetpSelBVPN()->GetIDPtr().MP ) );
		}
	}

	ChangeBv( pBV );
}

void CSSheetPanelCirc2::ChangeBvLoc( CDS_HydroMod::CBV *pBV )
{
	if( NULL == pBV )
	{
		return;
	}

	sBoxBv *prBoxBv = InitpBoxBv( pBV->GetHMObjectType(), pBV );
	TASApp.OverridedSetRedraw( this, FALSE );
	CDS_HydroMod *pHM = pBV->GetpParentHM();

	if( true == prBoxBv->bMeasuringValve && NULL != pHM->GetpDpC() )
	{
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Loc, SS_CBM_RESETCONTENT, 0, 0 );
		CString str = TASApp.LoadLocalizedString( ( eMvLoc::MvLocPrimary == pHM->GetpDpC()->GetMvLoc() ) ? IDS_DLGCBHM_MVONPRIMARY : IDS_DLGCBHM_MVONSECONDARY );
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Loc, SS_CBM_ADDSTRING, 0, (LPARAM)( ( LPCTSTR )str ) );
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Loc, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Loc, (LPARAM)( pHM->GetpDpC()->GetMvLoc() ) );
	}
}

void CSSheetPanelCirc2::ChangeBv( CDS_HydroMod::CBV *pBV )
{
	if( NULL == pBV )
	{
		return;
	}

	sBoxBv *prBoxBv = InitpBoxBv( pBV->GetHMObjectType(), pBV );
	TASApp.OverridedSetRedraw( this, FALSE );

	ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, SS_CBM_RESETCONTENT, 0, 0 );
	ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, SS_CBM_SETCURSEL, 0, 0 );

	if( NULL != pBV && NULL != pBV->GetIDPtr().MP )
	{
		// Update Balancing valve.
		// HYS-1221 : If the selected product is deleted display it in red color.
		if( true == pBV->GetIDPtr().MP->IsDeleted() )
		{
			SetForeColor( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, _RED );
		}
		else
		{
			SetForeColor( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, _BLACK );
		}

		if( false == m_pHM->IsLocked( pBV->GetHMObjectType() ) )
		{
			SetFontBold( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, false );
		}
		else
		{
			SetFontBold( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, true );
		}

		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, SS_CBM_ADDSTRING, 0,
							 (LPARAM)( ( ( CDB_RegulatingValve * )( pBV->GetIDPtr().MP ) )->GetName() ) );
		ComboBoxSendMessage( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Name, (LPARAM)( pBV->GetIDPtr().MP ) );

		// Update Setting and Dp and Dp Signal.
		double dSI = pBV->GetDp();
		int iYPos = ( true == prBoxBv->bExt ) ? prBoxBv->BV_PN : 0;

		if( true == prBoxBv->bExt )
		{
			SetCUDouble( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Dp + iYPos, _U_DIFFPRESS, dSI );
		}
		else
		{
			if( false == pBV->IsKvSignalEquipped() )
			{
				SS_CELLTYPE rCellType;
				GetCellType( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Dp + iYPos, &rCellType );

				SetCUDouble( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Dp + iYPos, _U_DIFFPRESS, dSI );
			}

		}

		if( eb3False == pBV->CheckDpMinMax() )
		{
			SetForeColor( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Dp + iYPos, _RED );
		}
		else
		{
			SetForeColor( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Dp + iYPos, _BLACK );
		}

		CString str = pBV->GetSettingStr();
		SetCellText( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Setting + iYPos, str );

		if( eb3False == pBV->CheckMinOpen() )
		{
			SetForeColor( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Setting + iYPos, _RED );
		}
		else
		{
			SetForeColor( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Setting + iYPos, _BLACK );
		}

		str = _T("");

		if( !( false == pBV->IsKvSignalEquipped() || pBV->GetDpSignal() <= 0 ) )
		{
			str = WriteCUDouble( _U_DIFFPRESS, pBV->GetDpSignal() );
		}

		if( true == pBV->IsKvSignalEquipped() )
		{
			if( true == prBoxBv->bExt )
			{
				SetCellText( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_DpSignal + iYPos, str );
			}
			else
			{
				SetCellText( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Dp + iYPos, str );
			}
		}

		// Refresh LockPicture.
		// 		COLORREF BackgroundColor = 0;
		// 		BackgroundColor = GetBackColor( prBoxBv->pos.x + eBCol::Desc, prBoxBv->pos.y);
		SetPictureLock( prBoxBv->pos.x + eBCol::Lock, prBoxBv->pos.y, m_pHM->IsLocked( pBV->GetHMObjectType() )/*, BackgroundColor*/ );
	}
	else
	{
		// Clear cells.
		int iYPos = ( true == prBoxBv->bExt ) ? prBoxBv->BV_PN : 0;
		SetCUDouble( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Dp + iYPos, _U_DIFFPRESS, 0.0 );
		SetCellText( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_Setting + iYPos, _T("") );

		if( !( false == prBoxBv->bDpSignalExist || pBV->GetDpSignal() <= 0 ) )
		{
			SetCellText( prBoxBv->pos.x + eBCol::Val, prBoxBv->pos.y + prBoxBv->BV_DpSignal + iYPos, _T("") );
		}
	}

	if( CDS_HydroMod::eHMObj::eBVprim == pBV->GetHMObjectType() )
	{
		if( true == prBoxBv->bMeasuringValve )
		{
			ChangeBvLoc( pBV );
		}
	}

	// Change picture.
	m_pHM->ResetSchemeIDPtr();
	FormatBoxPicture();
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

void CSSheetPanelCirc2::_FillCVLoc()
{
	CString str ;
	LPARAM selPos = CvLocation::CvLocNone;

	if( true == m_pHM->GetpCV()->IsCVLocAuto() )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGCBHM_CVONAUTO );
	}
	else
	{
		if( eb3True == m_pHM->GetpCV()->IsCVLocInPrimary() )
		{
			selPos = CvLocation::CvLocPrimSide;
			str = TASApp.LoadLocalizedString( IDS_DLGCBHM_CVONPRIMARY );
		}
		else
		{
			selPos = CvLocation::CvLocSecSide;
			str = TASApp.LoadLocalizedString( IDS_DLGCBHM_CVONSECONDARY );
		}
	}

	ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CV_Loc, SS_CBM_RESETCONTENT, 0, 0 );
	ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CV_Loc, SS_CBM_ADDSTRING, 0, (LPARAM)( ( LPCTSTR )str ) );
	ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CV_Loc, SS_CBM_SETCURSEL, 0, 0 );
	SetCellParam( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CV_Loc, (LPARAM)( m_pHM->GetpCV()->IsCVLocAuto() ) );
}

void CSSheetPanelCirc2::FormatBoxCVKV()
{
	if( m_BoxCv.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	int iRows = m_Box.CVKV_nRows;

	// In case of OnOff CV rows Kvsmax and Kvs are hidden.
	if( true == m_pHM->GetpCV()->IsOn_Off() )
	{
		iRows -= 2;
	}

	// When localization is displayed add a line
	if( true == m_BoxCv.bLocalisationExist )
	{
		iRows ++;
	}

	int iYPos = ( true == m_pHM->GetpCV()->IsOn_Off() ) ? ( m_BoxCv.pos.y + m_Box.CVKV_Kvs - 1 ) : ( m_BoxCv.pos.y + m_Box.CVKV_Kvs );
	COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::ControlValve );
	FormatBox( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, iRows, IDS_PANELCIRC2_CVTITLE, BackGroundColor, true );
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Des, 2, 1 );
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, iYPos, 2, 1 );

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Localization
	if( true == m_BoxCv.bLocalisationExist )
	{
		// Static
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CV_Loc, 2, 1 );
		SetStaticText( m_BoxCv.pos.x  + eBCol::Lock, m_BoxCv.pos.y +  m_Box.CV_Loc, IDS_SSHEETPC2_STATICCVLOC );
		// Combo
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CV_Loc, 2, 1 );
		_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val,  m_BoxCv.pos.y + m_Box.CV_Loc );
		_FillCVLoc();
	}

	// Description fields.
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVKV_Des, 2, 1 );
	SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVKV_Des, IDS_PANELCIRC2_CVDESCRIPTION );
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, iYPos, 2, 1 );

	CString str;

	if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_KVS );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_CV );
	}

	SetStaticText( m_BoxCv.pos.x + eBCol::Lock, iYPos, str );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, iYPos - m_Box.CVKV_Kvs + m_Box.CVKV_Dp, 2, 1 );
	SetStaticText( m_BoxCv.pos.x + eBCol::Lock, iYPos - m_Box.CVKV_Kvs + m_Box.CVKV_Dp, IDS_PANELCIRC2_CVDP );

	if( false == m_pHM->GetpCV()->IsOn_Off() )
	{
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVKV_Kvsmax, 2, 1 );

		if( 0 == m_pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
		{
			str = TASApp.LoadLocalizedString( IDS_KVSMAX );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_CVMAX );
		}

		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVKV_Kvsmax, str );
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVKV_Auth, 2, 1 );
		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVKV_Auth, IDS_PANELCIRC2_CVAUTHORITY );
	}

	// Value fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatEditText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Des, _T("") );
	SetCellParam( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Des, m_Box.CVKV_Des );

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val, iYPos );

	// Activate mouse left button event for the left part of combo.
	SetCellProperty( m_BoxCv.pos.x + eBCol::Val, iYPos, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	SetStaticText( m_BoxCv.pos.x + eBCol::Val, iYPos - m_Box.CVKV_Kvs + m_Box.CVKV_Dp, _T("") );

	if( false == m_pHM->GetpCV()->IsOn_Off() )
	{
		SetStaticText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Kvsmax, _T("") );
		SetStaticText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Auth, _T("") );
	}

	// Unit fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( m_BoxCv.pos.x + eBCol::Unit, iYPos - m_Box.CVKV_Kvs + m_Box.CVKV_Dp, unitname );

	RefreshBoxCV( InputValuesOrigin::InputValuesComeFromHydroMod );

	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

void CSSheetPanelCirc2::ChangeCvKvs()
{
	int iPos = ( true == m_pHM->GetpCV()->IsOn_Off() ) ? ( m_BoxCv.pos.y + m_Box.CVKV_Kvs - 1 ) : ( m_BoxCv.pos.y + m_Box.CVKV_Kvs );

	ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, iPos, SS_CBM_RESETCONTENT, 0, 0 );

	if( false == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
	{
		SetFontBold( m_BoxCv.pos.x + eBCol::Val, iPos, false );
	}
	else
	{
		SetFontBold( m_BoxCv.pos.x + eBCol::Val, iPos, true );
	}

	CString str = WriteCUDouble( _C_KVCVCOEFF, m_pHM->GetpCV()->GetKvs() );
	ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, iPos, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR( str ) ) );
	ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, iPos, SS_CBM_SETCURSEL, 0, 0 );
}

void CSSheetPanelCirc2::FormatBoxCVTA()
{
	if( m_BoxCv.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( NULL == m_pHM->GetpCV() || false == m_pHM->GetpCV()->IsTaCV() )
	{
		return;
	}

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	TASApp.OverridedSetRedraw( this, FALSE );
	_ClearRange( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, m_BoxCv.pos.x + eBCol::Unit, m_BoxCv.pos.y + m_Box.CVTA_nRows - 1 );
	RemoveCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, m_BoxCv.pos.x + eBCol::Unit, m_BoxCv.pos.y + m_Box.CVTA_nRows - 1 );

	// Lock and erase all CVTA cells.
	SetCellProperty( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, m_BoxCv.pos.x + eBCol::Unit, m_BoxCv.pos.y + m_Box.CVTA_nRows - 1, CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetCellBorder( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, m_BoxCv.pos.x + eBCol::Unit, m_BoxCv.pos.y + m_Box.CVTA_nRows - 1, false );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	FormatStaticText( m_BoxCv.pos.x + eBCol::Butt, m_BoxCv.pos.y, m_BoxCv.pos.x + eBCol::Unit, m_BoxCv.pos.y + m_Box.CVTA_nRows - 1, _T("") );

	CDB_CircuitScheme *pSch = ( CDB_CircuitScheme * )( m_pHM->GetSchemeIDPtr().MP );

	// If presetting should be displayed increment position for following cells.
	bool bPreset = ( CDB_ControlProperties::eCVFUNC::PresetPT == pSch->GetCvFunc() || CDB_ControlProperties::eCVFUNC::Presettable == pSch->GetCvFunc() );
	m_BoxCv.rDp = ( true == bPreset ) ? ( m_Box.CVTA_Name + m_Box.CVTA_Dp + m_Box.CVTA_Preset ) : m_Box.CVTA_Dp + m_Box.CVTA_Name;
	m_BoxCv.rAuth = ( true == bPreset ) ? ( m_Box.CVTA_Name + m_Box.CVTA_Auth + m_Box.CVTA_Preset ) : m_Box.CVTA_Auth + m_Box.CVTA_Name;
	m_BoxCv.rPreSet = ( true == bPreset ) ? ( m_Box.CVTA_Name + m_Box.CVTA_Preset ) : 0;
	m_BoxCv.rActuator = ( true == bPreset ) ? ( m_Box.CVTA_Name + m_Box.CVTA_Actuator + m_Box.CVTA_Preset ) : m_Box.CVTA_Actuator + m_Box.CVTA_Name;

	// For ON/OFF control and for PICV doesn't display authority.
	if( true == m_pHM->GetpCV()->IsOn_Off() || eb3True == m_pHM->GetpCV()->IsPICV() )
	{
		m_BoxCv.rAuth = 0;
	}

	if( 0 == m_BoxCv.rAuth )
	{
		m_BoxCv.rActuator--;
	}

	// Adjust vertical position in regards of expanding button.
	// Draw button (+/-)cells in front of name.
	if( true == m_BoxCv.bExt )
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SMINUS, m_BoxCv.pos.x + eBCol::Butt, m_BoxCv.pos.y + m_Box.CVTA_Name, CSSheet::PictureCellType::Icon );
		}

		// Increment relative position.
		m_BoxCv.rDp += m_Box.CVTA_PN;
		m_BoxCv.rActuator += m_Box.CVTA_PN;

		if( 0 != m_BoxCv.rAuth )
		{
			m_BoxCv.rAuth += m_Box.CVTA_PN;
		}

		if( 0 != m_BoxCv.rPreSet )
		{
			m_BoxCv.rPreSet += m_Box.CVTA_PN;
		}
	}
	else
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SPLUS, m_BoxCv.pos.x + eBCol::Butt, m_BoxCv.pos.y + m_Box.CVTA_Name, CSSheet::PictureCellType::Icon );
		}
	}

	// Number of rows really displayed.
	int iRows = m_BoxCv.rActuator + 1;

	// Box title.
	if( eb3True == m_pHM->GetpCV()->IsPICV() )
	{
		COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::PICV );
		FormatBox( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, iRows, IDS_PANELCIRC2_PICVTITLE, BackGroundColor, true );
	}
	else
	{
		COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::ControlValve );
		FormatBox( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, iRows, IDS_PANELCIRC2_CVTITLE, BackGroundColor, true );
	}

	// Activate mouse left button event for this minus/plus button.
	SetCellProperty( m_BoxCv.pos.x + eBCol::Butt, m_BoxCv.pos.y + m_Box.CVTA_Name, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Cells spanning value
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, 2, 1 );

	if( true == m_BoxCv.bExt )
	{
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn, 2, 1 );
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers, 2, 1 );
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN, 2, 1 );
	}
	else
	{
		RemoveCellSpan( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn );
		RemoveCellSpan( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers );
		RemoveCellSpan( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN );
	}

	AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, 2, 1 );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVTA_Name, 2, 1 );
	SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVTA_Name, IDS_PANELCIRC2_CVTANAME );

	if( true == m_BoxCv.bExt )
	{
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn, 2, 1 );
		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn, IDS_PANELCIRC2_CVTACONN );
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers, 2, 1 );
		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers, IDS_PANELCIRC2_CVTAVERS );
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN, 2, 1 );
		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN, IDS_PANELCIRC2_CVPN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Draw combo-box for 3ways mixing circuit with decoupling bypass
	// Localization
	if( true == m_BoxCv.bLocalisationExist )
	{
		// Static
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_Box.CV_Loc, 2, 1 );
		SetStaticText( m_BoxCv.pos.x  + eBCol::Lock, m_BoxCv.pos.y +  m_Box.CV_Loc, IDS_SSHEETPC2_STATICCVLOC );
		// Combo
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CV_Loc, 2, 1 );
		_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val,  m_BoxCv.pos.y + m_Box.CV_Loc );
		_FillCVLoc();
	}

	if( true == bPreset )
	{
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rPreSet, 2, 1 );
		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rPreSet, IDS_PANELCIRC2_CVPRESET );
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rPreSet, 2, 1 );
	}

	// Dp.
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rDp, 2, 1 );

	if( false == m_pHM->GetpSch()->IsAnchorPtExist( CAnchorPt::eFunc::PICV ) )
	{
		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rDp, IDS_PANELCIRC2_CVDP );
	}
	else
	{
		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rDp, IDS_PANELCIRC2_CVDPMIN );
	}

	if( 0 != m_BoxCv.rAuth )
	{
		AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rAuth, 2, 1 );
		SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rAuth, IDS_PANELCIRC2_CVAUTHORITY );
	}

	// Actuator.
	AddCellSpanW( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rActuator, 2, 1 );
	SetStaticText( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y + m_BoxCv.rActuator, IDS_PANELCIRC2_ACTUATOR );

	// Combo and Value fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name );

	if( true == m_BoxCv.bExt )
	{
		_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn );
		_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers );
		_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN );
	}

	_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator );

	// Force right align
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );

	if( true == bPreset )
	{
		SetStaticText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rPreSet, _T("") );
	}

	SetStaticText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rDp, _T("") );

	if( 0 != m_BoxCv.rAuth )
	{
		SetStaticText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rAuth, _T("") );
	}

	// Unit fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( m_BoxCv.pos.x + eBCol::Unit, m_BoxCv.pos.y + m_BoxCv.rDp, unitname );

	ChangeCvConnect();
}

void CSSheetPanelCirc2::RefreshBoxCV( InputValuesOrigin eInputValuesOrigin )
{
	double dSI = 0.0;

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );

	// 2016-10-27: do not call 'm_pHM->IsCvExist()' because we just want to know if CCv exist but not if really a valve is
	//             selected.
	if( NULL == m_pHM->GetpCV() )
	{
		return;
	}

	if( m_BoxCv.pos == CPoint( 0, 0 ) )
	{
		return;    // Not yet initialized
	}

	// User has input values, we must read them and save them in the current hydraulic circuit 'CDS_HydroMod' object.
	if( InputValuesOrigin::InputValuesComeFromUser == eInputValuesOrigin )
	{
		// First save modified value into the temporary CDS.
		long lValue = ( long )GetCellParam( m_lCol, m_lRow );

		if( false == m_pHM->GetpCV()->IsTaCV() )
		{
			if( lValue == m_Box.CVKV_Des )
			{
				m_pHM->GetpCV()->SetDescription( m_strCellText );
			}
			else if( lValue == m_Box.CVKV_Kvs )
			{
				if( true == m_BoxCv.bKvEditMode )
				{
					double dSI = CDimValue::CUtoSI( _C_KVCVCOEFF, m_dCellDouble );

					if( true == m_pHM->GetpCV()->IsBestCV( dSI ) )
					{
						m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false );
					}
					else
					{
						m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, true );
					}

					m_pHM->GetpCV()->SetKvs( dSI );
				}
			}
		}
	}

	// Write informations.
	if( false == m_pHM->GetpCV()->IsTaCV() )
	{
		// Description.
		SetCellText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Des, m_pHM->GetpCV()->GetDescription() );

		// Kvs.
		ChangeCvKvs();

		if( true == m_pHM->GetpCV()->IsOn_Off() )
		{
			// Dp.
			dSI = m_pHM->GetpCV()->GetDp();
			SetCUDouble( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Dp - 1, _U_DIFFPRESS, dSI );
		}
		else
		{
			// Dp.
			dSI = m_pHM->GetpCV()->GetDp();
			SetCUDouble( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Dp, _U_DIFFPRESS, dSI );

			// Kvs max.
			dSI = m_pHM->GetpCV()->GetKvsmax();
			CString str;

			if( 0.0 == dSI )
			{
				// TO VERIFY : +m_Box.CVKV_Kvsmax or + m_Box.CVKV_Dp
				SetCellText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Kvsmax, _T("") );
			}
			else
			{
				str = WriteCUDouble( _C_KVCVCOEFF, dSI );
				SetCellText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Kvsmax, str );
			}

			// Authority.
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
			dSI = m_pHM->GetpCV()->GetAuth();

			if( 0.0 == dSI )
			{
				SetCellText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Auth, _T("") );
			}
			else
			{
				str = WriteCUDouble( _U_NODIM, dSI );

				if( eb3False == m_pHM->GetpCV()->CheckMinAuthor() )
				{
					SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Auth, _RED );
				}
				else
				{
					SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Auth, _BLACK );
				}

				SetCellText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVKV_Auth, str );
			}
		}

		// Refresh LockPicture.
		// 		COLORREF BackgroundColor = 0;
		// 		BackgroundColor = GetBackColor(  m_BoxCv.pos.x + eBCol::Desc,  m_BoxCv.pos.y);
		SetPictureLock( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) );
		TASApp.OverridedSetRedraw( this, TRUE );
		Invalidate();
	}
	else
	{
		ChangeCvConnect();
	}
}

void CSSheetPanelCirc2::ChangeCvConnect()
{
	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxCv.bExt )
	{
		ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpCV()->GetpSelCVConn() )
		{
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpCV()->GetpSelCVConn()->GetString() ) );
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn, (LPARAM)( m_pHM->GetpCV()->GetpSelCVConn()->GetIDPtr().MP ) );
		}
	}

	ChangeCvVersion();
}

void CSSheetPanelCirc2::ChangeCvVersion()
{
	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxCv.bExt )
	{
		ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpCV()->GetpSelCVVers() )
		{
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpCV()->GetpSelCVVers()->GetString() ) );
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers, (LPARAM)( m_pHM->GetpCV()->GetpSelCVVers()->GetIDPtr().MP ) );
		}
	}

	ChangeCvPN();
}

void CSSheetPanelCirc2::ChangeCvPN()
{
	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxCv.bExt )
	{
		ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpCV()->GetpSelCVPN() )
		{
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpCV()->GetpSelCVPN()->GetString() ) );
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN, (LPARAM)( m_pHM->GetpCV()->GetpSelCVPN()->GetIDPtr().MP ) );
		}
	}

	ChangeCv();
}

void CSSheetPanelCirc2::ChangeCv()
{
	TASApp.OverridedSetRedraw( this, FALSE );

	ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, SS_CBM_RESETCONTENT, 0, 0 );
	ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, SS_CBM_SETCURSEL, 0, 0 );

	if( true == m_pHM->IsCvExist( true ) )
	{
		CDB_RegulatingValve *pclControlValve = (CDB_RegulatingValve *)( m_pHM->GetpCV()->GetCvIDPtr().MP );

		// Update valve.
		// HYS-1221 : If the selected product is deleted display it in red color.
		if( true == pclControlValve->IsDeleted() )
		{
			SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, _RED );
		}
		else
		{
			SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, _BLACK );
		}

		if( false == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
		{
			SetFontBold( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, false );
		}
		else
		{
			SetFontBold( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, true );
		}

		CString strValveName = pclControlValve->GetName();

		if( eb3True == m_pHM->GetpCV()->GetCVSelectedAsaPackage() )
		{
			strValveName += TASApp.LoadLocalizedString( IDS_SSHEETSELPROD_SET );
		}

		ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, SS_CBM_ADDSTRING, 0, (LPARAM)( strValveName.GetBuffer() ) );

		ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_Box.CVTA_Name, (LPARAM)( m_pHM->GetpCV()->GetCvIDPtr().MP ) );

		// Update setting and Dp.
		double dSI = m_pHM->GetpCV()->GetDp();
		SetCUDouble( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rDp, _U_DIFFPRESS, dSI );

		if( eb3False == m_pHM->GetpCV()->CheckDpMinMax() )
		{
			SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rDp, _RED );
		}
		else
		{
			SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rDp, _BLACK );
		}

		CString str;

		if( 0 != m_BoxCv.rPreSet )
		{
			str = m_pHM->GetpCV()->GetSettingStr( true );
			SetCellText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rPreSet, str );

			if( eb3False == m_pHM->GetpCV()->CheckMinOpen() )
			{
				SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rPreSet, _RED );
			}
			else
			{
				SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rPreSet, _BLACK );
			}
		}

		// Authority exist only for proportional valve.
		str = _T("");

		if( 0 != m_BoxCv.rAuth )
		{
			SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			FormatCUDouble( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rAuth, _U_NODIM, m_pHM->GetpCV()->GetAuth(), SSS_ALIGN_RIGHT );

			if( eb3False == m_pHM->GetpCV()->CheckMinAuthor() )
			{
				SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rAuth, _RED );
			}
			else
			{
				SetForeColor( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rAuth, _BLACK );
			}
		}

		// Actuator Exist
		if( NULL != m_pHM->GetpCV()->GetActrIDPtr().MP )
		{
			LPARAM param = 0;
			CString str;
			param = (LPARAM) m_pHM->GetpCV()->GetActrIDPtr().MP;
			str = ( ( CDB_Actuator * )( m_pHM->GetpCV()->GetActrIDPtr().MP ) )->GetName();
			_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator );
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, SS_CBM_RESETCONTENT, 0, 0 );
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, SS_CBM_SETCURSEL, 0, 0 );

			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR ) str );
			ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, SS_CBM_SETCURSEL, 0, 0 );

			SetCellParam( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, param );
		}
		else
		{
			//Is there any possibility to have an actuator?
			CDB_ControlValve *pDBCV = dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP );
			str = TASApp.LoadLocalizedString( IDS_NOACTUATOR );

			// No possibility to add an actuator, no actuator group idptr
			if( NULL == pDBCV->GetActuatorGroupIDPtr().MP )
			{
				// Set None as a static text
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
				FormatStaticText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator,  m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, str );
			}
			else
			{
				LPARAM param = 0;
				_FormatComboList( m_BoxCv.m_eBoxType, m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator );
				ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, SS_CBM_RESETCONTENT, 0, 0 );
				ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, SS_CBM_SETCURSEL, 0, 0 );

				ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR ) str );
				ComboBoxSendMessage( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, SS_CBM_SETCURSEL, 0, 0 );

				SetCellParam( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, param );
			}
		}

		// If control valve is locked, we consider that actuator is also automatically locked.
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
		{
			SetFontBold( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, TRUE );
		}
		else
		{
			SetFontBold( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator, FALSE );
		}


		// Refresh LockPicture.
		// 		COLORREF BackgroundColor = 0;
		// 		BackgroundColor = GetBackColor( m_BoxCv.pos.x + eBCol::Desc, m_BoxCv.pos.y);
		SetPictureLock( m_BoxCv.pos.x + eBCol::Lock, m_BoxCv.pos.y, m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) );
	}
	else
	{
		// Clear setting and Dp.
		SetCUDouble( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rDp, _U_DIFFPRESS, 0.0 );
		SetCellText( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rPreSet, _T("") );

		if( 0 != m_BoxCv.rAuth )
		{
			SetCUDouble( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rAuth, _U_NODIM, 0.0 );
		}
	}

	// Change picture.
	m_pHM->ResetSchemeIDPtr();
	FormatBoxPicture();
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
	RefreshDispatch( _BoxType::ebCV );
}

void CSSheetPanelCirc2::FormatBoxRemark()
{
	if( m_BoxRem.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	_ClearRange( m_BoxRem.m_eBoxType, m_BoxRem.pos.x + eBCol::Lock, m_BoxRem.pos.y, m_BoxRem.pos.x + eBCol::Unit, m_BoxRem.pos.y +  m_Box.Rem_nRows - 1 );

	FormatBox( m_BoxRem.pos.x + eBCol::Lock, m_BoxRem.pos.y, m_Box.Rem_nRows, IDS_PANELCIRC2_REMTITLE, _TAH_BLACK_LIGHT, false, 4 );

	AddCellSpanW( m_BoxRem.pos.x + eBCol::Lock, m_BoxRem.pos.y + m_Box.Rem_l1, 4, 2 );
	SetCellProperty( m_BoxRem.pos.x + eBCol::Lock, m_BoxRem.pos.y + m_Box.Rem_l1, m_BoxRem.pos.x + eBCol::Lock + 3, m_BoxRem.pos.y + m_Box.Rem_l1 + 1,
					 CSSheet::_SSCellProperty::CellNoSelection, true, true );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatEditText( m_BoxRem.pos.x + eBCol::Lock, m_BoxRem.pos.y + m_Box.Rem_l1, _T(""), SSS_ALIGN_LEFT | ES_MULTILINE, 256 );
}

void CSSheetPanelCirc2::ChangeRemark( InputValuesOrigin eInputValuesOrigin )
{
	// User has input values, we must read them and save them in the current hydraulic circuit 'CDS_HydroMod' object.
	if( InputValuesOrigin::InputValuesComeFromUser == eInputValuesOrigin )
	{
		// Retrieve text.
		CString str = GetCellText( m_BoxRem.pos.x, m_BoxRem.pos.y + m_Box.Rem_l1 );
		m_pHM->SetRemL1( str );
	}

	SetCellText( m_BoxRem.pos.x, m_BoxRem.pos.y + m_Box.Rem_l1, ( LPCTSTR )( m_pHM->GetRemL1() ) );
}

void CSSheetPanelCirc2::FormatBoxPrimaryValues()
{
	if( m_BoxPrimaryValues.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	_ClearRange( m_BoxPrimaryValues.m_eBoxType, m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y, m_BoxPrimaryValues.pos.x + eBCol::Unit, m_BoxPrimaryValues.pos.y +  m_Box.TU_nRows - 1 );

	FormatBox( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y, m_Box.PrimaryValues_nRows, IDS_PANELCIRC2_PRIMARYVALUESTITLE );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Flow, 2, 1 );
	AddCellSpanW( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Hmin, 2, 1 );
	AddCellSpanW( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_AvailH, 2, 1 );
	AddCellSpanW( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Tsp, 2, 1 );
	AddCellSpanW( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Trp, 2, 1 );

	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Flow, IDS_PANELCIRC2_PRIMARYVALUES_FLOW );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Hmin, IDS_PANELCIRC2_PRIMARYVALUES_HMIN );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_AvailH, IDS_PANELCIRC2_PRIMARYVALUES_AVAILABLEH );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Tsp, IDS_PANELCIRC2_PRIMARYVALUES_SUPPLYTEMPERATURE );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Lock, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Trp, IDS_PANELCIRC2_PRIMARYVALUES_RETURNTEMPERATURE );

	// values.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Flow, _T("") );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Hmin, _T("") );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_AvailH, _T("") );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Tsp, _T("") );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Trp, _T("") );

	// Unit fields.
	TCHAR unitname[_MAXCHARS];
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Unit, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Flow, unitname );

	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Unit, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Hmin, unitname );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Unit, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_AvailH, unitname );

	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Unit, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Tsp, unitname );
	SetStaticText( m_BoxPrimaryValues.pos.x + eBCol::Unit, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Trp, unitname );
}

void CSSheetPanelCirc2::RefreshBoxPrimaryValues()
{
	if( m_BoxPrimaryValues.pos != CPoint( 0, 0 ) )
	{
		SetCUDouble( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y +  m_Box.PrimaryValues_Flow, _U_FLOW, m_pHM->GetQ() );
		SetCUDouble( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y +  m_Box.PrimaryValues_Hmin, _U_DIFFPRESS, m_pHM->GetHminForUserDisplay() );
		SetCUDouble( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_AvailH, _U_DIFFPRESS, m_pHM->GetHAvail() );

		double dCircuitSupplyPrimary = m_pHM->GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Supply, CAnchorPt::CircuitSide_Primary );
		SetCUDouble( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Tsp, _U_TEMPERATURE, dCircuitSupplyPrimary );

		double dCircuitReturnPrimary = m_pHM->GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Primary );
		SetCUDouble( m_BoxPrimaryValues.pos.x + eBCol::Val, m_BoxPrimaryValues.pos.y + m_Box.PrimaryValues_Trp, _U_TEMPERATURE, dCircuitReturnPrimary );
	}

	// OpenDlgSelectActr button ADD if HydroMod is completely defined or when the user add a circuit without Terminal Unit.
	// SSheetPanelCirc2 can be called by RightViewProj for print and print preview, in this case "Next button" doesn't exist.
	CRuntimeClass *pRC = this->GetParent()->GetRuntimeClass();
	CString str = (CString)pRC->m_lpszClassName;

	if( _T("CDialogEx") == str || _T( "CPanelCirc2") == str )
	{
		if( true == m_pHM->IsCompletelyDefined() || true == m_pHM->IsPending() )
		{
			( (CPanelCirc2 *)( this->GetParent() ) )->EnableButtonNext( true );
		}
		else
		{
			if( false == m_pHM->GetpTermUnit()->IsVirtual() )		//Circuit
			{
				( (CPanelCirc2 *)( this->GetParent() ) )->EnableButtonNext( false );
			}
			else													//Module
			{
				if( m_pHM->GetpSchcat() != NULL && false == m_pHM->GetpSchcat()->IsSecondarySideExist() )
				{
					( (CPanelCirc2 *)( this->GetParent() ) )->EnableButtonNext( true );
				}
				else												//Injection circuit
				{
					( (CPanelCirc2 *)( this->GetParent() ) )->EnableButtonNext( ( (CDS_HmInj *)m_pHM )->CheckIfTempAreValid() );
				}
			}
		}
	}
}

void CSSheetPanelCirc2::FormatBoxDpC()
{
	if( m_BoxDpC.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	_ClearRange( m_BoxDpC.m_eBoxType, m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y, m_BoxDpC.pos.x + eBCol::Unit, m_BoxDpC.pos.y +  m_Box.DpC_nRows - 1 );

	// Lock and erase all DpC cells.
	SetCellProperty( m_BoxDpC.pos.x + eBCol::Butt, m_BoxDpC.pos.y, m_BoxDpC.pos.x + eBCol::Unit, m_BoxDpC.pos.y + m_Box.DpC_nRows - 1, CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetCellBorder( m_BoxDpC.pos.x + eBCol::Butt, m_BoxDpC.pos.y, m_BoxDpC.pos.x + eBCol::Unit, m_BoxDpC.pos.y + m_Box.DpC_nRows - 1, false );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	FormatStaticText( m_BoxDpC.pos.x + eBCol::Butt, m_BoxDpC.pos.y, m_BoxDpC.pos.x + eBCol::Unit, m_BoxDpC.pos.y + m_Box.DpC_nRows - 1, _T("") );

	int iRows = m_Box.DpC_DpL + ( ( true == m_BoxDpC.bExt ) ? m_Box.DpC_PN : 0 );
	COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::DpC );
	FormatBox( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y, iRows + 1, IDS_PANELCIRC2_DPCTITLE, BackGroundColor, true );

	// Draw button cells in front of name.
	if( true == m_BoxDpC.bExt )
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SMINUS, m_BoxDpC.pos.x + eBCol::Butt, m_BoxDpC.pos.y + m_Box.DpC_Name, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxDpC.pos.y + m_Box.DpC_PN;
	}
	else
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SPLUS, m_BoxDpC.pos.x + eBCol::Butt, m_BoxDpC.pos.y + m_Box.DpC_Name, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxDpC.pos.y;
	}

	// Activate mouse left button event for this minus/plus button.
	SetCellProperty( m_BoxDpC.pos.x + eBCol::Butt, m_BoxDpC.pos.y + m_Box.DpC_Name, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Cell span for BVP Name, Connection, Version and PN.
	AddCellSpanW( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, 2, 1 );

	if( true == m_BoxDpC.bExt )
	{
		AddCellSpanW( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type, 2, 1 );
		AddCellSpanW( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect, 2, 1 );
		AddCellSpanW( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN, 2, 1 );
	}
	else
	{
		RemoveCellSpan( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type );
		RemoveCellSpan( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect );
		RemoveCellSpan( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN );
	}

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y + m_Box.DpC_Name, 2, 1 );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y + m_Box.DpC_Name, IDS_PANELCIRC2_DPCNAME );

	if( true == m_BoxDpC.bExt )
	{
		AddCellSpanW( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type, 2, 1 );
		SetStaticText( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type, IDS_PANELCIRC2_DPCTYPE );
		AddCellSpanW( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect, 2, 1 );
		SetStaticText( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect, IDS_PANELCIRC2_DPCCONNECT );
		AddCellSpanW( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN, 2, 1 );
		SetStaticText( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN, IDS_PANELCIRC2_DPCPN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxDpC.pos.x + eBCol::Lock, iRows + m_Box.DpC_Setting, 2, 1 );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Lock, iRows + m_Box.DpC_Setting, IDS_PANELCIRC2_DPCSETTING );
	AddCellSpanW( m_BoxDpC.pos.x + eBCol::Lock, iRows + m_Box.DpC_Dp, 2, 1 );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Lock, iRows + m_Box.DpC_Dp, IDS_PANELCIRC2_DPCDP );
	AddCellSpanW( m_BoxDpC.pos.x + eBCol::Lock, iRows + m_Box.DpC_DpL, 2, 1 );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Lock, iRows + m_Box.DpC_DpL, IDS_PANELCIRC2_DPCDPL );

	// Combo and Value fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( m_BoxDpC.m_eBoxType, m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name );

	if( true == m_BoxDpC.bExt )
	{
		_FormatComboList( m_BoxDpC.m_eBoxType, m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type );
		_FormatComboList( m_BoxDpC.m_eBoxType, m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect );
		_FormatComboList( m_BoxDpC.m_eBoxType, m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Val, iRows + m_Box.DpC_Setting, _T("") );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Val, iRows + m_Box.DpC_Dp, _T("") );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Val, iRows + m_Box.DpC_DpL, _T("") );

	// Unit fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Unit, iRows + m_Box.DpC_Setting, IDS_TURNS );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Unit, iRows + m_Box.DpC_Dp, unitname );
	SetStaticText( m_BoxDpC.pos.x + eBCol::Unit, iRows + m_Box.DpC_DpL, unitname );
	ChangeDpCType();
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

void CSSheetPanelCirc2::ChangeDpCType()
{
	if( false == m_pHM->IsDpCExist() )
	{
		return;
	}

	if( m_BoxDpC.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxDpC.bExt )
	{
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpDpC()->GetpSelDpCType() )
		{
			ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpDpC()->GetpSelDpCType()->GetString() ) );
			ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type, (LPARAM)( m_pHM->GetpDpC()->GetpSelDpCType()->GetIDPtr().MP ) );
		}
	}

	ChangeDpCConnect();
}

void CSSheetPanelCirc2::ChangeDpCConnect()
{
	if( false == m_pHM->IsDpCExist() )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxDpC.bExt )
	{
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpDpC()->GetpSelDpCConn() )
		{
			ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpDpC()->GetpSelDpCConn()->GetString() ) );
			ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect,
						  (LPARAM)( m_pHM->GetpDpC()->GetpSelDpCConn()->GetIDPtr().MP ) );
		}
	}

	ChangeDpCPN();
}

void CSSheetPanelCirc2::ChangeDpCPN()
{
	if( false == m_pHM->IsDpCExist() )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxDpC.bExt )
	{
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpDpC()->GetpSelDpCPN() )
		{
			ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpDpC()->GetpSelDpCPN()->GetString() ) );
			ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN, (LPARAM)( m_pHM->GetpDpC()->GetpSelDpCPN()->GetIDPtr().MP ) );
		}
	}

	ChangeDpC();
}

void CSSheetPanelCirc2::ChangeDpC()
{
	if( false == m_pHM->IsDpCExist() )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, SS_CBM_RESETCONTENT, 0, 0 );
	ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, SS_CBM_SETCURSEL, 0, 0 );

	// HYS-1221 : If the selected product is deleted display it in red color.
	if( (NULL != (CDB_DpController *)( m_pHM->GetpDpC()->GetIDPtr().MP ) ) 
			&&  true == m_pHM->GetpDpC()->GetIDPtr().MP->IsDeleted() )
	{
		SetForeColor( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, _RED );
	}
	else
	{
		SetFontBold( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, _BLACK );
	}
	
	if( false == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eDpC ) )
	{
		SetFontBold( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, false );
	}
	else
	{
		SetFontBold( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, true );
	}

	if( ( ( CDB_DpController * )( m_pHM->GetpDpC()->GetIDPtr().MP ) ) )
	{
		CDB_DpController *pDpC = ( CDB_DpController * )( m_pHM->GetpDpC()->GetIDPtr().MP );

		std::wstring str;
		pDpC->GetNameEx( &str, false );

		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, SS_CBM_ADDSTRING, 0, (LPARAM)( str.c_str() ) );
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, (LPARAM)( m_pHM->GetpDpC()->GetIDPtr().MP ) );

		int iRow = 0;

		if( true == m_BoxDpC.bExt )
		{
			iRow = m_Box.DpC_PN;
		}

		CString strSetting = _T("-");

		if( NULL != pDpC->GetDpCCharacteristic() )
		{
			strSetting = pDpC->GetDpCCharacteristic()->GetSettingString( m_pHM->GetpDpC()->GetDpCSetting() );
		}

		SetCellText( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + iRow + m_Box.DpC_Setting, strSetting );

		if( eb3False == m_pHM->GetpDpC()->CheckDpLRange() )
		{
			SetForeColor( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + iRow + m_Box.DpC_DpL, _RED );
		}
		else
		{
			SetForeColor( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + iRow + m_Box.DpC_DpL, _BLACK );
		}

		double dSI = m_pHM->GetpDpC()->GetDp();
		SetCUDouble( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + iRow + m_Box.DpC_Dp, _U_DIFFPRESS, dSI );

		dSI = m_pHM->GetpDpC()->GetDpL();
		SetCUDouble( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + iRow + m_Box.DpC_DpL, _U_DIFFPRESS, dSI );

		if( eb3False == m_pHM->GetpDpC()->CheckDpMinMax() )
		{
			SetForeColor( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + iRow + m_Box.DpC_Dp, _RED );
		}
		else
		{
			SetForeColor( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + iRow + m_Box.DpC_Dp, _BLACK );
		}

		// Refresh LockPicture.
		// 		COLORREF BackgroundColor = 0;
		// 		BackgroundColor = GetBackColor( m_BoxDpC.pos.x + eBCol::Desc, m_BoxDpC.pos.y);
		SetPictureLock( m_BoxDpC.pos.x + eBCol::Lock, m_BoxDpC.pos.y, m_pHM->IsLocked( CDS_HydroMod::eHMObj::eDpC ) );
	}
	else
	{
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, SS_CBM_ADDSTRING, 0, (LPARAM)( _T(" ") ) );
		ComboBoxSendMessage( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxDpC.pos.x + eBCol::Val, m_BoxDpC.pos.y + m_Box.DpC_Name, 0 );
	}

	// Change picture.
	m_pHM->ResetSchemeIDPtr();
	FormatBoxPicture();
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
	RefreshDispatch( _BoxType::ebDPC );
}

void CSSheetPanelCirc2::FormatBoxPicture()
{
	if( m_BoxPic.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( false == m_bRefreshBoxPicture )
	{
		return;
	}

	// Draw picture.
	AddCellSpanW( m_BoxPic.pos.x, m_BoxPic.pos.y, m_Box.Pic_nCols, m_Box.Pic_nRows );

	// Define the type of cell as a picture.
	CDB_CircuitScheme *pSch = dynamic_cast<CDB_CircuitScheme *>( m_pHM->GetSchemeIDPtr().MP );
	ASSERT( pSch );

	if( NULL == pSch )
	{
		return;
	}

	// Create the CEnBitmap.
	CEnBitmapPatchWork EnBmp;
	EnBmp.GetHydronicScheme( m_pHM );

	EnBmp.ResizeImage( 0.4 );
	
	// Put the picture in the Spread cell.
	SetPictureCellWithHandle( EnBmp, m_BoxPic.pos.x, m_BoxPic.pos.y, true, VPS_BMP | VPS_CENTER );
	EnBmp.DeleteObject();
}

void CSSheetPanelCirc2::FormatBoxPump()
{
	if( m_BoxPump.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		m_BoxPump.nRows = 5;
		m_BoxPump.iHAvailable = 1;
		m_BoxPump.iHmin = 2;
		m_BoxPump.iUserHead = 3;
		m_BoxPump.iFlow = 4;
	}
	else
	{
		m_BoxPump.nRows = 4;
		m_BoxPump.iHAvailable = -1;
		m_BoxPump.iHmin = 1;
		m_BoxPump.iUserHead = 2;
		m_BoxPump.iFlow = 3;
	}

	_ClearRange( m_BoxPump.m_eBoxType, m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y, m_BoxPump.pos.x + eBCol::Unit, m_BoxPump.pos.y +  m_BoxPump.nRows - 1 );

	// When we are localized on the secondary side change the box title color
	COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::PUMP );
	FormatBox( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y, m_BoxPump.nRows, IDS_PANELCIRC2_PUMPTITLE, BackGroundColor );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	
	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		AddCellSpanW( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iHAvailable, 2, 1 );
	}

	AddCellSpanW( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iHmin, 2, 1 );
	AddCellSpanW( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iUserHead, 2, 1 );
	AddCellSpanW( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iFlow, 2, 1 );
	
	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		SetStaticText( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iHAvailable, IDS_PANELCIRC2_PUMPHAVAILABLE );
	}

	SetStaticText( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iHmin, IDS_PANELCIRC2_PUMPHMIN );

	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		// Check if there is a warning message.
		int iValidity = m_pHM->CheckValidity();

		if( ( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam == ( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam & iValidity ) )
				|| ( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded == ( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded & iValidity ) ) )
		{
			SetForeColor( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iHmin, _TAH_ORANGE );
		}
	}

	SetStaticText( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iUserHead, IDS_PANELCIRC2_PUMPHEAD );
	SetStaticText( m_BoxPump.pos.x + eBCol::Lock, m_BoxPump.pos.y + m_BoxPump.iFlow, IDS_PANELCIRC2_TUFLOW );

	// Values.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	
	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		SetStaticText( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iHAvailable, _T("") );
	}

	SetStaticText( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iHmin, _T("") );
	SetStaticText( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iFlow, _T("") );
	FormatEditDouble( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iUserHead );

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );

	// Unit fields.
	TCHAR unDp[_MAXCHARS];
	TCHAR unQ[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unDp );
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_FLOW ), unQ );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	
	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		SetStaticText( m_BoxPump.pos.x + eBCol::Unit, m_BoxPump.pos.y + m_BoxPump.iHAvailable, unDp );
	}

	SetStaticText( m_BoxPump.pos.x + eBCol::Unit, m_BoxPump.pos.y + m_BoxPump.iHmin, unDp );

	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		// Check if there is a warning message.
		int iValidity = m_pHM->CheckValidity();

		if( ( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam == ( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam & iValidity ) )
				|| ( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded == ( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded & iValidity ) ) )
		{
			SetForeColor( m_BoxPump.pos.x + eBCol::Unit, m_BoxPump.pos.y + m_BoxPump.iHmin, _TAH_ORANGE );
		}
	}

	SetStaticText( m_BoxPump.pos.x + eBCol::Unit, m_BoxPump.pos.y +  m_BoxPump.iFlow, unQ );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
	SetStaticText( m_BoxPump.pos.x + eBCol::Unit, m_BoxPump.pos.y + m_BoxPump.iUserHead, unDp );
}

void CSSheetPanelCirc2::RefreshBoxPump( InputValuesOrigin eInputValuesOrigin )
{
	if( NULL == m_pHM->GetpPump() )
	{
		return;
	}

	double dSI = 0.0;

	// User has input values, we must read them and save them in the current hydraulic circuit 'CDS_HydroMod' object.
	if( InputValuesOrigin::InputValuesComeFromUser == eInputValuesOrigin )
	{
		if( m_lRow == ( m_BoxPump.pos.y + m_BoxPump.iUserHead ) && m_lCol == ( m_BoxPump.pos.x + eBCol::Val ) )
		{
			dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );

			// if( dSI >= m_pHM->GetpPump()->GetHmin() )
			// {
				m_pHM->GetpPump()->SetHpump( dSI, false );
			// }
		}
	}

	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		// H available is Hmin + Hbypass.
		double dHAvailable = 0.0;

		if( NULL != m_pHM->GetpBypBv() && m_pHM->GetpBypBv()->GetDp() > 0.0 )
		{
			dHAvailable = m_pHM->GetpPump()->GetHAvail() + m_pHM->GetpBypBv()->GetDp();
		}

		SetCUDouble( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iHAvailable, _U_DIFFPRESS, dHAvailable );
	}

	SetCUDouble( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iHmin, _U_DIFFPRESS, m_pHM->GetpPump()->GetHmin() );

	if( eDpStab::DpStabOnBVBypass == m_pHM->GetpSch()->GetDpStab() )
	{
		// HYS-1930: auto-adapting variable flow decoupling circuit.
		// Check if there is a warning message.
		int iValidity = m_pHM->CheckValidity();

		if( ( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam == ( CDS_HydroMod::eValidityFlags::evfSecPumpHMinTechParam & iValidity ) )
				|| ( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded == ( CDS_HydroMod::eValidityFlags::evfSecPumpNotNeeded & iValidity ) ) )
		{
			SetForeColor( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iHmin, _TAH_ORANGE );
		}
	}

	SetCUDouble( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iUserHead, _U_DIFFPRESS, m_pHM->GetpPump()->GetHpump() );
	SetCUDouble( m_BoxPump.pos.x + eBCol::Val, m_BoxPump.pos.y + m_BoxPump.iFlow, _U_FLOW, m_pHM->GetpTermUnit()->GetQ() );
}

void CSSheetPanelCirc2::FormatBoxSecondaryValues()
{
	if( m_BoxSecondaryValues.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	// HYS-1882: Remove the design secondary return temperature field.
	m_BoxSecondaryValues.nRows = 3;
	m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature = 1;
	m_BoxSecondaryValues.m_iCompoutedSecondaryReturnTemperature = 2;

	_ClearRange( m_BoxSecondaryValues.m_eBoxType, m_BoxSecondaryValues.pos.x + eBCol::Lock, m_BoxSecondaryValues.pos.y, m_BoxSecondaryValues.pos.x + eBCol::Unit, m_BoxSecondaryValues.pos.y +  m_BoxSecondaryValues.nRows - 1 );

	// When we are localized on the secondary side change the box title color
	COLORREF BackGroundColor = _TAH_ORANGE;

	if( NULL != m_pHM->GetpSchcat() && true == m_pHM->GetpSchcat()->IsSecondarySideExist() && CDB_CircSchemeCateg::e3wTypeMixing != m_pHM->GetpSchcat()->Get3WType() )
	{
		if( true == m_pHM->IsaModule() )
		{
			BackGroundColor = _IMI_GRAY_XLIGHT;
		}
		else
		{
			BackGroundColor = _TAH_GREEN_MED;
		}
	}

	FormatBox( m_BoxSecondaryValues.pos.x + eBCol::Lock, m_BoxSecondaryValues.pos.y, m_BoxSecondaryValues.nRows, IDS_PANELCIRC2_SECONDARYVALUESTITLE, BackGroundColor );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxSecondaryValues.pos.x + eBCol::Lock, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature, 2, 1 );
	AddCellSpanW( m_BoxSecondaryValues.pos.x + eBCol::Lock, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iCompoutedSecondaryReturnTemperature, 2, 1 );

	SetStaticText( m_BoxSecondaryValues.pos.x + eBCol::Lock, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature, IDS_PANELCIRC2_SECONDARYVALUES_SUPPLYTEMP );
	SetStaticText( m_BoxSecondaryValues.pos.x + eBCol::Lock, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iCompoutedSecondaryReturnTemperature, IDS_PANELCIRC2_SECONDARYVALUES_COMPUTEDRETURNTEMP );

	// Values.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	FormatEditDouble( m_BoxSecondaryValues.pos.x + eBCol::Val, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature );
	FormatStaticText( m_BoxSecondaryValues.pos.x + eBCol::Val, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iCompoutedSecondaryReturnTemperature, m_BoxSecondaryValues.pos.x + eBCol::Val, 
			m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iCompoutedSecondaryReturnTemperature, _T(""), (LPARAM)SSS_ALIGN_RIGHT );

	// Unit fields.
	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), unitname );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
	SetStaticText( m_BoxSecondaryValues.pos.x + eBCol::Unit, m_BoxSecondaryValues.pos.y +  m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature, unitname );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	SetStaticText( m_BoxSecondaryValues.pos.x + eBCol::Unit, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iCompoutedSecondaryReturnTemperature, unitname );
}

void CSSheetPanelCirc2::RefreshBoxSecondaryValues( InputValuesOrigin eInputValuesOrigin )
{
	if( NULL == m_pHM->GetpPump() )
	{
		return;
	}

	CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( m_pHM );

	if( NULL == pHMInj )
	{
		return;
	}

	// User has input values, we must read them and save them in the current hydraulic circuit 'CDS_HydroMod' object.
	if( InputValuesOrigin::InputValuesComeFromUser == eInputValuesOrigin )
	{
		if( m_lRow == ( m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature ) && m_lCol == ( m_BoxSecondaryValues.pos.x + eBCol::Val ) )
		{
			double dNewDesignCircuitSupplySecondaryTemperature = CDimValue::CUtoSI( _U_TEMPERATURE, m_dCellDouble );

			// HYS-1716: we must verify if there is at least one injection hydraulic circuit that have its design temperatures in error
			// with this user change.
			std::multimap<CDS_HydroMod *, CTable *> mmapInjectionHydraulicCircuitWithTemperatureError;
			std::vector<CDS_HydroMod *> vecAllInjectionCircuitsWithTemperatureError;
			CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() );
			ASSERT( NULL != pclHydraulicNetwork );

			CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode = CDlgInjectionError::ReturnCode::Undefined;
			bool bSaveValue = true;

			bool bIsInjectionCircuitTemperatureError = pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForEditOperation( m_pHM, dNewDesignCircuitSupplySecondaryTemperature, 
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
				if( NULL != m_pTechParam && ProjectType::Heating == m_pTechParam->GetProjectApplicationType()  
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
	}

	if( pHMInj->GetDesignCircuitSupplySecondaryTemperature() > -273.15 )
	{
		SetCUDouble( m_BoxSecondaryValues.pos.x + eBCol::Val, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature, _U_TEMPERATURE, pHMInj->GetDesignCircuitSupplySecondaryTemperature() );
	}

	// HYS-1882: Update the secondary computed return temperature. 
	double dDT = 0.0;
	bool bIsDTDisplayedInFlowMode = m_pHM->IsDTFieldDisplayedInFlowMode();

	if( true == bIsDTDisplayedInFlowMode )
	{
		dDT = m_pHM->GetpTermUnit()->GetDTFlowMode();
	}
	else if( CTermUnit::_QType::PdT == m_pHM->GetpTermUnit()->GetQType() )
	{
		dDT = m_pHM->GetpTermUnit()->GetDT();
	}

	if( dDT > 0.0 )
	{
		double dSupplyTemp = m_pHM->GetTemperature( CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Supply, CAnchorPt::CircuitSide::CircuitSide_Secondary );
		double dReturnTemp = ( ProjectType::Cooling == m_pTechParam->GetProjectApplicationType() ) ? dSupplyTemp + dDT : dSupplyTemp - dDT;

		pHMInj->SetDesignCircuitReturnSecondaryTemperature( dReturnTemp );

		// Refresh primary return temp. it was refreshed from the secondary. 
		m_pHM->SetTemperature( dReturnTemp, CAnchorPt::PipeType::PipeType_Circuit, CAnchorPt::PipeLocation::PipeLocation_Return, CAnchorPt::CircuitSide::CircuitSide_Primary );
	}

	double dComputedReturnTemperature = pHMInj->GetTemperature( CAnchorPt::PipeType_Circuit, CAnchorPt::PipeLocation_Return, CAnchorPt::CircuitSide_Secondary );
	
	if( dComputedReturnTemperature > -273.15 )
	{
		SetCUDouble( m_BoxSecondaryValues.pos.x + eBCol::Val, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iCompoutedSecondaryReturnTemperature, _U_TEMPERATURE, dComputedReturnTemperature );
	}

	if( false == pHMInj->CheckIfTempAreValid() )
	{
		SetForeColor( m_BoxSecondaryValues.pos.x + eBCol::Val, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature, _RED );
	}
	else
	{
		SetForeColor( m_BoxSecondaryValues.pos.x + eBCol::Val, m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature, _BLACK );
	}
}

void CSSheetPanelCirc2::FormatBoxSV()
{
	if( m_BoxShutoffValve.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	_ClearRange( m_BoxShutoffValve.m_eBoxType, m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y, m_BoxShutoffValve.pos.x + eBCol::Unit, m_BoxShutoffValve.pos.y +  m_BoxShutoffValve.nRows - 1 );

	// Lock and erase all shut-off valve cells.
	SetCellProperty( m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y, m_BoxShutoffValve.pos.x + eBCol::Unit, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.nRows - 1,
					 CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetCellBorder( m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y, m_BoxShutoffValve.pos.x + eBCol::Unit, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.nRows - 1, false );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	FormatStaticText( m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y, m_BoxShutoffValve.pos.x + eBCol::Unit, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.nRows - 1, _T("") );

	CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;
			
	if( NULL != m_pHM->GetpSch() )
	{
		if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
		{
			eHMObj = CDS_HydroMod::eShutoffValveSupply;
		}
		else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
		{
			eHMObj = CDS_HydroMod::eShutoffValveReturn;
		}
	}

	if( CDS_HydroMod::eNone == eHMObj )
	{
		ASSERT_RETURN;
	}

	CDS_HydroMod::CShutoffValve *pclMHShutoffValve = m_pHM->GetpShutoffValve( eHMObj );
	CDS_Hm2W *pHm2W = dynamic_cast<CDS_Hm2W*>( m_pHM );
	int iRows = 1;

	if( NULL != pHm2W && CDS_HydroMod::eUseProduct::Always == pHm2W->GetUseShutoffValve() )
	{
		iRows = m_BoxShutoffValve.SV_Dp + ( ( true == m_BoxShutoffValve.bExt ) ? m_BoxShutoffValve.SV_PN : 0 );
	}
		
	COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::ShutoffValve );
	FormatBox( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y, iRows + 1, IDS_PANELCIRC2_SVTITLE, BackGroundColor, true );

	if( true == m_pHM->GetpSch()->IsAnchorPtOptional( CAnchorPt::eFunc::ShutoffValve ) && NULL != pHm2W )
	{
		_DrawCheckbox( m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y, pHm2W->GetUseShutoffValve() );

		// Activate mouse left button event for this button.
		SetCellProperty( m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y, CSSheet::_SSCellProperty::CellCantLeftClick, false );
	}

	if( NULL != pHm2W && CDS_HydroMod::eUseProduct::Never == pHm2W->GetUseShutoffValve() )
	{
		// Don't go further is user doesn't want shutoff valve.
		TASApp.OverridedSetRedraw( this, TRUE );
		Invalidate();
		return;
	}

	// Draw button cells in front of name.
	if( true == m_BoxShutoffValve.bExt )
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SMINUS, m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_PN;
	}
	else
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SPLUS, m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxShutoffValve.pos.y;
	}

	// Activate mouse left button event for this minus/plus button.
	SetCellProperty( m_BoxShutoffValve.pos.x + eBCol::Butt, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Cell span for ShutoffValve Name, Connection, Version and PN.
	AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, 2, 1 );

	if( true == m_BoxShutoffValve.bExt )
	{
		AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type, 2, 1 );
		AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect, 2, 1 );
		AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version, 2, 1 );
		AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN, 2, 1 );
	}
	else
	{
		RemoveCellSpan( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type );
		RemoveCellSpan( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect );
		RemoveCellSpan( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version );
		RemoveCellSpan( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN );
	}

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, 2, 1 );
	SetStaticText( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, IDS_PANELCIRC2_SVNAME );

	if( true == m_BoxShutoffValve.bExt )
	{
		AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type, 2, 1 );
		SetStaticText( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type, IDS_PANELCIRC2_SVTYPE );
		AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect, 2, 1 );
		SetStaticText( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect, IDS_PANELCIRC2_SVCONNECT );
		AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version, 2, 1 );
		SetStaticText( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version, IDS_PANELCIRC2_SVVERSION );
		AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN, 2, 1 );
		SetStaticText( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN, IDS_PANELCIRC2_SVPN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxShutoffValve.pos.x + eBCol::Lock, iRows + m_BoxShutoffValve.SV_Dp, 2, 1 );
	SetStaticText( m_BoxShutoffValve.pos.x + eBCol::Lock, iRows + m_BoxShutoffValve.SV_Dp, IDS_PANELCIRC2_SVDP );

	// Combo and Value fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( m_BoxShutoffValve.m_eBoxType, m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name );

	if( true == m_BoxShutoffValve.bExt )
	{
		_FormatComboList( m_BoxShutoffValve.m_eBoxType, m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type );
		_FormatComboList( m_BoxShutoffValve.m_eBoxType, m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect );
		_FormatComboList( m_BoxShutoffValve.m_eBoxType, m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version );
		_FormatComboList( m_BoxShutoffValve.m_eBoxType, m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	SetStaticText( m_BoxShutoffValve.pos.x + eBCol::Val, iRows + m_BoxShutoffValve.SV_Dp, _T("") );

	// Unit fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( m_BoxShutoffValve.pos.x + eBCol::Unit, iRows + m_BoxShutoffValve.SV_Dp, unitname );

	ChangeSVType();

	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

void CSSheetPanelCirc2::ChangeSVType()
{
	if( false == m_pHM->IsShutoffValveExist( true ) )
	{
		return;
	}

	if( m_BoxShutoffValve.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxShutoffValve.bExt && NULL != m_pHM->GetpSch() )
	{
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type, SS_CBM_SETCURSEL, 0, 0 );

		CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;
			
		if( NULL != m_pHM->GetpSch() )
		{
			if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveSupply;
			}
			else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveReturn;
			}
		}

		if( CDS_HydroMod::eNone == eHMObj )
		{
			ASSERT_RETURN;
		}

		if( NULL != m_pHM->GetpShutoffValve( eHMObj )->GetpSelType() )
		{
			ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetpSelType()->GetString() ) );
			ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type, (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetpSelType()->GetIDPtr().MP ) );
		}
	}

	ChangeSVConnect();
}

void CSSheetPanelCirc2::ChangeSVConnect()
{
	if( false == m_pHM->IsShutoffValveExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxShutoffValve.bExt && NULL != m_pHM->GetpSch() )
	{
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect, SS_CBM_SETCURSEL, 0, 0 );

		CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

		if( NULL != m_pHM->GetpSch() )
		{
			if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveSupply;
			}
			else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveReturn;
			}
		}

		if( CDS_HydroMod::eNone == eHMObj )
		{
			ASSERT_RETURN;
		}
		
		if( NULL != m_pHM->GetpShutoffValve( eHMObj )->GetpSelConnection() )
		{
			ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetpSelConnection()->GetString() ) );
			ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect,
						  (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetpSelConnection()->GetIDPtr().MP ) );
		}
	}

	ChangeSVVersion();
}

void CSSheetPanelCirc2::ChangeSVVersion()
{
	if( false == m_pHM->IsShutoffValveExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxShutoffValve.bExt && NULL != m_pHM->GetpSch() )
	{
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version, SS_CBM_SETCURSEL, 0, 0 );
		
		CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

		if( NULL != m_pHM->GetpSch() )
		{
			if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveSupply;
			}
			else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveReturn;
			}
		}

		if( CDS_HydroMod::eNone == eHMObj )
		{
			ASSERT_RETURN;
		}

		if( NULL != m_pHM->GetpShutoffValve( eHMObj )->GetpSelVersion() )
		{
			ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetpSelVersion()->GetString() ) );
			ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version,
						  (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetpSelVersion()->GetIDPtr().MP ) );
		}
	}

	ChangeSVPN();
}

void CSSheetPanelCirc2::ChangeSVPN()
{
	if( false == m_pHM->IsShutoffValveExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxShutoffValve.bExt && NULL != m_pHM->GetpSch() )
	{
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN, SS_CBM_SETCURSEL, 0, 0 );

		CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

		if( NULL != m_pHM->GetpSch() )
		{
			if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveSupply;
			}
			else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveReturn;
			}
		}

		if( CDS_HydroMod::eNone == eHMObj )
		{
			ASSERT_RETURN;
		}

		if( NULL != m_pHM->GetpShutoffValve( eHMObj )->GetpSelPN() )
		{
			ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetpSelPN()->GetString() ) );
			ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN, (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetpSelPN()->GetIDPtr().MP ) );
		}
	}

	ChangeSV();
}

void CSSheetPanelCirc2::ChangeSV()
{
	if( false == m_pHM->IsShutoffValveExist( true ) || NULL == m_pHM->GetpSch() )
	{
		return;
	}

	CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;

	if( NULL != m_pHM->GetpSch() )
	{
		if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
		{
			eHMObj = CDS_HydroMod::eShutoffValveSupply;
		}
		else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
		{
			eHMObj = CDS_HydroMod::eShutoffValveReturn;
		}
	}

	if( CDS_HydroMod::eNone == eHMObj )
	{
		ASSERT_RETURN;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, SS_CBM_RESETCONTENT, 0, 0 );
	ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, SS_CBM_SETCURSEL, 0, 0 );

	if( false == m_pHM->IsLocked( m_pHM->GetpShutoffValve( eHMObj )->GetHMObjectType() ) )
	{
		SetFontBold( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, false );
	}
	else
	{
		SetFontBold( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, true );
	}

	if( ( (CDB_ShutoffValve *)( m_pHM->GetpShutoffValve( eHMObj )->GetIDPtr().MP ) ) )
	{
		CDB_ShutoffValve *pclShutoffValve = (CDB_ShutoffValve *)( m_pHM->GetpShutoffValve( eHMObj )->GetIDPtr().MP );

		std::wstring str;
		pclShutoffValve->GetNameEx( &str, false );

		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, SS_CBM_ADDSTRING, 0, (LPARAM)( str.c_str() ) );
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, (LPARAM)( m_pHM->GetpShutoffValve( eHMObj )->GetIDPtr().MP ) );

		int iRow = 0;

		if( true == m_BoxShutoffValve.bExt )
		{
			iRow = m_BoxShutoffValve.SV_PN;
		}

		double dSI = m_pHM->GetpShutoffValve( eHMObj )->GetDp();
		SetCUDouble( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + iRow + m_BoxShutoffValve.SV_Dp, _U_DIFFPRESS, dSI );

		if( eb3False == m_pHM->GetpShutoffValve( eHMObj )->CheckDpMinMax() )
		{
			SetForeColor( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + iRow + m_BoxShutoffValve.SV_Dp, _RED );
		}
		else
		{
			SetForeColor( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + iRow + m_BoxShutoffValve.SV_Dp, _BLACK );
		}

		// Refresh LockPicture.
		// 		COLORREF BackgroundColor = 0;
		// 		BackgroundColor = GetBackColor( m_BoxShutoffValve.pos.x + eBCol::Desc, m_BoxShutoffValve.pos.y);
		SetPictureLock( m_BoxShutoffValve.pos.x + eBCol::Lock, m_BoxShutoffValve.pos.y, m_pHM->IsLocked( m_pHM->GetpShutoffValve( eHMObj )->GetHMObjectType() ) );
	}
	else
	{
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, SS_CBM_ADDSTRING, 0, (LPARAM)( _T(" ") ) );
		ComboBoxSendMessage( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxShutoffValve.pos.x + eBCol::Val, m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name, 0 );
	}

	// Change picture.
	m_pHM->ResetSchemeIDPtr();
	FormatBoxPicture();
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
	RefreshDispatch( _BoxType::ebShutoffValve );
}

void CSSheetPanelCirc2::FormatBoxDpCBCV()
{
	if( m_BoxDpCBCV.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	_ClearRange( m_BoxDpCBCV.m_eBoxType, m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y, m_BoxDpCBCV.pos.x + eBCol::Unit, m_BoxDpCBCV.pos.y +  m_BoxDpCBCV.nRows - 1 );

	// Lock and erase all combined Dp controller, control and balancing valve cells.
	SetCellProperty( m_BoxDpCBCV.pos.x + eBCol::Butt, m_BoxDpCBCV.pos.y, m_BoxDpCBCV.pos.x + eBCol::Unit, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.nRows - 1,
					 CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetCellBorder( m_BoxDpCBCV.pos.x + eBCol::Butt, m_BoxDpCBCV.pos.y, m_BoxDpCBCV.pos.x + eBCol::Unit, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.nRows - 1, false );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	FormatStaticText( m_BoxDpCBCV.pos.x + eBCol::Butt, m_BoxDpCBCV.pos.y, m_BoxDpCBCV.pos.x + eBCol::Unit, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.nRows - 1, _T("") );

	int iRows = m_BoxDpCBCV.DpCBCV_Actuator + ( ( true == m_BoxDpCBCV.bExt ) ? m_BoxDpCBCV.DpCBCV_PN : 0 );
	COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::ShutoffValve );
	FormatBox( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y, iRows + 1, IDS_PANELCIRC2_DPCBCVTITLE, BackGroundColor, true );

	// Draw button cells in front of name.
	if( true == m_BoxDpCBCV.bExt )
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SMINUS, m_BoxDpCBCV.pos.x + eBCol::Butt, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_PN;
	}
	else
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SPLUS, m_BoxDpCBCV.pos.x + eBCol::Butt, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxDpCBCV.pos.y;
	}

	// Activate mouse left button event for this minus/plus button.
	SetCellProperty( m_BoxDpCBCV.pos.x + eBCol::Butt, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Cell span for DpCBCV name.
	AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, 2, 1 );

	if( true == m_BoxDpCBCV.bExt )
	{
		// Cell span also for 'Connection', 'Version' and 'PN'.
		AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect, 2, 1 );
		AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version, 2, 1 );
		AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN, 2, 1 );
	}
	else
	{
		RemoveCellSpan( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect );
		RemoveCellSpan( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version );
		RemoveCellSpan( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN );
	}

	if( true == m_BoxDpCBCV.bExt )
	{
		AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Actuator + m_BoxDpCBCV.DpCBCV_PN, 2, 1 );
	}
	else
	{
		AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Actuator, 2, 1 );
	}

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, 2, 1 );
	SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, IDS_PANELCIRC2_DPCBCVNAME );

	if( true == m_BoxDpCBCV.bExt )
	{
		AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect, 2, 1 );
		SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect, IDS_PANELCIRC2_DPCBCVCONNECT );
		AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version, 2, 1 );
		SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version, IDS_PANELCIRC2_DPCBCVVERSION );
		AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN, 2, 1 );
		SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN, IDS_PANELCIRC2_DPCBCVPN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Setting.
	AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Lock, iRows + m_BoxDpCBCV.DpCBCV_Presetting, 2, 1 );
	SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Lock, iRows + m_BoxDpCBCV.DpCBCV_Presetting, IDS_PANELCIRC2_DPCBCVPRESETTING );

	// Dp.
	AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Lock, iRows + m_BoxDpCBCV.DpCBCV_Dp, 2, 1 );
	SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Lock, iRows + m_BoxDpCBCV.DpCBCV_Dp, IDS_PANELCIRC2_DPCBCVDP );

	// Actuator.
	AddCellSpanW( m_BoxDpCBCV.pos.x + eBCol::Lock, iRows + m_BoxDpCBCV.DpCBCV_Actuator, 2, 1 );
	SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Lock, iRows + m_BoxDpCBCV.DpCBCV_Actuator, IDS_PANELCIRC2_ACTUATOR );

	// Combo and Value fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( m_BoxDpCBCV.m_eBoxType, m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name );

	if( true == m_BoxDpCBCV.bExt )
	{
		_FormatComboList( m_BoxDpCBCV.m_eBoxType, m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect );
		_FormatComboList( m_BoxDpCBCV.m_eBoxType, m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version );
		_FormatComboList( m_BoxDpCBCV.m_eBoxType, m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN );
	}

	_FormatComboList( m_BoxDpCBCV.m_eBoxType, m_BoxDpCBCV.pos.x + eBCol::Val, iRows + m_BoxDpCBCV.DpCBCV_Actuator );

	// Force right align.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );

	SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Val, iRows + m_BoxDpCBCV.DpCBCV_Presetting, _T("") );
	SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Val, iRows + m_BoxDpCBCV.DpCBCV_Dp, _T("") );

	// Unit fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Unit, iRows + m_BoxDpCBCV.DpCBCV_Presetting, IDS_TURNS );

	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( m_BoxDpCBCV.pos.x + eBCol::Unit, iRows + m_BoxDpCBCV.DpCBCV_Dp, unitname );

	ChangeDpCBCVConnect();

	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

void CSSheetPanelCirc2::ChangeDpCBCVConnect()
{
	if( false == m_pHM->IsCvExist( true ) || eb3False == m_pHM->GetpCV()->IsDpCBCV() )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxDpCBCV.bExt )
	{
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpCV()->GetpSelCVConn() )
		{
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpCV()->GetpSelCVConn()->GetString() ) );
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect,
						  (LPARAM)( m_pHM->GetpCV()->GetpSelCVConn()->GetIDPtr().MP ) );
		}
	}

	ChangeDpCBCVVersion();
}

void CSSheetPanelCirc2::ChangeDpCBCVVersion()
{
	if( false == m_pHM->IsCvExist( true ) || eb3False == m_pHM->GetpCV()->IsDpCBCV() )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxDpCBCV.bExt )
	{
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version, SS_CBM_SETCURSEL, 0, 0 );
		
		if( NULL != m_pHM->GetpCV()->GetpSelCVVers() )
		{
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpCV()->GetpSelCVVers()->GetString() ) );
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version,
						  (LPARAM)( m_pHM->GetpCV()->GetpSelCVVers()->GetIDPtr().MP ) );
		}
	}

	ChangeDpCBCVPN();
}

void CSSheetPanelCirc2::ChangeDpCBCVPN()
{
	if( false == m_pHM->IsCvExist( true ) || eb3False == m_pHM->GetpCV()->IsDpCBCV() )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxDpCBCV.bExt )
	{
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpCV()->GetpSelCVPN() )
		{
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN, SS_CBM_ADDSTRING, 0,
								 (LPARAM)( m_pHM->GetpCV()->GetpSelCVPN()->GetString() ) );
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN, SS_CBM_SETCURSEL, 0, 0 );
			SetCellParam( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN, (LPARAM)( m_pHM->GetpCV()->GetpSelCVPN()->GetIDPtr().MP ) );
		}
	}

	ChangeDpCBCV();
}

void CSSheetPanelCirc2::ChangeDpCBCV()
{
	if( false == m_pHM->IsCvExist( true ) || eb3False == m_pHM->GetpCV()->IsDpCBCV() )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, SS_CBM_RESETCONTENT, 0, 0 );
	ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, SS_CBM_SETCURSEL, 0, 0 );

	// HYS-1221 : If the selected product is deleted display it in Red color
	if( ( NULL != (CDB_DpCBCValve *)( m_pHM->GetpCV()->GetCvIDPtr().MP ) )
		&& true == m_pHM->GetpCV()->GetCvIDPtr().MP->IsDeleted() )
	{
		SetForeColor( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, _RED );
	}
	else
	{
		SetForeColor( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, _BLACK );
	}

	if( false == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
	{
		SetFontBold( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, false );
	}
	else
	{
		SetFontBold( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, true );
	}

	if( ( ( CDB_DpCBCValve * )( m_pHM->GetpCV()->GetCvIDPtr().MP ) ) )
	{
		CDB_DpCBCValve *pclDpCBCValve = ( CDB_DpCBCValve * )( m_pHM->GetpCV()->GetCvIDPtr().MP );

		CString str;

		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, SS_CBM_ADDSTRING, 0, (LPARAM)( pclDpCBCValve->GetName() ) );
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, (LPARAM)( m_pHM->GetpCV()->GetCvIDPtr().MP ) );

		int iRow = 0;

		if( true == m_BoxDpCBCV.bExt )
		{
			iRow = m_BoxDpCBCV.DpCBCV_PN;
		}

		SetCellText( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + iRow + m_BoxDpCBCV.DpCBCV_Presetting, m_pHM->GetpCV()->GetSettingStr() );

		double dSI = m_pHM->GetpCV()->GetDp();
		SetCUDouble( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + iRow + m_BoxDpCBCV.DpCBCV_Dp, _U_DIFFPRESS, dSI );

		// TODO: for the moment, we verify only Dpmax.
		if( dSI > m_pTechParam->VerifyValvMaxDp( ( CDB_TAProduct * )m_pHM->GetpCV()->GetCvIDPtr().MP ) )
		{
			SetForeColor( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + iRow + m_BoxDpCBCV.DpCBCV_Dp, _RED );
		}
		else
		{
			SetForeColor( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + iRow + m_BoxDpCBCV.DpCBCV_Dp, _BLACK );
		}

		// Check actuator.
		int iActuatorBaseRow = ( true == m_BoxDpCBCV.bExt ) ? m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_PN : m_BoxDpCBCV.pos.y;

		if( NULL != m_pHM->GetpCV()->GetActrIDPtr().MP )
		{
			LPARAM param = 0;
			CString str;
			param = (LPARAM) m_pHM->GetpCV()->GetActrIDPtr().MP;
			str = ( ( CDB_Actuator * )( m_pHM->GetpCV()->GetActrIDPtr().MP ) )->GetName();
			_FormatComboList( m_BoxDpCBCV.m_eBoxType, m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator );
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, SS_CBM_RESETCONTENT, 0, 0 );
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, SS_CBM_SETCURSEL, 0, 0 );

			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR ) str );
			ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, SS_CBM_SETCURSEL, 0, 0 );

			SetCellParam( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, param );
		}
		else
		{
			// Is there any possibility to have an actuator?
			CDB_ControlValve *pDBCV = dynamic_cast<CDB_ControlValve *>( m_pHM->GetpCV()->GetCvIDPtr().MP );
			str = TASApp.LoadLocalizedString( IDS_NOACTUATOR );

			// No possibility to add an actuator, no actuator group idptr.
			if( NULL == pDBCV->GetActuatorGroupIDPtr().MP )
			{
				// Set "None" as a static text.
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
				FormatStaticText( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Actuator, str );
			}
			else
			{
				LPARAM param = 0;
				_FormatComboList( m_BoxDpCBCV.m_eBoxType, m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator );
				ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, SS_CBM_RESETCONTENT, 0, 0 );
				ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, SS_CBM_SETCURSEL, 0, 0 );

				ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, SS_CBM_ADDSTRING, 0, (LPARAM)( LPCTSTR ) str );
				ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, SS_CBM_SETCURSEL, 0, 0 );

				SetCellParam( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, param );
			}
		}

		// If control valve is locked, we consider that actuator is also automatically locked.
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
		{
			SetFontBold( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, TRUE );
		}
		else
		{
			SetFontBold( m_BoxDpCBCV.pos.x + eBCol::Val, iActuatorBaseRow + m_BoxDpCBCV.DpCBCV_Actuator, FALSE );
		}

		// Refresh LockPicture.
		SetPictureLock( m_BoxDpCBCV.pos.x + eBCol::Lock, m_BoxDpCBCV.pos.y, m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) );
	}
	else
	{
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, SS_CBM_ADDSTRING, 0, (LPARAM)( _T(" ") ) );
		ComboBoxSendMessage( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name, 0 );
	}

	// Change picture.
	m_pHM->ResetSchemeIDPtr();
	FormatBoxPicture();
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
	RefreshDispatch( _BoxType::ebDpCBCValve );
}

void CSSheetPanelCirc2::FormatBoxSmartControlValve()
{
	if( m_BoxSmartControlValve.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Clear first the previous smart control valve box.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	_ClearRange( m_BoxSmartControlValve.m_eBoxType, m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y, m_BoxSmartControlValve.pos.x + eBCol::Unit, m_BoxSmartControlValve.pos.y +  m_BoxSmartControlValve.nRows - 1 );

	// Lock and erase all smart control valve cells.
	SetCellProperty( m_BoxSmartControlValve.pos.x + eBCol::Butt, m_BoxSmartControlValve.pos.y, m_BoxSmartControlValve.pos.x + eBCol::Unit, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.nRows - 1,
			CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetCellBorder( m_BoxSmartControlValve.pos.x + eBCol::Butt, m_BoxSmartControlValve.pos.y, m_BoxSmartControlValve.pos.x + eBCol::Unit, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.nRows - 1, false );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	FormatStaticText( m_BoxSmartControlValve.pos.x + eBCol::Butt, m_BoxSmartControlValve.pos.y, m_BoxSmartControlValve.pos.x + eBCol::Unit, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.nRows - 1, _T("") );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Now change some value in regards to the current control mode.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// If control mode is flow we show only the flow max.
	m_BoxSmartControlValve.nRows = SMARTCONTROLVALVENROWS;
	m_BoxSmartControlValve.iDp = m_BoxSmartControlValve.iFlowMax + 1;
	m_BoxSmartControlValve.iDpMin = m_BoxSmartControlValve.iDp + 1;
	
	if( SmartValveControlMode::SCVCM_Power == m_pHM->GetpSmartControlValve()->GetControlMode() )
	{
		// If control mode is power we show flow max and power max.
		m_BoxSmartControlValve.nRows++;
		m_BoxSmartControlValve.iDp++;
		m_BoxSmartControlValve.iDpMin++;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Draw the smart control valve box.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int iRows = m_BoxSmartControlValve.iDpMin + ( ( true == m_BoxSmartControlValve.bExt ) ? m_BoxSmartControlValve.iPN : 0 );
	COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::SmartControlValve );
	FormatBox( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y, iRows + 1, IDS_PANELCIRC2_SMARTCONTROLVALVETITLE, BackGroundColor, true );

	// Draw button cells in front of name.
	if( true == m_BoxSmartControlValve.bExt )
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SMINUS, m_BoxSmartControlValve.pos.x + eBCol::Butt, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iPN;
	}
	else
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SPLUS, m_BoxSmartControlValve.pos.x + eBCol::Butt, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxSmartControlValve.pos.y;
	}

	// Activate mouse left button event for this minus/plus button.
	SetCellProperty( m_BoxSmartControlValve.pos.x + eBCol::Butt, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Cell span for the name.
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, 2, 1 );

	if( true == m_BoxSmartControlValve.bExt )
	{
		// Cell span also for 'Body material', 'connection' and 'PN'.
		AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial, 2, 1 );
		AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect, 2, 1 );
		AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN, 2, 1 );
	}
	else
	{
		// Remove span for 'Body material', 'connection' and 'PN' combo boxes.
		RemoveCellSpan( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial );
		RemoveCellSpan( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect );
		RemoveCellSpan( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN );

		// Remove span for "Localization" and "Control mode" combo boxes.
		RemoveCellSpan( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iLocation );
		RemoveCellSpan( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iControlMode );
	}

	// Localization combo.
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iLocation, 2, 1 );

	// Control mode combo.
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iControlMode, 2, 1 );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, 2, 1 );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, IDS_PANELCIRC2_SMARTCONTROLVALVENAME );

	if( true == m_BoxSmartControlValve.bExt )
	{
		AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial, 2, 1 );
		SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial, IDS_PANELCIRC2_SMARTCONTROLVALVEBODYMATERIAL );
		AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect, 2, 1 );
		SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect, IDS_PANELCIRC2_SMARTCONTROLVALVECONNECT );
		AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN, 2, 1 );
		SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN, IDS_PANELCIRC2_SMARTCONTROLVALVEPN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Location.
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iLocation, 2, 1 );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iLocation, IDS_PANELCIRC2_SMARTCONTROLVALVELOCATION );

	// Control mode.
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iControlMode, 2, 1 );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iControlMode, IDS_PANELCIRC2_SMARTCONTROLVALVECONTROLMODE );

	// Q max.
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iFlowMax, 2, 1 );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iFlowMax, IDS_PANELCIRC2_SMARTCONTROLVALVEQMAX );

	// Power max.
	if( SmartValveControlMode::SCVCM_Power == m_pHM->GetpSmartControlValve()->GetControlMode() )
	{
		AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iPowerMax, 2, 1 );
		SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iPowerMax, IDS_PANELCIRC2_SMARTCONTROLVALVEPOWERMAX );
	}

	// Dp.
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iDp, 2, 1 );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iDp, IDS_PANELCIRC2_SMARTCONTROLVALVEDP );

	// Dp min.
	AddCellSpanW( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iDpMin, 2, 1 );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Lock, iRows + m_BoxSmartControlValve.iDpMin, IDS_PANELCIRC2_SMARTCONTROLVALVEDPMIN );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Combo and value fields.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( m_BoxSmartControlValve.m_eBoxType, m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName );

	if( true == m_BoxSmartControlValve.bExt )
	{
		_FormatComboList( m_BoxSmartControlValve.m_eBoxType, m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial );
		_FormatComboList( m_BoxSmartControlValve.m_eBoxType, m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect );
		_FormatComboList( m_BoxSmartControlValve.m_eBoxType, m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN );
	}

	_FormatComboList( m_BoxSmartControlValve.m_eBoxType, m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iLocation );
	_FormatComboList( m_BoxSmartControlValve.m_eBoxType, m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iControlMode );

	// Force right align.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );

	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iFlowMax, _T("") );

	if( SmartValveControlMode::SCVCM_Power == m_pHM->GetpSmartControlValve()->GetControlMode() )
	{
		SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iPowerMax, _T("") );
	}

	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iDp, _T("") );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iDpMin, _T("") );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Unit fields.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_FLOW ), unitname );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Unit, iRows + m_BoxSmartControlValve.iFlowMax, unitname );

	if( SmartValveControlMode::SCVCM_Power == m_pHM->GetpSmartControlValve()->GetControlMode() )
	{
		GetNameOf( m_pUnitDB->GetDefaultUnit( _U_TH_POWER ), unitname );
		SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Unit, iRows + m_BoxSmartControlValve.iPowerMax, unitname );
	}

	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Unit, iRows + m_BoxSmartControlValve.iDp, unitname );
	SetStaticText( m_BoxSmartControlValve.pos.x + eBCol::Unit, iRows + m_BoxSmartControlValve.iDpMin, unitname );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Values.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Location.
	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iLocation, SS_CBM_RESETCONTENT, 0, 0 );

	CString str = TASApp.LoadLocalizedString( IDS_SMARTVALVE_LOCATION_SUPPLY );
	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iLocation, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

	str = TASApp.LoadLocalizedString( IDS_SMARTVALVE_LOCATION_RETURN );
	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iLocation, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

	int iCurrentSel = ( SmartValveLocalization::SmartValveLocSupply == m_pHM->GetpSmartControlValve()->GetLocalization() ) ? 0 : 1;

	if( SmartValveLocalization::SmartValveLocNone != m_pHM->GetpSmartControlValve()->GetSelLocalization() )
	{
		iCurrentSel = ( SmartValveLocalization::SmartValveLocSupply == m_pHM->GetpSmartControlValve()->GetSelLocalization() ) ? 0 : 1;
	}

	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iLocation, SS_CBM_SETCURSEL, iCurrentSel, 0 );

	// Control mode.
	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iControlMode, SS_CBM_RESETCONTENT, 0, 0 );

	str = TASApp.LoadLocalizedString( IDS_SMARTCONTROLVALVE_CONTROLMODE_FLOW );
	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iControlMode, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

	str = TASApp.LoadLocalizedString( IDS_SMARTCONTROLVALVE_CONTROLMODE_POWER );
	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iControlMode, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

	if( SmartValveControlMode::SCVCM_Flow == m_pHM->GetpSmartControlValve()->GetControlMode() )
	{
		iCurrentSel = 0;
	}
	else if( SmartValveControlMode::SCVCM_Power == m_pHM->GetpSmartControlValve()->GetControlMode() )
	{
		iCurrentSel = 1;
	}

	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, iRows + m_BoxSmartControlValve.iControlMode, SS_CBM_SETCURSEL, iCurrentSel, 0 );

	ChangeSmartControlValveBodyMaterial();

	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

void CSSheetPanelCirc2::ChangeSmartControlValveBodyMaterial()
{
	if( false == m_pHM->IsSmartControlValveExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxSmartControlValve.bExt )
	{
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpSmartControlValve()->GetpSelBodyMaterial() )
		{
			ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial, 
					SS_CBM_ADDSTRING, 0, (LPARAM)( m_pHM->GetpSmartControlValve()->GetpSelBodyMaterial()->GetString() ) );

			ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial, 
					SS_CBM_SETCURSEL, 0, 0 );

			SetCellParam( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial,
					(LPARAM)( m_pHM->GetpSmartControlValve()->GetpSelBodyMaterial()->GetIDPtr().MP ) );
		}
	}

	ChangeSmartControlValveConnect();
}

void CSSheetPanelCirc2::ChangeSmartControlValveConnect()
{
	if( false == m_pHM->IsSmartControlValveExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxSmartControlValve.bExt )
	{
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpSmartControlValve()->GetpSelConnection() )
		{
			ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect, 
					SS_CBM_ADDSTRING, 0, (LPARAM)( m_pHM->GetpSmartControlValve()->GetpSelConnection()->GetString() ) );

			ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect, 
					SS_CBM_SETCURSEL, 0, 0 );
			
			SetCellParam( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect,
						  (LPARAM)( m_pHM->GetpSmartControlValve()->GetpSelConnection()->GetIDPtr().MP ) );
		}
	}

	ChangeSmartControlValvePN();
}

void CSSheetPanelCirc2::ChangeSmartControlValvePN()
{
	if( false == m_pHM->IsSmartControlValveExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxSmartControlValve.bExt )
	{
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpSmartControlValve()->GetpSelPN() )
		{
			ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN, 
					SS_CBM_ADDSTRING, 0, (LPARAM)( m_pHM->GetpSmartControlValve()->GetpSelPN()->GetString() ) );
			
			ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN, 
					SS_CBM_SETCURSEL, 0, 0 );
			
			SetCellParam( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN, 
					(LPARAM)( m_pHM->GetpSmartControlValve()->GetpSelPN()->GetIDPtr().MP ) );
		}
	}

	ChangeSmartControlValve();
}

void CSSheetPanelCirc2::ChangeSmartControlValveLocalization()
{
	if( false == m_pHM->IsSmartControlValveExist( false ) )
	{
		return;
	}

	long lLocationRow = ( false == m_BoxSmartControlValve.bExt ) ? m_BoxSmartControlValve.iLocation : m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iLocation;
	int iCurrentSel = ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + lLocationRow, SS_CBM_GETCURSEL, 0, 0 );

	if( 0 == iCurrentSel )
	{
		m_pHM->GetpSmartControlValve()->SetSelLocalizationUserChoice( SmartValveLocalization::SmartValveLocSupply );
		m_pHM->GetpSmartControlValve()->SetLocalization( SmartValveLocalization::SmartValveLocSupply );
	}
	else
	{
		m_pHM->GetpSmartControlValve()->SetSelLocalizationUserChoice( SmartValveLocalization::SmartValveLocReturn );
		m_pHM->GetpSmartControlValve()->SetLocalization( SmartValveLocalization::SmartValveLocReturn );
	}
}

void CSSheetPanelCirc2::ChangeSmartControlValveControlMode()
{
	if( false == m_pHM->IsSmartControlValveExist( false ) )
	{
		return;
	}

	long lControlModeRow = ( false == m_BoxSmartControlValve.bExt ) ? m_BoxSmartControlValve.iControlMode : m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iControlMode;
	int iCurrentSel = ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + lControlModeRow, SS_CBM_GETCURSEL, 0, 0 );

	SmartValveControlMode eSmartControlValveControlMode = SmartValveControlMode::SCVCM_Undefined;

	if( 0 == iCurrentSel )
	{
		eSmartControlValveControlMode = SmartValveControlMode::SCVCM_Flow;
	}
	else if( 1 == iCurrentSel )
	{
		eSmartControlValveControlMode = SmartValveControlMode::SCVCM_Power;
	}

	m_pHM->GetpSmartControlValve()->SetControlMode( eSmartControlValveControlMode );
}

void CSSheetPanelCirc2::ChangeSmartControlValve()
{
	if( false == m_pHM->IsSmartControlValveExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	int iRow = 0;

	if( true == m_BoxSmartControlValve.bExt )
	{
		iRow = m_BoxSmartControlValve.iPN;
	}

	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, SS_CBM_RESETCONTENT, 0, 0 );
	ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, SS_CBM_SETCURSEL, 0, 0 );

	if( NULL != dynamic_cast<CDB_SmartControlValve *>( m_pHM->GetpSmartControlValve()->GetIDPtr().MP ) )
	{
		CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( m_pHM->GetpSmartControlValve()->GetIDPtr().MP );

		// Set the name.
		std::wstring str;
		pclSmartControlValve->GetNameEx( &str, false );

		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, SS_CBM_ADDSTRING, 0, (LPARAM)( str.c_str() ) );
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, (LPARAM)( m_pHM->GetpSmartControlValve()->GetIDPtr().MP ) );

		bool bNameAlreadyInRed = false;

		// If the product is deleted, we set the name in red.
		if( true == pclSmartControlValve->GetIDPtr().MP->IsDeleted() )
		{
			SetForeColor( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, _RED );
			bNameAlreadyInRed = true;
		}
		else
		{
			SetForeColor( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, _BLACK );
		}

		// Check if there is a least on error.
		int iValidity = m_pHM->GetpSmartControlValve()->CheckValidity();

		if( false == bNameAlreadyInRed && 
				( CDS_HydroMod::eValidityFlags::evfDpTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooHigh ) 
					|| CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooHigh )
					|| CDS_HydroMod::eValidityFlags::evfFlowTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooLow )
					|| CDS_HydroMod::eValidityFlags::evfPowerTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooHigh )
					|| CDS_HydroMod::eValidityFlags::evfPowerTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooLow )
					|| CDS_HydroMod::eValidityFlags::evfTempTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooHigh ) 
					|| CDS_HydroMod::eValidityFlags::evfTempTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooLow ) ) )
		{
			SetForeColor( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, _RED );
		}

		// If product is locked, we set the name in bold.
		if( false == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartControlValve ) )
		{
			SetFontBold( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, false );
		}
		else
		{
			SetFontBold( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, true );
		}

		// Set flow max.
		double dSI = m_pHM->GetpSmartControlValve()->GetFlowMax();
		SetCUDouble( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + iRow + m_BoxSmartControlValve.iFlowMax, _U_FLOW, dSI );

		// Verify if there is an error with the flow.
		if( CDS_HydroMod::eValidityFlags::evfFlowTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooHigh )
					|| CDS_HydroMod::eValidityFlags::evfFlowTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfFlowTooLow ) )
		{
			SetForeColor( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iFlowMax, _RED );
		}

		// Set power max.
		if( SmartValveControlMode::SCVCM_Power == m_pHM->GetpSmartControlValve()->GetControlMode() )
		{
			dSI = m_pHM->GetpSmartControlValve()->GetPowerMax();
			SetCUDouble( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + iRow + m_BoxSmartControlValve.iPowerMax, _U_TH_POWER, dSI );

			// Verify if there is an error with the power.
			if( CDS_HydroMod::eValidityFlags::evfPowerTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooHigh )
						|| CDS_HydroMod::eValidityFlags::evfPowerTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfPowerTooLow ) )
			{
				SetForeColor( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iPowerMax, _RED );
			}
		}

		// Set dp.
		dSI = m_pHM->GetpSmartControlValve()->GetDp();
		SetCUDouble( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + iRow + m_BoxSmartControlValve.iDp, _U_DIFFPRESS, dSI );

		// Check if there is error with the pressure drop accross the valve.
		if( CDS_HydroMod::eValidityFlags::evfDpTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooHigh ) )
		{
			SetForeColor( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + iRow + m_BoxSmartControlValve.iDp, _RED );
		}
		else
		{
			SetForeColor( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + iRow + m_BoxSmartControlValve.iDp, _BLACK );
		}


		// HYS-1914: Dp min is the minimum pressure drop when the valve is fully open (Computed with design flow and Kvs).
		dSI = m_pHM->GetpSmartControlValve()->GetDpMin();
		SetCUDouble( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + iRow + m_BoxSmartControlValve.iDpMin, _U_DIFFPRESS, dSI );

		// Refresh the lock picture.
		SetPictureLock( m_BoxSmartControlValve.pos.x + eBCol::Lock, m_BoxSmartControlValve.pos.y, m_pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartControlValve ) );
	}
	else
	{
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, SS_CBM_ADDSTRING, 0, (LPARAM)( _T(" ") ) );
		ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName, 0 );
	}

	// Change picture.
	m_pHM->ResetSchemeIDPtr();
	FormatBoxPicture();
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
	RefreshDispatch( _BoxType::ebSmartControlValve );
}

void CSSheetPanelCirc2::FormatBoxSmartDpC()
{
	if( m_BoxSmartDpC.pos == CPoint( 0, 0 ) )
	{
		return;
	}

	if( NULL == m_pHM->GetpSch() )
	{
		ASSERT_RETURN;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Clear first the previous smart differential pressure controller box.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	_ClearRange( m_BoxSmartDpC.m_eBoxType, m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y, m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y +  m_BoxSmartDpC.nRows - 1 );

	// Lock and erase all smart differential pressure controller cells.
	SetCellProperty( m_BoxSmartDpC.pos.x + eBCol::Butt, m_BoxSmartDpC.pos.y, m_BoxSmartDpC.pos.x + eBCol::Unit, 
			m_BoxSmartDpC.pos.y + m_BoxSmartDpC.nRows - 1, CSSheet::_SSCellProperty::CellNoSelection | CSSheet::_SSCellProperty::CellNoMouseEvent, true );

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetCellBorder( m_BoxSmartDpC.pos.x + eBCol::Butt, m_BoxSmartDpC.pos.y, m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.nRows - 1, false );

	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
	FormatStaticText( m_BoxSmartDpC.pos.x + eBCol::Butt, m_BoxSmartDpC.pos.y, m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.nRows - 1, _T("") );

	m_BoxSmartDpC.nRows = SMARTDPCNROWS;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Draw the smart differential pressure controller box.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int iRows = m_BoxSmartDpC.iDpl + ( ( true == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iPN : 0 );
	COLORREF BackGroundColor = m_pHM->GetpSch()->GetAnchorPtColor( CAnchorPt::SmartDpC );
	FormatBox( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y, iRows + 1, IDS_PANELCIRC2_SMARTDPCTITLE, BackGroundColor, true );

	// Draw button cells in front of name.
	if( true == m_BoxSmartDpC.bExt )
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SMINUS, m_BoxSmartDpC.pos.x + eBCol::Butt, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iPN;
	}
	else
	{
		if( false == m_bPrinting )
		{
			SetPictureCellWithID( IDI_SPLUS, m_BoxSmartDpC.pos.x + eBCol::Butt, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, CSSheet::PictureCellType::Icon );
		}

		iRows = m_BoxSmartDpC.pos.y;
	}

	// Activate mouse left button event for this minus/plus button.
	SetCellProperty( m_BoxSmartDpC.pos.x + eBCol::Butt, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, CSSheet::_SSCellProperty::CellCantLeftClick, false );

	// Cell span for the name.
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, 2, 1 );

	if( true == m_BoxSmartDpC.bExt )
	{
		// Cell span also for 'Body material', 'connection' and 'PN'.
		AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial, 2, 1 );
		AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect, 2, 1 );
		AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN, 2, 1 );
	}
	else
	{
		// Remove span for 'Body material', 'connection' and 'PN' combo boxes.
		RemoveCellSpan( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial );
		RemoveCellSpan( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect );
		RemoveCellSpan( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN );

		// Remove span for "Localization".
		RemoveCellSpan( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iPN + m_BoxSmartDpC.iLocation );
	}

	// Localization combo.
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iLocation, 2, 1 );

	// Sets combo.
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iSets, 2, 1 );

	// Description fields.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, 2, 1 );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName,  IDS_PANELCIRC2_SMARTDPCNAME );

	if( true == m_BoxSmartDpC.bExt )
	{
		AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial, 2, 1 );
		SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial,  IDS_PANELCIRC2_SMARTDPCBODYMATERIAL );
		AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect, 2, 1 );
		SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect,  IDS_PANELCIRC2_SMARTDPCCONNECT );
		AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN, 2, 1 );
		SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN,  IDS_PANELCIRC2_SMARTDPCPN );
	}

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Location.
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iLocation, 2, 1 );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iLocation,  IDS_PANELCIRC2_SMARTDPCLOCATION );

	// Dp.
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iDp, 2, 1 );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iDp,  IDS_PANELCIRC2_SMARTDPCDP );

	// Dp min.
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iDpMin, 2, 1 );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iDpMin,  IDS_PANELCIRC2_SMARTDPCDPMIN );

	// Sets.
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iSets, 2, 1 );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iSets,  IDS_PANELCIRC2_SMARTDPCSETS );

	// Dpl.
	AddCellSpanW( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iDpl, 2, 1 );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Lock, iRows + m_BoxSmartDpC.iDpl,  IDS_PANELCIRC2_SMARTDPCDPL );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Combo and value fields.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	_FormatComboList( m_BoxSmartDpC.m_eBoxType, m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName );

	if( true == m_BoxSmartDpC.bExt )
	{
		_FormatComboList( m_BoxSmartDpC.m_eBoxType, m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial );
		_FormatComboList( m_BoxSmartDpC.m_eBoxType, m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect );
		_FormatComboList( m_BoxSmartDpC.m_eBoxType, m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN );
	}

	_FormatComboList( m_BoxSmartDpC.m_eBoxType, m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iLocation );

	if( false == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) )
	{
		_FormatComboList( m_BoxSmartDpC.m_eBoxType, m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iSets );
	}
	else
	{
		SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iSets, _T("") );
	}
	
	// Force right align.
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );

	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iDp, _T("") );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iDpMin, _T("") );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iDpl, _T("") );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Unit fields.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	TCHAR unitname[_MAXCHARS];
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), unitname );

	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Unit, iRows + m_BoxSmartDpC.iDp, unitname );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Unit, iRows + m_BoxSmartDpC.iDpMin, unitname );
	SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Unit, iRows + m_BoxSmartDpC.iDpl, unitname );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Values.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Location.
	ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iLocation, SS_CBM_RESETCONTENT, 0, 0 );

	CString str = TASApp.LoadLocalizedString( IDS_SMARTVALVE_LOCATION_SUPPLY );
	ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iLocation, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

	str = TASApp.LoadLocalizedString( IDS_SMARTVALVE_LOCATION_RETURN );
	ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iLocation, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)str );

	int iCurrentSel = ( SmartValveLocalization::SmartValveLocSupply == m_pHM->GetpSmartDpC()->GetLocalization() ) ? 0 : 1;

	if( SmartValveLocalization::SmartValveLocNone != m_pHM->GetpSmartDpC()->GetSelLocalization() )
	{
		iCurrentSel = ( SmartValveLocalization::SmartValveLocSupply == m_pHM->GetpSmartDpC()->GetSelLocalization() ) ? 0 : 1;
	}

	ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, iRows + m_BoxSmartDpC.iLocation, SS_CBM_SETCURSEL, iCurrentSel, 0 );

	// Dp sensor.
	// Remark: The "Dp sensor" combo will be filled in the 'ChangeSmartDpC' method.

	ChangeSmartDpCBodyMaterial();

	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
}

void CSSheetPanelCirc2::ChangeSmartDpCBodyMaterial()
{
	if( false == m_pHM->IsSmartDpCExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxSmartDpC.bExt )
	{
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpSmartDpC()->GetpSelBodyMaterial() )
		{
			ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial, 
					SS_CBM_ADDSTRING, 0, (LPARAM)( m_pHM->GetpSmartDpC()->GetpSelBodyMaterial()->GetString() ) );

			ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial, 
					SS_CBM_SETCURSEL, 0, 0 );

			SetCellParam( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial,
					(LPARAM)( m_pHM->GetpSmartDpC()->GetpSelBodyMaterial()->GetIDPtr().MP ) );
		}
	}

	ChangeSmartDpCConnect();
}

void CSSheetPanelCirc2::ChangeSmartDpCConnect()
{
	if( false == m_pHM->IsSmartDpCExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxSmartDpC.bExt )
	{
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpSmartDpC()->GetpSelConnection() )
		{
			ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect, 
					SS_CBM_ADDSTRING, 0, (LPARAM)( m_pHM->GetpSmartDpC()->GetpSelConnection()->GetString() ) );

			ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect, 
					SS_CBM_SETCURSEL, 0, 0 );
			
			SetCellParam( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect,
						  (LPARAM)( m_pHM->GetpSmartDpC()->GetpSelConnection()->GetIDPtr().MP ) );
		}
	}

	ChangeSmartDpCPN();
}

void CSSheetPanelCirc2::ChangeSmartDpCPN()
{
	if( false == m_pHM->IsSmartDpCExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	if( true == m_BoxSmartDpC.bExt )
	{
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN, SS_CBM_RESETCONTENT, 0, 0 );
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN, SS_CBM_SETCURSEL, 0, 0 );

		if( NULL != m_pHM->GetpSmartDpC()->GetpSelPN() )
		{
			ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN, 
					SS_CBM_ADDSTRING, 0, (LPARAM)( m_pHM->GetpSmartDpC()->GetpSelPN()->GetString() ) );
			
			ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN, 
					SS_CBM_SETCURSEL, 0, 0 );
			
			SetCellParam( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN, 
					(LPARAM)( m_pHM->GetpSmartDpC()->GetpSelPN()->GetIDPtr().MP ) );
		}
	}

	ChangeSmartDpC();
}

void CSSheetPanelCirc2::ChangeSmartDpCLocalization()
{
	if( false == m_pHM->IsSmartDpCExist( false ) )
	{
		return;
	}

	long lLocationRow = ( false == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iLocation : m_BoxSmartDpC.iPN + m_BoxSmartDpC.iLocation;
	int iCurrentSel = ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + lLocationRow, SS_CBM_GETCURSEL, 0, 0 );

	if( 0 == iCurrentSel )
	{
		m_pHM->GetpSmartDpC()->SetSelLocalizationUserChoice( SmartValveLocalization::SmartValveLocSupply );
		m_pHM->GetpSmartDpC()->SetLocalization( SmartValveLocalization::SmartValveLocSupply );
	}
	else
	{
		m_pHM->GetpSmartDpC()->SetSelLocalizationUserChoice( SmartValveLocalization::SmartValveLocReturn );
		m_pHM->GetpSmartDpC()->SetLocalization( SmartValveLocalization::SmartValveLocReturn );
	}
}

void CSSheetPanelCirc2::ChangeSmartDpCSets()
{
	if( false == m_pHM->IsSmartDpCExist( false ) )
	{
		return;
	}

	long lSetsCol = m_BoxSmartDpC.pos.x + eBCol::Val;
	long lSetsRow = ( false == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iSets : m_BoxSmartDpC.iPN + m_BoxSmartDpC.iSets;
	lSetsRow += m_BoxSmartDpC.pos.y;
	
	int iCurrentSel = ComboBoxSendMessage( lSetsCol, lSetsRow, SS_CBM_GETCURSEL, 0, 0 );
	CDB_Product *pclProductSetSelected = dynamic_cast<CDB_Product *>( (CData *)_GetComboBoxParam( _BoxType::ebSmartDpC, lSetsCol, lSetsRow, iCurrentSel ) );

	if( NULL != pclProductSetSelected )
	{
		m_pHM->GetpSmartDpC()->SetProductSetIDPtr( pclProductSetSelected->GetIDPtr() );
	}
}

void CSSheetPanelCirc2::ChangeSmartDpC()
{
	if( false == m_pHM->IsSmartDpCExist( true ) )
	{
		return;
	}

	TASApp.OverridedSetRedraw( this, FALSE );

	int iRow = 0;

	if( true == m_BoxSmartDpC.bExt )
	{
		iRow = m_BoxSmartDpC.iPN;
	}

	ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, SS_CBM_RESETCONTENT, 0, 0 );
	ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, SS_CBM_SETCURSEL, 0, 0 );

	double dDpl = m_pHM->GetpSmartDpC()->GetDpToStabilize();

	if( NULL != dynamic_cast<CDB_SmartControlValve *>( m_pHM->GetpSmartDpC()->GetIDPtr().MP ) )
	{
		CDB_SmartControlValve *pclSmartDpC = (CDB_SmartControlValve *)( m_pHM->GetpSmartDpC()->GetIDPtr().MP );
		
		// Set the name.
		std::wstring str;
		pclSmartDpC->GetNameEx( &str, false );

		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, SS_CBM_ADDSTRING, 0, (LPARAM)( str.c_str() ) );
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, (LPARAM)( m_pHM->GetpSmartDpC()->GetIDPtr().MP ) );

		bool bNameAlreadyInRed = false;
		
		// If the product is deleted, we set the name in red.
		if( true == pclSmartDpC->GetIDPtr().MP->IsDeleted() )
		{
			SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, _RED );
			bNameAlreadyInRed = true;
		}
		else
		{
			SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, _BLACK );
		}

		// Check if there is a least on error.
		int iValidity = m_pHM->GetpSmartDpC()->CheckValidity();

		if( false == bNameAlreadyInRed && 
				( CDS_HydroMod::eValidityFlags::evfDpTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooHigh ) 
					|| CDS_HydroMod::eValidityFlags::evfTempTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooHigh ) 
					|| CDS_HydroMod::eValidityFlags::evfTempTooLow == ( iValidity & CDS_HydroMod::eValidityFlags::evfTempTooLow ) ) )
		{
			SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, _RED );
		}

		// If product is locked, we set the name in bold.
		if( false == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) )
		{
			SetFontBold( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, FALSE );
		}
		else
		{
			SetFontBold( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, TRUE );
		}

		// Set dp.
		double dSI = m_pHM->GetpSmartDpC()->GetDp();
		SetCUDouble( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDp, _U_DIFFPRESS, dSI );

		// Check if there is error with the pressure drop accross the valve.
		if( CDS_HydroMod::eValidityFlags::evfDpTooHigh == ( iValidity & CDS_HydroMod::eValidityFlags::evfDpTooHigh ) )
		{
			SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDp, _RED );
			SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDp, _RED );
		}
		else
		{
			SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDp, _BLACK );
			SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDp, _BLACK );
		}

		// HYS-1914: Dp min is the minimum pressure drop when the valve is fully open (Computed with design flow and Kvs).
		dSI = m_pHM->GetpSmartDpC()->GetDpMin();
		SetCUDouble( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDpMin, _U_DIFFPRESS, dSI );

		//CTableSet *pclTableSet = dynamic_cast<CTableSet *>( m_pTADB->Get( pclSmartDpC->GetTableSetID() ).MP );
		CDB_RuledTable *pclRuledTable = dynamic_cast<CDB_RuledTable *>( pclSmartDpC->GetDpSensorGroupIDPtr().MP );

		// Sets.
		if( false == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) )
		{
			ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets, SS_CBM_RESETCONTENT, 0, 0 );
			ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets, SS_CBM_SETCURSEL, 0, 0 );

			if( /*NULL != pclTableSet ||*/ NULL != pclRuledTable )
			{
				int iCurrentSelected = -1;
				int iLoop = 0;
				IDPTR SetInHMIDPtr = m_pHM->GetpSmartDpC()->GetProductSetIDPtr();

				long iSetsCol = m_BoxSmartDpC.pos.x + eBCol::Val;
				long iSetsRow = m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets;
				CRank clAllSets;

				if( NULL != pclRuledTable )
				{
					//std::set<CDB_Set *> ProductSet;
					//pclTableSet->FindCompatibleSet( &ProductSet, pclSmartDpC->GetIDPtr().ID, _T( "" ) );

					// Sort the result (To display 'Connection set' at the end and sort 'Dp sensor set' in regards to their dpl range.
					for( IDPTR IDPtr = pclRuledTable->GetFirst(); _T( '\0' ) != *IDPtr.ID; IDPtr = pclRuledTable->GetNext( IDPtr.MP ) )
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
				/*
				else
				{
					for( IDPTR IDPtr = pclRuledTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclRuledTable->GetNext() )
					{
						CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( IDPtr.MP );

						if( NULL == pclDpSensor )
						{
							ASSERT_CONTINUE;
						}

						vecAllSets.push_back( pclDpSensor );
					}
				}
				*/

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

					ComboBoxSendMessage( iSetsCol, iSetsRow, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)strName );

					// Save the pointer of the set object.
					_AddComboBoxParam( _BoxType::ebSmartDpC, iSetsCol, iSetsRow, (LPARAM)( pclProductSet ) );

					if( -1 == iCurrentSelected && NULL != pclDpSensor )
					{
						// Be default we take the first set available in the combo.
						// But we verify also if it's a good one (Prefer to take a set with the correct Dpl range).
						if( dDpl >= pclDpSensor->GetMinMeasurableDp() && dDpl <= pclDpSensor->GetMaxMeasurableDp() )
						{
							iCurrentSelected = iLoop;
						}
					}

					if( pclProductSet == SetInHMIDPtr.MP )
					{
						// If user has already selected a set, we take this one.
						iCurrentSelected = iLoop;
					}

					iLoop++;
				}

				if( -1 == iCurrentSelected )
				{
					// No found, force selection on the first one.
					iCurrentSelected = 0;
				}

				ComboBoxSendMessage( iSetsCol, iSetsRow, SS_CBM_SETCURSEL, iCurrentSelected, 0 );

				// Allow to update the hydromod.
				// Remark: it is needed because when user set flow for the terminal unit, we select the best smart differential pressure controller and we choose 
				//         the first available Dp sensor that we need to save in the hydromod.
				ChangeSmartDpCSets();
			}
		}
		else
		{
			// Cell is already formated in the "FormatBoxSmartDpC" method.
			CString strSetName = _T("");

			if( NULL != m_pHM->GetpSmartDpC()->GetpProductSet() )
			{
				// Set is already in the hydromod.
				CDB_Product *pclSet = (CDB_Product *)( m_pHM->GetpSmartDpC()->GetpProductSet() );
				strSetName = pclSet->GetName();

				if( NULL != dynamic_cast<CDB_DpSensor *>( pclSet ) )
				{
					CDB_DpSensor *pclDpSensor = (CDB_DpSensor *)( pclSet );
					strSetName = pclDpSensor->GetFullName();
				}
			}

			SetStaticText( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets, strSetName );
			SetFontBold( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets, TRUE );
		}

		// With TSpread and for a native combo box it is not possible to put text color of one row in red and the other rows in black.
		// So, we use 'OnPaint' to refresh the text color of the set combo (See the "_VerifyComboBoxError" method).
		// For the other combo, we don't really use a combobox from TSpread but instead we use the "DlgComboBoxHM" when opening.

		// Refresh the lock picture.
		SetPictureLock( m_BoxSmartDpC.pos.x + eBCol::Lock, m_BoxSmartDpC.pos.y, m_pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) );
	}
	else
	{
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, SS_CBM_ADDSTRING, 0, (LPARAM)( _T(" ") ) );
		ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, SS_CBM_SETCURSEL, 0, 0 );
		SetCellParam( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName, 0 );
	}

	// Set Dp to stabilize.
	SetCUDouble( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDpl, _U_DIFFPRESS, dDpl );

	// Verify if we need to put 'Dpl' column in red.
	int iValidity = m_pHM->GetpSmartDpC()->CheckValidity();

	CString strDplErrorMsg = m_pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor,
			CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dpl );
	
	if( false == strDplErrorMsg.IsEmpty() )
	{
		SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDpl, _RED );
		SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDpl, _RED );
	}
	else
	{
		SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDpl, _BLACK );
		SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iDpl, _BLACK );
	}

	CString strTempErrorMsg = m_pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor,
			CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Temp );
	
	if( false == strTempErrorMsg.IsEmpty() )
	{
		SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets, _RED );
		SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets, _RED );
	}
	else
	{
		SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets, _BLACK );
		SetForeColor( m_BoxSmartDpC.pos.x + eBCol::Unit, m_BoxSmartDpC.pos.y + iRow + m_BoxSmartDpC.iSets, _BLACK );
	}

	// Change picture.
	m_pHM->ResetSchemeIDPtr();
	FormatBoxPicture();
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
	RefreshDispatch( _BoxType::ebSmartDpC );
}

bool CSSheetPanelCirc2::CheckValidity( double dVal )
{
	// Current Cell is m_Row, m_Col.
	// Current Text is m_CellText.
	// Current double is m_CellDouble.
	_BoxType CurrentBox = WhatBox( m_lCol, m_lRow );

	switch( CurrentBox )
	{
		case _BoxType::ebTU:
		{
			long lIndex = ( long )GetCellParam( m_lCol, m_lRow );

			switch( lIndex )
			{
				case _BoxTU::Des:
					break;

				case _BoxTU::Flow:
				case _BoxTU::Power:
				case _BoxTU::SupplyT:
				case _BoxTU::ReturnT:
				case _BoxTU::Dp:
				case _BoxTU::Qref:
				case _BoxTU::DpRef:
				case _BoxTU::Kv:
				case _BoxTU::Cv:
					if( dVal >= 0.0 )
					{
						return true;
					}

					break;

				case _BoxTU::DeltaT:
				{
					double dSI = CDimValue::CUtoSI( _U_DIFFTEMP, dVal );

					if( dSI > 0.0 )
					{
						return true;
					}
				}
				break;
			}
		}
		break;

		case _BoxType::ebBVB:
			break;

		case _BoxType::ebPump:
			if( dVal >= 0.0 )
			{
				return true;
			}

			return false;
			break;

		case _BoxType::ebBVP:
			break;

		case _BoxType::ebCV:
		{
			long lIndex = ( long )GetCellParam( m_lCol, m_lRow );

			if( false == m_pHM->GetpCV()->IsTaCV() )
			{
				if( m_Box.CVKV_Kvs == lIndex )
				{
					if( dVal >= 0.0 && dVal < 10000 )
					{
						return true;
					}

					break;
				}
			}
		}
		break;

		case _BoxType::ebDPC:
			break;

		case _BoxType::ebPrimaryValues:
			break;

		case _BoxType::ebCircuitPrimaryPipe:
			if( m_Box.Pipe_Length == ( m_lRow - m_BoxCircuitPrimaryPipe.pos.y ) )
			{
				if( dVal >= 0.0 )
				{
					return true;
				}
			}

			break;

		case _BoxType::ebCircuitSecondaryPipe:
			if( m_Box.Pipe_Length == ( m_lRow - m_BoxCircuitSecondaryPipe.pos.y ) )
			{
				if( dVal >= 0.0 )
				{
					return true;
				}
			}

			break;

		case _BoxType::ebDistributionPipe:
			if( m_Box.Pipe_Length == ( m_lRow - m_BoxDistributionSupplyPipe.pos.y ) )
			{
				if( dVal >= 0.0 )
				{
					return true;
				}
			}

			break;

		case _BoxType::ebDistributionReturnPipe:
			if( m_Box.Pipe_Length == ( m_lRow - m_BoxDistributionReturnPipe.pos.y ) )
			{
				if( dVal >= 0.0 )
				{
					return true;
				}
			}

			break;

		case _BoxType::ebSecondaryValues:
			if( m_lRow == ( m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature + m_BoxSecondaryValues.pos.y ) )
			{
				if( dVal >= -50.0 )
				{
					return true;
				}
			}

			break;


		case _BoxType::ebRem:
			break;

		case _BoxType::ebNoBox:
			break;

		default:
			break;
	}

	return false;
}

void CSSheetPanelCirc2::ArrowKeyDown( long lFromColumn, long lFromRow )
{
	// Find first editable cell.
	if( lFromRow < 0 )
	{
		lFromRow = 0;
	}

	if( lFromColumn < 0 )
	{
		lFromColumn = 0;
	}

	bool bContinue = true;
	bool bLoop = true;
	long lStartRow = lFromRow + 1;
	long lStopRow = GetMaxRows();
	long lStartColumn = lFromColumn;
	long lStopColumn = GetMaxCols();

	while( true == bContinue )
	{
		for( long lLoopColumn = lStartColumn; lLoopColumn <= lStopColumn && true == bContinue; lLoopColumn++ )
		{
			for( long lLoopRow = lStartRow; lLoopRow <= lStopRow && true == bContinue; lLoopRow++ )
			{
				if( true == IsCellCanbeEdited( lLoopColumn, lLoopRow ) )
				{
					bContinue = false;
					SetActiveCell( lLoopColumn, lLoopRow );
					SetEditMode( true );
				}
			}

			lStartRow = 1;
		}

		if( true == bContinue )
		{
			bContinue = bLoop;
		}

		lStartColumn = 1;
		bLoop = false;
	}
}

void CSSheetPanelCirc2::ArrowKeyUp( long lFromColumn, long lFromRow )
{
	// Find first editable cell.
	if( lFromRow <= 0 )
	{
		lFromRow = GetMaxRows();
	}

	if( lFromColumn < 0 )
	{
		lFromColumn = GetMaxCols();
	}

	bool bContinue = true;
	bool bLoop = true;
	long lStartRow = lFromRow - 1;
	long lStopRow = 0;
	long lStartColumn = lFromColumn;
	long lStopColumn = 0;

	while( true == bContinue )
	{
		for( long lLoopColumn = lStartColumn; lLoopColumn >= lStopColumn && true == bContinue; lLoopColumn-- )
		{
			for( long lLoopRow = lStartRow; lLoopRow >= lStopRow && true == bContinue; lLoopRow-- )
			{
				if( true == IsCellCanbeEdited( lLoopColumn, lLoopRow ) )
				{
					bContinue = false;
					SetActiveCell( lLoopColumn, lLoopRow );
					SetEditMode( true );
				}
			}

			lStartRow = GetMaxRows();
		}

		if( true == bContinue )
		{
			bContinue = bLoop;
		}

		lStartColumn = GetMaxCols();
		bLoop = false;
	}
}

void CSSheetPanelCirc2::CellClicked( long lColumn, long lRow )
{
	// Do a first test on the remark box. It's to have possibility to enter in edit mode even if we not click on the first column
	// of all ones for remarks field.
	_BoxType eCurrentBox = WhatBox( lColumn, lRow );

	// "Remark" box.
	if( _BoxType::ebRem == eCurrentBox )
	{
		if( lColumn >= m_BoxRem.pos.x && lColumn <= ( m_BoxRem.pos.x + 3 ) )
		{
			SetActiveCell( m_BoxRem.pos.x, lRow );
			SetEditMode( true );
			return;
		}
	}

	if( true == m_pHM->GetpPrjParam()->IsFreezed() )
	{
		return;
	}

	// "Control valve" box.
	if( eCurrentBox == _BoxType::ebCV )
	{
		// If user has chosen to work with Kvs values instead of IMI Hhydronic Engineering valves...
		if( false == m_pHM->GetpCV()->IsTaCV() )
		{
			// When user has chosen "On-Off" for the control type, there is no "Kvs max" row (This is why "-1").
			int iPos = ( true == m_pHM->GetpCV()->IsOn_Off() ) ? ( m_BoxCv.pos.y + m_Box.CVKV_Kvs - 1 ) : ( m_BoxCv.pos.y + m_Box.CVKV_Kvs );

			if( lRow == iPos )
			{
				// If user click in the left part of spanned cell, transform this combo in a EditCell
				// back conversion will be done when the user leave cell.
				if( false == m_BoxCv.bKvEditMode && ( lColumn == m_BoxCv.pos.x + eBCol::Val || lColumn == m_BoxCv.pos.x + eBCol::Unit ) )
				{
					m_BoxCv.bKvEditMode = true;
					CString str = WriteCUDouble( _C_KVCVCOEFF, m_pHM->GetpCV()->GetKvs() );
					FormatEditDouble( m_BoxCv.pos.x + eBCol::Val, iPos, str );
					SetCellParam( m_BoxCv.pos.x + eBCol::Val, iPos, m_Box.CVKV_Kvs );
					SetEditMode( true );
					return;
				}
			}
		}
	}

	// Don't recompute all if we are just opening a combo
	long lAnchorRow, lAnchorCol, lNumCols, lNumRows;

	if( SS_SPAN_NO == GetCellSpan( lColumn, lRow, &lAnchorCol, &lAnchorRow, &lNumCols, &lNumRows ) )
	{
		lAnchorRow = lRow;
		lAnchorCol = lColumn;
	}

	if( SS_TYPE_COMBOBOX == GetCellTypeW( lAnchorCol, lAnchorRow ) )
	{
		return;
	}

	bool bComputeAll = false;
	bool bSkip = false;

	// "Balancing valve: BV-P" box.
	if( false == bSkip && lColumn == m_BoxBvP.pos.x + eBCol::Butt )
	{
		// Check if user has clicked on the checkbox located at the left of the box title.
		// Remark: this checkbox exists if the primary balancing valve is optional and allows to the user to add or remove the valve.
		if( lRow == m_BoxBvP.pos.y && NULL != m_pHM->GetpSch() && true == m_pHM->GetpSch()->IsAnchorPtOptional( CAnchorPt::BV_P ) )
		{
			if( NULL != m_pHM->GetpBv() )
			{
				// Primary BV exist, remove it.
				// Before removing unlock the valve.
				m_pHM->SetLock( CDS_HydroMod::eBVprim, false, false );
				m_pHM->DeleteBv( m_pHM->GetppBv() );
			}
			else
			{
				// Primary BV doesn't exist, add it.
				m_pHM->CreateBv( CDS_HydroMod::eHMObj::eBVprim );
				m_pHM->GetpBv()->SetQ( m_pHM->GetQ() );
			}

			m_pHM->ComputeHM( CDS_HydroMod::eceResize );

			// Redraw BV-P box.
			FormatBoxBV( CDS_HydroMod::eBVprim );
			SetActiveCell( 0, 0 );

			bComputeAll = true;
			bSkip = true;
		}
		
		// Check if user has clicked on the plus or minus button located at the left of "Name" title.
		// Remark: this button allows to show or hide other information about the primary balancing valve (Type, connection, version and PN).
		if( false == bSkip && lRow == m_BoxBvP.pos.y + m_BoxBvP.BV_Name )
		{
			m_BoxBvP.bExt ^= 1;

			// Redraw BV-P box.
			FormatBoxBV( CDS_HydroMod::eBVprim );
			SetActiveCell( 0, 0 );
			bSkip = true;
		}
	}

	// "Balancing valve: BV-Sec" box.
	if( false == bSkip && lColumn == m_BoxBvS.pos.x + eBCol::Butt )
	{
		// Check if user has clicked on the checkbox located at the left of the box title.
		// Remark: this checkbox exists if the secondary balancing valve is optional and allows to the user to add or remove the valve.
		if( lRow == m_BoxBvS.pos.y && NULL != m_pHM->GetpSch() && m_pHM->GetpSch()->IsAnchorPtOptional( CAnchorPt::BV_S ) )
		{
			if( NULL != m_pHM->GetpSecBv() )
			{
				// Secondary BV exist, remove it.
				// Before removing unlock the valve.
				m_pHM->SetLock( CDS_HydroMod::eBVsec, false, false );
				m_pHM->DeleteBv( m_pHM->GetppSecBv() );
			}
			else
			{
				// Secondary BV doesn't exist, add it.
				m_pHM->CreateBv( CDS_HydroMod::eHMObj::eBVsec );
				m_pHM->GetpSecBv()->SetQ( m_pHM->GetpTermUnit()->GetQ() );
			}

			m_pHM->ComputeHM( CDS_HydroMod::eceResize );

			// Redraw BV-S box.
			FormatBoxBV( CDS_HydroMod::eBVsec );
			SetActiveCell( 0, 0 );

			bComputeAll = true;
			bSkip = true;
		}

		// Check if user has clicked on the plus or minus button located at the left of "Name" title.
		// Remark: this button allows to show or hide other information about the secondary balancing valve (Type, connection, version and PN).
		if( false == bSkip && NULL != m_pHM->GetpSecBv() && lRow == m_BoxBvS.pos.y + m_BoxBvS.BV_Name )
		{
			m_BoxBvS.bExt ^= 1;

			// Redraw BV-S box.
			FormatBoxBV( CDS_HydroMod::eBVsec );
			SetActiveCell( 0, 0 );

			bSkip = true;
		}
	}
	
	// "Shut-off valve" box.
	if( false == bSkip && lColumn == m_BoxShutoffValve.pos.x + eBCol::Butt )
	{
		// Check if user has clicked on the checkbox located at the left of the box title.
		// Remark: this checkbox exists if the shut-off valve is optional and allows to the user to add or remove the valve.
		if( lRow == m_BoxShutoffValve.pos.y && NULL != m_pHM->GetpSch() && true == m_pHM->GetpSch()->IsAnchorPtOptional( CAnchorPt::ShutoffValve )
				&& NULL != dynamic_cast<CDS_Hm2W*>( m_pHM ) )
		{
			CDS_Hm2W *pHm2W = dynamic_cast<CDS_Hm2W*>( m_pHM );
			CDS_HydroMod::eHMObj eHMShutoffValveLoc = CDS_HydroMod::eNone;

			if( NULL != m_pHM->GetpSch() )
			{
				if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMShutoffValveLoc = CDS_HydroMod::eShutoffValveSupply;
				}
				else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMShutoffValveLoc = CDS_HydroMod::eShutoffValveReturn;
				}
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

				// Redraw Shutoff valve box.
				FormatBoxSV();
				SetActiveCell( 0, 0 );

				bComputeAll = true;
				bSkip = true;
			}
		}

		// Check if user has clicked on the plus or minus button located at the left of "Name" title.
		// Remark: this button allows to show or hide other information about the shut-off valve (Type, connection and version).
		if( false == bSkip && lRow == m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name )
		{
			CDS_HydroMod::eHMObj eHMShutoffValveLoc = CDS_HydroMod::eNone;

			if( NULL != m_pHM->GetpSch() )
			{
				if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMShutoffValveLoc = CDS_HydroMod::eShutoffValveSupply;
				}
				else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
				{
					eHMShutoffValveLoc = CDS_HydroMod::eShutoffValveReturn;
				}
			}

			if( NULL != m_pHM->GetpShutoffValve( eHMShutoffValveLoc ) )
			{
				m_BoxShutoffValve.bExt ^= 1;

				// Redraw shut-off valve box.
				FormatBoxSV();
				SetActiveCell( 0, 0 );
				bSkip = true;
			}
		}
	}
	
	// "Balancing valve: BV-B" box.
	if( false == bSkip && lColumn == m_BoxBvB.pos.x + eBCol::Butt )
	{
		// Check if user has clicked on the tristate checkbox located at the left of the box title.
		// Remark: this checkbox exists if the bypass balancing valve is optional and allows to the user to add or remove the valve.
		if( lRow == m_BoxBvB.pos.y )
		{
			// Roll on choice: Auto, never or always.
			if( ( ( CDS_Hm3W * )m_pHM )->GetUseBypBv() < CDS_HydroMod::eUseProduct::Always )
			{
				( ( CDS_Hm3W * )m_pHM )->SetUseBypBv( ( CDS_HydroMod::eUseProduct )( ( ( CDS_Hm3W * )m_pHM )->GetUseBypBv() + 1 ) );
			}
			else
			{
				( ( CDS_Hm3W * )m_pHM )->SetUseBypBv( CDS_HydroMod::eUseProduct::Auto );
			}

			// 2016-09-15: This method is called in 'FormatBoxBV'.
			// _DrawCheckbox( lColumn, lRow, ( ( CDS_Hm3W * )m_pHM )->GetUseBypBv() );

			// Redraw BV-B box.
			FormatBoxBV( CDS_HydroMod::eBVbyp );
			SetActiveCell( 0, 0 );

			bComputeAll = true;
			bSkip = true;
		}

		// Check if user has clicked on the plus or minus button located at the left of "Name" title.
		// Remark: this button allows to show or hide other information about the bypass balancing valve (Type, connection and version).
		if( false == bSkip && lRow == m_BoxBvB.pos.y + m_BoxBvB.BV_Name )
		{
			m_BoxBvB.bExt ^= 1;

			// Redraw BV-B box.
			FormatBoxBV( CDS_HydroMod::eBVbyp );
			SetActiveCell( 0, 0 );
			bSkip = true;
		}
	}

	// "Dp Controller" box.
	if( false == bSkip && lColumn == m_BoxDpC.pos.x + eBCol::Butt )
	{
		// Check if user has clicked on the plus or minus button located at the left of "Name" title.
		// Remark: this button allows to show or hide other information about the Dp controller (Type, connection and PN).
		if( lRow == m_BoxDpC.pos.y + m_Box.DpC_Name )
		{
			m_BoxDpC.bExt ^= 1;

			// Redraw DpC box.
			FormatBoxDpC();
			SetActiveCell( 0, 0 );
			bSkip = true;
		}
	}

	// "Control valve" box.
	if( false == bSkip && lColumn == m_BoxCv.pos.x + eBCol::Butt )
	{
		if( NULL != m_pHM->GetpCV() && true == m_pHM->GetpCV()->IsTaCV() )
		{
			// Check if user has clicked on the plus or minus button located at the left of "Name" title.
			// Remark: this button allows to show or hide other information about the control valve (Type, connection, version and PN).
			if( lRow == m_BoxCv.pos.y + m_Box.CVTA_Name )
			{
				m_BoxCv.bExt ^= 1;

				// Redraw CV box.
				FormatBoxCVTA();
				SetActiveCell( 0, 0 );
				bSkip = true;
			}
		}
	}
	
	// "Comb. Dp controller, ctrl. & bal. valve" box.
	if( false == bSkip && lColumn == m_BoxDpCBCV.pos.x + eBCol::Butt )
	{
		if( NULL != m_pHM->GetpCV() && eb3True == m_pHM->GetpCV()->IsDpCBCV() )
		{
			// Check if user has clicked on the plus or minus button located at the left of "Name" title.
			// Remark: this button allows to show or hide other information about the combined Dp controller, control & balancing valve (Connection, version and PN).
			if( lRow == m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name )
			{
				m_BoxDpCBCV.bExt ^= 1;

				// Redraw DpCBCV box.
				FormatBoxDpCBCV();
				SetActiveCell( 0, 0 );
				bSkip = true;
			}
		}
	}

	// "Smart control valve" box.
	if( false == bSkip && lColumn == m_BoxSmartControlValve.pos.x + eBCol::Butt )
	{
		if( NULL != m_pHM->GetpSmartControlValve() )
		{
			// Check if user has clicked on the plus or minus button located at the left of "Name" title.
			// Remark: this button allows to show or hide other information about the smart control valve (Body material, connection and PN).
			if( lRow == m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName )
			{
				m_BoxSmartControlValve.bExt ^= 1;

				// Redraw smart control valve box.
				FormatBoxSmartControlValve();
				SetActiveCell( 0, 0 );
				bSkip = true;
			}
		}
	}

	// "Smart differential pressure controller" box.
	if( false == bSkip && lColumn == m_BoxSmartDpC.pos.x + eBCol::Butt )
	{
		if( NULL != m_pHM->GetpSmartDpC() )
		{
			// Check if user has clicked on the plus or minus button located at the left of "Name" title.
			// Remark: this button allows to show or hide other information about the smart differential pressure controller (Body material, connection and PN).
			if( lRow == m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName )
			{
				m_BoxSmartDpC.bExt ^= 1;

				// Redraw smart differential pressure controller box.
				FormatBoxSmartDpC();
				SetActiveCell( 0, 0 );
				bSkip = true;
			}
		}
	}

	// Check for the "Circuit primary pipe" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxCircuitPrimaryPipe.pos.x + eBCol::Lock && lRow == m_BoxCircuitPrimaryPipe.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe ) )
		{
			// Unlock recall best pipe.
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe, false );
			m_pHM->GetpCircuitPrimaryPipe()->SetIDPtr( m_pHM->GetpCircuitPrimaryPipe()->GetBestPipeIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe, true );
		}

		RefreshBoxPipe( &m_BoxCircuitPrimaryPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
		bSkip = true;
	}

	// Check for the "Circuit secondary pipe" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxCircuitSecondaryPipe.pos.x + eBCol::Lock && lRow == m_BoxCircuitSecondaryPipe.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe ) )
		{
			// Unlock recall best Pipe
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe, false );
			m_pHM->GetpCircuitSecondaryPipe()->SetIDPtr( m_pHM->GetpCircuitSecondaryPipe()->GetBestPipeIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe, true );
		}

		RefreshBoxPipe( &m_BoxCircuitSecondaryPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
		bSkip = true;
	}

	// Check for the "Distribution supply pipe" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxDistributionSupplyPipe.pos.x + eBCol::Lock && lRow == m_BoxDistributionSupplyPipe.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eDistributionSupplyPipe ) )
		{
			// Unlock recall best pipe.
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eDistributionSupplyPipe, false );
			m_pHM->GetpDistrSupplyPipe()->SetIDPtr( m_pHM->GetpDistrSupplyPipe()->GetBestPipeIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eDistributionSupplyPipe, true );
		}

		RefreshBoxPipe( &m_BoxDistributionSupplyPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
		bSkip = true;
	}

	// Check for the "Distribution return pipe" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxDistributionReturnPipe.pos.x + eBCol::Lock && lRow == m_BoxDistributionReturnPipe.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eDistributionReturnPipe ) )
		{
			// Unlock recall best pipe.
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eDistributionReturnPipe, false );
			m_pHM->GetpDistrReturnPipe()->SetIDPtr( m_pHM->GetpDistrReturnPipe()->GetBestPipeIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eDistributionReturnPipe, true );
		}

		RefreshBoxPipe( &m_BoxDistributionReturnPipe, InputValuesOrigin::InputValuesComeFromHydroMod );
		bSkip = true;
	}

	// Check for the "Balancing valve: BV-P" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxBvP.pos.x + eBCol::Lock && lRow == m_BoxBvP.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eBVprim ) )
		{
			// Unlock recall best Bv.
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eBVprim, false );
			m_pHM->GetpBv()->SetIDPtr( m_pHM->GetpBv()->GetBestBvIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eBVprim, true );
		}

		ChangeBvType( m_pHM->GetpBv() );
		bSkip = true;
	}

	// Check for the "Balancing valve: BV-S" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxBvS.pos.x + eBCol::Lock && lRow == m_BoxBvS.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eBVsec ) )
		{
			// Unlock recall best Bv.
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eBVsec, false );
			m_pHM->GetpSecBv()->SetIDPtr( m_pHM->GetpSecBv()->GetBestBvIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eBVsec, true );
		}

		ChangeBvType( m_pHM->GetpSecBv() );
		bSkip = true;
	}

	// Check for the "Balancing valve: BV-B" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxBvB.pos.x + eBCol::Lock && lRow == m_BoxBvB.pos.y )
	{
		if( m_pHM->IsLocked( CDS_HydroMod::eHMObj::eBVbyp ) )
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eBVbyp, false );
			m_pHM->GetpBypBv()->SetIDPtr( m_pHM->GetpBypBv()->GetBestBvIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eBVbyp, true );
		}

		ChangeBvType( m_pHM->GetpBypBv() );
		bSkip = true;
	}

	// Check for the "Dp Controller" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxDpC.pos.x + eBCol::Lock && lRow == m_BoxDpC.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eDpC ) )
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eDpC, false );
			m_pHM->GetpDpC()->SetIDPtr( m_pHM->GetpDpC()->GetBestDpCIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eDpC, true );
		}

		ChangeDpC();
		bSkip = true;
	}

	// Check for the "Control valve" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxCv.pos.x + eBCol::Lock && lRow == m_BoxCv.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false );

			if( true == m_pHM->GetpCV()->IsTaCV() )
			{
				m_pHM->GetpCV()->SetCvIDPtr( m_pHM->GetpCV()->GetBestCvIDPtr() );
			}
			else
			{
				m_pHM->GetpCV()->SetKvs( m_pHM->GetpCV()->GetBestKvs() );
			}

			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, true );
		}

		RefreshBoxCV( InputValuesOrigin::InputValuesComeFromHydroMod );
		bSkip = true;
	}

	// Check for the "Comb. Dp controller, ctrl. & bal. valve" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxDpCBCV.pos.x + eBCol::Lock && lRow == m_BoxDpCBCV.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eCV ) )
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, false );
			m_pHM->GetpCV()->SetCvIDPtr( m_pHM->GetpCV()->GetBestCvIDPtr() );
			m_pHM->GetpCV()->SetKvs( m_pHM->GetpCV()->GetBestKvs() );

			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eCV, true );
		}

		FormatBoxDpCBCV();
		bSkip = true;
	}

	// Check for the "Shut-off valve" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxShutoffValve.pos.x + eBCol::Lock && lRow == m_BoxShutoffValve.pos.y )
	{
		CDS_HydroMod::eHMObj eHMObj = CDS_HydroMod::eNone;
			
		if( NULL != m_pHM->GetpSch() )
		{
			if( ShutoffValveLoc::ShutoffValveLocSupply == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveSupply;
			}
			else if( ShutoffValveLoc::ShutoffValveLocReturn == m_pHM->GetpSch()->GetShutoffValveLoc() )
			{
				eHMObj = CDS_HydroMod::eShutoffValveReturn;
			}
		}

		if( CDS_HydroMod::eNone != eHMObj )
		{
			if( true == m_pHM->IsLocked( eHMObj ) )
			{
				m_pHM->SetLock( eHMObj, false );
				m_pHM->GetpShutoffValve( eHMObj )->SetIDPtr( m_pHM->GetpShutoffValve( eHMObj )->GetBestIDPtr() );
				bComputeAll = true;
			}
			else
			{
				m_pHM->SetLock( eHMObj, true );
			}

			ChangeSV();
			bSkip = true;
		}
	}

	// Check for the "Smart control valve" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxSmartControlValve.pos.x + eBCol::Lock && lRow == m_BoxSmartControlValve.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartControlValve ) )
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, false );
			m_pHM->GetpSmartControlValve()->SetIDPtr( m_pHM->GetpSmartControlValve()->GetBestIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartControlValve, true );
		}

		ChangeSmartControlValve();
		bSkip = true;
	}

	// Check for the "Smart differential pressure controller" box if user has clicked on the padlock at the left of the title.
	// Remark: this padlock allows to lock or unlock the product belonging to the current group.
	if( false == bSkip && lColumn == m_BoxSmartDpC.pos.x + eBCol::Lock && lRow == m_BoxSmartDpC.pos.y )
	{
		if( true == m_pHM->IsLocked( CDS_HydroMod::eHMObj::eSmartDpC ) )
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, false );
			m_pHM->GetpSmartDpC()->SetIDPtr( m_pHM->GetpSmartDpC()->GetBestIDPtr() );
			bComputeAll = true;
		}
		else
		{
			m_pHM->SetLock( CDS_HydroMod::eHMObj::eSmartDpC, true );
		}

		FormatBoxSmartDpC();
		bSkip = true;
	}

	// Check for the "Circuit primary pipe" box if user has clicked on the three point button at the left of the accessories.
	// Remark: this button allows to launch a dialog to add accessories to the pipe.
	if( false == bSkip && lColumn == m_BoxCircuitPrimaryPipe.pos.x + eBCol::Butt && lRow == ( m_BoxCircuitPrimaryPipe.pos.y + m_Box.Pipe_Acc ) )
	{
		bool bChangeDone = false;
		CDlgAccessory dlg( CTADatabase::FilterSelection::ForHMCalc );
		dlg.Display( m_pHM, m_pHM->GetpCircuitPrimaryPipe(), &bChangeDone );
		bComputeAll = bChangeDone;
		bSkip = true;
	}

	// Check for the "Circuit secondary pipe" box if user has clicked on the three point button at the left of the accessories.
	// Remark: this button allows to launch a dialog to add accessories to the pipe.
	if( false == bSkip && lColumn == m_BoxCircuitSecondaryPipe.pos.x + eBCol::Butt && lRow == ( m_BoxCircuitSecondaryPipe.pos.y + m_Box.Pipe_Acc ) )
	{
		bool bChangeDone = false;
		CDlgAccessory dlg( CTADatabase::FilterSelection::ForHMCalc );
		dlg.Display( m_pHM, m_pHM->GetpCircuitSecondaryPipe(), &bChangeDone );
		bComputeAll = bChangeDone;
		bSkip = true;
	}

	// Check for the "Distribution supply pipe" box if user has clicked on the three point button at the left of the accessories.
	// Remark: this button allows to launch a dialog to add accessories to the pipe.
	if( false == bSkip && lColumn == m_BoxDistributionSupplyPipe.pos.x + eBCol::Butt && lRow == ( m_BoxDistributionSupplyPipe.pos.y + m_Box.Pipe_Acc ) )
	{
		bool bChangeDone = false;
		CDlgAccessory dlg( CTADatabase::FilterSelection::ForHMCalc );
		dlg.Display( m_pHM, m_pHM->GetpDistrSupplyPipe(), &bChangeDone );
		bComputeAll = bChangeDone;
		bSkip = true;
	}

	// Check for the "Distribution return pipe" box if user has clicked on the three point button at the left of the accessories.
	// Remark: this button allows to launch a dialog to add accessories to the pipe.
	if( false == bSkip && lColumn == m_BoxDistributionReturnPipe.pos.x + eBCol::Butt && lRow == ( m_BoxDistributionReturnPipe.pos.y + m_Box.Pipe_Acc ) )
	{
		bool bChangeDone = false;
		CDlgAccessory dlg( CTADatabase::FilterSelection::ForHMCalc );
		dlg.Display( m_pHM, m_pHM->GetpDistrReturnPipe(), &bChangeDone );
		bComputeAll = bChangeDone;
		bSkip = true;
	}

	bool bRefreshToDispatch = true;

	if( false == bSkip && false == m_pHM->GetpPrjParam()->IsFreezed() 
			&& ( true == IsCellProperty( lColumn, lRow, CellText ) || true == IsCellProperty( lColumn, lRow, CellDouble ) ) )
	{
		// HYS-1422 : The description value field is a merged cell. When we click on the unit cell we have to activate
		// the val cell to get the right cell with which we work.
		if( _BoxType::ebTU == eCurrentBox && lRow == m_BoxTu.pos.y + m_Box.TU_Des )
		{
			if( lColumn >= m_BoxTu.pos.x && lColumn <= ( m_BoxTu.pos.x + eBCol::Unit ) )
			{
				SetActiveCell( m_BoxTu.pos.x + eBCol::Val, lRow );
			}
		}
		
		SetEditMode( true );
		bSkip = true;
		bRefreshToDispatch = false;
	}

	if( false == bSkip )
	{
		bRefreshToDispatch = false;
	}

	if( false == m_bPrinting && true == bComputeAll )
	{
		BeginWaitCursor();
		m_pHM->ComputeAll();
		EndWaitCursor();
	}

	if( true == bRefreshToDispatch )
	{
		RefreshDispatch();
	}
}

void CSSheetPanelCirc2::EditChange( long lColumn, long lRow )
{
	CString str = GetCellText( lColumn, lRow );

	// HYS-1307: Enable to use "-" sign for temperature
	bool bIsForTemperature = false;

	if( ( m_lRow == ( m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature ) && m_lCol == ( m_BoxSecondaryValues.pos.x + eBCol::Val ) ) )
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
				// Number is incomplete ?
				// Try to remove partial engineering notation if exist
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

				// Check validity field
				if( true == CheckValidity( dValue ) )
				{
					m_dCellDouble = dValue;
					m_strCellText = str;
					m_bCellModified = true;
				}
				// We keep the last double value when checkValidity is on error. NO CORRECT !
				break;
		}
	}
}

void CSSheetPanelCirc2::ComboDropDown( long lColumn, long lRow )
{
	_BoxType CurrentBox = WhatBox( lColumn, lRow );

	switch( CurrentBox )
	{
		case _BoxType::ebTU:

			if( lRow == ( m_BoxTu.pos.y + m_Box.TU_Flow ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_TU_Flow, lColumn, lRow );
			}
			else if( lRow == ( m_BoxTu.pos.y + m_BoxTu.m_iDpRow ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_TU_Dp, lColumn, lRow );
			}

			break;

		case _BoxType::ebBVB:

			if( lRow == ( m_BoxBvB.pos.y + m_BoxBvB.BV_Name ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvBName, lColumn, lRow );
			}

			if( true == m_BoxBvB.bExt )
			{
				if( lRow == ( m_BoxBvB.pos.y + m_BoxBvB.BV_Name + m_BoxBvB.BV_Type ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvBType, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvB.pos.y + m_BoxBvB.BV_Name + m_BoxBvB.BV_Connect ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvBConnection, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvB.pos.y + m_BoxBvB.BV_Name + m_BoxBvB.BV_Version ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvBVersion, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvB.pos.y + m_BoxBvB.BV_Name + m_BoxBvB.BV_PN ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvBPN, lColumn, lRow );
				}
			}

			break;

		case _BoxType::ebBVP:

			if( lRow == ( m_BoxBvP.pos.y + m_BoxBvP.BV_Name ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvPName, lColumn, lRow );
			}

			if( true == m_BoxBvP.bMeasuringValve )
			{
				if( lRow == ( m_BoxBvP.pos.y + m_BoxBvP.BV_Loc ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvLoc, lColumn, lRow );
				}
			}

			if( true == m_BoxBvP.bExt )
			{
				if( lRow == ( m_BoxBvP.pos.y + m_BoxBvP.BV_Name + m_BoxBvP.BV_Type ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvPType, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvP.pos.y + m_BoxBvP.BV_Name + m_BoxBvP.BV_Connect ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvPConnection, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvP.pos.y + m_BoxBvP.BV_Name + m_BoxBvP.BV_Version ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvPVersion, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvP.pos.y + m_BoxBvP.BV_Name + m_BoxBvP.BV_PN ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvPPN, lColumn, lRow );
				}
			}

			break;

		case _BoxType::ebBVS:

			if( lRow == ( m_BoxBvS.pos.y + m_BoxBvS.BV_Name ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvSName, lColumn, lRow );
			}

			if( true == m_BoxBvS.bExt )
			{
				if( lRow == ( m_BoxBvS.pos.y + m_BoxBvS.BV_Name + m_BoxBvS.BV_Type ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvSType, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvS.pos.y + m_BoxBvS.BV_Name + m_BoxBvS.BV_Connect ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvSConnection, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvS.pos.y + m_BoxBvS.BV_Name + m_BoxBvS.BV_Version ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvSVersion, lColumn, lRow );
				}

				if( lRow == ( m_BoxBvS.pos.y + m_BoxBvS.BV_Name + m_BoxBvS.BV_PN ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_BvSPN, lColumn, lRow );
				}
			}

			break;

		case _BoxType::ebCV:

			if( lRow == ( m_BoxCv.pos.y + m_Box.CV_Loc ) && true == m_BoxCv.bLocalisationExist )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CvLoc, lColumn, lRow );
			}
			else if( false == m_pHM->GetpCV()->IsTaCV() )
			{
				int iPos = ( true == m_pHM->GetpCV()->IsOn_Off() ) ? ( m_BoxCv.pos.y + m_Box.CVKV_Kvs - 1 ) : ( m_BoxCv.pos.y + m_Box.CVKV_Kvs );

				if( lRow == iPos )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CvKvs, lColumn, lRow );
				}
			}
			else
			{
				if( lRow == ( m_BoxCv.pos.y + m_Box.CVTA_Name ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_Cv, lColumn, lRow );
				}

				if( lRow == ( m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator ) )
				{
					m_pHMSelectActuator->OpenDlgSelectActr( m_pHM, m_BoxCv.pos.x + eBCol::Val, m_BoxCv.pos.y + m_BoxCv.rActuator );
				}

				if( true == m_BoxCv.bExt )
				{
					if( lRow == ( m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Conn ) )
					{
						m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CvConnection, lColumn, lRow );
					}

					if( lRow == ( m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_Vers ) )
					{
						m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CvVersion, lColumn, lRow );
					}

					if( lRow == ( m_BoxCv.pos.y + m_Box.CVTA_Name + m_Box.CVTA_PN ) )
					{
						m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CvPN, lColumn, lRow );
					}
				}
			}

			break;

		case _BoxType::ebDPC:

			if( lRow == ( m_BoxDpC.pos.y + m_Box.DpC_Name ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DpC, lColumn, lRow );
			}

			if( true == m_BoxDpC.bExt )
			{
				if( lRow == ( m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Type ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DpCType, lColumn, lRow );
				}

				if( lRow == ( m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_Connect ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DpCConnection, lColumn, lRow );
				}

				if( lRow == ( m_BoxDpC.pos.y + m_Box.DpC_Name + m_Box.DpC_PN ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DpCPN, lColumn, lRow );
				}
			}

			break;

		case _BoxType::ebShutoffValve:

			if( lRow == ( m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_ShutOffValve, lColumn, lRow );
			}

			if( true == m_BoxShutoffValve.bExt )
			{
				if( lRow == ( m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Type ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_ShutOffValveType, lColumn, lRow );
				}

				if( lRow == ( m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Connect ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_ShutOffValveConnection, lColumn, lRow );
				}

				if( lRow == ( m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_Version ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_ShutOffValveVersion, lColumn, lRow );
				}

				if( lRow == ( m_BoxShutoffValve.pos.y + m_BoxShutoffValve.SV_Name + m_BoxShutoffValve.SV_PN ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_ShutOffValvePN, lColumn, lRow );
				}
			}

			break;

		case _BoxType::ebDpCBCValve:

			if( lRow == ( m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DpCBCValve, lColumn, lRow );
			}

			if( lRow == ( m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Actuator ) )
			{
				m_pHMSelectActuator->OpenDlgSelectActr( m_pHM, m_BoxDpCBCV.pos.x + eBCol::Val, m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Actuator );
			}

			if( true == m_BoxDpCBCV.bExt )
			{
				if( lRow == ( m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Connect ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DpCBCValveConnection, lColumn, lRow );
				}

				if( lRow == ( m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_Version ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DpCBCValveVersion, lColumn, lRow );
				}

				if( lRow == ( m_BoxDpCBCV.pos.y + m_BoxDpCBCV.DpCBCV_Name + m_BoxDpCBCV.DpCBCV_PN ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DpCBCValvePN, lColumn, lRow );
				}
			}

			break;

		case _BoxType::ebSmartControlValve:

			if( lRow == ( m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_SmartControlValve, lColumn, lRow );
			}

			if( true == m_BoxSmartControlValve.bExt )
			{
				if( lRow == ( m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iBodyMaterial ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_SmartControlValveBody, lColumn, lRow );
				}

				if( lRow == ( m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iConnect ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_SmartControlValveConnection, lColumn, lRow );
				}

				if( lRow == ( m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName + m_BoxSmartControlValve.iPN ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_SmartControlValvePN, lColumn, lRow );
				}
			}

			break;

		case _BoxType::ebSmartDpC:

			if( lRow == ( m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_SmartDpC, lColumn, lRow );
			}

			if( true == m_BoxSmartDpC.bExt )
			{
				if( lRow == ( m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iBodyMaterial ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_SmartDpCBody, lColumn, lRow );
				}

				if( lRow == ( m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iConnect ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_SmartDpCConnection, lColumn, lRow );
				}

				if( lRow == ( m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName + m_BoxSmartDpC.iPN ) )
				{
					m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_SmartDpCPN, lColumn, lRow );
				}
			}

			break;

		case _BoxType::ebPrimaryValues:
			break;

		case _BoxType::ebCircuitPrimaryPipe:

			if( lRow == ( m_BoxCircuitPrimaryPipe.pos.y + m_Box.Pipe_Serie ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSerie, lColumn, lRow );
			}

			if( lRow == ( m_BoxCircuitPrimaryPipe.pos.y + m_Box.Pipe_Size ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSize, lColumn, lRow );
			}

			break;

		case _BoxType::ebCircuitSecondaryPipe:

			if( lRow == ( m_BoxCircuitSecondaryPipe.pos.y + m_Box.Pipe_Serie ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CircuitSecondaryPipeSerie, lColumn, lRow );
			}

			if( lRow == ( m_BoxCircuitSecondaryPipe.pos.y + m_Box.Pipe_Size ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_CircuitSecondaryPipeSize, lColumn, lRow );
			}

			break;

		case _BoxType::ebDistributionPipe:

			if( lRow == ( m_BoxDistributionSupplyPipe.pos.y + m_Box.Pipe_Serie ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DistributionSupplyPipeSerie, lColumn, lRow );
			}

			if( lRow == ( m_BoxDistributionSupplyPipe.pos.y + m_Box.Pipe_Size ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DistributionSupplyPipeSize, lColumn, lRow );
			}

			break;

		case _BoxType::ebDistributionReturnPipe:

			if( lRow == ( m_BoxDistributionReturnPipe.pos.y + m_Box.Pipe_Serie ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DistributionReturnPipeSerie, lColumn, lRow );
			}

			if( lRow == ( m_BoxDistributionReturnPipe.pos.y + m_Box.Pipe_Size ) )
			{
				m_pHMSCB->OpenDialogSCB( m_pHM, CDlgComboBoxHM::eSCB::SCB_DistributionReturnPipeSize, lColumn, lRow );
			}

			break;

		case _BoxType::ebRem:
			break;

		case _BoxType::ebNoBox:
			break;

		default:
			break;
	}
}

void CSSheetPanelCirc2::ComboSelChange( long lColumn, long lRow )
{
	// Special case for smart control valve with the "Localization" and "Control mode" combo boxes.
	// These combo boxes are not inherited from the 'CDlgComboBoxHM' as it's the case for all other ones.
	// Here we are using the native combo box of TSpread. So we don't wait that the 'CloseDialogSCB' method is called.
	// With TSpread we receive two messages: "SSM_COMBOSELCHANGE" and "SSM_EDITMODEOFF".

	_BoxType CurrentBox = WhatBox( lColumn, lRow );

	switch( CurrentBox )
	{
		case _BoxType::ebSmartControlValve:
		{
			long lLocationRow = ( false == m_BoxSmartControlValve.bExt ) ? m_BoxSmartControlValve.iLocation : m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iLocation;
			long lControlModeRow = ( false == m_BoxSmartControlValve.bExt ) ? m_BoxSmartControlValve.iControlMode : m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iControlMode;

			// Why we disable this event from TSpread ?
			// Because, when calling the 'ChangeSmartControlValveLocalization' or the 'ChangeSmartControlValveControlMode' methods we will
			// call the 'FormatBoxSmartControlValve'. This last method will first clear all the box. And, because editing the content of a 
			// combo box set the TSpread in an editing mode, TSpread will stop the editing mode and send the event. And in the 
			// 'EditModeOffFpspread' method, we call 'SaveAndCompute', and for these two cases we don't need of it.
			m_bManageEditOffSpreadNotification = false;

			if( m_lRow == m_BoxSmartControlValve.pos.y + lLocationRow )
			{
				int iCurrentSel = ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + lLocationRow, SS_CBM_GETCURSEL, 0, 0 );

				if( ( 0 == iCurrentSel && SmartValveLocalization::SmartValveLocSupply != m_pHM->GetpSmartControlValve()->GetLocalization() )
						|| ( 1 == iCurrentSel && SmartValveLocalization::SmartValveLocReturn != m_pHM->GetpSmartControlValve()->GetLocalization() ) )
				{
					TASApp.OverridedSetRedraw( this, FALSE );

					ChangeSmartControlValveLocalization();

					// Change picture.
					m_pHM->ResetSchemeIDPtr();
					FormatBoxPicture();

					SaveAndCompute();

					ChangeSmartControlValve();

					TASApp.OverridedSetRedraw( this, TRUE );
					Invalidate();
				}
			}
			else if( m_lRow == m_BoxSmartControlValve.pos.y + lControlModeRow )
			{
				int iCurrentSel = ComboBoxSendMessage( m_BoxSmartControlValve.pos.x + eBCol::Val, m_BoxSmartControlValve.pos.y + lControlModeRow, SS_CBM_GETCURSEL, 0, 0 );

				if( ( 0 == iCurrentSel && SmartValveControlMode::SCVCM_Flow != m_pHM->GetpSmartControlValve()->GetControlMode() )
					|| ( 1 == iCurrentSel && SmartValveControlMode::SCVCM_Power != m_pHM->GetpSmartControlValve()->GetControlMode() ) )
				{
					ChangeSmartControlValveControlMode();
					SaveAndCompute();

					FormatBoxSmartControlValve();
				}
			}

			m_bManageEditOffSpreadNotification = true;

			break;
		}

		case _BoxType::ebSmartDpC:
		{
			// Why we disable this event from TSpread ?
			// Because, when calling the 'ChangeSmartDpCLocalization' method we will
			// call the 'FormatBoxSmartDpC'. This last method will first clear all the box. And, because editing the content of a 
			// combo box set the TSpread in an editing mode, TSpread will stop the editing mode and send the event. And in the 
			// 'EditModeOffFpspread' method, we call 'SaveAndCompute', and for these two cases we don't need of it.
			m_bManageEditOffSpreadNotification = false;

			// Test smart differential pressure controller localization.
			long lLocationRow = ( false == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iLocation : m_BoxSmartDpC.iPN + m_BoxSmartDpC.iLocation;

			if( m_lRow == m_BoxSmartDpC.pos.y + lLocationRow )
			{
				int iCurrentSel = ComboBoxSendMessage( m_BoxSmartDpC.pos.x + eBCol::Val, m_BoxSmartDpC.pos.y + lLocationRow, SS_CBM_GETCURSEL, 0, 0 );

				if( ( 0 == iCurrentSel && SmartValveLocalization::SmartValveLocSupply != m_pHM->GetpSmartDpC()->GetLocalization() )
						|| ( 1 == iCurrentSel && SmartValveLocalization::SmartValveLocReturn != m_pHM->GetpSmartDpC()->GetLocalization() ) )
				{
					TASApp.OverridedSetRedraw( this, FALSE );

					ChangeSmartDpCLocalization();

					// Change picture.
					m_pHM->ResetSchemeIDPtr();
					FormatBoxPicture();

					SaveAndCompute();

					ChangeSmartDpC();

					TASApp.OverridedSetRedraw( this, TRUE );
					Invalidate();
				}
			}

			// Test sets.
			long lSetsRow = ( false == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iSets : m_BoxSmartDpC.iPN + m_BoxSmartDpC.iSets;
			lSetsRow += m_BoxSmartDpC.pos.y;
			long lDpSensorCol = m_BoxSmartDpC.pos.x + eBCol::Val;

			if( m_lCol == lDpSensorCol && m_lRow == lSetsRow )
			{
				TASApp.OverridedSetRedraw( this, FALSE );

				ChangeSmartDpCSets();

				// Call also this method because a change of Dp sensor can add/remove error on the Dpl field.
				ChangeSmartDpC();
				
				TASApp.OverridedSetRedraw( this, TRUE );
				Invalidate();
			}

			m_bManageEditOffSpreadNotification = true;

			break;
		}
	}
}

void CSSheetPanelCirc2::TextTipFetch( long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *pbShowTip )
{
	*pbShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;

	CString str;

	// Box:Pipe, Row Length && Row Size
	sBoxPipe *prBoxPipe = NULL;

	if( m_BoxCircuitPrimaryPipe.pos != CPoint( 0, 0 ) && m_BoxCircuitPrimaryPipe.pPipe != NULL )
	{
		if( lColumn == m_BoxCircuitPrimaryPipe.pos.x + eBCol::Val )
		{
			if( lRow == ( m_BoxCircuitPrimaryPipe.pos.y + m_Box.Pipe_Size ) )
			{
				prBoxPipe = &m_BoxCircuitPrimaryPipe;
			}

			if( lRow == ( m_BoxCircuitPrimaryPipe.pos.y + m_Box.Pipe_Length ) )
			{
				str = TASApp.LoadLocalizedString( IDS_PC2_TTCIRCUITPIPEL );
			}
		}
	}

	if( m_BoxCircuitSecondaryPipe.pos != CPoint( 0, 0 ) && m_BoxCircuitSecondaryPipe.pPipe != NULL )
	{
		if( lColumn == m_BoxCircuitSecondaryPipe.pos.x + eBCol::Val )
		{
			if( lRow == ( m_BoxCircuitSecondaryPipe.pos.y + m_Box.Pipe_Size ) )
			{
				prBoxPipe = &m_BoxCircuitSecondaryPipe;
			}

			if( lRow == ( m_BoxCircuitSecondaryPipe.pos.y + m_Box.Pipe_Length ) )
			{
				str = TASApp.LoadLocalizedString( IDS_PC2_TTSECPIPEL );
			}
		}
	}

	if( m_BoxDistributionSupplyPipe.pos != CPoint( 0, 0 ) && m_BoxDistributionSupplyPipe.pPipe != NULL )
	{
		if( lColumn == m_BoxDistributionSupplyPipe.pos.x + eBCol::Val )
		{
			if( lRow == ( m_BoxDistributionSupplyPipe.pos.y + m_Box.Pipe_Size ) )
			{
				prBoxPipe = &m_BoxDistributionSupplyPipe;
			}

			if( lRow == ( m_BoxDistributionSupplyPipe.pos.y + m_Box.Pipe_Length ) )
			{
				// Pay attention: this message is used only for direct return mode. In reverse mode, the supply pipe length is the
				// real length and does not include the return. In the reverse mode, the return length is defined in the distribution
				// return pipe with an specific box.
				if( CDS_HydroMod::ReturnType::Direct == m_pHM->GetReturnType() )
				{
					str = TASApp.LoadLocalizedString( IDS_PC2_TTDISTRIBPIPEL );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_PC2_TTDISTRIBSPIPEL );
				}
			}
		}
	}

	if( m_BoxDistributionReturnPipe.pos != CPoint( 0, 0 ) && m_BoxDistributionReturnPipe.pPipe != NULL )
	{
		if( lColumn == m_BoxDistributionReturnPipe.pos.x + eBCol::Val )
		{
			if( lRow == ( m_BoxDistributionReturnPipe.pos.y + m_Box.Pipe_Size ) )
			{
				prBoxPipe = &m_BoxDistributionReturnPipe;
			}

			if( lRow == ( m_BoxDistributionReturnPipe.pos.y + m_Box.Pipe_Length ) )
			{
				str = TASApp.LoadLocalizedString( IDS_PC2_TTDISTRIBRPIPEL );
			}
		}
	}

	if( NULL != prBoxPipe )
	{
		if( NULL != prBoxPipe->pPipe && NULL != prBoxPipe->pPipe->GetIDPtr().MP && 0.0 != prBoxPipe->pPipe->GetRealQ() )
		{
			// Retrieve linear pressure drop and velocity.
			double dLinDp = 0.0;
			double dVelocity = 0.0;

			if( NULL != dynamic_cast<CDB_Pipe*>( prBoxPipe->pPipe->GetIDPtr().MP ) )
			{
				CDB_Pipe *pclPipe = dynamic_cast<CDB_Pipe*>( prBoxPipe->pPipe->GetIDPtr().MP );
				dLinDp = pclPipe->GetLinearDp( prBoxPipe->pPipe->GetRealQ(), m_pWC->GetDens(), m_pWC->GetKinVisc() );
				dVelocity = pclPipe->GetVelocity( prBoxPipe->pPipe->GetRealQ() );
			}

			CString str1, str2;
			TCHAR tcUnitName[_MAXCHARS];
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), tcUnitName );
			str1 = TASApp.LoadLocalizedString( IDS_PANELCIRC2_PIPELINDP );
			str.Format( _T("%s : %s %s\n"), str1, WriteCUDouble( _U_LINPRESSDROP, dLinDp ), tcUnitName );
			GetNameOf( m_pUnitDB->GetDefaultUnit( _U_VELOCITY ), tcUnitName );
			str2 = TASApp.LoadLocalizedString( IDS_PANELCIRC2_PIPEVELOCITY );
			str1.Format( _T("%s : %s %s"), str2, WriteCUDouble( _U_VELOCITY, dVelocity ), tcUnitName );
			str += str1;
		}
	}

	// Bv in bypass show overflow.
	if( m_BoxBvB.pos != CPoint( 0, 0 ) )
	{
		if( ( lColumn == m_BoxBvB.pos.x + eBCol::Lock || lColumn == m_BoxBvB.pos.x + eBCol::Desc ) && lRow == m_BoxBvB.pos.y )
		{
			if( m_pHM->GetpSchcat() != NULL && CDB_ControlProperties::CV2W3W::CV3W == m_pHM->GetpSchcat()->Get2W3W() )
			{
				double OF = m_pHM->GetBypOverFlow();

				if( OF <= 0 )
				{
					return;
				}

				CString str1;
				str = TASApp.LoadLocalizedString( IDS_PANELCIRC2_OVERFLOW );
				str1.Format( _T("%s : %s%%"), str, WriteCUDouble( _U_NODIM, OF ) );
				str = str1;
			}
		}
	}

	// BvP measuring valve localization.
	if( m_BoxBvP.pos != CPoint( 0, 0 ) )
	{
		if( ( lColumn == m_BoxBvP.pos.x + eBCol::Val || lColumn == m_BoxBvP.pos.x + eBCol::Desc ) && lRow == m_BoxBvP.pos.y + m_BoxBvP.BV_Loc
			&& true == m_BoxBvP.bMeasuringValve )
		{
			if( NULL != m_pHM->GetpDpC() )
			{
				str = ( m_pHM->GetpDpC()->GetMvLoc() == eMvLoc::MvLocPrimary ) ? TASApp.LoadLocalizedString( IDS_DLGCBHM_MVONPRIMARYTOOLTIP ) : 
						TASApp.LoadLocalizedString( IDS_DLGCBHM_MVONSECONDARYTOOLTIP );
			}
		}
	}

	// HYS-1833: Show error messages for temperature.
	if( m_BoxSecondaryValues.pos != CPoint( 0, 0 ) )
	{
		CDS_HmInj *pHMInj = dynamic_cast<CDS_HmInj *>( m_pHM );

		if( NULL == pHMInj )
		{
			return;
		}

		if( lColumn == m_BoxSecondaryValues.pos.x + eBCol::Val && ( lRow == m_BoxSecondaryValues.pos.y + m_BoxSecondaryValues.m_iDesignSecondarySupplyTemperature ) )
		{
			int iErrors = 0;
			std::vector<CString> vecStrErrors;

			if( false == pHMInj->CheckIfTempAreValid( &iErrors, &vecStrErrors ) )
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
	}

	// Accessories
	if( m_BoxCircuitPrimaryPipe.pos != CPoint( 0, 0 ) && m_BoxCircuitPrimaryPipe.pPipe != NULL )
	{
		if( lColumn == m_BoxCircuitPrimaryPipe.pos.x + eBCol::Val && lRow == m_BoxCircuitPrimaryPipe.pos.y + m_Box.Pipe_Acc )
		{
			m_pHM->GetpCircuitPrimaryPipe()->FormatSingularityString( str );
		}
	}

	if( m_BoxCircuitSecondaryPipe.pos != CPoint( 0, 0 ) && m_BoxCircuitSecondaryPipe.pPipe != NULL )
	{
		if( lColumn == m_BoxCircuitSecondaryPipe.pos.x + eBCol::Val && lRow == m_BoxCircuitSecondaryPipe.pos.y + m_Box.Pipe_Acc )
		{
			m_pHM->GetpCircuitSecondaryPipe()->FormatSingularityString( str );
		}
	}

	if( m_BoxDistributionSupplyPipe.pos != CPoint( 0, 0 ) && m_BoxDistributionSupplyPipe.pPipe != NULL )
	{
		if( lColumn == m_BoxDistributionSupplyPipe.pos.x + eBCol::Val && lRow == m_BoxDistributionSupplyPipe.pos.y + m_Box.Pipe_Acc )
		{
			m_pHM->GetpDistrSupplyPipe()->FormatSingularityString( str );
		}
	}

	if( m_BoxDistributionReturnPipe.pos != CPoint( 0, 0 ) && m_BoxDistributionReturnPipe.pPipe != NULL )
	{
		if( lColumn == m_BoxDistributionReturnPipe.pos.x + eBCol::Val && lRow == m_BoxDistributionReturnPipe.pos.y + m_Box.Pipe_Acc )
		{
			m_pHM->GetpDistrReturnPipe()->FormatSingularityString( str );
		}
	}

	if( m_BoxSmartControlValve.pos != CPoint( 0, 0 ) && NULL != m_pHM->GetpSmartControlValve() && NULL != m_pHM->GetpSmartControlValve()->GetIDPtr().MP )
	{
		// Verify if we are on the smart control valve "Name" line.
		long lNameCol = m_BoxSmartControlValve.pos.x + eBCol::Val;
		long lNameRow = m_BoxSmartControlValve.pos.y + m_BoxSmartControlValve.iName;
		bool bSkip = false;
		int iValidity = m_pHM->GetpSmartControlValve()->CheckValidity();

		if( lColumn == lNameCol && lRow == lNameRow )
		{
			str = m_pHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All );
			bSkip = true;
		}

		// Check the flow column.
		if( false == bSkip )
		{
			long lFlowCol = lNameCol;
			long lFlowRow = ( false == m_BoxSmartControlValve.bExt ) ? m_BoxSmartControlValve.iFlowMax : m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iFlowMax;
			lFlowRow += m_BoxSmartControlValve.pos.y;

			if( lColumn == lFlowCol && lRow == lFlowRow )
			{
				str = m_pHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Flow );
				bSkip = true;
			}
		}

		// Check the power column.
		if( false == bSkip && SmartValveControlMode::SCVCM_Power == m_pHM->GetpSmartControlValve()->GetControlMode() )
		{
			long lPowerCol = lNameCol;
			long lPowerRow = ( false == m_BoxSmartControlValve.bExt ) ? m_BoxSmartControlValve.iPowerMax : m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iPowerMax;
			lPowerRow += m_BoxSmartControlValve.pos.y;

			if( lColumn == lPowerCol && lRow == lPowerRow )
			{
				str = m_pHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Power );
				bSkip = true;
			}
		}

		// Check the Dp column.
		if( false == bSkip )
		{
			// Verify if we are on the smart control valve "Dp" line.
			long lDpCol = lNameCol;
			long lDpRow = ( false == m_BoxSmartControlValve.bExt ) ? m_BoxSmartControlValve.iDp : m_BoxSmartControlValve.iPN + m_BoxSmartControlValve.iDp;
			lDpRow += m_BoxSmartControlValve.pos.y;

			if( lColumn == lDpCol && lRow == lDpRow )
			{
				str = m_pHM->GetpSmartControlValve()->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp );
				bSkip = true;
			}
		}
	}

	if( m_BoxSmartDpC.pos != CPoint( 0, 0 ) )
	{
		// Verify if we are on the smart differential pressure controller "Name" line.
		long lNameCol = m_BoxSmartDpC.pos.x + eBCol::Val;
		long lNameRow = m_BoxSmartDpC.pos.y + m_BoxSmartDpC.iName;
		bool bSkip = false;

		// Refresh error messages.
		int iValidity = m_pHM->GetpSmartDpC()->CheckValidity();

		if( lColumn == lNameCol && lRow == lNameRow )
		{
			str = m_pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
					CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All );

			bSkip = true;
		}

		// Check the Dp line for the smart differential pressure controller.
		if( false == bSkip )
		{
			long lDpCol = lNameCol;
			long lDpRow = ( false == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iDp : m_BoxSmartDpC.iPN + m_BoxSmartDpC.iDp;
			lDpRow += m_BoxSmartDpC.pos.y;

			if( lColumn == lDpCol && lRow == lDpRow )
			{
				str = m_pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_SmartDpC,
						CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dp );
				bSkip = true;
			}
		}
		
		// Check dp to stabilize for the Dp sensor.
		if( false == bSkip )
		{
			long lDplCol = lNameCol;
			long lDplUnitCol = m_BoxSmartDpC.pos.x + eBCol::Unit;
			long lDplRow = ( false == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iDpl : m_BoxSmartDpC.iPN + m_BoxSmartDpC.iDpl;
			lDplRow += m_BoxSmartDpC.pos.y;

			if( ( lColumn == lDplCol || lColumn == lDplUnitCol ) && lRow == lDplRow )
			{
				str = m_pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor, 
						CDS_HydroMod::ErrorWarningMessageFlags::EWMF_Dpl );

				bSkip = true;
			}
		}

		// Check the sets.
		if( false == bSkip )
		{
			long lSetsCol = lNameCol;
			long lSetsRow = ( false == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iSets : m_BoxSmartDpC.iPN + m_BoxSmartDpC.iSets;
			lSetsRow += m_BoxSmartDpC.pos.y;

			if( lColumn == lSetsCol && lRow == lSetsRow )
			{
				str = m_pHM->GetpSmartDpC()->GetErrorMessageString( CDS_HydroMod::CSmartDpC::eErrorMessageObject_DpSensor, 
						CDS_HydroMod::ErrorWarningMessageFlags::EWMF_All );

				bSkip = true;
			}
		}
	}

	if( m_BoxPump.pos != CPoint( 0, 0 ) )
	{
		// Verify if we are on the pump "Havailable" line.
		long lHminColMin = m_BoxPump.pos.x + eBCol::Lock;
		long lHminColMax = m_BoxPump.pos.x + eBCol::Unit;
		long lHminRow = m_BoxPump.pos.y + m_BoxPump.iHmin;
		bool bSkip = false;

		// Refresh error messages.
		int iValidity = m_pHM->CheckValidity();

		if( lColumn >= lHminColMin && lColumn <= lHminColMax && lRow == lHminRow )
		{
			str = m_pHM->GetErrorMessageString( CDS_HydroMod::ErrorWarningMessageFlags::EWMF_SecondaryPump );
			bSkip = true;
		}
	}

	if( false == str.IsEmpty() )
	{
		CRect rect( 0, 0, 0, 0 );
		CDC *pDC = GetDC();
		pDC->DrawText( str, &rect, DT_CENTER | DT_CALCRECT | DT_NOPREFIX );
		ReleaseDC( pDC );
		*pnTipWidth = ( SHORT )rect.right;
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, ( LPCTSTR )str, SS_TT_TEXTMAX );
		*pbShowTip = true;
	}
}

void CSSheetPanelCirc2::_DrawCheckbox( long lColumn, long lRow, CDS_HydroMod::eUseProduct eState )
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

void CSSheetPanelCirc2::_ChangeColumnSize()
{
	BOOL bRedraw = GetBool( SSB_REDRAW );

	if( TRUE == bRedraw )
	{
		SetBool( SSB_REDRAW, FALSE );
	}

	CRect rectClient;
	GetClientRect( &rectClient );

	int iLogWidthFix = 3 * PC2_COLWIDTH_FIXED;
	int iLogWidthVar = 3 * ( PC2_COLWIDTH_LOCK + PC2_COLWIDTH_DESCRIPTION + PC2_COLWIDTH_VALUE + PC2_COLWIDTH_UNIT );

	// Decrease spread font when application run in a configuration where large font are used.
	// Prevent resizing of spread sheet.
	CDC ScreenDC;
	VERIFY( ScreenDC.CreateIC( _T("DISPLAY"), NULL, NULL, NULL ) );

	const int iDiv = ScreenDC.GetDeviceCaps( LOGPIXELSX );
	// Used to compute/correct size of column in Large Font.
#pragma warning( disable : 4244)

	if( iDiv > 96 )
	{
		m_dFontFactor = 96.0 / ( double )iDiv;
	}
	else
	{
		m_dFontFactor = 1.0;
	}

#pragma warning( default : 4244)

	double dFontSize = GetFontSize( 1, 1 );
	double dPixWidthFix = iLogWidthFix * dFontSize;
	double dPixWidthVar = iLogWidthVar * dFontSize;
	m_dFontFactor = ( float )( ( rectClient.Width() - dPixWidthFix ) / dPixWidthVar );

	int iCorrection = 0;

	for( long lLoopColumn = 1; lLoopColumn <= m_Box.nCols; lLoopColumn++ )
	{
		double dWidth = dFontSize * m_mapColWidth[( lLoopColumn - 1 ) % 5];

		if( PC2_COLUMPROPERTY_FIXED == GetCellParam( lLoopColumn, 1 ) )
		{
			SetColWidthInPixels( lLoopColumn, ( int )dWidth );
			iCorrection += ( int )( PC2_COLWIDTH_FIXED * dFontSize );
			continue;
		}

		if( lLoopColumn < m_Box.nCols )
		{
			dWidth *= m_dFontFactor;
			SetColWidthInPixels( lLoopColumn, ( int )dWidth );
			iCorrection += ( int )dWidth;
		}
		else
		{
			int iLastWidth = rectClient.Width() - iCorrection;
			SetColWidthInPixels( lLoopColumn, iLastWidth );
		}
	}

	if( TRUE == bRedraw )
	{
		SetBool( SSB_REDRAW, TRUE );
	}
}

void CSSheetPanelCirc2::_ClearRange( enum _BoxType eBoxType, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2 )
{
	if( m_mapComboBoxList.count( eBoxType ) > 0 )
	{
		m_mapComboBoxList.erase( eBoxType );
	}

	ClearRange( Col, Row, Col2, Row2 );
}

void CSSheetPanelCirc2::_FormatComboList( enum _BoxType eBoxType, long lColumn, long lRow )
{
	if( 0 == m_mapComboBoxList.count( eBoxType ) )
	{
		mappointparam mapPointParams;
		m_mapComboBoxList.insert( pairboxtypemapptparam( eBoxType, mapPointParams ) );
	}

	vecparam vecParams;
	m_mapComboBoxList.at( eBoxType ).insert( pairptvecparam( CPoint( lColumn, lRow ), vecParams ) );

	FormatComboList( lColumn, lRow );
}

void CSSheetPanelCirc2::_AddComboBoxParam( enum _BoxType eBoxType, long lColumn, long lRow, LPARAM lpParam )
{
	if( 0 == m_mapComboBoxList.count( eBoxType ) )
	{
		ASSERT_RETURN;
	}

	CPoint ptCombo( lColumn, lRow );

	if( 0 == m_mapComboBoxList.at( eBoxType ).count( ptCombo ) )
	{
		ASSERT_RETURN;
	}

	m_mapComboBoxList.at( eBoxType ).at( ptCombo ).push_back( lpParam );
}

LPARAM CSSheetPanelCirc2::_GetComboBoxParam( enum _BoxType eBoxType, long lColumn, long lRow, int iIndex )
{
	if( 0 == m_mapComboBoxList.count( eBoxType ) )
	{
		return NULL;
	}

	CPoint ptCombo( lColumn, lRow );

	if( 0 == m_mapComboBoxList.at( eBoxType ).count( ptCombo ) )
	{
		return NULL;
	}

	if( iIndex >= (int)m_mapComboBoxList.at( eBoxType ).at( ptCombo ).size() )
	{
		ASSERTA_RETURN( NULL );
	}

	return m_mapComboBoxList.at( eBoxType ).at( ptCombo ).at( iIndex );
}

void CSSheetPanelCirc2::_VerifyComboBoxError()
{
	for( auto &iterBoxes : m_mapComboBoxList )
	{
		for( auto &iterComboBox : iterBoxes.second )
		{
			int iCurrentSel = -1;
			bool bSetTextInError = false;

			switch( WhatBox( iterComboBox.first.x, iterComboBox.first.y ) )
			{
				case _BoxType::ebSmartDpC:
					{
						long lpSetsRow = ( false == m_BoxSmartDpC.bExt ) ? m_BoxSmartDpC.iSets : m_BoxSmartDpC.iPN + m_BoxSmartDpC.iSets;
						lpSetsRow += m_BoxSmartDpC.pos.y;
						long lDpSensorCol = m_BoxSmartDpC.pos.x + eBCol::Val;

						if( iterComboBox.first.x != lDpSensorCol || iterComboBox.first.y != lpSetsRow )
						{
							break;
						}

						iCurrentSel = ComboBoxSendMessage( iterComboBox.first.x, iterComboBox.first.y, SS_CBM_GETCURSEL, 0, 0 );

						if( -1 == iCurrentSel )
						{
							break;
						}

						CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( (CData *)_GetComboBoxParam( _BoxType::ebSmartDpC, iterComboBox.first.x, iterComboBox.first.y, iCurrentSel ) );

						if( NULL == pclDpSensor )
						{
							break;
						}

						if( NULL == m_pHM->GetpSmartDpC() || 0.0 == m_pHM->GetpSmartDpC()->GetDpToStabilize() )
						{
							break;
						}

						double dDpToStabilize = m_pHM->GetpSmartDpC()->GetDpToStabilize();

						// Check if error.
						if( dDpToStabilize >= pclDpSensor->GetMinMeasurableDp() && dDpToStabilize <= pclDpSensor->GetMaxMeasurableDp() )
						{
							break;
						}

						bSetTextInError = true;
					}
					break;

				default:
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
			clRectCell.right -= GetSystemMetrics(SM_CXVSCROLL);

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
