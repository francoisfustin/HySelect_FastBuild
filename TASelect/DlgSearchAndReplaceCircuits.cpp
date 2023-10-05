#include "stdafx.h"
#include "afxdialogex.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "EnBitmapPatchWork.h"
#include "DlgSearchAndReplaceCircuits.h"
#include "DlgSearchAndReplaceCircuitsResults.h"

IMPLEMENT_DYNAMIC( CDlgSearchAndReplaceCircuits, CDialogExt )

CDlgSearchAndReplaceCircuits::CDlgSearchAndReplaceCircuits( CWnd *pParent )
	: CDialogExt( CDlgSearchAndReplaceCircuits::IDD, pParent )
{
	m_pParent = pParent;
	m_pHMSrc = NULL;
	m_vecSolutionsKept.clear();
	m_vecAvailSolutions.clear();
	m_vecChangesList.clear();
	m_pclSSheetSrc = NULL;
	m_pclSSheetTrg = NULL;
	m_uiIndexOfSelectedScheme = 0;
}

CDlgSearchAndReplaceCircuits::~CDlgSearchAndReplaceCircuits()
{
	if( NULL != m_pclSSheetSrc )
	{
		delete m_pclSSheetSrc;
	}

	if( NULL != m_pclSSheetTrg )
	{
		delete m_pclSSheetTrg;
	}
}

void CDlgSearchAndReplaceCircuits::SearchAndReplaceFinished( std::map<WPARAM, CDS_HydroMod *> *pMap )
{
	m_ButApply.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
	m_ButNext.EnableWindow( FALSE );

	CDlgSearchAndReplaceCircuitsResults dlg( this );
	dlg.SetInputParameters( m_Target.m_eBalType, ( ewcvTA == m_Target.m_eValveType ) ? true : false, m_Target.m_eControlType, m_Target.m_eControlValveType );
	dlg.SetChangesList( &m_vecChangesList );
	dlg.DoModal();

	// HYS-1766: ComputeAll when close dialog SearchAndReplaceResult.
	BeginWaitCursor();
	m_pHMSrc->ComputeAll();
	EndWaitCursor();

	// We warn the parent dialog to allow it to reset the interface in regards to the new hydraulic network.
	m_pParent->PostMessage( WM_USER_CLOSEPOPUPWND );

	CDialogExt::EndDialog( 1 );
}

void CDlgSearchAndReplaceCircuits::SSheetOnBeforePaint()
{
	CDC *pDC = GetDC();
	
	CRect rect;
	m_StaticRightRect.GetWindowRect( &rect );
	ScreenToClient( &rect );
	rect.InflateRect( -1, -1 );

	CBrush *pclBrush = CBrush::FromHandle( (HBRUSH)::GetStockObject( NULL_BRUSH ) );
	CPen clPen( BS_SOLID, 1, _WHITE );
	CBrush *pOldBrush = pDC->SelectObject( pclBrush );
	CPen *pOldPen = pDC->SelectObject( &clPen );
	pDC->Rectangle( &rect );
	pDC->SelectObject( pOldPen );
	pDC->SelectObject( pOldBrush );

	ReleaseDC( pDC );
}

void CDlgSearchAndReplaceCircuits::SSheetOnAfterPaint()
{
	_SetFocusOnTrgHMSchema();
}

BEGIN_MESSAGE_MAP( CDlgSearchAndReplaceCircuits, CDialogExt )
	ON_BN_CLICKED( IDC_BUTAPPLY, OnBnClickedApply )
	ON_BN_CLICKED( IDC_BUTAPPLYALL, OnBnClickedApplyAll )
	ON_BN_CLICKED( IDC_BUTNEXT, OnBnClickedNext )
	ON_BN_CLICKED( IDC_BUTCLOSE, OnBnClickedClose )
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CDlgSearchAndReplaceCircuits::DoDataExchange( CDataExchange* pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICLEFTRECT, m_StaticLeftRect );
	DDX_Control( pDX, IDC_STATICRIGHTRECT, m_StaticRightRect );
	DDX_Control( pDX, IDC_STATIC_SOURCE, m_StaticFindWhat );
	DDX_Control( pDX, IDC_STATIC_TARGET, m_StaticReplaceWith );
	DDX_Control( pDX, IDC_EDITHMNAME, m_EditHMName );
	DDX_Control( pDX, IDC_RESULTLIST, m_ResultList );
	DDX_Control( pDX, IDC_BUTAPPLY, m_ButApply );
	DDX_Control( pDX, IDC_BUTAPPLYALL, m_ButApplyAll );
	DDX_Control( pDX, IDC_BUTNEXT, m_ButNext );
	DDX_Control( pDX, IDC_BUTCLOSE, m_ButClose );
}

BOOL CDlgSearchAndReplaceCircuits::PreTranslateMessage( MSG *pMsg )
{
	if( WM_LBUTTONDOWN == pMsg->message && m_pclSSheetTrg->GetSafeHwnd() == pMsg->hwnd )
	{
		CPoint point( GET_X_LPARAM( pMsg->lParam ), GET_Y_LPARAM( pMsg->lParam ) );
		m_pclSSheetTrg->ClientToScreen( &point );
		ScreenToClient( &point );
		_ManageLButtonDown( point );
	}

	return __super::PreTranslateMessage( pMsg );
}

BOOL CDlgSearchAndReplaceCircuits::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CRect rectParent;
	::GetWindowRect( m_pParent->GetParent()->GetSafeHwnd(), &rectParent );
	::SetWindowPos( GetSafeHwnd(), HWND_TOP, rectParent.left + m_rectHMLeftTree.Width(), rectParent.top, -1, -1, SWP_NOSIZE );

	CDialogEx::SetBackgroundColor( GetSysColor( COLOR_3DFACE ) );

	SetWindowText( TASApp.LoadLocalizedString( IDS_TABDLGSRCIRCUIT_HEADERTITLE ) );
	m_uiIndexOfSelectedScheme = 0;

	m_EditHMName.SetBlockSelection( true );
	m_EditHMName.SetBlockCursorChange( true );

	// Create an instance of CSSheet.
	BOOL bReturn = FALSE;
	m_pclSSheetSrc = new CSSheet( );

	if( NULL == m_pclSSheetSrc )
	{
		return FALSE;
	}

	// Create CSSheet.
	bReturn = m_pclSSheetSrc->Create( ( GetDlgItem(IDC_STATICLEFTRECT)->GetStyle() | WS_CHILD | WS_VISIBLE & ~WS_BORDER ), CRect( 0, 0, 0, 0 ), GetDlgItem(IDC_STATICLEFTRECT), IDC_FPSPREAD );

	if( FALSE == bReturn )
	{
		return FALSE;
	}
		
	CRect rectClient;
	GetDlgItem( IDC_STATICLEFTRECT )->GetClientRect( &rectClient );
	m_pclSSheetSrc->SetWindowPos( NULL, 0, 0, rectClient.right, rectClient.bottom, SWP_NOACTIVATE | SWP_NOZORDER );
	_InitializeSSheetSrc( m_pclSSheetSrc );
			
	m_pclSSheetSrc->SetColWidthInPixels( 1, rectClient.Width() );
	m_pclSSheetSrc->SetRowHeightInPixels( 1, rectClient.Height() );
			
	_DrawSrcHMSchema();

	m_pclSSheetTrg = new CSSheet();

	if( NULL == m_pclSSheetTrg )
	{
		return FALSE;
	}

	// Allow to receive notification to paint on the sheet.
	m_pclSSheetTrg->RegisterNotificationHandler( this );

	// Create CSSheet.
	bReturn = m_pclSSheetTrg->Create( ( GetDlgItem( IDC_STATICRIGHTRECT )->GetStyle() | WS_CHILD | WS_VISIBLE & ~WS_BORDER ), CRect( 0, 0, 0, 0 ), GetDlgItem( IDC_STATICRIGHTRECT ), IDC_FPSPREAD );
	
	if( FALSE == bReturn )
	{
		return FALSE;
	}

	GetDlgItem( IDC_STATICRIGHTRECT )->GetClientRect( &rectClient );
	m_pclSSheetTrg->SetWindowPos( NULL, 0, 0, rectClient.right, rectClient.bottom, SWP_NOACTIVATE | SWP_NOZORDER );

	_InitializeSSheetTrg( m_pclSSheetTrg );

	m_StaticFindWhat.SetFontBold( true );
	m_StaticFindWhat.SetFontSize( 10 );
	m_StaticFindWhat.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUITS_FINDWHAT ) );

	m_StaticReplaceWith.SetFontBold( true );
	m_StaticReplaceWith.SetFontSize( 10 );
	m_StaticReplaceWith.SetWindowText( TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACECIRCUITS_REPLACEWITH ) );

	m_ButApply.SetWindowText( TASApp.LoadLocalizedString( IDS_BUTTON_APPLY ) );
	m_ButApplyAll.SetWindowText( TASApp.LoadLocalizedString( IDS_BUTTON_APPLYALL ) );
	m_ButNext.SetWindowText( TASApp.LoadLocalizedString( IDS_BUTTON_NEXT ) );
	m_ButClose.SetWindowText( TASApp.LoadLocalizedString( IDS_CLOSE ) );

	OnBnClickedNext();
	
	return TRUE;
}

void CDlgSearchAndReplaceCircuits::OnBnClickedClose()
{
	// Called when we intentionally click on the 'Close' button.
	BeginWaitCursor();
	m_pHMSrc->ComputeAll();
	EndWaitCursor();
	
	// We warn the parent dialog to allow it to reset the interface in regards to the new hydraulic network.
	m_pParent->PostMessage( WM_USER_CLOSEPOPUPWND );

	CDialogExt::EndDialog( 1 );
}

void CDlgSearchAndReplaceCircuits::OnBnClickedNext()
{
	m_pParent->SendMessage( WM_USER_REPLACEPOPUPFINDNEXT, (WPARAM)m_pHMSrc, NULL );
}

void CDlgSearchAndReplaceCircuits::OnBnClickedApply()
{
	CDS_HydroMod *pHMcopy = m_pHMSrc;
	CCircuit clCircuiSrc( pHMcopy );
	clCircuiSrc.m_bFoundSolution = true;

	_ReplaceCircuit();

	CCircuit clCircuitTrg( m_pHMSrc );
	clCircuitTrg.m_bFoundSolution = true;
	m_vecChangesList.push_back( std::pair<CCircuit, CCircuit>( clCircuiSrc, clCircuitTrg ) );
	
	// Post message to notify replacement.
	// Remark: Even if the contains pointed by 'pHMCopy' is no more valid, we need this pointer to modify the 
	// corresonding one in the left tree to replace by the new pointer 'm_pHMSrc'.
	m_pParent->PostMessage( WM_USER_REPLACEPOPUPREPLACE, (WPARAM)pHMcopy, (LPARAM)m_pHMSrc );
}

void CDlgSearchAndReplaceCircuits::OnBnClickedApplyAll()
{
	// User press 'Apply all' button, process to the replacement before launching the loop.
	// Loop is managed by the parent window CDlgSRPageCircuit that call ApplyAll function.
	// Loop can be interrupted if a similar circuit doesn't exist yet in the list of proceeded circuit.
	CDS_HydroMod *pHMcopy = m_pHMSrc;
	CCircuit clCircuiSrc( pHMcopy );
	clCircuiSrc.m_bFoundSolution = true;

	_ReplaceCircuit();

	CCircuit clCircuitTrg( m_pHMSrc );
	clCircuitTrg.m_bFoundSolution = true;
	m_vecChangesList.push_back( std::pair<CCircuit, CCircuit>( clCircuiSrc, clCircuitTrg ) );
	
	// Post message to notify replacement.
	m_pParent->PostMessage( WM_USER_REPLACEPOPUPREPLACEALL, (WPARAM)pHMcopy, (LPARAM)m_pHMSrc );
}

void CDlgSearchAndReplaceCircuits::OnClose()
{
	// Called when closing dialog with the upper right cross button.
	BeginWaitCursor();
	m_pHMSrc->ComputeAll();
	EndWaitCursor();
	
	// We warn the parent dialog to allow it to reset the interface in regards to the new hydraulic network.
	m_pParent->PostMessage( WM_USER_CLOSEPOPUPWND );

	// Automatically close this dialog.
	CDialogExt::EndDialog( 1 );
}

void CDlgSearchAndReplaceCircuits::ApplyAll()
{
	// Apply all only if a solution for this schema already exist.
	if( _FindPreviousUserChoice() >= 0 )
	{
		CDS_HydroMod *pHMcopy = m_pHMSrc;
		CCircuit clCircuiSrc( pHMcopy );
		clCircuiSrc.m_bFoundSolution = true;

		_ReplaceCircuit();

		CCircuit clCircuitTrg( m_pHMSrc );
		clCircuitTrg.m_bFoundSolution = true;
		m_vecChangesList.push_back( std::pair<CCircuit, CCircuit>( clCircuiSrc, clCircuitTrg ) );
		
		// Post message to notify replacement. (Changed from PostMessage due to a stack overflow)
		m_pParent->PostMessage( WM_USER_REPLACEPOPUPREPLACEALL, (WPARAM)pHMcopy, (LPARAM)m_pHMSrc );
	}
}

void CDlgSearchAndReplaceCircuits::UpdateData( CDS_HydroMod *pHMSource )
{
	m_pHMSrc = pHMSource;

	if( NULL != this->GetSafeHwnd() )
	{
		_DrawSrcHMSchema();
		_UpdateHMTrg();
	}
}

void CDlgSearchAndReplaceCircuits::DisableFindNextBtn()
{
	m_ButNext.EnableWindow( FALSE );
	m_ButApplyAll.EnableWindow( FALSE );
}

void CDlgSearchAndReplaceCircuits::SetFlagTarget( bool bBalType, bool bWorkForCtrlValve, bool bCtrlType, bool bCtrlValveType )
{
	m_Target.m_bIsBalTypeMustBeChecked = bBalType;
	m_Target.m_bIsCtrlTypeMustBeChecked = bCtrlType;
	m_Target.m_bIsCtrlValveTypeMustBeChecked = bCtrlValveType;
	m_Target.m_bIsValveTypeMustBeChecked = bWorkForCtrlValve;

}

void CDlgSearchAndReplaceCircuits::SetTarget( CDB_CircuitScheme::eBALTYPE BalType, CDB_CircSchemeCateg *pclCircSchemeCategory, _eWorkForCV WorkForCtrlValv, 
		CDB_ControlProperties::CvCtrlType CtrlType, CDB_ControlProperties::eCVFUNC CtrlValvType )
{
	// No (Straight pipe), manual balancing or Dp controller.
	m_Target.m_eBalType = BalType;

	// List of all circuit schematic categories selected by the user.
	m_Target.m_pclCircSchemeCategory = pclCircSchemeCategory;
	
	// IMI control valve or Kvs.
	m_Target.m_eValveType = WorkForCtrlValv;
	
	// On/Off, 3 points or proportional.
	m_Target.m_eControlType = CtrlType;
	
	// Control only, presettable, presettable and point tests.
	m_Target.m_eControlValveType = CtrlValvType;
}

void CDlgSearchAndReplaceCircuits::SetHMLeftTreeWidth( CRect &rect )
{
	m_rectHMLeftTree = rect;
}

int CDlgSearchAndReplaceCircuits::_FindPreviousUserChoice()
{
	// Are there several possibilities ?
	if( m_vecAvailSolutions.size() >= 1 )
	{
		// Try to find previous selected solution.
		std::vector<std::pair<CCircuit, CCircuit> >::iterator Itsaved = m_vecSolutionsKept.begin();

		for( ; Itsaved != m_vecSolutionsKept.end(); Itsaved ++ )
		{
			if( true == Itsaved->first.HMmatching( m_pHMSrc ) )
			{
				break;
			}
		}

		// If a saved solution exist for this type of schema try to find solution applied into the map available solution.
		if( Itsaved != m_vecSolutionsKept.end() )
		{
			// Feach available solution.
			int iLoop = 0;

			for( std::vector<CCircuit>::iterator it = m_vecAvailSolutions.begin(); it != m_vecAvailSolutions.end(); it++, iLoop++ )
			{
				// Test only user requested parameters.
				if( *it == Itsaved->second )
				{
					// Found! return the index into the available map of this circuit.
					return iLoop;
				}
			}
		}
	}
	
	return -1;
}

void CDlgSearchAndReplaceCircuits::_ReplaceCircuit()
{
	if( NULL == m_pHMSrc )
	{
		ASSERT_RETURN;
	}

	bool bTU = ( false == m_pHMSrc->IsaModule() );

	CDS_HydroMod *pHMcopy = m_pHMSrc;
	CDB_CircuitScheme *pSch = m_vecAvailSolutions.at( m_uiIndexOfSelectedScheme ).m_pclCircuitScheme;

	// Save the selected solution; if exist keep the latest user choice.
	// Try to find previous selected solution.
	std::vector<std::pair<CCircuit, CCircuit > >::iterator Itsaved = m_vecSolutionsKept.begin();

	for( ; Itsaved != m_vecSolutionsKept.end(); Itsaved++ )
	{
		if( true == Itsaved->first.HMmatching( m_pHMSrc ) )
		{
			break;
		}
	}
	
	if( Itsaved != m_vecSolutionsKept.end() )
	{
		Itsaved->second = m_vecAvailSolutions.at( m_uiIndexOfSelectedScheme );
	}
	else
	{
		CCircuit HMCircuit;
		HMCircuit.Init( m_pHMSrc );
		m_vecSolutionsKept.push_back( std::pair<CCircuit, CCircuit>( HMCircuit, m_vecAvailSolutions.at( m_uiIndexOfSelectedScheme ) ) );
		m_vecSolutionsKept.back().first.m_bFoundSolution = true;
	}

	CTable *pTabInsertIn = (CTable *)( m_pHMSrc->GetIDPtr().PP );

	if( NULL == pSch || NULL == pTabInsertIn )
	{
		ASSERT_RETURN;
	}

	// Create a new circuit with new user parameters, m_pHMSrc is updated with the new circuit pointer.
	CDB_ControlProperties::CvCtrlType CtrlType = m_vecAvailSolutions.at( m_uiIndexOfSelectedScheme ).m_eControlType;

	bool bTaCV = ( _eWorkForCV::ewcvTA == m_vecAvailSolutions.at( m_uiIndexOfSelectedScheme ).m_eValveType );

	m_pHMSrc = TASApp.GetpTADS()->CreateNewHM( pTabInsertIn, pSch, CtrlType, bTaCV, bTU, pHMcopy->GetReturnType() );

	// Remark: Once it has copied all the needed stuffs in 'm_pHM' this mehod will properly delete 'pHMCopy' !!
	// The copy goes from 'pHMcopy' to 'm_pHMSrc'.
	TASApp.GetpTADS()->CopyCommonHMData( pHMcopy, m_pHMSrc );

	if( NULL != m_pHMSrc->GetpDpC() )
	{
		if( eMvLoc::MvLocSecondary == pSch->GetMvLoc() )
		{
			m_pHMSrc->GetpDpC()->SetMvLoc( pSch->GetMvLoc() );
			m_pHMSrc->GetpDpC()->SetMvLocLocked( true );
		}
	}
	
	CString str;
	FormatString( str, IDS_DLGSEARCHREPLACECIRCUIT_CIRCUITPROCEED, m_pHMSrc->GetHMName() );
	INSERT_STRING_IN_LIST_AND_SCROLL( m_ResultList, str );

	m_pHMSrc->ComputeHM( CDS_HydroMod::eComputeHMEvent::eceResize );
}

void CDlgSearchAndReplaceCircuits::_InitializeSSheetSrc( CSSheet *pSheet )
{
	pSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );

	pSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

#ifdef DEBUG
	pSheet->SetBool( SSB_GRIDLINES, TRUE );
#endif

	// Set max rows.
	pSheet->SetMaxRows( 1 );
	pSheet->SetMaxCols( 3 );

	// All cells are static by default and filled with _T("").
	pSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pSheet->FormatStaticText( -1, -1, -1, -1, _T("") );
}

void CDlgSearchAndReplaceCircuits::_InitializeSSheetTrg( CSSheet *pSheet )
{
	pSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	
	pSheet->SetBool( SSB_VERTSCROLLBAR, TRUE );
	pSheet->SetBool( SSB_SCROLLBAREXTMODE, TRUE );

	pSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

#ifdef DEBUG
	pSheet->SetBool( SSB_GRIDLINES, TRUE );
#endif

	// Set max rows.
	pSheet->SetMaxRows( 4 );
	pSheet->SetMaxCols( 3 );

	// All cells are static by default and filled with _T("").
	pSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// The ratio are taken from the dialog unit.

	// Title row.
	// int iRowHeight = int( 14.0 / 346.0 * dHeight );
	pSheet->SetRowHeightInPixels( 1, 21 );
	pSheet->SetRowHeightInPixels( 3, 21 );

	// Graph row.
	// iRowHeight = int( 160.0 / 346.0 * dHeight );
	pSheet->SetRowHeightInPixels( 2, 254 );
	pSheet->SetRowHeightInPixels( 4, 254 );

	// Columns.
	// int iColWidth = int( 138.0 / 412.0 * dWidth );
	pSheet->SetColWidthInPixels( 1, 200 );
	pSheet->SetColWidthInPixels( 2, 200 );
	pSheet->SetColWidthInPixels( 3, 200 );
	
	// Bottom border for 1st title.
	pSheet->SetCellBorder( 1, 1, 3, 1, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Bottom border for 1st graph.
	pSheet->SetCellBorder( 1, 2, 3, 2, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Bottom border for 2nd title.
	pSheet->SetCellBorder( 1, 3, 3, 3, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	
	// Right border for first column.
	pSheet->SetCellBorder( 1, 1, 1, 4, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );

	// Right border for second column.
	pSheet->SetCellBorder( 2, 1, 2, 4, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );

	// Save area for each cells to ease code for left button down.
	CRect rectSheetPos;
	pSheet->GetClientRect( &rectSheetPos );
	pSheet->ClientToScreen( &rectSheetPos );
	ScreenToClient( &rectSheetPos );

	CRect rectSelectionSize = pSheet->GetSelectionInPixels( 1, 1, 1, 2 );
	
	// Cell on 1st row and 1st column.
	CRect rectCell = pSheet->GetCellCoordInPixel( 1, 1 );
	CRect rectPos = CRect( rectSheetPos.left + rectCell.left, rectSheetPos.top + rectCell.top, rectSheetPos.left + rectSelectionSize.Width(), rectSheetPos.top + rectSelectionSize.Height() );
	m_mapCellCoords.insert( std::pair<int, CRect>( 0, rectPos ) );

	// Cell on 1st row and 2nd column.
	CRect rectPos2 = rectPos;
	rectPos2.OffsetRect( rectSelectionSize.Width(), 0 );
	m_mapCellCoords.insert( std::pair<int, CRect>( 1, rectPos2 ) );

	// Cell on 1st row and 3rd column.
	rectPos2.OffsetRect( rectSelectionSize.Width(), 0 );
	m_mapCellCoords.insert( std::pair<int, CRect>( 2, rectPos2 ) );

	// Cell on 2nd row and 1st column.
	rectPos2 = rectPos;
	rectPos2.OffsetRect( 0, rectSelectionSize.Height() );
	m_mapCellCoords.insert( std::pair<int, CRect>( 3, rectPos2 ) );

	// Cell on 2nd row and 2nd column.
	rectPos2.OffsetRect( rectSelectionSize.Width(), 0 );
	m_mapCellCoords.insert( std::pair<int, CRect>( 4, rectPos2 ) );

	// Cell on 2nd row and 3rd column.
	rectPos2.OffsetRect( rectSelectionSize.Width(), 0 );
	m_mapCellCoords.insert( std::pair<int, CRect>( 5, rectPos2 ) );
}

void CDlgSearchAndReplaceCircuits::_DrawSrcHMSchema()
{
	CRect rectClient;
	GetDlgItem( IDC_STATICLEFTRECT )->GetClientRect( &rectClient );
	
	// Create the CEnBitmap.
	CEnBitmapPatchWork EnBmp;
	EnBmp.GetHydronicScheme( m_pHMSrc );
	EnBmp.ResizeImage( CSize( rectClient.Width(), rectClient.Height() - 4 ) );

	// Put the picture in the Spread cell.
	m_pclSSheetSrc->SetPictureCellWithHandle( EnBmp, 1, 1, true, VPS_BMP | VPS_CENTER );
	EnBmp.DeleteObject();
	CString str = m_pHMSrc->GetHMName();

	if( false == m_pHMSrc->GetDescription().IsEmpty() )
	{
		str += CString( _T(" - ") ) + m_pHMSrc->GetDescription(); 
	}

	m_EditHMName.SetWindowText( str );
}

void CDlgSearchAndReplaceCircuits::_UpdateHMTrg()
{
	m_vecAvailSolutions.clear();
	int iCount = 0;
	CDB_CircuitScheme *pSelSch = NULL;
	CDB_CircuitScheme *pclCircuitSchemeSource = m_pHMSrc->GetpSch();

	if( NULL == pclCircuitSchemeSource ) 
	{
		ASSERT_RETURN;
	}

	CDB_CircSchemeCateg *pclCircuitSchemeCategorySource = dynamic_cast<CDB_CircSchemeCateg *>( pclCircuitSchemeSource->GetSchemeCategIDPtr().MP );

	if( NULL == pclCircuitSchemeCategorySource )
	{
		ASSERT_RETURN;
	}

	bool bTU = !m_pHMSrc->IsaModule();

	// Table definition used to increase TADB research speed.
	CTable *pTabSch = (CTable *)( TASApp.GetpTADB()->Get( L"CIRCSCHEME_TAB" ).MP );

	if( NULL == pTabSch )
	{
		ASSERT_RETURN;
	}

	std::vector<CCircuit> vecCircuit;

	// Run all existing circuit schemes exisiting in the database.
	for( IDPTR IDPtrSch = pTabSch->GetFirst(); _T('\0') != *IDPtrSch.ID; IDPtrSch = pTabSch->GetNext() )
	{
		vecCircuit.clear();
		vecCircuit.push_back( m_Target );

		CDB_CircuitScheme *pSch = dynamic_cast<CDB_CircuitScheme *>( IDPtrSch.MP );

		if( NULL == pSch )
		{
			ASSERT_CONTINUE;
		}

		CDB_CircSchemeCateg *pSchCateg = dynamic_cast<CDB_CircSchemeCateg *>( pSch->GetSchemeCategIDPtr().MP );

		if( NULL == pSchCateg )
		{
			ASSERT_CONTINUE;
		}

		// HYS-1686: We don't include the same scheme than the source.
		if( 0 == IDcmp( pSch->GetIDPtr().ID, pclCircuitSchemeSource->GetIDPtr().ID ) )
		{
			continue;
		}
		
		// We can do a first test to check if the current circuit scheme belongs to the circuit scheme catagories selected by the user.
		bool bCircSchemeCategoryMatch = ( 0 == IDcmp( m_Target.m_pclCircSchemeCategory->GetIDPtr().ID, pSchCateg->GetIDPtr().ID ) ) ? true : false;

		if( false == bCircSchemeCategoryMatch )
		{
			continue;
		}

		////////////////////////////////////////////////////////////////////////
		// Terminal unit.
		// Keep only only schema that are matching terminal unit of current selected HM.
		if( false == bTU && CDB_CircuitScheme::eTERMUNIT::Compulsory == pSch->GetTermUnit() )
		{
			continue;
		}

		if( true == bTU && CDB_CircuitScheme::eTERMUNIT::NotAllowed == pSch->GetTermUnit() )
		{
			continue;
		}

		////////////////////////////////////////////////////////////////////////
		// Manual or dynamic balancing.
		if( false == m_Target.m_bIsBalTypeMustBeChecked )
		{
			// Balancing type is not an user search criteria, test if the current schema match the current hydromod schema .
			if( pSch->GetBalType() != pclCircuitSchemeSource->GetBalType() )
			{
				continue;
			}

			vecCircuit.at( 0 ).m_eBalType = pclCircuitSchemeSource->GetBalType();
		}
		else
		{
			// Balancing type is an user search criteria, test if the current schema match the current user selection.
			if( pSch->GetBalType() != m_Target.m_eBalType )
			{
				continue;
			}
		}

		// HYS-1481 : When the target circuit is distribution circuit Get2W3W = CVUnKnown. This case doesn't accept control !
		// Test control properties only if the target circuit accept control!
		if( CDB_ControlProperties::CV2W3W::LastCV2W3W != pSchCateg->Get2W3W() 
				&& CDB_ControlProperties::CV2W3W::CVUnknown != pSchCateg->Get2W3W() )	// Control valve exist
		{
			////////////////////////////////////////////////////////////////////////
			// Work for control valve TA or Kvs.
			if( false == m_Target.m_bIsValveTypeMustBeChecked )
			{
				// If user has not checked the valve type, thus no radio available at right, and the current circuit
				// type has control valve, we choose by default the IMI Hydronic Engineering valve type.
				vecCircuit.at( 0 ).m_eValveType = ewcvTA;
			}
			else
			{
				if( _eWorkForCV::ewcvKvs == m_Target.m_eValveType && CDB_ControlProperties::eCVFUNC::ControlOnly != pSch->GetCvFunc() )
				{
					continue;
				}
			}

			////////////////////////////////////////////////////////////////////////
			// Control type.
			// Remark: 'Proportional', '3-points' and 'On/Off' control type are not defined in the 'CDB_CircuitScheme'.
			if( false == m_Target.m_bIsCtrlTypeMustBeChecked )
			{
				// If user has not checked the control type, thus no radio available at right, and if the current circuit
				// type has a control valve, we accept by default at least 'Proportional' and 'On/Off' control types.
				vecCircuit.at( 0 ).m_eControlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
				
				// HYS-1679: Electronic balancing work with proportional control.
				if( CDB_CircuitScheme::eBALTYPE::ELECTRONIC != m_Target.m_eBalType )
				{
					vecCircuit.push_back( vecCircuit.at( 0 ) );
					vecCircuit.at( 1 ).m_eControlType = CDB_ControlProperties::CvCtrlType::eCvOnOff;
				}
			}
			else
			{
				// We keep the user choice.
			}

			////////////////////////////////////////////////////////////////////////
			// Control valve type.
			if( false == m_Target.m_bIsCtrlValveTypeMustBeChecked )
			{
				// If user has not checked the control valve type, thus no radio available at right, and if the current circuit
				// type has a control valve, we accept by default this four 'Standard control valve', 'Adjustable', 'Adjustable and measurable', 'Pressure independent' and
				// 'Smart'.
				// HYS-1686: We have to set m_eControlValveType with the same value of the current circuit.
				vecCircuit.at( 0 ).m_eControlValveType = pSch->GetCvFunc();
				if( 1 < vecCircuit.size() )
				{
					vecCircuit.at( 1 ).m_eControlValveType = pSch->GetCvFunc();
				}
			}
			else
			{
				// HYS-1637: Separate pressure independent and other presettable control valve
				if( (m_Target.m_eControlValveType & pSch->GetCvFunc()) != pSch->GetCvFunc() )
				{
					continue;
				}

				CDB_ControlProperties::eCVFUNC ePIBCVFunc = (CDB_ControlProperties::eCVFUNC)( CDB_ControlProperties::eCVFUNC::PresetPT | CDB_ControlProperties::eCVFUNC::Presettable );
				if( ePIBCVFunc == m_Target.m_eControlValveType && pSch->GetDpCType() != CDB_CircuitScheme::eDpCType::eDpCTypePICV )
				{
					continue;
				}
				else if( ePIBCVFunc != m_Target.m_eControlValveType && pSch->GetDpCType() == CDB_CircuitScheme::eDpCType::eDpCTypePICV )
				{
					continue;
				}
			}

			// Now we can check for each 'Circuit' in the 'vecCircuit' if exist solutions.
			CTADatabase *pTADB = TASApp.GetpTADB();
			std::vector<CCircuit>::iterator iter = vecCircuit.begin();

			while( iter != vecCircuit.end() )
			{
				// Try to find one CDB_RegulatingValve with matching ctrl properties.
				std::vector<_string> vecStrTab;

				// Verify if it's a PiCv scheme, allow only PiCv valves.
				if( CDB_CircuitScheme::eDpCTypePICV == pSch->GetDpCType() )
				{
					vecStrTab.push_back( L"PICTRLVALV_TAB" );
				}
				else if( CDB_CircuitScheme::eBALTYPE::ELECTRONIC == pSch->GetBalType() )
				{
					// HYS-1679: Add smart control valve in S&R process.
					vecStrTab.push_back( L"SMARTCONTROLVALVE_TAB" );
				}
				else
				{
					vecStrTab.push_back( L"REGVALV_TAB" );
					vecStrTab.push_back( L"BALCTRLVALV_TAB" );
					vecStrTab.push_back( L"CTRLVALV_TAB" );
					vecStrTab.push_back( L"TRVALV_TAB" );
				}

				bool bFound = false;

				for( UINT j = 0; j < vecStrTab.size() && false == bFound; j++ )
				{
					CTable *pTab = (CTable *)( TASApp.GetpTADB()->Get( vecStrTab[j].c_str() ).MP );
						
					for( IDPTR idptr = pTab->GetFirst(); _T('\0') != *idptr.ID && false == bFound; idptr = pTab->GetNext() )
					{
						CDB_RegulatingValve *pRv = dynamic_cast<CDB_RegulatingValve*>( idptr.MP );

						if( NULL == pRv )
						{
							continue;
						}

						if( false == pRv->IsSelectable( true ) )
						{
							continue;
						}

						if( true == pTADB->MustExclude( pRv, CTADatabase::FilterSelection::ForHMCalc ) )
						{
							continue;
						}

						// Test number of ways for the control valve and the control type (Control only, presetable, presetPT).
						if( false == pRv->IsValidCV( (*iter).m_pclCircSchemeCategory->Get2W3W(), (*iter).m_eControlValveType ) )
						{
							continue;
						}

						// Test the control type (Proportional, 3-points or On/Off).
						if( false == pRv->GetCtrlProp()->CvCtrlTypeFits( (*iter).m_eControlType ) )
						{
							continue;
						}

						CDB_ControlProperties *pCtrlProp = dynamic_cast<CDB_ControlProperties *>( pRv->GetCtrlProp() );

						if( NULL == pCtrlProp )
						{
							continue;
						}

						if( CDB_ControlProperties::PresetPT == pCtrlProp->GetCvFunc() || CDB_ControlProperties::Presettable == pCtrlProp->GetCvFunc() )
						{
							// To exclude valves without characteristics .... KTM50
							CDB_ValveCharacteristic *pValvChar = pRv->GetValveCharacteristic();

							if( NULL == pValvChar || false == pValvChar->HasKvCurve() )
							{
								continue;
							}
						}

						bFound = true;
					}
				}

				if( false == bFound )
				{
					iter = vecCircuit.erase( iter );
				}
				else
				{
					iter++;
				}
			}
		}
		
		// HYS-1637: Verify vecCircuit size before adding pSch. 
		if( vecCircuit.size() > 0 )
		{
			// HYS-1481 : We have to set the circuit scheme when the target circuit has no control
			int iCircuitPos = 0;

			while( iCircuitPos < (int)vecCircuit.size() )
			{
				vecCircuit.at( iCircuitPos ).m_pclCircuitScheme = pSch;
				iCircuitPos++;
			}
		}

		for( auto &iter : vecCircuit )
		{
			m_vecAvailSolutions.push_back( iter );
		}
	}

	m_uiIndexOfSelectedScheme = (UINT)max( _FindPreviousUserChoice(), 0 );

	// 'RDW_FRAME' to force the non client area to be cleaned.
	GetDlgItem( IDC_STATICRIGHTRECT )->RedrawWindow( NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW );

	m_pclSSheetTrg->SetREDRAW( FALSE );
	_ClearAllTrgHMSchema();

	if( (int)m_vecAvailSolutions.size() > 0 )
	{
		// Draw all schematics.
		for( int i = 0; i < (int)m_vecAvailSolutions.size(); i++ )
		{
			_DrawTrgHMSchema( i );
		}
	}
	else
	{
		// Put in the solution but with a message that we don't find anything for this circuit.
		CCircuit HMCircuitSrc;
		HMCircuitSrc.Init( m_pHMSrc );
		HMCircuitSrc.m_bFoundSolution = false;

		CCircuit HMCircuitTrg;
		HMCircuitTrg.m_bFoundSolution = false;
		m_vecSolutionsKept.push_back( std::pair<CCircuit, CCircuit>( HMCircuitSrc, HMCircuitTrg ) );
		m_vecChangesList.push_back( std::pair<CCircuit, CCircuit>( HMCircuitSrc, HMCircuitTrg ) );

		CString str;
		FormatString( str, IDS_DLGSEARCHREPLACECIRCUIT_CIRCUITNOTFOUND, m_pHMSrc->GetHMName() );
		INSERT_STRING_IN_LIST_AND_SCROLL( m_ResultList, str );

		// Here we sent that the old pointer is the same as the new because there is no change.
		// It allows the 'CDlgSearchReplace::UpdateTreeHMpointers' method to properly work on the left tree.
		m_pParent->PostMessage( WM_USER_REPLACEPOPUPREPLACE, (WPARAM)m_pHMSrc, (LPARAM)m_pHMSrc );
	}

	m_pclSSheetTrg->SetREDRAW( TRUE );
	m_pclSSheetTrg->Invalidate();
	m_pclSSheetTrg->UpdateWindow();
}

void CDlgSearchAndReplaceCircuits::_DrawTrgHMSchema( UINT nPos )
{
	if( nPos > m_vecAvailSolutions.size() )
	{
		return;
	}
	
	CCircuit clTarget = m_vecAvailSolutions.at( nPos );
	CDB_CircuitScheme *pSch = clTarget.m_pclCircuitScheme;

	if( NULL == pSch )
	{
		ASSERT_RETURN;
	}

	int iRow = ( (int)floor( nPos / 3.0 ) * 2 ) + 1;
	int iCol = nPos - ( (int)floor( nPos / 3.0 ) * 3 ) + 1;

	if( iRow > m_pclSSheetTrg->GetMaxRows() )
	{
		_AddOneAdditionalRow();
	}

	// Text.
	m_pclSSheetTrg->SetStaticText( iCol, iRow, pSch->GetSchemeCategName().c_str() );

	// Create the CEnBitmap.
	CEnBitmapPatchWork EnBmp;
	CArray<CAnchorPt::eFunc> aPtFunc;

	for( int j = 0; j < pSch->GetAnchorPtListSize(); j++ )
	{
		aPtFunc.Add( pSch->GetAnchorPtFunc( j ) );
	}

	CRect rectCellSize = m_pclSSheetTrg->GetSelectionInPixels( iCol, iRow + 1, iCol, iRow + 1 );

	EnBmp.Reset();
	// HYS-1637: 3-points control type gives modulating symbol on CV 
	EnBmp.GetHydronicScheme( pSch, ( false == m_pHMSrc->IsaModule() ), ( CDB_ControlProperties::eCvOnOff != clTarget.m_eControlType ), &aPtFunc, m_pHMSrc->GetLevel() > 0, m_pHMSrc->GetReturnType() );
	EnBmp.ResizeImage( CSize( rectCellSize.Width(), rectCellSize.Height() - 4 ) );

	// Put the picture in the spread cell.
	m_pclSSheetTrg->SetPictureCellWithHandle( EnBmp, iCol, iRow + 1, true, VPS_BMP | VPS_CENTER );
	EnBmp.DeleteObject();
}

void CDlgSearchAndReplaceCircuits::_AddOneAdditionalRow()
{
	m_pclSSheetTrg->SetMaxRows( m_pclSSheetTrg->GetMaxRows() + 2 );

	long lRow = m_pclSSheetTrg->GetMaxRows() - 1;

	// Title row.
	// int iRowHeight = int( 14.0 / 346.0 * dHeight );
	m_pclSSheetTrg->SetRowHeightInPixels( lRow, 21 );

	// Graph row.
	// iRowHeight = int( 160.0 / 346.0 * dHeight );
	m_pclSSheetTrg->SetRowHeightInPixels( lRow + 1, 254 );

	// Bottom border for previous graph.
	m_pclSSheetTrg->SetCellBorder( 1, lRow - 1, 3, lRow - 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Bottom border for the title.
	m_pclSSheetTrg->SetCellBorder( 1, lRow, 3, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Right border for first column.
	m_pclSSheetTrg->SetCellBorder( 1, lRow, 1, lRow + 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );

	// Right border for second column.
	m_pclSSheetTrg->SetCellBorder( 2, lRow, 2, lRow + 1, true, SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, _BLACK );

	// Save area for each cells to ease code for left button down.
	CRect rectSheetPos;
	m_pclSSheetTrg->GetClientRect( &rectSheetPos );
	m_pclSSheetTrg->ClientToScreen( &rectSheetPos );
	ScreenToClient( &rectSheetPos );

	CRect rectSelectionSize = m_pclSSheetTrg->GetSelectionInPixels( 1, lRow, 1, lRow + 1 );

	int iInsertPos = (int)( floor( lRow / 2 ) ) * 3;

	// Cell on 1st row and 1st column.
	CRect rectCell = m_pclSSheetTrg->GetCellCoordInPixel( 1, lRow );
	CRect rectPos = CRect( rectSheetPos.left + rectCell.left, rectSheetPos.top + rectCell.top, rectSheetPos.left + rectCell.left + rectSelectionSize.Width(), rectSheetPos.top + rectCell.top + rectSelectionSize.Height() );
	m_mapCellCoords.insert( std::pair<int, CRect>( iInsertPos++, rectPos ) );

	// Cell on 1st row and 2nd column.
	CRect rectPos2 = rectPos;
	rectPos2.OffsetRect( rectSelectionSize.Width(), 0 );
	m_mapCellCoords.insert( std::pair<int, CRect>( iInsertPos++, rectPos2 ) );

	// Cell on 1st row and 3rd column.
	rectPos2.OffsetRect( rectSelectionSize.Width(), 0 );
	m_mapCellCoords.insert( std::pair<int, CRect>( iInsertPos, rectPos2 ) );
}

void CDlgSearchAndReplaceCircuits::_RemoveAdditionalRows()
{
	m_pclSSheetTrg->SetMaxRows( 4 );
	std::map<int, CRect>::iterator first = m_mapCellCoords.find( 6 );

	if( first != m_mapCellCoords.end() )
	{
		m_mapCellCoords.erase( first, m_mapCellCoords.end() );
	}
}

void CDlgSearchAndReplaceCircuits::_ClearAllTrgHMSchema()
{
	for( int iCol = 1; iCol < 4; iCol++ )
	{
		for( int iRow = 2; iRow < 6; iRow += 2 )
		{
			m_pclSSheetTrg->ClearPictureCell( iCol, iRow );
		}
	}

	_RemoveAdditionalRows();
}

void CDlgSearchAndReplaceCircuits::_SetFocusOnTrgHMSchema()
{
	CDC *pDC = GetDC();

	CPen clPen( PS_SOLID, 2, _SELECTEDBLUE );
	CPen *pOldPen = pDC->SelectObject( &clPen );

	int iRow = ( (int)floor( m_uiIndexOfSelectedScheme / 3.0 ) * 2 ) + 1;
	int iCol = m_uiIndexOfSelectedScheme - ( (int)floor( m_uiIndexOfSelectedScheme / 3.0 ) * 3 ) + 1;

	// Check if there is a scroling.
	long lYOffset = 0;
	long lTopRow = m_pclSSheetTrg->GetTopRow();

	if( lTopRow > 1 )
	{
		for( int i = 1; i < lTopRow; i++ )
		{
			CRect rectCellSize = m_pclSSheetTrg->GetSelectionInPixels( 1, i, 1, i );
			lYOffset += rectCellSize.Height();
		}
	}

	if( false == m_pclSSheetTrg->GetVisible( iCol, iRow, SS_VISIBLE_PARTIAL ) 
			&& false == m_pclSSheetTrg->GetVisible( iCol, iRow + 1, SS_VISIBLE_PARTIAL ) )
	{
		return;
	}
	
	CRect rectSheetPos;
	m_pclSSheetTrg->GetClientRect( &rectSheetPos );
	m_pclSSheetTrg->ClientToScreen( &rectSheetPos );
	ScreenToClient( &rectSheetPos );

	CRgn rgn;
	rgn.CreateRectRgn( rectSheetPos.left - 1, rectSheetPos.top - 1, rectSheetPos.right, rectSheetPos.bottom + 1 );
	pDC->SelectClipRgn( &rgn );

	CRect rectSelectionSize = m_pclSSheetTrg->GetSelectionInPixels( iCol, iRow, iCol, iRow + 1 );
	CRect rectCellTextCoord = m_pclSSheetTrg->GetCellCoordInPixel( iCol, iRow );

	// Move to the top left position.
	pDC->MoveTo( rectSheetPos.left + rectCellTextCoord.left - 1, rectSheetPos.top + rectCellTextCoord.top - lYOffset );

	// Go to the top right position.
	pDC->LineTo( rectSheetPos.left + rectCellTextCoord.right - 1, rectSheetPos.top + rectCellTextCoord.top - lYOffset );

	// Go to the bottom right position.
	pDC->LineTo( rectSheetPos.left + rectCellTextCoord.right - 1, rectSheetPos.top + rectCellTextCoord.top + rectSelectionSize.Height() - 1 - lYOffset );

	// Go to the bottom left position.
	pDC->LineTo( rectSheetPos.left + rectCellTextCoord.left, rectSheetPos.top + rectCellTextCoord.top + rectSelectionSize.Height() - 1 - lYOffset );

	// Go to the top left position.
	pDC->LineTo( rectSheetPos.left + rectCellTextCoord.left, rectSheetPos.top + rectCellTextCoord.top - lYOffset );

	pDC->SelectObject( pOldPen );
	ReleaseDC( pDC );
}

void CDlgSearchAndReplaceCircuits::_ManageLButtonDown( CPoint point )
{
	// Check if there is a scrolling.
	long lYOffset = 0;
	long lTopRow = m_pclSSheetTrg->GetTopRow();

	if( lTopRow > 1 )
	{
		for( int i = 1; i < lTopRow; i++ )
		{
			CRect rectCellSize = m_pclSSheetTrg->GetSelectionInPixels( 1, i, 1, i );
			lYOffset += rectCellSize.Height();
		}
	}

	// Check if we click on a cell.
	for( auto &iter : m_mapCellCoords )
	{
		CRect rect = iter.second;
		rect.OffsetRect( 0, -lYOffset );

		if( TRUE == rect.PtInRect( point ) )
		{
			if( iter.first < (int)m_vecAvailSolutions.size() )
			{
				m_uiIndexOfSelectedScheme = iter.first;

				// 'RDW_FRAME' to force the non client area to be cleaned.
				GetDlgItem( IDC_STATICRIGHTRECT )->RedrawWindow( NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW );
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CDlgSearchAndReplaceCircuits::CCircuit class.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDlgSearchAndReplaceCircuits::CCircuit::CCircuit()
{
	m_bIsBalTypeMustBeChecked = false;
	m_bIsValveTypeMustBeChecked = false;
	m_bIsCtrlTypeMustBeChecked = false;
	m_bIsCtrlValveTypeMustBeChecked = false;
	m_eBalType = CDB_CircuitScheme::eBALTYPE::LastBALTYPE;
	m_pclCircSchemeCategory = NULL;
	m_eValveType = _eWorkForCV::ewcvLast;
	m_eControlValveType = CDB_ControlProperties::eCVFUNC::LastCVFUNC;
	m_eControlType = CDB_ControlProperties::CvCtrlType::eLastCvCtrlType;
	m_strHMName = _T("");
	m_pclCircuitScheme = NULL;
	m_bFoundSolution = false;
}

CDlgSearchAndReplaceCircuits::CCircuit::CCircuit( CDS_HydroMod *pHM )
{
	Init( pHM );
}

CDlgSearchAndReplaceCircuits::CCircuit &CDlgSearchAndReplaceCircuits::CCircuit::operator = ( CDlgSearchAndReplaceCircuits::CCircuit &Circ )
{
	m_bIsBalTypeMustBeChecked = Circ.m_bIsBalTypeMustBeChecked;
	m_bIsValveTypeMustBeChecked = Circ.m_bIsValveTypeMustBeChecked;
	m_bIsCtrlTypeMustBeChecked = Circ.m_bIsCtrlTypeMustBeChecked;
	m_bIsCtrlValveTypeMustBeChecked = Circ.m_bIsCtrlValveTypeMustBeChecked;
	
	m_eBalType = Circ.m_eBalType;
	m_pclCircSchemeCategory = Circ.m_pclCircSchemeCategory;
	m_eValveType = Circ.m_eValveType;
	m_eControlValveType = Circ.m_eControlValveType;
	m_eControlType = Circ.m_eControlType;

	m_strHMName = Circ.m_strHMName;
	m_pclCircuitScheme = Circ.m_pclCircuitScheme;
	m_bFoundSolution = Circ.m_bFoundSolution;

	return *this;
}

bool CDlgSearchAndReplaceCircuits::CCircuit::operator == ( CDlgSearchAndReplaceCircuits::CCircuit &Circ )
{
	if( m_pclCircuitScheme != Circ.m_pclCircuitScheme )
	{
		return false;
	}

	if( true == m_bIsBalTypeMustBeChecked && m_eBalType != Circ.m_eBalType)
	{
		return false;
	}

	if( m_pclCircSchemeCategory != Circ.m_pclCircSchemeCategory )
	{
		return false;
	}

	if( true == m_bIsValveTypeMustBeChecked && m_eValveType != Circ.m_eValveType)
	{
		return false;
	}

	if( true == m_bIsCtrlTypeMustBeChecked && m_eControlType != Circ.m_eControlType )
	{
		return false;
	}

	if( true == m_bIsCtrlValveTypeMustBeChecked && m_eControlValveType != Circ.m_eControlValveType )
	{
		return false;
	}

	return true;
}

bool CDlgSearchAndReplaceCircuits::CCircuit::Init( CDS_HydroMod *pHM )
{
	if( NULL == pHM || NULL == pHM->GetpSch() )
	{
		ASSERTA_RETURN( false );
	}

	CDB_CircSchemeCateg *pSchCateg = dynamic_cast<CDB_CircSchemeCateg *>( pHM->GetpSch()->GetSchemeCategIDPtr().MP );

	if( NULL == pSchCateg )
	{
		ASSERTA_RETURN( false );
	}

	m_pclCircuitScheme = pHM->GetpSch();
	m_pclCircSchemeCategory = m_pclCircuitScheme->GetpSchCateg();

	m_eBalType = pHM->GetpSch()->GetBalType();
	m_bIsBalTypeMustBeChecked = true;
	
	// HYS-1679: Consider smart control valve
	if( NULL != pHM->GetpCV() || NULL != pHM->GetpSmartControlValve() )
	{
		if( NULL != pHM->GetpSmartControlValve() || true == pHM->GetpCV()->IsTaCV() )
		{
			m_eValveType = ewcvTA;
		}
		else
		{
			m_eValveType = ewcvKvs;
		}

		if( NULL != pHM->GetpSmartControlValve() )
		{
			m_eControlType = CDB_ControlProperties::CvCtrlType::eCvProportional;
		}
		else
		{
			m_eControlType = pHM->GetpCV()->GetCtrlType();
		}
		m_bIsValveTypeMustBeChecked = true;
		m_bIsCtrlTypeMustBeChecked = true;
		m_bIsCtrlValveTypeMustBeChecked = true;
	}
	else
	{
		m_eValveType = ewcvLast;
	}

	m_eControlValveType = m_pclCircuitScheme->GetCvFunc();

	m_strHMName = pHM->GetHMName();

	return true;
}

bool CDlgSearchAndReplaceCircuits::CCircuit::HMmatching( CDS_HydroMod *pHM )
{
	if( NULL == pHM || NULL == pHM->GetpSch() )
	{
		ASSERTA_RETURN( false );
	}
	
	CDB_CircuitScheme *pCircuitScheme = dynamic_cast<CDB_CircuitScheme *>( pHM->GetpSch() );

	if( NULL == pCircuitScheme )
	{
		ASSERTA_RETURN( false );
	}
	
	if( true == m_bIsBalTypeMustBeChecked )
	{
		if( pCircuitScheme->GetBalType() != m_eBalType )
		{
			return false;
		}
	}

	if( 0 != IDcmp( pCircuitScheme->GetIDPtr().ID, m_pclCircuitScheme->GetIDPtr().ID ) )
	{
		return false;
	}

	if( true == m_bIsValveTypeMustBeChecked )
	{
		// HYS-1679: Consider smart control valve
		// No control valve in this circuit.
		if( NULL == pHM->GetpCV() && NULL == pHM->GetpSmartControlValve() )
		{
			return false;
		}
		
		if( NULL != pHM->GetpSmartControlValve() || true == pHM->GetpCV()->IsTaCV() )
		{
			if( ewcvTA != m_eValveType )
			{
				return false;
			}
		}
		else
		{
			if( ewcvKvs != m_eValveType )
			{
				return false;
			}
		}
	}
	
	if( true == m_bIsCtrlTypeMustBeChecked )
	{
		// HYS-1679: Consider smart control valve
		// No control valve in this circuit.
		if( NULL == pHM->GetpCV() && NULL == pHM->GetpSmartControlValve() )
		{
			return false;
		}

		if( NULL == pHM->GetpSmartControlValve() && pHM->GetpCV()->GetCtrlType() != m_eControlType )
		{
			return false;
		}
	}

	if( true == m_bIsCtrlValveTypeMustBeChecked )
	{
		// HYS-1679: Consider smart control valve
		// No control valve in this circuit.
		if( NULL == pHM->GetpCV() && NULL == pHM->GetpSmartControlValve() )
		{
			return false;
		}
		// HYS-1637: for eCVFUNC == Presettable | PresetPT
		if( m_pclCircuitScheme->GetCvFunc() != ( m_eControlValveType & m_pclCircuitScheme->GetCvFunc() ) )
		{
			return false;
		}
	}

	return true;
}
