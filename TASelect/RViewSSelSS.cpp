#include "stdafx.h"
#include <algorithm>
#include "TASelect.h"
#include "MainFrm.h"
#include "Utilities.h"

#include "DlgLeftTabSelManager.h"
#include "ProductSelectionParameters.h"
#include "DlgTender.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CCellDescriptionButton::CCellDescriptionButton( int iButtonType, CPoint ptCellPosition, CSheetDescription *pclSheetDescription, 
	bool bShowStatus, int iButtonState, CSSheet::PictureCellType ePictureType )
		: CCellDescription( iButtonType, ptCellPosition, pclSheetDescription )
{
	m_bShowStatus = bShowStatus;
	m_iButtonState = iButtonState;
	m_ePictureType = ePictureType;
}

void CCellDescriptionButton::SetButtonState( int iButtonState, bool bApplyChange )
{ 
	m_iButtonState = iButtonState;

	if( true == bApplyChange )
	{
		// Show/Hide corresponding image.
		SetShowStatus( m_bShowStatus );
	}
}

void CCellDescriptionButton::SetShowStatus( bool bShowStatus )
{
	m_bShowStatus = bShowStatus;
	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();
	
	if( NULL != pclSSheet )
	{
		if( true == m_bShowStatus )
		{
			if( m_mapStateImageID.count( m_iButtonState ) > 0 )
			{
				int iPictureID = m_mapStateImageID[m_iButtonState];
				pclSSheet->SetPictureCellWithID( iPictureID, m_ptCellPosition.x, m_ptCellPosition.y, m_ePictureType );
			}
		}
		else
		{
			DeleteButton();
		}
	}
}

void CCellDescriptionButton::ApplyInternalChange( void )
{
	// Show/Hide corresponding image.
	SetShowStatus( m_bShowStatus );
}

void CCellDescriptionButton::DeleteButton( void )
{
	// Change cell type to text.
	SS_CELLTYPE rCellType;
	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();
	
	if( NULL == pclSSheet )
	{
		return;
	}

	if( NULL == pclSSheet->SetTypeStaticText( &rCellType, SS_TEXT_LEFT ) )
	{
		return;
	}

	pclSSheet->SetCellType( m_ptCellPosition.x, m_ptCellPosition.y, &rCellType );

	// Reset content
	pclSSheet->SetData( m_ptCellPosition.x, m_ptCellPosition.y, _T("") );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CCDButtonBox
CCDButtonBox::CCDButtonBox( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, ButtonState eButtonState ) 
	: CCellDescriptionButton( RVSCellDescription::CDB_Box, ptCellPosition, pclSheetDescription, bShowStatus, (int)eButtonState, CSSheet::PictureCellType::Icon )
{
	// TODO: set correct button state and corresponding image ID.
	m_mapStateImageID[BoxOpened] = IDI_OPENBOX16x16;
	m_mapStateImageID[BoxClosed] = IDI_CLOSEDBOX16x16;
}

void CCDButtonBox::ToggleButtonState( bool bApplyChange )
{
	// Toggle button state.
	m_iButtonState = ( BoxOpened == m_iButtonState ) ? BoxClosed : BoxOpened;

	if( true == bApplyChange )
	{
		ApplyInternalChange();
	}
}
// End of class CCDButtonBox
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CCDButtonShowAllPriorities
CCDButtonShowAllPriorities::CCDButtonShowAllPriorities( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, 
		ButtonState eButtonState, long lFromRow, long lToRow ) 
	: CCellDescriptionButton( RVSCellDescription::CDB_ShowAllPriorities, ptCellPosition, pclSheetDescription, bShowStatus, 
		(int)eButtonState, CSSheet::PictureCellType::Bitmap ) 
{
	m_lFromRow = lFromRow;
	m_lToRow = lToRow;
	m_mapStateImageID[ShowPriorities] = IDB_ARROWDOWN_LOW;
	m_mapStateImageID[HidePriorities] = IDB_ARROWUP_LOW;
}

void CCDButtonShowAllPriorities::ToggleButtonState( bool bApplyChange )
{
	// Toggle button state.
	m_iButtonState = ( ShowPriorities == m_iButtonState ) ? HidePriorities : ShowPriorities;

	if( true == bApplyChange )
	{
		ApplyInternalChange();
	}
}

void CCDButtonShowAllPriorities::ApplyInternalChange( void )
{
	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		return;
	}
	
	CCellDescriptionButton::ApplyInternalChange();

	// Show/Hide rows.
	for( long lLoopRow = m_lFromRow; lLoopRow <= m_lToRow; lLoopRow++ )
	{
		// If button state is set to 'HidePriorities' (arrow up), it means that rows must be shown.
		pclSSheet->ShowRow( lLoopRow, ( HidePriorities == m_iButtonState ) );
	}
}
// End of class CCDButtonShowAllPriorities
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CCDButtonExpandCollapseRow
CCDButtonExpandCollapseRows::CCDButtonExpandCollapseRows( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, 
		ButtonState eButtonState, long lFromRow, long lToRow ) 
	: CCellDescriptionButton( RVSCellDescription::CDB_ExpandCollapseRow, ptCellPosition, pclSheetDescription, bShowStatus, (int)eButtonState, 
		CSSheet::PictureCellType::Bitmap ) 
{
	if( lFromRow < 1 )
	{
		lFromRow = ptCellPosition.y;
	}

	if( lToRow < 1 )
	{
		lToRow = ptCellPosition.y;
	}

	if( lFromRow > lToRow )
	{
		long lTemp = lFromRow;
		lFromRow = lToRow;
		lToRow = lTemp;
	}

	m_lFromRow = lFromRow;
	m_lToRow = lToRow;
	// Why not +1: because we don't need to keep status of current row.
	m_parbState = new BOOL[m_lToRow - m_lFromRow];
	m_mapStateImageID[CollapseRow] = IDB_ARROWCOLLAPSEROW;
	m_mapStateImageID[ExpandRow] = IDB_ARROWEXPANDROW;
}

CCDButtonExpandCollapseRows::~CCDButtonExpandCollapseRows()
{
	if( NULL != m_parbState )
	{
		delete m_parbState;
	}
}

void CCDButtonExpandCollapseRows::ToggleButtonState( bool bApplyChange )
{
	// Toggle button state.
	m_iButtonState = ( CollapseRow == m_iButtonState ) ? ExpandRow : CollapseRow;

	if( true == bApplyChange )
	{
		ApplyInternalChange();
	}
}

void CCDButtonExpandCollapseRows::ApplyInternalChange( void )
{
	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		return;
	}
	
	CCellDescriptionButton::ApplyInternalChange();

	// Show/Hide rows.
	long lLoopRow, lIndex;

	for( lLoopRow = m_lFromRow, lIndex = 0; lLoopRow <= m_lToRow; lLoopRow++ )
	{
		// If it's not the row where is placed the button...
		if( lLoopRow != GetCellPosition().y )
		{
			// If button is set in 'CollapseRow' it means that we must show rows.
			if( CollapseRow == m_iButtonState )
			{
				// State must be restored as it was before collapse has been done.
				pclSSheet->ShowRow( lLoopRow, m_parbState[lIndex] );
			}
			else
			{
				// Save state of each row before hide it.
				m_parbState[lIndex] = !pclSSheet->IsRowHidden( lLoopRow );
				pclSSheet->ShowRow( lLoopRow, FALSE );
			}

			lIndex++;
		}
	}
}

void CCDButtonExpandCollapseRows::DeleteButton( void )
{
	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		return;
	}
	
	// Before deleting button, be sure that rows are well expanded.
	if( ExpandRow == m_iButtonState )
	{
		// Show or hide rows.
		long lLoopRow, lIndex;

		for( lLoopRow = m_lFromRow, lIndex = 0; lLoopRow <= m_lToRow; lLoopRow++ )
		{
			// If it's not the row where is placed the button...
			if( lLoopRow != GetCellPosition().y )
			{
				// If button state is set to 'CollapseRow', it means that rows must be shown.
				// Remark: state must be restored as it was before collapse has been done.
				pclSSheet->ShowRow( lLoopRow, m_parbState[lIndex++] );
			}
		}
	}

	CCellDescriptionButton::DeleteButton();
}
// End of class CCDButtonExpandCollapseRow
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CCDButtonExpandCollapseGroup
CCDButtonExpandCollapseGroup::CCDButtonExpandCollapseGroup( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, 
		ButtonState eButtonState, long lFromRow, long lToRow ) 
	: CCellDescriptionButton( RVSCellDescription::CDB_ExpandCollapseGroup, ptCellPosition, pclSheetDescription, bShowStatus, (int)eButtonState, 
		CSSheet::PictureCellType::Bitmap ) 
{
	if( lFromRow < 1 )
	{
		lFromRow = ptCellPosition.y;
	}

	if( lToRow < 1 )
	{
		lToRow = ptCellPosition.y;
	}

	if( lFromRow > lToRow )
	{
		long lTemp = lFromRow;
		lFromRow = lToRow;
		lToRow = lTemp;
	}

	m_lFromRow = lFromRow;
	m_lToRow = lToRow;

	// PAY ATTENTION: if group is for a valve for example, we have 1 line by item. Thus 'm_lToRow' and 'm_lFromRow' are well set
	// to the line number (Ex: 2, 2 for one line -> new BOOL[ 2 - 2 + 1 ].
	// If group is for accessories for example, we have 2 lines by item. If 'm_lToRow' and 'm_lFromRow' are set respectively to
	// 3 and 5, that not means we have 3 lines of parameters. But well 2 lines: 3,4 for the first one and 5,6 for the second one.
	// Thus for 3, 5 -> new BOOL[ 5 - 3 + 2 ].
	m_parbState = new BOOL[m_lToRow - m_lFromRow + m_pclSheetDescription->GetRowsByItem() ];

	m_mapStateImageID[CollapseRow] = IDB_ARROWCOLLAPSEGROUP;
	m_mapStateImageID[ExpandRow] = IDB_ARROWEXPANDGROUP;
}

CCDButtonExpandCollapseGroup::~CCDButtonExpandCollapseGroup()
{
	if( NULL != m_parbState )
	{
		delete m_parbState;
	}
}

void CCDButtonExpandCollapseGroup::ToggleButtonState( bool bApplyChange )
{
	// Toggle button state.
	m_iButtonState = ( CollapseRow == m_iButtonState ) ? ExpandRow : CollapseRow;

	if( true == bApplyChange )
	{
		ApplyInternalChange();
	}
}

void CCDButtonExpandCollapseGroup::ApplyInternalChange( void )
{
	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		return;
	}
	
	CCellDescriptionButton::ApplyInternalChange();

	// Check if there is a selected line in this group. Because if it's the case, we don't want to collapse
	// the selected one with all the group.
	LPARAM lParam = (LPARAM)0;
	long lSelectedRow = 0;
// 	m_pclSheetDescription->GetUserVariable( CRViewSSelSS::_SDUV_SELECTEDPRODUCT, lParam );
// 	CCellDescriptionProduct* pclCellDescriptionProduct = (CCellDescriptionProduct*)lParam;
// 	if( NULL != pclCellDescriptionProduct )
// 		lSelectedRow = pclCellDescriptionProduct->GetCellPosition().y;

	// Show/Hide rows.
	int iRowsByItem = m_pclSheetDescription->GetRowsByItem();
	long lLoopRow, lIndex;
	
	for( lLoopRow = m_lFromRow, lIndex = 0; lLoopRow <= ( m_lToRow + iRowsByItem - 1 ); lLoopRow++ )
	{
		if( 0 == lSelectedRow || lLoopRow < lSelectedRow || lLoopRow > ( lSelectedRow + iRowsByItem - 1 ) )
		{
			// If button is set in 'CollapseRow' it means that we must show rows.
			if( CollapseRow == m_iButtonState )
			{
				// State must be restored as it was before collapse has been done.
				pclSSheet->ShowRow( lLoopRow, m_parbState[lIndex] );
			}
			else
			{
				// Save state of each row before hide it.
				m_parbState[lIndex] = !pclSSheet->IsRowHidden( lLoopRow );
				pclSSheet->ShowRow( lLoopRow, FALSE );
			}

			lIndex++;
		}
	}
}

void CCDButtonExpandCollapseGroup::DeleteButton( void )
{
	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		return;
	}

	// Before deleting button, be sure that rows are well expanded.
	if( ExpandRow == m_iButtonState )
	{
		// Show or hide rows.
		long lLoopRow, lIndex;
		int iRowsByItem = m_pclSheetDescription->GetRowsByItem();
		
		for( lLoopRow = m_lFromRow, lIndex = 0; lLoopRow <= m_lToRow; lLoopRow++ )
		{
			// If it's not the row where is placed the button...
			if( lLoopRow != GetCellPosition().y )
			{
				// If button state is set to 'CollapseRow', it means that rows must be shown.
				// Remark: state must be restored as it was before collapse has been done.
				for( int iLoopSubRows = 0; iLoopSubRows < iRowsByItem; iLoopSubRows++ )
				{
					pclSSheet->ShowRow( lLoopRow + iLoopSubRows, m_parbState[lIndex++] );
				}
			}
		}
	}

	CCellDescriptionButton::DeleteButton();
}
// End of class CCDButtonExpandCollapseGroup
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CCDButtonCheckbox
CCDButtonCheckbox::CCDButtonCheckbox( int iCheckboxType, CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, 
			bool bChecked, bool bEnabled, CString strText ) 
	: CCellDescriptionButton( iCheckboxType, ptCellPosition, pclSheetDescription, bShowStatus, (int)bChecked, CSSheet::PictureCellType::Bitmap ) 
{ 
	m_bEnabled = bEnabled;
	m_strText = strText;

	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();
	
	if( NULL != pclSSheet )
	{
		// Prepare text pattern
		pclSSheet->SetTextPattern(CSSheet::_SSTextPattern::Arial8 );
		
		if( false == m_bEnabled )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_VLIGHTGRAY );
		}

		pclSSheet->ApplyTextPattern( m_ptCellPosition.x, m_ptCellPosition.y );
	}
}

void CCDButtonCheckbox::ToggleButtonState( bool bApplyChange )
{
	// Toggle button state.
	m_iButtonState = ( Unchecked == m_iButtonState ) ? Checked : Unchecked;

	if( true == bApplyChange )
	{
		ApplyInternalChange();
	}
}

void CCDButtonCheckbox::ApplyInternalChange( void )
{
	CSSheet *pclSSheet = m_pclSheetDescription->GetSSheetPointer();
	
	if( NULL == pclSSheet )
	{
		return;
	}

	// Do not call 'CCDButtonCheckbox:ApplyInternalChange'  !!!
	pclSSheet->SetCheckBox( m_ptCellPosition.x, m_ptCellPosition.y, m_strText, ( Unchecked == m_iButtonState ) ? false : true, m_bEnabled );
}
// End of class CCDButtonCheckbox
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class CCDBCheckboxAccessory
CCDBCheckboxAccessory::CCDBCheckboxAccessory( CPoint ptCellPosition, CSheetDescription *pclSheetDescription, bool bShowStatus, bool bChecked, 
		bool bEnabled, CString strText, CDB_Product *pAccessory, CDB_RuledTableBase *pRuledTable )
	: CCDButtonCheckbox( RVSCellDescription::CDBC_Accessories, ptCellPosition, pclSheetDescription, bShowStatus, bChecked, bEnabled, strText ) 
{ 
	m_pAccessory = pAccessory;
	m_pRuledTable = pRuledTable;
	m_strToolTip = _T("");
}
// End of class CCDBCheckboxAccessory
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CRViewSSelSS
CRViewSSelSS::CRViewSSelSS( CMainFrame::RightViewList eRightViewID, bool bUseOnlyOneSpread, UINT nID )
	:CMultiSpreadInView( eRightViewID, bUseOnlyOneSpread, nID )
{
	m_pclProdSelParams = NULL;
	m_stCurrentFocus.Reset();
	m_bSetAccessoriesCheckboxRunning = false;
	m_strProductRightClicked = _T("");
	m_mapNotificationHandlerList.clear();
}

void CRViewSSelSS::RegisterNotificationHandler( INotificationHandler *pclHandler, short nNotificationHandlerFlags )
{
	for( short nLoop = INotificationHandler::NH_First; nLoop <= INotificationHandler::NH_Last; nLoop <<= 1 )
	{
		if( nLoop == ( nNotificationHandlerFlags & nLoop ) )
		{
			if( 0 == m_mapNotificationHandlerList.count( nLoop ) )
			{
				m_mapNotificationHandlerList[nLoop].push_back( pclHandler );
			}
			else
			{
				if( m_mapNotificationHandlerList[nLoop].end() == std::find( m_mapNotificationHandlerList[nLoop].begin(), m_mapNotificationHandlerList[nLoop].end(), pclHandler ) )
				{
					m_mapNotificationHandlerList[nLoop].push_back( pclHandler );
				}
			}
		}
	}
}

void CRViewSSelSS::UnregisterNotificationHandler( INotificationHandler *pclHandler )
{
	std::map<short, std::vector<INotificationHandler*>>::iterator mapIter;
	
	for( mapIter = m_mapNotificationHandlerList.begin(); mapIter != m_mapNotificationHandlerList.end(); ++mapIter )
	{
		std::vector<INotificationHandler*>::iterator vecIter;
		vecIter = std::find( mapIter->second.begin(), mapIter->second.end(), pclHandler );
		
		if( vecIter != mapIter->second.end() )
		{
			mapIter->second.erase( vecIter );
		}
	}
}

void CRViewSSelSS::SetFocusW( bool bFirst )
{
	CSheetDescription *pclSheetDescription = NULL;
	pclSheetDescription = ( true == bFirst ) ? m_ViewDescription.GetTopSheetDescription() : m_ViewDescription.GetBottomSheetDescription();

	if( NULL == pclSheetDescription )
	{
		return;
	}

	while( NULL != pclSheetDescription )
	{
		if( -1 != pclSheetDescription->GetFirstSelectableRow( true ) )
		{
			break;
		}
		else
		{
			if( true == bFirst )
			{
				pclSheetDescription = m_ViewDescription.GetNextSheetDescription( pclSheetDescription );
			}
			else
			{
				pclSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclSheetDescription );
			}
		}
	}

	if( NULL != pclSheetDescription )
	{
		long lNewRow = pclSheetDescription->GetFirstSelectableRow( true );
		long lNewCol = pclSheetDescription->GetFirstFocusedColumn();
		PrepareAndSetNewFocus( pclSheetDescription, lNewCol, lNewRow );
	}
}

BOOL CRViewSSelSS::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, 
		UINT nID, CCreateContext *pContext ) 
{
	// Create the RViewSSelSS.
	BOOL bResult = CMultiSpreadInView::Create( lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext );
	return bResult;
}

void CRViewSSelSS::Reset( void )
{
	m_stCurrentFocus.Reset();
	m_bSetAccessoriesCheckboxRunning = false;
	m_strProductRightClicked = _T("");
	CMultiSpreadInView::Reset();
}

BEGIN_MESSAGE_MAP( CRViewSSelSS, CMultiSpreadInView )
	ON_WM_CREATE()
	ON_COMMAND( ID_SSELAFLT_GETFULLINFO, OnGetFullinfo )
	ON_COMMAND( ID_SSELAFLT_GETTENDER, OnGetTender )
	ON_COMMAND( ID_SSELAFLT_SELECT, OnfltSelect )
	ON_UPDATE_COMMAND_UI(ID_SSELAFLT_GETTENDER, OnUpdateGetTender)
	ON_UPDATE_COMMAND_UI_RANGE( ID_SSELAFLT_SELECT, ID_SSELAFLT_GETFULLINFO, OnUpdateMenuText )
	ON_MESSAGE( SSM_COMBOCLOSEUP, OnComboDropCloseUp )
END_MESSAGE_MAP()

void CRViewSSelSS::DoDataExchange( CDataExchange *pDX )
{
	CMultiSpreadInView::DoDataExchange( pDX );
}

LRESULT CRViewSSelSS::OnComboDropCloseUp( WPARAM wParam, LPARAM lParam )
{
	return 0;
}

void CRViewSSelSS::OnGetFullinfo()
{
	if( true == m_strProductRightClicked.IsEmpty() )
	{
		return;
	}
	
	LPARAM lParam = (LPARAM)(LPCTSTR)m_strProductRightClicked;
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_DISPLAYDOCUMENTATION, 0, lParam );

	m_strProductRightClicked = _T("");
}

void CRViewSSelSS::OnfltSelect()
{
	std::vector<INotificationHandler*>::iterator vecIter;
	
	for( vecIter = m_mapNotificationHandlerList[INotificationHandler::NH_OnSelectProduct].begin(); 
			vecIter != m_mapNotificationHandlerList[INotificationHandler::NH_OnSelectProduct].end(); ++vecIter )
	{
		(*vecIter)->OnRViewSSelSelectProduct();
	}
}

void CRViewSSelSS::OnGetTender()
{
	if( NULL == m_pclProdSelParams || NULL == m_pclProdSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( true == m_strProductRightClicked.IsEmpty() )
	{
		return;
	}

	CTenderDlg Dlg;

	CDB_Product *pclProduct = (CDB_Product*)( m_pclProdSelParams->m_pTADB->Get( m_strProductRightClicked ).MP );

	if( NULL != pclProduct )
	{
		const TCHAR *ptcArticleNumber = pclProduct->GetBodyArtNum();

		_string strTenderText;

		TASApp.GetTender().GetTenderTxt( _string( ptcArticleNumber ), strTenderText );
		Dlg.SetTender( pclProduct->GetName(), _string( ptcArticleNumber ), strTenderText );

		Dlg.DoModal();
	}
}

void CRViewSSelSS::OnUpdateMenuText( CCmdUI *pCmdUI )
{
	// Update the status bar
	pMainFrame->UpdateMenuToolTip( pCmdUI );
}

void CRViewSSelSS::OnUpdateGetTender( CCmdUI *pCmdUI )
{
	if( NULL == m_pclProdSelParams || NULL == m_pclProdSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CDB_Product *pclProduct = (CDB_Product*)( m_pclProdSelParams->m_pTADB->Get( m_strProductRightClicked ).MP );

	if( NULL != pclProduct )
	{
		const TCHAR *ptcArticleNumber = pclProduct->GetBodyArtNum();

		int iTenderID = TASApp.GetTender().GetTenderID( _string( ptcArticleNumber ) );

		if( iTenderID >= 0 )
		{
			pCmdUI->Enable(TRUE);
		}
		else
		{
			pCmdUI->Enable(FALSE);
		}
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Overrides CView.
void CRViewSSelSS::OnActivateView( BOOL bActivate, CView *pActivateView, CView *pDeactiveView )
{
	// Set the focus on the first control.
	// Microsoft documentation: The 'pActivateView' and 'pDeactiveView' parameters point to the same view if the application's main frame window 
	// is activated with no change in the active view — for example, if the focus is being transferred from another application to this one,
	// rather than from one view to another within the application or when switching amongst MDI child windows.
	
	// Here, we don't want to send notification if user goes to other application and comes back on TASelect. In this case, as mentioned in the MFC
	// documentation, 'pActivateView' and 'pDeactiveView' are the same.
	
	if( TRUE == bActivate && pActivateView != pDeactiveView && this == pActivateView )
	{
		// Notify.
		std::vector<INotificationHandler*>::iterator vecIter;

		for( vecIter = m_mapNotificationHandlerList[INotificationHandler::NH_OnLostFocus].begin(); 
				vecIter != m_mapNotificationHandlerList[INotificationHandler::NH_OnLostFocus].end(); ++vecIter )
		{
			(*vecIter)->OnRViewSSelLostFocusWithTabKey( false );
		}
	}

	// HYS-1539 : If the Rview exists and was not activated when doing a _ShowAllPrioritiesArrowClicked,
	// the priority button is not updated. So when this Rview is activated we update it to have the good picture and tooltip.
	if( TRUE == bActivate && this == pActivateView )
	{
		// Check and update the show priority button
		// Run all sheet description.
		CViewDescription::vecSheetDescription vecSheetDescriptionList;
		m_ViewDescription.GetSheetDescriptionList( vecSheetDescriptionList );

		for( int iLoopSheet = 0; iLoopSheet < (int)vecSheetDescriptionList.size(); iLoopSheet++ )
		{
			CSheetDescription *pclSD = vecSheetDescriptionList[iLoopSheet];

			// Retrieve list of all defined Show/Hide all priorities buttons.
			CSheetDescription::vecCellDescription vecCellDescriptionList;
			pclSD->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CDB_ShowAllPriorities );

			// Run all buttons.
			for( int iLoopButton = 0; iLoopButton < (int)vecCellDescriptionList.size(); iLoopButton++ )
			{
				CCellDescriptionButton *pButton = dynamic_cast<CCellDescriptionButton *>( vecCellDescriptionList[iLoopButton] );

				if( NULL == pButton )
				{
					continue;
				}

				// HYS-1539 : If the Rview exists and was not activated when doing a _ShowAllPrioritiesArrowClicked,
				// the priority button is not updated. So when this Rview is activated we update it to have the good picture and tooltip.
				if( ( pButton->GetButtonState() == CCDButtonShowAllPriorities::ButtonState::HidePriorities && false == GetShowAllPrioritiesFlag( pclSD->GetSheetDescriptionID() ) )
						|| ( pButton->GetButtonState() == CCDButtonShowAllPriorities::ButtonState::ShowPriorities && true == GetShowAllPrioritiesFlag( pclSD->GetSheetDescriptionID() ) ) )
				{
					UpdateShowAllPrioritiesButtonState( pclSD, (CCDButtonShowAllPriorities*)pButton, vecCellDescriptionList[iLoopButton] );
				}
			}
		}
	}
}

// End of overrides CView.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Overrides RViewBase.
void CRViewSSelSS::OnCellClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}
	
	BeginWaitCursor();
	
	// Retrieve cell description from 'lClickedCol' and 'lClickedRow'.
	CCellDescription *pclCellDescription = pclSheetDescription->GetCellDescription( lClickedCol, lClickedRow );

	if( NULL != pclCellDescription )
	{
		int iNeedRefresh = RRT_NoRefreshNeeded;
		EnableSSheetDrawing( FALSE );

		// Switch in regards to cell description.
		switch( pclCellDescription->GetCellType() )
		{
			case RVSCellDescription::CDB_Box:
				// TODO!
				iNeedRefresh = _BoxButtonClicked( NULL, NULL );
				break;
					
			case RVSCellDescription::CDB_ShowAllPriorities:
				iNeedRefresh = _ShowAllPrioritiesArrowClicked( pclCellDescription, pclSheetDescription );
				break;

			case RVSCellDescription::CDB_ExpandCollapseRow:
				iNeedRefresh = _ExpandCollapseRowsClicked( pclCellDescription, pclSheetDescription );
				break;

			case RVSCellDescription::CDB_ExpandCollapseGroup:
				iNeedRefresh = _ExpandCollapseGroupClicked( pclCellDescription, pclSheetDescription );
				break;

			case RVSCellDescription::CDBC_Accessories:
				iNeedRefresh = _AccessoryCheckboxClicked( pclCellDescription, pclSheetDescription );
				break;

			case RVSCellDescription::CD_Product:
				{
					CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>( pclCellDescription );

					if( NULL == pCDProduct )
					{
						break;
					}

					// If product is defined (normally it's the case, it's only a sanity check!) ...
					if( pCDProduct->GetProduct() != 0 || pCDProduct->GetUserParam() != 0 )
					{
						// Select the product (call method in inherited class).
						iNeedRefresh = OnClickProduct( pclSheetDescription, pCDProduct, lClickedCol, lClickedRow );
					}
				}
				break;

			case RVSCellDescription::CD_Link:
				{
					CCellDescriptionLink *pCDLink = dynamic_cast<CCellDescriptionLink *>( pclCellDescription );

					if( NULL == pCDLink )
					{
						break;
					}

					OnCellDescriptionLinkClicked( pclSheetDescription, lClickedCol, lClickedRow, pCDLink->GetUserParam() );
				}
				break;
		}

		EnableSSheetDrawing( TRUE );
		
		if( RRT_NoRefreshNeeded != iNeedRefresh )
		{
			// Move different sheets to correct position in the right view.
			if( RRT_RefreshSheets == ( iNeedRefresh & RRT_RefreshSheets ) )
			{
				SetSheetSize();
			}
			
			if( RRT_RefreshPosition == ( iNeedRefresh & RRT_RefreshPosition ) )
			{
				// If a selection exist...
				if( NULL != m_stCurrentFocus.m_pclSheetDescription && NULL != m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer()
					&& FALSE == m_stCurrentFocus.m_rectCurrentFocus.IsRectNull() )
				{
					CRect rectFocusPixel = m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer()->GetSelectionCoordInPixels( m_stCurrentFocus.m_rectCurrentFocus );
					SetScrollPosition( ScrollPosition::ScrollToCompute, m_stCurrentFocus.m_pclSheetDescription, rectFocusPixel );
				}
			}

// 			// Pay attention: 'Invalidate' must be called after 'SetRedraw' because 'Invalidate' has no effect if 'SetRedraw' is set to FALSE.
// 			Invalidate();
// 			// Force a paint now.
// 			UpdateWindow();
		}
	}

	EndWaitCursor();
}

void CRViewSSelSS::OnCellDblClicked( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}
	
	BeginWaitCursor();
	
	// Retrieve cell description from 'lClickedCol' and 'lClickedRow'.
	CCellDescription *pclCellDescription = pclSheetDescription->GetCellDescription( lClickedCol, lClickedRow );

	if( NULL != pclCellDescription )
	{
		int iNeedRefresh = RRT_NoRefreshNeeded;
		EnableSSheetDrawing( FALSE );
	
		// Switch in regards to cell description.
		switch( pclCellDescription->GetCellType() )
		{
			case RVSCellDescription::CDB_TitleGroup:
				iNeedRefresh = _TitleGroupClicked( pclCellDescription, pclSheetDescription );
				break;
		}

		EnableSSheetDrawing( TRUE );
		
		if( RRT_NoRefreshNeeded != iNeedRefresh )
		{
			// Move different sheets to correct position in the right view.
			if( RRT_RefreshSheets == ( iNeedRefresh & RRT_RefreshSheets ) ) 
			{
				SetSheetSize();
			}

			// If a selection exist...
			if( RRT_RefreshPosition == ( iNeedRefresh & RRT_RefreshPosition ) ) 
			{
				if( NULL != m_stCurrentFocus.m_pclSheetDescription && NULL != m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer()
						&& FALSE == m_stCurrentFocus.m_rectCurrentFocus.IsRectNull() )
				{
					CRect rectFocusPixel = m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer()->GetSelectionCoordInPixels( m_stCurrentFocus.m_rectCurrentFocus );
					SetScrollPosition( ScrollPosition::ScrollToCompute, m_stCurrentFocus.m_pclSheetDescription, rectFocusPixel );
				}
			}

			// Pay attention: 'Invalidate' must be called after 'SetRedraw' because 'Invalidate' has no effect if 'SetRedraw' is set to FALSE.
			Invalidate();
			// Force a paint now.
			UpdateWindow();
		}
	}

	EndWaitCursor();
}

void CRViewSSelSS::OnRightClick( CSheetDescription *pclSheetDescription, long lClickedCol, long lClickedRow, long lXPos, long lYPos )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	// HYS-1412 : If we edit a product from batch selection the right click does not show the menu
	if( m_pclProdSelParams->m_eProductSelectionMode == ProductSelectionMode::ProductSelectionMode_Batch )
	{
		return;
	}

	// Try to retrieve product in the cell clicked.
	CCellDescription *pclCellDescriptionClicked = pclSheetDescription->GetCellDescription( lClickedCol, lClickedRow );
	
	if( NULL == pclCellDescriptionClicked )
	{
		// If no product on the cell, try to find parameter on the row.
		long lParamCol = pclSheetDescription->GetFirstParameterColumn();
		
		if( lParamCol != - 1 )
		{
			pclCellDescriptionClicked = pclSheetDescription->GetCellDescription( lParamCol, lClickedRow );
		}
	}

	// If no parameter...
	if( NULL == pclCellDescriptionClicked )
	{
		return;
	}

	// Add dynamic popup menu to select the product or get to the quick ref sheet or full cat sheet.
	CMenu menu;
	menu.LoadMenu( IDR_FLTMENU_SSEL );
	CMenu *pContextMenu = menu.GetSubMenu( 0 );

	CString str = pMainFrame->GetSplitStringFromResID( ID_SSELAFLT_SELECT );
	pContextMenu->ModifyMenu( ID_SSELAFLT_SELECT, MF_BYCOMMAND, ID_SSELAFLT_SELECT, str );		
	str = pMainFrame->GetSplitStringFromResID( ID_SSELAFLT_GETFULLINFO );
	pContextMenu->ModifyMenu( ID_SSELAFLT_GETFULLINFO, MF_BYCOMMAND, ID_SSELAFLT_GETFULLINFO, str );
	str = pMainFrame->GetSplitStringFromResID(ID_SSELAFLT_GETTENDER);
	pContextMenu->ModifyMenu(ID_SSELAFLT_GETTENDER, MF_BYCOMMAND, ID_SSELAFLT_GETTENDER, str);

	pMainFrame->DisableMenuResID( ID_SSELAFLT_SELECT );
	pMainFrame->DisableMenuResID( ID_SSELAFLT_GETFULLINFO );
	pMainFrame->DisableMenuResID( ID_SSELAFLT_GETTENDER );

	// Check if we can enable 'select' option in the menu.

	// If a current selection exist...
	if( true == IsSelectionReady() )
	{
		// Retrieve current selection.
		LPARAM lParam = (LPARAM)0;

		if( true == pclSheetDescription->GetUserVariable( _SDUV_SELECTEDPRODUCT, lParam ) && ( (LPARAM)0 != lParam ) )
		{
			CCellDescription *pclParamDescription = (CCellDescription*)lParam;

			// If right click happens on the same row as the selected parameter...
			if( lClickedRow == pclParamDescription->GetCellPosition().y )
			{
				pMainFrame->EnableMenuResID( ID_SSELAFLT_SELECT );
			}
		}
	}

	// Check if we can enable 'full catalog sheet' option in the menu.

	// If we are well on a product...
	if( RVSCellDescription::CD_Product == pclCellDescriptionClicked->GetCellType() )
	{
		CCellDescriptionProduct *pclCellProduct = dynamic_cast<CCellDescriptionProduct*>( pclCellDescriptionClicked );

		if( NULL != pclCellProduct )
		{
			CDB_Product *pclProduct = NULL;
			CDB_Actuator *pclActuator = NULL;

			if( NULL != pclCellProduct->GetProduct() )
			{
				pclProduct = dynamic_cast<CDB_Product*>( (CData*)pclCellProduct->GetProduct() );
				pclActuator = dynamic_cast<CDB_Actuator*>( (CData*)pclCellProduct->GetProduct() );
			}
			else
			{
				pclProduct = RetrieveProductFromUserParam( pclCellProduct->GetUserParam() );
			}

			if( NULL != pclProduct )
			{
				// For this two items, we need to memorize current product under mouse cursor because ON_COMMAND has no message.
				m_strProductRightClicked = pclProduct->GetIDPtr().ID;

				if( true == pclProduct->VerifyOneCatExist() )
				{
					pMainFrame->EnableMenuResID( ID_SSELAFLT_GETFULLINFO );
				}

				pMainFrame->EnableMenuResID(ID_SSELAFLT_GETTENDER);
			}
			else if( NULL != pclActuator )
			{
				m_strProductRightClicked = pclActuator->GetIDPtr().ID;

				if( true == pclActuator->VerifyOneCatExist() )
				{
					pMainFrame->EnableMenuResID( ID_SSELAFLT_GETFULLINFO );
				}

				pMainFrame->EnableMenuResID(ID_SSELAFLT_GETTENDER);
			}
		}
	}

	if (false == TASApp.GetTender().IsTenderNeeded())
	{
		pContextMenu->RemoveMenu(ID_SSELAFLT_GETTENDER, MF_BYCOMMAND);
	}
				
	// Show the popup menu.
	CPoint ptScreenCoord( lXPos, lYPos ); 
	pclSheetDescription->GetSSheetPointer()->ClientToScreen( &ptScreenCoord );
	TASApp.GetContextMenuManager()->ShowPopupMenu( HMENU(*pContextMenu), ptScreenCoord.x, ptScreenCoord.y, this, TRUE );
}

BOOL CRViewSSelSS::OnKeyDown( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed )
{
	if( NULL == m_stCurrentFocus.m_pclSheetDescription || NULL == m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer() )
	{
		return FALSE;
	}

	CSSheet *pclSSheet = m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer();

	// Do no dispatch message because it has been eaten by one of next cases.
	BOOL bReturn = TRUE;

	switch( wKeyPressed )
	{
		case VK_SPACE:
			// Simulate a click on the current cell that has the focus.
			if( NULL != m_stCurrentFocus.m_pclSheetDescription )
			{
				OnCellClicked( m_stCurrentFocus.m_pclSheetDescription, m_stCurrentFocus.m_rectCurrentFocus.left, m_stCurrentFocus.m_rectCurrentFocus.top );
			}
			break;

		case VK_UP:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_TAB:
			_CellFocused( m_stCurrentFocus.m_pclSheetDescription, wKeyPressed );
			break;

		default:
			// Key is not managed here, thus we can dispatch message.
			bReturn = TRUE;
			break;
	}

	std::vector<INotificationHandler*>::iterator vecIter;
	
	for( vecIter = m_mapNotificationHandlerList[INotificationHandler::NH_OnKeyboardVirtualKey].begin(); 
			vecIter != m_mapNotificationHandlerList[INotificationHandler::NH_OnKeyboardVirtualKey].end(); ++vecIter )
	{
		(*vecIter)->OnRViewSSelKeyboardEvent( (int)wKeyPressed );
	}

	return bReturn;
}

bool CRViewSSelSS::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
		TCHAR *pstrTipText, BOOL *pbShowTip )
{
	if( false == m_bInitialised )
	{
		return false;
	}

	CString str;
	bool bReturnValue = false;
	*pbShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;

	// Retrieve cell description from 'lColumn' and 'lRow'.
	CCellDescription *pclCellDescription = pclSheetDescription->GetCellDescription( lColumn, lRow );

	if( NULL != pclCellDescription )
	{
		// Switch in regards to cell description.
		// HYS-1385: if a least one bit is activated under the mask, we check the exact value after.
		if( 0 != ( pclCellDescription->GetCellType() & RVSCellDescription::CD_ButtonMask ) )
		{
			CCellDescriptionButton *pclCDButton = dynamic_cast<CCellDescriptionButton *>( pclCellDescription );
			
			if( NULL != pclCDButton )
			{
				switch( pclCDButton->GetCellType() )
				{
					case RVSCellDescription::CDB_Box:
						// Nothing for the moment!
						break;

					case RVSCellDescription::CDB_ShowAllPriorities:
						// Retrieve button state
						str = TASApp.LoadLocalizedString( ( true == GetShowAllPrioritiesFlag( pclSheetDescription->GetSheetDescriptionID() ) ) ? IDS_SSHEETSSEL_HIDEPRIORITY : IDS_SSHEETSSEL_SHOWPRIORITY );
						break;

					case RVSCellDescription::CDB_ExpandCollapseRow:
						{
							CCDButtonExpandCollapseRows *pclCDBExpandCollapseRows = dynamic_cast<CCDButtonExpandCollapseRows *>( pclCDButton );
							
							if( NULL != pclCDBExpandCollapseRows )
							{
								int iState = pclCDBExpandCollapseRows->GetButtonState();
								str = TASApp.LoadLocalizedString( ( CCDButtonExpandCollapseRows::ButtonState::CollapseRow == iState ) ? IDS_SSHEETSSEL_COLLAPSEROWS : IDS_SSHEETSSEL_EXPANDROWS );
							}
						}
						break;

					case RVSCellDescription::CDB_Checkbox:
						str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_PARTOFPACK );
						break;

					case RVSCellDescription::CDBC_Accessories:
						{
							CCDBCheckboxAccessory *pclCDBCheckbox = dynamic_cast<CCDBCheckboxAccessory *>( pclCDButton );
							
							if( pclCDBCheckbox != NULL )
							{
								str = pclCDBCheckbox->GetToolTip();
							}
						}
						break;
				}
			}
		}
	}

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = (SHORT)pclSheetDescription->GetSSheetPointer()->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pbShowTip = true;
		bReturnValue = true;
	}

	return bReturnValue;
}

bool CRViewSSelSS::GetCurrentFocus( CSheetDescription *&pclSheetDescription, CRect &rectFocus )
{
	if( NULL == m_stCurrentFocus.m_pclSheetDescription || NULL == m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer() )
	{
		return false;
	}

	pclSheetDescription = m_stCurrentFocus.m_pclSheetDescription;
	rectFocus = m_stCurrentFocus.m_rectCurrentFocus;

	return true;
}
// End of overrides RViewBase.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage CCellDescriptionProduct
CCellDescriptionProduct *CRViewSSelSS::CreateCellDescriptionProduct( long lColumn, long lRow, LPARAM lProduct, CSheetDescription *pclSSheetDescription )
{
	CCellDescriptionProduct *pCellDescriptionProduct = NULL;
	
	if( NULL != pclSSheetDescription && NULL != pclSSheetDescription->GetSSheetPointer() )
	{
		// Create cell description product.
		pCellDescriptionProduct = new CCellDescriptionProduct( CPoint( lColumn, lRow ), pclSSheetDescription, lProduct );

		if( NULL != pCellDescriptionProduct )
		{
			// Add cell description in cell description list.
			pclSSheetDescription->AddCellDescription( pCellDescriptionProduct );
		}
	}

	return pCellDescriptionProduct;
}

LPARAM CRViewSSelSS::GetCDProduct( long lColumn, long lRow, CSheetDescription *pclSheetDescription, CCellDescriptionProduct **ppclCDProduct )
{
	LPARAM lReturn = 0;
	
	// Check if we have a cell description.
	CCellDescription *pclCellDescription = pclSheetDescription->GetCellDescription( lColumn, lRow );
	
	if( NULL != pclCellDescription )
	{
		// Check if it's well a cell description product type.
		if( RVSCellDescription::CD_Product == pclCellDescription->GetCellType() )
		{
			// Try to retrieve cell description product.
			CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>( pclCellDescription );
			
			if( NULL != pCDProduct )
			{
				lReturn = pCDProduct->GetProduct();

				if( NULL != ppclCDProduct )
				{
					*ppclCDProduct = pCDProduct;
				}
			}
		}
		else if( RVSCellDescription::CDBC_Accessories == pclCellDescription->GetCellType() )
		{
			// Try to retrieve cell description accessory.
			CCDBCheckboxAccessory *pCDAccessory = dynamic_cast<CCDBCheckboxAccessory *>( pclCellDescription );

			if( NULL != pCDAccessory )
			{
				lReturn = pCDAccessory->GetAccessoryLParam();
			}
		}
	}

	return lReturn;
}

CCellDescriptionProduct *CRViewSSelSS::FindCDProduct( long lRow, LPARAM lpProduct, CSheetDescription *pclSheetDescription )
{
	if( 0 == lpProduct || NULL == pclSheetDescription )
	{
		return NULL;
	}

	CCellDescriptionProduct *pclCDProduct = NULL;
	CCellDescription *pclLoopCellDescription = pclSheetDescription->GetFirstCellDescription( RVSCellDescription::CD_Product );

	while( NULL != pclLoopCellDescription )
	{
		// Try to retrieve cell description product.
		pclCDProduct = dynamic_cast<CCellDescriptionProduct *>( pclLoopCellDescription );
		
		if( NULL == pclCDProduct )
		{
			continue;
		}

		if( ( -1 == lRow || lRow == pclCDProduct->GetCellPosition().y ) && lpProduct == pclCDProduct->GetProduct() )
		{
			break;
		}
		else
		{
			pclCDProduct = NULL;
			pclLoopCellDescription = pclSheetDescription->GetNextCellDescription( pclLoopCellDescription, RVSCellDescription::CD_Product );
		}
	}

	return pclCDProduct;
}

CCellDescriptionProduct *CRViewSSelSS::FindCDProductByUserParam( long lRow, LPARAM lpUserParam, CSheetDescription *pclSheetDescription )
{
	if( 0 == lpUserParam || NULL == pclSheetDescription )
	{
		return NULL;
	}

	CCellDescriptionProduct *pclCDProduct = NULL;
	CCellDescription *pclLoopCellDescription = pclSheetDescription->GetFirstCellDescription( RVSCellDescription::CD_Product );

	while( NULL != pclLoopCellDescription )
	{
		// Try to retrieve cell description product.
		pclCDProduct = dynamic_cast<CCellDescriptionProduct *>( pclLoopCellDescription );
		
		if( NULL != pclCDProduct )
		{
			if( ( -1 == lRow || lRow == pclCDProduct->GetCellPosition().y ) && lpUserParam == pclCDProduct->GetUserParam() )
			{
				break;
			}
		}
		
		pclLoopCellDescription = pclSheetDescription->GetNextCellDescription( pclLoopCellDescription, RVSCellDescription::CD_Product );
	}

	return pclCDProduct;
}

// End of methods to manage CCellDescriptionProduct
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage box button.
CCDButtonBox *CRViewSSelSS::CreateCellDescriptionBox( long lColumn, long lRow, bool fEnabled, CCDButtonBox::ButtonState eButtonState, 
		CSheetDescription *pclSSheetDescription )
{
	CCDButtonBox *pButtonBox = NULL;

	if( NULL != pclSSheetDescription && NULL != pclSSheetDescription->GetSSheetPointer() )
	{
		// Create button box.
		pButtonBox = new CCDButtonBox( CPoint( lColumn, lRow ), pclSSheetDescription, true, eButtonState );
		
		if( NULL != pButtonBox )
		{
			// Add button in cell description list.
			pclSSheetDescription->AddCellDescription( pButtonBox );
		}
	}

	return pButtonBox;
}

CCDButtonBox *CRViewSSelSS::GetCDButtonBox( long lColumn, long lRow, CSheetDescription *pclSheetDescription )
{
	CCDButtonBox *pCDReturn = NULL;
	
	// Check if we have a cell description.
	CCellDescription *pclCellDescription = pclSheetDescription->GetCellDescription( lColumn, lRow );
	
	if( NULL != pclCellDescription )
	{
		// Check if it's well a cell description product type.
		if( RVSCellDescription::CDB_Box == pclCellDescription->GetCellType() )
		{
			// Try to retrieve cell description product.
			pCDReturn = dynamic_cast<CCDButtonBox *>( pclCellDescription );
		}
	}

	return pCDReturn;
}
// End of methods to manage box button.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage Show/Hide all priorities button.
CCDButtonShowAllPriorities *CRViewSSelSS::CreateShowAllPrioritiesButton( long lColumn, long lRow, bool bEnabled, 
		CCDButtonShowAllPriorities::ButtonState eButtonState, long lFromRow, long lToRow, CSheetDescription *pclSSheetDescription )
{
	CCDButtonShowAllPriorities *pShowAllPriorities = NULL;

	if( NULL != pclSSheetDescription && NULL != pclSSheetDescription->GetSSheetPointer() )
	{
		// Create Show/Hide all priorities button.
		pShowAllPriorities = new CCDButtonShowAllPriorities( CPoint( lColumn, lRow ), pclSSheetDescription, true, eButtonState, 
				lFromRow, lToRow );

		if( NULL != pShowAllPriorities )
		{
			// Add button in cell description list.
			pclSSheetDescription->AddCellDescription( pShowAllPriorities );
		}
	}

	return pShowAllPriorities;
}

long CRViewSSelSS::GetShowAllPrioritiesButtonRow( CSheetDescription *pclSSheetDescription, CCDButtonShowAllPriorities **pclButton )
{
	if( NULL == pclSSheetDescription )
	{
		return -1;
	}

	CCDButtonShowAllPriorities *pButton = dynamic_cast<CCDButtonShowAllPriorities *>( pclSSheetDescription->GetFirstCellDescription( RVSCellDescription::CDB_ShowAllPriorities ) );

	if( NULL == pButton )
	{
		return -1;
	}

	if( NULL != pclButton )
	{
		*pclButton = pButton;
	}

	return pButton->GetCellPosition().y;
}

void CRViewSSelSS::CheckShowAllPrioritiesButtonState( CSheetDescription *pclSheetDescription, long lRowSelected )
{
	if( NULL == pclSheetDescription || lRowSelected <= 0 )
	{
		return;
	}

	CCDButtonShowAllPriorities *pclButton = NULL;
	long lShowAllPrioritiesButtonRow = GetShowAllPrioritiesButtonRow( pclSheetDescription, &pclButton );

	if( NULL != pclButton && -1 != lShowAllPrioritiesButtonRow && lRowSelected > lShowAllPrioritiesButtonRow )
	{
		if( CCDButtonShowAllPriorities::ShowPriorities == pclButton->GetButtonState() )
		{
			pclButton->ToggleButtonState( true );
		}
	}
}

// End of methods to manage Show/Hide all priorities button.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage Expand/Collapse rows button.
CCDButtonExpandCollapseRows* CRViewSSelSS::CreateExpandCollapseRowsButton( long lColumn, long lRow, bool bEnabled, 
		CCDButtonExpandCollapseRows::ButtonState eButtonState, long lFromRow, long lToRow, CSheetDescription *pclSSheetDescription )
{
	CCDButtonExpandCollapseRows *pExpandCollapseRows = NULL;

	if( NULL != pclSSheetDescription && NULL != pclSSheetDescription->GetSSheetPointer() )
	{
		// Create Expand/Collapse rows button.
		pExpandCollapseRows = new CCDButtonExpandCollapseRows( CPoint( lColumn, lRow ), pclSSheetDescription, true, 
				eButtonState, lFromRow, lToRow );

		if( NULL != pExpandCollapseRows )
		{
			// Add button in cell description list and in cell param.
			pclSSheetDescription->AddCellDescription( pExpandCollapseRows );
		}
	}

	return pExpandCollapseRows;
}

void CRViewSSelSS::DeleteExpandCollapseRowsButton( CCDButtonExpandCollapseRows *&pCDBExpandCollapseRows, CSheetDescription *pclSSheetDescription )
{
	if( NULL == pCDBExpandCollapseRows || NULL == pclSSheetDescription )
	{
		return;
	}

	// First clean properly the button.
	// Remark: this method will check if rows must be shown before the button will be deleted.
	pCDBExpandCollapseRows->DeleteButton();

	// Remove button from cell description list linked to sheet description.
	pclSSheetDescription->RemoveCellDescription( pCDBExpandCollapseRows );

	// Reset pointer
	pCDBExpandCollapseRows = NULL;
}
// End of methods to manage Expand/Collapse rows button.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage Expand/Collapse group button.
CCDButtonExpandCollapseGroup* CRViewSSelSS::CreateExpandCollapseGroupButton( long lColumn, long lRow, bool bEnabled, 
		CCDButtonExpandCollapseGroup::ButtonState eButtonState, long lFromRow, long lToRow, CSheetDescription *pclSSheetDescription )
{
	CCDButtonExpandCollapseGroup *pExpandCollapseGroup = NULL;

	if( NULL != pclSSheetDescription && NULL != pclSSheetDescription->GetSSheetPointer() )
	{
		// Create Expand/Collapse group button.
		pExpandCollapseGroup = new CCDButtonExpandCollapseGroup( CPoint( lColumn, lRow ), pclSSheetDescription, true, 
				eButtonState, lFromRow, lToRow );

		if( NULL != pExpandCollapseGroup )
		{
			// Add button in cell description list and in cell param.
			pclSSheetDescription->AddCellDescription( pExpandCollapseGroup );
		}
	}

	return pExpandCollapseGroup;
}

void CRViewSSelSS::DeleteExpandCollapseGroupButton( CCDButtonExpandCollapseGroup *&pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription )
{
	if( NULL == pCDBExpandCollapseGroup || NULL == pclSSheetDescription )
	{
		return;
	}

	// First clean properly the button.
	// Remark: this method will check if rows must be shown before the button will be deleted.
	pCDBExpandCollapseGroup->DeleteButton();

	// Remove button from cell description list linked to sheet description.
	pclSSheetDescription->RemoveCellDescription( pCDBExpandCollapseGroup );

	// Reset pointer
	pCDBExpandCollapseGroup = NULL;
}
// End of methods to manage Expand/Collapse group button.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage accessories
CCDBCheckboxAccessory* CRViewSSelSS::CreateCheckboxAccessory( long lColumn, long lRow, bool bChecked, bool bEnabled, CString strText, 
	CDB_Product *pAccessory, CDB_RuledTableBase *pRuledTable, vecCDCAccessoryList *pvecAccessoryList, CSheetDescription *pclSheetDescription )
{
	CCDBCheckboxAccessory* pCheckboxAccessory = NULL;
	
	if( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
	{
		// Create checkbox.
		pCheckboxAccessory = new CCDBCheckboxAccessory( CPoint( lColumn, lRow ), pclSheetDescription, true, bChecked, bEnabled, 
				strText, pAccessory, pRuledTable );
		
		if( NULL != pCheckboxAccessory )
		{
			// Add checkbox in cell description list.
			pclSheetDescription->AddCellDescription( pCheckboxAccessory );
			
			// Add checkbox in accessories list.
			pvecAccessoryList->push_back( pCheckboxAccessory );
		}
	}
	
	return pCheckboxAccessory;
}

void CRViewSSelSS::VerifyCheckboxAccessories( CDB_Product *pSelectedAccessory, bool bChecked, vecCDCAccessoryList *pvecAccessoryList, bool bForceChange )
{
	if( NULL == m_pclProdSelParams || NULL == m_pclProdSelParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pvecAccessoryList )
	{
		return;
	}

	std::vector<CData *> Set;
	
	// It's impossible to save list of pointer on 'CCDBCheckboxAccessory' because some variables in it will be changed in code below.
	// We need to keep CDB_Accessory, check and enable status.
	struct AccessorySaved
	{
		CDB_Product *pAccessory;
		bool fCheckStatus;
		bool fEnableStatus;
	};

	std::vector<AccessorySaved*> vecAccessoryListCopy;
	
	for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
	{
		CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;
		
		if( NULL != pCDBCheckboxAccessory )
		{
			AccessorySaved *pAccessorySaved = new AccessorySaved;
			
			if( NULL != pAccessorySaved )
			{
				pAccessorySaved->pAccessory = pCDBCheckboxAccessory->GetAccessoryPointer();
				pAccessorySaved->fCheckStatus = pCDBCheckboxAccessory->GetCheckStatus();
				pAccessorySaved->fEnableStatus = pCDBCheckboxAccessory->GetEnableStatus();
				vecAccessoryListCopy.push_back( pAccessorySaved );
			}
		}
	}

	// Update accessories map to follow user action.
	CDB_RuledTableBase *pFamRtab = (CDB_RuledTableBase *)( m_pclProdSelParams->m_pTADB->Get( _T("ACCSERIESRULES") ).MP );
	VERIFY( NULL != pFamRtab );

	CDB_RuledTableBase *pRuledTable = NULL;
	
	if( NULL != pSelectedAccessory && true == pSelectedAccessory->IsAnAccessory() )
	{
		CCDBCheckboxAccessory *pSelectedCheckboxAccessory = _IsAccessoryExistInList( pSelectedAccessory, pvecAccessoryList );

		// If accessory is found...
		if( NULL != pSelectedCheckboxAccessory && NULL != pSelectedCheckboxAccessory->GetAccessoryPointer() )
		{
			// If status is disabled we cannot modify it...
			if( true == pSelectedCheckboxAccessory->GetEnableStatus() || true == bForceChange )
			{
				pSelectedCheckboxAccessory->SetCheckStatus( bChecked );

				// Reset flag and reset Implicated
				for( int iState = 0; iState < 2; iState++ )
				{
					switch( iState )
					{
						// We verify first with the accessory family rules (series that exclude each other).
						case 0:	
							pRuledTable = pFamRtab;
							break;
						
						// We verify second with the accessory rules itself.
						case 1:
							pRuledTable = pSelectedCheckboxAccessory->GetRuledTable();
							break;
					}
					
					// Exclusion.
					// Remark: - In the first step we check the series. This is why we pass as argument the 'CDS_Accessory::GetSeriesIDPtr'.
					//           'GetExcluded' will send back all family that are excluded by the family of the selected accessory.
					//         - In the second step we check accessory itself.
					//           'GetExcluded' will send back all accessories that are excluded by the selected accessory.
					// 'Excluded' means that an accessory CAN'T be selected in the same time with a specific other accessory.
					int nCount = pRuledTable->GetExcluded( ( 0 == iState ) ? pSelectedAccessory->GetSeriesIDPtr().MP : pSelectedAccessory, &Set );
					
					if( nCount > 0 )
					{
						// Run all items sent back by 'GetExcluded' (series or accessories).
						for( int i = 0; i < nCount; i++ )
						{
							// Check all accessories in the list.
							for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
							{
								CCDBCheckboxAccessory *pCheckboxAccessoryInList = (*vecIter);

								if( NULL == pCheckboxAccessoryInList )
								{
									continue;
								}

								CDB_Product *pAccessoryInList = pCheckboxAccessoryInList->GetAccessoryPointer();

								if( NULL == pAccessoryInList || false == pAccessoryInList->IsAnAccessory() )
								{
									continue;
								}

								// Remark: - In the first step we check the series. This is why we pass as argument the 'CDS_Accessory::GetSeriesIDPtr'.
								//           If displayed accessory has its family excluded by the selected family accessory, we uncheck it.
								//         - in the second step we check accessory itself.
								//           If displayed accessory is excluded by the selected accessory, we uncheck it.
								CData *pData = ( 0 == iState ) ? pAccessoryInList->GetSeriesIDPtr().MP : pAccessoryInList;
								
								if( NULL != pData && pData == Set.at( i ) )
								{
									if( true == pCheckboxAccessoryInList->GetCheckStatus() )
									{
										// HYS-558: Verify accessories linked and clear status
										VerifyCheckboxAccessories( pAccessoryInList, false, pvecAccessoryList, true );
									}

									pCheckboxAccessoryInList->SetCheckStatus( false );
									pCheckboxAccessoryInList->SetEnableStatus( !bChecked );
								}
							}
						}
					}

					// Implication.
					// Remark: - In the first step we check the series. This is why we pass as argument the 'CDS_Accessory::GetSeriesIDPtr'.
					//           'GetImplicate' will send back all family that are linked with the family of the selected accessory.
					//         - In the second step we check accessory itself.
					//           'GetImplicate' will send back all accessories that are linked with the selected accessory.
					// 'Implicated' means that an accessory MUST be selected in the same time with a specific other accessory.
					nCount = pRuledTable->GetImplicate( ( 0 == iState ) ? pSelectedAccessory->GetSeriesIDPtr().MP : pSelectedAccessory, &Set );
					
					if( nCount )
					{
						// Run all items sent back by 'GetImplicate' (series or accessories).
						for( int i = 0; i < nCount; i++ )
						{
							// Check all accessories in the list.
							for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
							{
								CCDBCheckboxAccessory *pCheckboxAccessoryInList = (*vecIter);

								if( NULL == pCheckboxAccessoryInList )
								{
									continue;
								}
								
								CDB_Product *pAccessoryInList = pCheckboxAccessoryInList->GetAccessoryPointer();

								if( NULL == pAccessoryInList || false == pAccessoryInList->IsAnAccessory() )
								{
									continue;
								}

								// Remark: - In the first step we check the series. This is why we pass as argument the 'CDS_Accessory::GetSeriesIDPtr'.
								//           If displayed accessory has its family linked with the selected family accessory, we check it.
								//         - in the second step we check accessory itself.
								//           If displayed accessory is linked with the selected accessory, we check it.
								CData *pData = ( 0 == iState ) ? pAccessoryInList->GetSeriesIDPtr().MP : pAccessoryInList;

								if( NULL != pData && pData == Set.at( i ) )
								{
									pCheckboxAccessoryInList->SetCheckStatus( bChecked );
									pCheckboxAccessoryInList->SetEnableStatus( !bChecked );
								}
							}
						}
					}
				}
			}
		}
	}

	// Verify each accessory in the list is there are some one Built-In.
	for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
	{
		CCDBCheckboxAccessory *pCheckboxAccessoryInList = (*vecIter);

		if( NULL == pCheckboxAccessoryInList )
		{
			continue;
		}

		// Retrieve RuledTable.
		CDB_RuledTableBase *pRTab = pCheckboxAccessoryInList->GetRuledTable();

		if( NULL == pRTab )
		{
			continue;
		}

		int nCount = pRTab->GetBuiltIn( &Set );

		if( nCount > 0 )
		{
			for( int i = 0; i < nCount; i++ )
			{
				CCDBCheckboxAccessory *pCheckboxAccessoryBuiltIn = _IsAccessoryExistInList( (CDB_Product *)Set.at( i ), pvecAccessoryList );
				
				if( pCheckboxAccessoryBuiltIn != NULL )
				{
					// 'Built-in' accessory means that it can't be sold separately. Thus we force checkbox to 'true' and we disable the checkbox.
					pCheckboxAccessoryBuiltIn->SetCheckStatus( true );
					pCheckboxAccessoryBuiltIn->SetEnableStatus( false );
				}
			}
		}
	}	

	// verify Implication/Exclusion for Accessory family and for Accessory group.
	for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
	{
		CCDBCheckboxAccessory *pCheckboxAccessoryInList = (*vecIter);
		
		if( NULL == pCheckboxAccessoryInList )
		{
			continue;
		}
		
		CDB_Product *pAccessoryInList = pCheckboxAccessoryInList->GetAccessoryPointer();

		if( NULL == pAccessoryInList || false == pAccessoryInList->IsAnAccessory() )
		{
			continue;
		}

		if( true == pCheckboxAccessoryInList->GetCheckStatus() )
		{
			for( int iState = 0; iState < 2; iState++ )
			{
				switch( iState )
				{
					// We verify first with the accessory family rules (series that exclude each other).
					case 0:
						pRuledTable = pFamRtab;
						break;
					
					// We verify second with the accessory rules itself.
					case 1:
						pRuledTable = pCheckboxAccessoryInList->GetRuledTable();
						break;
				}

				// Exclusion.
				// Remark: - In the first step we check the series. This is why we pass as argument the 'CDS_Accessory::GetSeriesIDPtr'.
				//           'GetExcluded' will send back all family that are excluded by the family of the selected accessory.
				//         - In the second step we check accessory itself.
				//           'GetExcluded' will send back all accessories that are excluded by the selected accessory.
				// 'Excluded' means that an accessory CAN'T be selected in the same time with a specific other accessory.
				int nCount = pRuledTable->GetExcluded( ( 0 == iState ) ? pAccessoryInList->GetSeriesIDPtr().MP : pAccessoryInList, &Set );
				
				if( nCount > 0 )
				{
					// Run all items sent back by 'GetExcluded' (series or accessories).
					for( int i = 0; i < nCount; i++ )
					{
						// Check all accessories in the list.
						for( vecCDCAccessoryListIter vecIter2 = pvecAccessoryList->begin(); vecIter2 != pvecAccessoryList->end(); ++vecIter2 )
						{
							CCDBCheckboxAccessory *pCheckboxAccessoryInList2 = (*vecIter2);
							
							if( NULL == pCheckboxAccessoryInList2 )
							{
								continue;
							}
							
							CDB_Product *pAccessoryInList2 = pCheckboxAccessoryInList2->GetAccessoryPointer();

							if( NULL == pAccessoryInList2 || false == pAccessoryInList2->IsAnAccessory() )
							{
								continue;
							}

							// Remark: - In the first step we check the series. This is why we pass as argument the 'CDS_Accessory::GetSeriesIDPtr'.
							//           If displayed accessory has its family excluded by the selected family accessory, we uncheck it.
							//         - in the second step we check accessory itself.
							//           If displayed accessory is excluded by the selected accessory, we uncheck it.
							CData *pData = ( 0 == iState ) ? pAccessoryInList2->GetSeriesIDPtr().MP : pAccessoryInList2;

							if( NULL != pData && pData == Set.at( i ) )
							{
								pCheckboxAccessoryInList2->SetCheckStatus( false );
								pCheckboxAccessoryInList2->SetEnableStatus( false );
							}
						}
					}
				}

				// Implication.
				// Remark: - In the first step we check the series. This is why we pass as argument the 'CDS_Accessory::GetSeriesIDPtr'.
				//           'GetImplicate' will send back all family that are linked with the family of the selected accessory.
				//         - In the second step we check accessory itself.
				//           'GetImplicate' will send back all accessories that are linked with the selected accessory.
				// 'Implicated' means that an accessory MUST be selected in the same time with a specific other accessory.
				nCount = pRuledTable->GetImplicate( ( 0 == iState ) ? pAccessoryInList->GetSeriesIDPtr().MP : pAccessoryInList, &Set );
				
				if( nCount > 0 )
				{
					// Run all items sent back by 'GetImplicate' (series or accessories).
					for( int i = 0; i < nCount; i++ )
					{
						// Check all accessories in the list.
						for( vecCDCAccessoryListIter vecIter2 = pvecAccessoryList->begin(); vecIter2 != pvecAccessoryList->end(); ++vecIter2 )
						{
							CCDBCheckboxAccessory *pCheckboxAccessoryInList2 = (*vecIter2);
							
							if( NULL == pCheckboxAccessoryInList2 )
							{
								continue;
							}
							
							CDB_Product *pAccessoryInList2 = pCheckboxAccessoryInList2->GetAccessoryPointer();

							if( NULL == pAccessoryInList2 || false == pAccessoryInList2->IsAnAccessory() )
							{
								continue;
							}

							// Remark: - In the first step we check the series. This is why we pass as argument the 'CDS_Accessory::GetSeriesIDPtr'.
							//           If displayed accessory has its family linked with the selected family accessory, we check it.
							//         - in the second step we check accessory itself.
							//           If displayed accessory is linked with the selected accessory, we check it.
							CData *pData = ( 0 == iState ) ? pAccessoryInList2->GetSeriesIDPtr().MP : pAccessoryInList2;
							
							if( NULL != pData && pData == Set.at( i ) )
							{
								pCheckboxAccessoryInList2->SetCheckStatus( true );
								pCheckboxAccessoryInList2->SetEnableStatus( false );
							}
						}
					}
				}
			}
		}
	}

	// Reset displayed accessory state.
	for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
	{
		CCDBCheckboxAccessory *pclCheckboxAccessoryInList = (*vecIter);
		
		if( NULL == pclCheckboxAccessoryInList )
		{
			continue;
		}

		// Search in list saved.
		std::vector<AccessorySaved*>::iterator vecIter2 = vecAccessoryListCopy.begin();
		
		for( ; vecIter2 != vecAccessoryListCopy.end(); ++vecIter2 )
		{
			if( (*vecIter2)->pAccessory == pclCheckboxAccessoryInList->GetAccessoryPointer() )
			{
				break;
			}
		}
		
		if( vecIter2 != vecAccessoryListCopy.end() )
		{
			// Do nothing if no change!
			if( (*vecIter2)->fCheckStatus == pclCheckboxAccessoryInList->GetCheckStatus() && 
				(*vecIter2)->fEnableStatus == pclCheckboxAccessoryInList->GetEnableStatus() )
			{
				continue;
			}
		}
		pclCheckboxAccessoryInList->ApplyInternalChange();
	}

	// Clear saved list.
	std::vector<AccessorySaved*>::iterator vecIter2 = vecAccessoryListCopy.begin();

	for( ; vecIter2 != vecAccessoryListCopy.end(); ++vecIter2 )
	{
		delete (*vecIter2);
	}
}

void CRViewSSelSS::DisableCheckboxAccessories( LPCTSTR lpTabID, vecCDCAccessoryList *pvecAccessoryList )
{
	if( NULL == pvecAccessoryList )
	{
		return;
	}

	// Reset displayed accessory state.
	CString strOwnerTabID( lpTabID );

	for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
	{
		CCDBCheckboxAccessory *pCheckboxAccessoryInList = (*vecIter);

		if( NULL == pCheckboxAccessoryInList )
		{
			continue;
		}

		CDB_RuledTableBase *pRuledTable = pCheckboxAccessoryInList->GetRuledTable();
		
		if( NULL != pRuledTable )
		{
			// If owner is not defined (we disable all) or if owner of current accessory is the same as 'lpTabID' then we disable it.
			if( true == strOwnerTabID.IsEmpty() || 0 == IDcmp( pRuledTable->GetFirstOwnerID(), lpTabID ) )
			{
				pCheckboxAccessoryInList->SetEnableStatus( false );
				pCheckboxAccessoryInList->ApplyInternalChange();
			}
		}
	}
}

void CRViewSSelSS::EnableCheckboxAccessories( LPCTSTR lpTabID, vecCDCAccessoryList *pvecAccessoryList )
{
	if( NULL == pvecAccessoryList )
	{
		return;
	}

	CString strOwnerTabID( lpTabID );
	
	for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
	{
		CCDBCheckboxAccessory *pCheckboxAccessoryInList = (*vecIter);

		if( NULL == pCheckboxAccessoryInList )
		{
			continue;
		}

		CDB_RuledTableBase *pRuledTable = pCheckboxAccessoryInList->GetRuledTable();
		
		if( NULL != pRuledTable )
		{
			// If owner is not defined (we enable all) or if owner of current accessory is the same as 'lpTabID' then we enable it.
			if( true == strOwnerTabID.IsEmpty() || 0 == IDcmp( pRuledTable->GetFirstOwnerID(), lpTabID ) )
			{
				pCheckboxAccessoryInList->SetEnableStatus( true );
				pCheckboxAccessoryInList->ApplyInternalChange();
			}
		}
	}
}

void CRViewSSelSS::RemoveCheckboxAccessories( vecCDCAccessoryList *pvecAccessoryList, CSheetDescription *pclSSheetDescription, CDB_RuledTableBase *pRuledTable )
{
	if( NULL == pvecAccessoryList || NULL == pclSSheetDescription )
	{
		return;
	}

	// Find and clear already existing accessories.
	vecCDCAccessoryListIter It, Itnext;
	It = pvecAccessoryList->begin();
	
	while( It != pvecAccessoryList->end() )
	{
		CCDBCheckboxAccessory *pCheckboxAccessoryInList = (*It);
		
		if( NULL == pCheckboxAccessoryInList )
		{
			continue;
		}

		if( NULL == pRuledTable || ( NULL != pRuledTable && pCheckboxAccessoryInList->GetRuledTable() == pRuledTable ) )
		{
			Itnext = It;
			++Itnext;
			
			// First remove checkbox saved internally in the sheet description.
			pclSSheetDescription->RemoveCellDescription( (CCellDescription*)pCheckboxAccessoryInList );
			// Second remove entry in accessories list.
			pvecAccessoryList->erase( It );
			It = Itnext;
		}
		else
		{
			++It;
		}
	}
}
// End of methods to manage accessories
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage Title group button.
CCDButtonTitleGroup::CCDButtonTitleGroup( CPoint ptCellPosition, CSheetDescription *pclSheetDescription )
	: CCellDescriptionButton( RVSCellDescription::CDB_TitleGroup, ptCellPosition, pclSheetDescription, false, 0, CSSheet::PictureCellType::Icon )
{
	m_lFromColumn = 0;
	m_lToColumn = 0;
}

CCDButtonTitleGroup *CRViewSSelSS::CreateTitleGroupButtons( long lFromColumn, long lToColumn, long lRow, CSheetDescription *pclSSheetDescription )
{
	CCDButtonTitleGroup *pButtonTitleGroup = NULL;

	if( NULL != pclSSheetDescription && NULL != pclSSheetDescription->GetSSheetPointer() )
	{
		// Create title group buttons.
		for( long lLoop = lFromColumn; lLoop <= lToColumn; lLoop++ )
		{
			pButtonTitleGroup = new CCDButtonTitleGroup( CPoint( lLoop, lRow ), pclSSheetDescription );

			if( NULL != pButtonTitleGroup )
			{
				// Add button in cell description list.
				pclSSheetDescription->AddCellDescription( pButtonTitleGroup );
			}
		}
	}

	return pButtonTitleGroup;
}

void CRViewSSelSS::DeleteTitleGroupButton( CCDButtonTitleGroup *&pCDBTitleGroup, CSheetDescription *pclSSheetDescription )
{
	if( NULL == pCDBTitleGroup || NULL == pclSSheetDescription )
	{
		return;
	}

	// First clean properly the button.
	// Remark: this method will check if rows must be shown before the button will be deleted.
	pCDBTitleGroup->DeleteButton();

	// Remove button from cell description list linked to sheet description.
	pclSSheetDescription->RemoveCellDescription( pCDBTitleGroup );

	// Reset pointer
	pCDBTitleGroup = NULL;
}

// End of methods to Title group button.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods to manage link.

CCellDescriptionLink *CRViewSSelSS::CreateCellDescriptionLink( long lColumn, long lRow, CSheetDescription *pclSSheetDescription, LPARAM lpParam )
{
	CCellDescriptionLink *pclLink = NULL;

	if( NULL != pclSSheetDescription && NULL != pclSSheetDescription->GetSSheetPointer() )
	{
		// Create the link.
		pclLink = new CCellDescriptionLink( CPoint( lColumn, lRow ), pclSSheetDescription, lpParam );

		if( NULL != pclLink )
		{
			// Add the link in the cell description list.
			pclSSheetDescription->AddCellDescription( pclLink );
		}
	}

	return pclLink;
}

void CRViewSSelSS::DeleteCellDescriptionLink( CCellDescriptionLink *&pCDLink, CSheetDescription *pclSSheetDescription )
{
	if( NULL == pCDLink || NULL == pclSSheetDescription )
	{
		return;
	}

	// Remove link from cell description list linked to sheet description.
	pclSSheetDescription->RemoveCellDescription( pCDLink );

	// Reset pointer
	pCDLink = NULL;
}

// End of methods to manage link.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CRViewSSelSS::DefineNextFocusedRow( CSheetDescription *pclCurrentSheetDescription, long lColumn, long lRow, bool bAllowLoseFocus, long &lNewRow, 
		CSheetDescription *&pclNewSheetDescription, bool &bShiftPressed )
{
	if( NULL == pclCurrentSheetDescription )
	{
		return DNFERRC_ParameterNULL;
	}

	CSheetDescription *pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclCurrentSheetDescription );
	CSheetDescription *pclPrevSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclCurrentSheetDescription );

	bShiftPressed = ( GetKeyState( VK_SHIFT ) < 0 ) ? true : false;

	int iReturn = -1;

	if( ( false == bShiftPressed && NULL == pclNextSheetDescription ) ||
		( true == bShiftPressed && NULL == pclPrevSheetDescription ) )
	{
		iReturn = DNFERRC_RViewLoseFocusOK;
	}
	else
	{
		if( true == bShiftPressed )
		{
			while( NULL != pclPrevSheetDescription && -1 == iReturn )
			{
				// Jump to the previous sheet.
				lNewRow = pclPrevSheetDescription->GetFirstSelectableRow( true );

				if( -1 != lNewRow )
				{
					pclNewSheetDescription = pclPrevSheetDescription;
					iReturn = DNFERRC_FocusOnPreviousGroupOK;
				}
				else
				{
					pclPrevSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclPrevSheetDescription );

					if( NULL == pclPrevSheetDescription )
					{
						iReturn = DNFERRC_RViewLoseFocusOK;
					}
				}
			}
		}
		else
		{
			while( NULL != pclNextSheetDescription && -1 == iReturn )
			{
				// Check the next sheet if there is a selectable row.
				lNewRow = pclNextSheetDescription->GetFirstSelectableRow( true );
				
				if( -1 != lNewRow )
				{
					pclNewSheetDescription = pclNextSheetDescription;
					iReturn = DNFERRC_FocusOnNextGroupOK;
				}
				else
				{
					pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclNextSheetDescription );
					
					if( NULL == pclNextSheetDescription )
					{
						iReturn = DNFERRC_RViewLoseFocusOK;
					}
				}
			}
		}
	}

	if( DNFERRC_RViewLoseFocusOK == iReturn )
	{
		// If losing the focus is not allowed...
		if( false == bAllowLoseFocus )
		{
			iReturn = DNFERRC_RViewLoseFocusError;
		}
		else
		{
			// Unselect the focus.
			KillCurrentFocus();

			// Notify.
			std::vector<INotificationHandler*>::iterator vecIter;
			
			for( vecIter = m_mapNotificationHandlerList[INotificationHandler::NH_OnLostFocus].begin(); vecIter != m_mapNotificationHandlerList[INotificationHandler::NH_OnLostFocus].end(); ++vecIter )
			{
				(*vecIter)->OnRViewSSelLostFocusWithTabKey( bShiftPressed );
			}
		
			iReturn = DNFERRC_RViewLoseFocusOK;
		}
	}

	return iReturn;
}

void CRViewSSelSS::PrepareAndSetNewFocus( CSheetDescription *pclSheetDescription, long lColumn, long lRow, long lParam )
{
	// Kill old focus to be sure.
	KillCurrentFocus();

	_PrepareFocus( pclSheetDescription, lColumn, lRow, lParam );

	_SetFocus();
}

void CRViewSSelSS::KillCurrentFocus( void )
{
	if( NULL == m_stCurrentFocus.m_pclSheetDescription )
	{
		return;
	}

	CSSheet *pclSSheet = m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer();
	
	if( NULL == pclSSheet )
	{
		return;
	}

	// Run all selected rows.
	for( int iLoopRow = m_stCurrentFocus.m_rectCurrentFocus.top; iLoopRow <= m_stCurrentFocus.m_rectCurrentFocus.bottom; iLoopRow++ )
	{
		pclSSheet->UnSelectMultipleRows( iLoopRow );
	}

	// Reset the default borders
	_RestoreFocusedCellBorders();

	// Clean all infos on this focus.
	m_stCurrentFocus.Reset();
}

long CRViewSSelSS::GetCurrentFocusedColumn( void )
{
	return m_stCurrentFocus.m_rectCurrentFocus.left;
}

long CRViewSSelSS::GetCurrentFocusedRow( void )
{
	return m_stCurrentFocus.m_rectCurrentFocus.top;
}

bool CRViewSSelSS::GetShowAllPrioritiesFlag( int iButtonID )
{
	bool bShowAllPriorities = false;
	CDS_IndSelParameter *pclIndSelParameters = m_pclProdSelParams->m_pTADS->GetpIndSelParameter();

	if( NULL != pclIndSelParameters)
	{
		int iState = pclIndSelParameters->GetWindowShowAllPrioritiesState( (int)GetRViewID(), iButtonID );
		bShowAllPriorities = ( -1 == iState || 0 == iState ) ? false : true;
	}

	return bShowAllPriorities;
}

COLORREF CRViewSSelSS::GetTitleBackgroundColor( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return 0;
	}

	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

	// By default, we take the first color (_TAH_TITLE_MAIN -> Orange).
	// Remark: first parameter just allows to alternate between two colors (orange and beryllium) for each consecutive title.
	//         'm_fModificationInCourse' allow to change these colors if we are in edition mode.
	COLORREF backgroundColor = pclSSheet->GetColHeaderBackColor( false, m_pclProdSelParams->m_bEditModeRunning );

	// If previous sheet description exist...
	CSheetDescription *pclPrevSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclSheetDescription );

	if( NULL != pclPrevSheetDescription )
	{
		// Take the opposite color of the previous group title background color.
		COLORREF currentBackColor = 0;
		pclPrevSheetDescription->GetUserVariable( _SDUV_TITLEBACKCOLOR, (LPARAM&)currentBackColor );
		bool fEven = ( currentBackColor == pclSSheet->GetColHeaderBackColor( false, m_pclProdSelParams->m_bEditModeRunning ) );
		backgroundColor = pclSSheet->GetColHeaderBackColor( fEven, m_pclProdSelParams->m_bEditModeRunning );
	}

	return backgroundColor;
}

void CRViewSSelSS::SelectionHasChanged( bool bIsReady )
{
	std::vector<INotificationHandler*>::iterator vecIter;

	for( vecIter = m_mapNotificationHandlerList[INotificationHandler::NH_OnSelectionChanged].begin(); 
			vecIter != m_mapNotificationHandlerList[INotificationHandler::NH_OnSelectionChanged].end(); ++vecIter )
	{
		(*vecIter)->OnRViewSSelSelectionChanged( bIsReady );
	}
}

void CRViewSSelSS::TextTipFetchEllipsesHelper( long lColumn, long lRow, CSheetDescription *pclSheetDescription, SHORT *pnTipWidth, CString *pStrText )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pnTipWidth || NULL == pStrText )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

	if( lColumn < 1 || lColumn > pclSSheet->GetMaxCols() || lRow < 1 || lRow > pclSSheet->GetMaxRows() )
	{
		return;
	}

	// Check style of current cell.
	SS_CELLTYPE rCellType;
	pclSSheet->GetCellType( lColumn, lRow, &rCellType );
	
	if( SS_TYPE_STATICTEXT == rCellType.Type && SSS_ELLIPSES == ( SSS_ELLIPSES & rCellType.Style ) )
	{
		CString strCellText = pclSSheet->GetCellText( lColumn, lRow );
		
		if( false == strCellText.IsEmpty() )
		{
			double dMaxWidth, dMaxHeight, dCurrentWidth;
			pclSSheet->GetMaxTextCellSize( lColumn, lRow, &dMaxWidth, &dMaxHeight );
			pclSSheet->GetColWidth( lColumn, &dCurrentWidth );
			
			if( dCurrentWidth < dMaxWidth )
			{
				*pStrText = strCellText;
				*pnTipWidth = (SHORT)( pclSSheet->GetColWidthInPixelsW( lColumn ) >> 1 );
			}
		}
	}
}

void CRViewSSelSS::UpdateAccessoryList( vector<CCDBCheckboxAccessory*> vectAccChecked, CAccessoryList * pclAccListToUpdate, CAccessoryList::AccessoryType eAccType )
{
	if( pclAccListToUpdate != NULL && pclAccListToUpdate->GetCount( eAccType ) > 0 )
	{
		// Delete unchecked accessory in accessory list
		CAccessoryList::AccessoryItem rAccessoryItem = pclAccListToUpdate->GetFirst( eAccType );
		while( rAccessoryItem.IDPtr.MP != NULL )
		{
			bool bAccfound = false;
			for( vecCDCAccessoryListIter vecIter = vectAccChecked.begin(); vecIter != vectAccChecked.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					if( true == rAccessoryItem.IDPtr.IDMatch( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr().ID ) )
					{
						bAccfound = true;
						break;
					}
				}
			}
			if( ( false == bAccfound ) && ( true == pclAccListToUpdate->Remove( rAccessoryItem ) ) )
			{
				rAccessoryItem = pclAccListToUpdate->GetGlobalPostion();
			}
			else
			{
				rAccessoryItem = pclAccListToUpdate->GetNext( eAccType );
			}
		}
	}

	// Add new accessory in accessory list
	for( vecCDCAccessoryListIter vecIter = vectAccChecked.begin(); vecIter != vectAccChecked.end(); vecIter++ )
	{
		CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;
		bool bAccfound = false;
		if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
		{
			if( pclAccListToUpdate != NULL && pclAccListToUpdate->GetCount( eAccType ) > 0 )
			{
				bAccfound = pclAccListToUpdate->FindItem( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr() );
			}
			if( false == bAccfound )
			{
				pclAccListToUpdate->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), eAccType,
					pCDBCheckboxAccessory->GetRuledTable() );
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSelSS::_PrepareFocus( CSheetDescription *pclSheetDescription, long lColumn, long lRow, long lParam )
{
	if( NULL == pclSheetDescription )
	{
		return;
	}

	// Prepare parameter if 'lParam' passed as argument is NULL.
	m_stCurrentFocus.m_lParam = 0;
	
	if( 0 == lParam )
	{
		CCellDescription *pclCellDescription = pclSheetDescription->GetCellDescription( lColumn, lRow );
		
		if( NULL != pclCellDescription )
		{
			switch( pclCellDescription->GetCellType() )
			{
				case RVSCellDescription::CD_Product:
					{
						CCellDescriptionProduct *pclCellProduct = dynamic_cast<CCellDescriptionProduct *>( pclCellDescription );

						if( NULL != pclCellProduct )
						{
							m_stCurrentFocus.m_lParam = pclCellProduct->GetProduct();
						}
					}
					break;

				case RVSCellDescription::CDBC_Accessories:
					{
						CCDBCheckboxAccessory *pclCellAccessory = dynamic_cast<CCDBCheckboxAccessory *>( pclCellDescription );

						if( NULL != pclCellAccessory )
						{
							m_stCurrentFocus.m_lParam = pclCellAccessory->GetAccessoryLParam();
						}
					}
					break;
			}
		}
	}

	m_stCurrentFocus.m_pclSheetDescription = pclSheetDescription;
	
	if( CSheetDescription::SD_ParameterMode_Mono == pclSheetDescription->GetParameterMode() )
	{
		pclSheetDescription->GetFocusColumnRange( m_stCurrentFocus.m_rectCurrentFocus.left, m_stCurrentFocus.m_rectCurrentFocus.right );
		m_stCurrentFocus.m_rectCurrentFocus.top = m_stCurrentFocus.m_rectCurrentFocus.bottom = lRow;
	}
	else
	{
		// If we are in multi-mode that means we have only one column by parameter (for example accessory). This is why left is the 
		// same as right. And then we can directly retrieve parameter from the column position.
		m_stCurrentFocus.m_rectCurrentFocus.left = m_stCurrentFocus.m_rectCurrentFocus.right = lColumn;
		m_stCurrentFocus.m_rectCurrentFocus.top = pclSheetDescription->GetFirstRowOfItem( lRow );
		m_stCurrentFocus.m_rectCurrentFocus.bottom = m_stCurrentFocus.m_rectCurrentFocus.top + pclSheetDescription->GetRowsByItem() - 1;
	}
}

void CRViewSSelSS::_SetFocus( void )
{
	if( NULL == m_stCurrentFocus.m_pclSheetDescription 
			|| TRUE == m_stCurrentFocus.m_rectCurrentFocus.IsRectNull() )
	{
		return;
	}

	CSSheet *pclSSheet = m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer();
	
	if( NULL == pclSSheet )
	{
		return;
	}

	// Here a new feature. The background color of a line selected by user is in yellow color. If focus is set on the same line, it is clearer that
	// the background on this line stay in yellow. 
	COLORREF backColor = _TAH_UNDER_SELECTION;
	LPARAM lVarValue;
	
	if( true == m_stCurrentFocus.m_pclSheetDescription->GetUserVariable( _SDUV_SELECTEDPRODUCT, lVarValue ) )
	{
		CCellDescriptionProduct *pclCDProduct = dynamic_cast<CCellDescriptionProduct *>( (CCellDescription*)lVarValue );
		
		if( NULL != pclCDProduct && pclCDProduct->GetCellPosition().y == m_stCurrentFocus.m_rectCurrentFocus.top )
		{
			backColor = _TAH_SELECTED_ORANGE;
		}
	}

	// Run all selected rows.
	pclSSheet->SelectMutipleRows( m_stCurrentFocus.m_rectCurrentFocus.top, m_stCurrentFocus.m_rectCurrentFocus.bottom, 
			m_stCurrentFocus.m_rectCurrentFocus.left, m_stCurrentFocus.m_rectCurrentFocus.right, backColor );

	// Save the current border.
	_SaveFocusedCellBorders();

	// Define the new ones.
	// To clarify code.
	CRect rectFocus( m_stCurrentFocus.m_rectCurrentFocus );
		
	pclSSheet->SetCellBorder( rectFocus.left, rectFocus.top, rectFocus.left, rectFocus.bottom, true, SS_BORDERTYPE_LEFT, 
			SS_BORDERSTYLE_FINE_DOT, SPREAD_COLOR_NONE );

	pclSSheet->SetCellBorder( rectFocus.left, rectFocus.top, rectFocus.right, rectFocus.top, true, SS_BORDERTYPE_TOP, 
			SS_BORDERSTYLE_FINE_DOT, SPREAD_COLOR_NONE );

	pclSSheet->SetCellBorder( rectFocus.right, rectFocus.top, rectFocus.right, rectFocus.bottom, true, SS_BORDERTYPE_RIGHT, 
			SS_BORDERSTYLE_FINE_DOT, SPREAD_COLOR_NONE );

	pclSSheet->SetCellBorder( rectFocus.left, rectFocus.bottom, rectFocus.right, rectFocus.bottom, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_FINE_DOT, SPREAD_COLOR_NONE );

	// Set focus on the CSSheet to be sure to receive input and mouse event on it!
	pclSSheet->SetFocus();
}

void CRViewSSelSS::_SaveFocusedCellBorders()
{
	if( TRUE == m_stCurrentFocus.m_rectCurrentFocus.IsRectNull() )
	{
		return;
	}

	CSSheet *pclSSheet = m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer();
	
	if( NULL == pclSSheet )
	{
		return;
	}

	// To clarify code.
	CRect rectFocus( m_stCurrentFocus.m_rectCurrentFocus );

	WORD wStyle;
	COLORREF colRef;
	pclSSheet->GetCellBorder(rectFocus.left, rectFocus.top, SS_BORDERTYPE_LEFT, &wStyle, &colRef );
	m_stCurrentFocus.m_rectBorderStyleSaved.left = wStyle;
	m_stCurrentFocus.m_rectBorderColorSaved.left = colRef;

	pclSSheet->GetCellBorder(rectFocus.left, rectFocus.top, SS_BORDERTYPE_TOP, &wStyle, &colRef );
	m_stCurrentFocus.m_rectBorderStyleSaved.top = wStyle;
	m_stCurrentFocus.m_rectBorderColorSaved.top = colRef;

	pclSSheet->GetCellBorder(rectFocus.right, rectFocus.top, SS_BORDERTYPE_RIGHT, &wStyle, &colRef );
	m_stCurrentFocus.m_rectBorderStyleSaved.right = wStyle;
	m_stCurrentFocus.m_rectBorderColorSaved.right = colRef;

	pclSSheet->GetCellBorder(rectFocus.left, rectFocus.bottom, SS_BORDERTYPE_BOTTOM, &wStyle, &colRef );
	m_stCurrentFocus.m_rectBorderStyleSaved.bottom = wStyle;
	m_stCurrentFocus.m_rectBorderColorSaved.bottom = colRef;
}

void CRViewSSelSS::_RestoreFocusedCellBorders()
{
	if( TRUE == m_stCurrentFocus.m_rectCurrentFocus.IsRectNull() 
			|| TRUE == m_stCurrentFocus.m_rectBorderStyleSaved.IsRectNull() 
			|| TRUE == m_stCurrentFocus.m_rectBorderColorSaved.IsRectNull() )
	{
		return;
	}

	CSSheet *pclSheet = m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer();
	
	if( NULL == pclSheet )
	{
		return;
	}

	// To clarify code.
	CRect rectFocus( m_stCurrentFocus.m_rectCurrentFocus );

	// Restore the LEFT border.
	pclSheet->SetBorderRange( rectFocus.left, rectFocus.top, rectFocus.left, rectFocus.bottom, SS_BORDERTYPE_LEFT,
			(WORD)m_stCurrentFocus.m_rectBorderStyleSaved.left, (COLORREF)m_stCurrentFocus.m_rectBorderColorSaved.left );

	// Restore the UPPER border.
	pclSheet->SetBorderRange( rectFocus.left, rectFocus.top, rectFocus.right, rectFocus.top, SS_BORDERTYPE_TOP, 
			(WORD)m_stCurrentFocus.m_rectBorderStyleSaved.top, (COLORREF)m_stCurrentFocus.m_rectBorderColorSaved.top );

	// Restore the RIGHT border.
	pclSheet->SetBorderRange( rectFocus.right, rectFocus.top, rectFocus.right, rectFocus.bottom, SS_BORDERTYPE_RIGHT, 
			(WORD)m_stCurrentFocus.m_rectBorderStyleSaved.right, (COLORREF)m_stCurrentFocus.m_rectBorderColorSaved.right );

	// Restore the LOWER border.
	pclSheet->SetBorderRange( rectFocus.left, rectFocus.bottom, rectFocus.right, rectFocus.bottom, SS_BORDERTYPE_BOTTOM, 
			(WORD)m_stCurrentFocus.m_rectBorderStyleSaved.bottom, (COLORREF)m_stCurrentFocus.m_rectBorderColorSaved.bottom );
}

int CRViewSSelSS::_BoxButtonClicked( CCellDescriptionButton *pclCDButton, CSheetDescription *pclSheetDescription )
{
	// TODO!
	
	if( NULL == pclCDButton || NULL == pclSheetDescription )
	{
		return RRT_NoRefreshNeeded;
	}

	CCDButtonBox *pclCDBBox = dynamic_cast<CCDButtonBox *>( pclCDButton );
	
	if( NULL == pclCDBBox )
	{
		return RRT_NoRefreshNeeded;
	}

	CSSheet *pclSSheet = pclCDBBox->GetpSSheet();
	
	if( NULL == pclSSheet )
	{
		return RRT_NoRefreshNeeded;
	}

	return ( RRT_RefreshSheets | RRT_RefreshPosition );
}

int CRViewSSelSS::_ShowAllPrioritiesArrowClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription )
{
	if( 0 == m_ViewDescription.GetSheetNumber() )
	{
		return RRT_NoRefreshNeeded;
	}
	
	// For this kind of button, we have to pass all defined Show/Hide all priorities buttons in all sheets. Because
	// we must show/hide not only for the clicked one but also for all others same buttons.

	int iNeedRefresh = RRT_NoRefreshNeeded;

	// Retrieve list of all defined Show/Hide all priorities buttons.
	// Must have only one !
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescription->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CDB_ShowAllPriorities );

	if( vecCellDescriptionList.size() > 1 )
	{
		ASSERT( 0 );
	}

	CCellDescriptionButton *pButton = dynamic_cast<CCellDescriptionButton *>( vecCellDescriptionList[0] );
			
	if( NULL == pButton )
	{
		return RRT_NoRefreshNeeded;
	}
			
	pButton->ToggleButtonState( true );

	// Change in TASelect selection parameter.
	CDS_IndSelParameter *pclIndSelParameters = m_pclProdSelParams->m_pTADS->GetpIndSelParameter();

	if( NULL != pclIndSelParameters )
	{
		pclIndSelParameters->SetWindowShowAllPrioritiesState( (int)GetRViewID(), pclSheetDescription->GetSheetDescriptionID(), (int)pButton->GetButtonState() );
	}

	CWnd::SetRedraw( FALSE );

	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

	// Retrieve coordinates of the button pressed (in regards to sheet in which it belongs to).
	CRect rectOldButton( pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y, 
			pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y );

	CRect rectOldButtonPixel = pclSSheet->GetSelectionCoordInPixels( rectOldButton );
		
	// Convert the coordinates in absolute coordinate.
	CRect rectOldButtonScreen = rectOldButtonPixel;
	pclSSheet->ClientToScreen( &rectOldButtonScreen );

	// Convert coordinates in regards to right view client area.
	CRect rectOldButtonRView = rectOldButtonScreen;
	ScreenToClient( &rectOldButtonRView );

	// Move different sheets to correct position in the right view.
	SetSheetSize();

	// If there is a vertical scrolling bar...
	CRect rectRViewClient;
	GetClientRect( &rectRViewClient );
		
	if( m_rectVirtualSize.Height() > rectRViewClient.Height() )
	{
		SCROLLINFO rNewScrollInfo;
		ZeroMemory( &rNewScrollInfo, sizeof( SCROLLINFO ) );
		rNewScrollInfo.cbSize = sizeof( SCROLLINFO );
		rNewScrollInfo.fMask = SIF_ALL;
		GetScrollInfo( SB_VERT, &rNewScrollInfo );

		// Retrieve new coordinates of the button pressed (in regards to sheet in which it belongs to).
		CRect rectNewButton = CRect( pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y, 
				pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y );

		CRect rectNewButtonPixel = pclSSheet->GetSelectionCoordInPixels( rectNewButton );
			
		// Convert coordinate in regards to its real position with others sheets before.
		rectNewButtonPixel.OffsetRect( 0, pclSheetDescription->GetSSheetOffset().y );

		// Compute new position and verify it.
		int iNewnPos = rectNewButtonPixel.top - rectOldButtonRView.top;
			
		if( iNewnPos + (int)rNewScrollInfo.nPage > rNewScrollInfo.nMax )
		{
			iNewnPos = rNewScrollInfo.nMax - (int)rNewScrollInfo.nPage;
		}

		// Set the new position and scroll.
		SetScrollPos( SB_VERT, iNewnPos );
		ScrollWindow( 0, rNewScrollInfo.nPos - iNewnPos );
	}

	CWnd::SetRedraw( TRUE );

	// Pay attention: 'Invalidate' must be called after 'SetRedraw' because 'Invalidate' has no effect if 'SetRedraw' is set to FALSE.
	Invalidate();
		
	// Force a paint now.
	UpdateWindow();

	// return fNeedRefresh;
	return RRT_NoRefreshNeeded;
}

int CRViewSSelSS::_ExpandCollapseRowsClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription )
{
	if( NULL == pclCellDescription || NULL == pclSheetDescription )
	{
		return RRT_NoRefreshNeeded;
	}

	CCDButtonExpandCollapseRows *pclCDBExpandCollapeRow = dynamic_cast<CCDButtonExpandCollapseRows *>( pclCellDescription );
	
	if( NULL == pclCDBExpandCollapeRow )
	{
		return RRT_NoRefreshNeeded;
	}

	CSSheet *pclSSheet = pclCDBExpandCollapeRow->GetpSSheet();
	
	if( NULL == pclSSheet )
	{
		return RRT_NoRefreshNeeded;
	}

	// Change button status
	// Remark: 'true' to directly apply change.
	pclCDBExpandCollapeRow->ToggleButtonState( true );

	// If current focus is in the sheet, we must ensure that it is not set to an another line. Otherwise, we reset the focus on the
	// only visible row.
	if( m_stCurrentFocus.m_pclSheetDescription == pclSheetDescription )
	{
		// Kill focus on the current selection.
		KillCurrentFocus();

		// Prepare the new focus variables.
		_PrepareFocus( pclSheetDescription, pclCDBExpandCollapeRow->GetCellPosition().x, pclCDBExpandCollapeRow->GetCellPosition().y );

		// Set focus on product currently selected.
		_SetFocus();
	}

	return ( RRT_RefreshSheets | RRT_RefreshPosition );
}

int CRViewSSelSS::_ExpandCollapseGroupClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription )
{
	if( NULL == pclCellDescription || NULL == pclSheetDescription )
	{
		return RRT_NoRefreshNeeded;
	}

	CCDButtonExpandCollapseGroup *pclCDBExpandCollapeGroup = dynamic_cast<CCDButtonExpandCollapseGroup *>( pclCellDescription );

	if( NULL == pclCDBExpandCollapeGroup )
	{
		return RRT_NoRefreshNeeded;
	}

	CSSheet *pclSSheet = pclCDBExpandCollapeGroup->GetpSSheet();

	if( NULL == pclSSheet )
	{
		return RRT_NoRefreshNeeded;
	}

	// Change button status
	// Remark: 'true' to directly apply change.
	pclCDBExpandCollapeGroup->ToggleButtonState( true );
	int iButtonState = pclCDBExpandCollapeGroup->GetButtonState();

	// If current focus is in the sheet, we must set focus on another group (next or previous one).
	// Remark: we do that only when group is collapsed (then button is in the state of 'Expand').
	if( CCDButtonExpandCollapseGroup::ButtonState::ExpandRow == iButtonState && m_stCurrentFocus.m_pclSheetDescription == pclSheetDescription )
	{
		// Kill focus on the current selection.
		KillCurrentFocus();

		// Try to retrieve next group (or previous if next one doesn't exist).
		CSheetDescription *pclNewSheetDescription = m_ViewDescription.GetNextSheetDescription( pclSheetDescription );

		if( NULL == pclNewSheetDescription )
		{
			pclNewSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclSheetDescription );
		}

		if( NULL != pclNewSheetDescription )
		{
			long lNewFocusedRow = pclNewSheetDescription->GetFirstSelectableRow( true );
			
			if( -1 != lNewFocusedRow )
			{
				long lNewFocusedCol = pclNewSheetDescription->GetFirstParameterColumn();
				PrepareAndSetNewFocus( pclNewSheetDescription, lNewFocusedCol, lNewFocusedRow );
			}
		}
	}

	OnExpandCollapeGroupButtonClicked( pclCDBExpandCollapeGroup, pclSheetDescription );

	return RRT_RefreshSheets;
}

int CRViewSSelSS::_AccessoryCheckboxClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription )
{
	if( NULL == pclCellDescription || NULL == pclSheetDescription )
	{
		return RRT_NoRefreshNeeded;
	}

	CCDBCheckboxAccessory *pclCDBCheckboxAccessory = dynamic_cast<CCDBCheckboxAccessory *>( pclCellDescription );
	
	if( NULL == pclCDBCheckboxAccessory )
	{
		return RRT_NoRefreshNeeded;
	}

	CSSheet *pclSSheet = pclCDBCheckboxAccessory->GetpSSheet();
	
	if( NULL == pclSSheet )
	{
		return RRT_NoRefreshNeeded;
	}

	// HYS-1624: Do not refresh when the checkbox is disabled
	if( false == pclCDBCheckboxAccessory->GetEnableStatus() )
	{
		return RRT_NoRefreshNeeded;
	}
	// Change button status only if it is not disabled...
	else
	{
		// Remark: 'false' to not directly apply change.
		pclCDBCheckboxAccessory->ToggleButtonState( true );

		// Change focus only if needed!
		if( 0 != m_stCurrentFocus.m_lParam && pclCDBCheckboxAccessory->GetAccessoryLParam() != m_stCurrentFocus.m_lParam )
		{
			// Kill focus on the current selection.
			KillCurrentFocus();

			// Prepare the new focus variables.
			_PrepareFocus( pclSheetDescription, pclCDBCheckboxAccessory->GetCellPosition().x, pclCDBCheckboxAccessory->GetCellPosition().y );

			// Set focus on product currently selected.
			_SetFocus();
		}
	
		// Retrieve accessory list from current sheet description.
		vecCDCAccessoryList vecAccessoryList;
		CCellDescription *pclLoopCellDescription = pclSheetDescription->GetFirstCellDescription( RVSCellDescription::CDBC_Accessories );
		
		while( NULL != pclLoopCellDescription )
		{
			vecAccessoryList.push_back( (CCDBCheckboxAccessory*)pclLoopCellDescription );
			pclLoopCellDescription = pclSheetDescription->GetNextCellDescription( pclLoopCellDescription, RVSCellDescription::CDBC_Accessories );
		}
	
		// Verify only in debug mode.
		if( 0 == vecAccessoryList.size() )
		{
			ASSERT( 0 );
		}

		// Do default functionality for CheckBox by using the 'VerifyAccessoriesCheckbox' method.
		VerifyCheckboxAccessories( pclCDBCheckboxAccessory->GetAccessoryPointer(), pclCDBCheckboxAccessory->GetCheckStatus(), &vecAccessoryList );
	
		// Gray all incompatible actuators with the selected adapter.
		// Remark: inherited classes must managed itself this method!
		GrayUncompatibleAccessoriesCheckbox( pclCDBCheckboxAccessory->GetAccessoryPointer(), pclCDBCheckboxAccessory->GetCheckStatus() );
		
		// HYS-2031: Manage excluded adapter for selected accessory.
		GrayOtherExcludedOrUncompatibleProduct( pclCDBCheckboxAccessory->GetAccessoryPointer(), pclCDBCheckboxAccessory->GetCheckStatus() );
	}

	return ( RRT_RefreshSheets | RRT_RefreshPosition );
}

int CRViewSSelSS::_TitleGroupClicked( CCellDescription *pclCellDescription, CSheetDescription *pclSheetDescription )
{
	if( NULL == pclCellDescription || NULL == pclSheetDescription )
	{
		return RRT_NoRefreshNeeded;
	}

	CCDButtonTitleGroup *pclCDBTitleGroup = dynamic_cast<CCDButtonTitleGroup *>( pclCellDescription );
	
	if( NULL == pclCDBTitleGroup )
	{
		return RRT_NoRefreshNeeded;
	}

	CSSheet *pclSSheet = pclCDBTitleGroup->GetpSSheet();
	
	if( NULL == pclSSheet )
	{
		return RRT_NoRefreshNeeded;
	}

	bool bReturn = ResetColumnWidth( pclSheetDescription->GetSheetDescriptionID() );
	
	if( true == bReturn )
	{
		UINT uiSheetDescriptionID = pclSheetDescription->GetSheetDescriptionID();
		
		for( mapShortLongIter iter = m_mapSSheetColumnWidth[uiSheetDescriptionID].begin(); iter != m_mapSSheetColumnWidth[uiSheetDescriptionID].end(); ++iter )
		{
			pclSSheet->SetColWidthInPixels( iter->first, iter->second );
		}
	}

	// Remark: can't be NULL because 'CResizingColumnInfo' is a member variable of 'pclSheetDescription'.
	CResizingColumnInfo* pclResizingColumnInfo = pclSheetDescription->GetResizingColumnInfo();
	pclResizingColumnInfo->Rescan();	

	return ( RRT_RefreshSheets | RRT_RefreshPosition );
}

CCDBCheckboxAccessory *CRViewSSelSS::_IsAccessoryExistInList( CDB_Product *pAccessory, vecCDCAccessoryList *pvecAccessoryList )
{
	if( NULL == pAccessory || false == pAccessory->IsAnAccessory() || NULL == pvecAccessoryList )
	{
		return NULL;
	}

	if( 0 == pvecAccessoryList->size() )
	{
		// In debug mode we directly ASSERT because it's not possible to fall in this condition!!
		ASSERTA_RETURN( NULL );
	}

	for( vecCDCAccessoryListIter vecIter = pvecAccessoryList->begin(); vecIter != pvecAccessoryList->end(); ++vecIter )
	{
		if( NULL == *vecIter )
		{
			continue;
		}

		// Found we can return this accessory.
		if( pAccessory == (*vecIter)->GetAccessoryPointer() )
		{
			return (*vecIter);
		}
	}

	// Not found!
	return NULL;
}

void CRViewSSelSS::_CellFocused( CSheetDescription *pclSheetDescription, WPARAM wKeyPressed )
{
	if( NULL == pclSheetDescription || pclSheetDescription != m_stCurrentFocus.m_pclSheetDescription )
	{
		return;
	}

	EnableSSheetDrawing( FALSE );

	// Remark: m_stCurrentFocus contains last focus.
	CSheetDescription *pclNewSheetDescription = m_stCurrentFocus.m_pclSheetDescription;
	long lNewFocusedRow = -1;
	long lNewFocusedCol = -1;

	bool bExecuteFocus = true;
	ScrollPosition eScrollPosition = ScrollPosition::ScrollToCompute;

	switch( wKeyPressed )
	{
		case VK_UP:
			// Focus column.
			lNewFocusedCol = m_stCurrentFocus.m_rectCurrentFocus.left;

			// Focus row.
			m_ViewDescription.GetPrevSelectableRow( m_stCurrentFocus.m_pclSheetDescription, m_stCurrentFocus.m_rectCurrentFocus.left, 
					m_stCurrentFocus.m_rectCurrentFocus.top, true, lNewFocusedRow, pclNewSheetDescription );
			
			// If no previous row...
			if( -1 == lNewFocusedRow )
			{
				eScrollPosition = ScrollPosition::ScrollToTop;
				lNewFocusedRow = m_stCurrentFocus.m_rectCurrentFocus.top;
			}

			break;

		case VK_DOWN:
			// Focus column.
			lNewFocusedCol = m_stCurrentFocus.m_rectCurrentFocus.left;

			// Focus row.
			m_ViewDescription.GetNextSelectableRow( m_stCurrentFocus.m_pclSheetDescription, m_stCurrentFocus.m_rectCurrentFocus.left, 
					m_stCurrentFocus.m_rectCurrentFocus.top, true, lNewFocusedRow, pclNewSheetDescription );

			// If no next row...
			if( -1 == lNewFocusedRow )
			{
				eScrollPosition = ScrollPosition::ScrollToBottom;
				lNewFocusedRow = m_stCurrentFocus.m_rectCurrentFocus.top;
			}

			break;

		case VK_LEFT:
			{
				// Focus column.
				lNewFocusedCol = m_stCurrentFocus.m_rectCurrentFocus.left;

				// Retrieve previous parameter from the old one.
				long lParamFocusedCol = m_stCurrentFocus.m_pclSheetDescription->GetPrevParameterColumn( m_stCurrentFocus.m_rectCurrentFocus.left );
				
				if( lParamFocusedCol != -1 && lParamFocusedCol != m_stCurrentFocus.m_rectCurrentFocus.left )
				{
					// Check if there is at least a parameter on this column (case of accessories).
					CCellDescription *pclCellDescription = m_stCurrentFocus.m_pclSheetDescription->GetCellDescription( lParamFocusedCol, 
							m_stCurrentFocus.m_rectCurrentFocus.top );
					
					if( NULL != pclCellDescription )
					{
						// Check if it's well a product or an accessory.
						if( RVSCellDescription::CD_Product == pclCellDescription->GetCellType() 
								|| RVSCellDescription::CDBC_Accessories == pclCellDescription->GetCellType() )
						{
							lNewFocusedCol = lParamFocusedCol;
						}
					}
				}

				// Focus row.
				lNewFocusedRow = m_stCurrentFocus.m_rectCurrentFocus.top;
				
				// Sheet has not changed.
				pclNewSheetDescription = m_stCurrentFocus.m_pclSheetDescription;

				eScrollPosition = ScrollPosition::NoScroll;

				break;
			}

		case VK_RIGHT:
			{
				// Focus column.
				lNewFocusedCol = m_stCurrentFocus.m_rectCurrentFocus.left;

				// Retrieve next parameter from the old one.
				long lParamFocusedCol = m_stCurrentFocus.m_pclSheetDescription->GetNextParameterColumn( m_stCurrentFocus.m_rectCurrentFocus.left );
				
				if( lParamFocusedCol != -1 && lParamFocusedCol != m_stCurrentFocus.m_rectCurrentFocus.left )
				{
					// Check if there is at least a parameter on this column (case of accessories).
					CCellDescription *pclCellDescription = m_stCurrentFocus.m_pclSheetDescription->GetCellDescription( lParamFocusedCol, 
							m_stCurrentFocus.m_rectCurrentFocus.top );
					
					if( NULL != pclCellDescription )
					{
						// Check if it's well a product or an accessory.
						if( RVSCellDescription::CD_Product == pclCellDescription->GetCellType() 
								|| RVSCellDescription::CDBC_Accessories == pclCellDescription->GetCellType() )
						{
							lNewFocusedCol = lParamFocusedCol;
						}
					}
				}

				// Focus row.
				lNewFocusedRow = m_stCurrentFocus.m_rectCurrentFocus.top;
				
				// Sheet has not changed.
				pclNewSheetDescription = m_stCurrentFocus.m_pclSheetDescription;

				eScrollPosition = ScrollPosition::NoScroll;

				break;
			}

		case VK_TAB:
			lNewFocusedCol = m_stCurrentFocus.m_rectCurrentFocus.left;
			lNewFocusedRow = m_stCurrentFocus.m_rectCurrentFocus.top;

			bool fShiftPressed;
			int iReturnCode = DefineNextFocusedRow( m_stCurrentFocus.m_pclSheetDescription, m_stCurrentFocus.m_rectCurrentFocus.left, 
					m_stCurrentFocus.m_rectCurrentFocus.top, true, lNewFocusedRow, pclNewSheetDescription, fShiftPressed );
			
			if( ( DNFERRC_FocusOnPreviousGroupOK == iReturnCode || DNFERRC_FocusOnNextGroupOK == iReturnCode ) && NULL != pclNewSheetDescription )
			{
				lNewFocusedCol = pclNewSheetDescription->GetFirstParameterColumn();
			}
			else
			{
				bExecuteFocus = false;
				eScrollPosition = ( false == fShiftPressed ) ? ScrollPosition::ScrollToBottom : ScrollPosition::ScrollToTop;
			}

			break;
	}

	if( true == bExecuteFocus && lNewFocusedCol != -1 && lNewFocusedRow != -1 && pclNewSheetDescription != NULL )
	{
		// If new row is different from current focus...
		if( lNewFocusedRow != m_stCurrentFocus.m_rectCurrentFocus.top )
		{
			// If we change sheet...
			if( m_stCurrentFocus.m_pclSheetDescription != pclNewSheetDescription )
			{
				// Reset active columns to the correct position.
				lNewFocusedCol = pclNewSheetDescription->GetFirstFocusedColumn();
			}
		}

		if( m_stCurrentFocus.m_pclSheetDescription != pclNewSheetDescription || lNewFocusedCol != m_stCurrentFocus.m_rectCurrentFocus.left 
				|| lNewFocusedRow != m_stCurrentFocus.m_rectCurrentFocus.top )
		{
			// Kill previous focus.
			KillCurrentFocus();

			// Prepare new focus.
			_PrepareFocus( pclNewSheetDescription, lNewFocusedCol, lNewFocusedRow, NULL );

			// Set the new focus.
			_SetFocus();
		}
	}

	switch( eScrollPosition )
	{
		case ScrollPosition::ScrollToCompute:
			{
				CRect rectFocusPixel = m_stCurrentFocus.m_pclSheetDescription->GetSSheetPointer()->GetSelectionCoordInPixels( m_stCurrentFocus.m_rectCurrentFocus );
				SetScrollPosition( eScrollPosition, m_stCurrentFocus.m_pclSheetDescription, rectFocusPixel );
			}
			break;

		case ScrollPosition::ScrollToTop:
		case ScrollPosition::ScrollToBottom:
			SetScrollPosition( eScrollPosition );
			break;

		case ScrollPosition::NoScroll:
			break;
	}

	EnableSSheetDrawing( TRUE );
// 	Invalidate();
// 	UpdateWindow();
}

void CRViewSSelSS::UpdateShowAllPrioritiesButtonState( CSheetDescription* pclSheetDescription, CCDButtonShowAllPriorities* pclButton, CCellDescription* pclCellDescription )
{
	if( NULL == pclSheetDescription || NULL == pclCellDescription || NULL == pclButton )
	{
		return;
	}

	CWnd::SetRedraw( FALSE );

	// Update button sate
	pclButton->ToggleButtonState( true );

	CSSheet* pclSSheet = pclSheetDescription->GetSSheetPointer();

	// Retrieve coordinates of the button pressed (in regards to sheet in which it belongs to).
	CRect rectOldButton( pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y,
		pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y );

	CRect rectOldButtonPixel = pclSSheet->GetSelectionCoordInPixels( rectOldButton );

	// Convert the coordinates in absolute coordinate.
	CRect rectOldButtonScreen = rectOldButtonPixel;
	pclSSheet->ClientToScreen( &rectOldButtonScreen );

	// Convert coordinates in regards to right view client area.
	CRect rectOldButtonRView = rectOldButtonScreen;
	ScreenToClient( &rectOldButtonRView );

	// Move different sheets to correct position in the right view.
	SetSheetSize();

	// If there is a vertical scrolling bar...
	CRect rectRViewClient;
	GetClientRect( &rectRViewClient );

	if( m_rectVirtualSize.Height() > rectRViewClient.Height() )
	{
		SCROLLINFO rNewScrollInfo;
		ZeroMemory( &rNewScrollInfo, sizeof( SCROLLINFO ) );
		rNewScrollInfo.cbSize = sizeof( SCROLLINFO );
		rNewScrollInfo.fMask = SIF_ALL;
		GetScrollInfo( SB_VERT, &rNewScrollInfo );

		// Retrieve new coordinates of the button pressed (in regards to sheet in which it belongs to).
		CRect rectNewButton = CRect( pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y,
			pclCellDescription->GetCellPosition().x, pclCellDescription->GetCellPosition().y );

		CRect rectNewButtonPixel = pclSSheet->GetSelectionCoordInPixels( rectNewButton );

		// Convert coordinate in regards to its real position with others sheets before.
		rectNewButtonPixel.OffsetRect( 0, pclSheetDescription->GetSSheetOffset().y );

		// Compute new position and verify it.
		int iNewnPos = rectNewButtonPixel.top - rectOldButtonRView.top;

		if( iNewnPos + (int)rNewScrollInfo.nPage > rNewScrollInfo.nMax )
		{
			iNewnPos = rNewScrollInfo.nMax - (int)rNewScrollInfo.nPage;
		}

		// Set the new position and scroll.
		SetScrollPos( SB_VERT, iNewnPos );
		ScrollWindow( 0, rNewScrollInfo.nPos - iNewnPos );
	}

	CWnd::SetRedraw( TRUE );

	// Pay attention: 'Invalidate' must be called after 'SetRedraw' because 'Invalidate' has no effect if 'SetRedraw' is set to FALSE.
	Invalidate();

	// Force a paint now.
	UpdateWindow();
}
