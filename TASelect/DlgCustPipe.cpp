#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Units.h"
#include "Utilities.h"
#include "DataBObj.h"
#include "HydroMod.h"
#include "HMPipes.h"

#include "DlgCustPipe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgCustPipe::CDlgCustPipe( CWnd *pParent )
	: CDialogExt( CDlgCustPipe::IDD, pParent, false )
{
	m_pTADB = NULL;
	m_pclTreeImageList = NULL;
	m_TreeItemHeight = 0;
	m_pUnitDB = NULL;
	m_dIntDiameter = 0.0;
	m_dRoughness = 0.0;
	m_dTemperature = 0.0;
	m_dPressure = 0.0;
	m_DefaultPipeSeriesID = CString( _T( "" ) );
	m_bModified = false;
	m_eButtonCreateType = ButtonType::Nothing;
	m_eButtonDeleteType = ButtonType::Nothing;
	m_pTechParam = NULL;
}

CDlgCustPipe::~CDlgCustPipe()
{
	if( NULL != m_pclTreeImageList )
	{
		delete m_pclTreeImageList;
	}
}

BOOL CDlgCustPipe::PreTranslateMessage(MSG* pMsg)
{
	// Check whether its a keypress. 
	if( WM_KEYDOWN == pMsg->message )
	{
		// Check whether its enter key. 
		if( VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam )
		{
			// Get the edit control. 
			CEdit* pEdit = m_Tree.GetEditControl();

			// If Edit control is null, then user is not  
			// editing the label of tree control. 
			if( NULL != pEdit )
			{
				// Send Mouse LeftButton click to end edit. 
				// Use PostMessage since its async and won't distrub PreTranslateMessage() flow. 
				m_Tree.PostMessage( WM_LBUTTONDOWN, 0, MAKELPARAM( -1, -1 ) );

				if( VK_ESCAPE == pMsg->wParam )
				{
					return TRUE;
				}
			}
		}
	}

	return CDialog::PreTranslateMessage( pMsg );
}

int CDlgCustPipe::Display()
{
	m_pTADB = TASApp.GetpTADB();

	return DoModal();
}

CString CDlgCustPipe::BuildPipeID( bool fPipeSerie )
{
	CString strExt = ( true == fPipeSerie ) ? CString( _T("_UPIPS" ) ) : CString( _T( "_UPIPE") );
	CString strID = GetTimeBasedUniqID();
	strID += strExt;
	return strID;
}

BEGIN_MESSAGE_MAP( CDlgCustPipe, CDialogExt )
	ON_NOTIFY( NM_KILLFOCUS, IDC_TREE1, OnKillFocusPipeSeries )
	ON_NOTIFY( NM_SETFOCUS, IDC_TREE1, OnSetFocusPipeSeries )
	ON_NOTIFY( TVN_SELCHANGING, IDC_TREE1, OnListItemChangingSavePipe )
	ON_NOTIFY( TVN_SELCHANGED, IDC_TREE1, OnTreeSelChangedPipesSeries )
	ON_NOTIFY( NM_CLICK, IDC_TREE1, OnTreeClickPipesSeries )
	ON_NOTIFY( NM_DBLCLK , IDC_TREE1, OnTreeDoubleClickPipesSeries )
	ON_NOTIFY( NM_RCLICK, IDC_TREE1, OnTreeRightClickPipesSeries )
	ON_NOTIFY( TVN_BEGINLABELEDIT, IDC_TREE1, OnTreeBeginLabelEditPipeSeries )
	ON_NOTIFY( TVN_ENDLABELEDIT, IDC_TREE1, OnTreeEndLabelEditPipeSeries )

	ON_NOTIFY( NM_SETFOCUS, IDC_LISTPIPE, OnSetFocusListpipe )
	ON_NOTIFY( NM_CLICK, IDC_LISTPIPE, OnListClickPipe )
	ON_NOTIFY( NM_DBLCLK, IDC_LISTPIPE, OnListDoubleClickPipe )
	ON_NOTIFY( NM_RCLICK, IDC_LISTPIPE, OnListRightClickPipe )
	ON_NOTIFY( LVN_ITEMCHANGING, IDC_LISTPIPE, OnListItemChangingSavePipe )
	ON_NOTIFY( LVN_ITEMCHANGED, IDC_LISTPIPE, OnListItemChanged )
	ON_NOTIFY( LVN_ENDLABELEDIT, IDC_LISTPIPE, OnListEndLabelEditPipe )

	ON_CBN_SELCHANGE( IDC_COMBOVALVE, OnCbnSelChangeValve )
	ON_EN_KILLFOCUS( IDC_EDITINTDIAMETER, OnEnKillFocusInternalDiameter )
	ON_EN_KILLFOCUS( IDC_EDITROUGHNESS, OnEnKillFocusRoughness )
	ON_EN_KILLFOCUS( IDC_EDITMAXPRESSURE, OnEnKillFocusMaxPressure )
	ON_EN_KILLFOCUS( IDC_EDITMAXTEMP, OnEnKillFocusMaxTemp )

	ON_BN_CLICKED( IDC_BUTSELECT, OnBnClickedUnhideAllPipeSeries )
	ON_BN_CLICKED( IDC_BUTUNSELECT, OnBnClickedHideAllPipeSeries )
	ON_BN_CLICKED( IDC_BUTDEFAULTPIPESERIES, OnBnClickedDefaultPipeSeries )
	ON_BN_CLICKED( IDC_BUTTONEXP, OnBnClickedExpTxt )
	ON_BN_CLICKED( IDC_BUTTONCREATE, OnButtonCreate )
	ON_BN_CLICKED( IDC_BUTTONDELETE, OnButtonDelete )

	ON_COMMAND( ID_CUSTPIPEFLT_PIPEEDIT, OnFltMenuPipesEdit )
	ON_COMMAND( ID_CUSTPIPEFLT_PIPEDELETE, OnFltMenuPipesDelete )
	ON_COMMAND( ID_CUSTPIPEFLT_PIPEADD, OnFltMenuPipesAdd )

	ON_COMMAND( ID_CUSTPIPEFLT_SERIESEDIT, OnFltMenuSeriesEdit )
	ON_COMMAND( ID_CUSTPIPEFLT_SERIESSELECT, OnFltMenuSeriesSelect )
	ON_COMMAND( ID_CUSTPIPEFLT_SERIESUNSELECT, OnFltMenuSeriesUnselect )
	ON_COMMAND( ID_CUSTPIPEFLT_SERIESDEFAULT, OnFltMenuSeriesDefault )
	ON_COMMAND( ID_CUSTPIPEFLT_SERIESDELETE, OnFltMenuSeriesDelete )
	ON_COMMAND( ID_CUSTPIPEFLT_SERIESADD, OnFltMenuSeriesAdd )

	ON_MESSAGE( WM_USER_CHECKSTATECHANGE, OnCheckStateChange )
END_MESSAGE_MAP()

void CDlgCustPipe::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_GROUPPIPE, m_GroupPipe );
	DDX_Control( pDX, IDC_TREE1, m_Tree );
	DDX_Control( pDX, IDC_LISTPIPE, m_ListPipe );
	DDX_Control( pDX, IDC_COMBOVALVE, m_ComboValve );
	DDX_Control( pDX, IDC_EDITINTDIAMETER, m_EditDiameter );
	DDX_Control( pDX, IDC_EDITROUGHNESS, m_EditRoughness );
	DDX_Control( pDX, IDC_EDITMAXPRESSURE, m_EditPressure );
	DDX_Control( pDX, IDC_EDITMAXTEMP, m_EditTemp );
	DDX_Control( pDX, IDC_BUTSELECT, m_ButUnhide );
	DDX_Control( pDX, IDC_BUTUNSELECT, m_ButHide );
	DDX_Control( pDX, IDC_BUTDEFAULTPIPESERIES, m_ButtonSetAsDefaultPipeSeries );
	DDX_Control( pDX, IDC_BUTTONCREATE, m_ButtonCreate );
	DDX_Control( pDX, IDC_BUTTONDELETE, m_ButtonDelete );
}

BOOL CDlgCustPipe::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	#ifdef DEBUG
	GetDlgItem( IDC_BUTTONEXP )->ShowWindow( true );
	#endif

	// Initialize dialog strings.
	SetLocalizedTitle( IDS_DLGCUSTPIPE_CAPTION );

	SetLocalizedText( m_ButtonSetAsDefaultPipeSeries, IDS_DLGCUSTOMPIPE_BUTDEFAULTPIPESERIES );
	SetLocalizedText( IDC_STATICSERIES, IDS_DLGCUSTPIPE_STATICSERIES );
	SetLocalizedText( IDC_STATICPIPES, IDS_DLGCUSTPIPE_STATICPIPES );
	SetLocalizedText( IDC_STATICVALVE, IDS_DLGCUSTPIPE_STATICVALVE );
	SetLocalizedText( IDC_STATICINTDIAMETER, IDS_DLGCUSTPIPE_STATICINTDIAMETER );
	SetLocalizedText( IDC_STATICROUGHNESS, IDS_DLGCUSTPIPE_STATICROUGHNESS );
	SetLocalizedText( IDC_STATICMAXPRESSURE, IDS_DLGCUSTPIPE_STATICMAXPRESS );
	SetLocalizedText( IDC_STATIC_MAXTEMP, IDS_DLGCUSTPIPE_STATICMAXTEMP );
	SetLocalizedText( IDOK, IDS_OK );
	SetLocalizedText( IDCANCEL, IDS_CANCEL );

	m_pTechParam = TASApp.GetpTADS()->GetpTechParams();
	ASSERT( NULL != m_pTechParam );

	m_bModified = false;

	// First Save all Pipe DB in BAKDB.
	TASApp.GetpPipeDB()->CopyTo( &m_BackupDB );

	// Set the text for the static controls displaying units.
	TCHAR name[_MAXCHARS];
	m_pUnitDB = CDimValue::AccessUDB();

	// Set the text for the internal diameter units.
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_DIAMETER ), name );
	SetDlgItemText( IDC_STATICUNITINTDIAMETER, name );

	// Set the text for the roughness units.
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_ROUGHNESS ), name );
	SetDlgItemText( IDC_STATICUNITROUGHNESS, name );

	// Set the text for the static pressure units.
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_PRESSURE ), name );
	SetDlgItemText( IDC_STATICUNITPRESSURE, name );

	// Set the text for the temperature units.
	GetNameOf( m_pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATICUNITTEMP, name );

	// Insert icon in the group box.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupPipe.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Pipe );
	}

	// Create tooltips.
	m_ToolTip.Create( this, TTS_NOPREFIX );
	CString TTstr = TASApp.LoadLocalizedString( IDS_DLGCUSTOMPIPE_UNHIDEPIPESERIES );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTSELECT ), TTstr );
	TTstr = TASApp.LoadLocalizedString( IDS_DLGCUSTOMPIPE_HIDEPIPESERIES );
	m_ToolTip.AddToolWindow( GetDlgItem( IDC_BUTUNSELECT ), TTstr );

	// LoadButton images.
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );
	ASSERT( NULL != pclImgListButton );

	if( NULL != pclImgListButton )
	{
		m_ButUnhide.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_SelectAll ) );
		m_ButHide.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILB_UnselectAll ) );
	}

	// Retrieve a copy of the 'Custom pipe' image list in 'm_pcTreeImageList'.
	m_pclTreeImageList = TASApp.GetpRCImageManager()->GetImageListCopy( CRCImageManager::ILN_CustomPipe );

	if( NULL == m_pclTreeImageList )
	{
		return FALSE;
	}

	m_pclTreeImageList->SetOverlayImage( CRCImageManager::ILCP_SetAsDefault, OverlayMaskIndex::OMI_SetAsDefault );

	// Fill list series with the image list	.
	m_Tree.SetImageList( m_pclTreeImageList, TVSIL_NORMAL );

	// Clear and disable all editors.
	_SetEditor( false, true );

	// Initialize item height for the tree ctrl.
#pragma warning( disable : 4244)
	m_TreeItemHeight = m_Tree.GetItemHeight() * 1.2;
#pragma warning( default : 4244)

	// Init pipe series.
	_ResetAll( NULL );

	// Save default pipe series.
	m_DefaultPipeSeriesID = m_pTechParam->GetDefaultPipeSerieID();

	// Init corresponding valve size list.
	CTableDN *pclTableDN = (CTableDN *)( m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	CRank rank;
	IDPTR IDPtr = _NULL_IDPTR;

	// Sort valve size by order.
	for( IDPtr = pclTableDN->GetFirst( ); _T('\0') != *IDPtr.ID; IDPtr = pclTableDN->GetNext() )
	{
		CDB_StringID *ptrStr = dynamic_cast<CDB_StringID *>( IDPtr.MP );

		if( NULL == ptrStr )
		{
			continue;
		}

		int index = 0;
		( (CDB_StringID *)IDPtr.MP )->GetIDstrAs<int>( 0, index );

		rank.Add( ( (CDB_StringID *)IDPtr.MP )->GetString(), index, ( LPARAM )IDPtr.MP );
	}

	CString str;
	LPARAM lParam;
	rank.GetFirst( str, lParam );

	do
	{
		int iItem = m_ComboValve.AddString( ( TCHAR * )( (CDB_StringID *)lParam )->GetString() );
		m_ComboValve.SetItemData( iItem, lParam );
	}
	while( rank.GetNext( str, lParam ) );

	// Establish list of used pipe series.
	m_UsedPipeSeries.RemoveAll();
	
	CTable *pclTableHM = TASApp.GetpTADS()->GetpHydroModTable();
	ASSERT( NULL != pclTableHM );
	
	_FillUsedPipeSeriesList( pclTableHM );

	// Load contextual menus.
	m_FltMenuSeries.LoadMenu( IDR_FLTMENU_CUSTPIPE_SERIES );
	m_FltMenuPipes.LoadMenu( IDR_FLTMENU_CUSTPIPE_PIPES );
	
	return TRUE;
}

void CDlgCustPipe::OnOK()
{
	PREVENT_ENTER_KEY

	//	BAKDB is deleted by the destructor.

	// UPdate hidden status of all pipe series.
	HTREEITEM hItem = m_Tree.GetRootItem();

	while( NULL != hItem )
	{
		CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );

		if( NULL != pTab )
		{
			::AfxGetApp()->WriteProfileInt( _T("Pipe Series"), pTab->GetIDPtr().ID, ( true == pTab->IsHidden() ) ? 0 : 1 );
		}

		hItem = m_Tree.GetNextSiblingItem( hItem );
	}

	CString str = TASApp.LoadLocalizedString( AFXMSG_EMPTYPIPESERIES );

	if( true == m_bModified )
	{
		// Is there empty pipes series in the DB ?
		if( true == _CheckEmptySeries( TASApp.GetpPipeDB() ) )
		{
			// If user cancel deletion of series...
			if( IDYES != AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION , 0 ) )
			{
				return;
			}

			_CheckEmptySeries( TASApp.GetpPipeDB(), true);
		}
	}

	CDialogEx::OnOK();

	// HYS-1499 : If a pipe serie is selected as PipeCircSerieID, PipeDistSupplySerieID or PipeDistReturnSerieID in dlg tech param and unchecked
	// in the dialog pipe series we have to set their values to the m_pTechParam->GetDefaultPipeSerieID. If not, the hidden pipe
	// is selected in HMCalc and this causes a crash.
	if( NULL != TASApp.GetpTADS() && NULL != TASApp.GetpTADS()->GetpProjectParams() && NULL != TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams() )
	{
		IDPTR PipeCircPtr = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::PipeCircSerieID );
		if( NULL != PipeCircPtr.MP && false == PipeCircPtr.MP->IsSelectable() )
		{
			TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams()->SetPrjParamID( CPrjParams::PipeCircSerieID, m_pTechParam->GetDefaultPipeSerieID() );
		}

		IDPTR PipeDistSupplyPtr = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::PipeDistSupplySerieID );
		if( NULL != PipeDistSupplyPtr.MP && false == PipeDistSupplyPtr.MP->IsSelectable() )
		{
			TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams()->SetPrjParamID( CPrjParams::PipeDistSupplySerieID, m_pTechParam->GetDefaultPipeSerieID() );
		}

		IDPTR PipeDisttReturnPtr = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams()->GetPrjParamIDPtr( CPrjParams::PipeDistReturnSerieID );
		if( NULL != PipeDisttReturnPtr.MP && false == PipeDisttReturnPtr.MP->IsSelectable() )
		{
			TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams()->SetPrjParamID( CPrjParams::PipeDistReturnSerieID, m_pTechParam->GetDefaultPipeSerieID() );
		}
	}

	// Fill the ribbon bar with the new list of pipes.
	pMainFrame->FillPipeSeriesCB();
	pMainFrame->FillProductSelPipeSeriesCB();

	// Send message to notify that the pipe database has been modified.
	::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_PIPECHANGE, ( WPARAM )WMUserPipeChange::WM_UPC_ForProject );
	::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_PIPECHANGE, ( WPARAM )WMUserPipeChange::WM_UPC_ForProject );
}

void CDlgCustPipe::OnCancel()
{
	CString str = TASApp.LoadLocalizedString( AFXMSG_DISCARDALLCHANGE );

	if( true == m_bModified )
	{
		if( IDYES != AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) )
		{
			return;
		}
	}

	// Delete pipe table in pipe DB.
	TASApp.GetpPipeDB()->DeletePipeTab();

	// Copy pipe backup into user DB and delete backup database by DB destructor.
	TASApp.GetpPipeDB()->CopyFrom( &m_BackupDB );

	// Restore default pipe series.
	_SaveDefaultPipeSeries( m_DefaultPipeSeriesID );

	// After pipe restoring we should warn hydromod.
	CTable *pTab = TASApp.GetpTADS()->GetpHydroModTable();
	ASSERT( NULL != pTab );

	_RefreshPipeUsed( pTab );
	
	pTab = TASApp.GetpTADS()->GetpHUBSelectionTable();
	ASSERT( NULL != pTab );
	
	_RefreshPipeUsed( pTab );
	CDialogEx::OnCancel();
}

void CDlgCustPipe::OnKillFocusPipeSeries( NMHDR *pNMHDR, LRESULT *pResult )
{
	*pResult = 0;
}

void CDlgCustPipe::OnSetFocusPipeSeries( NMHDR *pNMHDR, LRESULT *pResult )
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );
	_EnableButtonDefaultPipeSeries( pTab );
	*pResult = 0;
}

void CDlgCustPipe::OnTreeSelChangedPipesSeries( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>( pNMHDR );
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );
	ASSERT( NULL != pTab );
	_EnableButtonDefaultPipeSeries( pTab );
	_FillListPipe( pTab );
	_SetState();
	*pResult = 0;
}

void CDlgCustPipe::OnTreeClickPipesSeries( NMHDR *pNMHDR, LRESULT *pResult )
{
	// On a TreeView Control with the CheckBoxes, there is no notification that the check state
	// of the item has been changed, you can just determine that the user has clicked the state Icon.
	// The TreeView Control toggles the state AFTER the click notification is sent, the solution is
	// post a user defined message as a notification that the check state has changed
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( ( int )( short )LOWORD( dwpos ) );
	ht.pt.y = ( ( int )( short )HIWORD( dwpos ) );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( TVHT_ONITEMSTATEICON == ( TVHT_ONITEMSTATEICON & ht.flags ) )
	{
		WPARAM bState = m_Tree.GetCheck( ht.hItem );
		// HYS-1499 : We can not uncheck a pipe series which is currently used
		if( TRUE == BOOL( bState ) )
		{
			// We check if the pipe is used in a current project
			CTable* pTab = (CTable*)m_Tree.GetItemData( ht.hItem );
			CDlgCustPipe::eIsLockedBy eCanHide = _CanHidePipeSeries( pTab->GetIDPtr() );
			if( NULL != pTab && CDlgCustPipe::eIsLockedBy::eUnlocked == eCanHide )
			{
				::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, bState, ( LPARAM )ht.hItem );
			}
			else
			{
				CString str = CteEMPTY_STRING;
				if( eCanHide == CDlgCustPipe::eIsLockedBy::eHydraulicNetwork )
				{
					str = TASApp.LoadLocalizedString( IDS_DLGCUSTOPIPE_USED_PROJECT );
				}
				else if( eCanHide == CDlgCustPipe::eIsLockedBy::eTechParamDefaultPipe )
				{
					str = TASApp.LoadLocalizedString( IDS_DLGCUSTOPIPE_USED_TP );
				}
				else if( eCanHide == CDlgCustPipe::eIsLockedBy::eMainPipe )
				{
					str = TASApp.LoadLocalizedString( IDS_DLGCUSTOPIPE_USED_AS_MAIN );
				}
				if( false == str.IsEmpty() )
				{
					AfxMessageBox( str, MB_OK, 0 );
				}
				*pResult = 1;
				m_Tree.SetFocus();
				_SetState();
				return;
			}
		}
		else
		{
			::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, bState, (LPARAM)ht.hItem );
		}
	}

	m_Tree.SetFocus();
	_SetState();

	*pResult = 0;
}

void CDlgCustPipe::OnTreeDoubleClickPipesSeries( NMHDR *pNMHDR, LRESULT *pResult )
{
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( ( int )( short )LOWORD( dwpos ) );
	ht.pt.y = ( ( int )( short )HIWORD( dwpos ) );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( TVHT_ONITEMLABEL == ( TVHT_ONITEMLABEL & ht.flags ) || TVHT_ONITEMRIGHT == ( TVHT_ONITEMRIGHT & ht.flags ) )
	{
		m_Tree.EditLabel( ht.hItem );
	}

	*pResult = 0;
}

void CDlgCustPipe::OnTreeRightClickPipesSeries( NMHDR *pNMHDR, LRESULT *pResult )
{
	TVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( ( int )( short )LOWORD( dwpos ) );
	ht.pt.y = ( ( int )( short )HIWORD( dwpos ) );

	CPoint point( ht.pt.x, ht.pt.y );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	TreeView_HitTest( pNMHDR->hwndFrom, &ht );

	if( TVHT_ONITEMINDENT == ( TVHT_ONITEMINDENT & ht.flags ) || TVHT_ONITEMSTATEICON == ( TVHT_ONITEMSTATEICON & ht.flags ) ||
		TVHT_ONITEMLABEL == ( TVHT_ONITEMLABEL & ht.flags ) || TVHT_ONITEMRIGHT == ( TVHT_ONITEMRIGHT & ht.flags ) ||
		TVHT_NOWHERE == ( TVHT_NOWHERE & ht.flags ) )
	{
		if( TVHT_NOWHERE != ( TVHT_NOWHERE & ht.flags ) )
		{
			m_Tree.SelectItem( ht.hItem );
		}

		_SetState();

		CTable *pPipeSeries = NULL;
		CDB_Pipe *pPipe = NULL;

		if( TVHT_NOWHERE != ( TVHT_NOWHERE & ht.flags ) )
		{
			pPipeSeries = (CTable*)m_Tree.GetItemData( ht.hItem );

			if( NULL == pPipeSeries )
			{
				return;
			}

			pPipe = dynamic_cast<CDB_Pipe*>( (CData*)( pPipeSeries->GetFirst( CLASS( CDB_Pipe ) ).MP ) );
		}

		m_FltMenuSeries.DestroyMenu();

		if( FALSE == m_FltMenuSeries.LoadMenu( IDR_FLTMENU_CUSTPIPE_SERIES ) )
		{
			return;
		}

		CMenu *pContextMenu = m_FltMenuSeries.GetSubMenu( 0 );

		if( NULL == pContextMenu )
		{
			return;
		}

		// Load correct strings.
		CString str;
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_SERIESEDIT );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_SERIESEDIT, MF_BYCOMMAND, ID_CUSTPIPEFLT_SERIESEDIT, str );
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_SERIESSELECT );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_SERIESSELECT, MF_BYCOMMAND, ID_CUSTPIPEFLT_SERIESSELECT, str );
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_SERIESUNSELECT );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_SERIESUNSELECT, MF_BYCOMMAND, ID_CUSTPIPEFLT_SERIESUNSELECT, str );
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_SERIESDEFAULT );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_SERIESDEFAULT, MF_BYCOMMAND, ID_CUSTPIPEFLT_SERIESDEFAULT, str );
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_SERIESDELETE );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_SERIESDELETE, MF_BYCOMMAND, ID_CUSTPIPEFLT_SERIESDELETE, str );
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_SERIESADD );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_SERIESADD, MF_BYCOMMAND, ID_CUSTPIPEFLT_SERIESADD, str );

		MENUITEMINFO rMenuItemInfo;
		ZeroMemory( &rMenuItemInfo, sizeof( MENUITEMINFO ) );
		rMenuItemInfo.cbSize = sizeof( MENUITEMINFO );
		rMenuItemInfo.fMask = MIIM_STATE;
		rMenuItemInfo.fState = MFS_DISABLED;

		// Edit series name.
		if( NULL == pPipeSeries || ( NULL != pPipe && true == pPipe->IsFixed() ) )
		{
			// If user doesn't right-click on a series, we don't show the 'Edit' item.
			// If user right clicks on a series, if this one is fixed we don't show the 'Edit' item.
			pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_SERIESEDIT, MF_BYCOMMAND );
		}

		// Select\Unselect series.
		if( NULL == pPipeSeries || CDlgCustPipe::eIsLockedBy::eUnlocked != _CanHidePipeSeries( pPipeSeries->GetIDPtr() ) )
		{
			// Remove both.
			pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_SERIESSELECT, MF_BYCOMMAND );
			pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_SERIESUNSELECT, MF_BYCOMMAND );
		}
		else
		{
			if( TRUE == m_Tree.GetCheck( ht.hItem ) )
			{
				pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_SERIESSELECT, MF_BYCOMMAND );
			}
			else
			{
				pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_SERIESUNSELECT, MF_BYCOMMAND );
			}
		}

		// Set as default.
		if( NULL == pPipeSeries || FALSE == m_Tree.GetCheck( ht.hItem ) )
		{
			pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_SERIESDEFAULT, MF_BYCOMMAND );
		}
		else if( CString( pPipeSeries->GetIDPtr().ID ) == m_pTechParam->GetDefaultPipeSerieID() )
		{
			// If it's already the default one, disable the item.
			pContextMenu->SetMenuItemInfo( ID_CUSTPIPEFLT_SERIESDEFAULT, &rMenuItemInfo, FALSE );
		}
		else if( NULL == pPipe || false == pPipe->IsFixed() )
		{
			// If no pipe defined or the series is fixed.
			pContextMenu->SetMenuItemInfo( ID_CUSTPIPEFLT_SERIESDEFAULT, &rMenuItemInfo, FALSE );
		}

		// Delete series.
		if( NULL == pPipeSeries || (  NULL != pPipe && true == pPipe->IsFixed() ) )
		{
			// If user doesn't right-click on a series, we don't show the 'Delete' item.
			// If user right clicks on a series, if this one is fixed we don't show the 'Delete' item.
			pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_SERIESDELETE, MF_BYCOMMAND );
		}

		// No check box space in left of the text.
		MENUINFO rMenuInfo;
		ZeroMemory( &rMenuInfo, sizeof( MENUINFO ) );
		rMenuInfo.cbSize = sizeof( MENUINFO );
		rMenuInfo.fMask = MIM_STYLE;
		rMenuInfo.dwStyle = MNS_AUTODISMISS | MNS_NOCHECK;
		pContextMenu->SetMenuInfo( &rMenuInfo );

		// Show the popup menu.
		pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, this );
	}
}

void CDlgCustPipe::OnTreeBeginLabelEditPipeSeries( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>( pNMHDR );
	*pResult = 0;

	HTREEITEM hItem = pTVDispInfo->item.hItem;

	if( NULL == hItem )
	{
		return;
	}

	CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );
	ASSERT( NULL != pTab );

	if( NULL == pTab )
	{
		return;
	}

	IDPTR IDPtr = pTab->GetFirst();

	if( NULL == IDPtr.MP )
	{
		return;
	}

	// Cancel pipe series edition if it's fixed.
	if( true == ( (CDB_Pipe *)IDPtr.MP )->IsFixed() )
	{
		*pResult = true;
	}
}

void CDlgCustPipe::OnTreeEndLabelEditPipeSeries( NMHDR *pNMHDR, LRESULT *pResult )
{
	try
	{
		LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>( pNMHDR );
		*pResult = 0;

		HTREEITEM hItem = pTVDispInfo->item.hItem;

		if( NULL == hItem )
		{
			return;
		}

		LPARAM lParam = m_Tree.GetItemData( hItem );

		if( NULL == lParam )
		{
			return;
		}

		// Found selected Item and take pointer on table.
		CTable *pTab = (CTable *)lParam;
		IDPTR IDPtr = pTab->GetFirst( CLASS( CDB_Pipe ) );

		// If series exist and is fixed, can't change its name.
		if( NULL != ( (CDB_Thing *)IDPtr.MP ) && true == ( (CDB_Thing *)IDPtr.MP )->IsFixed() )
		{
			return;
		}

		CString str = pTVDispInfo->item.pszText;

		// If new name is empty ...
		if( true == str.IsEmpty() )
		{
			return;
		}

		LVFINDINFO	lvFindInfo;
		lvFindInfo.flags = LVFI_STRING;
		lvFindInfo.vkDirection = VK_NEXT;
		lvFindInfo.psz = str;

		// If name already exist in this series...
		if( -1 != m_ListPipe.FindItem( &lvFindInfo, -1 ) )
		{
			return;
		}

		// Get IDPtr from pipes in PIPERDB.
		IDPTR PipeTabIDPtr = TASApp.GetpPipeDB()->GetPipeTab()->GetIDPtr();

		if( _NULL_IDPTR == PipeTabIDPtr )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve IDPTR of the 'PIPE_TAB' table from the pipe database.") );
		}

		pTab->Unlock( PipeTabIDPtr );
		pTab->SetName( str );
		pTab->Lock( PipeTabIDPtr );

		*pResult = true;
		_ResetAll( &str );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgCustPipe::OnTreeEndLabelEditPipeSeries'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgCustPipe::OnSetFocusListpipe( NMHDR *pNMHDR, LRESULT *pResult )
{
	// Disable button set as default when the focus is set to the pipe itself.
	m_ButtonSetAsDefaultPipeSeries.EnableWindow( FALSE );
	*pResult = 0;
}

void CDlgCustPipe::OnListClickPipe( NMHDR *pNMHDR, LRESULT *pResult )
{
	LRESULT	lRes;
	OnListItemChanged( NULL, &lRes );
	*pResult = 0;
}

void CDlgCustPipe::OnListDoubleClickPipe( NMHDR *pNMHDR, LRESULT *pResult )
{
	LVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( ( int )( short )LOWORD( dwpos ) );
	ht.pt.y = ( ( int )( short )HIWORD( dwpos ) );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	ListView_HitTest( pNMHDR->hwndFrom, &ht );

	if( LVHT_ONITEMLABEL == ( LVHT_ONITEMLABEL & ht.flags ) || LVHT_NOWHERE == ( LVHT_NOWHERE & ht.flags ) )
	{
		m_ListPipe.EditLabel( ht.iItem );
	}

	*pResult = 0;
}

void CDlgCustPipe::OnListRightClickPipe( NMHDR *pNMHDR, LRESULT *pResult )
{
	LRESULT	lRes;
	OnListItemChanged( NULL, &lRes );

	HTREEITEM hSeriesItem = m_Tree.GetSelectedItem();
	CTable *pclPipeSeries = (CTable*)m_Tree.GetItemData( hSeriesItem );
	CDB_Pipe *pclFirstPipe = NULL;

	if( NULL != pclPipeSeries && pclPipeSeries->GetItemCount() > 0 )
	{
		CDB_Pipe *pclFirstPipe = (CDB_Pipe*)( pclPipeSeries->GetFirst().MP );

		if( NULL != pclFirstPipe && true == pclFirstPipe->IsFixed() )
		{
			// If we are on a fixed series, menu for the list control is not available.
			return;
		}
	}

	LVHITTESTINFO ht = { 0 };
	DWORD dwpos = GetMessagePos();
	ht.pt.x = ( ( int )( short )LOWORD( dwpos ) );
	ht.pt.y = ( ( int )( short )HIWORD( dwpos ) );

	CPoint point( ht.pt.x, ht.pt.y );

	::MapWindowPoints( HWND_DESKTOP, pNMHDR->hwndFrom, &ht.pt, 1 );
	ListView_HitTest( pNMHDR->hwndFrom, &ht );

	if( LVHT_NOWHERE == ( LVHT_NOWHERE & ht.flags ) )
	{
		// With tree list control we have the 'TVHT_ONITEMRIGHT' flags when user clicks on the right part of an item.
		// For list view control when user clicks on the right, we receive 'LVHT_NOWHERE' and the 'ListView_HitTest' macro sends
		// back -1 in ht.iItem. No way to determine the item.
		// We test thus by changing x coordinate to be sure to be on the label.
		ht.pt.x = 10;
		ListView_HitTest( pNMHDR->hwndFrom, &ht );
	}

	// Remark: if flag = "LVHT_NOWHERE" it means that user has really clicked outside any items because the test just before.
	if( LVHT_ONITEMLABEL == ( LVHT_ONITEMLABEL & ht.flags ) || LVHT_NOWHERE == ( LVHT_NOWHERE & ht.flags ) )
	{
		CDB_Pipe *pPipe = NULL;

		if( -1 != ht.iItem )
		{
			m_ListPipe.SetItemState( ht.iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED );
			pPipe = (CDB_Pipe*)m_ListPipe.GetItemData( ht.iItem );

			if( NULL == pPipe )
			{
				return;
			}
		}

		m_FltMenuPipes.DestroyMenu();

		if( FALSE == m_FltMenuPipes.LoadMenu( IDR_FLTMENU_CUSTPIPE_PIPES ) )
		{
			return;
		}

		CMenu *pContextMenu = m_FltMenuPipes.GetSubMenu( 0 );

		if( NULL == pContextMenu )
		{
			return;
		}

		// Load correct strings.
		CString str;
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_PIPEEDIT );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_PIPEEDIT, MF_BYCOMMAND, ID_CUSTPIPEFLT_PIPEEDIT, str );
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_PIPEDELETE );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_PIPEDELETE, MF_BYCOMMAND, ID_CUSTPIPEFLT_PIPEDELETE, str );
		str = pMainFrame->GetSplitStringFromResID( ID_CUSTPIPEFLT_PIPEADD );
		pContextMenu->ModifyMenu( ID_CUSTPIPEFLT_PIPEADD, MF_BYCOMMAND, ID_CUSTPIPEFLT_PIPEADD, str );

		MENUITEMINFO rMenuItemInfo;
		ZeroMemory( &rMenuItemInfo, sizeof( MENUITEMINFO ) );
		rMenuItemInfo.cbSize = sizeof( MENUITEMINFO );
		rMenuItemInfo.fMask = MIIM_STATE;
		rMenuItemInfo.fState = MFS_DISABLED;

		// Edit and delete item.
		if( NULL == pPipe )
		{
			pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_PIPEEDIT, MF_BYCOMMAND );
			pContextMenu->DeleteMenu( ID_CUSTPIPEFLT_PIPEDELETE, MF_BYCOMMAND );
		}
		else if( true == pPipe->IsFixed() )
		{
			pContextMenu->SetMenuItemInfo( ID_CUSTPIPEFLT_PIPEEDIT, &rMenuItemInfo, FALSE );
			pContextMenu->SetMenuItemInfo( ID_CUSTPIPEFLT_PIPEDELETE, &rMenuItemInfo, FALSE );
		}

		// Add item.
		if( NULL == pPipe )
		{
			// Disable by default.
			pContextMenu->SetMenuItemInfo( ID_CUSTPIPEFLT_PIPEADD, &rMenuItemInfo, FALSE );

			if( NULL != pclPipeSeries )
			{
				if( NULL == pclFirstPipe || false == pclFirstPipe->IsFixed() )
				{
					// If first pipe doesn't exist, it means that is a user pipe and not the fixed ones. We can thus in this 
					// case add a pipi. If first pipe exist, we need to verify if it not belongs to a fixed series.
					rMenuItemInfo.fState = MFS_ENABLED;
					pContextMenu->SetMenuItemInfo( ID_CUSTPIPEFLT_PIPEADD, &rMenuItemInfo, FALSE );
				}
			}
		}
		else if( true == pPipe->IsFixed() )
		{
			pContextMenu->SetMenuItemInfo( ID_CUSTPIPEFLT_PIPEADD, &rMenuItemInfo, FALSE );
		}

		// Save param in menu.
		MENUINFO rMenuInfo;
		ZeroMemory( &rMenuInfo, sizeof( MENUINFO ) );
		rMenuInfo.cbSize = sizeof( MENUINFO );
		rMenuInfo.fMask = MIM_MENUDATA;
		rMenuInfo.dwMenuData = ht.iItem;
		m_FltMenuPipes.SetMenuInfo( &rMenuInfo );

		// No check box space in left of the text.
		ZeroMemory( &rMenuInfo, sizeof( MENUINFO ) );
		rMenuInfo.cbSize = sizeof( MENUINFO );
		rMenuInfo.fMask = MIM_STYLE;
		rMenuInfo.dwStyle = MNS_AUTODISMISS | MNS_NOCHECK;
		pContextMenu->SetMenuInfo( &rMenuInfo );

		// Show the popup menu.
		pContextMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, point.x, point.y, this );
	}

	m_ListPipe.SetFocus();
	_SetState();
}

void CDlgCustPipe::OnListItemChangingSavePipe( NMHDR *pNMHDR, LRESULT *pResult )
{
	CWnd *ctrlFocus = GetFocus();

	if( NULL == ctrlFocus )
	{
		return;
	}

	switch( ctrlFocus->GetDlgCtrlID() )
	{
		case IDC_EDITINTDIAMETER:
			OnEnKillFocusInternalDiameter();
			break;

		case IDC_EDITROUGHNESS:
			OnEnKillFocusRoughness();
			break;

		case IDC_EDITMAXPRESSURE:
			OnEnKillFocusMaxPressure();
			break;

		case IDC_EDITMAXTEMP:
			OnEnKillFocusMaxTemp();
			break;

		default:
			break;
	}
}

void CDlgCustPipe::OnListItemChanged( NMHDR *pNMHDR, LRESULT *pResult )
{
	int iItem;
	
	if( NULL != pNMHDR )
	{
		NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
		iItem = pNMListView->iItem;
	}
	else
	{
		iItem = m_ListPipe.GetNextItem( -1, LVNI_SELECTED );
	}

	*pResult = 0;

	if( iItem < 0 )
	{
		return;
	}

	// Found selected item and take pointer on table.
	LPARAM lp = m_ListPipe.GetItemData( iItem );

	if( NULL == lp )
	{
		return;
	}

	IDPTR IDPtr = ( (CDB_Pipe *)lp )->GetIDPtr();
	ASSERT( '\0' != *IDPtr.ID );

	// Fill the internal diameter, roughness, pressure and temperature fields.
	m_dIntDiameter = ( (CDB_Pipe *)IDPtr.MP )->GetIntDiameter();
	m_dRoughness = ( (CDB_Pipe *)IDPtr.MP )->GetRoughness();
	m_dPressure = ( (CDB_HydroThing *)IDPtr.MP )->GetPmaxmax();
	m_dTemperature = ( (CDB_HydroThing *)IDPtr.MP )->GetTmax();

	m_EditDiameter.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIAMETER, m_dIntDiameter ), 4, 0, true ) );
	m_EditRoughness.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_ROUGHNESS, m_dRoughness ), 4, 0, true ) );
	m_EditPressure.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_PRESSURE, m_dPressure ), 4, 0, true ) );
	m_EditTemp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_dTemperature ), 4, 0, true ) );

	// Only user series or pipes can be deleted...
	if( true == ( (CDB_Thing *)IDPtr.MP )->IsFixed() )
	{
		_SetButtons( ButtonType::Pipe, ButtonType::Pipe, false, false );
	}
	else
	{
		// Disable all edit and combo.
		_SetEditor( true, false );
		_SetButtons( ButtonType::Pipe, ButtonType::Pipe, true, true );
	}

	m_ComboValve.SelectString( -1, ( (CDB_Pipe *)IDPtr.MP )->GetSize( m_pTADB ) );
}

void CDlgCustPipe::OnListEndLabelEditPipe( NMHDR *pNMHDR, LRESULT *pResult )
{
	LV_DISPINFO *pDispInfo = (LV_DISPINFO *)pNMHDR;
	*pResult = 0;

	int iItem = m_ListPipe.GetNextItem( -1, LVNI_SELECTED );

	if( iItem < 0 )
	{
		return;
	}

	LPARAM lp = m_ListPipe.GetItemData( iItem );

	if( NULL == lp )
	{
		return;
	}

	// If series is fixed, can't change its name.
	if( true == ( (CDB_Thing *)lp )->IsFixed() )
	{
		return;
	}

	CString str = pDispInfo->item.pszText;

	// If new name is empty ...
	if( true == str.IsEmpty() )
	{
		return;
	}

	LVFINDINFO lvFindInfo;
	lvFindInfo.flags = LVFI_STRING;
	lvFindInfo.vkDirection = VK_NEXT;
	lvFindInfo.psz = str;

	// If name already exists in this series...
	if( -1 != m_ListPipe.FindItem( &lvFindInfo, -1 ) )
	{
		return;
	}

	// Set the new pipe name.
	( (CDB_Pipe *)lp )->SetName( str );
	_ResetAll( &m_Tree.GetItemText( m_Tree.GetSelectedItem() ) );
}

void CDlgCustPipe::OnCbnSelChangeValve()
{
	int iPipe = m_ListPipe.GetNextItem( -1, LVNI_SELECTED );
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( iPipe < 0 || NULL == hItem )
	{
		return;
	}

	//Found selected item and take pointer on table..
	LPARAM lp = m_ListPipe.GetItemData( iPipe );

	if( NULL == lp )
	{
		return;
	}

	// Fixed pipe can't change anything.
	if( true == ( (CDB_Thing *)lp )->IsFixed() )
	{
		return;
	}

	int iSel = m_ComboValve.GetCurSel();
	LPARAM lpSel = m_ComboValve.GetItemData( iSel );

	if( NULL == lpSel )
	{
		return;
	}

	// Retrieve ID of selected corresponding valve size.
	IDPTR IDPtr = ( (CDB_StringID *)lpSel )->GetIDPtr();
	ASSERT( '\0' != *IDPtr.ID );
	( (CDB_Pipe *)lp )->SetSizeID( IDPtr.ID );
	m_bModified = true;
}

void CDlgCustPipe::OnEnKillFocusInternalDiameter()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX
	int iPipe = m_ListPipe.GetNextItem( -1, LVNI_SELECTED );
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( iPipe < 0 || NULL == hItem )
	{
		return;
	}

	// Found selected item and take pointer on table.
	LPARAM lp = m_ListPipe.GetItemData( iPipe );

	if( NULL == lp )
	{
		m_EditDiameter.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIAMETER, m_dIntDiameter ), 4, 0, true ) );
		return;
	}

	if( true == ( (CDB_Thing *)lp )->IsFixed() )
	{
		m_EditDiameter.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIAMETER, m_dIntDiameter ), 4, 0, true ) );
		return;
	}

	double dValSI;

	if( false == _CheckEditBox( &m_EditDiameter, &dValSI, _U_DIAMETER ) )
	{
		m_EditDiameter.SetFocus();
	}
	else
	{
		if( dValSI <= 0.0 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_EditDiameter.SetSel( -1 );
			m_EditDiameter.SetFocus();
		}
		else
		{
			// Save value.
			m_bModified = true;
			( (CDB_Pipe *)lp )->SetIntDiameter( dValSI );
			m_dIntDiameter = dValSI;
			m_EditDiameter.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIAMETER, m_dIntDiameter ), 4, 0, true ) );
		}
	}
}

void CDlgCustPipe::OnEnKillFocusRoughness()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX
	int iPipe = m_ListPipe.GetNextItem( -1, LVNI_SELECTED );
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( iPipe < 0 || NULL == hItem )
	{
		return;
	}

	// Found selected item and take pointer on table.
	LPARAM lp = m_ListPipe.GetItemData( iPipe );

	if( NULL == lp )
	{
		m_EditRoughness.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_ROUGHNESS, m_dRoughness ), 4, 0, true ) );
		return;
	}

	if( true == ( (CDB_Thing *)lp )->IsFixed() )
	{
		m_EditRoughness.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_ROUGHNESS, m_dRoughness ), 4, 0, true ) );
		return;
	}

	double dValSI;

	if( false == _CheckEditBox( &m_EditRoughness, &dValSI, _U_ROUGHNESS ) )
	{
		m_EditRoughness.SetFocus();
	}
	else
	{
		if( dValSI <= 0.0 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_EditRoughness.SetFocus();
		}
		else
		{
			// Save value.
			m_bModified = true;
			( (CDB_Pipe *)lp )->SetRoughness( dValSI );
			m_dRoughness = dValSI;
			m_EditRoughness.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_ROUGHNESS, m_dRoughness ), 4, 0, true ) );
		}
	}
}

void CDlgCustPipe::OnEnKillFocusMaxPressure()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX
	int iPipe = m_ListPipe.GetNextItem( -1, LVNI_SELECTED );
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( iPipe < 0 || NULL == hItem )
	{
		return;
	}

	// Found selected item and take pointer on table.
	LPARAM lp = m_ListPipe.GetItemData( iPipe );

	if( NULL == lp )
	{
		m_EditPressure.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_PRESSURE, m_dPressure ), 4, 0, true ) );
		return;
	}

	if( true == ( (CDB_Thing *)lp )->IsFixed() )
	{
		m_EditPressure.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_PRESSURE, m_dPressure ), 4, 0, true ) );
		return;
	}

	double dValSI;

	if( false == _CheckEditBox( &m_EditPressure, &dValSI, _U_PRESSURE ) )
	{
		m_EditPressure.SetFocus();
	}
	else
	{
		if( dValSI <= 0.0 )
		{
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			m_EditPressure.SetFocus();
		}
		else
		{
			// Save value.
			m_bModified = true;
			( (CDB_HydroThing *)lp )->SetPmaxmax( dValSI );
			( (CDB_HydroThing *)lp )->SetPmaxmin( dValSI );
			m_dPressure = dValSI;
			m_EditPressure.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_PRESSURE, m_dPressure ), 4, 0, true ) );
		}
	}
}

void CDlgCustPipe::OnEnKillFocusMaxTemp()
{
	PREVENT_NOT_CANCEL_WITH_EMPTY_BOX
	int iPipe = m_ListPipe.GetNextItem( -1, LVNI_SELECTED );
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( iPipe < 0 || NULL == hItem )
	{
		return;
	}

	// Found selected item and take pointer on table.
	LPARAM lp = m_ListPipe.GetItemData( iPipe );

	if( NULL == lp )
	{
		m_EditTemp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_dTemperature ), 4, 0, true ) );
		return;
	}

	if( true == ( (CDB_Thing *)lp )->IsFixed() )
	{
		m_EditTemp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_dTemperature ), 4, 0, true ) );
		return;
	}

	double dValSI;

	if( false == _CheckEditBox( &m_EditTemp, &dValSI, _U_TEMPERATURE ) )
	{
		m_EditTemp.SetFocus();
	}
	else
	{
		// Save value.
		m_bModified = true;
		( (CDB_HydroThing *)lp )->SetTmax( dValSI );
		m_dTemperature = dValSI;
		m_EditTemp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_dTemperature ), 4, 0, true ) );
	}
}

void CDlgCustPipe::OnBnClickedUnhideAllPipeSeries()
{
	HTREEITEM hItem = m_Tree.GetRootItem();

	while( NULL != hItem )
	{
		CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );

		if( NULL != pTab )
		{
			pTab->SetHidden( false );

			for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext() )
			{
				( (CDB_Pipe *)IDPtr.MP )->SetHidden( false );
			}
		}

		hItem = m_Tree.GetNextSiblingItem( hItem );
	}

	_ResetAll( NULL );
}

void CDlgCustPipe::OnBnClickedHideAllPipeSeries()
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	CString SelectedItem;

	if( hItem >= 0 )
	{
		SelectedItem = m_Tree.GetItemText( hItem );
	}

	hItem = m_Tree.GetRootItem();

	while( NULL != hItem )
	{
		CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );

		if( NULL == pTab )
		{
			return;
		}

		if( CDlgCustPipe::eIsLockedBy::eUnlocked == _CanHidePipeSeries( pTab->GetIDPtr() ) )
		{
			pTab->SetHidden( true );

			for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext() )
			{
				( (CDB_Pipe *)IDPtr.MP )->SetHidden( true );
			}
		}

		hItem = m_Tree.GetNextSiblingItem( hItem );
	}

	_ResetAll( &SelectedItem );
}

void CDlgCustPipe::OnBnClickedDefaultPipeSeries()
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( NULL == hItem )
	{
		return;
	}

	CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );

	if( NULL == pTab )
	{
		return;
	}

	// A user pipe series can never be a default pipe.
	CDB_Pipe *pPipe = (CDB_Pipe *)( pTab->GetFirst().MP );

	if( false == pPipe->IsFixed() )
	{
		return;
	}

	// Update default pipe series ID.
	_SaveDefaultPipeSeries( pTab->GetIDPtr().ID );

	CString PipeSeriesName = pTab->GetName();
	_ResetAll( &PipeSeriesName );
}

void CDlgCustPipe::OnBnClickedExpTxt()
{
	CFileDialog dlg( false, _T("txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST );

	if( IDOK == dlg.DoModal() )
	{
		BeginWaitCursor();

		// Filename Exist.
		try
		{
			CString strFilename = dlg.GetPathName();
			CPath path( strFilename );
			CString File = path.SplitPath( ( CPath::ePathFields )( CPath::ePathFields::epfDrive + CPath::ePathFields::epfDir + CPath::ePathFields::epfFName ) ) +
						   _T(".txt");
			std::ofstream outf( (TCHAR *)( LPCTSTR )File, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );

			// Write txt for all selected pipe series.
			HTREEITEM hItem = m_Tree.GetRootItem();

			while( NULL != hItem )
			{
				CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );

				if( NULL != pTab && false == pTab->IsHidden() )
				{
					CString strTab;
					pTab->WriteTextAccess( outf, strTab );
					CRank rk;

					for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext() )
					{
						strTab.Empty();
						rk.Add( IDPtr.ID, ( (CDB_Pipe *)IDPtr.MP )->GetIntDiameter(), (LPARAM)IDPtr.MP );
					}

					CString str;
					LPARAM lparam;

					for( BOOL fContinue = rk.GetFirst( str, lparam ); TRUE == fContinue; fContinue = rk.GetNext( str, lparam ) )
					{
						( (CDB_Pipe *)lparam )->WriteTextAccess( outf, strTab );
					}
				}

				hItem = m_Tree.GetNextSiblingItem( hItem );
			}

			outf.close();
		}
		catch( ... )
		{};
	}
}

void CDlgCustPipe::OnButtonCreate()
{
	if( ButtonType::Series == m_eButtonCreateType )
	{
		_CreateSeries();
	}
	else
	{
		_CreatePipe();
	}
}

void CDlgCustPipe::OnButtonDelete()
{
	if( ButtonType::Series == m_eButtonDeleteType )
	{
		_DeleteSeries();
	}
	else
	{
		_DeletePipe();
	}
}

void CDlgCustPipe::OnFltMenuPipesEdit()
{
	MENUINFO rMenuInfo;
	ZeroMemory( &rMenuInfo, sizeof( MENUINFO ) );
	rMenuInfo.cbSize = sizeof( MENUINFO );
	rMenuInfo.fMask = MIM_MENUDATA;
	
	if( FALSE  == m_FltMenuPipes.GetMenuInfo( &rMenuInfo ) )
	{
		return;
	}

	m_ListPipe.EditLabel( (int)rMenuInfo.dwMenuData );
}

void CDlgCustPipe::OnFltMenuPipesDelete()
{
	_DeletePipe();
}

void CDlgCustPipe::OnFltMenuPipesAdd()
{
	_CreatePipe();
}

void CDlgCustPipe::OnFltMenuSeriesEdit()
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( NULL == hItem )
	{
		return;
	}

	m_Tree.EditLabel( hItem );
}

void CDlgCustPipe::OnFltMenuSeriesSelect()
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( NULL == hItem )
	{
		return;
	}

	m_Tree.SetCheck( hItem, TRUE );
	::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, WPARAM( FALSE ), ( LPARAM )hItem );
}

void CDlgCustPipe::OnFltMenuSeriesUnselect()
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( NULL == hItem )
	{
		return;
	}

	m_Tree.SetCheck( hItem, FALSE );
	::PostMessage( m_hWnd, WM_USER_CHECKSTATECHANGE, WPARAM( TRUE ), ( LPARAM )hItem );
}

void CDlgCustPipe::OnFltMenuSeriesDefault()
{
	OnBnClickedDefaultPipeSeries();
}

void CDlgCustPipe::OnFltMenuSeriesDelete()
{
	_DeleteSeries();
}

void CDlgCustPipe::OnFltMenuSeriesAdd()
{
	_CreateSeries();
}

LRESULT CDlgCustPipe::OnCheckStateChange( WPARAM wParam, LPARAM lParam )
{
	HTREEITEM hItem = ( HTREEITEM )lParam;
	bool bState = wParam & 0x01;
	CTable *pTab = (CTable *)m_Tree.GetItemData( hItem );

	if( NULL == pTab )
	{
		return 0;
	}

	if( CDlgCustPipe::eIsLockedBy::eUnlocked != _CanHidePipeSeries( pTab->GetIDPtr() ) )
	{
		m_Tree.SetCheck( hItem );
		return 0;
	}

	for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext() )
	{
		CDB_Pipe *pPipe = (CDB_Pipe *)( IDPtr.MP );
		pPipe->SetHidden( ( bool )bState );
	}

	pTab->SetHidden( ( bool )bState );

	m_Tree.SelectItem( hItem );
	_FillListPipe( pTab );
	_EnableButtonDefaultPipeSeries( pTab );
	return 0;
}

void CDlgCustPipe::_ResetAll( CString *pStr )
{
	// Clear series and pipes lists.
	m_Tree.DeleteAllItems();
	m_Tree.SetItemHeight( m_TreeItemHeight );
	m_ListPipe.DeleteAllItems();

	m_dIntDiameter = 0.0;
	m_dRoughness = 0.0;
	m_dPressure = 0.0;
	m_dTemperature = 0.0;

	// Button 'Create series' enabled, 'Delete series' disabled.
	_SetButtons( ButtonType::Series, ButtonType::Series, true, false );

	// Fill all pipe series names.
	CTable *pTab = TASApp.GetpPipeDB()->GetPipeTab();
	ASSERT( NULL != pTab );
	HTREEITEM hItem = NULL;
	CRank rkList;
	CString str;
	LPARAM lparam;

	for( IDPTR IDPtr = pTab->GetFirst( CLASS( CTable ) ); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( false == ( (CTable *)IDPtr.MP )->IsAvailable() )
		{
			continue;
		}

		if( true == ( (CTable *)IDPtr.MP )->IsDeleted() )
		{
			continue;
		}

		str = ( (CTable *)IDPtr.MP )->GetName();
		lparam = ( LPARAM )( IDPtr.MP );
		rkList.AddStrSort( str, 0, lparam, false );
	}

	for( BOOL fContinue = rkList.GetFirst( str, lparam ); TRUE == fContinue; fContinue = rkList.GetNext( str, lparam ) )
	{
		CTable *pTabPipe = (CTable *)lparam;

		// If pipe(s) exist...
		if( pTabPipe->GetItemCount( CLASS( CDB_Pipe ) ) > 0 )
		{
			bool fAddedToTree = false;

			for( IDPTR IDPtrPipe = pTabPipe->GetFirst( CLASS( CDB_Pipe ) ); '\0' != *IDPtrPipe.ID; IDPtrPipe = pTabPipe->GetNext() )
			{
				CDB_Pipe *pPipe = (CDB_Pipe *)( IDPtrPipe.MP );

				if( false == pPipe->IsAvailable() )
				{
					continue;
				}

				if( true == pPipe->IsDeleted() )
				{
					continue;
				}

				// Synchronize hidden flag.
				pPipe->SetHidden( pTabPipe->IsHidden() );

				if( false == fAddedToTree )
				{
					// If at least one pipe exists, pipe series exists.
					int iImage = ( true == pPipe->IsFixed() ) ? CRCImageManager::ILCP_LockUp : CRCImageManager::ILCP_UnLockUp;
					hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE,
											   ( LPCTSTR )( (CTable *)lparam )->GetName(), iImage, iImage, 0, TVIS_SELECTED, ( LPARAM )lparam, TVI_ROOT, TVI_LAST );

					m_Tree.SetCheck( hItem, !pPipe->IsHidden() );

					if( false == pPipe->IsHidden() )
					{
						CString strID = pTabPipe->GetIDPtr().ID;

						if( strID == m_pTechParam->GetDefaultPipeSerieID() )
						{
							m_Tree.SetItemState( hItem, INDEXTOOVERLAYMASK( OverlayMaskIndex::OMI_SetAsDefault ), TVIS_OVERLAYMASK );
						}
					}

					fAddedToTree = true;
				}
			}
		}
		else
		{
			// Empty list series ( user series !)
			int iImage = CRCImageManager::ILCP_UnLockUp;
			hItem = m_Tree.InsertItem( TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE,
									   ( LPCTSTR )( (CTable *)lparam )->GetName(), iImage, iImage, 0, TVIS_SELECTED, ( LPARAM )lparam, TVI_ROOT, TVI_LAST );
			m_Tree.SetCheck( hItem, true );
		}
	}

	if( NULL != pStr )
	{
		// Select item and set focus on it.
		HTREEITEM hItem = m_Tree.GetRootItem();
		ASSERT( NULL != hItem );

		while( NULL != hItem )
		{
			if( m_Tree.GetItemText( hItem ) == *pStr )
			{
				break;
			}

			hItem = m_Tree.GetNextSiblingItem( hItem );
		}

		m_Tree.SelectItem( hItem );
		m_Tree.SetFocus();
		// Button 'Create series' enabled, 'Delete series' disabled.
		_SetButtons( ButtonType::Series, ButtonType::Series, true, true );
	}
	else
	{
		// Select first item.
		m_Tree.SelectItem( m_Tree.GetRootItem() );
		m_Tree.SetFocus();
		// Button 'Create series' enabled, 'Delete series' disabled.
		_SetButtons( ButtonType::Series,  ButtonType::Series, true, false );
	}

	// Force to refresh the tree.
	m_Tree.Invalidate();
}

void CDlgCustPipe::_SetEditor( bool fEnabled, bool fClear )
{
	if( true == fClear )
	{
		m_ComboValve.SetCurSel( -1 );
		m_EditDiameter.SetWindowText( _T( "" ) );
		m_EditRoughness.SetWindowText( _T( "" ) );
		m_EditPressure.SetWindowText( _T( "" ) );
		m_EditTemp.SetWindowText( _T( "" ) );
	}

	m_ComboValve.EnableWindow( ( true == fEnabled ) ? TRUE : FALSE );
	m_EditDiameter.SetReadOnly( ( true == fEnabled ) ? FALSE : TRUE );
	m_EditRoughness.SetReadOnly( ( true == fEnabled ) ? FALSE : TRUE );
	m_EditPressure.SetReadOnly( ( true == fEnabled ) ? FALSE : TRUE );
	m_EditTemp.SetReadOnly( ( true == fEnabled ) ? FALSE : TRUE );
}

void CDlgCustPipe::_SetButtons( ButtonType eButtonCreate, ButtonType eButtonDelete, bool fEnableCreate, bool fEnableDelete )
{
	CString	str;

	switch( eButtonCreate )
	{
		case ButtonType::Nothing:
		case ButtonType::Series:
			str = TASApp.LoadLocalizedString( IDS_CUSTPIPE_CREATESERIES );
			m_ButtonCreate.SetWindowText( str );
			m_eButtonCreateType = ButtonType::Series;
			break;

		case ButtonType::Pipe:
			str = TASApp.LoadLocalizedString( IDS_CUSTPIPE_CREATEPIPE );
			m_ButtonCreate.SetWindowText( str );
			m_eButtonCreateType = ButtonType::Pipe;
			break;
	}

	switch( eButtonDelete )
	{
		case ButtonType::Nothing:
		case ButtonType::Series:
			str = TASApp.LoadLocalizedString( IDS_CUSTPIPE_DELETESERIES );
			m_ButtonDelete.SetWindowText( str );
			m_eButtonDeleteType = ButtonType::Series;
			break;

		case ButtonType::Pipe:
			str = TASApp.LoadLocalizedString( IDS_CUSTPIPE_DELETEPIPE );
			m_ButtonDelete.SetWindowText( str );
			m_eButtonDeleteType = ButtonType::Pipe;
			break;
	}

	m_ButtonCreate.EnableWindow( ( true == fEnableCreate ) ? TRUE : FALSE );
	m_ButtonDelete.EnableWindow( ( true == fEnableCreate ) ? TRUE : FALSE );
}

bool CDlgCustPipe::_FindFirstFreeName( CListCtrl *pList, CString *pStr )
{
	CString str;

	if( NULL != pList )
	{
		// Pipe list.
		LVFINDINFO lvFindInfo;
		lvFindInfo.flags = LVFI_STRING;
		lvFindInfo.vkDirection = VK_NEXT;

		for( int i = 1; i < 100; i++ )
		{
			if( i < 10 )
			{
				str.Format( _T("_ %d"), i );
			}
			else
			{
				str.Format( _T("_%d"), i );
			}

			str = *pStr + str;
			lvFindInfo.psz = str;

			if( pList->FindItem( &lvFindInfo, -1 ) < 0 )
			{
				*pStr = str;
				return true;
			}
		}
	}
	else
	{
		// Pipe series.
		for( int i = 1; i < 100; i++ )
		{
			if( i < 10 )
			{
				str.Format( _T("_ %d"), i );
			}
			else
			{
				str.Format( _T("_%d"), i );
			}

			str = *pStr + str;
			HTREEITEM hItem = m_Tree.GetRootItem();
			bool fFound = false;

			while( NULL != hItem )
			{
				if( str == m_Tree.GetItemText( hItem ) )
				{
					fFound = true;
					break;
				}

				hItem = m_Tree.GetNextSiblingItem( hItem );
			}

			if( false == fFound )
			{
				*pStr = str;
				return true;
			}
		}
	}

	return false;
}

int CDlgCustPipe::_DeleteSelPipe( int iPipe )
{
	CString str;
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	if( NULL == hItem || iPipe < 0 )
	{
		return -1;
	}

	LPARAM lp = m_ListPipe.GetItemData( iPipe );
	ASSERT( NULL != lp );

	if( NULL == lp )
	{
		return -1;
	}

	IDPTR IDPtrPipe = ( (CDB_Pipe *)lp )->GetIDPtr();
	ASSERT( '\0' != *IDPtrPipe.ID );

	// If pipe is fixed can't delete it...
	if( true == ( (CDB_Pipe *)IDPtrPipe.MP )->IsFixed() )
	{
		str = TASApp.LoadLocalizedString( IDS_CUSTPIPE_FIXED );
		AfxMessageBox( str, MB_OK, 0 );
		return -1;
	}

	( (CTable *)IDPtrPipe.PP )->Remove( IDPtrPipe );
	VERIFY( TASApp.GetpPipeDB()->DeleteObject( IDPtrPipe ) );

	m_ListPipe.SetItemData( iPipe, (DWORD_PTR)0 );

	return 0;
}

bool CDlgCustPipe::_CheckEditBox( CEdit *pEdit, double *pdValueSI, int iPhysType )
{
	double dVal, dValSI;
	CString str;

	if( _U_TEMPERATURE != iPhysType )
	{
		switch( ReadDouble( *pEdit, &dVal ) )
		{
			case RD_EMPTY:
				return false;
				break;

			case RD_NOT_NUMBER:
				pEdit->GetWindowText( str );

				if( _T("." ) == str || _T("," ) == str || _T( "e" ) == str.Right( 1 ) || _T( "e+") == str.Right( 2 ) || _T( "e-") == str.Right( 2 ) )
				{
					return false;
				}
				else
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
				}

				pEdit->SetWindowText( _T( "" ) );
				return false;
				break;

			case RD_OK:
				dValSI = CDimValue::CUtoSI( iPhysType, dVal );
				*pdValueSI = dValSI;
				return true;
				break;
		};
	}
	else
	{
		switch( ReadDouble( *pEdit, &dVal ) )
		{
			case RD_EMPTY:
				return false;
				break;

			case RD_NOT_NUMBER:
				pEdit->GetWindowText( str );

				if( _T("." ) == str || _T("," ) == str || _T( "-" ) == str || _T( "e" ) == str.Right( 1 ) || _T( "e+") == str.Right( 2 ) || _T( "e-") == str.Right( 2 ) )
				{
					return false;
				}
				else
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_NON_NUMERICAL_VALUE );
				}

				pEdit->SetWindowText( _T( "" ) );
				return false;
				break;

			case RD_OK:
				dValSI = CDimValue::CUtoSI( iPhysType, dVal );
				*pdValueSI = dValSI;

				if( dValSI < 0.0 )
				{
					TASApp.AfxLocalizeMessageBox( AFXMSG_BELOW_ABSZERO );
					m_EditTemp.SetWindowText( _T( "" ) );
					*pdValueSI = 0.0;
					return false;
				}
				else
				{
					return true;
				}

				break;
		};
	}

	return false;
}

bool CDlgCustPipe::_CheckEmptySeries( CPipeUserDatabase *pDB, bool fDelete )
{
	// Get a pointer to Pipes in pDB.
	CTable *pPipeTab = TASApp.GetpPipeDB()->GetPipeTab();
	ASSERT( NULL != pPipeTab );

	// Loop on pipe series tables.
	for( IDPTR IDPtr = pPipeTab->GetFirst( CLASS( CTable ) ); '\0' != *IDPtr.ID; )
	{
		CTable *pTab = (CTable *)( IDPtr.MP );

		// Loop on existing pipes in Pipe series table pTab, remove them from the table and delete them from pDb.
		if( pTab->GetItemCount( CLASS( CDB_Pipe ) ) > 0 )
		{
			IDPtr = pPipeTab->GetNext();
			continue;
		}

		if( false == fDelete )
		{
			return true;
		}

		// Remove the pipe series table from the pipe table and delete it.
		IDPTR IDPtrNext = pPipeTab->GetNext();
		pPipeTab->Remove( IDPtr );
		VERIFY( pDB->DeleteObject( IDPtr ) );
		IDPtr = IDPtrNext;
	}

	return false;
}

void CDlgCustPipe::_FillListPipe( CTable *pTab )
{
	// Clear the pipe list.
	m_ListPipe.DeleteAllItems();

	// Clear and disable all edit And combo.
	_SetEditor( false, true );

	// Fill the pipe list according to the selected pipe series.
	LVITEM	lvItem;
	lvItem.mask = LVIF_PARAM | LVIF_TEXT;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;
	lvItem.state = 0;
	lvItem.stateMask = 0;
	lvItem.pszText = NULL;
	lvItem.cchTextMax = _PIPE_NAME_LENGTH;
	lvItem.iImage = 0;
	lvItem.lParam = NULL;
	lvItem.iIndent = 1;

	// Modified loop to sort pipes according to their internal diameter.
	CRank rank;
	IDPTR IDPtr = _NULL_IDPTR;

	for( IDPtr = pTab->GetFirst( CLASS( CDB_Pipe ) ); '\0' != *IDPtr.ID; IDPtr = pTab->GetNext() )
	{
		if( false == ( (CDB_Pipe *)IDPtr.MP )->IsSelectable( true ) )
		{
			continue;
		}

		rank.Add( ( (CDB_Pipe *)IDPtr.MP )->GetName(), ( (CDB_Pipe *)IDPtr.MP )->GetIntDiameter(), ( LPARAM ) IDPtr.MP );
	}

	if( rank.GetCount() > 0 )
	{
		CString str;
		rank.GetFirst( str, lvItem.lParam );

		do
		{
			lvItem.pszText = ( TCHAR * )( LPCTSTR )str;
			m_ListPipe.InsertItem( &lvItem );
			lvItem.iItem ++;
		}
		while( rank.GetNext( str, lvItem.lParam ) );
	}

	_SetState();
}

void CDlgCustPipe::_EnableButtonDefaultPipeSeries( CTable *pclTable )
{
	m_ButtonSetAsDefaultPipeSeries.EnableWindow( FALSE );

	if( NULL == pclTable )
	{
		return;
	}

	CString strID = pclTable->GetIDPtr().ID;

	// Disabled if hidden, fixed or if we are on default pipe series.
	CDB_Pipe *pPipe = (CDB_Pipe *)pclTable->GetFirst().MP;

	if( NULL != pPipe )
	{
		if( true == pPipe->IsFixed() && false == pclTable->IsHidden() && strID != m_pTechParam->GetDefaultPipeSerieID() )
		{
			m_ButtonSetAsDefaultPipeSeries.EnableWindow( TRUE );
			return;
		}
	}

	m_ButtonSetAsDefaultPipeSeries.EnableWindow( FALSE );

}

void CDlgCustPipe::_FillUsedPipeSeriesList( CTable *pclTable )
{
	if( NULL == pclTable )
	{
		return;
	}

	// Cannot hide STEEL_GEN this pipe series is used as default pipe series
	// See CPipes::SelectBestPipe()
	if( NULL == m_UsedPipeSeries.Find( _T("STEEL_GEN") ) )
	{
		m_UsedPipeSeries.AddHead( _T("STEEL_GEN") );
	}

	for( IDPTR IDPtr = pclTable->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pclTable->GetNext() )
	{
		CDS_HydroMod *pHM = (CDS_HydroMod *)( IDPtr.MP );

		for( int i = 0; i < 4; i++ )
		{
			CPipes *pPipes = NULL;

			switch( i )
			{
				case 0:
					pPipes = pHM->GetpCircuitPrimaryPipe();
					break;

				case 1:
					pPipes = pHM->GetpCircuitSecondaryPipe();
					break;

				case 2:
					pPipes = pHM->GetpDistrSupplyPipe();
					break;

				case 3:
					pPipes = pHM->GetpDistrReturnPipe();
					break;
			}

			// If pipe exists...
			if( NULL != pPipes && pPipes->GetLength() >= 0.0 && NULL != pPipes->GetIDPtr().MP )
			{
				CString PipeSeriesID = pPipes->GetIDPtr().PP->GetIDPtr().ID;

				if( NULL == m_UsedPipeSeries.Find( PipeSeriesID ) )
				{
					m_UsedPipeSeries.AddHead( PipeSeriesID );
				}
			}
		}

		if( true == pHM->IsaModule() )
		{
			_FillUsedPipeSeriesList( (CTable *)pHM );
		}
	}
}

CDlgCustPipe::eIsLockedBy CDlgCustPipe::_CanHidePipeSeries( IDPTR IDPtrSerie )
{
	// Cannot hide default pipe series.
	if( 0 == StringCompare( IDPtrSerie.ID, m_pTechParam->GetDefaultPipeSerieID() ) )
	{
		return CDlgCustPipe::eIsLockedBy::eTechParamDefaultPipe;
	}

	// Cannot hide used pipe series.
	if( ( POSITION )0 != m_UsedPipeSeries.Find( IDPtrSerie.ID ) )
	{
		CDlgCustPipe::eIsLockedBy eReturn = CDlgCustPipe::eIsLockedBy::eUnlocked;

		if( 0 == StringCompare( IDPtrSerie.ID, _T( "STEEL_GEN" ) ) )
		{
			eReturn = CDlgCustPipe::eIsLockedBy::eMainPipe;
		}
		else
		{
			eReturn = CDlgCustPipe::eIsLockedBy::eHydraulicNetwork;
		}

		return eReturn;
	}
	
	return CDlgCustPipe::eIsLockedBy::eUnlocked;
}

bool CDlgCustPipe::_CheckPipeUsed( IDPTR IDPtrSerie, IDPTR IDPtrSize, CTable *pclTable )
{
	bool fSamePipeFound = false;

	if( NULL == pclTable )
	{
		return false;
	}

	for( IDPTR IDPtr = pclTable->GetFirst(); '\0' != *IDPtr.ID && false == fSamePipeFound; IDPtr = pclTable->GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );
		ASSERT( NULL != pHM );

		if( true == pHM->IsPipeUsed( IDPtrSerie, IDPtrSize ) )
		{
			return true;
		}

		if( true == pHM->IsaModule() )
		{
			fSamePipeFound = _CheckPipeUsed( IDPtrSerie, IDPtrSize, (CTable *)pHM );
		}
	}

	return fSamePipeFound;
}

void CDlgCustPipe::_RefreshPipeUsed( CTable *pclTable )
{
	if( NULL == pclTable )
	{
		return;
	}

	for( IDPTR IDPtr = pclTable->GetFirst(); '\0' != *IDPtr.ID; IDPtr = pclTable->GetNext() )
	{
		CDS_HydroMod *pHM = dynamic_cast<CDS_HydroMod *>( IDPtr.MP );
		ASSERT( NULL != pHM );
		
		pHM->UpdateAllPipesIDPtr();

		if( true == pHM->IsaModule() )
		{
			_RefreshPipeUsed( (CTable *)pHM );
		}
	}
}

void CDlgCustPipe::_SaveDefaultPipeSeries( CString strPipeID )
{
	// Update default pipe series ID.
	m_pTechParam->SetDefaultPipeSerieID( ( LPCTSTR )strPipeID );

	// Set the new default pipe series for the current project too.
	CPrjParams *pPrjParams = TASApp.GetpTADS()->GetpProjectParams()->GetpHmCalcParams();
	ASSERT( NULL != pPrjParams );

	// HYS-1590: In the 'OnCancel' method, we delete the current PipeUserDatabase of HySelect and we copy the backup.
	// So the MP pointers of IDPTR in "PrjParams" become invalid. We must force to update even if the ID stay the same
	pPrjParams->SetPrjParamID( CPrjParams::PipeCircSerieID, ( LPCTSTR )strPipeID, true );
	pPrjParams->SetPrjParamID( CPrjParams::PipeDistSupplySerieID, ( LPCTSTR )strPipeID, true );
	pPrjParams->SetPrjParamID( CPrjParams::PipeDistReturnSerieID, ( LPCTSTR )strPipeID, true );
}

void CDlgCustPipe::_SetState( void )
{
	CWnd *pCurrentFocus = GetFocus();

	if( pCurrentFocus->GetSafeHwnd() == m_Tree.GetSafeHwnd() )
	{
		HTREEITEM hItem = m_Tree.GetSelectedItem();

		if( NULL == hItem )
		{
			return;
		}

		CTable *pPipeSeries = (CTable*)m_Tree.GetItemData( hItem );

		if( NULL == pPipeSeries )
		{
			return;
		}

		IDPTR IDPtr = pPipeSeries->GetFirst( CLASS( CDB_Pipe ) );

		if( NULL == ( (CDB_Thing *)IDPtr.MP ) )
		{
			// Series is empty.
			_SetButtons( ButtonType::Pipe, ButtonType::Series, true, true );
		}
		else if( true == ( (CDB_Thing *)IDPtr.MP )->IsFixed() )
		{
			// Button 'Create series' enabled, 'Delete series' disabled.
			_SetButtons( ButtonType::Series, ButtonType::Series, true, false );
		}
		else
		{
			_SetButtons( ButtonType::Series, ButtonType::Series, true, true );
		}
	}
}

void CDlgCustPipe::_CreateSeries( void )
{
	try
	{
		// Create new name for this series.
		CString str = TASApp.LoadLocalizedString( IDS_CUSTPIPE_NEWSERIES );

		if( false == _FindFirstFreeName( NULL, &str ) )
		{
			return;
		}

		m_bModified = true;
		CString strID = BuildPipeID( true );
		
		IDPTR PipeSeriesIDPtr = _NULL_IDPTR;
		TASApp.GetpPipeDB()->CreateObject( PipeSeriesIDPtr, CLASS( CTable ), (LPCTSTR)strID );

		CTable *pPipeSeriesTab = (CTable *)( PipeSeriesIDPtr.MP );
		pPipeSeriesTab->SetName( str );
		
		CTable *pPipeTab = TASApp.GetpPipeDB()->GetPipeTab();
		
		if( NULL == pPipeTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'PIPE_TAB' table from the pipe database.") );
		}
		
		pPipeTab->Insert( PipeSeriesIDPtr );
		
		_ResetAll( &str );
		
		_SetButtons( ButtonType::Pipe, ButtonType::Series, true, true );
		_CreatePipe();
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgCustPipe::_CreateSeries'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgCustPipe::_CreatePipe( void )
{
	try
	{
		HTREEITEM hItem = m_Tree.GetSelectedItem();

		if( NULL == hItem )
		{
			return;
		}

		LPARAM lp = m_Tree.GetItemData( hItem );

		// Create a pipe in current series.
		// Take "Steel Gen 25" as reference.
		CDB_Pipe *pPipeRef = (CDB_Pipe *)( TASApp.GetpPipeDB()->Get( _T("STEEL_GEN_25") ).MP );
		
		if( NULL == pPipeRef )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'STEEL_GEN_25' pipe from the pipe database.") );
		}

		CString ID = BuildPipeID();

		IDPTR PipeIDPtr;
		TASApp.GetpPipeDB()->CreateObject( PipeIDPtr, CLASS( CDB_Pipe ), (LPCTSTR)ID );

		CDB_Pipe *pPipe = (CDB_Pipe *)( PipeIDPtr.MP );

		m_dIntDiameter = pPipeRef->GetIntDiameter();
		m_dRoughness = pPipeRef->GetRoughness();
		m_dPressure = pPipeRef->GetPmaxmax();
		m_dTemperature = pPipeRef->GetTmax();

		// Save Parameters to new pipe.
		pPipe->Unfix();
		
		// Create new name for this series.
		CString str = TASApp.LoadLocalizedString( IDS_CUSTPIPE_NEWPIPE );

		if( false == _FindFirstFreeName( &m_ListPipe, &str ) )
		{
			return;
		}

		m_bModified = true;
		pPipe->SetName( (LPCTSTR)str );
		pPipe->SetIntDiameter( m_dIntDiameter );
		pPipe->SetRoughness( m_dRoughness );
		pPipe->SetTmax( m_dTemperature );
		pPipe->SetTmin( _PIPE_TMIN_INIT );
		pPipe->SetPmaxmax( m_dPressure );
		pPipe->SetPmaxmin( m_dPressure );
		pPipe->SetSizeID( pPipeRef->GetSizeID() );
		pPipe->SetAvailable( true );
		pPipe->SetHidden( false );
		pPipe->SetDeleted( false );

		( (CTable *)lp )->Insert( PipeIDPtr );

		// Fill internal diameter, roughness, pressure and temperature fields.
		m_EditDiameter.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIAMETER, m_dIntDiameter ), 4, 0, true ) );
		m_EditRoughness.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_ROUGHNESS, m_dRoughness ), 4, 0, true ) );
		m_EditPressure.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_PRESSURE, m_dPressure ), 4, 0, true ) );
		m_EditTemp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_dTemperature ), 4, 0, true ) );

		_FillListPipe( (CTable *)lp );

		// Set focus on last created pipe.
		LVFINDINFO	lvFindInfo;
		lvFindInfo.flags = LVFI_STRING;
		lvFindInfo.vkDirection = VK_NEXT;
		lvFindInfo.psz = (LPCTSTR)str;
		m_ListPipe.SetFocus();
		m_ListPipe.SetItemState( m_ListPipe.FindItem( &lvFindInfo, -1 ), LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgCustPipe::_CreatePipe'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgCustPipe::_DeleteSeries( void )
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	// If no series is selected...
	if( NULL == hItem )
	{
		return;
	}

	CString str;
	FormatString( str, AFXMSG_DELETESERIES, m_Tree.GetItemText( hItem ) );

	if( IDYES != AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) )
	{
		return;
	}

	// Abort the deletion if the current pipe series is in use.
	IDPTR IDPtrSeries = ( (CTable *)m_Tree.GetItemData( hItem ) )->GetIDPtr();

	CTADatastruct *pTADS = TASApp.GetpTADS();
	
	CTable *pTab = pTADS->GetpHydroModTable();
	ASSERT( NULL != pTab );
	
	CTable *pTabHub = pTADS->GetpHUBSelectionTable();
	ASSERT( NULL != pTabHub );

	// HYS-1499 : We can't delete a pipe serie when it's used as defaultpipeserie in techparam.
	if( true == _CheckPipeUsed( IDPtrSeries, _NULL_IDPTR, pTab ) || true == _CheckPipeUsed( IDPtrSeries, _NULL_IDPTR, pTabHub ) 
		|| IDPtrSeries == m_pTechParam->GetDefaultPipeSerieIDPtr() )
	{
		// Do not allow deleting of the pipe series.
		CString str;
		str.Format( TASApp.LoadLocalizedString( AFXMSG_PIPESERIE_USED ), dynamic_cast<CTable *>( IDPtrSeries.MP )->GetName() );
		str += _T("\r\n") + TASApp.LoadLocalizedString( AFXMSG_PIPE_USED_GENMSG );
		AfxMessageBox( str );
		return;
	}

	m_bModified = true;

	// Delete all pipes.
	while( m_ListPipe.GetItemCount() > 0 )
	{
		if( _DeleteSelPipe( 0 ) < 0 )
		{
			break;
		}

		m_ListPipe.DeleteItem( 0 );
	}

	// If all pipes are deleted...
	if( 0 == m_ListPipe.GetItemCount() )
	{
		LPARAM lp = m_Tree.GetItemData( hItem );
		ASSERT( NULL != lp );

		if( NULL == lp )
		{
			return;
		}

		IDPTR IDPtrSeries = ( (CTable *)lp )->GetIDPtr();
		ASSERT( '\0' != *IDPtrSeries.ID );
		CTable *pPipeTab = TASApp.GetpPipeDB()->GetPipeTab();
		ASSERT( NULL != pPipeTab );
		pPipeTab->Remove( IDPtrSeries );
		VERIFY( TASApp.GetpPipeDB()->DeleteObject( IDPtrSeries ) );
	}

	_ResetAll( NULL );
}

void CDlgCustPipe::_DeletePipe( void )
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();

	// If no series is selected...
	if( NULL == hItem )
	{
		return;
	}

	int iPipe = m_ListPipe.GetNextItem( -1, LVNI_SELECTED );

	if( iPipe < 0 )
	{
		return;
	}

	CString str;
	FormatString( str, AFXMSG_DELETEPIPE, m_ListPipe.GetItemText( iPipe, 0 ) );

	if( IDYES != AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION, 0 ) )
	{
		return;
	}

	// Abort the deletion if the current pipe series is in use.
	IDPTR IDPtrSize = ( (CTable *)m_ListPipe.GetItemData( iPipe ) )->GetIDPtr();
	IDPTR IDPtrSeries = ( (CTable *)m_Tree.GetItemData( hItem ) )->GetIDPtr();

	CTADatastruct *pTADS = TASApp.GetpTADS();
	
	CTable *pTab = pTADS->GetpHydroModTable();
	ASSERT( NULL != pTab );
	
	CTable *pTabHub = pTADS->GetpHUBSelectionTable();
	ASSERT( NULL != pTabHub );

	if( true == _CheckPipeUsed( IDPtrSeries, IDPtrSize, pTab ) || true == _CheckPipeUsed( IDPtrSeries, IDPtrSize, pTabHub ) )
	{
		// Do not allow deleting of the pipe series.
		CString str;
		str.Format( TASApp.LoadLocalizedString( AFXMSG_PIPESIZE_USED ), dynamic_cast<CDB_Pipe *>( IDPtrSize.MP )->GetName() );
		str += _T("\r\n") + TASApp.LoadLocalizedString( AFXMSG_PIPE_USED_GENMSG );
		AfxMessageBox( str );
		return;
	}

	m_bModified = true;
	_DeleteSelPipe( iPipe );
	_ResetAll( &m_Tree.GetItemText( hItem ) );
}
