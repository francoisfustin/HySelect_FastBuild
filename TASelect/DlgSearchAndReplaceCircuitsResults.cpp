#include "stdafx.h"


#include "TASelect.h"
#include "HydroMod.h"
#include "DlgSearchAndReplaceCircuits.h"
#include "DlgSearchAndReplaceCircuitsResults.h"

IMPLEMENT_DYNAMIC( CDlgSearchAndReplaceCircuitsResults, CDialogExt )

CDlgSearchAndReplaceCircuitsResults::CDlgSearchAndReplaceCircuitsResults( CWnd *pParent )
	: CDialogExt( CDlgSearchAndReplaceCircuitsResults::IDD, pParent )
{
	m_eBalType = CDB_CircuitScheme::eBALTYPE::LastBALTYPE;
	m_bWorkWithIMIControlValve = false;
	m_eControlType = CDB_ControlProperties::CvCtrlType::eLastCvCtrlType;
	m_eCtrlValveType = CDB_ControlProperties::eCVFUNC::LastCVFUNC;
	m_pvecChangesList = NULL;
	m_pclSSheetSource = NULL;
	m_pclSSheetTarget = NULL;
	m_pclResizingColumnnInfoSource = NULL;
	m_pclResizingColumnnInfoTarget = NULL;
}

CDlgSearchAndReplaceCircuitsResults::~CDlgSearchAndReplaceCircuitsResults()
{
	if( m_ViewDescription.GetSheetNumber() > 0 )
	{
		CViewDescription::vecSheetDescription vecSheetDescriptionList;
		m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );
	
		for( auto &iter : vecSheetDescriptionList )
		{
			CSSheet *pclSSheet = iter->GetSSheetPointer();

			if( NULL != pclSSheet )
			{
				pclSSheet->MM_UnregisterAllNotificationHandlers( this );
				delete pclSSheet;
				iter->SetSSheetPointer( NULL );
			}
		}

		m_ViewDescription.RemoveAllSheetDescription( true );
	}

	m_pclSSheetSource = NULL;
	m_pclSSheetTarget = NULL;
}

void CDlgSearchAndReplaceCircuitsResults::SetInputParameters( CDB_CircuitScheme::eBALTYPE eBalType, bool bWorkWithIMIControlValve, CDB_ControlProperties::CvCtrlType eCtrlType, 
		CDB_ControlProperties::eCVFUNC eCtrlValveType )
{
	m_eBalType = eBalType;
	m_bWorkWithIMIControlValve = bWorkWithIMIControlValve;
	m_eControlType = eCtrlType;
	m_eCtrlValveType = eCtrlValveType;
}

void CDlgSearchAndReplaceCircuitsResults::SetChangesList( std::vector<std::pair<CDlgSearchAndReplaceCircuits::CCircuit, CDlgSearchAndReplaceCircuits::CCircuit>> *pvecChangesList )
{
	m_pvecChangesList = pvecChangesList;
}

void CDlgSearchAndReplaceCircuitsResults::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST_SOURCES, m_StaticSourcesRect );
	DDX_Control( pDX, IDC_LIST_TARGETS, m_StaticTargetsRect );
}

BOOL CDlgSearchAndReplaceCircuitsResults::OnInitDialog()
{
	CDialogExt::OnInitDialog();
	CDialogEx::SetBackgroundColor( GetSysColor( COLOR_3DFACE ) );

	CString str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_TITLE );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_FOUND );
	GetDlgItem( IDC_STATIC_SOURCE )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_REPLACEDBY );
	GetDlgItem( IDC_STATIC_TARGET )->SetWindowText( str );

	m_StaticSourcesRect.ShowWindow( SW_HIDE );
	m_StaticTargetsRect.ShowWindow( SW_HIDE );

	// Initiliaze source sheet.
	_InitializeSSheet( SheetDescription::SD_Source );
	_FillSheet( SheetDescription::SD_Source );

	_InitializeSSheet( SheetDescription::SD_Target );
	_FillSheet( SheetDescription::SD_Target );

	GetDlgItem( IDC_STATIC_SOURCE )->GetWindowRect( &m_rectSource );
	ScreenToClient( &m_rectSource );

	GetDlgItem( IDC_STATIC_TARGET )->GetWindowRect( &m_rectTarget );
	ScreenToClient( &m_rectTarget );

	m_pclSSheetSource->SetFocus();

	return FALSE;
}

BEGIN_MESSAGE_MAP( CDlgSearchAndReplaceCircuitsResults, CDialogExt )
	ON_MESSAGE( WM_USER_MESSAGEDATA, MM_OnMessageReceive )
END_MESSAGE_MAP()

LRESULT CDlgSearchAndReplaceCircuitsResults::MM_OnMessageReceive( WPARAM wParam, LPARAM lParam )
{
	// Notes to read the message we can use either 'm_pclSSheetSource' or 'm_pclSSheetTarget'.
	CMessageManager::CMessage *pclMessage = m_pclSSheetSource->MM_ReadMessage( wParam );

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

				CMessageManager::SSheetMEParams *prParams = pclSShetMEMsg->GetParams();

				if( NULL == prParams->m_hSheet || ( prParams->m_hSheet != m_pclSSheetSource->GetSafeHwnd() && prParams->m_hSheet != m_pclSSheetTarget->GetSafeHwnd() ) )
				{
					delete pclMessage;
					return 0;
				}

				SheetDescription eSheetDescription = ( prParams->m_hSheet == m_pclSSheetSource->GetSafeHwnd() ) ? SheetDescription::SD_Source : SheetDescription::SD_Target;
				CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( (UINT)eSheetDescription );

				if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetResizingColumnInfo() )
				{
					delete pclMessage;
					return 0;
				}

				switch( prParams->m_eMouseEventType )
				{
					case CMessageManager::SSheetMET::MET_LButtonDown:
						_OnCellClicked( pclSheetDescription, prParams->m_lColumn, prParams->m_lRow );
						break;
				}

				// Send manually to 'CViewDescription' for the class 'CResizingColumnInfo'. Because at now, 'MM_OnMessageReceive' messages are only sent to a 'CWnd'
				// and 'CResizingColumnInfo' is not.
				pclSheetDescription->GetResizingColumnInfo()->OnMouseEvent( prParams->m_eMouseEventType, prParams->m_lColumn, prParams->m_lRow, prParams->m_ptMouse );
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

				CMessageManager::SSheetKEParams *prParams = pclSShetKEMsg->GetParams();

				if( NULL == prParams->m_hSheet || ( prParams->m_hSheet != m_pclSSheetSource->GetSafeHwnd() && prParams->m_hSheet != m_pclSSheetTarget->GetSafeHwnd() ) )
				{
					delete pclMessage;
					return 0;
				}

				SheetDescription eSheetDescription = ( prParams->m_hSheet == m_pclSSheetSource->GetSafeHwnd() ) ? SheetDescription::SD_Source : SheetDescription::SD_Target;
				CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( (UINT)eSheetDescription );

				if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetResizingColumnInfo() )
				{
					delete pclMessage;
					return 0;
				}

				switch( prParams->m_eKeyboardEventType )
				{
					case CMessageManager::SSheetKET::KET_VirtualKeyDown:

						if( VK_DOWN == prParams->m_iKeyCode )
						{
							_OnKeyDown( pclSheetDescription );
						}
						else if( VK_UP == prParams->m_iKeyCode )
						{
							_OnKeyUp( pclSheetDescription );
						}
						else if( VK_HOME == prParams->m_iKeyCode )
						{
							_OnKeyHome( pclSheetDescription );
						}
						else if( VK_END == prParams->m_iKeyCode )
						{
							_OnKeyEnd( pclSheetDescription );
						}
						else if( VK_NEXT == prParams->m_iKeyCode )
						{
							_OnKeyPageDown( pclSheetDescription );
						}
						else if( VK_PRIOR == prParams->m_iKeyCode )
						{
							_OnKeyPageUp( pclSheetDescription );
						}

						break;
				}
				break;
			}

		case CMessageManager::SSheetMessageType::SSheetMST_ColumnWidthChanged:
			{
				CMessageManager::CSSheetCWCMsg *pclSShetCWCMsg = dynamic_cast<CMessageManager::CSSheetCWCMsg *>( pclMessage );

				if( NULL == pclSShetCWCMsg )
				{
					delete pclMessage;
					return 0;
				}

				CMessageManager::SSheetCWCParams *prParams = pclSShetCWCMsg->GetParams();

				if( NULL == prParams->m_hSheet || ( prParams->m_hSheet != m_pclSSheetSource->GetSafeHwnd() && prParams->m_hSheet != m_pclSSheetTarget->GetSafeHwnd() ) )
				{
					delete pclMessage;
					return 0;
				}

				if( prParams->m_hSheet == m_pclSSheetSource->GetSafeHwnd() )
				{
					long lNewColumnWidth = m_pclSSheetSource->GetColWidthInPixelsW( prParams->m_lColumn );
					m_pclSSheetTarget->SetColWidthInPixels( prParams->m_lColumn, lNewColumnWidth );
				}
				else
				{
					long lNewColumnWidth = m_pclSSheetTarget->GetColWidthInPixelsW( prParams->m_lColumn );
					m_pclSSheetSource->SetColWidthInPixels( prParams->m_lColumn, lNewColumnWidth );
				}
			}
			break;
	}

	delete pclMessage;
	return 0;
}

BOOL CDlgSearchAndReplaceCircuitsResults::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if( SSN_TOPROWCHANGE == HIWORD( wParam ) )
	{
		// Scroll if needed.
		// Remark: the SSheet that has the focus will scroll alone. So we need to check the other SSheet.
		long lSourceTopRow = m_pclSSheetSource->GetTopRow();
		long lTargetTopRow = m_pclSSheetTarget->GetTopRow();

		if( lSourceTopRow != lTargetTopRow )
		{
			if( (HWND)lParam == m_pclSSheetSource->GetSafeHwnd() )
			{
				m_pclSSheetTarget->ShowCell( (long)ColumnDefinition::CD_Index, lSourceTopRow, SS_SHOW_TOPLEFT );
			}
			else
			{
				m_pclSSheetSource->ShowCell( (long)ColumnDefinition::CD_Index, lTargetTopRow, SS_SHOW_TOPLEFT );
			}
		}
	}

	return CDialogEx::OnCommand( wParam, lParam );
}

void CDlgSearchAndReplaceCircuitsResults::ResizingColumnInfoNH_OnSSheetColumnWidthChanged( UINT uiSheetDescriptionID, long lColumnID )
{
	if( (UINT)SheetDescription::SD_Source == uiSheetDescriptionID )
	{
		long lNewColumnWidth = m_pclSSheetSource->GetColWidthInPixelsW( lColumnID );

		m_pclSSheetTarget->SetColWidthInPixels( lColumnID, lNewColumnWidth );
		m_pclResizingColumnnInfoTarget->Rescan();
	}
	else
	{
		long lNewColumnWidth = m_pclSSheetTarget->GetColWidthInPixelsW( lColumnID );
		m_pclSSheetSource->SetColWidthInPixels( lColumnID, lNewColumnWidth );
		m_pclResizingColumnnInfoSource->Rescan();
	}
}

void CDlgSearchAndReplaceCircuitsResults::_InitializeSSheet( SheetDescription eSheetDescription )
{
	CSSheet *pclSSheet = new CSSheet();

	if( NULL == pclSSheet )
	{
		ASSERT_RETURN;
	}

	int iStaticCtrlID = ( SheetDescription::SD_Source == eSheetDescription ) ? IDC_LIST_SOURCES : IDC_LIST_TARGETS;
	
	//  Take size of the static control.
	CRect rect;
	GetDlgItem( iStaticCtrlID )->GetClientRect( &rect );

	BOOL bReturn = pclSSheet->Create( ( GetDlgItem( iStaticCtrlID )->GetStyle() | WS_CHILD | WS_VISIBLE & ~WS_BORDER ), rect, this, IDC_FPSPREAD );

	if( FALSE == bReturn )
	{
		ASSERT_RETURN;
	}

	// Position of the sheet.
	GetDlgItem( iStaticCtrlID )->GetWindowRect( &rect );
	ScreenToClient( &rect );
	pclSSheet->SetWindowPos( &CWnd::wndTop, rect.left, rect.top, -1, -1, SWP_NOSIZE );

	CSheetDescription *pclSheetDescription = m_ViewDescription.AddSheetDescription( (UINT)eSheetDescription, -1, pclSSheet, CPoint( 0, 0 ) );

	if( NULL == pclSheetDescription )
	{
		ASSERT_RETURN;
	}

	if( SheetDescription::SD_Source == eSheetDescription )
	{
		m_pclSSheetSource = pclSSheet;
	}
	else if( SheetDescription::SD_Target == eSheetDescription )
	{
		m_pclSSheetTarget = pclSSheet;
	}

	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, TRUE );
	pclSSheet->SetBool( SSB_SCROLLBAREXTMODE, TRUE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );

	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

#ifdef DEBUG
	pclSSheet->SetBool( SSB_GRIDLINES, TRUE );
#endif

	// Set max rows.
	pclSSheet->SetMaxRows( 1 );
	pclSSheet->SetMaxCols( 7 );

	// Freeze the first row.
	pclSSheet->SetFreeze( 0, 1 );

	pclSheetDescription->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Set all widths.
	m_StaticSourcesRect.GetClientRect( &rect );
	double dAvailableWidth = pclSSheet->LogUnitsToColWidthW( rect.Width() );
	double dStandardWidthInPixel = pclSSheet->ColWidthToLogUnits( 1 ) - 1.0;

	double dTotalColWidth = (double)ColumWith::CW_Index + (double)ColumWith::CW_CircuitName + (double)ColumWith::CW_BalancingType;
	dTotalColWidth += (double)ColumWith::CW_CircuitType + (double)ColumWith::CW_ValveType + (double)ColumWith::CW_ControlType;
	dTotalColWidth += (double)ColumWith::CW_ControlValveType;
	double dRatio = dAvailableWidth / dTotalColWidth * dStandardWidthInPixel;

	// Index.
	long lWidthInPixel = (long)( dRatio * (double)ColumWith::CW_Index );
	long lTotalWidthInPixel = lWidthInPixel;
	pclSheetDescription->AddColumnInPixels( (long)ColumnDefinition::CD_Index, lWidthInPixel );

	// Circuit name.
	lWidthInPixel = (long)( dRatio * (double)ColumWith::CW_CircuitName );
	lTotalWidthInPixel += lWidthInPixel;
	pclSheetDescription->AddColumnInPixels( (long)ColumnDefinition::CD_CircuitName, lWidthInPixel );

	// Balancing type.
	lWidthInPixel = (long)( dRatio * (double)ColumWith::CW_BalancingType );
	lTotalWidthInPixel += lWidthInPixel;
	pclSheetDescription->AddColumnInPixels( (long)ColumnDefinition::CD_BalancingType, lWidthInPixel );

	// Circuit type.
	lWidthInPixel = (long)( dRatio * (double)ColumWith::CW_CircuitType );
	lTotalWidthInPixel += lWidthInPixel;
	pclSheetDescription->AddColumnInPixels( (long)ColumnDefinition::CD_CircuitType, lWidthInPixel );

	// Valve type.
	lWidthInPixel = (long)( dRatio * (double)ColumWith::CW_ValveType );
	lTotalWidthInPixel += lWidthInPixel;
	pclSheetDescription->AddColumnInPixels( (long)ColumnDefinition::CD_ValveType, lWidthInPixel );

	// Control type.
	lWidthInPixel = (long)( dRatio * (double)ColumWith::CW_ControlType );
	lTotalWidthInPixel += lWidthInPixel;
	pclSheetDescription->AddColumnInPixels( (long)ColumnDefinition::CD_ControlType, lWidthInPixel );

	// Control valve type.
	pclSheetDescription->AddColumnInPixels( (long)ColumnDefinition::CD_ControlValveType, rect.Width() - lTotalWidthInPixel );

	// Set the focus column.
	pclSheetDescription->SetActiveColumn( (long)ColumnDefinition::CD_Index );

	// Set range for selection.
	pclSheetDescription->SetFocusColumnRange( (long)ColumnDefinition::CD_Index, (long)ColumnDefinition::CD_ControlValveType );

	// Set all titles.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::ColumnHeader );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_TAH_TITLE_MAIN );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_WHITE );
	
	pclSSheet->SetStaticText( (long)ColumnDefinition::CD_Index, 1, _T("#") );
	pclSSheet->SetCellBorder( (long)ColumnDefinition::CD_Index, 1, true, SS_BORDERTYPE_RIGHT );

	CString str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_CIRCUITNAME );
	pclSSheet->SetStaticText( (long)ColumnDefinition::CD_CircuitName, 1, str );
	pclSSheet->SetCellBorder( (long)ColumnDefinition::CD_CircuitName, 1, true, SS_BORDERTYPE_RIGHT );

	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_BALANCINGTYPE );
	pclSSheet->SetStaticText( (long)ColumnDefinition::CD_BalancingType, 1, str );
	pclSSheet->SetCellBorder( (long)ColumnDefinition::CD_BalancingType, 1, true, SS_BORDERTYPE_RIGHT );

	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_CIRCUITTYPE );
	pclSSheet->SetStaticText( (long)ColumnDefinition::CD_CircuitType, 1, str );
	pclSSheet->SetCellBorder( (long)ColumnDefinition::CD_CircuitType, 1, true, SS_BORDERTYPE_RIGHT );

	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_VALVETYPE );
	pclSSheet->SetStaticText( (long)ColumnDefinition::CD_ValveType, 1, str );
	pclSSheet->SetCellBorder( (long)ColumnDefinition::CD_ValveType, 1, true, SS_BORDERTYPE_RIGHT );

	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_CONTROLTYPE );
	pclSSheet->SetStaticText( (long)ColumnDefinition::CD_ControlType, 1, str );
	pclSSheet->SetCellBorder( (long)ColumnDefinition::CD_ControlType, 1, true, SS_BORDERTYPE_RIGHT );

	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_CONTROLVALVETYPE );
	pclSSheet->SetStaticText( (long)ColumnDefinition::CD_ControlValveType, 1, str );
	pclSSheet->SetCellBorder( (long)ColumnDefinition::CD_ControlValveType, 1, true, SS_BORDERTYPE_RIGHT );

	// Draw black line below column header.
	pclSSheet->SetCellBorder( (long)ColumnDefinition::CD_Index, 1, (long)ColumnDefinition::CD_ControlValveType, 1, true, SS_BORDERTYPE_BOTTOM );
}

void CDlgSearchAndReplaceCircuitsResults::_FillSheet( SheetDescription eSheetDescription )
{
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( (UINT)eSheetDescription );

	if( NULL == pclSheetDescription )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		ASSERT_RETURN;
	}

	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		ASSERT_RETURN;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		ASSERT_RETURN;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT ) )
	{
		ASSERT_RETURN;
	}

	for( auto &iter : *m_pvecChangesList )
	{
		pclSheetDescription->AddRows( 1, true );
		CDlgSearchAndReplaceCircuits::CCircuit *pCircuit = ( SheetDescription::SD_Source == eSheetDescription ) ? &iter.first : &iter.second;

		long lRow = pclSheetDescription->GetLastRow();

		if( SheetDescription::SD_Source == eSheetDescription || true == pCircuit->m_bFoundSolution )
		{
			CString str;
			str.Format( _T("%i"), lRow - 1 );
			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_Index, lRow, str );
			
			// Hydraulic circuit name.
			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_CircuitName, lRow, pCircuit->m_strHMName );
			
			// Balancing type.
			str = _T("-");

			if( true == pCircuit->m_bIsBalTypeMustBeChecked )
			{
				str = CDB_CircuitScheme::GetBalTypeStr( pCircuit->m_eBalType ).c_str();
			}

			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_BalancingType, lRow, str );

			// Circuit type.
			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_CircuitType, lRow, pCircuit->m_pclCircSchemeCategory->GetSchName().c_str() );
			
			// Valve type.
			str = _T("-");

			if( true == pCircuit->m_bIsValveTypeMustBeChecked )
			{
				if( _eWorkForCV::ewcvTA == pCircuit->m_eValveType )
				{
					str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_IMIVALVE );
				}
				else
				{
					str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_KVSVALVE );
				}
			}

			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_ValveType, lRow, str );
			
			// Control type.
			str = _T("-");

			if( true == pCircuit->m_bIsCtrlTypeMustBeChecked )
			{
				str = CDB_ControlProperties::GetCvCtrlTypeStr( pCircuit->m_eControlType ).c_str();
			}

			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_ControlType, lRow, str );

			// Control valve type.
			str = _T("-");

			if( true == pCircuit->m_bIsCtrlValveTypeMustBeChecked )
			{
				str = CDB_ControlProperties::GetCvFuncStr( pCircuit->m_eControlValveType ).c_str();
			}

			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_ControlValveType, lRow, str );
		}
		else
		{
			CString str;
			str.Format( _T("%i"), lRow - 1 );
			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_Index, lRow, str );

			pclSSheet->AddCellSpan( (long)ColumnDefinition::CD_CircuitName, lRow, (long)ColumnDefinition::CD_ControlValveType - (long)ColumnDefinition::CD_CircuitName + 1, 1 );
			str = TASApp.LoadLocalizedString( IDS_DLGSEARCHANDREPLACECIRCUITSRESULTS_NOTFOUND );
			pclSSheet->SetStaticText( (long)ColumnDefinition::CD_CircuitName, lRow, str );
		}
	}

	// Select the fisrt row.
	pclSSheet->SelectOneRow( 2 );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescription->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, 2, pclSheetDescription->GetLastRow() );
	pclResizingColumnInfo->AddRangeColumn( (long)ColumnDefinition::CD_Index, (long)ColumnDefinition::CD_ControlValveType, 1, 1 );

	// Register notification to receive messages from CSSheet to allow to send them to the 'CResizingColumnInfo' class.
	pclSSheet->MM_RegisterNotificationHandler( this, CMessageManager::SSheetNHFlags::SSheetNHF_MouseMove 
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDown
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonUp
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseLButtonDblClk
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseRButtonDown
			| CMessageManager::SSheetNHFlags::SSheetNHF_MouseCaptureChanged
			| CMessageManager::SSheetNHFlags::SSheetNHF_KeyboardVirtualKeyDown );

	// Register to receive a notification when a column width has been changed.
	pclResizingColumnInfo->RegisterNotificationHandler( this );

	if( SheetDescription::SD_Source == eSheetDescription )
	{
		m_pclResizingColumnnInfoSource = pclResizingColumnInfo;
	}
	else
	{
		m_pclResizingColumnnInfoTarget = pclResizingColumnInfo;
	}
}

void CDlgSearchAndReplaceCircuitsResults::_OnCellClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow )
{
	if( NULL == pclSheetDescription || 1 == lClickedRow )
	{
		return;
	}

	long lStartCol;
	long lStartRow;
	long lEndCol;
	long lEndRow;
	m_pclSSheetSource->GetSelectionPos( lStartCol, lStartRow, lEndCol, lEndRow );

	if( lClickedRow == lStartRow )
	{
		return;
	}

	// Unselect previous.
	m_pclSSheetSource->UnSelectMultipleRows( lStartRow );
	m_pclSSheetTarget->UnSelectMultipleRows( lStartRow );

	// Select new.
	m_pclSSheetSource->SelectOneRow( lClickedRow );
	m_pclSSheetTarget->SelectOneRow( lClickedRow );
}

void CDlgSearchAndReplaceCircuitsResults::_OnKeyDown( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription )
	{
		return;
	}

	long lStartCol;
	long lStartRow;
	long lEndCol;
	long lEndRow;
	m_pclSSheetSource->GetSelectionPos( lStartCol, lStartRow, lEndCol, lEndRow );
	
	if( lStartRow != pclSheetDescription->GetLastRow() )
	{
		// Unselect previous.
		m_pclSSheetSource->UnSelectMultipleRows( lStartRow );
		m_pclSSheetTarget->UnSelectMultipleRows( lStartRow );

		// Select new.
		long lNewRow = lStartRow + 1;
		m_pclSSheetSource->SelectOneRow( lNewRow );
		m_pclSSheetSource->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

		m_pclSSheetTarget->SelectOneRow( lNewRow );
		m_pclSSheetTarget->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );
	}
}

void CDlgSearchAndReplaceCircuitsResults::_OnKeyUp( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription )
	{
		return;
	}

	long lStartCol;
	long lStartRow;
	long lEndCol;
	long lEndRow;
	m_pclSSheetSource->GetSelectionPos( lStartCol, lStartRow, lEndCol, lEndRow );
	
	if( 2 == lStartRow )
	{
		return;
	}

	// Unselect previous.
	m_pclSSheetSource->UnSelectMultipleRows( lStartRow );
	m_pclSSheetTarget->UnSelectMultipleRows( lStartRow );

	// Select new.
	long lNewRow = lStartRow - 1;
	m_pclSSheetSource->SelectOneRow( lNewRow );
	m_pclSSheetSource->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

	m_pclSSheetTarget->SelectOneRow( lNewRow );
	m_pclSSheetSource->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );
}

void CDlgSearchAndReplaceCircuitsResults::_OnKeyHome( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription )
	{
		return;
	}

	long lStartCol;
	long lStartRow;
	long lEndCol;
	long lEndRow;
	m_pclSSheetSource->GetSelectionPos( lStartCol, lStartRow, lEndCol, lEndRow );
	
	if( 2 == lStartRow )
	{
		return;
	}

	// Unselect previous.
	m_pclSSheetSource->UnSelectMultipleRows( lStartRow );
	m_pclSSheetTarget->UnSelectMultipleRows( lStartRow );

	// Select new.
	long lNewRow = 2;
	m_pclSSheetSource->SelectOneRow( lNewRow );
	m_pclSSheetSource->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

	m_pclSSheetTarget->SelectOneRow( lNewRow );
	m_pclSSheetTarget->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

	if( FALSE == m_pclSSheetSource->GetVisible( (long)ColumnDefinition::CD_Index, lNewRow, SS_VISIBLE_PARTIAL ) )
	{
		// Just need to do it for one sheet. Because we will receive the 'OnCommand' with the 'SSN_TOPROWCHANGE' notificaiton.
		m_pclSSheetSource->ShowCell( (long)ColumnDefinition::CD_Index, lNewRow, SS_SHOW_TOPLEFT );
	}
}

void CDlgSearchAndReplaceCircuitsResults::_OnKeyEnd( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription )
	{
		return;
	}

	long lStartCol;
	long lStartRow;
	long lEndCol;
	long lEndRow;
	m_pclSSheetSource->GetSelectionPos( lStartCol, lStartRow, lEndCol, lEndRow );
	
	if( lStartRow == pclSheetDescription->GetLastRow() )
	{
		return;
	}

	// Unselect previous.
	m_pclSSheetSource->UnSelectMultipleRows( lStartRow );
	m_pclSSheetTarget->UnSelectMultipleRows( lStartRow );

	// Select new.
	long lNewRow = pclSheetDescription->GetLastRow();
	m_pclSSheetSource->SelectOneRow( lNewRow );
	m_pclSSheetSource->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

	m_pclSSheetTarget->SelectOneRow( lNewRow );
	m_pclSSheetTarget->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

	if( FALSE == m_pclSSheetSource->GetVisible( (long)ColumnDefinition::CD_Index, lNewRow, SS_VISIBLE_PARTIAL ) )
	{
		// Just need to do it for one sheet. Because we will receive the 'OnCommand' with the 'SSN_TOPROWCHANGE' notificaiton.
		m_pclSSheetSource->ShowCell( (long)ColumnDefinition::CD_Index, lNewRow, SS_SHOW_BOTTOMLEFT );
	}
}

void CDlgSearchAndReplaceCircuitsResults::_OnKeyPageDown( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription )
	{
		return;
	}

	long lStartCol;
	long lStartRow;
	long lEndCol;
	long lEndRow;
	m_pclSSheetSource->GetSelectionPos( lStartCol, lStartRow, lEndCol, lEndRow );
	
	long lNewRow = min( pclSheetDescription->GetLastRow(), lStartRow + 5 );

	if( lNewRow != lStartRow )
	{
		// Unselect previous.
		m_pclSSheetSource->UnSelectMultipleRows( lStartRow );
		m_pclSSheetTarget->UnSelectMultipleRows( lStartRow );

		// Select new.
		m_pclSSheetSource->SelectOneRow( lNewRow );
		m_pclSSheetSource->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

		m_pclSSheetTarget->SelectOneRow( lNewRow );
		m_pclSSheetTarget->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

		if( FALSE == m_pclSSheetSource->GetVisible( (long)ColumnDefinition::CD_Index, lNewRow, SS_VISIBLE_PARTIAL ) )
		{
			// Just need to do it for one sheet. Because we will receive the 'OnCommand' with the 'SSN_TOPROWCHANGE' notificaiton.
			m_pclSSheetSource->ShowCell( (long)ColumnDefinition::CD_Index, lNewRow, SS_SHOW_BOTTOMLEFT );
		}
	}
}

void CDlgSearchAndReplaceCircuitsResults::_OnKeyPageUp( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription )
	{
		return;
	}

	long lStartCol;
	long lStartRow;
	long lEndCol;
	long lEndRow;
	m_pclSSheetSource->GetSelectionPos( lStartCol, lStartRow, lEndCol, lEndRow );
	
	long lNewRow = max( 2, lStartRow - 5 );

	if( lNewRow != lStartRow )
	{
		// Unselect previous.
		m_pclSSheetSource->UnSelectMultipleRows( lStartRow );
		m_pclSSheetTarget->UnSelectMultipleRows( lStartRow );

		// Select new.
		m_pclSSheetSource->SelectOneRow( lNewRow );
		m_pclSSheetSource->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

		m_pclSSheetTarget->SelectOneRow( lNewRow );
		m_pclSSheetTarget->SetActiveCell( (long)ColumnDefinition::CD_Index, lNewRow );

		if( FALSE == m_pclSSheetSource->GetVisible( (long)ColumnDefinition::CD_Index, lNewRow, SS_VISIBLE_PARTIAL ) )
		{
			// Just need to do it for one sheet. Because we will receive the 'OnCommand' with the 'SSN_TOPROWCHANGE' notificaiton.
			m_pclSSheetSource->ShowCell( (long)ColumnDefinition::CD_Index, lNewRow, SS_SHOW_BOTTOMLEFT );
		}
	}
}
