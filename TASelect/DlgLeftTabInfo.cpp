#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"

#include "Global.h"
#include "Utilities.h"
#include "Units.h"
#include "DlgLTtabctrl.h"
#include "RViewInfo.h"

#include "DlgLeftTabInfo.h"
#include "CsvParser.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgLeftTabInfo *pDlgLeftTabInfo = NULL;

CDlgLeftTabInfo::CDlgLeftTabInfo( CWnd *pParent )
	: CDlgLeftTabBase( CMyMFCTabCtrl::TabIndex::etiInfo, CDlgLeftTabInfo::IDD, pParent )
{
	m_pTADB = TASApp.GetpTADB();
	m_bExcludeUnlinkedQrs = false;
	m_strDocLanguage = _T("");
	m_strVersionLang = _T("");
	m_strDocDBPath = _T("");
	m_bUntranslatedDocExists = false;
}

CDlgLeftTabInfo::~CDlgLeftTabInfo()
{
	if( NULL != m_Tree.GetSafeHwnd() )
	{
		m_Tree.DeleteAllItems();
		m_Tree.DestroyWindow();
	}

	pDlgLeftTabInfo = NULL;
}

BEGIN_MESSAGE_MAP( CDlgLeftTabInfo, CDlgLeftTabBase )
	ON_WM_SIZE()
	ON_NOTIFY( TVN_ITEMEXPANDED, IDC_TREE, OnItemExpandedTree )
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREE, OnSelChangedTree )
	ON_MESSAGE( WM_USER_DISPLAYDOCUMENTATION, OnDisplayDocumentation )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgLeftTabInfo::ResetTree()
{
	// Retrieve the current documentation language.
	m_strDocLanguage = TASApp.GetDocLanguage();
	
	// Get the docPath for the current language and the default version language.
	m_strVersionLang = _T("");
	CDB_MultiString *pMStr = (CDB_MultiString *)( GetpTADB()->Get( _T("VERSION_LANGUAGE") ).MP );

	if( NULL != pMStr )
	{
		m_strVersionLang = pMStr->GetString( 0 );
	}
	
	// Get the current doc path.
	m_strDocDBPath = TASApp.GetDocBasePath();
	
	// First make 'm_Tree' empty if necessary.
	if( m_Tree.GetCount() != -1 )
	{
		m_Tree.DeleteAllItems();
	}

	// Initialize parameters for insertion of items.
	CString str = TASApp.LoadLocalizedString( IDS_RVINFO_ROOTITEM );
	
	TV_INSERTSTRUCT tvinsert;
	tvinsert.hParent = NULL;
	tvinsert.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
	tvinsert.item.hItem = NULL;
	tvinsert.item.state = 0;
	tvinsert.item.stateMask = 0;
	tvinsert.item.cchTextMax = 40;
	tvinsert.item.cChildren = 0;
	tvinsert.item.lParam = 0;
	tvinsert.item.iImage = CRCImageManager::ImgListInfoTree::ILIT_BookClosed;
	tvinsert.item.iSelectedImage = CRCImageManager::ImgListInfoTree::ILIT_BookClosed;
	tvinsert.item.pszText = (TCHAR *)(LPCTSTR)str;
	HTREEITEM hRoot = m_Tree.InsertItem( &tvinsert );
	tvinsert.hParent = hRoot;

	// Determine if Unlinked Qrs should be excluded	  NOT USED?.
	str = CString( ( (CDB_MultiString *)( m_pTADB->Get( _T("DOCEXT_PARAM") ).MP ) )->GetString( 2 ) );
	str.MakeLower();
	m_bExcludeUnlinkedQrs = false;
	
	// Order the DocTree_Tab.
	CTableOrdered *pTab = (CTableOrdered *)( m_pTADB->Get( _T("DOCTREE_TAB") ).MP );
	std::map<int, CData *> Map;
	pTab->FillMapListOrdered( &Map );
	{	// ****** Construct Tree from docdb.csv ******

		// Construct tree.
		tvinsert.hParent = hRoot;
		tvinsert.hInsertAfter = NULL;
		tvinsert.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;

		for( unsigned int i = 1; i < TASApp.GetDocDb().Size(); ++i )
		{
			_string strNodeTextDocLang = TASApp.GetDocDb().GetTextAt( i, _string( TASApp.GetDocLanguage() ) );

			if( TASApp.GetDocDb().GetNameAt(i).find(L"ta_") != std::wstring::npos )
			{
				// Not a document.
				tvinsert.item.iImage = CRCImageManager::ImgListInfoTree::ILIT_BookClosed;
				tvinsert.item.iSelectedImage = CRCImageManager::ImgListInfoTree::ILIT_BookClosed;

				if( true == strNodeTextDocLang.empty() )
				{ 
					continue;
				}

				tvinsert.item.pszText = const_cast<LPWSTR>( strNodeTextDocLang.c_str() );
				tvinsert.item.lParam = 0;

				for( unsigned int j = 0; j < i; ++j )
				{
					// Find parent.
					if( TASApp.GetDocDb().GetIdAt( i ).find( TASApp.GetDocDb().GetNameAt( j ) ) != std::wstring::npos )
					{
						_string strNodeText = TASApp.GetDocDb().GetTextAt( j, _string( TASApp.GetDocLanguage() ) );

						if( true == strNodeText.empty() )
						{
							// If there's no label the node was not created
							continue;
						}

						tvinsert.hParent = TASApp.GetDocDb().GetTreeItemAt( j );
						break;
					}
					else if( _T("0") == TASApp.GetDocDb().GetIdAt(i) )
					{
						tvinsert.hParent = hRoot;
						break;
					}	
				}

				TASApp.GetDocDb().GetTreeItemAt( i ) = m_Tree.InsertItem( &tvinsert );
			}
			else
			{
				// It's a document.
				tvinsert.item.iImage = CRCImageManager::ImgListInfoTree::ILIT_QuickReferenceSheetNormal;
				tvinsert.item.iSelectedImage = CRCImageManager::ImgListInfoTree::ILIT_QuickReferenceSheetSelected;

				if( true == strNodeTextDocLang.empty() )
				{ 
					continue;
				}

				tvinsert.item.pszText = const_cast<LPWSTR>( strNodeTextDocLang.c_str() );
				tvinsert.item.lParam = i;

				for( unsigned int j = 0; j < i; ++j )
				{
					if( TASApp.GetDocDb().GetIdAt( i ).find( TASApp.GetDocDb().GetNameAt( j ) ) != std::wstring::npos )
					{
						_string strNodeText = TASApp.GetDocDb().GetTextAt( j, _string( TASApp.GetDocLanguage() ) );

						if( true == strNodeText.empty() )
						{
							// If there's no label the node was not created
							continue;
						}

						tvinsert.hParent = TASApp.GetDocDb().GetTreeItemAt( j );
						break;
					}
					else if( _T("0") == TASApp.GetDocDb().GetIdAt( i ) )
					{
						tvinsert.hParent = hRoot;
						break;
					}

				}

				TASApp.GetDocDb().GetTreeItemAt( i ) = m_Tree.InsertItem( &tvinsert );

			}
		}
	} // ****** End Construct Tree from docdb.csv ******


	// Insert all Doc that belongs to the DocTree_Tab.
	// Special treatment is used in the function for Cat and Qrs docs.
	//_InsertDocs( &Map, &hRoot );

	// Loop on all folders and remove empty ones.
	_RemoveEmptyFolders( &hRoot );

	// Set by default an expanded tree for the first child hItem.
	m_Tree.Expand( hRoot, TVE_EXPAND );
	m_Tree.Expand( m_Tree.GetChildItem( hRoot ), TVE_EXPAND );

	if( NULL != pRViewInfo && pRViewInfo->GetSafeHwnd() )
	{
		CString strFolderDeco( TASApp.GetStartDir() );
		strFolderDeco += _T("FolderDeco.htm");
		pRViewInfo->DisplayPage( (LPCTSTR)strFolderDeco );
	}
}

void CDlgLeftTabInfo::InsertUntranslated( CTreeCtrl *pMainTree, HTREEITEM *phRoot, CString strDocToInsert, int iItemIndex )
{
	if( ( NULL == pMainTree ) || ( NULL == phRoot ) )
	{
		return;
	}

	if( false == m_bUntranslatedDocExists )
	{
		CString str = TASApp.LoadLocalizedString( IDS_RVINFO_UNTRANSLATED );
		TV_INSERTSTRUCT tvinsert;
		tvinsert.item.lParam = 0;
		tvinsert.hParent = *phRoot;
		tvinsert.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
		tvinsert.item.hItem = NULL;
		tvinsert.item.state = 0;
		tvinsert.item.stateMask = 0;
		tvinsert.item.cchTextMax = 40;
		tvinsert.item.cChildren = 0;
		tvinsert.item.iImage = CRCImageManager::ImgListInfoTree::ILIT_BookClosed;
		tvinsert.item.iSelectedImage = CRCImageManager::ImgListInfoTree::ILIT_BookClosed;
		tvinsert.item.pszText = (TCHAR *)(LPCTSTR)str;
		*phRoot = pMainTree->InsertItem( &tvinsert );
		m_bUntranslatedDocExists = true;
		hUnTranslatedRoot = *phRoot;
	}

	if( ( CteEMPTY_STRING != strDocToInsert ) && ( NULL == TASApp.GetDocDbEn().GetTreeItemAt( iItemIndex ) ) )
	{
		HTREEITEM hPA = pMainTree->InsertItem( TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM,
			strDocToInsert, 2, 3, 0, 0, iItemIndex, hUnTranslatedRoot, NULL );
		pMainTree->SelectItem( hPA );
		TASApp.GetDocDbEn().GetTreeItemAt( iItemIndex ) = hPA;
	}
	else if( NULL != TASApp.GetDocDbEn().GetTreeItemAt( iItemIndex ) )
	{
		pMainTree->SelectItem( TASApp.GetDocDbEn().GetTreeItemAt( iItemIndex ) );
	}

}

void CDlgLeftTabInfo::OnLTTabCtrlEnterTab( CMyMFCTabCtrl::TabIndex eTabIndex, CMyMFCTabCtrl::TabIndex ePrevTabIndex )
{
	// Base class.
	CDlgLeftTabBase::OnLTTabCtrlEnterTab( eTabIndex, ePrevTabIndex );

	CMainFrame *pFrame = static_cast<CMainFrame*>( AfxGetMainWnd() );
	pFrame->ActiveFormView( CMainFrame::RightViewList::eRVInfo );

	// Refresh tab title
	CString str;
	str = TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_INFO );

	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetWindowText( str );
	}

	// Force the first refreshing of doc sheet.
	LRESULT Result;
	OnSelChangedTree( NULL, &Result ); 
}

void CDlgLeftTabInfo::InitializeToolsDockablePaneContextID( void )
{
	CToolsDockablePane *pclToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL == pclToolsDockablePane )
	{
		return;
	}

	// Register context with the tools dockable pane for the 'Info' tab.
	CToolsDockablePane::ContextParameters *pclContext = pclToolsDockablePane->RegisterContext();

	if( NULL == pclContext )
	{
		return;
	}

	pclContext->m_bIsPaneVisible = false;
	pclContext->m_bIsPaneAutoHide = FALSE;
	pclContext->m_bIsGripperEnabled = FALSE;
	pclContext->m_bIsPaneFullScreen = false;
	pclContext->m_bIsContextMenuEnabled = true;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_HMCompilation] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_BatchSelection] = false;
	pclContext->m_arIsTabVisible[CToolsDockablePane::TabID::OutputID_PMGraphs] = false;
	pclContext->m_iLinkedRightView = CMainFrame::RightViewList::eRVInfo;

	m_iContextID = pclContext->m_iContextID;
	m_bToolsDockablePaneContextInitialized = true;
}

bool CDlgLeftTabInfo::GetToolsDockablePaneContextID( int &iContextID  )
{
	if( false == CDlgLeftTabBase::GetToolsDockablePaneContextID( iContextID ) )
	{
		return false;
	}

	iContextID = m_iContextID;
	return ( -1 != m_iContextID ) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgLeftTabInfo::DoDataExchange( CDataExchange *pDX )
{
	CDlgLeftTabBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_TREE, m_Tree );
}

BOOL CDlgLeftTabInfo::OnInitDialog() 
{
	CDlgLeftTabBase::OnInitDialog();
	pDlgLeftTabInfo	= this;

	// Build and attach an image list to m_Tree.
	CImageList *pclImgListInfoTree = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_InfoTree );
	ASSERT ( NULL != pclImgListInfoTree );

	// m_TreeImageList.Create( IDB_LVINFOTREE, 16, 1, _BLACK );
	// m_TreeImageList.SetBkColor( CLR_NONE );
	m_Tree.SetImageList(pclImgListInfoTree, TVSIL_NORMAL );

	ResetTree();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgLeftTabInfo::OnCommand( WPARAM wParam, LPARAM lParam )
{
	CWnd *pWnd = GetFocus();

	switch( wParam )
	{
		case IDOK: 
			if( pWnd != GetDlgItem( IDOK ) )
			{
				return FALSE;
			}

			break;

		case IDCANCEL:
			if( pWnd != GetDlgItem( IDCANCEL ) )
			{
				return FALSE;
			}

			break;
	}

	return CDlgLeftTabBase::OnCommand( wParam, lParam );
}

void CDlgLeftTabInfo::OnSize( UINT nType, int cx, int cy ) 
{
	CDlgLeftTabBase::OnSize( nType, cx, cy );
	
	// Get TabCtrl window size.
	CRect rect, rectBmp;
	
	// Get the size in pixel of the TA Logo.
	rectBmp = CRect( 0, 0, 32, 32 );
	rectBmp = GetTALogoSize();
	CTabCtrl *pTabCtrl = (CTabCtrl *)GetParent();
	pTabCtrl->GetClientRect( rect );
	pTabCtrl->AdjustRect( FALSE, &rect );
	pTabCtrl->ClientToScreen( rect );
	ScreenToClient( rect );

	// Test on existence of TreeCtrl because OnSize is already called before the controls of the dialog are created.
	CTreeCtrl *pTreeCtrl = (CTreeCtrl *)GetDlgItem( IDC_TREE );

	if( NULL != pTreeCtrl )
	{
		// Resize m_Tree.
		CDC *pDC = GetDC();
		int DeviceCaps = pDC->GetDeviceCaps( LOGPIXELSY );

		// NEVER forget to release a device context obtained with 'CWnd::GetDC()'.
		ReleaseDC( pDC );

		int iHeightOf7 = (int)( ( ( rectBmp.Height() * 1.5 ) + 6 ) * DeviceCaps / 96.0 ); //96 Dpi as reference
		m_Tree.SetWindowPos( &wndTop, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top - iHeightOf7, SWP_SHOWWINDOW );
	}
}

void CDlgLeftTabInfo::OnItemExpandedTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// Update the image when a parent item is expanded or collapsed
	// (Open and closed book).
	HTREEITEM hItem = (pNMTreeView->itemNew).hItem;

	if( m_Tree.ItemHasChildren( hItem ) )
	{
		if( TVE_EXPAND == pNMTreeView->action )
		{
			m_Tree.SetItemImage( hItem, 1, 1 );
		}
		else if( TVE_COLLAPSE == pNMTreeView->action )
		{
			m_Tree.SetItemImage( hItem, 0, 0 );
		}
	}
	
	*pResult = 0;
}

void CDlgLeftTabInfo::OnSelChangedTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;
	*pResult = 0;

	CString strFolderDeco( TASApp.GetStartDir() );
	strFolderDeco += _T("FolderDeco.htm");

	// Only of valid handle, first call is done before the right sheet is displayed.
	if( NULL == pRViewInfo || (HWND)NULL == pRViewInfo->GetSafeHwnd() )
	{
		return;
	}
	
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	
	if( (HTREEITEM)NULL == hTree )
	{
		return;
	}

	if( TRUE == m_Tree.ItemHasChildren( hTree ) )
	{
		if( NULL == m_Tree.GetItemData( hTree ) )
		{
			pRViewInfo->DisplayPage( (LPCTSTR)strFolderDeco );
			return;	
		}
		
		CString strDocID = TASApp.GetDocDb().GetTextAt( m_Tree.GetItemData( hTree ), _string( TASApp.GetDocLanguage() ) ).c_str();

		if( std::string::npos == TASApp.GetDocDb().GetNameAt( m_Tree.GetItemData( hTree ) ).find( _T("ta_") ) )
		{
			strDocID.Empty();
		}

		if( strDocID.GetLength() > 0 )
		{
			CString str = strDocID;

			// Try to find the doc in the current documentation language.
			CString strDocFile = m_strDocDBPath + m_strDocLanguage + _T("\\") + str;

			CFileFind finder;
			BOOL bFind = finder.FindFile( strDocFile );

			// If not found, try to find the doc in the default language.
			if( FALSE == bFind )
			{
				strDocFile = m_strDocDBPath + m_strVersionLang + _T("\\") + str;
				bFind = finder.FindFile( strDocFile );
			}

			// Set the new directory for the doc and set the boolean to true.
			if( TRUE == bFind )
			{
				pRViewInfo->DisplayPage( strDocFile );
			}
			else
				pRViewInfo->DisplayPage( (LPCTSTR)strFolderDeco );
		}
		else
		{
			pRViewInfo->DisplayPage( (LPCTSTR)strFolderDeco );
		}
		
		return;
	}

	// HYS-1057: Display the English catalog if doesn't exist in the current language.
	CString	strDocID = TASApp.GetDocDb().GetTextAt( m_Tree.GetItemData( hTree ), _string( TASApp.GetDocLanguage() ) ).c_str();

	if( CteEMPTY_STRING != strDocID )
	{

		if( TASApp.GetDocDb().GetNameAt( m_Tree.GetItemData( hTree ) ).find( _T("ta_") ) != std::string::npos
				|| TASApp.GetDocDb().GetLangDoc( _string( m_strDocLanguage ) ).empty() )
		{
			strDocID.Empty();
			CString strFolderDeco( TASApp.GetStartDir() );
			strFolderDeco += _T("FolderDeco.htm");
			pRViewInfo->DisplayPage( (LPCTSTR)strFolderDeco );
		}
		else
		{
			CString DocFile = m_strDocDBPath + TASApp.GetDocDb().GetLangDoc( _string( m_strDocLanguage ) ).c_str() + _T("\\cat\\");
			DocFile += TASApp.GetDocDb().GetPathFromHTreeItem( hTree, _string( TASApp.GetDocLanguage() ) ).c_str();
			pRViewInfo->DisplayPage( DocFile );
		}
	}
	else
	{
		strDocID = TASApp.GetDocDbEn().GetTextAt( m_Tree.GetItemData( hTree ), L"en" ).c_str();

		if( TASApp.GetDocDbEn().GetNameAt( m_Tree.GetItemData( hTree ) ).find( _T("ta_") ) != std::string::npos
				|| TASApp.GetDocDbEn().GetLangDoc( L"en" ).empty() )
		{
			strDocID.Empty();
			CString strFolderDeco( TASApp.GetStartDir() );
			strFolderDeco += _T("FolderDeco.htm");
			pRViewInfo->DisplayPage( (LPCTSTR)strFolderDeco );
		}
		else
		{
			CString DocFile = m_strDocDBPath + TASApp.GetDocDbEn().GetLangDoc( L"en" ).c_str() + _T("\\cat\\");
			DocFile += strDocID;
			pRViewInfo->DisplayPage( DocFile );
		}
	}	
}

LRESULT CDlgLeftTabInfo::OnDisplayDocumentation( WPARAM wParam, LPARAM lParam )
{
	// Send message to notify the display of the product info tab.
	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiInfo );
	}

	if( NULL == (LPCTSTR)lParam )
	{
		return 0;
	}

	if( _T('\0') == *(LPCTSTR)lParam )
	{
		return 0;
	}
	
	_string strBodyArtNum;

	// HYS-1057: Delete DocSheetType and DocSheetProduct enum: now we are only one product and one documentation
	CDB_Product *pclProduct = (CDB_Product *)( m_pTADB->Get( (LPCTSTR)lParam ).MP );
	ASSERT( NULL != pclProduct );

	if( NULL != pclProduct )
	{
		strBodyArtNum = pclProduct->GetBodyArtNum();
	}

	// First collapse all to focus only on the documentation to show.
	_CollapseAll( m_Tree.GetRootItem() );

	HTREEITEM treeNode = TASApp.GetDocDb().GetHTreeItemFromArticleNumber( strBodyArtNum );

	if( NULL != treeNode )
	{
		m_Tree.EnsureVisible( treeNode );
		m_Tree.SelectItem( treeNode );
	}
	else
	{
		// HYS-1057: Insert the untranslated catalogs
		HTREEITEM hRoot = m_Tree.GetRootItem();
		_string strNodeTextDocLang = TASApp.GetDocDbEn().GetTextAt( TASApp.GetDocDbEn().GetHTreeItemIndexFromArticleNumber( strBodyArtNum ), L"en" );
		InsertUntranslated( &( m_Tree ), &hRoot, strNodeTextDocLang.c_str(), TASApp.GetDocDbEn().GetHTreeItemIndexFromArticleNumber( strBodyArtNum ) );
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgLeftTabInfo::_CompleteDocInformation( CDB_Doc *pDoc )
{
	if( NULL == pDoc )
	{
		return;
	}

	CString str = pDoc->GetString( 1 );

	// Extract name after language.
	// Remark: pay attention that if user has already changed language during same session of TASelect, 'str' here is the full
	//         path. For example "c:\Users\FFustin\Documents\HySelect\Doc\fr\Qrs\STAD???.pdf".
	//         If we want thus only 'Qrs\STAD???.pdf' string, we need to reject what is before.
	int iPos = str.Find( m_strDocDBPath );

	if( iPos != - 1 )
	{
		ASSERT( ( str.GetLength() - m_strDocDBPath.GetLength() - 3 ) > 0 );
		CString strtmp = str.Right( str.GetLength() - m_strDocDBPath.GetLength() - 3 );
		str = strtmp;
	}

	// Try to find the doc in the current language.
	CString strDocFile = m_strDocDBPath + m_strDocLanguage + _T("\\") + str;

	CFileFind finder;
	BOOL bFound = finder.FindFile( strDocFile );

	// If not found, try to find the doc in the default localized language...
	if( FALSE == bFound )
	{
		strDocFile = m_strDocDBPath + m_strVersionLang + L"\\" + str;
		bFound = finder.FindFile( strDocFile );
	}
	
	// If not found, try to find the doc in the English language...
	if( FALSE == bFound )
	{
		strDocFile = m_strDocDBPath + L"en\\" + str;
		bFound = finder.FindFile( strDocFile );
	}

	// If found, set the new directory for the doc and set the boolean to true...
	if( TRUE == bFound )
	{
		pDoc->SetString( 1, _string( strDocFile ) );
		pDoc->SetString( 2, _T("1") );
	}
}

CDB_Doc *CDlgLeftTabInfo::_FindCatFile( CDB_Doc *pDoc )
{
	// If the Cat exist in the folder return the current Cat...
	if( 0 == IDcmp( pDoc->GetString( 2 ), _T("1") ) )
	{
		return pDoc;
	}
	
	// In all other cases.
	return NULL;
}

bool CDlgLeftTabInfo::_FindObjectInTree( CDB_Doc *pDoc, HTREEITEM *phParent )
{
	if( NULL == pDoc )
	{
		return false;
	}

	// Variables.
	bool bFound = false;
	HTREEITEM hTree = m_Tree.GetChildItem( *phParent );

	while( NULL != hTree )
	{
		if( TRUE == m_Tree.ItemHasChildren( hTree ) )
		{
			bFound = _FindObjectInTree( pDoc, &hTree );
		}
		else if( (DWORD)pDoc == m_Tree.GetItemData( hTree ) )
		{
			// It's a particular case when PDF document are opened in a external window (as Adobe Acrobat Reader).
			// If user close this external window and ask the same documentation, 'SelectItem' will not fire the 'OnSelChangedTree'
			// event and the document will be not open again.
			m_Tree.SelectItem( m_Tree.GetRootItem() );
			m_Tree.SelectItem( hTree );
			return true;
		}

		if( true == bFound )
		{
			return true;
		}

		hTree = m_Tree.GetNextSiblingItem( hTree );
	}

	return bFound;
}

void CDlgLeftTabInfo::_InsertDocs( std::map<int, CData *> *pMap, HTREEITEM *phParent )
{
	// Variables.
	std::map<int, CData*>::iterator it;
	TV_INSERTSTRUCT tvinsert;
	tvinsert.hParent = *phParent;
	tvinsert.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
		
	// Loop on all tables included into the map and add them to the tree.
	for( it = pMap->begin(); it != pMap->end(); it++ )
	{
		// In case a map's item contain a table...
		if( true == it->second->IsaTable() )
		{
			// Local variables.
			CString TabName;
			CDB_Doc *pTabData = NULL;

			tvinsert.item.iImage = CRCImageManager::ImgListInfoTree::ILIT_BookClosed;
			tvinsert.item.iSelectedImage = CRCImageManager::ImgListInfoTree::ILIT_BookClosed;

			TabName = (TCHAR *)( (CTableOrdered *)it->second )->GetName();
			pTabData = (CDB_Doc *)( m_pTADB->Get( TabName ).MP );
			tvinsert.item.pszText = ( NULL == pTabData ) ? (TCHAR *)(LPCTSTR)TabName : (TCHAR *)pTabData->GetString( 0 );
			tvinsert.item.lParam = (LPARAM)it->second;
			HTREEITEM hItem = m_Tree.InsertItem( &tvinsert);
			
			// Recursive the function to go trough all the tree.
			CTableOrdered *ChildTableOrdered = (CTableOrdered *)it->second;
			std::map<int, CData *> MapChild;
			ChildTableOrdered->FillMapListOrdered( &MapChild );
			_InsertDocs( &MapChild, &hItem );
		}
		else if( dynamic_cast<CDB_Doc *>( it->second ) )
		{

			// In case a map's item contain doc.

			// CTABLE_DATA are not inserted.
			if( 0 == _tcscmp( ( (CDB_MultiString *)it->second->GetIDPtr().MP )->GetString( 2 ), _T("CTABLE_DATA") ) )
			{
				continue;
			}

			// Is it a Qrs or a Cat sheet ? See the folder name where the docs are placed.
			CString str = ( (CDB_Doc *)it->second )->GetString( 1 );

			// Extract 'Qrs' or 'Cat' info.
			// Remark: pay attention that if user has already changed language during same session of TASelect, 'str' here is the full
			//         path. For example "c:\Users\FFustin\Documents\HySelect\Doc\fr\Qrs\STAD???.pdf"
			int iBackSlashPos = str.ReverseFind( _T('\\') );

			if( iBackSlashPos < 3 )
			{
				ASSERT( 0 );
				continue;
			}

			CString strQrsOrCat = str.Mid( iBackSlashPos - 3, 3 );
			strQrsOrCat.MakeLower();

			// Verify that the Cat or Qrs exist in the folder.
			// Define the complete path for each file.
			_CompleteDocInformation( (CDB_Doc *)it->second );
			
			if( 0 == strQrsOrCat.Compare( _T("qrs") ) )
			{
				// Check if product is selectable.
				if( ( true == m_bExcludeUnlinkedQrs || _T('0') == *( ( (CDB_Doc *)it->second )->GetString( 2 ) ) )
						|| 0 == str.Compare( ( (CDB_MultiString*)m_pTADB->Get( _NO_ID ).MP )->GetString( 1 ) ) )
				{
					continue;
				}

				tvinsert.item.iImage = CRCImageManager::ImgListInfoTree::ILIT_QuickReferenceSheetNormal;
				tvinsert.item.iSelectedImage = CRCImageManager::ImgListInfoTree::ILIT_QuickReferenceSheetSelected;
				tvinsert.item.pszText = (TCHAR *)( (CDB_Doc *)it->second )->GetString( 0 );
				tvinsert.item.lParam = (LPARAM)it->second;
			}
			else if( 0 == strQrsOrCat.Compare( _T("cat") ) )
			{
				CDB_Doc *pDoc = _FindCatFile( (CDB_Doc *)it->second );

				if( NULL == pDoc )
				{
					continue;
				}

				tvinsert.item.iImage = CRCImageManager::ImgListInfoTree::ILIT_FullCatalogNormal;
				tvinsert.item.iSelectedImage = CRCImageManager::ImgListInfoTree::ILIT_FullCatalogSelected;
				tvinsert.item.pszText = (TCHAR*)( (CDB_Doc *)it->second )->GetString( 0 );
				tvinsert.item.lParam = (LPARAM)pDoc;
			}
			else
			{
				continue;
			}

			m_Tree.InsertItem( &tvinsert );
		}
		else
		{
			ASSERT( 0 ); // File is not a doc or a Table
		}
	}
}

void CDlgLeftTabInfo::_RemoveEmptyFolders( HTREEITEM *phParent )
{
	// Variables.
	int iImg;
	int iSelImg;
	HTREEITEM hTree = m_Tree.GetChildItem( *phParent );

	while( NULL != hTree )
	{		
		HTREEITEM hNewTree = hTree;
		m_Tree.GetItemImage( hTree, iImg, iSelImg );

		// Img==0 && SelImg ==0 represent a folder
		if( TRUE == m_Tree.ItemHasChildren( hTree ) && 0 == iImg && 0 == iSelImg )
		{
			_RemoveEmptyFolders( &hTree );
		}

		hTree = m_Tree.GetNextSiblingItem( hTree );

		if( FALSE == m_Tree.ItemHasChildren( hNewTree ) && 0 == iImg && 0 == iSelImg )			
		{
			m_Tree.DeleteItem( hNewTree );
		}
	}
}

void CDlgLeftTabInfo::_CollapseAll( HTREEITEM hItem )
{
	if( NULL == hItem )
	{
		return;
	}

	HTREEITEM hChild = m_Tree.GetChildItem( hItem );

	if( NULL != hChild )
	{
		_CollapseAll( hChild );
	}

	if( NULL != m_Tree.GetChildItem( hItem ) )
	{
		m_Tree.SetItemImage( hItem, CRCImageManager::ImgListInfoTree::ILIT_BookClosed, CRCImageManager::ImgListInfoTree::ILIT_BookClosed );
	}

	m_Tree.Expand( hItem, TVE_COLLAPSE );
	hItem = m_Tree.GetNextSiblingItem( hItem );
	_CollapseAll( hItem );
}
