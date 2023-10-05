#include "stdafx.h"
#include "afxctl.h"
#include "TASelect.h"
#include "TASelectDoc.h"

#include "Global.h"
#include "Utilities.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "picture.h"
#include "dlgcomboboxhm.h"
#include "DlgHubCvCB.h"
#include "HMHub.h"
#include "RViewSSelHub.h"
#include "CbiRestrEdit.h"
#include "SSheetHub.h"

CSSheetTAHub::CSSheetTAHub()
{
	m_fPrinting = false;
	m_bCellModified = false;
	m_fHubMode = false;
	m_fModificationInCourse = false;
	m_fComboEditMode = false;
	m_lRow = 0;
	m_lCol = 0;
	m_dCellDouble = 0.0;
	m_strCellText = _T("");
	m_CellLParam = (LPARAM)0;
	m_pSCB = NULL;
	m_pSCvCB = NULL;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pHub = NULL;
	m_arlHub.RemoveAll();
	m_arlHubStation.RemoveAll();
}

CSSheetTAHub::~CSSheetTAHub()
{
	m_arlHub.RemoveAll();
	m_arlHubStation.RemoveAll();

	if( NULL != m_pSCB )
		delete m_pSCB;
	if( NULL != m_pSCvCB)
		delete m_pSCvCB;
	
	m_pHub = NULL;
}

BOOL CSSheetTAHub::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	if( !CSSheet::Create( dwStyle, rect, pParentWnd, nID ) ) 
		return FALSE;
	return TRUE;
}

void CSSheetTAHub::RestoreColExtended( )
{
	CArray <long> *parlHub;
	
	if( true == m_fHubMode )
	{
		parlHub = &m_arlHub;
	}
	else
	{
		parlHub = &m_arlHubStation;
	}
	
	for( int i = 0; i < parlHub->GetCount(); i++ )
	{
		ChangeExpandCollapseColumnButtonState( parlHub->GetAt( i ), m_HeaderRow.ButDlg, m_HeaderRow.FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_ToExpand );
	}

	if( NULL != pRViewSSelHub )
	{
		pRViewSSelHub->SendMessage( WM_SIZE, SIZE_RESTORED, 0 );
		pRViewSSelHub->SendMessage( WM_PAINT, 0, 0 );
	}
}

void CSSheetTAHub::RefreshCollapseExpandButton( CSSheetTAHub * pclOtherTAHub )
{
	// Not in Direct Mode.
	if( CDS_HmHub::SelMode::Direct == m_pHub->GetSelMode() || NULL == pclOtherTAHub )
		return;

	// Run all columns and check state of all Expand/Collapse column buttons.
	bool fAtLeastOneGroupCollapsed = false;
	bool fAtLeastOneGroupExpanded = false;
	CheckExpandCollapseColumnButtonState( m_Sheet.LabelID, m_Sheet.Pointer, m_HeaderRow.ButDlg, &fAtLeastOneGroupCollapsed, &fAtLeastOneGroupExpanded );

	// Run all columns from the other CCSheetHub and check state of all Expand/Collapse column buttons.
	bool fOtherAtLeastOneGroupCollapsed = false;
	bool fOtherAtLeastOneGroupExpanded = false;
	pclOtherTAHub->CheckExpandCollapseColumnButtonState( m_Sheet.LabelID, m_Sheet.Pointer, m_HeaderRow.ButDlg, &fOtherAtLeastOneGroupCollapsed, &fOtherAtLeastOneGroupExpanded );

	bool fAllAtLeastOneGroupCollapsed = fAtLeastOneGroupCollapsed | fOtherAtLeastOneGroupCollapsed;
	bool fAllAtLeastOneGroupExpanded = fAtLeastOneGroupExpanded | fOtherAtLeastOneGroupExpanded;

	CSSheetTAHub *pButtonOwner = ( true == m_fHubMode ) ? this : pclOtherTAHub;

	// If we need main Expand/Collapse column button...
	if( true == fAllAtLeastOneGroupCollapsed || true == fAllAtLeastOneGroupExpanded )
	{
		// Check which button we must set.
		bool fGeneralStateCollapse;
		bool fDoChange = true;
		if( true == fAllAtLeastOneGroupCollapsed && true == fAllAtLeastOneGroupExpanded )
		{
			// If there is a mix of columns collapsed and expanded, we must not change main button state.
			fDoChange = false;
		}
		else if( true == fAllAtLeastOneGroupCollapsed )
		{
			// It means that all groups are collapsed and then main button must be in expand mode.
			fGeneralStateCollapse = false;
		}
		else if( true == fAllAtLeastOneGroupExpanded )
		{
			// It means that all groups are expanded and then main button must be in collapse mode.
			fGeneralStateCollapse = true;
		}
		else
		{
			// This case not happens here!
		}

		// If main button state must be changed...
		if( true == fDoChange )
		{
			CSSheet::ExpandCollapseColumnAction eState = ( true == fGeneralStateCollapse ) ? CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse : CSSheet::ExpandCollapseColumnAction::ECCA_ToExpand;
			pButtonOwner->ChangeMainExpandCollapseButtonState( m_Sheet.Pos, m_HeaderRow.ButDlg, eState );
		}
	}
	else
	{
		// Delete main Expand/Collapse column button.
		SS_CELLTYPE rCellType;
		SetTypeStaticText( &rCellType, SSS_ALIGN_LEFT );
		pButtonOwner->SetCellType( m_Sheet.Pos, m_HeaderRow.ButDlg, &rCellType );
		pButtonOwner->SetCellTag( m_Sheet.Pos, m_HeaderRow.ButDlg, _T("") );
	}
}

void CSSheetTAHub::Init( bool fHubMode, CDS_HmHub *pHub, bool fPrint )
{
	BeginWaitCursor();

	m_fHubMode = fHubMode;
	m_fPrinting = fPrint;
	m_pTADS = TASApp.GetpTADS();
	m_pTADB = TASApp.GetpTADB();			ASSERT( m_pTADB );
	
	if( NULL == m_pSCB )
		m_pSCB = new CDlgComboBoxHM( this );
	if( NULL == m_pSCvCB )
		m_pSCvCB = new CDlgHubCvCB( this );
	
	m_bCellModified = false;
	m_dCellDouble = 0.0;
	m_CellLParam = NULL;
	m_strCellText = _T("");
	m_fModificationInCourse = false;
	
	if( NULL != pHub->GetpSelectedInfos() )
		m_fModificationInCourse = (*pHub->GetpSelectedInfos()->GetModifiedObjIDPtr().ID != 0 );

	CSSheet::Init();
	
	SetBool( SSB_HORZSCROLLBAR, FALSE );
	SetBool( SSB_VERTSCROLLBAR, FALSE );

	if( NULL != pHub )
		m_pHub = pHub;
	InitSheet( m_pHub );

	SetCellNoteIndicator( SS_CELLNOTEINDICATOR_SHOWANDFIREEVENT );
	SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	RestoreColExtended();

	EndWaitCursor();
}

void CSSheetTAHub::InitSheet( CDS_HmHub *pHub )
{
	bool fExistTerminalValve = false;						// true if at least one valve of terminal valve is selected.
	bool fExistVenturiBalancingValve = false;				// true if at least one valve of venturi balancing valve is selected.
	bool fAllValveAreTV = true;								// All valves are TV
	
	if( NULL != m_pHub )
	{
		if( false == m_fHubMode )
		{
			for( IDPTR IDPtr = m_pHub->GetFirst(); *IDPtr.ID; IDPtr = m_pHub->GetNext() )
			{		
				CDS_HmHubStation *pHM = (CDS_HmHubStation*)( IDPtr.MP );
				if( pHM->GetBalTypeID() == _T("RVTYPE_BVC") )
					fExistTerminalValve = true;
				else 
					fAllValveAreTV = false;
				if( m_pHub->GetSelMode() != CDS_HmHub::SelMode::Direct && pHM->GetBalTypeID() == _T("RVTYPE_VV") )
					fExistVenturiBalancingValve = true; 
			}
		}
		else
		{
			if( m_pHub->GetSelMode() != CDS_HmHub::SelMode::Direct && m_pHub->GetBalTypeID() == _T("RVTYPE_VV") )
				fExistVenturiBalancingValve = true;
		}
	}

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// No need to call again CSSheet::Init because 'Init' has already done
//	CSSheet::Init();
	
	SetBool( SSB_HORZSCROLLBAR, FALSE );
	SetBool( SSB_VERTSCROLLBAR, FALSE );
	
	SetMaxRows( m_HeaderRow.FirstCirc - 1 );
	SetMaxCols( m_Sheet.Pointer );
	
	SetFreeze( m_Sheet.Pos, m_HeaderRow.FirstCirc - 1 );

	// All cells are static by default and filled with _T("")
	SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase ROW height
	double dRowHeight = 12.75;// Reference
	SetRowHeight( m_HeaderRow.GroupName, dRowHeight * 1.5 );
	SetRowHeight( m_HeaderRow.ButDlg, dRowHeight * 1.2 );
	SetRowHeight( m_HeaderRow.ColName, dRowHeight * 2 );
	SetRowHeight( m_HeaderRow.Unit, dRowHeight * 1.2 );
	SetRowHeight( m_HeaderRow.Picture, dRowHeight );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Set columns size
	////////////////////////////////////////////////////////////////////////////////////////////////////

	SetColWidth( m_Sheet.Sub, .8 );
	SetColWidth( m_Sheet.Pos, 3 );
	SetColWidth( m_Sheet.CheckBox, 2 );
	SetColWidth( m_Sheet.LabelID, 10 );
	
	// Loop
	SetColWidth( m_Sheet.LoopDesc, 10 );
	SetColWidth( m_Sheet.LoopQ, 6 );
	SetColWidth( m_Sheet.LoopP, 6 );
	SetColWidth( m_Sheet.LoopDT, 6 );
	SetColWidth( m_Sheet.LoopDp, 6 );
	SetColWidth( m_Sheet.LoopSep, .8 );
	
	// Supply Side
	SetColWidth( m_Sheet.SupName, 10 );
	SetColWidth( m_Sheet.SupDp, 6 );
	SetColWidth( m_Sheet.SupSep, .8 );
	
	// Return Side - BV
	// Valve
	SetColWidth( m_Sheet.BvType, 15 );
	SetColWidth( m_Sheet.BvCtrlType, 10 );
	if( true == m_fHubMode && ( CDS_HmHub::SelMode::Direct == (pHub->GetSelMode() ) || ( CDS_HmHub::SelMode::Q == pHub->GetSelMode() ) ) )
		SetColWidth( m_Sheet.BvName, 20 );
	else
		SetColWidth( m_Sheet.BvName, 10 );
	SetColWidth( m_Sheet.BvPreSet, 6 );
	SetColWidth( m_Sheet.BvDp, 6 );
	SetColWidth( m_Sheet.BvDpSignal, 7 );
	SetColWidth( m_Sheet.BvDpMin, 6 );
	SetColWidth( m_Sheet.BvSep, .8 );
	
	// Actuator
	SetColWidth( m_Sheet.ActuatorType, 15 );
	SetColWidth( m_Sheet.ActuatorSep, .8 );
	
	// DpC
	SetColWidth( m_Sheet.DpCName, 15 );
	SetColWidth( m_Sheet.DpCDpl, 10 );
	SetColWidth( m_Sheet.DpCDpMin, 6 );
	SetColWidth( m_Sheet.DpCDpLr, 15 );
	SetColWidth( m_Sheet.DpCSet, 6 );
	SetColWidth( m_Sheet.DpCDp, 6 );
	SetColWidth( m_Sheet.DpCSep, .8 );
	
	// Terminal Unit
	SetColWidth( m_Sheet.UnitDp, 8 );
	SetColWidth( m_Sheet.UnitQref, 6 );
	SetColWidth( m_Sheet.UnitDpref, 6 );
	SetColWidth( m_Sheet.UnitSep, .8 );
	
	// CV
	if( CDS_HmHub::SelMode::LoopDetails == pHub->GetSelMode() )
		SetColWidth( m_Sheet.CVType, 20 ); 
	else
		SetColWidth( m_Sheet.CVType, 10 ); 
	SetColWidth( m_Sheet.CVName, 15 );
	SetColWidth( m_Sheet.CVDesc, 10 );
	SetColWidth( m_Sheet.CVKvsMax, 6 );
	SetColWidth( m_Sheet.CVKvs, 6 );
	SetColWidth( m_Sheet.CVDp, 6 );
	SetColWidth( m_Sheet.CVAuth, 10 );
	SetColWidth( m_Sheet.CVSep, .8 );
	
	// Pipe
	SetColWidth( m_Sheet.PipeSerie, 8 );
	SetColWidth( m_Sheet.PipeSize, 8 );
	SetColWidth( m_Sheet.PipeL, 6 );
	SetColWidth( m_Sheet.PipeDp, 6 );
	SetColWidth( m_Sheet.PipeLinDp, 6 );
	SetColWidth( m_Sheet.PipeV, 6 );
	SetColWidth( m_Sheet.PipeSep, .8 );
	
	// Accessories
	SetColWidth( m_Sheet.AccDesc, 15 );
	SetColWidth( m_Sheet.AccDp, 6 );
	SetColWidth( m_Sheet.AccSep, .8 );
	
	// Pointer for Hub/HubStation (one by line)
	SetColWidth( m_Sheet.Pointer, .8 );


	// All cells are not selectable not selectable
	SetCellProperty( m_Sheet.Pos, m_HeaderRow.GroupName, m_Sheet.Pointer, m_HeaderRow.FirstCirc - 1, CSSheet::_SSCellProperty::CellNoSelection, true );
	
	// Flag unhidden columns
	SetFlagShowEvenEmpty( m_Sheet.Pos, m_Sheet.Pos, true );
	SetFlagShowEvenEmpty( m_Sheet.CheckBox, m_Sheet.CheckBox, true );
	SetFlagShowEvenEmpty( m_Sheet.LabelID, m_Sheet.LabelID, true );
	SetFlagShowEvenEmpty( m_Sheet.LoopDesc, m_Sheet.LoopDesc, true );
	SetFlagShowEvenEmpty( m_Sheet.LoopDesc, m_Sheet.LoopDesc, true );
	SetFlagShowEvenEmpty( m_Sheet.LoopSep, m_Sheet.LoopSep, true );
	SetFlagShowEvenEmpty( m_Sheet.SupSep, m_Sheet.SupSep, true );
	SetFlagShowEvenEmpty( m_Sheet.BvSep, m_Sheet.BvSep, true );
	SetFlagShowEvenEmpty( m_Sheet.ActuatorSep, m_Sheet.ActuatorSep, true );
	SetFlagShowEvenEmpty( m_Sheet.DpCSep, m_Sheet.DpCSep, true );
	SetFlagShowEvenEmpty( m_Sheet.UnitSep, m_Sheet.UnitSep, true );
	SetFlagShowEvenEmpty( m_Sheet.CVSep, m_Sheet.CVSep, true );
	SetFlagShowEvenEmpty( m_Sheet.PipeSep, m_Sheet.PipeSep, true );
	SetFlagShowEvenEmpty( m_Sheet.AccSep, m_Sheet.AccSep, true );

	bool fColorGroup = false; 
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Unit Group
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main Title
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
	fColorGroup ^= 1;
	AddCellSpanW( m_Sheet.LabelID, m_HeaderRow.GroupName, m_Sheet.LoopSep - m_Sheet.LabelID, 2 );
	if( false == m_fHubMode )
		SetStaticText( m_Sheet.LabelID, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_LOOP );
	else
		SetStaticText( m_Sheet.LabelID, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_TAHUB );

	// LabelID
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.LabelID, m_HeaderRow.ColName, IDS_SSHEETTAHUB_LABELID );
	SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.LabelID, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

	// Description
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.LoopDesc, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DESC );
	SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.LoopDesc, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
		
	// Q
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.LoopQ, m_HeaderRow.ColName, IDS_SSHEETTAHUB_Q );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetStaticText( m_Sheet.LoopQ, m_HeaderRow.Unit, name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	if( false == m_fHubMode )
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.LoopQ, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
		
	// Power
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.LoopP, m_HeaderRow.ColName, IDS_SSHEETTAHUB_P );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ), name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetStaticText( m_Sheet.LoopP, m_HeaderRow.Unit, name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.LoopP, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

	// DT
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.LoopDT, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DT );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFTEMP ), name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetStaticText( m_Sheet.LoopDT, m_HeaderRow.Unit, name );	     				
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.LoopDT, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
	
	// Dp
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	if( false == m_fHubMode )
		SetStaticText( m_Sheet.LoopDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DPLOOP );
	else
		SetStaticText( m_Sheet.LoopDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_TOTDP );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetStaticText( m_Sheet.LoopDp, m_HeaderRow.Unit, name );	     				
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

	// Border
	SetCellBorder( m_Sheet.LabelID, m_HeaderRow.Picture - 1, m_Sheet.LoopSep - 1, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	SetCellBorder( m_Sheet.LabelID, m_HeaderRow.FirstCirc - 1, m_Sheet.LoopSep - 1, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	// Valve on Station Supply
	if( false == m_fHubMode || false == m_pHub->IsDpCExist() )
	{
		// Main Title
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
		fColorGroup ^= 1;
		AddCellSpanW( m_Sheet.SupName, m_HeaderRow.GroupName, m_Sheet.SupSep - m_Sheet.SupName, 2 );
		if( false == m_fHubMode )
			SetStaticText( m_Sheet.SupName, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_STATIONSUPPLYVALVES );
		else
			SetStaticText( m_Sheet.SupName, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_VALVESSUPPLY );

		// Name
		SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
		SetStaticText( m_Sheet.SupName, m_HeaderRow.ColName, IDS_SSHEETTAHUB_NAME );
		
		// Dp
		SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
		SetStaticText( m_Sheet.SupDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		SetStaticText( m_Sheet.SupDp, m_HeaderRow.Unit, name );	     				
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

		// Border
		SetCellBorder( m_Sheet.SupName, m_HeaderRow.Picture - 1, m_Sheet.SupSep - 1, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
		SetCellBorder( m_Sheet.SupName, m_HeaderRow.FirstCirc - 1, m_Sheet.SupSep - 1, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	}
	else
		SetColumnAlwaysHidden( m_Sheet.SupName, m_Sheet.SupSep, true );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Valve Group
	////////////////////////////////////////////////////////////////////////////////////////////////////

	// Main Title
	SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
	fColorGroup ^= 1;
	AddCellSpanW( m_Sheet.BvType, m_HeaderRow.GroupName, m_Sheet.BvSep - m_Sheet.BvType, 2 );
		
	if( false == m_fHubMode )
		SetStaticText( m_Sheet.BvType, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_STATIONRETURNVALVES );
	else
	{
		if( NULL != m_pHub )
		{
			if( true == m_pHub->IsDpCExist() )
				SetStaticText( m_Sheet.BvType, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_VALVESSUPPLY );
			else
				SetStaticText( m_Sheet.BvType, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_VALVESRETURN );
		}
		else
			SetStaticText( m_Sheet.BvType, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_VALVESRETURN );
	}
	
	// Type
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.BvType, m_HeaderRow.ColName, IDS_SSHEETTAHUB_TYPE );
	SetPictureCellWithID( IDI_COMBO, m_Sheet.BvType, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
	
	// Name
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.BvName, m_HeaderRow.ColName, IDS_SSHEETTAHUB_NAME );
	
	// Setting
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.BvPreSet, m_HeaderRow.ColName, IDS_SSHEETTAHUB_PRESETTING );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetStaticText( m_Sheet.BvPreSet, m_HeaderRow.Unit, IDS_TURNS );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
	// Dp
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.BvDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DP );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetStaticText( m_Sheet.BvDp, m_HeaderRow.Unit, name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
	// Dp Signal
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.BvDpSignal, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DPSIGNAL );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetStaticText( m_Sheet.BvDpSignal, m_HeaderRow.Unit, name );	     				
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
	// DpSignal column hidden in DirectMode or if venturi valve doesn't exist
	if( false == fExistVenturiBalancingValve ) 
		SetColumnAlwaysHidden( m_Sheet.BvDpSignal, m_Sheet.BvDpSignal, true );
	else
		SetColumnAlwaysHidden( m_Sheet.BvDpSignal, m_Sheet.BvDpSignal, false );
	
	// Dp 100%
	SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
	SetStaticText( m_Sheet.BvDpMin, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DP100 );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	SetStaticText( m_Sheet.BvDpMin, m_HeaderRow.Unit, name );	     				
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
	// Border
	SetCellBorder( m_Sheet.BvType, m_HeaderRow.Picture - 1, m_Sheet.BvSep - 1, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	SetCellBorder( m_Sheet.BvType, m_HeaderRow.FirstCirc - 1, m_Sheet.BvSep - 1, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Actuator Group
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if( false == m_fHubMode )
	{
		// Main Title
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
		fColorGroup ^= 1;
		AddCellSpanW( m_Sheet.ActuatorType, m_HeaderRow.GroupName, m_Sheet.ActuatorSep-m_Sheet.ActuatorType, 2 );
		SetStaticText( m_Sheet.ActuatorType, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_ACTUATOR );
		
		// Type
		SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
		SetStaticText( m_Sheet.ActuatorType, m_HeaderRow.ColName, IDS_SSHEETTAHUB_TYPE );
		SetPictureCellWithID( IDI_COMBO, m_Sheet.ActuatorType, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
		
		// Border
		SetCellBorder( m_Sheet.ActuatorType, m_HeaderRow.Picture - 1, m_Sheet.ActuatorSep - 1, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
		SetCellBorder( m_Sheet.ActuatorType, m_HeaderRow.FirstCirc - 1, m_Sheet.ActuatorSep - 1, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// DpC Group
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if( true == m_fHubMode && NULL != m_pHub )
	{
		if( true == m_pHub->IsDpCExist() )
		{
			// Main Title
			SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
			fColorGroup ^= 1;
			AddCellSpanW( m_Sheet.DpCName, m_HeaderRow.GroupName, m_Sheet.DpCSep - m_Sheet.DpCName, 2 );
			SetStaticText( m_Sheet.DpCName, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_DPC );

			// Name
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.DpCName, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DPCNAME );
			SetPictureCellWithID( IDI_COMBO, m_Sheet.DpCName, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

			// DpL range
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.DpCDpLr, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DPLRANGE );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.DpCDpLr, m_HeaderRow.Unit, name );	     				
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			// Setting
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.DpCSet, m_HeaderRow.ColName, IDS_SSHEETTAHUB_PRESETTING );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.DpCSet, m_HeaderRow.Unit, IDS_TURNS);	     				
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			// Dp
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.DpCDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DP );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.DpCDp, m_HeaderRow.Unit, name );	     				
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			// DpL
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.DpCDpl, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DPCDPL );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.DpCDpl, m_HeaderRow.Unit, name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			// DpMin
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.DpCDpMin, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DPMIN );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.DpCDpMin, m_HeaderRow.Unit, name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
			// Border
			SetCellBorder( m_Sheet.DpCName, m_HeaderRow.Picture - 1, m_Sheet.DpCDpMin, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( m_Sheet.DpCName, m_HeaderRow.FirstCirc - 1, m_Sheet.DpCDpMin, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			// Collapse button
			CreateExpandCollapseColumnButton( m_Sheet.DpCSep, m_HeaderRow.ButDlg, m_Sheet.DpCSet, m_Sheet.DpCDpMin, GetMaxRows() );
			SetFlagCanBeCollapsed( m_Sheet.DpCDp, m_Sheet.DpCDpl, false );
			ChangeExpandCollapseColumnButtonState( m_Sheet.DpCSep, m_HeaderRow.ButDlg, m_HeaderRow.FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_Toggle );
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Terminal Unit Group
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if( false == m_fHubMode && ( CDS_HmHub::SelMode::LoopDetails == m_pHub->GetSelMode() || CDS_HmHub::SelMode::QDpLoop == m_pHub->GetSelMode() ) )
	{
		// Main Title
		SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
		fColorGroup ^= 1;
		AddCellSpanW( m_Sheet.UnitDp, m_HeaderRow.GroupName, m_Sheet.UnitSep - m_Sheet.UnitDp, 2 );
		SetStaticText( m_Sheet.UnitDp, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_LOOPUNIT);

		// Dp
		SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
		SetStaticText( m_Sheet.UnitDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DP );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		SetStaticText( m_Sheet.UnitDp, m_HeaderRow.Unit, name );	     				
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.UnitDp , m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
	
		// Qref
		SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
		SetStaticText( m_Sheet.UnitQref, m_HeaderRow.ColName, IDS_SSHEETTAHUB_QREF );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ), name );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		SetStaticText( m_Sheet.UnitQref, m_HeaderRow.Unit, name );	     				
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.UnitQref, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

		// Dpref
		SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
		SetStaticText( m_Sheet.UnitDpref, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DPREF );
		GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		SetStaticText( m_Sheet.UnitDpref, m_HeaderRow.Unit, name );	     				
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
		SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.UnitDpref, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
		
		// Border
		SetCellBorder( m_Sheet.UnitDp, m_HeaderRow.Picture - 1, m_Sheet.UnitSep - 1, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
		SetCellBorder( m_Sheet.UnitDp, m_HeaderRow.FirstCirc - 1, m_Sheet.UnitSep - 1, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

		// Collapse button
		CreateExpandCollapseColumnButton( m_Sheet.UnitSep, m_HeaderRow.ButDlg, m_Sheet.UnitQref, m_Sheet.UnitDpref, GetMaxRows() );
		ChangeExpandCollapseColumnButtonState( m_Sheet.UnitSep, m_HeaderRow.ButDlg, m_HeaderRow.FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_Toggle );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// CV Group
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if( false == m_fHubMode && NULL != m_pHub )
	{
		if( CDS_HmHub::SelMode::LoopDetails == m_pHub->GetSelMode() )
		{
			// Main Title
			SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
			fColorGroup ^= 1;
			AddCellSpanW( m_Sheet.CVType, m_HeaderRow.GroupName, m_Sheet.CVSep - m_Sheet.CVType, 2 );
			SetStaticText( m_Sheet.CVType, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_CV );
			
			// Type
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.CVType, m_HeaderRow.ColName, IDS_SSHEETTAHUB_TYPE );
			SetPictureCellWithID( IDI_COMBO, m_Sheet.CVType, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );	

			// Name
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.CVName, m_HeaderRow.ColName, IDS_SSHEETTAHUB_NAME );
			SetPictureCellWithID( IDI_COMBO, m_Sheet.CVName, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

			// Description
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.CVDesc, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DESC );
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.CVDesc, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

			// Kvsmax
			CString str;
			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				str = TASApp.LoadLocalizedString( IDS_KVSMAX );
			else
				str = TASApp.LoadLocalizedString( IDS_CVMAX );
			SetStaticText( m_Sheet.CVKvsMax, m_HeaderRow.ColName, str );

			// Kvs
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
				str = TASApp.LoadLocalizedString( IDS_KVS );
			else
				str = TASApp.LoadLocalizedString( IDS_CV );
			SetStaticText( m_Sheet.CVKvs, m_HeaderRow.ColName, str );
			SetPictureCellWithID( IDI_COMBOEDIT, m_Sheet.CVKvs, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

			// Dp
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.CVDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DP );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.CVDp, m_HeaderRow.Unit, name );	     				
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			
			// Authority
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			if( false == m_pHub->IsDpCExist() )
				SetStaticText( m_Sheet.CVAuth, m_HeaderRow.ColName, IDS_SSHEETTAHUB_MINAUTHOR );
			else
				SetStaticText( m_Sheet.CVAuth, m_HeaderRow.ColName, IDS_HMCALC_MINAUTHOR );
			
			// Border
			SetCellBorder( m_Sheet.CVType, m_HeaderRow.Picture - 1, m_Sheet.CVSep - 1, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( m_Sheet.CVType, m_HeaderRow.FirstCirc - 1, m_Sheet.CVSep - 1, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			if( true == fAllValveAreTV )
				SetColumnAlwaysHidden( m_Sheet.CVType, m_Sheet.CVSep, true );
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Circuit Pipe Group
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if( false == m_fHubMode && NULL != m_pHub )
	{
		if( CDS_HmHub::SelMode::LoopDetails == m_pHub->GetSelMode() )
		{
			// Main Title
			SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
			fColorGroup ^= 1;
			AddCellSpanW( m_Sheet.PipeSerie, m_HeaderRow.GroupName, m_Sheet.PipeSep - m_Sheet.PipeSerie, 2 );
			SetStaticText( m_Sheet.PipeSerie, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_LOOPPIPES );

			// Pipe series
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.PipeSerie, m_HeaderRow.ColName, IDS_SSHEETTAHUB_PIPESERIE );
			SetPictureCellWithID( IDI_COMBO, m_Sheet.PipeSerie, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

			// Pipe Size
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.PipeSize, m_HeaderRow.ColName, IDS_SSHEETTAHUB_PIPESIZE );
			SetPictureCellWithID( IDI_COMBO, m_Sheet.PipeSize, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );

			// Length
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.PipeL, m_HeaderRow.ColName, IDS_SSHEETTAHUB_PIPEL );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ), name );
			SetStaticText( m_Sheet.PipeL, m_HeaderRow.Unit, name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			SetPictureCellWithID( IDI_PENCILDIAGSHORT, m_Sheet.PipeL, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
			
			// Dp
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.PipeDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DP );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.PipeDp, m_HeaderRow.Unit,  name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			// Linear dP
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.PipeLinDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_LINDP );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.PipeLinDp, m_HeaderRow.Unit,  name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );

			// v
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.PipeV, m_HeaderRow.ColName, IDS_SSHEETTAHUB_PIPEVA );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.PipeV, m_HeaderRow.Unit, name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	
			// Border
			SetCellBorder( m_Sheet.PipeSerie, m_HeaderRow.Picture - 1, m_Sheet.PipeSep - 1, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( m_Sheet.PipeSerie, m_HeaderRow.FirstCirc - 1, m_Sheet.PipeV, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );

			// Collapse button
			CreateExpandCollapseColumnButton( m_Sheet.PipeSep, m_HeaderRow.ButDlg, m_Sheet.PipeLinDp, m_Sheet.PipeV, GetMaxRows() );
			ChangeExpandCollapseColumnButtonState( m_Sheet.PipeSep, m_HeaderRow.ButDlg, m_HeaderRow.FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_Toggle );
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Accessories Group
	////////////////////////////////////////////////////////////////////////////////////////////////////
	
	if( false == m_fHubMode && NULL != m_pHub )
	{
		if( CDS_HmHub::SelMode::LoopDetails == m_pHub->GetSelMode() )
		{
			// Main Title
			SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)true );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetColHeaderBackColorForHub( m_fHubMode, fColorGroup, m_fModificationInCourse ) );
			fColorGroup ^= 1;
			AddCellSpanW( m_Sheet.AccDesc, m_HeaderRow.GroupName, m_Sheet.AccSep - m_Sheet.AccDesc, 2 );
			SetStaticText( m_Sheet.AccDesc, m_HeaderRow.GroupName, IDS_SSHEETTAHUB_ACCESSORIES );
			
			// Description
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.AccDesc, m_HeaderRow.ColName, IDS_SSHEETTAHUB_ACCDESCRIPTION );
			
			// Dp
			SetTextPattern( CSSheet::_SSTextPattern::ColumnHeaderWordWrap );
			SetStaticText( m_Sheet.AccDp, m_HeaderRow.ColName, IDS_SSHEETTAHUB_DP );
			GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), name );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetStaticText( m_Sheet.AccDp, m_HeaderRow.Unit, name );	     				
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
			
			// Border
			SetCellBorder( m_Sheet.AccDesc, m_HeaderRow.Picture - 1, m_Sheet.AccSep - 1, m_HeaderRow.Picture - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
			SetCellBorder( m_Sheet.AccDesc, m_HeaderRow.FirstCirc - 1, m_Sheet.AccSep - 1, m_HeaderRow.FirstCirc - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
		}
	}
	
	// Hide unused column, depending of type of visualization requested (m_bHubMod)
	if( true == m_fHubMode )
	{
		SheetHubHideColumns();
		CreateMainExpandCollapseButton( m_Sheet.Pos, m_HeaderRow.ButDlg );
	}
	else
		SheetStationHideColumns();
}

void CSSheetTAHub::SheetHubHideColumns()
{				
	SetBool( SSB_ALLOWDRAGDROP, FALSE );
	SetMaxRows( m_HeaderRow.FirstCirc );

	// Hide unnecessary columns
	SetColumnAlwaysHidden( m_Sheet.UnitDp, m_Sheet.Pointer, true );
	SetColumnAlwaysHidden( m_Sheet.BvCtrlType, true );
	SetColumnAlwaysHidden( m_Sheet.ActuatorType, m_Sheet.ActuatorSep, true );
	SetColumnAlwaysHidden( m_Sheet.UnitQref, true );
	SetColumnAlwaysHidden( m_Sheet.UnitDpref, true );
	SetColumnAlwaysHidden( m_Sheet.LoopDT, true );
	SetColumnAlwaysHidden( m_Sheet.LoopP, true );
	SetColumnAlwaysHidden( m_Sheet.BvType, true );

	if( NULL != m_pHub )
	{
		switch( m_pHub->GetSelMode() )
		{
			case CDS_HmHub::SelMode::Direct:
				SetColumnAlwaysHidden( m_Sheet.LoopQ, m_Sheet.LoopDp, true );
				SetColumnAlwaysHidden( m_Sheet.BvPreSet, m_Sheet.BvDpMin, true );
				SetColumnAlwaysHidden( m_Sheet.DpCSet, m_Sheet.DpCSep, true );
				break;

			case CDS_HmHub::SelMode::Q:
				// Collapse button
				CreateExpandCollapseColumnButton( m_Sheet.BvSep, m_HeaderRow.ButDlg, m_Sheet.BvPreSet, m_Sheet.BvDpMin, GetMaxRows() );
				// Set that 'DP Signal' column can't be collapsed
				SetFlagCanBeCollapsed( m_Sheet.BvDp, m_Sheet.BvDpSignal, false );
				// Collapse Information columns
				ChangeExpandCollapseColumnButtonState( m_Sheet.BvSep, m_HeaderRow.ButDlg, m_HeaderRow.FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_Toggle );
				break;

			case CDS_HmHub::SelMode::LoopDetails:
			case CDS_HmHub::SelMode::QDpLoop:
				// Collapse button
				CreateExpandCollapseColumnButton( m_Sheet.BvSep, m_HeaderRow.ButDlg, m_Sheet.BvPreSet, m_Sheet.BvDpMin, GetMaxRows() );
				SetFlagCanBeCollapsed( m_Sheet.BvDp, m_Sheet.BvDpSignal, false );
				ChangeExpandCollapseColumnButtonState( m_Sheet.BvSep, m_HeaderRow.ButDlg, m_HeaderRow.FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_Toggle );
				break;
		}

		if( true == m_pHub->IsDpCExist() )
			SetColumnAlwaysHidden( m_Sheet.BvDpSignal, m_Sheet.BvDpSignal, true );
		else
		{
			SetColumnAlwaysHidden( m_Sheet.DpCName, m_Sheet.DpCSep, true );
			SetPictureCellWithID( IDI_COMBO, m_Sheet.BvName, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
		}
	}
	
	SetCellProperty( m_Sheet.Pointer, (int)GetMaxRows(), CSSheet::_SSCellProperty::CellNoSelection, true );
}

void CSSheetTAHub::SheetStationHideColumns()
{
	SetPictureCellWithID( IDI_COMBO, m_Sheet.BvName, m_HeaderRow.Picture, CSSheet::PictureCellType::Icon );
	if( NULL != m_pHub )
	{
		SetMaxRows( m_HeaderRow.FirstCirc + ( m_pHub->GetNumberOfStations() - 1 ) );
		for( int i =  m_HeaderRow.FirstCirc; i <= GetMaxRows(); i++ )
		{
			SetCellBorder( m_Sheet.LabelID, i, m_Sheet.LoopSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( m_Sheet.SupName, i, m_Sheet.SupSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( m_Sheet.BvType, i, m_Sheet.BvSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( m_Sheet.ActuatorType, i, m_Sheet.ActuatorSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( m_Sheet.DpCName, i, m_Sheet.DpCSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( m_Sheet.UnitDp, i, m_Sheet.UnitSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( m_Sheet.CVType, i, m_Sheet.CVSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( m_Sheet.PipeSerie, i, m_Sheet.PipeSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );
			SetCellBorder( m_Sheet.AccDesc, i, m_Sheet.AccSep - 1, i, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _LIGHTGRAY );

			SetCellProperty( m_Sheet.Pointer, i, CSSheet::_SSCellProperty::CellNoSelection, true );
			SetCellProperty( m_Sheet.ActuatorSep, i, CSSheet::_SSCellProperty::CellNoSelection, true );
		}

		switch( m_pHub->GetSelMode() )
		{
			case CDS_HmHub::SelMode::Direct:
				{
					// Group General description
					SetColumnAlwaysHidden( m_Sheet.Sub, m_Sheet.LoopDesc, false );
					
					// Group Loop
					SetColumnAlwaysHidden( m_Sheet.LoopQ, m_Sheet.LoopSep, false );
					SetColumnAlwaysHidden( m_Sheet.LoopQ, m_Sheet.LoopDp, true );		//Hide Q; Dp
					
					// Group Supply Valve
					SetColumnAlwaysHidden( m_Sheet.SupName, m_Sheet.SupSep, false );
					SetColumnAlwaysHidden( m_Sheet.SupDp, m_Sheet.SupDp, true );		//Hide Dp
					
					// Group Return Valve
					SetColumnAlwaysHidden( m_Sheet.BvType, m_Sheet.BvSep, false );
					SetColumnAlwaysHidden( m_Sheet.BvPreSet, m_Sheet.BvDpMin, true );	//Hide Presetting, Dp, DpSignal, DpMin
					SetColumnAlwaysHidden( m_Sheet.BvCtrlType, false );
					
					// Group Actuator
					SetColumnAlwaysHidden( m_Sheet.ActuatorType, m_Sheet.ActuatorSep, false );
					
					// Group DpC
					SetColumnAlwaysHidden( m_Sheet.DpCName, m_Sheet.DpCSep, true );
					
					// Group Unit
					SetColumnAlwaysHidden( m_Sheet.UnitDp, m_Sheet.UnitSep, true );
					
					// Group Cv
					SetColumnAlwaysHidden( m_Sheet.CVType, m_Sheet.CVSep, true );
					
					// Group Pipe
					SetColumnAlwaysHidden( m_Sheet.PipeSerie, m_Sheet.PipeSep, true );
					
					// Group Accessories
					SetColumnAlwaysHidden( m_Sheet.AccDesc, m_Sheet.Pointer, true );
				}
				break;

			case CDS_HmHub::SelMode::Q:
				{
					// Group General description
					SetColumnAlwaysHidden( m_Sheet.Sub, m_Sheet.LoopDesc, false );
					
					// Group Loop
					SetColumnAlwaysHidden( m_Sheet.LoopQ, m_Sheet.LoopSep, false );
					CreateExpandCollapseColumnButton( m_Sheet.LoopSep, m_HeaderRow.ButDlg, m_Sheet.LoopP, m_Sheet.LoopDT, GetMaxRows() );
					SetFlagShowEvenEmpty( m_Sheet.LoopP, m_Sheet.LoopDT, true );
					ChangeExpandCollapseColumnButtonState( m_Sheet.LoopSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
					
					// Group Supply Valve
					SetColumnAlwaysHidden( m_Sheet.SupName, m_Sheet.SupSep, false );
					CreateExpandCollapseColumnButton( m_Sheet.BvSep, m_HeaderRow.ButDlg, m_Sheet.BvPreSet, m_Sheet.BvDpMin, GetMaxRows() );
					SetFlagCanBeCollapsed( m_Sheet.BvDp, m_Sheet.BvDpSignal, false );
					ChangeExpandCollapseColumnButtonState( m_Sheet.BvSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );

					// Group Return Valve
					SetColumnAlwaysHidden( m_Sheet.BvType, m_Sheet.BvSep, false );
					SetColumnAlwaysHidden( m_Sheet.BvCtrlType, false );
					
					// Group Actuator
					SetColumnAlwaysHidden( m_Sheet.ActuatorType, m_Sheet.ActuatorSep, false );
					
					// Group DpC
					SetColumnAlwaysHidden( m_Sheet.DpCName, m_Sheet.DpCSep, true );
					
					// Group Unit
					SetColumnAlwaysHidden( m_Sheet.UnitDp, m_Sheet.UnitSep, true );
					
					// Group Cv
					SetColumnAlwaysHidden( m_Sheet.CVType, m_Sheet.CVSep, true );
					
					// Group Pipe
					SetColumnAlwaysHidden( m_Sheet.PipeSerie, m_Sheet.PipeSep, true );
					
					// Group Accessories
					SetColumnAlwaysHidden( m_Sheet.AccDesc, m_Sheet.Pointer, true );
				}
				break;

			case CDS_HmHub::SelMode::QDpLoop:
				{
					// Group General description
					SetColumnAlwaysHidden( m_Sheet.Sub, m_Sheet.LoopDesc, false );
					
					// Group Loop
					SetColumnAlwaysHidden( m_Sheet.LoopQ, m_Sheet.LoopSep, false );
					CreateExpandCollapseColumnButton( m_Sheet.LoopSep, m_HeaderRow.ButDlg, m_Sheet.LoopP, m_Sheet.LoopDT, GetMaxRows() );
					SetFlagShowEvenEmpty( m_Sheet.LoopP, m_Sheet.LoopDT, true );
					ChangeExpandCollapseColumnButtonState( m_Sheet.LoopSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
					
					// Group Supply Valve
					SetColumnAlwaysHidden( m_Sheet.SupName, m_Sheet.SupSep, false );
					CreateExpandCollapseColumnButton( m_Sheet.BvSep, m_HeaderRow.ButDlg, m_Sheet.BvPreSet, m_Sheet.BvDpMin, GetMaxRows() );
					SetFlagCanBeCollapsed( m_Sheet.BvDp, m_Sheet.BvDpSignal, false );
					ChangeExpandCollapseColumnButtonState( m_Sheet.BvSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
					
					// Group Return Valve
					SetColumnAlwaysHidden( m_Sheet.BvType, m_Sheet.BvSep, false );
					SetColumnAlwaysHidden( m_Sheet.BvCtrlType, false );
					
					// Group Actuator
					SetColumnAlwaysHidden( m_Sheet.ActuatorType, m_Sheet.ActuatorSep, false );
					
					// Group DpC
					SetColumnAlwaysHidden( m_Sheet.DpCName, m_Sheet.DpCSep, true );
					
					// Group Unit
					SetColumnAlwaysHidden( m_Sheet.UnitDp, m_Sheet.UnitSep, true );
					
					// Group Cv
					SetColumnAlwaysHidden( m_Sheet.CVType, m_Sheet.CVSep, true );
					
					// Group Pipe
					SetColumnAlwaysHidden( m_Sheet.PipeSerie, m_Sheet.PipeSep, true );
					
					// Group Accessories
					SetColumnAlwaysHidden( m_Sheet.AccDesc, m_Sheet.Pointer, true );
				}
				break;

			case CDS_HmHub::SelMode::LoopDetails:
				{
					// Group General description
					SetColumnAlwaysHidden( m_Sheet.Sub, m_Sheet.LoopDesc, false );
					
					// Group Loop
					SetColumnAlwaysHidden( m_Sheet.LoopQ, m_Sheet.LoopSep, false );
					CreateExpandCollapseColumnButton( m_Sheet.LoopSep, m_HeaderRow.ButDlg, m_Sheet.LoopP, m_Sheet.LoopDT, GetMaxRows() );
					SetFlagShowEvenEmpty( m_Sheet.LoopP, m_Sheet.LoopDT, true );
					ChangeExpandCollapseColumnButtonState( m_Sheet.LoopSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
					
					// Group Supply Valve
					SetColumnAlwaysHidden( m_Sheet.SupName, m_Sheet.SupSep, false );
					CreateExpandCollapseColumnButton( m_Sheet.BvSep, m_HeaderRow.ButDlg, m_Sheet.BvPreSet, m_Sheet.BvDpMin, GetMaxRows() );
					SetFlagCanBeCollapsed( m_Sheet.BvDp, m_Sheet.BvDpSignal, false );
					ChangeExpandCollapseColumnButtonState( m_Sheet.BvSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
					
					// Group Return Valve
					SetColumnAlwaysHidden( m_Sheet.BvType, m_Sheet.BvSep, false );
					SetColumnAlwaysHidden( m_Sheet.BvCtrlType, false );
					
					// Group Actuator
					SetColumnAlwaysHidden( m_Sheet.ActuatorType, m_Sheet.ActuatorSep, false );
					
					// Group DpC
					SetColumnAlwaysHidden( m_Sheet.DpCName, m_Sheet.DpCSep, true );
					
					// Group Unit
					SetColumnAlwaysHidden( m_Sheet.UnitDp, m_Sheet.UnitSep, false );
					CreateExpandCollapseColumnButton( m_Sheet.UnitSep, m_HeaderRow.ButDlg, m_Sheet.UnitQref, m_Sheet.UnitDpref, GetMaxRows() );
					SetFlagShowEvenEmpty( m_Sheet.UnitQref, m_Sheet.UnitDpref, true );
					ChangeExpandCollapseColumnButtonState( m_Sheet.UnitSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
					
					// Group Cv
					SetColumnAlwaysHidden( m_Sheet.CVType, m_Sheet.CVSep, false );
					
					// Group Pipe
					SetColumnAlwaysHidden( m_Sheet.PipeSerie, m_Sheet.PipeSep, false );
					CreateExpandCollapseColumnButton( m_Sheet.PipeSep, m_HeaderRow.ButDlg, m_Sheet.PipeLinDp, m_Sheet.PipeV, GetMaxRows() );
					ChangeExpandCollapseColumnButtonState( m_Sheet.PipeSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
					
					// Group Accessories
					SetColumnAlwaysHidden( m_Sheet.AccDesc, m_Sheet.AccSep, false );
					SetColumnAlwaysHidden( m_Sheet.Pointer, m_Sheet.Pointer, true );
				}
				break;
		}
	}
}

void CSSheetTAHub::FillBvColumns( CDS_HydroMod *pHM, long lCurRow )
{
	// Bv Name Lock/Unlock
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	
	// Locking is showed only when BV is not used as a Measuring valve
	if( !( pHM == m_pHub && 0 == m_pHub->GetBalTypeID().CompareNoCase( _T("DPCTYPE_STD") ) ) )
	{
		if( true == m_pHub->IsLocked( pHM->GetpBv()->GetHMObjectType() ) )
			SetFontBold( m_Sheet.BvName, lCurRow, true );
		else
			SetFontBold( m_Sheet.BvName, lCurRow, false );
	}
	
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	
	// Setting
	SetStaticText( m_Sheet.BvPreSet, lCurRow, pHM->GetpBv()->GetSettingStr() );
	if( eb3False == pHM->GetpBv()->CheckMinOpen() )
		SetForeColor( m_Sheet.BvPreSet, lCurRow, _RED );
	else
		SetForeColor( m_Sheet.BvPreSet, lCurRow, _BLACK );

	// Dp
	if( false == pHM->GetpBv()->IsKvSignalEquipped() )
	{
		FormatCUDouble( m_Sheet.BvDp, lCurRow, _U_DIFFPRESS, pHM->GetpBv()->GetDp() );
		if( eb3False == pHM->GetpBv()->CheckDpMinMax() )
			SetForeColor( m_Sheet.BvDp, lCurRow, _RED );
		else
			SetForeColor( m_Sheet.BvDp, lCurRow, _BLACK );
	}
	else
		SetStaticText( m_Sheet.BvDp, lCurRow, _T("") );
	
	// Dp 100%
	FormatCUDouble( m_Sheet.BvDpMin, lCurRow, _U_DIFFPRESS, pHM->GetpBv()->GetDpMin( false ) );
	
	// Dp Signal
	if( true == pHM->GetpBv()->IsKvSignalEquipped() )
	{
		FormatCUDouble( m_Sheet.BvDpSignal, lCurRow, _U_DIFFPRESS, pHM->GetpBv()->GetDpSignal() );
		if( eb3False == pHM->GetpBv()->CheckDpMinMax( true ) )
			SetForeColor( m_Sheet.BvDpSignal, lCurRow, _RED );
		else
			SetForeColor( m_Sheet.BvDpSignal, lCurRow, _BLACK );
	}
	else
		SetStaticText( m_Sheet.BvDpSignal, lCurRow, _T("") );
}

void CSSheetTAHub::FillSheetHub()
{
	if( NULL == m_pHub )
		return;

	// Keep a trace of selected rows
	CArray<long> arlSelRows;
	GetSelectedRows( &arlSelRows );
	UnSelectMultipleRows();
	long lCurRow = m_HeaderRow.FirstCirc;
	
	// All cells are not selectable
	SetCellProperty( m_Sheet.Pos, lCurRow, m_Sheet.Pointer, GetMaxRows(), CSSheet::_SSCellProperty::CellNoSelection, true );
	// Save pointer on HM
	SetCellParam( m_Sheet.Pointer, lCurRow, (long)m_pHub );
	CDS_HydroMod *ptestHM = GetpHM( lCurRow );

	// LabelID
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	FormatEditText( m_Sheet.LabelID, lCurRow, m_pHub->GetHMName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES | ES_MULTILINE, TASApp.GetModuleNameMaxChar() );

	// Description
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	FormatEditText( m_Sheet.LabelID, lCurRow, m_pHub->GetHMName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES | ES_MULTILINE, HUB_MAXLENGTH_DESCRIPTION );
	
	// Total Flow && Total Dp
	if( m_pHub->GetSelMode() != CDS_HmHub::SelMode::Direct )
	{
		FormatCUDouble( m_Sheet.LoopQ, lCurRow, _U_FLOW, m_pHub->GetQ() );
		if( false == m_pHub->CheckMaxTotalFlow().IsEmpty() )
			SetForeColor( m_Sheet.LoopQ, lCurRow, _RED );
		else
			SetForeColor( m_Sheet.LoopQ, lCurRow, _BLACK );

		FormatCUDouble( m_Sheet.LoopDp, lCurRow, _U_DIFFPRESS, m_pHub->GetDp() );
	}
	
	// ?????????????
	// SetTypeMaxEditLen(TASApp.GetModuleNameMaxChar());

	// Valve Name
	SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	IDPTR IDPtrRetValve = m_pTADB->Get( m_pHub->GetReturnValveID() );
	IDPTR IDPtrSupValve = m_pTADB->Get( m_pHub->GetSupplyValveID() );

	if( true == m_pHub->IsDpCExist() )
	{
		if( m_pHub->GetSelMode() != CDS_HmHub::SelMode::Direct && NULL != *IDPtrRetValve.ID )
		{
			if( true == m_pHub->IsLocked( CDS_HydroMod::eHMObj::eDpC ) )
				SetFontBold( m_Sheet.DpCName, lCurRow, true );
			else
				SetFontBold( m_Sheet.DpCName, lCurRow, false );
			
			// DpC
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			CDB_DpController *pDpC = (CDB_DpController *)( m_pHub->GetpDpC()->GetIDPtr().MP );
			if( NULL == pDpC )
				return;
			FormatEditCombo( m_Sheet.DpCName, lCurRow, ( (CDB_HubValv *)IDPtrRetValve.MP )->GetName(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, (LPARAM)IDPtrRetValve.MP );
			if( true == m_pHub->IsLocked( CDS_HydroMod::eHMObj::eDpC ) )
				SetFontBold( m_Sheet.DpCName, lCurRow, true );
			else
				SetFontBold( m_Sheet.DpCName, lCurRow, false );

			// DpC - Dpl range
			CString str = pDpC->GetFormatedDplRange( false ).c_str();
			SetStaticText( m_Sheet.DpCDpLr, lCurRow, str );
			
			// DpC Setting
			FormatCUDouble( m_Sheet.DpCSet, lCurRow, _U_NODIM, m_pHub->GetpDpC()->GetDpCSetting() );
			
			// Dp
			FormatCUDouble( m_Sheet.DpCDp, lCurRow, _U_DIFFPRESS, m_pHub->GetpDpC()->GetDp() );
			if( eb3False == m_pHub->GetpDpC()->CheckDpMinMax() )
				SetForeColor( m_Sheet.DpCDp, lCurRow, _RED );
			else
				SetForeColor( m_Sheet.DpCDp, lCurRow, _BLACK );
			
			// DpL
			FormatCUDouble( m_Sheet.DpCDpl, lCurRow, _U_DIFFPRESS, m_pHub->GetpDpC()->GetDpL() );
			if( eb3False == m_pHub->GetpDpC()->CheckDpLRange() )
				SetForeColor( m_Sheet.DpCDpl, lCurRow, _RED );
			else
				SetForeColor( m_Sheet.DpCDpl, lCurRow, _BLACK );
			
			// DpMin
			FormatCUDouble( m_Sheet.DpCDpMin, lCurRow, _U_DIFFPRESS, m_pHub->GetpDpC()->GetDpmin( false ) );

			// BV
			SetStaticText( m_Sheet.BvName, lCurRow, ( (CDB_TAProduct *)( (CDB_HubValv *)IDPtrSupValve.MP )->GetValveIDPtr().MP )->GetName() );
			SetCellParam( m_Sheet.BvName, lCurRow, (LPARAM)IDPtrSupValve.MP );
			FillBvColumns( m_pHub, lCurRow );
		}
		else
		{
			CString str;
			LPARAM lParam=(LPARAM)0;

			m_pSCB->InitList( m_pHub, CDlgComboBoxHM::eSCB::SCB_DS_HubDpCName );
			if( NULL != *IDPtrRetValve.ID )
				lParam = (LPARAM)( IDPtrRetValve.MP );
			else
				m_pSCB->GetFirstListItem( str, lParam );
			
			m_pHub->SetReturnValveID( ( (CDB_HubValv *)lParam )->GetIDPtr().ID );
			FormatEditCombo( m_Sheet.DpCName, lCurRow, ( (CDB_HubValv *)lParam )->GetName(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, lParam );
			CDB_DpController *pDpC = (CDB_DpController *)( ( (CDB_HubValv *)lParam )->GetValveIDPtr().MP );
			ASSERT( pDpC );
			if( NULL == pDpC )
				return;
			
			// DpC - Dpl range
			str = pDpC->GetFormatedDplRange( false ).c_str();
			SetStaticText( m_Sheet.DpCDpLr, lCurRow, str );
			
			// BV
			IDPtrSupValve = m_pTADB->Get( m_pHub->GetSupplyValveID() );
			SetStaticText( m_Sheet.BvName, lCurRow, ( (CDB_TAProduct *)( ( (CDB_HubValv *)IDPtrSupValve.MP )->GetValveIDPtr().MP ) )->GetName() );
			SetCellParam( m_Sheet.BvName, lCurRow, (LPARAM)( IDPtrSupValve.MP ) );
			FillBvColumns( m_pHub, lCurRow );
		}

		if( NULL != pRViewSSelHub )
		{
			pRViewSSelHub->RefreshMvLoc();
		}
	}
	else	// No DpC
	{
		if( NULL != *IDPtrRetValve.ID )
		{
			// Return Side
			FormatEditCombo( m_Sheet.BvName, lCurRow, ( (CDB_TAProduct *)( ( (CDB_HubValv *)IDPtrRetValve.MP )->GetValveIDPtr().MP ) )->GetName(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, (LPARAM)( IDPtrRetValve.MP ) );

			// Fill BV
			if( m_pHub->GetSelMode() != CDS_HmHub::SelMode::Direct )
			{
				if( true == m_pHub->IsBvExist() )											// Bv doesn't exist in case of ShutOff only selection
				{
					FillBvColumns( m_pHub, lCurRow );
				}
				else if( true == m_pHub->IsShutoffValveExist(CDS_HydroMod ::eHMObj::eShutoffValveReturn ) )		// Shut-Off Only
				{
					FormatCUDouble( m_Sheet.BvDp, lCurRow, _U_DIFFPRESS, m_pHub->GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveReturn )->GetDp() );
					SetForeColor( m_Sheet.BvDp, lCurRow, _BLACK );
				}
				
				// Fill Supply side
				if( true == m_pHub->IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveSupply ) )			// Shut-Off
				{
					SetStaticText( m_Sheet.SupName, lCurRow, ( (CDB_TAProduct *)( ( m_pHub->GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveSupply )->GetIDPtr().MP ) ) )->GetName() );
					FormatCUDouble( m_Sheet.SupDp, lCurRow, _U_DIFFPRESS, m_pHub->GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveSupply )->GetDp() );
					SetForeColor( m_Sheet.SupDp, lCurRow, _BLACK );
				}
				else 
					ASSERT( 0 );
			}				
		}
		else // No Selection already done...
		{
			CString str;
			LPARAM lParam = (LPARAM)0;

			m_pSCB->InitList( m_pHub, CDlgComboBoxHM::eSCB::SCB_DS_HubValveName );
			m_pSCB->GetFirstListItem( str, lParam );
			if( lParam )
			{
				m_pHub->SetReturnValveID( ( (CDB_HubStaValv *)lParam )->GetIDPtr().ID );
				FormatEditCombo( m_Sheet.BvName, lCurRow, ( (CDB_TAProduct *)( (CDB_HubValv *)lParam )->GetValveIDPtr().MP )->GetName(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, lParam );
			}
			else
			{
				m_pHub->SetReturnValveID( _T("") );
				m_pHub->SetSupplyValveID( _T("") );
			}
			
			// Always show Dp column
			if( m_pHub->GetSelMode() != CDS_HmHub::SelMode::Direct )
				SetColumnAlwaysHidden( m_Sheet.BvDp, false );
			
			// Show column DpSignal only for venturi valve and when we are not in direct mode
			if( m_pHub->GetSelMode() == CDS_HmHub::SelMode::Direct || m_pHub->GetBalTypeID() != _T("RVTYPE_VV") ) 
			{
				SetColumnAlwaysHidden( m_Sheet.BvDpSignal, m_Sheet.BvDpSignal, true );
			}
			else
			{	
				SetColumnAlwaysHidden( m_Sheet.BvDpSignal, m_Sheet.BvDpSignal, false );
				if( TRUE == IsColHidden( m_Sheet.BvDp) )
					ChangeExpandCollapseColumnButtonState( m_Sheet.BvSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
			}
		}
	}

	// Hide empty columns
	HideEmptyColumns();

	// Check now if we can hide some collapse/expand buttons or not
	// Remark: this is only a check on the 'm_pSheetTAHub' sheet (top sheet of the two ones) !!!
	VerifyExpandCollapseColumnButtons( CRect( m_Sheet.LabelID, m_HeaderRow.FirstCirc, m_Sheet.AccSep, lCurRow ), m_HeaderRow.ButDlg );

	SelectMutipleRows( &arlSelRows );

	// Draw a horizontal black line below all.
	_DrawBlackLine();
}

void CSSheetTAHub::FillSheetHubStations()
{
	if( NULL == m_pHub) 
		return;
	
	bool fExistTerminalValve = false;			// true if at least one valve of terminal valve is selected.
	bool fExistVenturiBalancingValve = false;	// true if at least one valve of venturi balancing valve is selected.
	bool fAllValveAreTV = true;					// All valve are TV
	bool fAllCvAreNone = true;					// Used to hide columns CV
	bool fAllCvAreOnOff = true;					// Used to hide column Authority
	bool fTACvValveExist = false;				// Used to hide column Name CV

	// Keep a trace of selected rows
	CArray<long> arlSelRows;
	GetSelectedRows( &arlSelRows );
	UnSelectMultipleRows();

	CRank HMList;
	CString str;
	long lCurRow = m_HeaderRow.FirstCirc;
	int iHMRows = 0;
	for( IDPTR IDPtr =m_pHub->GetFirst(); *IDPtr.ID; IDPtr = m_pHub->GetNext() )
	{		
		CDS_HmHubStation *pHubStat= (CDS_HmHubStation*)( IDPtr.MP );
		HMList.Add( str, pHubStat->GetPos(), (LPARAM)( IDPtr.MP ) );
		iHMRows += 1;	// 1 row 
	}
	iHMRows += lCurRow;
	
	// All cells are not selectable
	SetCellProperty( m_Sheet.Pos, lCurRow, m_Sheet.Pointer, GetMaxRows(), CSSheet::_SSCellProperty::CellNoSelection, true );
	
	CDS_HmHubStation *pHubStat;
	LPARAM lParam = (LPARAM)0;
	long lRowStart = lCurRow;
	for( BOOL fContinue = HMList.GetFirst( str, lParam); (TRUE == fContinue ); fContinue = HMList.GetNext( str, lParam) )
	{
		pHubStat = (CDS_HmHubStation *)lParam;
		
		// Check Box
		if( true == pHubStat->IsPlugged() )
			FormatStaticText( -1, -1, -1, -1, _T("") );
		
		SetCheckBox( m_Sheet.CheckBox, lCurRow, _T(""), !pHubStat->IsPlugged(), true );

		// Save current Hm
		SetCellParam( m_Sheet.Pointer, lCurRow, lParam );
		TCHAR buf[10];
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );

		// Position
		_stprintf_s( buf, SIZEOFINTCHAR( buf ), _T("%d"), pHubStat->GetPos() );
		if( true == pHubStat->IsCircuitIndex() )
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_INDEXCOLOR );
		SetStaticText( m_Sheet.Pos, lCurRow, buf );	     				

		// LabelID
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		FormatEditText( m_Sheet.LabelID, lCurRow, pHubStat->GetHMName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES | ES_MULTILINE, TASApp.GetModuleNameMaxChar() );

		// Description
		FormatEditText( m_Sheet.LoopDesc, lCurRow, pHubStat->GetDescription(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER | SSS_ELLIPSES | ES_MULTILINE, HUB_MAXLENGTH_DESCRIPTION );
		
		// Flow
		SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
		SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );

		switch( pHubStat->GetpTermUnit()->GetQType() )
		{
			case CTermUnit::_QType::Q:
				FormatEditDouble( m_Sheet.LoopQ, lCurRow, _U_FLOW, pHubStat->GetpTermUnit()->GetQ(), SSS_ALIGN_CENTER | SSS_ELLIPSES );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
				FormatEditDouble( m_Sheet.LoopP, lCurRow, _U_TH_POWER, 0, SSS_ALIGN_CENTER | SSS_ELLIPSES );
				FormatEditDouble( m_Sheet.LoopDT, lCurRow, _U_NODIM, 0, SSS_ALIGN_CENTER | SSS_ELLIPSES );
				break;

			case CTermUnit::_QType::PdT:
				FormatEditDouble( m_Sheet.LoopP, lCurRow, _U_TH_POWER, pHubStat->GetpTermUnit()->GetP(), SSS_ALIGN_CENTER | SSS_ELLIPSES );
				FormatEditDouble( m_Sheet.LoopDT, lCurRow, _U_NODIM, pHubStat->GetpTermUnit()->GetDT(), SSS_ALIGN_CENTER | SSS_ELLIPSES );
					
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
				FormatEditDouble( m_Sheet.LoopQ, lCurRow, _U_FLOW, pHubStat->GetpTermUnit()->GetQ(), SSS_ALIGN_CENTER | SSS_ELLIPSES );
				break;
		}

		// ????????????????
//		SetTypeMaxEditLen(TASApp.GetModuleNameMaxChar());

		if( false == pHubStat->IsPlugged() )
		{
			// Total DP in case of Q-Dp selection the totalDp column becomes editable 
			if( CDS_HmHub::SelMode::QDpLoop == ( (CDS_HmHub*)pHubStat->GetIDPtr().PP )->GetSelMode() )
			{
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
				FormatEditDouble( m_Sheet.LoopDp, lCurRow, _U_DIFFPRESS, pHubStat->GetpTermUnit()->GetDp(), SSS_ALIGN_CENTER | SSS_ELLIPSES );
			}
			else
				FormatCUDouble( m_Sheet.LoopDp, lCurRow, _U_DIFFPRESS, pHubStat->GetDp() );

			// Unit Dp
			if( CDS_HmHub::SelMode::LoopDetails == m_pHub->GetSelMode() )
			{
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				switch( pHubStat->GetpTermUnit()->GetDpType() )
				{
					case CDS_HmHubStation::eDpType::Dp:
						FormatEditDouble( m_Sheet.UnitDp, lCurRow, _U_DIFFPRESS, pHubStat->GetpTermUnit()->GetDp(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
						FormatEditDouble( m_Sheet.UnitQref, lCurRow, _U_FLOW, 0, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						FormatEditDouble( m_Sheet.UnitDpref, lCurRow, _U_DIFFPRESS, 0, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						break;

					case CDS_HmHubStation::eDpType::QDpref:
						{
							double dQref, dDpref;			
							pHubStat->GetpTermUnit()->GetDp( NULL, &dDpref, &dQref );
							FormatEditDouble( m_Sheet.UnitQref, lCurRow, _U_FLOW, dQref, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
							FormatEditDouble( m_Sheet.UnitDpref, lCurRow, _U_DIFFPRESS, dDpref , SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER);
							SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
							FormatEditDouble( m_Sheet.UnitDp, lCurRow, _U_DIFFPRESS, pHubStat->GetpTermUnit()->GetDp(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
						}
						break;
				}
			}

			// Valve Type
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
			SetStaticText( m_Sheet.BvCtrlType, lCurRow, _T("") );
			FormatEditCombo( m_Sheet.BvType, lCurRow, ( (CDB_StringID *)pHubStat->GetBalTypeIDPtr().MP )->GetString(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER, (LPARAM)( pHubStat->GetBalTypeIDPtr().MP ) );
			if( pHubStat->GetBalTypeID() == _T("RVTYPE_BVC") )
			{
				fExistTerminalValve = true;
				if( NULL != pHubStat->GetpCV() )
				{
					_string str = CDB_ControlProperties::GetCvCtrlTypeStr( pHubStat->GetpCV()->GetCtrlType() );
					FormatEditCombo( m_Sheet.BvCtrlType, lCurRow, str.c_str(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER, pHubStat->GetpCV()->GetCtrlType() + 1 );
				}
			}
			else
				fAllValveAreTV = false;
			
			if( pHubStat->GetBalTypeID() == _T("RVTYPE_VV") )
				fExistVenturiBalancingValve = true; 
			
			// Valve Name
			IDPTR IDPtr = m_pTADB->Get( pHubStat->GetReturnValveID() );
			if( NULL != *IDPtr.ID )
				FormatEditCombo( m_Sheet.BvName, lCurRow, ( (CDB_TAProduct *)( (CDB_HubStaValv *)IDPtr.MP )->GetValveIDPtr().MP )->GetName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER, (LPARAM)( IDPtr.MP ) );
			else
			{
				m_pSCB->InitList( pHubStat, CDlgComboBoxHM::eSCB::SCB_DS_StationValveName );
				CString str;
				LPARAM lParam = (LPARAM)0;
				m_pSCB->GetFirstListItem( str, lParam );
				if( lParam )
				{
					pHubStat->SetReturnValveID( ( (CDB_HubStaValv *)lParam)->GetIDPtr().ID );
					FormatEditCombo( m_Sheet.BvName, lCurRow, ( (CDB_TAProduct *)( (CDB_HubStaValv *)lParam )->GetValveIDPtr().MP )->GetName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER, lParam );
				}
			}

			// BV
			if( ( (CDS_HmHub*)pHubStat->GetIDPtr().PP )->GetSelMode() != CDS_HmHub::SelMode::Direct )
			{
				if( true == pHubStat->IsBvExist() )
				{
					SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
					if( true == pHubStat->IsLocked( pHubStat->GetpBv()->GetHMObjectType() ) )
						SetFontBold( m_Sheet.BvName, lCurRow, true );
					else
						SetFontBold( m_Sheet.BvName, lCurRow, false );
					SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	
					// Setting
					SetStaticText( m_Sheet.BvPreSet, lCurRow, pHubStat->GetpBv()->GetSettingStr() );
					if( eb3False == pHubStat->GetpBv()->CheckMinOpen() )
						SetForeColor( m_Sheet.BvPreSet, lCurRow, _RED );
					else
						SetForeColor( m_Sheet.BvPreSet, lCurRow, _BLACK );

					// Dp
					if( false == pHubStat->GetpBv()->IsKvSignalEquipped() )
					{
						FormatCUDouble( m_Sheet.BvDp, lCurRow, _U_DIFFPRESS, pHubStat->GetpBv()->GetDp() );

						if( eb3False == pHubStat->GetpBv()->CheckDpMinMax() )
							SetForeColor( m_Sheet.BvDp, lCurRow, _RED );
						else
							SetForeColor( m_Sheet.BvDp, lCurRow, _BLACK );
					}
					else
						SetStaticText( m_Sheet.BvDp, lCurRow, _T("") );
					
					// Dp 100%
					FormatCUDouble( m_Sheet.BvDpMin, lCurRow, _U_DIFFPRESS, pHubStat->GetpBv()->GetDpMin( false ) );
					
					// Dp Signal
					if( true == pHubStat->GetpBv()->IsKvSignalEquipped() )
						FormatCUDouble( m_Sheet.BvDpSignal, lCurRow, _U_DIFFPRESS, pHubStat->GetpBv()->GetDpSignal() );
					else
						SetStaticText( m_Sheet.BvDpSignal, lCurRow, _T("") );
				}
				else if( true == pHubStat->IsCvExist() && pHubStat->GetBalTypeID() == _T("RVTYPE_BVC") )
				{
					SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
					if( true == pHubStat->IsLocked( CDS_HydroMod::eHMObj::eCV) )
						SetFontBold( m_Sheet.BvName, lCurRow, true );
					else
						SetFontBold( m_Sheet.BvName, lCurRow, false );
					SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
					
					// Setting
					SetStaticText( m_Sheet.BvPreSet, lCurRow, pHubStat->GetpCV()->GetSettingStr() );
					if( eb3False == pHubStat->GetpCV()->CheckMinOpen() )
						SetForeColor( m_Sheet.BvPreSet, lCurRow, _RED );
					else
						SetForeColor( m_Sheet.BvPreSet, lCurRow, _BLACK );
					
					// Dp
					FormatCUDouble( m_Sheet.BvDp, lCurRow, _U_DIFFPRESS, pHubStat->GetpCV()->GetDp() );
					if( eb3False == pHubStat->GetpCV()->CheckDpMinMax() )
						SetForeColor( m_Sheet.BvDp, lCurRow, _RED );
					else
						SetForeColor( m_Sheet.BvDp, lCurRow, _BLACK );
					
					// Dp 100%
					FormatCUDouble( m_Sheet.BvDpMin, lCurRow, _U_DIFFPRESS, pHubStat->GetpCV()->GetDpMin( false ) );
				}
			}

			// Fill Supply side (intentionally set after 'Valve on station return' group !!)
			if( true == pHubStat->IsShutoffValveExist( CDS_HydroMod::eHMObj::eShutoffValveSupply ) )	// Shut-Off
			{
				SetStaticText( m_Sheet.SupName, lCurRow, ( (CDB_TAProduct *)( pHubStat->GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveSupply )->GetIDPtr().MP ) )->GetName() );
				FormatCUDouble( m_Sheet.SupDp, lCurRow, _U_DIFFPRESS, pHubStat->GetpShutoffValve( CDS_HydroMod::eHMObj::eShutoffValveSupply )->GetDp(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				SetForeColor( m_Sheet.SupDp, lCurRow, _BLACK );
			}
			else
				ASSERT( 0 );

			// Actuator Type
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );
			IDPtr= m_pTADB->Get( pHubStat->GetReturnActuatorID() );
			if( NULL != *IDPtr.ID )
				FormatEditCombo( m_Sheet.ActuatorType, lCurRow, ( (CDB_Actuator *)IDPtr.MP )->GetName(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER, (LPARAM)( IDPtr.MP ) );
			else
			{
				CString str;
				LPARAM lParam = (LPARAM)0;
				if( pHubStat->GetBalTypeID() == _T("RVTYPE_BVC") )
				{
					m_pSCB->InitList( pHubStat, CDlgComboBoxHM::eSCB::SCB_DS_StationActuatorType );
					m_pSCB->GetFirstListItem( str, lParam );

					if( lParam )
					{
						pHubStat->SetReturnActuatorID( ( (CDB_StringID *)lParam)->GetIDPtr().ID );
					}
					else
						pHubStat->SetReturnActuatorID( _T("") );

					FormatEditCombo( m_Sheet.ActuatorType, lCurRow, str, SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER, lParam );
				}
				else
					SetStaticText( m_Sheet.ActuatorType, lCurRow, _T("") );
			}
			
			// CV
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

			if( pHubStat->GetBalTypeID() == _T("RVTYPE_BVC") )
				SetStaticText( m_Sheet.CVType, lCurRow, pHubStat->GetCvTypeString( CDS_HmHubStation::eCVTAKV::eNone ) );
			else
			{
				CString str = pHubStat->GetCvTypeString( pHubStat->GetCvTaKv() );
				if( pHubStat->GetCvTaKv() != CDS_HmHubStation::eCVTAKV::eNone )
				{
					str += _T(" - ");
					str += CDB_ControlProperties::GetCvCtrlTypeStr( pHubStat->GetCvCtrlType(), false ).c_str();
				}
				FormatEditCombo( m_Sheet.CVType, lCurRow, str, SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER, (LPARAM)pHubStat );
			}
			
			SetStaticText( m_Sheet.CVName, lCurRow, _T("") );
			SetStaticText( m_Sheet.CVDesc, lCurRow, _T("") );
			SetStaticText( m_Sheet.CVKvsMax, lCurRow, _T("") );
			SetStaticText( m_Sheet.CVKvs, lCurRow, _T("") );
			SetStaticText( m_Sheet.CVDp, lCurRow, _T("") );
			SetStaticText( m_Sheet.CVAuth, lCurRow, _T("") );
			
			if( true == pHubStat->IsCvExist() && pHubStat->GetBalTypeID() != _T("RVTYPE_BVC") )
			{
				fAllCvAreNone = false;
				if( false == pHubStat->GetpCV()->IsOn_Off() )
					fAllCvAreOnOff = false;
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
				
				if( true == pHubStat->GetpCV()->IsTaCV() )
				{
					fTACvValveExist = true;
					FormatComboList( m_Sheet.CVName, lCurRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
					ComboBoxSendMessage( m_Sheet.CVName, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );
					if( NULL != pHubStat->GetpCV()->GetCvIDPtr().MP )
					{
						ComboBoxSendMessage( m_Sheet.CVName, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( ( (CDB_RegulatingValve*)pHubStat->GetpCV()->GetCvIDPtr().MP )->GetName() ) );
						ComboBoxSendMessage( m_Sheet.CVName, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
					}
					SetCellParam( m_Sheet.CVName, lCurRow, (LPARAM)pHubStat->GetpCV() );
				
					if( true == pHubStat->IsLocked( CDS_HmHubStation::eHMObj::eCV ) )
						SetFontBold( m_Sheet.CVName, lCurRow, true );
					else
						SetFontBold( m_Sheet.CVName, lCurRow, false );
					
					// Kv and Kvmax are displayed only  when CV is not Presetable 
					if( false == pHubStat->GetpCV()->IsPresettable() && NULL != pHubStat->GetpCV()->GetCvIDPtr().MP )
					{
						// Kvs max	
						FormatCUDouble( m_Sheet.CVKvsMax, lCurRow, _C_KVCVCOEFF, pHubStat->GetpCV()->GetKvsmax() );
						// Kvs	
						FormatCUDouble( m_Sheet.CVKvs, lCurRow, _C_KVCVCOEFF, pHubStat->GetpCV()->GetKvs() );
					}
				}
				else
				{
					SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
					SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
					FormatEditText( m_Sheet.CVDesc, lCurRow, pHubStat->GetpCV()->GetDescription(), SSS_ALIGN_LEFT | SSS_ALIGN_VCENTER );
					
					// Kvs max	
					FormatCUDouble( m_Sheet.CVKvsMax, lCurRow, _C_KVCVCOEFF, pHubStat->GetpCV()->GetKvsmax() );
					
					// Kvs	
					FormatEditCombo( m_Sheet.CVKvs, lCurRow, _C_KVCVCOEFF, pHubStat->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, (LPARAM)pHubStat->GetpCV() );

					if( true == pHubStat->IsLocked( CDS_HmHubStation::eHMObj::eCV ) )
						SetFontBold( m_Sheet.CVKvs, lCurRow, true );
					else
						SetFontBold( m_Sheet.CVKvs, lCurRow, false );
				}
				
				// Dp
				FormatCUDouble( m_Sheet.CVDp, lCurRow, _U_DIFFPRESS, pHubStat->GetpCV()->GetDp() );
				if( eb3False == pHubStat->GetpCV()->CheckDpMinMax() )
					SetForeColor( m_Sheet.CVDp, lCurRow, _RED );
				else
					SetForeColor( m_Sheet.CVDp, lCurRow, _BLACK );

				// Authority
				if( false == pHubStat->GetpCV()->IsOn_Off() )
				{
					FormatCUDouble( m_Sheet.CVAuth, lCurRow, _U_NODIM, pHubStat->GetpCV()->GetAuth() );
					if( eb3False == pHubStat->GetpCV()->CheckMinAuthor() )
						SetForeColor( m_Sheet.CVAuth, lCurRow, _RED );
					else
						SetForeColor( m_Sheet.CVAuth, lCurRow, _BLACK );
				}
			}

			// Circuit Pipe
			SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			CPipes *pPipe = pHubStat->GetpCircuitPrimaryPipe();
			if( NULL != pPipe )
			{
				// Serie
				FormatComboList( m_Sheet.PipeSerie, lCurRow );
				ComboBoxSendMessage( m_Sheet.PipeSerie, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );

				// Pipe length exist but size and serie are not yet defined, flow is 0 for instance
				if( NULL != pPipe->GetPipeSeries() )
				{
					ComboBoxSendMessage( m_Sheet.PipeSerie, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( pPipe->GetPipeSeries()->GetName() ) );
					ComboBoxSendMessage( m_Sheet.PipeSerie, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
				}
				SetCellParam( m_Sheet.PipeSerie, lCurRow, (LPARAM)pPipe );
				
				// Size
				FormatComboList( m_Sheet.PipeSize, lCurRow );
				ComboBoxSendMessage( m_Sheet.PipeSize, lCurRow, SS_CBM_RESETCONTENT, 0, 0 );
				
				// Pipe length exist but size is not yet defined, flow is 0 for instance
				if( NULL != pPipe->GetIDPtr().MP )
				{
					ComboBoxSendMessage( m_Sheet.PipeSize, lCurRow, SS_CBM_ADDSTRING, 0, (LPARAM)( ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName() ) );
					ComboBoxSendMessage( m_Sheet.PipeSize, lCurRow, SS_CBM_SETCURSEL, 0, 0 );
				}
				SetCellParam( m_Sheet.PipeSize, lCurRow, (LPARAM)pPipe );
				if( false == pHubStat->IsLocked( pPipe->GetLocate() ) )
					SetFontBold( m_Sheet.PipeSize, lCurRow, false );
				else
					SetFontBold( m_Sheet.PipeSize, lCurRow, true );

				// Length
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_EDITCOLOR );
				FormatEditDouble( m_Sheet.PipeL, lCurRow, _U_LENGTH, pPipe->GetLength(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, false );

				// Dp
				SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
				double dSI = pPipe->GetPipeDp();
				FormatCUDouble( m_Sheet.PipeDp, lCurRow, _U_DIFFPRESS, pPipe->GetPipeDp() );
				
				// Linear Dp
				FormatCUDouble( m_Sheet.PipeLinDp, lCurRow, _U_LINPRESSDROP, pPipe->GetLinDp() );
				if( eb3False == pPipe->CheckLinDpMinMax() )
					SetForeColor( m_Sheet.PipeLinDp, lCurRow, _RED );
				else
					SetForeColor( m_Sheet.PipeLinDp, lCurRow, _BLACK );
				
				// Velocity
				FormatCUDouble( m_Sheet.PipeV, lCurRow, _U_VELOCITY, pPipe->GetVelocity() );
				if( eb3False == pPipe->CheckVelocityMinMax() )
					SetForeColor( m_Sheet.PipeV, lCurRow, _RED );
				else
					SetForeColor( m_Sheet.PipeV, lCurRow, _BLACK );
			}
			
			// Accessories
			if( true == m_fPrinting )
			{
				CPipes::CSingularity Singularity;
				int iSingulPos = pPipe->GetFirstSingularity( &Singularity );
				for( int r = 0; r < pPipe->GetSingularityCount(); r++ ) // One line by accessory
				{
					if( 0 != r )
						lCurRow++;
					
					// Description
					SetStaticText( m_Sheet.AccDesc, lCurRow, Singularity.GetDescription() );
					
					// Dp
					FormatCUDouble( m_Sheet.AccDp, lCurRow, _U_DIFFPRESS, pPipe->GetSingularityDp( &Singularity ) );

					iSingulPos = pPipe->GetNextSingularity( iSingulPos, &Singularity );
				}
			}
			else
			{
				CPipes::CSingularity Singularity;

				SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
				
				pPipe->GetSingularity( 0, &Singularity );
				int iCount = pPipe->GetSingularityCount(); 
				if( iCount > 2 || ( 2 == iCount && _tcscmp( Singularity.GetSingulIDPtr().ID, _T("SINGUL_NONE") ) ) )
				{
					CString str1;
					str1.Format( _T("%d"), pPipe->GetSingularityCount() );
					FormatString( str, IDS_HMCALC_SINGULFOUND, (LPCTSTR)str1 );
					SetStaticText( m_Sheet.AccDesc, lCurRow, str);
				}
				else
				{
					if( 2 == iCount )
					{	
						// First singularity is Singul_None
						int iPos = pPipe->GetFirstSingularity( &Singularity );
						if( 0 == Singularity.m_nID )
							pPipe->GetNextSingularity( iPos, &Singularity );
					}
					SetStaticText( m_Sheet.AccDesc, lCurRow, Singularity.GetDescription() );
				}
				
				// Dp
				FormatCUDouble( m_Sheet.AccDp, lCurRow, _U_DIFFPRESS, pPipe->GetSingularityTotalDp( true ) );
			}
		}
		lCurRow++;
	}
	
	// In LoopDetails show Control valve group if one circuit is different from TV
	if( CDS_HmHub::SelMode::LoopDetails == m_pHub->GetSelMode() )
	{
		SetColumnAlwaysHidden( m_Sheet.CVType, m_Sheet.CVSep, false );
		if( true == fAllValveAreTV )
			SetColumnAlwaysHidden( m_Sheet.CVType, m_Sheet.CVSep, true );
		else
		{
			if( true == fAllCvAreNone )
				SetColumnAlwaysHidden( m_Sheet.CVName, m_Sheet.CVSep - 1, true );
			else
			{
				if( true == fAllCvAreOnOff )
					SetColumnAlwaysHidden( m_Sheet.CVAuth, m_Sheet.CVAuth, true );
				if( false == fTACvValveExist )
					SetColumnAlwaysHidden( m_Sheet.CVName, m_Sheet.CVName, true );
			}
		}
	}
	
	// Hide Actuator group if no Actuator needed
	if( true == fExistTerminalValve )
	{
		SetColumnAlwaysHidden( m_Sheet.BvCtrlType, false );
		SetColumnAlwaysHidden( m_Sheet.ActuatorType, m_Sheet.ActuatorSep, false );
	}
	else 
	{
		SetColumnAlwaysHidden( m_Sheet.BvCtrlType, true );
		SetColumnAlwaysHidden( m_Sheet.ActuatorType, m_Sheet.ActuatorSep, true );
	}

	// Dp Signal column showed only for VV and when we are not in Direct mode
	if( CDS_HmHub::SelMode::Direct == m_pHub->GetSelMode() || false == fExistVenturiBalancingValve )
		SetColumnAlwaysHidden( m_Sheet.BvDpSignal, m_Sheet.BvDpSignal, true );
	else
	{	
		SetColumnAlwaysHidden( m_Sheet.BvDpSignal, m_Sheet.BvDpSignal, false );
		if( TRUE == IsColHidden( m_Sheet.BvDp ) )
			ChangeExpandCollapseColumnButtonState( m_Sheet.BvSep, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
	}

	// Hide empty columns
	HideEmptyColumns();

	// Check now if we can hide collapse button or not
	// Remark: this is only a check on the 'm_pSheetTAStation' sheet (bottom sheet of the two ones) !!!
	VerifyExpandCollapseColumnButtons( CRect( m_Sheet.LabelID, m_HeaderRow.FirstCirc, m_Sheet.AccSep, lCurRow - 1 ), m_HeaderRow.ButDlg );

	SelectMutipleRows( &arlSelRows );

	// Draw a horizontal black line below all.
	_DrawBlackLine();
}

void CSSheetTAHub::CellClicked( long lColumn, long lRow, CSSheetTAHub *pclOtherTAHub, bool *pfMustResize )
{
	// By default, no resize has to be done
	if( NULL != pfMustResize )
		*pfMustResize = false;

	// When application combo is not already chosen ('Select One' is set and not 'Heating' or 'Cooling'), user
	// can anyway expand/collapse the sheets.
	if( lRow == m_HeaderRow.ButDlg )
	{
		BeginWaitCursor();

		// If user has clicked on the expand/collapse ALL button...
		if( true == m_fHubMode && lColumn == m_Sheet.Pos )
		{
			// Expand all in this sheet.
			bool bHasExpanded = _ExpandCollapseAll();
			// Expand all in the other sheet (here it's 'm_pSheetTAStation').
			if( pclOtherTAHub )
			{
				if( true == bHasExpanded )
					pclOtherTAHub->ExpandAll();
				else
					pclOtherTAHub->CollapseAll();
			}
			if( NULL != pfMustResize )
				*pfMustResize = true;
		}
		else if( true == ChangeExpandCollapseColumnButtonState( lColumn, lRow, m_HeaderRow.FirstCirc, GetMaxRows(), ExpandCollapseColumnAction::ECCA_Toggle ) )
		{
			CArray <long> *parlHub;
			if( true == m_fHubMode )
				parlHub = &m_arlHub;
			else
				parlHub = &m_arlHubStation;

			parlHub->RemoveAll();
			for( long lLoopColumn = m_Sheet.Sub; lLoopColumn < m_Sheet.Pointer; lLoopColumn++ )
			{
				int iReturn = GetExpandCollapseColumnState( lLoopColumn, m_HeaderRow.ButDlg );
				if( CSSheet::ExpandCollapseColumnState::ECCS_Invalid == iReturn )
					continue;
				if( CSSheet::ExpandCollapseColumnState::ECCS_Expanded == iReturn)
					parlHub->Add( lLoopColumn );
			}
			
			RefreshCollapseExpandButton( pclOtherTAHub );

			if( NULL != pfMustResize )
				*pfMustResize = true;
		}

		EndWaitCursor();
	}

	// For the rest, user has to make a choice between 'Heating' or 'Cooling'.
	if( NULL == pRViewSSelHub || true == pRViewSSelHub->IsLocked() )
	{
		return;
	}
	
	m_lCol = lColumn;
	m_lRow = lRow;
	
	m_CellLParam = GetCellParam( lColumn, lRow );
	m_strCellText = GetCellText( lColumn, lRow );
	
	// Allow/Disable drag & drop operations
	if( false == m_fHubMode )
	{
		CDS_HmHubStation *pHubS = (CDS_HmHubStation *)GetpHM( lRow );
		if( NULL != pHubS && true == pHubS->IsPlugged() )
			SetBool( SSB_ALLOWDRAGDROP, FALSE );
		else
		{
			if( true == IsDDAvailable( lColumn, lRow ) )
				SetBool( SSB_ALLOWDRAGDROP, TRUE );
			else 
				SetBool( SSB_ALLOWDRAGDROP, FALSE );
		}
		SelectRow( lRow );
	}
	else 
		SetBool( SSB_ALLOWDRAGDROP, FALSE );
}

void CSSheetTAHub::CellDblClick( long lColumn,long lRow )
{
	CDS_HydroMod *pHM = GetpHM( lRow );
	
	// Combo Edit mode
	if( false == m_fComboEditMode && NULL != pHM && true == pHM->IsCvExist() && false == pHM->GetpCV()->IsTaCV() && lColumn == m_Sheet.CVKvs )
	{
		m_fComboEditMode = true;
		SaveTextPattern( lColumn, lRow );
		FormatEditDouble( m_Sheet.CVKvs, lRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
		RestoreTextPattern( lColumn, lRow );
		SetEditMode( true );
	}
}

void CSSheetTAHub::ButtonClickedFpspreadhubStation( long lColumn, long lRow, short nButtonDown )
{
	// Check / unCheck Plug
	CDS_HmHubStation *pHubS = dynamic_cast<CDS_HmHubStation *>( GetpHM( lRow ) );
	if( NULL != pHubS )
	{
		if( lColumn == m_Sheet.CheckBox )
		{
			TRACE( _T("\nButtonClick %d, %d, But%d"), lColumn, lRow, nButtonDown );
			bool fPlug = nButtonDown ? false : true;
			if( pHubS->IsPlugged() != fPlug )
			{
				BeginWaitCursor();			
				TASApp.OverridedSetRedraw( this, FALSE );
				pHubS->SetPlugged( fPlug );
				pHubS->ComputeAll();

				if( NULL != pRViewSSelHub )
				{
					pRViewSSelHub->RefreshSheets();
				}

				TASApp.OverridedSetRedraw( this, TRUE );
				Invalidate();
				EndWaitCursor();
			}
		}
	}
}

bool CSSheetTAHub::SaveAndCompute(long lColumn, long lRow, bool fComputeAll )
{
	BeginWaitCursor();

	double dSI = 0.0;
	CCbiRestrEdit CbiRestrEdit;
	
	CDS_HydroMod *pHM = GetpHM( lRow );
	if( NULL == pHM )
		return false;
	
	// Disable chain computing if needed
	if( CDS_HmHub::SelMode::Direct == m_pHub->GetSelMode() )
		fComputeAll = false;
	
	CTermUnit *pTermUnit = pHM->GetpTermUnit();			ASSERT( pTermUnit );
	if( NULL == pTermUnit )
		return false;
	
	bool fRefreshSheetNeeded = false;
	if( NULL != m_pHub && true == m_bCellModified )
	{
		fRefreshSheetNeeded = true;
		m_bCellModified = false;
		if( true == m_fHubMode )
		{	
			// Hub 
			switch( lColumn )
			{
				case m_Sheet.LabelID:
					{
						if( false == CbiRestrEdit.CheckCBICharSet( &m_strCellText ) )
						{
							TASApp.AfxLocalizeMessageBox( AFXMSG_CBICHARRESTRICTION );
						}
						else
						{
							if( _T('*') == *m_strCellText )
							{
								m_pHub->RenameMe();
							}
							else 
							{
								CTable *pTab = m_pTADS->GetpHUBSelectionTable();
								
								// Test selected objects	
								if( NULL != m_pTADS->FindHydroMod( m_strCellText, pTab, m_pHub ) )
								{
									m_pTADS->DifferentiateHMName( pTab, m_strCellText );
								}
								
								// Test temporary table
								pTab = (CTable *)( m_pTADS->Get( _T("TMPHUB_TAB") ).MP );

								if( NULL != m_pTADS->FindHydroMod( m_strCellText, pTab, m_pHub ) )
								{
									m_pTADS->DifferentiateHMName( pTab, m_strCellText );
								}

								m_pHub->SetHMName( m_strCellText );
							}
							m_pHub->RenameStations();
						}
					}
					break;

				case m_Sheet.LoopDesc:
					m_pHub->SetDescription( m_strCellText );
					pTermUnit->SetDescription( m_strCellText );
					fRefreshSheetNeeded = false;
					break;
				
				case m_Sheet.DpCName:
					break;
				
				case m_Sheet.BvName:
					break;
			}
		}
		else
		{
			CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
			CDS_HmHubStation *pHMHS = (CDS_HmHubStation *)pHM; 
			double dSI = 0.0;
			
			switch( lColumn )
			{
				case m_Sheet.LabelID:

					if( false == CbiRestrEdit.CheckCBICharSet( &m_strCellText ) )
					{
						TASApp.AfxLocalizeMessageBox( AFXMSG_CBICHARRESTRICTION );
					}
					else
					{
						// Test on already select objects.
						CTable *pTab = m_pTADS->GetpHUBSelectionTable();

						if( NULL != m_pTADS->FindHydroMod( m_strCellText, pTab, pHMHS ) )
						{
							m_pTADS->DifferentiateHMName( pTab, m_strCellText );
						}
							
						// Test temporary table.
						pTab = (CTable *)( m_pTADS->Get( _T("TMPHUB_TAB") ).MP );
						
						if( NULL != m_pTADS->FindHydroMod( m_strCellText, pTab, pHMHS ) )
						{
							m_pTADS->DifferentiateHMName( pTab, m_strCellText );
						}

						pHMHS->SetHMName( m_strCellText );
					}

					break;

				case m_Sheet.LoopDesc:
					pHMHS->SetDescription( m_strCellText );
					fRefreshSheetNeeded = false;
					break;

				case m_Sheet.LoopQ:
					dSI = CDimValue::CUtoSI( _U_FLOW, m_dCellDouble );
					if( dSI != pHMHS->GetpTermUnit()->GetQ() )
						pHMHS->GetpTermUnit()->SetQ( dSI );
					else
						fRefreshSheetNeeded = false;
					break;
				
				case m_Sheet.LoopP:
					{
						dSI = CDimValue::CUtoSI( _U_TH_POWER, m_dCellDouble );

						if( dSI != pHMHS->GetpTermUnit()->GetP() )
						{
							pHMHS->GetpTermUnit()->SetPdT( dSI, DBL_MAX );
						}
						else
						{
							fRefreshSheetNeeded = false;
						}
					}
					break;

				case m_Sheet.LoopDT:
					{
						dSI = CDimValue::CUtoSI( _U_DIFFTEMP, m_dCellDouble );

						if( dSI != pHMHS->GetpTermUnit()->GetDT() )
						{
							pHMHS->GetpTermUnit()->SetPdT( DBL_MAX, dSI );
						}
						else
						{
							fRefreshSheetNeeded = false;
						}
					}
					break;
				
				case m_Sheet.LoopDp:
				case m_Sheet.UnitDp:
					dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
					if( dSI != pHMHS->GetpTermUnit()->GetDp() )
						pHMHS->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::Dp, dSI, DBL_MAX );
					else
						fRefreshSheetNeeded = false;
					break;

				case m_Sheet.UnitQref:
					{
						double dQref, dDpref;
						dSI = CDimValue::CUtoSI( _U_FLOW, m_dCellDouble );
						pHMHS->GetpTermUnit()->GetDp( NULL, &dDpref, &dQref );
						if( dSI != dQref )
							pHMHS->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, DBL_MAX, dSI );
						else
							fRefreshSheetNeeded = false;
					}
					break;

				case m_Sheet.UnitDpref:
					{
						double dQref, dDpref;
						dSI = CDimValue::CUtoSI( _U_DIFFPRESS, m_dCellDouble );
						pHMHS->GetpTermUnit()->GetDp( NULL, &dDpref, &dQref );
						if( dSI != dDpref )
							pHMHS->GetpTermUnit()->SetDp( CDS_HydroMod::eDpType::QDpref, dSI, DBL_MAX );
						else
							fRefreshSheetNeeded = false;
					}
					break;

				case m_Sheet.CVDesc:
					pHMHS->GetpCV()->SetDescription( m_strCellText );
					fRefreshSheetNeeded = false;
					break;
				
				case m_Sheet.PipeL:
					dSI = CDimValue::CUtoSI( _U_LENGTH, m_dCellDouble );
					if( dSI != pHMHS->GetpCircuitPrimaryPipe()->GetLength() )
						pHMHS->GetpCircuitPrimaryPipe()->SetLength( dSI );
					else
						fRefreshSheetNeeded = false;
					break;
				
				case m_Sheet.BvType:
					// Needed for DD operations, 
					if( m_CellLParam && pHMHS->GetBalTypeIDPtr().MP != (CData *)m_CellLParam )
						if( true == ( (CData *)m_CellLParam )->IsClass( CLASS( CDB_StringID ) ) )
						{
							pHMHS->Init( ( (CDB_StringID *)m_CellLParam )->GetIDPtr().ID, (CTable *)pHMHS->GetParent(), pHMHS->GetCvTaKv() );
							pHMHS->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
						}
					break;
				
				case m_Sheet.BvCtrlType:
					// Needed for DD operations, 
					if( m_CellLParam && pHMHS->GetCvCtrlType() != (m_CellLParam - 1 ) )
					{
						if( NULL != pHMHS->GetpCV() )
							pHMHS->GetpCV()->SetCtrlType( (CDB_ControlProperties::CvCtrlType)( m_CellLParam - 1 ) );
						pHMHS->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
					}	
					break;
				
				case m_Sheet.BvName:
					// Needed for DD operations, 
					if( m_CellLParam && pHMHS->GetReturnValveIDPtr().MP != (CData *)m_CellLParam )
					{
						if( true == ( (CData *)m_CellLParam )->IsClass( CLASS( CDB_HubStaValv ) ) )
						{
							// Valves type matching?
							if( ( (CDB_TAProduct *)( (CDB_HubStaValv *)m_CellLParam )->GetValveIDPtr().MP )->GetTypeIDPtr().MP == pHMHS->GetBalTypeIDPtr().MP )
							{
								pHMHS->SetReturnValveID( ( (CDB_HubStaValv *)m_CellLParam )->GetIDPtr().ID );
								pHMHS->SetLock( CDS_HydroMod::eHMObj::eBVprim, true, false );
							}
						}
					}
					break;

				case m_Sheet.CVType:
					// Needed for DD operations, 
					if( m_CellLParam )
					{
						CDB_StringID *pConnect,*pVersion, *pPN;
						CDS_HmHubStation *pHmStation = (CDS_HmHubStation *)m_CellLParam;
						pConnect = pVersion = pPN = NULL;
						if( CDS_HmHubStation::eCVTAKV::eTAValve == pHmStation->GetCvTaKv() )
						{
							pConnect = pHmStation->GetpCV()->GetpSelCVConn();
							pVersion = pHmStation->GetpCV()->GetpSelCVVers();
							pPN = pHmStation->GetpCV()->GetpSelCVPN();
						}
						pHMHS->SetControlValve( pHmStation->GetCvTaKv(), pHmStation->GetCvCtrlType(), pConnect, pVersion, pPN );
					}
					break;
				
				case m_Sheet.CVKvs:
					{
						CDS_HydroMod::CCv *pCV = pHM->GetpCV();		
						if( NULL == pCV )
							break;
						
						// Needed for DD operations,
						if( m_CellLParam )
						{
							if( false == pHMHS->GetpCV()->IsOn_Off() )
							{
								CDS_HydroMod::CCv *pSrcCv = (CDS_HydroMod::CCv *)m_CellLParam;
								ASSERT( pSrcCv );
								if( NULL == pSrcCv )
									break;
								if( true == pHMHS->GetpCV()->IsPresettable() )
									break;
								pHMHS->SetLock( CDS_HydroMod::eHMObj::eCV, !pHMHS->GetpCV()->IsBestCV( pSrcCv->GetKvs() ) );
								pHMHS->GetpCV()->SetKvs( pSrcCv->GetKvs() );
							}
						}
						else if( m_dCellDouble )
						{
							// Drag&Drop source cell is in edition mode
							if( true == pCV->IsPresettable() )
								break;
							pHM->SetLock( CDS_HydroMod::eHMObj::eCV, !pCV->IsBestCV( m_dCellDouble ) );
							pCV->SetKvs( m_dCellDouble );
						}
					}
					break;

				case m_Sheet.PipeSerie:
					// Needed for DD operations, 
					if( m_CellLParam )
					{
						CTable *pTabPipeSerie = ( (CPipes *)m_CellLParam )->GetPipeSeries();
						if( true == pTabPipeSerie->IsClass( CLASS( CTableHM ) ) )
							GetpHM( lRow )->GetpCircuitPrimaryPipe()->SetPipeSeries( pTabPipeSerie );
					}
					break;

				case m_Sheet.PipeSize:
					// Needed for DD operations, 
					if( m_CellLParam )
					{
						CPipes* pSrcPipe = (CPipes *)m_CellLParam;  
						ASSERT( pSrcPipe );
						if( NULL == pSrcPipe )
							break;
						
						CPipes* pPipe = NULL;
						pPipe = pHM->GetpCircuitPrimaryPipe();
						ASSERT( pPipe );
						if( NULL == pPipe )
							break;
						
						pPipe->SetLock( !pPipe->IsBestPipe( pSrcPipe->GetIDPtr().MP ) );
						pPipe->SetIDPtr( pSrcPipe->GetIDPtr() );
					}
					break;

				case m_Sheet.ActuatorType:
					// Needed for DD operations, 
					if( m_CellLParam && ( pHMHS->GetBalTypeID() == _T("RVTYPE_BVC") ) )
					{
						CDB_Actuator *pActr = dynamic_cast<CDB_Actuator *>( (CData*)m_CellLParam );
						if( NULL != pActr )	
						{
							if( NULL == pHMHS->GetpCV() )
								break;
							CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pHMHS->GetpCV()->GetCvIDPtr().MP );
							if( NULL != pCV && true == pCV->IsActuatorFit( pActr ) )
								pHMHS->SetReturnActuatorID( pActr->GetIDPtr().ID );
						}
					}
					break;
			}
		}		
	}
	
	// Resize locally
	if( true == fRefreshSheetNeeded )
	{
		pHM->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
		pHM->ComputeAll();
	}
	
	EndWaitCursor();

	if( NULL != pRViewSSelHub )
	{
		pRViewSSelHub->CheckSelectButton( m_pHub );
	}

	return fRefreshSheetNeeded;
}

bool CSSheetTAHub::IsDDAvailable(long lColumn, long lRow)
{
	// Drag and Drop  : Only for station
	if( true == m_fHubMode)
		return false;
	
	CDS_HydroMod *pHM = GetpHM( lRow );
	if( NULL == pHM )
		return false;
	
	switch( lColumn )
	{
		case m_Sheet.LoopDesc:
		case m_Sheet.LoopQ:
		case m_Sheet.LoopP:
		case m_Sheet.LoopDT:
		case m_Sheet.UnitDp:
		case m_Sheet.UnitQref:
		case m_Sheet.UnitDpref:

		case m_Sheet.CVType:
		case m_Sheet.CVName:
		case m_Sheet.CVDesc:
		case m_Sheet.CVKvs:

		case m_Sheet.PipeSerie:
		case m_Sheet.PipeSize:
		case m_Sheet.PipeL:

		case m_Sheet.AccDesc:

		case m_Sheet.BvType:
		case m_Sheet.BvCtrlType:
		case m_Sheet.BvName:

		case m_Sheet.ActuatorType:
			return true;

		case m_Sheet.LoopDp:
			if( CDS_HmHub::SelMode::QDpLoop == m_pHub->GetSelMode() ) 
				return true;
			else
				return false;
		
		default:
			return false;
	}
	return false;
}

void CSSheetTAHub::DragDropBlock( long lOldColumnFrom, long lOldRowFrom, long lOldColumnTo, long lOldRowTo, long lNewColFrom, long lNewRowFrom, long lNewColumnTo, long lNewRowTo, BOOL &fCancel )
{
	// Only Vertical drag and drop
	if( lOldColumnFrom != lOldColumnTo || lOldColumnFrom != lNewColumnTo)
		return;
	
	// One cell at a time
	if( lOldRowFrom != lOldRowTo )
		return;
	
	if( false == IsDDAvailable( lOldColumnFrom, lOldRowFrom ) )
		return;
	
	CDS_HydroMod *pHM = GetpHM( lOldRowFrom );
	if( NULL == pHM )
		return;
	
	BeginWaitCursor();
	
	// Copy Accessories
	if( lOldColumnFrom == m_Sheet.AccDesc )
	{
		CPipes *pPipe = NULL;
		pPipe = pHM->GetpCircuitPrimaryPipe();

		if( NULL != pPipe )
		{
			for( long lLoopRow = min( lOldRowFrom, lNewRowFrom ); lLoopRow <= max( lOldRowFrom, lNewRowFrom ); lLoopRow++ )
			{
				if( lLoopRow == lOldRowFrom )
					continue;

				CDS_HydroMod *pTrgHM = GetpHM( lLoopRow );
				if( NULL != pTrgHM )
				{
					// Drop on the main module is not allowed (there is no pipe)
					if( 0 == pTrgHM->GetLevel() )
						continue;

					CPipes *pTrgPipe = NULL;
					if( CDS_HydroMod::eHMObj::eDistributionSupplyPipe == pPipe->GetLocate() )
						pTrgPipe = pTrgHM->GetpDistrSupplyPipe();
					else if( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe == pPipe->GetLocate() ) 
					{
						// Drop on a module is not allowed for circuit pipe (there is no pipe)
						if( true == pTrgHM->IsaModule() )
							continue;
						pTrgPipe = pTrgHM->GetpCircuitPrimaryPipe();
					}
					
					if( NULL != pTrgPipe )
						pPipe->CopyAllSingularities( pTrgPipe );
				}
			}
		}
	}

	EditChange( lOldColumnFrom, lOldRowFrom );
	if( true == m_bCellModified )
	{
		for( long lLoopRow = min( lOldRowFrom, lNewRowFrom ); lLoopRow <= max( lOldRowFrom, lNewRowFrom ); lLoopRow++ )
		{
			if( lLoopRow == lOldRowFrom )
				continue;
			
			m_lRow = lLoopRow;
			
			// SaveAndCompute reset flag m_CellModified, so keep it in state
			m_bCellModified = true;
			SaveAndCompute( lOldColumnFrom, lLoopRow, false );
		}
	}

	if( m_pHub->GetSelMode() != CDS_HmHub::SelMode::Direct )
	{
		pHM->ComputeAll();
	}

	if( NULL != pRViewSSelHub )
	{
		pRViewSSelHub->RefreshSheets();
	}

	EndWaitCursor();
	fCancel = true;
}

void CSSheetTAHub::TextTipFetch( long lColumn, long lRow, WORD* pwMultiLine, SHORT* pnTipWidth, TCHAR* pstrTipText, BOOL* pfShowTip )
{
	CString str;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;
	*pfShowTip = false;
	if( NULL == m_pHub )
		return;
	
	if( lRow >= m_HeaderRow.FirstCirc )
	{
		if( true == m_fHubMode && lColumn == m_Sheet.LoopQ )
		{
			str = m_pHub->CheckMaxTotalFlow();
		}
		else
			str = GetCellText( lColumn, lRow );
		
		if( true == str.IsEmpty() )
			return;

		CDC *pDC = GetDC();
		CRect rect( 0, 0, 0, 0 );
		pDC->DrawText( str, &rect, DT_CENTER | DT_CALCRECT | DT_NOPREFIX );
		ReleaseDC( pDC );
		*pnTipWidth = (SHORT)rect.right;
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pfShowTip = true;
	}
}

void CSSheetTAHub::LeaveCell( long lOldColumn, long lOldRow, long lNewColumn, long lNewRow, BOOL* pfCancel )
{
	if( NULL != pfCancel )
	{
		*pfCancel = true;
	}
	
	if( NULL == pRViewSSelHub || true == pRViewSSelHub->IsLocked() )
	{
		return;
	}
	
	if( true == m_fComboEditMode && false == m_bCellModified )
	{
		m_fComboEditMode = false;

		// Nothing Change restore combo
		CDS_HydroMod *pHM = GetpHM( lOldRow );

		if( NULL != pHM && true == pHM->IsCvExist() && false == pHM->GetpCV()->IsTaCV() && lOldColumn == m_Sheet.CVKvs )
		{	
			if( GetCellTypeW( lOldColumn, lOldRow ) != SS_TYPE_COMBOBOX )
			{
				SaveTextPattern( lOldColumn, lOldRow );
				FormatEditCombo( lOldColumn, lOldRow, _C_KVCVCOEFF, pHM->GetpCV()->GetKvs(), SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER, (LPARAM)pHM->GetpCV() );
				RestoreTextPattern( lOldColumn, lOldRow );
			}
		}
	}
	
	// Leave a modified cell compute depending parameters
	if( true == m_bCellModified )
	{
		if( true == m_fComboEditMode )
			m_fComboEditMode = false;
		
		bool fRefreshSheetNeeded = SaveAndCompute( lOldColumn, lOldRow );
		m_bCellModified = false;

		// No need to call again 'ComputeAll' because it's already done in 'SaveAndCompute'.
// 		if( m_pHub->GetSelMode() != CDS_HmHub::eSelMode::Direct )
// 			m_pHub->ComputeAll();		

		if( true == fRefreshSheetNeeded && NULL != pRViewSSelHub )
		{
			pRViewSSelHub->RefreshSheets();
		}

		SelectRow( lOldRow );
		return;
	}

	if( -1 == lNewColumn || -1 == lNewRow )
		return;
	
	if( lNewRow < m_HeaderRow.FirstCirc )
		return;

	if( NULL == GetpHM( lNewRow ) )
		return;
	
	if( lNewRow != lOldRow )
	{
		if( GetCellTypeW( lNewColumn, lNewRow ) == SS_TYPE_COMBOBOX )
			SelectRow( lNewRow );
	}
	SelectRow( lNewRow );
	if( NULL != pfCancel )
		*pfCancel = false;
}

void CSSheetTAHub::EditChange( long lColumn, long lRow )
{
	m_lRow = lRow;
	m_lCol = lColumn;

	CString str = GetCellText( lColumn, lRow );

	m_dCellDouble = 0.0;
	m_strCellText = _T("");
	m_CellLParam = NULL;
	m_bCellModified = false;

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
			str = _T("0.0");
		
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
							break;
					}

					i = str.FindOneOf( _T("-+") );
					if( i != -1 )
					{
						str.Delete( i );
						if( RD_OK == ReadDouble( str, &dValue ) )
							break;
					}
					SetCellText( lColumn, lRow, m_strCellText );
				}
				break;

			case RD_EMPTY: 
				break;

			case RD_OK:
				// Check validity field
				if( CheckValidity( dValue ) )
				{
					m_dCellDouble = dValue;
					m_strCellText = str;
					m_bCellModified = true;
				}
				break;
		};
	}
	else if( GetCellTypeW( lColumn, lRow ) == SS_TYPE_COMBOBOX )
	{
		m_CellLParam = GetCellParam( m_lCol, m_lRow );
		if( m_CellLParam )
			m_bCellModified = true;
	}
}

bool CSSheetTAHub::ValidateCell()
{
	long lRow = GetActiveRow();
	if( NULL == GetpHM( lRow ) )
		return false;
	long lColumn = GetActiveCol();
	EditChange( lColumn, lRow );
	LeaveCell( lColumn, lRow, -1, -1, NULL );

	return true;
}

void CSSheetTAHub::KeyPress( int *piKeyAscii )
{
	if( '\r' == *piKeyAscii )
		ValidateCell();
}

void CSSheetTAHub::KeyDown( WORD wKeyCode, WORD wShiftStatus )
{
	// ??????????????????
// 	if (*pnKeyCode == eKey::ArrowDown ||
// 		*pnKeyCode == eKey::ArrowUp ||
// 		*pnKeyCode == eKey::ArrowLeft ||
// 		*pnKeyCode == eKey::ArrowRight)
// 	{
// 	}
}

void CSSheetTAHub::ExpandAll()
{
	CArray <long> *parlHub;
	if( true == m_fHubMode )
		parlHub = &m_arlHub;
	else
		parlHub = &m_arlHubStation;
	
	parlHub->RemoveAll();
	for( long lLoopColumn = m_Sheet.Pos; lLoopColumn < m_Sheet.Pointer; lLoopColumn++ )
	{
		if( CSSheet::ExpandCollapseColumnState::ECCS_Collapsed == GetExpandCollapseColumnState( lLoopColumn, m_HeaderRow.ButDlg ) )
		{
			ChangeExpandCollapseColumnButtonState( lLoopColumn, m_HeaderRow.ButDlg, m_HeaderRow.FirstCirc, GetMaxRows(), CSSheet::ExpandCollapseColumnAction::ECCA_ToExpand );
			parlHub->Add( lLoopColumn );
		}
	}
}

void CSSheetTAHub::CollapseAll()
{
	CArray <long> *parlHub;
	if( true == m_fHubMode )
		parlHub = &m_arlHub;
	else
		parlHub = &m_arlHubStation;

	parlHub->RemoveAll();
	for( long lLoopColumn = m_Sheet.Pos; lLoopColumn < m_Sheet.Pointer; lLoopColumn++ )
	{
		if( CSSheet::ExpandCollapseColumnState::ECCS_Expanded == GetExpandCollapseColumnState( lLoopColumn, m_HeaderRow.ButDlg ) )
			ChangeExpandCollapseColumnButtonState( lLoopColumn, m_HeaderRow.ButDlg, -1, -1, CSSheet::ExpandCollapseColumnAction::ECCA_ToCollapse );
	}
}

bool CSSheetTAHub::SelectRow(long lRow)
{
	TASApp.OverridedSetRedraw( this, FALSE );
	UnSelectMultipleRows();
	
	bool fSelect = false;
	
	if( true == m_fHubMode )
		return false;
	
	if( lRow < m_HeaderRow.FirstCirc )
		return false;
	
	if( lRow > GetMaxRows() )
		return false;
	
	if( NULL == GetpHM( lRow ) )
		return false;

	SelectOneRow( lRow );
	TASApp.OverridedSetRedraw( this, TRUE );
	Invalidate();
	
	return fSelect;
}

CDS_HydroMod *CSSheetTAHub::GetpHM( long lRow )
{
	return ((CDS_HydroMod *)GetCellParam( m_Sheet.Pointer, lRow ) );
}

bool CSSheetTAHub::CheckValidity( double dVal )
{
	switch( m_lCol )
	{
		case m_Sheet.LoopQ:
		case m_Sheet.LoopP:
		case m_Sheet.LoopDT:
		case m_Sheet.UnitDp:
		case m_Sheet.UnitQref:
		case m_Sheet.UnitDpref:
			if( dVal > 0 )
				return true;		
		
		case m_Sheet.PipeL:
			if( dVal >= 0 )
				return true;		
			break;
	}
	return true;
}

void CSSheetTAHub::FillDpControllerColumns( long lRow )
{
	if( GetCellParam( m_Sheet.DpCName, lRow ) )
	{
		LPARAM tmpParam = GetCellParam( m_Sheet.DpCName, lRow );
		CDB_HubValv *tmpHV = (CDB_HubValv *)tmpParam;
		IDPTR tmpIDPtr = tmpHV->GetValveIDPtr();
		CDB_DpController *tmpDPC = (CDB_DpController*)tmpIDPtr.ID;

		CTable *pTab = 	(CTable *)m_pTADB->Get( _T("DPCONTR_TAB") ).MP;	ASSERT(pTab);	
		IDPTR IDPtr = _NULL_IDPTR;

		for( IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext() )
		{
			tmpDPC = (CDB_DpController*)IDPtr.MP;
			if( 0 == _tcscmp( tmpDPC->GetIDPtr().ID, tmpIDPtr.ID ) )
				break;
		}

		CString str = _T("");
		if( NULL != *IDPtr.ID )
			str = tmpDPC->GetFormatedDplRange().c_str();

		SetStaticText( m_Sheet.DpCDpLr, lRow, str );
	}
}

void CSSheetTAHub::ComboDropDown(long lColumn, long lRow )
{
	m_lCol = lColumn;
	m_lRow = lRow;
	SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_WHITE );

	CDS_HydroMod *pHM = GetpHM( lRow );
	ASSERT( m_pSCB );
	if( NULL == pHM || NULL == m_pSCB || NULL == m_pSCvCB )
		return;
	
	BeginWaitCursor();			// If ComboBox is already open Cursor will be relaxed in CloseDialogSCB
	
	if( true == m_pSCvCB->IsOpened() )
		return;
	
	if( true == m_pSCB->IsOpened() )
		return;
	
	m_bCellModified = true;
	switch( lColumn )
	{
		case m_Sheet.CVType:
			m_pSCvCB->OpenDialogSCB( pHM, lColumn, lRow );
			break;

		case m_Sheet.CVName:
			m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_Cv, lColumn, lRow );
			break;

		case m_Sheet.CVKvs:
			m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CvKvs, lColumn, lRow );
			break;

		case m_Sheet.PipeSerie:
			m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSerie, lColumn, lRow );
			break;

		case m_Sheet.PipeSize:
			m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_CircuitPrimaryPipeSize, lColumn, lRow );
			break;

		case m_Sheet.BvType:
			m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DS_StationValveType, lColumn, lRow );
			break;
	
		case m_Sheet.BvName:
			if( true == m_fHubMode )
			{
				// Shut-Off Only
				if( 0 == m_pHub->GetBalTypeID().CompareNoCase( _T("SHUTTYPE_BALL") ) )
				{
					if( CDS_HmHub::SelMode::Direct == m_pHub->GetSelMode() )
						m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DS_ShutOffValve, lColumn, lRow );
					else
						m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_ShutOffValve, lColumn, lRow );
				}
				else
				{
					if( CDS_HmHub::SelMode::Direct == m_pHub->GetSelMode() )
						m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DS_HubValveName, lColumn, lRow );
					else
						m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_BvPName, lColumn, lRow );
				}
			}
			else
			{
				if( CDS_HmHub::SelMode::Direct == m_pHub->GetSelMode() )
					m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DS_StationValveName, lColumn, lRow );
				else 
				{
					if( ( (CDS_HmHubStation*)pHM )->GetBalTypeID() != _T("RVTYPE_BVC") )
						m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_BvPName, lColumn, lRow );
					else
						m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_Cv, lColumn, lRow );
				}
			}
			break;

		case m_Sheet.BvCtrlType:
			m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DS_StationValveCtrlType, lColumn, lRow );
			break;

		case m_Sheet.DpCName:
			if( CDS_HmHub::SelMode::Direct == m_pHub->GetSelMode() )
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DS_HubDpCName, lColumn, lRow );
			else
				m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DpC, lColumn, lRow );
			break;	
	
		case m_Sheet.ActuatorType:
			m_pSCB->OpenDialogSCB( pHM, CDlgComboBoxHM::eSCB::SCB_DS_StationActuatorType, lColumn, lRow );
			break;

	}
	EndWaitCursor();
}

void CSSheetTAHub::CloseDialogSCB( CDialogExt *pDlgSCB, bool fModified )
{
	BeginWaitCursor();

	m_dCellDouble = 0;
	m_CellLParam = 0;
	LeaveCell( m_lCol, m_lRow, -1, -1, NULL );

	if( NULL != pRViewSSelHub )
	{
		pRViewSSelHub->SendMessage( WM_SIZE, SIZE_RESTORED, 0 );
	}

	TASApp.PumpMessage();
	
	if( NULL != pRViewSSelHub )
	{
		pRViewSSelHub->Invalidate();
	}
	
	EndWaitCursor();
}

void CSSheetTAHub::SheetChanged(short nOldSheet, short nNewSheet )
{

}

CDS_HydroMod::eHMObj CSSheetTAHub::GetHMObj( long lColumn, long lRow )
{
	CDS_HydroMod *pHM = GetpHM( lRow );
	if( NULL == pHM )
		return CDS_HydroMod::eHMObj::eNone;
	
	CDS_HydroMod::eHMObj HMObj = CDS_HydroMod::eHMObj::eNone;
	switch( lColumn )
	{
		// CV
		case m_Sheet.CVName:
		case m_Sheet.CVKvs:
			if( true == pHM->IsCvExist() )
				HMObj = CDS_HydroMod::eHMObj::eCV;
			break;

		// BV
		case m_Sheet.BvName:
			if( true == pHM->IsBvExist() )
				HMObj = CDS_HydroMod::eHMObj::eBVprim;
			break;

		// DpC
		case m_Sheet.DpCName:
			if( true == pHM->IsDpCExist() )
				HMObj = CDS_HydroMod::eHMObj::eDpC;
			break;

		// Circuit Pipe
		case m_Sheet.PipeSize:
			HMObj = CDS_HydroMod::eHMObj::eCircuitPrimaryPipe;
			break;
	}
	return HMObj;
}

bool CSSheetTAHub::IsEditAccAvailable( long lColumn, long lRow )
{
	CDS_HydroMod *pHM = GetpHM( lRow );
	if( NULL == pHM )
		return false;
	
	if( false == pHM->IsaModule() )	// No circuit pipe accessories for a module
		return true;

	return false;
}

void CSSheetTAHub::HideEmptyColumns()
{
	long lColStart = m_Sheet.LabelID;
	long lColEnd = m_Sheet.Pointer;
	long lRowStart = m_HeaderRow.FirstCirc;
	long lButDlg = m_HeaderRow.ButDlg;
	
	// Run all columns
	for( long lLoopCol = lColStart; lLoopCol <= lColEnd; lLoopCol++ )
	{
		// If column can be hidden...
		if( true == IsFlagCanBeCollapsed( lLoopCol ) )
		{
			// Run all rows
			bool fIsColEmpty = true;
			for( long lLoopRow = lRowStart; lLoopRow <= GetMaxRows() && fIsColEmpty; lLoopRow++ )
			{
				// If current column is already hidden...
				if( TRUE == IsColHidden( lLoopCol ) )
				{
					// No need to run all rows
					fIsColEmpty = true;
					break;
				}
				
				fIsColEmpty = GetCellText( lLoopCol, lLoopRow ).IsEmpty();
				if( false == fIsColEmpty )
				{
					// A element exist, no need to run all rows
					break;
				}
			}

			// Hide or not the current column
			ShowCol( lLoopCol, ( true == fIsColEmpty ) ? FALSE : TRUE );
		}
	}
}

// PRIVATE MEMBERS

bool CSSheetTAHub::_ExpandCollapseAll()
{
	// Important remark: this function is called ONLY when user clicks on the collapse/expand all button in the top sheet of the two ones !!
	bool fHasExpanded = true;
	
	// Change state of all child Expand/Collapse column buttons.
	int iState = GetMainExpandCollapseColumnState( m_Sheet.Pos, m_HeaderRow.ButDlg );
	switch( iState )
	{
		case CSSheet::ExpandCollapseColumnState::ECCS_Collapsed:
			ExpandAll();
			break;
		case CSSheet::ExpandCollapseColumnState::ECCS_Expanded:
			CollapseAll();
			fHasExpanded = false;
			break;
		case CSSheet::ExpandCollapseColumnState::ECCS_Invalid:
			break;
	}

	// Change state of main Expand/Collapse column button.
	ChangeMainExpandCollapseButtonState( m_Sheet.Pos, m_HeaderRow.ButDlg, CSSheet::ExpandCollapseColumnAction::ECCA_Toggle );

	Invalidate();
	return fHasExpanded;
}

long CSSheetTAHub::_GetLastVisibleColumn( void )
{
	long lLastVisibleColumn = -1;
	for( long lLoopColumn = m_Sheet.Pointer; lLoopColumn >= m_Sheet.LabelID; lLoopColumn-- )
	{
		if( false == IsFlagAlwaysHidden( lLoopColumn ) )
		{
			lLastVisibleColumn = lLoopColumn;
			break;
		}
	}
	return lLastVisibleColumn;
}
	
// Allow to correctly draw last black horizontal line below all
void CSSheetTAHub::_DrawBlackLine( void )
{
	long lLastVisibleColumn = _GetLastVisibleColumn();
	if( lLastVisibleColumn != - 1 )
		SetCellBorder( m_Sheet.LabelID, GetMaxRows(), lLastVisibleColumn - 1, GetMaxRows(), true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, SPREAD_COLOR_NONE );
}
