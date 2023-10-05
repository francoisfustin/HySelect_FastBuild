#include "stdafx.h"
#include "TASelect.h"
#include "DlgDocs.h"
#include "DlgLTtabctrl.h"
#include "DlgLeftTabBase.h"
#include "DlgLeftTabInfo.h"

IMPLEMENT_DYNAMIC( CDlgDocs, CDialogEx )

CDlgDocs::CDlgDocs( CWnd *pParent )
	: CDialogEx( CDlgDocs::IDD, pParent )
{
	m_bFirstOpening = true;
}

CDlgDocs::~CDlgDocs()
{
}

BEGIN_MESSAGE_MAP( CDlgDocs, CDialogEx )
	ON_NOTIFY( NM_CLICK, IDC_TREE_DOCS, OnNMClickTreeDocs )
	ON_BN_CLICKED( IDOK, OnBnClickedOk )
	ON_BN_CLICKED( IDCANCEL, OnBnClickedCancel )
	ON_CBN_SELCHANGE( IDC_COMBODFTLANGDOC, OnSelChangeComboDftLangDoc )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CDlgDocs::Display( bool fFirstOpening )
{
	// Init Variables.
	m_bFirstOpening = fFirstOpening;

	return DoModal();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgDocs::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATIC_GROUP_DOCS, m_GroupDoc );
	DDX_Control( pDX, IDC_STATIC_GROUP_TASCOPE, m_GroupTASCOPE );
	DDX_Control( pDX, IDC_TREE_DOCS, m_TreeDoc );
	DDX_Control( pDX, IDC_CHECK_TASCOPE, m_BtnTASCOPE );
	DDX_Control( pDX, IDC_COMBODFTLANGDOC, m_ChangeLanguage );
}

BOOL CDlgDocs::OnInitDialog()
{
	// Do the default function.
	CDialogEx::OnInitDialog();

	// Define the tree to use check boxes.
	m_TreeDoc.ModifyStyle( TVS_CHECKBOXES, 0 );
	m_TreeDoc.ModifyStyle( 0, TVS_CHECKBOXES );

	// Display or not the TA-SCOPE check box.
	if( false == m_bFirstOpening )
	{
		// Remove the check boxes and there controls.
		m_GroupTASCOPE.ShowWindow( SW_HIDE );
		m_BtnTASCOPE.ShowWindow( SW_HIDE );

		// Resize the doc group box, the list box.
		
		// Variables.
		CRect rectDoc, rectTASCOPE, rectLisDoc;
			
		// Get the Window position.
		m_GroupDoc.GetWindowRect( &rectDoc );
		m_GroupTASCOPE.GetWindowRect( &rectTASCOPE );
		m_TreeDoc.GetWindowRect( &rectLisDoc );
			
		// Get the position in the dialog.
		ScreenToClient( &rectDoc );
		ScreenToClient( &rectTASCOPE );
		ScreenToClient( &rectLisDoc );

		// Define the space between the doc group box and the doc list box.
		int iSpace = rectDoc.bottom - rectLisDoc.bottom;

		// Move the windows.
		CRect rectNewDocPos( rectDoc.left, rectDoc.top, rectDoc.right, rectTASCOPE.bottom );
		CRect rectNewListDocPos( rectLisDoc.left, rectLisDoc.top, rectLisDoc.right, rectNewDocPos.bottom - iSpace );
		m_GroupDoc.MoveWindow( &rectNewDocPos );
		m_TreeDoc.MoveWindow( &rectNewListDocPos );
	}
	else
	{
		this->SetWindowPos( &CWnd::wndTopMost , 0, 0, 0, 0, SWP_NOSIZE );
		CenterWindow(CWnd::GetDesktopWindow());
		// Remove the Cancel button.
		GetDlgItem( IDCANCEL )->ShowWindow( SW_HIDE );

		// Variable.
		CRect rectCancel;

		// Get the Window position.
		GetDlgItem( IDCANCEL )->GetWindowRect( &rectCancel );

		// Get the position in the dialog.
		ScreenToClient( &rectCancel );

		// Move the window.
		GetDlgItem( IDOK )->MoveWindow( &rectCancel );
	}
	
	// Initialize dialog strings.
	CString str;

	if( true == m_bFirstOpening )
	{
		str = TASApp.LoadLocalizedString( IDS_DLGDOCS_TITLE_FOP );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_DLGDOCS_TITLE );
	}

	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDOCS_DFTLANGDOC );
	GetDlgItem( IDC_STATIC_DFTLANGDOC )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGDOCS_GROUPDOCS );
	m_GroupDoc.SetWindowText( str );
		
	str = TASApp.LoadLocalizedString( IDS_DLGDOCS_GROUPTASCOPE );
	m_GroupTASCOPE.SetWindowText( str );
		
	str = TASApp.LoadLocalizedString( IDS_DLGDOCS_EDITDOCS );
	GetDlgItem( IDC_EDIT_DOCS )->SetWindowText( str );
		
	str = TASApp.LoadLocalizedString( IDS_DLGDOCS_MAINTAINSCOPEFILES );
	m_BtnTASCOPE.SetWindowText( str );
		
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );
		
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );

	_FillComboChangeLanguage();

	// Fill the tree box.
	_FillTreeBox();

	// Set the check boxes if it is the first opening.
	if( true == m_bFirstOpening )
	{
		m_BtnTASCOPE.SetCheck( BST_CHECKED );

		// Create entry into the registry.
		for( HTREEITEM hItem = m_TreeDoc.GetRootItem(); hItem != NULL; hItem = m_TreeDoc.GetNextSiblingItem( hItem ) )
		{
			::AfxGetApp()->WriteProfileInt( L"Documentation", (LPCTSTR)m_TreeDoc.GetItemData( hItem ), 0 );
		}

		::AfxGetApp()->WriteProfileInt( L"TA-SCOPE", (LPCTSTR)L"Maintain files", 1 );
	}
	else
	{
		// Nothing to do.
	}

	return TRUE;
}

void CDlgDocs::OnNMClickTreeDocs( NMHDR *pNMHDR, LRESULT *pResult )
{
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x= ( (int)(short)LOWORD( dwpos ) );		// GET_X_LPARAM(dwpos);
	ht.pt.y= ( (int)(short)HIWORD( dwpos ) );		// GET_Y_LPARAM(dwpos);

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	UINT uiCheckBoxFlag = 64;

	if( ( TVHT_ONITEMSTATEICON & ht.flags ) || ( TVHT_ONITEM & ht.flags ) )
	{
		// In case the user select the check box, do not change the state manually.
		if( ht.flags!=uiCheckBoxFlag )
		{
			BOOL BState = m_TreeDoc.GetCheck( ht.hItem );
			m_TreeDoc.SetCheck( ht.hItem, !BState );
		}
	}

	*pResult = 0;
}

void CDlgDocs::OnBnClickedOk()
{
	// Save default documentation language if needed.
	if( m_strPrevDocLanguage.CompareNoCase( m_strNewDocLanguage ) != 0 )
	{
		TASApp.SetDocLanguage( m_strNewDocLanguage );

		// Reload the docdb from the current language
		CString DocDbPath;
		DocDbPath.Format( _DOCDB_CSV_FILE, TASApp.GetDocLanguage() );
		TASApp.GetDocDb().SetCsvPath( TASApp.GetDocumentsFolderForDoc() + CString(_HYSELECT_NAME_BCKSLASH) + DocDbPath );
		
		// Reset tree in info dialog tab.
		if( NULL != pDlgLeftTabInfo )
		{
			pDlgLeftTabInfo->ResetTree();
		}
	}
	
	// Save all check boxes into the registry.
	for( HTREEITEM hItem = m_TreeDoc.GetRootItem(); hItem; hItem = m_TreeDoc.GetNextSiblingItem( hItem ) )
	{
		::AfxGetApp()->WriteProfileInt( _T("Documentation"), (LPCTSTR)m_TreeDoc.GetItemData( hItem ), m_TreeDoc.GetCheck( hItem ) );
	}

	// Save the check box state for the TA-SCOPE.
	if( true == m_bFirstOpening )
	{
		::AfxGetApp()->WriteProfileInt( _T("TA-SCOPE"), (LPCTSTR)_T("Maintain files"), m_BtnTASCOPE.GetCheck() );
	}
	else
	{
		// Nothing to do.
	}
	
	// Call the default function.
	CDialog::OnOK();
}

void CDlgDocs::OnBnClickedCancel()
{
	// Call the default function.
	CDialog::OnCancel();
}

void CDlgDocs::OnSelChangeComboDftLangDoc()
{
	int iSel = m_ChangeLanguage.GetCurSel();

	if( iSel != CB_ERR )
	{
		// Recuperate the value.
		DWORD_PTR pDWORD = m_ChangeLanguage.GetItemData( iSel );
		LPCTSTR lpctstr = (LPCTSTR)pDWORD;
		m_strNewDocLanguage = (CString)lpctstr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDlgDocs::_FillComboChangeLanguage( void )
{
	// Init variables.
	std::map<int, CData*> Map;
	std::map<int, CData*>::iterator it;
	int iCurLang = 0;

	// Remove all items.
	m_ChangeLanguage.Clear();

	// Get the map with all languages.
	CTableOrdered *pTab = (CTableOrdered*)( TASApp.GetpTADB()->Get( _T("LANG_TAB" ) ).MP );
	ASSERT( NULL != pTab );

	// Order the list.
	for( IDPTR IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		Map.insert( std::pair<int, CData*>( _ttoi( ( (CDB_StringID*)IDPtr.MP )->GetIDstr() ), IDPtr.MP ) );
	}

	// Fill the combo with all languages.
	for( it = Map.begin(); it != Map.end(); it++ )
	{
		LPCTSTR lpLang = ( (CDB_StringID*)it->second )->GetString();
		LPCTSTR lpTriGram = ( (CDB_StringID*)it->second )->GetIDstr2();
		int iPos = m_ChangeLanguage.AddString( lpLang );
		m_ChangeLanguage.SetItemData( iPos, (DWORD_PTR)lpTriGram );
	}

	// Do a loop to find the current documentation language.
	m_strPrevDocLanguage = TASApp.GetDocLanguage();

	for( int i = 0; i < m_ChangeLanguage.GetCount(); i++ )
	{
		DWORD_PTR pDWORD = m_ChangeLanguage.GetItemData( i );
		LPCTSTR lpctstr = (LPCTSTR)pDWORD;
		CString str = m_strPrevDocLanguage;
		
		if( 0 == str.CompareNoCase( lpctstr ) )
		{
			iCurLang = i;
		}
	}
	
	// Select the current language.
	m_ChangeLanguage.SetCurSel( iCurLang );
}

void CDlgDocs::_FillTreeBox()
{
	// Init variables.
	std::map<int, CData*> Map;
	std::map<int, CData*>::iterator it;
	CDB_MultiString *pMStrLng = (CDB_MultiString *)TASApp.GetpTADB()->Get(_T("VERSION_LANGUAGE")).MP;
	int iLng = pMStrLng->GetNumofLines();
	BOOL BCheck = FALSE;
	
	// Empty the tree docs.
	m_TreeDoc.DeleteAllItems();
	
	// Get the map with all languages.
	CTableOrdered* pTab = (CTableOrdered*)( TASApp.GetpTADB()->Get( _T("LANG_TAB") ).MP );
	ASSERT( NULL != pTab );

	// Order the list.
	for( IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; IDPtr = pTab->GetNext() )
		Map.insert( std::pair<int, CData*>( _ttoi( ( (CDB_StringID*)IDPtr.MP )->GetIDstr() ), IDPtr.MP ) );

	// Get the current TADBKey.
	CString strKey = TASApp.GetTADBKey();
	strKey.MakeLower();

	// Fill the list with all languages and define the check box state.
	HTREEITEM hFirstVisibleItem = NULL;
	for( it = Map.begin(); it != Map.end();it++ )
	{
		LPCTSTR lpLang = ( (CDB_StringID*)it->second )->GetString();
		LPCTSTR lpTriGram = ( (CDB_StringID*)it->second )->GetIDstr2();
		CString strTriGram = lpTriGram;
		strTriGram.MakeLower();
		
		// Verify the language is compatible with the current tree docs structure linked to the current version.
		// Normal case : each version has the same tree structure, but the US,UK and AU version have special tree structure.
		/*
		if( L"au" == strKey )
		{
			if( L"en-us" == strTriGram )
				continue;
		}
		else if( L"us" == strKey )
		{
			if( L"en-us" != strTriGram )
				continue;
		}
		else
		{
			if( L"en-au" == strTriGram || L"en-us" == strTriGram )
				continue;
		}
		*/

		// No documentation available for Australia
		if( L"en-au" == strTriGram )
		{
			continue;
		}

		HTREEITEM hItem = m_TreeDoc.InsertItem( TVIF_TEXT | TVIF_PARAM | TVIF_STATE, lpLang, 0, 0, 0, 0, (LPARAM)lpTriGram, TVI_ROOT, TVI_LAST );
		if( true == m_bFirstOpening )
		{
			// Check only the language corresponding to the database version.
			for( int i = 0; i < iLng; i++ )
			{
				CString str = pMStrLng->GetString( i );
				if( 0 == str.CompareNoCase( lpTriGram ) )
				{
					BCheck = TRUE;
					if( NULL == hFirstVisibleItem )
						hFirstVisibleItem = hItem;
				}
			}
			m_TreeDoc.SetCheck( hItem, BCheck );
			BCheck = FALSE;
		}
		else
		{
			BCheck = (BOOL)::AfxGetApp()->GetProfileInt( _T("Documentation"), lpTriGram, 0 );
			m_TreeDoc.SetCheck( hItem, BCheck );
			if( NULL == hFirstVisibleItem && TRUE == BCheck )
				hFirstVisibleItem = hItem;
		}
	}
	
	if( hFirstVisibleItem != NULL )
		m_TreeDoc.EnsureVisible( hFirstVisibleItem );
}

bool CDlgDocs::_FindFolder( CString strCurrentPath, CString strFolderName, bool fRecursive )
{
	// Variables.
	bool fRet = false;
	CFileFind finder;
	CString str;	

	BOOL fFound = finder.FindFile( strCurrentPath + L"*.*" );
	
	while( TRUE == fFound && false == fRet )
	{
		fFound = finder.FindNextFile();
		if( TRUE == finder.IsDots() )
			continue;

		if( TRUE == finder.IsDirectory() )
		{
			// Verify the founded directory.
			str = finder.GetFileTitle();
			if( str.Find( strFolderName ) != -1 )
				return true;

			// Do the recursive function if necessary...
			if( true == fRecursive )
			{
				str = finder.GetFilePath() + L"\\";
				fRet = _FindFolder( str, strFolderName );
			}
		}		
	}
	finder.Close();
	
	return fRet;
}
