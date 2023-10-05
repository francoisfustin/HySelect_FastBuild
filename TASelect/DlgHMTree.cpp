#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "Global.h"
#include "Utilities.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "HMPipes.h"
#include "dlgwizcircuit.h"
#include "DlgHMTree.h"


CDlgHMTree::CDlgHMTree( CWnd* pParent )
	: CDialogExt( CDlgHMTree::IDD, pParent )
{
	m_pParent = pParent;
	m_pTADS = NULL;
	m_pSelHM = NULL;
	m_bInitialized = false;
	m_fEditMode = false;
	m_strSectionName = _T("DialogHMTree");
	m_pclTreeImageList = NULL;
	m_hSelItem = NULL;
	m_fInitRunning = false;
}

CDlgHMTree::~CDlgHMTree()
{
	if( NULL != m_pclTreeImageList )
		delete m_pclTreeImageList;
}

BOOL CDlgHMTree::Create()
{
	return CDialogExt::Create( CDlgHMTree::IDD );
}

void CDlgHMTree::FillHMTreeView( CTable* pTab, HTREEITEM hParentItem )
{
	CString str;

	if( NULL == pTab )
	{
		m_Tree.DeleteAllItems();
		
		pTab = m_pTADS->GetpHydroModTable();
		ASSERT( NULL != pTab );
		
		str = TASApp.LoadLocalizedString( IDS_HYDRAULIC_NETWORK );
		hParentItem = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, 
										 CRCImageManager::ILPT_HydronicNetwork, CRCImageManager::ILPT_HydronicNetworkSelected, 
										 0, 0, NULL, TVI_ROOT, TVI_ROOT );
		m_Tree.Expand( hParentItem, TVE_EXPAND );
	}
	
	CRank SortList;

	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		SortList.Add( _T(""), ((CDS_HydroMod *)IDPtr.MP )->GetPos(), (long)IDPtr.MP );
	}
	
	LPARAM lparam;
	
	for( BOOL fContinue = SortList.GetFirst( str, lparam ); TRUE == fContinue; fContinue = SortList.GetNext( str, lparam ) )
	{
		CDS_HydroMod* pHM = (CDS_HydroMod*)lparam;
		HTREEITEM hChild = NULL;
		
		OverlayMaskIndex eOverlayImage = ( true == m_fEditMode ) ? OverlayMaskIndex::OMI_EditCircuit : OverlayMaskIndex::OMI_NewCircuit;
		
		if( true == pHM->IsaModule() )
		{
			if( pHM == m_pSelHM )
			{
				hChild = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHM->GetHMName(), 
											CRCImageManager::ILPT_HydronicCircuitSelected, CRCImageManager::ILPT_HydronicCircuitSelected, 
											0, 0, (long)pHM, hParentItem, TVI_LAST );
				m_hSelItem = hChild;
				m_Tree.SetItemState( hChild, INDEXTOOVERLAYMASK( (int)eOverlayImage ), TVIS_OVERLAYMASK );
				m_Tree.SelectItem( hChild );
			}
			else
				hChild = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHM->GetHMName(), 
											CRCImageManager::ILPT_HydronicCircuit, CRCImageManager::ILPT_HydronicCircuit, 
											0, 0, (long)pHM, hParentItem, TVI_LAST );

			m_Tree.Expand( hChild, TVE_EXPAND );
			FillHMTreeView( pHM, hChild );
		}
		else 
		{
			if( pHM == m_pSelHM )
			{	
				hChild = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHM->GetHMName(), 
											CRCImageManager::ILPT_TerminalUnitSelected, CRCImageManager::ILPT_TerminalUnitSelected, 
											0, 0, (long)pHM, hParentItem, TVI_LAST );
				m_hSelItem = hChild;
				m_Tree.SetItemState( hChild, INDEXTOOVERLAYMASK( (int)eOverlayImage ), TVIS_OVERLAYMASK );
				m_Tree.SelectItem( hChild );
			}
			else
				hChild = m_Tree.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHM->GetHMName(), 
											CRCImageManager::ILPT_TerminalUnit, CRCImageManager::ILPT_TerminalUnit, 
											0, 0, (long)pHM, hParentItem, TVI_LAST );

			m_Tree.Expand( hChild, TVE_EXPAND );
		}
	}
	m_Tree.Expand( hParentItem, TVE_EXPAND );
}

void CDlgHMTree::SetVisible( bool fFlag )
{
	::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("Visible"), fFlag );
	if( true == fFlag )
		Create();
}

bool CDlgHMTree::IsVisible()
{
	return ( (::AfxGetApp()->GetProfileInt( m_strSectionName, _T("Visible"), 1 ) ) ? true : false );
}

void CDlgHMTree::InitTree( CDS_HydroMod *pHM, bool fEditMode )
{
	if( true == m_fInitRunning )
		return;

	m_fInitRunning = true;

	m_pTADS = TASApp.GetpTADS();
	m_pSelHM = pHM;
	m_fEditMode = fEditMode;

	FillHMTreeView( NULL, NULL );
	if( NULL != m_hSelItem )
	{
		m_Tree.SelectItem( m_hSelItem );
		HTREEITEM h = m_Tree.GetParentItem( m_hSelItem );
		if( NULL != h )
			m_Tree.EnsureVisible( m_Tree.GetParentItem( m_hSelItem ) );
		else
			m_Tree.EnsureVisible( m_hSelItem );
	}
	
 	// Reset the focus on SpreadSheet.
 	if( NULL != m_pParent )
 		( (CDlgWizCircuit*)m_pParent )->SetFocusToSSheet();

	m_fInitRunning = false;
}

BEGIN_MESSAGE_MAP( CDlgHMTree, CDialogExt )
	ON_WM_MOVE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY( TVN_SELCHANGING, IDC_TREE, OnTvnSelchangingTree )
END_MESSAGE_MAP()

void CDlgHMTree::DoDataExchange( CDataExchange* pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TREE, m_Tree );
}

BOOL CDlgHMTree::OnInitDialog()
{
	CDialogExt::OnInitDialog();
	m_hSelItem = NULL;
	m_pTADS = TASApp.GetpTADS();

	// Retrieve a copy of the 'Project tree' image list in 'm_pclTreeImageList'.
	m_pclTreeImageList = TASApp.GetpRCImageManager()->GetImageListCopy( CRCImageManager::ILN_ProjectTree );
	if( NULL == m_pclTreeImageList )
		return FALSE;
	m_pclTreeImageList->SetBkColor( CLR_NONE );
	m_pclTreeImageList->SetOverlayImage( CRCImageManager::ILPT_OverlayEditCircuit, OverlayMaskIndex::OMI_EditCircuit );
	m_pclTreeImageList->SetOverlayImage( CRCImageManager::ILPT_OverlayNewCircuit, OverlayMaskIndex::OMI_NewCircuit );
	m_Tree.SetImageList( m_pclTreeImageList, TVSIL_NORMAL );
	m_Tree.SetBkColor( GetSysColor( COLOR_3DFACE ) );
	
	// Set the window position to the last stored position in registry.
	// If window position is not yet stored in the registry, the window is centered by default in the mainframe rect.
	CRect apprect, rect;
	::AfxGetApp()->m_pMainWnd->GetWindowRect( &apprect );
	GetWindowRect( &rect );
	int x = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("ScreenXPos"), apprect.CenterPoint().x - rect.Width() / 2 );
	int y = ::AfxGetApp()->GetProfileInt( m_strSectionName, _T("ScreenYPos"), apprect.CenterPoint().y - rect.Height() / 2 );
	SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	// If the window is placed on a screen that is currently deactivated, the windows is centered to the application.
	HMONITOR hMonitor = NULL;
	GetWindowRect( &rect );
	hMonitor = MonitorFromRect( &rect, MONITOR_DEFAULTTONULL );
	if( NULL == hMonitor )
		CenterWindow();

	m_bInitialized = true;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgHMTree::OnMove( int x, int y )
{
	CDialogExt::OnMove( x, y );

	// Modeless case -- stores window position in registry.
	if( NULL != m_pParent && true == m_bInitialized )
	{
		CRect rect;
		GetWindowRect( &rect );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("ScreenXPos"), x );
		::AfxGetApp()->WriteProfileInt( m_strSectionName, _T("ScreenYPos"), y );
	}
}

void CDlgHMTree::OnClose()
{
	if( NULL != m_pParent )
	{
		// Modeless case -- do not call base class OnCancel.
		m_pParent->PostMessage( WM_USER_DESTROYDIALOGHMTREE );
	}
	CDialogExt::OnClose();
}

void CDlgHMTree::OnDestroy()
{
	m_bInitialized = false;
	m_pclTreeImageList->DeleteImageList();
	CDialogExt::OnDestroy();
}

void CDlgHMTree::OnTvnSelchangingTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>( pNMHDR );
	*pResult = false;
	if( pNMTreeView->itemNew.lParam != (LPARAM)m_pSelHM )
		*pResult = true;
}
