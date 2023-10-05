#include "stdafx.h"

#define HMCALC	true

#include <windowsx.h>
#include <windows.h>
#include <float.h>
#include <math.h>

#include "MainFrm.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "Global.h"
#include "Utilities.h"
#include "Units.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "Select.h"
#include "SheetHMCalc.h"
#include "SSheetLDSumm.h"
#include "SSheetLogData.h"
#include "SSheetPanelCirc2.h"
#include "SSheetQuickMeas.h"
#include "SelProdPageBase.h"
#include "SelProdDockPane.h"
#include "RViewLogData.h"
#include "RViewGen.h"
#include "RViewProj.h"
#include "RViewChart.h"
#include "RViewLDSumm.h"
#include "RViewHMCalc.h"
#include "RViewQuickMeas.h"
#include "RViewHMSumm.h"
#include "DlgLTtabctrl.h"
#include "DlgWaterChar.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "WizardCBI.h"
#include "DlgPasteHM.h"
#include "CEditString.h"
#include "DlgEditString.h"
#include "DlgWizCircuit.h"
#include "DlgResolCBIPlantModif.h"
#include "ToolsDockablePane.h"
#include "DlgInjectionError.h"

#include "DlgLeftTabProject.h"
#include "Excel_Workbook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgLeftTabProject *pDlgLeftTabProject = NULL;

CDlgLeftTabProject::CDlgLeftTabProject( CWnd *pParent )
	: CDlgLeftTabBase( CMyMFCTabCtrl::TabIndex::etiProj, CDlgLeftTabProject::IDD, pParent )
{
	m_eCurrentRightView = CMainFrame::RightViewList::eRVGeneral;
	m_pTADB = NULL;
	m_pTADS = NULL;
	m_pclTreeImageList = NULL;
	m_hHNBranch = NULL;
    m_hLoggedDataBranch = NULL;
    m_hQuickMeasBranch = NULL;
    m_fAtLeastOneHM = false;
	m_fAtLeastOneLog = false;
	m_fAtLeastOneQM = false;
	m_hItem = NULL;
	m_vecSelectedItem.clear();
	pDlgLeftTabProject = NULL;
}

CDlgLeftTabProject::~CDlgLeftTabProject()
{
	if( NULL != m_pclTreeImageList )
	{
		delete m_pclTreeImageList;
	}

	pDlgLeftTabProject = NULL;
}

void CDlgLeftTabProject::OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex )
{
	// Base class.
	CDlgLeftTabBase::OnLTTabCtrlEnterTab( eTabIndex, ePrevTabIndex );

	// Refresh tab title
	CString str;
	str = TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_PROJECT );

	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetWindowText( str );
	}

	// Reset the tree.
	// This will reselect the previous selected item and this will activate the RefreshRightView()
	// depending on what was selected.
	ResetTree();
}

void CDlgLeftTabProject::InitializeToolsDockablePaneContextID( void )
{
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL == pclToolsDockablePane )
	{
		return;
	}

	// Register context with the tools dockable pane for the 'Project' tab and the project right view.
	CToolsDockablePane::ContextParameters *pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		ASSERT_RETURN;
	}

	pclContext->m_bIsPaneVisible = true;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = TRUE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVProj;
	pclContext->m_lHeight = 200;
	m_mapContextIDList[CMainFrame::RightViewList::eRVProj] = pclContext->m_iContextID;

	// Register context with the tools dockable pane for the 'Project' tab and the hydraulic calculator right view.
	pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		ASSERT_RETURN;
	}

	pclContext->m_bIsPaneVisible = true;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = TRUE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVHMCalc;
	pclContext->m_lHeight = 200;
	m_mapContextIDList[CMainFrame::RightViewList::eRVHMCalc] = pclContext->m_iContextID;

	// Register context with the tools dockable pane for the 'Project' tab and the general right view.
	pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		ASSERT_RETURN;
	}

	pclContext->m_bIsPaneVisible = false;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = FALSE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVGeneral;
	m_mapContextIDList[CMainFrame::RightViewList::eRVGeneral] = pclContext->m_iContextID;

	// Register context with the tools dockable pane for the 'Project' tab and the hydraulic summary right view.
	pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		ASSERT_RETURN;
	}

	pclContext->m_bIsPaneVisible = false;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = FALSE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVHMSumm;
	m_mapContextIDList[CMainFrame::RightViewList::eRVHMSumm] = pclContext->m_iContextID;

	// Register context with the tools dockable pane for the 'Project' tab and the log data summary right view.
	pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		ASSERT_RETURN;
	}

	pclContext->m_bIsPaneVisible = false;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = FALSE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVLDSumm;
	m_mapContextIDList[CMainFrame::RightViewList::eRVLDSumm] = pclContext->m_iContextID;

	// Register context with the tools dockable pane for the 'Project' tab and the log data chart right view.
	pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		ASSERT_RETURN;
	}

	pclContext->m_bIsPaneVisible = false;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = FALSE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVChart;
	m_mapContextIDList[CMainFrame::RightViewList::eRVChart] = pclContext->m_iContextID;

	// Register context with the tools dockable pane for the 'Project' tab and the log data right view.
	pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		ASSERT_RETURN;
	}

	pclContext->m_bIsPaneVisible = false;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = FALSE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVLogData;
	m_mapContextIDList[CMainFrame::RightViewList::eRVLogData] = pclContext->m_iContextID;

	// Register context with the tools dockable pane for the 'Project' tab and the quick measure right view.
	pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		ASSERT_RETURN;
	}

	pclContext->m_bIsPaneVisible = false;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = FALSE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVQuickMeas;
	m_mapContextIDList[CMainFrame::RightViewList::eRVQuickMeas] = pclContext->m_iContextID;

	m_bToolsDockablePaneContextInitialized = true;
}

bool CDlgLeftTabProject::GetToolsDockablePaneContextID( int &iContextID  )
{
	if( false == CDlgLeftTabBase::GetToolsDockablePaneContextID( iContextID ) )
	{
		return false;
	}

	iContextID = -1;

	if( NULL != m_hItem )
	{
		m_eCurrentRightView = m_Tree.GetRightViewAt( m_hItem );

		if( 0 != m_mapContextIDList.count( (int)m_eCurrentRightView ) )
		{
			iContextID = m_mapContextIDList[(int)m_eCurrentRightView];
		}
	}

	return ( -1 != iContextID ) ? true : false;
}

void CDlgLeftTabProject::OnChangeLocatedIn( CDlgCreateModule *pDialog )
{
	if( NULL == pDialog )
	{
		return;
	}

	ResetTreeAndSelectHM( pDialog->GetParentModule() );
}

void CDlgLeftTabProject::SetFlagCut( std::vector<CDS_HydroMod *> *pvecHydromod )
{
	m_Tree.ResetItemCut( NULL );

	if( NULL == pvecHydromod || 0 == (int)pvecHydromod->size() )
	{
		return;
	}

	for( int iLoop = 0; iLoop < (int)pvecHydromod->size(); iLoop++ )
	{
		HTREEITEM hFoundItem = _FindhItem( pvecHydromod->at( iLoop ), m_hHNBranch );

		if( (HTREEITEM)NULL != hFoundItem )
		{
			m_Tree.SetItemCut( hFoundItem );

			if( true == pvecHydromod->at( iLoop )->IsaModule() )
			{
				_AddChildrenInCutList( pvecHydromod->at( iLoop ) );
			}
		}
	}
}

void CDlgLeftTabProject::ResetFlagCut( void )
{
	m_Tree.ResetItemCut( NULL );
}

bool CDlgLeftTabProject::IsFlagCutSet( void )
{
	return m_Tree.IsAtLeastOneItemCut();
}

void CDlgLeftTabProject::SelectRootItem()
{
	m_Tree.SelectItem( m_Tree.GetRootItem() );
}

void CDlgLeftTabProject::ResetTreeWithString( CString strItemSelectedName, bool bSelectItem, bool bResetCutFlag )
{
	m_Tree.SetRedraw( FALSE );

	m_hHNBranch = NULL;
    m_hLoggedDataBranch = NULL;
    m_hQuickMeasBranch = NULL;
    m_fAtLeastOneHM = false;
	m_fAtLeastOneLog = false;
	m_fAtLeastOneQM = false;

	// Clear flag cut is exist.
	if( true == bResetCutFlag && NULL != pRViewHMCalc && true == pRViewHMCalc->IsFlagCutSet() )
	{
		pRViewHMCalc->ResetFlagCut();
	}

	// Makes 'm_Tree' empty if necessary.
	if( -1 != m_Tree.GetCount() )
	{
		m_Tree.DeleteAllItems();
	}
	
	if( true == pMainFrame->IsHMCalcChecked() && eb3False == m_pTADS->IsHMCalcMode() )
	{
		pMainFrame->SetHMCalcMode( false );
	}

	m_Tree.m_List.RemoveAll();
	
	// Initialize parameters for insertion of items.
	TV_INSERTSTRUCT tvinsert;
	tvinsert.hParent = NULL;
	tvinsert.hInsertAfter = TVI_FIRST; // handle of the item after which one inserts
	tvinsert.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	tvinsert.item.hItem = NULL;
	tvinsert.item.state = 0;
	tvinsert.item.stateMask = 0;
	tvinsert.item.cchTextMax = 40;
	tvinsert.item.cChildren = 0;

	// ===================================================================
	// Add project root: "Untitled Project"
	// ===================================================================
	tvinsert.item.iImage = CRCImageManager::ILPT_Root;
	tvinsert.item.iSelectedImage = CRCImageManager::ILPT_RootSelected;
	tvinsert.item.lParam = (long)m_Tree.AddToNodeList( CMainFrame::RightViewList::eRVGeneral );
	
	CDS_ProjectRef *pPrjRef = m_pTADS->GetpProjectRef();
	ASSERT( NULL != pPrjRef );
	
	CString str = pPrjRef->GetString( CDS_ProjectRef::Name );
	
	tvinsert.item.pszText = (TCHAR *)(LPCTSTR)str;
	HTREEITEM hRoot = m_Tree.InsertItem( &tvinsert );

	m_Tree.Expand( hRoot, TVE_EXPAND );

	// ===================================================================
	// For 'Piping Tab' create a hydraulic network where root module will be stored.
	// ===================================================================
	CTableHM *pTableHM = dynamic_cast<CTableHM *>( m_pTADS->GetpHydroModTable() );
	ASSERT( NULL != pTableHM );

	tvinsert.hParent = hRoot;
	tvinsert.hInsertAfter = TVI_LAST;
	tvinsert.item.iImage = CRCImageManager::ILPT_HydronicNetwork;
	tvinsert.item.iSelectedImage = CRCImageManager::ILPT_HydronicNetworkSelected;
	tvinsert.item.lParam = (long)m_Tree.AddToNodeList( CMainFrame::RightViewList::eRVHMSumm, pTableHM );
	str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
	tvinsert.item.pszText = (TCHAR *)(LPCTSTR)str;
	m_hHNBranch = m_Tree.InsertItem( &tvinsert );
	
	m_pTADS->RenameBranch(pTableHM);

	// Add all root and root's children.
	_AddTreeBranch( pTableHM->GetIDPtr(), m_hHNBranch );
	m_pTADS->ComputeFlow( pTableHM );
	_SetOverlayMask( m_hHNBranch );

	if( NULL != pTableHM->GetFirst().MP )
		m_fAtLeastOneHM = true;

	// ===================================================================
	// Add TA H.U.B. branch if HUB exists
	// ===================================================================
	if( true == TASApp.IsHubDisplayed() )
	{
		CTableHM *pSelHUBTable = dynamic_cast<CTableHM *>( m_pTADS->GetpHUBSelectionTable() );
		ASSERT( NULL != pSelHUBTable );

		if( NULL != pSelHUBTable->GetFirst().MP )
		{
			tvinsert.hParent = hRoot;
			tvinsert.hInsertAfter = TVI_LAST;
			tvinsert.item.iImage = CRCImageManager::ILPT_HUBBranch;
			tvinsert.item.iSelectedImage = CRCImageManager::ILPT_HUBBranchSelected;
			tvinsert.item.lParam = (long)m_Tree.AddToNodeList( CMainFrame::RightViewList::eRVGeneral, pSelHUBTable );
			str = TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_TAHUB );
			tvinsert.item.pszText = (TCHAR *)(LPCTSTR)str;
			HTREEITEM hNetItem = m_Tree.InsertItem( &tvinsert );
			
			// Add all root and root's children.
			_AddTreeBranch( pSelHUBTable->GetIDPtr(), hNetItem );
			m_pTADS->RenameBranch( pSelHUBTable );
			m_pTADS->ComputeFlow( pSelHUBTable );
			_SetOverlayMask( hNetItem );
		}
	}

	// ===================================================================
	// Add logged data branch.
	// ===================================================================
	tvinsert.hParent = hRoot;
	tvinsert.hInsertAfter = TVI_LAST;
	tvinsert.item.iImage = CRCImageManager::ILPT_LogBranch;
	tvinsert.item.iSelectedImage = CRCImageManager::ILPT_LogBranchSelected;
	tvinsert.item.lParam = (long)m_Tree.AddToNodeList( CMainFrame::RightViewList::eRVLDSumm );
	str = TASApp.LoadLocalizedString( IDS_LOGGEDDATAS );
	tvinsert.item.pszText = (TCHAR *)(LPCTSTR)str;
	m_hLoggedDataBranch = m_Tree.InsertItem( &tvinsert );

	CTable *pLogDataTable = m_pTADS->GetpLogDataTable();
	ASSERT( NULL != pLogDataTable );
	
	for( IDPTR LDIDPtr = pLogDataTable->GetFirst(); _T('\0') != *LDIDPtr.ID; LDIDPtr = pLogDataTable->GetNext() )
	{
		if( true == LDIDPtr.MP->IsaTable() )
		{
			continue;
		}
		
		CLog *pLD = (CLog *)( LDIDPtr.MP );
		ASSERT( NULL != pLD );

		// Add chart at the first level.
		tvinsert.hParent = m_hLoggedDataBranch;
		tvinsert.hInsertAfter = TVI_LAST;
		tvinsert.item.iImage = CRCImageManager::ILPT_LogChart;
		tvinsert.item.iSelectedImage = CRCImageManager::ILPT_LogChartSelected;
		tvinsert.item.lParam = (long)m_Tree.AddToNodeList( CMainFrame::RightViewList::eRVChart, (void *)pLD );
		tvinsert.item.pszText = (TCHAR *)pLD->GetName();
		HTREEITEM hLoggedData = m_Tree.InsertItem(&tvinsert);

		tvinsert.hParent = hLoggedData;
		tvinsert.hInsertAfter = TVI_LAST;
		tvinsert.item.iImage = CRCImageManager::ILPT_LogData;
		tvinsert.item.iSelectedImage = CRCImageManager::ILPT_LogDataSelected;
		tvinsert.item.lParam = (long)m_Tree.AddToNodeList( CMainFrame::RightViewList::eRVLogData, (void *)pLD );
		str=TASApp.LoadLocalizedString( IDS_DATALIST );
		tvinsert.item.pszText = (TCHAR *)(LPCTSTR)str;
		m_Tree.InsertItem( &tvinsert );

		m_fAtLeastOneLog = true;
	}

	// ===================================================================
	// Add quick measurements branch.
	// ===================================================================
	tvinsert.hParent = hRoot;
	tvinsert.hInsertAfter = TVI_LAST;
	tvinsert.item.iImage = CRCImageManager::ILPT_QuickMeasurementBranch;
	tvinsert.item.iSelectedImage = CRCImageManager::ILPT_QuickMeasurementBranchSelected;
	tvinsert.item.lParam = (long)m_Tree.AddToNodeList( CMainFrame::RightViewList::eRVQuickMeas );
	str=TASApp.LoadLocalizedString( IDS_QUICKMEASUREMENTS );
	tvinsert.item.pszText = (TCHAR *)(LPCTSTR)str;
	m_hQuickMeasBranch = m_Tree.InsertItem( &tvinsert );

	// Check if there is some quick measurement.
	CTable *pQMTable = TASApp.GetpTADS()->GetpQuickMeasureTable();

	if( NULL != pQMTable && pQMTable->GetItemCount() > 0 )
	{
		m_fAtLeastOneQM = true;
	}

	// Select an item.
	if( true == bSelectItem )
	{
		HTREEITEM hSelItem = NULL;

		// If name is defined, try to find it in the network.
		if( false == strItemSelectedName.IsEmpty() )
			hSelItem = _FindhItem( strItemSelectedName, hRoot );

		// If no item found...
		if( NULL == hSelItem )
		{
			// Verify if there is something in the current project.
			if( true == m_fAtLeastOneHM )
				hSelItem = m_hHNBranch;
			else if( true == m_fAtLeastOneLog )
				hSelItem = m_hLoggedDataBranch;
			else if( true == m_fAtLeastOneQM )
				hSelItem = m_hQuickMeasBranch;
			else
				hSelItem = hRoot;
		}

		m_Tree.SelectItem( hSelItem );
		m_Tree.Expand( hSelItem, TVE_EXPAND );
	}
	m_Tree.SetRedraw( TRUE );
	m_Tree.Invalidate();
}

void CDlgLeftTabProject::ResetTreeAndSelectHM( CDS_HydroMod *pHM, bool fResetCutFlag, bool fEditionOverlay )
{
	// Initialize the tree without a specific selection.
	ResetTreeWithString( _T(""), false, fResetCutFlag );
	
	// Try to find 'pHM' corresponding item.
	HTREEITEM hItem = NULL;

	if( NULL != pHM )
	{
		hItem = _FindhItem( pHM, m_Tree.GetRootItem() );
	}
	
	// If node doesn't exist...
	if( NULL == hItem )
	{
		// Verify if there is something in the current project.
		if( true == m_fAtLeastOneHM )
		{
			hItem = m_hHNBranch;
		}
		else if( true == m_fAtLeastOneLog )
		{
			hItem = m_hLoggedDataBranch;
		}
		else if( true == m_fAtLeastOneQM )
		{
			hItem = m_hQuickMeasBranch;
		}
		else
		{
			hItem = m_Tree.GetRootItem();
		}
	}


	if (true == fEditionOverlay)
	{
		//Add icon overlay to show which element is currently edited
		m_Tree.SetItemState( hItem, INDEXTOOVERLAYMASK( (int)OMI_EditCircuit ), TVIS_OVERLAYMASK );
	}

	m_Tree.SelectItem( hItem );
	m_Tree.Expand( hItem, TVE_EXPAND );
}

void CDlgLeftTabProject::ResetTree( bool fResetCutFlag )
{
	CString str;
	if( NULL != m_hItem )
		str = m_Tree.GetItemText( m_hItem );
	ResetTreeWithString( str, true, fResetCutFlag );
}

void CDlgLeftTabProject::ChangeSelection( CDS_HydroMod *pclHydroMod )
{
	if( NULL == m_hHNBranch || NULL == pclHydroMod )
		return;
	HTREEITEM hItem = _FindhItem( pclHydroMod, m_hHNBranch );
	if( NULL != hItem )
		m_Tree.SelectItem( hItem );
}

bool CDlgLeftTabProject::EditHydroMod( CDS_HydroMod *pHM, int iDlgMode )
{
	bool fAtLeastOneSettingChanged = false;

	// Get a pointer on his parent.
	CDS_HydroMod *pPHM = ( CDS_HydroMod * )( pHM->GetIDPtr().PP );

	HTREEITEM hItem = _FindhItem( pHM, m_Tree.GetRootItem() );
	m_Tree.SetItemState( hItem, INDEXTOOVERLAYMASK( ( int )OMI_EditCircuit ), TVIS_OVERLAYMASK );

	// If terminal unit exist we are in HMcalc mode
	if( NULL != pHM->GetpTermUnit() && true == pMainFrame->IsHMCalcChecked() )
	{
		CDlgWizCircuit dlg( this );
		dlg.Init( pHM, true );
		dlg.DisplayWizard();
		ResetTreeAndSelectHM( pHM );
		return true;
	}

	int iMaxIndex = 0;
	CDlgCreateModule dlg( this, this );

	// Find Maximum index in table.
	// Level == 0 we work on the root module.
	CString str;
	iMaxIndex = m_pTADS->GetLastBranchPos( pPHM );

	if( CDlgCreateModule::EditValve == iDlgMode )
	{
		FormatString( str, IDS_DLGCREATEMODULE_CPTNEDITVALVE, pHM->GetHMName() );
	}
	else
	{
		FormatString( str, IDS_DLGCREATEMODULE_CPTNEDTMODULE, pHM->GetHMName() );
	}

	int iReturn = dlg.Display( ( CDlgCreateModule::DialogMode )iDlgMode, str, pHM, iMaxIndex );
	ResetTreeAndSelectHM(pHM);

	if( IDOK == iReturn )
	{
		// Name changed ?
		CString zzz = pHM->GetHMName();
		CString yyy = dlg.GetReference();

		if( '*' != yyy.GetAt( 0 ) )
		{
			pHM->SetHMName( L"*" );
		}

		if( pHM->GetHMName() != dlg.GetReference() && '*' != yyy.GetAt( 0 ) )
		{
			// Verify if the name is not already used.
			// Remark: we do only that if user has intentionally set a none automatic name (without '*' at the first position).
			bool fFlag = false;
			
			CTable *pPipTab = m_pTADS->GetpHydroModTable();
			ASSERT( NULL != pPipTab );

			if( NULL != m_pTADS->FindHydroMod( dlg.GetReference(), pPipTab, pHM ) )
			{
				fFlag = true;
			}

			if( true == fFlag )
			{
				CString str1;
				::FormatString( str1, IDS_HYDROMODREF_ALREADYUSED, dlg.GetReference() );
				AfxMessageBox( str1 );
				return false;
			}
			else
			{
				pHM->SetHMName( dlg.GetReference() );
				fAtLeastOneSettingChanged = true;
			}
		}

		// Force no HMCalc mode.
		pHM->SetHMCalcMode( false );

		// Description Changed ?
		if( pHM->GetDescription() != dlg.GetDescription() )
		{
			pHM->SetDescription( dlg.GetDescription() );
			fAtLeastOneSettingChanged = true;
		}

		// VDescrType Changed ?
		if( pHM->GetVDescrType() != dlg.GetVDescrType() )
		{
			pHM->SetVDescrType( dlg.GetVDescrType() );

			if( edt_KvCv == dlg.GetVDescrType() )
			{
				pHM->SetKvCv( dlg.GetKvCv() );
			}
			else
			{
				pHM->SetKvCv( 0.0 );
			}

			fAtLeastOneSettingChanged = true;
		}

		if( true == dlg.IsValveChanged() )
		{
			pHM->SetCBIValveID( dlg.GetSelTADBID() );
			pHM->SetKvCv( dlg.GetKvCv() );

			// Erase HM calc computed valves.
			if( NULL != pHM->GetpBv() )
			{
				pHM->DeleteBv( pHM->GetppBv() );
			}

			if( NULL != pHM->GetpCV() )
			{
				pHM->DeleteCv();
			}

			fAtLeastOneSettingChanged = true;
		}

		// Index has changed! Keep same parent...
		if( pHM->GetPos() != dlg.GetPosition() )
		{
			// Scan TADS to swap index in this branch.
			for( IDPTR IDPtr = pPHM->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pPHM->GetNext() )
			{
				if( ( ( CDS_HydroMod * )IDPtr.MP ) == pHM )
				{
					continue;
				}

				if( ( ( CDS_HydroMod * )IDPtr.MP )->GetPos() == dlg.GetPosition() )
				{
					( ( CDS_HydroMod * )IDPtr.MP )->SetPos( pHM->GetPos() );
					break;
				}
			}

			pHM->SetPos( dlg.GetPosition() );
			fAtLeastOneSettingChanged = true;
		}

		// Check if flow has changed when editing a valve (i.e. not a module).
		if( false == pHM->IsaModule() && pHM->GetQDesign() != dlg.GetFlow() )
		{
			pHM->SetQDesign( dlg.GetFlow() );
			fAtLeastOneSettingChanged = true;
		}

		// Check if presetting has changed.
		if( pHM->GetPresetting() != dlg.GetPresetting() )
		{
			pHM->SetPresetting( dlg.GetPresetting() );
			fAtLeastOneSettingChanged = true;
		}
	}

	return fAtLeastOneSettingChanged;
}

CDS_HydroMod *CDlgLeftTabProject::GetSelectedHM()
{
	if( NULL == m_hItem )
		return NULL;
	
	CData *pdata = (CData *)( m_Tree.GetObjectAt( m_hItem ) );
	if( true == pdata->IsHMFamily() )
		return ( (CDS_HydroMod *)pdata );
	return NULL;
}

void CDlgLeftTabProject::RefreshRightView()
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	if( NULL == m_hItem )
	{
		return;
	}
	
	m_eCurrentRightView = m_Tree.GetRightViewAt( m_hItem );

	switch( m_eCurrentRightView )
	{
		case CMainFrame::RightViewList::eRVGeneral:

			if( NULL != pRViewGen )
			{
				pRViewGen->SetRedraw();
			}

			break;

		case CMainFrame::RightViewList::eRVHMSumm:

			if( NULL != pRViewHMSumm )
			{
				pRViewHMSumm->SetRedraw();
			}

			break;

		case CMainFrame::RightViewList::eRVProj:
		case CMainFrame::RightViewList::eRVHMCalc:
			{
				// for avoiding pHM initialization to be skipped by case label.
				CDS_HydroMod *pHM = (CDS_HydroMod *)( m_Tree.GetObjectAt( m_hItem ) );

				// Test if the user clicked on a Module node.
				if( NULL != pHM )
				{
					if( NULL != pRViewHMCalc )
					{
						pRViewHMCalc->RedrawHM( pHM );
						pRViewHMCalc->GetSheetHMCalc()->UnSelectMultipleRows();
						pRViewHMCalc->GetSheetHMCalc()->SelectRow( pHM );
					}
				}
			}
			break;

		case CMainFrame::RightViewList::eRVLDSumm:

			if( NULL != pRViewLDSumm )
			{
				pRViewLDSumm->SetRedraw();
			}

			break;

		case CMainFrame::RightViewList::eRVChart:
			{
				// For avoiding pLD initialization to be skipped by case label.
				CLog *pLD = (CLog *)m_Tree.GetObjectAt( m_hItem );
				
				// Test if the user clicked on a Chart node.
				if( NULL != pLD )
				{
					if( NULL != pRViewChart )
					{
						pRViewChart->SetRedraw( pLD );
					}
				}
			}
			break;
	
		case CMainFrame::RightViewList::eRVLogData:
			{
				// For avoiding pLD initialization to be skipped by case label.
				CLog *pLD = (CLog *)m_Tree.GetObjectAt( m_hItem );

				// Test if the user clicked on a List node.
				if( NULL != pLD )
				{
					if( NULL != pRViewLogData )
					{
						pRViewLogData->SetRedraw( pLD );
					}
				}
			}
			break;

		case CMainFrame::RightViewList::eRVQuickMeas:
			{
				if( NULL != pRViewQuickMeas )
				{
					pRViewQuickMeas->SetRedraw();
				}
			}
			break;

		default:
			break;
	}

	// Display the right Ribbon Tab associated to the tree.
	switch( m_eCurrentRightView )
	{
		case CMainFrame::RightViewList::eRVChart:
			pMainFrame->GetRibbonBar()->ShowContextCategories( ID_LOGDATA_CATEGORY, TRUE );
			break;

		default:
			pMainFrame->GetRibbonBar()->ShowContextCategories( ID_LOGDATA_CATEGORY, FALSE );
			break;
	}

	// Restore the context in the regard of the current view.
	int iContextID = -1;

	if( false == GetToolsDockablePaneContextID( iContextID ) )
	{
		return;
	}

	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL != pclToolsDockablePane )
	{
		pclToolsDockablePane->RestoreContext( iContextID );
	}

	// Refresh right view when the TabDialogProj is displayed.
	CMainFrame *pFrame = static_cast<CMainFrame*>( AfxGetMainWnd() );
	pFrame->ActiveFormView( m_eCurrentRightView );
}

void CDlgLeftTabProject::OnButtoncbi() 
{
	// Save the current location.
	void *pData = m_Tree.GetObjectAt( m_hItem ); 
	unsigned int uid = 0;
	if( NULL != pData && true == ( (CData *)pData)->IsClass( _T("CDS_HydroMod") ) )
		uid = ( (CDS_HydroMod *)pData )->GetUid();
	CWizardCBI dlg;
	BeginWaitCursor();
	dlg.DisplayWizard();

	// Select the Project Tab.
	// This function does not send the AFX_WM_CHANGE_ACTIVE_TAB notification to the parent window of the tab control.
	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiProj );
	}

	_ResetTree( uid );
}

BEGIN_MESSAGE_MAP( CDlgLeftTabProject, CDlgLeftTabBase )
	ON_WM_SIZE()
	
	ON_NOTIFY( NM_RCLICK, IDC_TREE, OnRclickTree )
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREE, OnSelchangedTree )

	ON_COMMAND( ID_TABPROJ_CREATEIN, OnFltMenuCreate )
	ON_COMMAND( ID_TABPROJ_ADDVALVE, OnFltMenuAddValve )
	ON_COMMAND( ID_TABPROJ_FINDINDEXCIRCUIT, OnFltMenuFindIndexCircuit )
	ON_COMMAND( ID_TABPROJ_EDIT, OnFltMenuEdit )
	ON_COMMAND( ID_TABPROJ_RENAME, OnFltMenuRename )
	ON_COMMAND( ID_TABPROJ_DELETE, OnFltMenuDelete )
	ON_COMMAND( ID_TABPROJ_DELETE_LD, OnFltMenuDelete )
	ON_COMMAND( ID_TABPROJ_CUT, OnFltMenuCut )
	ON_COMMAND( ID_TABPROJ_COPY, OnFltMenuCopy )
	ON_COMMAND( ID_TABPROJ_PASTEIN, OnFltMenuPasteIn )
	ON_COMMAND( ID_TABPROJ_PASTEBEFORE, OnFltMenuPasteBefore )
	ON_COMMAND( ID_TABPROJ_LOCK, OnFltMenuLock )
	ON_COMMAND( ID_TABPROJ_UNLOCK, OnFltMenuUnLock )
	ON_COMMAND( ID_TABPROJ_LOCKMODULE, OnFltMenuLockRecursive )
	ON_COMMAND( ID_TABPROJ_UNLOCKMODULE, OnFltMenuUnLockRecursive )
	ON_COMMAND( ID_TABPROJ_PRINT, OnFltMenuPrint )
	ON_COMMAND( ID_TABPROJ_PRINTRECURSIVE, OnFltMenuPrintRecursive )
	ON_COMMAND( ID_TABPROJ_EXPORT, OnFltMenuExport )
	ON_COMMAND( ID_TABPROJ_EXPORT_QM, OnFltMenuExport )
	ON_COMMAND( ID_TABPROJ_EXPORTRECURSIVE, OnFltMenuExportRecursive )
	ON_COMMAND( ID_TABPROJ_EXPORT_CHART, OnFltMenuExportChart )
	ON_COMMAND( ID_TABPROJ_EXPORT_LDLIST, OnFltMenuExportLDList )

// 	ON_COMMAND( ID_EDIT_CUT, OnEditCut )
// 	ON_COMMAND( ID_EDIT_COPY, OnEditCopy )
// 	ON_COMMAND( ID_EDIT_PASTE, OnEditPaste )
// 	
// 	ON_UPDATE_COMMAND_UI( ID_EDIT_CUT, OnUpdateEditCut )
// 	ON_UPDATE_COMMAND_UI( ID_EDIT_COPY, OnUpdateEditCopy )
// 	ON_UPDATE_COMMAND_UI( ID_EDIT_PASTE, OnUpdateEditPaste )

	ON_UPDATE_COMMAND_UI_RANGE( ID_TABPROJ_CREATEIN, ID_TABPROJ_EXPORT_LDLIST, OnUpdateMenuText )

	ON_MESSAGE( WM_USER_NEWDOCUMENT, OnNewDocument )
	ON_MESSAGE( WM_USER_PROJECTREFCHANGE, OnProjectRefChange )
	ON_MESSAGE( WM_USER_RESETHMTREE, OnResetTreeNeeded )
	ON_MESSAGE( WM_USER_RESETHMTREECREATE, OnResetTreeNeededCreate )
	ON_MESSAGE( WM_USER_RESETHMTREEEDIT, OnResetTreeNeededEdit )
	ON_MESSAGE( WM_USER_TECHPARCHANGE, OnTechParamChange )
	ON_MESSAGE( WM_USER_UNITCHANGE, OnUnitChange )
	ON_MESSAGE( WM_USER_WATERCHANGE, OnWaterChange )

END_MESSAGE_MAP()

void CDlgLeftTabProject::DoDataExchange( CDataExchange* pDX )
{
	CDlgLeftTabBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TREE, m_Tree );
}

BOOL CDlgLeftTabProject::OnInitDialog() 
{
	pDlgLeftTabProject = this;
	CDlgLeftTabBase::OnInitDialog();
	
	// Initialize member variables.
	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();
	
	// Retrieve a copy of the 'Project tree' image list.
	m_pclTreeImageList = TASApp.GetpRCImageManager()->GetImageListCopy( CRCImageManager::ILN_ProjectTree );
	if( NULL == m_pclTreeImageList )
		return FALSE;
	
	m_pclTreeImageList->SetBkColor( CLR_NONE );

	IMAGEINFO rImageInfo;
	m_pclTreeImageList->GetImageInfo( (int)CRCImageManager::ILPT_OverlayExclamation, &rImageInfo );
	
	// Exclamation.
	m_pclTreeImageList->SetOverlayImage( (int)CRCImageManager::ILPT_OverlayExclamation, (int)OverlayMaskIndex::OMI_Exclamation );
	
	// CrossMark.
	m_pclTreeImageList->SetOverlayImage( (int)CRCImageManager::ILPT_OverlayCrossMark, (int)OverlayMaskIndex::OMI_CrossMark );

	// PV.
	m_pclTreeImageList->SetOverlayImage( (int)CRCImageManager::ILPT_OverlayPartnerValve, (int)OverlayMaskIndex::OMI_PartnerValve );

	// Edition.
	m_pclTreeImageList->SetOverlayImage((int)CRCImageManager::ILPT_OverlayEditCircuit, (int)OverlayMaskIndex::OMI_EditCircuit);

	// Adding.
	m_pclTreeImageList->SetOverlayImage((int)CRCImageManager::ILPT_OverlayNewCircuit, (int)OverlayMaskIndex::OMI_NewCircuit);


	m_Tree.SetImageList( m_pclTreeImageList, TVSIL_NORMAL );

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgLeftTabProject::OnCommand( WPARAM wParam, LPARAM lParam )
{
	CWnd *pWnd = GetFocus();

	switch( wParam )
	{
	case IDOK:
		if( pWnd != GetDlgItem( IDOK ) )
			return FALSE;
		break;

	case IDCANCEL:
		if( pWnd != GetDlgItem( IDCANCEL ) )
			return FALSE;
		break;
	}

	return CDlgLeftTabBase::OnCommand( wParam, lParam );
}

BOOL CDlgLeftTabProject::PreTranslateMessage( MSG* pMsg )
{
	if( TRUE == IsWindowVisible() && NULL != pRViewHMCalc && pMsg->message == WM_KEYDOWN )
	{
		if( GetKeyState( VK_ESCAPE ) < 0 && CRViewHMCalc::LastOperation::Cut == pRViewHMCalc->GetLastCopyCutOperation() && true == pRViewHMCalc->IsFlagCutSet() )
		{
			bool fIsItemChanged = pRViewHMCalc->ResetFlagCut();
			m_pTADS->CleanClipboard();
			pRViewHMCalc->SetLastCopyCutOperation( CRViewHMCalc::LastOperation::Undefined );

			if( true == fIsItemChanged )
			{
				RefreshRightView();
			}
		}
	}

	return CDlgLeftTabBase::PreTranslateMessage( pMsg );
}

void CDlgLeftTabProject::OnSize( UINT nType, int cx, int cy )
{
	CDlgLeftTabBase::OnSize( nType, cx, cy );
	
	// Get TabCtrl window size.
	CRect rect;
	// Get the size in pixel of the TA Logo.
	CRect rectBmp = CRect( 0, 0, 32, 48 );
	rectBmp = GetTALogoSize();
	CTabCtrl* pTabCtrl = (CTabCtrl*)GetParent();
	pTabCtrl->GetClientRect( rect );
	pTabCtrl->AdjustRect( FALSE, &rect );
	pTabCtrl->ClientToScreen( rect );
	ScreenToClient( rect );

	// Test on existence of TreeCtrl because OnSize is already called before the controls of the dialog are created.
	CTreeCtrl *pTreeCtrl = (CTreeCtrl*)GetDlgItem( IDC_TREE );
	if( NULL != pTreeCtrl )
	{
		// Create an offset to take into account the new TabMFC.
		CDC* pDC = GetDC();
		int DeviceCaps = pDC->GetDeviceCaps( LOGPIXELSY );

		// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
		ReleaseDC( pDC );

		// Remark: 96 Dpi as reference.
		int iOffSet = 20 * DeviceCaps / 96;
		int iHeightOf7 = (int)( ( ( rectBmp.Height() * 1.5 ) + 6 ) * DeviceCaps / 96.0 );
		
		// Resize 'm_Tree'.
		m_Tree.SetWindowPos( &wndTop, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top - iHeightOf7, SWP_SHOWWINDOW );
	}
}

void CDlgLeftTabProject::OnRclickTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;
	
	// Determine where does click come from.
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = GET_X_LPARAM( dwpos );
	ht.pt.y = GET_Y_LPARAM( dwpos );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( NULL == ht.hItem )
	{
		return;
	}
	
	if( m_hItem != ht.hItem )
	{
		m_Tree.SelectItem( ht.hItem );
	}

	m_mapUpdateMenuTooltip.clear();

	// Enable all Menu ResID by default.
	pMainFrame->EnableMenuResID( NULL );

	// Load and prepare the context menu.
	CMenu menu;
	menu.LoadMenu( IDR_FLTMENU_TABPROJ );
	CMenu *pContextMenu = menu.GetSubMenu( 0 );

	// Load correct strings.
	CString str;
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_CREATEIN );
	pContextMenu->ModifyMenu( ID_TABPROJ_CREATEIN, MF_BYCOMMAND, ID_TABPROJ_CREATEIN, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_CREATEBEFORE );
	pContextMenu->ModifyMenu( ID_TABPROJ_CREATEBEFORE, MF_BYCOMMAND, ID_TABPROJ_CREATEBEFORE, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_ADDVALVE );
	pContextMenu->ModifyMenu( ID_TABPROJ_ADDVALVE, MF_BYCOMMAND, ID_TABPROJ_ADDVALVE, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EDIT );
	pContextMenu->ModifyMenu( ID_TABPROJ_EDIT, MF_BYCOMMAND, ID_TABPROJ_EDIT, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_RENAME );
	pContextMenu->ModifyMenu( ID_TABPROJ_RENAME, MF_BYCOMMAND, ID_TABPROJ_RENAME, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_DELETE );
	pContextMenu->ModifyMenu( ID_TABPROJ_DELETE, MF_BYCOMMAND, ID_TABPROJ_DELETE, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_DELETE_LD );
	pContextMenu->ModifyMenu( ID_TABPROJ_DELETE_LD, MF_BYCOMMAND, ID_TABPROJ_DELETE_LD, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_CUT );
	pContextMenu->ModifyMenu( ID_TABPROJ_CUT, MF_BYCOMMAND, ID_TABPROJ_CUT, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_COPY );
	pContextMenu->ModifyMenu( ID_TABPROJ_COPY, MF_BYCOMMAND, ID_TABPROJ_COPY, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_PASTEIN );
	pContextMenu->ModifyMenu( ID_TABPROJ_PASTEIN, MF_BYCOMMAND, ID_TABPROJ_PASTEIN, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_PASTEBEFORE );
	pContextMenu->ModifyMenu( ID_TABPROJ_PASTEBEFORE, MF_BYCOMMAND, ID_TABPROJ_PASTEBEFORE, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_LOCK );
	pContextMenu->ModifyMenu( ID_TABPROJ_LOCK, MF_BYCOMMAND, ID_TABPROJ_LOCK, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_UNLOCK );
	pContextMenu->ModifyMenu( ID_TABPROJ_UNLOCK, MF_BYCOMMAND, ID_TABPROJ_UNLOCK, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_LOCKMODULE );
	pContextMenu->ModifyMenu( ID_TABPROJ_LOCKMODULE, MF_BYCOMMAND, ID_TABPROJ_LOCKMODULE, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_UNLOCKMODULE );
	pContextMenu->ModifyMenu( ID_TABPROJ_UNLOCKMODULE, MF_BYCOMMAND, ID_TABPROJ_UNLOCKMODULE, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_PRINT );
	pContextMenu->ModifyMenu( ID_TABPROJ_PRINT, MF_BYCOMMAND, ID_TABPROJ_PRINT, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_PRINTRECURSIVE );
	pContextMenu->ModifyMenu( ID_TABPROJ_PRINTRECURSIVE, MF_BYCOMMAND, ID_TABPROJ_PRINTRECURSIVE, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EXPORT );
	pContextMenu->ModifyMenu( ID_TABPROJ_EXPORT, MF_BYCOMMAND, ID_TABPROJ_EXPORT, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EXPORT_QM );
	pContextMenu->ModifyMenu( ID_TABPROJ_EXPORT_QM, MF_BYCOMMAND, ID_TABPROJ_EXPORT_QM, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EXPORTRECURSIVE );
	pContextMenu->ModifyMenu( ID_TABPROJ_EXPORTRECURSIVE, MF_BYCOMMAND, ID_TABPROJ_EXPORTRECURSIVE, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EXPORT_CHART );
	pContextMenu->ModifyMenu( ID_TABPROJ_EXPORT_CHART, MF_BYCOMMAND, ID_TABPROJ_EXPORT_CHART, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_EXPORT_LDLIST );
	pContextMenu->ModifyMenu( ID_TABPROJ_EXPORT_LDLIST, MF_BYCOMMAND, ID_TABPROJ_EXPORT_LDLIST, str );
	
	str = pMainFrame->GetSplitStringFromResID( ID_TABPROJ_FINDINDEXCIRCUIT );
	pContextMenu->ModifyMenu( ID_TABPROJ_FINDINDEXCIRCUIT, MF_BYCOMMAND, ID_TABPROJ_FINDINDEXCIRCUIT, str );

	// TODO: Not activated for the moment!
	pContextMenu->RemoveMenu( ID_TABPROJ_CREATEBEFORE, MF_BYCOMMAND );

	pContextMenu->RemoveMenu( ID_TABPROJ_GETFULLINFO, MF_BYCOMMAND );
	
	if( CMainFrame::RightViewList::eRVGeneral == m_Tree.GetRightViewAt( ht.hItem ) )
	{
		// scan all menu item by position and remove all unnecessary items.
		int i = 0;

		while( i < (int)pContextMenu->GetMenuItemCount() )
		{
			if( ID_TABPROJ_RENAME != pContextMenu->GetMenuItemID( i ) )
			{
				pContextMenu->RemoveMenu( i, MF_BYPOSITION );
			}
			else
			{
				i++;
			}
		}
	}
	else if( CMainFrame::RightViewList::eRVHMSumm == m_Tree.GetRightViewAt( ht.hItem ) || CMainFrame::RightViewList::eRVHMCalc == m_Tree.GetRightViewAt( ht.hItem ) )
	{
		int iNbrProductSelected = (int)m_vecSelectedItem.size();
		int iNbrProductInClipboard = m_pTADS->GetCountHMClipboard();
		
		CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
		ASSERT( NULL != pPrjParam );
		
		CDS_HydroMod *pHMRightClicked = dynamic_cast<CDS_HydroMod *>( (CTable *)m_Tree.GetObjectAt( ht.hItem ) );
		
		bool bIsRibbonHMCalcChecked = pMainFrame->IsHMCalcChecked();
		bool bIsProjectFrozen = pPrjParam->IsFreezed();

		// Count the number of HM.
		CTable *pTab = TASApp.GetpTADS()->GetpHydroModTable();
		int iHMCount = 0;

		if( NULL != pTab )
		{
			iHMCount = TASApp.GetpTADS()->GetHMCount( pTab );
		}

		// Check if there is at least one module child under the selected lines (allow to activate or not 'Print recursive' or
		// 'Export recursive' menu items).
		// Also check if all selected lines contains at least one module and contains at least one child.
		// PAY ATTENTION: Clipboard can contain hydromod that user has copy (or cut) thanks to context menu in the 'RVHMCalc'. Thus, it is
		//                absolutely possible to have also circuit in clipboard and not only module!
		bool bChildModuleExist = false;
		bool bRootItemInSelection = false;
		bool bOnlyLevel0InSelection = true;
		bool bIsTerminalUnit = false;
		CDS_HydroMod *pclFirstHMInSelection = NULL;

		for( int iLoopSelectedLine = 0; iLoopSelectedLine < iNbrProductSelected; iLoopSelectedLine++ )
		{
			CTable *pHMTable = (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] );

			if( m_vecSelectedItem[iLoopSelectedLine] == m_hHNBranch )
			{
				// The current selected line is the main root.
				bRootItemInSelection = true;
			}
			else
			{
				// If it's not the root item, it's then a module. In the tree there is only modules.
		
				CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( pHMTable );
				
				if( NULL == pHM )
				{
					continue;
				}

				if (false == pHM->IsaModule())
				{
					bIsTerminalUnit = true;
				}

				if( NULL == pclFirstHMInSelection || pHM->GetLevel() < pclFirstHMInSelection->GetLevel() || pHM->GetPos() < pclFirstHMInSelection->GetPos() )
				{
					pclFirstHMInSelection = pHM;
				}

				if( pHM->GetLevel() > 0 )
				{
					bOnlyLevel0InSelection = false;
				}
			}

			for( IDPTR ChildIDPtr = pHMTable->GetFirst(); NULL != ChildIDPtr.MP && false == bChildModuleExist; ChildIDPtr = pHMTable->GetNext() )
			{
				CDS_HydroMod *pChildHM = dynamic_cast<CDS_HydroMod *>( (CData *)ChildIDPtr.MP );
				
				if( NULL != pChildHM )
				{
					if( true == pChildHM->IsaModule() )
					{
						bChildModuleExist = true;
					}
				}
			}
		}

		// Now verify clipboard content.
		bool bModuleExistInClipboard = false;
		bool bCircuitExistInClipboard = false;

		if( iNbrProductInClipboard > 0 )
		{
			IDPTR IDPtr = m_pTADS->GetFirstHMFromClipBoard();

			while( NULL != IDPtr.MP )
			{
				CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)( IDPtr.MP ) );

				if( NULL != pHM )
				{
					if( true == pHM->IsaModule() )
					{
						bModuleExistInClipboard = true;
					}
					else
					{
						bCircuitExistInClipboard = true;
					}
				}
				
				IDPtr = m_pTADS->GetNextHMFromClipBoard();
			}
		}

		// Two types of context menu can be displayed !!!!
		if( false == bIsRibbonHMCalcChecked )
		{
			// We are in CBI mode.

			if( NULL != pHMRightClicked && true == pHMRightClicked->IsForHub() )
			{
				return;
			}
		
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Create module' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (false == bIsTerminalUnit)
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_CREATEIN );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_CREATEIN );
			}

			str = pMainFrame->GetSplitStringFromResID( IDS_TABPROJ_CREATEMODULE );
			pContextMenu->ModifyMenu( ID_TABPROJ_CREATEIN, MF_BYCOMMAND, ID_TABPROJ_CREATEIN, str );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Add valves' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// User is allowed to add valve everywhere except on the main root.
			if( false == bRootItemInSelection && false == bIsTerminalUnit)
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_ADDVALVE );
			}
			else
			{
				pContextMenu->RemoveMenu(ID_TABPROJ_ADDVALVE, MF_BYCOMMAND);
			}
			
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Edit' and 'Rename' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// With the 'Edit' item, we launch 'CDlgCreateModule' and we authorize user to change reference (name of the module).
			// Thus we can also enable 'Rename'.
			if( 1 == iNbrProductSelected && false == bRootItemInSelection )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_EDIT );
				pMainFrame->EnableMenuResID( ID_TABPROJ_RENAME );
				pMainFrame->EnableMenuResID( ID_TABPROJ_FINDINDEXCIRCUIT );
			}
			else
			{
				// If more than one line are selected, we disable these two menu items.
				pMainFrame->DisableMenuResID( ID_TABPROJ_EDIT );
				pMainFrame->DisableMenuResID( ID_TABPROJ_RENAME );
				pContextMenu->RemoveMenu( ID_TABPROJ_FINDINDEXCIRCUIT, MF_BYCOMMAND );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Delete the logging' menu item unused here.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pContextMenu->RemoveMenu( ID_TABPROJ_DELETE_LD, MF_BYCOMMAND );
		
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Delete' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// We can always enable 'Delete' menu item and we have these cases:
			//  - If the selected line is a module, all the branch is deleted (no need to execute other selected lines under this module).
			//  - If the selected line is a circuit, simply delete this circuit.
			// Remark: user can delete a object that is in the cut mode. In this case, only this object will be erased and the cut operation
			//         will be reseted.
			pMainFrame->EnableMenuResID( ID_TABPROJ_DELETE );

			if( true == bRootItemInSelection )
			{
				// If a the root item is in the selection, we disable the 'Delete' menu item.
				pMainFrame->DisableMenuResID( ID_TABPROJ_DELETE );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Cut' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// User can cut in the following cases:
			//  - If there is only one module selected.
			//  - If more than one module are selected but they are all siblings.
			// Remark: - if more than one line is selected, 'pHMRightClicked' is NULL.
			//         - if at least one object in the selection is in the cut mode, it's not a problem. The previous cut operation
			//           will be reseted with the new one.
			pMainFrame->DisableMenuResID( ID_TABPROJ_CUT );

			if( false == bRootItemInSelection )
			{
				if( 1 == iNbrProductSelected || false == bChildModuleExist )
				{
					pMainFrame->EnableMenuResID( ID_TABPROJ_CUT );
				}
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Copy' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// User can copy what he wants EXCEPT if the root item is selected
			// Remark: if at least one object in the selection is in the cut mode, it's not a problem. The previous cut operation
			//         will be reseted with the copy operation.
			if( false == bRootItemInSelection )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_COPY );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_COPY );
			}
		
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Paste', 'Paste in' and 'Paste before' menu items.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			bool bAddPasteIn = false;
			bool bAddPasteBefore = false;
			bool bDisablePasteIn = false;
			CString strPasteIn( _T("") );
			CString strPasteBefore( _T("") );

			if( iNbrProductInClipboard > 0 )
			{
				if( 1 == iNbrProductSelected )
				{
					if( true == bRootItemInSelection && false == bCircuitExistInClipboard )
					{
						// There is only one line selected and it's the main root (Hydraulic network) and there is no circuit 
						// in the clipboard, user can paste in.
						FormatString( strPasteIn, IDS_TABPROJ_PASTEIN, TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK ), TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK ) );
						bAddPasteIn = true;
					}
					else if( false == bRootItemInSelection )
					{
						// There is only one line selected and it's not the main root (Hydraulic network) user can paste what he wants.
						FormatString( strPasteIn, IDS_TABPROJ_PASTEIN, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
						bAddPasteIn = true;
					}

					if( false == bRootItemInSelection && false == bCircuitExistInClipboard )
					{
						// User can insert before if the module is not the main root (Hydraulic network).
						// Remark: if there is at least one circuit in the clipboard, it's impossible to insert it before a module. Because
						//         a circuit must be in a module!
						FormatString( strPasteBefore, IDS_TABPROJ_PASTEBEFORE, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
						bAddPasteBefore = true;
					}
				}
				else
				{
					// If more than one modules are selected user can paste in each of them.
					strPasteIn = TASApp.LoadLocalizedString( IDS_TABPROJ_PASTEINALLMODULES );
					bAddPasteIn = true;

					if( false == bRootItemInSelection && false == bCircuitExistInClipboard )
					{
						// User can insert before if the module is not the main root (Hydraulic network).
						// Remark: if there is at least one circuit in the clipboard, it's impossible to insert it before a module. Because
						//         a circuit must be in a module!
						FormatString( strPasteBefore, IDS_TABPROJ_PASTEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
						bAddPasteBefore = true;
					}
				}
			
				// Particular case for 'Cut' operation.
				if( true == m_Tree.IsAtLeastOneItemCut() )
				{
					// We must verify that that the line (or lines) selected is not the same of the objects in the clipboard or one of its children.
				
					// Run all objects in the clipboard.
					bool bForbidden = false;
					bool bOnItself = false;
					IDPTR IDPtr = m_pTADS->GetFirstHMFromClipBoard();

					while( NULL != IDPtr.MP && false == bForbidden )
					{
						CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( (CData *)IDPtr.MP );

						// Run all line selected.
						for( int iLoopSelectedLine = 0; iLoopSelectedLine < iNbrProductSelected && false == bForbidden; iLoopSelectedLine++ )
						{
							CDS_HydroMod *pHMSelected = (CDS_HydroMod *)( m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) );

							if( NULL != pHMSelected )
							{
								// Remark: compare with the name because pointer on the HM in clipboard is never the same as in the circuit
								//         (because to save in the clipboard we create a new object).
								if( 0 == _tcscmp( pHMSelected->GetHMName(), pHMInClipboard->GetHMName() ) )
								{
									// Try to cut/paste on object on itself! That's not possible.
									bForbidden = true;
									bOnItself = true;
								}
								else
								{
								
									// Check if 'pHMSelected' is not a child of 'pHMInClipboard'.
									CDS_HydroMod *pParentHMSelected = dynamic_cast<CDS_HydroMod *>( (CData *)pHMSelected->GetIDPtr().PP );

									while( NULL != pParentHMSelected && false == bForbidden )
									{
										// Remark: compare with the name because pointer on the HM in clipboard is never the same as in the circuit
										//         (because to save in the clipboard we create a new object).
										if( 0 == _tcscmp( pParentHMSelected->GetHMName(), pHMInClipboard->GetHMName() ) )
										{
											bForbidden = true;
										}
										else
										{
											pParentHMSelected = dynamic_cast<CDS_HydroMod *>( (CData *)pParentHMSelected->GetIDPtr().PP );
										}
									}
								}
							}
						}
						IDPtr = m_pTADS->GetNextHMFromClipBoard();
					}

					if( true == bForbidden )
					{
						// If user tries to paste an object at the level of one of its children...
						if( false == bOnItself )
						{
							// We also disable the possibility to 'Paste before' this child.
							bAddPasteBefore = false;
						}

						if( false == bAddPasteBefore )
						{
							// If 'Paste before' menu item is not displayed, we add 'Paste' menu item but we disable it only.
							strPasteIn = TASApp.LoadLocalizedString( ID_TABPROJ_PASTEIN );
							bAddPasteIn = true;
							bDisablePasteIn = true;
						}
						else
						{
							// If 'Paste before' is already exist, we don't show 'Paste in' menu item.
							bAddPasteIn = false;
						}
					}
				}
			}
			else
			{
				// Add 'Paste module' menu item but disabled it.
				strPasteIn = TASApp.LoadLocalizedString( ID_TABPROJ_PASTEIN );
				bAddPasteIn = true;
				bDisablePasteIn = true;
			}

			if( true == bAddPasteIn )
			{
				CString strItemText = strPasteIn.Right( strPasteIn.GetLength() - strPasteIn.Find( '\n' ) );
				pContextMenu->ModifyMenu( ID_TABPROJ_PASTEIN, MF_BYCOMMAND, ID_TABPROJ_PASTEIN, strItemText );
				m_mapUpdateMenuTooltip[ID_TABPROJ_PASTEIN] = strPasteIn.Left( strPasteIn.Find( '\n' ) );
				
				if( false == bDisablePasteIn && false == bIsTerminalUnit ) 
				{
					pMainFrame->EnableMenuResID( ID_TABPROJ_PASTEIN );
				}
				else
				{
					pMainFrame->DisableMenuResID( ID_TABPROJ_PASTEIN );
				}
			}
			else
			{
				pContextMenu->RemoveMenu( ID_TABPROJ_PASTEIN, MF_BYCOMMAND );
			}

			if( true == bAddPasteBefore )
			{
				CString strItemText = strPasteBefore.Right( strPasteBefore.GetLength() - strPasteBefore.Find( '\n' ) );
				pContextMenu->ModifyMenu( ID_TABPROJ_PASTEBEFORE, MF_BYCOMMAND, ID_TABPROJ_PASTEBEFORE, strItemText );
				m_mapUpdateMenuTooltip[ID_TABPROJ_PASTEBEFORE] = strPasteBefore.Left( strPasteBefore.Find( '\n' ) );
				pMainFrame->EnableMenuResID( ID_TABPROJ_PASTEBEFORE );
			}
			else
			{
				pContextMenu->RemoveMenu( ID_TABPROJ_PASTEBEFORE, MF_BYCOMMAND );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Lock', 'Unlock', 'Lock recursive' and 'Unlock recursive' menu items.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// No possibility to lock/unlock if we are in CBI mode.
			pContextMenu->RemoveMenu( ID_TABPROJ_LOCK, MF_BYCOMMAND );
			pContextMenu->RemoveMenu( ID_TABPROJ_UNLOCK, MF_BYCOMMAND );
			pContextMenu->RemoveMenu( ID_TABPROJ_LOCKMODULE, MF_BYCOMMAND );
			pContextMenu->RemoveMenu( ID_TABPROJ_UNLOCKMODULE, MF_BYCOMMAND );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Print' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( 1 == iNbrProductSelected && false == bRootItemInSelection )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_PRINT );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_PRINT );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Print recursive' is enabled if there is at least one child module under the current module.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( 1 == iNbrProductSelected && false == bRootItemInSelection && true == bChildModuleExist )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_PRINTRECURSIVE );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_PRINTRECURSIVE );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Export' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( 1 == iNbrProductSelected )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_EXPORT );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_EXPORT );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Export' menu item for quick measurement unused here.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_QM, MF_BYCOMMAND );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Export recursive' is enabled if there is at least one child module under the current module.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( 1 == iNbrProductSelected && true == bChildModuleExist )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_EXPORTRECURSIVE );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_EXPORTRECURSIVE );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Export logged data chart' and 'Export logged data list' menu items unused here.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_CHART, MF_BYCOMMAND );
			pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_LDLIST, MF_BYCOMMAND );
		}
		else
		{
			// We are in HMCalc mode.

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Create module' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// In HM calculation mode, we change 'Create module' to 'Add circuit'.
			pMainFrame->EnableMenuResID( ID_TABPROJ_CREATEIN );
			CString str = TASApp.LoadLocalizedString( IDS_ADDCIRCUIT );
			pContextMenu->ModifyMenu( ID_TABPROJ_CREATEIN, MF_BYCOMMAND, ID_TABPROJ_CREATEIN, str );

			// If the project is frozen user can't create module.
			if( true == bIsProjectFrozen )
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_CREATEIN );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Add valve' can be done only in CBI mode!
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pContextMenu->RemoveMenu( ID_TABPROJ_ADDVALVE, MF_BYCOMMAND );
			
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Edit' and 'Rename' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// With the 'Edit' item, we launch 'CDlgCreateModule' and we authorize user to change reference (name of the module).
			// Thus we can also enable 'Rename'.
			if( 1 == iNbrProductSelected && false == bRootItemInSelection )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_EDIT );
				pMainFrame->EnableMenuResID( ID_TABPROJ_RENAME );
				pMainFrame->EnableMenuResID( ID_TABPROJ_FINDINDEXCIRCUIT );
			}
			else
			{
				// If more than one line are selected, we disable these two menu items.
				pMainFrame->DisableMenuResID( ID_TABPROJ_EDIT );
				pMainFrame->DisableMenuResID( ID_TABPROJ_RENAME );
				pContextMenu->RemoveMenu( ID_TABPROJ_FINDINDEXCIRCUIT, MF_BYCOMMAND );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Delete' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Remark: user can delete a object that is in the cut mode. In this case, only this object will be erased and the cut operation
			//         will be reseted.
			if( false == bIsProjectFrozen && false == bRootItemInSelection )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_DELETE );
			}
			else
			{
				// If the project is frozen or if the root item is in the selection, we disable this menu item.
				pMainFrame->DisableMenuResID( ID_TABPROJ_DELETE );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Delete the logging' menu item unused here.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pContextMenu->RemoveMenu( ID_TABPROJ_DELETE_LD, MF_BYCOMMAND );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Cut' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// User can cut in the following cases:
			//  - If there is only one module selected.
			//  - If more than one module are selected but they are all siblings.
			// Remark: - if more than one line is selected, 'pHMRightClicked' is NULL.
			//         - if at least one object in the selection is in the cut mode, it's not a problem. The previous cut operation
			//           will be reseted with the new one.
			pMainFrame->DisableMenuResID( ID_TABPROJ_CUT );

			if( false == bIsProjectFrozen && false == bRootItemInSelection )
			{
				if( 1 == iNbrProductSelected || false == bChildModuleExist )
				{
					pMainFrame->EnableMenuResID( ID_TABPROJ_CUT );
				}
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Copy' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// User can copy what he wants EXCEPT if the root item is selected.
			// Remark: if at least one object in the selection is in the cut mode, it's not a problem. The previous cut operation
			//         will be reseted with the copy operation.
			if( false == bIsProjectFrozen && false == bRootItemInSelection )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_COPY );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_COPY );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Paste', 'Paste in' and 'Paste before' menu items.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			bool bAddPasteIn = false;
			bool bAddPasteBefore = false;
			bool bDisablePasteIn = false;
			CString strPasteIn( _T("") );
			CString strPasteBefore( _T("") );

			if( false == bIsProjectFrozen && iNbrProductInClipboard > 0 )
			{
				if( 1 == iNbrProductSelected )
				{
					// If there is only one line selected and it's a module, user can paste in.
					if( true == bRootItemInSelection && false == bCircuitExistInClipboard )
					{
						// There is only one line selected and it's the main root (Hydraulic network) and there is no circuit 
						// in the clipboard, user can paste in.
						FormatString( strPasteIn, IDS_TABPROJ_PASTEIN, TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK ), TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK ) );
						bAddPasteIn = true;
					}
					else if( false == bRootItemInSelection )
					{
						// There is only one line selected and it's not the main root (Hydraulic network) user can paste what he wants.
						FormatString( strPasteIn, IDS_TABPROJ_PASTEIN, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
						bAddPasteIn = true;
					}
					
					if( false == bRootItemInSelection && false == bCircuitExistInClipboard )
					{
						// User can insert before if the module is not the main root (Hydraulic network).
						// Remark: if there is at least one circuit in the clipboard, it's impossible to insert it before a module. Because
						//         a circuit must be in a module!
						FormatString( strPasteBefore, IDS_TABPROJ_PASTEBEFORE, pHMRightClicked->GetHMName(), pHMRightClicked->GetHMName() );
						bAddPasteBefore = true;
					}
				}
				else
				{
					// There are only modules selected, user can paste in each modules.
					strPasteIn = TASApp.LoadLocalizedString( IDS_TABPROJ_PASTEINALLMODULES );
					bAddPasteIn = true;
					
					if( false == bRootItemInSelection && false == bCircuitExistInClipboard )
					{
						// User can insert before if the module is not the main root (Hydraulic network).
						// Remark: if there is at least one circuit in the clipboard, it's impossible to insert it before a module. Because
						//         a circuit must be in a module!
						FormatString( strPasteBefore, IDS_TABPROJ_PASTEBEFORE, pclFirstHMInSelection->GetHMName(), pclFirstHMInSelection->GetHMName() );
						bAddPasteBefore = true;
					}
				}

				// Particular case for 'Cut' operation.
				if( true == m_Tree.IsAtLeastOneItemCut() )
				{
					// We must verify that that the line (or lines) selected is not the same of the objects in the clipboard or one of its children.
				
					// Run all objects in the clipboard.
					bool bForbidden = false;
					bool bOnItself = false;
					IDPTR IDPtr = m_pTADS->GetFirstHMFromClipBoard();
					
					while( NULL != IDPtr.MP && false == bForbidden )
					{
						CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( (CData *)IDPtr.MP );
						
						// Run all line selected.
						for( int iLoopSelectedLine = 0; iLoopSelectedLine < iNbrProductSelected && false == bForbidden; iLoopSelectedLine++ )
						{
							CDS_HydroMod *pHMSelected = (CDS_HydroMod *)( m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) );

							if( NULL != pHMSelected )
							{
								// Remark: compare with the name because pointer on the HM in clipboard is never the same as in the circuit
								//         (because to save in the clipboard we create a new object).
								if( 0 == _tcscmp( pHMSelected->GetHMName(), pHMInClipboard->GetHMName() ) )
								{
									// Try to cut/paste on object on itself! That's not possible.
									bForbidden = true;
									bOnItself = true;
								}
								else
								{
								
									// Check if 'pHMSelected' is not a child of 'pHMInClipboard'.
									CDS_HydroMod *pParentHMSelected = dynamic_cast<CDS_HydroMod *>( (CData *)pHMSelected->GetIDPtr().PP );

									while( NULL != pParentHMSelected && false == bForbidden )
									{
										// Remark: compare with the name because pointer on the HM in clipboard is never the same as in the circuit
										//         (because to save in the clipboard we create a new object).
										if( 0 == _tcscmp( pParentHMSelected->GetHMName(), pHMInClipboard->GetHMName() ) )
										{
											bForbidden = true;
										}
										else
										{
											pParentHMSelected = dynamic_cast<CDS_HydroMod *>( (CData *)pParentHMSelected->GetIDPtr().PP );
										}
									}
								}
							}
						}
						IDPtr = m_pTADS->GetNextHMFromClipBoard();
					}

					if( true == bForbidden )
					{
						// If user tries to paste an object at the level of one of its children...
						if( false == bOnItself )
						{
							// We also disable the possibility to 'Paste before' this child.
							bAddPasteBefore = false;
						}

						if( false == bAddPasteBefore )
						{
							// If 'Paste before' menu item is not displayed, we add 'Paste' menu item but we disable it only.
							strPasteIn = TASApp.LoadLocalizedString( ID_TABPROJ_PASTEIN );
							bAddPasteIn = true;
							bDisablePasteIn = true;
						}
						else
						{
							// If 'Paste before' is already exist, we don't show 'Paste in' menu item.
							bAddPasteIn = false;
						}
					}
				}
			
				
			}
			else
			{
				// Add 'Paste' menu item but disabled it.
				strPasteIn = TASApp.LoadLocalizedString( ID_TABPROJ_PASTEIN );
				bAddPasteIn = true;
				bDisablePasteIn = true;
			}

			if( true == bAddPasteIn )
			{
				CString strItemText = strPasteIn.Right( strPasteIn.GetLength() - strPasteIn.Find( '\n' ) );
				pContextMenu->ModifyMenu( ID_TABPROJ_PASTEIN, MF_BYCOMMAND, ID_TABPROJ_PASTEIN, strItemText );
				m_mapUpdateMenuTooltip[ID_TABPROJ_PASTEIN] = strPasteIn.Left( strPasteIn.Find( '\n' ) );

				if( false == bDisablePasteIn && false == bIsTerminalUnit ) 
				{
					pMainFrame->EnableMenuResID( ID_TABPROJ_PASTEIN );
				}
				else
				{
					pMainFrame->DisableMenuResID( ID_TABPROJ_PASTEIN );
				}
			}
			else
			{
				pContextMenu->RemoveMenu( ID_TABPROJ_PASTEIN, MF_BYCOMMAND );
			}

			if( true == bAddPasteBefore )
			{
				CString strItemText = strPasteBefore.Right( strPasteBefore.GetLength() - strPasteBefore.Find( '\n' ) );
				pContextMenu->ModifyMenu( ID_TABPROJ_PASTEBEFORE, MF_BYCOMMAND, ID_TABPROJ_PASTEBEFORE, strItemText );
				m_mapUpdateMenuTooltip[ID_TABPROJ_PASTEBEFORE] = strPasteBefore.Left( strPasteBefore.Find( '\n' ) );
				pMainFrame->EnableMenuResID( ID_TABPROJ_PASTEBEFORE );
			}
			else
			{
				pContextMenu->RemoveMenu( ID_TABPROJ_PASTEBEFORE, MF_BYCOMMAND );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Lock', 'Unlock' menu items.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pMainFrame->DisableMenuResID( ID_TABPROJ_LOCK );
			pMainFrame->DisableMenuResID( ID_TABPROJ_UNLOCK );

			if( false == bRootItemInSelection )
			{
				// User can lock/unlock only if the project is not frozen.
				if( false == bIsProjectFrozen )
				{
					// Check if all selected objects are already all locked or unlocked.
					bool bAtLeastOneLocked = false;
					bool bAtLeastOneUnLocked = false;

					for( int iLoopSelectedLine = 0; iLoopSelectedLine < iNbrProductSelected; iLoopSelectedLine++ )
					{
						CDS_HydroMod *pHMSelected = dynamic_cast<CDS_HydroMod *>( (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) );

						if( NULL == pHMSelected )
						{
							continue;
						}

						eBool3 eReturn = pHMSelected->IsLockedTriState( CDS_HydroMod::eHMObj::eALL );

						if( eBool3::eb3True == eReturn )
						{
							bAtLeastOneLocked = true;
						}
						else if( eBool3::eb3False == eReturn )
						{
							bAtLeastOneUnLocked = true;
						}
					}

					if( false == bAtLeastOneLocked && true == bAtLeastOneUnLocked )
					{
						// It means that all objects are unlocked. We can thus lock all.
						pMainFrame->EnableMenuResID( ID_TABPROJ_LOCK );
					}
					else if( true == bAtLeastOneLocked && false == bAtLeastOneUnLocked )
					{
						// It means that all objects are locked. We can thus lock all.
						pMainFrame->EnableMenuResID( ID_TABPROJ_UNLOCK );
					}
					else if( false == bAtLeastOneLocked && false == bAtLeastOneUnLocked )
					{
						// All objects are not present or are in a undefined state. We can do nothing.
						pMainFrame->DisableMenuResID( ID_TABPROJ_LOCK );
						pMainFrame->DisableMenuResID( ID_TABPROJ_UNLOCK );
					}
					else
					{
						// We have some objects locked and unlocked.
						pMainFrame->EnableMenuResID( ID_TABPROJ_LOCK );
						pMainFrame->EnableMenuResID( ID_TABPROJ_UNLOCK );
					}
				}
			}
			else
			{
				// No menu if we are in the root item.
				pContextMenu->RemoveMenu( ID_TABPROJ_LOCK, MF_BYCOMMAND );
				pContextMenu->RemoveMenu( ID_TABPROJ_UNLOCK, MF_BYCOMMAND );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Lock recursive' and 'Unlock recursive' menu items
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pMainFrame->EnableMenuResID( ID_TABPROJ_LOCKMODULE );
			pMainFrame->EnableMenuResID( ID_TABPROJ_UNLOCKMODULE );

			if( true == bIsProjectFrozen || 0 == iHMCount )
			{
				// Lock/unlock recursive are disabled if project is frozen or if there is no module.
				pMainFrame->DisableMenuResID( ID_TABPROJ_LOCKMODULE );
				pMainFrame->DisableMenuResID( ID_TABPROJ_UNLOCKMODULE );
			}
			else if( false == bIsProjectFrozen && iHMCount > 0 )
			{
				bool bStop = false;
				eBool3 eLockStatus = eBool3::eb3Undef;
				std::vector<HTREEITEM> vecSelectedItem;
				int iTempNbrProductSelected = iNbrProductSelected;

				// Check if we right-click on the network.
				if( 1 == (int)m_vecSelectedItem.size() && NULL != dynamic_cast<CTableHM *>( (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem.at( 0 ) ) ) )
				{
					// Fill all selected lines with all roots.
					HTREEITEM hChildItem = m_Tree.GetChildItem( m_vecSelectedItem.at( 0 ) );
					iTempNbrProductSelected = 0;

					while( NULL != hChildItem )
					{
						iTempNbrProductSelected++;
						vecSelectedItem.push_back( hChildItem );
						hChildItem = m_Tree.GetNextSiblingItem( hChildItem );
					}
				}
				else
				{
					vecSelectedItem = m_vecSelectedItem;
				}

				for( int iLoopSelectedLine = 0; iLoopSelectedLine < iTempNbrProductSelected && false == bStop; iLoopSelectedLine++ )
				{
					CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CTable *)m_Tree.GetObjectAt( vecSelectedItem[iLoopSelectedLine] ) );

					if(	NULL == pHM )
					{
						continue;
					}

					bool eReturn = ( true == pHM->IsAtLeastOneObjectUnlockedRecursive( false, true ) && true == pHM->IsaModule() );

					// 'eb3Undef': the current module and its children are all defined in a CBI mode. We continue to check other modules.
					// 'eb3False': the current module and its children contain at least one that is not locked. We can stop here.
					// 'eb3True': the current module and its children are all locked.
					if( true == eReturn )
					{
						eLockStatus = eBool3::eb3False;
						bStop = true;
					}
					else
					{
						eLockStatus = eBool3::eb3True;
					}
				}

				bStop = false;
				eBool3 eUnLockStatus = eBool3::eb3Undef;

				for( int iLoopSelectedLine = 0; iLoopSelectedLine < iTempNbrProductSelected && false == bStop; iLoopSelectedLine++ )
				{
					CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CTable *)m_Tree.GetObjectAt( vecSelectedItem[iLoopSelectedLine] ) );

					if(	NULL == pHM )
					{
						continue;
					}

					bool bReturn = ( true == pHM->IsAtLeastOneObjectLockedRecursive( false, true ) && true == pHM->IsaModule() );
					
					if( true == bReturn )
					{
						eUnLockStatus = eBool3::eb3False;
						bStop = true;
					}
					else
					{
						eUnLockStatus = eBool3::eb3True;
					}
				}
				
				switch( eLockStatus )
				{
					case eBool3::eb3Undef:
						// All modules are defined in a CBI mode. We can't 'Lock' or 'Unlock'.
						pMainFrame->DisableMenuResID( ID_TABPROJ_LOCKMODULE );
						break;

					case eBool3::eb3False:
						// There is at least one module that is not locked. We can 'Lock' or 'Unlock'.
						break;

					case eBool3::eb3True:
						// All modules and circuits are already locked, disable the 'Lock recursive' menu item.
						pMainFrame->DisableMenuResID( ID_TABPROJ_LOCKMODULE );
						break;
				}
				
				switch( eUnLockStatus )
				{
					case eBool3::eb3Undef:
						pMainFrame->DisableMenuResID( ID_TABPROJ_UNLOCKMODULE );
						break;

					case eBool3::eb3False:
						break;

					case eBool3::eb3True:
						pMainFrame->DisableMenuResID( ID_TABPROJ_UNLOCKMODULE );
						break;
				}
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Print' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( 1 == iNbrProductSelected && false == bRootItemInSelection )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_PRINT );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_PRINT );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Print recursive' is enabled if there is at least one child module in the selected lines.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( 1 == iNbrProductSelected && false == bRootItemInSelection && true == bChildModuleExist )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_PRINTRECURSIVE );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_PRINTRECURSIVE );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Export' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( 1 == iNbrProductSelected )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_EXPORT );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_EXPORT );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Export' menu item for quick measurement unused here.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_QM, MF_BYCOMMAND );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Export recursive' menu item.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( 1 == iNbrProductSelected && true == bChildModuleExist )
			{
				pMainFrame->EnableMenuResID( ID_TABPROJ_EXPORTRECURSIVE );
			}
			else
			{
				pMainFrame->DisableMenuResID( ID_TABPROJ_EXPORTRECURSIVE );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 'Export logged data chart' and 'Export logged data list' menu items unused here.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_CHART, MF_BYCOMMAND );
			pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_LDLIST, MF_BYCOMMAND );
		}
	}
	else if( CMainFrame::RightViewList::eRVChart == m_Tree.GetRightViewAt( ht.hItem ) )
	{
		// Scan all menu item by position and remove all unnecessary menu items.
		int i = 0;
		while( i < (int)pContextMenu->GetMenuItemCount() )
		{
			// Don't remove ID_TABPROJ_DELETE item and the separator just below.
			if( ID_TABPROJ_DELETE_LD == pContextMenu->GetMenuItemID( i ) )
			{
				i += 2;
				continue;
			}

			if( ID_TABPROJ_RENAME != pContextMenu->GetMenuItemID( i ) && 
				ID_TABPROJ_EXPORT_CHART != pContextMenu->GetMenuItemID( i ) )
			{
				pContextMenu->RemoveMenu( i, MF_BYPOSITION );
			}
			else
				i++;
		}
	}
	else if( CMainFrame::RightViewList::eRVLogData == m_Tree.GetRightViewAt( ht.hItem ) )
	{
		// scan all menu item by position and remove all unnecessary items.
		int i = 0;

		while( i < (int)pContextMenu->GetMenuItemCount() )
		{
			if( ID_TABPROJ_EXPORT_LDLIST != pContextMenu->GetMenuItemID( i ) )
			{
				pContextMenu->RemoveMenu( i, MF_BYPOSITION );
			}
			else
			{
				i++;
			}
		}
	}
	else if( CMainFrame::RightViewList::eRVQuickMeas == m_Tree.GetRightViewAt( ht.hItem ) )
	{
		pMainFrame->DisableMenuResID( ID_TABPROJ_EDIT );

		// Remove unused menu.
		pContextMenu->RemoveMenu( ID_TABPROJ_CREATEIN, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_ADDVALVE, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_FINDINDEXCIRCUIT, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_EDIT, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_RENAME, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_DELETE, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_DELETE_LD, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_CUT, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_COPY, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_PASTEIN, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_PASTEBEFORE, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_LOCK, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_UNLOCK, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_LOCKMODULE, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_UNLOCKMODULE, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_PRINT, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_PRINTRECURSIVE, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_EXPORTRECURSIVE, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_CHART, MF_BYCOMMAND );
		pContextMenu->RemoveMenu( ID_TABPROJ_EXPORT_LDLIST, MF_BYCOMMAND );

		// Enable/Disable the export menu.
		CTable *pTab = TASApp.GetpTADS()->GetpQuickMeasureTable();

		if( pTab->GetItemCount() > 0 )
		{
			pMainFrame->EnableMenuResID( ID_TABPROJ_EXPORT_QM );
		}
		else
		{
			pMainFrame->DisableMenuResID( ID_TABPROJ_EXPORT_QM );
		}
	}
	else
	{
		// In other cases, do not display the Menu bar
		return;
	}
	
	CPoint pt( ht.pt.x, ht.pt.y );
	ClientToScreen( &pt );
	
	// Show the popup menu.
	TASApp.GetContextMenuManager()->ShowPopupMenu( HMENU( *menu.GetSubMenu( 0 ) ), pt.x, pt.y, this, TRUE );
}

void CDlgLeftTabProject::OnSelchangedTree( NMHDR* pNMHDR, LRESULT* pResult ) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	// Determine where does click come from.
	*pResult = 0;

	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = GET_X_LPARAM( dwpos );
	ht.pt.y = GET_Y_LPARAM( dwpos );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );
	if( NULL == ht.hItem )
	{
		if( pNMTreeView->itemNew.hItem != m_hItem )
			m_hItem = pNMTreeView->itemNew.hItem;
	}
	else
		m_hItem = ht.hItem;

	m_vecSelectedItem.clear();
	m_vecSelectedItem.push_back( m_hItem );
	RefreshRightView();
}

void CDlgLeftTabProject::OnFltMenuCreate() 
{
	_AddModule( -1 );
}

void CDlgLeftTabProject::OnFltMenuAddValve() 
{
	_AddValve( -1 );
}

void CDlgLeftTabProject::OnFltMenuFindIndexCircuit()
{
	if( 1 != ( int )m_vecSelectedItem.size() )
	{
		return;
	}

	CDS_HydroMod *pHMSelected = dynamic_cast<CDS_HydroMod *>( ( CData * )( m_Tree.GetObjectAt( m_vecSelectedItem[0] ) ) );

	if( NULL == pHMSelected )
	{
		return;
	}

	if (false == pHMSelected->IsaModule())
	{
		pHMSelected = pHMSelected->GetParent();
	}

	CDS_HydroMod *pHMIndex = pHMSelected->FindIndexCircuit();

	if( NULL == pHMIndex )
	{
		return;
	}

	HTREEITEM hItem = _FindhItem( pHMIndex, m_Tree.GetRootItem() );

	if( NULL == hItem )
	{
		return;
	}

	m_Tree.SelectItem(NULL);
	m_Tree.EnsureVisible(hItem);
	m_hItem = hItem;
	m_vecSelectedItem.clear();
	m_vecSelectedItem.push_back(m_hItem);
	RefreshRightView();
}

void CDlgLeftTabProject::OnFltMenuEdit() 
{
	if( CMainFrame::RightViewList::eRVChart == m_Tree.GetRightViewAt( m_hItem ) )
	{
		CDlgEditString dlg;
		CString strTitle = TASApp.LoadLocalizedString( IDS_RENAME );
		CString strStatic = TASApp.LoadLocalizedString( IDS_NEWLOGDATANAME );
		CLog *pLD = (CLog *)( m_Tree.GetObjectAt( m_hItem ) );
		CString strName = pLD->GetName();
		dlg.Display( strTitle, strStatic, &strName );
		if( 0 != _tcscmp( (LPCTSTR)strName, pLD->GetName() ) )
		{
			pLD->SetName( (LPCTSTR)strName );
			m_Tree.SetItemText( m_hItem, (LPCTSTR)strName );
		}
	}
	else
	{
		// Normally in 'OnRclickTree' we allow 'Edit' menu item if there is ONLY one line selected.
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)( m_Tree.GetObjectAt( m_vecSelectedItem[0] ) ) );
		if( NULL != pHM )
		{
			// Clean cutting object if exist.
			if( NULL != pRViewHMCalc )
			{
				pRViewHMCalc->ResetFlagCut();
			}

			m_pTADS->CleanClipboard();

			CString HMName = pHM->GetHMName();

			//Add icon overlay to show which element is currently edited
			m_Tree.SetItemState(m_vecSelectedItem[0], INDEXTOOVERLAYMASK((int)OMI_EditCircuit), TVIS_OVERLAYMASK);
			
			if( true == pMainFrame->IsHMCalcChecked() )
			{
				// In HM calc mode we can directly edit with 'RViewHMCalc'.
				if( NULL != pRViewHMCalc )
				{
					pRViewHMCalc->EditHM( pHM );
				}

				ResetTreeWithString( HMName );
			}
			else
			{
				EditHydroMod( pHM, ( true == pHM->IsaModule() ) ? CDlgCreateModule::DialogMode::EditModule : CDlgCreateModule::DialogMode::EditValve );
				pHM->SetHMCalcMode( false );
				ResetTreeWithString( HMName );
			}
		}
	}
}

void CDlgLeftTabProject::OnFltMenuRename()
{
	CDlgEditString dlg;
	CString strTitle = TASApp.LoadLocalizedString( IDS_RENAME );
	if( CMainFrame::RightViewList::eRVGeneral == m_Tree.GetRightViewAt( m_hItem ) )
	{
		bool fStop = false;
		while( false == fStop )
		{
			CString strStatic = TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_NEWPROJECTNAME );
			
			CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
			ASSERT( NULL != pPrjRef );
			
			if( NULL != pPrjRef )
			{
				CString strName = pPrjRef->GetString( CDS_ProjectRef::Name );
				if( IDOK == dlg.Display( strTitle, strStatic, &strName ) )
				{
					if( _tcslen( (LPCTSTR)strName ) > 0 )
					{
						pPrjRef->SetString( CDS_ProjectRef::Name, (LPCTSTR)strName );
						::SendMessage( pMainFrame->GetSafeHwnd(), WM_USER_PROJECTREFCHANGE, 0, 0 );
						::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PROJECTREFCHANGE );
						fStop = true;
					}
					else
					{
						CString str;
						::FormatString( str, IDS_TABDIALOGPROJ_PROJECTNAMECANTBEEMPTY, (LPCTSTR)strName );
						AfxMessageBox( str );
					}
				}
				else
					fStop = true;
			}
		}
	}
	else if( CMainFrame::RightViewList::eRVHMCalc == m_Tree.GetRightViewAt( m_hItem ) )
	{
		bool fStop = false;
		while( false == fStop )
		{
			CString strStatic = TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_NEWMODULENAME );
			
			// Normally in 'OnRclickTree' we allow 'Rename' menu item if there is ONLY one line selected.
			CDS_HydroMod *pHM = (CDS_HydroMod *)( m_Tree.GetObjectAt( m_vecSelectedItem[0] ) );
			CString strName = pHM->GetHMName();
			
			if( IDOK == dlg.Display( strTitle, strStatic, &strName ) )
			{
				if( true == strName.IsEmpty() )
				{
					strName = _T("*");
				}

				if( 0 != _tcscmp( (LPCTSTR)strName, pHM->GetHMName() ) )
				{
					bool fNameUncompatibleWithCBI = false;
					
					CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
					ASSERT( NULL != pPrjParam );

					if( true == pPrjParam->IsCompatibleCBI() )
					{
						// If input is not compatible to the CBI mode...
						CCbiRestrString CbiRestrString;
						
						if( false == CbiRestrString.CheckCBICharSet( &strName ) )
						{
							TASApp.AfxLocalizeMessageBox( AFXMSG_CBICHARRESTRICTION );
							fNameUncompatibleWithCBI = true;
						}
					}
					
					if( false == fNameUncompatibleWithCBI )
					{
						// Verify if name is not already exist.
						CTable *pPipTab = m_pTADS->GetpHydroModTable();
						ASSERT( NULL != pPipTab );

						IDPTR IDPtr = _NULL_IDPTR;
						
						if( NULL == m_pTADS->FindHydroMod( strName, pPipTab ) )
						{
							// Clean cutting object if exist.
							if( NULL != pRViewHMCalc )
							{
								pRViewHMCalc->ResetFlagCut();
							}

							m_pTADS->CleanClipboard();

							pHM->SetHMName( (LPCTSTR)strName );
							m_Tree.SetItemText( m_hItem, (LPCTSTR)strName );
							ResetTreeAndSelectHM( pHM );
							fStop = true;
						}
						else
						{
							CString str;
							::FormatString( str, IDS_HYDROMODREF_ALREADYUSED, (LPCTSTR)strName );
							AfxMessageBox( str );
						}
					}
				}
			}
			else
				fStop = true;
		}
	}
	else if( CMainFrame::RightViewList::eRVChart == m_Tree.GetRightViewAt( m_hItem ) )
	{
		CString strStatic = TASApp.LoadLocalizedString( IDS_NEWLOGDATANAME );
		CLog *pLD = (CLog *)( m_Tree.GetObjectAt( m_hItem ) );
		CString strName = pLD->GetName();
		if( IDOK == dlg.Display( strTitle, strStatic, &strName ) )
		{
			if( 0 != _tcscmp( (LPCTSTR)strName, pLD->GetName() ) )
			{
				pLD->SetName( (LPCTSTR)strName );
				m_Tree.SetItemText( m_hItem, (LPCTSTR)strName );

				if( NULL != pRViewChart )
				{
					pRViewChart->SetRedraw( pLD );
				}
			}
		}
	}
}

void CDlgLeftTabProject::OnFltMenuDelete() 
{
	BeginWaitCursor();
	CString str;
	if( CMainFrame::RightViewList::eRVChart == m_Tree.GetRightViewAt( m_hItem ) )
	{
		FormatString( str, IDS_DELETELOGDATA, m_Tree.GetItemText( m_hItem ) );
		if( IDYES == ::AfxMessageBox( (LPCTSTR)str, MB_YESNO | MB_DEFBUTTON2 | MB_ICONSTOP ) )
		{
			_DeleteChart( m_hItem );
			str = TASApp.LoadLocalizedString( IDS_LOGGEDDATAS );
			ResetTreeWithString( str );
		}
	}
	else
	{
		if( 1 == m_vecSelectedItem.size() )
			FormatString( str, IDS_DELETEMODULE, m_Tree.GetItemText( m_vecSelectedItem[0] ) );
		else
		{
			CString str1;
			str1.Format( _T("%d"), (int)m_vecSelectedItem.size() );
			FormatString( str, IDS_HMCALC_DELETEALLCIRCUITS, str1 );
		}

		if( IDYES == ::AfxMessageBox( (LPCTSTR)str, MB_YESNO | MB_DEFBUTTON2 | MB_ICONSTOP ) )
		{
			// We need to do this here. Because once hydromod will be deleted in the code below, pointer in the tree will be no more valid.
			if( NULL != pRViewHMCalc )
			{
				pRViewHMCalc->ResetFlagCut();
			}

			m_pTADS->CleanClipboard();

			// Keep a copy of the module that have the highest position for a clean reset of the tree.
			CDS_HydroMod *pclHydroModToReset = NULL;
			CString strHydroModToReset = _T("");
			bool fAtLeastOneDeleted = false;
			
			for( int iLoopSelectedLine = 0; iLoopSelectedLine < (int)m_vecSelectedItem.size(); iLoopSelectedLine++ )
			{
				CDS_HydroMod *pHMSelected = dynamic_cast<CDS_HydroMod *>( (CData *)( m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) ) );
				
				if( NULL == pHMSelected )
				{
					continue;
				}

				CDS_HydroMod *pHMInPipingTab = m_pTADS->FindHydroMod( pHMSelected->GetHMName(), m_pTADS->GetpHydroModTable(), NULL, CDatastruct::FindMode::Both );

				if( NULL == pHMInPipingTab )
				{
					continue;
				}

				if( NULL == pclHydroModToReset || pHMInPipingTab->GetLevel() < pclHydroModToReset->GetLevel() )
				{
					pclHydroModToReset = pHMInPipingTab;
					CTable *pTab = (CTable *)( pHMInPipingTab->GetIDPtr().PP );
					if( true == pTab->IsClass( CLASS( CTableHM ) ) )
						strHydroModToReset = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
					else
						strHydroModToReset = ( (CDS_HydroMod *)pTab )->GetHMName();
				}

				m_pTADS->DeleteHM( pHMInPipingTab );
				fAtLeastOneDeleted = true;
			}
	
			if( true == fAtLeastOneDeleted )
			{
				// Compute all installation only if TASelect is in HM calculation mode and the project is not frozen.
				CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
				ASSERT( NULL != pPrjParam );

				if( true == pMainFrame->IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
				{
					m_pTADS->ComputeAllInstallation();
				}

				ResetTreeWithString( strHydroModToReset );
			}
		}
	}
	EndWaitCursor();
}

void CDlgLeftTabProject::OnFltMenuCut()
{
	if( 0 == (int)m_vecSelectedItem.size() )
	{
		return;
	}
	
	BeginWaitCursor();

	m_pTADS->CleanClipboard();
	
	CTable *pTab = m_pTADS->GetpClipboardTable();
	ASSERT( NULL != pTab );

	std::vector<CDS_HydroMod *> vecHydromod;

	for( int iLoopSelectedLine = 0; iLoopSelectedLine < (int)m_vecSelectedItem.size(); iLoopSelectedLine++ )
	{
		CDS_HydroMod *pHMSelected = dynamic_cast<CDS_HydroMod *>( (CData *)( m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) ) );
		
		if( NULL == pHMSelected )
		{
			continue;
		}

		m_pTADS->CopyHMToClipBoard( pHMSelected, pTab );
		vecHydromod.push_back( pHMSelected );
	}

	if( vecHydromod.size() > 0 )
	{
		if( NULL != pRViewHMCalc )
		{
			pRViewHMCalc->SetLastCopyCutOperation( CRViewHMCalc::LastOperation::Cut );
			pRViewHMCalc->SetFlagCut( &vecHydromod );
		}

		RefreshRightView();
	}

	EndWaitCursor();
}

void CDlgLeftTabProject::OnFltMenuCopy() 
{
	if( 0 == (int)m_vecSelectedItem.size() )
		return;
	
	BeginWaitCursor();
	
	// Clean cutting object if exist.
	if( NULL != pRViewHMCalc )
	{
		pRViewHMCalc->ResetFlagCut();
	}

	m_pTADS->CleanClipboard();

	// For the moment, user can select only one line at a time.
	CTable *pTab = m_pTADS->GetpClipboardTable();
	ASSERT( NULL != pTab );

	CDS_HydroMod *pHMSelected = dynamic_cast<CDS_HydroMod *>( (CData *)( m_Tree.GetObjectAt( m_vecSelectedItem[0] ) ) );
	
	if( NULL != pHMSelected )
	{	
		m_pTADS->CopyHMToClipBoard( pHMSelected, pTab );
		
		if( NULL != pRViewHMCalc )
		{
			pRViewHMCalc->SetLastCopyCutOperation( CRViewHMCalc::LastOperation::Copy );
		}
	}

	EndWaitCursor();
}

void CDlgLeftTabProject::OnFltMenuPasteIn() 
{
	_TabProjPaste( PasteMode::PasteIn );
}

void CDlgLeftTabProject::OnFltMenuPasteBefore()
{
	_TabProjPaste( PasteMode::PasteBefore );
}

void CDlgLeftTabProject::OnFltMenuLock()
{
	BeginWaitCursor();

	bool fAtLeastOneLockDone = false;
	for( int iLoopSelectedLine = 0; iLoopSelectedLine < (int)m_vecSelectedItem.size(); iLoopSelectedLine++ )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) );
		if( NULL == pHM )
			continue;

		pHM->SetLock( CDS_HydroMod::eHMObj::eALL, true );
		fAtLeastOneLockDone = true;
	}

	if( true == fAtLeastOneLockDone )
	{
		// Clean cutting object if exist and clean clipboard.
		if( NULL != pRViewHMCalc )
		{
			pRViewHMCalc->ResetFlagCut();
		}

		m_pTADS->CleanClipboard();
		RefreshRightView();
	}

	EndWaitCursor();
}

void CDlgLeftTabProject::OnFltMenuUnLock()
{
	BeginWaitCursor();

	bool fAtLeastOneUnLockDone = false;
	for( int iLoopSelectedLine = 0; iLoopSelectedLine < (int)m_vecSelectedItem.size(); iLoopSelectedLine++ )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) );
		if( NULL == pHM )
			continue;

		pHM->SetLock( CDS_HydroMod::eHMObj::eALL, false );
		fAtLeastOneUnLockDone = true;
	}

	// 'Lock' and 'Unlock' menu items are activated ONLY if we are in HM calculation mode and the project is not frozen.
	// We can thus launch the compute without verify.
	if( true == fAtLeastOneUnLockDone )
	{
		// Clean cutting object if exist and clean clipboard.
		if( NULL != pRViewHMCalc )
		{
			pRViewHMCalc->ResetFlagCut();
		}

		m_pTADS->CleanClipboard();
		m_pTADS->ComputeAllInstallation();
		RefreshRightView();
	}

	EndWaitCursor();
}

void CDlgLeftTabProject::OnFltMenuLockRecursive()
{
	BeginWaitCursor();

	// Verify first if we have the root item in the selected line.
	bool fRootItemInSelection = false;
	for( int iLoopSelectedLine = 0; iLoopSelectedLine < (int)m_vecSelectedItem.size() && false == fRootItemInSelection; iLoopSelectedLine++ )
	{
		if( m_vecSelectedItem[iLoopSelectedLine] == m_hHNBranch )
			fRootItemInSelection = true;
	}

	bool fAtLeastOneLockRecursiveDone = false;
	if( false == fRootItemInSelection )
	{
		// Run all selected lines.
		for( int iLoopSelectedLine = 0; iLoopSelectedLine < (int)m_vecSelectedItem.size(); iLoopSelectedLine++ )
		{
			CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) );
			if( NULL == pHM )
				continue;

			pHM->SetRecursiveLock( CDS_HydroMod::eHMObj::eALL, true );
			fAtLeastOneLockRecursiveDone = true;
		}
	}
	else
	{
		// Run all level 0.
		for( HTREEITEM hChildItem = m_Tree.GetChildItem( m_hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
		{
			if( NULL != hChildItem )
			{
				// Get pointer on the current module.
				CDS_HydroMod *pHM = (CDS_HydroMod *)( m_Tree.GetObjectAt( hChildItem ) );
				if( NULL != pHM )
				{
					pHM->SetRecursiveLock( CDS_HydroMod::eHMObj::eALL, true );
					fAtLeastOneLockRecursiveDone = true;
				}
			}
		}
	}

	if( true == fAtLeastOneLockRecursiveDone )
	{
		// Clean cutting object if exist and clean clipboard.
		if( NULL != pRViewHMCalc )
		{
			pRViewHMCalc->ResetFlagCut();
		}

		m_pTADS->CleanClipboard();
		RefreshRightView();
	}

	EndWaitCursor();
}

void CDlgLeftTabProject::OnFltMenuUnLockRecursive()
{
	BeginWaitCursor();

	// Verify first if we have the root item in the selected line.
	bool fRootItemInSelection = false;
	for( int iLoopSelectedLine = 0; iLoopSelectedLine < (int)m_vecSelectedItem.size() && false == fRootItemInSelection; iLoopSelectedLine++ )
	{
		if( m_vecSelectedItem[iLoopSelectedLine] == m_hHNBranch )
			fRootItemInSelection = true;
	}

	bool fAtLeastOneUnLockRecursiveDone = false;
	if( false == fRootItemInSelection )
	{
		// Run all selected lines.
		for( int iLoopSelectedLine = 0; iLoopSelectedLine < (int)m_vecSelectedItem.size(); iLoopSelectedLine++ )
		{
			CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) );
			if( NULL == pHM )
				continue;

			pHM->SetRecursiveLock( CDS_HydroMod::eHMObj::eALL, false );
			fAtLeastOneUnLockRecursiveDone = true;
		}
	}
	else
	{
		// Run all level 0.
		for( HTREEITEM hChildItem = m_Tree.GetChildItem( m_hItem ); NULL != hChildItem; hChildItem = m_Tree.GetNextSiblingItem( hChildItem ) )
		{
			if( NULL != hChildItem )
			{
				// Get pointer on the current module.
				CDS_HydroMod *pHM = (CDS_HydroMod *)( m_Tree.GetObjectAt( hChildItem ) );
				if( NULL != pHM )
				{
					pHM->SetRecursiveLock( CDS_HydroMod::eHMObj::eALL, false );
					fAtLeastOneUnLockRecursiveDone = true;
				}
			}
		}
	}

	if( true == fAtLeastOneUnLockRecursiveDone )
	{
		// Clean cutting object if exist and clean clipboard.
		if( NULL != pRViewHMCalc )
		{
			pRViewHMCalc->ResetFlagCut();
		}

		m_pTADS->CleanClipboard();
		m_pTADS->ComputeAllInstallation();
		RefreshRightView();
	}
	
	EndWaitCursor();
}

void CDlgLeftTabProject::OnFltMenuPrint() 
{
	if( NULL == pRViewProj || 0 == (int)m_vecSelectedItem.size() )
	{
		return;
	}

	pRViewProj->ClearPrintingFlags();
	if( CMainFrame::RightViewList::eRVHMSumm == m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) || CMainFrame::RightViewList::eRVHMCalc == m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) )
	{
		// Clean cutting object if exist.
		if( NULL != pRViewHMCalc )
		{
			pRViewHMCalc->ResetFlagCut();
		}

		m_pTADS->CleanClipboard();

		CTable *pTable = (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem[0] );
		pRViewProj->PrintModule( pTable, false );
	}
	else
	{
		pRViewProj->Print();
	}
}

void CDlgLeftTabProject::OnFltMenuPrintRecursive() 
{
	if( NULL == pRViewProj || 0 == (int)m_vecSelectedItem.size() )
		return;

	if( CMainFrame::RightViewList::eRVHMSumm != m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) && CMainFrame::RightViewList::eRVHMCalc != m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) )
		return;
	
	// Clean cutting object if exist.
	if( NULL != pRViewHMCalc )
	{
		pRViewHMCalc->ResetFlagCut();
	}

	m_pTADS->CleanClipboard();

	CTable *pTable = (CTable *)m_Tree.GetObjectAt( m_vecSelectedItem[0] );
	pRViewProj->PrintModule( pTable, true );
}

void CDlgLeftTabProject::OnFltMenuExport() 
{
	if( 0 == (int)m_vecSelectedItem.size() )
		return;

	CString PrjDir = GetProjectDirectory();

	// Variables.
	bool fExportQuickMeas = false;
	bool fExportHMSummary = false;

	// Determine where does the click comes from.
	if( CMainFrame::eRVQuickMeas == m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) )
		fExportQuickMeas = true;
	if( CMainFrame::eRVHMSumm == m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) )
		fExportHMSummary = true;

	// Load file filter, compose file name and initialize CFileDialog.
	CString str = TASApp.LoadLocalizedString( IDS_EXPORTXLSXFILTER );
	CString strExt = _T("xlsx");
	CString strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

	CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, (LPCTSTR)str, NULL );
	dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;
	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();
		if( true == fExportQuickMeas )
		{
			// Create a sheet quick measurement with multiple tabs.
			CSSheetQuickMeas SheetQuickMeas;

			// Remark: for the parent, we intentionally take the main window. If we set 'CDlgLeftTabProject' as the parent, there are some refresh called
			//         (by who?) that cause flicking when exporting big project.
			SheetQuickMeas.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), TASApp.GetMainWnd(), IDC_TMPSSHEET );
			SheetQuickMeas.ModifyStyle( WS_VISIBLE, 0 );
			SheetQuickMeas.SetRedraw( true );
			//SheetQuickMeas.ExportExcelBook( dlg.GetPathName(), NULL );
			Excel_Workbook wbQuickMeas;
			wbQuickMeas.AddSheet( &SheetQuickMeas );
			wbQuickMeas.Write( dlg.GetPathName() );
		}
		else if( true == fExportHMSummary )
		{
			// Create a sheet HM summary with multiple tabs.
			CSSheetHMSumm SheetHMSummary;
			
			// Remark: for the parent, we intentionally take the main window. If we set 'CDlgLeftTabProject' as the parent, there are some refresh called
			//         (by who?) that cause flicking when exporting big project.
			SheetHMSummary.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), TASApp.GetMainWnd(), IDC_TMPSSHEET );
			SheetHMSummary.ModifyStyle( WS_VISIBLE, 0 );
			SheetHMSummary.SetRedraw( NULL, true );
			//SheetHMSummary.ExportExcelBook( dlg.GetPathName(), NULL );
			Excel_Workbook wbHMSummary;
			wbHMSummary.AddSheet( &SheetHMSummary );
			wbHMSummary.Write( dlg.GetPathName() );
		}
		else
		{
			// Clean cutting object if exist.
			if( NULL != pRViewHMCalc )
			{
				pRViewHMCalc->ResetFlagCut();
			}

			m_pTADS->CleanClipboard();

			// Create a sheetHMCalc with multiple tabs.
			CSheetHMCalc SheetHMCalc;

			// Remark: for the parent, we intentionally take the main window. If we set 'CDlgLeftTabProject' as the parent, there are some refresh called
			//         (by who?) that cause flicking when exporting big project.
			SheetHMCalc.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), TASApp.GetMainWnd(), IDC_TMPSSHEET );
			SheetHMCalc.ModifyStyle( WS_VISIBLE, 0 );

			CDS_HydroMod *pHM = (CDS_HydroMod *)m_Tree.GetObjectAt( m_vecSelectedItem[0] );

			SheetHMCalc.Init( pHM, false, 0, true );

			SheetHMCalc.PrepareforExport();
			SheetHMCalc.UnSelectMultipleRows();
			//SheetHMCalc.ExportExcelBook( dlg.GetPathName(), NULL );
			Excel_Workbook wbHMCalc;
			wbHMCalc.AddSheet( &SheetHMCalc );
			wbHMCalc.Write( dlg.GetPathName() );
		}
		EndWaitCursor();
	}
}

void CDlgLeftTabProject::OnFltMenuExportRecursive() 
{
	if( 0 == (int)m_vecSelectedItem.size() )
		return;

	CDS_HydroMod *pHM = (CDS_HydroMod *)( m_Tree.GetObjectAt( m_vecSelectedItem[0] ) );				ASSERT( pHM );
	if( NULL == pHM )
		return;

	// Verify that the number of tabs in the sheet will not go upper than 1000 tabs.
	// By default and to be sure, we take a maximum of 950.
	int iModuleCount = 0;
	bool fDisplaySummary = false;
	if( CMainFrame::RightViewList::eRVHMSumm != m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) )
		iModuleCount = pHM->GetCount( true, true );
	else
	{
		HTREEITEM hItem;
		hItem = m_Tree.GetChildItem( m_vecSelectedItem[0] );
		while( NULL != hItem )
		{
			pHM = (CDS_HydroMod *)( m_Tree.GetObjectAt( hItem ) );						ASSERT( pHM );
			if( NULL == pHM )
				return;
			iModuleCount += pHM->GetCount( true, true );
			// Go to all modules.
			hItem = m_Tree.GetNextSiblingItem( hItem );
		}
		// Summary sheet will be exported.
		fDisplaySummary = true;
	}
	
	// Remark: 'MAX_SHEETS' is defined in 'SSheet.h'.
	if( ( iModuleCount * CSheetHMCalc::SheetDescription::SLast ) + ( ( true == fDisplaySummary ) ? 1 : 0 ) > MAX_SHEETS )
	{
		CString str;
		str.Format( TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_EXPORTXLS_EXCEDED ), iModuleCount, MAX_SHEETS / CSheetHMCalc::SheetDescription::SLast );
		CWnd::MessageBox( str, TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_EXPORTXLS_EXCEDED_TITLE) , MB_ICONEXCLAMATION );
		return;
	}

	CString PrjDir = GetProjectDirectory();
	
	// Load file filter, compose file name and initialize 'CFileDialog'.
	CString str = TASApp.LoadLocalizedString( IDS_EXPORTXLSXFILTER );
	CString strExt = _T("xlsx");
	CString strName = m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name );

	CFileDialog dlg( false, strExt, strName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, (LPCTSTR)str, NULL );
	dlg.m_ofn.lpstrInitialDir = (LPCTSTR)PrjDir;
	if( IDOK == dlg.DoModal() )
	{
		// Avoid to go on Logged data tree nodes.
		if( CMainFrame::RightViewList::eRVHMSumm != m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) && CMainFrame::RightViewList::eRVHMCalc != m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) )
			return;

		// Clean cutting object if exist.
		if( NULL != pRViewHMCalc )
		{
			pRViewHMCalc->ResetFlagCut();
		}

		m_pTADS->CleanClipboard();
	
		// Create a control bar dialog.
		CDlgProgBarExportXls dlgExp( 0, TASApp.LoadLocalizedString( IDS_TABCDIALOGPROJ_EXPORTBOXTITLE ) );
		dlgExp.SetRange( 1, iModuleCount + ( ( true == fDisplaySummary ) ? 1 : 0 ) );

		// Create a sheetHMCalc with multiple tabs.
		CSheetHMCalc SheetHMCalc;

		// Remark: for the parent, we intentionally take the main window. If we set 'CDlgLeftTabProject' as the parent, there are some refresh called
		//         (by who?) that cause flicking when exporting big project.
		SheetHMCalc.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), TASApp.GetMainWnd(), IDC_TMPSSHEET );
		SheetHMCalc.ModifyStyle( WS_VISIBLE, 0 );

		if( true == fDisplaySummary )
		SheetHMCalc.ReserveFirstSheet();

		// Remark: 'MAX_SHEETS' is defined in 'SSheet.h'.
		if( CMainFrame::RightViewList::eRVHMSumm != m_Tree.GetRightViewAt( m_vecSelectedItem[0] ) )
		{
			dlgExp.SetParamForExport( &SheetHMCalc, pHM, MAX_SHEETS );
		}
		else
		{
			// If we are in the main module.
			CTable *pTab = TASApp.GetpTADS()->GetpHydroModTable();				
			ASSERT( NULL != pTab );

			dlgExp.SetParamForExport( &SheetHMCalc, pTab, MAX_SHEETS );
		}

		// Export first the HM summary directly to.
		if( true == fDisplaySummary )
		{
			// Create a sheet HM summary with multiple tabs.
			CSSheetHMSumm SheetHMSummary;
			
			// Remark: for the parent, we intentionally take the main window. If we set 'CDlgLeftTabProject' as the parent, there are some refresh called
			//         (by who?) that cause flicking when exporting big project.
			SheetHMSummary.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), TASApp.GetMainWnd(), IDC_TMPSSHEET );
			SheetHMSummary.ModifyStyle( WS_VISIBLE, 0 );
			SheetHMSummary.SetRedraw( &SheetHMCalc, true );
		}
		
		dlgExp.Display( false );
		if( true == dlgExp.CanExport() )
		{
			BeginWaitCursor();
			SheetHMCalc.PrepareforExport();
			SheetHMCalc.UnSelectMultipleRows();
			//SheetHMCalc.ExportExcelBook( dlg.GetPathName(), NULL );

			Excel_Workbook wbHMCalc;
			wbHMCalc.AddSheet( &SheetHMCalc );
			wbHMCalc.Write( dlg.GetPathName() );

			EndWaitCursor();	
		}
	}
}

void CDlgLeftTabProject::OnFltMenuExportChart() 
{
	if( NULL != pRViewChart )
	{
		pRViewChart->CallOnFileExportChart();
	}
}

void CDlgLeftTabProject::OnFltMenuExportLDList() 
{
	if( NULL != pRViewLogData )
	{
		pRViewLogData->CallOnFileExportLdlist();
	}
}

/*
void CDlgLeftTabProject::OnEditCut() 
{
	OnFltMenuCut();
}

void CDlgLeftTabProject::OnEditCopy() 
{
	OnFltMenuCopy();
}

void CDlgLeftTabProject::OnEditPaste() 
{
	OnFltMenuPaste();
}

void CDlgLeftTabProject::OnUpdateEditCut( CCmdUI* pCmdUI ) 
{
	if( NULL == m_hItem )
		return;
	
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_Tree.GetObjectAt( m_hItem );
	
	// Test if the user clicked on a Module node.
	if( NULL != pHM && CMainFrame::RightViewList::eRVHMCalc == m_Tree.GetViewTypeAt( m_hItem ) )
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->Enable( FALSE );
}

void CDlgLeftTabProject::OnUpdateEditCopy( CCmdUI* pCmdUI ) 
{
	if( NULL == m_hItem )
		return;
	
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_Tree.GetObjectAt( m_hItem );
	
	// Test if the user clicked on a Module node.
	if( NULL != pHM && CMainFrame::RightViewList::eRVHMCalc == m_Tree.GetViewTypeAt( m_hItem ) )
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->Enable( FALSE );
}

void CDlgLeftTabProject::OnUpdateEditPaste( CCmdUI* pCmdUI ) 
{
	if( NULL == m_hItem )
		return;
	
	CDS_HydroMod *pHM = (CDS_HydroMod *)m_Tree.GetObjectAt( m_hItem );
	if( NULL != pHM )
	{
		// Test if the user clicked on a Module node.
		if( CMainFrame::RightViewList::eRVHMCalc == m_Tree.GetViewTypeAt( m_hItem ) )
		{
			// Check if there is something into the clipboard.
			if( false == m_pTADS->IsClipBoardEmpty() )
				pCmdUI->Enable( TRUE );
			else
				pCmdUI->Enable( FALSE );
		}
		
		// Or on the network.
		if( CMainFrame::RightViewList::eRVHMSumm == m_Tree.GetViewTypeAt( m_hItem ) )
		{
			// Check if there is something into the clipboard.
			CDS_HydroMod *pCBHM = NULL;
			if( false == m_pTADS->IsClipBoardEmpty( &pCBHM ) )
			{
				if( true == pCBHM->IsaModule() )
					pCmdUI->Enable( TRUE );
				else
					pCmdUI->Enable( FALSE );
			}
			else
				pCmdUI->Enable( FALSE );
		}
	}
}
*/

void CDlgLeftTabProject::OnUpdateMenuText( CCmdUI *pCmdUI )
{
	// Update the status bar.
	if( m_mapUpdateMenuTooltip.count( pCmdUI->m_nID ) > 0 )
	{
		pMainFrame->UpdateMenuToolTip( pCmdUI, m_mapUpdateMenuTooltip[pCmdUI->m_nID] );
	}
	else
	{
		pMainFrame->UpdateMenuToolTip( pCmdUI );
	}
}

LRESULT CDlgLeftTabProject::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	m_pTADB = TASApp.GetpTADB();
	m_pTADS = TASApp.GetpTADS();

	m_pTADS->CleanClipboard();
	m_pTADS->Modified( 0 );

	// Initialize 'm_Tree'.	
	ResetTreeWithString( _T(""), true );

	// If TADS can not be in HMCalcMode un-check the HMCalc Button.
	if( true == m_pTADS->IsOneValveNotDefine() || eb3False == m_pTADS->IsHMCalcMode() )
	{
		pMainFrame->SetHMCalcMode( false );
	}
	return 0;
}

LRESULT CDlgLeftTabProject::OnProjectRefChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_hItem )
		ResetTreeWithString( m_Tree.GetItemText( m_hItem ) );
	else
		ResetTree();
	return 0;
}

LRESULT CDlgLeftTabProject::OnResetTreeNeeded( WPARAM wParam, LPARAM lParam )
{
	CDS_HydroMod *pHM = (CDS_HydroMod *)lParam;
	bool bResetCutFlag = ( 0 == wParam ) ? false : true;

	if( NULL != pHM )
	{
		ResetTreeAndSelectHM( pHM, bResetCutFlag );
	}
	else
	{
		ResetTree( bResetCutFlag );
	}

	return 0;
}

LRESULT CDlgLeftTabProject::OnResetTreeNeededCreate( WPARAM wParam, LPARAM lParam )
{
	CDS_HydroMod *pHM = ( CDS_HydroMod * )lParam;
	bool fResetCutFlag = ( 0 == wParam ) ? false : true;

	if( pHM != NULL )
	{
		ResetTreeAndSelectHM( pHM, fResetCutFlag );
		HTREEITEM hItem = _FindhItem(pHM, m_Tree.GetRootItem());
		m_Tree.SetItemState(hItem, INDEXTOOVERLAYMASK((int)OMI_NewCircuit), TVIS_OVERLAYMASK);
	}
	else
	{
		m_hItem = NULL;
		ResetTree( fResetCutFlag );
	}

	return 0;
}

LRESULT CDlgLeftTabProject::OnResetTreeNeededEdit(WPARAM wParam, LPARAM lParam)
{
	CDS_HydroMod *pHM = (CDS_HydroMod *)lParam;
	bool fResetCutFlag = (0 == wParam) ? false : true;

	if (pHM != NULL)
	{
		ResetTreeAndSelectHM(pHM, fResetCutFlag);
		HTREEITEM hItem = _FindhItem(pHM, m_Tree.GetRootItem());
		m_Tree.SetItemState(hItem, INDEXTOOVERLAYMASK((int)OMI_EditCircuit), TVIS_OVERLAYMASK);
	}
	else
	{
		ResetTree(fResetCutFlag);
	}

	return 0;
}

LRESULT CDlgLeftTabProject::OnTechParamChange( WPARAM wParam, LPARAM lParam )
{
	BeginWaitCursor();

	// Compute all installation only if TASelect is in HM calculation mode and the project is not frozen.
	CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pMainFrame->IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
	{
		if( true == m_pTADS->ComputeAllInstallation() )
		{
			RefreshRightView();
		}
	}

	EndWaitCursor();
	return 0;
}

LRESULT CDlgLeftTabProject::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	RefreshRightView();
	return 0;
}

LRESULT CDlgLeftTabProject::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProject != ( (WMUserWaterCharWParam)wParam & WM_UWC_WP_ForProject )
			|| WMUserWaterCharLParam::WM_UWC_LWP_NoChange == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	BeginWaitCursor();
	
	// Compute all installation only if TASelect is in HM calculation mode and the project is not frozen.
	CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( true == pMainFrame->IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
	{
		// HYS-1882: Update fluid characteristics when we change via the ribbon or water characteristics dialog.
		m_pTADS->VerifyProjectAllWaterCharacteristics( false );
		if( true == m_pTADS->ComputeAllInstallation() )
		{
			RefreshRightView();
		}
	}

	EndWaitCursor();
	return 0;
}

void CDlgLeftTabProject::_ResetTree( unsigned int uid )
{
	// Initialize tree without a specific selection.
	ResetTreeWithString( _T(""), false );

	CDS_HydroMod *pHM = NULL;
	
	// Find pointer on the HM matching uid.
	if( uid > 0 )
	{
		pHM = m_pTADS->FindHydroMod( uid, m_pTADS->GetpHydroModTable() );
	}

	// Find corresponding hTreeItem.
	HTREEITEM hItem = NULL;

	if( NULL != pHM )
	{
		hItem = _FindhItem( pHM, m_Tree.GetRootItem() );
	}
	
	// If node doesn't exist...
	if( NULL == hItem )
	{
		CString str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
		hItem = _FindhItem( str, m_Tree.GetRootItem() );
		if( NULL == hItem )
			hItem = m_Tree.GetRootItem();
	}

	m_Tree.SelectItem( hItem );
	m_Tree.Expand( hItem, TVE_EXPAND );
}

void CDlgLeftTabProject::_SetOverlayMask( HTREEITEM hItem )
{
	while( NULL != hItem )
	{
		if( CMainFrame::RightViewList::eRVHMCalc == m_Tree.GetRightViewAt( hItem ) )
		{
			CDS_HydroMod *pHM = (CDS_HydroMod *)m_Tree.GetObjectAt( hItem );
			OverlayMaskIndex eOverlayMaskIndex = OverlayMaskIndex::OMI_Undefined;
			
			// Scan all child valves of pHM.
			bool bAtLeastOneValve = false;

			// From JCC Slack 27th July 2021: Icon warning for lack of partner valve while all chidren circuits are equiped with
			// TA-Smart. Same is true if circuits were equiped with PIBCV. No PV is needed in this case.
			bool bAllChildrenWithPIBCVorSmartControlValve = true;

			for( IDPTR IDPtr = pHM->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pHM->GetNext() )
			{
				CDS_HydroMod *pChild = (CDS_HydroMod *)( IDPtr.MP );
				
				// if 'pChild' has one valve, so pHM has at least one.
				if( true == pChild->HasaValve() )
				{
					bAtLeastOneValve = true;

					if( false == pChild->IsSmartControlValveExist( true ) && ( false == pChild->IsCvExist() || false == pChild->GetpCV()->IsPICV() ) )
					{
						bAllChildrenWithPIBCVorSmartControlValve = false;
					}
					
					if( NULL != pChild->GetpSch() && eDpStab::DpStabOnBVBypass == pChild->GetpSch()->GetDpStab() )
					{
						bAllChildrenWithPIBCVorSmartControlValve = false;
					}
				}
				
				// if 'pChild' is a module, skip to next 'pChild'.
				if( true == pChild->IsaModule() )
				{
					continue;
				}
				
				// No valve specified or flow <= 0.
				if( false == pChild->HasaValve() || pChild->GetQDesign() <= 0.0)
				{
					eOverlayMaskIndex = OverlayMaskIndex::OMI_CrossMark;
				}
			}
			
			if( OverlayMaskIndex::OMI_Undefined == eOverlayMaskIndex )
			{
				// If no partner valve in pHM...
				if( false == pHM->HasaValve() && false == bAllChildrenWithPIBCVorSmartControlValve )
				{
					eOverlayMaskIndex = OverlayMaskIndex::OMI_PartnerValve;
				}
				
				// pHM flow <= 0
				if( pHM->GetQDesign() <= 0.0 )
				{
					eOverlayMaskIndex = OverlayMaskIndex::OMI_CrossMark;
				}
			}
			
			// HYS-1930: we add now auto-adaptive flow with decoupling bypass circuit. If a module is this kind of circuit,
			// no need of partner valve.
			if( OverlayMaskIndex::OMI_Undefined != eOverlayMaskIndex && NULL != pHM->GetpSch() && eDpStab::DpStabOnBVBypass == pHM->GetpSch()->GetDpStab() )
			{
				eOverlayMaskIndex = OverlayMaskIndex::OMI_Undefined;
			}

			// If there is not one single "child" valve in pHM...
			if( false == bAtLeastOneValve && true == pHM->IsaModule() )
			{
				eOverlayMaskIndex = OverlayMaskIndex::OMI_CrossMark;
			}

			// Set the Overlay Mask for hItem.
			m_Tree.SetItemState( hItem, INDEXTOOVERLAYMASK( (int)eOverlayMaskIndex ), TVIS_OVERLAYMASK );
		}
		
		if( TRUE == m_Tree.ItemHasChildren( hItem ) )
		{
			_SetOverlayMask( m_Tree.GetChildItem( hItem ) );
		}

		hItem = m_Tree.GetNextSiblingItem( hItem );
	};
}

void CDlgLeftTabProject::_ComposeModuleName( CString &str, HTREEITEM hItem, bool fCreate )
{
	ASSERT( hItem );
	if( NULL == hItem )
		return;
	
	HTREEITEM hParentItem = m_Tree.GetParentItem( hItem );
	CTable *pTab = (CTable *)m_Tree.GetObjectAt( hParentItem );
	CDS_HydroMod *pParentHM = NULL;
	if( false == pTab->IsClass( CLASS( CTableHM ) ) )
		pParentHM = (CDS_HydroMod *)pTab; 
	CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)m_Tree.GetObjectAt( hItem ) );				ASSERT( NULL != pHM );

	if( NULL == pHM )
		return;
	
	CString tmpstr;
	if( NULL == pParentHM )
	{
		// If root Module...
		if( 0 == pHM->GetLevel() )
			m_pTADS->ComposeRMName( str, pHM, pHM->GetPos() );
		return;
	}

	if( false == fCreate )
	{
		// Go one level up only if parent is not the root module and parent's name contains the automatic rename character.
		if( 0 != pParentHM->GetLevel() && '*' == m_Tree.GetItemText( hParentItem )[0] )
		{
			_ComposeModuleName( str, hParentItem );
			tmpstr.Format( _T(".%d"), pHM->GetPos() );
			str += tmpstr;
		}
		else
		{
			if( '*' == *pParentHM->GetHMName() )
				tmpstr.Format( _T("%s.%d"), pParentHM->GetHMName(), pHM->GetPos() );
			else
				tmpstr.Format( _T("*%s.%d"), pParentHM->GetHMName(), pHM->GetPos() );
			str += tmpstr;
		}
	}
	else
	{
		// In case of module/valve creation the parent is the current selected Item.
		if( '*' == m_Tree.GetItemText( hItem )[0] )
		{
			// Automatic rename authorized.
			_ComposeModuleName( str, hParentItem, true );
			tmpstr.Format( _T(".%d"), pHM->GetPos() );
			str += tmpstr;
		}
		else
		{
			if( '*' == pHM->GetHMName()[0] )
				tmpstr.Format( _T("%s"), pHM->GetHMName() );
			else
				tmpstr.Format( _T("*%s"), pHM->GetHMName() );
			str += tmpstr;
		}
	}
}

void CDlgLeftTabProject::_AddTreeBranch( IDPTR IDPtr, HTREEITEM hParentItem )
{
	ASSERT( '\0' != *IDPtr.ID );
	if( '\0' == *IDPtr.ID )
		return;
	
	CRank SortList;
	CDS_HydroMod *pHM = (CDS_HydroMod *)( IDPtr.MP );
	CTable *pTab = (CTable *)( IDPtr.MP );
	for( IDPTR IDPtrChild = pTab->GetFirst(); '\0' != *IDPtrChild.ID; IDPtrChild = pTab->GetNext() )
	{
		CDS_HydroMod *pChild = (CDS_HydroMod *)( IDPtrChild.MP );
		
		// Fill a CRank list to sort items.
		SortList.Add( _T(""), pChild->GetPos(), (long)pChild );
	}
	
	// Insert sorted items into the tree.
	CString str;
	LPARAM itemdata;
	for( BOOL fContinue = SortList.GetFirst( str, itemdata ); TRUE == fContinue; fContinue = SortList.GetNext( str, itemdata ) )
	{
		CDS_HydroMod *pChild = (CDS_HydroMod *)itemdata;
		HTREEITEM hChild = NULL;
		if( true == pChild->IsaModule() )
		{
			hChild = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
										(LPCTSTR)pChild->GetHMName(), 
										CRCImageManager::ILPT_HydronicCircuit, CRCImageManager::ILPT_HydronicCircuitSelected, 
										0, 0, (long)pChild, hParentItem, TVI_LAST );
			
			m_Tree.SetItemData( hChild, (long)m_Tree.AddToNodeList( CMainFrame::RightViewList::eRVHMCalc, pChild ) );

			if( pChild->GetQDesign() <= 0 )
				m_Tree.SetItemState( hChild, INDEXTOOVERLAYMASK( OverlayMaskIndex::OMI_CrossMark ), TVIS_OVERLAYMASK );
			_AddTreeBranch( pChild->GetIDPtr(), hChild );
		}
		else
		{
			// Insert Terminal Unit in Tree View
			hChild = m_Tree.InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
				(LPCTSTR)pChild->GetHMName(),
				CRCImageManager::ILPT_TerminalUnit, CRCImageManager::ILPT_TerminalUnitSelected,
				0, 0, (long)pChild, hParentItem, TVI_LAST);

			m_Tree.SetItemData(hChild, (long)m_Tree.AddToNodeList(CMainFrame::RightViewList::eRVHMCalc, pChild));

			if (pChild->GetQDesign() <= 0)
			{
				m_Tree.SetItemState(hChild, INDEXTOOVERLAYMASK(OverlayMaskIndex::OMI_CrossMark), TVIS_OVERLAYMASK);
			}
		}
	}
}


HTREEITEM CDlgLeftTabProject::_FindhItem( CString &str, HTREEITEM hItem )
{
	ASSERT( NULL != hItem );
	while( NULL != hItem )
	{
		if( str == m_Tree.GetItemText( hItem ) )
			return hItem;
		
		HTREEITEM hItemFound = NULL;
		if( TRUE == m_Tree.ItemHasChildren( hItem ) )
			hItemFound = _FindhItem( str, m_Tree.GetChildItem( hItem ) );
		if( NULL != hItemFound )
			return hItemFound;
		hItem = m_Tree.GetNextSiblingItem( hItem );
	}
	return NULL;
}

HTREEITEM CDlgLeftTabProject::_FindhItem( void *pData, HTREEITEM hItem )
{
	ASSERT( NULL != hItem );
	while( NULL != hItem )
	{
		if( pData == m_Tree.GetObjectAt( hItem ) )
			return hItem;
		
		HTREEITEM hItemFound = NULL;
		if( TRUE == m_Tree.ItemHasChildren( hItem ) )
			hItemFound = _FindhItem( pData, m_Tree.GetChildItem( hItem ) );
		if( NULL != hItemFound )
			return hItemFound;
		hItem = m_Tree.GetNextSiblingItem( hItem );
	}
	return NULL;
}

HTREEITEM CDlgLeftTabProject::_FindhItem( CDS_HydroMod *pHM, HTREEITEM hItem )
{
	ASSERT( NULL != hItem );
	while( NULL != hItem )
	{
		CDS_HydroMod *pTreeHM = dynamic_cast<CDS_HydroMod *>( (CData *)m_Tree.GetObjectAt( hItem ) );
		if( NULL != pTreeHM && pTreeHM == pHM )
			return hItem;
		
		HTREEITEM hItemFound = NULL;
		if( TRUE == m_Tree.ItemHasChildren( hItem ) )
			hItemFound = _FindhItem( pHM, m_Tree.GetChildItem( hItem ) );
		if( NULL != hItemFound )
			return hItemFound;
		hItem = m_Tree.GetNextSiblingItem( hItem );
	}
	return NULL;
}

void CDlgLeftTabProject::_AddModule( int iPosition )
{
	try
	{
		if( NULL == m_hItem )
		{
			return;
		}

		// If we are in HMCalc mode, it's a circuit creation.
		if( eb3False != m_pTADS->IsHMCalcMode() && true == pMainFrame->IsHMCalcChecked() )
		{
			CTable *pTab = ( CTable * )( m_Tree.GetObjectAt( m_hItem ) );

			if( NULL != pTab )
			{
				// In the panel circuit 1 & 2 we modify the structure of the hydraulic network. If we are in the panel circuit 2,
				// and if we go back in the panel circuit 1 to change the circuit scheme and go in the panel circuit 2, the 'm_pHM' 
				// variable of 'SheetHMCalc' is no more the good one. If for some reason 'CRViewHMCalc::OnDraw' is called, HySelect crashes.
				// This is why we need to block redraw while we are in the panel circuits.
				pRViewHMCalc->EnableOnDraw( false );

				CDlgWizCircuit dlg( this );
				dlg.Init( pTab, false );
				dlg.DisplayWizard();

				pRViewHMCalc->EnableOnDraw( true );
			}

			// Clean cutting object if exist.
			if( NULL != pRViewHMCalc )
			{
				pRViewHMCalc->ResetFlagCut();
			}

			m_pTADS->CleanClipboard();

			CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( pTab );
			CString strHMName = ( NULL != pHM ) ? pHM->GetHMName() : _T("");
			ResetTreeWithString( strHMName );
		}
		else
		{
			int iReturn = IDOK;
			int iMaxPosition = 0;
			CString strPreviousSelThingID = _T( "" );
			CString strPreviousTADBID = _T( "" );
			double dPreviousQ = 0.0;
			enum_VDescriptionType ePreviousVDescrType = edt_TADBValve;
			double dPreviousKvCv = 0.0;
			double dPreviousPresetting = -1.0;

			// Remark: for the first time, we set the position in regards to the passed argument. If the user changes level and position
			//         where to insert the new created module in the 'CDlgCreateModule' dialog, it's in regards to this new position that
			//         we will create the next module.
			int iPositionToSet;
			CData *pData = ( CData * )( m_Tree.GetObjectAt( m_hItem ) );

			if( true == pData->IsClass( CLASS( CTableHM ) ) )
			{
				iMaxPosition = m_pTADS->GetLastBranchPos( ( CTable * )pData );
				iPositionToSet = iMaxPosition + 1;
			}
			else
			{
				// Retrieve the last used index in this module.
				int iMaxPosition = m_pTADS->GetLastBranchPos( ( CDS_HydroMod * )pData );

				if( -1 == iPosition )
				{
					iPositionToSet = iMaxPosition + 1;
				}
				else if( 0 == iPosition )
				{
					iPositionToSet = 1;
				}
				else if( iPosition <= iMaxPosition )
				{
					iPositionToSet = iPosition;
				}
				else
				{
					iPositionToSet = iMaxPosition + 1;
				}
			}

			// Create modules.
			while( IDOK == iReturn )
			{
				CDlgCreateModule dlg( this, this );
				CString str, tmpstr;
				CDS_HydroMod *pHM = NULL;
				CData *pData = ( CData * )( m_Tree.GetObjectAt( m_hItem ) );

				if( true == pData->IsClass( CLASS( CTableHM ) ) )
				{
					// Root module.
					iMaxPosition = m_pTADS->GetLastBranchPos( ( CTable * )pData );
					// Index in 'CTADatastruct::ComposeRMName' is 0-indexed (in the 'CDS_HydroMod' class it's 1-indexed!).
					m_pTADS->ComposeRMName( str, 0, iPositionToSet - 1 );
				}
				else
				{
					// Hydromod.
					pHM = ( CDS_HydroMod * )pData;

					// Retrieve the last used index in this module.
					iMaxPosition = m_pTADS->GetLastBranchPos( pHM );
					_ComposeModuleName( str, m_hItem, true );

					tmpstr.Format( _T(".%d"), iPositionToSet );
					str += tmpstr;

					// If resulting module name is larger than max. size, make it "Unnamed".
					if( str.GetLength() > TASApp.GetModuleNameMaxChar() )
					{
						CTable *pTab = m_pTADS->GetpHydroModTable();
						
						if( NULL == pTab )
						{
							HYSELECT_THROW( _T("Internal error: Can't retrieve the parent hydromod table from the datastruct.") );
						}

						str = _T("*") + TASApp.LoadLocalizedString( IDS_VALVENONAME );
						m_pTADS->DifferentiateHMName( pTab, str );
					}
				}

				FormatString( tmpstr, IDS_DLGCREATEMODULE_CPTNCREATEMODULE, m_Tree.GetItemText( m_hItem ) );
				iReturn = dlg.Display( CDlgCreateModule::CreateModule, tmpstr, str, pHM, iPositionToSet, iMaxPosition + 1,
									   dPreviousQ, strPreviousSelThingID, ePreviousVDescrType, dPreviousKvCv, dPreviousPresetting );

				if( IDOK == iReturn )
				{
					pHM = dlg.GetParentModule();

					// Verify if the name is not already used.
					bool bFlag = false;
				
					CTable *pPipTab = m_pTADS->GetpHydroModTable();
					
					if( NULL == pPipTab )
					{
						HYSELECT_THROW( _T("Internal error: Can't retrieve the parent hydromod table from the datastruct.") );
					}

					// Do this only if the name is not set to automatic.
					if( dlg.GetReference().GetLength() > 0 && '*' != dlg.GetReference().GetAt( 0 ) )
					{
						if( NULL != m_pTADS->FindHydroMod( dlg.GetReference(), pPipTab ) )
						{
							bFlag = true;
						}

						if( true == bFlag )
						{
							CString str1;
							::FormatString( str1, IDS_HYDROMODREF_ALREADYUSED, dlg.GetReference() );
							AfxMessageBox( str1 );
							continue;
						}
					}

					// Clean cutting object if exist.
					if( NULL != pRViewHMCalc )
					{
						pRViewHMCalc->ResetFlagCut();
					}

					m_pTADS->CleanClipboard();

					// Verify if the new index is free, if not shift up.
					m_pTADS->VerifyNewPos( ( CTable * )pData, dlg.GetPosition() );

					// Create a new module in the current selected module m_hItem.
					IDPTR ChildIDptr;
					m_pTADS->CreateObject( ChildIDptr, CLASS( CDS_HydroMod ) );

					if( NULL != pHM )
					{
						pHM->Insert( ChildIDptr );
					}
					else
					{
						// Create a Root Module... attach to the PipingTaB.
						pPipTab->Insert( ChildIDptr );
					}
					
					CDS_HydroMod *pChildHM = ( ( CDS_HydroMod * )ChildIDptr.MP );

					// Set name.
					pChildHM->SetHMName( ( LPCTSTR )dlg.GetReference() );

					// Set description.
					pChildHM->SetDescription( dlg.GetDescription() );

					// Set level.
					if( NULL != pHM )
					{
						pChildHM->SetLevel( pHM->GetLevel() + 1 );
					}
					else
					{
						pChildHM->SetLevel( 0 );
					}

					// Set index, flow and flagModule.
					iPositionToSet = dlg.GetPosition();
					pChildHM->SetPos( iPositionToSet++ );

					dPreviousQ = dlg.GetFlow();
					pChildHM->SetQDesign( dPreviousQ );

					pChildHM->SetFlagModule( true );

					strPreviousTADBID = dlg.GetSelTADBID();
					pChildHM->SetCBIValveID( dlg.GetSelTADBID() );

					// Test to see if the partner valve has been created.
					// If true, associate a Uid to it.
					pChildHM->SetUid( m_pTADS->GetpProjectParams()->GetNextUid() );

					// Complete the module to be compatible in HMCalc mode.
					ePreviousVDescrType = dlg.GetVDescrType();
					pChildHM->SetVDescrType( ePreviousVDescrType );

					dPreviousKvCv = dlg.GetKvCv();
					pChildHM->SetKvCv( dPreviousKvCv );

					dPreviousPresetting = dlg.GetPresetting();
					pChildHM->SetPresetting( dPreviousPresetting );

					ResetTreeWithString( m_Tree.GetItemText( m_hItem ) );
				}
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgLeftTabProject::_AddModule'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgLeftTabProject::_AddValve( int iPosition )
{
	try
	{
		if( NULL == m_vecSelectedItem[0] )
		{
			return;
		}

		int iReturn = IDOK;
		CString strPreviousSelThingID = _T( "" );
		CString strPreviousTADBID = _T( "" );
		double dPreviousQ = 0.0;
		enum_VDescriptionType ePreviousVDescrType = edt_TADBValve;
		double dPreviousKvCv = 0.0;
		double dPreviousPresetting = -1.0;

		// Retrieve position of the last circuit.
		CDS_HydroMod *pHM = ( CDS_HydroMod * )m_Tree.GetObjectAt( m_vecSelectedItem[0] );
		int iMaxPosition = m_pTADS->GetLastBranchPos( pHM );

		int iPositionToSet;

		if( -1 == iPosition )
		{
			iPositionToSet = iMaxPosition + 1;
		}
		else if( 0 == iPosition )
		{
			iPositionToSet = 1;
		}
		else if( iPosition <= iMaxPosition )
		{
			iPositionToSet = iPosition;
		}
		else
		{
			iPositionToSet = iMaxPosition + 1;
		}

		while( IDOK == iReturn )
		{
			CDlgCreateModule dlg( this, this );
			CString str, tmpstr;
			_ComposeModuleName( str, m_vecSelectedItem[0], true );

			tmpstr.Format( _T(".%d"), iPositionToSet );
			str += tmpstr;

			// If resulting valve name is larger than max. size, make it "Unnamed".
			if( str.GetLength() > TASApp.GetModuleNameMaxChar() )
			{
				CTable *pTab = m_pTADS->GetpHydroModTable();
				
				if( NULL == pTab )
				{
					HYSELECT_THROW( _T("Internal error: Can't retrieve the parent hydromod table from the datastruct.") );
				}

				str = _T("*") + TASApp.LoadLocalizedString( IDS_VALVENONAME );
				m_pTADS->DifferentiateHMName( pTab, str );
			}

			CString strCaption;
			FormatString( strCaption, IDS_DLGCREATEMODULE_CPTNCREATEVALVE, m_Tree.GetItemText( m_vecSelectedItem[0] ) );

			iMaxPosition = m_pTADS->GetLastBranchPos( pHM );
			iReturn = dlg.Display( CDlgCreateModule::CreateValve, strCaption, str, pHM, iPositionToSet, iMaxPosition + 1,
								   dPreviousQ, strPreviousSelThingID, ePreviousVDescrType, dPreviousKvCv, dPreviousPresetting );

			if( IDOK == iReturn )
			{
				pHM = dlg.GetParentModule();

				// Verify if the name is not already used.
				// Do this only if the name is not set to automatic.
				if( dlg.GetReference().GetLength() > 0 && '*' != dlg.GetReference().GetAt( 0 ) )
				{
					bool bFlag = false;
				
					CTable *pPipTab = m_pTADS->GetpHydroModTable();
					
					if( NULL == pPipTab )
					{
						HYSELECT_THROW( _T("Internal error: Can't retrieve the parent hydromod table from the datastruct.") );
					}

					if( NULL != m_pTADS->FindHydroMod( dlg.GetReference(), pPipTab ) )
					{
						bFlag = true;
					}

					if( true == bFlag )
					{
						CString str1;
						::FormatString( str1, IDS_HYDROMODREF_ALREADYUSED, dlg.GetReference() );
						AfxMessageBox( str1 );
						continue;
					}
				}

				// Clean cutting object if exist.
				if( NULL != pRViewHMCalc )
				{
					pRViewHMCalc->ResetFlagCut();
				}

				m_pTADS->CleanClipboard();

				// Verify if the new index is free, if not shift up.
				m_pTADS->VerifyNewPos( pHM, dlg.GetPosition() );

				// Create a new valve in the current selected module m_hItem.
				IDPTR ChildIDptr;
				m_pTADS->CreateObject( ChildIDptr, CLASS( CDS_HydroMod ) );
				pHM->Insert( ChildIDptr );
			
				IDPTR IDPtr = pHM->GetIDPtr();
				CDS_HydroMod *pChildHM = ( ( CDS_HydroMod * )ChildIDptr.MP );
				pChildHM->SetHMName( ( LPCTSTR )dlg.GetReference() );
				pChildHM->SetLevel( pHM->GetLevel() + 1 );
				pChildHM->SetPos( dlg.GetPosition() );
				pChildHM->SetFlagModule( false );
				pChildHM->SetQDesign( dlg.GetFlow() );
				pChildHM->SetUid( m_pTADS->GetpProjectParams()->GetNextUid() );
				pChildHM->SetPresetting( dlg.GetPresetting() );
				pChildHM->SetDescription( dlg.GetDescription() );
				pChildHM->SetVDescrType( dlg.GetVDescrType() );
				pChildHM->SetKvCv( dlg.GetKvCv() );

				strPreviousTADBID = dlg.GetSelTADBID();
				ePreviousVDescrType = dlg.GetVDescrType();
				dPreviousKvCv = dlg.GetKvCv();
				dPreviousPresetting = dlg.GetPresetting();

				pChildHM->SetCBIValveID( dlg.GetSelTADBID() );
				dPreviousQ = dlg.GetFlow();

				// Complete the module to be compatible in HMCalc mode.
				pChildHM->SetKvCv( dPreviousKvCv );
				pChildHM->SetVDescrType( ePreviousVDescrType );

				//_RenameBranch( m_vecSelectedItem[0] );
				m_pTADS->RenameBranch( ( CTableHM * )pHM );
				m_pTADS->ComputeFlow( ( CDS_HydroMod * )ChildIDptr.MP, true );
				_SetOverlayMask( m_Tree.GetRootItem() );
				RefreshRightView();
				ResetTreeAndSelectHM(pHM);

				iPositionToSet = dlg.GetPosition() + 1;
			}
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgLeftTabProject::_AddValve'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgLeftTabProject::_DeleteChart( HTREEITEM hItem )
{
	// Get pointer and IDPTR on selected logged data.
	CLog *pLD = (CLog *)( m_Tree.GetObjectAt( hItem ) );
	ASSERT( NULL != pLD );

	IDPTR IDPtr = pLD->GetIDPtr();
	
	// Get parent table pointer.
	CTable *pTab = (CTable *)( IDPtr.PP );
	pTab->Remove( IDPtr );
	m_pTADS->DeleteObject( IDPtr );
}

CString CDlgLeftTabProject::_DeleteModule( HTREEITEM hItem )
{
	// Keep a copy of parent name to reset the tree at this position.
	CString str( _T("") );
	CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)( m_Tree.GetObjectAt( hItem ) ) );
	if( NULL != pHM )
	{
		CTable *pTab = (CTable *)( pHM->GetIDPtr().PP );
		if( true == pTab->IsClass( CLASS( CTableHM ) ) )
			str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
		else
			str = ( (CDS_HydroMod *)pTab )->GetHMName();
		m_pTADS->DeleteHM( pHM );
	}
	return str;
}

void CDlgLeftTabProject::_AddChildrenInCutList( CDS_HydroMod *pclParentHM )
{
	IDPTR ChildIDPtr = pclParentHM->GetFirst();
	while( NULL != ChildIDPtr.MP )
	{
		CDS_HydroMod *pChildHM = dynamic_cast<CDS_HydroMod *>( ChildIDPtr.MP );
		if( NULL != pChildHM )
		{
			HTREEITEM hFoundItem = _FindhItem( pChildHM, m_hHNBranch );
			if( (HTREEITEM)NULL != hFoundItem )
			{
				m_Tree.SetItemCut( hFoundItem );
				if( true == pChildHM->IsaModule() )
					_AddChildrenInCutList( pChildHM );
			}
		}
		ChildIDPtr = pclParentHM->GetNext();
	}
}

void CDlgLeftTabProject::_TabProjPaste( PasteMode ePasteMode )
{
	int iNbrProductSelected = (int)m_vecSelectedItem.size();
	int iNbrProductInClipboard = m_pTADS->GetCountHMClipboard();

	if( 0 == iNbrProductSelected || 0 == iNbrProductInClipboard )
	{
		return;
	}

	CTableHM *pclHydraulicNetwork = dynamic_cast<CTableHM *>( m_pTADS->GetpHydroModTable() );

	if( NULL == pclHydraulicNetwork )
	{
		ASSERT_RETURN;
	}

	// Add here a cleaning for the case 'PasteMode::PasteBefore'. If we have more that one line selected, we must just insert before
	// the line that have the lowest position in the module.
	std::vector<CTable *> vecHMSelected;
	CTable *pHMWhereToInsert = NULL;

	for( int iLoopSelectedLine = 0; iLoopSelectedLine < iNbrProductSelected; iLoopSelectedLine++ )
	{
		CTable *pHM = dynamic_cast<CTable *>( (CData *)( m_Tree.GetObjectAt( m_vecSelectedItem[iLoopSelectedLine] ) ) );

		if( NULL == pHM )
		{
			continue;
		}

		if( PasteMode::PasteBefore == ePasteMode && iNbrProductSelected > 1 )
		{
			// Here 'pHM' is inevitably a CDS_HydromMod. It's impossible to do multiple selection on the main root of the hydronic network.
			CDS_HydroMod *pHMCasted = dynamic_cast<CDS_HydroMod *>( pHM );

			if( NULL == pHMWhereToInsert
					|| ( 0 == CString( pHMWhereToInsert->GetClassName() ).Compare( _T("CDS_HydroMod") ) && pHMCasted->GetPos() < ( (CDS_HydroMod *)pHMWhereToInsert )->GetPos() ) )
			{
				pHMWhereToInsert = pHM;
			}
		}
		else
		{
			vecHMSelected.push_back( pHM );
		}
	}

	if( NULL != pHMWhereToInsert )
	{
		vecHMSelected.push_back( pHMWhereToInsert );
	}

	// Check if there is a root pump circuit in the clipboard.
	bool bAtLeastOneRootPump = false;
	IDPTR IDPtr = m_pTADS->GetFirstHMFromClipBoard();

	while( IDPtr != _NULL_IDPTR && false == bAtLeastOneRootPump )
	{
		CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( (CData *)IDPtr.MP );

		if( NULL != pHMInClipboard )
		{
			// We take only the case for root module (level = 0) with pump that are not a circuit in injection.
			if( 0 == pHMInClipboard->GetLevel() && NULL != pHMInClipboard->GetpPump()
					&& ( NULL != pHMInClipboard->GetpSchcat() && false == pHMInClipboard->GetpSchcat()->IsInjection() ) )
			{
				bAtLeastOneRootPump = true;
			}
		}

		if( false == bAtLeastOneRootPump )
		{
			IDPtr = m_pTADS->GetNextHMFromClipBoard();
		}
	}

	// If a root pump circuit exists...
	if( true == bAtLeastOneRootPump )
	{
		for( std::vector<CTable *>::iterator iterSelectedLine = vecHMSelected.begin(); iterSelectedLine != vecHMSelected.end(); iterSelectedLine++ )
		{
			CTableHM *pTableHM = NULL;
			CDS_HydroMod *pHM = NULL;

			if( 0 == CString( (*iterSelectedLine)->GetClassName() ).Compare( _T("CTableHM") ) )
			{
				pTableHM = dynamic_cast<CTableHM *>( *iterSelectedLine );
			}
			else
			{
				pHM = dynamic_cast<CDS_HydroMod *>( *iterSelectedLine );
			}

			if( NULL == pTableHM && NULL == pHM )
			{
				continue;
			}

			// Root pump circuit can be only paste in level 0.
			if( NULL != pHM && PasteMode::PasteIn == ePasteMode )
			{
				CString str = TASApp.LoadLocalizedString( IDS_RVHMCALC_CANTPASTEPUMPCIRCUIT );
				::AfxMessageBox( (LPCTSTR)str, MB_OK | MB_ICONSTOP );
				return;
			}
		}
	}

	// Verify the return mode of the objects in the clipboard regarding to the objects where user wants to paste them.
	bool bAllCanPasteIn = true;
	bool bAllCanPasteBefore = true;
	IDPtr = m_pTADS->GetFirstHMFromClipBoard();

	while( NULL != IDPtr.MP && ( true == bAllCanPasteIn || true == bAllCanPasteBefore ) )
	{
		CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( (CData *)IDPtr.MP );
					
		// Run all selected lines.
		for( std::vector<CTable *>::iterator iterSelectedLine = vecHMSelected.begin(); iterSelectedLine != vecHMSelected.end(); iterSelectedLine++ )
		{
			CTableHM *pTableHM = NULL;
			CDS_HydroMod *pHM = NULL;

			if( 0 == CString( (*iterSelectedLine)->GetClassName() ).Compare( _T("CTableHM") ) )
			{
				pTableHM = dynamic_cast<CTableHM *>( *iterSelectedLine );
			}
			else
			{
				pHM = dynamic_cast<CDS_HydroMod *>( *iterSelectedLine );
			}

			if( NULL == pTableHM && NULL == pHM )
			{
				continue;
			}

			// Test 'Paste in'.
			// If user wants to paste in main root (Hydraulic network), no need to verify the return mode.
			// And no need to check also if user wants to paste in a module that has no child.
			if( PasteMode::PasteIn == ePasteMode && NULL != pHM && pHM->GetCount() > 0 )
			{
				CDS_HydroMod *pCHildHM = (CDS_HydroMod *)( pHM->GetFirst().MP );

				if( NULL != pCHildHM && pHMInClipboard->GetReturnType() != pCHildHM->GetReturnType() )
				{
					bAllCanPasteIn = false;
				}
			}

			// Test 'Paste before'.
			// If user wants to paste before a root module (at level 0), no need to verity the return mode.
			if( PasteMode::PasteBefore == ePasteMode && NULL != pHM && pHM->GetLevel() > 0 )
			{
				if( pHMInClipboard->GetReturnType() != pHM->GetReturnType() )
				{
					bAllCanPasteBefore = false;
				}
			}
		}
		
		IDPtr = m_pTADS->GetNextHMFromClipBoard();
	}

	bool bApplyReturnCorrection = false;

	if( ( false == bAllCanPasteIn && PasteMode::PasteIn == ePasteMode ) 
			|| ( false == bAllCanPasteBefore && PasteMode::PasteBefore == ePasteMode ) )
	{
		// Either user wants to paste in a module or user wants to paste before a module (or circuit) but object(s) in the 
		// clipboard is not compatible in regards to the return mode.
		CString str = TASApp.LoadLocalizedString( IDS_RVHMCALC_INCOMPATIBLERETURNMODE );

		if( IDNO == ::AfxMessageBox( (LPCTSTR)str, MB_YESNO | MB_ICONWARNING ) )
		{
			return;
		}
		
		bApplyReturnCorrection = true;
	}

	// HYS-1716: verify if there is no error with secondary temperatures of main and child hydraulic circuits when pasting.
	IDPtr = m_pTADS->GetFirstHMFromClipBoard();
	bool bIsInjectionCircuitTemperatureError = false;
	std::multimap<CDS_HydroMod *, CTable *> mmapInjectionHydraulicCircuitWithTemperatureError;
	std::vector<CDS_HydroMod *> vecAllInjectionCircuitsWithTemperatureError;

	while( NULL != IDPtr.MP  )
	{
		CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( (CData *)IDPtr.MP );
					
		// Run all selected lines.
		for( auto &iterSelectedLine : vecHMSelected )
		{
			CTable *pclDestinationTable = NULL;

			if( PasteMode::PasteIn == ePasteMode )
			{
				pclDestinationTable = iterSelectedLine;
			}
			else
			{
				ASSERT( NULL != dynamic_cast<CDS_HydroMod *>( iterSelectedLine ) );
				pclDestinationTable = ( (CDS_HydroMod *)( iterSelectedLine ) )->GetParent();
			}
			
			bIsInjectionCircuitTemperatureError |= pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForPasteOperation( pHMInClipboard, pclDestinationTable, 
					&mmapInjectionHydraulicCircuitWithTemperatureError, &vecAllInjectionCircuitsWithTemperatureError );
		}
		
		IDPtr = m_pTADS->GetNextHMFromClipBoard();
	}

	CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode = CDlgInjectionError::ReturnCode::Undefined;

	if( true == bIsInjectionCircuitTemperatureError )
	{
		CDlgInjectionError DlgInjectionError( &vecAllInjectionCircuitsWithTemperatureError );
		eDlgInjectionErrorReturnCode = (CDlgInjectionError::ReturnCode)DlgInjectionError.DoModal();

		if( CDlgInjectionError::ReturnCode::Cancel == eDlgInjectionErrorReturnCode )
		{
			return;
		}

		// The correction will be applied at each paste. Otherwise we will do the correction in a temporary object.
		// (When doing a copy of hydraulic circuits, we do a copy of each one in the clipboard).
	}

	// Now we can run paste on all selected lines.
	BeginWaitCursor();

	CTable *pDestTable = NULL;
	CTable *pHMToReset = NULL;
	int iTargetLevel, iNewPos;
	bool bPasteDone = false;

	for( std::vector<CTable *>::iterator iterSelectedLine = vecHMSelected.begin(); iterSelectedLine != vecHMSelected.end(); iterSelectedLine++ )
	{
		CTableHM *pTableHM = NULL;
		CDS_HydroMod *pHM = NULL;
	
		if( 0 == CString( (*iterSelectedLine)->GetClassName() ).Compare( _T("CTableHM") ) )
		{
			pTableHM = dynamic_cast<CTableHM *>( *iterSelectedLine );
		}
		else
		{
			pHM = dynamic_cast<CDS_HydroMod *>( *iterSelectedLine );
		}

		if( NULL == pTableHM && NULL == pHM )
		{
			continue;
		}

		if( PasteMode::PasteIn == ePasteMode )
		{
			// User pastes object on a module, we add these objects at the end of this module.

			// Destination is set to the current module selected.
			pDestTable = ( NULL != pTableHM ) ? (CTable *)pTableHM : (CTable *)pHM;

			// Level will be the incremented by 1 because the object is copied under the module selected.
			iTargetLevel = ( NULL != pTableHM ) ? 0 : pHM->GetLevel() + 1;

			// Added at the end of the module selected.
			iNewPos = ( NULL != pTableHM) ? pTableHM->GetItemCount() + 1 : pHM->GetCount() + 1;

			// Check on which module we will reset.
			if( NULL == pHMToReset )
			{
				// If the reset module is not yet set, we take in regards to the current line selected.
				pHMToReset = ( NULL != pTableHM ) ? (CTable *)pTableHM : (CTable *)pHM;
			}
			else if( 0 == CString( pHMToReset->GetClassName() ).Compare( _T("CTableHM") ) )
			{
				// If reset module is already set and it's the main root table, we change only if the current line is
				// a 'CDS_HydroMod'.
				if( NULL != pHM )
				{
					pHMToReset = (CTable *)pHM;
				}
			}
			else if( 0 != pHM )
			{
				// If reset module is already set and it's a 'CDS_HydroMod',  we take the one that has the lower level.
				if( ( (CDS_HydroMod *)pHMToReset )->GetLevel() > pHM->GetLevel() )
				{
					pHMToReset = (CTable *)pHM;
				}
			}
		}
		else
		{
			// User wants to insert object before the selected object.

			// Destination is set to the parent of the current line selected.
			if( pHM->GetLevel() > 0 )
			{
				pDestTable = pHM->GetParent();
			}
			else
			{
				pDestTable = m_pTADS->GetpHydroModTable();
			}

			// Level is the same as the current object selected.
			iTargetLevel = pHM->GetLevel();

			// Added just before the selected object.
			iNewPos = pHM->GetPos();

			// We will reset on the module that has the lower level.
			if( NULL == pHMToReset )
			{
				pHMToReset = pHM->GetParent();
			}
			else if( ( (CDS_HydroMod *)pHMToReset )->GetLevel() > pHM->GetParent()->GetLevel() )
			{
				pHMToReset = pHM->GetParent();
			}
		}
		
		IDPTR IDPtrLoop = m_pTADS->GetFirstHMFromClipBoard();

 		while( NULL != IDPtrLoop.MP )
 		{
			IDPTR HMIDPtr = m_pTADS->CreateCopyHM( IDPtrLoop, pDestTable, false );
			CDS_HydroMod *pHMPaste = (CDS_HydroMod *)( HMIDPtr.MP );

			if( NULL != pHMPaste )
			{
				// If we must verify and apply correction about the return mode...
				if( true == bApplyReturnCorrection )
				{
					// If a previous circuit exists...
					if( iNewPos > 1 )
					{
						CDS_HydroMod *pFirstChildHM = (CDS_HydroMod *)( pDestTable->GetFirst().MP );

						if( pFirstChildHM->GetReturnType() != pHMPaste->GetReturnType() )
						{
							// If return type is different, we force the return type of the pasted object with the one of 
							// already existing.
							pHMPaste->SetReturnType( pFirstChildHM->GetReturnType() );
						}
					}
				}

				// Reset level.
				m_pTADS->SetHMLevel( pHMPaste, iTargetLevel );

				if( 0 == iTargetLevel )
				{
					// Reset distribution and circuit pipe length when module is at level 0.
					if( NULL != pHMPaste->GetpDistrSupplyPipe() )
					{
						pHMPaste->GetpDistrSupplyPipe()->SetLength( 0.0 );
					}

					if( NULL != pHMPaste->GetpDistrReturnPipe() )
					{
						pHMPaste->GetpDistrReturnPipe()->SetLength( 0.0 );
					}

					if( NULL != pHMPaste->GetpCircuitPrimaryPipe() )
					{
						pHMPaste->GetpCircuitPrimaryPipe()->SetLength( 0.0 );
					}
				}

				// If we insert, we change the position of the circuits just following the pasted one.
				if( PasteMode::PasteBefore == ePasteMode )
				{
					for( IDPTR IDPtr = pDestTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pDestTable->GetNext() )
					{
						if( ( (CDS_HydroMod *)IDPtr.MP )->GetPos() >= iNewPos )
						{
							( (CDS_HydroMod *)IDPtr.MP )->SetPos( ( (CDS_HydroMod *)IDPtr.MP )->GetPos() + 1 );
						}
					}
				}

				pHMPaste->SetPos( iNewPos++ );
				pHMPaste->RenameMe();

				// HYS-1716: Now we can correct injection temperatures that contain error.
				if( CDlgInjectionError::ReturnCode::ApplyWithCorrection == eDlgInjectionErrorReturnCode )
				{
					if( 0 != mmapInjectionHydraulicCircuitWithTemperatureError.count( (CDS_HydroMod *)( IDPtrLoop.MP ) ) )
					{
						// First we will extract the range in the multimap that is corresponding to the object we have pasted.
						// Remark: we can past one object in multi destinations. This is why we use a multimap.
						std::pair<std::multimap<CDS_HydroMod *, CTable *>::iterator, std::multimap<CDS_HydroMod *, CTable *>::iterator> ret;
						ret = mmapInjectionHydraulicCircuitWithTemperatureError.equal_range( (CDS_HydroMod *)( IDPtrLoop.MP ) );
					
						for( std::multimap<CDS_HydroMod *, CTable *>::iterator iter = ret.first; iter !=ret.second; iter++ )
						{
							// Now, for the object we check the one that is corresponding with the current selected line (or parent) with 'pDestTable'.
							if( iter->second == pDestTable )
							{
								pclHydraulicNetwork->CorrectOneInjectionCircuit( pHMPaste, iter->second );
								mmapInjectionHydraulicCircuitWithTemperatureError.erase( iter );
								break;
							}
						}
					}
				}

				bPasteDone = true;
			}
			
			IDPtrLoop = m_pTADS->GetNextHMFromClipBoard();
		}
	}

	if( true == bPasteDone )
	{
		// If user has 'Cut' object, we must destroy it.
		if( NULL != pRViewHMCalc && CRViewHMCalc::LastOperation::Cut == pRViewHMCalc->GetLastCopyCutOperation() )
		{
			// We need to do this here. Because once hydromod will be deleted in the code below, pointer in the tree will be no more valid.
			ResetFlagCut();

			CTable *pPipTab = m_pTADS->GetpHydroModTable();
			ASSERT( NULL != pPipTab );
			
			std::vector<CDS_HydroMod *> vecHMToDelete;
			IDPTR IDPtrLoop = m_pTADS->GetFirstHMFromClipBoard();

			while( NULL != IDPtrLoop.MP )
 			{
				CDS_HydroMod *pHMInClipboard = dynamic_cast<CDS_HydroMod *>( (CData *)( IDPtrLoop.MP ) );
				
				if( NULL != pHMInClipboard )
				{
					CDS_HydroMod *pHMInDataStruct = m_pTADS->FindHydroMod( pHMInClipboard->GetHMName(), pPipTab );
					
					if( NULL != pHMInDataStruct )
					{
						vecHMToDelete.push_back( pHMInDataStruct );
					}
				}
				
				IDPtrLoop = m_pTADS->GetNextHMFromClipBoard();
			}

			for( int iLoopHM = 0; iLoopHM < (int)vecHMToDelete.size(); iLoopHM++ )
			{
				m_pTADS->DeleteHM( vecHMToDelete[iLoopHM] );
			}

			// Also, if cut operation is done, we remove all objects in the clipboard (it's not the same as the copy operation where 
			// user can in this case do copy more than one time).
			m_pTADS->CleanClipboard();
			pRViewHMCalc->SetLastCopyCutOperation( CRViewHMCalc::LastOperation::Undefined );
		}

		// HYS-1716: Now that the paste operation has been applied, we need to run all chidren injection circuit to update their
		// primary flows.
		// Remark: here we pass as argument the hydraulic circuit in which we have done the paste because we do not need to check all the network 
		//         but only the children of this hydraulic circuit.
		pclHydraulicNetwork->CorrectAllPrimaryFlow( pDestTable );

		// Compute all installation only if TASelect is in HM calculation mode and the project is not frozen.
		CDS_ProjectParams *pPrjParam = m_pTADS->GetpProjectParams();
		ASSERT( NULL != pPrjParam );
		
		if( true == pMainFrame->IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
		{
			m_pTADS->ComputeAllInstallation();
		}
		
		PostMessage( WM_USER_RESETHMTREE, (WPARAM)true, (LPARAM)pHMToReset );
	}

	EndWaitCursor();
}

void CDlgLeftTabProject::EnsureVisible( CDS_HydroMod *pHM, OverlayMaskIndex omi /*= OMI_Undefined*/ )
{
	if( NULL == pHM )
		return;

	HTREEITEM hItem = _FindhItem( pHM, m_Tree.GetRootItem() );

	if( NULL == hItem )
		return;

	m_Tree.EnsureVisible( hItem );

	if( omi == OMI_Undefined )
	{
		_SetOverlayMask( hItem );
	}
	else
	{
		m_Tree.SetItemState( hItem, INDEXTOOVERLAYMASK( ( int )omi ), TVIS_OVERLAYMASK );
	}
}
