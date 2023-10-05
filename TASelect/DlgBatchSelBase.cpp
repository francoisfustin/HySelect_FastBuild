#include "stdafx.h"
#include <errno.h>
#include <algorithm>
#include "Taselect.h"
#include "MainFrm.h"
#include "Hydronic.h"

#include "DlgLeftTabSelManager.h"
#include "ProductSelectionParameters.h"

CDlgBatchSelBase::CDlgBatchSelBase( CBatchSelectionParameters &clBatchSelParams, UINT nID, CWnd *pParent )
	: CDlgSelectionBase( clBatchSelParams, nID, pParent )
{
	m_pclBatchSelParams = &clBatchSelParams;
	m_bInitialized = false;
	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;
	m_bMsgComboChangeDone = false;
	m_bMsgDoNotClearResult = false;
	m_bRadiosEnabled = true;
	m_mapColumnList.clear();
	m_eInputsVerificationStatus = BS_InputsVerificationStatus::BS_IVS_NotYetDone;
	m_eSuggestionStatus = BS_SuggestionStatus::BS_SS_NotYetDone;
	m_eValidationStatus = BS_ValidationStatus::BS_VS_NotYetDone;
	m_pclCurrentRowParameters = NULL;
	memset( &m_rTechParamSaved, 0, sizeof( m_rTechParamSaved ) );
}

CDlgBatchSelBase::~CDlgBatchSelBase()
{
	BS_ClearAllData();
}

bool CDlgBatchSelBase::IsButtonValidateEnabled()
{
	if( NULL == GetpBtnValidate() )
	{
		return false;
	}

	if( FALSE == GetpBtnValidate()->IsWindowEnabled() )
	{
		return false;
	}

	return true;
}

void CDlgBatchSelBase::SaveSelectionParameters()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::SaveSelectionParameters();

	CDS_BatchSelParameter *pclBatchSelParameters = m_pclBatchSelParams->m_pTADS->GetpBatchSelParameter();
	pclBatchSelParameters->SetPipeSeriesID( m_pclBatchSelParams->m_strPipeSeriesID );
	pclBatchSelParameters->SetPipeSizeID( m_pclBatchSelParams->m_strPipeID );
}

void CDlgBatchSelBase::LeaveLeftTabDialog()
{
	// If edition is running...
	if( true == m_bInitialized && BS_InputsVerificationStatus::BS_IVS_OK == m_eInputsVerificationStatus 
			&& BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus && NULL != m_pclCurrentRowParameters )
	{
		m_pclCurrentRowParameters = NULL;
		ClearAll();
		SetModificationMode( false );
		ResetRightView();
	}
}

void CDlgBatchSelBase::ActivateLeftTabDialog()
{
	if( NULL == m_pclBatchSelParams || NULL == m_pclBatchSelParams->m_pTADS || NULL == m_pclBatchSelParams->m_pTADS->GetpBatchSelParameter() )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::ActivateLeftTabDialog();

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	// Reinitialize pipes accordingly to the current context.
	CDS_BatchSelParameter *pclCDSBatchSelParameters = m_pclBatchSelParams->m_pTADS->GetpBatchSelParameter();
	m_pclBatchSelParams->m_strPipeSeriesID = pclCDSBatchSelParameters->GetPipeSeriesID( m_pclBatchSelParams );
	m_pclBatchSelParams->m_strPipeID = pclCDSBatchSelParameters->GetPipeSizeID();

	_BSFirstTimeInit();

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}
}

void CDlgBatchSelBase::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgSelectionBase::ChangeApplicationType( eProductSelectionApplicationType );

	// If batch selection is well initialized and results have already been suggested but not yet validated...
	if( true == m_bInitialized && BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus 
			&& BS_ValidationStatus::BS_VS_NotYetDone == m_eValidationStatus )
	{
		// We clear result because they no more match with the new parameters.
		OnBatchSelectionButtonClearResults();

		// We automatic suggest only if the window is visible.
		if( TRUE == IsWindowVisible() )
		{
			OnBnClickedSuggest();
		}
	}
}

bool CDlgBatchSelBase::IsRightViewInEdition()
{
	// If edition is running...
	bool bReturn = false;

	if( true == m_bInitialized && BS_InputsVerificationStatus::BS_IVS_OK == m_eInputsVerificationStatus 
		&& BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus && NULL != m_pclCurrentRowParameters )
	{
		bReturn = true;
	}

	return bReturn;
}

void CDlgBatchSelBase::OnRViewSSelSelectProduct()
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	// User selects a product by choosing it in the floating menu.
	OnBnClickedValidate();
}

void CDlgBatchSelBase::OnRViewSSelSelectionChanged( bool fSelectionActive )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	m_clButtonValidate.EnableWindow( ( true == fSelectionActive ) ? TRUE : FALSE );
}

void CDlgBatchSelBase::OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	switch( iKeyboardVirtualKey )
	{
		case VK_RETURN:
			if( true == IsButtonValidateEnabled() )
			{
				PostWMCommandToControl( GetpBtnValidate() );
			}
			else
			{
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			break;

		case VK_ESCAPE:
		{
			// Change the tools dockable pane to full screen mode.
			// HYS-1168 : We hide the DockablePane in edition mode, we have remove the auto hide mode when doing cancel
			CToolsDockablePane* pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
			if( NULL != pclToolsDockablePane )
			{
				pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
			}

			if( NULL != pDlgLeftTabSelManager )
			{
				pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, true );
			}

			m_pclCurrentRowParameters = NULL;
			ClearAll();
			SetModificationMode( false );

			// Reset the focus on the batch output dialog.
			m_clInterface.SetFocus();
		}
		break;
	}
}

CMainFrame::RightViewList CDlgBatchSelBase::GetRViewID()
{
	CMainFrame::RightViewList eRViewID = CMainFrame::eUndefined;
	CRViewSSelSS *pCurrentRightView = GetLinkedRightViewSSel();

	if( NULL != pCurrentRightView )
	{
		eRViewID = pCurrentRightView->GetRViewID();
	}

	return eRViewID;
}

void CDlgBatchSelBase::OnBatchSelectionButtonClearAll()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	ClearAll();
	BS_ClearAllData();
	EnableSuggestButton( false );

	m_clInterface.EnableCtxtMenuPasteData( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.EnableCtxtMenuAddColumn( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, false );
	m_clInterface.EnableEdition( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.RemoveAllTextTipFetchTrigger( (int)m_pclBatchSelParams->m_eProductSubCategory );

	// Change the tools dockable pane to full screen mode.
	// Do it only if windows is visible. 'OnBatchSelectionButtonClearResults' can be called event if windows is not visible. For example when the 'OnPipeChange'
	// is called, we want to apply to all batch selection opened (visible or not).
	bool bApply = ( TRUE == IsWindowVisible() ) ? true : false;

	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, bApply );
	}

	// Remark: the 'm_pclBatchSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( false );

	BS_EnableRadios( true );
	BS_EnableCombos( true );

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();
}

void CDlgBatchSelBase::OnBatchSelectionButtonClearResults()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	ClearAll();

	// Reset all results.
	if( 0 != m_mapAllRowData.size() )
	{
		for( mapLongRowParamsIter iter = m_mapAllRowData.begin(); iter != m_mapAllRowData.end(); ++iter )
		{
			if( NULL != iter->second.m_pclBatchResults )
			{
				delete iter->second.m_pclBatchResults;
			}

			iter->second.m_pclBatchResults = NULL;

			if( NULL != iter->second.m_pclCDSSelSelected )
			{
				IDPTR IDPtr = iter->second.m_pclCDSSelSelected->GetIDPtr();
				m_pclBatchSelParams->m_pTADS->DeleteObject( IDPtr );
			}

			iter->second.m_pclCDSSelSelected = NULL;
			iter->second.m_pclSelectedProduct = NULL;
			iter->second.m_pclSelectedSecondaryProduct = NULL;
			iter->second.m_pclSelectedActuator = NULL;
		}
	}

	m_clInterface.EnableCtxtMenuPasteData( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.EnableCtxtMenuAddColumn( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
	// HYS-1065: Add outputID to disable also the clear results button of oteher product
	m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.EnableEdition( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.RemoveAllTextTipFetchTrigger( (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_bAtLeastOneRowWithBestSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButAlternative = false;
	m_bMsgComboChangeDone = false;
	m_bMsgDoNotClearResult = false;
	m_eSuggestionStatus = BS_SuggestionStatus::BS_SS_NotYetDone;

	// Because we can clear result ONLY when suggestion has been done AND once suggestion has been done it's impossible to edit cell and change value,
	// we be sure that input verification is yet valid.
	m_eInputsVerificationStatus = BS_InputsVerificationStatus::BS_IVS_OK;

	// Change the tools dockable pane to full screen mode.

	// HYS-1656: (Caused by HYS-1168) We hide the DockablePane in edition mode, we have to remove the auto hide mode when we change combo
	CToolsDockablePane* pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();
	if( NULL != pclToolsDockablePane && TRUE == pclToolsDockablePane->IsAutoHideMode() )
	{
		pclToolsDockablePane->SetAutoHideMode( FALSE, pclToolsDockablePane->GetCurrentAlignment(), NULL, FALSE );
	}

	// Do it only if windows is visible. 'OnBatchSelectionButtonClearResults' can be called event if windows is not visible. For example when the 'OnPipeChange'
	// is called, we want to apply to all batch selection opened (visible or not).
	bool bApply = ( TRUE == IsWindowVisible() ) ? true : false;

	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, bApply );
	}

	// To be sure.
	// Remark: the 'm_pclBatchSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( false );

	if( NULL != GetpBtnSuggest() )
	{
		GetpBtnSuggest()->EnableWindow( TRUE );
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	// We save the block selection and the row selected.
	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_pclBatchSelParams->m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_pclBatchSelParams->m_eProductSubCategory );
	
	// We clear all selections (block and row).
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_pclBatchSelParams->m_eProductSubCategory );

	for( mapLongRowParamsIter iterRow = m_mapAllRowData.begin(); iterRow != m_mapAllRowData.end();
		 ++iterRow )
	{
		for( mapLongColParamsIter iterCol = m_mapColumnList.begin(); iterCol != m_mapColumnList.end();
			 ++iterCol )
		{
			// HYS-1995: Exclude flow column when paste action is forbiden. Case of PDT mode.
			if( false == iterCol->second.m_clDataColDef.IsPasteDataAllowed() && BS_ColumnID::BS_CID_Input_Flow != iterCol->second.m_iColumnID )
			{
				m_clInterface.ResetCellContent( iterCol->second.m_iColumnID, iterRow->first, -1, (int)m_pclBatchSelParams->m_eProductSubCategory );
			}
		}

		if( NULL != iterRow->second.m_pclBatchResults )
		{
			delete iterRow->second.m_pclBatchResults;
			iterRow->second.m_pclBatchResults = NULL;
		}

		iterRow->second.m_eStatus = RowStatus::BS_RS_NotYetDone;
	}

	// Restore the selected row.
	m_clInterface.SelectRow( lSelectedRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
	
	// Restore the selected block.
	m_clInterface.RestoreSelections( (int)m_pclBatchSelParams->m_eProductSubCategory );

	m_clInterface.EnableEdition( true, (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	BS_EnableCombos( true );
}

void CDlgBatchSelBase::OnBatchSelectionPasteData()
{
	m_eInputsVerificationStatus = BS_InputsVerificationStatus::BS_IVS_NotYetDone;
	BS_VerifyAllDataInput();
}

bool CDlgBatchSelBase::OnDlgOutputCellClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	if( 0 == m_mapAllRowData.count( lRowRelative ) 
			&& true == m_clInterface.IsRowValidForData( lRowRelative, (int)m_pclBatchSelParams->m_eProductSubCategory ) )
	{
		BSRowParameters rBSRowParameters;
		rBSRowParameters.m_lRow = lRowRelative;
		m_mapAllRowData[lRowRelative] = rBSRowParameters;
	}

	return true;
}

bool CDlgBatchSelBase::OnDlgOutputCellDblClicked( int iOutputID, int iColumnID, long lRowRelative )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	if( 0 == m_mapAllRowData.count( lRowRelative ) 
			&& true == m_clInterface.IsRowValidForData( lRowRelative, (int)m_pclBatchSelParams->m_eProductSubCategory ) )
	{
		BSRowParameters rBSRowParameters;
		rBSRowParameters.m_lRow = lRowRelative;
		m_mapAllRowData[lRowRelative] = rBSRowParameters;
	}

	return true;
}

bool CDlgBatchSelBase::OnDlgOutputCellEditModeOff( int iOutputID, int iColumnID, long lRowRelative, bool bChangeMade )
{
	if( false == bChangeMade )
	{
		return false;
	}

	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	if( 0 == m_mapAllRowData.count( lRowRelative ) )
	{
		BSRowParameters rBSRowParameters;
		rBSRowParameters.m_lRow = lRowRelative;
		m_mapAllRowData[lRowRelative] = rBSRowParameters;
	}
		
	DlgOutputHelper::mapIntCellBase mapCellData;

	bool bResult = m_clInterface.GetRow( &mapCellData, lRowRelative, (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bResult || 0 == (int)mapCellData.count( iColumnID ) )
	{
		mapCellData.clear();
		return false;
	}

	BSRowParameters *prBSRowParameters = &m_mapAllRowData[lRowRelative];

	if( iColumnID == BS_ColumnID::BS_CID_Input_Power || iColumnID == BS_ColumnID::BS_CID_Input_DT )
	{
		// HYS-1995: Save the current block selection before update flow column.
		m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_All, iOutputID );
		m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, iOutputID );
		BS_UpdateFlowCell( iOutputID, prBSRowParameters->m_lRow );
		m_clInterface.RestoreSelections( iOutputID );
	}

	BS_VerifyOneDataInput( mapCellData[iColumnID], prBSRowParameters );

	// Now we can delete the row read.
	DOH_DeleteMapCell( &mapCellData );

	return true;
}

void CDlgBatchSelBase::OnBatchSelectionDeleteRow( long lStartRowRelative, long lEndRowRelative )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	if( lStartRowRelative < 1 || lStartRowRelative > (int)m_mapAllRowData.size() 
			|| lEndRowRelative < 1 || lStartRowRelative > (int)m_mapAllRowData.size() )
	{
		return;
	}

	if( lStartRowRelative > lEndRowRelative )
	{
		long lTemp = lEndRowRelative;
		lEndRowRelative = lStartRowRelative;
		lStartRowRelative = lTemp;
	}
	
	// Erase concerned line.
	for( long lLoopRow = lStartRowRelative; lLoopRow <= lEndRowRelative; lLoopRow++ )
	{
		mapLongRowParamsIter iter = m_mapAllRowData.find( lLoopRow );

		if( iter != m_mapAllRowData.end() )
		{
			m_mapAllRowData.erase( iter );
		}
	}

	// Update number after the removed ones.
	if( m_mapAllRowData.size() > 0 )
	{
		mapLongRowParams mapNew;
		long lCount = 1;

		for( mapLongRowParamsIter iter = m_mapAllRowData.begin(); iter != m_mapAllRowData.end(); ++iter )
		{
			mapNew[lCount] = iter->second;
			mapNew[lCount].m_lRow = lCount;
			lCount++;
		}
	
		m_mapAllRowData = mapNew;
	}
}

bool CDlgBatchSelBase::OnBatchSelectionGetColumnList( CDlgBatchSelectionOutput::mapColData &mapColumnList )
{
	mapColumnList.clear();

	if( 0 == m_mapColumnList.size() )
	{
		return true;
	}

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); ++iter )
	{
		if( true == iter->second.m_bCantRemove )
		{
			continue;
		}

		CDlgBatchSelectionOutput::ColData rColData;
		rColData.m_lColumnID = iter->second.m_iColumnID;
		rColData.m_strName = iter->second.m_strHeaderName;
		rColData.m_bAlreadyDisplayed = iter->second.m_bDisplayed;
		mapColumnList[iter->first] = rColData;
	}

	return true;
}

void CDlgBatchSelBase::OnBatchSelectionAddColumn( int iColumnID )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_pclBatchSelParams->m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( -1 != iColumnID )
	{
		// Add only one column.

		// If column ID doesn't exist...
		if( 0 == m_mapColumnList.count( iColumnID ) )
		{
			return;
		}

		// If already displayed...
		if( true == m_mapColumnList[iColumnID].m_bDisplayed )
		{
			return;
		}

		// Add the column.
		BS_AddOneColumn( iColumnID );

		m_mapColumnList[iColumnID].m_eb3UserChoice = eBool3::eb3True;
	}
	else
	{
		// Add all columns.
		mapLongColParamsIter iter = m_mapColumnList.find( BS_ColumnID::BS_CID_Input_Last );

		if( iter != m_mapColumnList.end() )
		{
			for( ; iter != m_mapColumnList.end(); ++iter )
			{
				if( false == iter->second.m_bDisplayed && true == iter->second.m_bEnabled )
				{
					BS_AddOneColumn( iter->first );
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3True;
				}
			}
		}
	}

	_BSResizingColumnInit();

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}
}

void CDlgBatchSelBase::OnBatchSelectionRemoveColumn( int iColumnID )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_pclBatchSelParams->m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( -1 != iColumnID )
	{
		// Remove one column.
		if( 0 == m_mapColumnList.count( iColumnID ) )
		{
			return;
		}

		if( false == m_mapColumnList[iColumnID].m_bDisplayed )
		{
			return;
		}

		BS_RemoveOneColumn( iColumnID );
		m_mapColumnList[iColumnID].m_eb3UserChoice = eBool3::eb3False;
	}
	else
	{
		// Remove all columns.
		mapLongColParamsIter iter = m_mapColumnList.find( BS_ColumnID::BS_CID_Input_Last );

		if( iter != m_mapColumnList.end() )
		{
			for( ; iter != m_mapColumnList.end(); ++iter )
			{
				if( true == iter->second.m_bDisplayed && false == iter->second.m_bCantRemove )
				{
					BS_RemoveOneColumn( iter->first );
					m_mapColumnList[iter->first].m_eb3UserChoice = eBool3::eb3False;
				}
			}
		}
	}

	_BSResizingColumnInit();

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}
}

bool CDlgBatchSelBase::OnBatchSelectionKeyboardVirtualKey( int iVirtualKey )
{
	// By default, the caller can execute the key.
	bool bExecuteKey = true;

	switch( iVirtualKey )
	{
		case VK_DELETE:

			// If suggestion is already done, do no action with 'DELETE' key.
			if( false == m_bInitialized || BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
			{
				bExecuteKey = false;
			}

			break;

		case VK_RETURN:
		{
			int iStartColID, iEndColID;
			long lStartRow, lEndRow;

			if( false == m_clInterface.GetBlockSelection( iStartColID, lStartRow, iEndColID, lEndRow, (int)m_pclBatchSelParams->m_eProductSubCategory ) )
			{
				break;
			}

			if( lStartRow != lEndRow || lStartRow < 1 )
			{
				break;
			}

			if( true == OnDlgOutputCellDblClicked( (int)m_pclBatchSelParams->m_eProductSubCategory, iStartColID, lStartRow ) )
			{
				bExecuteKey = false;
			}
		}
		break;
	}

	return bExecuteKey;
}

bool CDlgBatchSelBase::OnBatchSelectionIsDataExist( bool &bDataExist, bool &bDataValidated )
{
	bDataExist = ( BS_InputsVerificationStatus::BS_IVS_OK == m_eInputsVerificationStatus
				   || BS_InputsVerificationStatus::BS_IVS_Error == m_eInputsVerificationStatus ) ? true : false;

	bDataValidated = ( true == bDataExist && BS_InputsVerificationStatus::BS_IVS_Error != m_eInputsVerificationStatus );
	
	return true;
}

bool CDlgBatchSelBase::OnBatchSelectionIsResultExist( bool &bResultExist, bool &bResultValidated )
{
	bResultExist = ( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus ) ? true : false;
	bResultValidated = ( BS_ValidationStatus::BS_VS_Done == m_eValidationStatus ) ? true : false;
	return true;
}

bool CDlgBatchSelBase::OnBatchSelectionIsCatalogExist( int iColumnID, long lRowRelative, bool &bCatExist, CData *&pclProduct )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}
	
	bCatExist = false;
	pclProduct = NULL;

	if( BS_SuggestionStatus::BS_SS_Done != m_eSuggestionStatus )
	{
		return false;
	}

	if( 0 == m_mapColumnList.count( iColumnID ) || 0 == m_mapAllRowData.count( lRowRelative ) )
	{
		return false;
	}

	return true;
}

bool CDlgBatchSelBase::OnBatchSelectionDelete( int iStartColID, int iEndColID, long lStartRowRelative, long lEndRowRelative )
{
	if( iStartColID > iEndColID )
	{
		int iTemp = iStartColID;
		iStartColID = iEndColID;
		iEndColID = iTemp;
	}

	bool bExecute = false;

	for( int iLoopCol = iStartColID; iLoopCol <= iEndColID && false == bExecute; iLoopCol++ )
	{
		if( iLoopCol < BS_CID_Input_Flow || iLoopCol >= BS_CID_Input_Separator )
		{
			continue;
		}

		bExecute = true;
	}

	if( true == bExecute )
	{
		BS_VerifyAllDataInput();
	}

	return true;
}

bool CDlgBatchSelBase::OnBatchSelectionEdit( long lRowRelative )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	// Verify.
	if( BS_SuggestionStatus::BS_SS_Done != m_eSuggestionStatus )
	{
		return false;
	}

	return OnDlgOutputCellDblClicked( (int)m_pclBatchSelParams->m_eProductSubCategory, -1, lRowRelative );
}

bool CDlgBatchSelBase::OnBatchSelectionSuggest()
{
	// Verify.
	if( FALSE == m_clButtonSuggest.IsWindowEnabled() )
	{
		return false;
	}

	OnBnClickedSuggest();
	return true;
}

bool CDlgBatchSelBase::OnBatchSelectionValidate()
{
	// Verify.
	if( FALSE == m_clButtonValidate.IsWindowEnabled() )
	{
		return false;
	}

	OnBnClickedValidate();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelBase, CDlgSelectionBase )
	ON_WM_DESTROY()

	ON_BN_CLICKED( IDC_BUTTONSUGGEST, OnBnClickedSuggest )
	ON_BN_CLICKED( IDC_BUTTONVALIDATE, OnBnClickedValidate )
	ON_BN_CLICKED( IDC_BUTTONCANCEL, OnBnClickedCancel )

	ON_CBN_SELCHANGE( IDC_COMBOTYPE1, OnCbnSelChangeTypeBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY1, OnCbnSelChangeFamilyBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL1, OnCbnSelChangeBdyMatBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT1, OnCbnSelChangeConnectBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION1, OnCbnSelChangeVersionBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOPN1, OnCbnSelChangePNBelow65 )

	ON_CBN_SELCHANGE( IDC_COMBOTYPE2, OnCbnSelChangeTypeAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY2, OnCbnSelChangeFamilyAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL2, OnCbnSelChangeBdyMatAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT2, OnCbnSelChangeConnectAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION2, OnCbnSelChangeVersionAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOPN2, OnCbnSelChangePNAbove50 )

	ON_BN_CLICKED( IDC_RADIOFLOW, OnBnClickedRadioFlowPower )
	ON_BN_CLICKED( IDC_RADIOPOWERDT, OnBnClickedRadioFlowPower )

END_MESSAGE_MAP()

void CDlgBatchSelBase::DoDataExchange( CDataExchange *pDX )
{
	CDlgSelectionBase::DoDataExchange( pDX );

	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );

	// Because some of inherited classes have not this control in their interface (CDlgBatchSelSeparator), we must verify
	// first if control exist.
	if( NULL != GetDlgItem( IDC_COMBOTYPE1 ) )
	{
		DDX_Control( pDX, IDC_COMBOTYPE1, m_ComboTypeBelow65 );
	}

	if( NULL != GetDlgItem( IDC_COMBOFAMILY1 ) )
	{
		DDX_Control( pDX, IDC_COMBOFAMILY1, m_ComboFamilyBelow65 );
	}

	if( NULL != GetDlgItem( IDC_COMBOMATERIAL1 ) )
	{
		DDX_Control( pDX, IDC_COMBOMATERIAL1, m_ComboBodyMaterialBelow65 );
	}

	if( NULL != GetDlgItem( IDC_COMBOCONNECT1 ) )
	{
		DDX_Control( pDX, IDC_COMBOCONNECT1, m_ComboConnectBelow65 );
	}

	if( NULL != GetDlgItem( IDC_COMBOVERSION1 ) )
	{
		DDX_Control( pDX, IDC_COMBOVERSION1, m_ComboVersionBelow65 );
	}

	if( NULL != GetDlgItem( IDC_COMBOPN1 ) )
	{
		DDX_Control( pDX, IDC_COMBOPN1, m_ComboPNBelow65 );
	}

	if( NULL != GetDlgItem( IDC_COMBOTYPE2 ) )
	{
		DDX_Control( pDX, IDC_COMBOTYPE2, m_ComboTypeAbove50 );
	}

	if( NULL != GetDlgItem( IDC_COMBOFAMILY2 ) )
	{
		DDX_Control( pDX, IDC_COMBOFAMILY2, m_ComboFamilyAbove50 );
	}

	if( NULL != GetDlgItem( IDC_COMBOMATERIAL2 ) )
	{
		DDX_Control( pDX, IDC_COMBOMATERIAL2, m_ComboBodyMaterialAbove50 );
	}

	if( NULL != GetDlgItem( IDC_COMBOCONNECT2 ) )
	{
		DDX_Control( pDX, IDC_COMBOCONNECT2, m_ComboConnectAbove50 );
	}

	if( NULL != GetDlgItem( IDC_COMBOVERSION2 ) )
	{
		DDX_Control( pDX, IDC_COMBOVERSION2, m_ComboVersionAbove50 );
	}

	if( NULL != GetDlgItem( IDC_COMBOPN2 ) )
	{
		DDX_Control( pDX, IDC_COMBOPN2, m_ComboPNAbove50 );
	}

	DDX_Control( pDX, IDC_GROUPINPUTS, m_clGroupInputs );

	if( NULL != GetDlgItem( IDC_GROUPVALVE1 ) )
	{
		DDX_Control( pDX, IDC_GROUPVALVE1, m_clGroupValveBelow65 );
	}

	if( NULL != GetDlgItem( IDC_GROUPVALVE2 ) )
	{
		DDX_Control( pDX, IDC_GROUPVALVE2, m_clGroupValveAbove50 );
	}

	DDX_Control( pDX, IDC_BUTTONSUGGEST, m_clButtonSuggest );
	DDX_Control( pDX, IDC_BUTTONVALIDATE, m_clButtonValidate );
	DDX_Control( pDX, IDC_BUTTONCANCEL, m_clButtonCancel );
}

BOOL CDlgBatchSelBase::OnInitDialog()
{
	CDlgSelectionBase::OnInitDialog();
	
	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	m_iDN50 = pclTableDN->GetSize( _T("DN_50") );
	m_iDN65 = pclTableDN->GetSize( _T("DN_65") );

	// Set proper style and add icons for the 'Flow - Power/DT' group.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupInputs.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pen );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_clGroupInputs.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Pen, true );
	}

	m_clGroupInputs.SetInOffice2007Mainframe( true );
	m_clGroupInputs.SaveChildList();

	if( NULL != GetpGroupValveBelow65() )
	{
		if( NULL != pclImgListGroupBox )
		{
			GetpGroupValveBelow65()->SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
		}

		pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
		ASSERT( NULL != pclImgListGroupBoxGrayed );

		if( NULL != pclImgListGroupBoxGrayed )
		{
			GetpGroupValveBelow65()->SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Valve, true );
		}

		GetpGroupValveBelow65()->SetInOffice2007Mainframe( true );
		GetpGroupValveBelow65()->SaveChildList();
	}

	if( NULL != GetpGroupValveAbove50() )
	{
		if( NULL != pclImgListGroupBox )
		{
			GetpGroupValveAbove50()->SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
		}

		pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
		ASSERT( NULL != pclImgListGroupBoxGrayed );

		if( NULL != pclImgListGroupBoxGrayed )
		{
			GetpGroupValveAbove50()->SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Valve, true );
		}

		GetpGroupValveAbove50()->SetInOffice2007Mainframe( true );
		GetpGroupValveAbove50()->SaveChildList();
	}

	CString str;
	str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_FLOW );
	GetDlgItem( IDC_RADIOFLOW )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_POWERDT );
	GetDlgItem( IDC_RADIOPOWERDT )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_BUTTONSUGGEST );
	m_clButtonSuggest.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_BUTTONVALIDATE );
	m_clButtonValidate.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_BUTTONCANCEL );
	m_clButtonCancel.SetWindowText( str );

	if( NULL != GetpGroupValveBelow65() )
	{
		str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_STATICBELOW65 );
		m_clGroupValveBelow65.SetWindowText( str );
	}

	if( NULL != GetpGroupValveAbove50() )
	{
		str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_STATICABOVE65 );
		m_clGroupValveAbove50.SetWindowText( str );
	}

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICTYPE );

	if( NULL != GetDlgItem( IDC_STATICTYPE1 ) )
	{
		GetDlgItem( IDC_STATICTYPE1 )->SetWindowText( str );
	}

	if( NULL != GetDlgItem( IDC_STATICTYPE2 ) )
	{
		GetDlgItem( IDC_STATICTYPE2 )->SetWindowText( str );
	}

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICFAM );

	if( NULL != GetDlgItem( IDC_STATICFAMILY1 ) )
	{
		GetDlgItem( IDC_STATICFAMILY1 )->SetWindowText( str );
	}

	if( NULL != GetDlgItem( IDC_STATICFAMILY2 ) )
	{
		GetDlgItem( IDC_STATICFAMILY2 )->SetWindowText( str );
	}

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICBDYMAT );

	if( NULL != GetDlgItem( IDC_STATICMATERIAL1 ) )
	{
		GetDlgItem( IDC_STATICMATERIAL1 )->SetWindowText( str );
	}

	if( NULL != GetDlgItem( IDC_STATICMATERIAL2 ) )
	{
		GetDlgItem( IDC_STATICMATERIAL2 )->SetWindowText( str );
	}

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICCON );

	if( NULL != GetDlgItem( IDC_STATICCONNECT1 ) )
	{
		GetDlgItem( IDC_STATICCONNECT1 )->SetWindowText( str );
	}

	if( NULL != GetDlgItem( IDC_STATICCONNECT2 ) )
	{
		GetDlgItem( IDC_STATICCONNECT2 )->SetWindowText( str );
	}

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICVER );

	if( NULL != GetDlgItem( IDC_STATICVERSION1 ) )
	{
		GetDlgItem( IDC_STATICVERSION1 )->SetWindowText( str );
	}

	if( NULL != GetDlgItem( IDC_STATICVERSION2 ) )
	{
		GetDlgItem( IDC_STATICVERSION2 )->SetWindowText( str );
	}

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICPN );

	if( NULL != GetDlgItem( IDC_STATICPN1 ) )
	{
		GetDlgItem( IDC_STATICPN1 )->SetWindowText( str );
	}

	if( NULL != GetDlgItem( IDC_STATICPN2 ) )
	{
		GetDlgItem( IDC_STATICPN2 )->SetWindowText( str );
	}

	return TRUE;
}

void CDlgBatchSelBase::OnCbnSelChangeTypeBelow65()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelParams->m_strComboTypeBelow65ID == m_ComboTypeBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboTypeBelow65.FindCBIDPtr( m_pclBatchSelParams->m_strComboTypeBelow65ID ), 0 );

		if( iPos != m_ComboTypeBelow65.GetCurSel() )
		{
			m_ComboTypeBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboTypeBelow65ID = m_ComboTypeBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxFamily( AboveOrBelow::Below65 );
	FillComboBoxBodyMaterial( AboveOrBelow::Below65 );
	FillComboBoxConnect( AboveOrBelow::Below65 );
	FillComboBoxVersion( AboveOrBelow::Below65 );
	FillComboBoxPN( AboveOrBelow::Below65 );
	
	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeFamilyBelow65()
{
	if( m_pclBatchSelParams->m_strComboFamilyBelow65ID == m_ComboFamilyBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboFamilyBelow65.FindCBIDPtr( m_pclBatchSelParams->m_strComboFamilyBelow65ID ), 0 );

		if( iPos != m_ComboFamilyBelow65.GetCurSel() )
		{
			m_ComboFamilyBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboFamilyBelow65ID = m_ComboFamilyBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxBodyMaterial( AboveOrBelow::Below65 );
	FillComboBoxConnect( AboveOrBelow::Below65 );
	FillComboBoxVersion( AboveOrBelow::Below65 );
	FillComboBoxPN( AboveOrBelow::Below65 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeBdyMatBelow65()
{
	if( m_pclBatchSelParams->m_strComboMaterialBelow65ID == m_ComboBodyMaterialBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboBodyMaterialBelow65.FindCBIDPtr( m_pclBatchSelParams->m_strComboMaterialBelow65ID ), 0 );

		if( iPos != m_ComboBodyMaterialBelow65.GetCurSel() )
		{
			m_ComboBodyMaterialBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboMaterialBelow65ID = m_ComboBodyMaterialBelow65.GetCBCurSelIDPtr().ID;
	FillComboBoxConnect( AboveOrBelow::Below65 );
	FillComboBoxVersion( AboveOrBelow::Below65 );
	FillComboBoxPN( AboveOrBelow::Below65 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeConnectBelow65()
{
	if( m_pclBatchSelParams->m_strComboConnectBelow65ID == m_ComboConnectBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboConnectBelow65.FindCBIDPtr( m_pclBatchSelParams->m_strComboConnectBelow65ID ), 0 );

		if( iPos != m_ComboConnectBelow65.GetCurSel() )
		{
			m_ComboConnectBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboConnectBelow65ID = m_ComboConnectBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxVersion( AboveOrBelow::Below65 );
	FillComboBoxPN( AboveOrBelow::Below65 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeVersionBelow65()
{
	if( m_pclBatchSelParams->m_strComboVersionBelow65ID == m_ComboVersionBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboVersionBelow65.FindCBIDPtr( m_pclBatchSelParams->m_strComboVersionBelow65ID ), 0 );

		if( iPos != m_ComboVersionBelow65.GetCurSel() )
		{
			m_ComboVersionBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboVersionBelow65ID = m_ComboVersionBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxPN( AboveOrBelow::Below65 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangePNBelow65()
{
	if( m_pclBatchSelParams->m_strComboPNBelow65ID == m_ComboPNBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// Remark: 'true' to signal that it is the last combo in the chain to be changed.
	//         It allows us to reset the 'm_prBatchSelectionVariables->m_fMsgComboChangeDone' variable to false.
	if( false == BS_ComboChange( true ) )
	{
		int iPos = max( m_ComboPNBelow65.FindCBIDPtr( m_pclBatchSelParams->m_strComboPNBelow65ID ), 0 );

		if( iPos != m_ComboPNBelow65.GetCurSel() )
		{
			m_ComboPNBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboPNBelow65ID = m_ComboPNBelow65.GetCBCurSelIDPtr().ID;

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeTypeAbove50()
{
	if( m_pclBatchSelParams->m_strComboTypeAbove50ID == m_ComboTypeAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboTypeAbove50.FindCBIDPtr( m_pclBatchSelParams->m_strComboTypeAbove50ID ), 0 );

		if( iPos != m_ComboTypeAbove50.GetCurSel() )
		{
			m_ComboTypeAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboTypeAbove50ID = m_ComboTypeAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxFamily( AboveOrBelow::Above50 );
	FillComboBoxBodyMaterial( AboveOrBelow::Above50 );
	FillComboBoxConnect( AboveOrBelow::Above50 );
	FillComboBoxVersion( AboveOrBelow::Above50 );
	FillComboBoxPN( AboveOrBelow::Above50 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeFamilyAbove50()
{
	if( m_pclBatchSelParams->m_strComboFamilyAbove50ID == m_ComboFamilyAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboFamilyAbove50.FindCBIDPtr( m_pclBatchSelParams->m_strComboFamilyAbove50ID ), 0 );

		if( iPos != m_ComboFamilyAbove50.GetCurSel() )
		{
			m_ComboFamilyAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboFamilyAbove50ID = m_ComboFamilyAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxBodyMaterial( AboveOrBelow::Above50 );
	FillComboBoxConnect( AboveOrBelow::Above50 );
	FillComboBoxVersion( AboveOrBelow::Above50 );
	FillComboBoxPN( AboveOrBelow::Above50 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeBdyMatAbove50()
{
	if( m_pclBatchSelParams->m_strComboMaterialAbove50ID == m_ComboBodyMaterialAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboBodyMaterialAbove50.FindCBIDPtr( m_pclBatchSelParams->m_strComboMaterialAbove50ID ), 0 );

		if( iPos != m_ComboBodyMaterialAbove50.GetCurSel() )
		{
			m_ComboBodyMaterialAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboMaterialAbove50ID = m_ComboBodyMaterialAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxConnect( AboveOrBelow::Above50 );
	FillComboBoxVersion( AboveOrBelow::Above50 );
	FillComboBoxPN( AboveOrBelow::Above50 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeConnectAbove50()
{
	if( m_pclBatchSelParams->m_strComboConnectAbove50ID == m_ComboConnectAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboConnectAbove50.FindCBIDPtr( m_pclBatchSelParams->m_strComboConnectAbove50ID ), 0 );

		if( iPos != m_ComboConnectAbove50.GetCurSel() )
		{
			m_ComboConnectAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboConnectAbove50ID = m_ComboConnectAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxVersion( AboveOrBelow::Above50 );
	FillComboBoxPN( AboveOrBelow::Above50 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangeVersionAbove50()
{
	if( m_pclBatchSelParams->m_strComboVersionAbove50ID == m_ComboVersionAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == BS_ComboChange() )
	{
		int iPos = max( m_ComboVersionAbove50.FindCBIDPtr( m_pclBatchSelParams->m_strComboVersionAbove50ID ), 0 );

		if( iPos != m_ComboVersionAbove50.GetCurSel() )
		{
			m_ComboVersionAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboVersionAbove50ID = m_ComboVersionAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxPN( AboveOrBelow::Above50 );

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnCbnSelChangePNAbove50()
{
	if( m_pclBatchSelParams->m_strComboPNAbove50ID == m_ComboPNAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	// Remark: 'true' to signal that it is the last combo in the chain to be changed.
	//         It allows us to reset the 'm_prBatchSelectionVariables->m_fMsgComboChangeDone' variable to false.
	if( false == BS_ComboChange( true ) )
	{
		int iPos = max( m_ComboPNAbove50.FindCBIDPtr( m_pclBatchSelParams->m_strComboPNAbove50ID ), 0 );

		if( iPos != m_ComboPNAbove50.GetCurSel() )
		{
			m_ComboPNAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelParams->m_strComboPNAbove50ID = m_ComboPNAbove50.GetCBCurSelIDPtr().ID;

	// Notify the inherited classes that the treatment for the combo change is finished.
	EndOfComboChange();

	CheckIfWeNeedToClearResults();
}

void CDlgBatchSelBase::OnBnClickedValidate()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	if( true == m_pclBatchSelParams->m_bEditModeRunning )
	{
		return;
	}

	// We are not in edition mode, thus we validate the batch selection.
	if( true == BS_Validate() )
	{
		BS_EnableCombos( false );
		m_clButtonValidate.EnableWindow( FALSE );
		m_eValidationStatus = BS_ValidationStatus::BS_VS_Done;
		CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_BSVALIDATEOK );
		AfxMessageBox( str, MB_OK, 0 );
	}
}

void CDlgBatchSelBase::OnBnClickedRadioFlowPower()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	UpdateFlowOrPowerDTState();

	if( 0 == m_mapAllRowData.size()
			|| BS_InputsVerificationStatus::BS_IVS_Empty == m_eInputsVerificationStatus
			|| BS_InputsVerificationStatus::BS_IVS_NotYetDone == m_eInputsVerificationStatus )
	{
		// When there is not yet data pasted in the batch selection output, user can change input method (Flow or Power/DT and Dp or not).
		BS_UpdateIOColumnLayout( BS_UpdateInputColumn::BS_UIC_FlowOrPowerDT );
	}
}

LRESULT CDlgBatchSelBase::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	CDlgSelectionBase::OnNewDocument( wParam, lParam );

	CDS_TechnicalParameter *pTechParameter = m_pclBatchSelParams->m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechParameter );

	if( NULL != pTechParameter )
	{
		m_rTechParamSaved.m_dTargetLinearDp = pTechParameter->GetPipeTargDp();
		m_rTechParamSaved.m_dMaxLinearDp = pTechParameter->GetPipeMaxDp();
		m_rTechParamSaved.m_dTargetWaterVelocity = pTechParameter->GetPipeTargVel();
		m_rTechParamSaved.m_dMaxWaterVelocity = pTechParameter->GetPipeMaxVel();
		m_rTechParamSaved.m_iTargetWaterVelocity = pTechParameter->GetPipeVtargUsed();
		m_rTechParamSaved.m_iSizeShiftAbove = pTechParameter->GetSizeShiftAbove();
		m_rTechParamSaved.m_iSizeShiftBelow = pTechParameter->GetSizeShiftBelow();
	}

	ClearAll();
	BS_ClearAllData( false );

	// Prepare list of all needed columns.
	BS_PrepareColumnList();

	SetModificationMode( false );

	EnableSuggestButton( false );
	BS_EnableRadios( true );
	BS_EnableCombos( true );

	m_bInitialized = false;
	return 0;
}

LRESULT CDlgBatchSelBase::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	if( WMUserPipeChange::WM_UPC_ForProductSelection != ( WMUserPipeChange )wParam )
	{
		return 0;
	}

	CDS_TechnicalParameter *pTechParameter = m_pclBatchSelParams->m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechParameter );

	if( NULL == pTechParameter )
	{
		return 0;
	}

	// If batch selection is well initialized and results have already been suggested but not yet validated...
	if( true == m_bInitialized && BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus
		&& BS_ValidationStatus::BS_VS_NotYetDone == m_eValidationStatus )
	{
		// We clear result because they no more match with the new parameters.
		OnBatchSelectionButtonClearResults();

		// We automatic suggest only if the window is visible.
		if( TRUE == IsWindowVisible() )
		{
			OnBnClickedSuggest();
		}
	}

	return 0;
}

LRESULT CDlgBatchSelBase::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	// If batch selection is well initialized and results has already been suggested but not yet validated...
	if( true == m_bInitialized &&  BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus 
		&& BS_ValidationStatus::BS_VS_NotYetDone == m_eValidationStatus )
	{
		// We clear result because they no more match with the new parameters.
		OnBatchSelectionButtonClearResults();

		// We automatic suggest only if the window is visible.
		if( TRUE == IsWindowVisible() )
		{
			OnBnClickedSuggest();
		}
	}

	return 0;
}

LRESULT CDlgBatchSelBase::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( -1 );
	}

	// If batch selection is well initialized and results have already been suggested but not yet validated...
	if( true == m_bInitialized && BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus &&
		BS_ValidationStatus::BS_VS_NotYetDone == m_eValidationStatus )
	{
		m_clInterface.ClearOutput( false, (int)m_pclBatchSelParams->m_eProductSubCategory );
		OnBatchSelectionButtonClearAll();
	}

	if( true == m_bInitialized )
	{
		// Change unit in the headers and clear all data if exist.
		BS_UpdateUnit();
	}

	return 0;
}

LRESULT CDlgBatchSelBase::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type.
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( ( WMUserWaterCharWParam )wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) 
			|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam 
			|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return -1;
	}

	CDlgSelectionBase::OnWaterChange( wParam, lParam );

	// If batch selection is well initialized and results have already been suggested but not yet validated...
	if( true == m_bInitialized && BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus 
			&& BS_ValidationStatus::BS_VS_NotYetDone == m_eValidationStatus )
	{
		// We clear result because they no more match with the new parameters.
		OnBatchSelectionButtonClearResults();

		// We automatic suggest only if the window is visible.
		if( TRUE == IsWindowVisible() )
		{
			OnBnClickedSuggest();
		}
	}

	return 0;
}

void CDlgBatchSelBase::OnAfterNewDocument()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	// Remark: I can't let this part of code in the 'OnNewDocument'. Because the '_BSFirstTimeInit' needs that the inherited class has already
	//         set its 'Flow/Power' radio state and eventually the 'Dp' checkbox.
	if( TRUE == IsWindowVisible() )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
		}

		m_clInterface.ClearOutput( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableCtxtMenuPasteData( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableCtxtMenuAddColumn( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, false );
		m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, false );
		_BSFirstTimeInit();

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
		}
	}

	// Change the tools dockable pane to full screen mode.
	bool bApply = ( TRUE == IsWindowVisible() ) ? true : false;

	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->SetToolsDockablePaneFullScreenMode( true, bApply );
	}
}

void CDlgBatchSelBase::SetModificationMode( bool bFlag )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	if( false == bFlag )
	{
		m_pclBatchSelParams->m_SelIDPtr = _NULL_IDPTR;
		m_clButtonValidate.SetWindowText( TASApp.LoadLocalizedString( IDS_BATCHSELBASE_BUTTONVALIDATE ) );
		m_clButtonValidate.EnableWindow( FALSE );

		// Enable 'Validate' button only if results have been suggested and there either one solution or one alternative.
		if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus 
			&& ( true == m_bAtLeastOneRowWithBestSolution || true == m_bAtLeastOneRowWithSolutionButAlternative ) )
		{
			m_clButtonValidate.EnableWindow( TRUE );
		}

		m_clButtonCancel.ShowWindow( SW_HIDE );
		m_pclBatchSelParams->m_bEditModeRunning = false;
	}
	else
	{
		m_clButtonValidate.SetWindowText( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_APPLYMODIFICATION ) );
		m_clButtonValidate.EnableWindow( ( m_pclBatchSelParams->m_SelIDPtr != _NULL_IDPTR ) ? TRUE : FALSE );
		m_clButtonCancel.ShowWindow( SW_SHOW );
		m_clButtonCancel.EnableWindow( TRUE );
		m_pclBatchSelParams->m_bEditModeRunning = true;
	}
}

void CDlgBatchSelBase::EnableSuggestButton( bool bEnable )
{
	m_clButtonSuggest.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
}

void CDlgBatchSelBase::BS_ClearAllData( bool bForceDelete )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	// Reset all results.
	if( 0 != m_mapAllRowData.size() )
	{
		for( mapLongRowParamsIter iter = m_mapAllRowData.begin(); iter != m_mapAllRowData.end(); ++iter )
		{
			if( NULL != iter->second.m_pclBatchResults )
			{
				delete iter->second.m_pclBatchResults;
			}

			iter->second.m_pclBatchResults = NULL;

			if( NULL != iter->second.m_pclCDSSelSelected )
			{
				CTable *pTab = m_pclBatchSelParams->m_pTADS->GetpClipboardTable();
				ASSERT( NULL != pTab );

				IDPTR IDPtr = iter->second.m_pclCDSSelSelected->GetIDPtr();

				if( NULL != pTab && _T('\0') == *IDPtr.ID )
				{
					pTab->Remove( IDPtr );
					m_pclBatchSelParams->m_pTADS->DeleteObject( IDPtr );
				}
			}

			iter->second.m_pclCDSSelSelected = NULL;
			iter->second.m_pclSelectedProduct = NULL;
			iter->second.m_pclSelectedSecondaryProduct = NULL;
			iter->second.m_pclSelectedActuator = NULL;
		}

		m_mapAllRowData.clear();
	}

	m_bAtLeastOneRowWithNoSolution = false;
	m_bAtLeastOneRowWithSolutionButNotBest = false;
	m_bAtLeastOneRowWithNoSolution = false;
	m_bMsgComboChangeDone = false;
	m_bMsgDoNotClearResult = false;
	m_eInputsVerificationStatus = BS_InputsVerificationStatus::BS_IVS_NotYetDone;
	m_eSuggestionStatus = BS_SuggestionStatus::BS_SS_NotYetDone;
	m_eValidationStatus = BS_ValidationStatus::BS_VS_NotYetDone;
	m_pclCurrentRowParameters = NULL;
}

void CDlgBatchSelBase::BS_PrepareColumnList()
{
	BSColParameters rColParameters;

	// Prepare header and data parameters.
	rColParameters.m_clHeaderColDef.SetAllParams( DlgOutputHelper::CColDef::Text, 10.0, BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Input,
			DlgOutputHelper::CColDef::AutoResizeDisabled,
			DlgOutputHelper::CColDef::Visible, DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionDisabled,
			DlgOutputHelper::CColDef::BlockSelectionDisabled,
			DlgOutputHelper::CColDef::RowSelectionDisabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );
	
	rColParameters.m_clHeaderColDef.SetColSeparatorFlag( true, DLGBATCHSELBASE_COLOR_HEADERCOLSEPARATOR );

	rColParameters.m_clHeaderFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Center, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled,
			DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_clDataColDef.SetAllParams( DlgOutputHelper::CColDef::Bitmap, 10.0, BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Input,
			DlgOutputHelper::CColDef::AutoResizeDisabled,
			DlgOutputHelper::CColDef::Visible, DlgOutputHelper::CColDef::MouseEventEnabled, DlgOutputHelper::CColDef::SelectionEnabled,
			DlgOutputHelper::CColDef::BlockSelectionDisabled,
			DlgOutputHelper::CColDef::RowSelectionEnabled, DlgOutputHelper::CColDef::PasteDataDisabled, DlgOutputHelper::CColDef::CopyDataDisabled );
	
	rColParameters.m_clDataColDef.SetRowSeparatorFlag( true, DLGBATCHSELBASE_COLOR_DATAROWSEPARATOR );

	rColParameters.m_clDataFontDef.SetAllParams( _T("Arial Unicode MS"), 8, DlgOutputHelper::CFontDef::HA_Left, DlgOutputHelper::CFontDef::VA_Center,
			DlgOutputHelper::CFontDef::BoldDisabled,
			DlgOutputHelper::CFontDef::ItalicDisabled, DlgOutputHelper::CFontDef::UnderlineDisabled, _WHITE, _BLACK );

	rColParameters.m_iColumnID = BS_CID_Input_Status;
	rColParameters.m_clHeaderColDef.SetMainHeaderLinks( -1, -1 );
	rColParameters.m_clDataColDef.SetMainHeaderLinks( -1, -1 );
	rColParameters.m_bCantRemove = true;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_STATUS );
	rColParameters.m_bLineBelowHeader = true;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_Reference1;
	rColParameters.m_clHeaderColDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Input );
	rColParameters.m_clDataColDef.SetMainHeaderLinks( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Input );
	rColParameters.m_clDataColDef.SetBlockSelectionFlag( true );
	rColParameters.m_clDataColDef.SetPasteDataFlag( true );
	rColParameters.m_clDataColDef.SetCopyDataFlag( true );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_clDataColDef.SetEditableFlag( true );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_REFERENCE1 );
	rColParameters.m_iHeaderUnit = -1;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_Reference2;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_REFERENCE2 );
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_Flow;
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Number );
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_FLOW );
	rColParameters.m_iHeaderUnit = _U_FLOW;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_Power;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_POWER );
	rColParameters.m_iHeaderUnit = _U_TH_POWER;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_DT;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_DT );
	rColParameters.m_iHeaderUnit = _U_DIFFTEMP;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_Dp;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_DP );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_DpBranch;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_DPBRANCH );
	rColParameters.m_iHeaderUnit = _U_DIFFPRESS;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_Kvs;
	rColParameters.m_strHeaderName = TASApp.LoadLocalizedString( IDS_KVS );
	rColParameters.m_iHeaderUnit = _C_KVCVCOEFF;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_FirstColumn;
	rColParameters.m_clHeaderColDef.SetMainHeaderLinks( -1, -1 );
	rColParameters.m_clHeaderColDef.SetWidth( 2 );
	rColParameters.m_clHeaderFontDef.SetTextBackgroundColor( _WHITE );
	rColParameters.m_clDataColDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	rColParameters.m_clDataColDef.SetMainHeaderLinks( -1, -1 );
	rColParameters.m_clDataColDef.SetSelectionFlag( false );
	rColParameters.m_clDataColDef.SetBlockSelectionFlag( false );
	rColParameters.m_clDataColDef.SetPasteDataFlag( false );
	rColParameters.m_clDataColDef.SetCopyDataFlag( false );
	rColParameters.m_clDataColDef.SetEditableFlag( false );
	rColParameters.m_clDataColDef.SetRowSeparatorFlag( false, -1 );
	rColParameters.m_clDataFontDef.SetTextBackgroundColor( _WHITE );
	rColParameters.m_strHeaderName = _T( "" );
	rColParameters.m_iHeaderUnit = -1;
	rColParameters.m_bLineBelowHeader = false;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;

	rColParameters.m_iColumnID = BS_CID_Input_Separator;
	m_mapColumnList[rColParameters.m_iColumnID] = rColParameters;
}

bool CDlgBatchSelBase::BS_InitMainHeaders()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	m_clInterface.AddBlankLineBeforeMainHeader( DLBBATCHSELBASE_ROWHEIGHT_FIRSTLINE, (int)m_pclBatchSelParams->m_eProductSubCategory );

	// Add main header for 'Input'.
	DlgOutputHelper::CColDef clColMHInDef;
	clColMHInDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHInDef.SetMouseEventFlag( true );
	m_clInterface.AddMainHeaderDef( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Input, clColMHInDef,
			TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_BSINPUT ), (int)m_pclBatchSelParams->m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHInDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center, 
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false,	_TAH_ORANGE, _WHITE );

	m_clInterface.AddMainHeaderFontDef( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Input, clFontMHInDef,
			(int)m_pclBatchSelParams->m_eProductSubCategory );

	// Add main header for 'Output'.
	DlgOutputHelper::CColDef clColMHOutDef;
	clColMHOutDef.SetContentType( DlgOutputHelper::CColDef::ContentType::Text );
	clColMHOutDef.SetMouseEventFlag( true );
	m_clInterface.AddMainHeaderDef( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output, clColMHOutDef,
			TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_BSOUTPUT ), (int)m_pclBatchSelParams->m_eProductSubCategory );

	DlgOutputHelper::CFontDef clFontMHOutDef( _T("Verdana"), 9, DlgOutputHelper::CFontDef::HorzAlign::HA_Center,
			DlgOutputHelper::CFontDef::VertAlign::VA_Center, true, false, false, _TAH_ORANGE, _WHITE );

	m_clInterface.AddMainHeaderFontDef( BS_MainHeaderID::BS_MHID_InputOutput, BS_MainHeaderSubID::BS_MHSUBID_Output, clFontMHOutDef,
			(int)m_pclBatchSelParams->m_eProductSubCategory );

	m_clInterface.SetMainHeaderRowHeight( BS_MainHeaderID::BS_MHID_InputOutput, DLBBATCHSELBASE_ROWHEIGHT_MAINHEADER, 
			(int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelBase::BS_InitColumns()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	m_clInterface.SetColumnHeaderRows( 2, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.FreezeAllHeaderRows( true, (int)m_pclBatchSelParams->m_eProductSubCategory );

	BS_AddOneColumn( BS_ColumnID::BS_CID_Input_FirstColumn );
	BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Status );
	BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Reference1 );
	BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Reference2 );

	if( CDS_SelProd::efdFlow == m_pclBatchSelParams->m_eFlowOrPowerDTMode )
	{
		// HYS-1995: Add flow column with correct properties when the last previous mode is PDT mode.
		_UpdateFlowCellProperties( true );
		BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Flow );
	}
	else
	{
		// HYS-1995: Add flow column with correct properties when the current mode is PDT mode.
		_UpdateFlowCellProperties( false );
		BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Flow );
		BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Power );
		BS_AddOneColumn( BS_ColumnID::BS_CID_Input_DT );
	}

	if( true == m_pclBatchSelParams->m_bIsDpGroupChecked )
	{
		BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Dp );
	}

	BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Separator );

	m_clInterface.EnableRowSelection( true, BS_CID_Input_FirstColumn, DLGBATCHSELBASE_SELECTIONBACKCOLOR, (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	return true;
}

void CDlgBatchSelBase::BS_VerifyAllDataInput()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	if( BS_IVS_OK == m_eInputsVerificationStatus )
	{
		return;
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	DlgOutputHelper::mapIntCellBase mapCellData;
	long lCurrentRow;
	bool bResult = m_clInterface.GetFirstRow( &mapCellData, lCurrentRow, (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bResult )
	{
		// There is no data in the sheet.
		return;
	}

	// Prepare list of columns to verify.
	// Remark: Here we verify only cells that are number.
	std::vector<int> vecColumnList;

	for( int iLoopColumn = BS_CID_Input_FirstColumn; iLoopColumn < BS_CID_Input_Separator; iLoopColumn++ )
	{
		if( 0 == m_mapColumnList.count( iLoopColumn ) || false == m_mapColumnList[iLoopColumn].m_bDisplayed 
				|| false == m_mapColumnList[iLoopColumn].m_clDataColDef.IsEditAllowed() 
				|| DlgOutputHelper::CColDef::Number != m_mapColumnList[iLoopColumn].m_clDataColDef.GetContentType() 
				|| 0 == mapCellData.count( iLoopColumn ) )
		{
			continue;
		}

		vecColumnList.push_back( iLoopColumn );
	}

	if( 0 == (int)vecColumnList.size() )
	{
		return;
	}

	// Will contain number of data row (not included headers).
	long lMaxRows;
	m_clInterface.GetRows( lMaxRows, (int)m_pclBatchSelParams->m_eProductSubCategory );

	// Here, we can't use 'SaveAllSelections', 'ClearCurrentSelections' and 'RestoreSelections'. Let's take an example, one line with 'Ref1', 'Ref2' and 'Q'
	// columns. Imagine the row is selected (with blue background) and the cell 'Q' is selected (block selection). User enters an invalid flow (with characters
	// for example). When calling 'SaveAllSelections', all three cells are saved because their property 'CellCantBeSelected' is set to 'false'.
	// After that we reset all background to white and do the verification of inputs. Because the 'Q' cell is not correct, the background is set to red color
	// and the cell 'CellCantBeSelected' property is set to true (why? To allow to keep red background color event if the blue row selection must be painted
	// on the same line). When calling 'RestoreSelections', 'CSSheet' doesn't touch the cell concerned by the error (because flag 'CellCantBeSelected' is set
	// to true). And thus CSSheet internally doesn't memorize the previous background that was normally blue.
	// The solution is to saved only the block selection, clear all, do input verification, reselect the line and restore the block selection.
	long lRowSelected = -1;
	m_clInterface.GetSelectedRow( lRowSelected, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.SaveSelections( CSSheet::SSC_BlockSelection, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_pclBatchSelParams->m_eProductSubCategory );

	// First clear all previous cells marked in red.
	m_clInterface.ChangeCellBackground( BS_CID_Input_Flow, BS_CID_Input_Separator - 1, lCurrentRow,
			lCurrentRow + lMaxRows - 1, DLGBATCHSELBASE_BACKCOLOR_NORMAL, false, (int)m_pclBatchSelParams->m_eProductSubCategory );

	bool bIsError = false;
	bool bIsEmpty = true;

	// Loop on all rows.
	while( true == bResult )
	{
		// Loop on all columns.
		for( std::vector<int>::iterator iterCol = vecColumnList.begin(); iterCol != vecColumnList.end(); ++iterCol )
		{
			ReadDoubleReturn_enum eError = ReadDoubleReturn_enum::RD_EMPTY;

			double dValue;
			eError = mapCellData[*iterCol]->GetNumber( dValue );

			if( ReadDoubleReturn_enum::RD_NOT_NUMBER == eError )
			{
				bIsError = true;
				m_clInterface.ChangeCellBackground( *iterCol, *iterCol, lCurrentRow, lCurrentRow, DLGBATCHSELBASE_BACKCOLOR_DATAERROR, true,
						(int)m_pclBatchSelParams->m_eProductSubCategory );
			}

			if( ReadDoubleReturn_enum::RD_EMPTY != eError )
			{
				bIsEmpty = false;
			}

			// HYS-1995: Manage flow column in PDT mode when using paste action.
			if( *iterCol == BS_ColumnID::BS_CID_Input_Power || *iterCol == BS_ColumnID::BS_CID_Input_DT )
			{
				BS_UpdateFlowCell( -1, lCurrentRow );
			}
		}

		// Save data.
		BSRowParameters clBSRowParameters;
		clBSRowParameters.m_lRow = lCurrentRow;

		// Transfer data for 'mapCellData' to 'clBSRowParameters'.
		BS_ReadInputData( &mapCellData, &clBSRowParameters );
		m_mapAllRowData[lCurrentRow] = clBSRowParameters;

		// Next data.
		DOH_DeleteMapCell( &mapCellData );

		if( lCurrentRow <= lMaxRows )
		{
			bResult = m_clInterface.GetNextRow( &mapCellData, lCurrentRow, (int)m_pclBatchSelParams->m_eProductSubCategory );
		}
		else
		{
			bResult = false;
		}
	}

	if( false == bIsEmpty )
	{
		m_eInputsVerificationStatus = ( false == bIsError ) ? BS_InputsVerificationStatus::BS_IVS_OK :
				BS_InputsVerificationStatus::BS_IVS_Error;

		EnableSuggestButton( ( false == bIsError ) ? true : false );
		BS_EnableRadios( false );
	}
	else
	{
		m_eInputsVerificationStatus = BS_InputsVerificationStatus::BS_IVS_Empty;

		m_clInterface.EnableCtxtMenuPasteData( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableCtxtMenuAddColumn( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, false );
		m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, false );

		EnableSuggestButton( false );
		m_clButtonValidate.EnableWindow( FALSE );

		// To be sure.
		m_clButtonCancel.EnableWindow( FALSE );
		m_clButtonCancel.ShowWindow( SW_HIDE );

		BS_EnableRadios( true );
	}

	if( -1 != lRowSelected )
	{
		m_clInterface.SelectRow( lRowSelected, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	m_clInterface.RestoreSelections( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	bool bEnable = ( 0 != m_mapAllRowData.size() ) ? true : false;
	m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, bEnable );
}

void CDlgBatchSelBase::BS_VerifyOneDataInput( DlgOutputHelper::CCellBase *pclCellToVerify, CDlgBatchSelBase::BSRowParameters *pclBSRowParameters )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclCellToVerify || NULL == pclBSRowParameters )
	{
		return;
	}

	// We verify only if it's a number.
	if( DlgOutputHelper::CColDef::ContentType::Number == pclCellToVerify->GetContentType() )
	{
		BS_InputsVerificationStatus ePreviousIVSStatus = m_eInputsVerificationStatus;

		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked((int)m_pclBatchSelParams->m_eProductSubCategory);

		if (false == bIsRedrawBlocked)
		{
			m_clInterface.BlockRedraw((int)m_pclBatchSelParams->m_eProductSubCategory);
		}
		
		// Here, we can't use 'SaveAllSelections', 'ClearCurrentSelections' and 'RestoreSelections'. Let's take an example, one line with 'Ref1', 'Ref2' and 'Q'
		// columns. Imagine the row is selected (with blue background) and the cell 'Q' is selected (block selection). User enters an invalid flow (with characters
		// for example). When calling 'SaveAllSelections', all three cells are saved because their property 'CellCantBeSelected' is set to 'false'.
		// After that we reset all background to white and do the verification of inputs. Because the 'Q' cell is not correct, the background is set to red color
		// and the cell 'CellCantBeSelected' property is set to true (why? To allow to keep red background color event if the blue row selection must be painted
		// on the same line). When calling 'RestoreSelections', 'CSSheet' doesn't touch the cell concerned by the error (because flag 'CellCantBeSelected' is set
		// to true). And thus CSSheet internally doesn't memorize the previous background that was normally blue.
		// The solution is to saved only the block selection, clear all, do input verification, reselect the line and restore the block selection.
		long lRowSelected = -1;
		m_clInterface.GetSelectedRow( lRowSelected, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.SaveSelections( CSSheet::SSC_BlockSelection, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_pclBatchSelParams->m_eProductSubCategory );

		// First clear the cell if it was previously in red.
		m_clInterface.ChangeCellBackground( pclCellToVerify->GetColumnID(), pclCellToVerify->GetColumnID(), pclBSRowParameters->m_lRow,
				pclBSRowParameters->m_lRow, DLGBATCHSELBASE_BACKCOLOR_NORMAL, false, (int)m_pclBatchSelParams->m_eProductSubCategory );

		bool bIsError = false;
		bool bIsEmpty = true;
		double dValue;
		ReadDoubleReturn_enum eError = pclCellToVerify->GetNumber( dValue );

		if( ReadDoubleReturn_enum::RD_NOT_NUMBER == eError )
		{
			bIsError = true;

			m_clInterface.ChangeCellBackground( pclCellToVerify->GetColumnID(), pclCellToVerify->GetColumnID(), pclBSRowParameters->m_lRow, 
					pclBSRowParameters->m_lRow, DLGBATCHSELBASE_BACKCOLOR_DATAERROR, true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		}

		if (ReadDoubleReturn_enum::RD_EMPTY != eError)
		{
			bIsEmpty = false;
		}

		if( false == bIsEmpty )
		{
			// If there was already an error with a previous data input, we don't change the status.
			if( BS_InputsVerificationStatus::BS_IVS_Error != ePreviousIVSStatus )
			{
				m_eInputsVerificationStatus = (false == bIsError) ? BS_InputsVerificationStatus::BS_IVS_OK :
						BS_InputsVerificationStatus::BS_IVS_Error;

				EnableSuggestButton( ( false == bIsError ) ? true : false );
				BS_EnableRadios( false );
			}
		}
		else
		{
			// If it's the first value to be input and it is empty...
			if( BS_InputsVerificationStatus::BS_IVS_NotYetDone == ePreviousIVSStatus )
			{
				m_clInterface.EnableCtxtMenuPasteData( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
				m_clInterface.EnableCtxtMenuAddColumn( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
				m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, false );
				m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, false );

				EnableSuggestButton( false );
				m_clButtonValidate.EnableWindow( FALSE );

				// To be sure.
				m_clButtonCancel.EnableWindow( FALSE );
				m_clButtonCancel.ShowWindow( SW_HIDE );

				BS_EnableRadios( true );
			}
		}

		if( -1 != lRowSelected )
		{
			m_clInterface.SelectRow( lRowSelected, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
		}

		m_clInterface.RestoreSelections( (int)m_pclBatchSelParams->m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
		}

		bool bEnable = ( 0 != m_mapAllRowData.size() ) ? true : false;
		m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, bEnable );
	}

	// Now we can transfer data in 'pclBSRowParameters'.
	_BS_ReadOneInputData( pclCellToVerify, pclBSRowParameters );
}

void CDlgBatchSelBase::BS_ReadInputData( DlgOutputHelper::mapIntCellBase *pmapCellData, CDlgBatchSelBase::BSRowParameters *pclBSRowParameters )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	if( NULL == pmapCellData || NULL == pclBSRowParameters )
	{
		return;
	}
	
	pclBSRowParameters->m_strReference1 = _T("");
	pclBSRowParameters->m_strReference2 = _T("");
	pclBSRowParameters->m_dFlow = 0.0;
	pclBSRowParameters->m_dDp = 0.0;
	pclBSRowParameters->m_dPower = 0.0;
	pclBSRowParameters->m_dDT = 0.0;
	pclBSRowParameters->m_dKvs = 0.0;
	pclBSRowParameters->m_dDpBranch = 0.0;

	for( int iLoopColumn = BS_CID_Input_FirstColumn; iLoopColumn < BS_CID_Input_Separator; iLoopColumn++ )
	{
		if( 0 == m_mapColumnList.count( iLoopColumn ) || 0 == pmapCellData->count( iLoopColumn )
				|| false == m_mapColumnList[iLoopColumn].m_bDisplayed 
				|| false == m_mapColumnList[iLoopColumn].m_clDataColDef.IsEditAllowed() )
		{
			continue;
		}

		_BS_ReadOneInputData( pmapCellData->at( iLoopColumn ), pclBSRowParameters );
	}
}

bool CDlgBatchSelBase::BS_Validate()
{
	bool bValidate = true;

	if( true == m_bAtLeastOneRowWithNoSolution || true == m_bAtLeastOneRowWithSolutionButNotBest )
	{
		if( IDCANCEL == MessageBox( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_VALIDATEWARNING ), NULL, MB_OKCANCEL ) )
		{
			bValidate = false;
		}
	}

	return bValidate;
}

void CDlgBatchSelBase::BS_EnableRadios( bool bEnable )
{
	if( m_bRadiosEnabled == bEnable )
	{
		return;
	}

	if( false == bEnable )
	{
		// If disable required, we can do it recursively on all controls in the group.
		m_clGroupInputs.EnableWindow( false, true );
	}
	else if( true == bEnable )
	{
		// Otherwise, we enable only what we need.
		m_clGroupInputs.EnableWindow( true, false );

		// Radio 'Flow'.
		GetDlgItem( IDC_RADIOFLOW)->EnableWindow( TRUE );

		// Radio 'Power/DT'.
		GetDlgItem( IDC_RADIOPOWERDT )->EnableWindow( TRUE );
	}

	// Dp if exist.
	if( NULL != GetpCheckboxDp() )
	{
		GetpCheckboxDp()->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_bRadiosEnabled = bEnable;
}

void CDlgBatchSelBase::BS_EnableCombos( bool fEnable )
{
	if( NULL != GetpGroupValveBelow65() )
	{
		GetpGroupValveBelow65()->EnableWindow( fEnable, false );
	}

	if( NULL != GetpComboTypeBelow65() )
	{
		GetpComboTypeBelow65()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboTypeBelow65()->GetCount() <= 1 )
		{
			GetpComboTypeBelow65()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICTYPE1 ) )
	{
		GetDlgItem( IDC_STATICTYPE1 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboFamilyBelow65() )
	{
		GetpComboFamilyBelow65()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboFamilyBelow65()->GetCount() <= 1 )
		{
			GetpComboFamilyBelow65()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICFAMILY1 ) )
	{
		GetDlgItem( IDC_STATICFAMILY1 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboBdyMatBelow65() )
	{
		GetpComboBdyMatBelow65()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboBdyMatBelow65()->GetCount() <= 1 )
		{
			GetpComboBdyMatBelow65()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICMATERIAL1 ) )
	{
		GetDlgItem( IDC_STATICMATERIAL1 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboConnectBelow65() )
	{
		GetpComboConnectBelow65()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboConnectBelow65()->GetCount() <= 1 )
		{
			GetpComboConnectBelow65()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICCONNECT1 ) )
	{
		GetDlgItem( IDC_STATICCONNECT1 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboVersionBelow65() )
	{
		GetpComboVersionBelow65()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboVersionBelow65()->GetCount() <= 1 )
		{
			GetpComboVersionBelow65()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICVERSION1 ) )
	{
		GetDlgItem( IDC_STATICVERSION1 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboPNBelow65() )
	{
		GetpComboPNBelow65()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboPNBelow65()->GetCount() <= 1 )
		{
			GetpComboPNBelow65()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICPN1 ) )
	{
		GetDlgItem( IDC_STATICPN1 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpGroupValveAbove50() )
	{
		GetpGroupValveAbove50()->EnableWindow( fEnable, false );
	}

	if( NULL != GetpComboTypeAbove50() )
	{
		GetpComboTypeAbove50()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboTypeAbove50()->GetCount() <= 1 )
		{
			GetpComboTypeAbove50()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICTYPE2 ) )
	{
		GetDlgItem( IDC_STATICTYPE2 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboFamilyAbove50() )
	{
		GetpComboFamilyAbove50()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboFamilyAbove50()->GetCount() <= 1 )
		{
			GetpComboFamilyAbove50()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICFAMILY2 ) )
	{
		GetDlgItem( IDC_STATICFAMILY2 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboBdyMatAbove50() )
	{
		GetpComboBdyMatAbove50()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboBdyMatAbove50()->GetCount() <= 1 )
		{
			GetpComboBdyMatAbove50()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICMATERIAL2 ) )
	{
		GetDlgItem( IDC_STATICMATERIAL2 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboConnectAbove50() )
	{
		GetpComboConnectAbove50()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboConnectAbove50()->GetCount() <= 1 )
		{
			GetpComboConnectAbove50()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICCONNECT2 ) )
	{
		GetDlgItem( IDC_STATICCONNECT2 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboVersionAbove50() )
	{
		GetpComboVersionAbove50()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboVersionAbove50()->GetCount() <= 1 )
		{
			GetpComboVersionAbove50()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICVERSION2 ) )
	{
		GetDlgItem( IDC_STATICVERSION2 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}

	if( NULL != GetpComboPNAbove50() )
	{
		GetpComboPNAbove50()->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );

		if( TRUE == fEnable && GetpComboPNAbove50()->GetCount() <= 1 )
		{
			GetpComboPNAbove50()->EnableWindow( FALSE );
		}
	}

	if( NULL != GetDlgItem( IDC_STATICPN2 ) )
	{
		GetDlgItem( IDC_STATICPN2 )->EnableWindow( ( true == fEnable ) ? TRUE : FALSE );
	}
}

bool CDlgBatchSelBase::BS_SuggestSelection()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	EnableSuggestButton( false );

	// Enable 'Validate' button only if one solution or one alternative solution exist.
	if( true == m_bAtLeastOneRowWithBestSolution || true == m_bAtLeastOneRowWithSolutionButAlternative )
	{
		m_clButtonValidate.EnableWindow( TRUE );
		m_eValidationStatus = BS_ValidationStatus::BS_VS_NotYetDone;
	}

	m_clInterface.EnableCtxtMenuPasteData( false, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, true );
	m_clInterface.EnableEdition( false, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_eSuggestionStatus = BS_SuggestionStatus::BS_SS_Done;

	// Reset the focus on the batch output dialog.
	m_clInterface.SetFocus();

	return true;
}

bool CDlgBatchSelBase::BS_AddOneColumn( int iColumnID )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	if( 0 == m_mapColumnList.count( iColumnID ) )
	{
		return false;
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	BSColParameters *pclBSColParameters = &m_mapColumnList[iColumnID];

	// Add header name.
	m_clInterface.AddColumnHeaderDef( iColumnID, pclBSColParameters->m_clHeaderColDef, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.AddColumnHeaderFontDef( iColumnID, pclBSColParameters->m_clHeaderFontDef, (int)m_pclBatchSelParams->m_eProductSubCategory );
	
	m_clInterface.FillColumnHeader( iColumnID, BS_RowRelativePos::BS_RID_HeaderName, pclBSColParameters->m_strHeaderName,
			DLBBATCHSELBASE_ROWHEIGHT_HEADERNAME, false, (int)m_pclBatchSelParams->m_eProductSubCategory );

	// Add header unit.
	if( -1 != pclBSColParameters->m_iHeaderUnit )
	{
		pclBSColParameters->m_strHeaderUnit = _T("(" ) + CString( GetNameOf( pUnitDB->GetDefaultUnit( pclBSColParameters->m_iHeaderUnit ) ).c_str() ) + _T( ")");
	}

	m_clInterface.FillColumnHeader( iColumnID, BS_RowRelativePos::BS_RID_HeaderUnit, pclBSColParameters->m_strHeaderUnit,
			DLBBATCHSELBASE_ROWHEIGHT_HEADERUNIT, pclBSColParameters->m_bLineBelowHeader, (int)m_pclBatchSelParams->m_eProductSubCategory );

	// Add column definition.
	m_clInterface.AddColumnDefinition( iColumnID, pclBSColParameters->m_clDataColDef, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.AddColumnFontDefiniton( iColumnID, pclBSColParameters->m_clDataFontDef, (int)m_pclBatchSelParams->m_eProductSubCategory );

	pclBSColParameters->m_bDisplayed = true;

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	return true;
}

bool CDlgBatchSelBase::BS_RemoveOneColumn( int iColumnID )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERTA_RETURN( false );
	}

	if( 0 == m_mapColumnList.count( iColumnID ) )
	{
		return false;
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	m_clInterface.RemoveColumn( iColumnID, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_mapColumnList[iColumnID].m_bDisplayed = false;

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	return true;
}

void CDlgBatchSelBase::BS_UpdateUnit()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); ++iter )
	{
		BSColParameters *pclBSColParameters = &iter->second;
		// HYS-825: To change value when unit changes
		DlgOutputHelper::mapIntCellBase mapCellData;
		BSRowParameters* pclBSRowParameters;
		int iRow = BS_RID_FirstData;

		if( -1 != pclBSColParameters->m_iHeaderUnit )
		{
			// HYS-825
			bool bFound = m_clInterface.GetRow( &mapCellData, iRow, (int)m_pclBatchSelParams->m_eProductSubCategory );

			pclBSColParameters->m_strHeaderUnit = GetNameOf( pUnitDB->GetDefaultUnit( pclBSColParameters->m_iHeaderUnit ) ).c_str();
			m_clInterface.FillColumnHeader( pclBSColParameters->m_iColumnID, BS_RID_HeaderUnit, pclBSColParameters->m_strHeaderUnit,
					DLBBATCHSELBASE_ROWHEIGHT_HEADERUNIT, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
			// HYS-825
			while( bFound )
			{
				pclBSRowParameters = &m_mapAllRowData[iRow];
				BS_UpdateFlowPowerDpCell( m_clInterface.GetMessage(), pclBSRowParameters, pclBSColParameters->m_iColumnID, iRow );
				iRow++;
				bFound = m_clInterface.GetRow( &mapCellData, iRow, (int)m_pclBatchSelParams->m_eProductSubCategory );
			}
		}
	}

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}
}

void CDlgBatchSelBase::BS_UpdateFlowPowerDpCell( CDlgBatchSelectionOutput::CBSMessage *pclBSMessage, BSRowParameters *pclRowParameters, int iColumnID, int iRow )
{
	if( NULL == pclBSMessage || NULL == pclRowParameters  || iColumnID < BS_CID_Input_Flow
		|| iColumnID >= BS_CID_Input_Last )
	{
		return;
	}


	switch( iColumnID )
	{
		case BS_ColumnID::BS_CID_Input_Flow:

			if( CDS_SelProd::efdFlow == m_pclBatchSelParams->m_eFlowOrPowerDTMode )
			{
				//Flow
				double dCU = CDimValue::SItoCU( _U_FLOW, pclRowParameters->m_dFlow );
				pclBSMessage->SetCellNumber( iColumnID, dCU, _U_FLOW, 3, 1, true );
				pclBSMessage->Clear();
				pclBSMessage->SetCellNumberAsText( iColumnID, WriteCUDouble(_U_FLOW, pclRowParameters->m_dFlow), _U_FLOW, 3, 1, true );
				pclBSMessage->SendMessage( iRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
				break;
			}
			break;

		case BS_ColumnID::BS_CID_Input_Power:

			if( CDS_SelProd::efdPower == m_pclBatchSelParams->m_eFlowOrPowerDTMode )
			{
				//Power
				double dCU = CDimValue::SItoCU( _U_TH_POWER, pclRowParameters->m_dPower );
				pclBSMessage->SetCellNumber( iColumnID, dCU, _U_TH_POWER, 3, 1, true );
				pclBSMessage->Clear();
				pclBSMessage->SetCellNumberAsText( iColumnID, WriteCUDouble( _U_TH_POWER, pclRowParameters->m_dPower ), _U_TH_POWER, 3, 1, true );
				pclBSMessage->SendMessage( iRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
				break;
			}
			break;

		case BS_ColumnID::BS_CID_Input_DT:

			if( CDS_SelProd::efdPower == m_pclBatchSelParams->m_eFlowOrPowerDTMode )
			{
				//DT
				double dCU = CDimValue::SItoCU( _U_DIFFTEMP, pclRowParameters->m_dDT );
				pclBSMessage->SetCellNumber( iColumnID, dCU, _U_DIFFTEMP, 3, 1, true );
				pclBSMessage->Clear();
				pclBSMessage->SetCellNumberAsText( iColumnID, WriteCUDouble( _U_DIFFTEMP, pclRowParameters->m_dDT ), _U_DIFFTEMP, 3, 1, true );
				pclBSMessage->SendMessage( iRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
				break;
			}
			break;
			
		case BS_ColumnID::BS_CID_Input_Dp:

			if( true == m_pclBatchSelParams->m_bIsDpGroupChecked )
			{
				double dCU = CDimValue::SItoCU( _U_DIFFPRESS, pclRowParameters->m_dDp );
				pclBSMessage->SetCellNumber( iColumnID, dCU, _U_DIFFPRESS, 3, 1, true );
				pclBSMessage->Clear();
				pclBSMessage->SetCellNumberAsText( iColumnID, WriteCUDouble( _U_DIFFPRESS, pclRowParameters->m_dDp ), _U_DIFFPRESS, 3, 1, true );
				pclBSMessage->SendMessage( iRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
			}

			break;

		case BS_ColumnID::BS_CID_Input_DpBranch:

			double dCU = CDimValue::SItoCU( _U_DIFFPRESS, pclRowParameters->m_dDpBranch );
			pclBSMessage->SetCellNumber( iColumnID, dCU, _U_DIFFPRESS, 3, 1, true );
			pclBSMessage->Clear();
			pclBSMessage->SetCellNumberAsText( iColumnID, WriteCUDouble( _U_DIFFPRESS, pclRowParameters->m_dDpBranch ), _U_DIFFPRESS, 3, 1, true );
			pclBSMessage->SendMessage( iRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );

			break;
	}
}

void CDlgBatchSelBase::BS_UpdateFlowCell( int iOutputID, long lRow )
{
	DlgOutputHelper::mapIntCellBase mapCellData;
	m_clInterface.GetRow( &mapCellData, lRow, (int)m_pclBatchSelParams->m_eProductSubCategory );

	// Get value from cell.
	double dPower = 0.0;
	mapCellData[BS_ColumnID::BS_CID_Input_Power]->GetNumber( dPower );
	dPower = CDimValue::CUtoSI( _U_TH_POWER, dPower );
	double dDT = 0.0;
	mapCellData[BS_ColumnID::BS_CID_Input_DT]->GetNumber( dDT );
	dDT = CDimValue::CUtoSI( _U_DIFFTEMP, dDT );
	double dFlow = 0.0;
	mapCellData[BS_ColumnID::BS_CID_Input_Flow]->GetNumber( dFlow );
	dFlow = CDimValue::CUtoSI( _U_FLOW, dFlow );

	if( 0 != dPower && 0 != dDT )
	{
		bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
		}

		// Generate flow from Power an DT.
		CWaterChar clWaterCharReturn = m_pclBatchSelParams->m_WC;
		double dReturnTemperature = (ProjectType::Cooling == m_pclBatchSelParams->m_eApplicationType) ? m_pclBatchSelParams->m_WC.GetTemp() + dDT :
			m_pclBatchSelParams->m_WC.GetTemp() - dDT;

		clWaterCharReturn.UpdateFluidData( dReturnTemperature );

		dFlow = m_pclBatchSelParams->m_WC.GetTemp() * m_pclBatchSelParams->m_WC.GetDens() * m_pclBatchSelParams->m_WC.GetSpecifHeat();
		dFlow -= (clWaterCharReturn.GetTemp() * clWaterCharReturn.GetDens() * clWaterCharReturn.GetSpecifHeat());
		dFlow = abs( dPower / dFlow );

		// Display flow value.
		double dCU = CDimValue::SItoCU( _U_FLOW, dFlow );
		m_clInterface.GetMessage()->SetCellNumber( BS_ColumnID::BS_CID_Input_Flow, dCU, _U_FLOW, 3, 1, true );
		m_clInterface.GetMessage()->Clear();
		m_clInterface.GetMessage()->SetCellNumberAsText( BS_ColumnID::BS_CID_Input_Flow, WriteCUDouble( _U_FLOW, dFlow ), _U_FLOW, 3, 1, true );
		m_clInterface.GetMessage()->SendMessage( lRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );

		if( false == bIsRedrawBlocked )
		{
			m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
		}
	}
}

void CDlgBatchSelBase::BS_UpdateIOColumnLayout( int iUpdateWhat )
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	bool bIsRedrawBlocked = m_clInterface.IsRedrawBlocked( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.BlockRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	if( true == m_clInterface.IsBlockSelectionExist(), (int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		m_clInterface.ResetBlockSelectionToOrigin( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}

	m_clInterface.SaveSelections( CSSheet::SaveSelectionChoice::SSC_BlockSelection, (int)m_pclBatchSelParams->m_eProductSubCategory );
	
	long lSelectedRow;
	m_clInterface.GetSelectedRow( lSelectedRow, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.ClearCurrentSelections( CSSheet::SaveSelectionChoice::SSC_All, (int)m_pclBatchSelParams->m_eProductSubCategory );

	switch( iUpdateWhat )
	{
		case BS_UpdateInputColumn::BS_UIC_FlowOrPowerDT:

			if( CDS_SelProd::efdFlow == m_pclBatchSelParams->m_eFlowOrPowerDTMode )
			{
				BS_RemoveOneColumn( BS_ColumnID::BS_CID_Input_Power );
				BS_RemoveOneColumn( BS_ColumnID::BS_CID_Input_DT );
				// HYS-1995: Add flow column with correct properties when the last previous mode is PDT mode.
				BS_RemoveOneColumn( BS_ColumnID::BS_CID_Input_Flow );
				_UpdateFlowCellProperties( true );
				BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Flow );
			}
			else
			{
				// HYS-1995: Add flow column with correct properties when the current mode is PDT mode.
				BS_RemoveOneColumn( BS_ColumnID::BS_CID_Input_Flow );
				_UpdateFlowCellProperties( false );
				BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Flow );
				BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Power );
				BS_AddOneColumn( BS_ColumnID::BS_CID_Input_DT );
			}
			break;

		case BS_UpdateInputColumn::BS_UIC_Dp:

			if( true == m_pclBatchSelParams->m_bIsDpGroupChecked )
			{
				BS_AddOneColumn( BS_ColumnID::BS_CID_Input_Dp );
			}
			else
			{
				BS_RemoveOneColumn( BS_ColumnID::BS_CID_Input_Dp );
			}
			break;
	}

	_BSResizingColumnInit();

	m_clInterface.SelectRow( lSelectedRow, false, (int)m_pclBatchSelParams->m_eProductSubCategory );
	m_clInterface.RestoreSelections( (int)m_pclBatchSelParams->m_eProductSubCategory );

	if( false == bIsRedrawBlocked )
	{
		m_clInterface.ReleaseRedraw( (int)m_pclBatchSelParams->m_eProductSubCategory );
	}
}

bool CDlgBatchSelBase::BS_ComboChange( bool bIsLast )
{
	// When user changes a combo and there is already a suggestion that has been done, we ask to the user if he wants
	// to clear the results. If user doesn't want to clear results the method returns 'false' to signal to the calling method
	// to not change following combos.

	// Why 'm_bMsgComboChangeDone'? 'BS_ComboChange' is called in all 'OnCbnSelChangeXXX' handlers.
	// User can effectively change a combo in the middle of the chain and we must ask him if he wants to clear or not the results.
	//
	// To illustrate, we have these handlers: 'OnCbnSelChangeTypeBelow65()', 'OnCbnSelChangeFamilyBelow65()',
	// 'OnCbnSelChangeBdyMatBelow65()', 'OnCbnSelChangeConnectBelow65()', 'OnCbnSelChangeVersionBelow65()' and
	// 'OnCbnSelChangePNBelow65()'.
	//
	// Each handler calls its children: 'OnCbnSelChangeTypeBelow65()' calls 'OnCbnSelChangeFamilyBelow65()', 'OnCbnSelChangeFamilyBelow65()'
	// calls 'OnCbnSelChangeBdyMatBelow65()', 'OnCbnSelChangeBdyMatBelow65()' calls 'OnCbnSelChangeConnectBelow65()' and so on.
	//
	// If user change the body material combo, the 'OnCbnSelChangeBdyMatBelow65()' is called. The handler calls 'BS_ComboChange()' to know
	// if user wants to clear results or not. If he doesn't want to clear results, we do nothing. Otherwise, 'OnCbnSelChangeBdyMatBelow65()'
	// applies appropriate changes and calls 'OnCbnSelChangeConnectBelow65()'.
	//
	// The handler 'OnCbnSelChangeConnectBelow65()' also calls 'BS_ComboChange()'. But we don't want to ask again to the user because it is
	// already be done in the previous handler. This is why we set the 'm_bMsgComboChangeDone' variable to 'true'.
	// This variable stay to 'true' until the 'BS_ComboChange' method is called with the 'fIsLast' argument set to 'true'. 

	bool bReturn = true;

	if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
	{
		// If there is already some suggestion...

		// If user has just clicked a combo and changed it...
		if( false == m_bMsgComboChangeDone )
		{
			// Ask him if he wants to delete current results.
			CString str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_BSCLEARRESULTS );

			if( IDNO == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION, 0 ) )
			{
				// If user doesn't want, we return 'false' to specify to not change combo and to not continue to call other combo change events.
				bReturn = false;
				m_bMsgComboChangeDone = false;
			}
			else
			{
				m_bMsgComboChangeDone = true;
			}
		}

		if( true == bIsLast )
		{
			m_bMsgComboChangeDone = false;
		}
	}

	return bReturn;
}


void CDlgBatchSelBase::FillComboBoxType( AboveOrBelow eAboveOrBelow, CString strTypeID )
{
	CExtNumEditComboBox *pCCombo;
	CRankEx TypeList;

	if( Above50 == eAboveOrBelow )
	{
		pCCombo = &m_ComboTypeAbove50;
	}
	else
	{
		pCCombo = &m_ComboTypeBelow65;
	}

	GetTypeList( &TypeList, eAboveOrBelow );

	pCCombo->FillInCombo( &TypeList, strTypeID );

	if( Above50 == eAboveOrBelow )
	{
		m_pclBatchSelParams->m_strComboTypeAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelParams->m_strComboTypeBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelBase::FillComboBoxFamily( AboveOrBelow eAboveOrBelow, CString strFamilyID )
{
	CExtNumEditComboBox *pCCombo;
	CRankEx FamilyList;

	if( Above50 == eAboveOrBelow )
	{
		pCCombo = &m_ComboFamilyAbove50;
	}
	else
	{
		pCCombo = &m_ComboFamilyBelow65;
	}

	GetFamilyList( &FamilyList, eAboveOrBelow );

	pCCombo->FillInCombo( &FamilyList, strFamilyID );

	if( Above50 == eAboveOrBelow )
	{
		m_pclBatchSelParams->m_strComboFamilyAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelParams->m_strComboFamilyBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelBase::FillComboBoxBodyMaterial( AboveOrBelow eAboveOrBelow, CString strBdyMatID )
{
	CExtNumEditComboBox *pCCombo;
	CRankEx BodyMaterialList;

	if( Above50 == eAboveOrBelow )
	{
		pCCombo = &m_ComboBodyMaterialAbove50;
	}
	else
	{
		pCCombo = &m_ComboBodyMaterialBelow65;
	}

	GetMaterialList( &BodyMaterialList, eAboveOrBelow );

	pCCombo->FillInCombo( &BodyMaterialList, strBdyMatID );

	if( Above50 == eAboveOrBelow )
	{
		m_pclBatchSelParams->m_strComboMaterialAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelParams->m_strComboMaterialBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelBase::FillComboBoxConnect( AboveOrBelow eAboveOrBelow, CString strConnectID )
{
	CExtNumEditComboBox *pCCombo;
	CRankEx ConnectList;

	if( Above50 == eAboveOrBelow )
	{
		pCCombo = &m_ComboConnectAbove50;
	}
	else
	{
		pCCombo = &m_ComboConnectBelow65;
	}

	GetConnectList( &ConnectList, eAboveOrBelow );

	pCCombo->FillInCombo( &ConnectList, strConnectID );

	if( Above50 == eAboveOrBelow )
	{
		m_pclBatchSelParams->m_strComboConnectAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelParams->m_strComboConnectBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelBase::FillComboBoxVersion( AboveOrBelow eAboveOrBelow, CString strVersionID )
{
	CExtNumEditComboBox *pCCombo;
	CRankEx VersionList;

	if( Above50 == eAboveOrBelow )
	{
		pCCombo = &m_ComboVersionAbove50;
	}
	else
	{
		pCCombo = &m_ComboVersionBelow65;
	}

	GetVersionList( &VersionList, eAboveOrBelow );

	pCCombo->FillInCombo( &VersionList, strVersionID );

	if( Above50 == eAboveOrBelow )
	{
		m_pclBatchSelParams->m_strComboVersionAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelParams->m_strComboVersionBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelBase::FillComboBoxPN( AboveOrBelow eAboveOrBelow, CString strPNID )
{
	CExtNumEditComboBox *pCCombo;
	CRankEx PNList;

	if( Above50 == eAboveOrBelow )
	{
		pCCombo = &m_ComboPNAbove50;
	}
	else
	{
		pCCombo = &m_ComboPNBelow65;
	}

	GetPNList( &PNList, eAboveOrBelow );

	pCCombo->FillInCombo( &PNList, strPNID );

	if( Above50 == eAboveOrBelow )
	{
		m_pclBatchSelParams->m_strComboPNAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelParams->m_strComboPNBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelBase::GetTypeList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	if( NULL == pclRankEx )
	{
		return;
	}

	if( ProductSubCategory::PSC_BC_RegulatingValve == (ProductSubCategory)(int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		CRankEx TempList;

		if( Above50 == eAboveOrBelow )
		{
			m_pclBatchSelParams->m_pTADB->GetBVTypeList( &TempList, CTADatabase::FilterSelection::ForIndAndBatchSel, m_iDN65, INT_MAX, true );
		}
		else
		{
			m_pclBatchSelParams->m_pTADB->GetBVTypeList( &TempList, CTADatabase::FilterSelection::ForIndAndBatchSel, 0, m_iDN50, true );
		}

		// TO REMOVE -> Fixed orifice are not yet implemented in the batch selection.
		_string str;
		LPARAM lParam;

		for( BOOL bContinue = TempList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = TempList.GetNext( str, lParam ) )
		{
			CDB_StringID *pTypeID = ( CDB_StringID * )lParam;

			if( 0 != CString( pTypeID->GetIDPtr().ID ).Compare( _T("RVTYPE_FO") ) )
			{
				pclRankEx->Add( pTypeID->GetString(), _ttoi( pTypeID->GetIDstr() ) - 1, ( LPARAM )pTypeID );
			}
		}
	}
}

void CDlgBatchSelBase::GetFamilyList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	if( NULL == pclRankEx )
	{
		return;
	}

	if( ProductSubCategory::PSC_BC_RegulatingValve == (ProductSubCategory)(int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		if( Above50 == eAboveOrBelow )
		{
			m_pclBatchSelParams->m_pTADB->GetBVFamilyList(
					pclRankEx,
					(LPCTSTR)m_pclBatchSelParams->m_strComboTypeAbove50ID,
					CTADatabase::FilterSelection::ForIndAndBatchSel,
					m_iDN65,
					INT_MAX );
		}
		else
		{
			m_pclBatchSelParams->m_pTADB->GetBVFamilyList(
					pclRankEx,
					(LPCTSTR)m_pclBatchSelParams->m_strComboTypeBelow65ID,
					CTADatabase::FilterSelection::ForIndAndBatchSel,
					0,
					m_iDN50 );
		}
	}
}

void CDlgBatchSelBase::GetMaterialList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	if( NULL == pclRankEx )
	{
		return;
	}

	if( ProductSubCategory::PSC_BC_RegulatingValve == (ProductSubCategory)(int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		if( Above50 == eAboveOrBelow )
		{
			m_pclBatchSelParams->m_pTADB->GetBVBdyMatList(
				pclRankEx,
				(LPCTSTR)m_pclBatchSelParams->m_strComboTypeAbove50ID,
				(LPCTSTR)m_pclBatchSelParams->m_strComboFamilyAbove50ID,
				CTADatabase::FilterSelection::ForIndAndBatchSel,
				m_iDN65,
				INT_MAX );
		}
		else
		{
			m_pclBatchSelParams->m_pTADB->GetBVBdyMatList(
				pclRankEx,
				(LPCTSTR)m_pclBatchSelParams->m_strComboTypeBelow65ID,
				(LPCTSTR)m_pclBatchSelParams->m_strComboFamilyBelow65ID,
				CTADatabase::FilterSelection::ForIndAndBatchSel,
				0,
				m_iDN50 );
		}
	}
}

void CDlgBatchSelBase::GetConnectList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	if( NULL == pclRankEx )
	{
		return;
	}

	if( ProductSubCategory::PSC_BC_RegulatingValve == (ProductSubCategory)(int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		if( Above50 == eAboveOrBelow )
		{
			m_pclBatchSelParams->m_pTADB->GetBVConnList(
					pclRankEx,
					(LPCTSTR)m_pclBatchSelParams->m_strComboTypeAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboFamilyAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboMaterialAbove50ID,
					CTADatabase::FilterSelection::ForIndAndBatchSel,
					m_iDN65, INT_MAX );
		}
		else
		{
			m_pclBatchSelParams->m_pTADB->GetBVConnList(
					pclRankEx,
					(LPCTSTR)m_pclBatchSelParams->m_strComboTypeBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboFamilyBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboMaterialBelow65ID,
					CTADatabase::FilterSelection::ForIndAndBatchSel,
					0,
					m_iDN50 );
		}
	}
}

void CDlgBatchSelBase::GetVersionList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	if( NULL == pclRankEx )
	{
		return;
	}

	if( ProductSubCategory::PSC_BC_RegulatingValve == (ProductSubCategory)(int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		if( Above50 == eAboveOrBelow )
		{
			m_pclBatchSelParams->m_pTADB->GetBVVersList(
					pclRankEx,
					(LPCTSTR)m_pclBatchSelParams->m_strComboTypeAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboFamilyAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboMaterialAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboConnectAbove50ID,
					CTADatabase::FilterSelection::ForIndAndBatchSel,
					m_iDN65, INT_MAX );
		}
		else
		{
			m_pclBatchSelParams->m_pTADB->GetBVVersList(
					pclRankEx,
					(LPCTSTR)m_pclBatchSelParams->m_strComboTypeBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboFamilyBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboMaterialBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboConnectBelow65ID,
					CTADatabase::FilterSelection::ForIndAndBatchSel,
					0, m_iDN50 );
		}
	}
}

void CDlgBatchSelBase::GetPNList( CRankEx *pclRankEx, AboveOrBelow eAboveOrBelow )
{
	if( NULL == pclRankEx )
	{
		return;
	}

	if( ProductSubCategory::PSC_BC_RegulatingValve == (ProductSubCategory)(int)m_pclBatchSelParams->m_eProductSubCategory )
	{
		if( Above50 == eAboveOrBelow )
		{
			m_pclBatchSelParams->m_pTADB->GetBVPNList(
					pclRankEx,
					(LPCTSTR)m_pclBatchSelParams->m_strComboTypeAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboFamilyAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboMaterialAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboConnectAbove50ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboVersionAbove50ID,
					CTADatabase::FilterSelection::ForIndAndBatchSel,
					m_iDN65, INT_MAX );
		}
		else
		{
			m_pclBatchSelParams->m_pTADB->GetBVPNList(
					pclRankEx,
					(LPCTSTR)m_pclBatchSelParams->m_strComboTypeBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboFamilyBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboMaterialBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboConnectBelow65ID,
					(LPCTSTR)m_pclBatchSelParams->m_strComboVersionBelow65ID,
					CTADatabase::FilterSelection::ForIndAndBatchSel,
					0, m_iDN50 );
		}
	}
}

void CDlgBatchSelBase::UpdateFlowOrPowerDTState()
{
	// 'TRUE' to fill 'm_iRadioFlowPowerDT' variable with the radio state in the dialog.
	UpdateData( TRUE );

	m_pclBatchSelParams->m_eFlowOrPowerDTMode = ( Radio_Flow == m_iRadioFlowPowerDT ) ? CDS_SelProd::efdFlow : CDS_SelProd::efdPower;
}

void CDlgBatchSelBase::CheckIfWeNeedToClearResults()
{
	if( false == m_bMsgDoNotClearResult )
	{
		if( BS_SuggestionStatus::BS_SS_Done == m_eSuggestionStatus )
		{
			OnBatchSelectionButtonClearResults();
			OnBnClickedSuggest();
		}
		else
		{
			ClearAll();
		}
	}
}

void CDlgBatchSelBase::_BSFirstTimeInit()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}

	CDS_BatchSelParameter *pclBatchSelParameters = m_pclBatchSelParams->m_pTADS->GetpBatchSelParameter();

	if( NULL != pclBatchSelParameters && false == m_bInitialized && true == m_clInterface.Init() )
	{
		UpdateFlowOrPowerDTState();
		m_pclBatchSelParams->m_bIsDpGroupChecked = ( NULL != GetpCheckboxDp() && BST_CHECKED == GetpCheckboxDp()->GetCheck() ) ? true : false;
		m_clInterface.Reset( (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.AutomaticLastBlankLine( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.SetSelectionColor( DLGBATCHSELBASE_SELECTIONBACKCOLOR, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableBlockSelection( true, DLGBATCHSELBASE_BLOCKSELECTIONSTYLE, DLGBATCHSELBASE_BLOCKSELECTIONCOLOR, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.SetDataRowHeight( DLBBATCHSELBASE_ROWHEIGHT_DATA, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableCtxtMenuPasteData( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableCtxtMenuAddColumn( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARALL, false );
		m_clInterface.EnableTopPanelButton( DLGBSO_BUTTONID_CLEARRESULTS, false );
		
		m_clInterface.RegisterNotificationHandler( this, CDlgBatchSelectionOutput::IBaseNotificationHandler::NH_All |
				CDlgBatchSelectionOutput::IBSNotificationHandler::BSNH_All, (int)m_pclBatchSelParams->m_eProductSubCategory );

		m_clInterface.ResizingColumnActivate( true, (int)m_pclBatchSelParams->m_eProductSubCategory );
		m_clInterface.EnableEdition( true, (int)m_pclBatchSelParams->m_eProductSubCategory );

		if( true == BS_InitMainHeaders() )
		{
			if( true == BS_InitColumns() )
			{
				BS_ReadAllColumnWidth( pclBatchSelParameters );
				m_bInitialized = true;
			}
		}

		_BSResizingColumnInit();
	}
}

void CDlgBatchSelBase::_BSResizingColumnInit()
{
	if( NULL == m_pclBatchSelParams )
	{
		ASSERT_RETURN;
	}
	
	// Run all column headers.
	std::map<long, bool> mapResizable;

	for( mapLongColParamsIter iter = m_mapColumnList.begin(); iter != m_mapColumnList.end(); ++iter )
	{
		if( false == iter->second.m_bDisplayed )
		{
			continue;
		}

		long lColAbsolute;
		m_clInterface.GetColID2Absolute( iter->second.m_iColumnID, lColAbsolute, (int)m_pclBatchSelParams->m_eProductSubCategory );
		
		int iMainHeaderID, iMainHeaderSubID;
		iter->second.m_clHeaderColDef.GetMainHeaderLinks( iMainHeaderID, iMainHeaderSubID );
		
		mapResizable[lColAbsolute] = ( -1 != iMainHeaderID && -1 != iMainHeaderSubID ) ? true : false;
	}

	m_clInterface.ResizingColumnRemoveAll( (int)m_pclBatchSelParams->m_eProductSubCategory );
	
	long lStartCol = 0;
	long lEndCol = 0;
	long lColHeaderStartAbsoluteRow;
	long lColHeaderEndAbsolueRow;
	m_clInterface.GetColumnHeaderRowsAbsolute( lColHeaderStartAbsoluteRow, lColHeaderEndAbsolueRow, (int)m_pclBatchSelParams->m_eProductSubCategory );

	for( std::map<long, bool>::iterator iter = mapResizable.begin(); iter != mapResizable.end(); ++iter )
	{
		if( false == iter->second )
		{
			if( 0 != lStartCol )
			{
				m_clInterface.ResizingColumnAddRange( lStartCol, lEndCol, lColHeaderStartAbsoluteRow, lColHeaderEndAbsolueRow, 
						(int)m_pclBatchSelParams->m_eProductSubCategory );
			}

			lStartCol = 0;
			lEndCol = 0;
		}
		else
		{
			if( 0 == lStartCol )
			{
				lStartCol = iter->first;
				lEndCol = iter->first;
			}
			else
			{
				lEndCol++;
			}
		}

	}

	if( 0 != lStartCol )
	{
		m_clInterface.ResizingColumnAddRange( lStartCol, lEndCol, lColHeaderStartAbsoluteRow, lColHeaderEndAbsolueRow, 
				(int)m_pclBatchSelParams->m_eProductSubCategory );
	}
}

void CDlgBatchSelBase::_BS_ReadOneInputData( DlgOutputHelper::CCellBase *pclCellData, BSRowParameters *pclBSRowParameters )
{
	if( NULL == pclCellData || NULL == pclBSRowParameters )
	{
		return;
	}

	if( DlgOutputHelper::CColDef::Number == pclCellData->GetContentType() )
	{
		double dValue = 0.0;
		ReadDoubleReturn_enum eError = pclCellData->GetNumber( dValue );

		if( RD_OK != eError )
		{
			return;
		}

		switch( pclCellData->GetColumnID() )
		{
			case BS_ColumnID::BS_CID_Input_Flow:
				pclBSRowParameters->m_dFlow = CDimValue::CUtoSI( _U_FLOW, dValue );
				break;

			case BS_ColumnID::BS_CID_Input_Power:
				pclBSRowParameters->m_dPower = CDimValue::CUtoSI( _U_TH_POWER, dValue );
				break;

			case BS_ColumnID::BS_CID_Input_DT:
				pclBSRowParameters->m_dDT = CDimValue::CUtoSI( _U_DIFFTEMP, dValue );
				break;

			case BS_ColumnID::BS_CID_Input_Dp:
				pclBSRowParameters->m_dDp = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
				break;

			case BS_ColumnID::BS_CID_Input_DpBranch:
				pclBSRowParameters->m_dDpBranch = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
				break;

			case BS_ColumnID::BS_CID_Input_Kvs:
				pclBSRowParameters->m_dKvs = CDimValue::CUtoSI( _C_KVCVCOEFF, dValue );
				break;
		}
	}
	else if( DlgOutputHelper::CColDef::Text == pclCellData->GetContentType() )
	{
		CString strText = pclCellData->GetText();

		switch( pclCellData->GetColumnID() )
		{
			case BS_ColumnID::BS_CID_Input_Reference1:
				pclBSRowParameters->m_strReference1 = strText;
				break;

			case BS_ColumnID::BS_CID_Input_Reference2:
				pclBSRowParameters->m_strReference2 = strText;
				break;
		}
	}
}

void CDlgBatchSelBase::_UpdateFlowCellProperties( bool bStatus )
{
	// If column ID doesn't exist...
	if( 0 == m_mapColumnList.count( BS_ColumnID::BS_CID_Input_Flow ) )
	{
		return;
	}

	m_mapColumnList[BS_ColumnID::BS_CID_Input_Flow].m_clDataColDef.SetBlockSelectionFlag( bStatus );
	m_mapColumnList[BS_ColumnID::BS_CID_Input_Flow].m_clDataColDef.SetEditableFlag( bStatus );
	m_mapColumnList[BS_ColumnID::BS_CID_Input_Flow].m_clDataColDef.SetPasteDataFlag( bStatus );
}
