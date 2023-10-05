#include "stdafx.h"
#include "TASelect.h"
#include "hydromod.h"
#include "HMTreeListCtrl.h"

CHMTreeListCtrl::CHMTreeListCtrl() : CTreeListCtrl()
{
	m_bAllhItemDragDrop = true;
	m_CTreeDropTarget.SetProperDragDrop( false );
	m_hItemToExpand = NULL;
	m_nTimer = (UINT_PTR)0;
}

bool CHMTreeListCtrl::FillHMNode( HTREEITEM hPItem, CTable *pTab, bool fOnlyModule )
{
	if( NULL == pTab )
	{
		return false;
	}

	std::map<int,CDS_HydroMod *>mapHM;
	std::map<int,CDS_HydroMod *>::iterator mapIt;

	for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );

		if( NULL == pHM )
		{
			continue;
		}

		mapHM.insert( std::pair<int, CDS_HydroMod *>( pHM->GetPos(), pHM ) );
	}

	for( mapIt = mapHM.begin() ; mapIt != mapHM.end(); mapIt++ )
	{
		CDS_HydroMod *pHM = (*mapIt).second;

		if( NULL == pHM )
		{
			ASSERT_CONTINUE;
		}
		
		CString str = pHM->GetHMName();

		if( false == pHM->GetDescription().IsEmpty() )
		{
			str += CString( _T(" - ") ) + pHM->GetDescription();
		}
		
		if( true == pHM->IsaModule() )
		{
			HTREEITEM hItem = InsertItem( (LPCTSTR)str, CRCImageManager::ILHM_HydronicCircuit, CRCImageManager::ILHM_HydronicCircuitSelected, hPItem );
			SetItem( hItem, 0, TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, CRCImageManager::ILHM_HydronicCircuit, CRCImageManager::ILHM_HydronicCircuitSelected, 0, 0, (LPARAM)pHM );
			FillHMNode( hItem, pHM, fOnlyModule );
			Expand( hItem, TVE_EXPAND );
			
			// Complete UserData information.
			m_UserData *pUD = (m_UserData *)GetUserData( hItem );
			pUD->bImported = false;
			pUD->pHM = pHM;
		}
		else if( false == fOnlyModule )
		{
			HTREEITEM hItem = InsertItem( (LPCTSTR)str, CRCImageManager::ILHM_TerminalUnit, CRCImageManager::ILHM_TerminalUnitSelected, hPItem );
			SetItem( hItem, 0, TVIF_IMAGE | TVIF_TEXT | TVIF_PARAM, (LPCTSTR)str, CRCImageManager::ILHM_TerminalUnit, CRCImageManager::ILHM_TerminalUnitSelected, 0, 0, (LPARAM)pHM );
			
			// Complete UserData information.
			m_UserData *pUD = (m_UserData *)GetUserData( hItem );
			pUD->bImported = false;
			pUD->pHM = pHM;
		}
	}

	return true;
}

bool CHMTreeListCtrl::FillHMRoot( CTable *pTab, CString RootName, bool fOnlyModule )
{
	DeleteAllItems();

	if( NULL == pTab )
	{
		ASSERTA_RETURN( false );
	}

	HTREEITEM hItem = InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
								 (LPCTSTR)RootName, CRCImageManager::ImgListHMTree::ILHM_HydronicNetwork, CRCImageManager::ILHM_HydronicNetworkSelected, 0, 0, (LPARAM)pTab, TVI_ROOT, TVI_ROOT );
	
	FillHMNode( hItem, pTab, fOnlyModule );

	// Complete UserData information.
	m_UserData *pUD = (m_UserData *)GetUserData( hItem );
	pUD->bImported = false;
	pUD->pHM = (CDS_HydroMod *)pTab;

	return true;
}

void CHMTreeListCtrl::SetProperDragDrop( bool fDrag ) 
{ 
	m_CTreeDropTarget.SetProperDragDrop( fDrag );
}

HTREEITEM CHMTreeListCtrl::AddItem( HTREEITEM hParent, CString csItemName, HTREEITEM hInsAfter, int iSelImage, int iNonSelImage, long lParam, m_UserData *pUDSource, DROPEFFECT dropEffect )
{
	HTREEITEM hItem;
	TV_ITEM tvItem;
	TV_INSERTSTRUCT tvIns;

	tvItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	int nLength = csItemName.GetLength() ;
	tvItem.pszText = csItemName.GetBuffer( nLength );
	tvItem.cchTextMax = nLength;
	tvItem.lParam = lParam;

	tvItem.iImage = iNonSelImage;
	tvItem.iSelectedImage = iSelImage;
	
	tvIns.item = tvItem;
	tvIns.hInsertAfter = hInsAfter;
	tvIns.hParent = hParent;
	
	hItem = InsertItem( &tvIns );

	// Display in Red added hItem.
	SetItemTextColor( hItem, 0, _RED );

	// Specify the m_UserData.
	m_UserData *pUD = (m_UserData *)GetUserData( hItem );
	pUD->bImported = true;
	pUD->pHM = (CDS_HydroMod *)lParam;
	pUD->bUnused1 = pUDSource->bUnused1;
	pUD->bUnused2 = pUDSource->bUnused2;
	pUD->bUnused3 = pUDSource->bUnused3;
	pUD->bUnused4 = pUDSource->bUnused4;
	pUD->bUnused5 = pUDSource->bUnused5;
	pUD->bUnused6 = pUDSource->bUnused6;
	pUD->bUnused7 = pUDSource->bUnused7;

	// HYS-1716: to easily find the HTREEITEM in the destination tree with a CDS_HydroMod pointer returned by the 
	// 'IsInjectionCircuitTemperatureErrorForPasteOperation' method in the vector that contains circuis in error, for each
	// item added in the tree we save the corresponding HTREEITEM in the position of the 'CDS_HydroMod'. 
	// It's not a problem because we don't need the position.
	pUD->pHM->SetPos( -(int)hItem );

	// Notify parent an hItem has been inserted. Specify if it's a copy or a move.
	GetParent()->SendMessage( WM_USER_HMTREEITEMINSERTED, (WPARAM)dropEffect, (LPARAM)hItem );
	
	// Notify parent the tree has been modified.
	NotifyToParentTreeModified();
	
	return hItem;
}

BOOL CHMTreeListCtrl::DeleteItem( _In_ HTREEITEM hItem )
{
	BOOL Bret = CTreeListCtrl::DeleteItem( hItem );
	
	// Notify parent the tree has been modified.
	NotifyToParentTreeModified();
	
	return Bret;
}

void CHMTreeListCtrl::ExpandRecursive( HTREEITEM hItem, UINT nCode )
{
	// Do the Expand function to the current hItem.
	Expand( hItem, nCode );

	// Do the Expand function recursively.
	for( hItem = GetChildItem( hItem ); hItem; hItem = GetNextSiblingItem( hItem ) )
	{
		Expand( hItem, nCode );
		
		// Do the recursive function.
		if( GetChildItem( hItem ) )
		{
			ExpandRecursive( hItem, nCode );
		}
	}
}

void CHMTreeListCtrl::NotifyToParentTreeModified()
{
	CWnd *pWndParent = GetParent();

	if( NULL != pWndParent )
	{
		pWndParent->SendMessage( WM_USER_HMTREEMODIFIED );
	}
}

short CHMTreeListCtrl::GetTreeDeep( bool fVisible )
{
	short nCurrentDeep = 0;
	short nTreeDeep = 0;

	std::vector<HTREEITEM> vecTreeList;
	HTREEITEM hRoot = GetRootItem( );

	if( NULL == hRoot )
	{
		return -1;
	}

	HTREEITEM hNextItem = hRoot;
	bool bCanStop = false;

	do 
	{
		if( NULL == hNextItem )
		{
			if( vecTreeList.size() > 0 )
			{
				nCurrentDeep--;
				hNextItem = vecTreeList.back();
				hNextItem = GetNextSiblingItem( hNextItem );
				vecTreeList.pop_back();
			}
			else
			{
				bCanStop = true;
			}
		}
		else if( NULL != GetChildItem( hNextItem ) )
		{
			// Verify if we must take into account only visible items.
			if( true == fVisible && 0 == GetItemState( hNextItem, TVIS_EXPANDED ) )
			{
				hNextItem = GetNextSiblingItem( hNextItem );
			}
			else
			{
				nCurrentDeep++;

				if( nCurrentDeep > nTreeDeep )
				{
					nTreeDeep = nCurrentDeep;
				}

				vecTreeList.push_back( hNextItem );
				hNextItem = GetChildItem( hNextItem );
			}
		}
		else
		{
			hNextItem = GetNextSiblingItem( hNextItem );
		}

	}while( false == bCanStop );

	return nTreeDeep;
}

void CHMTreeListCtrl::SetCheckByLevel( int iLevel, bool fCheck, bool fClearAfter )
{
	std::vector<HTREEITEM> vecTreeList;
	HTREEITEM hRoot = GetRootItem( );

	if( NULL == hRoot )
	{
		return;
	}

	int iCurrentDeep = 0;
	HTREEITEM hNextItem = hRoot;
	bool bCanStop = false;

	do 
	{
		if( NULL != hNextItem )
		{
			if( iCurrentDeep <= iLevel )
			{
				SetItemState( hNextItem, 0, INDEXTOSTATEIMAGEMASK( ( true == fCheck ) ? 1 : 0 ), TVIS_STATEIMAGEMASK );
			}
			else if( true == fClearAfter )
			{
				SetItemState( hNextItem, 0, INDEXTOSTATEIMAGEMASK( 0 ), TVIS_STATEIMAGEMASK );
			}
		}

		if( NULL == hNextItem )
		{
			if( vecTreeList.size() > 0 )
			{
				iCurrentDeep--;
				hNextItem = vecTreeList.back();
				hNextItem = GetNextSiblingItem( hNextItem );
				vecTreeList.pop_back();
			}
			else
			{
				bCanStop = true;
			}
		}
		else if( NULL != GetChildItem( hNextItem ) )
		{
			iCurrentDeep++;
			vecTreeList.push_back( hNextItem );
			hNextItem = GetChildItem( hNextItem );
		}
		else
		{
			hNextItem = GetNextSiblingItem( hNextItem );
		}

	}while( false == bCanStop );
}

void CHMTreeListCtrl::SetCheckSpecial( HTREEITEM hItem, bool fDoParents, bool fDoChildren, bool fCheckParents, bool fCheckChildren )
{
	if( NULL == hItem )
	{
		return;
	}

	// If we must check/uncheck all parents...
	if( true == fDoParents )
	{
		HTREEITEM hNextItem = hItem;

		while( ( hNextItem = GetParentItem( hNextItem ) ) != NULL )
		{
			SetItemState( hNextItem, 0, INDEXTOSTATEIMAGEMASK( ( true == fCheckParents ) ? 1 : 0 ), TVIS_STATEIMAGEMASK );
		}
	}

	// If we must check/uncheck all children...
	if( true == fDoChildren )
	{
		std::vector<HTREEITEM> vecTreeList;
		int iCurrentDeep = 0;
		bool bCanStop = false;
		HTREEITEM hNextItem = GetChildItem( hItem );
		
		do 
		{
			// If item exist...
			if( NULL != hNextItem )
			{
				SetItemState( hNextItem, 0, INDEXTOSTATEIMAGEMASK( ( true == fCheckChildren ) ? 1 : 0 ), TVIS_STATEIMAGEMASK );
			}

			// If there is no more item at the current level...
			if( NULL == hNextItem )
			{
				// If there is a previous level...
				if( vecTreeList.size() > 0 )
				{
					iCurrentDeep--;
					hNextItem = vecTreeList.back();
					hNextItem = GetNextSiblingItem( hNextItem );
					vecTreeList.pop_back();
				}
				else
				{
					bCanStop = true;
				}
			}
			else if( NULL != GetChildItem( hNextItem ) )
			{
				// Current item has a child.
				iCurrentDeep++;
				vecTreeList.push_back( hNextItem );
				hNextItem = GetChildItem( hNextItem );
			}
			else
			{
				hNextItem = GetNextSiblingItem( hNextItem );
			}

		}while( false == bCanStop );
	}
}

bool CHMTreeListCtrl::IsAtLeastOneChecked( void )
{
	std::vector<HTREEITEM> vecTreeList;
	HTREEITEM hRoot = GetRootItem( );

	if( NULL == hRoot )
	{
		return false;
	}

	int iCurrentDeep = 0;
	HTREEITEM hNextItem = hRoot;
	bool bAtLeastOneChecked = false;
	bool bCanStop = false;

	do 
	{
		if( NULL != hNextItem )
		{
			// Remark: 0x1000 hard coded in 'TreeListWnd'.
			if( 0x1000 == GetItemState( hNextItem, TVIS_STATEIMAGEMASK ) )
			{
				bAtLeastOneChecked = true;
				bCanStop = true;
			}
		}

		if( NULL == hNextItem )
		{
			if( vecTreeList.size() > 0 )
			{
				iCurrentDeep--;
				hNextItem = vecTreeList.back();
				hNextItem = GetNextSiblingItem( hNextItem );
				vecTreeList.pop_back();
			}
			else
			{
				bCanStop = true;
			}
		}
		else if( NULL != GetChildItem( hNextItem ) )
		{
			iCurrentDeep++;
			vecTreeList.push_back( hNextItem );
			hNextItem = GetChildItem( hNextItem );
		}
		else
		{
			hNextItem = GetNextSiblingItem( hNextItem );
		}

	}while( false == bCanStop );

	return bAtLeastOneChecked;
}

void CHMTreeListCtrl::ExpandNodeAfterDelay( HTREEITEM hItem, UINT uiDelay )
{
	if( NULL == hItem || uiDelay <= 0 )
	{
		return;
	}
	
	if( 0 != m_nTimer )
	{
		KillTimer( m_nTimer );
	}

	m_hItemToExpand = hItem;
	m_nTimer = SetTimer( _TIMERID_HMTREELISTCTRL, uiDelay, NULL );
}

BEGIN_MESSAGE_MAP( CHMTreeListCtrl, CTreeListCtrl )
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT( TVN_BEGINDRAG, OnBegindrag )
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CHMTreeListCtrl::PreTranslateMessage( MSG *pMsg )
{
	// Interpret Delete key.
	if( WM_KEYDOWN == pMsg->message && VK_DELETE == pMsg->wParam )
	{
		HTREEITEM hItem = GetSelectedItem();
		m_UserData *pUD = (m_UserData *)GetUserData( hItem );

		if( pUD->bImported )
		{
			DeleteItem( hItem );
		}
	}

	return __super::PreTranslateMessage( pMsg );
}

int CHMTreeListCtrl::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( -1 == CTreeListCtrl::OnCreate( lpCreateStruct ) )
	{
		return -1;
	}

	// Register Tree control as a drop target.
	m_CTreeDropTarget.Register( this );
	return 0;
}

void CHMTreeListCtrl::OnBegindrag( NMHDR *pNMHDR, LRESULT *pResult ) 
{
	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;
	HTREEITEM hTSelItem = pNMTreeView->itemNew.hItem;

	m_UserData *pUD = (m_UserData *)GetUserData( hTSelItem );

	if( false == pUD->pHM->IsHMFamily() )
	{
		return;
	}
	
	// Verify drag and drop is allowed for the current hItem.
	if( false == m_bAllhItemDragDrop )
	{
		m_UserData *pUD = (m_UserData *)GetUserData( hTSelItem );

		if( false == pUD->bImported )
		{
			return;
		}
	}

	// Highlight selected item.
	SelectItem( hTSelItem );
	Select( hTSelItem, TVGN_DROPHILITE );
	
	COleDataSource *poleSourceObj = new COleDataSource;

	// Attache handle of this window to the ole data source.
	HGLOBAL hgDrop = GlobalAlloc( GHND | GMEM_SHARE, sizeof( HANDLE ) );

	if( NULL == hgDrop )
	{
		return;
	}

	PHANDLE pHandle = (PHANDLE)GlobalLock( hgDrop );
	
	if( NULL == pHandle )
	{
		GlobalFree( hgDrop );
		return;
	}
	
	*pHandle = m_hWnd;
	GlobalUnlock ( hgDrop );
	
	FORMATETC rFormatEtc = { CF_PRIVATEFIRST + 1, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	poleSourceObj->CacheGlobalData( CF_PRIVATEFIRST + 1, hgDrop, &rFormatEtc );

	// Begin drag operation.
	DROPEFFECT dropeffect = poleSourceObj->DoDragDrop();

	// If drop has been cancel, we must free the memory.
	// Remarks: if success, it's 'CHMTreeDropTarget' that frees the memory.
	if( DROPEFFECT_NONE == dropeffect )
	{
		GlobalFree( hgDrop );
	}
	
	// Remove the highlighting.
	SendMessage( TVM_SELECTITEM, TVGN_DROPHILITE, 0 );

	delete poleSourceObj;
	*pResult = 0;
}

void CHMTreeListCtrl::OnTimer( UINT_PTR nIDEvent )
{
	CTreeListCtrl::OnTimer( nIDEvent );

	if( (UINT_PTR)0 == m_nTimer || m_nTimer != nIDEvent )
	{
		return;
	}

	Expand( m_hItemToExpand, TVE_EXPAND );
	KillTimer( m_nTimer );
	m_nTimer = (UINT_PTR)0;
}
