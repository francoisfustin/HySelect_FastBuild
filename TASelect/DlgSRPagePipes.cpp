#include "stdafx.h"
#include "TASelect.h"
#include "HydroMod.h"
#include "HMPipes.h"
#include "HMTreeDroptarget.h"
#include "DlgSearchReplace.h"
#include "DlgReplacePopup.h"
#include "DlgSRPagePipes.h"


IMPLEMENT_DYNAMIC( CDlgSRPagePipes, CDlgCtrlPropPage )

CDlgSRPagePipes::CDlgSRPagePipes( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_PAGENAME );
	m_pTree = NULL;
	m_PipeList.PurgeAll();
	m_lparam = NULL;
	m_lparamCurrent = NULL;
	m_strStaticFind1 = L"";
	m_strStaticFind2 = L"";
	m_strStaticReplace = L"";
	m_pDlgReplacePopup = NULL;
}

bool CDlgSRPagePipes::OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams )
{
	m_PipeList.PurgeAll();
	m_PipeList.SetReadDeleteFlag( false );

	// Recuperate the pipe list that will be modified.
	_GetPipeListFromTree( &m_PipeList, eTypeInfo::eTIPipes );

	// Set to the first position.
	CString str;
	m_PipeList.GetFirst( str, m_lparam );

	// Reset the edit info.
	m_EditInfo.SetWindowText( L"" );
	
	// Display the 'CDlgReplacePopup' dialog.
	CDlgReplacePopup Dlg( this );
	m_pDlgReplacePopup = &Dlg;
	m_pDlgReplacePopup->SetGroupFind( TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_POPUP_GROUPFIND ) );
	m_pDlgReplacePopup->SetGroupReplace( TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_POPUP_GROUPREPLACE ) );

	Dlg.DoModal();
	return true;
}

void CDlgSRPagePipes::SetTree( LPARAM pHMTreeListCtrl )
{
	// Define the tree from the parent.
	m_pTree = (CHMTreeListCtrl*)pHMTreeListCtrl;

	// Fill the pipes.
	_FillComboBoxReplacePipesSerie();
	_FillComboBoxFindPipesSerie();
}

BEGIN_MESSAGE_MAP( CDlgSRPagePipes, CDlgCtrlPropPage)
	ON_MESSAGE( WM_USER_HMTREEITEMCHECKCHANGED, &CDlgSRPagePipes::OnTreeItemCheckStatusChanged )
	ON_MESSAGE( WM_USER_REPLACEPOPUPFINDNEXT, OnFindNextPipe )
	ON_MESSAGE( WM_USER_REPLACEPOPUPREPLACE, OnReplacePipe )
	ON_MESSAGE( WM_USER_REPLACEPOPUPREPLACEALL, OnReplaceAll )
	ON_MESSAGE( WM_USER_CLOSEPOPUPWND, OnClosePopup )

	ON_CBN_SELCHANGE( IDC_COMBO_FINDPIPESERIE, OnCbnSelChangeFindPipeSerie )
	ON_CBN_SELCHANGE( IDC_COMBO_REPLACEPIPESERIE, OnCbnSelChangeReplacePipeSerie )
	ON_CBN_SELCHANGE( IDC_COMBO_FINDPIPESIZE, OnCbnSelChangeFindPipeSize )
	ON_CBN_SELCHANGE( IDC_COMBO_REPLACEPIPESIZE, OnCbnSelChangeReplacePipeSize )
	ON_BN_CLICKED( IDC_CHECK_DISTPIPES, OnBnClickedCheckDistPipes )
	ON_BN_CLICKED( IDC_CHECK_CIRCPIPES, OnBnClickedCheckCircPipes )
	ON_BN_CLICKED( IDC_CHECK_APPLYLOCKED, OnBnClickedCheckApplyLocked )
END_MESSAGE_MAP()

void CDlgSRPagePipes::DoDataExchange( CDataExchange *pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBO_FINDPIPESERIE, m_CBFindPipeSerie );
	DDX_Control( pDX, IDC_COMBO_FINDPIPESIZE, m_CBFindPipeSize );
	DDX_Control( pDX, IDC_COMBO_REPLACEPIPESERIE, m_CBReplacePipeSerie );
	DDX_Control( pDX, IDC_COMBO_REPLACEPIPESIZE, m_CBReplacePipeSize );
	DDX_Control( pDX, IDC_GROUP_FIND, m_GroupFind );
	DDX_Control( pDX, IDC_CHECK_DISTPIPES, m_ChBDistPipes );
	DDX_Control( pDX, IDC_CHECK_CIRCPIPES, m_ChBCircPipes );
	DDX_Control( pDX, IDC_CHECK_APPLYLOCKED, m_ChBApplyLocked );
	DDX_Control( pDX, IDC_EDIT_INFO, m_EditInfo );
}

BOOL CDlgSRPagePipes::OnInitDialog() 
{
	// Do default function.
	BOOL bReturn = CDlgCtrlPropPage::OnInitDialog();

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_FINDWHAT );
	GetDlgItem( IDC_STATIC_FIND )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_REPLACEWITH );
	GetDlgItem( IDC_STATIC_REPLACE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_FINDWHERE );
	m_GroupFind.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_DISTPIPES );
	m_ChBDistPipes.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_CIRCPIPES );
	m_ChBCircPipes.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_APPLYLOCKED );
	m_ChBApplyLocked.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_REPPIPES );
	GetDlgItem( IDC_STATIC_REP_INFO )->SetWindowText( str );

	// Define the Ctrl style for the group box.
	m_GroupFind.SetControlStyle( CXGroupBox::CONTROL_STYLE::header, FALSE );
	m_GroupFind.SetBckgndColor( _WHITE_DLGBOX, FALSE );

	// Define check box status.
	m_ChBDistPipes.SetCheck( BST_CHECKED );
	m_ChBCircPipes.SetCheck( BST_CHECKED );
	m_ChBApplyLocked.SetCheck( BST_CHECKED );

	return bReturn;
}

LRESULT CDlgSRPagePipes::OnTreeItemCheckStatusChanged( WPARAM wParam, LPARAM lParam )
{
	_FillComboBoxFindPipesSerie();
	return 0;
}

LRESULT CDlgSRPagePipes::OnFindNextPipe( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pDlgReplacePopup || NULL == m_pDlgReplacePopup->GetSafeHwnd() )
	{
		return 0;
	}

	// Update data to the popup.
	_UpdatePopupData();

	// Define the current lparam used.
	// Useful for the replace pipe function.
	m_lparamCurrent = m_lparam;
	
	// Try to reach the next element in the rank list.
	// If it fail, disable the "Find Next" button.
	CString str;
	
	if( FALSE == m_PipeList.GetNext( str, m_lparam ) )
	{
		m_pDlgReplacePopup->DisableFindNextBtn();
	}
	
	return 0;
}

LRESULT CDlgSRPagePipes::OnReplacePipe( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pDlgReplacePopup || NULL == m_pDlgReplacePopup->GetSafeHwnd() )
	{
		return 0;
	}

	// Recuperate the current pointer.
	CPipes *pPipe = (CPipes *)m_lparamCurrent;

	// Recuperate the information from the combo box.
	CTable *pCBTab = (CTable *)m_CBReplacePipeSerie.GetItemData( m_CBReplacePipeSerie.GetCurSel() );
	CDB_Pipe *pCBPipe = (CDB_Pipe *)m_CBReplacePipeSize.GetItemData( m_CBReplacePipeSize.GetCurSel() );

	// Replace the pipe.
	if( NULL != pCBPipe )
	{
		pPipe->SetIDPtr( pCBPipe->GetIDPtr() );
	}
	else
	{
		pPipe->SetPipeSeries( pCBTab );
	}

	// Update the 'edit' box.
	_UpdateEditData();
	
	// Pipe is locked when the pipe size is specified.
	// Otherwise pipe is released to be computed later.
	pPipe->SetLock( pCBPipe ? true : false );

	// Delete the item from the rank list.
	m_PipeList.Delete( L"", m_lparamCurrent );
	
	// Verify another pipe exist.
	if( 0 == m_PipeList.GetCount() || true == m_pDlgReplacePopup->IsFindNextBtnDisable() )
	{
		m_pDlgReplacePopup->OnBnClickedBtnClose();
		return 0;
	}

	// Jump to the next pipe.
	OnFindNextPipe( wParam, lParam );
	
	return 0;
}

LRESULT CDlgSRPagePipes::OnReplaceAll( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pDlgReplacePopup || NULL == m_pDlgReplacePopup->GetSafeHwnd() ) 
	{
		return 0;
	}

	BeginWaitCursor();

	CString str;
	
	// Reset to the first position.
	m_PipeList.GetFirst( str, m_lparam );
	OnFindNextPipe( wParam, lParam );
	
	// Do a loop on the rank list and replace each pipe.
	while( 0 != m_PipeList.GetCount() )
	{
		OnReplacePipe( wParam, lParam );
	};

	EndWaitCursor();

	return 0;
}

LRESULT CDlgSRPagePipes::OnClosePopup( WPARAM wParam, LPARAM lParam )
{
	// Compute All is needed to correctly resize distribution pipe.
	TASApp.GetpTADS()->ComputeAllInstallation();
	m_pDlgReplacePopup = NULL;
	return TRUE;
}

void CDlgSRPagePipes::OnCbnSelChangeFindPipeSerie()
{
	_FillComboBoxFindPipesSize();
}

void CDlgSRPagePipes::OnCbnSelChangeReplacePipeSerie()
{
	_FillComboBoxReplacePipesSize();
}

void CDlgSRPagePipes::OnCbnSelChangeFindPipeSize()
{
	_UpdateStartBtn();
}

void CDlgSRPagePipes::OnCbnSelChangeReplacePipeSize()
{
	_UpdateStartBtn();
}

void CDlgSRPagePipes::OnBnClickedCheckDistPipes()
{
	// Gray the circuit pipes in consequences of check status.
	m_ChBCircPipes.EnableWindow( ( BST_CHECKED == m_ChBDistPipes.GetCheck() ) ? TRUE : FALSE );
	_FillComboBoxFindPipesSerie();
}

void CDlgSRPagePipes::OnBnClickedCheckCircPipes()
{
	// Gray the distribution pipes in consequences of check status.
	m_ChBDistPipes.EnableWindow( ( BST_CHECKED == m_ChBCircPipes.GetCheck() ) ? TRUE : FALSE );
	_FillComboBoxFindPipesSerie();
}

void CDlgSRPagePipes::OnBnClickedCheckApplyLocked()
{
	_FillComboBoxFindPipesSerie();
}

void CDlgSRPagePipes::_GetPipeListFromTree( CRank *pList, eTypeInfo eInfo, HTREEITEM hItem )
{
	if( NULL == pList )
	{
		return;
	}

	if( NULL == hItem )
	{
		hItem = m_pTree->GetRootItem();
	}

	// Do a loop on the tree.
	for( HTREEITEM hChildItem = m_pTree->GetChildItem( hItem ); NULL != hChildItem; hChildItem = m_pTree->GetNextSiblingItem( hChildItem ) )
	{
		if( INDEXTOSTATEIMAGEMASK( CRCImageManager::ILCB_Selected + 1 ) == m_pTree->GetItemState( hChildItem, TVIS_STATEIMAGEMASK ) )
		{
			CDS_HydroMod *pHM = (CDS_HydroMod *)m_pTree->GetItemData( hChildItem );
			
			// Insert distribution supply tables.
			if( BST_CHECKED == m_ChBDistPipes.GetCheck() )
			{
				for( int iLoopDistrPipes = 0; iLoopDistrPipes < 2; iLoopDistrPipes++ )
				{
					CPipes *pDistPipes = ( 0 == iLoopDistrPipes ) ? pHM->GetpDistrSupplyPipe() : pHM->GetpDistrReturnPipe();
				
					if( NULL != pDistPipes && ( BST_CHECKED == m_ChBApplyLocked.GetCheck() || false == pDistPipes->IsLocked() ) )
					{
						CTable *pCBTab = (CTable *)m_CBFindPipeSerie.GetItemData( m_CBFindPipeSerie.GetCurSel() );
						CDB_Pipe *pCBPipe = (CDB_Pipe *)m_CBFindPipeSize.GetItemData( m_CBFindPipeSize.GetCurSel() );
						CTable *pTab = pDistPipes->GetPipeSeries();
						CDB_Pipe *pPipe = (CDB_Pipe *)pDistPipes->GetIDPtr().MP;

						if( eTypeInfo::eTIPipeSerie == eInfo )
						{
							// Pipes series.
							if( NULL != pTab)
							{
								pList->AddStrSort( pTab->GetName(), 0, (LPARAM)pTab, false, false );
							}
						}
						else if( eTypeInfo::eTIPipeSize == eInfo )
						{
							// Pipes size.
							if( pCBTab == pTab )
							{
								if( NULL != pPipe )
								{
									pList->Add( pPipe->GetName(), pPipe->GetIntDiameter(), (LPARAM)pPipe, true, false );
								}
							}
						}
						else if( eTypeInfo::eTIPipes == eInfo )
						{
							// HydroMod.

							// Add only pipes that match the current combo item.
							if( ( NULL == pCBTab || pCBTab == pTab ) && ( NULL == pCBPipe || pCBPipe == pPipe ) )
							{
								double dKey = pHM->GetLevel() * 1000 + pHM->GetPos();

								if( NULL != pTab && NULL != pDistPipes )
								{
									pList->Add( pTab->GetName(), dKey, (LPARAM)pDistPipes, true, false );
								}
							}
						}
						else
						{
							ASSERT( 0 );
						}
					}
				}
			}
			
			// Insert circuit pipe tables.
			if( BST_CHECKED == m_ChBCircPipes.GetCheck() )
			{
				for( int iLoopCircPipes = 0; iLoopCircPipes < 2; iLoopCircPipes++ )
				{
					CPipes *pCircPipes = ( 0 == iLoopCircPipes ) ? pHM->GetpCircuitPrimaryPipe() : pHM->GetpCircuitSecondaryPipe();
				
					if( NULL != pCircPipes && ( BST_CHECKED == m_ChBApplyLocked.GetCheck() || false == pCircPipes->IsLocked() ) )
					{
						CTable *pCBTab = (CTable *)m_CBFindPipeSerie.GetItemData( m_CBFindPipeSerie.GetCurSel() );
						CDB_Pipe *pCBPipe = (CDB_Pipe *)m_CBFindPipeSize.GetItemData( m_CBFindPipeSize.GetCurSel() );
						CTable *pTab = pCircPipes->GetPipeSeries();
						CDB_Pipe *pPipe = (CDB_Pipe *)pCircPipes->GetIDPtr().MP;

						if( eTypeInfo::eTIPipeSerie == eInfo )
						{
							// Pipes series.
							if( NULL != pTab )
							{
								pList->AddStrSort( pTab->GetName(), 0, (LPARAM)pTab, false, false );
							}
						}
						else if( eTypeInfo::eTIPipeSize == eInfo )
						{
							// Pipes size.
							if( pCBTab == pTab )
							{
								if( NULL != pPipe )
								{
									pList->Add( pPipe->GetName(), pPipe->GetIntDiameter(), (LPARAM)pPipe, true, false );
								}
							
							}
						}
						else if( eTypeInfo::eTIPipes == eInfo )
						{
							// HydroMod.
						
							// Add only pipes that match the current combo item.
							if( ( NULL == pCBTab || pCBTab == pTab ) && ( NULL == pCBPipe || pCBPipe == pPipe ) )
							{
								double dKey = pHM->GetLevel() * 1000 + pHM->GetPos();

								if( NULL != pTab && NULL != pCircPipes )
								{
									pList->Add( pTab->GetName(), dKey, (LPARAM)pCircPipes, true, false );
								}
							}
						}
						else
						{
							ASSERT( 0 );
						}
					}
				}
			}
		}
		
		// Do the recursive function.
		if( NULL != m_pTree->GetChildItem( hChildItem ) )
		{
			_GetPipeListFromTree( pList, eInfo, hChildItem );
		}
	}
}

void CDlgSRPagePipes::_FillComboBoxFindPipesSerie()
{
	CRank List;
	_GetPipeListFromTree( &List, eTypeInfo::eTIPipeSerie );
	_FillComboBox( &List, &m_CBFindPipeSerie );

	// Add "** All pipe series**" as first element.
	if( m_CBFindPipeSerie.GetCount() > 1 )
	{
		m_CBFindPipeSerie.InsertString( 0, TASApp.LoadLocalizedString( IDS_COMBOTEXT_ALL_PIPESERIES ) );
		m_CBFindPipeSerie.SetCurSel( 0 );
	}

	_FillComboBoxFindPipesSize();
}

void CDlgSRPagePipes::_FillComboBoxFindPipesSize()
{
	CRank List;
	int iItem = m_CBFindPipeSerie.GetCurSel();
	CTable *pTab = (CTable *)m_CBFindPipeSerie.GetItemData( m_CBFindPipeSerie.GetCurSel() );
	
	if( CB_ERR != iItem && NULL != pTab )
	{
		_GetPipeListFromTree( &List, eTypeInfo::eTIPipeSize );
	}
	
	_FillComboBox( &List, &m_CBFindPipeSize );

	// Add "** All size **" as first element.
	if( 1 < m_CBFindPipeSize.GetCount() || NULL == pTab )
	{
		m_CBFindPipeSize.InsertString( 0, TASApp.LoadLocalizedString( IDS_COMBOTEXT_ALL_SIZES ) );
		m_CBFindPipeSize.SetCurSel( 0 );
	}

	_UpdateStartBtn();
}

void CDlgSRPagePipes::_FillComboBoxReplacePipesSerie()
{
	CRank List;
	TASApp.GetpTADB()->GetPipeSerieList( &List );
	_FillComboBox( &List, &m_CBReplacePipeSerie );
	_FillComboBoxReplacePipesSize();
}

void CDlgSRPagePipes::_FillComboBoxReplacePipesSize()
{
	CRank List;
	CTable *pTab = (CTable *)m_CBReplacePipeSerie.GetItemData( m_CBReplacePipeSerie.GetCurSel() );
	
	TASApp.GetpTADB()->GetPipeSizeList( &List, pTab, CTADatabase::FilterSelection::ForHMCalc );
	_FillComboBox( &List, &m_CBReplacePipeSize );

	// Add all size as first element.
	if( m_CBReplacePipeSize.GetCount() > 1 )
	{
		m_CBReplacePipeSize.InsertString( 0, TASApp.LoadLocalizedString( IDS_COMBOTEXT_ALL_SIZES ) );
		m_CBReplacePipeSize.SetCurSel( 0 );
	}

	_UpdateStartBtn();
}

void CDlgSRPagePipes::_FillComboBox( CRank *pList, CExtNumEditComboBox *pclComboBox )
{
	if( NULL == pList || NULL == pclComboBox )
	{
		return;
	}
	
	// Transfer the list to the combo Box.
	pList->Transfer( pclComboBox );

	// Set the first item has the selected one.
	pclComboBox->SetCurSel( 0 );
	
	// Enable/Disable the window depending on the number of items.
	if( pclComboBox->GetCount() < 2 )
	{
		pclComboBox->EnableWindow( FALSE );
	}
	else
	{
		pclComboBox->EnableWindow( TRUE );
	}
	
	// Purge the list.
	pList->PurgeAll();
}

void CDlgSRPagePipes::_UpdatePopupData()
{
	if( NULL == m_lparam || CB_ERR == m_CBReplacePipeSerie.GetCurSel() || CB_ERR == m_CBReplacePipeSize.GetCurSel() )
	{
		return;
	}

	// Define the current pipe to show and his HydroMod.
	CPipes *pPipe = (CPipes *)m_lparam;
	CDS_HydroMod *pHM = pPipe->GetpParent();
	
	// Define the text where the pipe came from.
	CString strPipeFrom = L"";
	
	if( NULL != pHM && pPipe == pHM->GetpDistrSupplyPipe() )
	{
		strPipeFrom = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_POPUP_DISTPIPE );
	}
	else if( NULL != pHM && pPipe == pHM->GetpCircuitPrimaryPipe() )
	{
		strPipeFrom = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_POPUP_CIRCPIPE );
	}
	else
	{
		ASSERT( 0 );
	}
	
	// Define strings for 'm_strStaticFind1'.
	m_strStaticFind1 = pHM->GetHMName();
	m_strStaticFind1 += L" - ";
	m_strStaticFind1 += strPipeFrom;
	
	// Define strings for 'm_strStaticFind2'.
	m_strStaticFind2 = pPipe->GetPipeSeries()->GetName();
	m_strStaticFind2 += L", ";
	m_strStaticFind2 += ( (CDB_Pipe *)pPipe->GetIDPtr().MP )->GetName();
	
	// Define strings for 'm_strStaticReplace'.
	CString strPipeSerie( _T("") );
	CString strPipeSize( _T("") );
	m_CBReplacePipeSerie.GetLBText( m_CBReplacePipeSerie.GetCurSel(), strPipeSerie );
	m_CBReplacePipeSize.GetLBText( m_CBReplacePipeSize.GetCurSel(), strPipeSize );
	
	CDB_Pipe *pCBPipe = (CDB_Pipe *)m_CBReplacePipeSize.GetItemData( m_CBReplacePipeSize.GetCurSel() );
	m_strStaticReplace = strPipeSerie;	
	
	if( NULL != pCBPipe )
	{
		m_strStaticReplace += L", ";
		m_strStaticReplace += strPipeSize;
	}

	// Update text values in the popup window.
	m_pDlgReplacePopup->SetStaticFind1( m_strStaticFind1 );
	m_pDlgReplacePopup->SetStaticFind2( m_strStaticFind2 );
	m_pDlgReplacePopup->SetStaticReplace( m_strStaticReplace );
	m_pDlgReplacePopup->UpdateData();
}

void CDlgSRPagePipes::_UpdateEditData()
{
	if( NULL == m_lparamCurrent )
	{
		return;
	}

	// Define the current pipe to show and his HydroMod.
	CPipes *pPipe = ( CPipes * )m_lparamCurrent;
	CDS_HydroMod *pHM = pPipe->GetpParent();

	CString str = L"";
	m_EditInfo.GetWindowText( str );

	// Define the text where the pipe came from.
	if( NULL != pHM && NULL != pPipe )
	{
		CString str1;

		if( pPipe == pHM->GetpDistrSupplyPipe() )
		{
			str1 = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_EDIT_DISTPIPE );
		}
		else
		{
			str1 = TASApp.LoadLocalizedString( IDS_TABDLGSRPIPES_EDIT_CIRCPIPE );
		}

		CString str2, str3, str4;
		CTable *pipeSerie = pPipe->GetPipeSeries();
		str3 = pipeSerie ? pipeSerie->GetName() : _T( "" );

		IDPTR pipe = pPipe->GetIDPtr();
		str4 = pipe.MP ? ( ( CDB_Pipe * )pipe.MP )->GetName() : _T( "" );
		FormatString( str2, str1, pHM->GetHMName(), str3, str4 );
		str += str2;
		str += L"\r\n";
	}

	// Update text values in the edit ctrl.
	m_EditInfo.SetWindowText( str );
}

void CDlgSRPagePipes::_UpdateStartBtn()
{
	BOOL bEnable = TRUE;
	
	// Variables.
	CTable *pCBFindTab = (CTable *)m_CBFindPipeSerie.GetItemData( m_CBFindPipeSerie.GetCurSel() );
	CTable *pCBReplaceTab = (CTable *)m_CBReplacePipeSerie.GetItemData( m_CBReplacePipeSerie.GetCurSel() );
	CDB_Pipe *pCBFindPipe = (CDB_Pipe *)m_CBFindPipeSize.GetItemData( m_CBFindPipeSize.GetCurSel() );
	CDB_Pipe *pCBReplacePipe = (CDB_Pipe *)m_CBReplacePipeSize.GetItemData( m_CBReplacePipeSize.GetCurSel() );

	// If one combo box is empty disable the 'Start' button.
	bool b1 = ( m_CBFindPipeSerie.GetCount() > 0 ) ? false : true;
	bool b2 = ( m_CBReplacePipeSerie.GetCount() > 0 ) ? false : true;
	bool b3 = ( m_CBFindPipeSize.GetCount() > 0 ) ? false : true;
	bool b4 = ( m_CBReplacePipeSize.GetCount() > 0 ) ? false : true;
	
	if( true == b1 || true == b2 || true == b3 || true == b4 )
	{
		bEnable = FALSE;
	}
	else if( pCBFindTab == pCBReplaceTab && pCBFindPipe == pCBReplacePipe )
	{
		// Verify the user do not try to replace the selection by the same selection.
		bEnable = FALSE;
	}

	// Enable 'Start' button.
	if( NULL != pDlgSearchReplace )
	{
		pDlgSearchReplace->EnableButtonStart( bEnable );
	}
}
