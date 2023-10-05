#include "stdafx.h"
#include "TASelect.h"
#include "DlgWizTAScope.h"
#include "DlgPanelTAScopeUpload.h"
#include "HydroMod.h"

CDlgPanelTAScopeUpload::CDlgPanelTAScopeUpload( CWnd* pParent )
	: CDlgWizard( CDlgPanelTAScopeUpload::IDD, pParent )
{
	m_pParent = NULL;
	m_Pathfn = _T("");
	m_PlantName = _T("");
	m_pPrjParam = NULL;
	m_nTimer = (UINT_PTR)0;
}

BEGIN_MESSAGE_MAP( CDlgPanelTAScopeUpload, CDlgWizard )
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED( IDC_BUTPREVIEW, OnBnClickedPreview )
	ON_NOTIFY( NM_CLICK, IDC_TADSTREE, OnNMClickTree )
	ON_NOTIFY( TVN_CBSTATECHANGED,IDC_TADSTREE, OnCbStateChanged )
END_MESSAGE_MAP()

void CDlgPanelTAScopeUpload::DoDataExchange( CDataExchange* pDX )
{
	CDlgWizard::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICVERSION, m_StaticVersionNbr );
	DDX_Control( pDX, IDC_BUTPREVIEW, m_ButtonPreview );
}

BOOL CDlgPanelTAScopeUpload::OnInitDialog()
{
	CDlgWizard::OnInitDialog();
	m_pParent = (CDlgWizTAScope *)GetWizMan();

	// Set the white rectangle to the first drawn window.
	GetDlgItem( IDC_STATICRECTANGLE )->BringWindowToTop();
	m_brWhiteBrush.CreateSolidBrush( RGB( 255, 255, 255 ) );

	// Strings.
	GetDlgItem( IDC_EDITTITLE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPLOAD_TITLE ) );
	GetDlgItem( IDC_STATICDETECTEDTASCOPE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELSTART_VERSIONDETECTED ) );
	GetDlgItem( IDC_STATICTITLEADSTREE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPLOAD_TITLETADSTREE ) );
	GetDlgItem( IDC_STATICTITLEADSXTREE )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPLOAD_TITLETADSXTREE ) );
	GetDlgItem( IDC_STATICINFOBLACKBOX )->SetWindowText( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPLOAD_BLACKBOXINFO ) );

	// Preview button.
	m_ButtonPreview.SetImage( IDB_PREVIEW_HYDRSUBSET_32, IDB_PREVIEW_HYDRSUBSET_32 );
	m_ButtonPreview.EnableFullTextTooltip( true );
   
	// Set the tooltip of the button.
	m_ButtonPreview.EnableFullTextTooltip( true );
	m_ButtonPreview.SetTooltip( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPLOAD_TOOLTIP_PREVIEWBUTTON ) );

	// TADS TREE.
	CRect sRect;
	CPoint sPoint( 0, 0 );
	GetDlgItem( IDC_FRAMETADS )->GetWindowRect( &sRect );
	ClientToScreen( &sPoint );
	sRect.left -= sPoint.x;
	sRect.right -= sPoint.x;
	sRect.top -= sPoint.y;
	sRect.bottom -= sPoint.y;

	DWORD uStyle = TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_FULLROWSELECT | TVS_CHECKBOXES;
	DWORD uExStyle = TVS_DISABLEDRAGDROP | TVS_EX_BITCHECKBOX | TVS_EX_SUBSELECT | TVS_EX_HIDEHEADERS;
	m_TreeTADS.CreateEx( WS_EX_CLIENTEDGE, uStyle | WS_CHILD | WS_VISIBLE | WS_TABSTOP, sRect, this, IDC_TADSTREE );
	m_TreeTADS.SetExtendedStyle( uExStyle );
	
	// User Data.
	m_TreeTADS.SetUserDataSize( sizeof( m_UserData ) );

	// Build and attach an image list to m_Tree.
	m_TreeImageList.CreateTC( IDB_IMGLST_HMTREE, 16, 16, _BLACK );
	m_TreeImageList.SetBkColor( CLR_NONE );
	m_TreeImageList.SetOverlayImage( etiOExclamation, etoExclamation );
	m_TreeImageList.SetOverlayImage( etiOCrossMark, etoCrossMark );
	m_TreeImageList.SetOverlayImage( etiOMissingPV, etoMissingPV );
	m_TreeImageList.SetOverlayImage( etiOWhiteCross, etoWhiteCross );
	m_TreeImageList.SetOverlayImage( etiOClosedBox, etoClosedBox );
	m_TreeTADS.SetImageList( &m_TreeImageList, TVSIL_NORMAL );
	
	// Check box image list.
	m_TreeCheckBox.Create( IDB_IMGLST_CHECKBOX1616, 16, 10, _BLACK );
	m_TreeCheckBox.SetBkColor( CLR_NONE );
	m_TreeTADS.SetImageList( &m_TreeCheckBox, TVSIL_STATE );
	m_TreeTADS.SetImageList( &m_TreeCheckBox, TVSIL_CHECK );

	m_TreeTADS.InsertColumn( 0, _T("") );
	m_TreeTADS.InsertColumn( 1, _T(""), TVCFMT_LEFT, 16 );

	// Tree TADSX.
	GetDlgItem( IDC_FRAMETADSX )->GetWindowRect( &sRect );
	sRect.left -= sPoint.x;
	sRect.right -= sPoint.x;
	sRect.top -= sPoint.y;
	sRect.bottom -= sPoint.y;

	uStyle = TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_FULLROWSELECT;
	uExStyle = TVS_DISABLEDRAGDROP | TVS_EX_SUBSELECT | TVS_EX_HIDEHEADERS;
	m_TreeTADSX.CreateEx( WS_EX_CLIENTEDGE, uStyle | WS_CHILD | WS_VISIBLE | WS_TABSTOP, sRect, this, IDC_TADSXTREE );
	m_TreeTADSX.SetExtendedStyle( uExStyle );

	m_TreeTADSX.SetImageList( &m_TreeImageList, TVSIL_NORMAL );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPanelTAScopeUpload::OnTimer( UINT_PTR nIDEvent )
{
	CDlgWizard::OnTimer( nIDEvent );

	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
	{
		return;
	}

	CString str, str1;
	m_StaticVersionNbr.GetWindowText( str );

	if( str != m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) )
	{
		m_StaticVersionNbr.SetWindowText( m_pParent->GetpArCurrentVersion()->GetAt( CDlgWizTAScope::eSoftwareVersion::eswvHH ) );
	}
}

HBRUSH CDlgPanelTAScopeUpload::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor )
{
	HBRUSH hbr;

	if( pWnd->m_hWnd == GetDlgItem( IDC_EDITTITLE )->m_hWnd )
	{
		pDC->SetTextColor( RGB( 0, 0, 0 ) );
		pDC->SetBkColor( RGB( 255, 255, 255 ) );
		return (HBRUSH)m_brWhiteBrush;
	}
	
	hbr = CDlgWizard::OnCtlColor( pDC, pWnd, nCtlColor );
	return hbr;
}

void CDlgPanelTAScopeUpload::OnBnClickedPreview()
{
	m_TADSX.Init();

	TASApp.GetpTADS()->SaveToTADataStructX( &m_TADSX );

	FillTADSXHNTree();
	m_pParent->ShowButtons( CWizardManager::WizButNext, true );
}

void CDlgPanelTAScopeUpload::OnNMClickTree( NMHDR* pNMHDR, LRESULT* pResult )
{
	// On a TreeView Control with the CheckBoxes, there is no notification that the check state
	// of the item has been changed, you can just determine that the user has clicked the state Icon
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is 
	// post a user defined message as a notification that the check state has changed

	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x= ( (int)(short)LOWORD( dwpos ) );		//GET_X_LPARAM(dwpos);
	ht.pt.y= ( (int)(short)HIWORD( dwpos ) );		//GET_Y_LPARAM(dwpos);

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( 0x01200000 == ht.flags )
	{
		// Clear all selection into the TADSXTree.
		m_TreeTADSX.DeleteAllItems();
		m_pParent->ShowButtons( CWizardManager::WizButNext, false );

		if( 1 == m_TreeTADS.GetFocusColumn() )
		{
			int iImage = m_TreeTADS.GetItemImageEx( ht.hItem, 1 );  	
			
			if( iImage > -1 )
			{
				iImage = ( iImage == etiOpenBox ) ? etiClosedBox : etiOpenBox;
				bool bFlag = ( iImage == etiClosedBox ) ? true : false;

				m_TreeTADS.SetItem( ht.hItem, 1, TVIF_IMAGE,_T(""), iImage, iImage, 0, 0 );
				m_UserData *pUD = (m_UserData *)m_TreeTADS.GetUserData( ht.hItem );

				if( NULL == pUD )
				{
					ASSERT( 0 );
				}
				else
				{
					pUD->bBlackBox = bFlag;
				}
				
				// Black box status into Hydromod circuit.
				CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData*)m_TreeTADS.GetItemData( ht.hItem ) );
				
				if( NULL != pHM )
				{
					unsigned char ucNewStatus = pHM->GetHMStatus();

					if( true == bFlag ) 
					{
						ucNewStatus |= CDS_HydroMod::eHMStatusFlags::sfBlackBox;
					}
					else
					{
						ucNewStatus &= ~CDS_HydroMod::eHMStatusFlags::sfBlackBox;
					}

					pHM->SetHMStatus( ucNewStatus );
				}

				SetBlackBoxModule( ht.hItem, bFlag );
			}
		}
	}

	if( NULL != pResult )
	{
		*pResult = 0;
	}
}

void CDlgPanelTAScopeUpload::OnCbStateChanged( NMHDR *pNmHdr, LRESULT *pResult )
{
	//The state of an check box was changed.
	NM_TREEVIEW *pNmTreeView = (NM_TREEVIEW *)pNmHdr;
	
	HTREEITEM hItem = pNmTreeView->itemNew.hItem;
	m_UserData *pUD = (m_UserData *)m_TreeTADS.GetUserData( hItem );
	
	if( NULL == pUD )
	{
		return; 
	}

	// Clear all selection into the TADSXTree.
	m_TreeTADSX.DeleteAllItems();
	m_pParent->ShowButtons( CWizardManager::WizButNext, false );

	// Verify image state, only etcbSquare can be Check/Uncheck.
	UINT uiState = ( m_TreeTADS.GetItemState( hItem, TVIS_STATEIMAGEMASK ) >> 12 );
	UINT iImage = uiState >> 1;
	
	if( iImage != etcbSquare )
	{
		// Return to previous state.
		if( 0x01 == ( uiState & 0x01 ) )
		{
			uiState &= ~0x01;
		}
		else
		{
			uiState |= 0x01;
		}

		m_TreeTADS.SetItemState( hItem, 0, INDEXTOSTATEIMAGEMASK( uiState ), TVIS_STATEIMAGEMASK );
		return;
	}

	bool bFlag = ( ( pNmTreeView->itemNew.state >> 12 ) & 0x01 ) ? true : false;

	CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)m_TreeTADS.GetItemData( pNmTreeView->itemNew.hItem ) );
	
	if( NULL != pHM && true == bFlag )
	{
		if( false == pHM->IsAvailableForTASCOPE() )
		{
			// Return to previous state.
			if( 0x01 == ( uiState & 0x01 ) )
			{
				uiState &= ~0x01;
			}
			else
			{
				uiState |= 0x01;
			}

			m_TreeTADS.SetItemState( hItem, 0, INDEXTOSTATEIMAGEMASK( uiState ), TVIS_STATEIMAGEMASK );
			return;
		}
	}

	pUD->bCheck = bFlag;
	
	// Store exportation status  into the HydroMod.
	if( NULL != pHM )
	{
		unsigned char ucNewStatus = pHM->GetHMStatus();

		if( true == bFlag )
		{
			ucNewStatus |= CDS_HydroMod::eHMStatusFlags::sfSelected;
		}
		else
		{
			ucNewStatus &= ~CDS_HydroMod::eHMStatusFlags::sfSelected;
		}

		pHM->SetHMStatus( ucNewStatus );
	}

	CheckAllChildren( hItem, bFlag );

	if( AtLeastOneChildrenSelect( m_TreeTADS.GetRootItem() ) )
	{
		m_ButtonPreview.EnableWindow( true );
	}
	else
	{
		m_ButtonPreview.EnableWindow( false );
	}

	if( NULL != pResult )
	{
		*pResult = 0;
	}
}

BOOL CDlgPanelTAScopeUpload::OnWizCancel()
{
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
	return TRUE;
}

BOOL CDlgPanelTAScopeUpload::OnWizNext()
{
	// Used to Upload Data into the TA-SCOPE.
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
	
	if( true == Upload() )
	{
		CDS_HMXTable *pTabHMX = dynamic_cast<CDS_HMXTable*>( m_TADSX.Get( _T("PIPING_TAB") ).MP );
		
		if( NULL != pTabHMX )
		{
			CDS_HMXTable::sHMXTab HMXTab;
			memset( &HMXTab, 0, sizeof( HMXTab ) );
			pTabHMX->CountBalancedValves( pTabHMX, &HMXTab );
			
			CString str, str1, str2, str3;
			str = TASApp.LoadLocalizedString( IDS_DLGPANELUPLOAD_UPLOADED );
			FormatString( str1, IDS_DLGPANELUPLOAD_PLANTNAME, m_PlantName );
			str += _T("\r\n") + str1;
			str2.Format( _T("%d"), HMXTab.HMXNbrMod );
			str3.Format( _T("%d"), HMXTab.HMXNbrValve );
			FormatString( str1, IDS_DLGPANELUPLOAD_MODULEVALVENBR, str2, str3 );
			str += _T("\r\n") + str1;
			
			m_pParent->m_DlgPanelStart.SetBtnInfoText( IDC_STATICINFOBTNUPLOAD, str );
		}
	}

	m_pParent->JumpToPanel( CDlgWizTAScope::epanStart );
	return FALSE;
}

BOOL CDlgPanelTAScopeUpload::OnWizFinish()
{
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
	m_pParent->JumpToPanel( CDlgWizTAScope::epanStart );
	return FALSE;
}

bool CDlgPanelTAScopeUpload::OnAfterActivate()
{
	try
	{
		m_pParent->m_DlgPanelStart.SetBtnInfoText( IDC_STATICINFOBTNUPLOAD, _T("") );
		m_PlantName = _T("");
		m_pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
		
		if( NULL == m_pPrjParam )
		{
			// HYSELECT_THROW( _T("Internal error: can't retrieve the 'GENERAL_PARAMS' object from the datastruct.") );
		}

		m_TreeTADSX.DeleteAllItems();
		FillTADSHNTree();
	
		// 'Next' and 'Finish' buttons.
		m_pParent->ShowButtons( CWizardManager::WizButNext | CWizardManager::WizButBack, false );
		m_StaticVersionNbr.SetWindowText( _T("") );
		m_MetaData.Init();

		BeginWaitCursor();

		// Prepare and load metadata file.
		m_pParent->ActivateRefresh( false );
		m_pParent->WaitMilliSec( 200 );

		bool fReturn = m_pParent->GetMetaData();
		m_pParent->ActivateRefresh( true );
	
		if( true == fReturn )
		{
			m_MetaData.Init();

			CString fn = CTAScopeUtil::GetTAScopeMaintenanceFolder() + _T("\\") + METAFILE;
			CFileStatus fs;

			// Check if the file exists and get the status.
			if( TRUE == CFile::GetStatus( fn, fs ) )
			{

				std::ifstream inpf( fn, std::ifstream::in | std::ifstream::binary );
				m_MetaData.Read( inpf );

				// Just refreshing the connection status.
				m_nTimer = SetTimer( _TIMERID_DLGPANELTASCOPEUPLOAD, 1000, 0 );
			}
		}

		// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
		m_TreeTADSX.SetFocus();

		EndWaitCursor();
		return fReturn;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgPanelTAScopeUpload::OnAfterActivate'."), __LINE__, __FILE__ );
		throw;
	}
}

int CDlgPanelTAScopeUpload::OnWizButFinishTextID()
{
	return IDS_DLGPANELUPLOAD_CANCEL;
}

int CDlgPanelTAScopeUpload::OnWizButNextTextID()
{
	return IDS_DLGPANELUPLOAD_UPLOAD;
}

bool CDlgPanelTAScopeUpload::AtLeastOneChildrenSelect( HTREEITEM hItem )
{
	if( TRUE == m_TreeTADS.ItemHasChildren( hItem ) )
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = m_TreeTADS.GetChildItem( hItem );

		while( hChildItem != NULL )
		{
			hNextItem = m_TreeTADS.GetNextItem( hChildItem, TVGN_NEXT );
			m_UserData * pUD = (m_UserData *)m_TreeTADS.GetUserData( hChildItem );
			
			if( NULL != pUD && true == pUD->bCheck )
			{
				return true;
			}

			if( TRUE == m_TreeTADS.ItemHasChildren( hChildItem ) )
			{
				if( true == AtLeastOneChildrenSelect( hChildItem ) )
				{
					return true;
				}
			}

			hChildItem = hNextItem;
		}
	}

	return false;
}

void CDlgPanelTAScopeUpload::CheckAllChildren( HTREEITEM hItem, bool bFlag )
{
	if( TRUE == m_TreeTADS.ItemHasChildren( hItem ) )
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = m_TreeTADS.GetChildItem( hItem );

		while( hChildItem != NULL )
		{
			hNextItem = m_TreeTADS.GetNextItem( hChildItem, TVGN_NEXT );
			UINT uiState = ( m_TreeTADS.GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) >> 12 );
			CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData*)m_TreeTADS.GetItemData( hChildItem ) );
			
			// If the valve is not available for TAScope clear the selection flag...
			if( NULL != pHM )
			{
				bFlag &= pHM->IsAvailableForTASCOPE();
			}
			
			if( true == bFlag )
			{
				uiState |= 0x01;
			}
			else
			{
				uiState &= ~0x01;
			}

			UINT iImage = uiState >> 1;
			
			if( etcbSquare == iImage || etcbSquareGray == iImage )
			{
				if( true == bFlag )
				{	
					m_TreeTADS.SetItemState( hChildItem, 0, INDEXTOSTATEIMAGEMASK( etcbSquareGray * 2 + 1 ), TVIS_STATEIMAGEMASK );
				}
				else
				{	
					m_TreeTADS.SetItemState( hChildItem, 0, INDEXTOSTATEIMAGEMASK( etcbSquare * 2 ), TVIS_STATEIMAGEMASK );
				}
			}

			m_UserData * pUD = (m_UserData *)m_TreeTADS.GetUserData( hChildItem );
			
			if( NULL != pUD )
			{
				pUD->bCheck = bFlag;
			}
			
			// Store exportation status  into the HydroMod.
			if( NULL != pHM )
			{
				unsigned char ucNewStatus = pHM->GetHMStatus();

				if( true == bFlag )
				{
					ucNewStatus |= CDS_HydroMod::eHMStatusFlags::sfSelected;
				}
				else
				{
					ucNewStatus &= ~CDS_HydroMod::eHMStatusFlags::sfSelected;
				}

				pHM->SetHMStatus( ucNewStatus );
			}
			
			if( TRUE == m_TreeTADS.ItemHasChildren( hChildItem ) )
			{
				CheckAllChildren( hChildItem, bFlag );
			}

			hChildItem = hNextItem;
		}
	}
}

void CDlgPanelTAScopeUpload::SetBlackBoxModule( HTREEITEM hItem, bool bFlag )
{
	if( TRUE == m_TreeTADS.ItemHasChildren( hItem ) )
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = m_TreeTADS.GetChildItem( hItem );

		while( hChildItem != NULL )
		{
			hNextItem = m_TreeTADS.GetNextItem( hChildItem, TVGN_NEXT );
			m_TreeTADS.SetItemTextColor( hChildItem, 0, ( true == bFlag ) ? RGB( 192, 192, 192 ) : TV_NOCOLOR );
			
			m_UserData *pUD = (m_UserData *)m_TreeTADS.GetUserData( hChildItem );
			
			if( NULL == pUD )
			{
				ASSERT( 0 );
				continue;
			}
			
			pUD->bIncludedintoBB = bFlag;
			
			// In case of module change the image to gray square.
			CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData*)m_TreeTADS.GetItemData( hChildItem ) );
			
			if( NULL != pHM )
			{
				if( true == pHM->IsaModule() )
				{
					if( true == bFlag )
					{
						// Replace Check box by a Grey ones.
						m_TreeTADS.SetItemState( hChildItem, 0, INDEXTOSTATEIMAGEMASK( etcbSquareGray * 2 + 1 ), TVIS_STATEIMAGEMASK );
						// Close the box.
						m_TreeTADS.SetItem( hChildItem, 1, TVIF_IMAGE, _T(""), etiClosedBox, etiClosedBox, 0, 0 );
					}
					else
					{
						if( true == pUD->bCheck )
						{
							m_TreeTADS.SetItemState( hChildItem, 0, INDEXTOSTATEIMAGEMASK( etcbSquare * 2 + 1 ), TVIS_STATEIMAGEMASK );
						}
						else
						{
							m_TreeTADS.SetItemState( hChildItem, 0, INDEXTOSTATEIMAGEMASK( etcbSquare * 2 ), TVIS_STATEIMAGEMASK );
						}
						
						// Open the box.
						m_TreeTADS.SetItem( hChildItem, 1, TVIF_IMAGE, _T(""), etiOpenBox, etiOpenBox, 0, 0 );
					}
				}

				// Store status into the HydroMod.
				unsigned char ucNewStatus = pHM->GetHMStatus();
				
				if( true == bFlag ) 
				{
					ucNewStatus |= CDS_HydroMod::eHMStatusFlags::sfIncludedIntoBlackBox;
				}
				else
				{
					ucNewStatus &= ~CDS_HydroMod::eHMStatusFlags::sfIncludedIntoBlackBox;
				}

				pHM->SetHMStatus( ucNewStatus );
			}
			
			if( TRUE == m_TreeTADS.ItemHasChildren( hChildItem ) )
			{
				SetBlackBoxModule( hChildItem, bFlag );
			}

			hChildItem = hNextItem;
		}
	}
}

bool CDlgPanelTAScopeUpload::FillTADSTreeNode( HTREEITEM hPItem, CTable *pTab )
{
	if( NULL == hPItem || NULL == pTab )
	{
		return false;
	}

	// Sort items.
	std::map<int, CDS_HydroMod *>mapHM;
	std::map<int, CDS_HydroMod *>::iterator mapIt;
	
	for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( (CData *)IDPtr.MP );

		if( NULL == pHM )
		{
			continue;
		}

		mapHM.insert( std::pair<int, CDS_HydroMod *>( pHM->GetPos(), pHM ) );
	}
	
	for( mapIt = mapHM.begin(); mapIt != mapHM.end(); mapIt++ )
	{
		CDS_HydroMod *pHM = (*mapIt).second;
		
		if( NULL == pHM ) 
		{
			ASSERT( 0 );
			continue;
		}
		
		// Reset module status.
		pHM->SetHMStatus( 0 );
		
		// SetUID.
		if( 0 == pHM->GetUid() )
		{
			pHM->SetUid( m_pPrjParam->GetNextUid() );
		}
		
		// If module...
		if( true == pHM->IsaModule() )
		{
			HTREEITEM hItem = m_TreeTADS.InsertItem( (LPCTSTR)pHM->GetHMName(), etiHMmodule, etiHMmoduleS, hPItem );
			m_TreeTADS.SetItem( hItem, 0, TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHM->GetHMName(), etiHMmodule, etiHMmoduleS, 0, 0, (LPARAM)pHM );
			
			if( true == pHM->IsAvailableForTASCOPE() )
			{
				// Black box status -> Open Box image.
				m_TreeTADS.SetItem( hItem, 1, TVIF_IMAGE, _T(""), etiOpenBox, etiOpenBox, 0, 0 );
				
				// Test if PV exist.
				if ( !( ( NULL != pHM->GetpBv() && NULL != pHM->GetpBv()->GetIDPtr().MP ) ||
					  ( NULL != pHM->GetpCV() && true == pHM->GetpCV()->IsPresettable() && NULL != pHM->GetpCV()->GetCvIDPtr().MP ) ) )
				{
					if ( edt_KvCv != pHM->GetVDescrType() )
					{
						m_TreeTADS.SetItemState( hItem, INDEXTOOVERLAYMASK( etoMissingPV ), TVIS_OVERLAYMASK );
					}
				}

				m_TreeTADS.SetItemState( hItem, 0, INDEXTOSTATEIMAGEMASK( etcbSquare * 2) + 1, TVIS_STATEIMAGEMASK );
			}
			else 
			{
				m_TreeTADS.SetItemState( hItem, INDEXTOOVERLAYMASK( etoCrossMark ), TVIS_OVERLAYMASK );
				m_TreeTADS.SetItemState( hItem, 0, INDEXTOSTATEIMAGEMASK( etcbSquareGray * 2 ) + 1, TVIS_STATEIMAGEMASK );
			}
			
			m_UserData *pUD = (m_UserData *)m_TreeTADS.GetUserData( hItem );
			
			if( NULL != pUD )
			{
				pUD->bBlackBox = false;
				pUD->bIncludedintoBB = false;
				pUD->bCheck=false;
			}

			FillTADSTreeNode( hItem, pHM );
			m_TreeTADS.Expand( hItem, TVE_EXPAND );
		}
		else				// Terminal unit
		{
			HTREEITEM hItem = m_TreeTADS.InsertItem( (LPCTSTR)pHM->GetHMName(), etiHMTU, etiHMTUS, hPItem );
			m_TreeTADS.SetItem( hItem, 0, TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHM->GetHMName(), etiHMTU, etiHMTUS, 0, 0, (LPARAM)pHM );
			m_TreeTADS.SetItemState(hItem,0,INDEXTOSTATEIMAGEMASK(etcbDot*2+1),TVIS_STATEIMAGEMASK);
			
			if( false == pHM->IsAvailableForTASCOPE() )
			{
				m_TreeTADS.SetItemState( hItem, INDEXTOOVERLAYMASK( etoCrossMark ), TVIS_OVERLAYMASK );
			}
			
			m_UserData *pUD = (m_UserData *)m_TreeTADS.GetUserData( hItem );
			
			pUD->bBlackBox = false;
			pUD->bIncludedintoBB = false;
			pUD->bCheck=false;
		}
	}

	return true;
}

void CDlgPanelTAScopeUpload::FillTADSHNTree()
{
	m_ButtonPreview.EnableWindow( FALSE );
	m_TreeTADS.DeleteAllItems();
	
	// Main structure.
	// TODO HUB table
	CArray<CString> arTabID;
	arTabID.Add( _T("PIPING_TAB") );

	for( int i = 0; i < arTabID.GetCount(); i++ )
	{
		CTable *pTab = dynamic_cast<CTable *>( TASApp.GetpTADS()->Get( (LPCTSTR)arTabID[i]).MP );

		if( NULL == pTab )
		{
			continue;
		}
		
		CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
		ASSERT( NULL != pPrjRef );

		if( NULL == pPrjRef )
		{
			return;
		}

		HTREEITEM hItem = m_TreeTADS.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT|TVIF_PARAM,
												(LPCTSTR)pPrjRef->GetString( CDS_ProjectRef::Name ), etiNetwork, etiNetworkS, 0, 0,
												(LPARAM)pTab, TVI_ROOT,TVI_ROOT );

		m_TreeTADS.SetItemState( hItem, 0, INDEXTOSTATEIMAGEMASK( etcbDot * 2 + 1 ),TVIS_STATEIMAGEMASK );
		m_UserData *pUD = (m_UserData *)m_TreeTADS.GetUserData( hItem );
		
		if( NULL != pUD )
		{
			pUD->bIncludedintoBB = false;
			pUD->bBlackBox = false;
			pUD->bCheck=false;
		}
		
		FillTADSTreeNode( hItem, pTab );
		m_TreeTADS.Expand( hItem, TVE_EXPAND );
	}
}

bool CDlgPanelTAScopeUpload::FillTADSXTreeNode( HTREEITEM hPItem, CTable *pTab )
{
	if( NULL == hPItem || NULL == pTab)
	{
		return false;
	}

	// Sort items.
	std::map<int,CDS_HydroModX *>mapHM;
	std::map<int,CDS_HydroModX *>::iterator mapIt;
	
	for( IDPTR IDPtr = pTab->GetFirst(); IDPtr.MP; IDPtr = pTab->GetNext() )
	{
		CDS_HydroModX *pHMX = dynamic_cast<CDS_HydroModX *>( (CData *)IDPtr.MP );

		if( NULL == pHMX )
		{
			continue;
		}

		mapHM.insert(std::pair<int, CDS_HydroModX *>( pHMX->GetPosition(), pHMX ) );
	}
	
	for( mapIt = mapHM.begin() ; mapIt != mapHM.end(); mapIt++ )
	{
		CDS_HydroModX *pHMX = (*mapIt).second;

		if( NULL == pHMX )
		{
			ASSERT( 0 );
			continue;
		}
		
		// If module...
		if( true == pHMX->IsaModule() )
		{
			HTREEITEM hItem = m_TreeTADSX.InsertItem( (LPCTSTR)pHMX->GetHMName(), etiHMmodule, etiHMmoduleS, hPItem );
			m_TreeTADSX.SetItem( hItem, 0, TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHMX->GetHMName()->c_str(), etiHMmodule, etiHMmoduleS, 0, 0, (LPARAM)pHMX );
			
			if( true == pHMX->HasHMXStatus( CDS_HydroModX::eHMStatusFlags::sfBlackBox ) )
			{
				m_TreeTADSX.SetItemState( hItem, INDEXTOOVERLAYMASK( etoClosedBox ), TVIS_OVERLAYMASK );
			}

			FillTADSXTreeNode( hItem, pHMX );
			m_TreeTADSX.Expand( hItem, TVE_EXPAND );
		}
		else				// Terminal unit
		{
			HTREEITEM hItem = m_TreeTADSX.InsertItem( (LPCTSTR)pHMX->GetHMName(), etiHMTU, etiHMTUS, hPItem );
			m_TreeTADSX.SetItem( hItem, 0, TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)pHMX->GetHMName()->c_str(), etiHMTU, etiHMTUS, 0, 0, (LPARAM)pHMX );
		}
	}
	return true;
}

void CDlgPanelTAScopeUpload::FillTADSXHNTree()
{
	m_TreeTADSX.DeleteAllItems();
	
	// Main structure.
	// TODO HUB table for TASCOPE !!!!
	CArray<CString> arTabID;
	arTabID.Add( _T("PIPING_TAB") );
	
	for( int i = 0; i < arTabID.GetCount(); i++ )
	{
		CTable *pTab = dynamic_cast<CTable *>( m_TADSX.Get( (LPCTSTR)arTabID[i] ).MP );

		if( NULL == pTab )
		{
			continue;
		}
		
		CDS_PlantX *pPlant = dynamic_cast<CDS_PlantX *>( m_TADSX.Get( _T("PLANT_INFO") ).MP );
		ASSERT( NULL != pPlant );

		if( NULL == pPlant )
		{
			return;
		}

		HTREEITEM hItem = m_TreeTADSX.InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
												(LPCTSTR)pPlant->GetString( CDS_PlantX::ePlant::Name ), etiNetwork, etiNetworkS, 0, 0,
												(LPARAM)pTab,TVI_ROOT, TVI_ROOT );

		FillTADSXTreeNode( hItem, pTab );
		m_TreeTADSX.Expand( hItem, TVE_EXPAND );
	}
}

bool CDlgPanelTAScopeUpload::Upload()
{
	bool fFoundUID = false;
	bool fFoundName = false;
	CDS_PlantX *pPlant = NULL;
	CDS_PlantX *pProjectPlant = dynamic_cast<CDS_PlantX *>( m_TADSX.Get( _T("PLANT_INFO") ).MP );
	ASSERT( NULL != pProjectPlant );
	
	if( NULL == pProjectPlant )
	{
		return false;
	}

	CString str1, str2;
	
	for( IDPTR IDPtr = m_MetaData.Access().GetFirst(); IDPtr.MP && false == fFoundUID && false == fFoundName; IDPtr = m_MetaData.Access().GetNext() )
	{
		pPlant = dynamic_cast<CDS_PlantX*>( IDPtr.MP );
		
		if( NULL != pPlant )
		{
			str1 = pPlant->GetString( CDS_PlantX::ProjectUID );
			str2 = pProjectPlant->GetString( CDS_PlantX::ProjectUID );
			
			// Test UID.
			if( str1 == str2 )
			{
				fFoundUID = true;
			}

			// Check project name.
			str1 = pPlant->GetString( CDS_PlantX::Name );
			str2 = pProjectPlant->GetString( CDS_PlantX::Name );
			
			if( str1 == str2 )
			{
				fFoundName = true;
			}
		}	
	}

	if( NULL != pPlant )
	{
		if( true == fFoundUID )
		{
			if( true == fFoundName )
			{
				// "The current project already exist into the TA-SCOPE.\r\nDo you agree to overwrite it?".
				if( IDOK == ::AfxMessageBox( TASApp.LoadLocalizedString( IDS_TASCOPE_PANELUPLOAD_PLANTUIDEXIST ), MB_OKCANCEL | MB_ICONEXCLAMATION ) )
				{
					m_pParent->ActivateRefresh( false );
					m_pParent->DeleteTASCOPEFile( pPlant->GetTSCFilename() );
					// We don't need anymore to check name.
					fFoundName = false;
				}
				else
				{
					return false;
				}
			}
			else
			{
				// Two different projects with the same UID.
				//"The current project is labeled with an unique identifier that is already used by project %1 loaded in the TA-SCOPE.\r\nDo you agree to update the unique identifier of the current project?".
				CString str;
				FormatString( str, IDS_TASCOPE_PANELUPLOAD_DIFPLANTWITHSAMEUIDEXIST, pPlant->GetString( CDS_PlantX::Name ) );
				
				if( IDOK == ::AfxMessageBox( str, MB_OKCANCEL | MB_ICONEXCLAMATION ) )
				{
					TASApp.GetpTADS()->SetUID( (LPCTSTR)GetTimeBasedUniqID() );
				}
				else
				{
					return false;
				}
			}
		}
		
		if( true == fFoundName )
		{
			// Try to find an alternative name.
			CString str, strNewName;
			int i = 1;

			while( true == fFoundName )
			{
				fFoundName = false;
				strNewName.Format( _T("%s-%d"), pProjectPlant->GetString( CDS_PlantX::Name ), i );
				
				for( IDPTR IDPtr = m_MetaData.Access().GetFirst(); IDPtr.MP && false == fFoundName; IDPtr = m_MetaData.Access().GetNext() )
				{
					pPlant = dynamic_cast<CDS_PlantX*>( IDPtr.MP );
					
					if( NULL != pPlant )
					{
						// Check project name.
						CString str1 = pPlant->GetString( CDS_PlantX::Name );
						
						if( str1 == strNewName )
						{
							fFoundName = true;
						}
					}	
				}

				i++;
			}
			
			//"Another installation named '%1' is already loaded in the TA-SCOPE. The present installation will be renamed '%2' in order to be uploaded to the TA-SCOPE.".
			FormatString( str, IDS_TASCOPE_PANELUPLOAD_PLANTNAMEEXIST, pProjectPlant->GetString( CDS_PlantX::Name ), strNewName );
			
			if( IDOK == ::AfxMessageBox( str, MB_OKCANCEL | MB_ICONEXCLAMATION ) )
			{
				pProjectPlant->SetString( CDS_PlantX::Name, (LPCTSTR)strNewName );
				CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
				ASSERT( NULL != pPrjRef );

				if( NULL != pPrjRef )
				{
					pPrjRef->SetString( CDS_ProjectRef::Name, strNewName);
				}
			}
			else
			{
				return false;
			}
		}
	}

	m_PlantName = pProjectPlant->GetString( CDS_PlantX::Name );
	
	// Write the file TADSX file....
	time_t t;
	tm tm;
	t = time( NULL );
	gmtime_s( &tm, &t );
	CString fn;
	fn.Format( _T("%02d%02d%02d-%02d%02d%02d.tsc"), tm.tm_year - 100, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec );
	m_Pathfn = CTAScopeUtil::GetTAScopeDataFolder() + _T("\\") + fn; 
	std::ofstream outf( m_Pathfn, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
	ASSERT( !( outf.rdstate() & std::ofstream::failbit ) );

	if( NULL != pProjectPlant )
	{
		pProjectPlant->SetLastModDate( (__time32_t)t );
		CStringA fnA = CStringA( fn );
		pProjectPlant->SetTSCFileName( (char *)(LPCSTR)fnA );
	}

	m_TADSX.Write( outf );
	outf.close();

	if ( true == m_pParent->IsComForcedOffLine() )
	{
		AfxMessageBox(L"File saved into Document TAScope folder");	
		return true;
	}

	// TODO Check space needed versus space available
	// Adding CRC
	m_pParent->ActivateRefresh( false );

	ServiceManager::eComStatus ecsRet = ServiceManager::service_F_ADD_CRC( m_Pathfn );
	
	// Writing file into the TASCOPE in secure mode, with progress bar.
	bool fSuccess = true;

	if( false == m_pParent->WriteFile( m_Pathfn, true, true ) )
	{
		fSuccess = false;
	}

	m_pParent->ActivateRefresh( true );

	//TODO how to manage transfer success or fail

	// Remove local file!!!!
	RemoveFile( m_Pathfn );
	
	return true;
}
