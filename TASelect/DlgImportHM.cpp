#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "HydroMod.h"
#include "TerminalUnit.h"
#include "DlgLeftTabProject.h"
#include "DlgTSPOpeningInfo.h"
#include "DlgImportHMProp.h"
#include "HMTreeDroptarget.h"
#include "HMTreeListCtrl.h"
#include "DlgInjectionError.h"
#include "DlgImportHM.h"


IMPLEMENT_DYNAMIC( CDlgImportHM, CDialogEx )

CDlgImportHM::CDlgImportHM( CWnd *pParent )
	: CDialogEx( CDlgImportHM::IDD, pParent )
{
	m_pDataStructToImport = NULL;
	m_eImportProjectType = ProjectType::InvalidProjectType;
	
	// HYS-1376 : Variables for unit tests.
	m_FileNameForUnitTests = _T("");
	m_pInputUnitTests = NULL;
	m_iArraySize = 0;
}

CDlgImportHM::CDlgImportHM( CString strFileNameToImport, CStringArray *pArrayParam, int iArraySize, CWnd *pParent )
	: CDialogEx( CDlgImportHM::IDD, pParent )
{
	m_pDataStructToImport = NULL;

	if( false == TASApp.IsUnitTest() )
	{
		m_FileNameForUnitTests = _T("");
		m_pInputUnitTests = NULL;
		m_iArraySize = 0;
	}
	else
	{
		// Input variables from unit test
		m_FileNameForUnitTests = strFileNameToImport;
		m_pInputUnitTests = pArrayParam;
		m_iArraySize = iArraySize;
	}
}

CDlgImportHM::~CDlgImportHM()
{
	if( NULL != m_pDataStructToImport )
	{
		delete m_pDataStructToImport;
	}
}

void CDlgImportHM::AddItemsUnitTests( HTREEITEM hItemParentSrc, HTREEITEM hItemParentDest )
{
	HTREEITEM hItem = NULL;
	HTREEITEM hItemDest = NULL;
	HTREEITEM hItemNew = NULL;

	// correspondance map for index where to insert and item parent and item before
	map<vector<HTREEITEM>, vector<HTREEITEM>> mapImportUnitTest;
	map<vector<HTREEITEM>, vector<HTREEITEM>>::iterator Itmap;

	if( false == TASApp.IsUnitTest() )
	{
		return;
	}

	if( NULL == hItemParentSrc )
	{
		// the root is "untitled"
		hItem = m_TreeProjToImport.GetRootItem();
		hItemDest = m_TreeProjCurrent.GetRootItem();
		int pos = 0;
		int nbEltFound = 0;
		
		// Find correspondance between input informations m_TreeProjToImport and m_CurrentProj.
		for( hItem = m_TreeProjToImport.GetChildItem( hItem ); hItem; hItem = m_TreeProjToImport.GetNextVisibleItem( hItem ) )
		{
			CString ItemToImport, ItemParent, ParentItemToInsert, ItemToInsertAfter;
			vector<HTREEITEM> ItemInfToImport;
			vector<HTREEITEM> ItemInfToInsert;
			
			if( nbEltFound == m_iArraySize - 1 )
			{
				break; // All elements are found
			}
			
			int i = 1;
			
			while( i < m_iArraySize )
			{
				CString strline = m_pInputUnitTests->ElementAt( i );
				
				if( false == strline.IsEmpty() )
				{
					int pos = 0;
					ItemToImport = strline.Tokenize( _T( "=>" ), pos );
					
					if( false == ItemToImport.IsEmpty() )
					{
						ItemToImport.Trim();
						
						if( 0 == ItemToImport.Compare( m_TreeProjToImport.GetItemText( hItem ) ) )
						{
							ItemParent = strline.Tokenize( _T( "=>" ), pos );
							ItemParent.Trim();

							ParentItemToInsert = strline.Tokenize( _T( "=>" ), pos );
							ParentItemToInsert.Trim();

							ItemToInsertAfter = strline.Tokenize( _T( "=>" ), pos );
							ItemToInsertAfter.Trim();

							ItemInfToImport.push_back( hItem );
							ItemInfToImport.push_back( m_TreeProjToImport.GetParentItem( hItem ) );
							break;
						}
					}
				}
				
				i++;
			}

			if( true == ItemInfToImport.empty() )
			{
				continue;
			}
			
			for( hItemDest = m_TreeProjCurrent.GetRootItem(); hItemDest; hItemDest = m_TreeProjCurrent.GetNextVisibleItem( hItemDest ) )
			{
				if( true == ItemToInsertAfter.IsEmpty() && ( 0 == ParentItemToInsert.Compare( m_TreeProjCurrent.GetItemText( hItemDest ) ) ) )
				{
					ItemInfToInsert.push_back( hItemDest );
					ItemInfToInsert.push_back( NULL );
					break;
				}

				if( false == ItemToInsertAfter.IsEmpty() && ( 0 == ItemToInsertAfter.Compare( m_TreeProjCurrent.GetItemText( hItemDest ) ) ) )
				{
					ItemInfToInsert.push_back( m_TreeProjCurrent.GetParentItem( hItemDest ) );
					ItemInfToInsert.push_back( hItemDest );
					break;
				}
			}

			if( false == ItemInfToInsert.empty() )
			{
				mapImportUnitTest.insert( pair<vector<HTREEITEM>, vector<HTREEITEM>>( ItemInfToImport, ItemInfToInsert ) );
				nbEltFound++;
			}
		}

		// Reinitialize variables
		hItem = m_TreeProjToImport.GetRootItem();
		hItemDest = m_TreeProjCurrent.GetRootItem();

		Itmap = mapImportUnitTest.begin();
	}
	else
	{
		hItem = hItemParentSrc;
	}

	// Insert in m_TreeCurrentProj
	for( hItemNew = m_TreeProjToImport.GetChildItem( hItem ) ; hItemNew; hItemNew = m_TreeProjToImport.GetNextVisibleItem( hItemNew ) )
	{
		if( NULL != hItemParentSrc && m_TreeProjToImport.GetParentItem( hItemNew ) != hItemParentSrc )
		{
			continue;
		}
		
		if( NULL == hItemParentSrc && Itmap == mapImportUnitTest.end() )
		{
			break;
		}
		
		if( ( NULL == hItemParentSrc && Itmap->first.at(0) == hItemNew ) || NULL != hItemParentSrc )
		{
			int nImage = 0;
			int nSelImage = 0;
			CImageList *pImageList = m_TreeProjToImport.GetImageList( TVSIL_NORMAL );

			// If no image list is associated with the tree control, return.
			if( NULL == pImageList )
			{
				nImage = 0;
				nSelImage = 0;
			}
			else
			{
				// If no image list is associated with Destination tree control.
				// Set the image list of source tree control.
				if( NULL == m_TreeProjCurrent.GetImageList( TVSIL_NORMAL ) )
				{
					m_TreeProjCurrent.SetImageList( pImageList, TVSIL_NORMAL );
				}

				// Get the image indexes.
				m_TreeProjToImport.GetItemImage( hItemNew, nImage, nSelImage );
			}

			CHMTreeListCtrl::m_UserData *pUD = (CHMTreeListCtrl::m_UserData *)m_TreeProjToImport.GetUserData( hItemNew );

			if( hItemParentSrc == NULL && Itmap->second.at(1) == NULL )
			{
				HTREEITEM hTDropItem = m_TreeProjCurrent.GetFirstItem();
				HTREEITEM ItemDest = m_TreeProjCurrent.AddItem( hTDropItem, m_TreeProjToImport.GetItemText( hItemNew ), TVI_LAST, nImage, nSelImage, m_TreeProjToImport.GetItemData( hItemNew ), pUD, DROPEFFECT_COPY );
				Itmap++;
				
				if( NULL != m_TreeProjToImport.GetChildItem( hItemNew ) )
				{
					// recursive call for children, they will added directly without any conditions
					AddItemsUnitTests( hItemNew, ItemDest );
				}
			}
			else if( hItemParentSrc != NULL )
			{
				HTREEITEM ItemDest;
				ItemDest = m_TreeProjCurrent.AddItem( hItemParentDest, m_TreeProjToImport.GetItemText( hItemNew ), TVI_LAST, nImage, nSelImage, m_TreeProjToImport.GetItemData( hItemNew ), pUD, DROPEFFECT_COPY );
				
				if( NULL != m_TreeProjToImport.GetChildItem( hItemNew ) )
				{
					// recursive call for children, they will added directly without any conditions
					AddItemsUnitTests( hItemNew, ItemDest );
				}
			}
			else
			{
				// Define the parent Item and after which item the item we have to insert.
				HTREEITEM hParent = NULL;
				HTREEITEM hInsAfter = NULL;
				
				if( hItemParentSrc == NULL && Itmap != mapImportUnitTest.end() )
				{
					hParent = ( Itmap->second).at( 0 );
					hInsAfter = ( Itmap->second).at( 1 );
					Itmap++;
				}
				else
				{
					ASSERT( 0 );
				}

				if( NULL == hInsAfter )
				{
					hInsAfter = TVI_FIRST;
				}
				
				HTREEITEM ItemDest;
				ItemDest = m_TreeProjCurrent.AddItem( hParent, m_TreeProjToImport.GetItemText( hItemNew ), hInsAfter, nImage, nSelImage, m_TreeProjToImport.GetItemData( hItemNew ), pUD, DROPEFFECT_COPY );

				if( NULL != m_TreeProjToImport.GetChildItem( hItemNew ) )
				{
					// appel recursive pour les enfants boucles sur les enfants
					AddItemsUnitTests( hItemNew, ItemDest );
				}
			}
		}
	}

	if( NULL == hItemParentSrc ) // Import HM and computeAll
	{
		ImportHMRecursivelly();

		if( NULL != pDlgLeftTabProject )
		{
			pDlgLeftTabProject->ResetTree();
		}

		// Compute all the installation to fix the name into Ref1.
		// Remark: Do compute only if we are in the hydronic circuit calculation mode and the project is not frozen.
		CDS_ProjectParams* pPrjParam = TASApp.GetpTADS()->GetpProjectParams();

		if( NULL != pPrjParam && NULL != pMainFrame && true == pMainFrame->IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
		{
			TASApp.GetpTADS()->ComputeAllInstallation();
		}
	}
}

BEGIN_MESSAGE_MAP( CDlgImportHM, CDialogEx )
	ON_MESSAGE( WM_USER_HMTREEITEMINSERTED, OnItemAdded )
	ON_MESSAGE( WM_USER_HMTREEMODIFIED, UpdateBtns )
	ON_NOTIFY( NM_CLICK, IDC_CURRPROJTREE, OnNMClickTree )
	ON_NOTIFY( TVN_GETINFOTIP, IDC_CURRPROJTREE, OnTvnGetInfoTip )
	ON_BN_CLICKED( IDC_BUTTONOPENPROJ, OnBnClickedOpenProject )
	ON_BN_CLICKED( IDC_BUTTONIMPORTPROP, OnBnClickedImportProject )
	ON_BN_CLICKED( IDOK, OnBnClickedOk )
	ON_BN_CLICKED( IDC_BTNEXPANDTREE1, OnBnClickedExpandTree1 )
	ON_BN_CLICKED( IDC_BTNCOLLAPSETREE1, OnBnClickedCollapseTree1 )
	ON_BN_CLICKED( IDC_BTNEXPANDTREE2, OnBnClickedExpandTree2 )
	ON_BN_CLICKED( IDC_BTNCOLLAPSETREE2, OnBnClickedBtnCollapseTree2 )
END_MESSAGE_MAP()

void CDlgImportHM::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_BUTTONOPENPROJ, m_BtnOpenProj );
	DDX_Control( pDX, IDC_EDITINFO, m_EditInfo );
	DDX_Control( pDX, IDC_STATICEXPPROJ, m_StaticExtProj );
	DDX_Control( pDX, IDC_STATICCURRPROJ, m_StaticCurrProj );
	DDX_Control( pDX, IDOK, m_BtnImport );
	DDX_Control( pDX, IDC_BTNEXPANDTREE1, m_BtnExpandTree1 );
	DDX_Control( pDX, IDC_BTNCOLLAPSETREE1, m_BtnCollapseTree1 );
	DDX_Control( pDX, IDC_BTNEXPANDTREE2, m_BtnExpandTree2 );
	DDX_Control( pDX, IDC_BTNCOLLAPSETREE2, m_BtnCollapseTree2 );
}

BOOL CDlgImportHM::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	// Initial strings.
	CString str = TASApp.LoadLocalizedString( IDS_DLGIMPORTHM_CAPTION );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGIMPORTHM_EXTPROJ );
	m_StaticExtProj.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGIMPORTHM_CURRPROJ );
	m_StaticCurrProj.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGIMPORTHM_INFO );
	m_EditInfo.SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_DLGIMPORTHM_OPENPROJ );
	m_BtnOpenProj.SetWindowTextW( str );

	// Set bitmap into the buttons.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );
	
	if( NULL != pclImgListButton )
	{
		m_BtnExpandTree1.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_ExpandAllTree ) );
		m_BtnCollapseTree1.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_CollapseAllTree ) );
		m_BtnExpandTree2.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_ExpandAllTree ) );
		m_BtnCollapseTree2.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_CollapseAllTree ) );
	}

	// Set tooltips into the buttons.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BTNEXPANDTREE1 ), TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTEXTEND ) );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BTNEXPANDTREE2 ), TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTEXTEND ) );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BTNCOLLAPSETREE1 ), TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTCOLLAPSE ) );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BTNCOLLAPSETREE2 ), TASApp.LoadLocalizedString( IDS_DLGPRINT_BUTCOLLAPSE ) );

	// Build and attach an image list to m_Tree.
	m_TreeImageList.CreateTC( IDB_IMGLST_HMTREE, 16, 16, _BLACK );
	m_TreeImageList.SetBkColor( CLR_NONE );

	// Set the style and position correctly the m_TreeExtProj.
	CRect rect;
	GetDlgItem( IDC_FRAME_EXTPROJ )->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_TreeProjToImport.Create( WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS, rect, this, IDC_EXTPROJTREE );
	m_TreeProjToImport.SetExtendedStyle( TVS_EX_HIDEHEADERS );
	m_TreeProjToImport.SetImageList( &m_TreeImageList, TVSIL_NORMAL );

	// User data.
	m_TreeProjToImport.SetUserDataSize( sizeof( CHMTreeListCtrl::m_UserData ) );

	// Set the style and position correctly the m_TreeCurrProj.
	GetDlgItem( IDC_FRAME_CURRPROJ )->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_TreeProjCurrent.Create( WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_INFOTIP | TVS_LINESATROOT | TVS_HASBUTTONS, rect, this, IDC_CURRPROJTREE );
	m_TreeProjCurrent.SetExtendedStyle( TVS_EX_HIDEHEADERS | TVS_EX_TOOLTIPNOTIFY );
	m_TreeProjCurrent.SetImageList( &m_TreeImageList, TVSIL_NORMAL );
	m_TreeProjCurrent.SetProperDragDrop( true );
	m_TreeProjCurrent.SetFullDragDrop( false );

	// Split the tree in two columns.
	m_TreeProjCurrent.InsertColumn( 0, L"" );
	m_TreeProjCurrent.InsertColumn( 1, L"", TVCFMT_LEFT, 16 );

	// User Data.
	m_TreeProjCurrent.SetUserDataSize( sizeof( CHMTreeListCtrl::m_UserData ) );

	// Fill the current project tree view.
	CTable *pTab = TASApp.GetpTADS()->GetpHydroModTable();
	ASSERT( NULL != pTab );
	
	CDS_ProjectRef *pPrjRef = TASApp.GetpTADS()->GetpProjectRef();
	ASSERT( NULL != pPrjRef );
	
	if( NULL != pTab && NULL != pPrjRef )
	{
		m_TreeProjCurrent.FillHMRoot( pTab,pPrjRef->GetString( CDS_ProjectRef::Name ) );
		m_TreeProjCurrent.Expand( m_TreeProjCurrent.GetRootItem(), TVE_EXPAND );
		m_eCurrentProjectType = TASApp.GetpTADS()->GetpTechParams()->GetProjectApplicationType();
 	}

	// Open a project for import.
	OnBnClickedOpenProject();

	UpdateBtns();

	// Force to center the window, if the DlgTSPOpeningInfo is displayed the DlgImportHM is not centered.
	CenterWindow();

	return TRUE;
}

LRESULT CDlgImportHM::OnItemAdded( WPARAM wParam, LPARAM lParam )
{
	CString SectionName = _T("DlgImportHMProp");

	// Recuperate the parameters.
	DROPEFFECT dropEffect = (DROPEFFECT)wParam;
	HTREEITEM hItem = (HTREEITEM)lParam;

	// Recuperate information on the registry about the lock properties in case it's a copy.
	m_UserData *pUD = (m_UserData *)m_TreeProjCurrent.GetUserData( hItem );
	
	if( DROPEFFECT_COPY == dropEffect )
	{
		pUD->bDistPipes	= ::AfxGetApp()->GetProfileInt( SectionName, L"DistPipes", 0 ) ? true : false;
		pUD->bCircPipes	= ::AfxGetApp()->GetProfileInt( SectionName, L"CircPipes", 0 ) ? true : false;
		pUD->bBv		= ::AfxGetApp()->GetProfileInt( SectionName, L"Bv", 0 ) ? true : false;
		pUD->bBvByp		= ::AfxGetApp()->GetProfileInt( SectionName, L"BvByp", 0 ) ? true : false;
		pUD->bDpC		= ::AfxGetApp()->GetProfileInt( SectionName, L"DpC", 0 ) ? true :false;
		pUD->bCv		= ::AfxGetApp()->GetProfileInt( SectionName, L"Cv", 0 ) ? true : false;
	}

	// Determine the padlocked to set.
	int iHMTreeImage;
	
	if( true == pUD->bDistPipes && true == pUD->bCircPipes && true == pUD->bBv && true == pUD->bBvByp && true == pUD->bDpC && true == pUD->bCv )
	{
		iHMTreeImage = CRCImageManager::ILHM_PadLockRed;
	}
	else if( false == pUD->bDistPipes && false == pUD->bCircPipes && false == pUD->bBv && false == pUD->bBvByp && false == pUD->bDpC && false == pUD->bCv )
	{
		iHMTreeImage = CRCImageManager::ILHM_PadLockYellow;
	}
	else
	{
		iHMTreeImage = CRCImageManager::ILHM_PadLockOrange;
	}
	
	// Add the padlocked associated to imported properties.
	m_TreeProjCurrent.SetItem( hItem, 1, TVIF_IMAGE, L"", iHMTreeImage, iHMTreeImage, 0, 0 );

	// Redefine the item text.
	CString str = L"*x.x(" + pUD->pHM->GetHMName() + L")";
	m_TreeProjCurrent.SetItemText( hItem, str );
	
	return 0;
}

LRESULT CDlgImportHM::UpdateBtns( WPARAM wParam, LPARAM lParam )
{
	bool bHMImported = IsHMImported();
	m_BtnImport.EnableWindow( ( true == bHMImported ) ? TRUE : FALSE );
	m_BtnOpenProj.EnableWindow( ( false == bHMImported ) ? TRUE : FALSE );

	return 0;
}

void CDlgImportHM::OnNMClickTree( NMHDR *pNMHDR, LRESULT *pResult )
{
	TVHITTESTINFO ht = { 0 };
	DWORD dwPos = GetMessagePos();
	ht.pt.x= ( (int)(short)LOWORD( dwPos ) );				// GET_X_LPARAM(dwpos);
	ht.pt.y= ( (int)(short)HIWORD( dwPos ) );				// GET_Y_LPARAM(dwpos);

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( 0x01200000 == ht.flags )
	{
		// Recuperate the parameters from the hItem and send them to the CDlgImportHMProp dialog box.
		m_UserData *pUD = (m_UserData *)m_TreeProjCurrent.GetUserData( ht.hItem );
		CDlgImportHMProp::sCheckBoxState sCBState;
		sCBState.bUseCheckBoxState = true;

		sCBState.bDistPipes = pUD->bDistPipes;
		sCBState.bCircPipes = pUD->bCircPipes;
		sCBState.bBv		= pUD->bBv;
		sCBState.bBvByp		= pUD->bBvByp;
		sCBState.bDpC		= pUD->bDpC;
		sCBState.bCv		= pUD->bCv;

		CDlgImportHMProp Dlg( sCBState );
		sCBState = Dlg.Display();
		
		pUD->bDistPipes = sCBState.bDistPipes;
		pUD->bCircPipes = sCBState.bCircPipes;
		pUD->bBv		= sCBState.bBv;
		pUD->bBvByp		= sCBState.bBvByp;
		pUD->bDpC		= sCBState.bDpC;
		pUD->bCv		= sCBState.bCv;

		// Determine the padlocked to set.
		int iHMTreeImage;

		if( true == pUD->bDistPipes && true == pUD->bCircPipes && true == pUD->bBv && true == pUD->bBvByp && true == pUD->bDpC && true == pUD->bCv )
		{
			iHMTreeImage = CRCImageManager::ILHM_PadLockRed;
		}
		else if( false == pUD->bDistPipes && false == pUD->bCircPipes && false == pUD->bBv && false == pUD->bBvByp && false == pUD->bDpC && false == pUD->bCv )
		{
			iHMTreeImage = CRCImageManager::ILHM_PadLockYellow;
		}
		else
		{
			iHMTreeImage = CRCImageManager::ILHM_PadLockOrange;
		}
		
		// Add the padlocked associated to imported properties.
		m_TreeProjCurrent.SetItem( ht.hItem, 1, TVIF_IMAGE, L"", iHMTreeImage, iHMTreeImage, 0, 0 );
		
	}
	
	*pResult = 0;
}

void CDlgImportHM::OnTvnGetInfoTip( NMHDR *pNMHDR, LRESULT *pResult )
{
	*pResult = 0;
}

void CDlgImportHM::OnBnClickedOpenProject()
{
	BeginWaitCursor();

	CTADatastruct *pDataStruct = new CTADatastruct();
	
	// HYS-1376 : For unit test open the specific file.
	CString path = CteEMPTY_STRING;

	if( true == TASApp.IsUnitTest() )
	{
		path = TASApp.FileOpen( pDataStruct, m_FileNameForUnitTests );
	}
	else
	{
		path = TASApp.FileOpen( pDataStruct, NULL );
	}
	
	if( false == path.IsEmpty() )
	{
		if( NULL != m_pDataStructToImport )
		{
			delete m_pDataStructToImport;
		}

		m_pDataStructToImport = pDataStruct;

		// Display a dialog box with information on the *.tsp file for deleted and not available valves.

		// HYS-1583: even in unit test mode we need to launch this dialog because there is a verification on the
		// individual selection and HM calc if exist. And we can have errors. For example, the PR 6000 Filter product 
		// has been removed from the database but not put in the deleted file. So when opening in debug mode there is
		// an ASSERT in the 'DCDlgTSPOpeningInfo' dialog. We need also this ASSERT when running unit test.
		CDlgTSPOpeningInfo dlg;
		dlg.Display( TASApp.IsUnitTest() );

		// Fill the selected TSP file in the tree.
		CTable *pTab = m_pDataStructToImport->GetpHydroModTable();
		ASSERT( NULL != pTab );

		CDS_ProjectRef *pPrjRef = m_pDataStructToImport->GetpProjectRef();
		ASSERT( NULL != pPrjRef );

		if( NULL != pTab && NULL != pPrjRef )
		{
			m_TreeProjToImport.FillHMRoot( pTab, pPrjRef->GetString( CDS_ProjectRef::Name ) );
			m_TreeProjToImport.Expand( m_TreeProjCurrent.GetRootItem(), TVE_EXPAND );
			m_eImportProjectType = m_pDataStructToImport->GetpTechParams()->GetProjectApplicationType();
		}
	}
	else
	{
		delete pDataStruct;
		m_eImportProjectType = ProjectType::InvalidProjectType;
	}

	EndWaitCursor();
}

void CDlgImportHM::OnBnClickedImportProject()
{
	CDlgImportHMProp::sCheckBoxState sCBState;
	sCBState.bUseCheckBoxState = false;
	
	CDlgImportHMProp Dlg( sCBState );
	Dlg.DoModal();
}

void CDlgImportHM::OnBnClickedOk()
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpHydroModTable() || NULL == dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() ) )
	{
		ASSERT_RETURN;
	}

	CTableHM *pclHydraulicNetwork = (CTableHM *)TASApp.GetpTADS()->GetpHydroModTable();
	std::vector<CDS_HydroMod *> vecErrorList;
	CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode = CDlgInjectionError::ReturnCode::Undefined;
	
	if( true == IsSecondaryTemperatureErrors( NULL, &vecErrorList ) )
	{
		// Show the dialog to ask user if he wants to apply this import without changing all the injection circuits in errors, or if wants to apply and 
		// automatically correct the errors or if he wants to cancel.
		CDlgInjectionError DlgInjectionError( &vecErrorList, false );
		eDlgInjectionErrorReturnCode = (CDlgInjectionError::ReturnCode)DlgInjectionError.DoModal();

		if( CDlgInjectionError::ReturnCode::Cancel == eDlgInjectionErrorReturnCode )
		{
			return;
		}

		// The correction will be applied at each import below.
	}

	BeginWaitCursor();
	
	// Switch to project view.
	if( NULL != pDlgLTtabctrl )
	{
		pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiProj );
	}

	// Loop on the tree and add new HM.
	bool bStatus = ImportHMRecursivelly( NULL, NULL, eDlgInjectionErrorReturnCode );
	
	if (false == bStatus )
	{
		return;
	}
	
	// Reset the upper and lower limits for measurements.
	if( NULL != pMainFrame )
	{
		pMainFrame->ResetMeasurementsLimits();
	}

	if( NULL != pDlgLeftTabProject )
	{
		pDlgLeftTabProject->ResetTree();
	}
	
	// Compute all the installation to fix the name into Ref1.
	// Remark: Do compute only if we are in the hydronic circuit calculation mode and the project is not frozen.
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL != pPrjParam && NULL != pMainFrame && true == pMainFrame->IsHMCalcChecked() && false == pPrjParam->IsFreezed() )
	{
		TASApp.GetpTADS()->ComputeAllInstallation();
	}

	EndWaitCursor();
	CDialog::OnOK();	
}

void CDlgImportHM::OnBnClickedExpandTree1()
{
	m_TreeProjToImport.ExpandRecursive( m_TreeProjToImport.GetRootItem(), TVE_EXPAND );
}

void CDlgImportHM::OnBnClickedCollapseTree1()
{
	m_TreeProjToImport.ExpandRecursive( m_TreeProjToImport.GetRootItem(), TVE_COLLAPSE );
	m_TreeProjToImport.Expand( m_TreeProjToImport.GetRootItem(), TVE_EXPAND );
}

void CDlgImportHM::OnBnClickedExpandTree2()
{
	m_TreeProjCurrent.ExpandRecursive( m_TreeProjCurrent.GetRootItem(), TVE_EXPAND );
}

void CDlgImportHM::OnBnClickedBtnCollapseTree2()
{
	m_TreeProjCurrent.ExpandRecursive( m_TreeProjCurrent.GetRootItem(), TVE_COLLAPSE );
	m_TreeProjCurrent.Expand( m_TreeProjCurrent.GetRootItem(), TVE_EXPAND );
}

bool CDlgImportHM::ImportHMRecursivelly( HTREEITEM hItem, CDS_HydroMod *pHMParent, CDlgInjectionError::ReturnCode eDlgInjectionErrorReturnCode )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpHydroModTable() || NULL == dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() ) )
	{
		ASSERTA_RETURN( false );
	}

	CTableHM *pclHydraulicNetwork = (CTableHM *)TASApp.GetpTADS()->GetpHydroModTable();

	if( NULL == hItem )
	{
		hItem = m_TreeProjCurrent.GetRootItem();
	}

	// HYS-1359.
	bool bReturn = true;
	int iPos = 0;

	for( hItem = m_TreeProjCurrent.GetChildItem( hItem ); NULL != hItem; hItem = m_TreeProjCurrent.GetNextSiblingItem( hItem ) )
	{
		m_UserData *pUD = (m_UserData *)m_TreeProjCurrent.GetUserData( hItem );
		iPos++;
		
		// Do the recursive function.
		if( NULL != m_TreeProjCurrent.GetChildItem( hItem ) && false == pUD->bImported )
		{
			// HYS-1359 : Check the return value.
			bReturn = ImportHMRecursivelly( hItem, NULL, eDlgInjectionErrorReturnCode );

			if( false == bReturn )
			{
				break;
			}
		}
		
		// Verify the HM must be added.
		if( false == pUD->bImported )
		{
			continue;
		}
		
		// Recuperate the parent if it doesn't exist yet.
		if( NULL == pHMParent )
		{
			HTREEITEM hParentItem = m_TreeProjCurrent.GetParentItem( hItem );
			pHMParent = (CDS_HydroMod *)m_TreeProjCurrent.GetItemData( hParentItem );
		}

		// HYS-1359 : Look if this pUD->pHM can be imported.
		bool bCanbeImported = true; // The element can be imported successfully.
		bool bMustBeCompleted = false; // The element must be completed to allow the conversion in HMCalc mode.

		// HYS-1397 : Like the copy-paste if the circuit is inserted at the same level than the root we cannot cast
		// pHMParent to CDS_HydroMod.
		CTableHM *pTableHM = NULL;
		CDS_HydroMod *pCHildHM = NULL;

		if( 0 == ( (CString)pHMParent->GetClassName() ).Compare( _T("CTableHM") ) )
		{
			pTableHM = (CTableHM *)pHMParent;
		}
		else
		{
			pCHildHM = (CDS_HydroMod *)( pHMParent->GetFirst().MP );
		}

		// To know the return mode of elements in the same level.
		if( NULL != pCHildHM && pUD->pHM->GetReturnType() != pCHildHM->GetReturnType() )
		{
			// In the same level we have to get the same return mode.
			bCanbeImported = false;
		}
		
		if( NULL != pCHildHM && pUD->pHM->IsHMCalcMode() != pCHildHM->IsHMCalcMode() )
		{
			// In the current project we try to import elements with a different HMCalcMode.
			if( true == pCHildHM->IsHMCalcMode() )
			{
				bMustBeCompleted = true; // Must be completed and converted.
			}
		}
		else if( pMainFrame->IsHMCalcChecked() != pUD->pHM->IsHMCalcMode() )
		{
			// There is no project but the MainFrame HMCalc mode is different than the mode of the element to be imported.
			if( true == pMainFrame->IsHMCalcChecked() )
			{
				bMustBeCompleted = true; // Must be completed and converted.
			}
		}

		if( false == bCanbeImported )
		{
			// The element to be imported is not compatible regarding to the return mode.
			CString str = TASApp.LoadLocalizedString( IDS_RVHMCALC_INCOMPATIBLERETURNMODEIMPORT );

			if( IDNO == ::AfxMessageBox( (LPCTSTR)str, MB_YESNO | MB_ICONWARNING ) )
			{
				bReturn = false;
				break;
			}
			
			pUD->pHM->SetReturnType( pCHildHM->GetReturnType() );
		}

		// Import the new HM.
		// Children are not imported? This is why below we goo deeper in the tree if children exist.
		CDS_HydroMod *pNewHM = pHMParent->ImportHydroMod( pUD->pHM, iPos );
		
		// HYS-1359 : If the element is not in HMCalc mode complete it and convert it.
		if( true == bMustBeCompleted )
		{
			if( true == TASApp.GetpTADS()->IsOneValveNotDefine( pHMParent ) )
			{
				CString str = TASApp.LoadLocalizedString( AFXMSG_ONE_VALVE_PARTIALLYDEF );
				AfxMessageBox( (LPCTSTR)str );
				bReturn = false;
				break;
			}
			
			TASApp.GetpTADS()->CompleteProjForHydroCalcMode( pHMParent, false );
		}
		
		// Define the lock positions.
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eDistributionSupplyPipe, pUD->bDistPipes, true, true );
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eDistributionReturnPipe, pUD->bDistPipes, true, true );
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eCircuitPrimaryPipe, pUD->bCircPipes, true, true );
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eCircuitSecondaryPipe, pUD->bCircPipes, true, true );
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eBVprim, pUD->bBv, true, true );
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eBVsec, pUD->bBv, true, true );
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eBVbyp, pUD->bBvByp, true, true );
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eDpC, pUD->bDpC, true, true );
		pNewHM->SetLock( CDS_HydroMod::eHMObj::eCV, pUD->bCv, true, true );

		// HYS-1716: Now that the circuit has been imported, we can correct it if there were secondary temperatures errors on it.
		if( CDlgInjectionError::ReturnCode::ApplyWithCorrection == eDlgInjectionErrorReturnCode && true == pUD->bIsSecondaryTemperaturesError 
				&& true == pNewHM->IsInjectionCircuit() )
		{
			bool bProjectTypeDifferent = ( m_eCurrentProjectType != m_eImportProjectType ) ? true : false;
			pclHydraulicNetwork->CorrectOneInjectionCircuit( pNewHM, pHMParent, bProjectTypeDifferent );
			pNewHM->SetHMPrimAndSecFlow( pNewHM->GetpTermUnit()->GetQ() );
		}
		
		// Do the recursive function with new HM.
		if( m_TreeProjCurrent.GetChildItem( hItem ) )
		{
			bReturn = ImportHMRecursivelly( hItem, pNewHM, eDlgInjectionErrorReturnCode );
			
			if( false == bReturn )
			{
				break;
			}
		}
	}

	return bReturn;
}

bool CDlgImportHM::IsHMImported( HTREEITEM hItem )
{
	if( NULL == hItem )
	{
		hItem = m_TreeProjCurrent.GetRootItem();
	}

	for( hItem = m_TreeProjCurrent.GetChildItem( hItem ); NULL != hItem; hItem = m_TreeProjCurrent.GetNextSiblingItem( hItem ) )
	{
		m_UserData *pUD = (m_UserData *)m_TreeProjCurrent.GetUserData( hItem );

		if( true == pUD->bImported)
		{
			return true;
		}
		
		// Do the recursive function.
		if( m_TreeProjCurrent.GetChildItem( hItem ) )
		{
			if( true == IsHMImported( hItem ) )
			{
				return true;
			}
		}
	}

	return false;
}

bool CDlgImportHM::IsSecondaryTemperatureErrors( HTREEITEM hCurrentItem, std::vector<CDS_HydroMod *> *pvecErrorList )
{
	// See 'DlgImportHM.h' for details about this method.

	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpHydroModTable() || NULL == dynamic_cast<CTableHM *>( TASApp.GetpTADS()->GetpHydroModTable() ) )
	{
		ASSERTA_RETURN( false );
	}

	CTableHM *pclHydraulicNetwork = (CTableHM *)TASApp.GetpTADS()->GetpHydroModTable();

	bool bAtLeastOneError = false;

	if( NULL == hCurrentItem )
	{
		hCurrentItem = m_TreeProjCurrent.GetRootItem();

		// Run all roots
		for( HTREEITEM hChildItem = m_TreeProjCurrent.GetChildItem( hCurrentItem ); NULL != hChildItem; hChildItem = m_TreeProjCurrent.GetNextSiblingItem( hChildItem ) )
		{
			bAtLeastOneError |= IsSecondaryTemperatureErrors( hChildItem, pvecErrorList );
		}
	}
	else
	{
		// Check first the current HTREEITEM.
		m_UserData *pCurrentUserData = (m_UserData *)m_TreeProjCurrent.GetUserData( hCurrentItem );

		if( true == pCurrentUserData->bImported )
		{
			HTREEITEM hParentItem = m_TreeProjCurrent.GetParentItem( hCurrentItem );
			CTable *pclDirectParentHM = NULL;

			if( NULL != hParentItem )
			{
				m_UserData *pParentUserData = (m_UserData *)m_TreeProjCurrent.GetUserData( hParentItem );

				if( NULL != pParentUserData )
				{
					pclDirectParentHM = pParentUserData->pHM;
				}
			}

			// If the current item is not physically the child of the current parent in the tree we must verify temperatures because it is 
			// not yet already done.
			if( NULL == pclDirectParentHM || pclDirectParentHM != pCurrentUserData->pHM->GetParent() )
			{
				CTable *pclParentForVerificationHM = NULL;
				HTREEITEM hParentForVerifivation = GetParentNotInError( hCurrentItem );

				if( NULL != hParentForVerifivation )
				{
					m_UserData *pParentForVerificationUserData = (m_UserData *)m_TreeProjCurrent.GetUserData( hParentForVerifivation );

					if( NULL != pParentForVerificationUserData )
					{
						pclParentForVerificationHM = pParentForVerificationUserData->pHM;
					}
				}

				bool bProjectTypeDifferent = ( m_eCurrentProjectType != m_eImportProjectType ) ? true : false;

				if( true == pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForPasteOperation( pCurrentUserData->pHM, pclParentForVerificationHM, NULL, pvecErrorList, bProjectTypeDifferent ) )
				{
					SetTemperatureErrorFlag( pvecErrorList );
					bAtLeastOneError |= true;
				}
			}
		}

		for( HTREEITEM hChildItem = m_TreeProjCurrent.GetChildItem( hCurrentItem ); NULL != hChildItem; hChildItem = m_TreeProjCurrent.GetNextSiblingItem( hChildItem ) )
		{
			m_UserData *pChildUserData = (m_UserData *)m_TreeProjCurrent.GetUserData( hChildItem );
		
			// Do the recursive function if the current item is a module and it's not an imported item.
			if( NULL != m_TreeProjCurrent.GetChildItem( hChildItem ) && false == pChildUserData->bImported )
			{
				bAtLeastOneError |= IsSecondaryTemperatureErrors( hChildItem, pvecErrorList );
			}
		
			// Verify if this current circuit must be imported.
			if( false == pChildUserData->bImported )
			{
				continue;
			}

			if( NULL == m_TreeProjCurrent.GetChildItem( hChildItem ) )
			{
				// Retrieve the item parent in the destination tree.
				HTREEITEM hParentItem = m_TreeProjCurrent.GetParentItem( hChildItem );
				CTable *pclDirectParentHM = NULL;

				if( NULL != hParentItem )
				{
					// Retrieve the pointer on the 'CDS_HydroMod' object that is in this parent in the destination tree.
					m_UserData *pParentUserData = (m_UserData *)m_TreeProjCurrent.GetUserData( hParentItem );

					if( NULL != pParentUserData )
					{
						pclDirectParentHM = pParentUserData->pHM;
					}
				}

				// If the current item is not physically the child of the current parent in the tree we must verify temperatures because it is 
				// not yet already done.
				if( NULL == pclDirectParentHM || pclDirectParentHM != pChildUserData->pHM->GetParent() )
				{
					CTable *pclParentForVerificationHM = NULL;
					HTREEITEM hParent = GetParentNotInError( hChildItem );

					if( NULL != hParent )
					{
						m_UserData *pParentVorVerificationUserData = (m_UserData *)m_TreeProjCurrent.GetUserData( hParentItem );

						if( NULL != pParentVorVerificationUserData )
						{
							pclParentForVerificationHM = pParentVorVerificationUserData->pHM;
						}
					}
					
					bool bProjectTypeDifferent = ( m_eCurrentProjectType != m_eImportProjectType ) ? true : false;

					if( true == pclHydraulicNetwork->IsInjectionCircuitTemperatureErrorForPasteOperation(  pChildUserData->pHM, pclParentForVerificationHM, NULL, pvecErrorList, bProjectTypeDifferent ) )
					{
						SetTemperatureErrorFlag( pvecErrorList );
						bAtLeastOneError |= true;
					}
				}
			}
			else
			{
				// Now, if the current item verified is a module, we need to go deeper. As described in the 'DlgImportHM.h', we can have child in the 
				// current item with no link with it. So, we need to pass also children and manage those that are not physically belong to the current item.
				if( NULL != m_TreeProjCurrent.GetChildItem( hChildItem ) )
				{
					bAtLeastOneError |= IsSecondaryTemperatureErrors( hChildItem, pvecErrorList );
				}
			}
		}
	}

	return bAtLeastOneError;
}

HTREEITEM CDlgImportHM::GetParentNotInError( HTREEITEM hItem )
{
	// See 'DlgImportHM.h' for details about this method.

	bool bFind = false;
	HTREEITEM hParentItem = m_TreeProjCurrent.GetParentItem( hItem );

	while( NULL != hParentItem && false == bFind )
	{
		m_UserData *pParentUserData = (m_UserData *)m_TreeProjCurrent.GetUserData( hParentItem );

		if( ( true == pParentUserData->bImported && false == pParentUserData->bIsSecondaryTemperaturesError )
				|| false == pParentUserData->bImported )
		{
			bFind = true;
		}
		else
		{
			hParentItem = m_TreeProjCurrent.GetParentItem( hParentItem );
		}
	}

	return hParentItem;
}

void CDlgImportHM::SetTemperatureErrorFlag( std::vector<CDS_HydroMod *> *pvecErrorList )
{
	if( NULL == pvecErrorList )
	{
		return;
	}

	for( auto &iter : *pvecErrorList )
	{
		m_UserData *pUserData = (m_UserData *)m_TreeProjCurrent.GetUserData( (HTREEITEM)-iter->GetPos() );
		pUserData->bIsSecondaryTemperaturesError = true;
	}
}