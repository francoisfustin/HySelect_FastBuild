#include "stdafx.h"
#include "Taselect.h"
#include "MainFrm.h"
#include "DlgLeftTabSelManager.h"
#include "ProductSelectionParameters.h"

CDlgSelectionBase::CDlgSelectionBase( CProductSelelectionParameters &clProductSelectionParam, UINT nID, CWnd* pParent )
	: CDialogExt( nID, pParent ), CDlgSelectionTabHelper( this )
{
	m_pclProdSelParam = &clProductSelectionParam;
	m_bInitialised = false;
	m_bRViewEmpty = true;
	m_iToolsDockablePaneContextID = -1;
	m_bXGBListInitialized = false;
}

void CDlgSelectionBase::ActivateLeftTabDialog()
{
	SetFocusOnControl( SetFocusWhere::First | SetFocusWhere::Edit );

	if( NULL != pDlgLTtabctrl )
	{
		CString str = GetDlgCaption();
		str.Replace( _T("&"), _T("&&") );
		pDlgLTtabctrl->SetWindowText( str );
	}
}

void CDlgSelectionBase::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	if( NULL == m_pclProdSelParam )
	{
		ASSERT_RETURN;
	}

	m_pclProdSelParam->m_eApplicationType = eProductSelectionApplicationType;

	// Update water characteristics.
	CDS_WaterCharacteristic *pclWaterCharacteristic = TASApp.GetpTADS()->GetpWCForProductSelection();

	if( NULL == pclWaterCharacteristic || NULL == pclWaterCharacteristic->GetpWCData() )
	{
		ASSERT_RETURN;
	}

	CWaterChar *pWC = pclWaterCharacteristic->GetpWCData();
	m_pclProdSelParam->m_WC = *pWC;
}

void CDlgSelectionBase::OnModifySelectedProductUpdateTechData( CSelectedInfos *pclSelectedInfos )
{
	if( NULL == pclSelectedInfos || NULL == m_pclProdSelParam || NULL == m_pclProdSelParam->m_pTADS 
			|| NULL == m_pclProdSelParam->m_pTADS->GetpWCForProductSelection() )
	{
		ASSERT_RETURN;
	}

	// Copy also the water characteristic in the technical parameters because the 'CMainFrame::InitializeRibbonRichEditBoxesSSel' 
	// method uses it instead of the one in 'PMSelectionHelper'.
	CWaterChar *pTechDataWC = m_pclProdSelParam->m_pTADS->GetpWCForProductSelection()->GetpWCData();
	*pTechDataWC = m_pclProdSelParam->m_WC;

	// Copy also additive name, PC weight, supply temperature and DT in the technical parameters because the main frame
	// uses these variables to update the fluid characteristics in the ribbon.
	CDS_TechnicalParameter *pclTechParams = m_pclProdSelParam->m_pTADS->GetpTechParams();
	*pclTechParams->GetDefaultISWC() = m_pclProdSelParam->m_WC;
	pclTechParams->SetDefaultISDT( pclSelectedInfos->GetDT() );
}

void CDlgSelectionBase::OnGroupIconClicked( CXGroupBox* pXGroupBoxClicked )
{
	if( NULL == pXGroupBoxClicked || 0 == m_mapListOfControlsInGroup.count( pXGroupBoxClicked->GetSafeHwnd() ) )
		return;

	CWnd::SetRedraw( FALSE );
	
	// Show or hide in regards to the current group state.
	for( int iLoopWnd = 0; iLoopWnd < (int)m_mapListOfControlsInGroup[pXGroupBoxClicked->GetSafeHwnd()].size(); iLoopWnd++ )
	{
		CWnd* pWnd = CWnd::FromHandle( m_mapListOfControlsInGroup[pXGroupBoxClicked->GetSafeHwnd()][iLoopWnd] );
		if( CXGroupBox::CONTROL_STYLE::header == pXGroupBoxClicked->GetControlStyle() )
		{
			// Memorize current 'show' and 'enable' status.
			m_mapControlShowStatus[pWnd->GetSafeHwnd()] = ( ( WS_VISIBLE == ( pWnd->GetStyle() & WS_VISIBLE) ) ? TRUE : FALSE );
			m_mapControlEnableStatus[pWnd->GetSafeHwnd()] = ( ( WS_DISABLED != ( pWnd->GetStyle() & WS_DISABLED) ) ? TRUE : FALSE );
			pWnd->ShowWindow( SW_HIDE );
			pWnd->EnableWindow( FALSE );
		}
		else
		{
			pWnd->ShowWindow( ( TRUE == m_mapControlShowStatus[pWnd->GetSafeHwnd()] ) ? SW_SHOW : SW_HIDE );
			pWnd->EnableWindow( m_mapControlEnableStatus[pWnd->GetSafeHwnd()] );
		}
	}

	// Determine what is the border line.
	CRect rectGroupClient( m_mapGroupCoordinate[pXGroupBoxClicked->GetSafeHwnd()] );
	
	CRect rectTabSSelClient;
	GetClientRect( &rectTabSSelClient );
	std::vector<LONG> vecLimit;
	
	LONG lLimit;
	if( CXGroupBox::CONTROL_STYLE::groupbox == pXGroupBoxClicked->GetControlStyle() )
	{
		vecLimit.insert( vecLimit.begin(), rectTabSSelClient.Width() + 1, rectTabSSelClient.bottom );
		vecLimit.erase( vecLimit.begin() + rectGroupClient.left, vecLimit.begin() + rectGroupClient.right );

		// Previously the group was collapsed.
		lLimit = rectGroupClient.top + pXGroupBoxClicked->GetHeaderHeight();
		vecLimit.insert( vecLimit.begin() + rectGroupClient.left, rectGroupClient.Width(), rectTabSSelClient.bottom );
	}
	else
	{
		vecLimit.insert( vecLimit.begin(), rectTabSSelClient.Width() + 1, 0 );
		vecLimit.erase( vecLimit.begin() + rectGroupClient.left, vecLimit.begin() + rectGroupClient.right );

		// Previously the group was expanded.
		lLimit = rectGroupClient.top + pXGroupBoxClicked->GetHeaderHeight();
		vecLimit.insert( vecLimit.begin() + rectGroupClient.left, rectGroupClient.Width(), lLimit );
	}

	// Determine amount of pixels to move.
	int iYToMove = m_mapGroupCoordinate[pXGroupBoxClicked->GetSafeHwnd()].Height() - pXGroupBoxClicked->GetHeaderHeight();

	// Move all other controls after this group.
	mmapIntHWNDIter iterControl;
	mmapIntHWNDRevIter reviterControl;
	CWnd* pclCurrentControl = NULL;
	bool fCanStop = false;
	if( CXGroupBox::CONTROL_STYLE::header == pXGroupBoxClicked->GetControlStyle() )
	{
		iterControl = m_mmapListOfControls.begin();
		if( iterControl != m_mmapListOfControls.end() )
			pclCurrentControl = CWnd::FromHandle( iterControl->second );
		else
			fCanStop = true;
	}
	else
	{
		reviterControl = m_mmapListOfControls.rbegin();
		if( reviterControl != m_mmapListOfControls.rend() )
			pclCurrentControl = CWnd::FromHandle( reviterControl->second );
		else
			fCanStop = true;
	}
	
	while( false == fCanStop )
	{
		bool fCanMove = true;
		bool fCanSetLimit = false;
		if( NULL == pclCurrentControl )
		{
			ASSERT( 0 );
			fCanMove = false;
		}

		// If current control is the group that user has clicked, we do nothing.
		if( true == fCanMove && pclCurrentControl == pXGroupBoxClicked )
			fCanMove = false;

		// First: check if current control belongs to a group.
		// Remark: if control belongs to a group we do nothing with it. Moving of all controls in a group is done below when
		//         we move the group.
		if( true == fCanMove && m_mapLinkControlAndGroup.count( pclCurrentControl->GetSafeHwnd() ) != 0 )
		{
			// Retrieve pointer on the group.
			CWnd* pWnd = CWnd::FromHandle( m_mapLinkControlAndGroup[pclCurrentControl->GetSafeHwnd()] );
			if( TRUE == pWnd->IsKindOf( RUNTIME_CLASS( CXGroupBox ) ) )
				fCanMove = false;
		}
		
		// Second: check if control is below the current group.
		CRect rectControl;
		int iNewBottomOrTop = 0;
		if( true == fCanMove )
		{
			// Retrieve coordinate of this control.
			pclCurrentControl->GetWindowRect( &rectControl );
			ScreenToClient( &rectControl );
			if( rectControl.top <= lLimit )
				fCanMove = false;

			iNewBottomOrTop = ( CXGroupBox::CONTROL_STYLE::header == pXGroupBoxClicked->GetControlStyle() ) ? rectControl.bottom : rectControl.top;
			fCanSetLimit = true;
		}
		
		if( true == fCanMove )
		{
			if( CXGroupBox::CONTROL_STYLE::header == pXGroupBoxClicked->GetControlStyle() )
			{
				// Group has been collapsed.

				// Check what is the bigger value in the control width range.
				int iBiggerValue = 0;
				for( int iLoopVec = rectControl.left; iLoopVec <= rectControl.right; iLoopVec++ )
				{
					if( vecLimit[iLoopVec] > iBiggerValue )
						iBiggerValue = vecLimit[iLoopVec];
				}
				
				// If there is enough space to move the current control...
				int iNewBottom = rectControl.bottom;
				if( rectControl.top - iYToMove > iBiggerValue )
				{
					// Move the current control.
					pclCurrentControl->SetWindowPos( NULL, rectControl.left, rectControl.top - iYToMove, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

					// If current control is a group...
					if( TRUE == pclCurrentControl->IsKindOf( RUNTIME_CLASS( CXGroupBox ) ) )
					{
						CXGroupBox* pclCurrentXGroupBox = (CXGroupBox*)pclCurrentControl;
						for( int iLoopWnd = 0; iLoopWnd < (int)m_mapListOfControlsInGroup[pclCurrentXGroupBox->GetSafeHwnd()].size(); iLoopWnd++ )
						{
							CWnd* pclControlInGroup = CWnd::FromHandle( m_mapListOfControlsInGroup[pclCurrentXGroupBox->GetSafeHwnd()][iLoopWnd] );
							CRect rectControlInGroup;
							pclControlInGroup->GetWindowRect( &rectControlInGroup );
							ScreenToClient( &rectControlInGroup );
							pclControlInGroup->SetWindowPos( NULL, rectControlInGroup.left, rectControlInGroup.top - iYToMove, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
						}
					}

					iNewBottomOrTop -= iYToMove;
				}
			}
			else
			{
				// Group has been expanded.

				// Check what is the lower value in the control width range.
				int iLowerValue = INT_MAX;
				for( int iLoopVec = rectControl.left; iLoopVec <= rectControl.right; iLoopVec++ )
				{
					if( vecLimit[iLoopVec] < iLowerValue )
						iLowerValue = vecLimit[iLoopVec];
				}

				// If there is enough space to move the current control...
				int iNewTop = rectControl.top;
				if( rectControl.bottom + iYToMove < iLowerValue )
				{
					// Verify the move. Never go farther that the original position.
					int iNewY = rectControl.top + iYToMove;
					if( iNewY > m_mapControlInitialSize[pclCurrentControl->GetSafeHwnd()].top )
						iNewY = m_mapControlInitialSize[pclCurrentControl->GetSafeHwnd()].top;

					// Move the current control.
					pclCurrentControl->SetWindowPos( NULL, rectControl.left, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

					// If current control is a group...
					if( TRUE == pclCurrentControl->IsKindOf( RUNTIME_CLASS( CXGroupBox ) ) )
					{
						CXGroupBox* pclCurrentXGroupBox = (CXGroupBox*)pclCurrentControl;
						for( int iLoopWnd = 0; iLoopWnd < (int)m_mapListOfControlsInGroup[pclCurrentXGroupBox->GetSafeHwnd()].size(); iLoopWnd++ )
						{
							CWnd* pclControlInGroup = CWnd::FromHandle( m_mapListOfControlsInGroup[pclCurrentXGroupBox->GetSafeHwnd()][iLoopWnd] );
							CRect rectControlInGroup;
							pclControlInGroup->GetWindowRect( &rectControlInGroup );
							ScreenToClient( &rectControlInGroup );

							// Verify the move. Never go farther that the original position.
							iNewY = rectControlInGroup.top + iYToMove;
							if( iNewY > m_mapControlInitialSize[pclControlInGroup->GetSafeHwnd()].top )
								iNewY = m_mapControlInitialSize[pclControlInGroup->GetSafeHwnd()].top;

							pclControlInGroup->SetWindowPos( NULL, rectControlInGroup.left, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
						}
					}

					iNewBottomOrTop += iYToMove;
				}
			}
		}

		// If we can change limits in the vector.
		if( true == fCanSetLimit )
		{
			vecLimit.erase( vecLimit.begin() + rectControl.left, vecLimit.begin() + rectControl.right );
			vecLimit.insert( vecLimit.begin() + rectControl.left, rectControl.Width(), iNewBottomOrTop );
		}

		// Go to next control.
		if( CXGroupBox::CONTROL_STYLE::header == pXGroupBoxClicked->GetControlStyle() )
		{
			iterControl++;
			if( iterControl != m_mmapListOfControls.end() )
				pclCurrentControl = CWnd::FromHandle( iterControl->second );
			else
				fCanStop = true;
		}
		else
		{
			reviterControl++;
			if( reviterControl != m_mmapListOfControls.rend() )
				pclCurrentControl = CWnd::FromHandle( reviterControl->second );
			else
				fCanStop = true;
		}
	}

	CWnd::SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CDlgSelectionBase::OnRViewSSelLostFocusWithTabKey( bool fShiftPressed )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	// Reset the focus on the left tab.
	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->SetFocus();
	}
	
	// Set the focus on the first available edit control or last control.
	SetFocusOnControl( ( false == fShiftPressed ) ? ( SetFocusWhere::First | SetFocusWhere::Edit ) : SetFocusWhere::Last );
}

void CDlgSelectionBase::ClearAll( void )
{
	// Clear the right sheet.
	ResetRightView();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgSelectionBase, CDialogExt )
	ON_WM_DESTROY()
	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
	ON_MESSAGE( WM_USER_PIPECHANGE, OnPipeChange )
	ON_MESSAGE( WM_USER_TECHPARCHANGE, OnTechParChange )
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
	ON_MESSAGE( WM_USER_WATERCHANGE, OnWaterChange )
//	ON_WM_LBUTTONDOWN()

END_MESSAGE_MAP()

BOOL CDlgSelectionBase::OnInitDialog() 
{
	CDialogExt::OnInitDialog();

	// Reset pointers on 'Database' and 'TADataStruct' to those ones by default.
	m_pclProdSelParam->m_pTADB = TASApp.GetpTADB();
	m_pclProdSelParam->m_pTADS = TASApp.GetpTADS();
	m_pclProdSelParam->m_bEditModeRunning = false;

//	_InitializeGroupList();

	m_ToolTip.Create(this, TTS_NOPREFIX);

	return TRUE;
}

BOOL CDlgSelectionBase::OnCommand( WPARAM wParam, LPARAM lParam )
{
	BOOL fReturn = TRUE;
	CWnd *pWnd = GetFocus();

	switch( wParam )
	{
		case IDOK: 

			if( pWnd != GetDlgItem( IDOK ) )
			{
				fReturn = FALSE;
			}

			break;

		case IDCANCEL:

			if( pWnd != GetDlgItem( IDCANCEL ) )
			{
				fReturn = FALSE;
			}

			break;
	}

	if( TRUE == fReturn )
	{
		fReturn = CDialogExt::OnCommand( wParam, lParam );
	}

	return fReturn;
}

BOOL CDlgSelectionBase::PreTranslateMessage( MSG *pMsg )
{
	if( TRUE == CDlgSelectionTabHelper::OnPreTranslateMessage( pMsg ) )
	{
		return TRUE;
	}

	if( WM_MOUSEMOVE == pMsg->message )
	{ 
		// Pass a mouse message to a tool tip control for processing 
		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.RelayEvent( pMsg );
		}
	}

	return CDialogExt::PreTranslateMessage( pMsg );
}

LRESULT CDlgSelectionBase::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclProdSelParam || NULL == m_pclProdSelParam->m_pTADS || NULL == m_pclProdSelParam->m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	m_bInitialised = false;

	ClearAll();

	// Reset pointers on 'Database' and 'TADataStruct' to those ones by default.
	m_pclProdSelParam->m_pTADB = TASApp.GetpTADB();
	m_pclProdSelParam->m_pTADS = TASApp.GetpTADS();
	m_pclProdSelParam->m_pUserDB = TASApp.GetpUserDB();
	m_pclProdSelParam->m_pPipeDB = TASApp.GetpPipeDB();
	m_pclProdSelParam->m_bEditModeRunning = false;

	CDS_WaterCharacteristic *pclWaterCharacteristic = m_pclProdSelParam->m_pTADS->GetpWCForProductSelection();

	if( NULL == pclWaterCharacteristic || NULL == pclWaterCharacteristic->GetpWCData() )
	{
		ASSERTA_RETURN( -1 );
	}

	CWaterChar *pWC = pclWaterCharacteristic->GetpWCData();

	m_pclProdSelParam->m_WC = *pWC;
	m_pclProdSelParam->m_eApplicationType = m_pclProdSelParam->m_pTADS->GetpTechParams()->GetProductSelectionApplicationType();

	m_bRViewEmpty = true;

	return 0;
}

LRESULT CDlgSelectionBase::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != ( WMUserPipeChange )wParam )
	{
		return 0;
	}

	_UpdatePipeSeriesSize();

	return 0;
}

LRESULT CDlgSelectionBase::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WM_UWC_WP_ForProductSel )
			|| WMUserWaterCharLParam::WM_UWC_LWP_NoChange == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	if( NULL == m_pclProdSelParam || NULL == m_pclProdSelParam->m_pTADS || NULL == m_pclProdSelParam->m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( 0 );
	}

	CDS_WaterCharacteristic *pclWaterCharacteristic = TASApp.GetpTADS()->GetpWCForProductSelection();

	if( NULL == pclWaterCharacteristic || NULL == pclWaterCharacteristic->GetpWCData() )
	{
		ASSERTA_RETURN( 0 );
	}

	CWaterChar *pWC = pclWaterCharacteristic->GetpWCData();
	m_pclProdSelParam->m_WC = *pWC;

	return 0;
}

/*
void CDlgSelectionBase::OnLButtonDown( UINT nFlags, CPoint point )
{
	// Remark: This method catch mouse left button event and allow to enable some special fields. 
	//		   For example Group Dp when user clicks on all the surface of the group and not only on 
	//		   the Dp radio button.

	bool fCallParent = true;
	CPoint pointScreenCord = point;
	ClientToScreen( &pointScreenCord );

	// Check if user has clicked in Group Dp.
	if( NULL != m_clGroupDp.GetSafeHwnd() )
	{
		CRect rectGroupDp;
		m_clGroupDp.GetWindowRect( &rectGroupDp );
		if( TRUE == rectGroupDp.PtInRect( pointScreenCord ) )
		{
			m_clGroupDp.ClickedOnGroup();
			fCallParent = false;
		}
	}

	// Check if user has clicked in Group Q.
	if( NULL != m_clGroupQ.GetSafeHwnd() )
	{
		CRect rectGroupQ;
		m_clGroupQ.GetWindowRect( &rectGroupQ );
		if( TRUE == rectGroupQ.PtInRect( pointScreenCord ) )
		{
			// Be more accurate.

			CButton *pFlowRadioButton = (CButton *)GetDlgItemW( IDC_RADIOFLOW );
			
			// PAY ATTENTION: power item is not always there !! (case for TRV).
			CButton *pPowerRadioButton = (CButton *)GetDlgItemW( IDC_RADIOPOWER );
			
			CRect rectPowerRadioButton;
			if( NULL != pPowerRadioButton )
				pPowerRadioButton->GetWindowRect( &rectPowerRadioButton );

			// If there is only flow or if power exist but user click on the left part...
			if( NULL == pPowerRadioButton || ( NULL != pPowerRadioButton && pointScreenCord.x < rectPowerRadioButton.left ) )
			{
				// Post message to the flow radio button to enable/disable it.
				::PostMessage( pFlowRadioButton->GetSafeHwnd(), BM_CLICK, 0, 0 );
			}
			else
			{
				// By default post message to the power radio button to enable/disable it.
				::PostMessage( pPowerRadioButton->GetSafeHwnd(), BM_CLICK, 0, 0 );

				// Change focus on DT if needed (see 'm_fMustFocusDT' remark in DlgSelectionBase.h and use of it in
				// CDlgSelectionBase::OnBnClickedRadioflowPower() ).
				CWnd *pStaticText = GetDlgItemW( IDC_STATICDT );
				CRect rectStaticText;
				pStaticText->GetWindowRect( &rectStaticText );
				if( pointScreenCord.y >= rectStaticText.top )
					m_fMustFocusDT = true;
			}
			fCallParent = false;
		}
	}

	if( true == fCallParent )
		CDialogExt::OnLButtonDown( nFlags, point );
}
*/

void CDlgSelectionBase::SelectCurrentComboPipes( CString strPipeSeriesID, CString strPipeSizeID )
{
	pMainFrame->SetRibbonComboSelPipeSeries( strPipeSeriesID );
	pMainFrame->SetRibbonComboSelPipeSize( strPipeSizeID );
	_UpdatePipeSeriesSize();
}

void CDlgSelectionBase::PostWMCommandToControl( CWnd *pWnd )
{
	if( NULL == pWnd || NULL == pWnd->GetSafeHwnd() )
	{
		return;
	}

	::PostMessage( GetSafeHwnd(), WM_COMMAND, (WPARAM)pWnd->GetDlgCtrlID(), (LPARAM)pWnd->GetSafeHwnd() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDlgSelectionBase::_UpdatePipeSeriesSize()
{
	m_pclProdSelParam->m_strPipeSeriesID = pMainFrame->GetRibbonComboSelPipeSeriesID();
	m_pclProdSelParam->m_strPipeID = pMainFrame->GetRibbonComboSelPipeSizeID();

	// Save also directly in the corresponding CDS_XXXSelParameters from CTADatastruct to be well synchronized.
	switch( m_pclProdSelParam->m_eProductSelectionMode )
	{
		case ProductSelectionMode::ProductSelectionMode_Individual:
			{
				CDS_IndSelParameter *pclCDSIndSelParameters = m_pclProdSelParam->m_pTADS->GetpIndSelParameter();

				if( NULL != pclCDSIndSelParameters )
				{
					pclCDSIndSelParameters->SetPipeSeriesID( m_pclProdSelParam->m_strPipeSeriesID );
					pclCDSIndSelParameters->SetPipeSizeID( m_pclProdSelParam->m_strPipeID );
				}
			}
			break;

		case ProductSelectionMode::ProductSelectionMode_Batch:
			{
				CDS_BatchSelParameter *pclCDSBatchSelParameters = m_pclProdSelParam->m_pTADS->GetpBatchSelParameter();

				if( NULL != pclCDSBatchSelParameters )
				{
					pclCDSBatchSelParameters->SetPipeSeriesID( m_pclProdSelParam->m_strPipeSeriesID );
					pclCDSBatchSelParameters->SetPipeSizeID( m_pclProdSelParam->m_strPipeID );
				}
			}
			break;

		case ProductSelectionMode::ProductSelectionMode_Wizard:
			{
				CDS_WizardSelParameter *pclCDSWizardSelParameters = m_pclProdSelParam->m_pTADS->GetpWizardSelParameter();

				if( NULL != pclCDSWizardSelParameters )
				{
					pclCDSWizardSelParameters->SetPipeSeriesID( m_pclProdSelParam->m_strPipeSeriesID );
					pclCDSWizardSelParameters->SetPipeSizeID( m_pclProdSelParam->m_strPipeID );
				}
			}
			break;
	}
}

void CDlgSelectionBase::_InitializeGroupList( void )
{
	if( false == m_bXGBListInitialized )
	{
		// Do a list of all controls belonging to 'CDlgSelectionBase' and sort them in regards to there top position.
		CWnd* pWnd = GetWindow( GW_CHILD );
		while( pWnd != NULL )
		{
			CRect rect;
			pWnd->GetWindowRect( &rect );
			ScreenToClient( &rect );
			m_mmapListOfControls.insert( pair<int, HWND>( rect.top, pWnd->GetSafeHwnd() ) );
			m_mapControlInitialSize[pWnd->GetSafeHwnd()] = rect;

			if( TRUE == pWnd->IsKindOf( RUNTIME_CLASS( CXGroupBox ) ) )
			{
				CRect rect;
				pWnd->GetWindowRect( &rect );
				ScreenToClient( &rect );
				m_mapGroupCoordinate.insert( pair<HWND, CRect>( pWnd->GetSafeHwnd(), rect ) );

				vecpHWND vecTemp;
				m_mapListOfControlsInGroup[pWnd->GetSafeHwnd()] = vecTemp;
			}

			pWnd = pWnd->GetWindow( GW_HWNDNEXT );
		}

		// Now we can check which controls belong to a group and which are independent.
		for( mmapIntHWNDIter iterControl = m_mmapListOfControls.begin(); iterControl != m_mmapListOfControls.end(); iterControl++ )
		{
			// If current control is not a 'CXGroupBox'...
			if( 0 == m_mapGroupCoordinate.count( iterControl->second ) )
			{
				// Check if this control belongs to a group.
				CRect rectControl;
				pWnd = CWnd::FromHandle( iterControl->second );
				pWnd->GetWindowRect( &rectControl );
				ScreenToClient( &rectControl );

				for( mapGroupRectIter iterGroup = m_mapGroupCoordinate.begin(); iterGroup != m_mapGroupCoordinate.end(); iterGroup++ )
				{
					CRect rectIntersected;
					rectIntersected.UnionRect( &iterGroup->second, &rectControl );
					if( rectIntersected.EqualRect( &iterGroup->second ) )
					{
						// Add current control to its group.
						m_mapListOfControlsInGroup[iterGroup->first].push_back( pWnd->GetSafeHwnd() );
						// Add link between current control and its group.
						m_mapLinkControlAndGroup[pWnd->GetSafeHwnd()] = iterGroup->first;
						break;
					}
				}
			}
		}
		m_bXGBListInitialized = true;
	}
}
